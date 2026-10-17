// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2009 Extreme Engineering Solutions, Inc.
 */

#include "compiler.h"

/*
 * Include additional files required for supporting different operating systems
 */

#if defined(__APPLE__) && __DARWIN_C_LEVEL < 200809L
#include "getline.c"
#endif
