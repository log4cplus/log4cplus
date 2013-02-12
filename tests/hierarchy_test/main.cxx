
#include <iostream>
#include <string>
#include "log4cplus/hierarchy.h"
#include "log4cplus/helpers/loglog.h"

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;

int
main()
{
    log4cplus::initialize ();
    {

        Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("test"));
        log4cplus::tcout << "Logger name: " << logger.getName()
             << " Parent = " << logger.getParent().getName() << endl;
        
        logger = Logger::getInstance(LOG4CPLUS_TEXT("test2"));
        log4cplus::tcout << "Logger name: " << logger.getName()
             << " Parent = " << logger.getParent().getName() << endl;
        
        logger = Logger::getInstance(LOG4CPLUS_TEXT("test.subtest.a.b.c"));
        log4cplus::tcout << "Logger name: " << logger.getName()
             << " Parent = " << logger.getParent().getName() << endl;
        
        logger = Logger::getInstance(LOG4CPLUS_TEXT("test.subtest.a"));
        log4cplus::tcout << "Logger name: " << logger.getName()
             << " Parent = " << logger.getParent().getName() << endl;
        
        logger = Logger::getInstance(LOG4CPLUS_TEXT("test.subtest"));
        log4cplus::tcout << "Logger name: " << logger.getName()
             << " Parent = " << logger.getParent().getName() << endl;
        
        logger = Logger::getInstance(LOG4CPLUS_TEXT("test.subtest.a"));
        log4cplus::tcout << "Logger name: " << logger.getName()
             << " Parent = " << logger.getParent().getName() << endl;
        
        logger = Logger::getInstance(LOG4CPLUS_TEXT("test.subtest.a.b.c"));
        log4cplus::tcout << "Logger name: " << logger.getName()
             << " Parent = " << logger.getParent().getName() << endl;
        
        logger = Logger::getInstance(LOG4CPLUS_TEXT("test.subtest.a.b.c.d"));
        log4cplus::tcout << "Logger name: " << logger.getName()
             << " Parent = " << logger.getParent().getName() << endl;
        
        logger = Logger::getInstance(LOG4CPLUS_TEXT("test.subtest.a.b.c"));
        log4cplus::tcout << "Logger name: " << logger.getName()
             << " Parent = " << logger.getParent().getName() << endl;
        
        logger = Logger::getInstance(LOG4CPLUS_TEXT("test.subtest.a"));
        log4cplus::tcout << "Logger name: " << logger.getName()
             << " Parent = " << logger.getParent().getName() << endl;
        
        logger = Logger::getInstance(LOG4CPLUS_TEXT("test.subtest"));
        log4cplus::tcout << "Logger name: " << logger.getName()
             << " Parent = " << logger.getParent().getName() << endl;
        
        Logger::shutdown();
    }
    log4cplus::tcout << "Exiting main()..." << endl;
    return 0;
}

