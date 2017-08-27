Introduction to CMake
---------------------

CMake is a multi-platform build tool that can generate build files for many
different target platforms. See more info at http://www.cmake.org

CMake also allows/recommends you to do "out of source"-builds, that is,
the build files are separated from your sources, so there is no need to
create elaborate clean scripts to get a clean source tree, instead you
simply remove your build directory.

Libwebsockets has been tested to build successfully on the following platforms
with SSL support (both OpenSSL/wolfSSL):

- Windows (Visual Studio)
- Windows (MinGW)
- Linux (x86 and ARM)
- OSX
- NetBSD

Building the library and test apps
----------------------------------

The project settings used by CMake to generate the platform specific build
files is called [CMakeLists.txt](CMakeLists.txt). CMake then uses one of its "Generators" to
output a Visual Studio project or Make file for instance. To see a list of
the available generators for your platform, simply run the "cmake" command.

Note that by default OpenSSL will be linked, if you don't want SSL support
see below on how to toggle compile options.

Building on Unix:
-----------------

1. Install CMake 2.8 or greater: http://cmake.org/cmake/resources/software.html
   (Most Unix distributions comes with a packaged version also)

2. Install OpenSSL.

3. Generate the build files (default is Make files):

    ```bash
	$ cd /path/to/src
	$ mkdir build
	$ cd build
	$ cmake ..
    ```

	(**NOTE**: The `build/`` directory can have any name and be located anywhere
	 on your filesystem, and that the argument `..` given to cmake is simply
	 the source directory of **libwebsockets** containing the [CMakeLists.txt](CMakeLists.txt)
	 project file. All examples in this file assumes you use "..")

	**NOTE2**:
	A common option you may want to give is to set the install path, same
	as --prefix= with autotools.  It defaults to /usr/local.
	You can do this by, eg

    ```bash
	$ cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
    ```

	**NOTE3**:
	On machines that want libraries in lib64, you can also add the
	following to the cmake line

    ```bash
	-DLIB_SUFFIX=64
    ```

	**NOTE4**:
	If you are building against a non-distro OpenSSL (eg, in order to get
	access to ALPN support only in newer OpenSSL versions) the nice way to
	express that in one cmake command is eg,

    ```bash
	$ cmake .. -DOPENSSL_ROOT_DIR=/usr/local/ssl \
		 -DCMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE=/usr/local/ssl \
		 -DLWS_WITH_HTTP2=1
    ```

	When you run the test apps using non-distro SSL, you have to force them
	to use your libs, not the distro ones

    ```bash
	$ LD_LIBRARY_PATH=/usr/local/ssl/lib libwebsockets-test-server --ssl
    ```

4. Finally you can build using the generated Makefile:

    ```bash
	$ make
    ```

Quirk of cmake
--------------

When changing cmake options, for some reason the only way to get it to see the
changes sometimes is delete the contents of your build directory and do the
cmake from scratch.

Building on Windows (Visual Studio)
-----------------------------------
1. Install CMake 2.6 or greater: http://cmake.org/cmake/resources/software.html

2. Install OpenSSL binaries. http://www.openssl.org/related/binaries.html

   (**NOTE**: Preferably in the default location to make it easier for CMake to find them)

   **NOTE2**: 
   Be sure that OPENSSL_CONF environment variable is defined and points at 
   <OpenSSL install location>\bin\openssl.cfg
	 
3. Generate the Visual studio project by opening the Visual Studio cmd prompt:

   ```bash
   cd <path to src>
   md build
   cd build
   cmake -G "Visual Studio 10" ..
   ```

   (**NOTE**: There is also a cmake-gui available on Windows if you prefer that)
   
   **NOTE2**:
   See this link to find out the version number corresponding to your Visual Studio edition:
   http://superuser.com/a/194065

4. Now you should have a generated Visual Studio Solution in  your
   `<path to src>/build` directory, which can be used to build.

Building on Windows (MinGW)
---------------------------
1. Install MinGW: http://sourceforge.net/projects/mingw/files

   (**NOTE**: Preferably in the default location C:\MinGW)

2. Fix up MinGW headers

   a) Add the following lines to C:\MinGW\include\winsock2.h:
   
   ```c
   #if(_WIN32_WINNT >= 0x0600)

   typedef struct pollfd {

       SOCKET  fd;
       SHORT   events;
       SHORT   revents;

   } WSAPOLLFD, *PWSAPOLLFD, FAR *LPWSAPOLLFD;

   WINSOCK_API_LINKAGE int WSAAPI WSAPoll(LPWSAPOLLFD fdArray, ULONG fds, INT timeout);

   #endif // (_WIN32_WINNT >= 0x0600)
   ```

   b) Create C:\MinGW\include\mstcpip.h and copy and paste the content from following link into it:
    
   http://wine-unstable.sourcearchive.com/documentation/1.1.32/mstcpip_8h-source.html

3. Install CMake 2.6 or greater: http://cmake.org/cmake/resources/software.html

4. Install OpenSSL binaries. http://www.openssl.org/related/binaries.html

   (**NOTE**: Preferably in the default location to make it easier for CMake to find them)

   **NOTE2**: 
   Be sure that OPENSSL_CONF environment variable is defined and points at 
   <OpenSSL install location>\bin\openssl.cfg

5. Generate the build files (default is Make files) using MSYS shell:

   ```bash
   $ cd /drive/path/to/src
   $ mkdir build
   $ cd build
   $ cmake -G "MSYS Makefiles" -DCMAKE_INSTALL_PREFIX=C:/MinGW ..
   ```

   (**NOTE**: The `build/`` directory can have any name and be located anywhere
    on your filesystem, and that the argument `..` given to cmake is simply
    the source directory of **libwebsockets** containing the [CMakeLists.txt](CMakeLists.txt)
    project file. All examples in this file assumes you use "..")

   **NOTE2**:
   To generate build files allowing to create libwebsockets binaries with debug information
   set the CMAKE_BUILD_TYPE flag to DEBUG:

   ```bash
   $ cmake -G "MSYS Makefiles" -DCMAKE_INSTALL_PREFIX=C:/MinGW -DCMAKE_BUILD_TYPE=DEBUG ..
   ```

6. Finally you can build using the generated Makefile and get the results deployed into your MinGW installation:

   ```bash
   $ make
   $ make install
   ```

Setting compile options
-----------------------

To set compile time flags you can either use one of the CMake gui applications
or do it via command line.

Command line
------------
To list avaialable options (ommit the H if you don't want the help text):

	cmake -LH ..

Then to set an option and build (for example turn off SSL support):

	cmake -DLWS_WITH_SSL=0 ..
or
	cmake -DLWS_WITH_SSL:BOOL=OFF ..

Building on mbed3
-----------------
MBED3 is a non-posix embedded OS targeted on Cortex M class chips.

https://www.mbed.com/

It's quite unlike any other Posixy platform since the OS is linked statically
in with lws to form one binary.

At the minute server-only is supported and due to bugs in mbed3 network support,
the port is of alpha quality.  However it can serve the test html, favicon.ico
and logo png and may be able to make ws connections.  The binary for that
including the OS, test app, lws and all the assets is only 117KB.

0) Today mbed3 only properly works on FRDM K64F $35 Freescale Dev Board with
1MB Flash, 256KB SRAM and Ethernet.

http://www.freescale.com/products/arm-processors/kinetis-cortex-m/k-series/k6x-ethernet-mcus/freescale-freedom-development-platform-for-kinetis-k64-k63-and-k24-mcus:FRDM-K64F

1) Get a working mbed3 environment with arm-none-eabi-cs toolchain
(available in Fedora, Ubuntu and other distros)

2) Confirm you can build things using yotta by following the getting started guide here

https://docs.mbed.com/docs/getting-started-mbed-os/en/latest/

3)

git clone https://github.com/warmcat/lws-test-server

and cd into it

4) mkdir -p yotta_modules ; cd yotta_modules

5) git clone https://github.com/warmcat/libwebsockets ; mv libwebsockets websockets ; cd ..

6) yotta target frdm-k64f-gcc

7) yotta install

8) yotta build


Unix GUI
--------
If you have a curses-enabled build you simply type:
(not all packages include this, my debian install does not for example).

	ccmake

Windows GUI
-----------
On windows CMake comes with a gui application:
	Start -> Programs -> CMake -> CMake (cmake-gui)

wolfSSL/CyaSSL replacement for OpenSSL
--------------------------------------
wolfSSL/CyaSSL is a lightweight SSL library targeted at embedded systems:
https://www.wolfssl.com/wolfSSL/Products-wolfssl.html

It contains a OpenSSL compatibility layer which makes it possible to pretty
much link to it instead of OpenSSL, giving a much smaller footprint.

**NOTE**: wolfssl needs to be compiled using the `--enable-opensslextra` flag for
this to work.

Compiling libwebsockets with wolfSSL
------------------------------------

```bash
cmake .. -DLWS_USE_WOLFSSL=1 \
	 -DLWS_WOLFSSL_INCLUDE_DIRS=/path/to/wolfssl \
	 -DLWS_WOLFSSL_LIBRARIES=/path/to/wolfssl/wolfssl.a ..
```

**NOTE**: On windows use the .lib file extension for `LWS_WOLFSSL_LIBRARIES` instead.

Compiling libwebsockets with CyaSSL
-----------------------------------

```bash
cmake .. -DLWS_USE_CYASSL=1 \
	 -DLWS_CYASSL_INCLUDE_DIRS=/path/to/cyassl \
	 -DLWS_CYASSL_LIBRARIES=/path/to/wolfssl/cyassl.a ..
```

**NOTE**: On windows use the .lib file extension for `LWS_CYASSL_LIBRARIES` instead.

Reproducing HTTP2.0 tests
-------------------------

You must have built and be running lws against a version of openssl that has
ALPN / NPN.  Most distros still have older versions.  You'll know it's right by
seeing

```bash
lwsts[4752]:  Compiled with OpenSSL support
lwsts[4752]:  Using SSL mode
lwsts[4752]:  HTTP2 / ALPN enabled
```

at lws startup.

For non-SSL HTTP2.0 upgrade

```bash
$ nghttp -nvasu http://localhost:7681/test.htm
```

For SSL / ALPN HTTP2.0 upgrade

```
$ nghttp -nvas https://localhost:7681/test.html
```

Cross compiling
---------------
To enable cross-compiling **libwebsockets** using CMake you need to create
a "Toolchain file" that you supply to CMake when generating your build files.
CMake will then use the cross compilers and build paths specified in this file
to look for dependencies and such.

**Libwebsockets** includes an example toolchain file [cross-arm-linux-gnueabihf.cmake](cross-arm-linux-gnueabihf.cmake)
you can use as a starting point.

The commandline to configure for cross with this would look like

```bash
$ cmake .. -DCMAKE_INSTALL_PREFIX:PATH=/usr \
	 -DCMAKE_TOOLCHAIN_FILE=../cross-arm-linux-gnueabihf.cmake \
	 -DWITHOUT_EXTENSIONS=1 -DWITH_SSL=0
```

The example shows how to build with no external cross lib dependencies, you
need to provide the cross libraries otherwise.

**NOTE**: start from an EMPTY build directory if you had a non-cross build in there
	before the settings will be cached and your changes ignored.

Additional information on cross compilation with CMake:
	http://www.vtk.org/Wiki/CMake_Cross_Compiling

Memory efficiency
-----------------

Embedded server-only configuration without extensions (ie, no compression
on websocket connections), but with full v13 websocket features and http
server, built on ARM Cortex-A9:

Update at 8dac94d (2013-02-18)

```bash
$ ./configure --without-client --without-extensions --disable-debug --without-daemonize

Context Creation, 1024 fd limit[2]:   16720 (includes 12 bytes per fd)
Per-connection [3]:                      72 bytes, +1328 during headers

.text	.rodata	.data	.bss
11512	2784	288	4
```

This shows the impact of the major configuration with/without options at
13ba5bbc633ea962d46d using Ubuntu ARM on a PandaBoard ES.

These are accounting for static allocations from the library elf, there are
additional dynamic allocations via malloc.  These are a bit old now but give
the right idea for relative "expense" of features.

Static allocations, ARM9

|                                | .text   | .rodata | .data | .bss |
|--------------------------------|---------|---------|-------|------|
| All (no without)               | 35024   | 9940    | 336   | 4104 |
| without client                 | 25684   | 7144    | 336   | 4104 |
| without client, exts           | 21652   | 6288    | 288   | 4104 |
| without client, exts, debug[1] | 19756   | 3768    | 288   | 4104 |
| without server                 | 30304   | 8160    | 336   | 4104 |
| without server, exts           | 25382   | 7204    | 288   | 4104 |
| without server, exts, debug[1] | 23712   | 4256    | 288   | 4104 |

[1] `--disable-debug` only removes messages below `lwsl_notice`.  Since that is
the default logging level the impact is not noticeable, error, warn and notice
logs are all still there.

[2] `1024` fd per process is the default limit (set by ulimit) in at least Fedora
and Ubuntu.  You can make significant savings tailoring this to actual expected
peak fds, ie, at a limit of `20`, context creation allocation reduces to `4432 +
240 = 4672`)

[3] known header content is freed after connection establishment
