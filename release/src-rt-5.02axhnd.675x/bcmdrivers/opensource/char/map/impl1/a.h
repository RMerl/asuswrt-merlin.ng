/*************************************************************************
 *
 * a.h :
 *   
 * Aligning Functions are Forked When Compiling Linksys Images.
 *
 * Copyright (C) 2013 CERNET Network Center
 * All rights reserved.
 * 
 * Design and coding: 
 *   Xing Li <xing@cernet.edu.cn> 
 *	 Congxiao Bao <congxiao@cernet.edu.cn>
 * 	 Yuncheng Zhu <haoyu@cernet.edu.cn>
 * 	 Wentao Shang <wentaoshang@gmail.com>
 * 	 Guoliang Han <bupthgl@gmail.com>
 * 
 * Contributions:
 *
 * This file is part of MAP-T/MAP-E Kernel Module.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * You should have received a copy of the GNU General Public License 
 * along with MAP-T/MAP-E Kernel Module. If not, see 
 * <http://www.gnu.org/licenses/>.
 *
 * For more versions, please send an email to <bupthgl@gmail.com> to
 * obtain an password to access the svn server.
 *
 * LIC: GPLv2
 *
 ************************************************************************/

#ifndef _LINUX_UNALIGNED_LE_STRUCT_H
#define _LINUX_UNALIGNED_LE_STRUCT_H

#include "packed_struct.h"
#include <linux/unaligned/packed_struct.h>

static inline u16 get_unaligned_be16(const void *p)
{
        return __get_unaligned_cpu16((const u8 *)p);
}

static inline u32 get_unaligned_be32(const void *p)
{
        return __get_unaligned_cpu32((const u8 *)p);
}

static inline u64 get_unaligned_be64(const void *p)
{
        return __get_unaligned_cpu64((const u8 *)p);
}

static inline void put_unaligned_be16(u16 val, void *p)
{
        __put_unaligned_cpu16(val, p);
}

static inline void put_unaligned_be32(u32 val, void *p)
{
        __put_unaligned_cpu32(val, p);
}

static inline void put_unaligned_be64(u64 val, void *p)
{
        __put_unaligned_cpu64(val, p);
}

#endif /* _LINUX_UNALIGNED_LE_STRUCT_H */

