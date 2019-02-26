/*
 * Copyright (C) 2010-2015 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup android android
 * @{ @ingroup compat
 */

#ifndef ANDROID_H_
#define ANDROID_H_

#include <android/api-level.h>

/* stuff defined in AndroidConfig.h, which is included using the -include
 * command-line option, thus cannot be undefined using -U CFLAGS options.
 * the reason we have to undefine these flags in the first place, is that
 * AndroidConfig.h defines them as 0, which in turn means that they are
 * actually defined. */
#undef HAVE_BACKTRACE

/* API level 21 changed quite a few things, we define some stuff here and not
 * via CFLAGS in Android.mk files as it is easier to compare versions */
#if __ANDROID_API__ >= 21

#define HAVE_PTHREAD_CONDATTR_INIT 1
#define HAVE_CONDATTR_CLOCK_MONOTONIC 1

#define HAVE_SYS_CAPABILITY_H 1

#else /* __ANDROID_API__ */

#define HAVE_PTHREAD_COND_TIMEDWAIT_MONOTONIC 1

#endif /* __ANDROID_API__ */

#endif /** ANDROID_H_ @}*/
