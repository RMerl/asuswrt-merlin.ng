Please do not compile this package (GLib) in paths that contain
spaces in them-as strange problems may occur during compilation or during
the use of the library.

Please refer to the following GNOME Live! page for more detailed
instructions on building GLib and its dependencies with Visual C++:

https://live.gnome.org/GTK%2B/Win32/MSVCCompilationOfGTKStack

This VS10 solution and the projects it includes are intented to be used
in a GLib source tree unpacked from a tarball. In a git checkout you
first need to use some Unix-like environment or run build/win32/setup.py, 
which will do the work for you:

$python build/win32/setup.py --perl path_to_your_perl.exe

for more usage on this script, run
$python build/win32/setup.py -h/--help

The required dependencies are zlib, proxy-libintl and LibFFI. Fetch the latest
proxy-libintl-dev and zlib-dev zipfiles from
http://ftp.gnome.org/pub/GNOME/binaries/win32/dependencies/ for 32-bit
builds, and correspondingly
http://ftp.gnome.org/pub/GNOME/binaries/win64/dependencies/ for 64-bit
builds.

One may wish to build his/her own ZLib-It is recommended that ZLib is
built using the win32/Makefile.msc makefile with VS10 with the ASM routines
to avoid linking problems-see win32/Makefile.msc in ZLib for more details.

For LibFFI, please get version 3.0.10 or later, as Visual C++ build support
was added in the 3.0.10 release series.  Please see the README file that
comes with the LibFFI source package for more details on how to build LibFFI
on Visual C++-please note that the mozilla-build package from Mozilla is needed
in order to build LibFFI on Windows.

One may optionally use his/her own PCRE installation by selecting the
(BuildType)_ExtPCRE configuration, but please note the PCRE must be built
with VS10 with unicode support using the /MD (release) or /MDd (debug)
runtime option which corresponds to your GLib build flavour (release, debug).
(These are the defaults set by CMAKE, which is used in recent versions of PCRE.)
Not doing so will most probably result in unexpected crashes in 
your programs due to the use of different CRTs.  If using a static PCRE
build, add PCRE_STATIC to the "preprocessor definitions".
Note that one may still continue to build with the bundled PCRE by selecting
the (BuildType) configuration.

Set up the source tree as follows under some arbitrary top
folder <root>:

<root>\<this-glib-source-tree>
<root>\vs10\<PlatformName>

*this* file you are now reading is thus located at
<root>\<this-glib-source-tree>\build\win32\vs10\README.

<PlatformName> is either Win32 or x64, as in VS10 project files.

You should unpack the proxy-libintl-dev zip file into
<root>\vs10\<PlatformName>, so that for instance libintl.h end up at
<root>\vs10\<PlatformName>\include\libintl.h.

For LibFFI, one should also put the generated ffi.h and ffitarget.h
into <root>\vs10\<PlatformName>\include\ and the compiled static libffi.lib
(or copy libffi-convenience.lib into libffi.lib) into 
<root>\vs10\<PlatformName>\lib\.

The "install" project will copy build results and headers into their
appropriate location under <root>\vs10\<PlatformName>. For instance,
built DLLs go into <root>\vs10\<PlatformName>\bin, built LIBs into
<root>\vs10\<PlatformName>\lib and GLib headers into
<root>\vs10\<PlatformName>\include\glib-2.0. This is then from where
project files higher in the stack are supposed to look for them, not
from a specific GLib source tree.

Note: If you see C4819 warnings and you are compiling GLib on a DBCS
(Chinese/Korean/Japanese) version of Windows, you may need to switch
to an English locale in Control Panel->Region and Languages->System->
Change System Locale, reboot and rebuild to ensure GLib, Pango, GDK-Pixbuf,
ATK and GTK+ is built correctly.  This is due to a bug in Visual C++ running
on DBCS locales.

--Tor Lillqvist <tml@iki.fi>
--Updated by Chun-wei Fan <fanc999@gmail.com>
