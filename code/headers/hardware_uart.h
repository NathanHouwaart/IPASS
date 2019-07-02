/**
 * @file
 * @brief     This file implements the functions declared in hardware_uart.hpp
 * 
 * @author    Nathan Houwaart
 * @license   See LICENSE
 */

#ifndef V1_OOPC_18_NATHANHOUWAART_HARDWARE_UART_H
#define V1_OOPC_18_NATHANHOUWAART_HARDWARE_UART_H

#include "hwlib.hpp"
#include "../../hwlib/library/hwlib.hpp"

template<size_t n>
class buffer{
private:
    unsigned int begin;
    unsigned int end;
    unsigned int count;

    uint8_t fifo[n];

public:
    buffer(){
        begin= 0;
        end = 0;
        count = 0;
    }

    int returnBegin()
    {
        return begin;
    }

    int returnEnd()
    {
        return end;
    }

    int returnSize(){
        return count;
    }

    void push_back(const uint8_t c){
        if(end <= n){
            // stay within buffer boundaries
            fifo[end] = c;
            end++;
            count++;
        }
        if(end > n){
            end = 0;
        }
    }

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

    void clear(){
        begin = end;
        count = 0;
    }
};


class uart_abstract {

public:
    virtual void uart_init() = 0;
    virtual int avialable() = 0;
    virtual void sendByte(const uint8_t c) = 0;
    //virtual void receiveBuffer(uint8_t* receiveBuffer, int n_bytes) = 0;
    virtual uint8_t receiveByte() = 0;
    virtual bool uart_has_overrun () = 0;
    virtual bool uart_has_rx_error () = 0;
    virtual bool rxReady() = 0;
    virtual uint8_t getC() = 0;


};


class HardwareUart : public uart_abstract{
private:

    Usart* hw_uart0 = USART0;
    unsigned int baudrate;
    buffer<255> rxBuffer;
    int timeout = 1000;
   
public:

    HardwareUart(int baudrate):baudrate(baudrate){
        uart_init();
    }

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

    int avialable() override {
        hwlib::cout << ";;;";
    
        for(int i = 0; i < 50;i++){
            while(!rxReady()){uart_has_overrun();}
                rxBuffer.push_back(receiveByte());
        }
        return rxBuffer.returnSize();
    }

    //////35.7.3.11 Receiver Time-out 

    bool uart_has_overrun () override
    {
        if (hw_uart0 == NULL || !(hw_uart0->US_CSR & US_IER_RXBUFF)){
            return false;
        }
         hwlib::cout << "overrun" << hwlib::endl;
        // clear flag
        (void)hw_uart0->US_CSR;
        return true;
    }

    bool uart_has_rx_error () override
    {
        if (hw_uart0 == NULL || !(hw_uart0->US_CSR & US_IER_OVRE))
            return false;

        // clear flag
        // hw_uart0->US_CSR & US_IER_OVRE = false;
        hwlib::cout << "error" << hwlib::endl;
        return true;
    }

    uint8_t receiveByte() override {
        return hw_uart0->US_RHR; 
    }


    bool rxReady() override {
        //Checks the rxReady flag. If this returns 0. A complete byte is in the rxByffer
        return ((hw_uart0->US_CSR & 1) != 0);
    }

    uint8_t getC() override {
        if(rxBuffer.returnSize() > 0){
            return rxBuffer.pop();
        }
        return 1;
    }

    void sendByte(const uint8_t c) override {
        while( ( hw_uart0->US_CSR & 2 ) == 0 ){}
        hw_uart0->US_THR = c;
    }
};

#endif