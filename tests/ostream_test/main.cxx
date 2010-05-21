
#include "log4cplus/logger.h"
#include "log4cplus/consoleappender.h"
#include "log4cplus/loglevel.h"
#include <log4cplus/loggingmacros.h>
#include <iomanip>

using namespace std;
using namespace log4cplus;

int
main()
{
    SharedAppenderPtr append_1(new ConsoleAppender());
    append_1->setName(LOG4CPLUS_TEXT("First"));
    Logger::getRoot().addAppender(append_1);

    Logger root = Logger::getRoot();
    Logger test = Logger::getInstance(LOG4CPLUS_TEXT("test"));

    LOG4CPLUS_DEBUG(root,
                    "This is"
                    << " a reall"
                    << "y long message." << endl
                    << "Just testing it out" << endl
                    << "What do you think?");
    test.setLogLevel(NOT_SET_LOG_LEVEL);
    LOG4CPLUS_DEBUG(test, "This is a bool: " << true);
    LOG4CPLUS_INFO(test, "This is a char: " << 'x');
    LOG4CPLUS_INFO(test, "This is a short: " << (short)-100);
    LOG4CPLUS_INFO(test, "This is a unsigned short: " << (unsigned short)100);
    LOG4CPLUS_INFO(test, "This is a int: " << (int)1000);
    LOG4CPLUS_INFO(test, "This is a unsigned int: " << (unsigned int)1000);
    LOG4CPLUS_INFO(test, "This is a long(hex): " << hex << (long)100000000);
    LOG4CPLUS_INFO(test, "This is a unsigned long: " << (unsigned long)100000000);
    LOG4CPLUS_WARN(test, "This is a float: " << (float)1.2345);
    LOG4CPLUS_ERROR(test, 
                    "This is a double: " 
                    << setprecision(15) 
                    << (double)1.2345234234);
    LOG4CPLUS_FATAL(test, 
                    "This is a long double: " 
                    << setprecision(15) 
                    << (long double)123452342342.342);
    LOG4CPLUS_WARN(test, "The following message is empty:");
    LOG4CPLUS_WARN(test, "");

    return 0;
}


