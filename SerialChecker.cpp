#include "SerialChecker.h"

/**
 * @brief      Constructs the object. Dynamically creates a char array to hold message buffers. Assigns the the default arduino serial port. 
 */
// SerialChecker::SerialChecker(){
//     this->serialType = serialTypes::HardWare;
//     this->port.hardware = &Serial;
//     rawMessage = new char[msgMaxLen];
//     message = rawMessage;
// }

/**
 * @brief      Constructs the object. Dynamically creates a char array to hold message buffers. Assigns the the default arduino serial port. 
 */
// SerialChecker::SerialChecker(){
//     message = new char[msgMaxLen];
//     // this->HSerial = &Serial;
//     this->serialType = serialTypes::HardWare;
//     this->port.hardware = &Serial;
// }
SerialChecker::SerialChecker(HardwareSerial& port){
    serialType = serialTypes::HardWare;
    this->port.hardware = &port;
    rawMessage = new char[msgMaxLen];
    message = rawMessage;
}

/**
 * @brief      Constructs the object. As above but lets user choose serial port and baudrate.
 *
 * @param[in]  msgMaxLen  The message maximum length
 * @param      HSerial    The serial port. Can be Serial, Serial1, Serial2, Serial3 for an Arduino Mega (Atmega 2560).
 * @param[in]  baudrate   The baudrate
 */
SerialChecker::SerialChecker(HardwareSerial& port, uint32_t baudrate){
    this->serialType = serialTypes::HardWare;
    this->port.hardware = &port;
    this->baudrate = baudrate;
    rawMessage = new char[msgMaxLen];
    message = rawMessage;
    
}

/**
 * @brief      Constructs the object. As above but lets user choose serial port and baudrate.
 *
 * @param[in]  msgMaxLen  The message maximum length
 * @param      HSerial    The serial port. Can be Serial, Serial1, Serial2, Serial3 for an Arduino Mega (Atmega 2560).
 * @param[in]  baudrate   The baudrate
 */
SerialChecker::SerialChecker(uint16_t msgMaxLen, HardwareSerial& HSerial, uint32_t baudrate){
    this->msgMaxLen = msgMaxLen;
    this->baudrate = baudrate;
    rawMessage = new char[msgMaxLen];
    message = rawMessage;
}

// USBSerial
#ifdef USBserial_h_
SerialChecker::SerialChecker(usb_serial_class& port){
    this->serialType = serialTypes::USB;
    this->port.usb = &port;
    rawMessage = new char[msgMaxLen];
    message = rawMessage;
}

SerialChecker::SerialChecker(usb_serial_class& port, uint32_t baudrate) {
    // this->msgMaxLen = msgMaxLen;
    // this->HSerial = &HSerial;
    this->serialType = serialTypes::USB;
    this->port.usb = &port;
    this->baudrate = baudrate;
    // message = new char[msgMaxLen];
}
#endif

#ifdef USBCON // I think this is for atmega32u4 based arduinos
SerialChecker::SerialChecker(Serial_& port){
    serialType = serialTypes::ATMEGAXXU4;
    this->port.atmegaXXu4 = &port;
    rawMessage = new char[msgMaxLen];
    message = rawMessage;
}

SerialChecker::SerialChecker(Serial_& port, uint32_t baudrate){
    this->serialType = serialTypes::ATMEGAXXU4;
    this->port.atmegaXXu4 = &port;
    this->baudrate = baudrate;
    rawMessage = new char[msgMaxLen];
    message = rawMessage;
}
#endif

/**
 * @brief      Destroys the object and frees the memory used by message buffer
 */
SerialChecker::~SerialChecker(){
    delete [] rawMessage;
    delete [] address;
}

/**
 * @brief      This is functionally the same as Serial.begin(baudrate);
 */
void SerialChecker::init(){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->begin(baudrate);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->begin(baudrate);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->begin(baudrate);
            break;
    }
}

/**
 * @brief      Disables the use of Acknowledge and Naknowledge messages. This really only disables the use of Nak messages. The user must choose to send an Ack with sendAck() command.
 */
void SerialChecker::disableAckNak(){
    useAckNak = false;
}

/**
 * @brief      Enables the use of Acknowledge and Naknowledge messages. If an invalid message is received then a Nak is returned to the sender. This uses the default Ack and Nak chars, 'A' and 'N'. Naks get sent when a message does not start with the start char, if use of STX is required, see enableSTX(). Naks also get sent if the message is below the minimum length set by setMsgMinLen(). The default for that is two chars. Naks are also sent if a message is received with an invalid checksum, if enableChecksum() is used.
 */
void SerialChecker::enableAckNak(){
    useAckNak = true;
}

/**
 * @brief      Enables the use of Acknowledge and Naknowledge messages. If an invalid message is received then a Nak is returned to the sender. Here, the Ack and Nak chars are chosen by the user.
 *
 * @param[in]  Ack   The acknowledgement char to be sent by sendAck().
 * @param[in]  Nak   The naknowledgement char to be sent on receipt of invalid message or when sendNak() is called.
 */
void SerialChecker::enableAckNak(char Ack, char Nak){
    useAckNak = true;
    this->Ack = Ack;
    this->Nak = Nak;
}

/**
 * @brief      Disables the checking of checksums.
 */
void SerialChecker::disableChecksum(){
    useChecksum = false;
}

/**
 * @brief      Enables the checking of checksums. When messages are received, the last char before the ETX char must be a checksum char as calculated by the algorithm given in calcChecksum(char* rawMessage).
 */
void SerialChecker::enableChecksum(){
    useChecksum = true;
}

/**
 * @brief      Sets the checksum type.
 *
 * @param[in]  checksumType  The checksum type to be used as defined by the checksumTypeEnum. use like setChecksumType(checksumTypeEnum::spellmanMPS) for example.
 */
void SerialChecker::setChecksumType(checksumTypeEnum checksumType){
    this->checksumType = checksumType;
}

/**
 * @brief      Enables the use of and STX char at the start of a received message. If requireSTX == false, presence of an STX char in the received message array will reset the start of the message to the next char. This is useful in case the message received consists of some garbled chars followed by a valid message. If requireSTX == true, then messages will only be valid if an STX char is received. The message is then parsed from that point on. In this case, subsequent STX chars are counted the same as any other message chars and do not reset the start index of the received message. This uses the default STX char which is '$'.
 *
 * @param[in]  requireSTX  A flag to enforce the use of starting a message with the STX symbol. If true, messages must start with STX. If false, messages can optionally use STX at the start. If an STX is found part way through the message though, the preceding chars will be discarded. 
 */
void SerialChecker::enableSTX(bool requireSTX){
    useSTX = true;
    this->requireSTX = requireSTX;
    if(requireSTX){
        receiveStarted = false;
    }
}

/**
 * @brief      As above but allows the user to set a new STX char. The ascii char set does contain both an STX and ETX symbol but these are not human readable so serial monitors such as that used by the arduino IDE will not display them. This can make debugging harder. The default STX char is '$'. On the other hand, if enableSTX is used while requireSTX is false AND a checksum is used, if the checksum produced happens to be the STX symbol, then the message can not be successfully received since the checksum will be interpreted as an STX char, and the message receive process reset. If checksums and start characters are both required, ensure that requireSTX flag is set to true.
 *
 * @param[in]  requireSTX  A flag to enforce the use of starting a message with the STX symbol. If true, messages must start with STX. If false, messages can optionally use STX at the start. If an STX is found part way through the message though, the preceding chars will be discarded.
 * @param[in]  STX         The STX char to be used. Default is '$'.
 */
void SerialChecker::enableSTX(bool requireSTX, char STX){
    useSTX = true;
    this->requireSTX = requireSTX;
    if(requireSTX){
        receiveStarted = false;
    }
    this->STX = STX;
}


/**
 * @brief      Disables the use of the STX char at the start of messages. Disabled by default. See enableSTX() for more details.
 */
void SerialChecker::disableSTX(){
    useSTX = false;
    requireSTX = false;
}

/**
 * @brief      Sets the ETX char to be used. Serial data is received and the message buffer is filled until the ETX char is received. By default the ETX char is '\n' (newline character). This can be changed at runtime.
 *
 * @param[in]  ETX   The etx
 */
void SerialChecker::setETX(char ETX){
    this->ETX = ETX;
}

/**
 * @brief      Sets whether \r carriage return chars are valid. In the case where we are receiving a println() message from another arduino, the message will be terminated with \r\n, i.e. two end chars. This will break checksum checking and number conversion. By default, \r chars are just removed. They aren't human readable anyway... Regardless of how this flag is set, \r carriage returns could still be used as the ETX message termination char.
 *
 * @param[in]  allowCR  Indicates if \r carriage returns are allowed
 */
void SerialChecker::setAllowCR(bool allowCR){
    this->allowCR = allowCR;
}

/**
 * @brief      Gets the allow carriage return \r flag.
 *
 * @return     The \r carriage return allowed flag.
 */
bool SerialChecker::getAllowCR(){
    return allowCR;
}

/**
 * @brief      Call this function as often as you like to check for new messages. Valid messages cause the function to return the length of received message. This is also available by calling getMsgLen(). If no message, or an incomplete message is received, tt transfers the partial message (any message not terminated by an ETX char) from the arduino's serial buffer to this class's message buffer and returns a 0. 
 * 
 * If the message is too long or below the minimum length, it returns a 0 and deletes the message. 
 * 
 * If enableSTX(false) is used, a start STX char is not required, but if one is received, any previous received chars will be delected. This is useful to discard partial or garbled messages. If enableSTX(true) is used, a received message must begin with an STX char. Subsequent STX chars in this case will be counted as valid message body chars. it restarts the message if a it If an incomplete message is sitting in the serial buffer, this function will append it to this class's message buffer. Once an ETX end char is received (default is newline '\n' char), the message is checked This is the main function. 
 * 
 * By default, the class does not use checksums but if enableChecksum() is used, the char preceding the ETX char must be a checksum char. A local checksum is calculated from the rest of the message and compared with the received checksum. If valid, the message length is returned, else a 0.
 * 
 * If enableAckNak() is used, the check function will send back Nak chars in the event that the message received is not valid based on the above explained conditions. It is left to the user to send back Ack messages if they are needed using sendAck(). For example, a message might be received that sets a parameter. It might not make sense to send this back to the other device but sending an Ack char would notify the device that its message was received and successfully implemented. On the other hand, sendNak() can be used if the received set parameter is out of the allowed set range for example. 
 *
 * @return     A uint8_t value is returned representing the length of the message received, excluding the STX start char if used, the checksum char if used, or the ETX end char.
 */
uint8_t SerialChecker::check(){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            return checkUSBSerial();
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            return checkATMEGAXXU4Serial();
            break;
    #endif
        case serialTypes::HardWare:
            return checkHardwareSerial();
            break;
    }
}

#ifdef USBserial_h_
uint8_t SerialChecker::checkUSBSerial()(){
    while(port.usb->available()) {
        char in = port.usb->read();
        // HSerial->println(in);
        if(receiveStarted){
            if(useSTX && in == STX){
                msgIndex = 0;
                // HSerial->println("STX");
            }
            else if(in != ETX && msgIndex < msgMaxLen){
                //add to message
                if((in != '\r') || allowCR){
                    rawMessage[msgIndex] = in;
                    msgIndex++;
                }
                // HSerial->println("Adding to message");
            }
            else if(in == ETX){
                // message complete so calculate the checksum and compare it
                // HSerial->println("ETX");
                rawMessage[msgIndex] = '\0';
                // HSerial->println(msgIndex);
                if(msgIndex >= msgMinLen){ // make sure message is long enough
                    // HSerial->println("Long enough");
                    if(useChecksum){
                        rawMsgLen = msgIndex - 1;
                        char msgChecksum = rawMessage[rawMsgLen];
                        rawMessage[rawMsgLen] = '\0';
                        //HSerial->println(calcChecksum(message, rawMsgLen));
                        if(msgChecksum == calcChecksum(rawMessage, rawMsgLen)){
                            //parseMessage();
                            msgIndex = 0;
                            if(requireSTX){
                                receiveStarted = false;
                            }
                            return rawMsgLen;
                        }
                        else if(useAckNak){
                            port.usb->println(Nak);
                        }
                    }
                    else{
                        rawMsgLen = msgIndex;
                        msgIndex = 0;
                        if(requireSTX){
                            receiveStarted = false;
                        }
                        return rawMsgLen;
                    }
                }
                else if(useAckNak){
                    port.usb->println(Nak);
                }
                // reset megIndex for next message
                msgIndex = 0;
            }
            else{
                // message too long so scrap it and start again.
                msgIndex = 0;
                // HSerial->println("Too long");
                if(useAckNak){
                    port.usb->println(Nak);
                }
            } 
        }
        else{
            if(in == STX){
                receiveStarted = true;
            }
            else if(in == '\n'){
                if(useAckNak){
                    port.usb->println(Nak);
                }
            }
        }
    }
    return 0;
}
#endif

#ifdef USBCON
uint8_t SerialChecker::checkATMEGAXXU4Serial(){
    while(port.atmegaXXu4->available()) {
        char in = port.atmegaXXu4->read();
        // HSerial->println(in);
        if(receiveStarted){
            if(useSTX && in == STX){
                msgIndex = 0;
                // HSerial->println("STX");
            }
            else if(in != ETX && msgIndex < msgMaxLen){
                //add to message
                if((in != '\r') || allowCR){
                    rawMessage[msgIndex] = in;
                    msgIndex++;
                }
                // HSerial->println("Adding to message");
            }
            else if(in == ETX){
                // message complete so calculate the checksum and compare it
                // HSerial->println("ETX");
                rawMessage[msgIndex] = '\0';
                // HSerial->println(msgIndex);
                if(msgIndex >= msgMinLen){ // make sure message is long enough
                    // HSerial->println("Long enough");
                    if(useChecksum){
                        rawMsgLen = msgIndex - 1;
                        char msgChecksum = rawMessage[rawMsgLen];
                        rawMessage[rawMsgLen] = '\0';
                        //HSerial->println(calcChecksum(message, rawMsgLen));
                        if(msgChecksum == calcChecksum(rawMessage, rawMsgLen)){
                            //parseMessage();
                            msgIndex = 0;
                            if(requireSTX){
                                receiveStarted = false;
                            }
                            return rawMsgLen;
                        }
                        else if(useAckNak){
                            port.atmegaXXu4->println(Nak);
                        }
                    }
                    else{
                        rawMsgLen = msgIndex;
                        msgIndex = 0;
                        if(requireSTX){
                            receiveStarted = false;
                        }
                        return rawMsgLen;
                    }
                }
                else if(useAckNak){
                    port.atmegaXXu4->println(Nak);
                }
                // reset megIndex for next message
                msgIndex = 0;
            }
            else{
                // message too long so scrap it and start again.
                msgIndex = 0;
                // HSerial->println("Too long");
                if(useAckNak){
                    port.atmegaXXu4->println(Nak);
                }
            } 
        }
        else{
            if(in == STX){
                receiveStarted = true;
            }
            else if(in == '\n'){
                if(useAckNak){
                    port.atmegaXXu4->println(Nak);
                }
            }
        }
    }
    return 0;
}
#endif

uint8_t SerialChecker::checkHardwareSerial(){
    while(port.hardware->available()) {
        char in = port.hardware->read();
        // HSerial->println(in);
        if(receiveStarted){
            if(useSTX && in == STX){
                msgIndex = 0;
                // HSerial->println("STX");
            }
            else if(in != ETX && msgIndex < msgMaxLen){
                //add to message
                if((in != '\r') || allowCR){
                    rawMessage[msgIndex] = in;
                    msgIndex++;
                }
                // HSerial->println("Adding to message");
            }
            else if(in == ETX){
                // message complete so calculate the checksum and compare it
                // HSerial->println("ETX");
                rawMessage[msgIndex] = '\0';
                // HSerial->println(msgIndex);
                if(msgIndex >= msgMinLen){ // make sure message is long enough
                    // HSerial->println("Long enough");
                    if(useChecksum){
                        rawMsgLen = msgIndex - 1;
                        char msgChecksum = rawMessage[rawMsgLen];
                        rawMessage[rawMsgLen] = '\0';
                        //HSerial->println(calcChecksum(message, rawMsgLen));
                        if(msgChecksum == calcChecksum(rawMessage, rawMsgLen)){
                            //parseMessage();
                            msgIndex = 0;
                            if(requireSTX){
                                receiveStarted = false;
                            }
                            return rawMsgLen;
                        }
                        else if(useAckNak){
                            port.hardware->println(Nak);
                        }
                    }
                    else{
                        rawMsgLen = msgIndex;
                        msgIndex = 0;
                        if(requireSTX){
                            receiveStarted = false;
                        }
                        return rawMsgLen;
                    }
                }
                else if(useAckNak){
                    port.hardware->println(Nak);
                }
                // reset megIndex for next message
                msgIndex = 0;
            }
            else{
                // message too long so scrap it and start again.
                msgIndex = 0;
                // HSerial->println("Too long");
                if(useAckNak){
                    port.hardware->println(Nak);
                }
            } 
        }
        else{
            if(in == STX){
                receiveStarted = true;
            }
            else if(in == '\n'){
                if(useAckNak){
                    port.hardware->println(Nak);
                }
            }
        }
    }
    return 0;
}


/**
 * @brief      Returns the address that the message was sent to as a c-style char string. Do not call this if the @addressLen variable is set to the default of zero.
 *
 * @return     The address.
 */
char* SerialChecker::getAddress(){
    if(addressLen > 0){
        for(uint8_t i = 0; i < addressLen; i++){
            address[i] = rawMessage[i];
        }
        address[addressLen] = '\0';
        return address;
    }
    else{
        return '\0';
    }
}

/**
 * @brief      Gets the raw message. This is the full message including the address if there is one.
 *
 * @return     The raw message.
 */
char* SerialChecker::getRawMsg(){
    return rawMessage;
}

/**
 * @brief      Gets the raw message length. This is the length of the full message including the address if there is one.
 *
 * @return     The raw message length.
 */
uint8_t SerialChecker::getRawMsgLen(){
    return rawMsgLen;
}

/**
 * @brief      Gets the contents of the message buffer. The message buffer is updated if new chars are received by check().
 *
 * @return     The message buffer which is null terminated.
 */
char* SerialChecker::getMsg(){
    return message;
}

/**
 * @brief      Gets the message starting at index startIndex.
 *
 * @param[in]  startIndex  The start index
 *
 * @return     The null terminated message starting at index startIndex.
 */
char* SerialChecker::getMsg(uint8_t startIndex){
    return &message[startIndex];
}

/**
 * @brief      Gets the length of the message in the message buffer.
 *
 * @return     The message length.
 */
uint8_t SerialChecker::getMsgLen(){
    return rawMsgLen - addressLen;
}

/**
 * @brief      Sets the valid message minimum length. Received messages that are shorter than this will be discarded and check() will return a 0.
 *
 * @param[in]  msgMinLen  The message minimum valid length
 */
void SerialChecker::setMsgMinLen(uint8_t msgMinLen){
    this->msgMinLen = msgMinLen;
}

/**
 * @brief      Sets the valid message maximum length. Received messages that are longer than this will be discarded and check() will return a 0.
 *
 * @param[in]  msgMaxLen  The message maximum valid length
 */
void SerialChecker::setMsgMaxLen(uint8_t msgMaxLen){
    this->msgMaxLen = msgMaxLen;
    delete [] message;
    message = new char[msgMaxLen];
}

/**
 * @brief      Sets the number of chars used as an address. The message follows on from this.
 *
 * @param[in]  len   The new value
 */
void SerialChecker::setAddressLen(uint8_t len){
    address = new char[len + 1]; // + 1 to allow for null terminator
    address[0] = '\0';
    addressLen = len;
    message = &rawMessage[addressLen]; // Move the message pointer past the addres.
}

/**
 * @brief      Gets the number of chars used as an address. The message follows on from this.
 *
 * @return     The address length.
 */
uint8_t SerialChecker::getAddressLen(){
    return addressLen;
}

/**
 * @brief      Check to see if the received message contains a char array starting at startIndex. With this function the user can check to see what type of message has been sent.
 *
 * @param[in]  snippet  The snippet char array to be compared.
 *
 * @return     Returns true if the snippet test char array is present and false if not.
 */
bool SerialChecker::contains(char* snippet, uint8_t startIndex){
    // check if the shippet is present starting at index startIndex.
    // snippet char array must be null terminated.
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

/**
 * @brief      Check to see if the received message contains a char array. With this function the user can check to see what type of message has been sent.
 *
 * @param[in]  snippet  The snippet char array to be compared.
 *
 * @return     Returns true if the snippet test char array is present and false if not.
 */
bool SerialChecker::contains(char* snippet){
    // check if the shippet is present starting at index 0.
    // snippet char array must be null terminated.
    return contains(snippet, 0);
}

/**
 * @brief      Check to see if the received message contains a char (uint8_t). With this function the user can check to see what type of message has been sent.
 *
 * @param[in]  c      The char to be compared at the index given in the received message.
 * @param[in]  index  The index at which to test the char match in the message.
 *
 * @return     Returns true if the char c is present in the message.
 */
bool SerialChecker::contains(const char& c, uint8_t index){
    if(message[index] == c){
        return true;
    }
    return false;
}

/* Check to see if the received message contains a char (uint8_t) at the start of the message. With this function the user can check to see what type of message has been sent.
 *
 * @param[in]  c      The char to be compared at the start of the received message.
 *
 * @return     Returns true if the char c is present at the start of the message.
 */
bool SerialChecker::contains(const char& c){
    return contains(c, 0);
}



/**
 * @brief      Calculates a checksum of the rawMessage array of length len using which ever algorithm is set to be used by the setChecksumType() function. There are currently two options. 
 * 1. Spellman MPS: Simple checksum algorithm that produces 64 possible chars. This is as defined in the [Spellman MPS Digital Interface manual](https://www.spellmanhv.com/-/media/en/Products/MPS-Digital-Interface.pdf), designed for use with their MPS power supplies.
 * 2. 8 bit printable chars: This is also a simple checksum which is calculated by summing over all the chars in the message and then mapping the sum to the ascii chars which are printable by the arduino IDE. This is chosen for easier debugging and provides 94 unique checksums.
 * A limitation of both of these methods is that they are not dependent on the order of the chars in the message and only consider the total sum. They also don't produce very many checksums so the chances of two flipped bits due to noise causing valid checksum to still be produced is higher than 1/100. Having said that, these algorithms have been successfully used in a physics research lab for years without noticeable errors.
 * 
 * Use this function to generate a checksum of a return message.
 *
 * @param      rawMessage  The message to be sent that needs a checksum.
 * @param[in]  len         The length of the message
 *
 * @return     The checksum char.
 */
char SerialChecker::calcChecksum(char* rawMessage, int len){
    char checksum;
    switch(checksumType){
        case checksumTypeEnum::SpellmanMPS:
            checksum = chksmSpellmanMPS(rawMessage, len);
            break;
        case checksumTypeEnum::Readable8bitChars:
            checksum = chksm8bitAllReadableChars(rawMessage, len);
            break;
    }
    return checksum;
}


/**
 * @brief      Calculates the checksum of the null terminated rawMessage array. I can't remember where I got this algorithm from but it produces a checksum char in the human readable asciii charset range of which there are 128 possibilities. Whilst this will not guarantee an error free message, it will hugely reduce the chances of getting a message with an error in it that matches the checksum.
 * 
 * Use this function to generate a checksum of a return message.
 *
 * @param      rawMessage  The message to be sent that needs a checksum.
 *
 * @return     The checksum char.
 */
char SerialChecker::calcChecksum(char* rawMessage){
    char checksum;
    switch(checksumType){
        case checksumTypeEnum::SpellmanMPS:
            checksum = chksmSpellmanMPS(rawMessage);
            break;
        case checksumTypeEnum::Readable8bitChars:
            checksum = chksm8bitAllReadableChars(rawMessage);
            break;
    }
    return checksum;
}

/**
 * @brief      Calculates a checksum that is compatible with the Spellman MPS range of high voltage power supplies as detailed [here](https://www.spellmanhv.com/-/media/en/Products/MPS-Digital-Interface.pdf).
 *
 * @param      rawMessage  The raw message
 * @param[in]  len         The length of the message
 *
 * @return     the calculated checksum char
 */
char SerialChecker::chksmSpellmanMPS(char* rawMessage, int len){
    uint8_t checksum=0; // used to use uint16_t but 8 works
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

/**
 * @brief      Calculates a checksum that is compatible with the Spellman MPS range of high voltage power supplies as detailed [here](https://www.spellmanhv.com/-/media/en/Products/MPS-Digital-Interface.pdf).
 *
 * @param      rawMessage  The raw message
 *
 * @return     the calculated checksum char
 */
char SerialChecker::chksmSpellmanMPS(char* rawMessage){
    uint8_t checksum=0; // used to use uint16_t but 8 works
    while(*rawMessage){
        checksum += *rawMessage;
        rawMessage++;
    }
    checksum = ~checksum+1; //the checksum is currently a unsigned 16bit int. Invert all the bits and add 1.
    checksum = 0x7F & checksum; // discard the 8 MSBs and clear the remaining MSB (B0000000001111111)
    checksum = 0x40 | checksum; //bitwise or bit6 with 0x40 (or with a seventh bit which is set to 1.) (B0000000001000000)
    return (char) checksum;
}

/**
 * @brief      Calculates a checksum where the resultant char is in the range of chars that are printable by the arduino IDE. That is 33: '!' to 126: '~'
 *
 * @param      rawMessage  The raw message
 * @param[in]  len         The length of the message
 *
 * @return     the calculated checksum char
 */
char SerialChecker::chksm8bitAllReadableChars(char* rawMessage, int len){
    uint8_t checksum=0; // used to use uint16_t but 8 works
    for(uint8_t i = 0; i < len; i++)
    { //add the command
        checksum += rawMessage[i];
    }
    checksum &= 0x7F; // clear the MSB (0b1111111) so that the number can only be between 0 and 127.
    checksum += 33; // 33 is the minimum printable char '!'
    if(checksum > 126){
        checksum -= 94; // 126 is the last readable char '~', so wrap back to the first printable char by subtracting 94.
    }
    return (char) checksum;
}

/**
 * @brief      Calculates a checksum where the resultant char is in the range of chars that are printable by the arduino IDE. That is 33: '!' to 126: '~'
 *
 * @param      rawMessage  The raw message
 *
 * @return     the calculated checksum char
 */
char SerialChecker::chksm8bitAllReadableChars(char* rawMessage){
    uint8_t checksum=0; // used to use uint16_t but 8 works
    while(*rawMessage){
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

// def testChecksum8bitReadableChars(i: int) -> int:
//     checksum = i
//     checksum &= 127 #0x7F 0b1111111
//     checksum += 33 # first readable char is '!'
//     if(checksum > 126): # last readable char is '~'
//         checksum -= 94
//     return checksum

/**
 * @brief      Sets the check conversion flag. When converting strings of chars to int or float types, checks to see whether the char is '0' to '9', or a '.' in the case of float conversion. If a non valid char is found, the conversion ends. This allows follow-up fields in the message to be used after the number is sent.
 *
 * @param[in]  checkConversion  The check conversion
 */
// void SerialChecker::setCheckConversion(bool checkConversion){
//     this->checkConversion = checkConversion;
// }

/**
 * @brief      Gets the check conversion flag. See setCheckConversion(bool checkConversion) for details.
 *
 * @return     The check conversion flag.
 */
// bool SerialChecker::getCheckConversion(){
//     return checkConversion;
// }

// /**
//  * @brief      Gets the number type and returns it as an enum. The char can be an interger '0' to '9', a minus sign '-', a decimal place '.' or not a number char.
//  *
//  * @param[in]  c     { parameter_description }
//  *
//  * @return     The number type.
//  */
// charNumTypeEnum SerialChecker::getNumType(const char& c){
//     if( (c >= '0') && (c <= '9') ){
//         return charNumTypeEnum::Integer;
//     }
//     else if(c == '-'){
//         return charNumTypeEnum::MinusSign;
//     }
//     else if(c == '.'){
//         return charNumTypeEnum::DecPoint;
//     }
//     return charNumTypeEnum::NaN;
// }

// /**
//  * @brief      Determines whether the specified char, c, is number character. '0' to '9' and '-'' for minus sign and '.'' for decimal point are valid. 
//  *
//  * @param[in]  c     The char to check.
//  *
//  * @return     True if the specified c is number character, False otherwise.
//  */
// bool SerialChecker::isNumChar(const char& c){
//     if( (c >= '0') && (c <= '9') ){
//         return true;
//     }
//     else if( (c == '-') || (c == '.') ){
//         return true;
//     }
//     return false;
// }

/**
 * @brief      Converts the message in the message buffer starting at startIndex to a float
 *
 * @param[in]  startIndex  The start indexc
 *
 * @return     the float value that has been converted.
 */
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
    // for(int i = startIndex; i < rawMsgLen - addressLen; i++) 
    // {
    //     if(message[i] == '.'){
    //         units = false;
    //     }
    //     else{
    //         if(units){
    //             number *= 10.0;
    //             number += float(message[i] -'0');
    //         }
    //         else{
    //             number += float(message[i] - '0') * pow(0.1, decimalN);
    //             decimalN++;
    //         }
    //     }
    // }
    while(message[startIndex]){
        if(message[startIndex] == '.'){
            units = false;
        }
        else if( (message[startIndex] >= '0' && message[startIndex] <= '9') ){
            if(units){
                number *= 10.0;
                number += float(message[startIndex] -'0');
            }
            else{
                number += float(message[startIndex] - '0') * pow(0.1, decimalN);
                decimalN++;
            }
        }
        else{
            break;
        }
        startIndex++;
    }
    if(negative){
        number *= -1.0;
    }
    return number;
}
// Sketch uses 7006 bytes (22%) of program storage space. Maximum is 30720 bytes.
// Global variables use 353 bytes (17%) of dynamic memory, leaving 1695 bytes for local variables. Maximum is 2048 bytes.
// Sketch uses 7008 bytes (22%) of program storage space. Maximum is 30720 bytes.
// Global variables use 354 bytes (17%) of dynamic memory, leaving 1694 bytes for local variables. Maximum is 2048 bytes.
// 

/**
 * @brief      Converts the message in the message buffer to a float
 *
 * @return     the converted float value
 */
float SerialChecker::toFloat(){
    uint8_t number = 0;
    bool negative = false;
    uint8_t startIndex = 0;
    while(message[startIndex]){
        if((message[startIndex] == '-') || (message[startIndex] >= '0' && message[startIndex] <= '9')){
            break;
        }
        // charNumTypeEnum numType = getNumType(message[startIndex]);
        // if(numType == charNumTypeEnum::Integer || numType == charNumTypeEnum::MinusSign){
        //     break
        // } 
        startIndex++;
    }
    return toFloat(startIndex);
}

/**
 * @brief      Converts the message buffer (starting at startIndex) to an int8_t. This works for both unsigned and signed ints as long as the container it is loaded in to is of the correct type.
 *
 * @param[in]  startIndex  The start index
 *
 * @return     the converted int8_t
 */
uint8_t SerialChecker::toInt8(uint8_t startIndex){
    // Returns the number stored in a char array, starting at startIndex
    uint8_t number = 0;
    bool negative = false;
    if( message[startIndex] == '-'){
        negative = true;
        startIndex++;
    }
    while(message[startIndex]){
        if( (message[startIndex] >= '0' && message[startIndex] <= '9') ){
            number *= 10;
            number += (message[startIndex] -'0');
        }
        else{
            break;
        }
        startIndex++;
    }
    // for(int i = startIndex; i < rawMsgLen - addressLen; i++) 
    // {
    //     number *= 10;
    //     number += (message[i] -'0');
    // }
    if(negative){
        number *= -1;
    }
    return number;
}

/**
 * @brief      Converts the message buffer to an int8_t. This works for both unsigned and signed ints as long as the container it is loaded in to is of the correct type.
 *
 * @return     the converted int8_t
 */
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

/**
 * @brief      Converts the message buffer (starting at startIndex) to an int16_t. This works for both unsigned and signed ints as long as the container it is loaded in to is of the correct type.
 *
 * @param[in]  startIndex  The start index
 *
 * @return     the converted int16_t
 */
uint16_t SerialChecker::toInt16(uint8_t startIndex){
    // Returns the number stored in a char array, starting at startIndex
    uint16_t number = 0;
    bool negative = false;
    if( message[startIndex] == '-'){
        negative = true;
        startIndex++;
    }
    // for(int i = startIndex; i < rawMsgLen - addressLen; i++) 
    // {
    //     number *= 10;
    //     number += (message[i] -'0');        
    // }
    while(message[startIndex]){
        if( (message[startIndex] >= '0' && message[startIndex] <= '9') ){
            number *= 10;
            number += (message[startIndex] -'0');
        }
        else{
            break;
        }
        startIndex++;
    }
    if(negative){
        number *= -1;
    }
    return number;
}

/**
 * @brief      Converts the message buffer to an int16_t. This works for both unsigned and signed ints as long as the container it is loaded in to is of the correct type.
 *
 * @return     the converted int16_t
 */
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

/**
 * @brief      Converts the message buffer (starting at startIndex) to an int32_t. This works for both unsigned and signed ints as long as the container it is loaded in to is of the correct type.
 *
 * @param[in]  startIndex  The start index
 *
 * @return     the converted int32_t
 */
uint32_t SerialChecker::toInt32(uint8_t startIndex){
    // Returns the number stored in a char array, starting at startIndex
    uint32_t number = 0;
    bool negative = false;
    if( message[startIndex] == '-'){
        negative = true;
        startIndex++;
    }
    // for(int i = startIndex; i < rawMsgLen - addressLen; i++) 
    // {
    //     number *= 10;
    //     number += (message[i] -'0');
    // }
    while(message[startIndex]){
        if( (message[startIndex] >= '0' && message[startIndex] <= '9') ){
            number *= 10;
            number += (message[startIndex] -'0');
        }
        else{
            break;
        }
        startIndex++;
    }
    if(negative){
        number *= -1;
    }
    return number;
}

/**
 * @brief      Converts the message buffer to an int32_t. This works for both unsigned and signed ints as long as the container it is loaded in to is of the correct type.
 *
 * @return     the converted int32_t
 */
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

/**
 * @brief      Sends an Ack char followed by the ETX char.
 */
void SerialChecker::sendAck(){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->println(Ack);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->println(Ack);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->println(Ack);
            break;
    }
}

/**
 * @brief      Sends an Nak char followed by the ETX char.
 */
void SerialChecker::sendNak(){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->println(Nak);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->println(Nak);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->println(Nak);
            break;
    }
}

/**
 * @brief      Same as Serial's .print method.
 *
 * @param      message  The c style string message
 */
void SerialChecker::print(char* message){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->print(message);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->print(message);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->print(message);
            break;
    }
}

/**
 * @brief      Same as Serial's .print method.
 *
 * @param[in]  c     The char to print
 */
void SerialChecker::print(char c){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->print(c);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->print(c);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->print(c);
            break;
    }
}

/**
 * @brief      Same as Serial's .print method.
 *
 * @param[in]  n     The uint8_t to print
 */
void SerialChecker::print(uint8_t n){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->print(n);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->print(n);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->print(n);
            break;
    }
}

/**
 * @brief      Same as Serial's .print method.
 *
 * @param[in]  n     The uint16_t to print
 */
void SerialChecker::print(uint16_t n){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->print(n);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->print(n);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->print(n);
            break;
    }
}

/**
 * @brief      Same as Serial's .print method.
 *
 * @param[in]  n     The uint32_t to print
 */
void SerialChecker::print(uint32_t n){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->print(n);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->print(n);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->print(n);
            break;
    }
}

/**
 * @brief      Same as Serial's .print method.
 *
 * @param[in]  n     The int8_t to print
 */
void SerialChecker::print(int8_t n){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->print(n);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->print(n);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->print(n);
            break;
    }
}

/**
 * @brief      Same as Serial's .print method.
 *
 * @param[in]  n     The int16_t to print
 */
void SerialChecker::print(int16_t n){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->print(n);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->print(n);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->print(n);
            break;
    }
}

/**
 * @brief      Same as Serial's .print method.
 *
 * @param[in]  n     The int32_t to print
 */
void SerialChecker::print(int32_t n){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->print(n);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->print(n);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->print(n);
            break;
    }
}

/**
 * @brief      Same as Serial's .print method.
 *
 * @param[in]  n     The float to print
 */
void SerialChecker::print(float n){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->print(n);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->print(n);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->print(n);
            break;
    }
}

/**
 * @brief      Same as Serial's .print method.
 *
 * @param[in]  n     The float to print
 */
void SerialChecker::print(double n){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->print(n);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->print(n);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->print(n);
            break;
    }
}

/**
 * @brief      Same as Serial's .println method.
 *
 * @param      message  The c-style string to print
 */
void SerialChecker::println(char* message){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->println(message);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->println(message);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->println(message);
            break;
    }
}

/**
 * @brief      Same as Serial's .println method.
 *
 * @param[in]  c     The char to print
 */
void SerialChecker::println(char c){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->println(c);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->println(c);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->println(c);
            break;
    }
}

/**
 * @brief      Same as Serial's .println method.
 *
 * @param[in]  n     The uint8_t to print
 */
void SerialChecker::println(uint8_t n){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->println(n);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->println(n);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->println(n);
            break;
    }
}

/**
 * @brief      Same as Serial's .println method.
 *
 * @param[in]  n     The uint16_t to print
 */
void SerialChecker::println(uint16_t n){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->println(n);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->println(n);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->println(n);
            break;
    }
}

/**
 * @brief      Same as Serial's .println method.
 *
 * @param[in]  n     The uint32_t to print
 */
void SerialChecker::println(uint32_t n){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->println(n);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->println(n);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->println(n);
            break;
    }
}

/**
 * @brief      Same as Serial's .println method.
 *
 * @param[in]  n     The int8_t to print
 */
void SerialChecker::println(int8_t n){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->println(n);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->println(n);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->println(n);
            break;
    }
}

/**
 * @brief      Same as Serial's .println method.
 *
 * @param[in]  n     The int16_t to print
 */
void SerialChecker::println(int16_t n){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->println(n);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->println(n);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->println(n);
            break;
    }
}

/**
 * @brief      Same as Serial's .println method.
 *
 * @param[in]  n     The int32_t to print
 */
void SerialChecker::println(int32_t n){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->println(n);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->println(n);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->println(n);
            break;
    }
}

/**
 * @brief      Same as Serial's .println method.
 *
 * @param[in]  n     The float to print
 */
void SerialChecker::println(float n){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->println(n);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->println(n);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->println(n);
            break;
    }
}

/**
 * @brief      Same as Serial's .println method.
 *
 * @param[in]  n     The float to print
 */
void SerialChecker::println(double n){
    switch (serialType) {
    #ifdef USBserial_h_
        case serialTypes::USB:
            port.usb->println(n);
            break;
    #endif
    #ifdef USBCON
        case serialTypes::ATMEGAXXU4:
            port.atmegaXXu4->println(n);
            break;
    #endif
        case serialTypes::HardWare:
            port.hardware->println(n);
            break;
    }
}