NOTE: log4cplus build for iOS is based on iOS.cmake toolchain file,
      orininally taken from https://code.google.com/p/ios-cmake/

To build the library for iOS, being in current folder, perform the steps below:
For armv7 architecture:
$ ./scripts/cmake_ios_armv7.sh
$ cmake --build ./build_armv7 --config "Release"
$ cmake --build ./build_armv7 --config "Debug"

For i386 architecture:
$ ./scripts/cmake_ios_i386.sh
$ cmake --build ./build_i386 --config "Release"
$ cmake --build ./build_i386 --config "Debug"
