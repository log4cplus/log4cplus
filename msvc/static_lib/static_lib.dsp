# Microsoft Developer Studio Project File - Name="static_lib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=static_lib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "static_lib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "static_lib.mak" CFG="static_lib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "static_lib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "static_lib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "static_lib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "static_lib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "static_lib - Win32 Release"
# Name "static_lib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\appender.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\appenderattachableimpl.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\category.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\categoryimpl.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\consoleappender.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fileappender.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\hierarchy.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\layout.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\loglog.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\ndc.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\pointer.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\priority.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\threads.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\log4cplus\appender.h
# End Source File
# Begin Source File

SOURCE=..\..\include\log4cplus\spi\appenderattachable.h
# End Source File
# Begin Source File

SOURCE=..\..\include\log4cplus\helpers\appenderattachableimpl.h
# End Source File
# Begin Source File

SOURCE=..\..\include\log4cplus\category.h
# End Source File
# Begin Source File

SOURCE=..\..\include\log4cplus\spi\categoryfactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\log4cplus\spi\categoryimpl.h
# End Source File
# Begin Source File

SOURCE=..\..\include\log4cplus\config.h
# End Source File
# Begin Source File

SOURCE=..\..\include\log4cplus\consoleappender.h
# End Source File
# Begin Source File

SOURCE=..\..\include\log4cplus\fileappender.h
# End Source File
# Begin Source File

SOURCE=..\..\include\log4cplus\hierarchy.h
# End Source File
# Begin Source File

SOURCE=..\..\include\log4cplus\layout.h
# End Source File
# Begin Source File

SOURCE=..\..\include\log4cplus\spi\loggingevent.h
# End Source File
# Begin Source File

SOURCE=..\..\include\log4cplus\helpers\loglog.h
# End Source File
# Begin Source File

SOURCE=..\..\include\log4cplus\ndc.h
# End Source File
# Begin Source File

SOURCE=..\..\include\log4cplus\helpers\pointer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\log4cplus\priority.h
# End Source File
# Begin Source File

SOURCE=..\..\include\log4cplus\spi\rootcategory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\log4cplus\helpers\threadkey.h
# End Source File
# Begin Source File

SOURCE=..\..\include\log4cplus\helpers\threads.h
# End Source File
# End Group
# End Target
# End Project
