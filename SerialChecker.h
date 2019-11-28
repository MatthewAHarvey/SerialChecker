#ifndef SERIALCHECKER_H
#define SERIALCHECKER_H

#include<arduino.h>
#include<HardwareSerial.h>

/**
 * @brief      Borrowing from https://github.com/synfinatic/AnySerial to get USB and HardwareSerial working
 *              Need an enum to tell what type of serial is used
 */
enum class serialTypes{ USB, HardWare, ATMEGAXXU4 };

/**
 * @brief      Borrowing from https://github.com/synfinatic/AnySerial to get USB and HardwareSerial working
 *              Need this typedef thingy to tell which type of port is used.
 */
typedef union {
    HardwareSerial *hardware;
#ifdef USBserial_h_
    usb_serial_class *usb;
#endif
#ifdef USBCON
    Serial_ *atmegaXXu4;
#endif
} portType;

/**
 * @brief      Different types of checksum algorithm can be used. At the moment the choice is limited to just two simple ones that only produce a limited set of printable chars.
 */
enum class checksumTypeEnum{ SpellmanMPS, Readable8bitChars };
// enum class charNumTypeEnum{ NaN, DecPoint, MinusSign, Integer };
/**
 * @brief      SerialChecker is an Arduino based class for the easy handling of serial messages.
 *              SerialChecker can be used to check incoming messages  
 */
class SerialChecker{
public:
    // SerialChecker(); // defaults to message of length 13, Serial and baudrate of 250000 
    SerialChecker(uint16_t msgMaxLen, HardwareSerial& HSerial, uint32_t baudrate);
    SerialChecker(HardwareSerial& port);
    // SerialChecker(); // defaults to message of length 13, Serial and baudrate of 250000 
    SerialChecker(HardwareSerial& port, uint32_t baudrate);
    #ifdef USBserial_h_
    SerialChecker(usb_serial_class& port);
    SerialChecker(usb_serial_class& port, uint32_t baudrate);
    #endif
    #ifdef USBCON // I think this is for atmega32u4 based arduinos
    SerialChecker(Serial_& port);
    SerialChecker(Serial_& port, uint32_t baudrate);
    #endif
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
    bool contains(char* snippet);
    bool contains(const char& c, uint8_t startIndex);
    bool contains(const char& c);
    char calcChecksum(char* rawMessage, int len);
    char calcChecksum(char* rawMessage);
    char chksmSpellmanMPS(char* rawMessage, int len);
    char chksmSpellmanMPS(char* rawMessage);
    char chksm8bitAllReadableChars(char* rawMessage, int len);
    char chksm8bitAllReadableChars(char* rawMessage);
    // void setCheckConversion(bool checkConversion);
    // bool getCheckConversion();
    // charNumTypeEnum getNumType(const char& c);
    // bool isNumChar(const char& c);
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
    serialTypes serialType;
    portType port;

    bool useChecksum = false;
    checksumTypeEnum checksumType = checksumTypeEnum::Readable8bitChars; 
    bool useAckNak = false;
    bool useSTX = false;
    bool requireSTX = false;
    bool receiveStarted = true;
    bool allowCR = false;
    // bool checkConversion = false;
    uint8_t msgMinLen = 1;
    uint8_t msgMaxLen = 13;
    char STX = '$';
    char ETX = '\n'; 
    char Ack = 'A';//6; Acknowledge char
    char Nak = 'N';//21; Not Acknowledge char
    uint8_t msgIndex;
    uint8_t msgLen;
    char* message = nullptr; // message excluding the address section, if present
    char* rawMessage = nullptr; // the full message including the address section, if present
    uint8_t rawMsgLen;
    uint8_t addressLen = 0;
    char* address = nullptr;

    #ifdef USBserial_h_
    uint8_t checkUSBSerial();
    #endif
    #ifdef USBCON
    uint8_t checkATMEGAXXU4Serial();
    #endif
    uint8_t checkHardwareSerial();
    

};

#endif