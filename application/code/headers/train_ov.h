/**
 * @file
 * @brief     Train implementation of the abstract ov class
 * 
 * @author    Nathan Houwaart
 * @license   See LICENSE
 */

#ifndef V1_OOPC_18_NATHANHOUWAART_TRAIN_OV_H
#define V1_OOPC_18_NATHANHOUWAART_TRAIN_OV_H

#include "ov.h"

/// \brief
/// Train implementation of a ov public transportation system
/// \details
/// \You can check in, out, tup up the card balance and travel to different stations.
class train : public ovTracker{
private:

    nfc::cardKeys keys;
    
public:
    /// \brief
    /// Constructor of a train class application
    /// \details
    /// This function will need all the data fields of the ovTracker supporter
    /// Upon initialising the ovTracker class, the train class will initialise itself by calling init()
    /// @param nfc                  pn532 chip class
    /// @param display              Display that can be written to
    /// @param pins                 Station/ mode slection pins
    /// @param price                Price per kilometer
    /// @param maxCardBalance       The max balance a card can have
    /// @param topUpValue           The value the card will be topped up with
    /// @param AorB                 Wether the user wants to autenticate the sector with keyA or keyB
    /// @param valueBlockLocation   Number of the page where the value block is located
    /// @param sectorLocation       Number of the sector trailer page that needs to be authenticated
    train(
        nfc::NFC& nfc, 
        hwlib::terminal_from& display, 
        hwlib::pin_in& stationPin1, hwlib::pin_in& stationPin2, hwlib::pin_in& stationPin3, hwlib::pin_in& stationPin4, 
        hwlib::pin_in& stationPin5, hwlib::pin_in& stationPin6, hwlib::pin_in& stationPin7, hwlib::pin_in& stationPin8, 
        hwlib::pin_in& modeSelectPin1, hwlib::pin_in& modeSelectPin2, hwlib::pin_in& modeSelectPin3, hwlib::pin_in& modeSelectPin4,
        float pricePerKilometer, 
        int maxCardBalance, 
        int topUpValue,
        nfc::mifareCommands AorB,
        uint8_t valueBlockLocation,
        uint8_t sectorLocation);
        
    /// \brief
    /// This function will initialise the train class
    /// \details
    /// The pn532 nfc chip will be setup to read the proper cardtypes
    void init() override;
        
    /// \brief
    /// This function will wait for a card to be present in the nfc's rf field
    /// \details
    /// Once a card has been detected, it will call the aproprate function (check in/ check out)
    void waitCard() override;

    /// \brief
    /// This function is used to checkin a card that has not yet been checked in
    /// \details
    /// The function will check for a apropriate card
    /// The function will check if there is space in the buffer
    /// 
    /// Upon checking in, this function will put the read card in the buffer among with the checkin station
    /// Upon checking in, this function will also show the balance of card
    /// @param cardinfo     card class with stored UID
    void checkIn(card& cardinfo) override ;

    /// \brief
    /// This function is used to checkout a card that has already checked in
    /// \details
    /// This function will handle the complete calculations needed in order to decrement the correct value from the card
    /// It will decrement x amount based on the km´s traveled
    /// Upon checking out, this function will display the remaining balance on the card
    /// Upon checking out, this function will clear the read card from the buffer.
    /// @param index    index of where the card info is stored in the array
    void checkOut(const int index) override;

    /// \brief
    /// This function will read out the 8 GPIO pins to determine the current station
    /// \details
    /// By reading the 8 gpio pins and storing them in a 8 bit value,
    /// @return uint8_t value of the pins read
    Mode getMode() override;

    /// \brief
    /// This function will set the mode of apllication
    /// \details
    /// @param  uint8_t     mode
    void setMode(const Mode newMode) override;

    /// \brief
    /// This function is used to get and set a new station
    /// \details
    /// this function changes the internal current station.id member
    bool getAndSetStation() override;

    /// \brief
    /// This function is used to calculate the price of the ride based on distance traveled
    /// \details
    /// @param  index   cardindex of the card that the price needs to be calculated for
    /// @return float   price of the ride
    float calculate_price(const int index) override;

    /// \brief
    /// This fucntion will check wether a card is formatted properly for the application
    /// \details
    /// It will achieve this by decrementing 0 and then transfering it to the page. If the page
    /// Is not properly formatted, the transfer command will throw an error
    /// @param  cardinfo    card class with stored card information
    /// @return true        card properly formatted
    /// @return false       card not properly formatted
    bool validateCard(card & cardinfo) override;

    /// \brief
    /// This function will get the current balance of a given card
    /// \details
    /// It will do this by converting the 4 byte int value to a 4 byte float value;
    /// @param  cardinfo    card class with stored card information
    /// @return balance     the balance of the card
    uint32_t getBalance(card & cardinfo) override;

    /// \brief
    /// This function will top up the balance of a given card
    /// \details
    /// It will top up the card by the given incrementvalue
    void topUp(int incrementValue) override;

};

#endif