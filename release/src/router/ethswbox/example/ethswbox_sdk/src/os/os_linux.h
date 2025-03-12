#ifndef _OS_LINUX_H
#define _OS_LINUX_H
/******************************************************************************

    Copyright 2022 Maxlinear

    SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0-only)

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.
******************************************************************************/

#ifdef __cplusplus
   extern "C" {
#endif

/* ============================= */
/* Includes                      */
/* ============================= */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>				  

#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/select.h>


#include "os_types.h"



/** A type for handling boolean issues. */
typedef enum {
   /** false */
   OS_FALSE = 0,
   /** true */
   OS_TRUE = 1
} OS_boolean_t;


/**
   This type is used for parameters that should enable
   and disable a dedicated feature. */
typedef enum {
   /** disable */
   OS_DISABLE = 0,
   /** enable */
   OS_ENABLE = 1
} OS_enDis_t;

/**
   This type has two states, success and error
*/
typedef enum {
   /** operation failed */
   OS_ERROR   = (-1),
   /** operation succeeded */
   OS_SUCCESS = 0
} OS_return_t;


#ifdef __cplusplus
}
#endif

#endif /* _OS_LINUX_H */
