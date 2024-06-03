// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2010,2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * Portions from Coreboot mainboard/google/link/romstage.c
 * Copyright (C) 2007-2010 coresystems GmbH
 * Copyright (C) 2011 Google Inc.
 */

#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <net.h>
#include <rtc.h>
#include <spi.h>
#include <spi_flash.h>
#include <syscon.h>
#include <sysreset.h>
#include <asm/cpu.h>
#include <asm/processor.h>
#include <asm/gpio.h>
#include <asm/global_data.h>
#include <asm/intel_regs.h>
#include <asm/mrccache.h>
#include <asm/mrc_common.h>
#include <asm/mtrr.h>
#include <asm/pci.h>
#include <asm/report_platform.h>
#include <asm/arch/me.h>
#include <asm/arch/pei_data.h>
#include <asm/arch/pch.h>
#include <asm/post.h>
#include <asm/arch/sandybridge.h>

DECLARE_GLOBAL_DATA_PTR;

#define CMOS_OFFSET_MRC_SEED		152
#define CMOS_OFFSET_MRC_SEED_S3		156
#define CMOS_OFFSET_MRC_SEED_CHK	160

ulong board_get_usable_ram_top(ulong total_size)
{
	return mrc_common_board_get_usable_ram_top(total_size);
}

int dram_init_banksize(void)
{
	mrc_common_dram_init_banksize();

	return 0;
}

static int read_seed_from_cmos(struct pei_data *pei_data)
{
	u16 c1, c2, checksum, seed_checksum;
	struct udevice *dev;
	int ret = 0;

	ret = uclass_get_device(UCLASS_RTC, 0, &dev);
	if (ret) {
		debug("Cannot find RTC: err=%d\n", ret);
		return -ENODEV;
	}

	/*
	 * Read scrambler seeds from CMOS RAM. We don't want to store them in
	 * SPI flash since they change on every boot and that would wear down
	 * the flash too much. So we store these in CMOS and the large MRC
	 * data in SPI flash.
	 */
	ret = rtc_read32(dev, CMOS_OFFSET_MRC_SEED, &pei_data->scrambler_seed);
	if (!ret) {
		ret = rtc_read32(dev, CMOS_OFFSET_MRC_SEED_S3,
				 &pei_data->scrambler_seed_s3);
	}
	if (ret) {
		debug("Failed to read from RTC %s\n", dev->name);
		return ret;
	}

	debug("Read scrambler seed    0x%08x from CMOS 0x%02x\n",
	      pei_data->scrambler_seed, CMOS_OFFSET_MRC_SEED);
	debug("Read S3 scrambler seed 0x%08x from CMOS 0x%02x\n",
	      pei_data->scrambler_seed_s3, CMOS_OFFSET_MRC_SEED_S3);

	/* Compute seed checksum and compare */
	c1 = compute_ip_checksum((u8 *)&pei_data->scrambler_seed,
				 sizeof(u32));
	c2 = compute_ip_checksum((u8 *)&pei_data->scrambler_seed_s3,
				 sizeof(u32));
	checksum = add_ip_checksums(sizeof(u32), c1, c2);

	seed_checksum = rtc_read8(dev, CMOS_OFFSET_MRC_SEED_CHK);
	seed_checksum |= rtc_read8(dev, CMOS_OFFSET_MRC_SEED_CHK + 1) << 8;

	if (checksum != seed_checksum) {
		debug("%s: invalid seed checksum\n", __func__);
		pei_data->scrambler_seed = 0;
		pei_data->scrambler_seed_s3 = 0;
		return -EINVAL;
	}

	return 0;
}

static int prepare_mrc_cache(struct pei_data *pei_data)
{
	struct mrc_data_container *mrc_cache;
	struct mrc_region entry;
	int ret;

	ret = read_seed_from_cmos(pei_data);
	if (ret)
		return ret;
	ret = mrccache_get_region(NULL, &entry);
	if (ret)
		return ret;
	mrc_cache = mrccache_find_current(&entry);
	if (!mrc_cache)
		return -ENOENT;

	pei_data->mrc_input = mrc_cache->data;
	pei_data->mrc_input_len = mrc_cache->data_size;
	debug("%s: at %p, size %x checksum %04x\n", __func__,
	      pei_data->mrc_input, pei_data->mrc_input_len,
	      mrc_cache->checksum);

	return 0;
}

static int write_seeds_to_cmos(struct pei_data *pei_data)
{
	u16 c1, c2, checksum;
	struct udevice *dev;
	int ret = 0;

	ret = uclass_get_device(UCLASS_RTC, 0, &dev);
	if (ret) {
		debug("Cannot find RTC: err=%d\n", ret);
		return -ENODEV;
	}

	/* Save the MRC seed values to CMOS */
	rtc_write32(dev, CMOS_OFFSET_MRC_SEED, pei_data->scrambler_seed);
	debug("Save scrambler seed    0x%08x to CMOS 0x%02x\n",
	      pei_data->scrambler_seed, CMOS_OFFSET_MRC_SEED);

	rtc_write32(dev, CMOS_OFFSET_MRC_SEED_S3, pei_data->scrambler_seed_s3);
	debug("Save s3 scrambler seed 0x%08x to CMOS 0x%02x\n",
	      pei_data->scrambler_seed_s3, CMOS_OFFSET_MRC_SEED_S3);

	/* Save a simple checksum of the seed values */
	c1 = compute_ip_checksum((u8 *)&pei_data->scrambler_seed,
				 sizeof(u32));
	c2 = compute_ip_checksum((u8 *)&pei_data->scrambler_seed_s3,
				 sizeof(u32));
	checksum = add_ip_checksums(sizeof(u32), c1, c2);

	rtc_write8(dev, CMOS_OFFSET_MRC_SEED_CHK, checksum & 0xff);
	rtc_write8(dev, CMOS_OFFSET_MRC_SEED_CHK + 1, (checksum >> 8) & 0xff);

	return 0;
}

/* Use this hook to save our SDRAM parameters */
int misc_init_r(void)
{
	int ret;

	ret = mrccache_save();
	if (ret)
		printf("Unable to save MRC data: %d\n", ret);

	return 0;
}

static void post_system_agent_init(struct udevice *dev, struct udevice *me_dev,
				   struct pei_data *pei_data)
{
	uint16_t done;

	/*
	 * Send ME init done for SandyBridge here.  This is done inside the
	 * SystemAgent binary on IvyBridge
	 */
	dm_pci_read_config16(dev, PCI_DEVICE_ID, &done);
	done &= BASE_REV_MASK;
	if (BASE_REV_SNB == done)
		intel_early_me_init_done(dev, me_dev, ME_INIT_STATUS_SUCCESS);
	else
		intel_me_status(me_dev);

	/* If PCIe init is skipped, set the PEG clock gating */
	if (!pei_data->pcie_init)
		setbits_le32(MCHBAR_REG(0x7010), 1);
}

static int recovery_mode_enabled(void)
{
	return false;
}

static int copy_spd(struct udevice *dev, struct pei_data *peid)
{
	const void *data;
	int ret;

	ret = mrc_locate_spd(dev, sizeof(peid->spd_data[0]), &data);
	if (ret) {
		debug("%s: Could not locate SPD (ret=%d)\n", __func__, ret);
		return ret;
	}

	memcpy(peid->spd_data[0], data, sizeof(peid->spd_data[0]));

	return 0;
}

/**
 * sdram_find() - Find available memory
 *
 * This is a bit complicated since on x86 there are system memory holes all
 * over the place. We create a list of available memory blocks
 *
 * @dev:	Northbridge device
 */
static int sdram_find(struct udevice *dev)
{
	struct memory_info *info = &gd->arch.meminfo;
	uint32_t tseg_base, uma_size, tolud;
	uint64_t tom, me_base, touud;
	uint64_t uma_memory_base = 0;
	unsigned long long tomk;
	uint16_t ggc;
	u32 val;

	/* Total Memory 2GB example:
	 *
	 *  00000000  0000MB-1992MB  1992MB  RAM     (writeback)
	 *  7c800000  1992MB-2000MB     8MB  TSEG    (SMRR)
	 *  7d000000  2000MB-2002MB     2MB  GFX GTT (uncached)
	 *  7d200000  2002MB-2034MB    32MB  GFX UMA (uncached)
	 *  7f200000   2034MB TOLUD
	 *  7f800000   2040MB MEBASE
	 *  7f800000  2040MB-2048MB     8MB  ME UMA  (uncached)
	 *  80000000   2048MB TOM
	 * 100000000  4096MB-4102MB     6MB  RAM     (writeback)
	 *
	 * Total Memory 4GB example:
	 *
	 *  00000000  0000MB-2768MB  2768MB  RAM     (writeback)
	 *  ad000000  2768MB-2776MB     8MB  TSEG    (SMRR)
	 *  ad800000  2776MB-2778MB     2MB  GFX GTT (uncached)
	 *  ada00000  2778MB-2810MB    32MB  GFX UMA (uncached)
	 *  afa00000   2810MB TOLUD
	 *  ff800000   4088MB MEBASE
	 *  ff800000  4088MB-4096MB     8MB  ME UMA  (uncached)
	 * 100000000   4096MB TOM
	 * 100000000  4096MB-5374MB  1278MB  RAM     (writeback)
	 * 14fe00000   5368MB TOUUD
	 */

	/* Top of Upper Usable DRAM, including remap */
	dm_pci_read_config32(dev, TOUUD + 4, &val);
	touud = (uint64_t)val << 32;
	dm_pci_read_config32(dev, TOUUD, &val);
	touud |= val;

	/* Top of Lower Usable DRAM */
	dm_pci_read_config32(dev, TOLUD, &tolud);

	/* Top of Memory - does not account for any UMA */
	dm_pci_read_config32(dev, 0xa4, &val);
	tom = (uint64_t)val << 32;
	dm_pci_read_config32(dev, 0xa0, &val);
	tom |= val;

	debug("TOUUD %llx TOLUD %08x TOM %llx\n", touud, tolud, tom);

	/* ME UMA needs excluding if total memory <4GB */
	dm_pci_read_config32(dev, 0x74, &val);
	me_base = (uint64_t)val << 32;
	dm_pci_read_config32(dev, 0x70, &val);
	me_base |= val;

	debug("MEBASE %llx\n", me_base);

	/* TODO: Get rid of all this shifting by 10 bits */
	tomk = tolud >> 10;
	if (me_base == tolud) {
		/* ME is from MEBASE-TOM */
		uma_size = (tom - me_base) >> 10;
		/* Increment TOLUD to account for ME as RAM */
		tolud += uma_size << 10;
		/* UMA starts at old TOLUD */
		uma_memory_base = tomk * 1024ULL;
		debug("ME UMA base %llx size %uM\n", me_base, uma_size >> 10);
	}

	/* Graphics memory comes next */
	dm_pci_read_config16(dev, GGC, &ggc);
	if (!(ggc & 2)) {
		debug("IGD decoded, subtracting ");

		/* Graphics memory */
		uma_size = ((ggc >> 3) & 0x1f) * 32 * 1024ULL;
		debug("%uM UMA", uma_size >> 10);
		tomk -= uma_size;
		uma_memory_base = tomk * 1024ULL;

		/* GTT Graphics Stolen Memory Size (GGMS) */
		uma_size = ((ggc >> 8) & 0x3) * 1024ULL;
		tomk -= uma_size;
		uma_memory_base = tomk * 1024ULL;
		debug(" and %uM GTT\n", uma_size >> 10);
	}

	/* Calculate TSEG size from its base which must be below GTT */
	dm_pci_read_config32(dev, 0xb8, &tseg_base);
	uma_size = (uma_memory_base - tseg_base) >> 10;
	tomk -= uma_size;
	uma_memory_base = tomk * 1024ULL;
	debug("TSEG base 0x%08x size %uM\n", tseg_base, uma_size >> 10);

	debug("Available memory below 4GB: %lluM\n", tomk >> 10);

	/* Report the memory regions */
	mrc_add_memory_area(info, 1 << 20, 2 << 28);
	mrc_add_memory_area(info, (2 << 28) + (2 << 20), 4 << 28);
	mrc_add_memory_area(info, (4 << 28) + (2 << 20), tseg_base);
	mrc_add_memory_area(info, 1ULL << 32, touud);

	/* Add MTRRs for memory */
	mtrr_add_request(MTRR_TYPE_WRBACK, 0, 2ULL << 30);
	mtrr_add_request(MTRR_TYPE_WRBACK, 2ULL << 30, 512 << 20);
	mtrr_add_request(MTRR_TYPE_WRBACK, 0xaULL << 28, 256 << 20);
	mtrr_add_request(MTRR_TYPE_UNCACHEABLE, tseg_base, 16 << 20);
	mtrr_add_request(MTRR_TYPE_UNCACHEABLE, tseg_base + (16 << 20),
			 32 << 20);

	/*
	 * If >= 4GB installed then memory from TOLUD to 4GB
	 * is remapped above TOM, TOUUD will account for both
	 */
	if (touud > (1ULL << 32ULL)) {
		debug("Available memory above 4GB: %lluM\n",
		      (touud >> 20) - 4096);
	}

	return 0;
}

static void rcba_config(void)
{
	/*
	 *             GFX    INTA -> PIRQA (MSI)
	 * D28IP_P3IP  WLAN   INTA -> PIRQB
	 * D29IP_E1P   EHCI1  INTA -> PIRQD
	 * D26IP_E2P   EHCI2  INTA -> PIRQF
	 * D31IP_SIP   SATA   INTA -> PIRQF (MSI)
	 * D31IP_SMIP  SMBUS  INTB -> PIRQH
	 * D31IP_TTIP  THRT   INTC -> PIRQA
	 * D27IP_ZIP   HDA    INTA -> PIRQA (MSI)
	 *
	 * TRACKPAD                -> PIRQE (Edge Triggered)
	 * TOUCHSCREEN             -> PIRQG (Edge Triggered)
	 */

	/* Device interrupt pin register (board specific) */
	writel((INTC << D31IP_TTIP) | (NOINT << D31IP_SIP2) |
	       (INTB << D31IP_SMIP) | (INTA << D31IP_SIP), RCB_REG(D31IP));
	writel(NOINT << D30IP_PIP, RCB_REG(D30IP));
	writel(INTA << D29IP_E1P, RCB_REG(D29IP));
	writel(INTA << D28IP_P3IP, RCB_REG(D28IP));
	writel(INTA << D27IP_ZIP, RCB_REG(D27IP));
	writel(INTA << D26IP_E2P, RCB_REG(D26IP));
	writel(NOINT << D25IP_LIP, RCB_REG(D25IP));
	writel(NOINT << D22IP_MEI1IP, RCB_REG(D22IP));

	/* Device interrupt route registers */
	writel(DIR_ROUTE(PIRQB, PIRQH, PIRQA, PIRQC), RCB_REG(D31IR));
	writel(DIR_ROUTE(PIRQD, PIRQE, PIRQF, PIRQG), RCB_REG(D29IR));
	writel(DIR_ROUTE(PIRQB, PIRQC, PIRQD, PIRQE), RCB_REG(D28IR));
	writel(DIR_ROUTE(PIRQA, PIRQH, PIRQA, PIRQB), RCB_REG(D27IR));
	writel(DIR_ROUTE(PIRQF, PIRQE, PIRQG, PIRQH), RCB_REG(D26IR));
	writel(DIR_ROUTE(PIRQA, PIRQB, PIRQC, PIRQD), RCB_REG(D25IR));
	writel(DIR_ROUTE(PIRQA, PIRQB, PIRQC, PIRQD), RCB_REG(D22IR));

	/* Enable IOAPIC (generic) */
	writew(0x0100, RCB_REG(OIC));
	/* PCH BWG says to read back the IOAPIC enable register */
	(void)readw(RCB_REG(OIC));

	/* Disable unused devices (board specific) */
	setbits_le32(RCB_REG(FD), PCH_DISABLE_ALWAYS);
}

int dram_init(void)
{
	struct pei_data _pei_data __aligned(8) = {
		.pei_version = PEI_VERSION,
		.mchbar = MCH_BASE_ADDRESS,
		.dmibar = DEFAULT_DMIBAR,
		.epbar = DEFAULT_EPBAR,
		.pciexbar = CONFIG_PCIE_ECAM_BASE,
		.smbusbar = SMBUS_IO_BASE,
		.wdbbar = 0x4000000,
		.wdbsize = 0x1000,
		.hpet_address = CONFIG_HPET_ADDRESS,
		.rcba = DEFAULT_RCBABASE,
		.pmbase = DEFAULT_PMBASE,
		.gpiobase = DEFAULT_GPIOBASE,
		.thermalbase = 0xfed08000,
		.system_type = 0, /* 0 Mobile, 1 Desktop/Server */
		.tseg_size = CONFIG_SMM_TSEG_SIZE,
		.ts_addresses = { 0x00, 0x00, 0x00, 0x00 },
		.ec_present = 1,
		.ddr3lv_support = 1,
		/*
		 * 0 = leave channel enabled
		 * 1 = disable dimm 0 on channel
		 * 2 = disable dimm 1 on channel
		 * 3 = disable dimm 0+1 on channel
		 */
		.dimm_channel0_disabled = 2,
		.dimm_channel1_disabled = 2,
		.max_ddr3_freq = 1600,
		.usb_port_config = {
			/*
			 * Empty and onboard Ports 0-7, set to un-used pin
			 * OC3
			 */
			{ 0, 3, 0x0000 }, /* P0= Empty */
			{ 1, 0, 0x0040 }, /* P1= Left USB 1  (OC0) */
			{ 1, 1, 0x0040 }, /* P2= Left USB 2  (OC1) */
			{ 1, 3, 0x0040 }, /* P3= SDCARD      (no OC) */
			{ 0, 3, 0x0000 }, /* P4= Empty */
			{ 1, 3, 0x0040 }, /* P5= WWAN        (no OC) */
			{ 0, 3, 0x0000 }, /* P6= Empty */
			{ 0, 3, 0x0000 }, /* P7= Empty */
			/*
			 * Empty and onboard Ports 8-13, set to un-used pin
			 * OC4
			 */
			{ 1, 4, 0x0040 }, /* P8= Camera      (no OC) */
			{ 1, 4, 0x0040 }, /* P9= Bluetooth   (no OC) */
			{ 0, 4, 0x0000 }, /* P10= Empty */
			{ 0, 4, 0x0000 }, /* P11= Empty */
			{ 0, 4, 0x0000 }, /* P12= Empty */
			{ 0, 4, 0x0000 }, /* P13= Empty */
		},
	};
	struct pei_data *pei_data = &_pei_data;
	struct udevice *dev, *me_dev;
	int ret;

	/* We need the pinctrl set up early */
	ret = syscon_get_by_driver_data(X86_SYSCON_PINCONF, &dev);
	if (ret) {
		debug("%s: Could not get pinconf (ret=%d)\n", __func__, ret);
		return ret;
	}

	ret = uclass_first_device_err(UCLASS_NORTHBRIDGE, &dev);
	if (ret) {
		debug("%s: Could not get northbridge (ret=%d)\n", __func__,
		      ret);
		return ret;
	}
	ret = syscon_get_by_driver_data(X86_SYSCON_ME, &me_dev);
	if (ret) {
		debug("%s: Could not get ME (ret=%d)\n", __func__, ret);
		return ret;
	}
	ret = copy_spd(dev, pei_data);
	if (ret) {
		debug("%s: Could not get SPD (ret=%d)\n", __func__, ret);
		return ret;
	}
	pei_data->boot_mode = gd->arch.pei_boot_mode;
	debug("Boot mode %d\n", gd->arch.pei_boot_mode);
	debug("mrc_input %p\n", pei_data->mrc_input);

	/*
	 * Do not pass MRC data in for recovery mode boot,
	 * Always pass it in for S3 resume.
	 */
	if (!recovery_mode_enabled() ||
	    pei_data->boot_mode == PEI_BOOT_RESUME) {
		ret = prepare_mrc_cache(pei_data);
		if (ret)
			debug("prepare_mrc_cache failed: %d\n", ret);
	}

	/* If MRC data is not found we cannot continue S3 resume. */
	if (pei_data->boot_mode == PEI_BOOT_RESUME && !pei_data->mrc_input) {
		debug("Giving up in sdram_initialize: No MRC data\n");
		sysreset_walk_halt(SYSRESET_COLD);
	}

	/* Pass console handler in pei_data */
	pei_data->tx_byte = sdram_console_tx_byte;

	/* Wait for ME to be ready */
	ret = intel_early_me_init(me_dev);
	if (ret) {
		debug("%s: Could not init ME (ret=%d)\n", __func__, ret);
		return ret;
	}
	ret = intel_early_me_uma_size(me_dev);
	if (ret < 0) {
		debug("%s: Could not get UMA size (ret=%d)\n", __func__, ret);
		return ret;
	}

	ret = mrc_common_init(dev, pei_data, false);
	if (ret) {
		debug("%s: mrc_common_init() failed (ret=%d)\n", __func__, ret);
		return ret;
	}

	ret = sdram_find(dev);
	if (ret) {
		debug("%s: sdram_find() failed (ret=%d)\n", __func__, ret);
		return ret;
	}
	gd->ram_size = gd->arch.meminfo.total_32bit_memory;

	debug("MRC output data length %#x at %p\n", pei_data->mrc_output_len,
	      pei_data->mrc_output);

	post_system_agent_init(dev, me_dev, pei_data);
	report_memory_config();

	/* S3 resume: don't save scrambler seed or MRC data */
	if (pei_data->boot_mode != PEI_BOOT_RESUME) {
		/*
		 * This will be copied to SDRAM in reserve_arch(), then written
		 * to SPI flash in mrccache_save()
		 */
		gd->arch.mrc_output = (char *)pei_data->mrc_output;
		gd->arch.mrc_output_len = pei_data->mrc_output_len;
		ret = write_seeds_to_cmos(pei_data);
		if (ret)
			debug("Failed to write seeds to CMOS: %d\n", ret);
	}

	writew(0xCAFE, MCHBAR_REG(SSKPD));
	if (ret)
		return ret;

	rcba_config();

	return 0;
}
