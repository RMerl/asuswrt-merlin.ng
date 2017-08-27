/*
 *  C Preprocessor definitions we do *NOT* want to leave defined autoconf.h
 *  Which are dependent on where the header is being used.
 *
 *  Version: $Id$
 */


/*
 *  If were building a module we may have local PACKAGE_* defines if
 *  AC_INIT() was called with full arguments.
 */
#ifdef IS_MODULE
#	undef PACKAGE_BUGREPORT
#	undef PACKAGE_NAME
#	undef PACKAGE_STRING
#	undef PACKAGE_TARNAME
#	undef PACKAGE_URL
#	undef PACKAGE_VERSION
#endif

