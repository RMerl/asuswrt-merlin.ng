Change Log
==========

All notable changes in libConfuse are documented in this file.


[v3.2.2][] - 2018-08-19
-----------------------

This is a security patch release for CVE-2018-14447.  The vulnerability
affects all releases since v3.1 when the `CFGF_COMMENTS` functionality
was first introduced.

### Fixes
* Issue #109: Out of bounds read in `lexer.l:trim_whitespace()`.


[v3.2.1][] - 2017-08-17
-----------------------

### Fixes
* Issue #101: Bump ABI major number due to incompatible change in struct
  when adding option annotation/comments in [v3.1][]: `1.1.0 -> 2.0.0`


[v3.2][] - 2017-06-03
---------------------

### Fixes
* Issue #96: Add Windows/mingw compatible `fmemopen()` replacement
* Issue #98: Fix v3.1 regression, segfault on comment-only lines


[v3.1][] - 2017-05-24
---------------------

### Changes
* Refactored `CFGF_IGNORE_UNKNOWN` support, libConfuse now properly
  ignores any type and sub-section without the need for declaring an
  `__unknown` option.  When the flag is set all unknown options,
  including unknown sub-sections with, in turn, unknown options, are
  now fully ignored
* Issue #69: New API for creating titled sections at runtime,
  by Jonas Johansson @jonasj76
* Issue #92: Support for option annotation/comments.  Every option can
  now have a comment, which is both read and written from/to file.
  Disabled by default, enable with `CFGF_COMMENTS` flag in `cfg_init()`
* ABI bump: 1.0.0 --> 1.1.0, due to new functionality

### Fixes
* Build unit tests statically for easier debugging
* Issue #21: Major refactor of lexer to fix memory leaks, `cfg_free()`
  now properly releases all memory.  By Joachim Nilsson @troglobit
* Issue #64: Fixed MSVC build errors, by George Koskeridis @Gikoskos
* Issue #65: SIGSEGV when parsed default values are used with include,
  by Dmitri Zhabinski
* Issue #71: Fix syntax in rpm spec file, for CentOS/RHEL7
* Issue #73: Adjust gettext version requirement to build on CentOS/RHEL7.
  GNU gettext v0.18.2.1 update `AM_GNU_GETTEXT()` to use AC_PROG_MKDIR_P
  instead of `AM_PROG_MKDIR_P`, but v0.18.1.1 is included in Ubuntu 12.04
  LTS.  Fortunately Ubuntu 14.04 LTS ships v0.18.3.1 and Debian Jessie
  ships v0.19.3.  Unfortunately, CentOS7 and RHEL7 ships v0.18.2.1, so
  for best compat. level at this point in time we require v0.18.2.
* Issue #74: Fix typos in documentation, by Luca Ceresoli
* Issue #79: Add `fmemopen()` compat for *BSD, including macOS


[v3.0][] - 2016-03-03
---------------------

This release signifies a major change in libConfuse.  On out-of-memory
conditions at run time, invalid API input, and some other odd use-cases,
libConfuse will no longer `assert()`.  Instead, `NULL` or `CFG_FAIL` is
returned with an error code for you to handle.  For some users this will
completely change how your application works, so heads up!  The library
ABI version has also been stepped due to this.

Special thanks in this release goes out to Frank Hunleth, Peter Rosin
and David Grayson for their tireless efforts in helping improve this
library!

**Note:** libConfuse no longer calls `setlocale()` for `LC_MESSAGES` and
  `LC_CTYPE`.  See the documentation for `cfg_init()` for details.

### Changes

* Support for handling unknown options.  The idea is to provide future
  proofing of configuration files, i.e. if a new parameter is added, the
  new config file will not fail if loaded in an older version of your
  program.  See the `CFGF_IGNORE_UNKNOWN` flag in the documentation for
  more information.  Idea and implementation by Frank Hunleth.
* Add public API for removing sections at runtime, by Peter Rosin.
* Allow `cfg_opt_getval()` on options that are `CFGF_MULTI` sections,
  by Peter Rosin.
* Add `cfg_setmulti()` and `cfg_opt_setmulti()`, by Peter Rosin.
* Add CLI example of how to manage configuration changes at runtime,
  also by Peter Rosin.
* Support for Travis-CI and Coverity Scan, by Joachim Nilsson.
* Use `autoreconf` in `autogen.sh` instead of calling tools separately.
* Powershell script for AppVeyor CI to build libConfuse with MSYS2
  by David Grayson.
* Removed calls to `setlocale()` intended to localize messages, with
  `LC_MESSAGES`, and region specific types, with `LC_CTYPE`.  This is
  now the responsibility of the user of the library.
* Reindent to Linux coding style for a clear and well defined look,
  this to ease future maintenance.  Issue #33
* Add support for `CFGF_DEPRECATED` and `CFGF_DROP` option flags.  The
  former causes libConfuse to print a deprecated warning message and the
  latter drops the read value on input.  Idea and implementation by
  Sebastian Geiger.  Issue #24
* Add `HACKING.md` document to detail maintenance and release checklists

### Fixes

* Do not assert on API input validation, memory allocation, or similar.
  Instead, return error code to user for further handling.  This change
  also includes fixes for a lot of unchecked API return values, e.g.,
  `strdup()`.  Issue #37
* Protect callers arguments to `cfg_setopt()`, by Peter Rosin
* If new value to `cfg_setopt()` fails parsing, do not lose old value,
  by Peter Rosin.
* Fixes to update support for older versions of Microsoft Visual Studio
  as well as MSYS2/mingw-w64 by Peter Rosin and David Grayson.
* Issue #45: `cfg_init()` does not report error on multiple options with
  the same name.  Fixed by Peter Rosin.
* Fixes for memory leaks, invalid expressions, unused variables and
  missing error handling, all thanks to Coverity Scan


[v2.8][] - 2015-10-14
---------------------

### Changes

* Support for specifying a searchpath for `cfg_parse()`, by J.J. Green
* Restore build of shared library by default, by Nathan Phillip Brink
* Added German translation, contributed by Chris Leick, Aurelien Jarno,
  and Tux^verdreifelt.
* Document `CFG_SIMPLE_STR` for doxygen, by Nathan Phillip Brink
* Update ISC license to 2007 version, by Joachim Nilsson
* Write files in a Bourne shell compatible way, by Alvaro G. M
* Fix mid-string environment variable substitution, by Frank Hunleth

### Fixes
* Various ISO C90 and `-ansi` fixes by Carlo Marcelo Arenas Belon
* Fix C++ compiler warnings for `const` strings, by Craig McQueen
* Fix `make distcheck` and out-of-source builds, by Nathan Phillip Brink
* Fix missing `.gitignore` files, by Carlo Marcelo Arenas Belon
* Fix `CFG_SIMPLE_INT` on 64-bit systems, by Carlo Marcelo Arenas Belon
* Coding style cleanup by J.J. Green
* Fix issue #27: searchpath free problems.  Fix to new feature
  introduced in this release cycle.
* Improved support for MSYS2 by David Grayson.


[v2.7][] - 2010-02-20
---------------------

### Changes

* Expose `cfg_setopt()` function in public API, suggested by Daniel Pocock
* Add doxygen documentation for `cfg_setopt()`, by Daniel Pocock
* Don't fail on compiler warnings, remove `-Werror`, by Martin Hedenfalk
* Avoid aborting processing of the configuration buffer after returning
  from include processing.  By Carlo Marcelo Arenas Belon
* Make building of examples optional.

### Fixes

* Fix user defined error callbacks, by Martin Hedenfalk
* Include `locale.h`, required by `setlocale()`, patch by Diego Petteno
* Check for `inet_ntoa()` in external library, needed for tests to pass
  on Solaris, patch by Diego Petteno
* Fixes for build warnings, by Martin Hedenfalk and Carlo Marcelo Arenas Belon
* Fix segfault when processing a buffer that called `cfg_include()`, patch
  submitted by Carlo Marcelo Arenas Belon
* Fix lexer match problem with unquoted strings ending in a slash.  Fixed by
  Martin Hedenfalk, reported by Sylvain Bertrand.


[v2.6][] - 2007-10-13
---------------------

### Changes

* added French translation contributed by Matthieu Sion
* added build script and instructions for compiling with Mingw under
  Windows (contributed by Matthieu Sion)
* now accepts a simplified list append syntax:
    
    option += "value"
      instead of
    option += {"value"}
    
* added flag `CFGF_NO_TITLE_DUPES`: multiple section titles must be
  unique (duplicates raises an error, only applies to sections)
  (suggested by Brian Fallik)
* remove obsolete `confuse-config` script in favour of `pkg-config`
* windows build files now only in separate zip distribution

### Fixes

* fixed rpm builds, patch by Dan Lipsitt
* always installs `pkg-config` .pc script
* fixed a bug reported by Josh Kropf with single sections with titles
* added patch that escapes values with quotes and backslashes when printing.
* fixed a memory leak in default values for string lists,
  reported by Vineeth Neelakant.


[v2.5][] - 2004-10-17
---------------------

### Changes

* added flag `CFGF_NODEFAULT` (option has no default value)
* added a tutorial
* updated autoconf stuff, libconfuse installs with appropriate suffix now
* added data file for `pkg-config` (try `pkg-config --libs libconfuse`)
* updated `confuse-config` script (now only installed if `pkg-config` not found)
* added `cfg_name()` and `cfg_opt_name()` functions

### Fixes

* fixed `cfg_set_validate_func()` for sections, using the "|" syntax


[v2.4][] - 2004-08-09
---------------------

### Changes

* added option type `CFGT_PTR` as a user-defined type

### Fixes

* fixed building of shared libraries


[v2.3][] - 2004-05-22
---------------------

### Changes

* options passed to `cfg_init()` are now dynamically duplicated, so it
  is no longer necessary to declare the `cfg_opt_t array` static
* added tests using 'check' (a unit testing framework for C)
* added config script `confuse-config`

### Fixes

* fixes compilation errors with gcc < 3.3


[v2.2][] - 2003-09-25
---------------------

### Changes

* Allows more characters in an unquoted string (thanks Mike)
* added `cfg_opt_get` functions
* added `cfg_opt_size` function
* added support to print options to a file
* added print callback function per option
* simple options can be retrieved with the `cfg_get` functions (allows
  using the `cfg_print` function for simple values)
* added validating callback function per option


[v2.1][] - 2003-07-13
---------------------

### Changes

* Reversed logic in `cfg_getXXX` functions, they now abort if given an
  undeclared option name, and NULL/false if no value is set. Suggested
  by Ademar de Souza Reis Jr.
* Sections without `CFGF_MULTI` flag now have default values
* The `cfg_getXXX` functions now accept an extended syntax for the
  option name, try `cfg_getxxx(cfg, "sectionname|optionname")`.  This
  way one doesn't have to first get the section with `cfg_getsec()`.
* Added project files for MS Visual C++ 6.0
* Includes io.h on windows
* Setting a list to the empty list in the config file now possible.
* Appending to default values in a list is now OK.
* Hexadecimal escape sequences allowed in double-quoted strings
* Only include NLS support if gettext found in libc or preinstalled
* Documented the `cfg_setlist` and `cfg_addlist` functions
* The `cfg_opt_setxxx` functions no longer take a `cfg_t?` parameter (unused anyway)

### Fixes

* Fixed two more memory leaks. (`val->section` and `cfg->filename`)
* Fixed unterminated string bug in replacement strndup function
* Fixed initialization of default values for lists, when given a NULL
  string. Now initialized to the empty list. Noted by Juraj Variny.
* Corrected line number with multi-line quoted strings
* Fixed undetected `/*comment*/` (ie, without space between /* and the text)
* Forgot to `fclose()` include file after use, found by James Haley


v2.0 - 2003-04-29
-----------------

**NOTE:** Compatibility with earlier versions is broken!

### Changes

* Changed `cfg_flag_t` from `enum` to `int` (should now compile with C++)
* Variable number of arguments to functions: function types should no
  longer specify number of expected arguments in the initializer, the
  callback should instead check the `argc` variable.
* Added documentation for the value parsing callback
* Changed the definitions of `cfg_func_t` and `cfg_callback_t`, the cfg
  and option context are now both passed as parameters
* Added a bunch of `cfg_setXXX` functions to set option values after parsing
* Some types renamed for consistency (`cfgopt_t` to `cfg_opt_t`, `cfgval_t`
  to `cfg_value_t`, `cfgbool_t` to `cfg_bool_t`)
* `cfg_free_val()` renamed to `cfg_free_value()`
* Lexer symbols now uses prefix `cfg_` to ease linking with other lexers
* Sections with same title are always overwritten
* Lists can now have (complete) default values in the form of a string
  that is parsed in the same way as the config file (see doc + examples)
* Added support for building as a DLL on Windows
* Included project files for Borland C++ Builder 6.0
* Included project files for Dev-Cpp 5.0
* Included project files for MS Visual Studio
* Pre-built documentation now included in the source tarball

### Fixes

* Fixed the `cfg_tilde_expand` function
* Fixed and extended the example programs
* Forgot to close the file in `cfg_parse()`
* Memory leaks fixed (checked with valgrind)


v1.2.3 - 2002-12-18
-------------------

### Changes

* added callback support

### Fixes

* fixed segfault due to uninitialized user-defined error function


v1.2.2 - 2002-11-27
-------------------

### Changes

* changed name to libConfuse (libcfg was way too common)
* Don't build shared libraries by default (only static)
* More Swedish translations
* Implemented the `cfg_free()` function (previous versions had only a stub)
* New function: `cfg_free_val()`
* updated the manual


[UNRELEASED]: https://github.com/martinh/libconfuse/compare/v3.2.2...HEAD
[v3.2.2]: https://github.com/martinh/libconfuse/compare/v3.2.1...v3.2.2
[v3.2.1]: https://github.com/martinh/libconfuse/compare/v3.2...v3.2.1
[v3.2]:   https://github.com/martinh/libconfuse/compare/v3.1...v3.2
[v3.1]:   https://github.com/martinh/libconfuse/compare/v3.0...v3.1
[v3.0]:   https://github.com/martinh/libconfuse/compare/v2.8...v3.0
[v2.8]:   https://github.com/martinh/libconfuse/compare/v2.7...v2.8
[v2.7]:   https://github.com/martinh/libconfuse/compare/v2.6...v2.7
[v2.6]:   https://github.com/martinh/libconfuse/compare/v2.5...v2.6
[v2.5]:   https://github.com/martinh/libconfuse/compare/v2.4...v2.5
[v2.4]:   https://github.com/martinh/libconfuse/compare/v2.3...v2.4
[v2.3]:   https://github.com/martinh/libconfuse/compare/v2.2...v2.3
[v2.2]:   https://github.com/martinh/libconfuse/compare/v2.1...v2.2
[v2.1]:   https://github.com/martinh/libconfuse/compare/v2.0...v2.1
