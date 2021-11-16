#ifndef DataLog_h
#define DataLog_h

// 
#include <Particle.h>


//
class DataLog {
    public:
        // PUBLIC - Class Variables
        enum eventType : uint8_t {
            SYSTEM              = 0,
            DEBUG               = 1,
            TEMPERATURE         = 2,
        };

        enum eventFlag : uint8_t {
            NEW                 = 0,
            ALERT_NEW           = 1,
            ALERT_RESOLVED      = 2,
        };

        uint32_t    logLength;
        
        // PUBLIC - Class Functions
        DataLog(uint32_t logLength);
        ~DataLog();
        bool addEntry(eventType, String eventData, eventFlag);

    private:
        // PRIVATE - Class Variables
        struct LogEntry {
            uint32_t        entryNumber;
            time_t          time;
            eventType       eventType;
            String          eventData;
            eventFlag       eventFlag;
            bool            published;
        };

        // PRIVATE - Class Functions

};

#endif