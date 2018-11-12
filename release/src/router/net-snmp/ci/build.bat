echo "Build type %BUILD%"
@echo on
goto %BUILD%
echo "Error: unknown build type %BUILD%"
goto eof

:MSVCDYNAMIC64
call "ci\openssl.bat"
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
cd win32
perl Configure --config=release --with-sdk --with-ipv6 --with-winextdll --linktype=dynamic --with-ssl --with-sslincdir=C:\OpenSSL-Win64\include --with-ssllibdir=C:\OpenSSL-Win64\lib\vc
nmake
goto eof

:MSVCSTATIC64
call "ci\openssl.bat"
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
cd win32
perl Configure --config=release --with-sdk --with-ipv6 --with-winextdll --linktype=static --with-ssl --with-sslincdir=C:\OpenSSL-Win64\include --with-ssllibdir=C:\OpenSSL-Win64\lib\vc

nmake
goto eof

:MinGW32
REM to do - install MinGW32 first.
C:\mingw\msys\1.0\usr\bin\bash --login -c 'set -x; cd "${APPVEYOR_BUILD_FOLDER}"; ci/build.sh'
goto eof

:MinGW64
C:\msys64\usr\bin\bash --login -c 'set -x; cd "${APPVEYOR_BUILD_FOLDER}"; ci/build.sh'
goto eof

:Cygwin32
c:\cygwin\setup-x86.exe --quiet-mode --no-shortcuts --only-site --site "%CYG_MIRROR%" --packages openssl-devel > NUL
c:\cygwin\bin\bash --login -c 'set -x; cd "${APPVEYOR_BUILD_FOLDER}"; ci/build.sh'
goto eof

:Cygwin64
c:\cygwin64\setup-x86_64.exe --quiet-mode --no-shortcuts --only-site --site "%CYG_MIRROR%" --packages openssl-devel > NUL
c:\cygwin64\bin\bash --login -c 'set -x; cd "${APPVEYOR_BUILD_FOLDER}"; ci/build.sh'
goto eof

:eof
