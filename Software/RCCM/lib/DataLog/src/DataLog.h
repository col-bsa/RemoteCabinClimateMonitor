#ifndef DataLog_h
#define DataLog_h

// 
#include <Particle.h>


//
class DataLog {
    public:
        // PUBLIC - Class Variables
        enum eventTypes : uint8_t {
            SYSTEM              = 0,
            DEBUG               = 1,
            TEMPERATURE         = 2,
        };

        enum eventFlags : uint8_t {
            NEW                 = 0,
            ALERT_NEW           = 1,
            ALERT_RESOLVED      = 2,
        };

        uint32_t    logLength;
        
        // PUBLIC - Class Functions
        DataLog(uint32_t logLength);
        ~DataLog();
        bool addEntry(eventTypes, String eventData, eventFlags);

    private:
        // PRIVATE - Class Variables
        struct LogEntry {
            uint32_t        entryNumber;
            time_t          time;
            eventTypes      eventType;
            String          eventData       = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
            eventFlags      eventFlag1;
            bool            published;
        };

        // PRIVATE - Class Functions

};

#endif