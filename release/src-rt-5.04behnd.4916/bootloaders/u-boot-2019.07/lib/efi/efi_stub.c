// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 *
 * EFI information obtained here:
 * http://wiki.phoenix.com/wiki/index.php/EFI_BOOT_SERVICES
 *
 * Loads a payload (U-Boot) within the EFI environment. This is built as an
 * EFI application. It can be built either in 32-bit or 64-bit mode.
 */

#include <common.h>
#include <debug_uart.h>
#include <efi.h>
#include <efi_api.h>
#include <errno.h>
#include <ns16550.h>
#include <asm/cpu.h>
#include <asm/io.h>
#include <linux/err.h>
#include <linux/types.h>

#ifndef CONFIG_X86
/*
 * Problem areas:
 * - putc() uses the ns16550 address directly and assumed I/O access. Many
 *	platforms will use memory access
 * get_codeseg32() is only meaningful on x86
 */
#error "This file needs to be ported for use on architectures"
#endif

static struct efi_priv *global_priv;
static bool use_uart;

struct __packed desctab_info {
	uint16_t limit;
	uint64_t addr;
	uint16_t pad;
};

/*
 * EFI uses Unicode and we don't. The easiest way to get a sensible output
 * function is to use the U-Boot debug UART. We use EFI's console output
 * function where available, and assume the built-in UART after that. We rely
 * on EFI to set up the UART for us and just bring in the functions here.
 * This last bit is a bit icky, but it's only for debugging anyway. We could
 * build in ns16550.c with some effort, but this is a payload loader after
 * all.
 *
 * Note: We avoid using printf() so we don't need to bring in lib/vsprintf.c.
 * That would require some refactoring since we already build this for U-Boot.
 * Building an EFI shared library version would have to be a separate stem.
 * That might push us to using the SPL framework to build this stub. However
 * that would involve a round of EFI-specific changes in SPL. Worth
 * considering if we start needing more U-Boot functionality. Note that we
 * could then move get_codeseg32() to arch/x86/cpu/cpu.c.
 */
void _debug_uart_init(void)
{
}

void putc(const char ch)
{
	if (ch == '\n')
		putc('\r');

	if (use_uart) {
		NS16550_t com_port = (NS16550_t)0x3f8;

		while ((inb((ulong)&com_port->lsr) & UART_LSR_THRE) == 0)
			;
		outb(ch, (ulong)&com_port->thr);
	} else {
		efi_putc(global_priv, ch);
	}
}

void puts(const char *str)
{
	while (*str)
		putc(*str++);
}

static void _debug_uart_putc(int ch)
{
	putc(ch);
}

DEBUG_UART_FUNCS

void *memcpy(void *dest, const void *src, size_t size)
{
	unsigned char *dptr = dest;
	const unsigned char *ptr = src;
	const unsigned char *end = src + size;

	while (ptr < end)
		*dptr++ = *ptr++;

	return dest;
}

void *memset(void *inptr, int ch, size_t size)
{
	char *ptr = inptr;
	char *end = ptr + size;

	while (ptr < end)
		*ptr++ = ch;

	return ptr;
}

static void jump_to_uboot(ulong cs32, ulong addr, ulong info)
{
#ifdef CONFIG_EFI_STUB_32BIT
	/*
	 * U-Boot requires these parameters in registers, not on the stack.
	 * See _x86boot_start() for this code.
	 */
	typedef void (*func_t)(int bist, int unused, ulong info)
		__attribute__((regparm(3)));

	((func_t)addr)(0, 0, info);
#else
	cpu_call32(cs32, CONFIG_SYS_TEXT_BASE, info);
#endif
}

#ifdef CONFIG_EFI_STUB_64BIT
static void get_gdt(struct desctab_info *info)
{
	asm volatile ("sgdt %0" : : "m"(*info) : "memory");
}
#endif

static inline unsigned long read_cr3(void)
{
	unsigned long val;

	asm volatile("mov %%cr3,%0" : "=r" (val) : : "memory");
	return val;
}

/**
 * get_codeseg32() - Find the code segment to use for 32-bit code
 *
 * U-Boot only works in 32-bit mode at present, so when booting from 64-bit
 * EFI we must first change to 32-bit mode. To do this we need to find the
 * correct code segment to use (an entry in the Global Descriptor Table).
 *
 * @return code segment GDT offset, or 0 for 32-bit EFI, -ENOENT if not found
 */
static int get_codeseg32(void)
{
	int cs32 = 0;

#ifdef CONFIG_EFI_STUB_64BIT
	struct desctab_info gdt;
	uint64_t *ptr;
	int i;

	get_gdt(&gdt);
	for (ptr = (uint64_t *)(unsigned long)gdt.addr, i = 0; i < gdt.limit;
	     i += 8, ptr++) {
		uint64_t desc = *ptr;
		uint64_t base, limit;

		/*
		 * Check that the target U-Boot jump address is within the
		 * selector and that the selector is of the right type.
		 */
		base = ((desc >> GDT_BASE_LOW_SHIFT) & GDT_BASE_LOW_MASK) |
			((desc >> GDT_BASE_HIGH_SHIFT) & GDT_BASE_HIGH_MASK)
				<< 16;
		limit = ((desc >> GDT_LIMIT_LOW_SHIFT) & GDT_LIMIT_LOW_MASK) |
			((desc >> GDT_LIMIT_HIGH_SHIFT) & GDT_LIMIT_HIGH_MASK)
				<< 16;
		base <<= 12;	/* 4KB granularity */
		limit <<= 12;
		if ((desc & GDT_PRESENT) && (desc & GDT_NOTSYS) &&
		    !(desc & GDT_LONG) && (desc & GDT_4KB) &&
		    (desc & GDT_32BIT) && (desc & GDT_CODE) &&
		    CONFIG_SYS_TEXT_BASE > base &&
		    CONFIG_SYS_TEXT_BASE + CONFIG_SYS_MONITOR_LEN < limit
		) {
			cs32 = i;
			break;
		}
	}

#ifdef DEBUG
	puts("\ngdt: ");
	printhex8(gdt.limit);
	puts(", addr: ");
	printhex8(gdt.addr >> 32);
	printhex8(gdt.addr);
	for (i = 0; i < gdt.limit; i += 8) {
		uint32_t *ptr = (uint32_t *)((unsigned long)gdt.addr + i);

		puts("\n");
		printhex2(i);
		puts(": ");
		printhex8(ptr[1]);
		puts("  ");
		printhex8(ptr[0]);
	}
	puts("\n ");
	puts("32-bit code segment: ");
	printhex2(cs32);
	puts("\n ");

	puts("page_table: ");
	printhex8(read_cr3());
	puts("\n ");
#endif
	if (!cs32) {
		puts("Can't find 32-bit code segment\n");
		return -ENOENT;
	}
#endif

	return cs32;
}

static int setup_info_table(struct efi_priv *priv, int size)
{
	struct efi_info_hdr *info;
	efi_status_t ret;

	/* Get some memory for our info table */
	priv->info_size = size;
	info = efi_malloc(priv, priv->info_size, &ret);
	if (ret) {
		printhex2(ret);
		puts(" No memory for info table: ");
		return ret;
	}

	memset(info, '\0', sizeof(*info));
	info->version = EFI_TABLE_VERSION;
	info->hdr_size = sizeof(*info);
	priv->info = info;
	priv->next_hdr = (char *)info + info->hdr_size;

	return 0;
}

static void add_entry_addr(struct efi_priv *priv, enum efi_entry_t type,
			   void *ptr1, int size1, void *ptr2, int size2)
{
	struct efi_entry_hdr *hdr = priv->next_hdr;

	hdr->type = type;
	hdr->size = size1 + size2;
	hdr->addr = 0;
	hdr->link = ALIGN(sizeof(*hdr) + hdr->size, 16);
	priv->next_hdr += hdr->link;
	memcpy(hdr + 1, ptr1, size1);
	memcpy((void *)(hdr + 1) + size1, ptr2, size2);
	priv->info->total_size = (ulong)priv->next_hdr - (ulong)priv->info;
}

/**
 * efi_main() - Start an EFI image
 *
 * This function is called by our EFI start-up code. It handles running
 * U-Boot. If it returns, EFI will continue.
 */
efi_status_t EFIAPI efi_main(efi_handle_t image,
			     struct efi_system_table *sys_table)
{
	struct efi_priv local_priv, *priv = &local_priv;
	struct efi_boot_services *boot = sys_table->boottime;
	struct efi_mem_desc *desc;
	struct efi_entry_memmap map;
	struct efi_gop *gop;
	struct efi_entry_gopmode mode;
	struct efi_entry_systable table;
	efi_guid_t efi_gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	efi_uintn_t key, desc_size, size;
	efi_status_t ret;
	u32 version;
	int cs32;

	ret = efi_init(priv, "Payload", image, sys_table);
	if (ret) {
		printhex2(ret);
		puts(" efi_init() failed\n");
		return ret;
	}
	global_priv = priv;

	cs32 = get_codeseg32();
	if (cs32 < 0)
		return EFI_UNSUPPORTED;

	/* Get the memory map so we can switch off EFI */
	size = 0;
	ret = boot->get_memory_map(&size, NULL, &key, &desc_size, &version);
	if (ret != EFI_BUFFER_TOO_SMALL) {
		printhex2(EFI_BITS_PER_LONG);
		putc(' ');
		printhex2(ret);
		puts(" No memory map\n");
		return ret;
	}
	size += 1024;	/* Since doing a malloc() may change the memory map! */
	desc = efi_malloc(priv, size, &ret);
	if (!desc) {
		printhex2(ret);
		puts(" No memory for memory descriptor\n");
		return ret;
	}
	ret = setup_info_table(priv, size + 128);
	if (ret)
		return ret;

	ret = boot->locate_protocol(&efi_gop_guid, NULL, (void **)&gop);
	if (ret) {
		puts(" GOP unavailable\n");
	} else {
		mode.fb_base = gop->mode->fb_base;
		mode.fb_size = gop->mode->fb_size;
		mode.info_size = gop->mode->info_size;
		add_entry_addr(priv, EFIET_GOP_MODE, &mode, sizeof(mode),
			       gop->mode->info,
			       sizeof(struct efi_gop_mode_info));
	}

	ret = boot->get_memory_map(&size, desc, &key, &desc_size, &version);
	if (ret) {
		printhex2(ret);
		puts(" Can't get memory map\n");
		return ret;
	}

	table.sys_table = (ulong)sys_table;
	add_entry_addr(priv, EFIET_SYS_TABLE, &table, sizeof(table), NULL, 0);

	ret = boot->exit_boot_services(image, key);
	if (ret) {
		/*
		 * Unfortunately it happens that we cannot exit boot services
		 * the first time. But the second time it work. I don't know
		 * why but this seems to be a repeatable problem. To get
		 * around it, just try again.
		 */
		printhex2(ret);
		puts(" Can't exit boot services\n");
		size = sizeof(desc);
		ret = boot->get_memory_map(&size, desc, &key, &desc_size,
					   &version);
		if (ret) {
			printhex2(ret);
			puts(" Can't get memory map\n");
			return ret;
		}
		ret = boot->exit_boot_services(image, key);
		if (ret) {
			printhex2(ret);
			puts(" Can't exit boot services 2\n");
			return ret;
		}
	}

	/* The EFI UART won't work now, switch to a debug one */
	use_uart = true;

	map.version = version;
	map.desc_size = desc_size;
	add_entry_addr(priv, EFIET_MEMORY_MAP, &map, sizeof(map), desc, size);
	add_entry_addr(priv, EFIET_END, NULL, 0, 0, 0);

	memcpy((void *)CONFIG_SYS_TEXT_BASE, _binary_u_boot_bin_start,
	       (ulong)_binary_u_boot_bin_end -
	       (ulong)_binary_u_boot_bin_start);

#ifdef DEBUG
	puts("EFI table at ");
	printhex8((ulong)priv->info);
	puts(" size ");
	printhex8(priv->info->total_size);
#endif
	putc('\n');
	jump_to_uboot(cs32, CONFIG_SYS_TEXT_BASE, (ulong)priv->info);

	return EFI_LOAD_ERROR;
}
