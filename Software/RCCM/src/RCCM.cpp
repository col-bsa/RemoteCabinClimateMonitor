/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/home/zach/GitHub/RemoteCabinClimateMonitor/Software/RCCM/src/RCCM.ino"
/*
 * Project: Remote Cabin Climate Monitor (RCCM)
 * Description: A Particle Mesh based environmental sensor board, whose primary purpose is to monitor unoccupied cabin temperatures at Resica Falls Scout Reservation.
 * Author: Z. Cross
 * Date: Oct2021
 */


// === STATIC CONFIGURATION ===
void setup();
void loop();
int publish_alert(String alertType);
void timer_interval_environment_data(void);
int collect_environment_data(String junk);
String power_source_cast(int intPowerSource);
String battery_state_cast(int intBatteryState);
#line 10 "/home/zach/GitHub/RemoteCabinClimateMonitor/Software/RCCM/src/RCCM.ino"
#define FW_VERSION      	"0.1.2-DEV"

// Timer; not currently in use.
#define INTERVAL_ENVIRONMENT_DATA_DELAY_MS      15000     // 15 Seconds 

#define THRESH_TEMP_LOW     40
#define THRESH_TEMP_HIGH    95
#define THRESH_TEMP_DELTA   0.2
#define THRESH_BATT_LOW     25

#define INTERNAL_COLLECTION_INTERVAL    (60*15)            // 15 Minutes
//#define HEARTBEAT_INTERVAL              (60*60*24)         // 1 Day
#define HEARTBEAT_INTERVAL              (60*60)         // 1 Hour

#define ALERT_THROTTLE_DELAY            1010               // ms


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
#include <JsonParserGeneratorRK.h>
#include <LocalTimeRK.h>
#include "secrets.h"


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
    double      batteryCharge;
    int32_t     batteryState;
    int32_t     powerSource;
    double      temperatureF;
    double      humidity;    
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
    Particle.variable("battCharge", environmentDataInterval.batteryCharge);
    Particle.variable("battState", environmentDataInterval.batteryState);
    Particle.variable("humidity", environmentDataInterval.humidity);
    Particle.variable("lightLevel", environmentDataInterval.lightLevel);
    Particle.variable("pwrSrc", environmentDataInterval.powerSource);
    Particle.variable("tempF", environmentDataInterval.temperatureF);
    Particle.variable("time", environmentDataInterval.time);
    Particle.variable("timeStr", environmentDataInterval.timeString);
    Particle.variable("timeVal", environmentDataInterval.timeValid);

    // Particle Cloud Function Registration
    Particle.function("collect_environment_data", collect_environment_data);
    Particle.function("publish_alert", publish_alert);

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

    // Set time zone to Eastern USA daylight saving time
    Time.zone(-4);
    // (https://docs.particle.io/cards/libraries/l/LocalTimeRK/), does not modify base Time class timezone!
    LocalTime::instance().withConfig(LocalTimePosixTimezone("EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00"));

    // Local Temp & Humidity Sensor
    Si7021.begin();

    // Ephemeral Debug Log Message
    Particle.publish("RCCM_Debug: Setup Function");
    
    // Publish Startup Alert
    collect_environment_data(" ");
    publish_alert("SYS_STARTUP");

}   // END setup


// 
void loop() {
    // Local Variable Declarations
    //float fTempDelta;
    static long  lLastDataCollectTime = 0;
    static long  lLastHeartbeatTime   = 0;


    // === TASK SCHEDULING ===
    if (Time.now() >= (lLastDataCollectTime + INTERNAL_COLLECTION_INTERVAL)) {
        lLastDataCollectTime = Time.now();
        bCollectIntervalEnvironmentData = true;

    }


    // === TASK ===
    // Collect interval environment data.  (Timer flag based.)
    if (bCollectIntervalEnvironmentData == true) {
        // Ephemeral Debug Log Message
        Particle.publish("RCCM_Debug: Collect Internal Environment Data");

        // Store current data as last for delta based alert comparison.
        environmentDataLastInterval = environmentDataInterval;

        // Collect New Data
        collect_environment_data(" ");

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
            else
            {
                activeAlertsInterval.bTempLow = false;
            }

            // TEMP_HIGH
            if (environmentDataInterval.temperatureF > fThreshTempHigh) {
                // Clear TEMP_CLEAR alert.
                activeAlertsInterval.bTempClear = false;

                // Set TEMP_HIGH alert.
                activeAlertsInterval.bTempHigh = true;
            }
            else 
            {
                activeAlertsInterval.bTempHigh = false;
            }

            // TEMP_DELTA
            // TODO: Consider how / if to do this, given deep sleep plans.

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
            else 
            {
                activeAlertsInterval.bTempClear = false;
            }

            // POWER_LOSS
            if ((environmentDataInterval.powerSource == POWER_SOURCE_BATTERY) || (environmentDataInterval.powerSource == POWER_SOURCE_UNKNOWN)) {
                // Clear POWER_RESTORE alert.
                activeAlertsInterval.bPowerRestore = false;

                // Set POWER_LOSS alert.
                // TODO: Debounce?
                activeAlertsInterval.bPowerLoss = true;
            }
            else
            {
                activeAlertsInterval.bPowerLoss = false;
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
            else
            {
                activeAlertsInterval.bPowerRestore = false;
            }

            // BATTERY_LOW
            if (environmentDataInterval.batteryCharge < fThreshBattLow) {
                // Set BATTERY_LOW alert.
                activeAlertsInterval.bBatteryLow = true;
            }
            else
            {
                activeAlertsInterval.bBatteryLow = false;
            }

            // HEARTBEAT
            if ((Time.now() >= (lLastHeartbeatTime + HEARTBEAT_INTERVAL))) {
                activeAlertsInterval.bHeartbeat = true;
                lLastHeartbeatTime = Time.now();
            }
            else
            {
                activeAlertsInterval.bHeartbeat = false;
            }


        // Reset Collect Flag
        bCollectIntervalEnvironmentData = false;
    }


    // === PROCESS ALERTS ===
    // NOTE: Alerts don't mute, for now.

        // TEMP_LOW
        if (activeAlertsInterval.bTempLow == true) {
            // Publish Alert
            publish_alert("TEMP_LOW");

            // Clear Alert
            activeAlertsInterval.bTempLow = false;
        }

        // TEMP_HIGH
        if (activeAlertsInterval.bTempHigh == true) {
            // Publish Alert
            publish_alert("TEMP_HIGH");

            // Clear Alert
            activeAlertsInterval.bTempHigh = false;
        }

        // TEMP_DELTA
        if (activeAlertsInterval.bTempDelta == true) {
            // Publish Alert
            publish_alert("TEMP_DELTA");

            // Clear Alert
            activeAlertsInterval.bTempDelta = false;
        }

        // TEMP_CLEAR
        if (activeAlertsInterval.bTempClear == true) {
            // Publish Alert
            publish_alert("TEMP_CLEAR");

            // Clear Alert
            activeAlertsInterval.bTempClear = false;
        }

        // POWER_LOSS
        if (activeAlertsInterval.bPowerLoss == true) {
            // Publish Alert
            publish_alert("POWER_LOSS");

            // Clear Alert
            activeAlertsInterval.bPowerLoss = false;
        }

        // POWER_RESTORE
        if (activeAlertsInterval.bPowerRestore == true) {
            // Publish Alert
            publish_alert("POWER_RESTORE");

            // Clear Alert
            activeAlertsInterval.bPowerRestore = false;          
        }

        // BATTERY_LOW
        if (activeAlertsInterval.bBatteryLow == true) {
            // Publish Alert
            publish_alert("BATTERY_LOW");

            // Clear Alert
            activeAlertsInterval.bBatteryLow = false;
        }

        // HEARTBEAT
        if (activeAlertsInterval.bHeartbeat == true) {
            // Publish Alert
            publish_alert("HEARTBEAT");

            // Clear Alert
            activeAlertsInterval.bHeartbeat = false;
        }


}   // END loop


// 
int publish_alert(String alertType) {
    // Variable Declarations
    String body = "Cloud SMS Test";
    String batteryState = battery_state_cast(environmentDataInterval.batteryState);
    String powerSource = power_source_cast(environmentDataInterval.powerSource);

    // Detect manual function calls.
    if (alertType.length()  == 0) {
        alertType = "DEBUG";
    }

    // Build & Format SMS Body
    body = String::format("=== %s ===\nAlert: %s\nTemp: %.1fF\nHumidity: %.1f%%\nBatt: %.1f%%\nBatt State: %s\nPWR SRC: %s\nFW: %s\n%s\n", SECRET_LOCATION, alertType.c_str(), environmentDataInterval.temperatureF, environmentDataInterval.humidity, environmentDataInterval.batteryCharge, batteryState.c_str(), powerSource.c_str(), FW_VERSION, environmentDataInterval.timeString.c_str());


    // Throttle Alert Publishing
    delay(ALERT_THROTTLE_DELAY);

    // JSON Writer Example: https://github.com/rickkas7/JsonParserGeneratorRK/blob/master/examples/2-generator/2-generator-JsonParserGeneratorRK.cpp
    // {{Moustache}} templates used to populate To/From/Body form fields in Twilio API call.
    JsonWriterStatic<256> jwA;
    JsonWriterStatic<256> jwB;
    {
        JsonWriterAutoObject obj(&jwA);

        jwA.insertKeyValue("SMS_TO", SECRET_SMS_TO_A);
        jwA.insertKeyValue("SMS_FROM", SECRET_SMS_FROM);
        jwA.insertKeyValue("SMS_BODY", body);
    }

    {
        JsonWriterAutoObject obj(&jwB);

        jwB.insertKeyValue("SMS_TO", "");
        jwB.insertKeyValue("SMS_FROM", SECRET_SMS_FROM);
        jwB.insertKeyValue("SMS_BODY", body);
    }

    // Publish Alert Data
    Particle.publish("twilio_sms", jwA.getBuffer());
    delay(ALERT_THROTTLE_DELAY);
    Particle.publish("twilio_sms", jwB.getBuffer());
    delay(ALERT_THROTTLE_DELAY);
    
    // Return Length of Alert Body
    return body.length();

}   // END publish_alert


//
void timer_interval_environment_data(void) {
    bCollectIntervalEnvironmentData = true;
}   // END timer_interval_environment_data



int collect_environment_data(String junk) {
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

    // Return Current Temperature (cast to int)
    return (int)environmentDataReading.temperatureF;

}   // END collect_environment_data


//
String power_source_cast(int intPowerSource) {
    // https://docs.particle.io/cards/firmware/system-calls/powersource/

    String strPowerSource = "NONE_ERR";

    switch (intPowerSource) {
        case POWER_SOURCE_UNKNOWN:      // 0
        strPowerSource = "UNKNOWN";
        break;

        case POWER_SOURCE_VIN:          // 1
        strPowerSource = "VIN";
        break;

        case POWER_SOURCE_USB_HOST:     // 2
        strPowerSource = "USB_HOST";
        break;

        case POWER_SOURCE_USB_ADAPTER:  // 3
        strPowerSource = "USB_ADAPTER";
        break;

        case POWER_SOURCE_USB_OTG:      // 4
        strPowerSource = "USB_OTG";
        break;

        case POWER_SOURCE_BATTERY:      // 5
        strPowerSource = "BATTERY";
        break;
    }

    return strPowerSource;
}   // END power_source_cast


String battery_state_cast(int intBatteryState) {
    // https://docs.particle.io/cards/firmware/system-calls/batterystate/

    String strBatteryState = "NONE_ERR";

    switch (intBatteryState) {
        case BATTERY_STATE_UNKNOWN:         // 0
        strBatteryState = "UNKNOWN";
        break;

        case BATTERY_STATE_NOT_CHARGING:    // 1
        strBatteryState = "NOT_CHARGING";
        break;

        case BATTERY_STATE_CHARGING:        // 2
        strBatteryState = "CHARGING";
        break;

        case BATTERY_STATE_CHARGED:         // 3
        strBatteryState = "CHARGED";
        break;

        case BATTERY_STATE_DISCHARGING:     // 4
        strBatteryState = "DISCHARGING";
        break;

        case BATTERY_STATE_FAULT:           // 5
        strBatteryState = "FAULT";
        break;

        case BATTERY_STATE_DISCONNECTED:    // 6
        strBatteryState = "DISCONNECTED";
        break;
    }

    return strBatteryState;
}