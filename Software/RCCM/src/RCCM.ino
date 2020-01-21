/*
 * Project: Remote Cabin Climate Monitor (RCCM)
 * Description: A Particle Mesh based environmental sensor board, whose primary purpose is to monitor unoccupied cabin temperatures at Resica Falls Scout Reservation.
 * Author: Z. Cross & D. Gibbons
 * Date: 08Jan2020
 */


// PIN DEFINITIONS
#define PIN_LIGHT_SEN     19  // Internal Light Sensor Signal (Analog)
#define PIN_ADC_1         18  // External GPIO Port Signal
#define PIN_ADC_2         17  // External GPIO Port Signal
#define PIN_PIR           16  // PIR Sensor Port Signal
#define PIN_INT_SIG       15  // Internal Sensor Port Signal
#define PIN_DIP_1         14  // Internal DIP Switch Position 1
#define PIN_DIP_2         13  // Internal DIP Switch Position 2
#define PIN_DIP_3         12  // Internal DIP Switch Position 3
#define PIN_DIP_4         11  // Internal DIP Switch Position 4
#define PIN_UART_Rx       10  // External UART Port Signal Rx
#define PIN_UART_Tx       9   // External UART Port Signal Tx

#define PIN_S1            8   // Internal Pushbutton S1
#define PIN_S2            7   // Internal Pushbutton S2
#define PIN_LED_HEARTB    4   // Internal Heartbeat LED (Green)
#define PIN_LED_LOCATE    5   // Internal Locate LED (Red)
#define PIN_LED_ACTIVITY  6   // Internal Activity LED (Red)
#define PIN_1W            3   // External 1-Wire Bus Signal
#define PIN_DIP_5         2   // Internal DIP Switch Position 5



// INCLUDES
#include "Si7021_MultiWire.h"   // Si7021
#include "DS18B20.h"            // DS18B20
#include <math.h>               // DS18B20


// FUNCTION PROTOTYPES
void self_test_blocking(void);
void ledFlutter(int pin, int time, int loops);
void allOn(void);
void allOff(void);
double getTemp(void);


// Initialize objects from the lib
Si7021_MultiWire sensor = Si7021_MultiWire(Wire1);


// DS18B20
const int      MAXRETRY          = 4;
const uint32_t msSAMPLE_INTERVAL = 2500;
const uint32_t msMETRIC_PUBLISH  = 30000;

DS18B20  ds18b20(PIN_1W, true); //Sets Pin D2 for Water Temp Sensor and 
                            // this is the only sensor on bus
char     szInfo[64];
double   celsius;
double   fahrenheit;
uint32_t msLastMetric;
uint32_t msLastSample;


// 
void setup() {
    // Internal Sensor Expansion
    pinMode(PIN_INT_SIG, INPUT);

    // PIR Sensor
    pinMode(PIN_PIR, INPUT);

    // Internal Switches
    pinMode(PIN_DIP_1,  INPUT_PULLUP);
    pinMode(PIN_DIP_2,  INPUT_PULLUP);
    pinMode(PIN_DIP_3,  INPUT_PULLUP);
    pinMode(PIN_DIP_4,  INPUT_PULLUP);
    pinMode(PIN_DIP_5,  INPUT_PULLUP);
    pinMode(PIN_S1,     INPUT_PULLUP);
    pinMode(PIN_S2,     INPUT_PULLUP);

    // Internal Status LEDs
    pinMode(PIN_LED_HEARTB,     OUTPUT);
    pinMode(PIN_LED_LOCATE,     OUTPUT);
    pinMode(PIN_LED_ACTIVITY,   OUTPUT);

    // Internal Sensor Interfaces
    pinMode(PIN_PIR,        INPUT_PULLUP);
    pinMode(PIN_INT_SIG,    INPUT_PULLUP);

    // External Sensor Interfaces
    pinMode(PIN_ADC_1,  INPUT_PULLUP);
    pinMode(PIN_ADC_2,  INPUT_PULLUP);
    pinMode(PIN_1W,     INPUT_PULLUP);

    // Initialize Debug UART
    Serial.begin(9600);
    Serial.println("=== RCCM STARTUP ===");

    // I2C Bus Initizalition
    Serial.println(sensor.begin());


    // Run Self Test
    //self_test_blocking();

}

// 
void loop() {

    //
    waitFor(Serial.isConnected, 30000);
    self_test_blocking();
    
}


void self_test_blocking() {

    // SELF-TEST START
    Serial.println("=== RCCM SELF TEST START ===");
    for (int i = 10; i > 0; i--)
    {
        Serial.println(i);
        delay(500);
    }
    Serial.println("");
    Serial.println("");
    Serial.println("");
    delay(3000);


    // EXTERNAL 1-W DS18B20 SENSOR TEST
    Serial.println("TEST - 1-W DS18B20");
    //Serial.println("      Info: Confirm valid temperature readings (10x repeat, once per second).");
    for (int i = 0; i < 10; i++)
    {
        getTemp();
        //Serial.print("Temperature:    ");
        Serial.println(fahrenheit, 2);
        delay(1000);
    }
    Serial.println("");
    Serial.println("");
    Serial.println("");


    // Si7021 SENSOR TEST
    Serial.println("TEST - Si7021");
    //Serial.println("      Info: Confirm valid temperature and humidity readings (10x repeat, once per second).");
    for (int i = 0; i < 10; i++)
    {
        Serial.print("Humidity:    ");
        Serial.print(sensor.getRH(), 2);
        Serial.print("    Temperature: ");
        Serial.println(sensor.readTempF(), 2);
        delay(1000);
    }
    Serial.println("");
    Serial.println("");
    Serial.println("");
    

    // AMBIENT LIGHT SENSOR TEST
    Serial.println("TEST - Ambient Light Sensor");
    //Serial.println("      Info: Confirm valid ambient light readings (10x repeat, once per second).");
    for (int i = 0; i < 10; i++)
    {
        int lightLevel = 0;
        lightLevel = analogRead(PIN_LIGHT_SEN);
        Serial.print("Light Level:    ");
        Serial.println(lightLevel);
        delay(1000);
    }
    Serial.println("");
    Serial.println("");
    Serial.println("");


    // ONBOARD SWITCH TEST
    Serial.println("TEST - Onboard Switches");
    Serial.println("        Info: Toggle S1, S2, and dip 1-5 in order.");

    while (digitalRead(PIN_S1) == HIGH);
    Serial.println("        S1: Pass");
    ledFlutter(PIN_LED_HEARTB, 50, 10);
    allOff();

    while (digitalRead(PIN_S2) == HIGH);
    Serial.println("        S2: Pass");
    ledFlutter(PIN_LED_HEARTB, 50, 10);
    allOff();

    while (digitalRead(PIN_DIP_1) == HIGH);
    while (digitalRead(PIN_DIP_1) == LOW);
    while (digitalRead(PIN_DIP_1) == HIGH);
    Serial.println("        DIP1: Pass");
    ledFlutter(PIN_LED_HEARTB, 50, 10);
    allOff();

    while (digitalRead(PIN_DIP_2) == HIGH);
    while (digitalRead(PIN_DIP_2) == LOW);
    while (digitalRead(PIN_DIP_2) == HIGH);
    Serial.println("        DIP2: Pass");
    ledFlutter(PIN_LED_HEARTB, 50, 10);
    allOff();

    while (digitalRead(PIN_DIP_3) == HIGH);
    while (digitalRead(PIN_DIP_3) == LOW);
    while (digitalRead(PIN_DIP_3) == HIGH);
    Serial.println("        DIP3: Pass");
    ledFlutter(PIN_LED_HEARTB, 50, 10);
    allOff();

    while (digitalRead(PIN_DIP_4) == HIGH);
    while (digitalRead(PIN_DIP_4) == LOW);
    while (digitalRead(PIN_DIP_4) == HIGH);
    Serial.println("        DIP4: Pass");
    ledFlutter(PIN_LED_HEARTB, 50, 10);
    allOff();

    while (digitalRead(PIN_DIP_5) == HIGH);
    while (digitalRead(PIN_DIP_5) == LOW);
    while (digitalRead(PIN_DIP_5) == HIGH);
    Serial.println("        DIP5: Pass");
    ledFlutter(PIN_LED_HEARTB, 50, 10);
    allOff();

    Serial.println("");
    Serial.println("");
    Serial.println("");


    // ONBOARD LED TEST
    Serial.println("TEST - Onboard LEDs");
    Serial.println("      Info: Confirm sequential flicker of D4, D5, D6.");
    ledFlutter(PIN_LED_HEARTB, 50, 10);
    ledFlutter(PIN_LED_LOCATE, 50, 10);
    ledFlutter(PIN_LED_ACTIVITY, 50, 10);
    allOff();

    Serial.println("");
    Serial.println("");
    Serial.println("");


    // ONBOARD SENSOR INTERFACE TEST
    Serial.println("TEST - Onboard Sensor Interfaces");

    Serial.println("        Info: Toggle PIR sensor signal pin (J4) from floating (high pullup) to low and back.");
    while (digitalRead(PIN_PIR) == HIGH);
    Serial.println("        PIR SIG: Pass");
    ledFlutter(PIN_LED_HEARTB, 50, 10);
    allOff();

    Serial.println("        Info: Toggle internal sensor expansion signal pin (J5) from floating (high pullup) to low and back.");
    while (digitalRead(PIN_INT_SIG) == HIGH);
    Serial.println("        PIN_INT_SIG: Pass");
    ledFlutter(PIN_LED_HEARTB, 50, 10);
    allOff();

    Serial.println("");
    Serial.println("");
    Serial.println("");


    // EXTERNAL SENSOR INTERFACE TEST
    Serial.println("TEST - External Sensor Interfaces");

    Serial.println("        Info: Set GPIO A&B jumpers to ADC mode and press S1 to continue.");
    while (digitalRead(PIN_S1) == HIGH);

    Serial.println("        Info: Toggle GPIO A signal pin (J10) from floating (high pullup) to low and back.");
    while (digitalRead(PIN_ADC_1) == HIGH);
    Serial.println("        GPIO A: Pass");
    ledFlutter(PIN_LED_HEARTB, 50, 10);
    allOff();

    Serial.println("        Info: Toggle GPIO B signal pin (J10) from floating (high pullup) to low and back.");
    while (digitalRead(PIN_ADC_2) == HIGH);
    Serial.println("        GPIO B: Pass");
    ledFlutter(PIN_LED_HEARTB, 50, 10);
    allOff();

    Serial.println("        Info: Toggle GPIO C signal pin (J13) from floating (high pullup) to low and back.");
    while (digitalRead(PIN_ADC_1) == HIGH);
    Serial.println("        GPIO C: Pass");
    ledFlutter(PIN_LED_HEARTB, 50, 10);
    allOff();

    Serial.println("        Info: Toggle GPIO D signal pin (J13) from floating (high pullup) to low and back.");
    while (digitalRead(PIN_ADC_2) == HIGH);
    Serial.println("        GPIO D: Pass");
    ledFlutter(PIN_LED_HEARTB, 50, 10);
    allOff();

    Serial.println("        Info: Toggle 1-W signal pin (J6) from floating (high pullup) to low and back.");
    while (digitalRead(PIN_1W) == HIGH);
    Serial.println("        GPIO 1-W: Pass");
    ledFlutter(PIN_LED_HEARTB, 50, 10);
    allOff();

    Serial.println("");
    Serial.println("");
    Serial.println("");


    // TEST COMPLETE
    Serial.println("=== RCCM SELF TEST COMPLETE ===");

}


void ledFlutter(int pin, int time, int loops) {

    for (int i = 0; i < loops; i++) {
        digitalWrite(pin, HIGH);
        delay(time);
        digitalWrite(pin, LOW);
        delay(time);
    }

}


void allOn() {
    digitalWrite(PIN_LED_HEARTB, HIGH);
    digitalWrite(PIN_LED_LOCATE, HIGH);
    digitalWrite(PIN_LED_ACTIVITY, HIGH);
}


void allOff() {
    digitalWrite(PIN_LED_HEARTB, LOW);
    digitalWrite(PIN_LED_LOCATE, LOW);
    digitalWrite(PIN_LED_ACTIVITY, LOW);
}


double getTemp(){
  float _temp;
  int   i = 0;

  do {
    _temp = ds18b20.getTemperature();
  } while (!ds18b20.crcCheck() && MAXRETRY > i++);

  if (i < MAXRETRY) {
    celsius = _temp;
    fahrenheit = ds18b20.convertToFahrenheit(_temp);
    //Serial.println(fahrenheit);
  }
  else {
    celsius = fahrenheit = NAN;
    //Serial.println("Invalid reading");
  }
  msLastSample = millis();

  return fahrenheit;
}