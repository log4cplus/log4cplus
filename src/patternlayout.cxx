// Module:  Log4CPLUS
// File:    patternlayout.cxx
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//
// $Log: not supported by cvs2svn $
// Revision 1.3  2003/04/03 01:17:30  tcsmith
// Changed to support the rename of Category to Logger and Priority to
// LogLevel.
//

#include <log4cplus/layout.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/loggingevent.h>

#include <stdlib.h>
#include <exception>

#include <ctime>
#ifdef TM_IN_SYS_TIME
#include <sys/time.h>
#endif

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::spi;


#define ESCAPE_CHAR LOG4CPLUS_TEXT('%')
#define MILLIS_PER_SEC 1000
#define BUFFER_SIZE 30


namespace log4cplus {
    namespace pattern {

        /**
         * This is used by PatternConverter class to inform them how to format
         * their output.
         */
        struct FormattingInfo {
            int  minLen;
            int  maxLen;
            bool leftAlign;
            FormattingInfo() { reset(); }

            void reset();
            void dump();
        };



        /**
         * This is the base class of all "Converter" classes that format a
         * field of InternalLoggingEvent objects.  In fact, the PatternLayout
         * class simply uses an array of PatternConverter objects to format
         * and append a logging event.
         */
        class PatternConverter {
        public:
            PatternConverter(const FormattingInfo& info);
            virtual ~PatternConverter() {}
            void formatAndAppend(log4cplus::tostream& output, 
                                 const InternalLoggingEvent& event);

        protected:
            virtual log4cplus::tstring convert(const InternalLoggingEvent& event) = 0;

        private:
            int minLen;
            int maxLen;
            bool leftAlign;
        };



        /**
         * This PatternConverter returns a constant string.
         */
        class LiteralPatternConverter : public PatternConverter {
        public:
            LiteralPatternConverter(const log4cplus::tstring& str);
            virtual log4cplus::tstring convert(const InternalLoggingEvent& event) {
                return str;
            }

        private:
            log4cplus::tstring str;
        };



        /**
         * This PatternConverter is used to format most of the "simple" fields
         * found in the InternalLoggingEvent object.
         */
        class BasicPatternConverter : public PatternConverter {
        public:
            enum Type { RELATIVE_TIME_CONVERTER,
                        THREAD_CONVERTER,
                        LOGLEVEL_CONVERTER,
                        NDC_CONVERTER,
                        MESSAGE_CONVERTER,
                        NEWLINE_CONVERTER,
                        FILE_CONVERTER,
                        LINE_CONVERTER,
                        FULL_LOCATION_CONVERTER };
            BasicPatternConverter(const FormattingInfo& info, Type type);
            virtual log4cplus::tstring convert(const InternalLoggingEvent& event);

        private:
            LogLevelManager& llmCache;
            const long CLOCKS_PER_MILLIS;
            Type type;
        };



        /**
         * This PatternConverter is used to format the Logger field found in
         * the InternalLoggingEvent object.
         */
        class LoggerPatternConverter : public PatternConverter {
        public:
            LoggerPatternConverter(const FormattingInfo& info, int precision);
            virtual log4cplus::tstring convert(const InternalLoggingEvent& event);

        private:
            int precision;
        };



        /**
         * This PatternConverter is used to format the timestamp field found in
         * the InternalLoggingEvent object.  It will be formatted according to
         * the specified "pattern".
         */
        class DatePatternConverter : public PatternConverter {
        public:
            DatePatternConverter(const FormattingInfo& info, 
                                 const log4cplus::tstring& pattern, 
                                 bool use_gmtime);
            virtual log4cplus::tstring convert(const InternalLoggingEvent& event);

        private:
            bool use_gmtime;
            log4cplus::tstring format;
        };



        /**
         * This class parses a "pattern" string into an array of
         * PatternConverter objects.
         * <p>
         * @see PatternLayout for the formatting of the "pattern" string.
         */
        class PatternParser {
        public:
            PatternParser(const log4cplus::tstring& pattern);
            std::vector<PatternConverter*> parse();

        private:
          // Types
            enum ParserState { LITERAL_STATE, 
                               CONVERTER_STATE,
                               MINUS_STATE,
                               DOT_STATE,
                               MIN_STATE,
                               MAX_STATE };

          // Methods
            log4cplus::tstring extractOption();
            int extractPrecisionOption();
            void finalizeConverter(char c);

          // Data
            log4cplus::tstring pattern;
            FormattingInfo formattingInfo;
            std::vector<PatternConverter*> list;
            ParserState state;
            int pos;
            log4cplus::tstring currentLiteral;
        };
    }
}
using namespace log4cplus::pattern;
typedef std::vector<log4cplus::pattern::PatternConverter*> PatternConverterList;



////////////////////////////////////////////////
// PatternConverter methods:
////////////////////////////////////////////////

void 
log4cplus::pattern::FormattingInfo::reset() {
    minLen = -1;
    maxLen = 0x7FFFFFFF;
    leftAlign = false;
}


void 
log4cplus::pattern::FormattingInfo::dump() {
    log4cplus::tostringstream buf;
    buf << LOG4CPLUS_TEXT("min=") << minLen
        << LOG4CPLUS_TEXT(", max=") << maxLen
        << LOG4CPLUS_TEXT(", leftAlign=")
	<< (buf ? LOG4CPLUS_TEXT("true") : LOG4CPLUS_TEXT("false"));
    getLogLog().debug(buf.str());
}




////////////////////////////////////////////////
// PatternConverter methods:
////////////////////////////////////////////////

log4cplus::pattern::PatternConverter::PatternConverter(const FormattingInfo& i)
{
    minLen = i.minLen;
    maxLen = i.maxLen;
    leftAlign = i.leftAlign;
}



void
log4cplus::pattern::PatternConverter::formatAndAppend
                     (log4cplus::tostream& output, const InternalLoggingEvent& event)
{
    log4cplus::tstring s = convert(event);
    int len = s.length();

    if(len > maxLen) {
        output << s.substr(len - maxLen);
    }
    else if(len < minLen) {
        if(leftAlign) {
            output << s;
            output << log4cplus::tstring(minLen - len, LOG4CPLUS_TEXT(' '));
        }
        else {
            output << log4cplus::tstring(minLen - len, LOG4CPLUS_TEXT(' '));
            output << s;
        }
    }
    else {
        output << s;
    }
}



////////////////////////////////////////////////
// LiteralPatternConverter methods:
////////////////////////////////////////////////

log4cplus::pattern::LiteralPatternConverter::LiteralPatternConverter
                                                      (const log4cplus::tstring& str)
: PatternConverter(FormattingInfo()),
  str(str)
{
}



////////////////////////////////////////////////
// BasicPatternConverter methods:
////////////////////////////////////////////////

log4cplus::pattern::BasicPatternConverter::BasicPatternConverter
                                        (const FormattingInfo& info, Type type)
: PatternConverter(info),
  llmCache(getLogLevelManager()),
  CLOCKS_PER_MILLIS(CLOCKS_PER_SEC / MILLIS_PER_SEC),
  type(type)
{
    if(   type == RELATIVE_TIME_CONVERTER
       && CLOCKS_PER_SEC < MILLIS_PER_SEC) 
    {
        getLogLog().error
                 (LOG4CPLUS_TEXT("RELATIVE_TIME will not display correctly on this platform"));
    }
}



log4cplus::tstring
log4cplus::pattern::BasicPatternConverter::convert
                                            (const InternalLoggingEvent& event)
{
    switch(type) {
    case LOGLEVEL_CONVERTER: return llmCache.toString(event.ll);
    case NDC_CONVERTER:      return event.ndc;
    case MESSAGE_CONVERTER:  return event.message;
    case NEWLINE_CONVERTER:  return LOG4CPLUS_TEXT("\n");
    case FILE_CONVERTER:     return (event.file ? event.file : log4cplus::tstring());

    case RELATIVE_TIME_CONVERTER:
        {
	    log4cplus::tostringstream buf;
            buf << (event.clock_ticks / CLOCKS_PER_MILLIS);
            return buf.str();
        }

    case THREAD_CONVERTER:        
        {
	    log4cplus::tostringstream buf;
            buf << event.thread;
            return buf.str();
        }

    case LINE_CONVERTER:
        {
            if(event.line != -1) {
	       log4cplus::tostringstream buf;
                buf << event.line;
                return buf.str();
            }
            else {
                return log4cplus::tstring();
            }
        }

    case FULL_LOCATION_CONVERTER:
        {
            if(event.file != 0) {
		log4cplus::tostringstream buf;
                buf << event.file << LOG4CPLUS_TEXT(':') << event.line;
                return buf.str();
            }
            else {
                return LOG4CPLUS_TEXT(":");
            }
        }
    }

    return LOG4CPLUS_TEXT("INTERNAL LOG4CPLUS ERROR");
}



////////////////////////////////////////////////
// LoggerPatternConverter methods:
////////////////////////////////////////////////

log4cplus::pattern::LoggerPatternConverter::LoggerPatternConverter
                                    (const FormattingInfo& info, int precision)
: PatternConverter(info),
  precision(precision)
{
}



log4cplus::tstring
log4cplus::pattern::LoggerPatternConverter::convert
                                            (const InternalLoggingEvent& event)
{
    const log4cplus::tstring& name = event.loggerName;
    if (precision <= 0) {
        return name;
    }
    else {
        int len = name.length();

        // We substract 1 from 'len' when assigning to 'end' to avoid out of
        // bounds exception in return r.substring(end+1, len). This can happen
        // if precision is 1 and the logger name ends with a dot. 
        int end = len - 1;
        for(int i=precision; i>0; --i) {
            end = name.rfind(LOG4CPLUS_TEXT('.'), end - 1);
            if(end == tstring::npos) {
                return name;
            }
        }
        return name.substr(end + 1);
    }
}



////////////////////////////////////////////////
// DatePatternConverter methods:
////////////////////////////////////////////////


log4cplus::pattern::DatePatternConverter::DatePatternConverter
                                               (const FormattingInfo& info, 
                                                const log4cplus::tstring& pattern, 
                                                bool use_gmtime)
: PatternConverter(info),
  format(pattern),
  use_gmtime(use_gmtime)
{
}



log4cplus::tstring 
log4cplus::pattern::DatePatternConverter::convert
                                            (const InternalLoggingEvent& event)
{
    char buffer[BUFFER_SIZE];
    struct tm* time;
    if(use_gmtime) {
        time = gmtime(&event.timestamp);
    }
    else {
        time = localtime(&event.timestamp);
    }

    int result = strftime(buffer, BUFFER_SIZE, format.c_str(), time);
    if(result > 0) {
        return log4cplus::tstring(buffer, result);
    }
    else {
        return LOG4CPLUS_TEXT("INVALID DATE PATTERN");
    }
}




////////////////////////////////////////////////
// PatternParser methods:
////////////////////////////////////////////////

log4cplus::pattern::PatternParser::PatternParser(const log4cplus::tstring& pattern) 
: pattern(pattern), 
  state(LITERAL_STATE),
  pos(0)
{
}



log4cplus::tstring 
log4cplus::pattern::PatternParser::extractOption() 
{
    if (   (pos < pattern.length()) 
        && (pattern.at(pos) == LOG4CPLUS_TEXT('{'))) 
    {
        int end = pattern.find_first_of(LOG4CPLUS_TEXT('}'), pos);
        if (end > pos) {
            log4cplus::tstring r = pattern.substr(pos + 1, end - pos - 1);
            pos = end + 1;
            return r;
        }
    }

    return LOG4CPLUS_TEXT("");
}


int 
log4cplus::pattern::PatternParser::extractPrecisionOption() 
{
    log4cplus::tstring opt = extractOption();
    int r = 0;
    if(opt.length() > 0) {
        r = atoi(opt.c_str());
    }
    return r;
}



PatternConverterList
log4cplus::pattern::PatternParser::parse() 
{
    char c;
    pos = 0;
    while(pos < pattern.length()) {
        c = pattern.at(pos++);
        switch (state) {
        case LITERAL_STATE :
            // In literal state, the last char is always a literal.
            if(pos == pattern.length()) {
                currentLiteral += c;
                continue;
            }
            if(c == ESCAPE_CHAR) {
                // peek at the next char. 
                switch (pattern.at(pos)) {
                case ESCAPE_CHAR:
                    currentLiteral += c;
                    pos++; // move pointer
                    break;
                case LOG4CPLUS_TEXT('n'):
                    currentLiteral += LOG4CPLUS_TEXT('\n');
                    pos++; // move pointer
                    break;
                default:
                    if(currentLiteral.length() != 0) {
                        list.push_back
                             (new LiteralPatternConverter(currentLiteral));
                        //getLogLog().debug("Parsed LITERAL converter: \"" 
                        //                  +currentLiteral+"\".");
                    }
                    currentLiteral.resize(0);
                    currentLiteral += c; // append %
                    state = CONVERTER_STATE;
                    formattingInfo.reset();
                }
            }
            else {
                currentLiteral += c;
            }
            break;

        case CONVERTER_STATE:
            currentLiteral += c;
            switch (c) {
            case LOG4CPLUS_TEXT('-'):
                formattingInfo.leftAlign = true;
                break;
            case LOG4CPLUS_TEXT('.'):
                state = DOT_STATE;
                break;
            default:
                if(c >= LOG4CPLUS_TEXT('0') && c <= LOG4CPLUS_TEXT('9')) {
                    formattingInfo.minLen = c - LOG4CPLUS_TEXT('0');
                    state = MIN_STATE;
                }
                else {
                    finalizeConverter(c);
                }
            } // switch
            break;

        case MIN_STATE:
            currentLiteral += c;
            if (c >= LOG4CPLUS_TEXT('0') && c <= LOG4CPLUS_TEXT('9')) {
                formattingInfo.minLen = formattingInfo.minLen * 10 + (c - LOG4CPLUS_TEXT('0'));
            }
            else if(c == LOG4CPLUS_TEXT('.')) {
                state = DOT_STATE;
            }
            else {
                finalizeConverter(c);
            }
            break;

        case DOT_STATE:
            currentLiteral += c;
            if(c >= LOG4CPLUS_TEXT('0') && c <= LOG4CPLUS_TEXT('9')) {
                formattingInfo.maxLen = c - LOG4CPLUS_TEXT('0');
                state = MAX_STATE;
            }
            else {
		log4cplus::tostringstream buf;
                buf << LOG4CPLUS_TEXT("Error occured in position ")
                    << pos
                    << LOG4CPLUS_TEXT(".\n Was expecting digit, instead got char \"")
                    << c
                    << LOG4CPLUS_TEXT("\".");
                getLogLog().error(buf.str());
                state = LITERAL_STATE;
            }
            break;

         case MAX_STATE:
            currentLiteral += c;
            if (c >= LOG4CPLUS_TEXT('0') && c <= LOG4CPLUS_TEXT('9'))
                formattingInfo.maxLen = formattingInfo.maxLen * 10 + (c - LOG4CPLUS_TEXT('0'));
            else {
                finalizeConverter(c);
                state = LITERAL_STATE;
            }
            break;
        } // end switch
    } // end while

    if(currentLiteral.length() != 0) {
        list.push_back(new LiteralPatternConverter(currentLiteral));
      //getLogLog().debug("Parsed LITERAL converter: \""+currentLiteral+"\".");
    }

    return list;
}



void
log4cplus::pattern::PatternParser::finalizeConverter(char c) 
{
    PatternConverter* pc = 0;
    switch (c) {
        case LOG4CPLUS_TEXT('c'):
            pc = new LoggerPatternConverter(formattingInfo, 
                                            extractPrecisionOption());
            getLogLog().debug( LOG4CPLUS_TEXT("LOGGER converter.") );
            formattingInfo.dump();      
            break;

        case LOG4CPLUS_TEXT('d'):
        case LOG4CPLUS_TEXT('D'):
            {
                log4cplus::tstring dOpt = extractOption();
                if(dOpt.length() == 0) {
                    dOpt = LOG4CPLUS_TEXT("%Y-%m-%d %H:%M:%S");
                }
                bool use_gmtime = c == LOG4CPLUS_TEXT('d');
                pc = new DatePatternConverter(formattingInfo, dOpt, use_gmtime);
                //if(use_gmtime) {
                //    getLogLog().debug("GMT DATE converter.");
                //}
                //else {
                //    getLogLog().debug("LOCAL DATE converter.");
                //}
                //formattingInfo.dump();      
            }
            break;

        case LOG4CPLUS_TEXT('F'):
            pc = new BasicPatternConverter
                          (formattingInfo, 
                           BasicPatternConverter::FILE_CONVERTER);
            //getLogLog().debug("FILE NAME converter.");
            //formattingInfo.dump();      
            break;

        case LOG4CPLUS_TEXT('l'):
            pc = new BasicPatternConverter
                          (formattingInfo, 
                           BasicPatternConverter::FULL_LOCATION_CONVERTER);
            //getLogLog().debug("FULL LOCATION converter.");
            //formattingInfo.dump();      
            break;

        case LOG4CPLUS_TEXT('L'):
            pc = new BasicPatternConverter
                          (formattingInfo, 
                           BasicPatternConverter::LINE_CONVERTER);
            //getLogLog().debug("LINE NUMBER converter.");
            //formattingInfo.dump();      
            break;

        case LOG4CPLUS_TEXT('m'):
            pc = new BasicPatternConverter
                          (formattingInfo, 
                           BasicPatternConverter::MESSAGE_CONVERTER);
            //getLogLog().debug("MESSAGE converter.");
            //formattingInfo.dump();      
            break;

        case LOG4CPLUS_TEXT('n'):
            pc = new BasicPatternConverter
                          (formattingInfo, 
                           BasicPatternConverter::NEWLINE_CONVERTER);
            //getLogLog().debug("MESSAGE converter.");
            //formattingInfo.dump();      
            break;

        case LOG4CPLUS_TEXT('p'):
            pc = new BasicPatternConverter
                          (formattingInfo, 
                           BasicPatternConverter::LOGLEVEL_CONVERTER);
            //getLogLog().debug("LOGLEVEL converter.");
            //formattingInfo.dump();
            break;

        case LOG4CPLUS_TEXT('r'):
            pc = new BasicPatternConverter
                          (formattingInfo, 
                           BasicPatternConverter::RELATIVE_TIME_CONVERTER);
            //getLogLog().debug("RELATIVE time converter.");
            //formattingInfo.dump();
            break;

        case LOG4CPLUS_TEXT('t'):
            pc = new BasicPatternConverter
                          (formattingInfo, 
                           BasicPatternConverter::THREAD_CONVERTER);
            //getLogLog().debug("THREAD converter.");
            //formattingInfo.dump();      
            break;

        case LOG4CPLUS_TEXT('x'):
            pc = new BasicPatternConverter
                          (formattingInfo, 
                           BasicPatternConverter::NDC_CONVERTER);
            //getLogLog().debug("NDC converter.");      
            break;

        default:
	    log4cplus::tostringstream buf;
            buf << LOG4CPLUS_TEXT("Unexpected char [")
                << c
                << LOG4CPLUS_TEXT("] at position ")
                << pos
                << LOG4CPLUS_TEXT(" in conversion patterrn.");
            getLogLog().error(buf.str());
            pc = new LiteralPatternConverter(currentLiteral);
    }

    currentLiteral.resize(0);
    list.push_back(pc);
    state = LITERAL_STATE;
    formattingInfo.reset();
}





////////////////////////////////////////////////
// PatternLayout methods:
////////////////////////////////////////////////

PatternLayout::PatternLayout(const log4cplus::tstring& pattern)
{
    init(pattern);
}


PatternLayout::PatternLayout(log4cplus::helpers::Properties properties)
{
    if(!properties.exists( LOG4CPLUS_TEXT("Pattern") )) {
        throw std::runtime_error("Pattern not specified in properties");
    }

    init(properties.getProperty( LOG4CPLUS_TEXT("Pattern") ));
}


void
PatternLayout::init(const log4cplus::tstring& pattern)
{
    this->pattern = pattern;
    this->parsedPattern = PatternParser(pattern).parse();

    // Let's validate that our parser didn't give us any NULLs.  If it did,
    // we will convert them to a valid PatternConverter that does nothing so
    // at least we don't core.
    for(PatternConverterList::iterator it=parsedPattern.begin(); 
        it!=parsedPattern.end(); 
        ++it)
    {
        if( (*it) == 0 ) {
            getLogLog().error(LOG4CPLUS_TEXT("Parsed Pattern created a NULL PatternConverter"));
            (*it) = new LiteralPatternConverter( LOG4CPLUS_TEXT("") );
        }
    }
    if(parsedPattern.size() == 0) {
        getLogLog().warn(LOG4CPLUS_TEXT("PatternLayout pattern is empty.  Using default..."));
        parsedPattern.push_back
           (new BasicPatternConverter(FormattingInfo(), 
                                      BasicPatternConverter::MESSAGE_CONVERTER));
    }
}



PatternLayout::~PatternLayout()
{
    for(PatternConverterList::iterator it=parsedPattern.begin(); 
        it!=parsedPattern.end(); 
        ++it)
    {
        delete (*it);
    }
}



void
PatternLayout::formatAndAppend(log4cplus::tostream& output, 
                               const InternalLoggingEvent& event)
{
    for(PatternConverterList::iterator it=parsedPattern.begin(); 
        it!=parsedPattern.end(); 
        ++it)
    {
        (*it)->formatAndAppend(output, event);
    }
}


