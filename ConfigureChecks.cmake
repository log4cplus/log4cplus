include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckTypeSize)
include(CheckCSourceCompiles)

check_include_files(dlfcn.h  HAVE_DLFCN_H )

check_include_files(sys/types.h  LOG4CPLUS_HAVE_SYS_TYPES_H )
check_include_files(sys/socket.h LOG4CPLUS_HAVE_SYS_SOCKET_H )
check_include_files(sys/time.h   LOG4CPLUS_HAVE_SYS_TIME_H )
check_include_files(sys/timeb.h  LOG4CPLUS_HAVE_SYS_TIMEB_H )
check_include_files(sys/stat.h   LOG4CPLUS_HAVE_SYS_STAT_H )
check_include_files(syslog.h     LOG4CPLUS_HAVE_SYSLOG_H )
check_include_files(netinet/in.h LOG4CPLUS_HAVE_NETINET_IN_H )
check_include_files(netdb.h      LOG4CPLUS_HAVE_NETDB_H )
check_include_files(unistd.h     LOG4CPLUS_HAVE_UNISTD_H )
check_include_files(stdio.h      LOG4CPLUS_HAVE_STDIO_H )
check_include_files(wchar.h      LOG4CPLUS_HAVE_WCHAR_H )
check_include_files(stdarg.h     LOG4CPLUS_HAVE_STDARG_H )
check_include_files(time.h       LOG4CPLUS_HAVE_TIME_H )
check_include_files(errno.h      LOG4CPLUS_HAVE_ERRNO_H )


check_include_files(inttypes.h   HAVE_INTTYPES_H )
check_include_files(memory.h     HAVE_MEMORY_H )
check_include_files(stdint.h     HAVE_STDINT_H )
check_include_files(stdlib.h     HAVE_STDLIB_H )
check_include_files(strings.h    HAVE_STRINGS_H )
check_include_files(string.h     HAVE_STRING_H )

check_include_files("stdlib.h;stdio.h;stdarg.h;string.h;float.h" STDC_HEADERS )

check_function_exists(gmtime_r      LOG4CPLUS_HAVE_GMTIME_R )
check_function_exists(localtime_r   LOG4CPLUS_HAVE_LOCALTIME_R )
check_function_exists(gettimeofday  LOG4CPLUS_HAVE_GETTIMEOFDAY )
check_function_exists(getpid        LOG4CPLUS_HAVE_GETPID )
check_function_exists(ftime         LOG4CPLUS_HAVE_FTIME )
check_function_exists(stat          LOG4CPLUS_HAVE_STAT )
check_function_exists(lstat         LOG4CPLUS_HAVE_LSTAT )
check_function_exists(htons         LOG4CPLUS_HAVE_HTONS )
check_function_exists(ntohs         LOG4CPLUS_HAVE_NTOHS )
check_function_exists(htonl         LOG4CPLUS_HAVE_HTONL )
check_function_exists(ntohl         LOG4CPLUS_HAVE_NTOHL )

check_symbol_exists(ENAMETOOLONG errno.h LOG4CPLUS_HAVE_ENAMETOOLONG)

# clock_gettime() needs -lrt here
check_function_exists(clock_gettime LOG4CPLUS_HAVE_CLOCK_GETTIME ) # TODO AC says this exists


check_function_exists(gethostbyname_r LOG4CPLUS_HAVE_GETHOSTBYNAME_R) # TODO more complicated test in AC
check_function_exists(getaddrinfo     LOG4CPLUS_HAVE_GETADDRINFO ) # TODO more complicated test in AC


# check for declspec stuff
check_c_source_compiles("int __attribute__ ((visibility(\"default\"))) main(void) { return 0; }" HAVE_ATTRIBUTE_VISIBILITY_DEFAULT)
if(HAVE_ATTRIBUTE_VISIBILITY_DEFAULT)
  set(LOG4CPLUS_DECLSPEC_EXPORT "__attribute__ ((visibility(\"default\")))" )
endif()
set(LOG4CPLUS_DECLSPEC_IMPORT "" )

set(CMAKE_EXTRA_INCLUDE_FILES sys/socket.h)
check_type_size(socklen_t _SOCKLEN_SIZE)
if (_SOCKLEN_SIZE)
  set(socklen_t)
else()
  set(socklen_t TRUE)
endif()


find_library(LIBADVAPI32 advapi32)
find_library(LIBKERNEL32 kernel32)
find_library(LIBNSL nsl)
find_library(LIBRT rt)
find_library(LIBSOCKET socket)
find_library(LIBWS2_32 ws2_32)

macro(PATH_TO_HAVE _pathVar )
  if (${_pathVar})
    set(HAVE_${_pathVar} TRUE)
  else ()
    set(HAVE_${_pathVar} FALSE)
  endif ()
endmacro()


path_to_have(LIBADVAPI32)
path_to_have(LIBKERNEL32)
path_to_have(LIBNSL)
path_to_have(LIBRT)
path_to_have(LIBSOCKET)
path_to_have(LIBWS2_32)


set(HAVE_SYS_STAT_H ${LOG4CPLUS_HAVE_SYS_STAT_H} )
set(HAVE_SYS_TYPES_H ${LOG4CPLUS_HAVE_SYS_TYPES_H} )
set(HAVE_UNISTD_H ${LOG4CPLUS_HAVE_UNISTD_H} )


set(HAVE_FTIME  ${LOG4CPLUS_HAVE_FTIME} )
set(HAVE_GETPID ${LOG4CPLUS_HAVE_GETPID} )
set(HAVE_GETTIMEOFDAY ${LOG4CPLUS_HAVE_GETTIMEOFDAY} )
set(HAVE_GETADDRINFO  ${LOG4CPLUS_HAVE_GETADDRINFO} )
set(HAVE_GETHOSTBYNAME_R ${LOG4CPLUS_HAVE_GETHOSTBYNAME_R} )
set(HAVE_GMTIME_R ${LOG4CPLUS_HAVE_GMTIME_R} )
set(HAVE_HTONL ${LOG4CPLUS_HAVE_HTONL} )
set(HAVE_HTONS ${LOG4CPLUS_HAVE_HTONS} )
set(HAVE_LSTAT ${LOG4CPLUS_HAVE_LSTAT} )
set(HAVE_LOCALTIME_R ${LOG4CPLUS_HAVE_LOCALTIME_R} )
set(HAVE_NTOHL ${LOG4CPLUS_HAVE_NTOHL} )
set(HAVE_NTOHS ${LOG4CPLUS_HAVE_NTOHS} )
set(HAVE_STAT  ${LOG4CPLUS_HAVE_STAT} )

