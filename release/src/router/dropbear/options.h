#ifndef DROPBEAR_OPTIONS_H
#define DROPBEAR_OPTIONS_H

/* 
Local compile-time configuration should be defined in localoptions.h
See default_options.h.in for a description of the available options.
*/

#ifdef LOCALOPTIONS_H_EXISTS
#include "localoptions.h"
#endif

#include "default_options.h"

/* Some other defines that mostly should be left alone are defined
 * in sysoptions.h */
#include "sysoptions.h"

#endif /* DROPBEAR_OPTIONS_H */
