#include "SerialChecker.h"

SerialChecker sc;

void setup(){
    sc.init();
    sc.enableACKNAK('%', '*');
    sc.enableSTX(false, '£');
}

void loop(){
    delay(100);
    int len = sc.check();
    if(len){
        Serial.print(sc.getLen());
        Serial.print(", ");
        Serial.print(len);
        Serial.print(", ");
        Serial.println(sc.getMsg());
        if(sc.contains("TEST")){
            Serial.println("contains TEST");
        }
        else if(sc.contains("U")){
            uint16_t num = sc.toInt16(1);
            Serial.println(num);
        }
        else if(sc.contains("I")){
            int16_t num = sc.toInt16(); // don't need to specify a start index!
            Serial.println(num);
        }
        else if(sc.contains("F")){
            Serial.println(sc.toFloat());
        }
        else if(sc.contains("Calc")){
            Serial.println(sc.calcChecksum(sc.getMsg()));
        }
        else if(sc.contains("M")){
            Serial.println(sc.getMsg(3));
        }
        else if(sc.contains("EE")){
            sc.sendACK();
            sc.sendNAK();
        }
    }
}