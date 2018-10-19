#include "SerialChecker.h"

/**
 * @brief      Constructs the object. Dynamically creates a char array to hold message buffers. Assigns the the default arduino serial port. 
 */
SerialChecker::SerialChecker(){
    message = new char[msgMaxLen];
    this->HSerial = &Serial;
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
    this->HSerial = &HSerial;
    this->baudrate = baudrate;
    message = new char[msgMaxLen];
}

/**
 * @brief      Destroys the object and frees the memory used by message buffer
 */
SerialChecker::~SerialChecker(){
    delete [] message;
}

/**
 * @brief      This is functionally the same as Serial.begin(baudrate);
 */
void SerialChecker::init(){
    HSerial->begin(baudrate); 
    // HSerial->println("Connected to SerialChecker test.");
}

/**
 * @brief      Disables the use of Acknowledge and Naknowledge messages. This really only disables the use of NAK messages. The user must choose to send an ACK with sendACK() command.
 */
void SerialChecker::disableACKNAK(){
    useACKNAK = false;
}

/**
 * @brief      Enables the use of Acknowledge and Naknowledge messages. If an invalid message is received then a NAK is returned to the sender. This uses the default ACK and NAK chars, 'A' and 'N'. NAKs get sent when a message does not start with the start char, if use of STX is required, see enableSTX(). NAKs also get sent if the message is below the minimum length set by setMsgMinLen(). The default for that is two chars. NAKs are also sent if a message is received with an invalid checksum, if enableChecksum() is used.
 */
void SerialChecker::enableACKNAK(){
    useACKNAK = true;
}

/**
 * @brief      Enables the use of Acknowledge and Naknowledge messages. If an invalid message is received then a NAK is returned to the sender. Here, the ACK and NAK chars are chosen by the user.
 *
 * @param[in]  ACK   The acknowledgement char to be sent by sendACK().
 * @param[in]  NAK   The naknowledgement char to be sent on receipt of invalid message or when sendNAK() is called.
 */
void SerialChecker::enableACKNAK(char ACK, char NAK){
    useACKNAK = true;
    this->ACK = ACK;
    this->NAK = NAK;
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
 * @brief      Call this function as often as you like to check for new messages. Valid messages cause the function to return the length of received message. This is also available by calling getMsgLen(). If no message, or an incomplete message is received, tt transfers the partial message (any message not terminated by an ETX char) from the arduino's serial buffer to this class's message buffer and returns a 0. 
 * 
 * If the message is too long or below the minimum length, it returns a 0 and deletes the message. 
 * 
 * If enableSTX(false) is used, a start STX char is not required, but if one is received, any previous received chars will be delected. This is useful to discard partial or garbled messages. If enableSTX(true) is used, a received message must begin with an STX char. Subsequent STX chars in this case will be counted as valid message body chars. it restarts the message if a it If an incomplete message is sitting in the serial buffer, this function will append it to this class's message buffer. Once an ETX end char is received (default is newline '\n' char), the message is checked This is the main function. 
 * 
 * By default, the class does not use checksums but if enableChecksum() is used, the char preceding the ETX char must be a checksum char. A local checksum is calculated from the rest of the message and compared with the received checksum. If valid, the message length is returned, else a 0.
 * 
 * If enableACKNAK() is used, the check function will send back NAK chars in the event that the message received is not valid based on the above explained conditions. It is left to the user to send back ACK messages if they are needed using sendACK(). For example, a message might be received that sets a parameter. It might not make sense to send this back to the other device but sending an ACK char would notify the device that its message was received and successfully implemented. On the other hand, sendNAK() can be used if the received set parameter is out of the allowed set range for example. 
 *
 * @return     A uint8_t value is returned representing the length of the message received, excluding the STX start char if used, the checksum char if used, or the ETX end char.
 */
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
    return msgLen;
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
bool SerialChecker::contains(const char* snippet){
    // check if the shippet is present starting at index 0.
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
        if((message[startIndex] == '-') || 
            (message[startIndex] >= '0' && message[startIndex] <= '9')){
            break;
        }
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
 * @brief      Sends an ACK char followed by the ETX char.
 */
void SerialChecker::sendACK(){
    HSerial->println(ACK);
}

/**
 * @brief      Sends an NAK char followed by the ETX char.
 */
void SerialChecker::sendNAK(){
    HSerial->println(NAK);
}