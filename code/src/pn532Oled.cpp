/**
 * @file
 * @brief     This file implements the functions declared in pn532Oled.h
 * 
 * @author    Nathan Houwaart
 * @license   See LICENSE
 */


#include "../headers/pn532Oled.h"

namespace nfc{

// ------------------------------------------------------------------------------- //
// Constructor                                                                     //
// ------------------------------------------------------------------------------- //

NfcOled::NfcOled(
    NFC & slave, 
    hwlib::terminal_from& display, 
    communication::protocol& _protocol
):
    NFC(_protocol), 
    slave(slave), 
    display(display) 
{
        init();
}

// ------------------------------------------------------------------------------- //
// Basic communciation functions                                                   //
// ------------------------------------------------------------------------------- //

void NfcOled::init()
{
        slave.init();
        display << "\f\v" << "NFC  Setting up" << hwlib::flush;
        hwlib::wait_ms(500);
}

void NfcOled::sendData(uint8_t *commandBuffer, const uint8_t nBytes)
{
    slave.sendData(commandBuffer, nBytes);
}   

void NfcOled::getData(uint8_t *buffer, const uint8_t nBytes) 
{
    slave.getData(buffer, nBytes);
} 

statusCode NfcOled::writeRegister(const uint16_t reg, const uint8_t val)
{
    return slave.writeRegister(reg, val);
}

std::array<uint8_t, 2> NfcOled::readRegister(const uint16_t reg)
{
    return slave.readRegister(reg);
}

statusCode NfcOled::writeGPIO(uint8_t newPinState) 
{
    return slave.writeGPIO(newPinState);
}

std::array<uint8_t, 2> NfcOled::readGPIO()
{
    return slave.readGPIO();
}

// ------------------------------------------------------------------------------- //
// Basic functions                                                                 //
// ------------------------------------------------------------------------------- //

bool NfcOled::waitForChip(const int timeout) 
{
    return slave.waitForChip(timeout);
}

bool NfcOled::checkAck(const uint8_t *buffer, const uint8_t n)
{
    return slave.checkAck(buffer, n);
}


Result NfcOled::sendCommandAndCheckAck(setupSendCommand &command)
{
    return slave.sendCommandAndCheckAck(command);
}


// ------------------------------------------------------------------------------- //   
// More advanced functions                                                         //
// ------------------------------------------------------------------------------- //

statusCode NfcOled::performSelftest()
{
    auto status = slave.performSelftest();
    if(status != statusCode::pn532StatusOK){return status;}

    display << "\v" << "\n\n" << "Selftest status" << "\n" << "OK" << hwlib::flush;
    hwlib::wait_ms(1000);

    return status;
}

std::array<uint8_t, 5> NfcOled::getGeneralStatus()
{
    auto status = slave.getGeneralStatus();
    if(status[0] != statusCode::pn532StatusOK){return status;}

    display     << "\v" << "\n\n" << "General status:" << 
        '\n' << "Last err: 0x" << hwlib::setw(2) << hwlib::setfill('0') << hwlib::hex << status[1] << 
        '\n' << "External RF: " << status[2] << 
        '\n' << "Cards ctr: " << status[3] << 
        '\n' << "SAM status: 0x" << hwlib::setw(2) << hwlib::setfill('0') << hwlib::hex << status[4] << hwlib::flush;
    hwlib::wait_ms(5000);

    return status;
}

std::array<uint8_t, 5> NfcOled::getFirmwareVersion()
{
    auto status = slave.getFirmwareVersion();
    if(status[0] != statusCode::pn532StatusOK){
        display << "\v\n\n" << "No device found" << hwlib::flush;
    }else{
        display << "\v" << "\n\n" << "Found device:" << "\n" <<  "PN5" << hwlib::hex << status[1] << 
                    "\n" << "Firmware version:"  << "\n" << status[2] << '.' << status[3] << 
                    "\n" << "Support version:" << "\n" << status[4] << hwlib::flush;
    }
    hwlib::wait_ms(2000);
    return status;
}

statusCode NfcOled::SAMConfiguration(const uint8_t mode)
{
    auto status = slave.SAMConfiguration(mode);
    if(status != statusCode::pn532StatusOK){
        display << "\f" << "NFC   SAM Error" << hwlib::flush;
    }else{
        display << "\f" << "NFC       RF: ON" << hwlib::flush;
    }
    hwlib::wait_ms(1000);
    return status;
}

statusCode NfcOled::RFField(const bool state)
{
    auto status = slave.RFField(state);
    if(status != statusCode::pn532StatusOK){return status;}

    if(state){display << "\v" << "NFC       RF: ON";}
    else{display << "\v" << "NFC       RF:OFF";}
    display << hwlib::flush;

    return status;
}

statusCode NfcOled::setMaxRetries(const uint8_t maxRetries)
{
    auto status = slave.setMaxRetries(maxRetries);
    if(status != statusCode::pn532StatusOK){
        display << "\v\n\n" << "RF config: Fail" << hwlib::flush;
    }else{
        display << "\v\n\n" << "RF config: OK" << hwlib::flush;
    }
    hwlib::wait_ms(1000);
    return status;
}

bool NfcOled::detectCard(card& cardinfo, const uint8_t nCards, const uint8_t cardtype)
{
    display << "\v\n\n" << " "<<"\n" << "Present card" << hwlib::flush;
    return slave.detectCard(cardinfo, nCards, cardtype);
}

statusCode NfcOled::selectCard()
{
    return statusCode::pn532StatusOK;
}

statusCode NfcOled::setSerialBaudrate(const baudRate br)
{
    return slave.setSerialBaudrate(br);
}


// ------------------------------------------------------------------------------- //
// Mifare specific functions                                                       //
// ------------------------------------------------------------------------------- //

statusCode NfcOled::mifareReadPage(card &cardinfo, const uint8_t cardNumber, const uint8_t pageNumber)
{
    return slave.mifareReadPage(cardinfo, cardNumber, pageNumber);
};

statusCode NfcOled::mifareAuthenticate(card&cardinfo, uint8_t cardNumber, mifareCommands AorB, uint8_t pagenr, const uint8_t* key)
{
    auto status = slave.mifareAuthenticate(cardinfo, cardNumber, AorB, pagenr, key);
    if(status != statusCode::pn532StatusOK){
        display << "\v\n\n\n\n\n\n" << "Auth error..." << hwlib::flush;
    }
    return status;
}

statusCode NfcOled::mifareWritePage(card& cardinfo, uint8_t cardNumber, uint8_t pageNumber,const  char* data)
{
    return slave.mifareWritePage(cardinfo, cardNumber, pageNumber, data);
}

statusCode NfcOled::mifareReadCard(card& cardInfo, const uint8_t cardNumber, const mifareCommands AorB, const cardKeys& authenticationKeys)
{
    display << "\v\n\n\n\n\n\n" << "Reading card" << hwlib::flush;
    auto status = slave.mifareReadCard(cardInfo, cardNumber, AorB, authenticationKeys);

    display << "\v\n\n\n\n\n\n" << "Reading complete" << hwlib::flush;
    return status;
}

statusCode NfcOled::mifareMakeValueBlock(card&cardinfo, const uint8_t cardNumber, const mifareCommands AorB, const uint8_t pagenr, const uint8_t sector, const uint8_t* key)
{
    auto status = slave.mifareMakeValueBlock(cardinfo, cardNumber, AorB, pagenr, sector,key);
    if(status != statusCode::pn532StatusOK){
        display << "\v\n\n\n\n\n\n" << "value block" << "\n" << "format error" << hwlib::flush;
    }else{
            display << "\v\n\n\n\n\n\n" << "value block" << "\n" << "format ok" << hwlib::flush;
    }
    hwlib::wait_ms(1000);
    return status;
}

statusCode NfcOled::mifareIncrement(card&cardinfo, const uint8_t cardNumber, const mifareCommands AorB, const uint8_t pagenr, const uint8_t sector, const uint8_t* key, const uint32_t value)
{
    return slave.mifareIncrement(cardinfo, cardNumber, AorB, pagenr, sector, key, value);
}

statusCode NfcOled::mifareTransfer(card&cardinfo, const uint8_t cardnumber, const mifareCommands AorB, const uint8_t pagenr, const uint8_t sector, const uint8_t* key)
{
    return slave.mifareTransfer(cardinfo, cardnumber, AorB, pagenr, sector, key);
}

statusCode NfcOled::mifareDecrement(card&cardinfo, const uint8_t cardnumber, const mifareCommands AorB, const uint8_t pagenr, const uint8_t sector, const uint8_t* key, const uint32_t value)
{
    return slave.mifareDecrement(cardinfo, cardnumber, AorB, pagenr, sector, key, value);
}

} // namespace n{
