/**
 * @file
 * @brief     This file implements a ringbuffer, abstract UART interface and an implementation of this interface
 * 
 * @note    The hardware UART implementation is only working when sending a command to a slave. Reveiving
 *          data over the UART bus is not yet possible. In order to make this happen, cpu interrupts need
 *          to be added tho interrupt the cpu when RXRDY register is 1. This is however out of my knowledge
 *          and therefore not yet implemented. Also this file is not yet split up in to .cpp files and .h 
 *          files. This will be done as soon as the receive function is working
 * 
 * @author    Nathan Houwaart
 * @license   See LICENSE
 */

#ifndef V1_OOPC_18_NATHANHOUWAART_HARDWARE_UART_H
#define V1_OOPC_18_NATHANHOUWAART_HARDWARE_UART_H

#include "hwlib.hpp"
#include "../../hwlib/library/hwlib.hpp"

namespace hwuart
{


/// \brief
/// This is a ringbuffer class that is supposed to store the received data from the UART bus
/// \details
/// This buffer is templated and can be decided at compile time. It is reccomended to make the buffer no larger than 255 bytes for speed purposes
template<size_t n>
class buffer{
private:
    unsigned int begin;
    unsigned int end;
    unsigned int count;

    uint8_t fifo[n];

public:
    /// \brief
    /// Default construcctor for the buffer. The buffer will initially be empty when created.
    buffer(){
        begin= 0;
        end = 0;
        count = 0;
    }

    /// \brief
    /// Returns the begin index of the buffer
    int returnBegin()
    {
        return begin;
    }

    /// \brief
    /// Returns the end index of the buffer
    int returnEnd()
    {
        return end;
    }

    /// \brief
    /// Returns the size of the buffer
    int returnSize(){
        return count;
    }

    /// \brief
    /// Pushes back a character at the back of the buffer
    /// \details
    /// If the buffer has reached the end index of the buffer,
    /// it will wrap around and start at index 0 again. This
    /// will possibly overwrite the characters already there
    void push_back(const uint8_t c){
        if(end <= n){
            // stay within buffer boundaries
            fifo[end] = c;
            end++;
            count++;
        }
        if(end >= n){
            end = 0;
        }
    }

    /// \brief
    /// Gets the first element of the buffer and removes it.
    /// \details
    /// If the buffer has reached the end index of the buffer,
    /// it will wrap around and start at index 0 again and start
    /// popping characters form there
    uint8_t pop(){
        if ( count <= 0) return 0;
        else{
            uint8_t retVal = fifo[begin];
            begin++;
            count--;
            if(begin > n){
                begin = 0;
            }
            return retVal;
        }
    }

    /// reset the buffer
    void clear(){
        begin = end;
        count = 0;
    }
};

/// \brief
/// This is a pure abstract class of the hardware UART
class uart_abstract {

public:
    /// \brief
    /// This function is used to initialise the UART bus
    /// \details
    /// In this function, the proper registers should be set in order
    /// to activate the harware uart bus on the host controller
    virtual void uart_init() = 0;

    /// \brief    
    /// Chekcs whether there is a character available in the ringbuffer
    virtual int avialable() = 0;

    /// \brief
    /// Sends a byte over the UART bus
    virtual void sendByte(const uint8_t c) = 0;

    /// \brief
    /// Stores a byte in the ringbuffer
    virtual void storeByte(const char c) = 0;

    /// \brief
    /// Receives a byte form the UART bus
    virtual uint8_t receiveByte() = 0;

    /// \brief
    /// A function to check whether the rx buffer has overrun
    virtual bool uart_has_overrun () = 0;

    /// \brief
    /// A function to check whether the receive register is ready to be read from
    virtual bool rxReady() = 0;

    /// \brief
    /// Get a character from the ringbuffer    
    virtual uint8_t getC() = 0;
};

/// \brief
/// Implementation of the abstract UART class
/// \details
/// This will only implement TX3 & RX3
class HardwareUart : public uart_abstract{
private:

    Usart* hw_uart0 = USART0;
    unsigned int baudrate;
    buffer<255> rxBuffer;
    int timeout = 1000;
   
public:

    /// \brief
    /// Constructor of the HardwareUart class
    /// \details
    /// Once created it will initialise itself
    /// @param baudrate The baudrate the bus needs to be set to    
    HardwareUart(int baudrate):baudrate(baudrate){
        uart_init();
    }

    /// \brief
    /// Initialise the hardware uart on the host controller
    void uart_init() override{
        // Enable the clock to port A
        PMC->PMC_PCER0 = 1 << ID_PIOA;

        // Disable the PIO control and set up for peripheral A
        // http://ww1.microchip.com/downloads/en/devicedoc/atmel-11057-32-bit-cortex-m3-microcontroller-sam3x-sam3a_datasheet.pdf
        // P.773 section 35.6.1
        PIOA->PIO_PDR   = PIO_PA11; 
        PIOA->PIO_ABSR &= ~PIO_PA11; 
        PIOA->PIO_PDR   = PIO_PA10; 
        PIOA->PIO_ABSR &= ~PIO_PA10;

        // enable the clock to the UART
        PMC->PMC_PCER0 = ( 0x01 << ID_USART0 );

        // Reset and disable receiver and transmitter.
        hw_uart0->US_CR = UART_CR_RSTRX | UART_CR_RSTRX | UART_CR_RSTRX | UART_CR_RSTRX;

        //Set initial baudrate to int baudrate
        hw_uart0->US_BRGR = 45;

        // No parity, normal channel mode.
        hw_uart0->US_MR = UART_MR_PAR_NO | UART_MR_CHMODE_NORMAL | US_MR_CHRL_8_BIT;

        // Disable all interrupts.	  
        // hw_uart0->US_IDR = 0xFFFFFFFF; 

        // Enable the receiver and the trasmitter.
        hw_uart0->US_CR = UART_CR_RXEN | UART_CR_TXEN;
    }

    /// \brief
    /// Checks how many bytes are stored in the buffer
    int avialable() override {
        return rxBuffer.returnSize();
    }

    /// \brief
    /// Stores a byte in the ringbuffer
    void storeByte(const char c) override {
        rxBuffer.push_back(c);
    }

    /// \brief
    /// Checks whether the rx buffer has overrun
    /// \details
    /// If overrun, it wil try to clear the register
    bool uart_has_overrun () override
    {
        // return false if buffer has not overrun
        if (!(hw_uart0->US_CSR & US_IER_RXBUFF)){
            return false;
        }
        
         hwlib::cout << "overrun" << hwlib::endl;
        // clear flag
        hw_uart0->US_CSR = false;
        return true;
    }

    /// \brief
    /// Receives a byte form the UART bus
    uint8_t receiveByte() override {
        return hw_uart0->US_RHR; 
    }

    /// \brief
    /// A function to check whether the receive register is ready to be read from
    bool rxReady() override {
        //Checks the rxReady flag. If this returns 0. A complete byte is in the rxByffer
        return ((hw_uart0->US_CSR & 1) != 0);
    }

    /// \brief
    /// Get a character from the rungbuffer if available
    uint8_t getC() override {
        if(rxBuffer.returnSize() > 0){
            return rxBuffer.pop();
        }
        return 0;
    }

    /// \brief
    /// Send a byte over the UART bus
    void sendByte(const uint8_t c) override {
        while( ( hw_uart0->US_CSR & 2 ) == 0 ){}
        hw_uart0->US_THR = c;
    }
};

} // namespace hwuart

#endif