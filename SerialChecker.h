#ifndef SERIALCHECKER_H
#define SERIALCHECKER_H

#include<arduino.h>
#include<HardwareSerial.h>

/**
 * @brief      SerialChecker is an Arduino based class for the easy handling of serial messages.
 *              SerialChecker can be used to check incoming messages  
 */
class SerialChecker{
private:
    uint32_t baudrate = 250000;
    HardwareSerial* HSerial;
    bool useChecksum = false;
    bool useACKNAK = false;
    bool useSTX = false;
    bool requireSTX = false;
    bool receiveStarted = true;
    uint8_t msgMinLen = 2;
    uint8_t msgMaxLen = 13;
    char STX = '$';
    char ETX = '\n'; 
    char ACK = 'A';//6; Acknowledge char
    char NAK = 'N';//21; Not Acknowledge char
    uint8_t msgIndex;
    uint8_t msgLen;
    char* message;

public:
    SerialChecker(); // defaults to message of length 13, Serial and baudrate of 250000 
    SerialChecker(uint16_t msgMaxLen, HardwareSerial& HSerial, uint32_t baudrate);
    ~SerialChecker();
    void init();
    void disableACKNAK();
    void enableACKNAK();
    void enableACKNAK(char ACK, char NAK);
    void disableChecksum();
    void enableChecksum();
    void enableSTX(bool requireSTX);
    void enableSTX(bool requireSTX, char STX);
    void disableSTX();
    void setETX(char ETX);
    uint8_t check();
    char* getMsg();
    char* getMsg(uint8_t startIndex);
    uint8_t getMsgLen();
    void setMsgMinLen(uint8_t msgMinLen);
    bool contains(char* snippet, uint8_t startIndex);
    bool contains(const char* snippet);
    char calcChecksum(char* rawMessage, int len);
    char calcChecksum(char* rawMessage);
    float toFloat(uint8_t startIndex);
    float toFloat();
    uint8_t toInt8(uint8_t startIndex);
    uint8_t toInt8();
    uint16_t toInt16(uint8_t startIndex);
    uint16_t toInt16();
    uint32_t toInt32(uint8_t startIndex); // reads until end of message
    uint32_t toInt32(); // reads from first numeric or minus sign
    void sendACK(); // sends an acknowledge char
    void sendNAK(); // sends a not acknowledge char
};

#endif