/**
 * @file
 * @brief     Abstract protocol class and it's specific protocol implementation
 * 
 * @author    Nathan Houwaart
 * @license   See LICENSE
 */

#ifndef V1_OOPC_18_NATHANHOUWAART_INTERFACE_H
#define V1_OOPC_18_NATHANHOUWAART_INTERFACE_H

#include "hwlib.hpp"
#include "pn532Command.h"
#include "hardware_uart.h"

namespace communication{

/// \brief
/// Abstract protocol class
class protocol
{
    
public:
    /// \brief
    /// Abstract function to wakeup / initialise a protocol
    virtual void wakeUp() = 0;

    /// \brief
    /// Abstract function to send data over the given protocol
    /// \details
    /// @param      commandBuffer   Pointer to the start of the commandbuffer that needs to be send
    /// @param      nBytes          Amount of bytes that need to send 
    virtual void sendData(uint8_t *commandBuffer, uint8_t nBytes) = 0;

    /// \brief
    /// Abstract function to receive data over the given protocol
    /// \details
    /// @param      receiveBuffer   Pointer to the start of the receiveBuffer where data can be stored in
    /// @param      nBytes          Amount of bytes that need to be received over the given protocol
    virtual void receiveData(uint8_t *receiveBuffer, uint8_t nBytes) = 0;

    /// \brief
    /// Function to end a specific protocol transaction
    /// \details
    /// This function is implemented because some chips do not work with the provided hwlib bit banged default implementations
    /// The deconstructor there will cause some chips to not receive data properly
    /// Therefore this function is not mandatory to implement
    virtual void endTransaction(){};
};



/// i2c implementation of the abstract protocol class
class i2c : public protocol
{
protected:
   
    const uint_fast8_t adress;

    hwlib::i2c_bus &bus;
    hwlib::pin_in &irq;

public:

    /// \brief
    /// Constructor of the i2c class
    /// \details
    /// @param  adress  The i2c adress of the chip that needs to be communicated with
    /// @param  bus     A reference to a hwlib::i2c_bus
    /// @ param irq     Address of the irq pin needed by the pn532
    i2c(
        const uint_fast8_t adress,
        hwlib::i2c_bus &bus,
        hwlib::pin_in &irq
    );

    /// \brief
    /// This Funtion will wake up / initialise the i2c bus / chip for communication
    /// \details
    /// Function will send the i2c adress over the interface to wake the chip if it is in sleep mode
    void wakeUp() override;

    /// \brief
    /// This function sends nBytes from the initialised adress over the i2c protocol
    /// \details
    /// @param      commandBuffer   Pointer to the start of the commandbuffer that needs to be send
    /// @param      nBytes          Amount of bytes that need to be send over i2c
    void sendData(uint8_t *commandBuffer, uint8_t nBytes);

    /// \brief
    /// This function receives data from the initialised adress over the i2c protocol
    /// \details
    /// @param      receiveBuffer   Pointer to the start of the receiveBuffer where data can be stored in
    /// @param      nBytes          Amount of bytes that need to be received over i2c
    void receiveData(uint8_t *receiveBuffer, uint8_t nBytes);
};


/// spi implementation of the abstract protocol class
class spi : public protocol
{
protected:
    hwlib::spi_bus &bus;
    hwlib::pin_in &irq;
    hwlib::pin_out &sel;  

public:
    
    /// \brief
    /// Constructor of the spi class
    /// \details
    /// @param  bus     A reference to a hwlib::spi_bus
    /// @param  sel     SlaveSelect pin the chip is connected to
    /// @ param irq     Address of the irq pin needed by the pn532
    spi(
        hwlib::spi_bus &bus,
        hwlib::pin_out &sel,
        hwlib::pin_in &irq
    );

    /// \brief
    /// This Funtion will wake up / initialise the spi bus / chip for communication
    /// \details
    /// Function will send a quick pulse on the sel pin to wake up the connected chip if it was asleep
    void wakeUp() override;

    /// \brief
    /// This function sends nBytes to the chip connected to the sel pin over the spi protocol
    /// \details
    /// @param      commandBuffer   Pointer to the start of the commandbuffer that needs to be send
    /// @param      nBytes          Amount of bytes that need to send over spi
    void sendData(uint8_t *commandBuffer, uint8_t nBytes) override;

    /// \brief
    /// This function receives nBytes from the chip connected to the sel pin over the spi protocol
    /// \details
    /// @param      receiveBuffer   Pointer to the start of the receiveBuffer where data can be stored in
    /// @param      nBytes          Amount of bytes that need to be received over spi
    void receiveData(uint8_t *receiveBuffer, uint8_t nBytes) override;

    /// \brief
    /// This fucntion will end the transaction of the sel chip
    /// \details
    /// It will acheive this by rising the sel pin again after all data has been send
    void endTransaction() override;
};

/// \brief
/// Hardware UART implementation of the abstract protocol class
/// \details
/// @warning    This hardware implementation is not yet finished!
///             This implementation can send data over the UART protocol,
///             but cannot properly receive data yet.
///             This is a work in progress
class uart : public protocol {
private:
    hwuart::uart_abstract& bus;

public:

    /// \brief
    /// Constructor of the UART class
    /// \details
    /// @param  abstract_uart   A reference to a abstract uart class
    uart(hwuart::uart_abstract& bus);

    /// \brief
    /// This Funtion will wake up / initialise the spi bus / chip for communication
    /// \details
    /// Function will send a rather long preamble with 0x55 as the first byte. This is
    /// in order to wake up the chip if it was asleep
    void wakeUp() override;

    /// \brief
    /// This function sends nBytes to the chip connected to the UART bus
    /// \details
    /// @param      commandBuffer   Pointer to the start of the commandbuffer that needs to be send
    /// @param      nBytes          Amount of bytes that need to send over spi
    void sendData(uint8_t *commandBuffer, uint8_t nBytes);

    /// \brief
    /// This function receives nBytes from the chip connected to the UART bus
    /// \details
    ///
    /// @warning    This function does not work properly yet! 
    ///             Internal UART Receive register may overflow and therefore interrupt
    ///             causing undefined behaviour
    ///
    /// @param      receiveBuffer   Pointer to the start of the receiveBuffer where data can be stored in
    /// @param      nBytes          Amount of bytes that need to be received over spi
    void receiveData(uint8_t *receiveBuffer, uint8_t nBytes);

};

}// namespace communication

#endif //V1_OOPC_18_NATHANHOUWAART_INTERFACE_H