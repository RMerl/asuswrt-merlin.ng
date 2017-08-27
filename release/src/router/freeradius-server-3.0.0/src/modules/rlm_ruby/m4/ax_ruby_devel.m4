# ===========================================================================
#             http://autoconf-archive.cryp.to/ax_ruby_devel.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_RUBY_DEVEL([version])
#
# DESCRIPTION
#
#   This macro checks for Ruby and tries to get the include path to
#   'ruby.h'. It provides the $(RUBY_CFLAGS) and $(RUBY_LDFLAGS) output
#   variables. It also exports $(RUBY_EXTRA_LIBS) for embedding Ruby in your
#   code.
#
#   You can search for some particular version of Ruby by passing a
#   parameter to this macro, for example "1.8.6".
#
# LICENSE
#
#   Copyright (c) 2008 Rafal Rzepecki <divided.mind@gmail.com>
#   Copyright (c) 2008 Sebastian Huber <sebastian-huber@web.de>
#   Copyright (c) 2008 Alan W. Irwin <irwin@beluga.phys.uvic.ca>
#   Copyright (c) 2008 Rafael Laboissiere <rafael@laboissiere.net>
#   Copyright (c) 2008 Andrew Collier <colliera@ukzn.ac.za>
#   Copyright (c) 2008 Matteo Settenvini <matteo@member.fsf.org>
#   Copyright (c) 2008 Horst Knorr <hk_classes@knoda.org>
#
#   This program is free software: you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation, either version 3 of the License, or (at your
#   option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
#   Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program. If not, see <http://www.gnu.org/licenses/>.
#
#   As a special exception, the respective Autoconf Macro's copyright owner
#   gives unlimited permission to copy, distribute and modify the configure
#   scripts that are the output of Autoconf when processing the Macro. You
#   need not follow the terms of the GNU General Public License when using
#   or distributing such scripts, even though portions of the text of the
#   Macro appear in them. The GNU General Public License (GPL) does govern
#   all other use of the material that constitutes the Autoconf Macro.
#
#   This special exception to the GPL applies to versions of the Autoconf
#   Macro released by the Autoconf Archive. When you make and distribute a
#   modified version of the Autoconf Macro, you may extend this special
#   exception to the GPL to apply to your modified version as well.
AC_DEFUN([AX_RUBY_DEVEL],[
    AC_REQUIRE([AX_WITH_RUBY])
    AS_IF([test -n "$1"], [AX_PROG_RUBY_VERSION([$1])])

    #
    # Check if you have mkmf, else fail
    #
    AC_MSG_CHECKING([for the mkmf Ruby package])
    ac_mkmf_result=`$RUBY -rmkmf -e ";" 2>&1`
    if test -z "$ac_mkmf_result"; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
        AC_MSG_WARN([cannot import Ruby module "mkmf".
Please check your Ruby installation. The error was:
$ac_distutils_result])
    fi

    #
    # Check for Ruby include path
    #
    if test -z "$RUBY_CFLAGS"; then
        #
        # Check for Ruby cflags
        #
        AC_MSG_CHECKING([for Ruby cflags])
        if test -z "$RUBY_CFLAGS"; then
            RUBY_CFLAGS=`$RUBY -rmkmf -e 'print RbConfig::CONFIG.fetch(%q(CFLAGS))'`
        fi
        AC_MSG_RESULT([$RUBY_CFLAGS])
    
        #
        # Check for Ruby include path
        #
        AC_MSG_CHECKING([for Ruby include path])
        ruby_path=`$RUBY -rmkmf -e 'c = RbConfig::CONFIG; print c.has_key?(%q(rubyhdrdir)) ? \
            c.fetch(%q(rubyhdrdir)) : c.fetch(%q(archdir))'`
            
        ruby_arch=`$RUBY -rmkmf -e 'print RbConfig::CONFIG.fetch(%q(arch))'`
            
        if test -n "${ruby_path}"; then
            #
            #  For some reason ruby 1.9.1 on linux seems to put its
            #  config.h file in ${ruby_path}/${ruby_arch}/ruby/config.h
            #  Aside from the fact that it is WRONG to include your own
            #  config.h file, it means we can't use the headers unless we
            #  add both paths.
            #
            if test -d "${ruby_path}/${ruby_arch}"; then
                 ruby_path=" -I${ruby_path} -I${ruby_path}/${ruby_arch}"
            else
                 ruby_path=" -I${ruby_path}"
            fi
        fi
        
        RUBY_CFLAGS+="$ruby_path"
        AC_MSG_RESULT([$ruby_path])
    fi
    
    AC_SUBST([RUBY_CFLAGS])

    if test -z "$RUBY_LDFLAGS"; then
        #
        # Check for Ruby library path
        #
        AC_MSG_CHECKING([for Ruby library path])
        if test -z "$RUBY_LIBRARY_PATH"; then
            RUBY_LIBRARY_PATH=`$RUBY -rmkmf -e 'print RbConfig::CONFIG.fetch(%q(libdir))'`
            if test -n "${RUBY_LIBRARY_PATH}"; then
                RUBY_LIBRARY_PATH=" -L$RUBY_LIBRARY_PATH"
            fi
        fi
        
        AC_MSG_RESULT([$RUBY_LIBRARY_PATH])  
        
        #
        # Check for Ruby linking flags
        #
        AC_MSG_CHECKING([for Ruby linking flags])
    
        RUBY_LDFLAGS=`$RUBY -rmkmf -e 'print RbConfig::CONFIG.fetch(%q(LIBRUBYARG_SHARED))'`
        AC_MSG_RESULT([$RUBY_LDFLAGS])

        RUBY_LDFLAGS="${RUBY_LIBRARY_PATH} ${RUBY_LDFLAGS}"
    fi

    AC_SUBST([RUBY_LDFLAGS])

    #
    # Check for site packages
    #
    AC_MSG_CHECKING([for Ruby site-packages path])
    if test -z "$RUBY_SITE_PKG"; then
        RUBY_SITE_PKG=`$RUBY -rmkmf -e 'print RbConfig::CONFIG.fetch(%q(sitearchdir))'`
    fi
    AC_MSG_RESULT([$RUBY_SITE_PKG])
    AC_SUBST([RUBY_SITE_PKG])

    #
    # libraries which must be linked in when embedding
    #
    AC_MSG_CHECKING([for Ruby extra libraries])
    if test -z "$RUBY_EXTRA_LIBS"; then
       RUBY_EXTRA_LIBS=`$RUBY -rmkmf -e 'print RbConfig::CONFIG.fetch(%q(SOLIBS))'`
    fi
    AC_MSG_RESULT([$RUBY_EXTRA_LIBS])
    AC_SUBST(RUBY_EXTRA_LIBS)

    #
    # linking flags needed when embedding
    # (is it even needed for Ruby?)
    #
    # AC_MSG_CHECKING([for Ruby extra linking flags])
    # if test -z "$RUBY_EXTRA_LIBS"; then
    # RUBY_EXTRA_LIBS=`$RUBY -rmkmf -e 'print RubyConfig::CONFIG.fetch(%q(LINKFORSHARED))'`
    # fi
    # AC_MSG_RESULT([$RUBY_EXTRA_LIBS])
    # AC_SUBST(RUBY_EXTRA_LIBS)

    # this flags breaks ruby.h, and is sometimes defined by KDE m4 macros
    CFLAGS="`echo "$CFLAGS" | sed -e 's/-std=iso9899:1990//g;'`"
    #
    # final check to see if everything compiles alright
    #
    AC_MSG_CHECKING([consistency of all components of ruby development environment])
    AC_LANG_PUSH([C])
    # save current global flags
    ac_save_LIBS="$LIBS"
    LIBS="$ac_save_LIBS $RUBY_LDFLAGS"
    ac_save_CFLAGS="$CFLAGS"
    CFLAGS="$ac_save_CFLAGS $RUBY_CFLAGS"
    AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([#include <ruby.h>],[ruby_init()])],
        [rubyexists=yes],
        [rubyexists=no])

    AC_MSG_RESULT([$rubyexists])

    if test ! "$rubyexists" = "yes"; then
       AC_MSG_WARN([
  Could not link test program to Ruby. Maybe the main Ruby library has been
  installed in some non-standard library path. If so, pass it to configure,
  via the LDFLAGS environment variable.
  Example: ./configure LDFLAGS="-L/usr/non-standard-path/ruby/lib"
  ============================================================================
   You probably have to install the development version of the Ruby package
   for your distribution.  The exact name of this package varies among them.
  ============================================================================
       ])
      RUBY_VERSION=""
    fi
    AC_LANG_POP
    # turn back to default flags
    CFLAGS="$ac_save_CFLAGS"
    LIBS="$ac_save_LIBS"

    #
    # all done!
    #
])

