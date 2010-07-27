// Module:  Log4CPLUS
// File:    patternlayout.cxx
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright 2001-2010 Tad E. Smith
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <log4cplus/layout.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/timehelper.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/helpers/socket.h>
#include <log4cplus/spi/loggingevent.h>

#ifdef LOG4CPLUS_HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <exception>

#ifdef LOG4CPLUS_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef LOG4CPLUS_HAVE_UNISTD_H
#include <unistd.h>
#endif


namespace
{

static
#if defined (_WIN32)
DWORD
get_process_id ()
{
    return GetCurrentProcessId ();
}

#elif defined (LOG4CPLUS_HAVE_GETPID)
pid_t
get_process_id ()
{
    return getpid ();
}

#else
int
get_process_id ()
{
    return 0; 
}

#endif


static
log4cplus::tstring
get_basename (const log4cplus::tstring& filename)
{
#if defined(_WIN32)
    log4cplus::tchar const dir_sep(LOG4CPLUS_TEXT('\\'));
#else
    log4cplus::tchar const dir_sep(LOG4CPLUS_TEXT('/'));
#endif

    log4cplus::tstring::size_type pos = filename.rfind(dir_sep);
    if (pos != log4cplus::tstring::npos)
        return filename.substr(pos+1);
    else
        return filename;
}


} // namespace


using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::spi;


#define ESCAPE_CHAR LOG4CPLUS_TEXT('%')


namespace log4cplus {
    namespace pattern {

        /**
         * This is used by PatternConverter class to inform them how to format
         * their output.
         */
        struct FormattingInfo {
            int minLen;
            size_t maxLen;
            bool leftAlign;
            FormattingInfo() { reset(); }

            void reset();
            void dump(log4cplus::helpers::LogLog&);
        };



        /**
         * This is the base class of all "Converter" classes that format a
         * field of InternalLoggingEvent objects.  In fact, the PatternLayout
         * class simply uses an array of PatternConverter objects to format
         * and append a logging event.
         */
        class PatternConverter : protected log4cplus::helpers::LogLogUser {
        public:
            PatternConverter(const FormattingInfo& info);
            virtual ~PatternConverter() {}
            void formatAndAppend(log4cplus::tostream& output, 
                                 const InternalLoggingEvent& event);

        protected:
            virtual log4cplus::tstring convert(const InternalLoggingEvent& event) = 0;

        private:
            int minLen;
            size_t maxLen;
            bool leftAlign;
        };



        /**
         * This PatternConverter returns a constant string.
         */
        class LiteralPatternConverter : public PatternConverter {
        public:
            LiteralPatternConverter(const log4cplus::tstring& str);
            virtual log4cplus::tstring convert(const InternalLoggingEvent&) {
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
            enum Type { THREAD_CONVERTER,
                        PROCESS_CONVERTER,
                        LOGLEVEL_CONVERTER,
                        NDC_CONVERTER,
                        MESSAGE_CONVERTER,
                        NEWLINE_CONVERTER,
                        BASENAME_CONVERTER,
                        FILE_CONVERTER,
                        LINE_CONVERTER,
                        FULL_LOCATION_CONVERTER };
            BasicPatternConverter(const FormattingInfo& info, Type type);
            virtual log4cplus::tstring convert(const InternalLoggingEvent& event);

        private:
          // Disable copy
            BasicPatternConverter(const BasicPatternConverter&);
            BasicPatternConverter& operator=(BasicPatternConverter&);
            
            LogLevelManager& llmCache;
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
         * This PatternConverter is used to format the hostname field.
         */
        class HostnamePatternConverter : public PatternConverter {
        public:
            HostnamePatternConverter(const FormattingInfo& info, bool fqdn);
            virtual log4cplus::tstring convert(const InternalLoggingEvent& event);

        private:
            log4cplus::tstring hostname_;
        };



        /**
         * This PatternConverter is used to format the NDC field found in
         * the InternalLoggingEvent object, optionally limited to
         * \c precision levels (using space to separate levels).
         */
        class NDCPatternConverter : public PatternConverter {
        public:
            NDCPatternConverter(const FormattingInfo& info, int precision);
            virtual log4cplus::tstring convert(const InternalLoggingEvent& event);

        private:
            int precision;
        };



        /**
         * This class parses a "pattern" string into an array of
         * PatternConverter objects.
         * <p>
         * @see PatternLayout for the formatting of the "pattern" string.
         */
        class PatternParser : protected log4cplus::helpers::LogLogUser {
        public:
            PatternParser(const log4cplus::tstring& pattern, unsigned ndcMaxDepth);
            std::vector<PatternConverter*> parse();

        private:
          // Types
            enum ParserState { LITERAL_STATE, 
                               CONVERTER_STATE,
                               DOT_STATE,
                               MIN_STATE,
                               MAX_STATE };

          // Methods
            log4cplus::tstring extractOption();
            int extractPrecisionOption();
            void finalizeConverter(log4cplus::tchar c);

          // Data
            log4cplus::tstring pattern;
            FormattingInfo formattingInfo;
            std::vector<PatternConverter*> list;
            ParserState state;
            tstring::size_type pos;
            log4cplus::tstring currentLiteral;
            unsigned ndcMaxDepth;
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
log4cplus::pattern::FormattingInfo::dump(log4cplus::helpers::LogLog& loglog) {
    log4cplus::tostringstream buf;
    buf << LOG4CPLUS_TEXT("min=") << minLen
        << LOG4CPLUS_TEXT(", max=") << maxLen
        << LOG4CPLUS_TEXT(", leftAlign=")
        << (leftAlign ? LOG4CPLUS_TEXT("true") : LOG4CPLUS_TEXT("false"));
    loglog.debug(buf.str());
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
    size_t len = s.length();

    if(len > maxLen) {
        output << s.substr(len - maxLen);
    }
    else if(static_cast<int>(len) < minLen) {
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
                                                      (const log4cplus::tstring& str_)
: PatternConverter(FormattingInfo()),
  str(str_)
{
}



////////////////////////////////////////////////
// BasicPatternConverter methods:
////////////////////////////////////////////////

log4cplus::pattern::BasicPatternConverter::BasicPatternConverter
                                        (const FormattingInfo& info, Type type_)
: PatternConverter(info),
  llmCache(getLogLevelManager()),
  type(type_)
{
}



log4cplus::tstring
log4cplus::pattern::BasicPatternConverter::convert
                                            (const InternalLoggingEvent& event)
{
    switch(type) {
    case LOGLEVEL_CONVERTER: return llmCache.toString(event.getLogLevel());
    case NDC_CONVERTER:      return event.getNDC();
    case MESSAGE_CONVERTER:  return event.getMessage();
    case NEWLINE_CONVERTER:  return LOG4CPLUS_TEXT("\n");
    case BASENAME_CONVERTER: return get_basename(event.getFile());
    case FILE_CONVERTER:     return event.getFile();
    case THREAD_CONVERTER:   return event.getThread(); 
    case PROCESS_CONVERTER:  return convertIntegerToString(get_process_id ()); 

    case LINE_CONVERTER:
        {
            int line = event.getLine();
            if(line != -1) {
                return convertIntegerToString(line);
            }
            else {
                return log4cplus::tstring();
            }
        }

    case FULL_LOCATION_CONVERTER:
        {
            tstring const & filename = event.getFile();
            if(! filename.empty ()) {
                return   filename 
                       + LOG4CPLUS_TEXT(":") 
                       + convertIntegerToString(event.getLine());
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
                                    (const FormattingInfo& info, int precision_)
: PatternConverter(info),
  precision(precision_)
{
}



log4cplus::tstring
log4cplus::pattern::LoggerPatternConverter::convert
                                            (const InternalLoggingEvent& event)
{
    const log4cplus::tstring& name = event.getLoggerName();
    if (precision <= 0) {
        return name;
    }
    else {
        size_t len = name.length();

        // We substract 1 from 'len' when assigning to 'end' to avoid out of
        // bounds exception in return r.substring(end+1, len). This can happen
        // if precision is 1 and the logger name ends with a dot. 
        tstring::size_type end = len - 1;
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
                                                bool use_gmtime_)
: PatternConverter(info),
  use_gmtime(use_gmtime_),
  format(pattern)
{
}



log4cplus::tstring 
log4cplus::pattern::DatePatternConverter::convert
                                            (const InternalLoggingEvent& event)
{
    return event.getTimestamp().getFormattedTime(format, use_gmtime);
}




////////////////////////////////////////////////
// HostnamePatternConverter methods:
////////////////////////////////////////////////

log4cplus::pattern::HostnamePatternConverter::HostnamePatternConverter (
    const FormattingInfo& info, bool fqdn)
    : PatternConverter(info)
    , hostname_ (helpers::getHostname (fqdn))
{ }


log4cplus::tstring
log4cplus::pattern::HostnamePatternConverter::convert (
    const InternalLoggingEvent &)
{
    return hostname_;
}



////////////////////////////////////////////////
// NDCPatternConverter methods:
////////////////////////////////////////////////

log4cplus::pattern::NDCPatternConverter::NDCPatternConverter (
    const FormattingInfo& info, int precision_)
    : PatternConverter(info)
    , precision(precision_)
{ }


log4cplus::tstring
log4cplus::pattern::NDCPatternConverter::convert (
    const InternalLoggingEvent& event)
{
    const log4cplus::tstring& text = event.getNDC();
    if (precision <= 0)
        return text;
    else
    {
        tstring::size_type p = text.find(LOG4CPLUS_TEXT(' '));
        for (int i = 1; i < precision && p != tstring::npos; ++i)
            p = text.find(LOG4CPLUS_TEXT(' '), p + 1);

        return text.substr(0, p);
    }
}



////////////////////////////////////////////////
// PatternParser methods:
////////////////////////////////////////////////

log4cplus::pattern::PatternParser::PatternParser(
    const log4cplus::tstring& pattern_, unsigned ndcMaxDepth_)
    : pattern(pattern_)
    , state(LITERAL_STATE)
    , pos(0)
    , ndcMaxDepth (ndcMaxDepth_)
{
}



log4cplus::tstring 
log4cplus::pattern::PatternParser::extractOption() 
{
    if (   (pos < pattern.length()) 
        && (pattern[pos] == LOG4CPLUS_TEXT('{'))) 
    {
        tstring::size_type end = pattern.find_first_of(LOG4CPLUS_TEXT('}'), pos);
        if (end != tstring::npos) {
            log4cplus::tstring r = pattern.substr(pos + 1, end - pos - 1);
            pos = end + 1;
            return r;
        }
        else {
            log4cplus::tostringstream buf;
            buf << LOG4CPLUS_TEXT("No matching '}' found in conversion pattern string \"")
                << pattern
                << LOG4CPLUS_TEXT("\"");
            getLogLog().error(buf.str());
            pos = pattern.length();
        }
    }

    return LOG4CPLUS_TEXT("");
}


int 
log4cplus::pattern::PatternParser::extractPrecisionOption() 
{
    log4cplus::tstring opt = extractOption();
    int r = 0;
    if(! opt.empty ()) {
        r = std::atoi(LOG4CPLUS_TSTRING_TO_STRING(opt).c_str());
    }
    return r;
}



PatternConverterList
log4cplus::pattern::PatternParser::parse() 
{
    tchar c;
    pos = 0;
    while(pos < pattern.length()) {
        c = pattern[pos++];
        switch (state) {
        case LITERAL_STATE :
            // In literal state, the last char is always a literal.
            if(pos == pattern.length()) {
                currentLiteral += c;
                continue;
            }
            if(c == ESCAPE_CHAR) {
                // peek at the next char. 
                switch (pattern[pos]) {
                case ESCAPE_CHAR:
                    currentLiteral += c;
                    pos++; // move pointer
                    break;
                default:
                    if(! currentLiteral.empty ()) {
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

    if(! currentLiteral.empty ()) {
        list.push_back(new LiteralPatternConverter(currentLiteral));
      //getLogLog().debug("Parsed LITERAL converter: \""+currentLiteral+"\".");
    }

    return list;
}



void
log4cplus::pattern::PatternParser::finalizeConverter(log4cplus::tchar c) 
{
    PatternConverter* pc = 0;
    switch (c) {
        case LOG4CPLUS_TEXT('b'):
            pc = new BasicPatternConverter
                          (formattingInfo, 
                           BasicPatternConverter::BASENAME_CONVERTER);
            //getLogLog().debug("BASENAME converter.");
            //formattingInfo.dump(getLogLog());      
            break;
            
        case LOG4CPLUS_TEXT('c'):
            pc = new LoggerPatternConverter(formattingInfo, 
                                            extractPrecisionOption());
            getLogLog().debug( LOG4CPLUS_TEXT("LOGGER converter.") );
            formattingInfo.dump(getLogLog());      
            break;

        case LOG4CPLUS_TEXT('d'):
        case LOG4CPLUS_TEXT('D'):
            {
                log4cplus::tstring dOpt = extractOption();
                if(dOpt.empty ()) {
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
                //formattingInfo.dump(getLogLog());      
            }
            break;

        case LOG4CPLUS_TEXT('F'):
            pc = new BasicPatternConverter
                          (formattingInfo, 
                           BasicPatternConverter::FILE_CONVERTER);
            //getLogLog().debug("FILE NAME converter.");
            //formattingInfo.dump(getLogLog());      
            break;

        case LOG4CPLUS_TEXT('h'):
        case LOG4CPLUS_TEXT('H'):
            {
                bool fqdn = (c == LOG4CPLUS_TEXT('H'));
                pc = new HostnamePatternConverter(formattingInfo, fqdn);
                // getLogLog().debug( LOG4CPLUS_TEXT("HOSTNAME converter.") );
                // formattingInfo.dump(getLogLog());
            }
            break;

        case LOG4CPLUS_TEXT('i'):
            pc = new BasicPatternConverter
                          (formattingInfo, 
                           BasicPatternConverter::PROCESS_CONVERTER);
            //getLogLog().debug("PROCESS_CONVERTER converter.");
            //formattingInfo.dump(getLogLog());      
            break;

        case LOG4CPLUS_TEXT('l'):
            pc = new BasicPatternConverter
                          (formattingInfo, 
                           BasicPatternConverter::FULL_LOCATION_CONVERTER);
            //getLogLog().debug("FULL LOCATION converter.");
            //formattingInfo.dump(getLogLog());      
            break;

        case LOG4CPLUS_TEXT('L'):
            pc = new BasicPatternConverter
                          (formattingInfo, 
                           BasicPatternConverter::LINE_CONVERTER);
            //getLogLog().debug("LINE NUMBER converter.");
            //formattingInfo.dump(getLogLog());      
            break;

        case LOG4CPLUS_TEXT('m'):
            pc = new BasicPatternConverter
                          (formattingInfo, 
                           BasicPatternConverter::MESSAGE_CONVERTER);
            //getLogLog().debug("MESSAGE converter.");
            //formattingInfo.dump(getLogLog());      
            break;

        // 'M' is METHOD converter in log4j.
        // Not implemented.
        case LOG4CPLUS_TEXT('M'):
            goto not_implemented;

        case LOG4CPLUS_TEXT('n'):
            pc = new BasicPatternConverter
                          (formattingInfo, 
                           BasicPatternConverter::NEWLINE_CONVERTER);
            //getLogLog().debug("MESSAGE converter.");
            //formattingInfo.dump(getLogLog());      
            break;

        case LOG4CPLUS_TEXT('p'):
            pc = new BasicPatternConverter
                          (formattingInfo, 
                           BasicPatternConverter::LOGLEVEL_CONVERTER);
            //getLogLog().debug("LOGLEVEL converter.");
            //formattingInfo.dump(getLogLog());
            break;

        // 'r' is RELATIVE time converter in log4j.
        // Not implemented.
        case LOG4CPLUS_TEXT('r'):
            goto not_implemented;

        case LOG4CPLUS_TEXT('t'):
            pc = new BasicPatternConverter
                          (formattingInfo, 
                           BasicPatternConverter::THREAD_CONVERTER);
            //getLogLog().debug("THREAD converter.");
            //formattingInfo.dump(getLogLog());      
            break;

        case LOG4CPLUS_TEXT('x'):
            pc = new NDCPatternConverter (formattingInfo, ndcMaxDepth);
            //getLogLog().debug("NDC converter.");      
            break;

        // 'X' is MDC in log4j.
        // Not implemented.
        case LOG4CPLUS_TEXT('X'):
            goto not_implemented;

not_implemented:;
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

PatternLayout::PatternLayout(const log4cplus::tstring& pattern_)
{
    init(pattern_, 0);
}


PatternLayout::PatternLayout(const log4cplus::helpers::Properties& properties)
{
    unsigned ndcMaxDepth
        = std::atoi (LOG4CPLUS_TSTRING_TO_STRING (
            properties.getProperty (
                LOG4CPLUS_TEXT ("NDCMaxDepth"),
                LOG4CPLUS_TEXT ("0"))).c_str ());

    bool hasPattern = properties.exists( LOG4CPLUS_TEXT("Pattern") );
    bool hasConversionPattern = properties.exists( LOG4CPLUS_TEXT("ConversionPattern") );
    
    if(hasPattern) {
        getLogLog().warn( LOG4CPLUS_TEXT("PatternLayout- the \"Pattern\" property has been deprecated.  Use \"ConversionPattern\" instead."));
    }
    
    if(hasConversionPattern) {
        init(properties.getProperty( LOG4CPLUS_TEXT("ConversionPattern") ),
            ndcMaxDepth);
    }
    else if(hasPattern) {
        init(properties.getProperty( LOG4CPLUS_TEXT("Pattern") ), ndcMaxDepth);
    }
    else {
        throw std::runtime_error("ConversionPattern not specified in properties");
    }

}


void
PatternLayout::init(const log4cplus::tstring& pattern_, unsigned ndcMaxDepth)
{
    this->pattern = pattern_;
    this->parsedPattern = PatternParser(pattern, ndcMaxDepth).parse();

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
    if(parsedPattern.empty ()) {
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


