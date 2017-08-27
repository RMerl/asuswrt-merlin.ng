/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-version.c: Version information
 *
 * Copyright (C) 2012 Igalia S.L.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "soup-version.h"

/**
 * SECTION:soup-version
 * @short_description: Variables and functions to check the libsoup version
 **/

/**
 * SOUP_MAJOR_VERSION:
 *
 * Like soup_get_major_version(), but from the headers used at
 * application compile time, rather than from the library linked
 * against at application run time.
 *
 * Since: 2.42
 */

/**
 * SOUP_MINOR_VERSION:
 *
 * Like soup_get_minor_version(), but from the headers used at
 * application compile time, rather than from the library linked
 * against at application run time.
 *
 * Since: 2.42
 */

/**
 * SOUP_MICRO_VERSION:
 *
 * Like soup_get_micro_version(), but from the headers used at
 * application compile time, rather than from the library linked
 * against at application run time.
 *
 * Since: 2.42
 */

/**
 * SOUP_CHECK_VERSION:
 * @major: major version (e.g. 2 for version 2.42.0)
 * @minor: minor version (e.g. 42 for version 2.42.0)
 * @micro: micro version (e.g. 0 for version 2.42.0)
 *
 * Returns: %TRUE if the version of the libsoup header files
 * is the same as or newer than the passed-in version.
 *
 * Since: 2.42
 */

/**
 * soup_get_major_version:
 *
 * Returns the major version number of the libsoup library.
 * (e.g. in libsoup version 2.42.0 this is 2.)
 *
 * This function is in the library, so it represents the libsoup library
 * your code is running against. Contrast with the #SOUP_MAJOR_VERSION
 * macro, which represents the major version of the libsoup headers you
 * have included when compiling your code.
 *
 * Returns: the major version number of the libsoup library
 *
 * Since: 2.42
 */
guint
soup_get_major_version (void)
{
    return SOUP_MAJOR_VERSION;
}

/**
 * soup_get_minor_version:
 *
 * Returns the minor version number of the libsoup library.
 * (e.g. in libsoup version 2.42.0 this is 42.)
 *
 * This function is in the library, so it represents the libsoup library
 * your code is running against. Contrast with the #SOUP_MINOR_VERSION
 * macro, which represents the minor version of the libsoup headers you
 * have included when compiling your code.
 *
 * Returns: the minor version number of the libsoup library
 *
 * Since: 2.42
 */
guint
soup_get_minor_version (void)
{
    return SOUP_MINOR_VERSION;
}

/**
 * soup_get_micro_version:
 *
 * Returns the micro version number of the libsoup library.
 * (e.g. in libsoup version 2.42.0 this is 0.)
 *
 * This function is in the library, so it represents the libsoup library
 * your code is running against. Contrast with the #SOUP_MICRO_VERSION
 * macro, which represents the micro version of the libsoup headers you
 * have included when compiling your code.
 *
 * Returns: the micro version number of the libsoup library
 *
 * Since: 2.42
 */
guint
soup_get_micro_version (void)
{
    return SOUP_MICRO_VERSION;
}

/**
 * soup_check_version:
 * @major: the major version to check
 * @minor: the minor version to check
 * @micro: the micro version to check
 *
 * Like SOUP_CHECK_VERSION, but the check for soup_check_version is
 * at runtime instead of compile time. This is useful for compiling
 * against older versions of libsoup, but using features from newer
 * versions.
 *
 * Returns: %TRUE if the version of the libsoup currently loaded
 * is the same as or newer than the passed-in version.
 *
 * Since: 2.42
 */
gboolean
soup_check_version (guint major,
                    guint minor,
                    guint micro)
{
    return SOUP_CHECK_VERSION (major, minor, micro);
}

/**
 * SOUP_VERSION_MIN_REQUIRED:
 *
 * A macro that should be defined by the user prior to including
 * libsoup.h. The definition should be one of the predefined libsoup
 * version macros: %SOUP_VERSION_2_24, %SOUP_VERSION_2_26, ...
 *
 * This macro defines the earliest version of libsoup that the package
 * is required to be able to compile against.
 *
 * If the compiler is configured to warn about the use of deprecated
 * functions, then using functions that were deprecated in version
 * %SOUP_VERSION_MIN_REQUIRED or earlier will cause warnings (but
 * using functions deprecated in later releases will not).
 *
 * Since: 2.42
 */

/**
 * SOUP_VERSION_MAX_ALLOWED:
 *
 * A macro that should be defined by the user prior to including
 * libsoup.h. The definition should be one of the predefined libsoup
 * version macros: %SOUP_VERSION_2_24, %SOUP_VERSION_2_26, ...
 *
 * This macro defines the latest version of the libsoup API that the
 * package is allowed to make use of.
 *
 * If the compiler is configured to warn about the use of deprecated
 * functions, then using functions added after version
 * %SOUP_VERSION_MAX_ALLOWED will cause warnings.
 *
 * Unless you are using SOUP_CHECK_VERSION() or the like to compile
 * different code depending on the libsoup version, then this should be
 * set to the same value as %SOUP_VERSION_MIN_REQUIRED.
 *
 * Since: 2.42
 */

/**
 * SOUP_VERSION_2_24:
 *
 * A macro that evaluates to the 2.24 version of libsoup, in a format
 * that can be used by %SOUP_VERSION_MIN_REQUIRED and
 * %SOUP_VERSION_MAX_ALLOWED.
 *
 * Since: 2.42
 */

/**
 * SOUP_VERSION_2_26:
 *
 * A macro that evaluates to the 2.26 version of libsoup, in a format
 * that can be used by %SOUP_VERSION_MIN_REQUIRED and
 * %SOUP_VERSION_MAX_ALLOWED.
 *
 * Since: 2.42
 */

/**
 * SOUP_VERSION_2_28:
 *
 * A macro that evaluates to the 2.28 version of libsoup, in a format
 * that can be used by %SOUP_VERSION_MIN_REQUIRED and
 * %SOUP_VERSION_MAX_ALLOWED.
 *
 * Since: 2.42
 */

/**
 * SOUP_VERSION_2_30:
 *
 * A macro that evaluates to the 2.30 version of libsoup, in a format
 * that can be used by %SOUP_VERSION_MIN_REQUIRED and
 * %SOUP_VERSION_MAX_ALLOWED.
 *
 * Since: 2.42
 */

/**
 * SOUP_VERSION_2_32:
 *
 * A macro that evaluates to the 2.32 version of libsoup, in a format
 * that can be used by %SOUP_VERSION_MIN_REQUIRED and
 * %SOUP_VERSION_MAX_ALLOWED.
 *
 * Since: 2.42
 */

/**
 * SOUP_VERSION_2_34:
 *
 * A macro that evaluates to the 2.34 version of libsoup, in a format
 * that can be used by %SOUP_VERSION_MIN_REQUIRED and
 * %SOUP_VERSION_MAX_ALLOWED.
 *
 * Since: 2.42
 */

/**
 * SOUP_VERSION_2_36:
 *
 * A macro that evaluates to the 2.36 version of libsoup, in a format
 * that can be used by %SOUP_VERSION_MIN_REQUIRED and
 * %SOUP_VERSION_MAX_ALLOWED.
 *
 * Since: 2.42
 */

/**
 * SOUP_VERSION_2_38:
 *
 * A macro that evaluates to the 2.38 version of libsoup, in a format
 * that can be used by %SOUP_VERSION_MIN_REQUIRED and
 * %SOUP_VERSION_MAX_ALLOWED.
 *
 * Since: 2.42
 */

/**
 * SOUP_VERSION_2_40:
 *
 * A macro that evaluates to the 2.40 version of libsoup, in a format
 * that can be used by %SOUP_VERSION_MIN_REQUIRED and
 * %SOUP_VERSION_MAX_ALLOWED.
 *
 * Since: 2.42
 */

/**
 * SOUP_VERSION_2_42:
 *
 * A macro that evaluates to the 2.42 version of libsoup, in a format
 * that can be used by %SOUP_VERSION_MIN_REQUIRED and
 * %SOUP_VERSION_MAX_ALLOWED.
 *
 * Since: 2.42
 */

