/* SPDX-License-Identifier: LGPL-2.0+ */
/*
 * Copyright 2009 Extreme Engineering Solutions, Inc.
 */

#ifndef __OS_SUPPORT_H_
#define __OS_SUPPORT_H_

#include "compiler.h"

/*
 * Include additional files required for supporting different operating systems
 */
#ifdef __MINGW32__
#include "mingw_support.h"
#endif

#if defined(__APPLE__) && __DARWIN_C_LEVEL < 200809L
#include "getline.h"
#endif

#endif /* __OS_SUPPORT_H_ */
