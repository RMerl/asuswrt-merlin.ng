/*
 *  Embedded Linux library
 *
 *  Copyright (C) 2015  Intel Corporation. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __ELL_BASE64_H
#define __ELL_BASE64_H

#ifdef __cplusplus
extern "C" {
#endif

uint8_t *l_base64_decode(const char *in, size_t in_len, size_t *n_written);

char *l_base64_encode(const uint8_t *in, size_t in_len, int columns,
				size_t *n_written);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_BASE64_H */
