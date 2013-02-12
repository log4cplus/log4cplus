
#include <iostream>
#include <log4cplus/helpers/loglog.h>

using namespace std;
using namespace log4cplus::helpers;

void printMsgs() {
    cout << "Entering printMsgs()..." << endl;
    LogLog::getLogLog()->debug(LOG4CPLUS_TEXT("This is a Debug statement..."));
    LogLog::getLogLog()->warn(LOG4CPLUS_TEXT("This is a Warning..."));
    LogLog::getLogLog()->error(LOG4CPLUS_TEXT("This is a Error..."));
    cout << "Exiting printMsgs()..." << endl << endl;
}


int
main() {
    log4cplus::initialize ();
    printMsgs();

    cout << "Turning on debug..." << endl;
    LogLog::getLogLog()->setInternalDebugging(true);
    printMsgs();

    cout << "Turning on quiet mode..." << endl;
    LogLog::getLogLog()->setQuietMode(true);
    printMsgs();

    return 0;
}

