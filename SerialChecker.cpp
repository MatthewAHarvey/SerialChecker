#include "SerialChecker.h"

SerialChecker::SerialChecker(){
    message = new char[msgMaxLen];
    this->HSerial = &Serial;
}

SerialChecker::SerialChecker(uint16_t msgMaxLen, HardwareSerial& HSerial, uint32_t baudrate){
    this->msgMaxLen = msgMaxLen;
    this->HSerial = &HSerial;
    this->baudrate = baudrate;
    message = new char[msgMaxLen];
}

SerialChecker::~SerialChecker(){
    delete [] message;
}

void SerialChecker::init(){
    HSerial->begin(baudrate); 
    // HSerial->println("Connected to SerialChecker test.");
}

void SerialChecker::disableACKNAK(){
    useACKNAK = false;
}

void SerialChecker::enableACKNAK(){
    useACKNAK = true;
}

void SerialChecker::enableACKNAK(char ACK, char NAK){
    useACKNAK = true;
    this->ACK = ACK;
    this->NAK = NAK;
}

void SerialChecker::disableChecksum(){
    useChecksum = false;
}

void SerialChecker::enableChecksum(){
    useChecksum = true;
}

void SerialChecker::enableSTX(bool requireSTX){
    useSTX = true;
    this->requireSTX = requireSTX;
    if(requireSTX){
        receiveStarted = false;
    }
}

void SerialChecker::enableSTX(bool requireSTX, char STX){
    useSTX = true;
    this->requireSTX = requireSTX;
    if(requireSTX){
        receiveStarted = false;
    }
    this->STX = STX;
}

void SerialChecker::disableSTX(){
    useSTX = false;
    requireSTX = false;
}

void SerialChecker::setETX(char ETX){
    this->ETX = ETX;
}

uint8_t SerialChecker::check(){
    while(HSerial->available()) {
        char in = HSerial->read();
        // HSerial->println(in);
        if(receiveStarted){
            if(useSTX && in == STX){
                msgIndex = 0;
                // HSerial->println("STX");
            }
            else if(in != ETX && msgIndex < msgMaxLen){
                //add to message
                message[msgIndex] = in;
                msgIndex++;
                // HSerial->println("Adding to message");
            }
            else if(in == ETX){
                // message complete so calculate the checksum and compare it
                // HSerial->println("ETX");
                message[msgIndex] = '\0';
                // HSerial->println(msgIndex);
                if(msgIndex >= msgMinLen){ // make sure message is long enough
                    // HSerial->println("Long enough");
                    if(useChecksum){
                        msgLen = msgIndex - 1;
                        char msgChecksum = message[msgLen];
                        message[msgLen] = '\0';
                        //HSerial->println(calcChecksum(message, msgLen));
                        if(msgChecksum == calcChecksum(message, msgLen)){
                            //parseMessage();
                            msgIndex = 0;
                            if(requireSTX){
                                receiveStarted = false;
                            }
                            return msgLen;
                        }
                        else if(useACKNAK){
                            HSerial->println(NAK);
                        }
                    }
                    else{
                        msgLen = msgIndex;
                        msgIndex = 0;
                        if(requireSTX){
                            receiveStarted = false;
                        }
                        return msgLen;
                    }
                }
                else if(useACKNAK){
                    HSerial->println(NAK);
                }
                // reset megIndex for next message
                msgIndex = 0;
            }
            else{
                // message too long so scrap it and start again.
                msgIndex = 0;
                // HSerial->println("Too long");
                if(useACKNAK){
                    HSerial->println(NAK);
                }
            } 
        }
        else{
            if(in == STX){
                receiveStarted = true;
            }
            else if(in == '\n'){
                if(useACKNAK){
                    HSerial->println(NAK);
                }
            }
        }
    }
    return 0;
}

char* SerialChecker::getMsg(){
    return message;
}

char* SerialChecker::getMsg(uint8_t startIndex){
    return &message[startIndex];
}

uint8_t SerialChecker::getLen(){
    return msgLen;
}

bool SerialChecker::contains(char* snippet, uint8_t startIndex){

}

bool SerialChecker::contains(const char* snippet){
    // check if the shippet is present starting at index startIndex.
    // snippet char array must be null terminated.
    const uint8_t startIndex = 0;
    int i = 0;
    const char* p = snippet;
    while(*p){
        if(*p != message[startIndex + i++]){
            return false;
        }
        p++;
    }
    return true;
}

char SerialChecker::calcChecksum(char* rawMessage, int len){
    uint16_t checksum=0;
    for(uint8_t i = 0; i < len; i++)
    { //add the command
        checksum += rawMessage[i];
    }
    //Calculate checksum based on MPS manual
    checksum = ~checksum+1; //the checksum is currently a unsigned 16bit int. Invert all the bits and add 1.
    checksum = 0x7F & checksum; // discard the 8 MSBs and clear the remaining MSB (B0000000001111111)
    checksum = 0x40 | checksum; //bitwise or bit6 with 0x40 (or with a seventh bit which is set to 1.) (B0000000001000000)
    return (char) checksum;
}

char SerialChecker::calcChecksum(char* rawMessage){
    uint16_t checksum=0;
    while(*rawMessage){
        checksum += *rawMessage;
        rawMessage++;
    }
    //Calculate checksum based on MPS manual
    checksum = ~checksum+1; //the checksum is currently a unsigned 16bit int. Invert all the bits and add 1.
    checksum = 0x7F & checksum; // discard the 8 MSBs and clear the remaining MSB (B0000000001111111)
    checksum = 0x40 | checksum; //bitwise or bit6 with 0x40 (or with a seventh bit which is set to 1.) (B0000000001000000)
    return (char) checksum;
}

float SerialChecker::toFloat(uint8_t startIndex){
    // Returns the number stored in a char array, starting at startIndex
    float number = 0;
    bool units = true; // deal with the units then the decimals
    int decimalN = 1;
    bool negative = false;
    if( message[startIndex] == '-'){
        negative = true;
        startIndex++;
    }
    for(int i = startIndex; i < msgLen; i++) 
    {
        if(message[i] == '.'){
            units = false;
        }
        else{
            if(units){
                number *= 10.0;
                number += float(message[i] -'0');
            }
            else{
                number += float(message[i] - '0') * pow(0.1, decimalN);
                decimalN++;
            }
        }
    }
    if(negative){
        number *= -1.0;
    }
    return number;
}

float SerialChecker::toFloat(){
    uint8_t number = 0;
    bool negative = false;
    uint8_t startIndex = 0;
    while(message[startIndex]){
        if((message[startIndex] == '-') || 
            (message[startIndex] >= '0' && message[startIndex] <= '9')){
            break;
        }
        startIndex++;
    }
    return toFloat(startIndex);
}

uint8_t SerialChecker::toInt8(uint8_t startIndex){
    // Returns the number stored in a char array, starting at startIndex
    uint8_t number = 0;
    bool negative = false;
    if( message[startIndex] == '-'){
        negative = true;
        startIndex++;
    }
    for(int i = startIndex; i < msgLen; i++) 
    {
        number *= 10;
        number += (message[i] -'0');
    }
    if(negative){
        number *= -1;
    }
    return number;
}

uint8_t SerialChecker::toInt8(){
    // Returns the number stored in a char array, starting at startIndex
    uint8_t number = 0;
    bool negative = false;
    uint8_t startIndex = 0;
    while(message[startIndex]){
        if((message[startIndex] == '-') || 
            (message[startIndex] >= '0' && message[startIndex] <= '9')){
            break;
        }
        startIndex++;
    }
    return toInt8(startIndex);
}

uint16_t SerialChecker::toInt16(uint8_t startIndex){
    // Returns the number stored in a char array, starting at startIndex
    uint16_t number = 0;
    bool negative = false;
    if( message[startIndex] == '-'){
        negative = true;
        startIndex++;
    }
    for(int i = startIndex; i < msgLen; i++) 
    {
        number *= 10;
        number += (message[i] -'0');
    }
    if(negative){
        number *= -1;
    }
    return number;
}

uint16_t SerialChecker::toInt16(){
    // Returns the number stored in a char array, starting at startIndex
    uint8_t startIndex = 0;
    while(message[startIndex]){
        if((message[startIndex] == '-') || 
            (message[startIndex] >= '0' && message[startIndex] <= '9')){
            break;
        }
        startIndex++;
    }
    return toInt16(startIndex);
}

// uint16_t SerialChecker::toInt16(){
//     // Returns the number stored in a char array, starting at startIndex
//     uint16_t number = 0;
//     bool negative = false;
//     uint8_t startIndex = 0;
//     while(message[startIndex]){
//         if((message[startIndex] == '-') || 
//             (message[startIndex] >= '0' && message[startIndex] <= '9')){
//             break;
//         }
//         startIndex++;
//     }
//     if( message[startIndex] == '-'){
//         negative = true;
//         startIndex++;
//     }
//     for(int i = startIndex; i < msgLen; i++) 
//     {
//         number *= 10;
//         number += (message[i] -'0');
//     }
//     if(negative){
//         number *= -1;
//     }
//     return number;
// }

uint32_t SerialChecker::toInt32(uint8_t startIndex){
    // Returns the number stored in a char array, starting at startIndex
    uint32_t number = 0;
    bool negative = false;
    if( message[startIndex] == '-'){
        negative = true;
        startIndex++;
    }
    for(int i = startIndex; i < msgLen; i++) 
    {
        number *= 10;
        number += (message[i] -'0');
    }
    if(negative){
        number *= -1;
    }
    return number;
}

uint32_t SerialChecker::toInt32(){
    // Returns the number stored in a char array, starting at startIndex
    uint32_t number = 0;
    bool negative = false;
    uint8_t startIndex = 0;
    while(message[startIndex]){
        if((message[startIndex] == '-') || 
            (message[startIndex] >= '0' && message[startIndex] <= '9')){
            break;
        }
        startIndex++;
    }
    return toInt32(startIndex);
}

void SerialChecker::sendACK(){
    HSerial->println(ACK);
}

void SerialChecker::sendNAK(){
    HSerial->println(NAK);
}