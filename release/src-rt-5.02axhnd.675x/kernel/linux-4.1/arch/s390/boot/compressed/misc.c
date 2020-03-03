/*
 * Definitions and wrapper functions for kernel decompressor
 *
 * Copyright IBM Corp. 2010
 *
 * Author(s): Martin Schwidefsky <schwidefsky@de.ibm.com>
 */

#include <asm/uaccess.h>
#include <asm/page.h>
#include <asm/sclp.h>
#include <asm/ipl.h>
#include "sizes.h"

/*
 * gzip declarations
 */
#define STATIC static

#undef memset
#undef memcpy
#undef memmove
#define memmove memmove
#define memzero(s, n) memset((s), 0, (n))

/* Symbols defined by linker scripts */
extern char input_data[];
extern int input_len;
extern char _text, _end;
extern char _bss, _ebss;

static void error(char *m);

static unsigned long free_mem_ptr;
static unsigned long free_mem_end_ptr;

#ifdef CONFIG_HAVE_KERNEL_BZIP2
#define HEAP_SIZE	0x400000
#else
#define HEAP_SIZE	0x10000
#endif

#ifdef CONFIG_KERNEL_GZIP
#include "../../../../lib/decompress_inflate.c"
#endif

#ifdef CONFIG_KERNEL_BZIP2
#include "../../../../lib/decompress_bunzip2.c"
#endif

#ifdef CONFIG_KERNEL_LZ4
#include "../../../../lib/decompress_unlz4.c"
#endif

#ifdef CONFIG_KERNEL_LZMA
#include "../../../../lib/decompress_unlzma.c"
#endif

#ifdef CONFIG_KERNEL_LZO
#include "../../../../lib/decompress_unlzo.c"
#endif

#ifdef CONFIG_KERNEL_XZ
#include "../../../../lib/decompress_unxz.c"
#endif

static int puts(const char *s)
{
	_sclp_print_early(s);
	return 0;
}

void *memset(void *s, int c, size_t n)
{
	char *xs;

	xs = s;
	while (n--)
		*xs++ = c;
	return s;
}

void *memcpy(void *dest, const void *src, size_t n)
{
	const char *s = src;
	char *d = dest;

	while (n--)
		*d++ = *s++;
	return dest;
}

void *memmove(void *dest, const void *src, size_t n)
{
	const char *s = src;
	char *d = dest;

	if (d <= s) {
		while (n--)
			*d++ = *s++;
	} else {
		d += n;
		s += n;
		while (n--)
			*--d = *--s;
	}
	return dest;
}

static void error(char *x)
{
	unsigned long long psw = 0x000a0000deadbeefULL;

	puts("\n\n");
	puts(x);
	puts("\n\n -- System halted");

	asm volatile("lpsw %0" : : "Q" (psw));
}

/*
 * Safe guard the ipl parameter block against a memory area that will be
 * overwritten. The validity check for the ipl parameter block is complex
 * (see cio_get_iplinfo and ipl_save_parameters) but if the pointer to
 * the ipl parameter block intersects with the passed memory area we can
 * safely assume that we can read from that memory. In that case just copy
 * the memory to IPL_PARMBLOCK_ORIGIN even if there is no ipl parameter
 * block.
 */
static void check_ipl_parmblock(void *start, unsigned long size)
{
	void *src, *dst;

	src = (void *)(unsigned long) S390_lowcore.ipl_parmblock_ptr;
	if (src + PAGE_SIZE <= start || src >= start + size)
		return;
	dst = (void *) IPL_PARMBLOCK_ORIGIN;
	memmove(dst, src, PAGE_SIZE);
	S390_lowcore.ipl_parmblock_ptr = IPL_PARMBLOCK_ORIGIN;
}

unsigned long decompress_kernel(void)
{
	void *output, *kernel_end;

	output = (void *) ALIGN((unsigned long) &_end + HEAP_SIZE, PAGE_SIZE);
	kernel_end = output + SZ__bss_start;
	check_ipl_parmblock((void *) 0, (unsigned long) kernel_end);

#ifdef CONFIG_BLK_DEV_INITRD
	/*
	 * Move the initrd right behind the end of the decompressed
	 * kernel image. This also prevents initrd corruption caused by
	 * bss clearing since kernel_end will always be located behind the
	 * current bss section..
	 */
	if (INITRD_START && INITRD_SIZE && kernel_end > (void *) INITRD_START) {
		check_ipl_parmblock(kernel_end, INITRD_SIZE);
		memmove(kernel_end, (void *) INITRD_START, INITRD_SIZE);
		INITRD_START = (unsigned long) kernel_end;
	}
#endif

	/*
	 * Clear bss section. free_mem_ptr and free_mem_end_ptr need to be
	 * initialized afterwards since they reside in bss.
	 */
	memset(&_bss, 0, &_ebss - &_bss);
	free_mem_ptr = (unsigned long) &_end;
	free_mem_end_ptr = free_mem_ptr + HEAP_SIZE;

	puts("Uncompressing Linux... ");
	__decompress(input_data, input_len, NULL, NULL, output, 0, NULL, error);
	puts("Ok, booting the kernel.\n");
	return (unsigned long) output;
}

