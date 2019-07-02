/**
 * @file
 * @brief     Struct declarations and staion declerations of ov class
 *
 * @author    Nathan Houwaart 
 * @license   See LICENSE
 */

#ifndef V1_OOPC_18_NATHANHOUWAART_STATIONS_H
#define V1_OOPC_18_NATHANHOUWAART_STATIONS_H

#include "../../../code/headers/pn532Oled.h"

/// \brief
/// Station struct. Data for one perticulair staion can be stored in here
/// \details
/// A station has a name, identifier ( id ), longitude an latitude
/// The long- and latitude of a perticulair place can be fount on Google Maps
struct Station{
    char naam[30] = "template";
    uint8_t id = -1;
    long double longitude = 0;
    long double latitude = 0;
};

/// enum class with differrent modes of the application
enum class Mode : const uint8_t{
    travelMode = 0x01,
    topUpMode  = 0x02,
    makeCardMode = 0x03,
    invalidMode= 0xFF
};

/// \brief
/// Cardbuffer struct.
/// \details
/// This struct will be for managing the checked in cards and managing the free spaces in the buffer
/// A cardbuffer has : 
///     spaces_occupied = amount of spaces occupied by a checked in card
///     checkinStation  = place where the perticulair card has checked in
///     checkins        = UID's of the checked in chards
///     available       = array which indicates what index is free for a new card
template<int n>
struct cardBuffer{
    int spaces_occupied;
    std::array<Station, n> checkinStation;
    std::array<card, n> checkins;
    std::array<bool, n> available;

    int getSize(){
        return n;
    }
};


/// Station declerations
const constexpr Station amersfoort = { "Amersfoort", 0  , 5.3878266 , 52.1561113 };
const constexpr Station utrecht    = { "Utrecht"   , 1  , 5.1214201 , 52.0907374 };
const constexpr Station amsterdam  = { "Amsterdam" , 2  , 4.895168  , 52.370216  };
const constexpr Station schiphol   = { "Schiphol"  , 4  , 4.7682744 , 52.3105386 };
const constexpr Station haarlem    = { "Haarlem"   , 8  , 4.646219  , 52.387386  };
const constexpr Station denHaag    = { "DenHaag"   , 16 , 4.3006999 , 52.0704978 };
const constexpr Station rotterdam  = { "Rotterdam" , 32 , 4.4777325 , 51.9244201 };
const constexpr Station gouda      = { "Gouda"     , 64 , 4.7104633 , 52.0115205 };
const constexpr Station eindhoven  = { "Eindhoven" , 128, 5.4697225 , 51.441642  };

// for easy acces, the staions are stored in an array
static constexpr std::array<Station, 9> stations = {
    amersfoort, utrecht, amsterdam, schiphol, haarlem, 
    denHaag, rotterdam, gouda, eindhoven};



#endif