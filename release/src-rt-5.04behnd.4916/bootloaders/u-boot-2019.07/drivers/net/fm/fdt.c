// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 */
#include <asm/io.h>
#include <fsl_qe.h>	/* For struct qe_firmware */

#ifdef CONFIG_SYS_DPAA_FMAN
/**
 * fdt_fixup_fman_firmware -- insert the Fman firmware into the device tree
 *
 * The binding for an Fman firmware node is documented in
 * Documentation/powerpc/dts-bindings/fsl/dpaa/fman.txt.  This node contains
 * the actual Fman firmware binary data.  The operating system is expected to
 * be able to parse the binary data to determine any attributes it needs.
 */
void fdt_fixup_fman_firmware(void *blob)
{
	int rc, fmnode, fwnode = -1;
	uint32_t phandle;
	struct qe_firmware *fmanfw;
	const struct qe_header *hdr;
	unsigned int length;
	uint32_t crc;
	const char *p;

	/* The first Fman we find will contain the actual firmware. */
	fmnode = fdt_node_offset_by_compatible(blob, -1, "fsl,fman");
	if (fmnode < 0)
		/* Exit silently if there are no Fman devices */
		return;

	/* If we already have a firmware node, then also exit silently. */
	if (fdt_node_offset_by_compatible(blob, -1, "fsl,fman-firmware") > 0)
		return;

	/* If the environment variable is not set, then exit silently */
	p = env_get("fman_ucode");
	if (!p)
		return;

	fmanfw = (struct qe_firmware *)simple_strtoul(p, NULL, 16);
	if (!fmanfw)
		return;

	hdr = &fmanfw->header;
	length = fdt32_to_cpu(hdr->length);

	/* Verify the firmware. */
	if ((hdr->magic[0] != 'Q') || (hdr->magic[1] != 'E') ||
	    (hdr->magic[2] != 'F')) {
		printf("Data at %p is not an Fman firmware\n", fmanfw);
		return;
	}

	if (length > CONFIG_SYS_QE_FMAN_FW_LENGTH) {
		printf("Fman firmware at %p is too large (size=%u)\n",
		       fmanfw, length);
		return;
	}

	length -= sizeof(u32);	/* Subtract the size of the CRC */
	crc = fdt32_to_cpu(*(u32 *)((void *)fmanfw + length));
	if (crc != crc32_no_comp(0, (void *)fmanfw, length)) {
		printf("Fman firmware at %p has invalid CRC\n", fmanfw);
		return;
	}

	length += sizeof(u32);

	/* Increase the size of the fdt to make room for the node. */
	rc = fdt_increase_size(blob, length);
	if (rc < 0) {
		printf("Unable to make room for Fman firmware: %s\n",
		       fdt_strerror(rc));
		return;
	}

	/* Create the firmware node. */
	fwnode = fdt_add_subnode(blob, fmnode, "fman-firmware");
	if (fwnode < 0) {
		char s[64];
		fdt_get_path(blob, fmnode, s, sizeof(s));
		printf("Could not add firmware node to %s: %s\n", s,
		       fdt_strerror(fwnode));
		return;
	}
	rc = fdt_setprop_string(blob, fwnode, "compatible",
					"fsl,fman-firmware");
	if (rc < 0) {
		char s[64];
		fdt_get_path(blob, fwnode, s, sizeof(s));
		printf("Could not add compatible property to node %s: %s\n", s,
		       fdt_strerror(rc));
		return;
	}
	phandle = fdt_create_phandle(blob, fwnode);
	if (!phandle) {
		char s[64];
		fdt_get_path(blob, fwnode, s, sizeof(s));
		printf("Could not add phandle property to node %s: %s\n", s,
		       fdt_strerror(rc));
		return;
	}
	rc = fdt_setprop(blob, fwnode, "fsl,firmware", fmanfw, length);
	if (rc < 0) {
		char s[64];
		fdt_get_path(blob, fwnode, s, sizeof(s));
		printf("Could not add firmware property to node %s: %s\n", s,
		       fdt_strerror(rc));
		return;
	}

	/* Find all other Fman nodes and point them to the firmware node. */
	while ((fmnode = fdt_node_offset_by_compatible(blob, fmnode,
		"fsl,fman")) > 0) {
		rc = fdt_setprop_cell(blob, fmnode, "fsl,firmware-phandle",
				      phandle);
		if (rc < 0) {
			char s[64];
			fdt_get_path(blob, fmnode, s, sizeof(s));
			printf("Could not add pointer property to node %s: %s\n",
			       s, fdt_strerror(rc));
			return;
		}
	}
}
#endif
