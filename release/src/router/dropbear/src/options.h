#ifndef DROPBEAR_OPTIONS_H
#define DROPBEAR_OPTIONS_H

/*
Local compile-time configuration should be defined in localoptions.h
in the build directory, or src/distrooptions.h
See default_options.h.in for a description of the available options.
*/

/* Some configuration options or checks depend on system config */
#include "config.h"

/* Distribution custom build options, used if it exists */
#ifdef DISTROOPTIONS_H_EXISTS
#include "distrooptions.h"
#endif

/* Local compile-time configuration in the build directory,
 * used if it exists */
#ifdef LOCALOPTIONS_H_EXISTS
#include "localoptions.h"
#endif

/* default_options.h is processed to add #ifndef guards */
#include "default_options_guard.h"

/* Some other defines that mostly should be left alone are defined
 * in sysoptions.h */
#include "sysoptions.h"

#endif /* DROPBEAR_OPTIONS_H */
