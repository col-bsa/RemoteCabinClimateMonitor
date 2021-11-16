
// INCLUDEs
#include "DataLog.h"




// CONSTRUCTOR
DataLog::DataLog(uint32_t logLength) {
    // Create array of logEntry structs, thus instantiating our fixed length log.
    struct LogEntry LogEntries[logLength];
}


// DESTRUCTOR
DataLog::~DataLog() {

}


// 
bool DataLog::addEntry(eventTypes, String eventData, eventFlags) {

    return true;
}