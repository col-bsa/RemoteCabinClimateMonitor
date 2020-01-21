/*
 Si7021 Temperature Library for Particle Devices
 By: Brandon Satrom
 Particle
 Date: May 10, 2018
*/

#include "Si7021_MultiWire.h"
#include "Particle.h"

TwoWire& wire = Wire;

//Initialize
Si7021_MultiWire::Si7021_MultiWire(){}

Si7021_MultiWire::Si7021_MultiWire(TwoWire& wireInstance){
  wire = wireInstance;
}

bool Si7021_MultiWire::begin()
{
  wire.begin();

  uint8_t ID_Temp_Hum = checkID();

  int x = 0;

  if(ID_Temp_Hum == 0x15)//Ping CheckID register
    x = 1;
  else if(ID_Temp_Hum == 0x32)
    x = 2;
  else
    x = 0;

  if(x == 1)
  {
    Serial.println("Si7021 Found");
    //Serial.println(ID_Temp_Hum, HEX);
  }
  else if(x == 2)
  {
    Serial.println("HTU21D Found");
    //Serial.println(ID_Temp_Hum, HEX);
  }
  else
  	Serial.println("No Devices Detected");
  	//Serial.println(ID_Temp_Hum, HEX);
}

/****************Si7021 & HTU21D Functions**************************************/


float Si7021_MultiWire::getRH()
{
	// Measure the relative humidity
	uint16_t RH_Code = makeMeasurment(HUMD_MEASURE_NOHOLD);
	float result = (125.0*RH_Code/65536)-6;
	return result;
}

float Si7021_MultiWire::readTemp()
{
	// Read temperature from previous RH measurement.
	uint16_t temp_Code = makeMeasurment(TEMP_PREV);
	float result = (175.72*temp_Code/65536)-46.85;
	return result;
}

float Si7021_MultiWire::getTemp()
{
	// Measure temperature
	uint16_t temp_Code = makeMeasurment(TEMP_MEASURE_NOHOLD);
	float result = (175.72*temp_Code/65536)-46.85;
	return result;
}
//Give me temperature in fahrenheit!
float Si7021_MultiWire::readTempF()
{
  return((readTemp() * 1.8) + 32.0); // Convert celsius to fahrenheit
}

float Si7021_MultiWire::getTempF()
{
  return((getTemp() * 1.8) + 32.0); // Convert celsius to fahrenheit
}


void Si7021_MultiWire::heaterOn()
{
	// Turns on the ADDRESS heater
	uint8_t regVal = readReg();
	regVal |= _BV(HTRE);
	//turn on the heater
	writeReg(regVal);
}

void Si7021_MultiWire::heaterOff()
{
	// Turns off the ADDRESS heater
	uint8_t regVal = readReg();
	regVal &= ~_BV(HTRE);
	writeReg(regVal);
}

void Si7021_MultiWire::changeResolution(uint8_t i)
{
	// Changes to resolution of ADDRESS measurements.
	// Set i to:
	//      RH         Temp
	// 0: 12 bit       14 bit (default)
	// 1:  8 bit       12 bit
	// 2: 10 bit       13 bit
	// 3: 11 bit       11 bit

	uint8_t regVal = readReg();
	// zero resolution bits
	regVal &= 0b011111110;
	switch (i) {
	  case 1:
	    regVal |= 0b00000001;
	    break;
	  case 2:
	    regVal |= 0b10000000;
	    break;
	  case 3:
	    regVal |= 0b10000001;
	  default:
	    regVal |= 0b00000000;
	    break;
	}
	// write new resolution settings to the register
	writeReg(regVal);
}

void Si7021_MultiWire::reset()
{
	//Reset user resister
	writeReg(SOFT_RESET);
}

uint8_t Si7021_MultiWire::checkID()
{
	uint8_t ID_1;

 	// Check device ID
	wire.beginTransmission(ADDRESS);
	wire.write(0xFC);
	wire.write(0xC9);
	wire.endTransmission();

    wire.requestFrom(ADDRESS,1);

    ID_1 = wire.read();

    return(ID_1);
}

uint16_t Si7021_MultiWire::makeMeasurment(uint8_t command)
{
	// Take one ADDRESS measurement given by command.
	// It can be either temperature or relative humidity
	// TODO: implement checksum checking

	uint16_t nBytes = 3;
	// if we are only reading old temperature, read olny msb and lsb
	if (command == 0xE0) nBytes = 2;

	wire.beginTransmission(ADDRESS);
	wire.write(command);
	wire.endTransmission();
	// When not using clock stretching (*_NOHOLD commands) delay here
	// is needed to wait for the measurement.
	// According to datasheet the max. conversion time is ~22ms
	 delay(100);

	wire.requestFrom(ADDRESS,nBytes);
	if(wire.available() != nBytes)
  	return 100;

	unsigned int msb = wire.read();
	unsigned int lsb = wire.read();
	// Clear the last to bits of LSB to 00.
	// According to datasheet LSB of RH is always xxxxxx10
	lsb &= 0xFC;
	unsigned int mesurment = msb << 8 | lsb;

	return mesurment;
}

void Si7021_MultiWire::writeReg(uint8_t value)
{
	// Write to user register on ADDRESS
	wire.beginTransmission(ADDRESS);
	wire.write(WRITE_USER_REG);
	wire.write(value);
	wire.endTransmission();
}

uint8_t Si7021_MultiWire::readReg()
{
	// Read from user register on ADDRESS
	wire.beginTransmission(ADDRESS);
	wire.write(READ_USER_REG);
	wire.endTransmission();
	wire.requestFrom(ADDRESS,1);
	uint8_t regVal = wire.read();
	return regVal;
}
