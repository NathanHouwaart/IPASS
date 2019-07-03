/**
 * @file
 * @brief     abstract OV tracker class
 *
 * Abstract OV class for storing cardata and manipulating valueblocks
 * 
 * Code for the Haversine formula is based on https://www.geeksforgeeks.org/program-distance-two-points-earth/ 
 * the Author of this code is Aayush Chaturvedi 
 * 
 * @author    Nathan Houwaart
 * @license   See LICENSE
 */

#ifndef V1_OOPC_18_NATHANHOUWAART_OV_H
#define V1_OOPC_18_NATHANHOUWAART_OV_H

#include "stations.h"
#include <math.h>

/// \brief
/// Abstract OV class with build in calculation functions
class ovTracker{
protected:

    // Variables and declarations //
    nfc::NFC&             nfc;
    hwlib::terminal_from&   display;

    hwlib::pin_in& stationPin1;
    hwlib::pin_in& stationPin2;
    hwlib::pin_in& stationPin3;
    hwlib::pin_in& stationPin4;
    hwlib::pin_in& stationPin5;
    hwlib::pin_in& stationPin6;
    hwlib::pin_in& stationPin7;
    hwlib::pin_in& stationPin8;

    hwlib::pin_in& modeSelectPin1;
    hwlib::pin_in& modeSelectPin2;
    hwlib::pin_in& modeSelectPin3;
    hwlib::pin_in& modeSelectPin4;

    uint8_t         cardNumber;
    float           pricePerKilometer;
    Station         currentStation; 
    uint32_t        maxCardBalance;
    int        minimumCardBalance;
    uint32_t        topUpValue;

    cardBuffer<5> checkinInformation;

    nfc::mifareCommands authenticateAorB;
    uint8_t             valueBlockLocation;
    uint8_t             sectorLocation;

    // Fucntions

    /// \brief
    /// This function converts a longitude or latitude to Radians
    /// \details
    /// This code is based on: https://www.geeksforgeeks.org/program-distance-two-points-earth/ 
    /// @param  degree          Longitude or latitude that needs to be converted
    /// @return long double     Converted degree
    long double toRadians( long double degree);

    /// \brief
    /// This function calculates the distance between two stations using the Haversine Formula
    /// This code is based on: https://www.geeksforgeeks.org/program-distance-two-points-earth/ 
    /// @author Aayush Chaturvedi 
    /// @param lat1             Latitude of station1
    /// @param long1            Longitude of station1
    /// @param lat2             Latitude of station2
    /// @param long2            Longitude of station2
    /// @return long double     Distance between the two stations
    long double distance(long double lat1, long double long1, long double lat2, long double long2);

public:

    /// \brief
    /// Constructor for the ovTracker class
    /// \details
    /// @param nfc                  pn532 chip class
    /// @param display              Display that can be written to
    /// @param pins                 Station/ mode slection pins
    /// @param price                Price per kilometer
    /// @param maxCardBalance       The max balance a card can have
    /// @param topUpValue           The value the card will be topped up with
    /// @param AorB                 Wether the user wants to autenticate the sector with keyA or keyB
    /// @param valueBlockLocation   Number of the page where the value block is located
    /// @param sectorLocation       Number of the sector trailer page that needs to be authenticated
    ovTracker(
        nfc::NFC & nfc, 
        hwlib::terminal_from& display,  
        hwlib::pin_in& stationPin1, hwlib::pin_in& stationPin2, hwlib::pin_in& stationPin3, hwlib::pin_in& stationPin4, 
        hwlib::pin_in& stationPin5, hwlib::pin_in& stationPin6, hwlib::pin_in& stationPin7, hwlib::pin_in& stationPin8, 
        hwlib::pin_in& modeSelectPin1, hwlib::pin_in& modeSelectPin2, hwlib::pin_in& modeSelectPin3, hwlib::pin_in& modeSelectPin4,
	uint8_t cardNumber,
        float pricePerKilometer,
        uint32_t maxCardbalance, 
        int minimumCardBalance,
        uint32_t topUpValue,
        nfc::mifareCommands AorB,
        uint8_t valueBlockLocation,
        uint8_t sectorLocation
    );
        

    /// \brief
    /// This function is used to set up all the required settings for the nfc reader
    virtual void init() = 0;

    /// \brief
    /// This function will wait for a card to be present in the nfc's rf field
    /// \details
    /// Once a card has been detected, it will call the aproprate function (check in/ check out)
    virtual void waitCard() = 0;

    /// \brief
    /// This function is used to checkin a card that has not yet been checked in
    /// \details
    /// Upon checking in, this function will put the read card in the buffer among with the checkin station
    /// @param cardinfo     Card class with stored UID
    virtual void checkIn(card& cardinfo) = 0;

    /// \brief
    /// This function is used to checkout a card that has already checked in
    /// \details
    /// Upon checking out, this function will clear the read card from the buffer.
    /// @param index    Index of where the card info is stored in the array
    virtual void checkOut(const int index) = 0;

    /// \brief
    /// This function will read out the 8 GPIO pins to determine the current station
    /// \details
    /// By reading the 8 gpio pins and storing them in a 8 bit value,
    /// @return Mode    the mode the pins are set in
    virtual Mode getMode() = 0;

    /// \brief
    /// This function will set the mode of apllication
    /// \details
    /// @param  newMode     newMode
    virtual void setMode(const Mode newMode) = 0;

    /// \brief
    /// This function is used to get and set a new station
    /// \details
    /// This function changes the internal current station.id member
    virtual bool getAndSetStation() = 0;

    /// \brief
    /// This function is used to calculate the price of the ride based on distance traveled
    /// \details
    /// @param  index   Cardindex of the card that the price needs to be calculated for
    /// @return float   Price of the ride
    virtual float calculate_price(const int index) = 0;

    /// \brief
    /// This fucntion will check wether a card is formatted properly for the application
    /// \details
    /// @param  cardinfo    card class with stored card information
    /// @return true        card properly formatted
    /// @return false       card not properly formatted
    virtual bool validateCard(card & cardinfo) = 0;
    
    /// \brief
    /// This function will get the current balance of a given card
    /// \details
    /// It will do this by converting the 4 byte int value to a 4 byte float value;
    /// @param  cardinfo    card class with stored card information
    virtual uint32_t getBalance(card & cardinfo) = 0;

    /// \brief
    /// This function is used to top up a cards' balance
    /// \details
    /// @param incrementValue   value that the balance of valueblock of a mifare classic card needs to be incremented by
    virtual void topUp(uint32_t incrementValue) = 0;

};

#endif
