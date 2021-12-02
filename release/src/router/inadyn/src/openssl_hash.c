/* OpenSSL interface for hash functions
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
#include <openssl/md5.h>
#include <openssl/sha.h>

/* Calculate the MD5 hash checksum of the given input */
void md5(const unsigned char *input, size_t ilen, unsigned char output[16])
{
	MD5_CTX ctx;

	MD5_Init(&ctx);
	MD5_Update(&ctx, input, ilen);
	MD5_Final(output, &ctx);
}

/* Calculate the SHA-1 hash checksum of the given input */
void sha1(const unsigned char *input, size_t ilen, unsigned char output[20])
{
	SHA_CTX ctx;

	SHA1_Init(&ctx);
	SHA1_Update(&ctx, input, ilen);
	SHA1_Final(output, &ctx);
}
