#include <log4cplus/helpers/timehelper.h>
#include <iostream>

using namespace log4cplus;
using namespace log4cplus::helpers;


int
main()
{
    std::cout << "Entering main()..." << std::endl;
    try
    {
        Time time (0, 1234567);
        tstring str (
            time.getFormattedTime ("%s, %Q%%q%q %%Q %%q=%%%q%%;%%q, %%Q=%Q"));
        std::cout << str << std::endl;

        time = Time (0, 7);
        str = time.getFormattedTime ("%s, %Q%%q%q %%Q %%q=%%%q%%;%%q, %%Q=%Q");
        std::cout << str << std::endl;

        time = Time (0, 17);
        str = time.getFormattedTime ("%s, %Q%%q%q %%Q %%q=%%%q%%;%%q, %%Q=%Q");
        std::cout << str << std::endl;

        time = Time (0, 123);
        str = time.getFormattedTime ("%s, %Q%%q%q %%Q %%q=%%%q%%;%%q, %%Q=%Q");
        std::cout << str << std::endl;

        time = Time (0, 1234);
        str = time.getFormattedTime ("%s, %Q%%q%q %%Q %%q=%%%q%%;%%q, %%Q=%Q");
        std::cout << str << std::endl;

        time = Time (0, 12345);
        str = time.getFormattedTime ("%s, %Q%%q%q %%Q %%q=%%%q%%;%%q, %%Q=%Q");
        std::cout << str << std::endl;

        time = Time (0, 123456);
        str = time.getFormattedTime ("%s, %Q%%q%q %%Q %%q=%%%q%%;%%q, %%Q=%Q");
        std::cout << str << std::endl;

        time = Time (0, 0);
        str = time.getFormattedTime ("%s, %Q%%q%q %%Q %%q=%%%q%%;%%q, %%Q=%Q");
        std::cout << str << std::endl;
    }
    catch(...)
    {
        std::cout << "Exception..." << std::endl;
    }

    std::cout << "Exiting main()..." << std::endl;
    return 0;
}
