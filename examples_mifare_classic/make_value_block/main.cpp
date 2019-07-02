/**
 * @file
 * @brief     Example use of the nfc library to create a Mifare value block on a Mifare classic 1k using the pn532
 * 
 * This file will demonstrate how to use the nfc library to create a valueblock with the pn532
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
 * a succesfull authentication, it will continue to write the correct data format to the page that needs to be converted to a value block.
 * 
 * When the valueblock is succesfully created, the pn532 will read that specific page and prints it out to make sure it has been formatted properly.
 * The layout of the valueblock is as follows:
 * 
 * 
 *    Value Block Bytes \n
 *    0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15   \n
 *    [   Value   ]   [   ~Value  ]   [   Value   ]   [A  ~A  A   ~A]
 * 
 * where:
 *      - value  : 4 byte signed value of the value block
 *      - ~value : 4 byte inverted valye
 *      - A      : Location of a backup value block
 *      - ~A     : inverted A, also storing location of a bakcup value block
 * 
 * If one of the functions fails, it will return a statuscode other than pn532StatusOK. The error cause can be deducted 
 * by comapring the given error code with the errors declared in: declarations.h
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

    nfc::cardKeys card1Keys;                // initialise card keys
    uint8_t  cardType               = nfc::pn532::command::CardType::TypeA_ISO_IEC14443;    // Declare the card type you want to read
    uint8_t  valueBlockPage         = 0x05;                                                 // Page that the valueblock needs to be created on
    uint8_t  sector                 = 0x07;                                                 // Sector the valueblock will be located in
    uint8_t* sectorKey              = card1Keys.aKeys[2];                                   // Key that needs to be authenticated with
    nfc::mifareCommands athenticateAorB  = nfc::mifareCommands::authenticateKeyA;           // Authenticate with key a or b
    
    //-----------------------//
    // ---- DECLARATIONS ----//
    //-----------------------//

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

    nfc::NFC *nfc = &chip;


    uint8_t sam = nfc->SAMConfiguration(nfc::pn532::command::SAMmode::Normal_mode);
    if(sam != nfc::statusCode::pn532StatusOK){hwlib::cout << "Error configuring SAM" << hwlib::endl; return sam;}
    uint8_t rf = nfc->setMaxRetries(0xFF);
    if(rf != nfc::statusCode::pn532StatusOK){hwlib::cout << "Error configuring max retries" << hwlib::endl; return sam;}

    for(;;){
        hwlib::cout << "Present card" << hwlib::endl;
        auto cardinfo = card();
        while(!nfc->detectCard(cardinfo, 0x01, cardType)){}

        auto result = nfc->mifareMakeValueBlock(cardinfo, 0x01, athenticateAorB, valueBlockPage, sector, sectorKey);
        if(result !=  nfc::statusCode::pn532StatusOK){hwlib::cout << "Error constructing value block" << hwlib::endl; hwlib::wait_ms(2000); continue;} 
        
        nfc->mifareAuthenticate(cardinfo, 0x01, athenticateAorB, sector, sectorKey);
        nfc->mifareReadPage(cardinfo, 0x01, 0x05);
        cardinfo.readPage(0x05); hwlib::cout << hwlib::endl;

        hwlib::cout << hwlib::endl<< "Valueblock created succesfully" << hwlib::endl;

        hwlib::wait_ms(5000);
    }
}