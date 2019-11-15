# SerialChecker

SerialChecker is an arduino based class for the easy handling of serial messages between an Arduino and another device such as a PC or another arduino. At the moment, it only works with the hardware serial port on an arduino such as Serial. That is, just Serial on an Arduino Uno and Serial1, Serial2 or Serial3 on an Arduino Mega etc.

### Features

1. Checks received message length is between a minimum and maximum number of chars
2. Non-blocking checking code. The arduino can be busy doing other things rather than waiting for a message to finish being sent.
3. Choose the end of message char. Default is '\n'.
4. Choose optional start of message char such as '$'. Not required by default.
5. Send acknowledge and not acknowledge (ACK and NAK) messages. NAK messages can be automatically sent if the received message does not meet validity requirements.
6. Can handle checking for valid checksums at end of received messages. Not required by default. The checksum function can be used to calculated a checksum for outgoing messages as well.
7. Can check for the presence of char arrays in the incoming message making it simple for the user to decide how to handle the message. For example, check for presence of "ID" to know the user wants to know which arduino is connected to COM3...
8. Can convert chosen parts of the message to floats and both signed and unsigned 8, 16 and 32 bit integers.

### Typical usage

This code was written for use in a research laboratory where most of the hardware is controlled by arduino compatible microcontroller boards. Most of the hardware, such as power supplies and detectors are made inhouse. Arduinos are used for things like setting voltages and temperatures, getting readings and controlling stepper motors. Serial messages are transferred between the arduinos and monitoring and controlling computers, mostly running LabVIEW control systems.

 Asynchronous serial communications are susceptible to noise or timing issues which can result in garbled messages being received or messages missed entirely. In these uses, especially when setting voltages or currents on sensitive equipment, a garbled serial message could cause catastrophic damage. For example, accidentally setting a high voltage on a detector power supply would 'spike' the device with enough charge to destroy it.

If a message is sent to the arduino along with a checksum, errors are significantly reduced. The arduino can send back NAK (not acknowledged) chars in the case that a valid message was not received, thereby alerting the sender to resend etc. Sending ACK messages lets the sender know the message was successfully received as well. This serial library handles these cases, provides an easy way to recognise commands and interprete ascii messages in to floats and ints.

### Installation

For now, just place SerialChecker.h and SerialChecker.cpp in the same folder as the arduino .ino file and type `#include "SerialChecker.h"` at the top of the file.

Documentation is available at: https://matthewaharvey.github.io/SerialChecker/html/class_serial_checker.html

### Examples

This first example starts a SerialChecker instance (`sc`) and uses it to control the brightness of the builtin LED on pin 13's PWM mode. In the loop, `sc.check()` returns the message length if a message is received. 

`sc.contains(char* command)` is then used to check if `command` string is present at the start of the received message. If the start of the message is "ON", pin 13's LED is switched on. If the start of the message is "OFF", pin 13's LED is switched off. 

If the first char is 'B', then `sc.int8()` is used to convert the rest of the message to a uint8_t value that is then used to set the brightness of the LED on pin 13. 

`sc.int8()` was not given an argument so it looks through the received message until a minus sign or an integer number is found. The function then converts the rest of the message into an int8. This works for both signed and unsigned values as long as the container is of the correct type. In this case, `uint8_t brightness` is used.

```
#include "SerialChecker.h"

SerialChecker sc;

void setup(){
    sc.init();
    sc.println("Connected to SerialCheckerExample.ino");
    pinMode(13, OUTPUT);
}

void loop(){
    if(sc.check()){
        if(sc.contains("ON")){
            analogWrite(13, 255);
        }
        else if(sc.contains("OFF")){}
            analogWrite(13, 0);
        }
        else if(sc.contains("B")){}
            uint8_t brightness = sc.int8();
            analogWrite(13, brightness);
        }
    }
    
    // Do other stuff...
}
```

### Checksum algorithm

A standard 8 bit checksum sums all of the chars in a message in to a byte. If the sum is greater than 255 (2^8 - 1), it just wraps around. The checksum is then the negative of this number. In processors that use [two's complement](https://www.cs.cornell.edu/~tomf/notes/cps104/twoscomp.html) signed numbers (the way to represent positive and negative integers), this is done by flipping all the bits and adding 1. Or you can just write `checksum = -checksum` and have the compiler do it for you.

So to send a message with a checksum, compute the checksum for the message to be sent, append it to the message char array and send it. To check if a received message is valid, sum all of the chars in the received message. If the value is 0, then the message passed the checksum. Why does this work? The value of the checksum is the negative of the sum value of the rest of the message. 

This technique works well for clocked serial messages but not so well with ascii serial messages that are typically used by arduinos and lab equipment. In these cases, it is common for messages to end with a newline '\n' or other chosen char. A char is a byte, i.e., number between 0 and 255, and these numbers are mapped to a long established [ascii table](https://www.rapidtables.com/code/text/ascii-table.html). In this mapping, the newline '\n' char is the integer number 10, 0x0A in hexadecimal and 0b00001010 in a binary byte. If the simple checksum algoritm described above is used, then there is nothing stopping it producing the '\n' char as the computed checksum. When the arduino receives this, it would think that the message has ended and that the previously received char was the checksum. This would prevent the message being received successfully.

A simple way around this is to restrict the range of char values that the checksum calculation can produce. In SerialChecker, there are two implemented algorithms to do this:
1. The first is based on that used by Spellman MPS high voltage power supplies. See the[Spellman MPS Digital Interface manual](https://www.spellmanhv.com/-/media/en/Products/MPS-Digital-Interface.pdf) for details. This algorithm produces 64 unique checksum chars and none of these overlap with '\n' or the start and end chars used by those power supplies to signify the start and end of their serial messages.
2. The second method sums all the message chars in to a byte before clearing the most significant bit. This has the effect of limiting the sum to between 0 and 127. This is then further restricted to the range of 33 to 126. This range is the entire range of chars from '!' to '~' that is printable by the arduino IDE. This range is chosen to aid debugging and the manual entry of checksums when communicating with the arduino via the arduino IDE's limited serial monitor. 

When either of these algorithms are used, the sending device must append the calculated checksum. The receiving device splits the last received char (the checksum) off from the message, recalculates the checksum from the received message and then compares it to the received checksum. If it matches, then the message is valid as far as the checksum can tell.

The first method can produce 64 checksum values. The second method can produce 94 checksum values so should be preferred (unless the user happens to have a Spellman MPS power supply!).

Why not use something like [Fletcher's checksum](https://en.wikipedia.org/wiki/Fletcher%27s_checksum) or an [8 bit CRC](https://en.wikipedia.org/wiki/Cyclic_redundancy_check) (Cyclic Redundancy Check)? They will produce chars that will be confused with the end char (default is '\n'). Why not use a 16 bit method for increased resilience against errors? Oer several years of using the above checksum algorithms in a physics experimental laboratory, they have proved sufficiently robust. 

#### C++ implementation of the second algorithm

```
char chksm8bitAllReadableChars(char* rawMessage){
    uint8_t checksum=0;
    while(*rawMessage){ // this relies on the char array being NULL terminated!
        checksum += *rawMessage;
        rawMessage++;
    }
    checksum &= 0x7F; // clear the MSB (0b1111111) so that the number can only be between 0 and 127.
    checksum += 33; // 33 is the minimum printable char '!'
    if(checksum > 126){
        checksum -= 94; // 126 is the last readable char '~', so wrap back to the first printable char by subtracting 94.
    }
    return (char) checksum;
}
```

A version that works in python could be:
```
def chksm8bitAllReadableChars(message: str) -> str:
    checksum = 0
    for c in message:
        checksum += ord(c) # convert string char to int
    checksum &= 127 #0x7F 0b1111111
    checksum += 33 # first readable char is '!'
    if(checksum > 126): # last readable char is '~'
        checksum -= 94
    return chr(checksum) # convert int back to string char
```

### To do:

1. Prevent errors in conversion of floats and ints so that they stop conversion if a non numeric char is received.
2. Add more examples and an example code folder.
3. Style as real arduino library with command highlighting in the arduino ide.
4. Submit to arduino library list thingy.
5. Add option for all printable character checksum.
