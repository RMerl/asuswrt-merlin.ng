Stubby for Windows
--------------------
We consider Windows support for Stubby to be Beta at this time. It has been 
tested on Windows 10 and Windows 8. There is limited support for Windows 7 
(see below). User testing reports, bug reports and patches/pull requests are all
welcomed via github!
https://github.com/getdnsapi/stubby

Installation
--------------------
Download and run the installer.

This installs the following files in C:\Program Files\Stubby:
* README.txt containing these instructions
* stubby.exe binary
* stubby.yml  configuration file
* getdns_query.exe tool for testing stubby
* Powershell scripts for modifying system resolvers:
  * stubby_setdns_windows.ps1
  * stubby_resetdns_windows.ps1
  * Windows 7 versions of Powershell scripts
    * stubby_setdns_windows7.ps1
    * stubby_resetdns_windows7.ps1
* Scripts to enable Stubby to be configured as a Scheduled Task
  * stubby.bat
  * stubby.xml

NOTE: The 32bit build of Stubby is installed in C:\Program Files (x86)\Stubby on
64 bit systems so use that path below as required. 


Version
--------
This version of the installer is version %INSTALLER_VERSION%. It is built from:
getdns version:  %GETDNS_VERSION%
stubby version:  %STUBBY_VERSION%
openssl version: %OPENSSL_VERSION%

Configuration
--------------------
It is recommended to use the default configuration file provided which will use 
'Strict' privacy mode and spread the DNS queries among several of the current 
DNS Privacy test servers. Note that this file contains both IPv4 and IPv6 
addresses. It installed in "C:\Program Files\Stubby\stubby.yml"

More information on how to customise the configuration can be found here.
https://dnsprivacy.org/wiki/display/DP/Configuring+Stubby

Run Stubby
--------------------
Simply invoke Stubby on the command line from a command prompt window (To get a 
command prompt go to the Windows search box and type 'cmd' and then choose the 
'Command prompt' option)

> "C:\Program Files\Stubby\stubby.exe" -l

The -l flag enables full logging. Alternatively a specific logging level can be 
controlled by using the -v flag (run '"C:\Program Files\Stubby\stubby.exe" - h' 
for details of available levels).

A different location for the configuration file can be specified by adding:
-C "<full path to stubby.yml>"

We are working on support for running stubby as a service on Windows 10

Test Stubby
--------------------
A quick test can be done by using getdns_query (or your favourite DNS tool) on 
the loopback address:

> "C:\Program Files\Stubby\getdns_query" -s @127.0.0.1 www.example.com

You should see a status of GETDNS_RESPSTATUS_GOOD and and rcode of 
GETDNS_RCODE_NOERROR in the getdns_query output. You should also see a 
connection being made in the stubby logs.

Modify your upstream resolvers (Windows 8 and 10)
----------------------------------------
*Once this change is made all your DNS queries will be re-directed to Stubby and 
sent over TLS!*
(You may need to restart some applications to have them pick up the network 
settings).


For Stubby to re-send outgoing DNS queries over TLS the recursive resolvers 
configured on your machine must be changed to send all the local queries to the
loopback interface on which Stubby is listening. It might be useful to note your
existing default nameservers before making this change!

To change nameservers:
* From Windows search box type 'cmd' and on the 'Command prompt' option that 
appears right click and select 'run as Administrator'
* In the command prompt window that appears type

>PowerShell -ExecutionPolicy bypass -file  "C:\Program Files\Stubby\stubby_setdns_windows.ps1"

to switch the system DNS resolvers to use Stubby. 

* Use the same command but with 
"C:\Program Files\Stubby\stubby_resetdns_windows.ps1" instead to switch back to 
the default DNS nameservers.

You can monitor the DNS traffic using Wireshark watching on port 853.
If you encounter problems reverse this change to restore your default settings 
(no DNS Privacy).

Modify your upstream resolvers (Windows 7)
----------------------------------------
Follow the procedure above, but use the scripts:

* C:\Program Files\Stubby\stubby_setdns_windows7.ps1 and
* C:\Program Files\Stubby\stubby_resetdns_windows7.ps1

WARNING: These scripts can only update DNS servers on the IPv4 service. IPv6
will still use the default DNS servers, sending queries in clear text so one
option is to disable IPv6.

Create a Scheduled Task
----------------------
If you want Stubby to always start when you boot your system, you can create
a Scheduled task for this. A template for the task is provided: to create the
task just run

schtasks /create /tn Stubby /XML "C:\Program Files\Stubby\stubby.xml" /RU <you_user_name>


Known Issues
--------------------
* We are aware of occasional issues when Windows sleeps and resumes when Stubby 
must be restarted to work correctly.
* The help command of stubby.exe shows the wrong path for the installed 
configuration file. An issue has been opened for this.
* The installer currently overwrites the stubby.yml file so if changes have been
made a backup should be created before upgrading


Building the Windows Installer
--------------------------------

The Visual Studio solution provided in the stubby source code to build the
installer requires the Wix Toolset designer plugin (wixtoolset.org).

It assumes stubby was built using MinGW64 in the following way:

$ git clone https://github.com/getdnsapi/getdns.git
$ cd getdns/
$ git submodule update --init
$ libtoolize -ci
$ autoreconf -fi
$ mkdir build
$ cd build
$ CFLAGS="-O2" LDFLAGS="-Wl,-static -lpthread" ../configure --host=x86_64-w64-mingw32 --without-libidn --with-ssl=/mingw64 --with-libyaml=/mingw64 --enable-stub-only --with-stubby --disable-shared
$ make
$ make install

