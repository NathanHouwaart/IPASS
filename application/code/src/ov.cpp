/**
 * @file
 * @brief     This file implements the functions declared in ov.h
 * 
 * @author    Nathan Houwaart
 * @license   See LICENSE
 */


#include "../headers/ov.h"

ovTracker::ovTracker(
    nfc::NFC & nfc, 
    hwlib::terminal_from& display,  
    hwlib::pin_in& stationPin1, hwlib::pin_in& stationPin2, hwlib::pin_in& stationPin3, hwlib::pin_in& stationPin4, 
    hwlib::pin_in& stationPin5, hwlib::pin_in& stationPin6, hwlib::pin_in& stationPin7, hwlib::pin_in& stationPin8, 
    hwlib::pin_in& modeSelectPin1, hwlib::pin_in& modeSelectPin2, hwlib::pin_in& modeSelectPin3, hwlib::pin_in& modeSelectPin4,
    float pricePerKilometer,
    uint32_t maxCardBalance,
    uint32_t topUpValue,
    nfc::mifareCommands AorB,
    uint8_t valueBlockLocation,
    uint8_t sectorLocation
): 
    nfc(nfc), 
    display(display), 
    stationPin1(stationPin1), stationPin2(stationPin2), stationPin3(stationPin3), stationPin4(stationPin4), 
    stationPin5(stationPin5), stationPin6(stationPin6), stationPin7(stationPin7), stationPin8(stationPin8), 
    modeSelectPin1(modeSelectPin1), modeSelectPin2(modeSelectPin2), modeSelectPin3(modeSelectPin3), modeSelectPin4(modeSelectPin4),
    pricePerKilometer(pricePerKilometer),
    maxCardBalance(maxCardBalance),
    topUpValue(topUpValue),
    authenticateAorB(AorB),
    valueBlockLocation(valueBlockLocation),
    sectorLocation(sectorLocation) 
{
    // initialise the checkinInformation struct
    currentStation  = {Station()};
    checkinInformation.checkinStation = {Station()};
    checkinInformation.checkins = {card()};

    // Set all spaces available
    for(auto i: checkinInformation.available){
        checkinInformation.available[i] = 1;
    }
    checkinInformation.spaces_occupied = 0;
}

long double ovTracker::toRadians( long double degree){
    long double one_deg = (3.14159265358979323846) / 180;
    return one_deg * degree;
}

long double ovTracker::distance(long double lat1, long double long1, long double lat2, long double long2) 
{ 
    // Convert the latitudes and longitudes from degree to radians. 
    lat1 = toRadians(lat1); 
    long1 = toRadians(long1); 
    lat2 = toRadians(lat2); 
    long2 = toRadians(long2); 

    // Haversine Formula 
    long double dlong = long2 - long1; 
    long double dlat = lat2 - lat1; 

    long double ans = pow(sin(dlat / 2), 2) + cos(lat1) * cos(lat2) * pow(sin(dlong / 2), 2); 

    ans = 2 * asin(sqrt(ans)); 

    // Radius of Earth in Kilometers, R = 6371 or Miles, R = 3956
    long double R = 6371; 
    
    // Calculate the result 
    ans = ans * R; 
    return ans; 
} 
