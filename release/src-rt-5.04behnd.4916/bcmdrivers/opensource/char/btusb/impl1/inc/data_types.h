/*
 * <:copyright-BRCM:2015:GPL/GPL:standard
 * 
 *    Copyright (c) 2015 Broadcom 
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 */

#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#ifndef NULL
#define NULL     0
#endif

#ifndef FALSE
#define FALSE  0
#endif

typedef unsigned char   UINT8;
typedef unsigned short  UINT16;
typedef unsigned int    UINT32, *PUINT32;
typedef signed   int    INT32, *PINT32;
typedef signed   char   INT8;
typedef signed   short  INT16;


#if defined(_WIDE_CHAR) || defined(_UNICODE)
typedef unsigned short  WIDECHAR;      /* for _BT_WIN32 possibly use wchar_t */
#define WIDE_NULL_CHAR  ((WIDECHAR)0)
#else
typedef unsigned char   WIDECHAR;
#define WIDE_NULL_CHAR  '\0'
#endif

typedef UINT32          TIME_STAMP;


#ifndef _WINDOWS_VXD
typedef unsigned char   BOOLEAN;

#ifndef TRUE
#define TRUE   (!FALSE)
#endif
#endif

typedef unsigned char   UBYTE;

#define PACKED
#define INLINE

#ifndef BIG_ENDIAN
#define BIG_ENDIAN FALSE
#endif

#define UINT16_LOW_BYTE(x)      ((x) & 0xff)
#define UINT16_HI_BYTE(x)       ((x) >> 8)

#define ARRAY_SIZEOF(x) (sizeof(x)/sizeof(x[0]))

#endif
