/**
 * @file
 * @brief     This file implements the functions declared in pn532Command.h
 * 
 * @author    Nathan Houwaart
 * @license   See LICENSE
 */


#include "../headers/pn532Command.h"

setupSendCommand::setupSendCommand(const uint8_t *commandsToSend, uint8_t commandSize)
{
    finalbuffer[0] = nfc::pn532::general::Preamble1;
    finalbuffer[1] = nfc::pn532::general::Preamble2;
    finalbuffer[2] = commandSize + static_cast<uint8_t>(1);
    finalbuffer[3] = calculateChecksum(finalbuffer, 2, 1);
    finalbuffer[4] = nfc::pn532::general::HostToPn532;
    uint8_t commandBufferIndex = 5;
    for (int i = 0; i < commandSize; i++)
    {
        finalbuffer[commandBufferIndex] = commandsToSend[i];
        commandBufferIndex++;
    }
    finalbuffer[commandBufferIndex] = calculateChecksum(finalbuffer, startCommand, commandSize + static_cast<uint8_t>(1));
    length = commandBufferIndex + static_cast<uint8_t>(1);
    ;
}

uint8_t setupSendCommand::calculateChecksum(const uint8_t *buffer, int index, uint8_t n)
{
    uint8_t som = 0x00;
    for (uint8_t i = 0; i < n; i++)
    {
        som += buffer[index + i];
    }
    uint8_t checksum = ~som;
    return checksum + static_cast<uint8_t>(1);
}


// TODO ::Remove or do something with isSucces
receivedCommand::receivedCommand(const uint8_t *receiveBufferP, uint8_t bufferSize)
{
    isSucces = true;
    if (receiveBufferP[0] == 0x00 && receiveBufferP[1]==0xff)
    {   
        length = receiveBufferP[2] + static_cast<uint8_t>(4);
        for (uint8_t i = 0; i < length; i++)
        {
            finalBuffer[i] = receiveBufferP[i + 2];
        }
    }
    else if(receiveBufferP[0]==1 && receiveBufferP[1]==0){
        isSucces = true;
        length = receiveBufferP[4] + static_cast<uint8_t>(4);
        for (uint8_t i = 0; i < length; i++)
        {
            finalBuffer[i] = receiveBufferP[i + 4];
        }
    }
}