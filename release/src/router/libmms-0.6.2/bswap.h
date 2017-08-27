#ifndef BSWAP_H_INCLUDED
#define BSWAP_H_INCLUDED

/*
 * Copyright (C) 2004 Maciej Katafiasz <mathrick@users.sourceforge.net>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#include <inttypes.h>

#define SWAP_ENDIAN_16(val) \
	(val[1] | (val[0] << 8))
#define SWAP_ENDIAN_32(val) \
	(val[3] | (val[2] << 8) | (val[1] << 16) | (val[0] << 24))
#define SWAP_ENDIAN_64(val) \
	(val[7] | (val[6] << 8) | (val[5] << 16) | (val[4] << 24) | \
	((uint64_t)val[3] << 32) | ((uint64_t)val[2] << 40) | \
	((uint64_t)val[1] << 48) | ((uint64_t)val[0] << 56))

#define SAME_ENDIAN_16(val) \
	(val[0] | (val[1] << 8))
#define SAME_ENDIAN_32(val) \
	(val[0] | (val[1] << 8) | (val[2] << 16) | (val[3] << 24))
#define SAME_ENDIAN_64(val) \
	(val[0] | (val[1] << 8) | (val[2] << 16) | (val[3] << 24) | \
	((uint64_t)val[4] << 32) | ((uint64_t)val[5] << 40) | \
	((uint64_t)val[6] << 48) | ((uint64_t)val[7] << 56))

#ifndef WORDS_BIGENDIAN

/* Little endian */

#define LE_16(val) SAME_ENDIAN_16(((uint8_t *)(val)))
#define LE_32(val) SAME_ENDIAN_32(((uint8_t *)(val)))
#define LE_64(val) SAME_ENDIAN_64(((uint8_t *)(val)))
#define BE_16(val) SWAP_ENDIAN_16(((uint8_t *)(val)))
#define BE_32(val) SWAP_ENDIAN_32(((uint8_t *)(val)))
#define BE_64(val) SWAP_ENDIAN_64(((uint8_t *)(val)))

#elif WORDS_BIGENDIAN == 1

/* Big endian */

#define LE_16(val) SWAP_ENDIAN_16(((uint8_t *)(val)))
#define LE_32(val) SWAP_ENDIAN_32(((uint8_t *)(val)))
#define LE_64(val) SWAP_ENDIAN_64(((uint8_t *)(val)))
#define BE_16(val) SAME_ENDIAN_16(((uint8_t *)(val)))
#define BE_32(val) SAME_ENDIAN_32(((uint8_t *)(val)))
#define BE_64(val) SAME_ENDIAN_64(((uint8_t *)(val)))

#else
#error Unknown endianness!
#endif

#endif /* BSWAP_H_INCLUDED */
