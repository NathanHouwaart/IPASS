/**
 * @file
 * @brief     This file implements the functions declared in train_ov.h
 * 
 * @author    Nathan Houwaart
 * @license   See LICENSE
 */

#include "../headers/train_ov.h"

train::train(
    nfc::NFC& nfc, 
    hwlib::terminal_from& display, 
    hwlib::pin_in& stationPin1, hwlib::pin_in& stationPin2, hwlib::pin_in& stationPin3, hwlib::pin_in& stationPin4, 
    hwlib::pin_in& stationPin5, hwlib::pin_in& stationPin6, hwlib::pin_in& stationPin7, hwlib::pin_in& stationPin8, 
    hwlib::pin_in& modeSelectPin1, hwlib::pin_in& modeSelectPin2, hwlib::pin_in& modeSelectPin3, hwlib::pin_in& modeSelectPin4,
    uint8_t cardNumber,
    float pricePerKilometer,
    uint32_t maxCardBalance,
    int minimumCardBalance,
    uint32_t topUpValue,
    nfc::mifareCommands AorB,
    uint8_t valueBlockLocation,
    uint8_t sectorLocation
):
    ovTracker(
        nfc, display, 
        stationPin1, stationPin2, stationPin3, stationPin4, 
        stationPin5, stationPin6, stationPin7, stationPin8,
        modeSelectPin1, modeSelectPin2, modeSelectPin3, modeSelectPin4,
        cardNumber, pricePerKilometer, maxCardBalance, minimumCardBalance, topUpValue,
        AorB, valueBlockLocation, sectorLocation)
{ 
    init();
}

void train::init()
{
    nfc.getFirmwareVersion();
    nfc.SAMConfiguration(nfc::pn532::command::SAMmode::Normal_mode);
    nfc.setMaxRetries(0xFF); 
    getMode();
}

void train::waitCard(){
    auto cardinfo = card();
    if(!nfc.detectCard(cardinfo, cardNumber, nfc::pn532::command::CardType::TypeA_ISO_IEC14443)){hwlib::cout << "."; return;}
    auto carduid = cardinfo.getUID();

    for(int i = 0; i < 10; i++){
        if(checkinInformation.checkins[i].getUID() == carduid){
            checkOut(i);
            return;
        }
    }
    checkIn(cardinfo);
}

void train::checkIn( card& cardinfo ){
    //checks for if a valid card is presented
    if(!validateCard(cardinfo)){ display << "\v\n\n\n" << "Please use a" << "\n" << "valid card"<<hwlib::flush; hwlib::wait_ms(3000); return;}
    // Checks if the buffer is full
    if(checkinInformation.spaces_occupied >= checkinInformation.getSize()){ display << "\v\n\n\n" << "Full buffer" << hwlib::flush; hwlib::wait_ms(2000);return;}
    // checks wether card has enough saldo
    auto saldo = getBalance(cardinfo);
    if(static_cast<int>(saldo) < minimumCardBalance){ display << "\v\n\n\n" << "Balance too low" << "\n" << "Balance: " << hwlib::dec << static_cast<int>(saldo) << hwlib::flush; hwlib::wait_ms(4000);return;}
    display << "\v\n\n\n\n\n\n" << "Checked in" << "\n" << "Balance: " << hwlib::dec <<  static_cast<int>(saldo) <<  hwlib::flush;
    
    // writes the approved card in a avaialble spot in the buffer
    for(int i = 0; i < checkinInformation.getSize(); i++){
        if(checkinInformation.available[i]){
            checkinInformation.checkinStation[i] = currentStation;
            checkinInformation.checkins[i] = cardinfo;
            checkinInformation.available[i] = false;
            checkinInformation.spaces_occupied++;
            break;
        }
    }
    
    hwlib::wait_ms(4000);
}

void train::checkOut(const int index){
    auto price =  static_cast<uint32_t>(calculate_price(index));
    nfc.mifareDecrement(checkinInformation.checkins[index], cardNumber, authenticateAorB, valueBlockLocation, sectorLocation, keys.aKeys[2], price);
    if(nfc.mifareTransfer(checkinInformation.checkins[index], cardNumber, authenticateAorB, valueBlockLocation, sectorLocation, keys.aKeys[2])!= nfc::statusCode::pn532StatusOK){hwlib::cout << "error checking out"; return;};

    // check wether a card has moved stations
    if(checkinInformation.checkinStation[index].id == currentStation.id){ display << "\v\n\n\n" << "Cancelled";}
    else{display << "\v\n\n\n" << checkinInformation.checkinStation[index].naam << " - " << "\n" << currentStation.naam;}

    /// will check the saldo of the card
    display << "\v\n\n\n\n\n" << hwlib::dec << "price: " << static_cast<int>(price) << "\n" << "Checked out" << "\n" <<  "Balance:" <<  static_cast<int>(getBalance(checkinInformation.checkins[index])) << hwlib::flush;
   
    // remove card and data from the specific index it was stored 
    checkinInformation.checkinStation[index] = Station();
    checkinInformation.checkins[index]       = card();
    checkinInformation.available[index]      = true;
    checkinInformation.spaces_occupied--;

    hwlib::wait_ms(5000);
}


Mode train::getMode(){
    uint8_t reading = 0;
    reading |= (!modeSelectPin1.read() << 3);
    reading |= (!modeSelectPin2.read() << 2);
    reading |= (!modeSelectPin3.read() << 1);
    reading |= (!modeSelectPin4.read() << 0);

    // This switch can be expanded for more future modes
    switch (reading)
    {
    case 0:
        return Mode::travelMode;
        break;
    case 1:
        return Mode::topUpMode;
        break;
    case 2:
        return Mode::makeCardMode;
    default:
        display << "\v\n\n\n" << "invalid mode" << hwlib::flush;
        return Mode::invalidMode;
        break;
    }

}

void train::setMode(const Mode newMode){
    switch (newMode)
    {
    case Mode::travelMode:
        if(getAndSetStation()){
                waitCard();
        };
        break;
    case Mode::topUpMode:
        topUp(topUpValue);
        break;
    case Mode::makeCardMode:
        break;
    default:
        getMode();
    }
}

bool train::getAndSetStation(){
    uint8_t reading = 0;
    reading |= (!stationPin1.read() << 7);
    reading |= (!stationPin2.read() << 6);
    reading |= (!stationPin3.read() << 5);
    reading |= (!stationPin4.read() << 4);
    reading |= (!stationPin5.read() << 3);
    reading |= (!stationPin6.read() << 2);
    reading |= (!stationPin7.read() << 1);
    reading |= (!stationPin8.read() << 0);

    if(reading == currentStation.id){return true;}

    for(uint8_t i = 0; i < stations.size(); i++){
        if(reading == stations[i].id){
            currentStation = stations[i];  
            display << "\v\n" << currentStation.naam << hwlib::flush; 
            return true;
        }
    }
    display << "\v\n" << "Invalid sation" << hwlib::flush;
    return false;
}

float train::calculate_price(const int index) {
    float distanceTravelled = distance(checkinInformation.checkinStation[index].latitude, checkinInformation.checkinStation[index].longitude, currentStation.latitude, currentStation.longitude);
    return pricePerKilometer*distanceTravelled;
}

bool train::validateCard(card & cardinfo){
    // Checks wether the card is properly formatted
    if(nfc.mifareDecrement(cardinfo, cardNumber, authenticateAorB, valueBlockLocation, sectorLocation, keys.aKeys[2], 0)!= nfc::statusCode::pn532StatusOK){hwlib::cout<< "card not formatted properly" << hwlib::endl;return false;}
    if(nfc.mifareTransfer(cardinfo, cardNumber, authenticateAorB, valueBlockLocation, sectorLocation, keys.aKeys[2])!= nfc::statusCode::pn532StatusOK){hwlib::cout<< "card not formatted properly" << hwlib::endl;return false;}
    return true;
}


uint32_t train::getBalance(card & cardinfo){
    // Gets remaining balance
    nfc.mifareAuthenticate(cardinfo, cardNumber, authenticateAorB, sectorLocation, keys.aKeys[2]);
    nfc.mifareReadPage(cardinfo, cardNumber, valueBlockLocation);

    auto valueBlock = cardinfo.getPage(valueBlockLocation);

    // rearrange the bytes. Bytes are stored in least signigicant byte first
    uint32_t saldo = (valueBlock[0]) | (valueBlock[1] << 8) | (valueBlock[2] << 16) | (valueBlock[3] << 24);
    if(saldo > maxCardBalance){ saldo = saldo - 0xFFFFFFFF;};  // if the uint32_t has flipped around because of a negative saldo, it will be restored here

    return saldo;
}

void train::topUp(uint32_t increment_value){
    display << "\v\n" << "Top up" << hwlib::flush;  // Write on the display
    currentStation = Station();                     // reset the current station

    auto cardinfo = card();
    if(!nfc.detectCard(cardinfo, cardNumber, nfc::pn532::command::CardType::TypeA_ISO_IEC14443)) return;  // wait for a card to enter the pn532's rf-field

    auto balance = getBalance(cardinfo);        // get the current balance

    display << "\v\n\n\n" << "Old balance:" << hwlib::dec << static_cast<int>(balance);

    auto TopUpRemainder = maxCardBalance - static_cast<int>(balance);                     // Maximum amount of balance that can be stored on the card
    if(increment_value > TopUpRemainder){ increment_value = TopUpRemainder;}    // If the increment value will exceed the max balance, 
                                                                                // only the difference from current balance to max balance is stored


    nfc.mifareIncrement(cardinfo, cardNumber, authenticateAorB, valueBlockLocation, sectorLocation, keys.aKeys[2], static_cast<uint32_t>(increment_value));
    if(nfc.mifareTransfer(cardinfo, cardNumber, authenticateAorB, valueBlockLocation, sectorLocation, keys.aKeys[2])!= nfc::statusCode::pn532StatusOK){hwlib::cout << "error incrementing"; return;};

    balance = getBalance(cardinfo);
    display << "\n\n" << "new balance:" << hwlib::dec << balance << hwlib::flush;     // Display new balance on oled and flush the screen
    

    hwlib::wait_ms(2000);
}
