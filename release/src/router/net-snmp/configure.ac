# Portions of this file are subject to the following copyright.  See
# the Net-SNMP's COPYING file for more details and other copyrights
# that may apply:
#
# Portions Copyright © 2003 Sun Microsystems, Inc. All rights reserved.
# Use is subject to license terms specified in the COPYING file
# distributed with the Net-SNMP package.
#
# Portions of this file are copyrighted by:
# Copyright (c) 2016 VMware, Inc. All rights reserved.
# Use is subject to license terms specified in the COPYING file
# distributed with the Net-SNMP package.
#

#
# Process this file with autoconf to produce a configure script.
#
AC_INIT([Net-SNMP], [5.8], [net-snmp-coders@lists.sourceforge.net])
AC_PREREQ([2.63])
AC_CONFIG_SRCDIR([agent/snmpd.c])
AC_REVISION([$Revision$])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER(include/net-snmp/net-snmp-config.h)

AC_MSG_CACHE_INIT()

#
# save the configure arguments
#
AC_DEFINE_UNQUOTED(NETSNMP_CONFIGURE_OPTIONS,"$ac_configure_args",
  [configure options specified])
CONFIGURE_OPTIONS="\"$ac_configure_args\""
AC_SUBST(CONFIGURE_OPTIONS)

#
# Check whether _GNU_SOURCE etc. should be defined
#
AC_USE_SYSTEM_EXTENSIONS

##
## Recommended structure:
##
#     information on the package
#     checks for programs
#     checks for libraries
#     checks for header files
#     checks for types
#     checks for structures
#     checks for compiler characteristics
#     checks for library functions
#     checks for system services

m4_include([configure.d/config_net_snmp_config_h])
m4_include([configure.d/config_project_with_enable])
##
# Protect against CFLAGS with -Werror which causes failures for some tests
#   (e.g. it causes type mismatches in the AC_CV_FUNCS call)
##
noerr_CFLAGS="$CFLAGS"
CFLAGS=`echo $CFLAGS | sed 's/-Werror[[^ ]]*//g'`
CFLAGS_without_Werror=$CFLAGS
export CFLAGS
m4_include([configure.d/config_project_manual])
m4_include([configure.d/config_os_progs])
m4_include([configure.d/config_project_paths])
m4_include([configure.d/config_os_headers])
AC_CACHE_SAVE
m4_include([configure.d/config_project_ipv6_types])
m4_include([configure.d/config_modules_lib])
AC_CACHE_SAVE
m4_include([configure.d/config_os_libs1])
m4_include([configure.d/config_modules_agent])
m4_include([configure.d/config_os_misc1])
m4_include([configure.d/config_os_libs2])
AC_CACHE_SAVE
m4_include([configure.d/config_project_perl_python])
m4_include([configure.d/config_os_misc2])
m4_include([configure.d/config_project_types])
m4_include([configure.d/config_os_misc3])
AC_CACHE_SAVE
m4_include([configure.d/config_os_functions])
m4_include([configure.d/config_os_struct_members])
m4_include([configure.d/config_os_misc4])
AC_CACHE_SAVE
##
# Add back in the original CFLAGS setting
##
CFLAGS="$noerr_CFLAGS ${CFLAGS#$CFLAGS_without_Werror}"
export CFLAGS

#
# define the agent libraries variables
#
AC_SUBST(LMIBLIBS)
AC_SUBST(AGENTLIBS)
AC_SUBST(OTHERAGENTLIBOBJS)
AC_SUBST(OTHERAGENTLIBLOBJS)
AC_SUBST(OTHERAGENTLIBLFTS)

AC_SUBST(DLLIBS)

AC_SUBST(PARTIALTARGETFLAGS)
AC_SUBST(ac_cv_NETSNMP_SYSTEM_INCLUDE_FILE)

AC_CONFIG_FILES([Makefile:Makefile.top:Makefile.in:Makefile.rules])
AC_CONFIG_FILES([snmplib/Makefile:Makefile.top:snmplib/Makefile.in:Makefile.rules:snmplib/Makefile.depend])
AC_CONFIG_FILES([apps/Makefile:Makefile.top:apps/Makefile.in:Makefile.rules:apps/Makefile.depend])
AC_CONFIG_FILES([apps/snmpnetstat/Makefile:Makefile.top:apps/snmpnetstat/Makefile.in:Makefile.rules:apps/snmpnetstat/Makefile.depend])
AC_CONFIG_FILES([agent/Makefile:Makefile.top:agent/Makefile.in:Makefile.rules:agent/Makefile.depend])
AC_CONFIG_FILES([agent/helpers/Makefile:Makefile.top:agent/helpers/Makefile.in:Makefile.rules:agent/helpers/Makefile.depend])
AC_CONFIG_FILES([agent/mibgroup/Makefile:Makefile.top:agent/mibgroup/Makefile.in:Makefile.rules:agent/mibgroup/Makefile.depend])
AC_CONFIG_FILES([local/Makefile:Makefile.top:local/Makefile.in:Makefile.rules])
AC_CONFIG_FILES([testing/Makefile:Makefile.top:testing/Makefile.in])
AC_CONFIG_FILES([man/Makefile:Makefile.top:man/Makefile.in:Makefile.rules])
AC_CONFIG_FILES([mibs/Makefile:Makefile.top:mibs/Makefile.in:Makefile.rules])
AC_CONFIG_FILES([net-snmp-config:net-snmp-config.in],
		[chmod +x net-snmp-config])
AC_CONFIG_FILES([net-snmp-create-v3-user:net-snmp-create-v3-user.in],
		[chmod +x net-snmp-create-v3-user])
AC_CONFIG_FILES([dist/generation-scripts/gen-variables:dist/generation-scripts/gen-variables.in])
AC_CONFIG_FILES([local/snmpconf])

AC_CONFIG_COMMANDS([default], echo timestamp > stamp-h)

#
# protect PACKAGE_* variables
#
AH_VERBATIM([PACKAGE_BUGREPORT],[/* Define to the address where bug reports for this package should be sent. */
#ifndef PACKAGE_BUGREPORT
#undef PACKAGE_BUGREPORT
#endif])
AH_VERBATIM([PACKAGE_NAME],[/* Define to the full name of this package. */
#ifndef PACKAGE_NAME
#undef PACKAGE_NAME
#endif])
AH_VERBATIM([PACKAGE_STRING],[/* Define to the full name and version of this package. */
#ifndef PACKAGE_STRING
#undef PACKAGE_STRING
#endif])
AH_VERBATIM([PACKAGE_TARNAME],[/* Define to the one symbol short name of this package. */
#ifndef PACKAGE_TARNAME
#undef PACKAGE_TARNAME
#endif])
AH_VERBATIM([PACKAGE_VERSION],[/* Define to the version of this package. */
#ifndef PACKAGE_VERSION
#undef PACKAGE_VERSION
#endif])

AC_OUTPUT

AC_MSG_CACHE_DISPLAY()
# Local Variables:
# mode: Autoconf
# comment-start: "#"
# End:
