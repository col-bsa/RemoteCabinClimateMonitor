/*
 * Project: Remote Cabin Climate Monitor (RCCM)
 * Description: A Particle Mesh based environmental sensor board, whose primary purpose is to monitor unoccupied cabin temperatures at Resica Falls Scout Reservation.
 * Author: Z. Cross
 * Date: Oct2021
 */


// === STATIC CONFIGURATION ===
#define FW_VERSION      	"0.0.1-IFM"

#define INTERVAL_ENVIRONMENT_DATA_DELAY_MS      15000     // 15 Seconds

#define THRESH_TEMP_LOW     35
#define THRESH_TEMP_HIGH    95
#define THRESH_TEMP_DELTA   0.2
#define THRESH_BATT_LOW     10


// === PCB PINPOUT DEFINITIONS ===
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


// === INTERNAL MACROS ===
#define C_TO_F(celsius)     (((celsius) * 1.8) + 32)


// === INCLUDES ===
#include <DataLog.h>
#include <Adafruit_Si7021.h>


// === GLOBAL OBJECTS ===
DataLog         dataLog(100);
Adafruit_Si7021 Si7021 = Adafruit_Si7021();     // Onboard I2C Temp & Humidity Sensor
FuelGauge       fuel;                           // Onboard Battery Fuel Gauge


// === TIMERS ===
Timer tCollectEnvironmentData(INTERVAL_ENVIRONMENT_DATA_DELAY_MS, timer_interval_environment_data);    


// === GLOBAL STRUCT / ENUM DEFINITIONS ===
// Reasons to generate user alert.
struct alertList {
    bool bTempLow;
    bool bTempHigh;
    bool bTempDelta;
    bool bTempClear;
    bool bPowerLoss;
    bool bPowerRestore;
    bool bBatteryLow;
    bool bHeartbeat;
};

// Environmental Data Collected
struct environmentData {
    long        time;
    String      timeString;
    bool        timeValid;
    float       batteryCharge;
    int32_t     batteryState;
    int32_t     powerSource;
    float       temperatureF;
    float       humidity;    
    int32_t     lightLevel;
};


// === GLOBAL VARIABLES ===
const String    sFwVersion                          = FW_VERSION;
bool            bCollectIntervalEnvironmentData     = false;
float           fThreshTempLow                      = THRESH_TEMP_LOW;
float           fThreshTempHigh                     = THRESH_TEMP_HIGH;
float           fThreshTempDelta                    = THRESH_TEMP_DELTA;
bool            bCurrentTempAlert                   = false;
float           fThreshBattLow                      = THRESH_BATT_LOW;
struct environmentData  environmentDataInterval;
struct environmentData  environmentDataLastInterval;
struct alertList        activeAlertsInterval;
struct alertList        activeAlertsLastInterval;    


// === PARTICLE CONFIGURATION ===
//SYSTEM_MODE(SEMI_AUTOMATIC);
//SYSTEM_THREAD(ENABLED);

// 
void setup() {
    // Particle Cloud Variable Registration
    Particle.variable("sFwVersion", sFwVersion);


    // Particle Cloud Function Registration


    // I/O
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


    // Debug UART
    Serial.begin(115200);
    Serial.println("=== REMOTE CABIN CLIMATE MONITOR ===");
    Serial.println(FW_VERSION);

    // Local Temp & Humidity Sensor
    Si7021.begin();


}   // END setup


// 
void loop() {
    // Local Variable Declarations
    float fTempDelta;          

    // Collect interval environment data.  (Timer flag based.)
    if (bCollectIntervalEnvironmentData == true) {
        // Store current data as last for delta based alert comparison.
        environmentDataLastInterval = environmentDataInterval;

        // Collect New Data
        collect_environment_data();

        // Generate Alerts Based On New Data
            // Store current data as last for delta based alert comparison.
            activeAlertsLastInterval = activeAlertsInterval;

            // TEMP_LOW
            if (environmentDataInterval.temperatureF < fThreshTempLow) {
                // Clear TEMP_CLEAR alert.
                activeAlertsInterval.bTempClear = false;

                // Set TEMP_LOW alert.
                activeAlertsInterval.bTempLow = true;
            }

            // TEMP_HIGH
            if (environmentDataInterval.temperatureF > fThreshTempHigh) {
                // Clear TEMP_CLEAR alert.
                activeAlertsInterval.bTempClear = false;

                // Set TEMP_HIGH alert.
                activeAlertsInterval.bTempHigh = true;
            }

            // TEMP_DELTA
            fTempDelta = (environmentDataInterval.temperatureF / environmentDataLastInterval.temperatureF);
            fTempDelta = abs(fTempDelta);
            if (fTempDelta > fThreshTempDelta) {
                // Set TEMP_DELTA alert.
                activeAlertsInterval.bTempDelta = true;
            }

            // TEMP_CLEAR
            // (Dependent on TEMP_LOW & TEMP_HIGH alert evaluation logic occurring first.)
            // Temp Alert Currently False...
            if ((activeAlertsInterval.bTempLow == false) && (activeAlertsInterval.bTempHigh == false)) {            
                // ...AND Temp Alert WAS True
                if ((activeAlertsLastInterval.bTempLow == true) || (activeAlertsLastInterval.bTempHigh == true)) {
                    // Set TEMP_CLEAR alert.
                    activeAlertsInterval.bTempClear = true;
                }
            }

            // POWER_LOSS
            if ((environmentDataInterval.powerSource == POWER_SOURCE_BATTERY) || (environmentDataInterval.powerSource == POWER_SOURCE_UNKNOWN)) {
                // Clear POWER_RESTORE alert.
                activeAlertsInterval.bPowerRestore = false;

                // Set POWER_LOSS alert.
                activeAlertsInterval.bPowerLoss = true;
            }

            // POWER_RESTORE
            // (Dependent on POWER_LOSS alert evaluation logic occurring first.)
            // Power Loss Alert Currently False...
            if (activeAlertsInterval.bPowerLoss == false) {
                // ...AND Power Loss Alert WAS True
                if (activeAlertsLastInterval.bPowerLoss == true) {
                    // Set POWER_RESTORE alert.
                    activeAlertsInterval.bPowerRestore = true;
                }
            }

            // BATTERY_LOW
            if (environmentDataInterval.batteryCharge < fThreshBattLow) {
                // Set BATTERY_LOW alert.
                activeAlertsInterval.bBatteryLow = true;
            }

            // HEARTBEAT


        // Reset Collect Flag
        bCollectIntervalEnvironmentData = false;
    }


    // Process Alerts


    /*
    Serial.println("Time: ");
    Serial.println(currentEnvironmentData.time);
    Serial.println("Time Str: ");
    Serial.println(currentEnvironmentData.timeString);
    Serial.println("Time IsValid: ");
    Serial.println(currentEnvironmentData.timeValid);

    Serial.println("Power Source: ");
    Serial.println(currentEnvironmentData.powerSource);
    Serial.println("Battery State: ");
    Serial.println(currentEnvironmentData.batteryState);
    Serial.println("Battery Charge: ");
    Serial.println(currentEnvironmentData.batteryCharge);

    Serial.println("Temperature: ");
    Serial.println(currentEnvironmentData.temperatureF);
    Serial.println("Humidity: ");
    Serial.println(currentEnvironmentData.humidity);

    Serial.println("LightLevel: ");
    Serial.println(currentEnvironmentData.lightLevel);
    delay(10000);
    */

}   // END loop


// 
void publish_alert(void) {

}   // END publish_alert


//
void timer_interval_environment_data(void) {
    bCollectIntervalEnvironmentData = true;
}   // END timer_interval_environment_data



void collect_environment_data(void) {
    // Local Variable Declarations
    struct environmentData environmentDataReading;

    // Time
    environmentDataReading.time             = Time.now();
    environmentDataReading.timeString       = Time.timeStr();
    environmentDataReading.timeValid        = Time.isValid();

    // System Power
    environmentDataReading.powerSource      = System.powerSource();
    environmentDataReading.batteryState     = System.batteryState();
    environmentDataReading.batteryCharge    = System.batteryCharge();

    // Temperature & Humidity
    environmentDataReading.temperatureF     = C_TO_F(Si7021.readTemperature());
    environmentDataReading.humidity         = Si7021.readHumidity();

    // Light Level
    environmentDataReading.lightLevel       = analogRead(PIN_LIGHT_SEN);

    // Save Last Interval Reading & Save New Data
    environmentDataLastInterval = environmentDataInterval;
    environmentDataInterval = environmentDataReading;

}   // END collect_environment_data


//
String power_source_cast(int intPowerSource) {
    // https://docs.particle.io/cards/firmware/system-calls/powersource/

    String strPowerSource = "NONE_ERR";

    switch (intPowerSource) {
        case POWER_SOURCE_UNKNOWN:      // 0
        strPowerSource = "POWER_SOURCE_UNKNOWN";
        break;

        case POWER_SOURCE_VIN:          // 1
        strPowerSource = "POWER_SOURCE_VIN";
        break;

        case POWER_SOURCE_USB_HOST:     // 2
        strPowerSource = "POWER_SOURCE_USB_HOST";
        break;

        case POWER_SOURCE_USB_ADAPTER:  // 3
        strPowerSource = "POWER_SOURCE_USB_ADAPTER";
        break;

        case POWER_SOURCE_USB_OTG:      // 4
        strPowerSource = "POWER_SOURCE_USB_OTG";
        break;

        case POWER_SOURCE_BATTERY:      // 5
        strPowerSource = "POWER_SOURCE_BATTERY";
        break;
    }

    return strPowerSource;
}   // END power_source_cast


String battery_state_cast(int intBatteryState) {
    // https://docs.particle.io/cards/firmware/system-calls/batterystate/

    String strBatteryState = "NONE_ERR";

    switch (intBatteryState) {
        case BATTERY_STATE_UNKNOWN:         // 0
        strBatteryState = "BATTERY_STATE_UNKNOWN";
        break;

        case BATTERY_STATE_NOT_CHARGING:    // 1
        strBatteryState = "BATTERY_STATE_NOT_CHARGING";
        break;

        case BATTERY_STATE_CHARGING:        // 2
        strBatteryState = "BATTERY_STATE_CHARGING";
        break;

        case BATTERY_STATE_CHARGED:         // 3
        strBatteryState = "BATTERY_STATE_CHARGED";
        break;

        case BATTERY_STATE_DISCHARGING:     // 4
        strBatteryState = "BATTERY_STATE_DISCHARGING";
        break;

        case BATTERY_STATE_FAULT:           // 5
        strBatteryState = "BATTERY_STATE_FAULT";
        break;

        case BATTERY_STATE_DISCONNECTED:    // 6
        strBatteryState = "BATTERY_STATE_DISCONNECTED";
        break;
    }

    return strBatteryState;
}