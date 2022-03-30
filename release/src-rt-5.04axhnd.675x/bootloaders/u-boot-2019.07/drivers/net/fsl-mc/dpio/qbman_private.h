/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Freescale Semiconductor
 */

/* Perform extra checking */
#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/types.h>
#include <asm/atomic.h>
#include <malloc.h>
#include <asm/arch/soc.h>
#include <fsl-mc/fsl_qbman_base.h>

#define QBMAN_CHECKING

/* Any time there is a register interface which we poll on, this provides a
 * "break after x iterations" scheme for it. It's handy for debugging, eg.
 * where you don't want millions of lines of log output from a polling loop
 * that won't, because such things tend to drown out the earlier log output
 * that might explain what caused the problem. (NB: put ";" after each macro!)
 * TODO: we should probably remove this once we're done sanitising the
 * simulator...
 */
#define DBG_POLL_START(loopvar) (loopvar = 10)
#define DBG_POLL_CHECK(loopvar) \
	do {if (!(loopvar--)) BUG_ON(NULL == "DBG_POLL_CHECK"); } while (0)

/* For CCSR or portal-CINH registers that contain fields at arbitrary offsets
 * and widths, these macro-generated encode/decode/isolate/remove inlines can
 * be used.
 *
 * Eg. to "d"ecode a 14-bit field out of a register (into a "uint16_t" type),
 * where the field is located 3 bits "up" from the least-significant bit of the
 * register (ie. the field location within the 32-bit register corresponds to a
 * mask of 0x0001fff8), you would do;
 *                uint16_t field = d32_uint16_t(3, 14, reg_value);
 *
 * Or to "e"ncode a 1-bit boolean value (input type is "int", zero is FALSE,
 * non-zero is TRUE, so must convert all non-zero inputs to 1, hence the "!!"
 * operator) into a register at bit location 0x00080000 (19 bits "in" from the
 * LS bit), do;
 *                reg_value |= e32_int(19, 1, !!field);
 *
 * If you wish to read-modify-write a register, such that you leave the 14-bit
 * field as-is but have all other fields set to zero, then "i"solate the 14-bit
 * value using;
 *                reg_value = i32_uint16_t(3, 14, reg_value);
 *
 * Alternatively, you could "r"emove the 1-bit boolean field (setting it to
 * zero) but leaving all other fields as-is;
 *                reg_val = r32_int(19, 1, reg_value);
 *
 */
#define MAKE_MASK32(width) (width == 32 ? 0xffffffff : \
				 (uint32_t)((1 << width) - 1))
#define DECLARE_CODEC32(t) \
static inline uint32_t e32_##t(uint32_t lsoffset, uint32_t width, t val) \
{ \
	BUG_ON(width > (sizeof(t) * 8)); \
	return ((uint32_t)val & MAKE_MASK32(width)) << lsoffset; \
} \
static inline t d32_##t(uint32_t lsoffset, uint32_t width, uint32_t val) \
{ \
	BUG_ON(width > (sizeof(t) * 8)); \
	return (t)((val >> lsoffset) & MAKE_MASK32(width)); \
} \
static inline uint32_t i32_##t(uint32_t lsoffset, uint32_t width, \
				uint32_t val) \
{ \
	BUG_ON(width > (sizeof(t) * 8)); \
	return e32_##t(lsoffset, width, d32_##t(lsoffset, width, val)); \
} \
static inline uint32_t r32_##t(uint32_t lsoffset, uint32_t width, \
				uint32_t val) \
{ \
	BUG_ON(width > (sizeof(t) * 8)); \
	return ~(MAKE_MASK32(width) << lsoffset) & val; \
}
DECLARE_CODEC32(uint32_t)
DECLARE_CODEC32(uint16_t)
DECLARE_CODEC32(uint8_t)
DECLARE_CODEC32(int)

	/*********************/
	/* Debugging assists */
	/*********************/

static inline void __hexdump(unsigned long start, unsigned long end,
			unsigned long p, size_t sz, const unsigned char *c)
{
	while (start < end) {
		unsigned int pos = 0;
		char buf[64];
		int nl = 0;

		pos += sprintf(buf + pos, "%08lx: ", start);
		do {
			if ((start < p) || (start >= (p + sz)))
				pos += sprintf(buf + pos, "..");
			else
				pos += sprintf(buf + pos, "%02x", *(c++));
			if (!(++start & 15)) {
				buf[pos++] = '\n';
				nl = 1;
			} else {
				nl = 0;
				if (!(start & 1))
					buf[pos++] = ' ';
				if (!(start & 3))
					buf[pos++] = ' ';
			}
		} while (start & 15);
		if (!nl)
			buf[pos++] = '\n';
		buf[pos] = '\0';
		debug("%s", buf);
	}
}
static inline void hexdump(const void *ptr, size_t sz)
{
	unsigned long p = (unsigned long)ptr;
	unsigned long start = p & ~(unsigned long)15;
	unsigned long end = (p + sz + 15) & ~(unsigned long)15;
	const unsigned char *c = ptr;

	__hexdump(start, end, p, sz, c);
}

#if defined(__BIG_ENDIAN)
#define DQRR_TOK_OFFSET 0
#else
#define DQRR_TOK_OFFSET 24
#endif

/* Similarly-named functions */
#define upper32(a) upper_32_bits(a)
#define lower32(a) lower_32_bits(a)

	/****************/
	/* arch assists */
	/****************/

static inline void dcbz(void *ptr)
{
	uint32_t *p = ptr;
	BUG_ON((unsigned long)ptr & 63);
	p[0] = 0;
	p[1] = 0;
	p[2] = 0;
	p[3] = 0;
	p[4] = 0;
	p[5] = 0;
	p[6] = 0;
	p[7] = 0;
	p[8] = 0;
	p[9] = 0;
	p[10] = 0;
	p[11] = 0;
	p[12] = 0;
	p[13] = 0;
	p[14] = 0;
	p[15] = 0;
}

#define lwsync()

void qbman_version(u32 *major, u32 *minor)
{
	u32 svr_dev_id;

	/*
	 * LS2080A SoC and its personalities has qbman cotroller version 4.0
	 * New SoCs like LS2088A, LS1088A has qbman conroller version 4.1
	 */
	svr_dev_id = get_svr();
	if (IS_SVR_DEV(svr_dev_id, SVR_DEV(SVR_LS2080A))) {
		*major = 4;
		*minor = 0;
	} else {
		*major = 4;
		*minor = 1;
	}
}

#include "qbman_sys.h"
