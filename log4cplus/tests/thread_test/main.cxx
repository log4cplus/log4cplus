
#include <log4cplus/category.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/ndc.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/threads.h>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>


using namespace std;
using namespace log4cplus;
using namespace log4cplus::thread;


#define MILLIS_TO_NANOS 1000
#define NUM_THREADS 4
#define NUM_LOOPS 10

class SlowObject {
public:
    SlowObject() : mutex( LOG4CPLUS_MUTEX_CREATE ), cat(Category::getInstance("SlowObject")) {}

    void doSomething() {
        cat.debug("Entering doSomething()...");
        LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
            cat.info("Actually doing something...");
            sleep(0, 75 * MILLIS_TO_NANOS);
            cat.info("Actually doing something...DONE");
        LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
        yield();
        cat.debug("Exiting doSomething()...");
    }

private:
    LOG4CPLUS_MUTEX_PTR_DECLARE mutex;
    Category cat;
};
SlowObject *global;


class TestThread : public AbstractThread {
public:
    TestThread(std::string n) 
     : name(n), cat(Category::getInstance("test.TestThread")) {}

    virtual void run();

private:
   Category cat;
   std::string name;
};


int
main() 
{
    auto_ptr<SlowObject> globalContainer(new SlowObject());
    global = globalContainer.get();

    try {
        log4cplus::helpers::getLogLog().setInternalDebugging(true);
        Category cat = Category::getInstance("main");
        Category::getRoot().setPriority(Priority::INFO_PRI);
        const Priority *p = cat.getPriority();
        if(p == NULL) {
            cout << "main Priority: is NOT set." << endl;
        }
        else {
            cout << "main Priority: " << *p << endl;
        }

        helpers::SharedObjectPtr<Appender> append_1(new ConsoleAppender());
        Category::getRoot().addAppender(append_1);
        append_1->setName("cout");

        log4cplus::helpers::SharedObjectPtr<TestThread> threads[NUM_THREADS];
        int i = 0;
        for(i=0; i<NUM_THREADS; ++i) {
            stringstream s;
            s << "Thread-" << i;
            threads[i] = new TestThread(s.str());
        }

        for(i=0; i<NUM_THREADS; ++i) {
            threads[i]->start();
        }
        cat.debug("All Threads started...");

        for(i=0; i<NUM_THREADS; ++i) {
            while(threads[i]->isRunning()) {
                sleep(0, 200 * MILLIS_TO_NANOS);
            }
        }
        cat.debug("Exiting main()...");
    }
    catch(std::exception &e) {
        Category::getRoot().fatal("main()- Exception occured: " + std::string(e.what()));
    }
    catch(...) {
        Category::getRoot().fatal("main()- Exception occured");
    }

    return 0;
}


void
TestThread::run()
{
    try {
        cat.warn(name + " TestThread.run()- Starting...");
        NDC& ndc = getNDC();
        NDCContextCreator _first_ndc(name);
        cat.debug("Entering Run()...");
        for(int i=0; i<NUM_LOOPS; ++i) {
            NDCContextCreator _ndc("loop");
            global->doSomething();
            cat.info("Exiting loop...");
        }
        cat.debug("Exiting Run()...");

        ndc.remove();
    }
    catch(exception& e) {
        cat.fatal(std::string("TestThread.run()- Exception occurred: ") + e.what());
    }
    catch(...) {
        cat.fatal("TestThread.run()- Exception occurred!!");
    }
    cat.warn(name + " TestThread.run()- Finished");
} // end "run"

