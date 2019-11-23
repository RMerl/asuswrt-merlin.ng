
David Acker has patched socat to add OpenSSL FIPS.
See http://oss-institute.org/fips-faq.html and
http://linuxdevices.com/news/NS4742716157.html for more information.

The patch that is integrated into socat 1.5 does the following:

Add support for LDFLAGS in Makefile.  LDFLAGS can be specified on the
configure command line and then will be carried over into the make.

Add fips support.  Requires OpenSSL 0.9.7j-fips-dev from
http://www.openssl.org/source/OpenSSL-fips-1.0.tar.gz built with fips
support turned on. use ./Configure fips [os-arc], for example
./Configure fips linux-pentium

The LDFLAGS is needed to point a build against a library
located in a non-standard location.  For example, if you download and
build openssl manually, it gets installed in /usr/local/ssl by default.

The FIPS support patches involve adding an option to enable/disable fips
in configure (enabled by default), checking the system for FIPS support
during configure, and then adding a fips option to socats openssl address
to turn on fips mode.  The openssl binary uses an environment variable 
instead of a command line flag.
FIPS mode requires both a compile time flag of OPENSSL_FIPS and a
runtime call of FIPS_mode_set(1).  Fips mode requires building with the 
fipsld script provided by OpenSSL. FIPS tracks the pid of the process that 
initializes things so after a fork, the child must reinitialize.  When the 
ssl code detects a forks occur and if FIPS mode was enabled, it reinitializes 
FIPS by disabling and then enabling it again.

To produce Davids enviroment, do the following:
To build openssl
download  OpenSSL 0.9.7j-fips-dev from
http://www.openssl.org/source/OpenSSL-fips-1.0.tar.gz
tar xzf OpenSSL-fips-1.0.tar.gz
cd openssl
./Configure fips linux-pentium
make
make test
(become root)
make install
This leaves an install in /usr/local/ssl

To build socat:
setup directory with socat 1.5 or higher.
cd socat-1.5.0.0
./configure CPPFLAGS=-I/usr/local/ssl/include/ LDFLAGS=-L/usr/local/ssl/lib/ FIPSLD=/usr/local/ssl/bin/fipsld
make
(become root)
make install

To run tests we make sure the new openssl is used:

export PATH=/usr/local/ssl/bin:$PATH
./test.sh fips

There are two tests in test.sh that depend on fips:

OPENSSL_FIPS_BOTHAUTH performs a SSL client to server connection with
certificate based authentication in both directions. If it works FIPS mode
seems to be ok.

OPENSSL_FIPS_SECURITY generates a certificaet/key pair without fips support. It
then tries a SSL connection in "normal" mode which is expected to work. In the
second phase it uses fips mode with these credentials which is expected to
fail. If so, the test succeeded.
