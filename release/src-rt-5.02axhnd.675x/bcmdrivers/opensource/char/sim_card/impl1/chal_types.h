/*
 <:copyright-BRCM:2014:DUAL/GPL:standard
 
    Copyright (c) 2014 Broadcom 
    All Rights Reserved
 
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
*/

/**
*
*  @file   chal_types.h
*
*  @brief  All variable types used by cHAL are defined here
*
*  @note 
******************************************************************************/


#ifndef _CHAL_TYPES_H_
#define _CHAL_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup cHAL_Interface 
 * @{
 */

/*****************************************************************************
// typedef declarations
******************************************************************************/

typedef unsigned char      cUInt8;   ///< unsigned character (8 bits wide)
typedef unsigned short     cUInt16;  ///< unsigned short integer (16 bits wide)
typedef unsigned long      cUInt32;  ///< unsigned long integer (32 bits wide)
#ifdef __GNUC__
typedef unsigned long long cUInt64;  ///< unsigned long long integer (64 bits wide)
#else
typedef unsigned __int64   cUInt64;  ///< unsigned long long integer (64 bits wide)
#endif

typedef signed char        cInt8;    ///< signed character (8 bits wide)
typedef signed short       cInt16;   ///< signed short integer (16 bits wide)
typedef signed long        cInt32;   ///< signed long integer (32 bits wide)
#ifdef __GNUC__
typedef signed long long   cInt64;   ///< signed long long integer (64 bits wide) 
#else
typedef signed __int64     cInt64;   ///< signed long long integer (64 bits wide) 
#endif

#ifndef Boolean
typedef unsigned char      Boolean;  ///< unsiged character (8 bits wide)
#endif

typedef unsigned char      cBool;    ///< unsigned character (8 bits wide)
typedef void               cVoid;    ///< void

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

// Needed for RDB register structures
#ifndef UInt32
typedef unsigned long      UInt32;    ///< unsigned long integer (32 bits wide)
#endif

//
// For backward compatibility, will be removed
//
#ifndef UInt8
typedef unsigned char      UInt8;     ///< unsigned character (8 bits Wide)
#endif
#ifndef UInt16
typedef unsigned short     UInt16;    ///< unsigned short integer (16 bits wide)
#endif
#ifdef __GNUC__
#ifndef UInt64
typedef unsigned long long UInt64;    ///< unsigned long long integer (64 bits wide)
#endif
#else
#ifndef UInt64
typedef unsigned __int64   UInt64;    ///< unsigned long long integer (64 bits wide)
#endif
#endif

#ifndef Int8
typedef signed char        Int8;      ///< signed character (8 bits wide )
#endif
#ifndef Int16
typedef signed short       Int16;     ///< signed short integer (16 bits wide)
#endif
#ifndef Int32
typedef signed long        Int32;     ///< signed long integer (32 bits wide)
#endif

#ifdef __GNUC__
#ifndef Int64
typedef signed long long   Int64;     ///< signed long long integer (64 bits wide)
#endif
#else
#ifndef Int64
typedef signed __int64     Int64;     ///< signed long long integer (64 bits wide)
#endif
#endif

//
// Generic cHAL handler
//
#ifndef CHAL_HANDLE
typedef void* CHAL_HANDLE;            ///< void pointer (32 bits wide)
#endif

//


/** @} */

#ifdef __cplusplus
}
#endif

#endif // _CHAL_TYPES_H_

