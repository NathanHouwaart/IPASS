/**
 * @file
 * @brief     This file implements a decorator for the NFC class in order to be used with an oled display
 * 
 * This file implements a decorator for the abstract NFC class so an oled screen can be used to display data on
 
 * Make sure the external GPIO pins of the pn532 are set to the proper protocol and match the protocol selected in this file.
 * 
 * GPIO pin layout: \n
 *      P1 | P2 =  protocol: \n
 *       0 | 0  =  UART \n
 *       0 | 1  =  SPI \n
 *       1 | 0  =  i2C \n
 * 
 * Functions that write to the display by default:
 *      - NFC->Init()
 *      - NFC->performSelfTest()
 *      - NFC->getGeneralStatus()
 *      - NFC->getFirmwareVersion()
 *      - NFC->SAMConfiguration()
 *      - NFC->RFField()
 *      - NFC->detectCard()
 *      - NFC->mifareAuthenticate()
 *      - NFC->mifareReadCard()
 *      - NFC->mifareMakeValueBlock()
 * 
 * @note    The content that is written to the display can be changed / altered in the pn532Oled.cpp file. When you do,
 *          just make sure there is a hwlib::wait_ms( x ) after you write to the oled. Otherwise the contend will be
 *          overwritten too quickly in order to read it. 
 * 
 * 
 * @author    Nathan Houwaart
 * @license   See LICENSE
 */

#ifndef V1_OOPC_18_NATHANHOUWAART_NFCOLED_H
#define V1_OOPC_18_NATHANHOUWAART_NFCOLED_H

#include "pn532.h"

namespace nfc {

/// \brief
/// Oled decorator for the NFC class.
/// \details
/// With a decorator, additional features can be added on top of the already existing NFC funcions.
/// This specific decorator does that by executing a command, and displaying ( most of ) it's 
/// output on an oled display.
/// Not all functions make use of the oled display (yet). Those functions will just call their slave
/// counterfit functions
class NfcOled : public NFC {
private:

    NFC                 & slave;
    hwlib::terminal_from& display;

public:

    // ------------------------------------------------------------------------------- //
    // Constructor                                                                     //
    // ------------------------------------------------------------------------------- //

    /// \brief
    /// Constructor for the Oled decorator
    /// \details
    /// Takes a display as additional argument
    NfcOled(
        NFC & slave, 
        hwlib::terminal_from& display, 
        communication::protocol& _protocol
    );

    // ------------------------------------------------------------------------------- //
    // Basic communciation functions                                                   //
    // ------------------------------------------------------------------------------- //

    /// \brief
    /// Initialise the pn532 and display the state on the display
    void init() override;

    /// \brief
    /// Same as slave.sendData()
    void sendData(uint8_t *commandBuffer, const uint8_t nBytes) override;

    /// \brief
    /// Same as slave.getData()
    void getData(uint8_t *buffer, const uint8_t nBytes) override;

    /// \brief
    /// Same as slave.writeRegister()
    statusCode writeRegister(const uint16_t reg, const uint8_t val) override;

    /// \brief
    /// Same as slave.readRegister()
    std::array<uint8_t, 2> readRegister(const uint16_t reg) override;

    /// \brief
    /// Same as slave.writeGPIO()
    statusCode writeGPIO(uint8_t newPinState) override;

    /// \brief
    /// Same as slave.readGPIO()
    std::array<uint8_t, 2> readGPIO() override;


    // ------------------------------------------------------------------------------- //
    // Basic functions                                                                 //
    // ------------------------------------------------------------------------------- //

    /// \brief
    /// Same as slave.waitForChip
    bool waitForChip(const int timeout) override;

    /// \brief
    /// Same as slave.checkAck()
    bool checkAck(const uint8_t *buffer, const uint8_t n) override;

    /// \brief
    /// Same as slave.sendCommandAndCheckAck()
    Result sendCommandAndCheckAck(setupSendCommand &command) override;


    // ------------------------------------------------------------------------------- //   
    // More advanced functions                                                         //
    // ------------------------------------------------------------------------------- //

    /// \brief
    /// Perform a selftest and display it's outcome on the display
    statusCode performSelftest() override;

    /// \brief
    /// Get the general status of the pn532 and display the result on the display
    std::array<uint8_t, 5> getGeneralStatus() override;

    /// \brief
    /// Get the firmware version of the pn532 and display the result on the display
    std::array<uint8_t, 5> getFirmwareVersion() override;

    /// \brief
    /// Conigure tha SAM and display the status on the display
    statusCode SAMConfiguration(const uint8_t mode) override;

    /// \brief
    /// Switch the RF fiel on or off and display the result on the display
    statusCode RFField(const bool state) override;

    /// \brief
    /// Set the max retries and display the result on the display
    statusCode setMaxRetries(const uint8_t maxRetries)override;

    /// \brief
    /// Wait for a card and update the display to inform that the pn532 is ready to detect a card
    bool detectCard(card& cardinfo, const uint8_t nCards, const uint8_t cardtype) override;

    /// \brief
    /// Same as slave.selectCard()
    statusCode selectCard() override;

    /// \brief
    /// Same as slave.setSerialBaudrate()
    statusCode setSerialBaudrate(const baudRate br) override;


    // ------------------------------------------------------------------------------- //
    // Mifare specific functions                                                       //
    // ------------------------------------------------------------------------------- //

    /// \brief
    /// Same as slave.mifareReadPage()
    statusCode mifareReadPage(card &cardinfo, const uint8_t cardNumber, const uint8_t pageNumber);

    /// \brief
    /// Try to authenticate a sector trailer block and display the status on the display
    statusCode mifareAuthenticate(card&cardinfo, uint8_t cardNumber, mifareCommands AorB, uint8_t pagenr, const uint8_t* key) override;

    /// \brief
    /// Same as slave.mifareWritePage()    
    statusCode mifareWritePage(card& cardinfo, uint8_t cardNumber, uint8_t pageNumber,const  char* data)override;

    /// \brief
    /// Will try to read a card and will display the status on the display()
    statusCode mifareReadCard(card& cardInfo, const uint8_t cardNumber, const mifareCommands AorB, const cardKeys& authenticationKeys)override;

    /// \brief
    /// Will try to make a value block and will display the status on the display()
    statusCode mifareMakeValueBlock(card&cardinfo, const uint8_t cardNumber, const mifareCommands AorB, const uint8_t pagenr, const uint8_t sector, const uint8_t* key)override;

    /// \brief
    /// Same as slave.mifareIncrement()
    statusCode mifareIncrement(card&cardinfo, const uint8_t cardNumber, const mifareCommands AorB, const uint8_t pagenr, const uint8_t sector, const uint8_t* key, const uint32_t value) override;

    /// \brief
    /// Same as slave.mifareTransfer()
    statusCode mifareTransfer(card&cardinfo, const uint8_t cardnumber, const mifareCommands AorB, const uint8_t pagenr, const uint8_t sector, const uint8_t* key)override;

    /// \brief
    /// Same as slave.mifareDecrement()
    statusCode mifareDecrement(card&cardinfo, const uint8_t cardnumber, const mifareCommands AorB, const uint8_t pagenr, const uint8_t sector, const uint8_t* key, const uint32_t value)override;

};
} // namespace nfc


#endif