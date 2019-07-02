/**
 * @file
 * @brief     Example use of the nfc library to write data to a mifare classic card with the pn532
 * 
 * This file will demonstrate how to use the nfc library to write a page of a mifare classic card.
 * The declerations can be changed in order to write variable data to a different page.
 * 
 * Make sure the external GPIO pins of the pn532 are set to the proper protocol and match the protocol selected in this file
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
 * setMaxRetries() command is used.
 * 
 * Afther this the pn532 is setup properly to write a card. Before a page can be written, the sector the page is located in needs to be authenticated
 * using the proper six byte key (default: 0xff 0xff 0xff 0xff 0xff 0xff). Once authenticated, the mifare classic card allows the pages of that 
 * specific vector to be written.
 * 
 * @warning:    If a sector trailer block is written in a wrong format, that specific sector might become corrupt/ unusable.
 *              Use care when writing to a sector trailer block!
 * 
 * Possible error causes:
 *      - The key is wrong for that specific vector
 *      - The access rights of the cards are set so that you can only autenticate with key A or B
 *      - The pn532 gave a timeout error
 *      - The pn532 has received a NACK (Not ACKnowleged) frame
 * 
 * The error cause can be deducted by comapring the given error code with the errors declared in: declarations.h
 * 
 * After a succesfull write, the pn532 will try to read the full content of the card. 
 * If the access bits / access keys have been changed using the write command, the pn532 may throw an authentication error.
 * Update the declarations.h keys for a specific sector if the key has been changed.
 * 
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

    nfc::cardKeys card1Keys;                                                                                                     // Initialize default card keys 6 x { 0xFF }
    uint8_t  cardType                    = nfc::pn532::command::CardType::TypeA_ISO_IEC14443;                                    // Cardtype we want to detect
    uint8_t  Page                        = 0x06;                                                                                 // Page we want to write
    uint8_t  sector                      = 0x07;                                                                                 // Sector we want to write in
    uint8_t* sectorKey                   = card1Keys.aKeys[2];                                                                   // Key of the sector we want to write in
    char     data[]                      = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0xFF,0x07,0x80,0x69,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};   // Data we want to write to the card
    nfc::mifareCommands athenticateAorB  = nfc::mifareCommands::authenticateKeyB;                                                // Wether we want to authenticate with key a or key b

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
    auto uartbus    = HardwareUart(115200);
    
    auto uartInterface  = communication::uart(uartbus);
    auto spiInterface   = communication::spi(spiBus, ss, irq);
    auto i2cInterface   = communication::i2c(nfc::pn532::general::Pn532Address, i2cbus, irq);

    auto oled    = hwlib::glcd_oled( i2cbus, 0x3c ); 
    auto font    = hwlib::font_default_8x8();
    auto display = hwlib::terminal_from( oled, font );   
    
    //auto chip = PN532_chip(spiInterface, irq);
    //auto chip = PN532_chip(uartInterface, irq);
    auto chip = nfc::PN532_chip(i2cInterface, irq);

    nfc::NFC *nfc = &chip;

    uint8_t sam = nfc->SAMConfiguration(nfc::pn532::command::SAMmode::Normal_mode);
    if(sam != nfc::statusCode::pn532StatusOK){hwlib::cout << "Error configuring SAM" << hwlib::endl; return sam;}
    uint8_t rf = nfc->setMaxRetries(0xFF);
    if(rf != nfc::statusCode::pn532StatusOK){hwlib::cout << "Error setting max retries" << hwlib::endl;return rf;}

    for(;;){
        hwlib::cout << "Present card" << hwlib::endl;
        auto cardinfo = card();
        while(!nfc->detectCard(cardinfo, 0x01, cardType)){}

        auto authenticate = nfc->mifareAuthenticate(cardinfo, 0x01, athenticateAorB, sector, sectorKey);
        if(authenticate != nfc::statusCode::pn532StatusOK){return authenticate;}

        auto write = nfc->mifareWritePage(cardinfo, 0x01, Page, data);
        if(write != nfc::statusCode::pn532StatusOK){hwlib::cout << "Write unsuccesfull: "<< write << hwlib::endl;}

        nfc->mifareAuthenticate(cardinfo, 0x01, athenticateAorB, sector, sectorKey);
        nfc->mifareReadPage(cardinfo, 0x01, Page);
        cardinfo.readPage(Page); hwlib::cout << hwlib::endl;

        hwlib::cout << hwlib::endl<< "Written page succesfully" << hwlib::endl;

        
        hwlib::wait_ms(5000);
    }
}