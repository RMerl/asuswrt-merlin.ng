// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>

/* Memory test
 *
 * General observations:
 * o The recommended test sequence is to test the data lines: if they are
 *   broken, nothing else will work properly.  Then test the address
 *   lines.  Finally, test the cells in the memory now that the test
 *   program knows that the address and data lines work properly.
 *   This sequence also helps isolate and identify what is faulty.
 *
 * o For the address line test, it is a good idea to use the base
 *   address of the lowest memory location, which causes a '1' bit to
 *   walk through a field of zeros on the address lines and the highest
 *   memory location, which causes a '0' bit to walk through a field of
 *   '1's on the address line.
 *
 * o Floating buses can fool memory tests if the test routine writes
 *   a value and then reads it back immediately.  The problem is, the
 *   write will charge the residual capacitance on the data bus so the
 *   bus retains its state briefely.  When the test program reads the
 *   value back immediately, the capacitance of the bus can allow it
 *   to read back what was written, even though the memory circuitry
 *   is broken.  To avoid this, the test program should write a test
 *   pattern to the target location, write a different pattern elsewhere
 *   to charge the residual capacitance in a differnt manner, then read
 *   the target location back.
 *
 * o Always read the target location EXACTLY ONCE and save it in a local
 *   variable.  The problem with reading the target location more than
 *   once is that the second and subsequent reads may work properly,
 *   resulting in a failed test that tells the poor technician that
 *   "Memory error at 00000000, wrote aaaaaaaa, read aaaaaaaa" which
 *   doesn't help him one bit and causes puzzled phone calls.  Been there,
 *   done that.
 *
 * Data line test:
 * ---------------
 * This tests data lines for shorts and opens by forcing adjacent data
 * to opposite states. Because the data lines could be routed in an
 * arbitrary manner the must ensure test patterns ensure that every case
 * is tested. By using the following series of binary patterns every
 * combination of adjacent bits is test regardless of routing.
 *
 *     ...101010101010101010101010
 *     ...110011001100110011001100
 *     ...111100001111000011110000
 *     ...111111110000000011111111
 *
 * Carrying this out, gives us six hex patterns as follows:
 *
 *     0xaaaaaaaaaaaaaaaa
 *     0xcccccccccccccccc
 *     0xf0f0f0f0f0f0f0f0
 *     0xff00ff00ff00ff00
 *     0xffff0000ffff0000
 *     0xffffffff00000000
 *
 * To test for short and opens to other signals on our boards, we
 * simply test with the 1's complemnt of the paterns as well, resulting
 * in twelve patterns total.
 *
 * After writing a test pattern. a special pattern 0x0123456789ABCDEF is
 * written to a different address in case the data lines are floating.
 * Thus, if a byte lane fails, you will see part of the special
 * pattern in that byte lane when the test runs.  For example, if the
 * xx__xxxxxxxxxxxx byte line fails, you will see aa23aaaaaaaaaaaa
 * (for the 'a' test pattern).
 *
 * Address line test:
 * ------------------
 *  This function performs a test to verify that all the address lines
 *  hooked up to the RAM work properly.  If there is an address line
 *  fault, it usually shows up as two different locations in the address
 *  map (related by the faulty address line) mapping to one physical
 *  memory storage location.  The artifact that shows up is writing to
 *  the first location "changes" the second location.
 *
 * To test all address lines, we start with the given base address and
 * xor the address with a '1' bit to flip one address line.  For each
 * test, we shift the '1' bit left to test the next address line.
 *
 * In the actual code, we start with address sizeof(ulong) since our
 * test pattern we use is a ulong and thus, if we tried to test lower
 * order address bits, it wouldn't work because our pattern would
 * overwrite itself.
 *
 * Example for a 4 bit address space with the base at 0000:
 *   0000 <- base
 *   0001 <- test 1
 *   0010 <- test 2
 *   0100 <- test 3
 *   1000 <- test 4
 * Example for a 4 bit address space with the base at 0010:
 *   0010 <- base
 *   0011 <- test 1
 *   0000 <- (below the base address, skipped)
 *   0110 <- test 2
 *   1010 <- test 3
 *
 * The test locations are successively tested to make sure that they are
 * not "mirrored" onto the base address due to a faulty address line.
 * Note that the base and each test location are related by one address
 * line flipped.  Note that the base address need not be all zeros.
 *
 * Memory tests 1-4:
 * -----------------
 * These tests verify RAM using sequential writes and reads
 * to/from RAM. There are several test cases that use different patterns to
 * verify RAM. Each test case fills a region of RAM with one pattern and
 * then reads the region back and compares its contents with the pattern.
 * The following patterns are used:
 *
 *  1a) zero pattern (0x00000000)
 *  1b) negative pattern (0xffffffff)
 *  1c) checkerboard pattern (0x55555555)
 *  1d) checkerboard pattern (0xaaaaaaaa)
 *  2)  bit-flip pattern ((1 << (offset % 32))
 *  3)  address pattern (offset)
 *  4)  address pattern (~offset)
 *
 * Being run in normal mode, the test verifies only small 4Kb
 * regions of RAM around each 1Mb boundary. For example, for 64Mb
 * RAM the following areas are verified: 0x00000000-0x00000800,
 * 0x000ff800-0x00100800, 0x001ff800-0x00200800, ..., 0x03fff800-
 * 0x04000000. If the test is run in slow-test mode, it verifies
 * the whole RAM.
 */

#include <post.h>
#include <watchdog.h>

#if CONFIG_POST & (CONFIG_SYS_POST_MEMORY | CONFIG_SYS_POST_MEM_REGIONS)

DECLARE_GLOBAL_DATA_PTR;

/*
 * Define INJECT_*_ERRORS for testing error detection in the presence of
 * _good_ hardware.
 */
#undef  INJECT_DATA_ERRORS
#undef  INJECT_ADDRESS_ERRORS

#ifdef INJECT_DATA_ERRORS
#warning "Injecting data line errors for testing purposes"
#endif

#ifdef INJECT_ADDRESS_ERRORS
#warning "Injecting address line errors for testing purposes"
#endif


/*
 * This function performs a double word move from the data at
 * the source pointer to the location at the destination pointer.
 * This is helpful for testing memory on processors which have a 64 bit
 * wide data bus.
 *
 * On those PowerPC with FPU, use assembly and a floating point move:
 * this does a 64 bit move.
 *
 * For other processors, let the compiler generate the best code it can.
 */
static void move64(const unsigned long long *src, unsigned long long *dest)
{
	*dest = *src;
}

/*
 * This is 64 bit wide test patterns.  Note that they reside in ROM
 * (which presumably works) and the tests write them to RAM which may
 * not work.
 *
 * The "otherpattern" is written to drive the data bus to values other
 * than the test pattern.  This is for detecting floating bus lines.
 *
 */
const static unsigned long long pattern[] = {
	0xaaaaaaaaaaaaaaaaULL,
	0xccccccccccccccccULL,
	0xf0f0f0f0f0f0f0f0ULL,
	0xff00ff00ff00ff00ULL,
	0xffff0000ffff0000ULL,
	0xffffffff00000000ULL,
	0x00000000ffffffffULL,
	0x0000ffff0000ffffULL,
	0x00ff00ff00ff00ffULL,
	0x0f0f0f0f0f0f0f0fULL,
	0x3333333333333333ULL,
	0x5555555555555555ULL
};
const unsigned long long otherpattern = 0x0123456789abcdefULL;


static int memory_post_dataline(unsigned long long * pmem)
{
	unsigned long long temp64 = 0;
	int num_patterns = ARRAY_SIZE(pattern);
	int i;
	unsigned int hi, lo, pathi, patlo;
	int ret = 0;

	for ( i = 0; i < num_patterns; i++) {
		move64(&(pattern[i]), pmem++);
		/*
		 * Put a different pattern on the data lines: otherwise they
		 * may float long enough to read back what we wrote.
		 */
		move64(&otherpattern, pmem--);
		move64(pmem, &temp64);

#ifdef INJECT_DATA_ERRORS
		temp64 ^= 0x00008000;
#endif

		if (temp64 != pattern[i]){
			pathi = (pattern[i]>>32) & 0xffffffff;
			patlo = pattern[i] & 0xffffffff;

			hi = (temp64>>32) & 0xffffffff;
			lo = temp64 & 0xffffffff;

			post_log("Memory (data line) error at %08x, "
				  "wrote %08x%08x, read %08x%08x !\n",
					  pmem, pathi, patlo, hi, lo);
			ret = -1;
		}
	}
	return ret;
}

static int memory_post_addrline(ulong *testaddr, ulong *base, ulong size)
{
	ulong *target;
	ulong *end;
	ulong readback;
	ulong xor;
	int   ret = 0;

	end = (ulong *)((ulong)base + size);	/* pointer arith! */
	xor = 0;
	for(xor = sizeof(ulong); xor > 0; xor <<= 1) {
		target = (ulong *)((ulong)testaddr ^ xor);
		if((target >= base) && (target < end)) {
			*testaddr = ~*target;
			readback  = *target;

#ifdef INJECT_ADDRESS_ERRORS
			if(xor == 0x00008000) {
				readback = *testaddr;
			}
#endif
			if(readback == *testaddr) {
				post_log("Memory (address line) error at %08x<->%08x, "
					"XOR value %08x !\n",
					testaddr, target, xor);
				ret = -1;
			}
		}
	}
	return ret;
}

static int memory_post_test1(unsigned long start,
			      unsigned long size,
			      unsigned long val)
{
	unsigned long i;
	ulong *mem = (ulong *) start;
	ulong readback;
	int ret = 0;

	for (i = 0; i < size / sizeof (ulong); i++) {
		mem[i] = val;
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	for (i = 0; i < size / sizeof (ulong) && !ret; i++) {
		readback = mem[i];
		if (readback != val) {
			post_log("Memory error at %08x, "
				  "wrote %08x, read %08x !\n",
					  mem + i, val, readback);

			ret = -1;
			break;
		}
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	return ret;
}

static int memory_post_test2(unsigned long start, unsigned long size)
{
	unsigned long i;
	ulong *mem = (ulong *) start;
	ulong readback;
	int ret = 0;

	for (i = 0; i < size / sizeof (ulong); i++) {
		mem[i] = 1 << (i % 32);
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	for (i = 0; i < size / sizeof (ulong) && !ret; i++) {
		readback = mem[i];
		if (readback != (1 << (i % 32))) {
			post_log("Memory error at %08x, "
				  "wrote %08x, read %08x !\n",
					  mem + i, 1 << (i % 32), readback);

			ret = -1;
			break;
		}
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	return ret;
}

static int memory_post_test3(unsigned long start, unsigned long size)
{
	unsigned long i;
	ulong *mem = (ulong *) start;
	ulong readback;
	int ret = 0;

	for (i = 0; i < size / sizeof (ulong); i++) {
		mem[i] = i;
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	for (i = 0; i < size / sizeof (ulong) && !ret; i++) {
		readback = mem[i];
		if (readback != i) {
			post_log("Memory error at %08x, "
				  "wrote %08x, read %08x !\n",
					  mem + i, i, readback);

			ret = -1;
			break;
		}
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	return ret;
}

static int memory_post_test4(unsigned long start, unsigned long size)
{
	unsigned long i;
	ulong *mem = (ulong *) start;
	ulong readback;
	int ret = 0;

	for (i = 0; i < size / sizeof (ulong); i++) {
		mem[i] = ~i;
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	for (i = 0; i < size / sizeof (ulong) && !ret; i++) {
		readback = mem[i];
		if (readback != ~i) {
			post_log("Memory error at %08x, "
				  "wrote %08x, read %08x !\n",
					  mem + i, ~i, readback);

			ret = -1;
			break;
		}
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	return ret;
}

static int memory_post_test_lines(unsigned long start, unsigned long size)
{
	int ret = 0;

	ret = memory_post_dataline((unsigned long long *)start);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_addrline((ulong *)start, (ulong *)start,
				size);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_addrline((ulong *)(start+size-8),
				(ulong *)start, size);
	WATCHDOG_RESET();

	return ret;
}

static int memory_post_test_patterns(unsigned long start, unsigned long size)
{
	int ret = 0;

	ret = memory_post_test1(start, size, 0x00000000);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_test1(start, size, 0xffffffff);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_test1(start, size, 0x55555555);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_test1(start, size, 0xaaaaaaaa);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_test2(start, size);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_test3(start, size);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_test4(start, size);
	WATCHDOG_RESET();

	return ret;
}

static int memory_post_test_regions(unsigned long start, unsigned long size)
{
	unsigned long i;
	int ret = 0;

	for (i = 0; i < (size >> 20) && (!ret); i++) {
		if (!ret)
			ret = memory_post_test_patterns(start + (i << 20),
				0x800);
		if (!ret)
			ret = memory_post_test_patterns(start + (i << 20) +
				0xff800, 0x800);
	}

	return ret;
}

static int memory_post_tests(unsigned long start, unsigned long size)
{
	int ret = 0;

	ret = memory_post_test_lines(start, size);
	if (!ret)
		ret = memory_post_test_patterns(start, size);

	return ret;
}

/*
 * !! this is only valid, if you have contiguous memory banks !!
 */
__attribute__((weak))
int arch_memory_test_prepare(u32 *vstart, u32 *size, phys_addr_t *phys_offset)
{
	bd_t *bd = gd->bd;

	*vstart = CONFIG_SYS_SDRAM_BASE;
	*size = (gd->ram_size >= 256 << 20 ?
			256 << 20 : gd->ram_size) - (1 << 20);

	/* Limit area to be tested with the board info struct */
	if ((*vstart) + (*size) > (ulong)bd)
		*size = (ulong)bd - *vstart;

	return 0;
}

__attribute__((weak))
int arch_memory_test_advance(u32 *vstart, u32 *size, phys_addr_t *phys_offset)
{
	return 1;
}

__attribute__((weak))
int arch_memory_test_cleanup(u32 *vstart, u32 *size, phys_addr_t *phys_offset)
{
	return 0;
}

__attribute__((weak))
void arch_memory_failure_handle(void)
{
	return;
}

int memory_regions_post_test(int flags)
{
	int ret = 0;
	phys_addr_t phys_offset = 0;
	u32 memsize, vstart;

	arch_memory_test_prepare(&vstart, &memsize, &phys_offset);

	ret = memory_post_test_lines(vstart, memsize);
	if (!ret)
		ret = memory_post_test_regions(vstart, memsize);

	return ret;
}

int memory_post_test(int flags)
{
	int ret = 0;
	phys_addr_t phys_offset = 0;
	u32 memsize, vstart;

	arch_memory_test_prepare(&vstart, &memsize, &phys_offset);

	do {
		if (flags & POST_SLOWTEST) {
			ret = memory_post_tests(vstart, memsize);
		} else {			/* POST_NORMAL */
			ret = memory_post_test_regions(vstart, memsize);
		}
	} while (!ret &&
		!arch_memory_test_advance(&vstart, &memsize, &phys_offset));

	arch_memory_test_cleanup(&vstart, &memsize, &phys_offset);
	if (ret)
		arch_memory_failure_handle();

	return ret;
}

#endif /* CONFIG_POST&(CONFIG_SYS_POST_MEMORY|CONFIG_SYS_POST_MEM_REGIONS) */
