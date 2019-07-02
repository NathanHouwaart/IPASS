/**
 * @file
 * @brief     This file implements the functions declared in interface.h
 * 
 * @author    Nathan Houwaart
 * @license   See LICENSE
 */

#include "../headers/interface.h"

namespace communication{

// i2c function implementations

i2c::i2c(
    const uint_fast8_t adress,
    hwlib::i2c_bus &bus,
    hwlib::pin_in &irq) : adress(adress),
                            bus(bus),
                            irq(irq)
{}

void i2c::wakeUp()
{   
   bus.write(adress).write(adress);
}

void i2c::sendData(uint8_t *commandBuffer, uint8_t nBytes)
{
    bus.write(adress).write(commandBuffer, nBytes);
}

void i2c::receiveData(uint8_t *receiveBuffer, uint8_t nBytes)
{
    bus.read(adress).read(receiveBuffer, nBytes);
}


// spi function implementations

spi::spi(
    hwlib::spi_bus &bus,
    hwlib::pin_out &sel,
    hwlib::pin_in &irq) : bus(bus),
                            irq(irq),
                            sel(sel)
{}

void spi::wakeUp()
{
    sel.write(0);
    sel.flush();
    hwlib::wait_ms(2);
    sel.write(1);
    sel.flush();
}

void spi::sendData(uint8_t *commandBuffer, uint8_t nBytes)
{
    bus.transaction(sel).write(1);
    bus.transaction(sel).write(nBytes, commandBuffer);
}


void spi::receiveData(uint8_t *receiveBuffer, uint8_t nBytes)
{
    bus.transaction(sel).write(3);
    bus.transaction(sel).read(nBytes, receiveBuffer);
}

void spi::endTransaction()
{
    bus.transaction(sel).endTransaction();
}



// UART specific functions

uart::uart(uart_abstract& bus):bus(bus){
    wakeUp();
}

void uart::wakeUp()
{
    uint8_t wake[] = {0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    sendData(wake, 7);
}

void uart::sendData(uint8_t *commandBuffer, uint8_t nBytes)
{
    for(int i = 0; i < nBytes; i++){
        bus.sendByte(commandBuffer[i]);
    }
}

void uart::receiveData(uint8_t *receiveBuffer, uint8_t nBytes)
{
    hwlib::cout << "iets";
    auto x = bus.avialable();
    for(int i = 0; i < x; i++){
        uint8_t x = bus.getC();
        receiveBuffer[i] = x;
    }
}
    
} // namespace communication