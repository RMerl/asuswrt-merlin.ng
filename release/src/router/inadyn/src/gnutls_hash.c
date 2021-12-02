/* Nettle interface for hash functions
 *
 * Copyright (C) 2021  Dan Fandrich <dan@coneharvesters.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, visit the Free Software Foundation
 * website at http://www.gnu.org/licenses/gpl-2.0.html or write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "md5.h"
#include "sha1.h"
#include <nettle/md5.h>
#include <nettle/sha.h>

/* Calculate the MD5 hash checksum of the given input */
void md5(const unsigned char *input, size_t ilen, unsigned char output[16])
{
	struct md5_ctx ctx;

	md5_init(&ctx);
	md5_update(&ctx, ilen, input);
	md5_digest(&ctx, MD5_DIGEST_SIZE, output);
}

/* Calculate the SHA-1 hash checksum of the given input */
void sha1(const unsigned char *input, size_t ilen, unsigned char output[20])
{
	struct sha1_ctx ctx;

	sha1_init(&ctx);
	sha1_update(&ctx, ilen, input);
	sha1_digest(&ctx, SHA1_DIGEST_SIZE, output);
}
