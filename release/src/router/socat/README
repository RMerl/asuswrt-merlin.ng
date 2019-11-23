
about
-----

socat is a relay for bidirectional data transfer between two independent data
channels. Each of these data channels may be a file, pipe, device (serial line
etc. or a pseudo terminal), a socket (UNIX, IP4, IP6 - raw, UDP, TCP), an
SSL socket, proxy CONNECT connection, a file descriptor (stdin etc.), the GNU
line editor (readline), a program, or a combination of two of these. 
These modes include generation of "listening" sockets, named pipes, and pseudo
terminals.

socat can be used, e.g., as TCP port forwarder (one-shot or daemon), as an
external socksifier, for attacking weak firewalls, as a shell interface to UNIX
sockets, IP6 relay, for redirecting TCP oriented programs to a serial line, to
logically connect serial lines on different computers, or to establish a
relatively secure environment (su and  chroot) for running client or server
shell scripts with network connections. 

Many options are available to refine socats behaviour:
terminal parameters, open() options, file permissions, file and process owners,
basic socket options like bind address, advanced socket options like IP source
routing, linger, TTL, TOS (type of service), or TCP performance tuning.

More capabilities, like daemon mode with forking, client address check,
"tail -f" mode, some stream data processing (line terminator conversion),
choosing sockets, pipes, or ptys for interprocess communication, debug and
trace options, logging to syslog, stderr or file, and last but not least
precise error messages make it a versatile tool for many different purposes.

In fact, many of these features already exist in specialized tools; but until
now, there does not seem to exists another tool that provides such a generic,
flexible, simple and almost comprehensive (UNIX) byte stream connector.


packages
--------

before bothering with compilers, dependencies and include files, you might
try to get a binary distribution that matches your platform. Have a look at 
the projects home page for actual information regarding socat binary 
distributions.


platforms
---------

socat 1.7.0 was compiled and more or less successfully tested under the
following operating systems:

Debian lenny/sid on x86, kernel 2.6.24
FreeBSD 6.1 on x86
NetBSD 4.0  on x86
OpenBSD 4.3 on x86
OpenSolaris 10 on x86 with gcc
Mac OS X 10.5.5 on iMac G5, with libreadline
HP-UX 11.23
AIX 5.3 on 64bit Power4 with gcc
Cygwin 1.5.25 on i686

tests on Tru64 can no longer be performed because HP testdrive has taken down
these hosts.

Some versions of socat have been reported to successfully compile under older
Linux versions back to RedHat 2.1 (kernel 1.2.13, gcc 2.7.0), under AIX 4.1 and
4.3, SunOS 5.7-5.8, FreeBSD 4.2 - 4.9, MacOS X 10.1, Cygwin, Solaris 8 on x86, 
OSR 5.0.6, NetBSD 1.6.1 and 2.0.2, OpenBSD 3.4 and 3.8, Tru64 5.1B, Mac OS X
10.1-10.2, and HP-UX 11

It might well compile and run under other UNIX like operating systems.


install
-------

Get the tarball and extract it:
	tar xzf socat.tar.gz
	cd socat-1.7.3.0
	./configure
	make
	su
	make install	# installs socat, filan, and procan in /usr/local/bin

For compiling socat, gcc (or egc) is recommended.
If gcc is not available, the configure script will fail to determine
some features; then you'd better begin with one of the Makefiles and config.h's
from the Config directory.

If you have problems with the OpenSSL library, you can apply the option
"--disable-openssl" to configure.

If you have problems with the readline library or (n)curses, you can apply the
option "--disable-readline" to configure.

If you have problems with the tcp wrappers library, you can apply the option
"--disable-libwrap" to configure.

If you still get errors or a tremendous amount of warnings you can exclude 
the features for system call tracing and file descriptor analyzing by
applying the options "--disable-sycls --disable-filan" to configure.

You still need the functions vsnprintf and snprintf that are in the GNU libc,
but might not be available with some proprietary libc's.

The configure script looks for headers and libraries of openssl, readline, and
tcp wrappers in the OS'es standard places and in the subdirectories include/
and lib/ of the following places: 
   /sw/
   /usr/local/
   /opt/freeware/
   /usr/sfw/
and for openssl also in:
   /usr/local/ssl/
In case of unexpected behaviour it is important to understand that configure
first searches for the appropriate include file and then expects to find the
library in the associated lib directory. That means, when e.g. a OpenSSL
installation resides under /usr/local and there is a symbolic link from
/usr/include/ssl/ssl.h to /usr/local/ssl/include/ssl/ssl.h, configure will find
the /usr/include/... header and will therefore expect libssl in /usr/lib
instead of /usr/local/...

If configure does not find a header file or library but you know where it is,
you can specify additional search locations, e.g.:
   export LIBS="-L$HOME/lib"
   export CPPFLAGS="-I$HOME/include"
before running configure and make.

For other operating systems, if socat does not compile without errors, refer to
the file PORTING.


platform specifics - redhat
---------------------------

Install the following packages before building socat:
  tcp_wrappers-devel
  readline-devel
  openssl-devel

On RedHat Linux 9.0, including openssl/ssl.h might fail due to problems with
the krb5-devel package. configure reacts with disabling openssl integration. 
To solve this issue, help cpp to find the krb5.h include file:
CPPFLAGS="-I/usr/kerberos/include" ./configure


platform specifics - aix
------------------------

The flock() prototype is not available but the function is. Thus, to enable the
socat flock options, run configure and then change in config.h the line 
/* #undef HAVE_FLOCK */
to
#define HAVE_FLOCK 1
and continue the build process.

When using the OpenSSL rpm provided by IBM, configure might need the 
environment variable setting:
LIBS="-L/opt/freeware/lib"

When using the OpenSSL bundle provided by IBM, egd needs to be installed too
to get enough entropy.

socat compiles not only with gcc, but also with xlc. Just adapt the Makefile:
replace gcc by /usr/vac/bin/xlc and remove gcc specific options 
"-Wall -Wno-parentheses".

When linking with the OpenSSL library provided by IBM, errors may occur:
ld: 0711-317 ERROR: Undefined symbol: .__umoddi3
In this case, you need to link with libgcc or compile libcrypt yourself using
xlc, or disable SSL (in config.h, undefine WITH_OPENSSL and recompile)

The score of test.sh can be improved by uncommenting MISCDELAY=1 in this
script.


platform specifics - solaris
----------------------------

If libreadline or libssl are in a directory not searched by the loader per
default, e.g. /opt/sfw/lib, you must add this directory to $LD_LIBRARY_PATH,
for running both configure and the socat executables, e.g.:
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/sfw/lib

For some shell scripts, it is preferable to have /usr/xpg4/bin at a prominent
position in $PATH.

With the default compiler define _GNU_SOURCE, the CMSG_* macros are not
available, and therefore ancillary messages cannot be used. To enable these try
the following:
After running ./configure, edit Makefile and replace "-D_GNU_SOURCE" with
"-D_XPG4_2 -D__EXTENSIONS__" and run make


platform specifics - hp-ux
--------------------------

Ancillary messages cannot be compiled in with socat: both struct msghdr and
struct cmsghdr are required. Compiling with -D_XOPEN_SOURCE_EXTENDED provides
struct msghdr but disables struct cmsghdr while -D_OPEN_SOURCE disables struct
msghdr but disables struct cmsghdr. Please contact socat development if you
know a solution.

Shutting down the write channel of a UNIX domain socket does not seem to
trigger an EOF on the peer socket. This makes problems with the exec and
system addresses.

This OS provides the type "long long", but not the strtoll() function to read
data into a long long variable.

UNIX domain sockets are only supported with SOCK_STREAM, not with datagrams
(see man 7 unix).

With UDP sockets it seems to happen that the select() call reports available
data (or EOF) but a subsequent read() call hangs.


platform specifics - tru64
--------------------------

When the use of the readline address fails with an error like:
socat: /sbin/loader: Fatal Error: Reference to unresolvable symbol "tgetent" in ".../libreadline.so.4"
and you still want to use shared libraries, try the following workaround: 
$ make distclean; LIBS="-static" ./configure
remove the "-static" occurrence in Makefile
$ make


documentation
-------------

These files reside in the doc subdirectory:

socat.1 is the man page, socat.html is the HTML based man page. It is actual,
but describes only the more useful options.

xio.help is an older, but more exact description in text form; with socat
version 1.6.0 it is outdated.

doc/socat-openssltunnel.html is a simple tutorial for a private SSL connection.
doc/socat-multicast.html is a short tutorial for multicast and broadcast
communications.
doc/socat-tun shows how to build a virtual network between two hosts.

socat.1 and socat.html can be generated from socat.yo (which is released with
socat 1.6.0.1 and later) using the yodl document language package. Maintenance
of yodl had been discontinued by its author
(http://www.xs4all.nl/~jantien/yodl/) (there seems to be a revival at
http://yodl.sourceforge.net/ though). For socat, the old version 1.31 is used;
an rpm is still distributed with recent OpenSuSE versions (confirmed for
OpenSuSE 10.1 in suse/i586/yodl-1.31.18-1142.i586.rpm). It appears to install
smoothly also under RedHat Linux. After yodl 1.31 installation, the following
correction must be performed in /usr/share/yodl/shared.yo in two places:
< whenhtml(htmlcommand(<!)ARG1+htmlcommand(>)))
> whenhtml(htmlcommand(<!--)ARG1+htmlcommand(-->)))


license
-------

socat is distributed under the terms of the GNU GPLv2;
except for install-sh, which is copyright MIT, with its own license;

In addition, as a special exception, the copyright holder
gives permission to link the code of this program with
any version of the OpenSSL library which is distributed
under a license identical to that listed in the included
COPYING.OpenSSL file, and distribute linked combinations
including the two. You must obey the GNU General Public
License in all respects for all of the code used other
than OpenSSL. If you modify this file, you may extend this
exception to your version of the file, but you are not
obligated to do so. If you do not wish to do so, delete
this exception statement from your version.


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 2 of the License

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


contact
-------

For questions, bug reports, ideas, contributions etc. please contact
socat@dest-unreach.org

For socat source distribution, bug fixes, and latest news see
        http://www.dest-unreach.org/socat/

www.socat.org is an alternate site providing the same contents.

public git repository:
	git://repo.or.cz/socat.git
	http://repo.or.cz/r/socat.git
