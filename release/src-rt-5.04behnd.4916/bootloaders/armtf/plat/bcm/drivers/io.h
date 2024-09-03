/*
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom 
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
/*
 * io.h for atf
 * Created by <nschichan@freebox.fr> on Thu Apr 18 19:26:20 2019
 */

#pragma once

#include <mmio.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t  s8;

static inline u64 readq(void *mem)
{
	return mmio_read_64((uintptr_t)mem);
}

static inline u32 readl(void *mem)
{
	return mmio_read_32((uintptr_t)mem);
}

static inline u16 readw(void *mem)
{
	return mmio_read_16((uintptr_t)mem);
}

static inline u8 readb(void *mem)
{
	return mmio_read_8((uintptr_t)mem);
}

static inline void writeq(u64 v, void *mem)
{
	return mmio_write_64((uintptr_t)mem, v);
}

static inline void writel(u32 v, void *mem)
{
	return mmio_write_32((uintptr_t)mem, v);
}

static inline void writew(u16 v, void *mem)
{
	return mmio_write_16((uintptr_t)mem, v);
}

static inline void writeb(u8 v, void *mem)
{
	return mmio_write_8((uintptr_t)mem, v);
}
