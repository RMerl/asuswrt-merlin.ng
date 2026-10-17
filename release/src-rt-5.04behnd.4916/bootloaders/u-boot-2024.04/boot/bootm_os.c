// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <bootm.h>
#include <bootstage.h>
#include <cpu_func.h>
#include <efi_loader.h>
#include <env.h>
#include <fdt_support.h>
#include <image.h>
#include <lmb.h>
#include <log.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <malloc.h>
#include <mapmem.h>
#include <vxworks.h>
#include <tee/optee.h>

DECLARE_GLOBAL_DATA_PTR;

static int do_bootm_standalone(int flag, struct bootm_info *bmi)
{
	struct bootm_headers *images = bmi->images;
	int (*appl)(int, char *const[]);

	if (!env_get_autostart()) {
		env_set_hex("filesize", images->os.image_len);
		return 0;
	}
	appl = (int (*)(int, char * const []))images->ep;
	appl(bmi->argc, bmi->argv);
	return 0;
}

/*******************************************************************/
/* OS booting routines */
/*******************************************************************/

#if defined(CONFIG_BOOTM_NETBSD) || defined(CONFIG_BOOTM_PLAN9)
static void copy_args(char *dest, int argc, char *const argv[], char delim)
{
	int i;

	for (i = 0; i < argc; i++) {
		if (i > 0)
			*dest++ = delim;
		strcpy(dest, argv[i]);
		dest += strlen(argv[i]);
	}
}
#endif

static void __maybe_unused fit_unsupported_reset(const char *msg)
{
	if (CONFIG_IS_ENABLED(FIT_VERBOSE)) {
		printf("! FIT images not supported for '%s' - must reset board to recover!\n",
		       msg);
	}
}

#ifdef CONFIG_BOOTM_NETBSD
static int do_bootm_netbsd(int flag, struct bootm_info *bmi)
{
	struct bootm_headers *images = bmi->images;
	void (*loader)(struct bd_info *bd, struct legacy_img_hdr *hdr,
		       char *console, char *cmdline);
	struct legacy_img_hdr *os_hdr, *hdr;
	ulong kernel_data, kernel_len;
	char *cmdline;

	if (flag != BOOTM_STATE_OS_GO)
		return 0;

#if defined(CONFIG_FIT)
	if (!images->legacy_hdr_valid) {
		fit_unsupported_reset("NetBSD");
		return 1;
	}
#endif
	hdr = images->legacy_hdr_os;

	/*
	 * Booting a (NetBSD) kernel image
	 *
	 * This process is pretty similar to a standalone application:
	 * The (first part of an multi-) image must be a stage-2 loader,
	 * which in turn is responsible for loading & invoking the actual
	 * kernel.  The only differences are the parameters being passed:
	 * besides the board info strucure, the loader expects a command
	 * line, the name of the console device, and (optionally) the
	 * address of the original image header.
	 */
	os_hdr = NULL;
	if (image_check_type(&images->legacy_hdr_os_copy, IH_TYPE_MULTI)) {
		image_multi_getimg(hdr, 1, &kernel_data, &kernel_len);
		if (kernel_len)
			os_hdr = hdr;
	}

	if (bmi->argc > 0) {
		ulong len;
		int   i;

		for (i = 0, len = 0; i < bmi->argc; i += 1)
			len += strlen(bmi->argv[i]) + 1;
		cmdline = malloc(len);
		copy_args(cmdline, bmi->argc, bmi->argv, ' ');
	} else {
		cmdline = env_get("bootargs");
		if (cmdline == NULL)
			cmdline = "";
	}

	loader = (void (*)(struct bd_info *, struct legacy_img_hdr *, char *, char *))images->ep;

	printf("## Transferring control to NetBSD stage-2 loader (at address %08lx) ...\n",
	       (ulong)loader);

	bootstage_mark(BOOTSTAGE_ID_RUN_OS);

	/*
	 * NetBSD Stage-2 Loader Parameters:
	 *   arg[0]: pointer to board info data
	 *   arg[1]: image load address
	 *   arg[2]: char pointer to the console device to use
	 *   arg[3]: char pointer to the boot arguments
	 */
	(*loader)(gd->bd, os_hdr, "", cmdline);

	return 1;
}
#endif /* CONFIG_BOOTM_NETBSD*/

#ifdef CONFIG_BOOTM_RTEMS
static int do_bootm_rtems(int flag, struct bootm_info *bmi)
{
	struct bootm_headers *images = bmi->images;
	void (*entry_point)(struct bd_info *);

	if (flag != BOOTM_STATE_OS_GO)
		return 0;

#if defined(CONFIG_FIT)
	if (!images->legacy_hdr_valid) {
		fit_unsupported_reset("RTEMS");
		return 1;
	}
#endif

	entry_point = (void (*)(struct bd_info *))images->ep;

	printf("## Transferring control to RTEMS (at address %08lx) ...\n",
	       (ulong)entry_point);

	bootstage_mark(BOOTSTAGE_ID_RUN_OS);

	/*
	 * RTEMS Parameters:
	 *   r3: ptr to board info data
	 */
	(*entry_point)(gd->bd);

	return 1;
}
#endif /* CONFIG_BOOTM_RTEMS */

#if defined(CONFIG_BOOTM_OSE)
static int do_bootm_ose(int flag, struct bootm_info *bmi)
{
	struct bootm_headers *images = bmi->images;
	void (*entry_point)(void);

	if (flag != BOOTM_STATE_OS_GO)
		return 0;

#if defined(CONFIG_FIT)
	if (!images->legacy_hdr_valid) {
		fit_unsupported_reset("OSE");
		return 1;
	}
#endif

	entry_point = (void (*)(void))images->ep;

	printf("## Transferring control to OSE (at address %08lx) ...\n",
	       (ulong)entry_point);

	bootstage_mark(BOOTSTAGE_ID_RUN_OS);

	/*
	 * OSE Parameters:
	 *   None
	 */
	(*entry_point)();

	return 1;
}
#endif /* CONFIG_BOOTM_OSE */

#if defined(CONFIG_BOOTM_PLAN9)
static int do_bootm_plan9(int flag, struct bootm_info *bmi)
{
	struct bootm_headers *images = bmi->images;
	void (*entry_point)(void);
	char *s;

	if (flag != BOOTM_STATE_OS_GO)
		return 0;

#if defined(CONFIG_FIT)
	if (!images->legacy_hdr_valid) {
		fit_unsupported_reset("Plan 9");
		return 1;
	}
#endif

	/* See README.plan9 */
	s = env_get("confaddr");
	if (s != NULL) {
		char *confaddr = (char *)hextoul(s, NULL);

		if (bmi->argc) {
			copy_args(confaddr, bmi->argc, bmi->argv, '\n');
		} else {
			s = env_get("bootargs");
			if (s != NULL)
				strcpy(confaddr, s);
		}
	}

	entry_point = (void (*)(void))images->ep;

	printf("## Transferring control to Plan 9 (at address %08lx) ...\n",
	       (ulong)entry_point);

	bootstage_mark(BOOTSTAGE_ID_RUN_OS);

	/*
	 * Plan 9 Parameters:
	 *   None
	 */
	(*entry_point)();

	return 1;
}
#endif /* CONFIG_BOOTM_PLAN9 */

#if defined(CONFIG_BOOTM_VXWORKS) && \
	(defined(CONFIG_PPC) || defined(CONFIG_ARM))

static void do_bootvx_fdt(struct bootm_headers *images)
{
#if defined(CONFIG_OF_LIBFDT)
	int ret;
	char *bootline;
	ulong of_size = images->ft_len;
	char **of_flat_tree = &images->ft_addr;
	struct lmb *lmb = &images->lmb;

	if (*of_flat_tree) {
		boot_fdt_add_mem_rsv_regions(lmb, *of_flat_tree);

		ret = boot_relocate_fdt(lmb, of_flat_tree, &of_size);
		if (ret)
			return;

		/* Update ethernet nodes */
		fdt_fixup_ethernet(*of_flat_tree);

		ret = fdt_add_subnode(*of_flat_tree, 0, "chosen");
		if ((ret >= 0 || ret == -FDT_ERR_EXISTS)) {
			bootline = env_get("bootargs");
			if (bootline) {
				ret = fdt_find_and_setprop(*of_flat_tree,
						"/chosen", "bootargs",
						bootline,
						strlen(bootline) + 1, 1);
				if (ret < 0) {
					printf("## ERROR: %s : %s\n", __func__,
					       fdt_strerror(ret));
					return;
				}
			}
		} else {
			printf("## ERROR: %s : %s\n", __func__,
			       fdt_strerror(ret));
			return;
		}
	}
#endif

	boot_prep_vxworks(images);

	bootstage_mark(BOOTSTAGE_ID_RUN_OS);

#if defined(CONFIG_OF_LIBFDT)
	printf("## Starting vxWorks at 0x%08lx, device tree at 0x%08lx ...\n",
	       (ulong)images->ep, (ulong)*of_flat_tree);
#else
	printf("## Starting vxWorks at 0x%08lx\n", (ulong)images->ep);
#endif
	flush();

	boot_jump_vxworks(images);

	puts("## vxWorks terminated\n");
}

static int do_bootm_vxworks_legacy(int flag, struct bootm_info *bmi)
{
	struct bootm_headers *images = bmi->images;

	if (flag != BOOTM_STATE_OS_GO)
		return 0;

	do_bootvx_fdt(images);

	return 1;
}

int do_bootm_vxworks(int flag, struct bootm_info *bmi)
{
	char *bootargs;
	int pos;
	unsigned long vxflags;
	bool std_dtb = false;

	/* get bootargs env */
	bootargs = env_get("bootargs");

	if (bootargs != NULL) {
		for (pos = 0; pos < strlen(bootargs); pos++) {
			/* find f=0xnumber flag */
			if ((bootargs[pos] == '=') && (pos >= 1) &&
			    (bootargs[pos - 1] == 'f')) {
				vxflags = hextoul(&bootargs[pos + 1], NULL);
				if (vxflags & VXWORKS_SYSFLG_STD_DTB)
					std_dtb = true;
			}
		}
	}

	if (std_dtb) {
		if (flag & BOOTM_STATE_OS_PREP)
			printf("   Using standard DTB\n");
		return do_bootm_linux(flag, bmi);
	} else {
		if (flag & BOOTM_STATE_OS_PREP)
			printf("   !!! WARNING !!! Using legacy DTB\n");
		return do_bootm_vxworks_legacy(flag, bmi);
	}
}
#endif

#if defined(CONFIG_CMD_ELF)
static int do_bootm_qnxelf(int flag, struct bootm_info *bmi)
{
	struct bootm_headers *images = bmi->images;
	char *local_args[2];
	char str[16];
	int dcache;

	if (flag != BOOTM_STATE_OS_GO)
		return 0;

#if defined(CONFIG_FIT)
	if (!images->legacy_hdr_valid) {
		fit_unsupported_reset("QNX");
		return 1;
	}
#endif

	sprintf(str, "%lx", images->ep); /* write entry-point into string */
	local_args[0] = bmi->argv[0];
	local_args[1] = str;	/* and provide it via the arguments */

	/*
	 * QNX images require the data cache is disabled.
	 */
	dcache = dcache_status();
	if (dcache)
		dcache_disable();

	do_bootelf(NULL, 0, 2, local_args);

	if (dcache)
		dcache_enable();

	return 1;
}
#endif

#ifdef CONFIG_INTEGRITY
static int do_bootm_integrity(int flag, struct bootm_info *bmi)
{
	struct bootm_headers *images = bmi->images;
	void (*entry_point)(void);

	if (flag != BOOTM_STATE_OS_GO)
		return 0;

#if defined(CONFIG_FIT)
	if (!images->legacy_hdr_valid) {
		fit_unsupported_reset("INTEGRITY");
		return 1;
	}
#endif

	entry_point = (void (*)(void))images->ep;

	printf("## Transferring control to INTEGRITY (at address %08lx) ...\n",
	       (ulong)entry_point);

	bootstage_mark(BOOTSTAGE_ID_RUN_OS);

	/*
	 * INTEGRITY Parameters:
	 *   None
	 */
	(*entry_point)();

	return 1;
}
#endif

#ifdef CONFIG_BOOTM_OPENRTOS
static int do_bootm_openrtos(int flag, struct bootm_info *bmi)
{
	struct bootm_headers *images = bmi->images;
	void (*entry_point)(void);

	if (flag != BOOTM_STATE_OS_GO)
		return 0;

	entry_point = (void (*)(void))images->ep;

	printf("## Transferring control to OpenRTOS (at address %08lx) ...\n",
		(ulong)entry_point);

	bootstage_mark(BOOTSTAGE_ID_RUN_OS);

	/*
	 * OpenRTOS Parameters:
	 *   None
	 */
	(*entry_point)();

	return 1;
}
#endif

#ifdef CONFIG_BOOTM_OPTEE
static int do_bootm_tee(int flag, struct bootm_info *bmi)
{
	struct bootm_headers *images = bmi->images;
	int ret;

	/* Validate OPTEE header */
	ret = optee_verify_bootm_image(images->os.image_start,
				       images->os.load,
				       images->os.image_len);
	if (ret)
		return ret;

	/* From here we can run the regular linux boot path */
	return do_bootm_linux(flag, bmi);
}
#endif

#ifdef CONFIG_BOOTM_EFI
static int do_bootm_efi(int flag, struct bootm_info *bmi)
{
	struct bootm_headers *images = bmi->images;
	int ret;
	void *image_buf;

	if (flag != BOOTM_STATE_OS_GO)
		return 0;

	/* We expect to return */
	images->os.type = IH_TYPE_STANDALONE;

	image_buf = map_sysmem(images->ep, images->os.image_len);

	/* Run EFI image */
	printf("## Transferring control to EFI (at address %08lx) ...\n",
	       images->ep);
	bootstage_mark(BOOTSTAGE_ID_RUN_OS);

	ret = efi_binary_run(image_buf, images->os.image_len,
			     images->ft_len
			     ? images->ft_addr : EFI_FDT_USE_INTERNAL);

	return ret;
}
#endif

static boot_os_fn *boot_os[] = {
	[IH_OS_U_BOOT] = do_bootm_standalone,
#ifdef CONFIG_BOOTM_LINUX
	[IH_OS_LINUX] = do_bootm_linux,
#endif
#ifdef CONFIG_BOOTM_NETBSD
	[IH_OS_NETBSD] = do_bootm_netbsd,
#endif
#ifdef CONFIG_BOOTM_RTEMS
	[IH_OS_RTEMS] = do_bootm_rtems,
#endif
#if defined(CONFIG_BOOTM_OSE)
	[IH_OS_OSE] = do_bootm_ose,
#endif
#if defined(CONFIG_BOOTM_PLAN9)
	[IH_OS_PLAN9] = do_bootm_plan9,
#endif
#if defined(CONFIG_BOOTM_VXWORKS) && \
	(defined(CONFIG_PPC) || defined(CONFIG_ARM) || defined(CONFIG_RISCV))
	[IH_OS_VXWORKS] = do_bootm_vxworks,
#endif
#if defined(CONFIG_CMD_ELF)
	[IH_OS_QNX] = do_bootm_qnxelf,
#endif
#ifdef CONFIG_INTEGRITY
	[IH_OS_INTEGRITY] = do_bootm_integrity,
#endif
#ifdef CONFIG_BOOTM_OPENRTOS
	[IH_OS_OPENRTOS] = do_bootm_openrtos,
#endif
#ifdef CONFIG_BOOTM_OPTEE
	[IH_OS_TEE] = do_bootm_tee,
#endif
#ifdef CONFIG_BOOTM_EFI
	[IH_OS_EFI] = do_bootm_efi,
#endif
};

/* Allow for arch specific config before we boot */
__weak void arch_preboot_os(void)
{
	/* please define platform specific arch_preboot_os() */
}

/* Allow for board specific config before we boot */
__weak void board_preboot_os(void)
{
	/* please define board specific board_preboot_os() */
}

int boot_selected_os(int state, struct bootm_info *bmi, boot_os_fn *boot_fn)
{
	arch_preboot_os();
	board_preboot_os();

	boot_fn(state, bmi);

	/* Stand-alone may return when 'autostart' is 'no' */
	if (bmi->images->os.type == IH_TYPE_STANDALONE ||
	    IS_ENABLED(CONFIG_SANDBOX) ||
	    state == BOOTM_STATE_OS_FAKE_GO) /* We expect to return */
		return 0;
	bootstage_error(BOOTSTAGE_ID_BOOT_OS_RETURNED);
	debug("\n## Control returned to monitor - resetting...\n");

	return BOOTM_ERR_RESET;
}

boot_os_fn *bootm_os_get_boot_func(int os)
{
	return boot_os[os];
}
