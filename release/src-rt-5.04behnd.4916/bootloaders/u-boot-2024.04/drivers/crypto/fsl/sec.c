// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#if CONFIG_SYS_FSL_SEC_COMPAT == 2 || CONFIG_SYS_FSL_SEC_COMPAT >= 4
#include <fsl_sec.h>
#endif

/*
 * update crypto node properties to a specified revision of the SEC
 * called with sec_rev == 0 if not on an E processor
 */
#if CONFIG_SYS_FSL_SEC_COMPAT == 2 /* SEC 2.x/3.x */
void fdt_fixup_crypto_node(void *blob, int sec_rev)
{
	static const struct sec_rev_prop {
		u32 sec_rev;
		u32 num_channels;
		u32 channel_fifo_len;
		u32 exec_units_mask;
		u32 descriptor_types_mask;
	} sec_rev_prop_list[] = {
		{ 0x0200, 4, 24, 0x07e, 0x01010ebf }, /* SEC 2.0 */
		{ 0x0201, 4, 24, 0x0fe, 0x012b0ebf }, /* SEC 2.1 */
		{ 0x0202, 1, 24, 0x04c, 0x0122003f }, /* SEC 2.2 */
		{ 0x0204, 4, 24, 0x07e, 0x012b0ebf }, /* SEC 2.4 */
		{ 0x0300, 4, 24, 0x9fe, 0x03ab0ebf }, /* SEC 3.0 */
		{ 0x0301, 4, 24, 0xbfe, 0x03ab0ebf }, /* SEC 3.1 */
		{ 0x0303, 4, 24, 0x97c, 0x03a30abf }, /* SEC 3.3 */
	};
	static char compat_strlist[ARRAY_SIZE(sec_rev_prop_list) *
				   sizeof("fsl,secX.Y")];
	int crypto_node, sec_idx, err;
	char *p;
	u32 val;

	/* locate crypto node based on lowest common compatible */
	crypto_node = fdt_node_offset_by_compatible(blob, -1, "fsl,sec2.0");
	if (crypto_node == -FDT_ERR_NOTFOUND)
		return;

	/* delete it if not on an E-processor */
	if (crypto_node > 0 && !sec_rev) {
		fdt_del_node(blob, crypto_node);
		return;
	}

	/* else we got called for possible uprev */
	for (sec_idx = 0; sec_idx < ARRAY_SIZE(sec_rev_prop_list); sec_idx++)
		if (sec_rev_prop_list[sec_idx].sec_rev == sec_rev)
			break;

	if (sec_idx == ARRAY_SIZE(sec_rev_prop_list)) {
		puts("warning: unknown SEC revision number\n");
		return;
	}

	err = fdt_setprop_u32(blob, crypto_node, "fsl,num-channels",
			      sec_rev_prop_list[sec_idx].num_channels);
	if (err < 0)
		printf("WARNING: could not set crypto property: %s\n",
		       fdt_strerror(err));

	err = fdt_setprop_u32(blob, crypto_node, "fsl,descriptor-types-mask",
			      sec_rev_prop_list[sec_idx].descriptor_types_mask);
	if (err < 0)
		printf("WARNING: could not set crypto property: %s\n",
		       fdt_strerror(err));

	err = fdt_setprop_u32(blob, crypto_node, "fsl,exec-units-mask",
			      sec_rev_prop_list[sec_idx].exec_units_mask);
	if (err < 0)
		printf("WARNING: could not set crypto property: %s\n",
		       fdt_strerror(err));

	err = fdt_setprop_u32(blob, crypto_node, "fsl,channel-fifo-len",
			      sec_rev_prop_list[sec_idx].channel_fifo_len);
	if (err < 0)
		printf("WARNING: could not set crypto property: %s\n",
		       fdt_strerror(err));

	val = 0;
	while (sec_idx >= 0) {
		p = compat_strlist + val;
		val += sprintf(p, "fsl,sec%d.%d",
			(sec_rev_prop_list[sec_idx].sec_rev & 0xff00) >> 8,
			sec_rev_prop_list[sec_idx].sec_rev & 0x00ff) + 1;
		sec_idx--;
	}
	err = fdt_setprop(blob, crypto_node, "compatible", &compat_strlist,
			  val);
	if (err < 0)
		printf("WARNING: could not set crypto property: %s\n",
		       fdt_strerror(err));
}
#elif CONFIG_SYS_FSL_SEC_COMPAT >= 4  /* SEC4 */
/**
 * caam_get_era() - fetch the CAAM's era
 *
 * The SEC module povides an "Era" which can be used to differentiate
 * between different revisions.
 *
 * Return: era of the SEC.
 */
u8 caam_get_era(void)
{
	static const struct {
		u16 ip_id;
		u8 maj_rev;
		u8 era;
	} caam_eras[] = {
		{0x0A10, 1, 1},
		{0x0A10, 2, 2},
		{0x0A12, 1, 3},
		{0x0A14, 1, 3},
		{0x0A14, 2, 4},
		{0x0A16, 1, 4},
		{0x0A10, 3, 4},
		{0x0A11, 1, 4},
		{0x0A18, 1, 4},
		{0x0A11, 2, 5},
		{0x0A12, 2, 5},
		{0x0A13, 1, 5},
		{0x0A1C, 1, 5}
	};

	ccsr_sec_t __iomem *sec = (void __iomem *)CFG_SYS_FSL_SEC_ADDR;
	u32 secvid_ms = sec_in32(&sec->secvid_ms);
	u32 ccbvid = sec_in32(&sec->ccbvid);
	u16 ip_id = (secvid_ms & SEC_SECVID_MS_IPID_MASK) >>
				SEC_SECVID_MS_IPID_SHIFT;
	u8 maj_rev = (secvid_ms & SEC_SECVID_MS_MAJ_REV_MASK) >>
				SEC_SECVID_MS_MAJ_REV_SHIFT;
	u8 era = (ccbvid & SEC_CCBVID_ERA_MASK) >> SEC_CCBVID_ERA_SHIFT;

	int i;

	if (era)	/* This is '0' prior to CAAM ERA-6 */
		return era;

	for (i = 0; i < ARRAY_SIZE(caam_eras); i++)
		if (caam_eras[i].ip_id == ip_id &&
		    caam_eras[i].maj_rev == maj_rev)
			return caam_eras[i].era;

	return 0;
}

static void fdt_fixup_crypto_era(void *blob, u32 era)
{
	int err;
	int crypto_node;

	crypto_node = fdt_path_offset(blob, "crypto");
	if (crypto_node < 0) {
		printf("WARNING: Missing crypto node\n");
		return;
	}

	err = fdt_setprop_u32(blob, crypto_node, "fsl,sec-era", era);
	if (err < 0) {
		printf("ERROR: could not set fsl,sec-era property: %s\n",
		       fdt_strerror(err));
	}
}

void fdt_fixup_crypto_node(void *blob, int sec_rev)
{
	u8 era;

	if (!sec_rev) {
		fdt_del_node_and_alias(blob, "crypto");
		return;
	}

	/* Add SEC ERA information in compatible */
	era = caam_get_era();
	if (era) {
		fdt_fixup_crypto_era(blob, era);
	} else {
		printf("WARNING: Unable to get ERA for CAAM rev: %d\n",
		       sec_rev);
	}
}
#endif
