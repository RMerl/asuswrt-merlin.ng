#ifndef MKSQUASHFS_H
#define MKSQUASHFS_H
/*
 * Squashfs
 *
 * Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009
 * Phillip Lougher <phillip@lougher.demon.co.uK>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * mksquashfs.h
 *
 */

#if __BYTE_ORDER == __BIG_ENDIAN
#define SQUASHFS_SWAP_SHORTS(s, d, n) swap_le16_num(s, d, n)
#define SQUASHFS_SWAP_INTS(s, d, n) swap_le32_num(s, d, n)
#define SQUASHFS_SWAP_LONG_LONGS(s, d, n) swap_le64_num(s, d, n)

#define SWAP_LE16(s, d, field)	swap_le16(&((s)->field), &((d)->field))
#define SWAP_LE32(s, d, field)	swap_le32(&((s)->field), &((d)->field))
#define SWAP_LE64(s, d, field)	swap_le64(&((s)->field), &((d)->field))
#define SWAP_LES16(s, d, field)	swap_le16((unsigned short *) &((s)->field), \
				(unsigned short *) &((d)->field))
#else
#define SQUASHFS_MEMCPY(s, d, n)	memcpy(d, s, n)
#define SQUASHFS_SWAP_SHORTS(s, d, n)	memcpy(d, s, n * sizeof(short))
#define SQUASHFS_SWAP_INTS(s, d, n)	memcpy(d, s, n * sizeof(int))
#define SQUASHFS_SWAP_LONG_LONGS(s, d, n) \
					memcpy(d, s, n * sizeof(long long))
#endif
#endif
