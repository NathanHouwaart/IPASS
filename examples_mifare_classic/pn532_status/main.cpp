/**
 * @file
 * @brief     Example use of the nfc library to get the general status of the PN532
 * 
 * This file will demonstrate how to use the nfc library to get the general status of the PN532
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
 * There are three commands that can give the user information about the pn532's status:
 *      - GetFirmwareVersion()
 *      - performSelftest()
 *      - getGeneralStatus()
 * 
 * GetFirmwareVersion() is often used at the beginning of a program to see wether the pn532 is connected with the host controller or not.
 * performSelftest() is a helpfull tool to check if the communication between the chip and host controller is working properly
 * getGeneralStatus() is a used to determine the current state of the pn532. The last error thrown, the amount of cards controlled and the SAM (Security Access Module) state
 * 
 * If one of the functions fails, it will return a statuscode other than pn532StatusOK. The error cause can be deducted 
 * by comapring the given error code with the errors declared in: declarations.h
 * 
 * 
 * @author    Nathan Houwaart
 * @license   See LICENSE
 */

#include "../../code/headers/pn532.h"

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
    auto uartbus = hwuart::HardwareUart(115200);
    
    auto uartInterface = communication::uart(uartbus);
    auto spiInterface = communication::spi(spiBus, ss, irq);
    auto i2cInterface = communication::i2c(nfc::pn532::general::Pn532Address, i2cbus, irq);
    hwlib::wait_ms(2);

    auto oled    = hwlib::glcd_oled( i2cbus, 0x3c ); 
   
    auto font    = hwlib::font_default_8x8();
    auto display = hwlib::terminal_from( oled, font );   
    
    //auto chip = PN532_chip(spiInterface, irq);
    //auto chip = PN532_chip(uartInterface, irq);
    auto chip = nfc::PN532_chip(i2cInterface, irq);

    nfc::NFC *nfc = &chip;

    for(;;){
        // ----- FIRMWARE ----- //
        auto firmware = nfc->getFirmwareVersion();
        if(firmware[0] == nfc::statusCode::pn532StatusOK){
            hwlib::cout << "Found device:     pn5" << hwlib::hex << firmware[1] << hwlib::endl;
            hwlib::cout << "Firmware version: " << static_cast<uint8_t>(firmware[2]) << '.' << static_cast<uint8_t>(firmware[3]) << hwlib::endl;
            hwlib::cout << "Support version:  " << static_cast<uint8_t>(firmware[4]) << hwlib::endl;
            hwlib::cout << hwlib::endl;
        }else{
            hwlib::cout << "Error getting firmware version: " << firmware[0] <<  hwlib::endl;
        }

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
