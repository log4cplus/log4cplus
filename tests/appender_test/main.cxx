

#include <exception>
#include <iostream>
#include <string>
#include "log4cplus/category.h"
#include "log4cplus/consoleappender.h"
#include "log4cplus/helpers/appenderattachableimpl.h"
#include "log4cplus/helpers/loglog.h"
#include "log4cplus/helpers/pointer.h"
#include "log4cplus/spi/loggingevent.h"

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::spi;

void
printAppenderList(SharedAppenderPtrList list)
{
   cout << "List size: " << list.size() << endl;
   for(SharedAppenderPtrList::iterator it=list.begin(); it!=list.end(); ++it) {
      cout << "Loop Body: Appender name = " << (*it)->getName() << endl;
   }
}


int
main()
{
    log4cplus::helpers::getLogLog().setInternalDebugging(true);
    {
        AppenderAttachableImpl aai;
        try {
            SharedObjectPtr<Appender> append_1(new ConsoleAppender());
            append_1->setName("First");

            SharedObjectPtr<Appender> append_2(new ConsoleAppender());
            append_2->setName("Second");

            InternalLoggingEvent event(Category::getInstance("test").getName(),
                                       Priority(Priority::DEBUG_PRI),
                                       "This is a test...", NULL, 0);

            aai.addAppender(append_1);
            aai.addAppender(append_2);
            aai.addAppender(append_1);
            aai.addAppender(append_2);
            printAppenderList(aai.getAllAppenders());

            aai.removeAppender(append_2);
            printAppenderList(aai.getAllAppenders());

            aai.removeAppender("First");
            printAppenderList(aai.getAllAppenders());

            aai.addAppender(append_1);
            aai.addAppender(append_2);
            aai.addAppender(append_1);
            aai.addAppender(append_2);
            cout << "Should be First: " << aai.getAppender("First")->getName() << endl;
            cout << "Should be Second: " << aai.getAppender("Second")->getName() << endl << endl;
            append_1->doAppend(event);
            append_2->doAppend(event);
        }
        catch(std::exception& e) {
            cout << "**** Exception occured: " << e.what() << endl;
        }

//      cout << "*** Calling close()..." << endl;
//      append_2->close();
//      cout << "*** Done calling close()..." << endl;
    }

    cout << "Exiting main()..." << endl;
    return 0;
}

