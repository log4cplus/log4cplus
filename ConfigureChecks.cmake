include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckTypeSize)
include(CheckCSourceCompiles)

check_include_files(dlfcn.h  HAVE_DLFCN_H )

check_include_files(errno.h       LOG4CPLUS_HAVE_ERRNO_H )
check_include_files(iconv.h       LOG4CPLUS_HAVE_ICONV_H )
check_include_files(limits.h      LOG4CPLUS_HAVE_LIMITS_H )
check_include_files(sys/types.h   LOG4CPLUS_HAVE_SYS_TYPES_H )
check_include_files(sys/socket.h  LOG4CPLUS_HAVE_SYS_SOCKET_H )
check_include_files(sys/syscall.h LOG4CPLUS_HAVE_SYS_SYSCALL_H )
check_include_files(sys/time.h    LOG4CPLUS_HAVE_SYS_TIME_H )
check_include_files(sys/timeb.h   LOG4CPLUS_HAVE_SYS_TIMEB_H )
check_include_files(sys/stat.h    LOG4CPLUS_HAVE_SYS_STAT_H )
check_include_files(sys/file.h    LOG4CPLUS_HAVE_SYS_FILE_H )
check_include_files(syslog.h      LOG4CPLUS_HAVE_SYSLOG_H )
check_include_files(netinet/in.h  LOG4CPLUS_HAVE_NETINET_IN_H )
check_include_files(netinet/tcp.h LOG4CPLUS_HAVE_NETINET_TCP_H )
check_include_files(netdb.h       LOG4CPLUS_HAVE_NETDB_H )
check_include_files(unistd.h      LOG4CPLUS_HAVE_UNISTD_H )
check_include_files(fcntl.h       LOG4CPLUS_HAVE_FCNTL_H )
check_include_files(stdio.h       LOG4CPLUS_HAVE_STDIO_H )
check_include_files(stdarg.h      LOG4CPLUS_HAVE_STDARG_H )
check_include_files(stdlib.h      LOG4CPLUS_HAVE_STDLIB_H )
check_include_files(time.h        LOG4CPLUS_HAVE_TIME_H )
check_include_files(wchar.h       LOG4CPLUS_HAVE_WCHAR_H )


check_include_files(inttypes.h    HAVE_INTTYPES_H )
check_include_files(memory.h      HAVE_MEMORY_H )
check_include_files(stdint.h      HAVE_STDINT_H )
check_include_files(strings.h     HAVE_STRINGS_H )
check_include_files(string.h      HAVE_STRING_H )


check_include_files("stdlib.h;stdio.h;stdarg.h;string.h;float.h" STDC_HEADERS )

check_function_exists(gmtime_r      LOG4CPLUS_HAVE_GMTIME_R )
check_function_exists(localtime_r   LOG4CPLUS_HAVE_LOCALTIME_R )
check_function_exists(gettimeofday  LOG4CPLUS_HAVE_GETTIMEOFDAY )
check_function_exists(getpid        LOG4CPLUS_HAVE_GETPID )
check_function_exists(ftime         LOG4CPLUS_HAVE_FTIME )
check_function_exists(stat          LOG4CPLUS_HAVE_STAT )
check_function_exists(lstat         LOG4CPLUS_HAVE_LSTAT )
check_function_exists(fcntl         LOG4CPLUS_HAVE_FCNTL )
check_function_exists(lockf         LOG4CPLUS_HAVE_FLOCK )
check_function_exists(flock         LOG4CPLUS_HAVE_LOCKF )
check_function_exists(htons         LOG4CPLUS_HAVE_HTONS )
check_function_exists(ntohs         LOG4CPLUS_HAVE_NTOHS )
check_function_exists(htonl         LOG4CPLUS_HAVE_HTONL )
check_function_exists(ntohl         LOG4CPLUS_HAVE_NTOHL )
check_function_exists(iconv_open    LOG4CPLUS_HAVE_ICONV_OPEN )
check_function_exists(iconv_close   LOG4CPLUS_HAVE_ICONV_CLOSE )
check_function_exists(iconv         LOG4CPLUS_HAVE_ICONV )
#check_function_exists(libiconv_open LOG4CPLUS_HAVE_ICONV_OPEN )
#check_function_exists(libiconv_close LOG4CPLUS_HAVE_ICONV_CLOSE )
#check_function_exists(libiconv      LOG4CPLUS_HAVE_ICONV )
check_function_exists(vsnprintf     LOG4CPLUS_HAVE_VSNPRINTF )
check_function_exists(_vsnprintf    LOG4CPLUS_HAVE__VSNPRINTF )
check_function_exists(vsprintf_s    LOG4CPLUS_HAVE_VSPRINTF_S )
check_function_exists(vswprintf_s   LOG4CPLUS_HAVE_VSWPRINTF_S )
check_function_exists(vfprintf_s    LOG4CPLUS_HAVE_VFPRINTF_S )
check_function_exists(vfwprintf_s   LOG4CPLUS_HAVE_VFWPRINTF_S )
check_function_exists(_vsnprintf_s  LOG4CPLUS_HAVE__VSNPRINTF_S )
check_function_exists(_vsnwprintf_s LOG4CPLUS_HAVE__VSNWPRINTF_S )


check_symbol_exists(ENAMETOOLONG          errno.h       LOG4CPLUS_HAVE_ENAMETOOLONG )
check_symbol_exists(SYS_gettid            sys/syscall.h LOG4CPLUS_HAVE_GETTID )
check_symbol_exists(__FUNCTION__          ""            LOG4CPLUS_HAVE_FUNCTION_MACRO )
check_symbol_exists(__PRETTY_FUNCTION__   ""            LOG4CPLUS_HAVE_PRETTY_FUNCTION_MACRO )

check_c_source_compiles("#include <stdlib.h> \n int main() { int x = 1; int y = __sync_add_and_fetch (&x, 1); return y;}"
                        LOG4CPLUS_HAVE___SYNC_ADD_AND_FETCH )

check_c_source_compiles("#include <stdlib.h> \n int main() { int x = 1; int y = __sync_sub_and_fetch (&x, 1); return y;}"
                        LOG4CPLUS_HAVE___SYNC_SUB_AND_FETCH )

check_c_source_compiles("#include <stdio.h>\n #define MACRO(buf, args...) (sprintf (buf, \"%d\", args))\n int main() {char a[10]; MACRO(a, 1); return 0; }"
                        LOG4CPLUS_HAVE_GNU_VARIADIC_MACROS )

check_c_source_compiles("#include <stdio.h>\n #define MACRO(buf, ...) (sprintf (buf, \"%d\",  __VA_ARGS__))\n int main() {char a[10]; MACRO(a, 1); return 0; }"
                        LOG4CPLUS_HAVE_C99_VARIADIC_MACROS )


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


set(HAVE_STDLIB_H              ${LOG4CPLUS_HAVE_STDLIB_H} )
set(HAVE_SYS_STAT_H            ${LOG4CPLUS_HAVE_SYS_STAT_H} )
set(HAVE_SYS_TYPES_H           ${LOG4CPLUS_HAVE_SYS_TYPES_H} )
set(HAVE_SYS_FILE_H            ${LOG4CPLUS_HAVE_SYS_FILE_H} )
set(HAVE_UNISTD_H              ${LOG4CPLUS_HAVE_UNISTD_H} )


set(HAVE_FTIME                 ${LOG4CPLUS_HAVE_FTIME} )
set(HAVE_GETPID                ${LOG4CPLUS_HAVE_GETPID} )
set(HAVE_GETTIMEOFDAY          ${LOG4CPLUS_HAVE_GETTIMEOFDAY} )
set(HAVE_GETADDRINFO           ${LOG4CPLUS_HAVE_GETADDRINFO} )
set(HAVE_GETHOSTBYNAME_R       ${LOG4CPLUS_HAVE_GETHOSTBYNAME_R} )
set(HAVE_GMTIME_R              ${LOG4CPLUS_HAVE_GMTIME_R} )
set(HAVE_HTONL                 ${LOG4CPLUS_HAVE_HTONL} )
set(HAVE_HTONS                 ${LOG4CPLUS_HAVE_HTONS} )
set(HAVE_ICONV_OPEN            ${LOG4CPLUS_HAVE_ICONV_OPEN} )
set(HAVE_ICONV_CLOSE           ${LOG4CPLUS_HAVE_ICONV_CLOSE} )
set(HAVE_ICONV                 ${LOG4CPLUS_HAVE_ICONV} )
set(HAVE_LSTAT                 ${LOG4CPLUS_HAVE_LSTAT} )
set(HAVE_FCNTL                 ${LOG4CPLUS_HAVE_FCNTL} )
set(HAVE_LOCKF                 ${LOG4CPLUS_HAVE_LOCKF} )
set(HAVE_FLOCK                 ${LOG4CPLUS_HAVE_FLOCK} )
set(HAVE_LOCALTIME_R           ${LOG4CPLUS_HAVE_LOCALTIME_R} )
set(HAVE_NTOHL                 ${LOG4CPLUS_HAVE_NTOHL} )
set(HAVE_NTOHS                 ${LOG4CPLUS_HAVE_NTOHS} )
set(HAVE_STAT                  ${LOG4CPLUS_HAVE_STAT} )

set(HAVE_VFPRINTF_S            ${LOG4CPLUS_HAVE_VFPRINTF_S} )
set(HAVE_VFWPRINTF_S           ${LOG4CPLUS_HAVE_VFWPRINTF_S} )
set(HAVE_VSNPRINTF             ${LOG4CPLUS_HAVE_VSNPRINTF} )
set(HAVE_VSPRINTF_S            ${LOG4CPLUS_HAVE_VSPRINTF_S} )
set(HAVE_VSWPRINTF_S           ${LOG4CPLUS_HAVE_VSWPRINTF_S} )
set(HAVE__VSNPRINTF            ${LOG4CPLUS_HAVE__VSNPRINTF} )
set(HAVE__VSNPRINTF_S          ${LOG4CPLUS_HAVE__VSNPRINTF_S} )
set(HAVE__VSNWPRINTF_S         ${LOG4CPLUS_HAVE__VSNWPRINTF_S} )

set(HAVE_FUNCTION_MACRO        ${LOG4CPLUS_HAVE_FUNCTION_MACRO} )
set(HAVE_PRETTY_FUNCTION_MACRO ${LOG4CPLUS_HAVE_PRETTY_FUNCTION_MACRO} )

set(HAVE___SYNC_ADD_AND_FETCH  ${LOG4CPLUS_HAVE___SYNC_ADD_AND_FETCH} )
set(HAVE___SYNC_SUB_AND_FETCH  ${LOG4CPLUS_HAVE___SYNC_SUB_AND_FETCH} )
