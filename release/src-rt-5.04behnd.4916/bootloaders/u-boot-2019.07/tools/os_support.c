// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2009 Extreme Engineering Solutions, Inc.
 */

/*
 * Include additional files required for supporting different operating systems
 */
#include "compiler.h"
#ifdef __MINGW32__
#include "mingw_support.c"
#endif
#if defined(__APPLE__) && __DARWIN_C_LEVEL < 200809L
#include "getline.c"
#endif
