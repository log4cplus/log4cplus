# Microsoft Developer Studio Project File - Name="log4cplus_static" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=log4cplus_static - Win32 Debug_Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "log4cplus_static.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "log4cplus_static.mak" CFG="log4cplus_static - Win32 Debug_Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "log4cplus_static - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "log4cplus_static - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "log4cplus_static - Win32 Release_Unicode" (based on "Win32 (x86) Static Library")
!MESSAGE "log4cplus_static - Win32 Debug_Unicode" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "log4cplus_static - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release/Static"
# PROP Intermediate_Dir "Release/Static"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /D "LOG4CPLUS_STATIC" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Release/Static\log4cplusS.lib"

!ELSEIF  "$(CFG)" == "log4cplus_static - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug/Static"
# PROP Intermediate_Dir "Debug/Static"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /D "_DEBUG" /D "_USRDLL" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /D "LOG4CPLUS_STATIC" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\Static\log4cplusSD.lib"

!ELSEIF  "$(CFG)" == "log4cplus_static - Win32 Release_Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "log4cplus_static___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "log4cplus_static___Win32_Release_Unicode"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release/Static"
# PROP Intermediate_Dir "Release/Static"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /D "LOG4CPLUS_BUILD_STATIC" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /D "NDEBUG" /D "_UNICODE" /D "UNICODE" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /D "LOG4CPLUS_STATIC" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Release/Static\log4cplusS.lib"
# ADD LIB32 /nologo /out:"Release/Static\log4cplusSU.lib"

!ELSEIF  "$(CFG)" == "log4cplus_static - Win32 Debug_Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "log4cplus_static___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "log4cplus_static___Win32_Debug_Unicode"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug/Static"
# PROP Intermediate_Dir "Debug/Static"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /D "_USRDLL" /D "LOG4CPLUS_BUILD_STATIC" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /D "_DEBUG" /D "_USRDLL" /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /D "LOG4CPLUS_STATIC" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Debug\Static\log4cplusSD.lib"
# ADD LIB32 /nologo /out:"Debug\Static\log4cplusSDU.lib"

!ENDIF 

# Begin Target

# Name "log4cplus_static - Win32 Release"
# Name "log4cplus_static - Win32 Debug"
# Name "log4cplus_static - Win32 Release_Unicode"
# Name "log4cplus_static - Win32 Debug_Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\appender.cxx
# End Source File
# Begin Source File

SOURCE=..\src\appenderattachableimpl.cxx
# End Source File
# Begin Source File

SOURCE=..\src\configurator.cxx
# End Source File
# Begin Source File

SOURCE=..\src\consoleappender.cxx
# End Source File
# Begin Source File

SOURCE=..\src\factory.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fileappender.cxx
# End Source File
# Begin Source File

SOURCE=..\src\filter.cxx
# End Source File
# Begin Source File

SOURCE="..\src\global-init.cxx"
# End Source File
# Begin Source File

SOURCE=..\src\hierarchy.cxx
# End Source File
# Begin Source File

SOURCE=..\src\hierarchylocker.cxx
# End Source File
# Begin Source File

SOURCE=..\src\layout.cxx
# End Source File
# Begin Source File

SOURCE=..\src\logger.cxx
# End Source File
# Begin Source File

SOURCE=..\src\loggerimpl.cxx
# End Source File
# Begin Source File

SOURCE=..\src\loggingserver.cxx
# End Source File
# Begin Source File

SOURCE=..\src\loggingevent.cxx
# End Source File
# Begin Source File

SOURCE=..\src\loglevel.cxx
# End Source File
# Begin Source File

SOURCE=..\src\loglog.cxx
# End Source File
# Begin Source File

SOURCE=..\src\logloguser.cxx
# End Source File
# Begin Source File

SOURCE=..\src\ndc.cxx
# End Source File
# Begin Source File

SOURCE=..\src\nteventlogappender.cxx
# End Source File
# Begin Source File

SOURCE=..\src\nullappender.cxx
# End Source File
# Begin Source File

SOURCE=..\src\objectregistry.cxx
# End Source File
# Begin Source File

SOURCE=..\src\patternlayout.cxx
# End Source File
# Begin Source File

SOURCE=..\src\pointer.cxx
# End Source File
# Begin Source File

SOURCE=..\src\property.cxx
# End Source File
# Begin Source File

SOURCE=..\src\rootlogger.cxx
# End Source File
# Begin Source File

SOURCE=..\src\sleep.cxx
# End Source File
# Begin Source File

SOURCE="..\src\socket-win32.cxx"
# End Source File
# Begin Source File

SOURCE=..\src\socket.cxx
# End Source File
# Begin Source File

SOURCE=..\src\socketappender.cxx
# End Source File
# Begin Source File

SOURCE=..\src\socketbuffer.cxx
# End Source File
# Begin Source File

SOURCE=..\src\stringhelper.cxx
# End Source File
# Begin Source File

SOURCE=..\src\threads.cxx
# End Source File
# Begin Source File

SOURCE=..\src\timehelper.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\log4cplus\appender.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\spi\appenderattachable.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\helpers\appenderattachableimpl.h
# End Source File
# Begin Source File

SOURCE="..\include\log4cplus\config-macosx.h"
# End Source File
# Begin Source File

SOURCE="..\include\log4cplus\config-win32.h"
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\config.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\configurator.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\consoleappender.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\spi\factory.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\fileappender.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\spi\filter.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\fstreams.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\hierarchy.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\hierarchylocker.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\layout.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\logger.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\spi\loggerfactory.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\spi\loggerimpl.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\spi\loggingevent.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\loggingmacros.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\loglevel.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\helpers\loglog.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\helpers\logloguser.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\ndc.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\nteventlogappender.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\nullappender.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\spi\objectregistry.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\helpers\pointer.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\helpers\property.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\spi\rootlogger.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\helpers\sleep.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\helpers\socket.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\socketappender.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\helpers\socketbuffer.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\streams.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\helpers\stringhelper.h
# End Source File
# Begin Source File

SOURCE="..\include\log4cplus\helpers\thread-config.h"
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\helpers\threads.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\helpers\timehelper.h
# End Source File
# Begin Source File

SOURCE=..\include\log4cplus\tstring.h
# End Source File
# End Group
# End Target
# End Project
