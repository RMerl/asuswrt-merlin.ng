#ifndef DROPBEAR_OPTIONS_H
#define DROPBEAR_OPTIONS_H

/* 
            > > > Don't edit this file any more! < < <
            
Local compile-time configuration should be defined in localoptions.h
See default_options.h.in for a description of the available options.
*/

/* Some configuration options or checks depend on system config */
#include "config.h"

#ifdef LOCALOPTIONS_H_EXISTS
#include "localoptions.h"
#endif

/* default_options.h is processed to add #ifndef guards */
#include "default_options_guard.h"

/* Some other defines that mostly should be left alone are defined
 * in sysoptions.h */
#include "sysoptions.h"

#endif /* DROPBEAR_OPTIONS_H */
