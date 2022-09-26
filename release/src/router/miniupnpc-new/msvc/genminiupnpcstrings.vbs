' VBScript to generate miniupnpcstrings.h
' Copyright 2018 Thomas Bernard
'Set WshShell = CreateObject("WScript.Shell")
Set FSO = CreateObject("Scripting.FileSystemObject")
versionfile = "..\version"
infile = "..\miniupnpcstrings.h.in"
outfile = "..\miniupnpcstrings.h"
outfilerc = "..\rc_version.h"

On Error Resume Next

'Wscript.Echo revision

Err.Clear
Set f = FSO.OpenTextFile(versionfile, 1, False) ' 1 = Read
If Err.Number = 0 Then
	version = f.ReadLine
	f.Close
Else
	' Exit error
	WScript.Quit 1
End If

os_version = "0.0.0"
strComputer = "."
Set objWMIService = GetObject("winmgmts:" & "{impersonationLevel=impersonate}!\\" & strComputer & "\root\cimv2")
Set colOperatingSystems = objWMIService.ExecQuery ("Select * from Win32_OperatingSystem")
For Each objOperatingSystem in colOperatingSystems
    'Wscript.Echo objOperatingSystem.Caption & " -- " 
	os_version = objOperatingSystem.Version
Next

'Wscript.Echo os_version

Dim array
needWrite = True

' First Check if the file already contains the right versions
Err.Clear
Set f_in = FSO.OpenTextFile(outfile, 1, False)
If Err.Number = 0 Then
	old_version = ""
	old_os_version = ""
	Do Until f_in.AtEndOfStream
		line = f_in.ReadLine
		If Len(line) > 0 Then
			array = Split(line, " ")
			If UBound(array) >= 2 And array(0) = "#define" Then
				If array(1) = "OS_STRING" Then
					old_os_version = Replace(array(2), Chr(34), "")
				ElseIf array(1) = "MINIUPNPC_VERSION_STRING" Then
					old_version = Replace(array(2), Chr(34), "")
				End if
			End if
		End If
	Loop
	f_in.Close
	If old_version = version And old_os_version = "MSWindows/" & os_version Then
		needWrite = False
	Else
		needWrite = True
	End If
End If

If Not needWrite Then
	' check files dates
	Set fIn1 = FSO.GetFile(versionfile)
	Set fIn2 = FSO.GetFile(infile)
	Set fOut = FSO.GetFile(outfile)
	If DateDiff("s", fIn1.DateLastModified, fOut.DateLastModified) < 0 Then
		needWrite = True
	End If
	If DateDiff("s", fIn2.DateLastModified, fOut.DateLastModified) < 0 Then
		needWrite = True
	End If
End If

If Not needWrite Then
    ' nothing to do
	WScript.Quit 0
End if

' generate the file
Err.Clear
Set f_in = FSO.OpenTextFile(infile, 1, False)
If Err.Number = 0 Then
	Set f_out = FSO.OpenTextFile(outfile, 2, True) ' 2 = Write
	Do Until f_in.AtEndOfStream
		line = f_in.ReadLine
		If Len(line) > 0 Then
			array = Split(line, " ")
			If UBound(array) >= 2 And array(0) = "#define" Then
				If array(1) = "OS_STRING" Then
					line = "#define OS_STRING " & Chr(34) & "MSWindows/" & os_version & Chr(34)
				ElseIf array(1) = "MINIUPNPC_VERSION_STRING" Then
					line = "#define MINIUPNPC_VERSION_STRING " & Chr(34) & version & Chr(34)
				End if
			End if
		End If
		f_out.WriteLine line
	Loop
	f_in.Close
	f_out.Close
End If

Set f_out = FSO.OpenTextFile(outfilerc, 2, True) ' 2 = Write
f_out.WriteLine "#define LIBMINIUPNPC_DOTTED_VERSION " & Chr(34) & version & Chr(34)
ver = Split(version, ".")
f_out.WriteLine "#define LIBMINIUPNPC_MAJOR_VERSION " & ver(0)
f_out.WriteLine "#define LIBMINIUPNPC_MINOR_VERSION " & ver(1)
f_out.WriteLine "#define LIBMINIUPNPC_MICRO_VERSION " & ver(2)
f_out.Close
