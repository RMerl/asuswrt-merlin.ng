// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY UCLASS_RAM

#include <common.h>
#include <console.h>
#include <init.h>
#include <log.h>
#include <rand.h>
#include <watchdog.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/log2.h>
#include "stm32mp1_tests.h"

#define ADDR_INVALID	0xFFFFFFFF

#define PATTERN_DEFAULT	"-"

DECLARE_GLOBAL_DATA_PTR;

static int get_bufsize(char *string, int argc, char *argv[], int arg_nb,
		       size_t *bufsize, size_t default_size, size_t min_size)
{
	unsigned long value;

	if (argc > arg_nb) {
		if (strict_strtoul(argv[arg_nb], 0, &value) < 0) {
			sprintf(string, "invalid %d parameter %s",
				arg_nb, argv[arg_nb]);
			return -1;
		}
		if (value > STM32_DDR_SIZE || value < min_size) {
			sprintf(string, "invalid size %s (min=%d)",
				argv[arg_nb], min_size);
			return -1;
		}
		if (value & (min_size - 1)) {
			sprintf(string, "unaligned size %s (min=%d)",
				argv[arg_nb], min_size);
			return -1;
		}
		*bufsize = value;
	} else {
		if (default_size != STM32_DDR_SIZE)
			*bufsize = default_size;
		else
			*bufsize = get_ram_size((long *)STM32_DDR_BASE,
						STM32_DDR_SIZE);
	}
	return 0;
}

static int get_nb_loop(char *string, int argc, char *argv[], int arg_nb,
		       u32 *nb_loop, u32 default_nb_loop)
{
	unsigned long value;

	if (argc > arg_nb) {
		if (strict_strtoul(argv[arg_nb], 0, &value) < 0) {
			sprintf(string, "invalid %d parameter %s",
				arg_nb, argv[arg_nb]);
			return -1;
		}
		if (value == 0)
			printf("WARNING: infinite loop requested\n");
		*nb_loop = value;
	} else {
		*nb_loop = default_nb_loop;
	}

	return 0;
}

static int get_addr(char *string, int argc, char *argv[], int arg_nb,
		    u32 *addr)
{
	unsigned long value;

	if (argc > arg_nb) {
		if (strict_strtoul(argv[arg_nb], 16, &value) < 0) {
			sprintf(string, "invalid %d parameter %s",
				arg_nb, argv[arg_nb]);
			return -1;
		}
		if (value < STM32_DDR_BASE) {
			sprintf(string, "too low address %s", argv[arg_nb]);
			return -1;
		}
		if (value & 0x3 && value != ADDR_INVALID) {
			sprintf(string, "unaligned address %s",
				argv[arg_nb]);
			return -1;
		}
		*addr = value;
	} else {
		*addr = STM32_DDR_BASE;
	}

	return 0;
}

static int get_pattern(char *string, int argc, char *argv[], int arg_nb,
		       u32 *pattern, u32 default_pattern)
{
	unsigned long value;

	if (argc > arg_nb) {
		if (!strcmp(argv[arg_nb], PATTERN_DEFAULT)) {
			*pattern = default_pattern;
			return 0;
		}
		if (strict_strtoul(argv[arg_nb], 16, &value) < 0) {
			sprintf(string, "invalid %d parameter %s",
				arg_nb, argv[arg_nb]);
			return -1;
		}
		*pattern = value;
	} else {
		*pattern = default_pattern;
	}

	return 0;
}

static u32 check_addr(u32 addr, u32 value)
{
	u32 data = readl(addr);

	if (value !=  data) {
		printf("0x%08x: 0x%08x <=> 0x%08x", addr, data, value);
		data = readl(addr);
		printf("(2nd read: 0x%08x)", data);
		if (value == data)
			printf("- read error");
		else
			printf("- write error");
		printf("\n");
		return -1;
	}
	return 0;
}

static int progress(u32 offset)
{
	if (!(offset & 0xFFFFFF)) {
		putc('.');
		if (ctrlc()) {
			printf("\ntest interrupted!\n");
			return 1;
		}
	}
	return 0;
}

static int test_loop_end(u32 *loop, u32 nb_loop, u32 progress)
{
	(*loop)++;
	if (nb_loop && *loop >= nb_loop)
		return 1;
	if ((*loop) % progress)
		return 0;
	/* allow to interrupt the test only for progress step */
	if (ctrlc()) {
		printf("test interrupted!\n");
		return 1;
	}
	printf("loop #%d\n", *loop);
	schedule();

	return 0;
}

/**********************************************************************
 *
 * Function:    memTestDataBus()
 *
 * Description: Test the data bus wiring in a memory region by
 *              performing a walking 1's test at a fixed address
 *              within that region.  The address is selected
 *              by the caller.
 *
 * Notes:
 *
 * Returns:     0 if the test succeeds.
 *              A non-zero result is the first pattern that failed.
 *
 **********************************************************************/
static u32 databus(u32 *address)
{
	u32 pattern;
	u32 read_value;

	/* Perform a walking 1's test at the given address. */
	for (pattern = 1; pattern != 0; pattern <<= 1) {
		/* Write the test pattern. */
		writel(pattern, address);

		/* Read it back (immediately is okay for this test). */
		read_value = readl(address);
		log_debug("%x: %x <=> %x\n",
			  (u32)address, read_value, pattern);

		if (read_value != pattern)
			return pattern;
	}

	return 0;
}

/**********************************************************************
 *
 * Function:    memTestAddressBus()
 *
 * Description: Test the address bus wiring in a memory region by
 *              performing a walking 1's test on the relevant bits
 *              of the address and checking for aliasing. This test
 *              will find single-bit address failures such as stuck
 *              -high, stuck-low, and shorted pins.  The base address
 *              and size of the region are selected by the caller.
 *
 * Notes:       For best results, the selected base address should
 *              have enough LSB 0's to guarantee single address bit
 *              changes.  For example, to test a 64-Kbyte region,
 *              select a base address on a 64-Kbyte boundary.  Also,
 *              select the region size as a power-of-two--if at all
 *              possible.
 *
 * Returns:     NULL if the test succeeds.
 *              A non-zero result is the first address at which an
 *              aliasing problem was uncovered.  By examining the
 *              contents of memory, it may be possible to gather
 *              additional information about the problem.
 *
 **********************************************************************/
static u32 *addressbus(u32 *address, u32 nb_bytes)
{
	u32 mask = (nb_bytes / sizeof(u32) - 1);
	u32 offset;
	u32 test_offset;
	u32 read_value;

	u32 pattern     = 0xAAAAAAAA;
	u32 antipattern = 0x55555555;

	/* Write the default pattern at each of the power-of-two offsets. */
	for (offset = 1; (offset & mask) != 0; offset <<= 1)
		writel(pattern, &address[offset]);

	/* Check for address bits stuck high. */
	test_offset = 0;
	writel(antipattern, &address[test_offset]);

	for (offset = 1; (offset & mask) != 0; offset <<= 1) {
		read_value = readl(&address[offset]);
		log_debug("%x: %x <=> %x\n",
			  (u32)&address[offset], read_value, pattern);
		if (read_value != pattern)
			return &address[offset];
	}

	writel(pattern, &address[test_offset]);

	/* Check for address bits stuck low or shorted. */
	for (test_offset = 1; (test_offset & mask) != 0; test_offset <<= 1) {
		writel(antipattern, &address[test_offset]);
		if (readl(&address[0]) != pattern)
			return &address[test_offset];

		for (offset = 1; (offset & mask) != 0; offset <<= 1) {
			if (readl(&address[offset]) != pattern &&
			    offset != test_offset)
				return &address[test_offset];
		}
		writel(pattern, &address[test_offset]);
	}

	return NULL;
}

/**********************************************************************
 *
 * Function:    memTestDevice()
 *
 * Description: Test the integrity of a physical memory device by
 *              performing an increment/decrement test over the
 *              entire region.  In the process every storage bit
 *              in the device is tested as a zero and a one.  The
 *              base address and the size of the region are
 *              selected by the caller.
 *
 * Notes:
 *
 * Returns:     NULL if the test succeeds.
 *
 *              A non-zero result is the first address at which an
 *              incorrect value was read back.  By examining the
 *              contents of memory, it may be possible to gather
 *              additional information about the problem.
 *
 **********************************************************************/
static u32 *memdevice(u32 *address, u32 nb_bytes)
{
	u32 offset;
	u32 nb_words = nb_bytes / sizeof(u32);

	u32 pattern;
	u32 antipattern;

	puts("Fill with pattern");
	/* Fill memory with a known pattern. */
	for (pattern = 1, offset = 0; offset < nb_words; pattern++, offset++) {
		writel(pattern, &address[offset]);
		if (progress(offset))
			return NULL;
	}

	puts("\nCheck and invert pattern");
	/* Check each location and invert it for the second pass. */
	for (pattern = 1, offset = 0; offset < nb_words; pattern++, offset++) {
		if (readl(&address[offset]) != pattern)
			return &address[offset];

		antipattern = ~pattern;
		writel(antipattern, &address[offset]);
		if (progress(offset))
			return NULL;
	}

	puts("\nCheck inverted pattern");
	/* Check each location for the inverted pattern and zero it. */
	for (pattern = 1, offset = 0; offset < nb_words; pattern++, offset++) {
		antipattern = ~pattern;
		if (readl(&address[offset]) != antipattern)
			return &address[offset];
		if (progress(offset))
			return NULL;
	}
	printf("\n");

	return NULL;
}

static enum test_result databuswalk0(struct stm32mp1_ddrctl *ctl,
				     struct stm32mp1_ddrphy *phy,
				     char *string, int argc, char *argv[])
{
	int i;
	u32 loop = 0, nb_loop;
	u32 addr;
	u32 error = 0;
	u32 data;

	if (get_nb_loop(string, argc, argv, 0, &nb_loop, 100))
		return TEST_ERROR;
	if (get_addr(string, argc, argv, 1, &addr))
		return TEST_ERROR;

	printf("running %d loops at 0x%x\n", nb_loop, addr);
	while (!error) {
		for (i = 0; i < 32; i++)
			writel(~(1 << i), addr + 4 * i);
		for (i = 0; i < 32; i++) {
			data = readl(addr + 4 * i);
			if (~(1 << i) !=  data) {
				error |= 1 << i;
				log_debug("%x: error %x expected %x => error:%x\n",
					  addr + 4 * i, data, ~(1 << i), error);
			}
		}
		if (test_loop_end(&loop, nb_loop, 1000))
			break;
		for (i = 0; i < 32; i++)
			writel(0, addr + 4 * i);
	}
	if (error) {
		sprintf(string, "loop %d: error for bits 0x%x",
			loop, error);
		return TEST_FAILED;
	}
	sprintf(string, "no error for %d loops", loop);
	return TEST_PASSED;
}

static enum test_result databuswalk1(struct stm32mp1_ddrctl *ctl,
				     struct stm32mp1_ddrphy *phy,
				     char *string, int argc, char *argv[])
{
	int i;
	u32 loop = 0, nb_loop;
	u32 addr;
	u32 error = 0;
	u32 data;

	if (get_nb_loop(string, argc, argv, 0, &nb_loop, 100))
		return TEST_ERROR;
	if (get_addr(string, argc, argv, 1, &addr))
		return TEST_ERROR;
	printf("running %d loops at 0x%x\n", nb_loop, addr);
	while (!error) {
		for (i = 0; i < 32; i++)
			writel(1 << i, addr + 4 * i);
		for (i = 0; i < 32; i++) {
			data = readl(addr + 4 * i);
			if ((1 << i) !=  data) {
				error |= 1 << i;
				log_debug("%x: error %x expected %x => error:%x\n",
					  addr + 4 * i, data, (1 << i), error);
			}
		}
		if (test_loop_end(&loop, nb_loop, 1000))
			break;
		for (i = 0; i < 32; i++)
			writel(0, addr + 4 * i);
	}
	if (error) {
		sprintf(string, "loop %d: error for bits 0x%x",
			loop, error);
		return TEST_FAILED;
	}
	sprintf(string, "no error for %d loops", loop);
	return TEST_PASSED;
}

static enum test_result test_databus(struct stm32mp1_ddrctl *ctl,
				     struct stm32mp1_ddrphy *phy,
				     char *string, int argc, char *argv[])
{
	u32 addr;
	u32 error;

	if (get_addr(string, argc, argv, 0, &addr))
		return TEST_ERROR;
	error = databus((u32 *)addr);
	if (error) {
		sprintf(string, "0x%x: error for bits 0x%x",
			addr, error);
		return TEST_FAILED;
	}
	sprintf(string, "address 0x%x", addr);
	return TEST_PASSED;
}

static enum test_result test_addressbus(struct stm32mp1_ddrctl *ctl,
					struct stm32mp1_ddrphy *phy,
					char *string, int argc, char *argv[])
{
	u32 addr;
	u32 bufsize;
	u32 error;

	if (get_bufsize(string, argc, argv, 0, &bufsize, STM32_DDR_SIZE, 4))
		return TEST_ERROR;
	if (!is_power_of_2(bufsize)) {
		sprintf(string, "size 0x%x is not a power of 2",
			(u32)bufsize);
		return TEST_ERROR;
	}
	if (get_addr(string, argc, argv, 1, &addr))
		return TEST_ERROR;

	printf("running at 0x%08x length 0x%x\n", addr, bufsize);
	error = (u32)addressbus((u32 *)addr, bufsize);
	if (error) {
		sprintf(string, "0x%x: error for address 0x%x",
			addr, error);
		return TEST_FAILED;
	}
	sprintf(string, "address 0x%x, size 0x%x",
		addr, bufsize);
	return TEST_PASSED;
}

static enum test_result test_memdevice(struct stm32mp1_ddrctl *ctl,
				       struct stm32mp1_ddrphy *phy,
				       char *string, int argc, char *argv[])
{
	u32 addr;
	size_t bufsize;
	u32 error;

	if (get_bufsize(string, argc, argv, 0, &bufsize, 4 * 1024, 4))
		return TEST_ERROR;
	if (get_addr(string, argc, argv, 1, &addr))
		return TEST_ERROR;
	error = (u32)memdevice((u32 *)addr, (unsigned long)bufsize);
	if (error) {
		sprintf(string, "0x%x: error for address 0x%x",
			addr, error);
		return TEST_FAILED;
	}
	sprintf(string, "address 0x%x, size 0x%x",
		addr, bufsize);
	return TEST_PASSED;
}

/**********************************************************************
 *
 * Function:    sso
 *
 * Description: Test the Simultaneous Switching Output.
 *              Verifies succes sive reads and writes to the same memory word,
 *              holding one bit constant while toggling all other data bits
 *              simultaneously
 *              => stress the data bus over an address range
 *
 *              The CPU writes to each address in the given range.
 *              For each bit, first the CPU holds the bit at 1 while
 *              toggling the other bits, and then the CPU holds the bit at 0
 *              while toggling the other bits.
 *              After each write, the CPU reads the address that was written
 *              to verify that it contains the correct data
 *
 **********************************************************************/
static enum test_result test_sso(struct stm32mp1_ddrctl *ctl,
				 struct stm32mp1_ddrphy *phy,
				 char *string, int argc, char *argv[])
{
	int i, j;
	u32 addr, bufsize, remaining, offset;
	u32 error = 0;
	u32 data;

	if (get_bufsize(string, argc, argv, 0, &bufsize, 4 * 1024, 4))
		return TEST_ERROR;
	if (get_addr(string, argc, argv, 1, &addr))
		return TEST_ERROR;

	printf("running sso at 0x%x length 0x%x", addr, bufsize);
	offset = addr;
	remaining = bufsize;
	while (remaining) {
		for (i = 0; i < 32; i++) {
			/* write pattern. */
			for (j = 0; j < 6; j++) {
				switch (j) {
				case 0:
				case 2:
					data = 1 << i;
					break;
				case 3:
				case 5:
					data = ~(1 << i);
					break;
				case 1:
					data = ~0x0;
					break;
				case 4:
					data = 0x0;
					break;
				}

				writel(data, offset);
				error = check_addr(offset, data);
				if (error)
					goto end;
			}
		}
		offset += 4;
		remaining -= 4;
		if (progress(offset << 7))
			goto end;
	}
	puts("\n");

end:
	if (error) {
		sprintf(string, "error for pattern 0x%x @0x%x",
			data, offset);
		return TEST_FAILED;
	}
	sprintf(string, "no error for sso at 0x%x length 0x%x", addr, bufsize);
	return TEST_PASSED;
}

/**********************************************************************
 *
 * Function:    Random
 *
 * Description: Verifies r/w with pseudo-ramdom value on one region
 *              + write the region (individual access)
 *              + memcopy to the 2nd region (try to use burst)
 *              + verify the 2 regions
 *
 **********************************************************************/
static enum test_result test_random(struct stm32mp1_ddrctl *ctl,
				    struct stm32mp1_ddrphy *phy,
				    char *string, int argc, char *argv[])
{
	u32 addr, offset, value = 0;
	size_t bufsize;
	u32 loop = 0, nb_loop;
	u32 error = 0;
	unsigned int seed;

	if (get_bufsize(string, argc, argv, 0, &bufsize, 4 * 1024, 8))
		return TEST_ERROR;
	if (get_nb_loop(string, argc, argv, 1, &nb_loop, 1))
		return TEST_ERROR;
	if (get_addr(string, argc, argv, 2, &addr))
		return TEST_ERROR;

	bufsize /= 2;
	printf("running %d loops copy from 0x%x to 0x%x (buffer size=0x%x)\n",
	       nb_loop, addr, addr + bufsize, bufsize);
	while (!error) {
		seed = rand();
		for (offset = 0; offset < bufsize; offset += 4)
			writel(rand(), addr + offset);

		memcpy((void *)addr + bufsize, (void *)addr, bufsize);

		srand(seed);
		for (offset = 0; offset < 2 * bufsize; offset += 4) {
			if (offset == bufsize)
				srand(seed);
			value = rand();
			error = check_addr(addr + offset, value);
			if (error)
				break;
			if (progress(offset))
				return TEST_FAILED;
		}
		if (test_loop_end(&loop, nb_loop, 100))
			break;
	}
	putc('\n');

	if (error) {
		sprintf(string,
			"loop %d: error for address 0x%x: 0x%x expected 0x%x",
			loop, offset, readl(offset), value);
		return TEST_FAILED;
	}
	sprintf(string, "no error for %d loops, size 0x%x",
		loop, bufsize);
	return TEST_PASSED;
}

/**********************************************************************
 *
 * Function:    noise
 *
 * Description: Verifies r/w while forcing switching of all data bus lines.
 *              optimised 4 iteration write/read/write/read cycles...
 *              for pattern and inversed pattern
 *
 **********************************************************************/
void do_noise(u32 addr, u32 pattern, u32 *result)
{
	__asm__("push {R0-R11}");
	__asm__("mov r0, %0" : : "r" (addr));
	__asm__("mov r1, %0" : : "r" (pattern));
	__asm__("mov r11, %0" : : "r" (result));

	__asm__("mvn r2, r1");

	__asm__("str r1, [r0]");
	__asm__("ldr r3, [r0]");
	__asm__("str r2, [r0]");
	__asm__("ldr r4, [r0]");

	__asm__("str r1, [r0]");
	__asm__("ldr r5, [r0]");
	__asm__("str r2, [r0]");
	__asm__("ldr r6, [r0]");

	__asm__("str r1, [r0]");
	__asm__("ldr r7, [r0]");
	__asm__("str r2, [r0]");
	__asm__("ldr r8, [r0]");

	__asm__("str r1, [r0]");
	__asm__("ldr r9, [r0]");
	__asm__("str r2, [r0]");
	__asm__("ldr r10, [r0]");

	__asm__("stmia R11!, {R3-R10}");

	__asm__("pop {R0-R11}");
}

static enum test_result test_noise(struct stm32mp1_ddrctl *ctl,
				   struct stm32mp1_ddrphy *phy,
				   char *string, int argc, char *argv[])
{
	u32 addr, pattern;
	u32 result[8];
	int i;
	enum test_result res = TEST_PASSED;

	if (get_pattern(string, argc, argv, 0, &pattern, 0xFFFFFFFF))
		return TEST_ERROR;
	if (get_addr(string, argc, argv, 1, &addr))
		return TEST_ERROR;

	printf("running noise for 0x%x at 0x%x\n", pattern, addr);

	do_noise(addr, pattern, result);

	for (i = 0; i < 0x8;) {
		if (check_addr((u32)&result[i++], pattern))
			res = TEST_FAILED;
		if (check_addr((u32)&result[i++], ~pattern))
			res = TEST_FAILED;
	}

	return res;
}

/**********************************************************************
 *
 * Function:    noise_burst
 *
 * Description: Verifies r/w while forcing switching of all data bus lines.
 *              optimised write loop witrh store multiple to use burst
 *              for pattern and inversed pattern
 *
 **********************************************************************/
void do_noise_burst(u32 addr, u32 pattern, size_t bufsize)
{
	__asm__("push {R0-R9}");
	__asm__("mov r0, %0" : : "r" (addr));
	__asm__("mov r1, %0" : : "r" (pattern));
	__asm__("mov r9, %0" : : "r" (bufsize));

	__asm__("mvn r2, r1");
	__asm__("mov r3, r1");
	__asm__("mov r4, r2");
	__asm__("mov r5, r1");
	__asm__("mov r6, r2");
	__asm__("mov r7, r1");
	__asm__("mov r8, r2");

	__asm__("loop1:");
	__asm__("stmia R0!, {R1-R8}");
	__asm__("stmia R0!, {R1-R8}");
	__asm__("stmia R0!, {R1-R8}");
	__asm__("stmia R0!, {R1-R8}");
	__asm__("subs r9, r9, #128");
	__asm__("bge loop1");
	__asm__("pop {R0-R9}");
}

/* chunk size enough to allow interruption with Ctrl-C*/
#define CHUNK_SIZE	0x8000000
static enum test_result test_noise_burst(struct stm32mp1_ddrctl *ctl,
					 struct stm32mp1_ddrphy *phy,
					 char *string, int argc, char *argv[])
{
	u32 addr, offset, pattern;
	size_t bufsize, remaining, size;
	int i;
	enum test_result res = TEST_PASSED;

	if (get_bufsize(string, argc, argv, 0, &bufsize, 4 * 1024, 128))
		return TEST_ERROR;
	if (get_pattern(string, argc, argv, 1, &pattern, 0xFFFFFFFF))
		return TEST_ERROR;
	if (get_addr(string, argc, argv, 2, &addr))
		return TEST_ERROR;

	printf("running noise burst for 0x%x at 0x%x + 0x%x",
	       pattern, addr, bufsize);

	offset = addr;
	remaining = bufsize;
	size = CHUNK_SIZE;
	while (remaining) {
		if (remaining < size)
			size = remaining;
		do_noise_burst(offset, pattern, size);
		remaining -= size;
		offset += size;
		if (progress(offset)) {
			res = TEST_FAILED;
			goto end;
		}
	}
	puts("\ncheck buffer");
	for (i = 0; i < bufsize;) {
		if (check_addr(addr + i, pattern))
			res = TEST_FAILED;
		i += 4;
		if (check_addr(addr + i, ~pattern))
			res = TEST_FAILED;
		i += 4;
		if (progress(i)) {
			res = TEST_FAILED;
			goto end;
		}
	}
end:
	puts("\n");
	return res;
}

/**********************************************************************
 *
 * Function:    pattern test
 *
 * Description: optimized loop for read/write pattern (array of 8 u32)
 *
 **********************************************************************/
#define PATTERN_SIZE	8
static enum test_result test_loop(const u32 *pattern, u32 *address,
				  const u32 bufsize)
{
	int i;
	int j;
	enum test_result res = TEST_PASSED;
	u32 offset, testsize, remaining;

	offset = (u32)address;
	remaining = bufsize;
	while (remaining) {
		testsize = bufsize > 0x1000000 ? 0x1000000 : bufsize;

		__asm__("push {R0-R10}");
		__asm__("mov r0, %0" : : "r" (pattern));
		__asm__("mov r1, %0" : : "r" (offset));
		__asm__("mov r2, %0" : : "r" (testsize));
		__asm__("ldmia r0!, {R3-R10}");

		__asm__("loop2:");
		__asm__("stmia r1!, {R3-R10}");
		__asm__("stmia r1!, {R3-R10}");
		__asm__("stmia r1!, {R3-R10}");
		__asm__("stmia r1!, {R3-R10}");
		__asm__("subs r2, r2, #128");
		__asm__("bge loop2");
		__asm__("pop {R0-R10}");

		offset += testsize;
		remaining -= testsize;
		if (progress((u32)offset)) {
			res = TEST_FAILED;
			goto end;
		}
	}

	puts("\ncheck buffer");
	for (i = 0; i < bufsize; i += PATTERN_SIZE * 4) {
		for (j = 0; j < PATTERN_SIZE; j++, address++)
			if (check_addr((u32)address, pattern[j])) {
				res = TEST_FAILED;
				goto end;
			}
		if (progress(i)) {
			res = TEST_FAILED;
			goto end;
		}
	}

end:
	puts("\n");
	return res;
}

const u32 pattern_div1_x16[PATTERN_SIZE] = {
	0x0000FFFF, 0x0000FFFF, 0x0000FFFF, 0x0000FFFF,
	0x0000FFFF, 0x0000FFFF, 0x0000FFFF, 0x0000FFFF
};

const u32 pattern_div2_x16[PATTERN_SIZE] = {
	0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000,
	0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000
};

const u32 pattern_div4_x16[PATTERN_SIZE] = {
	0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000,
	0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000
};

const u32 pattern_div4_x32[PATTERN_SIZE] = {
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0x00000000, 0x00000000, 0x00000000, 0x00000000
};

const u32 pattern_mostly_zero_x16[PATTERN_SIZE] = {
	0x00000000, 0x00000000, 0x00000000, 0x0000FFFF,
	0x00000000, 0x00000000, 0x00000000, 0x00000000
};

const u32 pattern_mostly_zero_x32[PATTERN_SIZE] = {
	0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF,
	0x00000000, 0x00000000, 0x00000000, 0x00000000
};

const u32 pattern_mostly_one_x16[PATTERN_SIZE] = {
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x0000FFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
};

const u32 pattern_mostly_one_x32[PATTERN_SIZE] = {
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
};

#define NB_PATTERN	5
static enum test_result test_freq_pattern(struct stm32mp1_ddrctl *ctl,
					  struct stm32mp1_ddrphy *phy,
					  char *string, int argc, char *argv[])
{
	const u32 * const patterns_x16[NB_PATTERN] = {
		pattern_div1_x16,
		pattern_div2_x16,
		pattern_div4_x16,
		pattern_mostly_zero_x16,
		pattern_mostly_one_x16,
	};
	const u32 * const patterns_x32[NB_PATTERN] = {
		pattern_div2_x16,
		pattern_div4_x16,
		pattern_div4_x32,
		pattern_mostly_zero_x32,
		pattern_mostly_one_x32
	};
	const char *patterns_comments[NB_PATTERN] = {
		"switching at frequency F/1",
		"switching at frequency F/2",
		"switching at frequency F/4",
		"mostly zero",
		"mostly one"
	};

	enum test_result res = TEST_PASSED, pattern_res;
	int i, bus_width;
	const u32 **patterns;
	u32 bufsize, addr;

	if (get_bufsize(string, argc, argv, 0, &bufsize, 4 * 1024, 128))
		return TEST_ERROR;
	if (get_addr(string, argc, argv, 1, &addr))
		return TEST_ERROR;

	switch (readl(&ctl->mstr) & DDRCTRL_MSTR_DATA_BUS_WIDTH_MASK) {
	case DDRCTRL_MSTR_DATA_BUS_WIDTH_HALF:
	case DDRCTRL_MSTR_DATA_BUS_WIDTH_QUARTER:
		bus_width = 16;
		break;
	default:
		bus_width = 32;
		break;
	}

	printf("running test pattern at 0x%08x length 0x%x width = %d\n",
	       addr, bufsize, bus_width);

	patterns =
		(const u32 **)(bus_width == 16 ? patterns_x16 : patterns_x32);

	for (i = 0; i < NB_PATTERN; i++) {
		printf("test data pattern %s:", patterns_comments[i]);
		pattern_res = test_loop(patterns[i], (u32 *)addr, bufsize);
		if (pattern_res != TEST_PASSED)	{
			printf("Failed\n");
			return pattern_res;
		}
		printf("Passed\n");
	}

	return res;
}

/**********************************************************************
 *
 * Function:    pattern test with size
 *
 * Description: loop for write pattern
 *
 **********************************************************************/

static enum test_result test_loop_size(const u32 *pattern, u32 size,
				       u32 *address,
				       const u32 bufsize)
{
	int i, j;
	enum test_result res = TEST_PASSED;
	u32 *p = address;

	for (i = 0; i < bufsize; i += size * 4) {
		for (j = 0; j < size ; j++, p++)
			*p = pattern[j];
		if (progress(i)) {
			res = TEST_FAILED;
			goto end;
		}
	}

	puts("\ncheck buffer");
	p = address;
	for (i = 0; i < bufsize; i += size * 4) {
		for (j = 0; j < size; j++, p++)
			if (check_addr((u32)p, pattern[j])) {
				res = TEST_FAILED;
				goto end;
			}
		if (progress(i)) {
			res = TEST_FAILED;
			goto end;
		}
	}

end:
	puts("\n");
	return res;
}

static enum test_result test_checkboard(struct stm32mp1_ddrctl *ctl,
					struct stm32mp1_ddrphy *phy,
					char *string, int argc, char *argv[])
{
	enum test_result res = TEST_PASSED;
	u32 bufsize, nb_loop, loop = 0, addr;
	int i;

	u32 checkboard[2] = {0x55555555, 0xAAAAAAAA};

	if (get_bufsize(string, argc, argv, 0, &bufsize, 4 * 1024, 8))
		return TEST_ERROR;
	if (get_nb_loop(string, argc, argv, 1, &nb_loop, 1))
		return TEST_ERROR;
	if (get_addr(string, argc, argv, 2, &addr))
		return TEST_ERROR;

	printf("running %d loops at 0x%08x length 0x%x\n",
	       nb_loop, addr, bufsize);
	while (1) {
		for (i = 0; i < 2; i++) {
			res = test_loop_size(checkboard, 2, (u32 *)addr,
					     bufsize);
			if (res)
				return res;
			checkboard[0] = ~checkboard[0];
			checkboard[1] = ~checkboard[1];
		}
		if (test_loop_end(&loop, nb_loop, 1))
			break;
	}
	sprintf(string, "no error for %d loops at 0x%08x length 0x%x",
		loop, addr, bufsize);

	return res;
}

static enum test_result test_blockseq(struct stm32mp1_ddrctl *ctl,
				      struct stm32mp1_ddrphy *phy,
				      char *string, int argc, char *argv[])
{
	enum test_result res = TEST_PASSED;
	u32 bufsize, nb_loop, loop = 0, addr, value;
	int i;

	if (get_bufsize(string, argc, argv, 0, &bufsize, 4 * 1024, 4))
		return TEST_ERROR;
	if (get_nb_loop(string, argc, argv, 1, &nb_loop, 1))
		return TEST_ERROR;
	if (get_addr(string, argc, argv, 2, &addr))
		return TEST_ERROR;

	printf("running %d loops at 0x%08x length 0x%x\n",
	       nb_loop, addr, bufsize);
	while (1) {
		for (i = 0; i < 256; i++) {
			value = i | i << 8 | i << 16 | i << 24;
			printf("pattern = %08x", value);
			res = test_loop_size(&value, 1, (u32 *)addr, bufsize);
			if (res != TEST_PASSED)
				return res;
		}
		if (test_loop_end(&loop, nb_loop, 1))
			break;
	}
	sprintf(string, "no error for %d loops at 0x%08x length 0x%x",
		loop, addr, bufsize);

	return res;
}

static enum test_result test_walkbit0(struct stm32mp1_ddrctl *ctl,
				      struct stm32mp1_ddrphy *phy,
				      char *string, int argc, char *argv[])
{
	enum test_result res = TEST_PASSED;
	u32 bufsize, nb_loop, loop = 0, addr, value;
	int i;

	if (get_bufsize(string, argc, argv, 0, &bufsize, 4 * 1024, 4))
		return TEST_ERROR;
	if (get_nb_loop(string, argc, argv, 1, &nb_loop, 1))
		return TEST_ERROR;
	if (get_addr(string, argc, argv, 2, &addr))
		return TEST_ERROR;

	printf("running %d loops at 0x%08x length 0x%x\n",
	       nb_loop, addr, bufsize);
	while (1) {
		for (i = 0; i < 64; i++) {
			if (i < 32)
				value = 1 << i;
			else
				value = 1 << (63 - i);

			printf("pattern = %08x", value);
			res = test_loop_size(&value, 1, (u32 *)addr, bufsize);
			if (res != TEST_PASSED)
				return res;
		}
		if (test_loop_end(&loop, nb_loop, 1))
			break;
	}
	sprintf(string, "no error for %d loops at 0x%08x length 0x%x",
		loop, addr, bufsize);

	return res;
}

static enum test_result test_walkbit1(struct stm32mp1_ddrctl *ctl,
				      struct stm32mp1_ddrphy *phy,
				      char *string, int argc, char *argv[])
{
	enum test_result res = TEST_PASSED;
	u32 bufsize, nb_loop, loop = 0, addr, value;
	int i;

	if (get_bufsize(string, argc, argv, 0, &bufsize, 4 * 1024, 4))
		return TEST_ERROR;
	if (get_nb_loop(string, argc, argv, 1, &nb_loop, 1))
		return TEST_ERROR;
	if (get_addr(string, argc, argv, 2, &addr))
		return TEST_ERROR;

	printf("running %d loops at 0x%08x length 0x%x\n",
	       nb_loop, addr, bufsize);
	while (1) {
		for (i = 0; i < 64; i++) {
			if (i < 32)
				value = ~(1 << i);
			else
				value = ~(1 << (63 - i));

			printf("pattern = %08x", value);
			res = test_loop_size(&value, 1, (u32 *)addr, bufsize);
			if (res != TEST_PASSED)
				return res;
		}
		if (test_loop_end(&loop, nb_loop, 1))
			break;
	}
	sprintf(string, "no error for %d loops at 0x%08x length 0x%x",
		loop, addr, bufsize);

	return res;
}

/*
 * try to catch bad bits which are dependent on the current values of
 * surrounding bits in either the same word32
 */
static enum test_result test_bitspread(struct stm32mp1_ddrctl *ctl,
				       struct stm32mp1_ddrphy *phy,
				       char *string, int argc, char *argv[])
{
	enum test_result res = TEST_PASSED;
	u32 bufsize, nb_loop, loop = 0, addr, bitspread[4];
	int i, j;

	if (get_bufsize(string, argc, argv, 0, &bufsize, 4 * 1024, 32))
		return TEST_ERROR;
	if (get_nb_loop(string, argc, argv, 1, &nb_loop, 1))
		return TEST_ERROR;
	if (get_addr(string, argc, argv, 2, &addr))
		return TEST_ERROR;

	printf("running %d loops at 0x%08x length 0x%x\n",
	       nb_loop, addr, bufsize);
	while (1) {
		for (i = 1; i < 32; i++) {
			for (j = 0; j < i; j++) {
				if (i < 32)
					bitspread[0] = (1 << i) | (1 << j);
				else
					bitspread[0] = (1 << (63 - i)) |
						       (1 << (63 - j));
				bitspread[1] = bitspread[0];
				bitspread[2] = ~bitspread[0];
				bitspread[3] = ~bitspread[0];
				printf("pattern = %08x", bitspread[0]);

				res = test_loop_size(bitspread, 4, (u32 *)addr,
						     bufsize);
				if (res != TEST_PASSED)
					return res;
			}
		}
		if (test_loop_end(&loop, nb_loop, 1))
			break;
	}
	sprintf(string, "no error for %d loops at 0x%08x length 0x%x",
		loop, addr, bufsize);

	return res;
}

static enum test_result test_bitflip(struct stm32mp1_ddrctl *ctl,
				     struct stm32mp1_ddrphy *phy,
				     char *string, int argc, char *argv[])
{
	enum test_result res = TEST_PASSED;
	u32 bufsize, nb_loop, loop = 0, addr;
	int i;

	u32 bitflip[4];

	if (get_bufsize(string, argc, argv, 0, &bufsize, 4 * 1024, 32))
		return TEST_ERROR;
	if (get_nb_loop(string, argc, argv, 1, &nb_loop, 1))
		return TEST_ERROR;
	if (get_addr(string, argc, argv, 2, &addr))
		return TEST_ERROR;

	printf("running %d loops at 0x%08x length 0x%x\n",
	       nb_loop, addr, bufsize);
	while (1) {
		for (i = 0; i < 32; i++) {
			bitflip[0] = 1 << i;
			bitflip[1] = bitflip[0];
			bitflip[2] = ~bitflip[0];
			bitflip[3] = bitflip[2];
			printf("pattern = %08x", bitflip[0]);

			res = test_loop_size(bitflip, 4, (u32 *)addr, bufsize);
			if (res != TEST_PASSED)
				return res;
		}
		if (test_loop_end(&loop, nb_loop, 1))
			break;
	}
	sprintf(string, "no error for %d loops at 0x%08x length 0x%x",
		loop, addr, bufsize);

	return res;
}

/**********************************************************************
 *
 * Function: infinite read access to DDR
 *
 * Description: continuous read the same pattern at the same address
 *
 **********************************************************************/
static enum test_result test_read(struct stm32mp1_ddrctl *ctl,
				  struct stm32mp1_ddrphy *phy,
				  char *string, int argc, char *argv[])
{
	u32 *addr;
	u32 data;
	u32 loop = 0;
	int i, size = 1024 * 1024;
	bool random = false;

	if (get_addr(string, argc, argv, 0, (u32 *)&addr))
		return TEST_ERROR;

	if (get_pattern(string, argc, argv, 1, &data, 0xA5A5AA55))
		return TEST_ERROR;

	if ((u32)addr == ADDR_INVALID) {
		printf("running random\n");
		random = true;
	} else {
		printf("running at 0x%08x with pattern=0x%08x\n",
		       (u32)addr, data);
		writel(data, addr);
	}

	while (1) {
		for (i = 0; i < size; i++) {
			if (random)
				addr = (u32 *)(STM32_DDR_BASE +
				       (rand() & (STM32_DDR_SIZE - 1) & ~0x3));
			data = readl(addr);
		}
		if (test_loop_end(&loop, 0, 1))
			break;
	}
	if (random)
		sprintf(string, "%d loops random", loop);
	else
		sprintf(string, "%d loops at 0x%x: %x", loop, (u32)addr, data);

	return TEST_PASSED;
}

/**********************************************************************
 *
 * Function: infinite write access to DDR
 *
 * Description: continuous write the same pattern at the same address
 *
 **********************************************************************/
static enum test_result test_write(struct stm32mp1_ddrctl *ctl,
				   struct stm32mp1_ddrphy *phy,
				   char *string, int argc, char *argv[])
{
	u32 *addr;
	u32 data;
	u32 loop = 0;
	int i, size = 1024 * 1024;
	bool random = false;

	if (get_addr(string, argc, argv, 0, (u32 *)&addr))
		return TEST_ERROR;

	if (get_pattern(string, argc, argv, 1, &data, 0xA5A5AA55))
		return TEST_ERROR;

	if ((u32)addr == ADDR_INVALID) {
		printf("running random\n");
		random = true;
	} else {
		printf("running at 0x%08x with pattern 0x%08x\n",
		       (u32)addr, data);
	}

	while (1) {
		for (i = 0; i < size; i++) {
			if (random) {
				addr = (u32 *)(STM32_DDR_BASE +
				       (rand() & (STM32_DDR_SIZE - 1) & ~0x3));
				data = rand();
			}
			writel(data, addr);
		}
		if (test_loop_end(&loop, 0, 1))
			break;
	}
	if (random)
		sprintf(string, "%d loops random", loop);
	else
		sprintf(string, "%d loops at 0x%x: %x", loop, (u32)addr, data);

	return TEST_PASSED;
}

#define NB_TEST_INFINITE 2
static enum test_result test_all(struct stm32mp1_ddrctl *ctl,
				 struct stm32mp1_ddrphy *phy,
				 char *string, int argc, char *argv[])
{
	enum test_result res = TEST_PASSED, result;
	int i, j, nb_error = 0, len;
	u32 loop = 0, nb_loop;
	int argc_test;
	char *argv_test[4];
	char loop_string[] = "1";
	char pattern_string[] = PATTERN_DEFAULT;
	u32 *addr;

	if (get_nb_loop(string, argc, argv, 0, &nb_loop, 1))
		return TEST_ERROR;

	if (get_addr(string, argc, argv, 2, (u32 *)&addr))
		return TEST_ERROR;

	while (!nb_error) {
		/* execute all the test except the lasts which are infinite */
		for (i = 1; i < test_nb - NB_TEST_INFINITE; i++) {
			argc_test = 0;
			j = 0;
			len = strlen(test[i].usage);
			if (argc > 1 && j < len &&
			    !strncmp("[size]", &test[i].usage[j], 6)) {
				argv_test[argc_test++] = argv[1];
				j += 7;
			}
			if (argc > 2) {
				if (j < len &&
				    !strncmp("[loop]", &test[i].usage[j], 6)) {
					argv_test[argc_test++] = loop_string;
					j += 7;
				}
				if (j < len &&
				    !strncmp("[pattern]", &test[i].usage[j],
					     9)) {
					argv_test[argc_test++] = pattern_string;
					j += 10;
				}
				if (j < len &&
				    !strncmp("[addr]", &test[i].usage[j], 6)) {
					argv_test[argc_test++] = argv[2];
					j += 7;
				}
			}
			printf("execute %d:%s\n", (int)i, test[i].name);
			result = test[i].fct(ctl, phy, string,
					     argc_test, argv_test);
			printf("result %d:%s = ", (int)i, test[i].name);
			if (result != TEST_PASSED) {
				nb_error++;
				res = TEST_FAILED;
				puts("Failed");
			} else {
				puts("Passed");
			}
			puts("\n\n");
		}
		printf("loop %d: %d/%d test failed\n\n\n",
		       loop + 1, nb_error, test_nb - NB_TEST_INFINITE);
		if (test_loop_end(&loop, nb_loop, 1))
			break;
	}
	if (res != TEST_PASSED) {
		sprintf(string, "loop %d: %d/%d test failed", loop, nb_error,
			test_nb - NB_TEST_INFINITE);
	} else {
		sprintf(string, "loop %d: %d tests passed", loop,
			test_nb - NB_TEST_INFINITE);
	}
	return res;
}

/****************************************************************
 * TEST Description
 ****************************************************************/

const struct test_desc test[] = {
	{test_all, "All", "[loop] [size] [addr]", "Execute all tests", 3 },
	{test_databus, "Simple DataBus", "[addr]",
	 "Verifies each data line by walking 1 on fixed address",
	 1
	 },
	{databuswalk0, "DataBusWalking0", "[loop] [addr]",
	 "Verifies each data bus signal can be driven low (32 word burst)",
	 2
	},
	{databuswalk1, "DataBusWalking1", "[loop] [addr]",
	 "Verifies each data bus signal can be driven high (32 word burst)",
	 2
	},
	{test_addressbus, "AddressBus", "[size] [addr]",
	 "Verifies each relevant bits of the address and checking for aliasing",
	 2
	 },
	{test_memdevice, "MemDevice", "[size] [addr]",
	 "Test the integrity of a physical memory (test every storage bit in the region)",
	 2
	 },
	{test_sso, "SimultaneousSwitchingOutput", "[size] [addr] ",
	 "Stress the data bus over an address range",
	 2
	},
	{test_noise, "Noise", "[pattern] [addr]",
	 "Verifies r/w while forcing switching of all data bus lines.",
	 3
	},
	{test_noise_burst, "NoiseBurst", "[size] [pattern] [addr]",
	 "burst transfers while forcing switching of the data bus lines",
	 3
	},
	{test_random, "Random", "[size] [loop] [addr]",
	 "Verifies r/w and memcopy(burst for pseudo random value.",
	 3
	},
	{test_freq_pattern, "FrequencySelectivePattern", "[size] [addr]",
	 "write & test patterns: Mostly Zero, Mostly One and F/n",
	 2
	},
	{test_blockseq, "BlockSequential", "[size] [loop] [addr]",
	 "test incremental pattern",
	 3
	},
	{test_checkboard, "Checkerboard", "[size] [loop] [addr]",
	 "test checker pattern",
	 3
	},
	{test_bitspread, "BitSpread", "[size] [loop] [addr]",
	 "test Bit Spread pattern",
	 3
	},
	{test_bitflip, "BitFlip", "[size] [loop] [addr]",
	 "test Bit Flip pattern",
	 3
	},
	{test_walkbit0, "WalkingOnes", "[size] [loop] [addr]",
	 "test Walking Ones pattern",
	 3
	},
	{test_walkbit1, "WalkingZeroes", "[size] [loop] [addr]",
	 "test Walking Zeroes pattern",
	 3
	},
	/* need to the the 2 last one (infinite) : skipped for test all */
	{test_read, "infinite read", "[addr] [pattern]",
	 "basic test : infinite read access (random: addr=0xFFFFFFFF)", 2},
	{test_write, "infinite write", "[addr] [pattern]",
	 "basic test : infinite write access (random: addr=0xFFFFFFFF)", 2},
};

const int test_nb = ARRAY_SIZE(test);
