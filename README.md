% log4cplus README

Short Description
=================

[log4cplus] is a simple to use C++23 logging API providing thread--safe,
flexible, and arbitrarily granular control over log management and
configuration.  It is modeled after the Java log4j API.

[log4cplus]: https://github.com/log4cplus/log4cplus


Latest Project Information
==========================

The latest up-to-date information for this project can be found on the
[GitHub][13] project page or the [log4cplus wiki][4]. Please submit bugs,
patches, feature requests, and so on on [GitHub][13].

[4]: https://github.com/log4cplus/log4cplus/wiki
[13]: https://github.com/log4cplus/log4cplus


Mission statement
=================

The aim of this project is to develop a log4j-like logging framework for
use primarily in C++. One of the major design goals is to avoid
huge dependencies (like Boost) in the core functionality and to use
standard C++ facilities instead. Where possible, the project takes
inspiration from other logging libraries, besides log4j (e.g.,
from log4net, log4cxx, log4cpp).


Platform support
================

[log4cplus] version 3.0 and beyond require C++23. [log4cplus] has been
continuously built and tested by GitHub Actions on these host platforms:

  - Ubuntu 24.04 on AMD64 with GCC 14 and on ARM64 with GCC;
  - macOS 15 on ARM64 with Apple Clang and LLVM 18;
  - Windows Server 2022 on AMD64 and Windows 11 on ARM64 with Visual Studio
    2022;
  - FreeBSD 13.5, 14.4 and 15.0, plus current OpenBSD, NetBSD and DragonFly BSD
    runner images, using both CMake and Autotools where supported.

GitHub Actions also performs compile and installation validation for these
cross-compiled targets:

  - Android API 21 or later with NDK Clang for `armeabi-v7a`, `arm64-v8a`,
    `x86` and `x86_64`. The Android job compiles and links the test suite,
    validates production installations and links external consumers, but does
    not execute on an Android device or emulator;
  - iOS 15 or later for ARM64 devices and ARM64/x86_64 Simulator targets.
    The iOS job also creates an unsigned static-library XCFramework. It does
    not execute the test suite on an iOS device or in Simulator.

Platforms not listed above may still work, but are not continuously validated
on the `master` branch.

The oldest Windows version that is supported by 3.x releases is Windows 10.

The following platforms were supported by the 1.x series of [log4cplus]. They
either do not have a reasonably C++23-capable compiler or have not yet been
checked with [log4cplus] 3.x:

  - Minix 3.3.0/i386 with Clang version 3.4 (branches/release_34) with
    `--disable-threads`
  - Linux/AMD64 with Intel Parallel Studio XE 2015, ICPC version 15.0.1
  - OpenSolaris with `-library=stlport4`
  - Solaris with `-library=stlport4` and with `-library=Cstd`.
  - Solaris 5.10/Sparc
  - MacOS X 10.8
  - MacOS X 11.4.2
  - HP-UX (hppa2.0w-hp-hpux11.11)
  - Haiku R1 Alpha 4.1
  - AIX 5.3 with IBM XL C/C++ for AIX


Installation instructions
=========================

Generic Autotools installation instructions are in the `INSTALL` file. The
following are [log4cplus]-specific instructions.

[log4cplus] uses Git sub-modules. Always use `--recurse-submodules` option when
doing `git clone`.


Configure script options
========================

`--enable-debugging`
--------------------

This option is disabled by default.  This option mainly affects GCC
builds but it also has some limited effect on non-GCC builds.  It
turns on debugging information generation, undefines `NDEBUG` symbol
and adds `-fstack-check` (GCC).


`--enable-warnings`
-------------------

This option is enabled by default.  It adds platform / compiler
dependent warning options to compiler command line.


`--enable-so-version`
---------------------

This option is enabled by default.  It enables SO version decoration
on resulting library file, e.g., the `.2.0.0` in
`liblog4cplus-1.2.so.2.0.0`.


`--enable-release-version`
--------------------------

This option is enabled by default.  It enables release version
decoration on the resulting library file, e.g., the `-1.2` in
`liblog4cplus-1.2.so.2.0.0`.


`--enable-symbols-visibility-options`
-------------------------------------

This option is enabled by default.  It enables use of compiler and
platform specific option for symbols visibility.  See also the
[Visibility][8] page on GCC Wiki.

[8]: http://gcc.gnu.org/wiki/Visibility


`--enable-profiling`
--------------------

This option is disabled by default.  This option adds profiling
information generation compiler option `-pg` to GCC and Sun CC /
Solaris Studio builds.


`--enable-threads`
------------------

This option is enabled by default.  It turns on detection of necessary
compiler and linker flags that enable POSIX threading support.

While this detection usually works well, some platforms still need
help with configuration by supplying additional flags to the
`configure` script. One of the known deficiencies is Solaris Studio on
Linux. See one of the later notes for details.


`--enable-tests`
----------------

This option is enabled by default.  It enables compilation of test executables.


`--enable-unit-tests`
---------------------

This option is disabled by default. It enables compilation of unit tests along
with their units. These unit tests can then be executed through the
`unit_tests` test executable that is built during compilation.


`--enable-implicit-initialization`
----------------------------------

This option is enabled by default.  It enables implicit initialization of
[log4cplus]. When it is turned off, [log4cplus] has to be explicitly
initialized by calling either `log4cplus::initialize()` or by instantiating
`log4cplus::Initializer`.


`--enable-lto`
--------------

This option is disabled by default.  It enables LTO (Link-Time Optimization)
builds.


`--with-wchar_t-support`
------------------------

This option is enabled by default.  When enabled, additional binaries will be
built, marked with `U` suffix in file name and compiled with `-DUNICODE=1`
flag. In effect, these binaries assume that `log4cplus::tchar` is `wchar_t`.


`--with-working-locale`
-----------------------

This is one of three locale and `wchar_t`↔`char` conversion related
options.  It is disabled by default.

It is known to work well with GCC on Linux. Other platforms generally
have lesser locale support in their implementations of the C++
standard library.  It is known not to work well on any BSDs.

See also docs/unicode.txt.


`--with-working-c-locale`
-------------------------

This is the second of the `wchar_t`↔`char` conversion-related options. It is
disabled by default.

It is known to work well on most Unix--like platforms, including
recent Cygwin.


`--with-iconv`
--------------

This is the third of the `wchar_t`↔`char` conversion-related options. It is
disabled by default.

The conversion using iconv() function always uses `"UTF-8"` and
`"WCHAR_T"` as source/target encoding.  It is known to work well on
platforms with GNU iconv.  Different implementations of `iconv()`
might not support `"WCHAR_T"` encoding selector.

Either the system-provided `iconv()` or the library-provided `libiconv()` is
detected and accepted. Both SUSv3 and GNU `iconv()` function
signatures are accepted.


`--with-qt`
-----------

This option is disabled by default.  It enables compilation of a
separate shared library (liblog4cplusqt4debugappender) that implements
`Qt4DebugAppender`.  It requires Qt4 and pkg-config to be installed.


`--with-qt5`
------------

This option is disabled by default.  It enables compilation of a separate
shared library (liblog4cplusqt5debugappender) that implements
`Qt5DebugAppender`.  It requires Qt5 and pkg-config to be available.


Notes
=====

Compilation
-----------

On Unix--like platforms, [log4cplus] can be compiled using either the
Autotools-based build system or the CMake build system. The
Autotools-based build system is considered to be primary for
Unix--like platforms.

On Windows, the primary build system is Visual Studio solution
and projects (`msvc14/log4cplus.sln`), currently tested with Visual Studio 2022.

Apple embedded platforms, including iOS and iOS Simulator, are supported using
CMake 3.20 or later with the Xcode generator.

Android is supported using CMake 3.20 or later with the toolchain file supplied
by the Android NDK.

MinGW is supported by the Autotools-based build system. The CMake build system
is supported as well, and it should be used to compile [log4cplus] with
older versions of Visual Studio or with less common compiler suites
(e.g., Embarcadero, Code::Blocks, etc.).


Cygwin
------

Cygwin 2.5.x has a problem[^pr64697] linking binaries that use language-level
thread-local storage and share thread-local variables across translation
units. To avoid the issue, language-level thread-local storage is not used on
Cygwin and traditional POSIX thread-local storage is used instead.

[^pr64697]: <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=64697>


MinGW and MSVCRT version
------------------------

[log4cplus] can use functions like `_vsnprintf_s()` (Microsoft's
secure version of `vsnprintf()`). MinGW toolchains (by default) link
to the system `MSVCRT.DLL`. Unfortunately, older systems, like Windows
XP, ship with `MSVCRT.DLL` that lacks these functions. It is possible
to compile [log4cplus] with MinGW toolchains but _without_ using
Microsoft's secure functions by defining `__MSVCRT_VERSION__` to a value
less than `0x900` and vice versa.

    $ ../configure CPPFLAGS="-D__MSVCRT_VERSION__=0x700"


Windows and Visual Studio
-------------------------

[log4cplus] uses C++11 thread and synchronization facilities. The
synchronization facilities are implemented in Visual Studio C++ standard
library in a way that utilizes global variables. Therefore it is impossible
(due to "static initialization order fiasco") to use them outside
`main()`. This issue manifests as a deadlock on exit during destruction of
[log4cplus]' thread pool.

To overcome this limitation,

  - always use `log4cplus::Initializer initializer;` as the first thing in
`main()`;

  - never try to log from static/global objects constructors;

  - never try to log from static/global object destructors.

Defining the `log4cplus::Initializer` instance as the first thing in `main()`
ensures that [log4cplus] is initialized. More importantly, it ensures that
[log4cplus] shuts down before the execution leaves the `main()`
function. [log4cplus] will try to automatically initialize at process startup
and/or on DLL load, and will not tear down until all `log4cplus:Initializer`
instances are destroyed.


Windows and file Appenders
--------------------------

On Windows, file-based appenders use [log4cplus]' Win32-backed file stream
instead of the standard C++ file streams. The Win32-backed stream opens files
using `CreateFileW()` with read, write, and delete sharing. This allows log
files to be renamed or deleted while they are open by another process and lets
rolling file appenders roll over files in that situation. It stores file
contents as UTF-8.

Older [log4cplus] releases used the standard C++ file streams on Windows. Those
streams did not open the underlying Win32 file `HANDLE` with
`FILE_SHARE_DELETE`, which could make rollover fail with error code 13 if
another process still had the log file open. This was tracked as
[bug #167](https://sourceforge.net/p/log4cplus/bugs/167/) on SourceForge.


Windows and TLS
---------------

[log4cplus] uses thread--local storage (TLS) for NDC, MDC and to
optimize use of some temporary objects.  On Windows there are two ways
to get TLS:

1. using `TlsAlloc()`, etc., functions
2. using `__declspec(thread)`

While method (2) generates faster code, it has
[some limitations prior to Windows Vista][tlsvista].  If
`log4cplus.dll` is loaded at run time using `LoadLibrary()` (or as a
dependency of such loaded library), then accessing
`__declspec(thread)` variables can cause general protection fault
(GPF) errors.  This is because Windows prior to Windows Vista do not
extend the TLS for libraries loaded at run time using `LoadLibrary()`.
To allow using the best available method, [log4cplus] enables the
method (2) when compiling [log4cplus] targeted to Windows 10 or later.

[tlsvista]: https://learn.microsoft.com/en-us/cpp/parallel/thread-local-storage-tls?view=msvc-170


Linking on Windows
------------------

If you are linking your application with the DLL variant of [log4cplus], define
`LOG4CPLUS_BUILD_DLL` preprocessor symbol. This changes definition of
`LOG4CPLUS_EXPORT` symbol to `__declspec(dllimport)`.


Android and CMake
-----------------

[log4cplus] is distributed as source code for Android. Build it with CMake and
the toolchain file supplied by the Android NDK so that it uses the same NDK,
minimum API level, C++ runtime and compiler settings as the rest of the
application. Continuous integration uses NDK r29 (`29.0.14206865`), C++23 and
API level 21 as its baseline. It checks the `armeabi-v7a`, `arm64-v8a`, `x86`
and `x86_64` ABIs.

For example, build and install a shared ARM64 library using the shared C++
runtime as follows:

    $ NDK="$ANDROID_SDK_ROOT/ndk/29.0.14206865"
    $ cmake -S . -B build/android-arm64-v8a-shared -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$PWD/build/android-arm64-v8a-shared/install" \
        -DANDROID_ABI=arm64-v8a \
        -DANDROID_PLATFORM=android-21 \
        -DANDROID_STL=c++_shared \
        -DBUILD_SHARED_LIBS=ON \
        -DLOG4CPLUS_BUILD_TESTING=OFF \
        -DWITH_UNIT_TESTS=OFF
    $ cmake --build build/android-arm64-v8a-shared
    $ cmake --install build/android-arm64-v8a-shared

The application or Gradle packaging must include exactly one compatible copy
of `libc++_shared.so`. [log4cplus] does not distribute that runtime.

Android builds also provide `log4cplus::AndroidAppender`, which writes to the
main Android Logcat buffer through the NDK `liblog` API. The Logcat tag is
configurable and defaults to `log4cplus`:

    log4cplus.appender.LOGCAT=log4cplus::AndroidAppender
    log4cplus.appender.LOGCAT.Tag=MyApplication
    log4cplus.appender.LOGCAT.layout=log4cplus::PatternLayout
    log4cplus.appender.LOGCAT.layout.ConversionPattern=%c - %m

The equivalent programmatic setup is:

    #include <log4cplus/androidappender.h>

    log4cplus::SharedAppenderPtr appender (
        new log4cplus::AndroidAppender (
            LOG4CPLUS_TEXT ("MyApplication")));
    logger.addAppender (appender);

For compatibility with Android API levels 21 through 25, tags are limited to
23 UTF-8 bytes. Formatted messages longer than a Logcat entry are split at
UTF-8 boundaries. Log levels map to the corresponding Logcat priorities, with
TRACE using `VERBOSE` and FATAL using `FATAL`.

The shared Android library therefore has an intentional dependency on the
system-provided `liblog.so`. Static CMake package consumers receive the `log`
link requirement transitively. Android supplies this library; [log4cplus]
does not package or distribute it.

Alternatively, build static [log4cplus] and link it into the application's
final JNI shared library. This permits the final shared library to contain one
statically linked C++ runtime:

    $ cmake -S . -B build/android-arm64-v8a-static -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$PWD/build/android-arm64-v8a-static/install" \
        -DANDROID_ABI=arm64-v8a \
        -DANDROID_PLATFORM=android-21 \
        -DANDROID_STL=c++_static \
        -DBUILD_SHARED_LIBS=OFF \
        -DLOG4CPLUS_BUILD_TESTING=OFF \
        -DWITH_UNIT_TESTS=OFF
    $ cmake --build build/android-arm64-v8a-static
    $ cmake --install build/android-arm64-v8a-static

Use a separate build directory for every ABI and shared/static runtime choice.
The Android logging server is disabled by default. The test suite remains
enabled by default and can be cross-compiled and linked, but it is not executed
without an Android device or emulator. Android CI does not publish libraries,
libc++, `liblog` or other Android artifacts.


Threads and signals
-------------------

[log4cplus] is not safe to be used from asynchronous signals'
handlers.  This is a property of most threaded programmes in general.
If you are going to use [log4cplus] in threaded application and if you
want to use [log4cplus] from signal handlers then your only option is
to block signals in all threads but one that will handle all signals.
On POSIX platforms, this is possible using the `sigwait()` call.
[log4cplus] enables this approach by blocking all signals in any
threads created through its threads helpers.


IBM's XL C/C++ compiler
-----------------------

IBM's XL C/C++ compiler executable has [many variants][1].  To compile
[log4cplus] with threading support specify one of the compiler
variants that support threading using the `CXX` variable on
`configure` script command line.  E.g.:

    $ ../configure --enable-threads CXX=xlC_r

[1]: https://www.ibm.com/docs/en/xl-c-and-cpp-aix/13.1.0?topic=applications-invoking-compiler


AIX reentrancy problem
----------------------

There appears to be a reentrancy problem with AIX 5.3 and xlC 8 which
can result into a deadlock condition in some circumstances.  It is
unknown whether the problem manifests with other versions of either
the OS or the compiler, too.  The problem was initially reported in a
bug report [#103][2].

The core of the problem is that IBM's/xlC's standard C++ IOStreams
implementation uses global non recursive lock to protect some of its
state.  The application in the bug report was trying to do logging
using [log4cplus] from inside `overflow()` member function of a class
derived from `std::streambuf` class.  [log4cplus] itself uses
`std::ostringstream`.  This resulted into an attempt to recursively
lock the global non recursive lock and a deadlock.

[2]: https://sourceforge.net/p/log4cplus/bugs/103/


Solaris / SunOS
---------------

Some older version of this operating system might have problems
linking [log4cplus] due to [missing `__tls_get_addr`][3] in their
unpatched state.

[3]: https://groups.google.com/g/comp.unix.solaris/c/AAMqkK0QZ6U/m/zlkVKA1L_QcJ


Solaris Studio
--------------

Solaris Studio compilers' default standard C++ library is very
non-standard.  It seems that it is not conforming enough in, e.g., Sun
C++ 5.12 Linux_i386 2011/11/16 (missing `std::time_t`, etc.), but it
works well enough on Solaris with Sun C++ 5.8 2005/10/13.  Thus
[log4cplus] adds `-library=stlport4` to the `CXXFLAGS` environment
variable, unless a switch matching `-library=(stlport4|stdcxx4|Cstd)`
is already present there.  If you want to override the default
supplied by [log4cplus], just set it into `CXXFLAGS` on `configure`
script command line.

Solaris Studio supports the `__func__` symbol which can be used by
[log4cplus] to record function name in logged events.  To enable this
feature, add `-features=extensions` switch to `CXXFLAGS` for
`configure` script.  Subsequently, you will have to add this switch to
your application's build flags as well.


Solaris Studio on GNU/Linux
---------------------------

The Autotools and our `configure.ac` combination do not handle the Solaris
Studio compiler on Linux well enough and need a little help with the
configuration of POSIX threads:

```sh
$ COMMON_FLAGS="-L/lib/x86_64-linux-gnu/ \
-L/usr/lib/x86_64-linux-gnu/ -mt=yes -O"

$ ../configure --enable-threads=yes \
CC=/opt/solarisstudio12.3/bin/cc \
CXX=/opt/solarisstudio12.3/bin/CC \
CFLAGS="$COMMON_FLAGS" \
CXXFLAGS="$COMMON_FLAGS" \
LDFLAGS="-lpthread"
```


HP-UX with `aCC`
----------------

It is necessary to turn on C++98 mode of `aCC` by providing the `-AA`
flag:

    $ ../configure --enable-threads=yes CXXFLAGS="-AA"


HP-UX with `aCC` on IA64
------------------------

There is a problem on IA64 HP-UX with `aCC` (HP C/aC++ B3910B
A.06.20). The problem manifests as
[unsatisfied symbols during linking of `loggingserver`][9]:

    ld: Unsatisfied symbol "virtual table of loggingserver::ClientThread" in file loggingserver.o

The problem appears to be a deficiency in `aCC` and its support of
`__declspec(dllexport)`. To work around this issue, add
`--disable-symbols-visibility-options` to `configure` script command
line:

    $ ../configure --disable-symbols-visibility-options \
    --enable-threads=yes CXXFLAGS="-AA"

[9]: https://community.hpe.com/t5/languages-and-scripting/building-log4cplus-fails-with-quot-ld-unsatisfied-symbol-virtual/m-p/6262849#.Y_5ho4DMImM


Haiku
-----

Haiku is supported with GCC 4+. The default GCC version in Haiku is
set to version 2 (based on GCC 2.95.x). To change the default GCC
version to 4, please run the `setgcc gcc4` command. This is to
avoid linking errors like this:

    main.cpp:(.text.startup+0x54a): undefined reference to `_Unwind_Resume'

Running the command switches the _current_ GCC version to version 4.
This change is permanent and global. See also the Haiku ticket
[#8368](https://dev.haiku-os.org/ticket/8368).


Qt4 / Win32 / MSVC
------------------

In order to use [log4cplus] in Qt4 programs it is necessary to set the
following option: `Treat WChar_t As Built-in Type: No (/Zc:wchar_t-)`

Set this option for the [log4cplus] project and the `Qt4DebugAppender`
project in MS Visual Studio. Remember to use the Unicode versions of
[log4cplus] libraries with Qt. It is also necessary to make a clear
distinction between debug and release builds of the Qt project and
[log4cplus]. Do not use the [log4cplus] release library with the debug
version of a Qt program and vice versa.

To register the Qt4DebugAppender library at runtime, call this
function: `log4cplus::Qt4DebugAppender::registerAppender()`

Add these lines to the qmake project file to use [log4cplus] and
`Qt4DebugAppender`:

    INCLUDEPATH += C:\log4cplus\include
    win32 {
        CONFIG(debug, debug|release) {
            LIBS += -LC:\log4cplus\msvc14\Win32\bin.Debug_Unicode -llog4cplusUD
            LIBS += -LC:\log4cplus\msvc14\Win32\bin.Debug_Unicode -llog4cplus-Qt4DebugAppender
        } else {
            LIBS += -LC:\log4cplus\msvc14\Win32\bin.Release_Unicode -llog4cplusU
            LIBS += -LC:\log4cplus\msvc14\Win32\bin.Release_Unicode -llog4cplus-Qt4DebugAppender
        }
    }


Qt / GCC
--------

You might encounter the following error during compilation with
`--with-qt` option:

    qglobal.h:943: error: ISO C++ does not support 'long long'

This is caused by `-pedantic` option that [log4cplus] adds to
`CXXFLAGS` when compiling with GCC.  To work around this issue, add
`-Wno-long-long` GCC option to `CXXFLAGS`.


OpenBSD
-------

OpenBSD 5.2 and earlier have a bug in `wcsftime()` function in
handling of `%%` and `%N` where N is not a supported formatter. This
is fixed in OpenBSD 5.3 and later. This shows as failing
`timeformat_test` when [log4cplus] is compiled with `-DUNICODE` in
`CXXFLAGS`.


iOS support
-----------

iOS builds require macOS, Xcode and CMake 3.20 or later. The minimum supported
deployment target is iOS 15. Configure device and Simulator builds in separate
directories so that CMake's SDK-specific feature and dependency checks do not
leak between the two targets.

Build and install the ARM64 device library:

    $ cmake -S . -B build/ios-device -G Xcode \
        -DCMAKE_SYSTEM_NAME=iOS \
        -DCMAKE_OSX_SYSROOT=iphoneos \
        -DCMAKE_OSX_ARCHITECTURES=arm64 \
        -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0 \
        -DCMAKE_INSTALL_PREFIX="$PWD/build/ios-device/install" \
        -DBUILD_SHARED_LIBS=OFF
    $ cmake --build build/ios-device --config Release --target install

Build and install a universal Simulator library for Apple Silicon and Intel
Macs:

    $ cmake -S . -B build/ios-simulator -G Xcode \
        -DCMAKE_SYSTEM_NAME=iOS \
        -DCMAKE_OSX_SYSROOT=iphonesimulator \
        '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64' \
        -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0 \
        -DCMAKE_INSTALL_PREFIX="$PWD/build/ios-simulator/install" \
        -DBUILD_SHARED_LIBS=OFF
    $ cmake --build build/ios-simulator --config Release --target install

The iOS defaults disable the logging server and executable test suite while
retaining normal multithreading and thread-local storage detection. Release
IPO is also disabled so that the static archives contain Mach-O object files
whose architectures `xcodebuild -create-xcframework` can inspect. These options
can also be set explicitly with `LOG4CPLUS_BUILD_LOGGINGSERVER` and
`LOG4CPLUS_BUILD_TESTING`.

Package both installed variants as an XCFramework:

    $ xcodebuild -create-xcframework \
        -library build/ios-device/install/lib/liblog4cplusS.a \
        -headers build/ios-device/install/include \
        -library build/ios-simulator/install/lib/liblog4cplusS.a \
        -headers build/ios-simulator/install/include \
        -output build/log4cplus.xcframework

Device and Simulator libraries are different platform variants even when both
contain ARM64 code. Do not combine their slices with `lipo`; keep them separate
and use an XCFramework.


`LOG4CPLUS_*_FMT()` and UNICODE
-------------------------------

Beware, the `%s` specifier does not work the same way on Unix--like
platforms as it does on Windows with Visual Studio. With Visual Studio
the `%s` specifier changes its meaning by printing a `wchar_t` string
when used with `wprintf()` and a `char` string when used with
`printf()`. On the other hand, Unix-like platforms keep the meaning of
printing `char` strings when used with both `wprintf()` and
`printf()`. It is necessary to use the `%ls` (C99) specifier or the `%S`
(SUSv2) specifier to print `wchar_t` strings on Unix-like platforms.

The common ground for both platforms appears to be using `%ls` and a
`wchar_t` string to print strings with an unmodified format string on
both Unix-like platforms and Windows. The conversion of `wchar_t` back
to `char` then depends on the C locale.


Unsupported compilers and platforms
-----------------------------------

[log4cplus] does not support too old or broken C++ compilers. Since [log4cplus]
version 3.0.0, it means it does not support any platform or compiler without
decent C++23 support.

  - Visual Studio prior to 2022
  - GCC prior to 4.8


Bug reporting instructions
--------------------------

For successful resolution of reported bugs, it is necessary to provide enough information:

- [log4cplus]
    - What is the exact release version or Git branch and revision?
    - What is the build system that you are building [log4cplus] with
      (Autotools, Visual Studio solution and its version, CMake).
    - Autotools -- Provide `configure` script parameters and environment
      variables, attach generated `config.log` and `defines.hxx` files.
    - CMake -- Provide build configuration (`Release`, `Debug`,
      `RelWithDebInfo`) and non--default `CMAKE_*` variables values.
    - Visual Studio -- Provide project configuration (`Release`,
      `Release_Unicode`, `Debug`, `Debug_Unicode`) and Visual Studio version.
    - Provide target OS and CPU. In case of MinGW, provide its exact compiler
      distribution -- TDM? Nuwen? Other?

- [log4cplus] client application
    - Are you using shared library [log4cplus] or as static library [log4cplus]?
    - Is [log4cplus] linked into an executable or into a shared library (DLL or
      SO)?
    - If [log4cplus] is linked into a shared library, is this library
      loaded dynamically or not?
    - What library file you are linking your application with --
      `log4cplus.lib`, `log4cplusUSD.lib`, `liblog4cplus.dll.a`, etc., on
      Windows?
    - Is your application is using Unicode/`wchar_t` or not?
    - Provide any error messages.
    - Provide stack trace.
    - Provide [log4cplus] properties/configuration files.
    - Provide a self--contained test case, if possible.


License
=======

This library is licensed under the Apache Public License 2.0 and the
two-clause BSD license. Please read the included [LICENSE](./LICENSE) file for
details.


Contributions
=============

[log4cplus] (bug tracker, files, wiki) is hosted on GitHub. See also the
[Contributions](https://github.com/log4cplus/log4cplus/wiki/Development#contributions)
topic on the wiki.


Patches
-------

Anybody can contribute to log4cplus development.
If you are contributing a source code change, use a reasonable
form: a merge request of a Git branch.


Formatting
----------

Please use common sense. Follow the style of the surrounding code. You can
use the following Emacs style, which is based on Microsoft's style, as a
guideline:

```lisp
;; Custom MS like indentation style.
(c-add-style "microsoft"
             '("stroustrup"
               (c-offsets-alist
                (innamespace . -)
                (inline-open . 0)
                (inher-cont . c-lineup-multi-inher)
                (arglist-cont-nonempty . +)
                (template-args-cont . +))))
```


Tools
-----

### Build system

[log4cplus] supports multiple build systems (GNU Autoconf/Automake/Libtool aka
Autotools, CMake and Visual Studio solution and project files).

Autotools is considered the primary build system on Unix--like
platforms. However, CMake should still be usable on Unix--like platforms as
well.

On Windows, it depends on the compiler and toolchain that you want to use. When
using Visual Studio, use Visual Studio solution and project files. However,
the CMake build system should still work and produce useful results. When using
some form of MinGW64 toolchain, the CMake build system is considered primary
and the Autotools-based build system is unsupported. Use the `MinGW Makefiles`
option and build with `mingw-make` (or similar). The `MSYS Makefiles` option is
untested and unsupported.

For Apple embedded platforms, use CMake with the Xcode generator and CMake's
native platform variables. Custom iOS toolchain files are not supported.

For Android, use CMake with the toolchain file supplied by the Android NDK.
Legacy third-party Android toolchain files and their custom variables are not
supported.

#### Autotools

The `Makefile.am` files for this build system are hand-written. Some of them,
however, are generated from `Makefile.am.tpl` and `Makefile.am.def` by
[GNU Autogen][12]. This is to make adding new files to the source easier.

To regenerate `Makefile.am` files, `configure` script, `testsuite` script or
any other part of the Autotools build system, use the `scripts/doautoreconf.sh`
script from the source root directory. It will invoke all the necessary tools in
the correct order.

[log4cplus] closely follows Autoconf's, Automake's and Libtool's development
and its master branch files are always generated using the latest available
version of the tools.

[12]: https://www.gnu.org/software/autogen/
