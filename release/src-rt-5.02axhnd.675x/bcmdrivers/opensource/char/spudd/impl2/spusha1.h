/*
<:copyright-BRCM:2007:GPL/GPL:standard

   Copyright (c) 2007 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
/******************************************************************************
 *
 *  Broadcom IPsec SPU Driver Common API
 *  Description: Header file for SHA1 Authentication algorithm implementation
 *  File: spusha1.h
 *  Author: Pavan Kumar
 *  Date: 11/16/2007
 *
 *****************************************************************************/
#ifndef _SHA_H_
#define _SHA_H_

/* define the following line to get FIPS 180-1 enhancements */
#define SHA_UPDATE

#define SHA_BLOCK_LENGTH  64    /* in bytes */
#define SHA_HASH_LENGTH   20    /* in bytes */

typedef unsigned char BYTE;

typedef struct {
    unsigned int Numbytes;
    unsigned int Numblocks[2];    /* each block contains 64 bytes */
    unsigned int Mblock[16];
    unsigned int buffer[5];
} SHA_CTX;

#endif /*_SHA_H_*/
