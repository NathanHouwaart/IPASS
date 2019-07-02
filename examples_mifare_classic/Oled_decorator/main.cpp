/**
 * @file
 * @brief     Example use of the nfc oled decorator to integrate an oled display with the pn532
 * 
 * This file will demonstrate how to use the nfc oled decorator to integrate an oled display with the pn532
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
 * In order to use an oled display in combination with the nfc chip, the class nfc::NfcOled can be used.
 * This class takes three agruments in it's constructor:
 *      - The already made NFC chip class ( nfc::PN532_chip )
 *      - A display class that communicates with the display ( hwlib::terminal_from )
 *      - A protocol . This variable is not used, but is needed to instantiate the decorator
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
 * @author    Nathan Houwaart
 * @license   See LICENSE
 */

#include "../../code/headers/pn532Oled.h"

namespace target = hwlib::target;

int main() {
    // Kill watchdog
    WDT->WDT_MR = WDT_MR_WDDIS;
    // Wait till everything is initialised
    hwlib::wait_ms(1000);

    // set i2c communication pins
    auto scl = target::pin_oc(target::pins::scl);
    auto sda = target::pin_oc(target::pins::sda);
    auto irq = target::pin_in(target::pins::d2);

    // SPI communication pins
    auto mosi = target::pin_out( target::pins::d11 );
    auto miso   = target::pin_in( target::pins::d12 );
    auto clock = target::pin_out( target::pins::d13 );
    auto ss = target::pin_out( target::pins::d10 );

    // construct communication busses
    auto i2cbus = hwlib::i2c_bus_bit_banged_scl_sda(scl, sda);
    auto spiBus  = hwlib::spi_bus_bit_banged_sclk_mosi_miso(clock, mosi, miso);
    auto uartbus = HardwareUart(115200);
    
    auto uartInterface = communication::uart(uartbus);
    auto spiInterface = communication::spi(spiBus, ss, irq);
    auto i2cInterface = communication::i2c(nfc::pn532::general::Pn532Address, i2cbus, irq);
    hwlib::wait_ms(2);

    auto oled    = hwlib::glcd_oled( i2cbus, 0x3c ); 
   
    auto font    = hwlib::font_default_8x8();
    auto display = hwlib::terminal_from( oled, font );   

    hwlib::wait_ms(1000);

    //auto card = nfc::PN532_chip(spiInterface, irq);
    //auto card = nfc::PN532_chip(uartInterface, irq);
    auto card = nfc::PN532_chip(i2cInterface, irq);


    auto chip = nfc::NfcOled(card, display, spiInterface);  /// < ------- nfc::NfcOled instead of nfc::PN532_chip

    nfc::NFC *nfc = &chip;

   for(;;){
        

        hwlib::wait_ms(2000);

        // ----- SELFTEST ----- //
        uint8_t selftest = nfc->performSelftest();
        if(selftest == nfc::statusCode::pn532StatusOK){
            hwlib::cout << "Comminication line test: Passed" << hwlib::endl;
        }else{
            hwlib::cout << "Communication line test: failed" << hwlib::endl;
        }

        hwlib::wait_ms(2000);
        
        // ----- GENERAL STATUS ----- //
        auto generalStatus = nfc->getGeneralStatus();
        if(generalStatus[0] == nfc::statusCode::pn532StatusOK){
            hwlib::cout << "General Status pn532" << hwlib::endl;
            hwlib::cout << "Last error: 0x"<< hwlib::setw(2) << hwlib::setfill('0') << hwlib::hex << generalStatus[1] << hwlib::endl;
            hwlib::cout << "External RF field detected: "<< generalStatus[2] << hwlib::endl;
            hwlib::cout << "Amount of cards controlled: "<< generalStatus[3] << hwlib::endl;
            hwlib::cout << "Sam status connection: " << generalStatus[4] << hwlib::endl;
            hwlib::cout << hwlib::endl;
        }else{
            hwlib::cout << "error" << hwlib::endl;
        }


        hwlib::wait_ms(2000);
    }
    
}
