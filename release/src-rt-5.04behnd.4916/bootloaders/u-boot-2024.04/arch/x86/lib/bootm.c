// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Copyright (C) 2001  Erik Mouw (J.A.K.Mouw@its.tudelft.nl)
 */

#include <common.h>
#include <bootm.h>
#include <bootstage.h>
#include <command.h>
#include <efi.h>
#include <hang.h>
#include <log.h>
#include <asm/global_data.h>
#include <dm/device.h>
#include <dm/root.h>
#include <errno.h>
#include <fdt_support.h>
#include <image.h>
#include <u-boot/zlib.h>
#include <asm/bootparam.h>
#include <asm/cpu.h>
#include <asm/byteorder.h>
#include <asm/zimage.h>
#ifdef CONFIG_SYS_COREBOOT
#include <asm/arch/timestamp.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define COMMAND_LINE_OFFSET 0x9000

void bootm_announce_and_cleanup(void)
{
	printf("\nStarting kernel ...\n\n");

#ifdef CONFIG_SYS_COREBOOT
	timestamp_add_now(TS_START_KERNEL);
#endif
	bootstage_mark_name(BOOTSTAGE_ID_BOOTM_HANDOFF, "start_kernel");
#if IS_ENABLED(CONFIG_BOOTSTAGE_REPORT)
	bootstage_report();
#endif

	/*
	 * Call remove function of all devices with a removal flag set.
	 * This may be useful for last-stage operations, like cancelling
	 * of DMA operation or releasing device internal buffers.
	 */
	dm_remove_devices_flags(DM_REMOVE_ACTIVE_ALL);
}

#if defined(CONFIG_OF_LIBFDT) && !defined(CONFIG_OF_NO_KERNEL)
int arch_fixup_memory_node(void *blob)
{
	struct bd_info	*bd = gd->bd;
	int bank;
	u64 start[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		start[bank] = bd->bi_dram[bank].start;
		size[bank] = bd->bi_dram[bank].size;
	}

	return fdt_fixup_memory_banks(blob, start, size, CONFIG_NR_DRAM_BANKS);
}
#endif

/* Subcommand: PREP */
static int boot_prep_linux(struct bootm_headers *images)
{
	char *cmd_line_dest = NULL;
	struct legacy_img_hdr *hdr;
	int is_zimage = 0;
	void *data = NULL;
	size_t len;
	int ret;

	if (CONFIG_IS_ENABLED(OF_LIBFDT) && IS_ENABLED(CONFIG_LMB) && images->ft_len) {
		debug("using: FDT\n");
		if (image_setup_linux(images)) {
			puts("FDT creation failed! hanging...");
			hang();
		}
	}

	if (images->legacy_hdr_valid) {
		hdr = images->legacy_hdr_os;
		if (image_check_type(hdr, IH_TYPE_MULTI)) {
			ulong os_data, os_len;

			/* if multi-part image, we need to get first subimage */
			image_multi_getimg(hdr, 0, &os_data, &os_len);
			data = (void *)os_data;
			len = os_len;
		} else {
			/* otherwise get image data */
			data = (void *)image_get_data(hdr);
			len = image_get_data_size(hdr);
		}
		is_zimage = 1;
#if defined(CONFIG_FIT)
	} else if (images->fit_uname_os && is_zimage) {
		ret = fit_image_get_data(images->fit_hdr_os,
				images->fit_noffset_os,
				(const void **)&data, &len);
		if (ret) {
			puts("Can't get image data/size!\n");
			goto error;
		}
		is_zimage = 1;
#endif
	}

	if (is_zimage) {
		ulong load_address;
		char *base_ptr;

		base_ptr = (char *)load_zimage(data, len, &load_address);
		if (!base_ptr) {
			puts("## Kernel loading failed ...\n");
			goto error;
		}
		images->os.load = load_address;
		cmd_line_dest = base_ptr + COMMAND_LINE_OFFSET;
		images->ep = (ulong)base_ptr;
	} else if (images->ep) {
		cmd_line_dest = (void *)images->ep + COMMAND_LINE_OFFSET;
	} else {
		printf("## Kernel loading failed (missing x86 kernel setup) ...\n");
		goto error;
	}

	printf("Setup at %#08lx\n", images->ep);
	ret = setup_zimage((void *)images->ep, cmd_line_dest,
			0, images->rd_start,
			images->rd_end - images->rd_start, 0);

	if (ret) {
		printf("## Setting up boot parameters failed ...\n");
		return 1;
	}

	return 0;

error:
	return 1;
}

int boot_linux_kernel(ulong setup_base, ulong entry, bool image_64bit)
{
	bootm_announce_and_cleanup();

#ifdef CONFIG_SYS_COREBOOT
	timestamp_add_now(TS_U_BOOT_START_KERNEL);
#endif

	/*
	 * Exit EFI boot services just before jumping, after all console
	 * output, since the console won't be available afterwards.
	 */
	if (IS_ENABLED(CONFIG_EFI_APP)) {
		int ret;

		ret = efi_store_memory_map(efi_get_priv());
		if (ret)
			return ret;
		printf("Exiting EFI boot services\n");
		ret = efi_call_exit_boot_services();
		if (ret)
			return ret;
	}

	if (image_64bit) {
		if (!cpu_has_64bit()) {
			puts("Cannot boot 64-bit kernel on 32-bit machine\n");
			return -EFAULT;
		}
		/*
		 * At present 64-bit U-Boot only supports booting a 64-bit
		 * kernel.
		 *
		 * TODO(sjg@chromium.org): Support booting 32-bit kernels from
		 * 64-bit U-Boot
		 */
		if (CONFIG_IS_ENABLED(X86_64)) {
			typedef void (*h_func)(ulong zero, ulong setup);
			h_func func;

			/* jump to Linux with rdi=0, rsi=setup_base */
			func = (h_func)entry;
			func(0, setup_base);
		} else {
			return cpu_jump_to_64bit(setup_base, entry);
		}
	} else {
		/*
		* Set %ebx, %ebp, and %edi to 0, %esi to point to the
		* boot_params structure, and then jump to the kernel. We
		* assume that %cs is 0x10, 4GB flat, and read/execute, and
		* the data segments are 0x18, 4GB flat, and read/write.
		* U-Boot is setting them up that way for itself in
		* arch/i386/cpu/cpu.c.
		*
		* Note: this is incomplete for EFI kernels!
		*
		* This can boot a kernel while running as an EFI application,
		* but if the kernel requires EFI support then that support needs
		* to be enabled first (see EFI_LOADER). Also the EFI information
		* must enabled with setup_efi_info(). See setup_zimage() for
		* how this is done with the stub.
		*/
		__asm__ __volatile__ (
		"movl $0, %%ebp\n"
		"cli\n"
		"jmp *%[kernel_entry]\n"
		:: [kernel_entry]"a"(entry),
		[boot_params] "S"(setup_base),
		"b"(0), "D"(0)
		);
	}

	/* We can't get to here */
	return -EFAULT;
}

/* Subcommand: GO */
static int boot_jump_linux(struct bootm_headers *images)
{
	debug("## Transferring control to Linux (at address %08lx, kernel %08lx) ...\n",
	      images->ep, images->os.load);

	return boot_linux_kernel(images->ep, images->os.load,
				 images->os.arch == IH_ARCH_X86_64);
}

int do_bootm_linux(int flag, struct bootm_info *bmi)
{
	struct bootm_headers *images = bmi->images;

	/* No need for those on x86 */
	if (flag & BOOTM_STATE_OS_BD_T || flag & BOOTM_STATE_OS_CMDLINE)
		return -1;

	if (flag & BOOTM_STATE_OS_PREP)
		return boot_prep_linux(images);

	if (flag & BOOTM_STATE_OS_GO)
		return boot_jump_linux(images);

	return boot_jump_linux(images);
}

static ulong get_sp(void)
{
	ulong ret;

#if CONFIG_IS_ENABLED(X86_64)
	asm("mov %%rsp, %0" : "=r"(ret) : );
#else
	asm("mov %%esp, %0" : "=r"(ret) : );
#endif

	return ret;
}

void arch_lmb_reserve(struct lmb *lmb)
{
	arch_lmb_reserve_generic(lmb, get_sp(), gd->ram_top, 4096);
}
