#include "SerialChecker.h"

SerialChecker sc(Serial);
SerialChecker sc1(Serial1);

void setup(){
    sc.init();
    sc1.init();
    sc.setAddressLen(1);
    Serial.println("Connected to SerialChecker.ino");
    // Can now print messages via the SerialChecker class instance itself:
    sc.println("Still connected to SerialChecker.ino...");
    // You can use Serial.print or the class instance method since they are the same. The advantage of using the class instance message becomes apparent when you have multiple serial comms targets.
    sc.setETX('\r');
    //sc.enableAckNak('%', '*');// Ack = '%', Nak = '*'. Nak is sent automatically if bad message received. Ack must be sent manually with sc.sendAck();. Nak can also be sent with sendNak();
    //sc.enableSTX(false, '£'); // STX use is not enforced but will be recognised as '£'. This does mean that '£' can't be used as a character in the message...
    
}

void loop(){
    delay(100);
    int len = sc.check();
    if(len){
        sc.print("Address: ");
        sc.println(sc.getAddress());
        sc.print(sc.getMsgLen());
        sc.print(", ");
        sc.print(len);
        sc.print(", ");
        sc.println(sc.getMsg());
        if(sc.contains("TEST")){
            sc.println("contains TEST");
        }
        else if(sc.contains("U")){
            uint16_t num = sc.toInt16(1);
            sc.println(num);
        }
        else if(sc.contains("I")){
            int16_t num = sc.toInt16(); // don't need to specify a start index!
            sc.println(num);
        }
        else if(sc.contains("F")){
            sc.println(sc.toFloat());
        }
        else if(sc.contains("Calc")){
            sc.println(sc.calcChecksum(sc.getRawMsg()));
        }
        else if(sc.contains("M")){
            sc.println(sc.getMsg());
        }
        else if(sc.contains("R")){
            sc.println(sc.getRawMsg());
        }
        else if(sc.contains("EE")){
            sc.sendAck();
            sc.sendNak();
        }
        else if(sc.contains("SC1")){
            sc.println("Checksum turned on.");
            sc.enableChecksum();
        }
        else if(sc.contains("SC0")){ 
        //the checksum is calculated for the rawMessage, i.e., the address is included in the calculation. 
        // If there is no address, SC0g will turn it off. If there is a 1 char address, 1SC0: will turn it off.
            sc.println("Checksum turned off.");
            sc.disableChecksum();
        }
    }
}