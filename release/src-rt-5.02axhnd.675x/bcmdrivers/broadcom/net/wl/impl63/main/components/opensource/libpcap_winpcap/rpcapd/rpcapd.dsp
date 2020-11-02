# Microsoft Developer Studio Project File - Name="rpcapd" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=rpcapd - Win32 Debug REMOTE DAG
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "rpcapd.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "rpcapd.mak" CFG="rpcapd - Win32 Debug REMOTE DAG"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "rpcapd - Win32 Debug REMOTE" (based on "Win32 (x86) Console Application")
!MESSAGE "rpcapd - Win32 Debug REMOTE DAG" (based on "Win32 (x86) Console Application")
!MESSAGE "rpcapd - Win32 Release REMOTE" (based on "Win32 (x86) Console Application")
!MESSAGE "rpcapd - Win32 Release REMOTE DAG" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "rpcapd - Win32 Debug REMOTE"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug_REMOTE"
# PROP BASE Intermediate_Dir "Debug_REMOTE"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_REMOTE"
# PROP Intermediate_Dir "Debug_REMOTE"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "../" /I "../bpf/" /I "../Win32/Include" /I "../../../Common/" /I "win32-pthreads" /D "_DEBUG" /D "_CONSOLE" /D "WIN32" /D "_MBCS" /D "HAVE_SNPRINTF" /D "HAVE_VSNPRINTF" /D "HAVE_REMOTE" /FR /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../" /I "../bpf/" /I "../Win32/Include" /I "../../../Common/" /I "win32-pthreads" /D "_DEBUG" /D "_CONSOLE" /D "WIN32" /D "_MBCS" /D "HAVE_SNPRINTF" /D "HAVE_VSNPRINTF" /D "HAVE_REMOTE" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ws2_32.lib wpcap.lib pthreadVC.lib packet.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"../../PRJ/Debug_REMOTE/rpcapd.exe" /pdbtype:sept /libpath:"../../lib" /libpath:"../../../Common/" /libpath:"win32-pthreads"
# ADD LINK32 ws2_32.lib wpcap.lib pthreadVC.lib packet.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"../../PRJ/Debug_REMOTE/rpcapd.exe" /pdbtype:sept /libpath:"../../../Common/" /libpath:"win32-pthreads" /libpath:"../../lib"

!ELSEIF  "$(CFG)" == "rpcapd - Win32 Debug REMOTE DAG"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug_REMOTE_DAG"
# PROP BASE Intermediate_Dir "Debug_REMOTE_DAG"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_REMOTE_DAG"
# PROP Intermediate_Dir "Debug_REMOTE_DAG"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "../" /I "../bpf/" /I "../Win32/Include" /I "../../../Common/" /I "win32-pthreads" /D "_DEBUG" /D "_CONSOLE" /D "WIN32" /D "_MBCS" /D "HAVE_SNPRINTF" /D "HAVE_VSNPRINTF" /D "HAVE_REMOTE" /FR /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../" /I "../bpf/" /I "../Win32/Include" /I "../../../Common/" /I "win32-pthreads" /D "_DEBUG" /D "_CONSOLE" /D "WIN32" /D "_MBCS" /D "HAVE_SNPRINTF" /D "HAVE_VSNPRINTF" /D "HAVE_REMOTE" /D "HAVE_DAG_API" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ws2_32.lib wpcap.lib pthreadVC.lib packet.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"../../PRJ/Debug_REMOTE_DAG/rpcapd.exe" /pdbtype:sept /libpath:"../../lib" /libpath:"../../../Common/" /libpath:"win32-pthreads"
# ADD LINK32 ws2_32.lib wpcap.lib pthreadVC.lib packet.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"../../PRJ/Debug_REMOTE_DAG/rpcapd.exe" /pdbtype:sept /libpath:"../../PRJ\Debug_REMOTE_DAG" /libpath:"../../../Common/" /libpath:"win32-pthreads"

!ELSEIF  "$(CFG)" == "rpcapd - Win32 Release REMOTE"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release_REMOTE"
# PROP BASE Intermediate_Dir "Release_REMOTE"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_REMOTE"
# PROP Intermediate_Dir "Release_REMOTE"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "../" /I "../bpf/" /I "../Win32/Include" /I "../../../Common/" /I "win32-pthreads" /D "NDEBUG" /D "_CONSOLE" /D "WIN32" /D "_MBCS" /D "HAVE_SNPRINTF" /D "HAVE_VSNPRINTF" /D "HAVE_REMOTE" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /I "../" /I "../bpf/" /I "../Win32/Include" /I "../../../Common/" /I "win32-pthreads" /D "NDEBUG" /D "_CONSOLE" /D "WIN32" /D "_MBCS" /D "HAVE_SNPRINTF" /D "HAVE_VSNPRINTF" /D "HAVE_REMOTE" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ws2_32.lib wpcap.lib pthreadVC.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /libpath:"../../lib" /libpath:"../../../Common/" /libpath:"win32-pthreads"
# ADD LINK32 ws2_32.lib wpcap.lib pthreadVC.lib packet.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:"../../PRJ/Release_REMOTE/rpcapd.pdb" /debug /machine:I386 /out:"../../PRJ/Release_REMOTE/rpcapd.exe" /libpath:"../../lib" /libpath:"../../../Common/" /libpath:"win32-pthreads" /opt:ref
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "rpcapd - Win32 Release REMOTE DAG"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release_REMOTE_DAG"
# PROP BASE Intermediate_Dir "Release_REMOTE_DAG"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_REMOTE_DAG"
# PROP Intermediate_Dir "Release_REMOTE_DAG"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "../" /I "../bpf/" /I "../Win32/Include" /I "../../../Common/" /I "win32-pthreads" /D "NDEBUG" /D "_CONSOLE" /D "WIN32" /D "_MBCS" /D "HAVE_SNPRINTF" /D "HAVE_VSNPRINTF" /D "HAVE_REMOTE" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /I "../" /I "../bpf/" /I "../Win32/Include" /I "../../../Common/" /I "win32-pthreads" /D "NDEBUG" /D "_CONSOLE" /D "WIN32" /D "_MBCS" /D "HAVE_SNPRINTF" /D "HAVE_VSNPRINTF" /D "HAVE_REMOTE" /D "HAVE_DAG_API" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ws2_32.lib wpcap.lib pthreadVC.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /libpath:"../../lib" /libpath:"../../../Common/" /libpath:"win32-pthreads"
# ADD LINK32 ws2_32.lib wpcap.lib pthreadVC.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"../../PRJ/Release_REMOTE_DAG/rpcapd.exe" /libpath:"../../PRJ\Release_REMOTE_DAG" /libpath:"../../../Common/" /libpath:"win32-pthreads"

!ENDIF 

# Begin Target

# Name "rpcapd - Win32 Debug REMOTE"
# Name "rpcapd - Win32 Debug REMOTE DAG"
# Name "rpcapd - Win32 Release REMOTE"
# Name "rpcapd - Win32 Release REMOTE DAG"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\daemon.c
# End Source File
# Begin Source File

SOURCE=.\fileconf.c
# End Source File
# Begin Source File

SOURCE="..\pcap-new.c"
# End Source File
# Begin Source File

SOURCE="..\pcap-remote.c"
# End Source File
# Begin Source File

SOURCE=.\rpcapd.c
# End Source File
# Begin Source File

SOURCE=..\sockutils.c
# End Source File
# Begin Source File

SOURCE=.\utils.c
# End Source File
# Begin Source File

SOURCE=.\version.rc
# End Source File
# Begin Source File

SOURCE=".\win32-svc.c"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\daemon.h
# End Source File
# Begin Source File

SOURCE=.\fileconf.h
# End Source File
# Begin Source File

SOURCE="..\pcap-remote.h"
# End Source File
# Begin Source File

SOURCE=.\rpcapd.h
# End Source File
# Begin Source File

SOURCE=..\sockutils.h
# End Source File
# Begin Source File

SOURCE=.\utils.h
# End Source File
# Begin Source File

SOURCE=".\win32-svc.h"
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
