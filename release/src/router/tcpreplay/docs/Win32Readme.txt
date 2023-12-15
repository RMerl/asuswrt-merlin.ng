$Id$
This document attempts to explain how to get tcpreplay compiled and running
under Windows.  Please note that this document is a work in progress and
Windows support in general considered EXPERIMENTAL right now.


Background:

Tcpreplay is not a native Win32 application right now.  Hence it requires
Cygwin. (http://www.cygwin.com).  Cygwin creates a Linux-like environment
on your Windows system which allows Linux/UNIX programs to run after a
recompile.

Tcpreplay supports numerous API's for sending packets depending on the 
operating system.  Under Windows, the only supported method of sending
packets is with WinPcap 4.0.  (http://www.winpcap.org)  Please be sure to
install both the WinPcap driver AND the developer pack.

Right now, I've only done testing under Windows XP.  My guess is that 2000
and 2003 should have no problems.  Since WinPcap and Cygwin are EOL'ing 
support for Win98/ME, I doubt that they'll ever be supported.  Not sure
the story on Vista, but I assume WinPcap & Cygwin will support them sooner
or later if not already.  Would love to hear if anyone has any luck one
way or another.

What you will need:

- Cygwin environment
- GCC compiler and system header files
- WinPcap 4.0 DLL
- WinPcap 4.0 Developer Pack aka WpdPack (headers, etc)

Additional requirements if building from SVN:
- GNU build chain tools (Autoconf, Automake, Autoheader)
- GNU Autogen  (*)

* NOTE: The guile package which comes with Cygwin is broken and breaks
Autogen, so you'll need to compile guile from scratch to fix.  Hence, I
strongly suggest you build Tcpreplay from the tarball and not SVN.  See 
below for how to "fix" this issue if you want to compile from SVN.



******************************* IMPORTANT ******************************
Note 1: 
People have reported problems with WpdPack (the developer pack for
Winpcap) being installed outside of the Cygwin root directory.  Hence, I
strongly recommend you install it under the Cygwin root as /WpdPack.

Note 2:
There's a big problem with the Cygwin Guile package which breaks
GNU Autogen which Tcpreplay depends on when building from Subversion. 

What this means is that to build from Subversion you must do the following:
- Download GNU Guile from http://www.gnu.org/software/guile/guile.html

- Extract the tarball and do the following:
libtoolize --copy --force
./configure
make
make install

This will install guile in /usr/local.

The other problem is that guile-config returns the linker flags in the wrong 
order.  To fix this, rename /usr/local/bin/guile-config to 
/usr/local/bin/guile-config.original and create a new shell script in it's
place:

---- BEGIN SHELL SCRIPT ----
#!/bin/bash
# Replacement /usr/local/bin/guile-config script
if test -z "$1" ; then
       guile-config.original
elif test "$1" == "link"; then
       echo "-L/usr/local/lib -lguile -lltdl -lgmp -lcrypt -lm -lltdl"
else
       guile-config.original $1
fi
---- END SHELL SCRIPT ----

******************************* IMPORTANT ******************************

Directions:
- Install all the requirements

- Enter into the Cygwin environment by clicking on the Cygwin icon

- If you checked out the code from SVN, see Note 2 above and then run 
  the autogen.sh bootstrapper:
	./autogen.sh

- Configure tcpreplay:
	./configure --enable-debug

- Build tcpreplay:	
	make
	
- Install:
	make install
	
- Try it out!




