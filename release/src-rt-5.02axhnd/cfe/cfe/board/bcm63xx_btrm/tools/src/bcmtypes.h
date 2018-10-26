
/*  *********************************************************************
    *
    <:copyright-BRCM:2012:DUAL/GPL:standard
    
       Copyright (c) 2012 Broadcom 
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
    *  
    *  bcmtypes.h misc. type re-map
    *  
    * 
    *
    *********************************************************************
*/
    
#ifndef BCMTYPES_H
#define BCMTYPES_H

typedef unsigned char   byte;
typedef unsigned char   uint8, UINT8;
typedef unsigned short  uint16, UINT16;
typedef unsigned long   uint32, UINT32, HANDLE;
typedef signed char     int8, INT8;
typedef signed short    int16, INT16;
typedef signed long     int32, INT32;

//typedef long long       int64, INT64;

typedef unsigned long   ULONG,*PULONG,DWORD,*PDWORD;
typedef signed long     LONG, *PLONG;

typedef unsigned int    UINT,*PUINT;
typedef signed int      INT,*PINT;

typedef unsigned short  USHORT,*PUSHORT;
typedef signed short    SHORT,*PSHORT,WORD,*PWORD;

typedef unsigned char   UCHAR,*PUCHAR;
typedef signed char     *PCHAR;

typedef void            VOID, *PVOID;

typedef unsigned char   BOOL, BOOLEAN, *PBOOL, *PBOOLEAN;

typedef unsigned char   BYTE,*PBYTE;


#define ARRAY_COUNT(_X_)    (sizeof(_X_) / sizeof(_X_[0]))

#define MAX_INT16 32767
#define MIN_INT16 -32768

#ifndef YES
#define YES 1
#endif

#ifndef NO
#define NO  0
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#endif
