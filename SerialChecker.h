#ifndef SERIALCHECKER_H
#define SERIALCHECKER_H

#include<arduino.h>
#include<HardwareSerial.h>


/**
 * @brief      Different types of checksum algorithm can be used. At the moment the choice is limited to just two simple ones that only produce a limited set of printable chars.
 */
enum class checksumTypeEnum{ SpellmanMPS, Readable8bitChars };

/**
 * @brief      SerialChecker is an Arduino based class for the easy handling of serial messages.
 *              SerialChecker can be used to check incoming messages  
 */
class SerialChecker{
public:
    SerialChecker(); // defaults to message of length 13, Serial and baudrate of 250000 
    SerialChecker(uint16_t msgMaxLen, HardwareSerial& HSerial, uint32_t baudrate);
    ~SerialChecker();
    void init();
    void disableAckNak();
    void enableAckNak();
    void enableAckNak(char Ack, char Nak);
    void disableChecksum();
    void enableChecksum();
    void setChecksumType(checksumTypeEnum checksumType);
    void enableSTX(bool requireSTX);
    void enableSTX(bool requireSTX, char STX);
    void disableSTX();
    void setETX(char ETX);
    void setAllowCR(bool allowCR);
    bool getAllowCR();
    uint8_t check();
    char* getAddress();
    char* getRawMsg();
    uint8_t getRawMsgLen();
    char* getMsg();
    char* getMsg(uint8_t startIndex);
    uint8_t getMsgLen();
    void setMsgMinLen(uint8_t msgMinLen);
    void setMsgMaxLen(uint8_t msgMaxLen);
    void setAddressLen(uint8_t len);
    uint8_t getAddressLen();
    bool contains(char* snippet, uint8_t startIndex);
    bool contains(const char* snippet);
    char calcChecksum(char* rawMessage, int len);
    char calcChecksum(char* rawMessage);
    char chksmSpellmanMPS(char* rawMessage, int len);
    char chksmSpellmanMPS(char* rawMessage);
    char chksm8bitAllReadableChars(char* rawMessage, int len);
    char chksm8bitAllReadableChars(char* rawMessage);
    float toFloat(uint8_t startIndex);
    float toFloat();
    uint8_t toInt8(uint8_t startIndex);
    uint8_t toInt8();
    uint16_t toInt16(uint8_t startIndex);
    uint16_t toInt16();
    uint32_t toInt32(uint8_t startIndex); // reads until end of message
    uint32_t toInt32(); // reads from first numeric or minus sign
    void sendAck(); // sends an acknowledge char
    void sendNak(); // sends a not acknowledge char
    void print(char* message);
    void print(char c);
    void print(uint8_t n);
    void print(uint16_t n);
    void print(uint32_t n);
    void print(int8_t n);
    void print(int16_t n);
    void print(int32_t n);
    void print(float n);
    void print(double n);

    void println(char* message);
    void println(char c);
    void println(uint8_t n);
    void println(uint16_t n);
    void println(uint32_t n);
    void println(int8_t n);
    void println(int16_t n);
    void println(int32_t n);
    void println(float n);
    void println(double n);
private:
    uint32_t baudrate = 250000;
    HardwareSerial* HSerial;
    bool useChecksum = false;
    checksumTypeEnum checksumType = checksumTypeEnum::Readable8bitChars; 
    bool useAckNak = false;
    bool useSTX = false;
    bool requireSTX = false;
    bool receiveStarted = true;
    bool allowCR = false;
    
    uint8_t msgMinLen = 1;
    uint8_t msgMaxLen = 13;
    char STX = '$';
    char ETX = '\n'; 
    char Ack = 'A';//6; Acknowledge char
    char Nak = 'N';//21; Not Acknowledge char
    uint8_t msgIndex;
    uint8_t rawMsgLen;
    char* message; // message excluding the address section, if present
    char* rawMessage; // the full message including the address section, if present
    uint8_t addressLen = 0;
    char* address;
};

#endif