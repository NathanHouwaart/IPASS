#include "code/headers/train_ov.h"
#include <math.h>

using namespace std; 

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

    // ---- Declarations ----- 
    nfc::cardKeys               card1Keys;          // If the defaul 0xFF keys are not valid, make a struct containing the right keys
    nfc::mifareCommands AorB    = nfc::authenticateKeyA;
    float   pricePerKm          = 0.335;
    uint8_t sectorLocation      = 0x07;
    uint8_t valueBlockLocation  = 0x05;
    int     baudrate            = 115200;
    uint8_t oledAdress          = 0x3C;
    int     maxCardBalance      = 1000;
    int     topUpValue          = 50;

    auto stationPin1 = target::pin_in( target::pins::d44 );
    auto stationPin2 = target::pin_in( target::pins::d45 );
    auto stationPin3 = target::pin_in( target::pins::d46 );
    auto stationPin4 = target::pin_in( target::pins::d47 );
    auto stationPin5 = target::pin_in( target::pins::d50 );
    auto stationPin6 = target::pin_in( target::pins::d51 );
    auto stationPin7 = target::pin_in( target::pins::d52 );
    auto stationPin8 = target::pin_in( target::pins::d53 );

    auto modeSelectPin1 = target::pin_in( target::pins::d38 );
    auto modeSelectPin2 = target::pin_in( target::pins::d39 );
    auto modeSelectPin3 = target::pin_in( target::pins::d40 );
    auto modeSelectPin4 = target::pin_in( target::pins::d41 );


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
    auto uartbus = HardwareUart(baudrate);
    
    /// Construct Busses
    auto uartInterface = communication::uart(uartbus);
    auto spiInterface = communication::spi(spiBus, ss, irq);
    auto i2cInterface = communication::i2c(nfc::pn532::general::Pn532Address, i2cbus, irq);

    //
    hwlib::wait_ms(2);

    auto oled    = hwlib::glcd_oled( i2cbus, oledAdress ); 
   
    auto font    = hwlib::font_default_8x8();
    auto display = hwlib::terminal_from( oled, font );   
    
    // auto card = nfc::PN532_chip(spiInterface, irq);
    // auto card = nfc::PN532_chip(uartInterface, irq);
    auto card = nfc::PN532_chip(i2cInterface, irq);

    auto terminal = nfc::NfcOled(card, display, spiInterface);  /// < ------- nfc::NfcOled instead of nfc::PN532_chip

    auto trainReader = train(
        terminal, display, 
        stationPin1, stationPin2, stationPin3, stationPin4, stationPin5, stationPin6, stationPin7, stationPin8, 
        modeSelectPin1, modeSelectPin2, modeSelectPin3, modeSelectPin4,
        pricePerKm, maxCardBalance, topUpValue, AorB, valueBlockLocation, sectorLocation
    );  


    while (1)
    {
        auto mode = trainReader.getMode();
        trainReader.setMode(mode);
    }
    
    
}