echo "Build type %BUILD%"
@echo on
@rem dir /b /s "C:\Program Files (x86)" | findstr /i /e "\vcvars64.bat"
set "VCVARSPATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build"
goto %BUILD%
echo "Error: unknown build type %BUILD%"
goto eof

:MSVCDYNAMIC64
call "%VCVARSPATH%\vcvars64.bat"
call ci/perl.bat MSVC142
if %errorlevel% neq 0 exit /b %errorlevel%
set PATH=c:\perl-msvc\bin;%PATH%
cd win32
perl Configure --config=release --enable-blumenthal-aes --with-sdk --with-ipv6 --with-winextdll --linktype=dynamic --with-ssl --with-sslincdir=C:\OpenSSL-Win64\include --with-ssllibdir=C:\OpenSSL-Win64\lib\vc
if %errorlevel% neq 0 exit /b %errorlevel%
nmake /nologo
if %errorlevel% neq 0 exit /b %errorlevel%
nmake /nologo perl
if %errorlevel% neq 0 exit /b %errorlevel%
nmake /nologo perl_test
if %errorlevel% neq 0 exit /b %errorlevel%
cd ..
goto eof

:MSVCSTATIC64
call "%VCVARSPATH%\vcvars64.bat"
call ci/perl.bat MSVC142
if %errorlevel% neq 0 exit /b %errorlevel%
set PATH=c:\perl-msvc\bin;%PATH%
cd win32
perl Configure --config=release --enable-blumenthal-aes --with-sdk --with-ipv6 --with-winextdll --linktype=static --with-ssl --with-sslincdir=C:\OpenSSL-Win64\include --with-ssllibdir=C:\OpenSSL-Win64\lib\vc
if %errorlevel% neq 0 exit /b %errorlevel%
nmake /nologo
if %errorlevel% neq 0 exit /b %errorlevel%
cd ..
goto eof

:INSTALLER
call "%VCVARSPATH%\vcvars64.bat"
call ci/perl.bat MSVC142
if %errorlevel% neq 0 exit /b %errorlevel%
set PATH=c:\perl-msvc\bin;%PATH%
set OPENSSLDIR=C:\OpenSSL-Win64
set PATH=%PATH%;C:\cygwin64\bin
perl win32\dist\build-binary.pl
if %errorlevel% neq 0 goto installer_build_error
mkdir installer
copy c:\usr\*.exe installer
goto eof

:installer_build_error
set e=%errorlevel%
type win32\make.out
exit /b %e%
goto eof

:MinGW32
rem MinGW is not present in the Visual Studio 2017 image. See also
rem https://www.appveyor.com/docs/windows-images-software/.
if exist C:\mingw goto MinGW32-get
mkdir C:\mingw
curl --no-alpn -L "https://osdn.net/dl/mingw/mingw-get-0.6.3-mingw32-pre-20170905-1-bin.zip" -o C:\mingw\mingw-get.zip
unzip C:\mingw\mingw-get.zip -d C:\mingw
:MinGW32-get
C:\mingw\bin\mingw-get install mingw32-binutils-bin mingw32-gcc-bin mingw32-gcc-dev mingw32-w32api-dev msys-autoconf-bin msys-automake-bin msys-bash-bin msys-core-bin msys-coreutils-bin msys-file-bin msys-gawk-bin msys-grep-bin msys-libncurses-dev msys-libopenssl-dev msys-m4-bin msys-make-bin msys-openssl-bin msys-perl-bin msys-sed-bin msys-tar-bin
set MSYSTEM=MINGW32
C:\mingw\msys\1.0\bin\bash --login -c 'set -x; cd "${APPVEYOR_BUILD_FOLDER}"; ci/build.sh'
goto eof

:MSYS2
set MSYSTEM=MSYS
C:\msys64\usr\bin\bash --login -c 'set -x; cd "${APPVEYOR_BUILD_FOLDER}"; ci/build.sh'
goto eof

:MinGW64
set MSYSTEM=MINGW64
C:\msys64\usr\bin\bash --login -c 'set -x; cd "${APPVEYOR_BUILD_FOLDER}"; ci/build.sh'
goto eof

:Cygwin32
c:\cygwin\setup-x86.exe --quiet-mode --no-shortcuts --only-site --site "%CYG_MIRROR%" --packages openssl-devel > NUL
c:\cygwin\setup-x86.exe --quiet-mode --no-shortcuts --only-site --site "%CYG_MIRROR%" --packages python38-devel > NUL
c:\cygwin\bin\bash --login -c 'set -x; cd "${APPVEYOR_BUILD_FOLDER}"; ci/build.sh'
goto eof

:Cygwin64
c:\cygwin64\setup-x86_64.exe --quiet-mode --no-shortcuts --only-site --site "%CYG_MIRROR%" --packages openssl-devel > NUL
c:\cygwin64\setup-x86_64.exe --quiet-mode --no-shortcuts --only-site --site "%CYG_MIRROR%" --packages python38-devel > NUL
c:\cygwin64\bin\bash --login -c 'set -x; cd "${APPVEYOR_BUILD_FOLDER}"; ci/build.sh'
goto eof

:eof
