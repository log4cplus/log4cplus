
#include <iostream>
#include <log4cplus/helpers/loglog.h>

using namespace std;
using namespace log4cplus::helpers;

void printMsgs() {
    cout << "Entering printMsgs()..." << endl;
    getLogLog().debug("This is a Debug statement...");
    getLogLog().warn("This is a Warning...");
    getLogLog().error("This is a Error...");
    cout << "Exiting printMsgs()..." << endl << endl;
}


int
main() {
    printMsgs();

    cout << "Turning on debug..." << endl;
    getLogLog().setInternalDebugging(true);
    printMsgs();

    cout << "Turning on quiet mode..." << endl;
    getLogLog().setQuietMode(true);
    printMsgs();

    return 0;
}

