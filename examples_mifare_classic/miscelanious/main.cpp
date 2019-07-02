/**
 * @file
 * @brief     Example use of the nfc library to execute various miscelanious commands on the PN532
 * 
 * This file will demonstrate how to use the nfc library to execute various miscelanious commands on the PN532
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
 * There are serveral commands that can be used by the pn532
 *      - writeRegister()       write to a specific register
 *      - readRegister()        read from a specific register
 *      - readGPIO()            read the gpio pin state
 *      - writeGPIO()           write the gpio pin state
 *      - RFField()             sitch the rf field on or off
 * 
 * @warning Use writeGPIO() function at your own risk. Misusing this function may cause the pn532 to be damaged or cause undefined behaviour
 * 
 * If one of the functions fails, it will return a statuscode other than pn532StatusOK. The error cause can be deducted 
 * by comapring the given error code with the errors declared in: declarations.h
 * 
 * @author    Nathan Houwaart
 * @license   See LICENSE
 */

#include "../../code/headers/pn532.h"

namespace target = hwlib::target;

// Serial Baudrate is aangtepast in: hwlib/core/defines. onderaan
// en in makefile due
// TMAX = 15 ms --- p. 40 manual
// if no ack frame detected within TMAX --> send same cmd again.
//pn532 does not inform if its awaken p.50. Wait at leas t_osc_start for sending a cmd


int main() {
    // Kill watchdog
    WDT->WDT_MR = WDT_MR_WDDIS;
    // Wait till everything is initialised
    hwlib::wait_ms(1000);

    nfc::cardKeys card1Keys;
    uint16_t reg1Addr   = 0x6332;       // Register 1 we want to read and write
    uint8_t reg1Data    = 0x39;         // Write value
    uint16_t reg2Addr   = 0xFFF4;       // Register 1 we want to read and write       
    uint8_t reg2Data    = 0x9A;         // Write value

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
    
    //auto chip = nfc::PN532_chip(spiInterface, irq);
    //auto chip = nfc::PN532_chip(uartInterface, irq);
    auto chip = nfc::PN532_chip(i2cInterface, irq);



    nfc::NFC *nfc = &chip;
   
    uint8_t sam = nfc->SAMConfiguration(nfc::pn532::command::SAMmode::Normal_mode);
    if(sam != nfc::statusCode::pn532StatusOK){hwlib::cout << "Error configuring SAM" << hwlib::endl; return sam;}

    uint8_t maxRetries = nfc->setMaxRetries(0xFF);
    if(maxRetries != nfc::statusCode::pn532StatusOK){ hwlib::cout << "Error setting max retries";}

    for(;;){
        // ----- write to a XRAM register ----- //
        // write 0x39 to register 0x6332        //
      
        hwlib::cout << hwlib::hex << "Writing 0x" << reg1Data << " in register: 0x" << reg1Addr << hwlib::endl;
        auto writeRegiser1Status = nfc->writeRegister(reg1Addr, reg1Data);    
        if(writeRegiser1Status != nfc::statusCode::pn532StatusOK){hwlib::cout << "Error writing to register: " << hwlib::hex << reg1Addr << hwlib::endl<< hwlib::endl;}
        else{hwlib::cout << hwlib::hex << "Register 0x" << reg1Addr << " written succesfully" << hwlib::endl<< hwlib::endl;} 


        // ----- read from a XRAM register ----- //
        // read from register 0x6332             //
        hwlib::cout << hwlib::hex << "Reading from register 0x" << reg1Addr << hwlib::endl;
        auto register1Content = nfc->readRegister(reg1Addr);
        if(register1Content[0] != nfc::statusCode::pn532StatusOK){hwlib::cout << hwlib::hex << "Error reading from register: " << reg1Addr << hwlib::endl<< hwlib::endl;}
        else{hwlib::cout << "register content: " << register1Content[1] << hwlib::endl<< hwlib::endl;}


        // ----- write from a SFR register ----- //
        // write 0xA0 to register 0xF4           // 
        hwlib::cout << hwlib::hex << "Writing 0x" << reg2Data << " in register: 0x" << reg2Addr << hwlib::endl;
        auto writeRegiser2Status = nfc->writeRegister(reg2Addr, reg2Data);
        if(writeRegiser2Status != nfc::statusCode::pn532StatusOK){hwlib::cout << "Error writing to register: " << hwlib::hex << reg2Addr << hwlib::endl<< hwlib::endl;}
        else{hwlib::cout << "register written succesfully: " << hwlib::endl<< hwlib::endl;} 


        // ----- read from a SFR register ----- //
        // read  from register 0xF4             //
        hwlib::cout << hwlib::hex << "Reading from register 0x" << reg2Addr << hwlib::endl;
        auto register2Content = nfc->readRegister(reg2Addr);
        if(register2Content[0] != nfc::statusCode::pn532StatusOK){hwlib::cout << hwlib::hex << "Error reading from register: " << reg1Addr << hwlib::endl<< hwlib::endl;}
        else{hwlib::cout << "register content: " << register2Content[1] << hwlib::endl << hwlib::endl; }


        // ----- read GPIO pins from the pn532 ----- //
        hwlib::cout << "Reading GPIO pins" << hwlib::endl;
        auto gpioState = nfc->readGPIO();
        if(gpioState[0]!= nfc::statusCode::pn532StatusOK){hwlib::cout << "Error reading GPIO pins: " << gpioState[0] << hwlib::endl << hwlib::endl;}
        else{hwlib::cout << "GPIO state: " << hwlib::hex << gpioState[1] << hwlib::endl << hwlib::endl;;}

        
        // ----- write GPIO pins of the pn532 ----- //
        // WARNING: This can cause undefined behaviour of the pn532 and may even damage the chip. Use at your own risk
        //auto gpioState = nfc->writeGPIO(0xFF);   
        

        // ----- switch antenna off ----- //
        auto antennaStatusOff = nfc->RFField(0);
        if(antennaStatusOff!= nfc::statusCode::pn532StatusOK){hwlib::cout<< "error turning off RF field: " << hwlib::hex << antennaStatusOff << hwlib::endl;}
        else{hwlib::cout<< "succesfully switched RF field off" << hwlib::endl;}
    
    
        hwlib::wait_ms(2000);


        // ----- switch antenna on ----- //
        auto antennaStatusOn = nfc->RFField(1);
        if(antennaStatusOn!= nfc::statusCode::pn532StatusOK){hwlib::cout<< "error turning on RF field: " << hwlib::hex << antennaStatusOn << hwlib::endl;}
        else{hwlib::cout<< "succesfully switched RF field on" << hwlib::endl;}


        hwlib::wait_ms(4000);
    }
    
}
