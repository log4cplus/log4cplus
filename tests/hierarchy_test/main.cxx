
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
    {

    Logger logger = Logger::getInstance("test");
    cout << "Logger name: " << logger.getName()
         << " Parent = " << logger.getParent().getName() << endl;

    logger = Logger::getInstance("test2");
    cout << "Logger name: " << logger.getName()
         << " Parent = " << logger.getParent().getName() << endl;

    logger = Logger::getInstance("test.subtest.a.b.c");
    cout << "Logger name: " << logger.getName()
         << " Parent = " << logger.getParent().getName() << endl;

    logger = Logger::getInstance("test.subtest.a");
    cout << "Logger name: " << logger.getName()
         << " Parent = " << logger.getParent().getName() << endl;

    logger = Logger::getInstance("test.subtest");
    cout << "Logger name: " << logger.getName()
         << " Parent = " << logger.getParent().getName() << endl;

    logger = Logger::getInstance("test.subtest.a");
    cout << "Logger name: " << logger.getName()
         << " Parent = " << logger.getParent().getName() << endl;

    logger = Logger::getInstance("test.subtest.a.b.c");
    cout << "Logger name: " << logger.getName()
         << " Parent = " << logger.getParent().getName() << endl;

    logger = Logger::getInstance("test.subtest.a.b.c.d");
    cout << "Logger name: " << logger.getName()
         << " Parent = " << logger.getParent().getName() << endl;

    logger = Logger::getInstance("test.subtest.a.b.c");
    cout << "Logger name: " << logger.getName()
         << " Parent = " << logger.getParent().getName() << endl;

    logger = Logger::getInstance("test.subtest.a");
    cout << "Logger name: " << logger.getName()
         << " Parent = " << logger.getParent().getName() << endl;

    logger = Logger::getInstance("test.subtest");
    cout << "Logger name: " << logger.getName()
         << " Parent = " << logger.getParent().getName() << endl;


    Logger::shutdown();
    }
    cout << "Exiting main()..." << endl;
    return 0;
}

