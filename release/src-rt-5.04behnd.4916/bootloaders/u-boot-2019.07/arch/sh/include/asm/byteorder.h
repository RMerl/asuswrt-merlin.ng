/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __ASM_SH_BYTEORDER_H_
#define __ASM_SH_BYTEORDER_H_

#include <config.h>
#include <asm/types.h>

#ifdef __LITTLE_ENDIAN__
#include <linux/byteorder/little_endian.h>
#else
#include <linux/byteorder/big_endian.h>
#endif

#endif
