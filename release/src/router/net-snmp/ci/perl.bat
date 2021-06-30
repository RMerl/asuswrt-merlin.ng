REM Download and install Perl
setlocal
echo on
if %1 == "" goto help
set PERL_VERSION=5.31.10
set INST_DRV=c:
set INST_TOP=c:\perl-msvc
rd /q /s %INST_TOP%
curl https://www.cpan.org/src/5.0/perl-%PERL_VERSION%.tar.gz -o perl-%PERL_VERSION%.tar.gz
if %errorlevel% neq 0 goto build_error
tar xzf perl-%PERL_VERSION%.tar.gz
if %errorlevel% neq 0 goto build_error
cd perl-%PERL_VERSION%\win32
if %errorlevel% neq 0 goto build_error
(echo CCTYPE=%1 && echo INST_DRV=%INST_DRV% && echo INST_TOP=%INST_TOP% && type Makefile | findstr /r /v "^CCTYPE" | findstr /r /v "^INST_DRV" | findstr /r /v "^INST_TOP") > Makefile2
if %errorlevel% neq 0 goto build_error
del Makefile
if %errorlevel% neq 0 goto build_error
ren Makefile2 Makefile
if %errorlevel% neq 0 goto build_error
findstr /r "^CCTYPE" Makefile
findstr /r "^INST_DRV" Makefile
findstr /r "^INST_TOP" Makefile
@rem For mt.exe
@rem dir /b /s "C:\Program Files (x86)" | findstr /i /e "\mt.exe"
set "PATH=%PATH%;C:\Program Files (x86)\Windows Kits\10\bin\10.0.18362.0\x64"
nmake "BUILDOPTEXTRA=/wd4244 /wd4267"
if %errorlevel% neq 0 goto build_error
nmake install
if %errorlevel% neq 0 goto build_error
set PATH=%INST_TOP%\bin;%PATH%
where perl
perl -v
cd ..\..
set INST_DRV=
set INST_TOP=
goto done

:build_error
endlocal
set e=%errorlevel%
exit /b %e%

:help
endlocal
echo "Compiler type argument has not been specified"
exit /b 1

:done
endlocal
