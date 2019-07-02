/**
 * @file
 * @brief     This file implements the functions declared in pn532.h
 * 
 * @author    Nathan Houwaart
 * @license   See LICENSE
 */

#include "../headers/pn532.h"

namespace nfc {
//   for(uint8_t i = 0; i < response.length; i++){
//         hwlib::cout << hwlib::hex << response.finalBuffer[i] << " . ";
//     }hwlib::cout << hwlib::endl;

// ------------------------------------------------------------------------------- //
// Constructor                                                                     //
// ------------------------------------------------------------------------------- //


PN532_chip::PN532_chip(
            communication::protocol& _protocol,
            hwlib::pin_in& irq
    ):
            NFC(_protocol),
            irq(irq)
    {
        init();
    }


// ------------------------------------------------------------------------------- //
// Basic communciation functions                                                   //
// ------------------------------------------------------------------------------- //


void PN532_chip::init()
{
     _protocol.wakeUp();
}

void PN532_chip::sendData(uint8_t *commandBuffer, const uint8_t nBytes)
{
    _protocol.wakeUp();
    _protocol.sendData(commandBuffer, nBytes);
    _protocol.endTransaction();
}

void PN532_chip::getData(uint8_t *buffer, const uint8_t nBytes)
{
    _protocol.receiveData(buffer, nBytes);
}

statusCode PN532_chip::writeRegister(const uint16_t reg, const uint8_t val){

    uint8_t adrH = ( reg >> 8 ) & 0xFF;
    uint8_t adrL = ( reg & 0xFF);

    uint8_t commands[] = {
        pn532::command::writeRegister,
        adrH,
        adrL,
        val
    };

    auto command = setupSendCommand(commands, sizeof(commands) / sizeof(uint8_t));

    auto [pass, response] = sendCommandAndCheckAck(command);
    if(pass != statusCode::pn532StatusOK) {return pass;}

    if(response.finalBuffer[3] != 0x09) {return statusCode::pn532StatusWrongCommand;}
    
    return statusCode::pn532StatusOK;
}

std::array<uint8_t, 2> PN532_chip::readRegister(const uint16_t reg){
    std::array<uint8_t, 2> readRegister = {0};

    uint8_t adrH = ( reg >> 8 ) & 0xFF;
    uint8_t adrL = ( reg & 0xFF );

    uint8_t commands[] = {
        pn532::command::readRegister,
        adrH,
        adrL
    };

    auto command = setupSendCommand(commands, sizeof(commands) / sizeof(uint8_t));

    auto [pass, response] = sendCommandAndCheckAck(command);
    if(pass != statusCode::pn532StatusOK) {readRegister[0] = pass; return readRegister;}

    if(response.finalBuffer[3] != 0x07) {readRegister[0] = statusCode::pn532StatusWrongCommand; return readRegister;}

    readRegister[1] = response.finalBuffer[4];

    return readRegister;
}

statusCode PN532_chip::writeGPIO(uint8_t newPinState){
    // make sure pin 32 and 34 are not changed
    newPinState |= (0x01 << pn532::command::GPIO::p32);
    newPinState |= (0x01 << pn532::command::GPIO::p34);

    // set validation bit
    newPinState |= pn532::command::GPIO::validationBit;

    uint8_t commands[] = {
        pn532::command::writeGPIO,
        newPinState,
        0x00        // p72 and p71 are reserved and thus not used.
    };

    auto command = setupSendCommand(commands, sizeof(commands) / sizeof(uint8_t));

    auto [pass, response] = sendCommandAndCheckAck(command);
    if(pass != statusCode::pn532StatusOK) {return pass;}

    if(response.finalBuffer[3] == 0x0F) { return statusCode::pn532StatusOK;}
    return statusCode::pn532StatusWrongCommand;
}

std::array<uint8_t, 2> PN532_chip::readGPIO(){
    std::array<uint8_t, 2> readGPIO = {0};
    uint8_t commands[] = {
        pn532::command::readGPIO
    };

    auto command = setupSendCommand(commands, sizeof(commands) / sizeof(uint8_t));

    auto [pass, response] = sendCommandAndCheckAck(command);
    if(pass != statusCode::pn532StatusOK) {readGPIO[0] = pass; return readGPIO;}

    if(response.finalBuffer[3] != 0x0D) { readGPIO[0] = statusCode::pn532StatusWrongCommand; return readGPIO;}

    uint8_t gpio_state = response.finalBuffer[4];
    uint8_t gpio_state_p7 = (response.finalBuffer[5] << 5);
    gpio_state |= gpio_state_p7;

    readGPIO[1] = gpio_state;

    return readGPIO;
}

// ------------------------------------------------------------------------------- //
// Basic functions                                                                 //
// ------------------------------------------------------------------------------- //

bool PN532_chip::waitForChip(const int timeout)
{
    int timer = 0;
    while (timer < timeout)
    {
        if (!irq.read()){ return true;}
        hwlib::wait_ms(10);
        timer += 10;
    }
    return false;
}

bool PN532_chip::checkAck(const uint8_t *buffer, const uint8_t n)
{
    // the ack buffers for spi and i2c slightly differ. 
    if(buffer[0] != 0x00){
        for (uint8_t i = 0; i < n-1; i++){
            if (pn532::general::Ack_buffer_template_i2c[i] != buffer[i]){
                return false;
            }
        }
        return true;
    }else{
        for (uint8_t i = 0; i < n-2; i++){
            if (pn532::general::Ack_buffer_template_spi[i] != buffer[i]){
                return false;
            }
        }
       return true;
    }  
}

Result PN532_chip::sendCommandAndCheckAck(setupSendCommand &command)
{
    sendData(command.finalbuffer, command.length);
    if (!waitForChip()) {return Result{statusCode::pn532StatusTimeout, receivedCommand()};}
    hwlib::wait_ms(10);

    getData(ackBuffer, 7); // size of the ack buffer

    if(!checkAck(ackBuffer, (sizeof(ackBuffer) / sizeof(ackBuffer[0])))) return Result{statusCode::pn532StatusInvalidAckFrame, receivedCommand()};
    
    if (!waitForChip()) {return Result{statusCode::pn532StatusTimeout, receivedCommand()};}

    uint8_t databuffer[64] = {0};
    getData(databuffer, 40);

    _protocol.endTransaction();

    auto response = receivedCommand(databuffer, 64);
    return Result{statusCode::pn532StatusOK, response};
}

// ------------------------------------------------------------------------------- //   
// More advanced functions                                                         //
// ------------------------------------------------------------------------------- //

statusCode PN532_chip::performSelftest()
{
    const uint8_t dummyData[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    const uint8_t commands[] = {
        pn532::command::PerformSelftest,
        pn532::command::diagnose::CommunicationLineTest,
        dummyData[0], dummyData[1], dummyData[2], dummyData[3],
        dummyData[4], dummyData[5], dummyData[6]};

    auto command = setupSendCommand(commands, sizeof(commands) / sizeof(commands[0]));

    auto [pass, response] = sendCommandAndCheckAck(command);
    if(pass != statusCode::pn532StatusOK) {return pass;}

    for (int i = 0; i < 7; i++)
    {
        if(response.finalBuffer[i+5] != dummyData[i])
        {
            return statusCode::pn532StatusSelftestFail;
        }          
    }

   
    return statusCode::pn532StatusOK;
}

std::array<uint8_t, 5> PN532_chip::getGeneralStatus()
{
    std::array<uint8_t, 5> generalStatus = {0};
    uint8_t commands[] = {
        pn532::command::getGeneralStatus};
    auto command = setupSendCommand(commands, sizeof(commands) / sizeof(uint8_t));

    auto [pass, response] = sendCommandAndCheckAck(command);
    if(pass != statusCode::pn532StatusOK) {generalStatus[0] = statusCode::pn532StatusInvalidAckFrame; return generalStatus;}

    generalStatus[0] = statusCode::pn532StatusOK;
    for(int i = 1; i < 5; i++){
        generalStatus[i] = response.finalBuffer[i+3];
    }
   
    return generalStatus;
}

std::array<uint8_t, 5> PN532_chip::getFirmwareVersion()
{
    std::array<uint8_t, 5> firmwareVersion = {0};

    uint8_t commands[] = {
        pn532::command::GetFirmwareVersion
    };
    
    auto command = setupSendCommand(commands, sizeof(commands) / sizeof(uint8_t));
    auto [pass, response] = sendCommandAndCheckAck(command);
    if(pass != statusCode::pn532StatusOK) {firmwareVersion[0] = pass; return firmwareVersion;}

    firmwareVersion[0] = statusCode::pn532StatusOK;
    for(uint8_t i = 1; i < 5; i++){
        firmwareVersion[i] = response.finalBuffer[i+3];
    }
    
    return firmwareVersion;
}

statusCode PN532_chip::SAMConfiguration(const uint8_t mode)
{
    uint8_t commands[] = {
        pn532::command::SAMConfiguration,
        mode};

    auto command = setupSendCommand(commands, sizeof(commands) / sizeof(uint8_t));

    auto [pass, response] = sendCommandAndCheckAck(command);
    if(pass != statusCode::pn532StatusOK) {return pass;}

    if(response.finalBuffer[3] != 0x15) {return statusCode::pn532statusSAMerror;}

    return statusCode::pn532StatusOK;
}

statusCode PN532_chip::RFField(const bool state)
{
    uint8_t commands[] = {
        pn532::command::RFConfiguration,
        pn532::command::RFItem::RFField,
        static_cast<uint8_t>(state)
    };

    auto command = setupSendCommand(commands, sizeof(commands) / sizeof(uint8_t));

    auto [pass, response] = sendCommandAndCheckAck(command);
    if(pass != statusCode::pn532StatusOK) {return pass;}

    if(response.finalBuffer[3]!= 0x33){return statusCode::pn532StatusWrongCommand;}

    return statusCode::pn532StatusOK;
}


statusCode PN532_chip::setMaxRetries(const uint8_t maxRetries)
{
    uint8_t commands[] = {
        pn532::command::RFConfiguration,
        pn532::command::RFItem::MaxRetries,
        0xFF, //MxRtyATR    default 0xff
        0xFF, //MxRtyPSL    default 0xff
        maxRetries
        /// Source : https://www.nxp.com/docs/en/user-guide/141520.pdf 
        /// P. 103   section 7.3.1
    };

    auto command = setupSendCommand(commands, sizeof(commands) / sizeof(uint8_t));

    auto [pass, response] = sendCommandAndCheckAck(command);
    if(pass != statusCode::pn532StatusOK) {return pass;}

    if(response.finalBuffer[3]!= 0x33){return statusCode::pn532StatusWrongCommand;}

    
    return statusCode::pn532StatusOK;
}

bool PN532_chip::detectCard(card& cardinfo, const uint8_t nCards, const uint8_t cardtype)
{
    uint8_t commands[] = {
        pn532::command::InListPassiveTarget,
        nCards,
        cardtype};

    auto command = setupSendCommand(commands, sizeof(commands) / sizeof(uint8_t));

    auto [pass, response] = sendCommandAndCheckAck(command);
    if(pass != statusCode::pn532StatusOK) {return false;}

    // 1 card has been found
    if (response.finalBuffer[4] == 1)
    {
        hwlib::cout << "Found 1 card with UID: ";
        cardinfo.setUID(response);
        for (int i = 0; i < response.finalBuffer[9]; i++)
        {
            hwlib::cout << hwlib::hex << response.finalBuffer[i + 10] << "  ";
        }
        hwlib::cout << hwlib::endl;
    }

    hwlib::cout << hwlib::endl;
    return true;
}

statusCode PN532_chip::setSerialBaudrate(const baudRate br)
{
    hwlib::cout << "Updating Serial Baudrate" << hwlib::endl;

    uint8_t commands[] = {
        pn532::command::setSerialBaudrate,
        br
    };
    auto command = setupSendCommand(commands, sizeof(commands) / sizeof(uint8_t));

    auto [pass, response] = sendCommandAndCheckAck(command);
    if(pass != statusCode::pn532StatusOK) {return pass;}

    if(response.finalBuffer[3] != 0x11){return statusCode::pn532StatusWrongCommand;}
    return statusCode::pn532StatusOK;
}

// ------------------------------------------------------------------------------- //
// Mifare specific functions                                                       //
// ------------------------------------------------------------------------------- //

statusCode PN532_chip::mifareReadPage(card &cardinfo, const uint8_t cardNumber, const uint8_t pageNumber)
{
    uint8_t commands[] = {
        pn532::command::InDataExchange,
        cardNumber,
        mifareCommands::Read16Bytes,
        pageNumber,
    };

    auto command = setupSendCommand(
        commands,
        sizeof(commands) / sizeof(uint8_t));

    auto [pass, response] = sendCommandAndCheckAck(command);
    if(pass != statusCode::pn532StatusOK) {return pass;}

    if (response.finalBuffer[4] != 0x00)
    {
        hwlib::cout << "read error: " << pageNumber << hwlib::endl;
        return statusCode::pn532StatusMifareAutError;
    }

    cardinfo.addPage(response.finalBuffer, response.length - 2, pageNumber);

    return statusCode::pn532StatusOK;
}

statusCode PN532_chip::mifareAuthenticate(card&cardinfo, uint8_t cardNumber, mifareCommands AorB, uint8_t pagenr, const uint8_t* key)
{
    hwlib::cout << "authenticate" << hwlib::endl;
    const std::array<uint8_t, 4> userid = cardinfo.getUID();

    uint8_t commands[] = {
        pn532::command::InDataExchange,
        cardNumber,
        AorB,
        pagenr,
        key[0], key[1], key[2], key[3], key[4], key[5],
        userid[0], userid[1], userid[2], userid[3]
    };

    auto command = setupSendCommand(
        commands,
        sizeof(commands) / sizeof(uint8_t)
    );

    auto [pass, response] = sendCommandAndCheckAck(command);
    if(pass != statusCode::pn532StatusOK) {return pass;}
    if(response.isSucces){
        pass = statusCode::pn532StatusInternalBufferOverflow;
    }
    if (response.finalBuffer[4]!= 0x00)
    {
        hwlib::cout << "Authentication error on page: " << pagenr << hwlib::endl;
        return statusCode::pn532StatusMifareAutError;
    }
    return statusCode::pn532StatusOK;
}

statusCode PN532_chip::mifareWritePage(card& cardinfo, uint8_t cardNumber, uint8_t pageNumber,const  char* data)
{
    hwlib::cout << "Writing page: " << pageNumber << hwlib::endl;

    uint8_t commands[] = {
        pn532::command::InDataExchange,
        cardNumber,
        mifareCommands::Write16Bytes,
        pageNumber,
        data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],
        data[8],data[9],data[10],data[11],data[12],data[13],data[14],data[15]
    };

    auto command = setupSendCommand(
        commands,
        sizeof(commands) / sizeof(uint8_t)
    );

    auto [pass, response] = sendCommandAndCheckAck(command);
    if(pass != statusCode::pn532StatusOK) {return pass;}
    if (response.finalBuffer[4] != 0x00)
    {
        hwlib::cout << "write unsuccessful" << hwlib::endl;
        return statusCode::pn532StatusMifareFramingError;
    }

    hwlib::cout << "write succesfull" << hwlib::endl;
    return statusCode::pn532StatusOK;
}

statusCode PN532_chip::mifareReadCard(card& cardInfo, const uint8_t cardNumber, const mifareCommands AorB, const cardKeys& authenticationKeys)
{
    hwlib::cout << "Reading entire mifare card" << hwlib::endl;
    uint8_t j = 0;
    for(int i = 0; i < 64; i++){
        // Make sure sector is authenticated first
        if(i%4 == 0){
            mifareAuthenticate(cardInfo, cardNumber, AorB, i+3, authenticationKeys.aKeys[j]); \
            j++;
        }
        // Read sector pages
        mifareReadPage(cardInfo, cardNumber, i);  
    }
    
    for(int i = 0; i < 64; i++){
        if(i%4 == 0){
                hwlib::cout << hwlib::endl;
        }
        cardInfo.readPage(i);
    }

 
    hwlib::cout << "reading complete" << hwlib::endl;
    hwlib::cout << hwlib::endl;hwlib::cout << hwlib::endl;hwlib::cout << hwlib::endl;

    return statusCode::pn532StatusOK;
}

statusCode PN532_chip::mifareMakeValueBlock(card&cardinfo, const uint8_t cardnumber, const mifareCommands AorB, const uint8_t pagenr, const uint8_t sector, const uint8_t* key)
{
    hwlib::cout << "Making value block on page: " << pagenr << hwlib::endl;
    char dataBlockFormat[] = {0x64,0x00,0x00,0x00,0x9B, 0xFF, 0xFF,0xFF,0x64,0x00,0x00,000,0x01,0xFE,0x01,0xFE}; // basic value block format of a mifare classic card

    auto auth_status = mifareAuthenticate(cardinfo, cardnumber, AorB, sector, key);
    auto status = mifareWritePage(cardinfo, cardnumber, pagenr, dataBlockFormat);

    if(status!= statusCode::pn532StatusOK || auth_status!= statusCode::pn532StatusOK) {return statusCode::pn532StatusWrongCommand;}

    return statusCode::pn532StatusOK;
}

statusCode PN532_chip::mifareIncrement(card&cardinfo, const uint8_t cardnumber, const mifareCommands AorB, const uint8_t pagenr, const uint8_t sector, const uint8_t* key, const uint32_t value)
{
  
    mifareAuthenticate(cardinfo, cardnumber, AorB, sector, key);

    uint8_t commands[] = {
        pn532::command::InDataExchange,
        cardnumber,
        mifareCommands::Incrementation,
        pagenr,
        static_cast<uint8_t>(value & 0xff),
        static_cast<uint8_t>((value >> 8) & 0xff),
        static_cast<uint8_t>((value >> 16) & 0xff),
        static_cast<uint8_t>((value >> 24) & 0xff),
    };
    
    auto command = setupSendCommand(
        commands,
        sizeof(commands) / sizeof(uint8_t)
    );

    auto [pass, response] = sendCommandAndCheckAck(command);
    if(pass != statusCode::pn532StatusOK) {return pass;}

    
    if(response.finalBuffer[4] != 0x00){return statusCode::pn532StatusWrongCommand;}

    return statusCode::pn532StatusOK;
}

statusCode PN532_chip::mifareTransfer(card&cardinfo, const uint8_t cardnumber, const mifareCommands AorB, const uint8_t pagenr, const uint8_t sector, const uint8_t* key)
{
    // make sure the block we want to transfer a value to is authenticated
    mifareAuthenticate(cardinfo, cardnumber, AorB, 0x07, key);

    uint8_t commands[] = {
        pn532::command::InDataExchange,
        cardnumber,
        nfc::mifareCommands::Transfare,
        pagenr
    };

    auto command = setupSendCommand(
        commands,
        sizeof(commands) / sizeof(uint8_t)
    );

    auto [pass, response] = sendCommandAndCheckAck(command);
    if(pass != statusCode::pn532StatusOK) {return pass;}
    
    if(response.finalBuffer[4] != 0x00){return statusCode::pn532StatusWrongCommand;}

    return statusCode::pn532StatusOK;
}

statusCode PN532_chip::mifareDecrement(card&cardinfo, const uint8_t cardnumber, const mifareCommands AorB, const uint8_t pagenr, const uint8_t sector, const uint8_t* key, const uint32_t value)
{
    // make sure the block we want to transfer a value to is authenticated
    mifareAuthenticate(cardinfo, cardnumber, AorB, sector, key);

    uint8_t commands[] = {
        pn532::command::InDataExchange,
        cardnumber,
        mifareCommands::Decrementation,
        pagenr,
        static_cast<uint8_t>(value & 0xff),
        static_cast<uint8_t>((value >> 8) & 0xff),
        static_cast<uint8_t>((value >> 16) & 0xff),
        static_cast<uint8_t>((value >> 24) & 0xff)     
    };

    auto command = setupSendCommand(
        commands,
        sizeof(commands) / sizeof(uint8_t)
    );

    auto [pass, response] = sendCommandAndCheckAck(command);
    if(pass != statusCode::pn532StatusOK) {return pass;}
    if(response.finalBuffer[4] != 0x00){return statusCode::pn532StatusWrongCommand;}

    return statusCode::pn532StatusOK;
}

} // namespace nfc