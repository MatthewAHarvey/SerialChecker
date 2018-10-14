# SerialChecker

SerialChecker is an Arduino based class for the easy handling of serial messages between an Arduino and another device such as a PC. At the moment, it only works with the hardware serial port on an arduino such as Serial. That is, just Serial on an Arduino Uno and Serial1, Serial2 or Serial3 on an Arduino Mega etc.

### Features

1. Checks received message length is between a minimum and maximum number of chars
2. Non-blocking checking code.
3. Choose the end of message char. Default is '\n'.
4. Choose optional start of message char such as '$'. Not required by default.
5. Send acknowledge and not acknowledge (ACK and NAK) messages. NAK messages can be automatically sent if the received message does not meet validity requirements.
6. Can handle checking for valid checksums at end of received messages. Not required by default. The checksum function can be used to calculated a checsum for outgoing messages as well.
7. Can check for the presence of char arrays in the incoming message making it simple for the user to decide how to handle the message.
8. Can convert chosen parts of the message to floats and both signed and unsigned 8, 16 and 32 bit integers.

### Installation

For now, just place SerialChecker.h and SerialChecker.cpp in the same folder as the arduino .ino file and type `#include "SerialChecker.h"` at the top of the file.

Documentation is available at: https://matthewaharvey.github.io/SerialChecker/html/class_serial_checker.html

### Examples

This first example starts a SerialChecker instance (`sc`) and uses it to control the brightness of the builtin LED on pin 13's PWM mode. In the loop, `sc.check()` returns the message length if a message is received. 

`sc.contains(char* command)` is then used to check if `command` string is present at the start of the received message. If the start of the message is "ON", pin 13's LED is switched on. If the start of the message is "OFF", pin 13's LED is switched off. 

If the first char is 'B', then `sc.int8()` is used to convert the rest of the message to a uint8_t value that is then used to set the brightness of the LED on pin 13. 

`sc.int8()` was not given an argument so it looks through the received message until a minus sign or an integer number is found. The function then converts the rest of the message into an int8. This works for both signed and unsigned values as long as the container is of the correct type. In this case, `uint8_t brightness` is used.

>#include "SerialChecker.h"
>
>SerialChecker sc;
>
>void setup(){
>    sc.init();
>    Serial.println("Connected to SerialCheckerExample.ino");
>    pinMode(13, OUTPUT);
>}
>
>void loop(){
>    if(sc.check()){
>        if(sc.contains("ON")){
>            analogWrite(13, 255);
>        }
>        else if(sc.contains("OFF")){}
>            analogWrite(13, 0);
>        }
>        else if(sc.contains("B")){}
>            uint8_t brightness = sc.int8();
>            analogWrite(13, brightness);
>        }
>    }
>    
>    // Do other stuff...
>}

#### Basic usage

This shows how to 