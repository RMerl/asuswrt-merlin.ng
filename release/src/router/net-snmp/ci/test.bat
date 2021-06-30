@echo on
goto %BUILD%
echo "Error: unknown build type %BUILD%"
goto eof

:MSVCDYNAMIC64
goto eof

:MSVCSTATIC64
goto eof

:INSTALLER
goto eof

:MinGW32
goto eof

:MSYS2
goto eof

:MinGW64
rem C:\msys64\usr\bin\bash --login -c 'set -x; cd "${APPVEYOR_BUILD_FOLDER}"; ci/net-snmp-run-tests'
goto eof

:Cygwin32
goto eof
c:\cygwin\bin\bash --login -c 'set -x; cd "${APPVEYOR_BUILD_FOLDER}"; ci/net-snmp-run-tests'
goto eof

:Cygwin64
goto eof
c:\cygwin64\bin\bash --login -c 'set -x; cd "${APPVEYOR_BUILD_FOLDER}"; ci/net-snmp-run-tests'
goto eof

:eof
