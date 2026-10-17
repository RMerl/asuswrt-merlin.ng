/* SPDX-License-Identifier: GPL-2.0 */

#include "libgcc.h"

#if BITS_PER_LONG == 32

#include <div64.h>

long long __udivdi3(long long u, word_type b)
{
	long long ret = u;

	__div64_32(&ret, b);
	return ret;
}

#endif /* BITS_PER_LONG == 32 */
