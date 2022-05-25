/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
 *
 ************************************************************************/

#ifndef __OS_DEFS_H__
#define __OS_DEFS_H__

#include <unistd.h>  /* for getopt */
#include <stdio.h>   /* for snprintf */
#include <stdint.h>  /* for the various integer types */
#include <stdlib.h>  /* for NULL */
#include <string.h>  /* for strlen, strncpy */
#include <ctype.h>   /* for isdigit */
#include <syslog.h>  /* for syslog */
#include <stdarg.h>  /* for va_list */
#include <inttypes.h>  /* for PRI macros */

#include "number_defs.h"  /* BCM defs for various number types */


/*!\file os_defs.h
 * \brief Various commonly used, but OS dependent definitions are defined here.
 *        Do not include any CMS specific defs here!
 *
 *  This file is for Linux.
 */


#ifndef BOOL_TYPE_ALREADY_DEFINED
#define BOOL_TYPE_ALREADY_DEFINED

/**Boolean type; use 1 byte only, possible values are TRUE(1) or FALSE(0) only.
 *
 * TRUE/FALSE defined in cms.h
 */
typedef uint8_t    UBOOL8;

#endif /* BOOL_TYPE_ALREADY_DEFINED */


#ifndef TRUE
/** TRUE = 1
 */
#define TRUE  1
#endif

#ifndef FALSE
/** FALSE = 0
 */
#define FALSE 0
#endif


/** Base64 encoded string representation of binary data.
 *
 * This is to support TR69 data types.
 */
typedef char *     BASE64;


/** Hex encoded string representation of binary data.
 * This data type was introduced in TR-106 Issue 1, Admendment 2, Sept. 2008
 *
 * This is to support TR69 data types.
 */
typedef char *     HEXBINARY;


/** String representation of date and time.
 *
 * This is to support TR69 data types.
 */
typedef char *     DATETIME;


/** String representation of IPv4/6 address.
 *
 * This is to support TR69 data types.
 */
typedef char *     IPADDRESS;


/** String representation of MAC address.
 *
 * This is to support TR69 data types.
 */
typedef char *     MACADDRESS;


#define MAC_ADDR_LEN    6     //!< Mac address len in an array of 6 bytes
#define MAC_STR_LEN     17    //!< Mac String len with ":". eg: xx:xx:xx:xx:xx:xx


/** Invalid file descriptor number */
#define CMS_INVALID_FD  (-1)


/** Invalid process id.
 *
 * Management entities should not need to use this constant.  It is used
 * by OS dependent code in the OAL layer.  But I put this constant here
 * to make it easier to find.
 */
#define CMS_INVALID_PID   0


/** Maximum value for a UINT64 */
#define MAX_UINT64 18446744073709551615ULL

/** Maximum value for a SINT64 */
#define MAX_SINT64 9223372036854775807LL

/** Minimum value for a SINT64 */
#define MIN_SINT64 (-1 * MAX_SINT64 - 1)

/** Maximum value for a UINT32 */
#define MAX_UINT32 4294967295U

/** Maximum value for a SINT32 */
#define MAX_SINT32 2147483647

/** Minimum value for a SINT32 */
#define MIN_SINT32 (-2147483648)

/** Maximum value for a UINT16 */
#define MAX_UINT16  65535

/** Maximum value for a SINT16 */
#define MAX_SINT16  32767

/** Minimum value for a SINT16 */
#define MIN_SINT16  (-32768)


/**************************************************************************
 *
 * The following defs really belong in cms.h, but is used by some code (omci)
 * that tries to be independent of CMS.  So put them here for now.
 *
 **************************************************************************/


/** broadcom interface name length (although Linux uses IFNAMSIZ which is 15) */
#define CMS_IFNAME_LENGTH  32


/** Maximum depth of objects in the Data Model that we can support.
 *  If the data model has a greater actual depth than what is defined here,
 *  cmsMdm_init() will fail.
 */
#define MAX_MDM_INSTANCE_DEPTH    6


/** A structure to keep track of MDM fullpath instance information.
 *
 * External callers can treat this as an opaque handle.
 */
typedef struct
{
   UINT8 currentDepth;                     /**< next index in the instance array
                                            *   to fill.  0 means empty. */
   UINT32 instance[MAX_MDM_INSTANCE_DEPTH];/**< Array of instance id's. */
} InstanceIdStack;


/** A number to identify a MdmObject (but not the specific instance of
 *  the object).
 *
 * MdmObjectId's are defined in mdm_oid.h.
 */
typedef UINT16 MdmObjectId;


/**
 * This is common used string length types.
 */
#define BUFLEN_4        4     //!< buffer length 4
#define BUFLEN_8        8     //!< buffer length 8
#define BUFLEN_16       16    //!< buffer length 16
#define BUFLEN_18       18    //!< buffer length 18 -- for ppp session id
#define BUFLEN_24       24    //!< buffer length 24 -- mostly for password
#define BUFLEN_32       32    //!< buffer length 32
#define BUFLEN_40       40    //!< buffer length 40
#define BUFLEN_48       48    //!< buffer length 48
#define BUFLEN_64       64    //!< buffer length 64
#define BUFLEN_80       80    //!< buffer length 80
#define BUFLEN_128      128   //!< buffer length 128
#define BUFLEN_256      256   //!< buffer length 256
#define BUFLEN_264      264   //!< buffer length 264
#define BUFLEN_512      512   //!< buffer length 512
#define BUFLEN_1024     1024  //!< buffer length 1024

#endif /* __OS_DEFS_H__ */
