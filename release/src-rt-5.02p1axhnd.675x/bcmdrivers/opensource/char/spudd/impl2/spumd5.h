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
 *  Description: Header file for MD5 Authentication Algorithm implementation
 *  File: spumd5.h
 *  Author: Pavan Kumar
 *  Date: 11/16/2007
 *
 *****************************************************************************/
#ifndef _MD5_H_
#define _MD5_H_

#define MD5_BLOCK_LENGTH  64    /* in bytes */
#define MD5_HASH_LENGTH   16    /* in bytes */
#define BYTESWAPSHORT(sval) ((((sval)&0xff00)>>8)+(((sval)&0xff)<<8))
#define BYTESWAPLONG(lval) (((BYTESWAPSHORT((lval)>>16))) + \
                           ((BYTESWAPSHORT((lval)&0xffff)<<16)))

typedef int uint32;
struct MD5Context {
    uint32 buf[4];
    uint32 bits[2];
    unsigned char in[64];
};

#endif /* _MD5_H_ */
