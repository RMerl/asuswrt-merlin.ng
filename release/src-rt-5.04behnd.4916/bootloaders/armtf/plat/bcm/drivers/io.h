/*
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom 
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
