/**
 * @file
 * @brief     Example use of the nfc library to read a entire Mifare Classic 1k card with the pn532
 * 
 * This file will demonstrate how to use the nfc library to read a entire Mifare Classic 1k card.
 * The declerations can be changed in order to write variable data to a different page.
 * 
 * Make sure the external GPIO pins of the pn532 are set to the proper protocol and match the protocol selected in this file.
 * 
 * GPIO pin layout: \n
 *      P1 | P2 =  protocol: \n
 *       0 | 0  =  UART \n
 *       0 | 1  =  SPI \n
 *       1 | 0  =  i2C \n
 * 
 * 
 * After the pn532 class has been made, the pn532 first needs to be initialised. The SAM (Security Access Module) needs to be deactivated
 * in order to setup te pn532 as a normal card reader. The pn532 will indefinitely try to activate a card within it's RF field when the 
 * setMaxRetries(0xFF) command is used.
 * 
 * After a card has entered the pn532's rf field, the pn532 will try to read the full content of the card. It is doing so by authenticating
 * every sector trailer block with the key specified in the struct cardKeys in declerations.h
 * 
 * Possible error causes:
 *      - The key is wrong for that specific vector
 *      - The access rights of the cards are set so that you can only autenticate with key A or B
 *      - The pn532 gave a timeout error
 *      - The pn532 has received a NACK (Not ACKnowleged) frame
 *      - If one sector throws an authentication error, the rest of the card cannot be read. It needs to be reinitialised by the reader
 *      - The card is removed from the pn532's rf field before completing the entire read
 * 
 * The error cause can be deducted by comapring the given error code with the errors declared in: declarations.h
 * 
 * After a full read ( succesfull, partial or not succesfull ), the complete Cardbuffer will be printed out in the terminal with the data
 * that the pn532 has extracted form the card. Sectors that have not been read properly, will contain all 0's (0x00)
 * 
 * @author    Nathan Houwaart
 * @license   See LICENSE
 */

#include "../../code/headers/pn532.h"

namespace target = hwlib::target;

int main() {
    //-----------------------//
    // ---- DECLARATIONS ----//
    //-----------------------//

    nfc::cardKeys card1Keys;                                                                // Initialise default card keys
    uint8_t  cardType               =  nfc::pn532::command::CardType::TypeA_ISO_IEC14443;   // Cardtype we want to detect
    
    // Kill watchdog
    WDT->WDT_MR = WDT_MR_WDDIS;
    // Wait till everything is initialised
    hwlib::wait_ms(1000);

    // set i2c communication pins
    auto scl    = target::pin_oc(target::pins::scl);
    auto sda    = target::pin_oc(target::pins::sda);
    auto irq    = target::pin_in(target::pins::d2);

    // SPI communication pins
    auto mosi   = target::pin_out( target::pins::d11 );
    auto miso   = target::pin_in( target::pins::d12 );
    auto clock  = target::pin_out( target::pins::d13 );
    auto ss     = target::pin_out( target::pins::d10 );

    // construct communication busses
    auto i2cbus     = hwlib::i2c_bus_bit_banged_scl_sda(scl, sda);
    auto spiBus     = hwlib::spi_bus_bit_banged_sclk_mosi_miso(clock, mosi, miso);
    auto uartbus    = hwuart::HardwareUart(115200);
    
    auto uartInterface  = communication::uart(uartbus);
    auto spiInterface   = communication::spi(spiBus, ss, irq);
    auto i2cInterface   = communication::i2c(nfc::pn532::general::Pn532Address, i2cbus, irq);

    auto oled    = hwlib::glcd_oled( i2cbus, 0x3c ); 
    auto font    = hwlib::font_default_8x8();
    auto display = hwlib::terminal_from( oled, font );   
    
    //auto chip = nfc::PN532_chip(spiInterface,display, irq);
    //auto chip = nfc::PN532_chip(uartInterface, display irq);
    auto chip = nfc::PN532_chip(i2cInterface, irq);

    nfc::NFC *nfc = &chip;

    uint8_t sam = nfc->SAMConfiguration(nfc::pn532::command::SAMmode::Normal_mode);
    if(sam != nfc::statusCode::pn532StatusOK){hwlib::cout << "Error configuring SAM" << hwlib::endl; return sam;}

    uint8_t rf = nfc->setMaxRetries(0xFF);
    if(rf != nfc::statusCode::pn532StatusOK){hwlib::cout << "Error setting max retries" << hwlib::endl;return rf;}

    for(;;){
        hwlib::cout << "Present card" << hwlib::endl;
        // construct an empty card class with a reset ( all 0x00's) buffer
        auto cardinfo = card();

        // Wait for a nfc card to be detected by the pn532
        while(!nfc->detectCard(cardinfo, 0x01, cardType)){}

        // If a card has been detected, try to read the entire card
        nfc->mifareReadCard(cardinfo, 0x01, nfc::mifareCommands::authenticateKeyA, card1Keys);

        hwlib::wait_ms(5000);
    }
}