
#include "customloglevel.h"

#define _CRITICAL_STRING LOG4CPLUS_TEXT("CRITICAL")



tstring
criticalToStringMethod(LogLevel ll)
{
    if(ll == CRITICAL_LOG_LEVEL) {
        return _CRITICAL_STRING;
    }
    else {
        return tstring();
    }
}



LogLevel
criticalFromStringMethod(const tstring& s) 
{
    if(s == _CRITICAL_STRING) return CRITICAL_LOG_LEVEL;

    return NOT_SET_LOG_LEVEL;
}



class CriticalLogLevelInitializer {
public:
    CriticalLogLevelInitializer() {
        getLogLevelManager().pushToStringMethod(criticalToStringMethod);
        getLogLevelManager().pushFromStringMethod(criticalFromStringMethod);
    }
};

CriticalLogLevelInitializer criticalLogLevelInitializer_;

