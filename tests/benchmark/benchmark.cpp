/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/layout.h>
#include <log4cplus/appender.h>
#include <log4cplus/asyncappender.h>
#include <benchmark/benchmark.h>
#include <thread>
#include <cstdlib>
#include <iomanip>

using namespace log4cplus;

class NullWriterAppender : public Appender
{
public:
    NullWriterAppender() {}
    ~NullWriterAppender() { destructorImpl(); }

    void close() override {}

    void append(const spi::InternalLoggingEvent& event) override
    {
        // This gets called whenever there is a valid event for our appender.
    }
};

class benchmarker : public ::benchmark::Fixture
{
public:
    Logger m_logger;
    void SetupLogger()
    {
        m_logger = Logger::getInstance(LOG4CPLUS_TEXT("bench_logger"));

        m_logger.removeAllAppenders();
        m_logger.setAdditivity(false);
        m_logger.setLogLevel(INFO_LOG_LEVEL);

        SharedAppenderPtr nullWriter(new NullWriterAppender);
        nullWriter->setName(LOG4CPLUS_TEXT("NullWriterAppender"));
        nullWriter->setLayout(std::make_unique<PatternLayout>(LOG4CPLUS_TEXT("%m%n")));
        m_logger.addAppender(nullWriter);
    }

    void SetUp(const ::benchmark::State& state)
    {
        std::setlocale( LC_ALL, "" ); /* Set locale for C functions */
        std::locale::global(std::locale("")); /* set locale for C++ functions */
        SetupLogger();
    }

    void TearDown(const ::benchmark::State& state)
    {
    }

    static int threadCount()
    {
        int threadCount = -1;
        auto value = std::getenv("LOG4CPLUS_BENCHMARK_THREAD_COUNT");
        if (value)
            threadCount = std::stoi(value);
        if (threadCount <= 0)
            threadCount = std::thread::hardware_concurrency() - 2;
        return threadCount;
    }

    static double warmUpSeconds()
    {
        double milliseconds = 100;
        if (auto value = std::getenv("LOG4CPLUS_BENCHMARK_WARM_UP_MILLISECONDS"))
            milliseconds = std::stoi(value);
        return milliseconds / 1000;
    }

};

BENCHMARK_DEFINE_F(benchmarker, logDisabledTrace)(benchmark::State& state)
{
    m_logger.setLogLevel(DEBUG_LOG_LEVEL);
    for (auto _ : state)
    {
        LOG4CPLUS_TRACE( m_logger, LOG4CPLUS_TEXT("This is a static string to see what happens"));
    }
}
BENCHMARK_REGISTER_F(benchmarker, logDisabledTrace)->Name("Testing disabled logging request")->MinWarmUpTime(benchmarker::warmUpSeconds());
BENCHMARK_REGISTER_F(benchmarker, logDisabledTrace)->Name("Testing disabled logging request")->Threads(benchmarker::threadCount());

BENCHMARK_DEFINE_F(benchmarker, logStaticString)(benchmark::State& state)
{
    m_logger.setLogLevel(INFO_LOG_LEVEL);
    for (auto _ : state)
    {
        LOG4CPLUS_INFO( m_logger, LOG4CPLUS_TEXT("This is a static string to see what happens"));
    }
}
BENCHMARK_REGISTER_F(benchmarker, logStaticString)->Name("Logging static string");

BENCHMARK_DEFINE_F(benchmarker, logStaticStringFMT)(benchmark::State& state)
{
    for (auto _ : state)
    {
        LOG4CPLUS_INFO_FMT(m_logger, LOG4CPLUS_TEXT("This is a static string to see what happens"), 0);
    }
}
BENCHMARK_REGISTER_F(benchmarker, logStaticStringFMT)->Name("Logging static string with printf");

BENCHMARK_DEFINE_F(benchmarker, logIntValueFMT)(benchmark::State& state)
{
    int x = 0;
    for (auto _ : state)
    {
        LOG4CPLUS_INFO_FMT( m_logger, LOG4CPLUS_TEXT("Hello: msg number %d"), ++x);
    }
}
BENCHMARK_REGISTER_F(benchmarker, logIntValueFMT)->Name("Logging int value with printf");
BENCHMARK_REGISTER_F(benchmarker, logIntValueFMT)->Name("Logging int value with printf")->Threads(benchmarker::threadCount());

BENCHMARK_DEFINE_F(benchmarker, logIntPlusFloatValueFMT)(benchmark::State& state)
{
    int x = 0;
    for (auto _ : state)
    {
        auto f = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        LOG4CPLUS_INFO_FMT( m_logger, LOG4CPLUS_TEXT("Hello: msg number %d pseudo-random float %.3f"), ++x, f);
    }
}
BENCHMARK_REGISTER_F(benchmarker, logIntPlusFloatValueFMT)->Name("Logging int+float with printf");
BENCHMARK_REGISTER_F(benchmarker, logIntPlusFloatValueFMT)->Name("Logging int+float with printf")->Threads(benchmarker::threadCount());

BENCHMARK_DEFINE_F(benchmarker, logIntValueStream)(benchmark::State& state)
{
    int x = 0;
    for (auto _ : state)
    {
        LOG4CPLUS_INFO( m_logger, "Hello: msg number " << ++x);
    }
}
BENCHMARK_REGISTER_F(benchmarker, logIntValueStream)->Name("Logging int value with streams");
BENCHMARK_REGISTER_F(benchmarker, logIntValueStream)->Name("Logging int value with streams")->Threads(benchmarker::threadCount());

BENCHMARK_DEFINE_F(benchmarker, logIntPlusFloatStream)(benchmark::State& state)
{
    int x = 0;
    for (auto _ : state)
    {
        auto f = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        LOG4CPLUS_INFO( m_logger, "Hello: msg number " << ++x
            << " pseudo-random float " << std::setprecision(3) << std::fixed << f);
    }
}
BENCHMARK_REGISTER_F(benchmarker, logIntPlusFloatStream)->Name("Logging int+float with streams");
BENCHMARK_REGISTER_F(benchmarker, logIntPlusFloatStream)->Name("Logging int+float with streams")->Threads(benchmarker::threadCount());

template <class ...Args>
void logWithConversionPattern(benchmark::State& state, Args&&... args)
{
    auto args_tuple = std::make_tuple(std::move(args)...);
    log4cplus::tstring conversionPattern = std::get<0>(args_tuple);

    auto logger = Logger::getInstance( LOG4CPLUS_TEXT("bench_logger") );
    logger.getAppender(LOG4CPLUS_TEXT("NullWriterAppender"))->setLayout(std::make_unique<PatternLayout>(conversionPattern));

    int x = 0;
    for (auto _ : state)
    {
        LOG4CPLUS_INFO( logger, LOG4CPLUS_TEXT("Hello m_logger: msg number ") << ++x);
    }
}
BENCHMARK_CAPTURE(logWithConversionPattern, NoFormat, LOG4CPLUS_TEXT("%m%n"))->Name("NoFormat pattern: %m%n");
BENCHMARK_CAPTURE(logWithConversionPattern, DateOnly, LOG4CPLUS_TEXT("[%d] %m%n"))->Name("DateOnly pattern: [%d] %m%n");
BENCHMARK_CAPTURE(logWithConversionPattern, DateClassLevel, LOG4CPLUS_TEXT("[%d] [%c] [%p] %m%n"))->Name("DateClassLevel pattern: [%d] [%c] [%p] %m%n");

static void SetAsyncAppender(const benchmark::State& state)
{
    Logger logger = Logger::getInstance( LOG4CPLUS_TEXT("bench_logger") );
    logger.removeAllAppenders();
    logger.setAdditivity( false );
    logger.setLogLevel(INFO_LOG_LEVEL);

    SharedAppenderPtr nullWriter(new NullWriterAppender);
    nullWriter->setLayout(std::make_unique<PatternLayout>(LOG4CPLUS_TEXT("%m%n")));
    SharedAppenderPtr asyncAppender(new AsyncAppender(nullWriter, 5));
    logger.addAppender(asyncAppender);
}
BENCHMARK_REGISTER_F(benchmarker, logIntValueStream)->Name("Logging int value with streams to AsyncAppender")->Setup(SetAsyncAppender);
BENCHMARK_REGISTER_F(benchmarker, logIntValueStream)->Name("Logging int value with streams to AsyncAppender")->Threads(benchmarker::threadCount());

BENCHMARK_MAIN();

