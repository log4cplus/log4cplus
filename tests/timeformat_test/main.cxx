#include <log4cplus/helpers/timehelper.h>
#include <log4cplus/streams.h>
#include <iostream>

using namespace log4cplus;
using namespace log4cplus::helpers;


log4cplus::tchar const fmtstr[] =
    LOG4CPLUS_TEXT("%s, %Q%%q%q %%Q %%q=%%%q%%;%%q, %%Q=%Q");


int
main()
{
    std::cout << "Entering main()..." << std::endl;
    log4cplus::initialize ();
    try
    {
        Time time;
        log4cplus::tstring str;

        time = Time::gettimeofday ();
        str = time.getFormattedTime (fmtstr);
        log4cplus::tcout << LOG4CPLUS_TEXT ("now: ") << str << std::endl;

        time = Time (0, 7);
        str = time.getFormattedTime (fmtstr);
        log4cplus::tcout << str << std::endl;

        time = Time (0, 17);
        str = time.getFormattedTime (fmtstr);
        log4cplus::tcout << str << std::endl;

        time = Time (0, 123);
        str = time.getFormattedTime (fmtstr);
        log4cplus::tcout << str << std::endl;

        time = Time (0, 1234);
        str = time.getFormattedTime (fmtstr);
        log4cplus::tcout << str << std::endl;

        time = Time (0, 12345);
        str = time.getFormattedTime (fmtstr);
        log4cplus::tcout << str << std::endl;

        time = Time (0, 123456);
        str = time.getFormattedTime (fmtstr);
        log4cplus::tcout << str << std::endl;

        time = Time (0, 0);
        str = time.getFormattedTime (fmtstr);
        log4cplus::tcout << str << std::endl;
    }
    catch(std::exception const & e)
    {
        std::cout << "Exception: " << e.what () << std::endl;
    }
    catch(...)
    {
        std::cout << "Exception..." << std::endl;
    }

    std::cout << "Exiting main()..." << std::endl;
    return 0;
}
