/**
 * @file
 * @brief     This file implements the functions declared in mifareClassic.h
 * 
 * @author    Nathan Houwaart
 * @license   See LICENSE
 */


#include "../headers/mifareClassic.h"

void card::addPage(uint8_t *bytes, size_t bytesSize, int pageNumber)
{
    int j = 0;
    for (size_t i = 5; i < bytesSize; i++)
    {
        cardData[(pageNumber * pageSize) + j] = bytes[i];
        j++;
    }
}

void card::readPage(int pageNumber) const
{

    for (size_t i = (pageNumber * pageSize) + 0; i < ((pageNumber * pageSize) + pageSize); i++)
    {
        hwlib::cout << "0x" << hwlib::setw(2) << hwlib::setfill('0') << hwlib::hex << cardData[i] << "  ";
    }
    hwlib::cout << hwlib::endl;
}

void card::setUID(receivedCommand& response){
    for(uint8_t i = 0; i < response.finalBuffer[9]; i++){
        cardUID[i] = response.finalBuffer[i+10];
    }
}

void card::setUID(uint8_t* uid){
    for(uint8_t i = 0; i < 4; i++){
        cardUID[i] = uid[i];
    }
}

std::array<uint8_t, 4> card::getUID() const {
    return cardUID;
}

std::array<uint8_t, nfc::pn532::general::Mifare1kPageSize> card::getPage(uint8_t page) const{
    using nfc::pn532::general::Mifare1kPageSize;

    uint8_t j = 0;
    std::array<uint8_t, Mifare1kPageSize> pageData = {};
    for(int i = page*Mifare1kPageSize; i < (page*Mifare1kPageSize)+Mifare1kPageSize; i++){
        pageData[j] = cardData[i];
        j++;
    }
    return pageData;
 }