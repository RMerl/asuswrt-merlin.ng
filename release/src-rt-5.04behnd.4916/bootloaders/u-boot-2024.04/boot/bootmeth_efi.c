// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootmethod for distro boot via EFI
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <common.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <command.h>
#include <dm.h>
#include <efi_loader.h>
#include <fs.h>
#include <malloc.h>
#include <mapmem.h>
#include <mmc.h>
#include <net.h>
#include <pxe_utils.h>
#include <linux/sizes.h>

#define EFI_DIRNAME	"efi/boot/"

/**
 * get_efi_leafname() - Get the leaf name for the EFI file we expect
 *
 * @str: Place to put leaf name for this architecture, e.g. "bootaa64.efi".
 *	Must have at least 16 bytes of space
 * @max_len: Length of @str, must be >=16
 */
static int get_efi_leafname(char *str, int max_len)
{
	const char *base;

	if (max_len < 16)
		return log_msg_ret("spc", -ENOSPC);
	if (IS_ENABLED(CONFIG_ARM64))
		base = "bootaa64";
	else if (IS_ENABLED(CONFIG_ARM))
		base = "bootarm";
	else if (IS_ENABLED(CONFIG_X86_RUN_32BIT))
		base = "bootia32";
	else if (IS_ENABLED(CONFIG_X86_RUN_64BIT))
		base = "bootx64";
	else if (IS_ENABLED(CONFIG_ARCH_RV32I))
		base = "bootriscv32";
	else if (IS_ENABLED(CONFIG_ARCH_RV64I))
		base = "bootriscv64";
	else if (IS_ENABLED(CONFIG_SANDBOX))
		base = "bootsbox";
	else
		return -EINVAL;

	strcpy(str, base);
	strcat(str, ".efi");

	return 0;
}

static int get_efi_pxe_arch(void)
{
	/* http://www.iana.org/assignments/dhcpv6-parameters/dhcpv6-parameters.xml */
	if (IS_ENABLED(CONFIG_ARM64))
		return 0xb;
	else if (IS_ENABLED(CONFIG_ARM))
		return 0xa;
	else if (IS_ENABLED(CONFIG_X86_64))
		return 0x6;
	else if (IS_ENABLED(CONFIG_X86))
		return 0x7;
	else if (IS_ENABLED(CONFIG_ARCH_RV32I))
		return 0x19;
	else if (IS_ENABLED(CONFIG_ARCH_RV64I))
		return 0x1b;
	else if (IS_ENABLED(CONFIG_SANDBOX))
		return 0;	/* not used */

	return -EINVAL;
}

static int get_efi_pxe_vci(char *str, int max_len)
{
	int ret;

	ret = get_efi_pxe_arch();
	if (ret < 0)
		return ret;

	snprintf(str, max_len, "PXEClient:Arch:%05x:UNDI:003000", ret);

	return 0;
}

/**
 * bootmeth_uses_network() - check if the media device is Ethernet
 *
 * @bflow: Bootflow to check
 * Returns: true if the media device is Ethernet, else false
 */
static bool bootmeth_uses_network(struct bootflow *bflow)
{
	const struct udevice *media = dev_get_parent(bflow->dev);

	return IS_ENABLED(CONFIG_CMD_DHCP) &&
	    device_get_uclass_id(media) == UCLASS_ETH;
}

static void set_efi_bootdev(struct blk_desc *desc, struct bootflow *bflow)
{
	const struct udevice *media_dev;
	int size = bflow->size;
	const char *dev_name;
	char devnum_str[9];
	char dirname[200];
	char *last_slash;

	/*
	 * This is a horrible hack to tell EFI about this boot device. Once we
	 * unify EFI with the rest of U-Boot we can clean this up. The same hack
	 * exists in multiple places, e.g. in the fs, tftp and load commands.
	 *
	 * Once we can clean up the EFI code to make proper use of driver model,
	 * this can go away.
	 */
	media_dev = dev_get_parent(bflow->dev);
	snprintf(devnum_str, sizeof(devnum_str), "%x:%x",
		 desc ? desc->devnum : dev_seq(media_dev),
		 bflow->part);

	strlcpy(dirname, bflow->fname, sizeof(dirname));
	last_slash = strrchr(dirname, '/');
	if (last_slash)
		*last_slash = '\0';

	log_debug("setting bootdev %s, %s, %s, %p, %x\n",
		  dev_get_uclass_name(media_dev), devnum_str, bflow->fname,
		  bflow->buf, size);
	dev_name = device_get_uclass_id(media_dev) == UCLASS_MASS_STORAGE ?
		 "usb" : dev_get_uclass_name(media_dev);
	efi_set_bootdev(dev_name, devnum_str, bflow->fname, bflow->buf, size);
}

static int efiload_read_file(struct bootflow *bflow, ulong addr)
{
	struct blk_desc *desc = NULL;
	loff_t bytes_read;
	int ret;

	if (bflow->blk)
		 desc = dev_get_uclass_plat(bflow->blk);
	ret = bootmeth_setup_fs(bflow, desc);
	if (ret)
		return log_msg_ret("set", ret);

	ret = fs_read(bflow->fname, addr, 0, bflow->size, &bytes_read);
	if (ret)
		return log_msg_ret("read", ret);
	bflow->buf = map_sysmem(addr, bflow->size);

	set_efi_bootdev(desc, bflow);

	return 0;
}

static int distro_efi_check(struct udevice *dev, struct bootflow_iter *iter)
{
	/* This only works on block and network devices */
	if (bootflow_iter_check_blk(iter) && bootflow_iter_check_net(iter))
		return log_msg_ret("blk", -ENOTSUPP);

	/* This works on block devices and network devices */
	if (iter->method_flags & BOOTFLOW_METHF_PXE_ONLY)
		return log_msg_ret("pxe", -ENOTSUPP);

	return 0;
}

/**
 * distro_efi_get_fdt_name() - Get the filename for reading the .dtb file
 *
 * @fname: Place to put filename
 * @size: Max size of filename
 * @seq: Sequence number, to cycle through options (0=first)
 * Returns: 0 on success, -ENOENT if the "fdtfile" env var does not exist,
 * -EINVAL if there are no more options, -EALREADY if the control FDT should be
 * used
 */
static int distro_efi_get_fdt_name(char *fname, int size, int seq)
{
	const char *fdt_fname;
	const char *prefix;

	/* select the prefix */
	switch (seq) {
	case 0:
		/* this is the default */
		prefix = "/dtb";
		break;
	case 1:
		prefix = "";
		break;
	case 2:
		prefix = "/dtb/current";
		break;
	default:
		return log_msg_ret("pref", -EINVAL);
	}

	fdt_fname = env_get("fdtfile");
	if (fdt_fname) {
		snprintf(fname, size, "%s/%s", prefix, fdt_fname);
		log_debug("Using device tree: %s\n", fname);
	} else if (IS_ENABLED(CONFIG_OF_HAS_PRIOR_STAGE)) {
		strcpy(fname, "<prior>");
		return log_msg_ret("pref", -EALREADY);
	/* Use this fallback only for 32-bit ARM */
	} else if (IS_ENABLED(CONFIG_ARM) && !IS_ENABLED(CONFIG_ARM64)) {
		const char *soc = env_get("soc");
		const char *board = env_get("board");
		const char *boardver = env_get("boardver");

		/* cf the code in label_boot() which seems very complex */
		snprintf(fname, size, "%s/%s%s%s%s.dtb", prefix,
			 soc ? soc : "", soc ? "-" : "", board ? board : "",
			 boardver ? boardver : "");
		log_debug("Using default device tree: %s\n", fname);
	} else {
		return log_msg_ret("env", -ENOENT);
	}

	return 0;
}

/*
 * distro_efi_try_bootflow_files() - Check that files are present
 *
 * This reads any FDT file and checks whether the bootflow file is present, for
 * later reading. We avoid reading the bootflow now, since it is likely large,
 * it may take a long time and we want to avoid needing to allocate memory for
 * it
 *
 * @dev: bootmeth device to use
 * @bflow: bootflow to update
 */
static int distro_efi_try_bootflow_files(struct udevice *dev,
					 struct bootflow *bflow)
{
	struct blk_desc *desc = NULL;
	ulong fdt_addr, size;
	char fname[256];
	int ret, seq;

	/* We require a partition table */
	if (!bflow->part)
		return -ENOENT;

	strcpy(fname, EFI_DIRNAME);
	ret = get_efi_leafname(fname + strlen(fname),
			       sizeof(fname) - strlen(fname));
	if (ret)
		return log_msg_ret("leaf", ret);

	if (bflow->blk)
		 desc = dev_get_uclass_plat(bflow->blk);
	ret = bootmeth_try_file(bflow, desc, NULL, fname);
	if (ret)
		return log_msg_ret("try", ret);

	/* Since we can access the file, let's call it ready */
	bflow->state = BOOTFLOWST_READY;

	fdt_addr = env_get_hex("fdt_addr_r", 0);

	/* try the various available names */
	ret = -ENOENT;
	*fname = '\0';
	for (seq = 0; ret == -ENOENT; seq++) {
		ret = distro_efi_get_fdt_name(fname, sizeof(fname), seq);
		if (ret == -EALREADY)
			bflow->flags = BOOTFLOWF_USE_PRIOR_FDT;
		if (!ret) {
			/* Limit FDT files to 4MB */
			size = SZ_4M;
			ret = bootmeth_common_read_file(dev, bflow, fname,
							fdt_addr, &size);
		}
	}

	if (*fname) {
		bflow->fdt_fname = strdup(fname);
		if (!bflow->fdt_fname)
			return log_msg_ret("fil", -ENOMEM);
	}

	if (!ret) {
		bflow->fdt_size = size;
		bflow->fdt_addr = fdt_addr;

		/*
		 * TODO: Apply extension overlay
		 *
		 * Here we need to load and apply the extension overlay. This is
		 * not implemented. See do_extension_apply(). The extension
		 * stuff needs an implementation in boot/extension.c so it is
		 * separate from the command code. Really the extension stuff
		 * should use the device tree and a uclass / driver interface
		 * rather than implementing its own list
		 */
	} else {
		log_debug("No device tree available\n");
		bflow->flags |= BOOTFLOWF_USE_BUILTIN_FDT;
	}

	return 0;
}

static int distro_efi_read_bootflow_net(struct bootflow *bflow)
{
	char file_addr[17], fname[256];
	char *tftp_argv[] = {"tftp", file_addr, fname, NULL};
	struct cmd_tbl cmdtp = {};	/* dummy */
	const char *addr_str, *fdt_addr_str, *bootfile_name;
	int ret, arch, size;
	ulong addr, fdt_addr;
	char str[36];

	ret = get_efi_pxe_vci(str, sizeof(str));
	if (ret)
		return log_msg_ret("vci", ret);
	ret = get_efi_pxe_arch();
	if (ret < 0)
		return log_msg_ret("arc", ret);
	arch = ret;

	ret = env_set("bootp_vci", str);
	if (ret)
		return log_msg_ret("vcs", ret);
	ret = env_set_hex("bootp_arch", arch);
	if (ret)
		return log_msg_ret("ars", ret);

	/* figure out the load address */
	addr_str = env_get("kernel_addr_r");
	addr = addr_str ? hextoul(addr_str, NULL) : image_load_addr;

	/* clear any previous bootfile */
	env_set("bootfile", NULL);

	/* read the kernel */
	ret = dhcp_run(addr, NULL, true);
	if (ret)
		return log_msg_ret("dhc", ret);

	size = env_get_hex("filesize", -1);
	if (size <= 0)
		return log_msg_ret("sz", -EINVAL);
	bflow->size = size;

    /* bootfile should be setup by dhcp*/
	bootfile_name = env_get("bootfile");
	if (!bootfile_name)
		return log_msg_ret("bootfile_name", ret);
	bflow->fname = strdup(bootfile_name);

	/* do the hideous EFI hack */
	efi_set_bootdev("Net", "", bflow->fname, map_sysmem(addr, 0),
			bflow->size);

	/* read the DT file also */
	fdt_addr_str = env_get("fdt_addr_r");
	if (!fdt_addr_str)
		return log_msg_ret("fdt", -EINVAL);
	fdt_addr = hextoul(fdt_addr_str, NULL);
	sprintf(file_addr, "%lx", fdt_addr);

	/* We only allow the first prefix with PXE */
	ret = distro_efi_get_fdt_name(fname, sizeof(fname), 0);
	if (ret)
		return log_msg_ret("nam", ret);

	bflow->fdt_fname = strdup(fname);
	if (!bflow->fdt_fname)
		return log_msg_ret("fil", -ENOMEM);

	if (!do_tftpb(&cmdtp, 0, 3, tftp_argv)) {
		bflow->fdt_size = env_get_hex("filesize", 0);
		bflow->fdt_addr = fdt_addr;
	} else {
		log_debug("No device tree available\n");
		bflow->flags |= BOOTFLOWF_USE_BUILTIN_FDT;
	}

	bflow->state = BOOTFLOWST_READY;

	return 0;
}

static int distro_efi_read_bootflow(struct udevice *dev, struct bootflow *bflow)
{
	int ret;

	/*
	 * bootmeth_efi doesn't allocate any buffer neither for blk nor net device
	 * set flag to avoid freeing static buffer.
	 */
	bflow->flags |= BOOTFLOWF_STATIC_BUF;

	if (bootmeth_uses_network(bflow)) {
		/* we only support reading from one device, so ignore 'dev' */
		ret = distro_efi_read_bootflow_net(bflow);
		if (ret)
			return log_msg_ret("net", ret);
	} else {
		ret = distro_efi_try_bootflow_files(dev, bflow);
		if (ret)
			return log_msg_ret("blk", ret);
	}

	return 0;
}

static int distro_efi_boot(struct udevice *dev, struct bootflow *bflow)
{
	ulong kernel, fdt;
	int ret;

	kernel = env_get_hex("kernel_addr_r", 0);
	if (!bootmeth_uses_network(bflow)) {
		ret = efiload_read_file(bflow, kernel);
		if (ret)
			return log_msg_ret("read", ret);

		/*
		 * use the provided device tree if not using the built-in fdt
		 */
		if (bflow->flags & ~BOOTFLOWF_USE_BUILTIN_FDT)
			fdt = bflow->fdt_addr;

	} else {
		/*
		 * This doesn't actually work for network devices:
		 *
		 * do_bootefi_image() No UEFI binary known at 0x02080000
		 *
		 * But this is the same behaviour for distro boot, so it can be
		 * fixed here.
		 */
		fdt = env_get_hex("fdt_addr_r", 0);
	}

	if (bflow->flags & BOOTFLOWF_USE_BUILTIN_FDT) {
		log_debug("Booting with built-in fdt\n");
		if (efi_binary_run(map_sysmem(kernel, 0), bflow->size,
				   EFI_FDT_USE_INTERNAL))
			return log_msg_ret("run", -EINVAL);
	} else {
		log_debug("Booting with external fdt\n");
		if (efi_binary_run(map_sysmem(kernel, 0), bflow->size,
				   map_sysmem(fdt, 0)))
			return log_msg_ret("run", -EINVAL);
	}

	return 0;
}

static int distro_bootmeth_efi_bind(struct udevice *dev)
{
	struct bootmeth_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->desc = IS_ENABLED(CONFIG_BOOTSTD_FULL) ?
		"EFI boot from an .efi file" : "EFI";

	return 0;
}

static struct bootmeth_ops distro_efi_bootmeth_ops = {
	.check		= distro_efi_check,
	.read_bootflow	= distro_efi_read_bootflow,
	.read_file	= bootmeth_common_read_file,
	.boot		= distro_efi_boot,
};

static const struct udevice_id distro_efi_bootmeth_ids[] = {
	{ .compatible = "u-boot,distro-efi" },
	{ }
};

U_BOOT_DRIVER(bootmeth_efi) = {
	.name		= "bootmeth_efi",
	.id		= UCLASS_BOOTMETH,
	.of_match	= distro_efi_bootmeth_ids,
	.ops		= &distro_efi_bootmeth_ops,
	.bind		= distro_bootmeth_efi_bind,
};
