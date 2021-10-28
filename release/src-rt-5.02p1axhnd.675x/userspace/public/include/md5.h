/***********************************************************************
 *
 *  Copyright (c) 2016  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2016:DUAL/GPL:standard
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
:>
 *
************************************************************************/

#ifndef MD5_H
#define MD5_H

#define MD5_DIGEST_LEN     16

typedef struct MD5Context {
  u_int32_t buf[4];
  u_int32_t bits[2];
  u_char in[64];
} MD5Context;

void MD5Init(MD5Context *ctx);
void MD5Update(MD5Context *ctx, u_char const *buf, u_int len);
void MD5Final(u_char digest[16], MD5Context *ctx);

#endif
