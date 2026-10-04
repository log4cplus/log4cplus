#include <log4cplus/appender.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/timehelper.h>
#include <log4cplus/initializer.h>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <dlfcn.h>
#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char **argv)
{
    if (argc != 2) {
        std::cerr << "usage: repro CONFIG < epoch-and-message-lines\n";
        return 2;
    }
    auto set_epoch = reinterpret_cast<void (*)(long long)>(
        dlsym(RTLD_DEFAULT, "issue588_set_epoch"));
    if (!set_epoch) {
        std::cerr << "Load fakeclock.so through LD_PRELOAD\n";
        return 2;
    }
    log4cplus::Initializer initializer;
    log4cplus::PropertyConfigurator::doConfigure(
        LOG4CPLUS_STRING_TO_TSTRING(argv[1]));
    log4cplus::Logger logger = log4cplus::Logger::getInstance(
        LOG4CPLUS_TEXT("Hope"));
    auto appenders = log4cplus::Logger::getRoot().getAllAppenders();
    if (appenders.size() != 1) {
        std::cerr << "Expected one configured root appender\n";
        return 2;
    }
    std::cout << "READY\n" << std::flush;
    long long epoch;
    std::string message;
    while (std::cin >> epoch) {
        std::getline(std::cin >> std::ws, message);
        set_epoch(epoch);
        if (log4cplus::helpers::to_time_t(log4cplus::helpers::now()) != epoch) {
            std::cerr << "Controlled clock was not applied\n";
            return 2;
        }
        LOG4CPLUS_INFO(logger, LOG4CPLUS_STRING_TO_TSTRING(message));
        for (auto const &appender : appenders)
            appender->waitToFinishAsyncLogging();
        std::cout << "LOGGED " << message << '\n' << std::flush;
    }
    // A normal return performs normal log4cplus shutdown through Initializer.
    return 0;
}
