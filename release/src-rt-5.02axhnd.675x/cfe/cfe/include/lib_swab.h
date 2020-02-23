/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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

#ifndef __LIB_SWAB_H__
#define __LIB_SWAB_H__

#define ___swab16(x) \
	((unsigned short)( \
		(((unsigned short)(x) & (unsigned short)0x00ffU) << 8) | \
		(((unsigned short)(x) & (unsigned short)0xff00U) >> 8) ))

#define ___swab32(x) \
	((unsigned int)( \
		(((unsigned int)(x) & (unsigned int)0x000000ffUL) << 24) | \
		(((unsigned int)(x) & (unsigned int)0x0000ff00UL) <<  8) | \
		(((unsigned int)(x) & (unsigned int)0x00ff0000UL) >>  8) | \
		(((unsigned int)(x) & (unsigned int)0xff000000UL) >> 24) )) 

#define ___swab64(x) \
	((unsigned long long)( \
		(((unsigned long long)(x) & (unsigned long long)0x00000000000000ffULL) << 56) |	\
		(((unsigned long long)(x) & (unsigned long long)0x000000000000ff00ULL) << 40) |	\
		(((unsigned long long)(x) & (unsigned long long)0x0000000000ff0000ULL) << 24) |	\
		(((unsigned long long)(x) & (unsigned long long)0x00000000ff000000ULL) <<  8) |	\
		(((unsigned long long)(x) & (unsigned long long)0x000000ff00000000ULL) >>  8) |	\
		(((unsigned long long)(x) & (unsigned long long)0x0000ff0000000000ULL) >> 24) |	\
		(((unsigned long long)(x) & (unsigned long long)0x00ff000000000000ULL) >> 40) |	\
		(((unsigned long long)(x) & (unsigned long long)0xff00000000000000ULL) >> 56)))

#define swab16 ___swab16
#define swab32 ___swab32
#define swab64 ___swab64

#endif
