/* camellia-vaes-avx2-amd64.S  -  VAES/AVX2 implementation of Camellia cipher
 *
 * Copyright (C) 2021 Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#ifdef __x86_64
#if (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS)) && \
    defined(ENABLE_AESNI_SUPPORT) && defined(ENABLE_AVX2_SUPPORT) && \
    defined(HAVE_GCC_INLINE_ASM_VAES_VPCLMUL)

#define CAMELLIA_VAES_BUILD 1
#define FUNC_NAME(func) _gcry_camellia_vaes_avx2_ ## func

#include "camellia-aesni-avx2-amd64.h"

#endif /* defined(ENABLE_AESNI_SUPPORT) && defined(ENABLE_AVX2_SUPPORT) */
#endif /* __x86_64 */
