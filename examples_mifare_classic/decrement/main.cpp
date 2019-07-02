/**
 * @file
 * @brief     Example use of the nfc library to decrement a value block on a Mifare classic 1k using the pn532
 * 
 * This file will demonstrate how to use the nfc library to decrement valueblock with the pn532
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
 * After a card has entered the pn532's rf field, the pn532 will try to authenticate the sector with the key specified in the declarations. After
 * a succesfull authentication, it will continue to decrement the value stored in the valueblock by a specified value.
 * 
 * The pn532 will first send out the decrement command to the mifare classic. After a succesfull decrementation the value block is not yet changed. The 
 * decrementation result is only temporarily stored in a specific buffer on the Mifare card. In order to transfer the data form the internal buffer
 * to the page itself, the mifareTransfer() command needs to be executed. 
 * 
 * If this is succesfull, the valueblock is updated with its new value
 * 
 * If one of the functions fails, it will return a statuscode other than pn532StatusOK. The error cause can be deducted 
 * by comapring the given error code with the errors declared in: declarations.h
 * 
 * @author    Nathan Houwaart
 * @license   See LICENSE
 */

#include "../../code/headers/pn532Oled.h"

namespace target = hwlib::target;


int main() {
    //-----------------------//
    // ---- DECLARATIONS ----//
    //-----------------------//

   nfc::cardKeys card1Keys;                                                                // initialise card keys
    uint8_t  cardType               = nfc::pn532::command::CardType::TypeA_ISO_IEC14443;    // Declare the card type you want to read
    uint8_t  valueBlockPage         = 0x05;                                                 // Page that the valueblock is located on
    uint8_t  sector                 = 0x07;                                                 // Sector the valueblock will be located in
    uint8_t* sectorKey              = card1Keys.aKeys[2];                                   // Key that needs to be authenticated with
    nfc::mifareCommands athenticateAorB  = nfc::mifareCommands::authenticateKeyA;           // Authenticate with key a or b

    uint32_t decrementValue         = 50;                                                   // Value the valueblock needs to be incremented by

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
    
    //auto chip = PN532_chip(spiInterface, irq);
    //auto chip = PN532_chip(uartInterface, irq);
    auto chip = nfc::PN532_chip(i2cInterface, irq);

    auto terminal = nfc::NfcOled(chip, display, i2cInterface);


    nfc::NFC *nfc = &terminal;

    uint8_t sam = nfc->SAMConfiguration(nfc::pn532::command::SAMmode::Normal_mode);
    if(sam != nfc::statusCode::pn532StatusOK){return sam;}
    
    uint8_t rf = nfc->setMaxRetries(0xFF);
    if(rf != nfc::statusCode::pn532StatusOK){return rf;}

    for(;;){
	hwlib::cout << "Present card" << hwlib::endl;
        auto cardinfo = card();
        while(!nfc->detectCard(cardinfo, 0x01, cardType)){}

        
        nfc->mifareAuthenticate(cardinfo, 0x01, athenticateAorB, sector, sectorKey);
        nfc->mifareReadPage(cardinfo, 0x01, valueBlockPage);
        hwlib::cout << "Old valueblock: " << hwlib::endl;
        cardinfo.readPage(valueBlockPage); hwlib::cout << hwlib::endl;

        auto decrement = nfc->mifareDecrement(cardinfo, 0x01, athenticateAorB, valueBlockPage, sector, sectorKey, decrementValue);
        if(decrement !=  nfc::statusCode::pn532StatusOK){hwlib::cout << "Error decrementing" << hwlib::endl;continue;}
      

        auto transfer = nfc->mifareTransfer(cardinfo, 0x01, athenticateAorB, valueBlockPage, sector, sectorKey);
        if(transfer != nfc::statusCode::pn532StatusOK){hwlib::cout << "Error transferring" << hwlib::endl;continue;}

       
        nfc->mifareAuthenticate(cardinfo, 0x01, athenticateAorB, sector, sectorKey);
        nfc->mifareReadPage(cardinfo, 0x01, valueBlockPage);
        hwlib::cout << "New valueblock: " << hwlib::endl;
        cardinfo.readPage(valueBlockPage); hwlib::cout << hwlib::endl;


        hwlib::wait_ms(2000);
    }
}
