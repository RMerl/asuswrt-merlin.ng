# how to install curl and libcurl

## Installing Binary Packages

Lots of people download binary distributions of curl and libcurl. This
document does not describe how to install curl or libcurl using such a binary
package. This document describes how to compile, build and install curl and
libcurl from source code.

## Building using vcpkg

You can download and install curl and libcurl using the [vcpkg](https://github.com/Microsoft/vcpkg/) dependency manager:

    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh
    ./vcpkg integrate install
    vcpkg install curl[tool]

The curl port in vcpkg is kept up to date by Microsoft team members and
community contributors. If the version is out of date, please [create an issue
or pull request](https://github.com/Microsoft/vcpkg) on the vcpkg repository.

## Building from git

If you get your code off a git repository instead of a release tarball, see
the `GIT-INFO` file in the root directory for specific instructions on how to
proceed.

# Unix

A normal Unix installation is made in three or four steps (after you have
unpacked the source archive):

    ./configure --with-openssl [--with-gnutls --with-wolfssl]
    make
    make test (optional)
    make install

(Adjust the configure line accordingly to use the TLS library you want.)

You probably need to be root when doing the last command.

Get a full listing of all available configure options by invoking it like:

    ./configure --help

If you want to install curl in a different file hierarchy than `/usr/local`,
specify that when running configure:

    ./configure --prefix=/path/to/curl/tree

If you have write permission in that directory, you can do 'make install'
without being root. An example of this would be to make a local install in
your own home directory:

    ./configure --prefix=$HOME
    make
    make install

The configure script always tries to find a working SSL library unless
explicitly told not to. If you have OpenSSL installed in the default search
path for your compiler/linker, you do not need to do anything special. If you
have OpenSSL installed in `/usr/local/ssl`, you can run configure like:

    ./configure --with-openssl

If you have OpenSSL installed somewhere else (for example, `/opt/OpenSSL`) and
you have pkg-config installed, set the pkg-config path first, like this:

    env PKG_CONFIG_PATH=/opt/OpenSSL/lib/pkgconfig ./configure --with-openssl

Without pkg-config installed, use this:

    ./configure --with-openssl=/opt/OpenSSL

If you insist on forcing a build without SSL support, you can run configure
like this:

    ./configure --without-ssl

If you have OpenSSL installed, but with the libraries in one place and the
header files somewhere else, you have to set the `LDFLAGS` and `CPPFLAGS`
environment variables prior to running configure. Something like this should
work:

    CPPFLAGS="-I/path/to/ssl/include" LDFLAGS="-L/path/to/ssl/lib" ./configure

If you have shared SSL libs installed in a directory where your runtime
linker does not find them (which usually causes configure failures), you can
provide this option to gcc to set a hard-coded path to the runtime linker:

    LDFLAGS=-Wl,-R/usr/local/ssl/lib ./configure --with-openssl

## Static builds

To force a static library compile, disable the shared library creation by
running configure like:

    ./configure --disable-shared

The configure script is primarily done to work with shared/dynamic third party
dependencies. When linking with shared libraries, the dependency "chain" is
handled automatically by the library loader - on all modern systems.

If you instead link with a static library, you need to provide all the
dependency libraries already at the link command line.

Figuring out all the dependency libraries for a given library is hard, as it
might involve figuring out the dependencies of the dependencies and they vary
between platforms and change between versions.

When using static dependencies, the build scripts will mostly assume that you,
the user, will provide all the necessary additional dependency libraries as
additional arguments in the build. With configure, by setting `LIBS` or
`LDFLAGS` on the command line.

Building statically is not for the faint of heart.

## Debug

If you are a curl developer and use gcc, you might want to enable more debug
options with the `--enable-debug` option.

curl can be built to use a whole range of libraries to provide various useful
services, and configure will try to auto-detect a decent default. But if you
want to alter it, you can select how to deal with each individual library.

## Select TLS backend

These options are provided to select the TLS backend to use.

 - AmiSSL: `--with-amissl`
 - BearSSL: `--with-bearssl`
 - GnuTLS: `--with-gnutls`.
 - mbedTLS: `--with-mbedtls`
 - NSS: `--with-nss`
 - OpenSSL: `--with-openssl` (also for BoringSSL, libressl and quictls)
 - rustls: `--with-rustls`
 - Schannel: `--with-schannel`
 - Secure Transport: `--with-secure-transport`
 - wolfSSL: `--with-wolfssl`

You can build curl with *multiple* TLS backends at your choice, but some TLS
backends cannot be combined: if you build with an OpenSSL fork (or wolfSSL),
you cannot add another OpenSSL fork (or wolfSSL) simply because they have
conflicting identical symbol names.

When you build with multiple TLS backends, you can select the active one at
run-time when curl starts up.

# Windows

## Building Windows DLLs and C runtime (CRT) linkage issues

 As a general rule, building a DLL with static CRT linkage is highly
 discouraged, and intermixing CRTs in the same app is something to avoid at
 any cost.

 Reading and comprehending Microsoft Knowledge Base articles KB94248 and
 KB140584 is a must for any Windows developer. Especially important is full
 understanding if you are not going to follow the advice given above.

 - [How To Use the C Run-Time](https://support.microsoft.com/help/94248/how-to-use-the-c-run-time)
 - [Run-Time Library Compiler Options](https://docs.microsoft.com/cpp/build/reference/md-mt-ld-use-run-time-library)
 - [Potential Errors Passing CRT Objects Across DLL Boundaries](https://docs.microsoft.com/cpp/c-runtime-library/potential-errors-passing-crt-objects-across-dll-boundaries)

If your app is misbehaving in some strange way, or it is suffering from memory
corruption, before asking for further help, please try first to rebuild every
single library your app uses as well as your app using the debug
multi-threaded dynamic C runtime.

 If you get linkage errors read section 5.7 of the FAQ document.

## MinGW32

Make sure that MinGW32's bin directory is in the search path, for example:

```cmd
set PATH=c:\mingw32\bin;%PATH%
```

then run `mingw32-make mingw32` in the root dir. There are other
make targets available to build libcurl with more features, use:

 - `mingw32-make mingw32-zlib` to build with Zlib support;
 - `mingw32-make mingw32-ssl-zlib` to build with SSL and Zlib enabled;
 - `mingw32-make mingw32-ssh2-ssl-zlib` to build with SSH2, SSL, Zlib;
 - `mingw32-make mingw32-ssh2-ssl-sspi-zlib` to build with SSH2, SSL, Zlib
   and SSPI support.

If you have any problems linking libraries or finding header files, be sure
to verify that the provided `Makefile.mk` files use the proper paths, and
adjust as necessary. It is also possible to override these paths with
environment variables, for example:

```cmd
set ZLIB_PATH=c:\zlib-1.2.12
set OPENSSL_PATH=c:\openssl-3.0.5
set LIBSSH2_PATH=c:\libssh2-1.10.0
```

It is also possible to build with other LDAP installations than MS LDAP;
currently it is possible to build with native Win32 OpenLDAP, or with the
*Novell CLDAP* SDK. If you want to use these you need to set these vars:

```cmd
set CPPFLAGS=-Ic:/openldap/include -DCURL_HAS_OPENLDAP_LDAPSDK
set LDFLAGS=-Lc:/openldap/lib
set LIBS=-lldap -llber
```

or for using the Novell SDK:

```cmd
set CPPFLAGS=-Ic:/openldapsdk/inc -DCURL_HAS_NOVELL_LDAPSDK
set LDFLAGS=-Lc:/openldapsdk/lib/mscvc
set LIBS=-lldapsdk -lldapssl -lldapx
```

If you want to enable LDAPS support then append `-ldaps` to the make target.

## Cygwin

Almost identical to the Unix installation. Run the configure script in the
curl source tree root with `sh configure`. Make sure you have the `sh`
executable in `/bin/` or you will see the configure fail toward the end.

Run `make`

## MS-DOS

Requires DJGPP in the search path and pointing to the Watt-32 stack via
`WATT_PATH=c:/djgpp/net/watt`.

Run `make -f Makefile.dist djgpp` in the root curl dir.

For build configuration options, please see the MinGW32 section.

Notes:

 - DJGPP 2.04 beta has a `sscanf()` bug so the URL parsing is not done
   properly. Use DJGPP 2.03 until they fix it.

 - Compile Watt-32 (and OpenSSL) with the same version of DJGPP. Otherwise
   things go wrong because things like FS-extensions and `errno` values have
   been changed between releases.

## AmigaOS

Run `make -f Makefile.dist amiga` in the root curl dir.

For build configuration options, please see the MinGW32 section.

## Disabling Specific Protocols in Windows builds

The configure utility, unfortunately, is not available for the Windows
environment, therefore, you cannot use the various disable-protocol options of
the configure utility on this platform.

You can use specific defines to disable specific protocols and features. See
[CURL-DISABLE](CURL-DISABLE.md) for the full list.

If you want to set any of these defines you have the following options:

 - Modify `lib/config-win32.h`
 - Modify `lib/curl_setup.h`
 - Modify `winbuild/Makefile.vc`
 - Modify the "Preprocessor Definitions" in the libcurl project

Note: The pre-processor settings can be found using the Visual Studio IDE
under "Project -> Properties -> Configuration Properties -> C/C++ ->
Preprocessor".

## Using BSD-style lwIP instead of Winsock TCP/IP stack in Win32 builds

In order to compile libcurl and curl using BSD-style lwIP TCP/IP stack it is
necessary to make the definition of the preprocessor symbol `USE_LWIPSOCK`
visible to libcurl and curl compilation processes. To set this definition you
have the following alternatives:

 - Modify `lib/config-win32.h` and `src/config-win32.h`
 - Modify `winbuild/Makefile.vc`
 - Modify the "Preprocessor Definitions" in the libcurl project

Note: The pre-processor settings can be found using the Visual Studio IDE
under "Project -> Properties -> Configuration Properties -> C/C++ ->
Preprocessor".

Once that libcurl has been built with BSD-style lwIP TCP/IP stack support, in
order to use it with your program it is mandatory that your program includes
lwIP header file `<lwip/opt.h>` (or another lwIP header that includes this)
before including any libcurl header. Your program does not need the
`USE_LWIPSOCK` preprocessor definition which is for libcurl internals only.

Compilation has been verified with lwIP 1.4.0.

This BSD-style lwIP TCP/IP stack support must be considered experimental given
that it has been verified that lwIP 1.4.0 still needs some polish, and libcurl
might yet need some additional adjustment.

## Important static libcurl usage note

When building an application that uses the static libcurl library on Windows,
you must add `-DCURL_STATICLIB` to your `CFLAGS`. Otherwise the linker will
look for dynamic import symbols.

## Legacy Windows and SSL

Schannel (from Windows SSPI), is the native SSL library in Windows. However,
Schannel in Windows <= XP is unable to connect to servers that
no longer support the legacy handshakes and algorithms used by those
versions. If you will be using curl in one of those earlier versions of
Windows you should choose another SSL backend such as OpenSSL.

# Apple Platforms (macOS, iOS, tvOS, watchOS, and their simulator counterparts)

On modern Apple operating systems, curl can be built to use Apple's SSL/TLS
implementation, Secure Transport, instead of OpenSSL. To build with Secure
Transport for SSL/TLS, use the configure option `--with-secure-transport`.

When Secure Transport is in use, the curl options `--cacert` and `--capath`
and their libcurl equivalents, will be ignored, because Secure Transport uses
the certificates stored in the Keychain to evaluate whether or not to trust
the server. This, of course, includes the root certificates that ship with the
OS. The `--cert` and `--engine` options, and their libcurl equivalents, are
currently unimplemented in curl with Secure Transport.

In general, a curl build for an Apple `ARCH/SDK/DEPLOYMENT_TARGET` combination
can be taken by providing appropriate values for `ARCH`, `SDK`, `DEPLOYMENT_TARGET`
below and running the commands:

```bash
# Set these three according to your needs
export ARCH=x86_64
export SDK=macosx
export DEPLOYMENT_TARGET=10.8

export CFLAGS="-arch $ARCH -isysroot $(xcrun -sdk $SDK --show-sdk-path) -m$SDK-version-min=$DEPLOYMENT_TARGET"
./configure --host=$ARCH-apple-darwin --prefix $(pwd)/artifacts --with-secure-transport
make -j8
make install
```

Above will build curl for macOS platform with `x86_64` architecture and `10.8` as deployment target.

Here is an example for iOS device:

```bash
export ARCH=arm64
export SDK=iphoneos
export DEPLOYMENT_TARGET=11.0

export CFLAGS="-arch $ARCH -isysroot $(xcrun -sdk $SDK --show-sdk-path) -m$SDK-version-min=$DEPLOYMENT_TARGET"
./configure --host=$ARCH-apple-darwin --prefix $(pwd)/artifacts --with-secure-transport
make -j8
make install
```

Another example for watchOS simulator for macs with Apple Silicon:

```bash
export ARCH=arm64
export SDK=watchsimulator
export DEPLOYMENT_TARGET=5.0

export CFLAGS="-arch $ARCH -isysroot $(xcrun -sdk $SDK --show-sdk-path) -m$SDK-version-min=$DEPLOYMENT_TARGET"
./configure --host=$ARCH-apple-darwin --prefix $(pwd)/artifacts --with-secure-transport
make -j8
make install
```

In all above, the built libraries and executables can be found in the
`artifacts` folder.

# Android

When building curl for Android it's recommended to use a Linux/macOS environment
since using curl's `configure` script is the easiest way to build curl
for Android. Before you can build curl for Android, you need to install the
Android NDK first. This can be done using the SDK Manager that is part of
Android Studio. Once you have installed the Android NDK, you need to figure out
where it has been installed and then set up some environment variables before
launching `configure`. On macOS, those variables could look like this to compile
for `aarch64` and API level 29:

```bash
export ANDROID_NDK_HOME=~/Library/Android/sdk/ndk/25.1.8937393 # Point into your NDK.
export HOST_TAG=darwin-x86_64 # Same tag for Apple Silicon. Other OS values here: https://developer.android.com/ndk/guides/other_build_systems#overview
export TOOLCHAIN=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/$HOST_TAG
export AR=$TOOLCHAIN/bin/llvm-ar
export AS=$TOOLCHAIN/bin/llvm-as
export CC=$TOOLCHAIN/bin/aarch64-linux-android21-clang
export CXX=$TOOLCHAIN/bin/aarch64-linux-android21-clang++
export LD=$TOOLCHAIN/bin/ld
export RANLIB=$TOOLCHAIN/bin/llvm-ranlib
export STRIP=$TOOLCHAIN/bin/llvm-strip
```

When building on Linux or targeting other API levels or architectures, you need
to adjust those variables accordingly. After that you can build curl like this:

    ./configure --host aarch64-linux-android --with-pic --disable-shared

Note that this will not give you SSL/TLS support. If you need SSL/TLS, you have
to build curl against a SSL/TLS layer, e.g. OpenSSL, because it's impossible for
curl to access Android's native SSL/TLS layer. To build curl for Android using
OpenSSL, follow the OpenSSL build instructions and then install `libssl.a` and
`libcrypto.a` to `$TOOLCHAIN/sysroot/usr/lib` and copy `include/openssl` to
`$TOOLCHAIN/sysroot/usr/include`. Now you can build curl for Android using
OpenSSL like this:

```bash
LIBS="-lssl -lcrypto -lc++" # For OpenSSL/BoringSSL. In general, you'll need to the SSL/TLS layer's transtive dependencies if you're linking statically.
./configure --host aarch64-linux-android --with-pic --disable-shared --with-openssl="$TOOLCHAIN/sysroot/usr"
```

# IBM i

For IBM i (formerly OS/400), you can use curl in two different ways:

- Natively, running in the **ILE**. The obvious use is being able to call curl
  from ILE C or RPG applications.
  - You will need to build this from source. See `packages/OS400/README` for
    the ILE specific build instructions.
- In the **PASE** environment, which runs AIX programs. curl will be built as
  it would be on AIX.
  - IBM provides builds of curl in their Yum repository for PASE software.
  - To build from source, follow the Unix instructions.

There are some additional limitations and quirks with curl on this platform;
they affect both environments.

## Multi-threading notes

By default, jobs in IBM i will not start with threading enabled. (Exceptions
include interactive PASE sessions started by `QP2TERM` or SSH.) If you use
curl in an environment without threading when options like asynchronous DNS
were enabled, you will get messages like:

```
getaddrinfo() thread failed to start
```

Do not panic. curl and your program are not broken. You can fix this by:

- Set the environment variable `QIBM_MULTI_THREADED` to `Y` before starting
  your program. This can be done at whatever scope you feel is appropriate.
- Alternatively, start the job with the `ALWMLTTHD` parameter set to `*YES`.

# Cross compile

Download and unpack the curl package.

`cd` to the new directory. (e.g. `cd curl-7.12.3`)

Set environment variables to point to the cross-compile toolchain and call
configure with any options you need. Be sure and specify the `--host` and
`--build` parameters at configuration time. The following script is an example
of cross-compiling for the IBM 405GP PowerPC processor using the toolchain on
Linux.

```bash
#! /bin/sh

export PATH=$PATH:/opt/hardhat/devkit/ppc/405/bin
export CPPFLAGS="-I/opt/hardhat/devkit/ppc/405/target/usr/include"
export AR=ppc_405-ar
export AS=ppc_405-as
export LD=ppc_405-ld
export RANLIB=ppc_405-ranlib
export CC=ppc_405-gcc
export NM=ppc_405-nm

./configure --target=powerpc-hardhat-linux
    --host=powerpc-hardhat-linux
    --build=i586-pc-linux-gnu
    --prefix=/opt/hardhat/devkit/ppc/405/target/usr/local
    --exec-prefix=/usr/local
```

You may also need to provide a parameter like `--with-random=/dev/urandom` to
configure as it cannot detect the presence of a random number generating
device for a target system. The `--prefix` parameter specifies where curl
will be installed. If `configure` completes successfully, do `make` and `make
install` as usual.

In some cases, you may be able to simplify the above commands to as little as:

    ./configure --host=ARCH-OS

# REDUCING SIZE

There are a number of configure options that can be used to reduce the size of
libcurl for embedded applications where binary size is an important factor.
First, be sure to set the `CFLAGS` variable when configuring with any relevant
compiler optimization flags to reduce the size of the binary. For gcc, this
would mean at minimum the -Os option, and potentially the `-march=X`,
`-mdynamic-no-pic` and `-flto` options as well, e.g.

    ./configure CFLAGS='-Os' LDFLAGS='-Wl,-Bsymbolic'...

Note that newer compilers often produce smaller code than older versions
due to improved optimization.

Be sure to specify as many `--disable-` and `--without-` flags on the
configure command-line as you can to disable all the libcurl features that you
know your application is not going to need. Besides specifying the
`--disable-PROTOCOL` flags for all the types of URLs your application will not
use, here are some other flags that can reduce the size of the library by
disabling support for some feature:

 - `--disable-alt-svc` (HTTP Alt-Svc)
 - `--disable-ares` (the C-ARES DNS library)
 - `--disable-cookies` (HTTP cookies)
 - `--disable-crypto-auth` (cryptographic authentication)
 - `--disable-dateparse` (date parsing for time conditionals)
 - `--disable-dnsshuffle` (internal server load spreading)
 - `--disable-doh` (DNS-over-HTTP)
 - `--disable-get-easy-options` (lookup easy options at runtime)
 - `--disable-hsts` (HTTP Strict Transport Security)
 - `--disable-http-auth` (all HTTP authentication)
 - `--disable-ipv6` (IPv6)
 - `--disable-libcurl-option` (--libcurl C code generation support)
 - `--disable-manual` (built-in documentation)
 - `--disable-netrc`  (.netrc file)
 - `--disable-ntlm-wb` (NTLM WinBind)
 - `--disable-progress-meter` (graphical progress meter in library)
 - `--disable-proxy` (HTTP and SOCKS proxies)
 - `--disable-pthreads` (multi-threading)
 - `--disable-socketpair` (socketpair for asynchronous name resolving)
 - `--disable-threaded-resolver`  (threaded name resolver)
 - `--disable-tls-srp` (Secure Remote Password authentication for TLS)
 - `--disable-unix-sockets` (UNIX sockets)
 - `--disable-verbose` (eliminates debugging strings and error code strings)
 - `--disable-versioned-symbols` (versioned symbols)
 - `--enable-symbol-hiding` (eliminates unneeded symbols in the shared library)
 - `--without-brotli` (Brotli on-the-fly decompression)
 - `--without-libpsl` (Public Suffix List in cookies)
 - `--without-nghttp2` (HTTP/2 using nghttp2)
 - `--without-ngtcp2` (HTTP/2 using ngtcp2)
 - `--without-zstd` (Zstd on-the-fly decompression)
 - `--without-libidn2` (internationalized domain names)
 - `--without-librtmp` (RTMP)
 - `--without-ssl` (SSL/TLS)
 - `--without-zlib` (on-the-fly decompression)

The GNU compiler and linker have a number of options that can reduce the
size of the libcurl dynamic libraries on some platforms even further.
Specify them by providing appropriate `CFLAGS` and `LDFLAGS` variables on
the configure command-line, e.g.

    CFLAGS="-Os -ffunction-sections -fdata-sections
            -fno-unwind-tables -fno-asynchronous-unwind-tables -flto"
    LDFLAGS="-Wl,-s -Wl,-Bsymbolic -Wl,--gc-sections"

Be sure also to strip debugging symbols from your binaries after compiling
using 'strip' (or the appropriate variant if cross-compiling). If space is
really tight, you may be able to remove some unneeded sections of the shared
library using the -R option to objcopy (e.g. the .comment section).

Using these techniques it is possible to create a basic HTTP-only libcurl
shared library for i386 Linux platforms that is only 133 KiB in size
(as of libcurl version 7.80.0, using gcc 11.2.0).

You may find that statically linking libcurl to your application will result
in a lower total size than dynamically linking.

Note that the curl test harness can detect the use of some, but not all, of
the `--disable` statements suggested above. Use will cause tests relying on
those features to fail. The test harness can be manually forced to skip the
relevant tests by specifying certain key words on the `runtests.pl` command
line. Following is a list of appropriate key words for those configure options
that are not automatically detected:

 - `--disable-cookies`          !cookies
 - `--disable-dateparse`        !RETRY-AFTER !`CURLOPT_TIMECONDITION` !`CURLINFO_FILETIME` !`If-Modified-Since` !`curl_getdate` !`-z`
 - `--disable-libcurl-option`   !`--libcurl`
 - `--disable-verbose`          !verbose\ logs

# PORTS

This is a probably incomplete list of known CPU architectures and operating
systems that curl has been compiled for. If you know a system curl compiles
and runs on, that is not listed, please let us know!

## 92 Operating Systems

    AIX, AmigaOS, Android, Aros, BeOS, Blackberry 10, Blackberry Tablet OS,
    Cell OS, Chrome OS, Cisco IOS, Cygwin, DG/UX, Dragonfly BSD, DR DOS, eCOS,
    FreeBSD, FreeDOS, FreeRTOS, Fuchsia, Garmin OS, Genode, Haiku, HardenedBSD,
    HP-UX, Hurd, Illumos, Integrity, iOS, ipadOS, IRIX, Linux, Lua RTOS,
    Mac OS 9, macOS, Mbed, Micrium, MINIX, MorphOS, MPE/iX, MS-DOS, NCR MP-RAS,
    NetBSD, Netware, Nintendo Switch, NonStop OS, NuttX, Omni OS, OpenBSD,
    OpenStep, Orbis OS, OS/2, OS/400, OS21, Plan 9, PlayStation Portable, QNX,
    Qubes OS, ReactOS, Redox, RICS OS, RTEMS, Sailfish OS, SCO Unix, Serenity,
    SINIX-Z, Solaris, SunOS, Syllable OS, Symbian, Tizen, TPF, Tru64, tvOS,
    ucLinux, Ultrix, UNICOS, UnixWare, VMS, vxWorks, watchOS, WebOS,
    Wii system software, Windows, Windows CE, Xbox System, Xenix, Zephyr,
    z/OS, z/TPF, z/VM, z/VSE

## 26 CPU Architectures

    Alpha, ARC, ARM, AVR32, CompactRISC, Elbrus, ETRAX, HP-PA, Itanium,
    LoongArch, m68k, m88k, MicroBlaze, MIPS, Nios, OpenRISC, POWER, PowerPC,
    RISC-V, s390, SH4, SPARC, Tilera, VAX, x86, Xtensa
