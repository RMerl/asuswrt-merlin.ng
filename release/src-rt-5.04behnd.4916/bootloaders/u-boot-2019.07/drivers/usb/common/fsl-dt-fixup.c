// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009, 2011 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2008, Excito Elektronik i Sk=E5ne AB
 *
 * Author: Tor Krill tor@excito.com
 */

#include <common.h>
#include <usb.h>
#include <asm/io.h>
#include <hwconfig.h>
#include <fsl_errata.h>
#include <fsl_usb.h>
#include <fdt_support.h>

#ifndef CONFIG_USB_MAX_CONTROLLER_COUNT
#define CONFIG_USB_MAX_CONTROLLER_COUNT 1
#endif

/* USB Controllers */
#define FSL_USB2_MPH	"fsl-usb2-mph"
#define FSL_USB2_DR	"fsl-usb2-dr"
#define CHIPIDEA_USB2	"chipidea,usb2"
#define SNPS_DWC3	"snps,dwc3"

static const char * const compat_usb_fsl[] = {
	FSL_USB2_MPH,
	FSL_USB2_DR,
	SNPS_DWC3,
	NULL
};

static int fdt_usb_get_node_type(void *blob, int start_offset,
				 int *node_offset, const char **node_type)
{
	int i;
	int ret = -ENOENT;

	for (i = 0; compat_usb_fsl[i]; i++) {
		*node_offset = fdt_node_offset_by_compatible
					(blob, start_offset,
					 compat_usb_fsl[i]);
		if (*node_offset >= 0) {
			*node_type = compat_usb_fsl[i];
			ret = 0;
			break;
		}
	}

	return ret;
}

static int fdt_fixup_usb_mode_phy_type(void *blob, const char *mode,
				       const char *phy_type, int start_offset)
{
	const char *prop_mode = "dr_mode";
	const char *prop_type = "phy_type";
	const char *node_type = NULL;
	int node_offset;
	int err;

	err = fdt_usb_get_node_type(blob, start_offset,
				    &node_offset, &node_type);
	if (err < 0)
		return err;

	if (mode) {
		err = fdt_setprop(blob, node_offset, prop_mode, mode,
				  strlen(mode) + 1);
		if (err < 0)
			printf("WARNING: could not set %s for %s: %s.\n",
			       prop_mode, node_type, fdt_strerror(err));
	}

	if (phy_type) {
		err = fdt_setprop(blob, node_offset, prop_type, phy_type,
				  strlen(phy_type) + 1);
		if (err < 0)
			printf("WARNING: could not set %s for %s: %s.\n",
			       prop_type, node_type, fdt_strerror(err));
	}

	return node_offset;
}

static int fsl_fdt_fixup_usb_erratum(void *blob, const char *prop_erratum,
				     const char *controller_type,
				     int start_offset)
{
	int node_offset, err;
	const char *node_type = NULL;
	const char *node_name = NULL;

	err = fdt_usb_get_node_type(blob, start_offset,
				    &node_offset, &node_type);
	if (err < 0)
		return err;

	if (!strcmp(node_type, FSL_USB2_MPH) || !strcmp(node_type, FSL_USB2_DR))
		node_name = CHIPIDEA_USB2;
	else
		node_name = node_type;
	if (strcmp(node_name, controller_type))
		return err;

	err = fdt_setprop(blob, node_offset, prop_erratum, NULL, 0);
	if (err < 0) {
		printf("ERROR: could not set %s for %s: %s.\n",
		       prop_erratum, node_type, fdt_strerror(err));
	}

	return node_offset;
}

static int fsl_fdt_fixup_erratum(int *usb_erratum_off, void *blob,
				 const char *controller_type, char *str,
				 bool (*has_erratum)(void))
{
	char buf[32] = {0};

	snprintf(buf, sizeof(buf), "fsl,usb-erratum-%s", str);
	if (!has_erratum())
		return -EINVAL;
	*usb_erratum_off = fsl_fdt_fixup_usb_erratum(blob, buf, controller_type,
						     *usb_erratum_off);
	if (*usb_erratum_off < 0)
		return -ENOSPC;
	debug("Adding USB erratum %s\n", str);
	return 0;
}

void fsl_fdt_fixup_dr_usb(void *blob, bd_t *bd)
{
	static const char * const modes[] = { "host", "peripheral", "otg" };
	static const char * const phys[] = { "ulpi", "utmi", "utmi_dual" };
	int usb_erratum_a006261_off = -1;
	int usb_erratum_a007075_off = -1;
	int usb_erratum_a007792_off = -1;
	int usb_erratum_a005697_off = -1;
	int usb_erratum_a008751_off = -1;
	int usb_mode_off = -1;
	int usb_phy_off = -1;
	char str[5];
	int i, j;
	int ret;

	for (i = 1; i <= CONFIG_USB_MAX_CONTROLLER_COUNT; i++) {
		const char *dr_mode_type = NULL;
		const char *dr_phy_type = NULL;
		int mode_idx = -1, phy_idx = -1;

		snprintf(str, 5, "%s%d", "usb", i);
		if (hwconfig(str)) {
			for (j = 0; j < ARRAY_SIZE(modes); j++) {
				if (hwconfig_subarg_cmp(str, "dr_mode",
							modes[j])) {
					mode_idx = j;
					break;
				}
			}

			for (j = 0; j < ARRAY_SIZE(phys); j++) {
				if (hwconfig_subarg_cmp(str, "phy_type",
							phys[j])) {
					phy_idx = j;
					break;
				}
			}

			if (mode_idx < 0 && phy_idx < 0) {
				printf("WARNING: invalid phy or mode\n");
				return;
			}

			if (mode_idx > -1)
				dr_mode_type = modes[mode_idx];

			if (phy_idx > -1)
				dr_phy_type = phys[phy_idx];
		}

		if (has_dual_phy())
			dr_phy_type = phys[2];

		usb_mode_off = fdt_fixup_usb_mode_phy_type(blob,
							   dr_mode_type, NULL,
							   usb_mode_off);

		if (usb_mode_off < 0)
			return;

		usb_phy_off = fdt_fixup_usb_mode_phy_type(blob,
							  NULL, dr_phy_type,
							  usb_phy_off);

		if (usb_phy_off < 0)
			return;

		ret = fsl_fdt_fixup_erratum(&usb_erratum_a006261_off, blob,
					    CHIPIDEA_USB2, "a006261",
					    has_erratum_a006261);
		if (ret == -ENOSPC)
			return;
		ret = fsl_fdt_fixup_erratum(&usb_erratum_a007075_off, blob,
					    CHIPIDEA_USB2, "a007075",
					    has_erratum_a007075);
		if (ret == -ENOSPC)
			return;
		ret = fsl_fdt_fixup_erratum(&usb_erratum_a007792_off, blob,
					    CHIPIDEA_USB2, "a007792",
					    has_erratum_a007792);
		if (ret == -ENOSPC)
			return;
		ret = fsl_fdt_fixup_erratum(&usb_erratum_a005697_off, blob,
					    CHIPIDEA_USB2, "a005697",
					    has_erratum_a005697);
		if (ret == -ENOSPC)
			return;
		ret = fsl_fdt_fixup_erratum(&usb_erratum_a008751_off, blob,
					    SNPS_DWC3, "a008751",
					    has_erratum_a008751);
		if (ret == -ENOSPC)
			return;

	}
}
