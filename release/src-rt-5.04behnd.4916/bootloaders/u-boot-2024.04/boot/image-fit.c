// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013, Google Inc.
 *
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#define LOG_CATEGORY LOGC_BOOT

#ifdef USE_HOSTCC
#include "mkimage.h"
#include <time.h>
#include <linux/libfdt.h>
#include <u-boot/crc.h>
#include <linux/kconfig.h>
#else
#include <linux/compiler.h>
#include <linux/sizes.h>
#include <common.h>
#include <errno.h>
#include <log.h>
#include <mapmem.h>
#include <asm/io.h>
#include <malloc.h>
#include <memalign.h>
#include <asm/global_data.h>
#ifdef CONFIG_DM_HASH
#include <dm.h>
#include <u-boot/hash.h>
#endif
DECLARE_GLOBAL_DATA_PTR;
#endif /* !USE_HOSTCC*/

#include <bootm.h>
#include <image.h>
#include <bootstage.h>
#include <u-boot/crc.h>
#include <u-boot/md5.h>
#include <u-boot/sha1.h>
#include <u-boot/sha256.h>
#include <u-boot/sha512.h>

/*****************************************************************************/
/* New uImage format routines */
/*****************************************************************************/
#ifndef USE_HOSTCC
static int fit_parse_spec(const char *spec, char sepc, ulong addr_curr,
		ulong *addr, const char **name)
{
	const char *sep;

	*addr = addr_curr;
	*name = NULL;

	sep = strchr(spec, sepc);
	if (sep) {
		if (sep - spec > 0)
			*addr = hextoul(spec, NULL);

		*name = sep + 1;
		return 1;
	}

	return 0;
}

/**
 * fit_parse_conf - parse FIT configuration spec
 * @spec: input string, containing configuration spec
 * @add_curr: current image address (to be used as a possible default)
 * @addr: pointer to a ulong variable, will hold FIT image address of a given
 * configuration
 * @conf_name double pointer to a char, will hold pointer to a configuration
 * unit name
 *
 * fit_parse_conf() expects configuration spec in the form of [<addr>]#<conf>,
 * where <addr> is a FIT image address that contains configuration
 * with a <conf> unit name.
 *
 * Address part is optional, and if omitted default add_curr will
 * be used instead.
 *
 * returns:
 *     1 if spec is a valid configuration string,
 *     addr and conf_name are set accordingly
 *     0 otherwise
 */
int fit_parse_conf(const char *spec, ulong addr_curr,
		ulong *addr, const char **conf_name)
{
	return fit_parse_spec(spec, '#', addr_curr, addr, conf_name);
}

/**
 * fit_parse_subimage - parse FIT subimage spec
 * @spec: input string, containing subimage spec
 * @add_curr: current image address (to be used as a possible default)
 * @addr: pointer to a ulong variable, will hold FIT image address of a given
 * subimage
 * @image_name: double pointer to a char, will hold pointer to a subimage name
 *
 * fit_parse_subimage() expects subimage spec in the form of
 * [<addr>]:<subimage>, where <addr> is a FIT image address that contains
 * subimage with a <subimg> unit name.
 *
 * Address part is optional, and if omitted default add_curr will
 * be used instead.
 *
 * returns:
 *     1 if spec is a valid subimage string,
 *     addr and image_name are set accordingly
 *     0 otherwise
 */
int fit_parse_subimage(const char *spec, ulong addr_curr,
		ulong *addr, const char **image_name)
{
	return fit_parse_spec(spec, ':', addr_curr, addr, image_name);
}
#endif /* !USE_HOSTCC */

#ifdef USE_HOSTCC
/* Host tools use these implementations for Cipher and Signature support */
static void *host_blob;

void image_set_host_blob(void *blob)
{
	host_blob = blob;
}

void *image_get_host_blob(void)
{
	return host_blob;
}
#endif /* USE_HOSTCC */

static void fit_get_debug(const void *fit, int noffset,
		char *prop_name, int err)
{
	debug("Can't get '%s' property from FIT 0x%08lx, node: offset %d, name %s (%s)\n",
	      prop_name, (ulong)fit, noffset, fit_get_name(fit, noffset, NULL),
	      fdt_strerror(err));
}

/**
 * fit_get_subimage_count - get component (sub-image) count
 * @fit: pointer to the FIT format image header
 * @images_noffset: offset of images node
 *
 * returns:
 *     number of image components
 */
int fit_get_subimage_count(const void *fit, int images_noffset)
{
	int noffset;
	int ndepth;
	int count = 0;

	/* Process its subnodes, print out component images details */
	for (ndepth = 0, count = 0,
		noffset = fdt_next_node(fit, images_noffset, &ndepth);
	     (noffset >= 0) && (ndepth > 0);
	     noffset = fdt_next_node(fit, noffset, &ndepth)) {
		if (ndepth == 1) {
			count++;
		}
	}

	return count;
}

#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_FIT_PRINT) || defined(CONFIG_TPL_FIT_PRINT)
/**
 * fit_image_print_data() - prints out the hash node details
 * @fit: pointer to the FIT format image header
 * @noffset: offset of the hash node
 * @p: pointer to prefix string
 * @type: Type of information to print ("hash" or "sign")
 *
 * fit_image_print_data() lists properties for the processed hash node
 *
 * This function avoid using puts() since it prints a newline on the host
 * but does not in U-Boot.
 *
 * returns:
 *     no returned results
 */
static void fit_image_print_data(const void *fit, int noffset, const char *p,
				 const char *type)
{
	const char *keyname;
	uint8_t *value;
	int value_len;
	const char *algo;
	const char *padding;
	bool required;
	int ret, i;

	debug("%s  %s node:    '%s'\n", p, type,
	      fit_get_name(fit, noffset, NULL));
	printf("%s  %s algo:    ", p, type);
	if (fit_image_hash_get_algo(fit, noffset, &algo)) {
		printf("invalid/unsupported\n");
		return;
	}
	printf("%s", algo);
	keyname = fdt_getprop(fit, noffset, FIT_KEY_HINT, NULL);
	required = fdt_getprop(fit, noffset, FIT_KEY_REQUIRED, NULL) != NULL;
	if (keyname)
		printf(":%s", keyname);
	if (required)
		printf(" (required)");
	printf("\n");

	padding = fdt_getprop(fit, noffset, "padding", NULL);
	if (padding)
		printf("%s  %s padding: %s\n", p, type, padding);

	ret = fit_image_hash_get_value(fit, noffset, &value,
				       &value_len);
	printf("%s  %s value:   ", p, type);
	if (ret) {
		printf("unavailable\n");
	} else {
		for (i = 0; i < value_len; i++)
			printf("%02x", value[i]);
		printf("\n");
	}

	debug("%s  %s len:     %d\n", p, type, value_len);

	/* Signatures have a time stamp */
	if (IMAGE_ENABLE_TIMESTAMP && keyname) {
		time_t timestamp;

		printf("%s  Timestamp:    ", p);
		if (fit_get_timestamp(fit, noffset, &timestamp))
			printf("unavailable\n");
		else
			genimg_print_time(timestamp);
	}
}

/**
 * fit_image_print_verification_data() - prints out the hash/signature details
 * @fit: pointer to the FIT format image header
 * @noffset: offset of the hash or signature node
 * @p: pointer to prefix string
 *
 * This lists properties for the processed hash node
 *
 * returns:
 *     no returned results
 */
static void fit_image_print_verification_data(const void *fit, int noffset,
					      const char *p)
{
	const char *name;

	/*
	 * Check subnode name, must be equal to "hash" or "signature".
	 * Multiple hash/signature nodes require unique unit node
	 * names, e.g. hash-1, hash-2, signature-1, signature-2, etc.
	 */
	name = fit_get_name(fit, noffset, NULL);
	if (!strncmp(name, FIT_HASH_NODENAME, strlen(FIT_HASH_NODENAME))) {
		fit_image_print_data(fit, noffset, p, "Hash");
	} else if (!strncmp(name, FIT_SIG_NODENAME,
				strlen(FIT_SIG_NODENAME))) {
		fit_image_print_data(fit, noffset, p, "Sign");
	}
}

/**
 * fit_conf_print - prints out the FIT configuration details
 * @fit: pointer to the FIT format image header
 * @noffset: offset of the configuration node
 * @p: pointer to prefix string
 *
 * fit_conf_print() lists all mandatory properties for the processed
 * configuration node.
 *
 * returns:
 *     no returned results
 */
static void fit_conf_print(const void *fit, int noffset, const char *p)
{
	char *desc;
	const char *uname;
	int ret;
	int fdt_index, loadables_index;
	int ndepth;

	/* Mandatory properties */
	ret = fit_get_desc(fit, noffset, &desc);
	printf("%s  Description:  ", p);
	if (ret)
		printf("unavailable\n");
	else
		printf("%s\n", desc);

	uname = fdt_getprop(fit, noffset, FIT_KERNEL_PROP, NULL);
	printf("%s  Kernel:       ", p);
	if (!uname)
		printf("unavailable\n");
	else
		printf("%s\n", uname);

	/* Optional properties */
	uname = fdt_getprop(fit, noffset, FIT_RAMDISK_PROP, NULL);
	if (uname)
		printf("%s  Init Ramdisk: %s\n", p, uname);

	uname = fdt_getprop(fit, noffset, FIT_FIRMWARE_PROP, NULL);
	if (uname)
		printf("%s  Firmware:     %s\n", p, uname);

	for (fdt_index = 0;
	     uname = fdt_stringlist_get(fit, noffset, FIT_FDT_PROP,
					fdt_index, NULL), uname;
	     fdt_index++) {
		if (fdt_index == 0)
			printf("%s  FDT:          ", p);
		else
			printf("%s                ", p);
		printf("%s\n", uname);
	}

	uname = fdt_getprop(fit, noffset, FIT_FPGA_PROP, NULL);
	if (uname)
		printf("%s  FPGA:         %s\n", p, uname);

	/* Print out all of the specified loadables */
	for (loadables_index = 0;
	     uname = fdt_stringlist_get(fit, noffset, FIT_LOADABLE_PROP,
					loadables_index, NULL), uname;
	     loadables_index++) {
		if (loadables_index == 0) {
			printf("%s  Loadables:    ", p);
		} else {
			printf("%s                ", p);
		}
		printf("%s\n", uname);
	}

	/* Process all hash subnodes of the component configuration node */
	for (ndepth = 0, noffset = fdt_next_node(fit, noffset, &ndepth);
	     (noffset >= 0) && (ndepth > 0);
	     noffset = fdt_next_node(fit, noffset, &ndepth)) {
		if (ndepth == 1) {
			/* Direct child node of the component configuration node */
			fit_image_print_verification_data(fit, noffset, p);
		}
	}
}

/**
 * fit_print_contents - prints out the contents of the FIT format image
 * @fit: pointer to the FIT format image header
 * @p: pointer to prefix string
 *
 * fit_print_contents() formats a multi line FIT image contents description.
 * The routine prints out FIT image properties (root node level) followed by
 * the details of each component image.
 *
 * returns:
 *     no returned results
 */
void fit_print_contents(const void *fit)
{
	char *desc;
	char *uname;
	int images_noffset;
	int confs_noffset;
	int noffset;
	int ndepth;
	int count = 0;
	int ret;
	const char *p;
	time_t timestamp;

	if (!CONFIG_IS_ENABLED(FIT_PRINT))
		return;

	/* Indent string is defined in header image.h */
	p = IMAGE_INDENT_STRING;

	/* Root node properties */
	ret = fit_get_desc(fit, 0, &desc);
	printf("%sFIT description: ", p);
	if (ret)
		printf("unavailable\n");
	else
		printf("%s\n", desc);

	if (IMAGE_ENABLE_TIMESTAMP) {
		ret = fit_get_timestamp(fit, 0, &timestamp);
		printf("%sCreated:         ", p);
		if (ret)
			printf("unavailable\n");
		else
			genimg_print_time(timestamp);
	}

	/* Find images parent node offset */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images_noffset < 0) {
		printf("Can't find images parent node '%s' (%s)\n",
		       FIT_IMAGES_PATH, fdt_strerror(images_noffset));
		return;
	}

	/* Process its subnodes, print out component images details */
	for (ndepth = 0, count = 0,
		noffset = fdt_next_node(fit, images_noffset, &ndepth);
	     (noffset >= 0) && (ndepth > 0);
	     noffset = fdt_next_node(fit, noffset, &ndepth)) {
		if (ndepth == 1) {
			/*
			 * Direct child node of the images parent node,
			 * i.e. component image node.
			 */
			printf("%s Image %u (%s)\n", p, count++,
			       fit_get_name(fit, noffset, NULL));

			fit_image_print(fit, noffset, p);
		}
	}

	/* Find configurations parent node offset */
	confs_noffset = fdt_path_offset(fit, FIT_CONFS_PATH);
	if (confs_noffset < 0) {
		debug("Can't get configurations parent node '%s' (%s)\n",
		      FIT_CONFS_PATH, fdt_strerror(confs_noffset));
		return;
	}

	/* get default configuration unit name from default property */
	uname = (char *)fdt_getprop(fit, noffset, FIT_DEFAULT_PROP, NULL);
	if (uname)
		printf("%s Default Configuration: '%s'\n", p, uname);

	/* Process its subnodes, print out configurations details */
	for (ndepth = 0, count = 0,
		noffset = fdt_next_node(fit, confs_noffset, &ndepth);
	     (noffset >= 0) && (ndepth > 0);
	     noffset = fdt_next_node(fit, noffset, &ndepth)) {
		if (ndepth == 1) {
			/*
			 * Direct child node of the configurations parent node,
			 * i.e. configuration node.
			 */
			printf("%s Configuration %u (%s)\n", p, count++,
			       fit_get_name(fit, noffset, NULL));

			fit_conf_print(fit, noffset, p);
		}
	}
}

/**
 * fit_image_print - prints out the FIT component image details
 * @fit: pointer to the FIT format image header
 * @image_noffset: offset of the component image node
 * @p: pointer to prefix string
 *
 * fit_image_print() lists all mandatory properties for the processed component
 * image. If present, hash nodes are printed out as well. Load
 * address for images of type firmware is also printed out. Since the load
 * address is not mandatory for firmware images, it will be output as
 * "unavailable" when not present.
 *
 * returns:
 *     no returned results
 */
void fit_image_print(const void *fit, int image_noffset, const char *p)
{
	char *desc;
	uint8_t type, arch, os, comp = IH_COMP_NONE;
	size_t size;
	ulong load, entry;
	const void *data;
	int noffset;
	int ndepth;
	int ret;

	if (!CONFIG_IS_ENABLED(FIT_PRINT))
		return;

	/* Mandatory properties */
	ret = fit_get_desc(fit, image_noffset, &desc);
	printf("%s  Description:  ", p);
	if (ret)
		printf("unavailable\n");
	else
		printf("%s\n", desc);

	if (IMAGE_ENABLE_TIMESTAMP) {
		time_t timestamp;

		ret = fit_get_timestamp(fit, 0, &timestamp);
		printf("%s  Created:      ", p);
		if (ret)
			printf("unavailable\n");
		else
			genimg_print_time(timestamp);
	}

	fit_image_get_type(fit, image_noffset, &type);
	printf("%s  Type:         %s\n", p, genimg_get_type_name(type));

	fit_image_get_comp(fit, image_noffset, &comp);
	printf("%s  Compression:  %s\n", p, genimg_get_comp_name(comp));

	ret = fit_image_get_data_and_size(fit, image_noffset, &data, &size);

	if (!tools_build()) {
		printf("%s  Data Start:   ", p);
		if (ret) {
			printf("unavailable\n");
		} else {
			void *vdata = (void *)data;

			printf("0x%08lx\n", (ulong)map_to_sysmem(vdata));
		}
	}

	printf("%s  Data Size:    ", p);
	if (ret)
		printf("unavailable\n");
	else
		genimg_print_size(size);

	/* Remaining, type dependent properties */
	if ((type == IH_TYPE_KERNEL) || (type == IH_TYPE_STANDALONE) ||
	    (type == IH_TYPE_RAMDISK) || (type == IH_TYPE_FIRMWARE) ||
	    (type == IH_TYPE_FLATDT)) {
		fit_image_get_arch(fit, image_noffset, &arch);
		printf("%s  Architecture: %s\n", p, genimg_get_arch_name(arch));
	}

	if ((type == IH_TYPE_KERNEL) || (type == IH_TYPE_RAMDISK) ||
	    (type == IH_TYPE_FIRMWARE)) {
		fit_image_get_os(fit, image_noffset, &os);
		printf("%s  OS:           %s\n", p, genimg_get_os_name(os));
	}

	if ((type == IH_TYPE_KERNEL) || (type == IH_TYPE_STANDALONE) ||
	    (type == IH_TYPE_FIRMWARE) || (type == IH_TYPE_RAMDISK) ||
	    (type == IH_TYPE_FPGA)) {
		ret = fit_image_get_load(fit, image_noffset, &load);
		printf("%s  Load Address: ", p);
		if (ret)
			printf("unavailable\n");
		else
			printf("0x%08lx\n", load);
	}

	/* optional load address for FDT */
	if (type == IH_TYPE_FLATDT && !fit_image_get_load(fit, image_noffset, &load))
		printf("%s  Load Address: 0x%08lx\n", p, load);

	if ((type == IH_TYPE_KERNEL) || (type == IH_TYPE_STANDALONE) ||
	    (type == IH_TYPE_RAMDISK)) {
		ret = fit_image_get_entry(fit, image_noffset, &entry);
		printf("%s  Entry Point:  ", p);
		if (ret)
			printf("unavailable\n");
		else
			printf("0x%08lx\n", entry);
	}

	/* Process all hash subnodes of the component image node */
	for (ndepth = 0, noffset = fdt_next_node(fit, image_noffset, &ndepth);
	     (noffset >= 0) && (ndepth > 0);
	     noffset = fdt_next_node(fit, noffset, &ndepth)) {
		if (ndepth == 1) {
			/* Direct child node of the component image node */
			fit_image_print_verification_data(fit, noffset, p);
		}
	}
}
#endif /* !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_FIT_PRINT) || defined(CONFIG_TPL_FIT_PRINT) */

/**
 * fit_get_desc - get node description property
 * @fit: pointer to the FIT format image header
 * @noffset: node offset
 * @desc: double pointer to the char, will hold pointer to the description
 *
 * fit_get_desc() reads description property from a given node, if
 * description is found pointer to it is returned in third call argument.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_get_desc(const void *fit, int noffset, char **desc)
{
	int len;

	*desc = (char *)fdt_getprop(fit, noffset, FIT_DESC_PROP, &len);
	if (*desc == NULL) {
		fit_get_debug(fit, noffset, FIT_DESC_PROP, len);
		return -1;
	}

	return 0;
}

/**
 * fit_get_timestamp - get node timestamp property
 * @fit: pointer to the FIT format image header
 * @noffset: node offset
 * @timestamp: pointer to the time_t, will hold read timestamp
 *
 * fit_get_timestamp() reads timestamp property from given node, if timestamp
 * is found and has a correct size its value is returned in third call
 * argument.
 *
 * returns:
 *     0, on success
 *     -1, on property read failure
 *     -2, on wrong timestamp size
 */
int fit_get_timestamp(const void *fit, int noffset, time_t *timestamp)
{
	int len;
	const void *data;

	data = fdt_getprop(fit, noffset, FIT_TIMESTAMP_PROP, &len);
	if (data == NULL) {
		fit_get_debug(fit, noffset, FIT_TIMESTAMP_PROP, len);
		return -1;
	}
	if (len != sizeof(uint32_t)) {
		debug("FIT timestamp with incorrect size of (%u)\n", len);
		return -2;
	}

	*timestamp = uimage_to_cpu(*((uint32_t *)data));
	return 0;
}

/**
 * fit_image_get_node - get node offset for component image of a given unit name
 * @fit: pointer to the FIT format image header
 * @image_uname: component image node unit name
 *
 * fit_image_get_node() finds a component image (within the '/images'
 * node) of a provided unit name. If image is found its node offset is
 * returned to the caller.
 *
 * returns:
 *     image node offset when found (>=0)
 *     negative number on failure (FDT_ERR_* code)
 */
int fit_image_get_node(const void *fit, const char *image_uname)
{
	int noffset, images_noffset;

	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images_noffset < 0) {
		debug("Can't find images parent node '%s' (%s)\n",
		      FIT_IMAGES_PATH, fdt_strerror(images_noffset));
		return images_noffset;
	}

	noffset = fdt_subnode_offset(fit, images_noffset, image_uname);
	if (noffset < 0) {
		debug("Can't get node offset for image unit name: '%s' (%s)\n",
		      image_uname, fdt_strerror(noffset));
	}

	return noffset;
}

/**
 * fit_image_get_os - get os id for a given component image node
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @os: pointer to the uint8_t, will hold os numeric id
 *
 * fit_image_get_os() finds os property in a given component image node.
 * If the property is found, its (string) value is translated to the numeric
 * id which is returned to the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_get_os(const void *fit, int noffset, uint8_t *os)
{
	int len;
	const void *data;

	/* Get OS name from property data */
	data = fdt_getprop(fit, noffset, FIT_OS_PROP, &len);
	if (data == NULL) {
		fit_get_debug(fit, noffset, FIT_OS_PROP, len);
		*os = -1;
		return -1;
	}

	/* Translate OS name to id */
	*os = genimg_get_os_id(data);
	return 0;
}

/**
 * fit_image_get_arch - get arch id for a given component image node
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @arch: pointer to the uint8_t, will hold arch numeric id
 *
 * fit_image_get_arch() finds arch property in a given component image node.
 * If the property is found, its (string) value is translated to the numeric
 * id which is returned to the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_get_arch(const void *fit, int noffset, uint8_t *arch)
{
	int len;
	const void *data;

	/* Get architecture name from property data */
	data = fdt_getprop(fit, noffset, FIT_ARCH_PROP, &len);
	if (data == NULL) {
		fit_get_debug(fit, noffset, FIT_ARCH_PROP, len);
		*arch = -1;
		return -1;
	}

	/* Translate architecture name to id */
	*arch = genimg_get_arch_id(data);
	return 0;
}

/**
 * fit_image_get_type - get type id for a given component image node
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @type: pointer to the uint8_t, will hold type numeric id
 *
 * fit_image_get_type() finds type property in a given component image node.
 * If the property is found, its (string) value is translated to the numeric
 * id which is returned to the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_get_type(const void *fit, int noffset, uint8_t *type)
{
	int len;
	const void *data;

	/* Get image type name from property data */
	data = fdt_getprop(fit, noffset, FIT_TYPE_PROP, &len);
	if (data == NULL) {
		fit_get_debug(fit, noffset, FIT_TYPE_PROP, len);
		*type = -1;
		return -1;
	}

	/* Translate image type name to id */
	*type = genimg_get_type_id(data);
	return 0;
}

/**
 * fit_image_get_comp - get comp id for a given component image node
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @comp: pointer to the uint8_t, will hold comp numeric id
 *
 * fit_image_get_comp() finds comp property in a given component image node.
 * If the property is found, its (string) value is translated to the numeric
 * id which is returned to the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_get_comp(const void *fit, int noffset, uint8_t *comp)
{
	int len;
	const void *data;

	/* Get compression name from property data */
	data = fdt_getprop(fit, noffset, FIT_COMP_PROP, &len);
	if (data == NULL) {
		fit_get_debug(fit, noffset, FIT_COMP_PROP, len);
		return -1;
	}

	/* Translate compression name to id */
	*comp = genimg_get_comp_id(data);
	return 0;
}

/**
 * fit_image_get_phase() - get the phase for a configuration node
 * @fit: pointer to the FIT format image header
 * @offset: configuration-node offset
 * @phasep: returns the phase
 *
 * Finds the phase property in a given configuration node. If the property is
 * found, its (string) value is translated to the numeric id which is returned
 * to the caller.
 *
 * Returns: 0 on success, -ENOENT if missing, -EINVAL for invalid value
 */
int fit_image_get_phase(const void *fit, int offset, enum image_phase_t *phasep)
{
	const void *data;
	int len, ret;

	/* Get phase name from property data */
	data = fdt_getprop(fit, offset, FIT_PHASE_PROP, &len);
	if (!data) {
		fit_get_debug(fit, offset, FIT_PHASE_PROP, len);
		*phasep = 0;
		return -ENOENT;
	}

	/* Translate phase name to id */
	ret = genimg_get_phase_id(data);
	if (ret < 0)
		return ret;
	*phasep = ret;

	return 0;
}

static int fit_image_get_address(const void *fit, int noffset, char *name,
			  ulong *load)
{
	int len, cell_len;
	const fdt32_t *cell;
	uint64_t load64 = 0;

	cell = fdt_getprop(fit, noffset, name, &len);
	if (cell == NULL) {
		fit_get_debug(fit, noffset, name, len);
		return -1;
	}

	cell_len = len >> 2;
	/* Use load64 to avoid compiling warning for 32-bit target */
	while (cell_len--) {
		load64 = (load64 << 32) | uimage_to_cpu(*cell);
		cell++;
	}

	if (len > sizeof(ulong) && (uint32_t)(load64 >> 32)) {
		printf("Unsupported %s address size\n", name);
		return -1;
	}

	*load = (ulong)load64;

	return 0;
}
/**
 * fit_image_get_load() - get load addr property for given component image node
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @load: pointer to the uint32_t, will hold load address
 *
 * fit_image_get_load() finds load address property in a given component
 * image node. If the property is found, its value is returned to the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_get_load(const void *fit, int noffset, ulong *load)
{
	return fit_image_get_address(fit, noffset, FIT_LOAD_PROP, load);
}

/**
 * fit_image_get_entry() - get entry point address property
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @entry: pointer to the uint32_t, will hold entry point address
 *
 * This gets the entry point address property for a given component image
 * node.
 *
 * fit_image_get_entry() finds entry point address property in a given
 * component image node.  If the property is found, its value is returned
 * to the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_get_entry(const void *fit, int noffset, ulong *entry)
{
	return fit_image_get_address(fit, noffset, FIT_ENTRY_PROP, entry);
}

/**
 * fit_image_get_data - get data property and its size for a given component image node
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @data: double pointer to void, will hold data property's data address
 * @size: pointer to size_t, will hold data property's data size
 *
 * fit_image_get_data() finds data property in a given component image node.
 * If the property is found its data start address and size are returned to
 * the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_get_data(const void *fit, int noffset,
		const void **data, size_t *size)
{
	int len;

	*data = fdt_getprop(fit, noffset, FIT_DATA_PROP, &len);
	if (*data == NULL) {
		fit_get_debug(fit, noffset, FIT_DATA_PROP, len);
		*size = 0;
		return -1;
	}

	*size = len;
	return 0;
}

/**
 * Get 'data-offset' property from a given image node.
 *
 * @fit: pointer to the FIT image header
 * @noffset: component image node offset
 * @data_offset: holds the data-offset property
 *
 * returns:
 *     0, on success
 *     -ENOENT if the property could not be found
 */
int fit_image_get_data_offset(const void *fit, int noffset, int *data_offset)
{
	const fdt32_t *val;

	val = fdt_getprop(fit, noffset, FIT_DATA_OFFSET_PROP, NULL);
	if (!val)
		return -ENOENT;

	*data_offset = fdt32_to_cpu(*val);

	return 0;
}

/**
 * Get 'data-position' property from a given image node.
 *
 * @fit: pointer to the FIT image header
 * @noffset: component image node offset
 * @data_position: holds the data-position property
 *
 * returns:
 *     0, on success
 *     -ENOENT if the property could not be found
 */
int fit_image_get_data_position(const void *fit, int noffset,
				int *data_position)
{
	const fdt32_t *val;

	val = fdt_getprop(fit, noffset, FIT_DATA_POSITION_PROP, NULL);
	if (!val)
		return -ENOENT;

	*data_position = fdt32_to_cpu(*val);

	return 0;
}

/**
 * Get 'data-size' property from a given image node.
 *
 * @fit: pointer to the FIT image header
 * @noffset: component image node offset
 * @data_size: holds the data-size property
 *
 * returns:
 *     0, on success
 *     -ENOENT if the property could not be found
 */
int fit_image_get_data_size(const void *fit, int noffset, int *data_size)
{
	const fdt32_t *val;

	val = fdt_getprop(fit, noffset, FIT_DATA_SIZE_PROP, NULL);
	if (!val)
		return -ENOENT;

	*data_size = fdt32_to_cpu(*val);

	return 0;
}

/**
 * Get 'data-size-unciphered' property from a given image node.
 *
 * @fit: pointer to the FIT image header
 * @noffset: component image node offset
 * @data_size: holds the data-size property
 *
 * returns:
 *     0, on success
 *     -ENOENT if the property could not be found
 */
int fit_image_get_data_size_unciphered(const void *fit, int noffset,
				       size_t *data_size)
{
	const fdt32_t *val;

	val = fdt_getprop(fit, noffset, "data-size-unciphered", NULL);
	if (!val)
		return -ENOENT;

	*data_size = (size_t)fdt32_to_cpu(*val);

	return 0;
}

/**
 * fit_image_get_data_and_size - get data and its size including
 *				 both embedded and external data
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @data: double pointer to void, will hold data property's data address
 * @size: pointer to size_t, will hold data property's data size
 *
 * fit_image_get_data_and_size() finds data and its size including
 * both embedded and external data. If the property is found
 * its data start address and size are returned to the caller.
 *
 * returns:
 *     0, on success
 *     otherwise, on failure
 */
int fit_image_get_data_and_size(const void *fit, int noffset,
				const void **data, size_t *size)
{
	bool external_data = false;
	int offset;
	int len = 0;
	int ret;

	if (!fit_image_get_data_position(fit, noffset, &offset)) {
		external_data = true;
	} else if (!fit_image_get_data_offset(fit, noffset, &offset)) {
		external_data = true;
		/*
		 * For FIT with external data, figure out where
		 * the external images start. This is the base
		 * for the data-offset properties in each image.
		 */
		offset += ((fdt_totalsize(fit) + 3) & ~3);
	}

	if (external_data) {
		debug("External Data\n");
		ret = fit_image_get_data_size(fit, noffset, &len);
		if (!ret) {
			*data = fit + offset;
			*size = len;
		}
	} else {
		ret = fit_image_get_data(fit, noffset, data, size);
	}

	return ret;
}

/**
 * fit_image_hash_get_algo - get hash algorithm name
 * @fit: pointer to the FIT format image header
 * @noffset: hash node offset
 * @algo: double pointer to char, will hold pointer to the algorithm name
 *
 * fit_image_hash_get_algo() finds hash algorithm property in a given hash node.
 * If the property is found its data start address is returned to the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_hash_get_algo(const void *fit, int noffset, const char **algo)
{
	int len;

	*algo = (const char *)fdt_getprop(fit, noffset, FIT_ALGO_PROP, &len);
	if (*algo == NULL) {
		fit_get_debug(fit, noffset, FIT_ALGO_PROP, len);
		return -1;
	}

	return 0;
}

/**
 * fit_image_hash_get_value - get hash value and length
 * @fit: pointer to the FIT format image header
 * @noffset: hash node offset
 * @value: double pointer to uint8_t, will hold address of a hash value data
 * @value_len: pointer to an int, will hold hash data length
 *
 * fit_image_hash_get_value() finds hash value property in a given hash node.
 * If the property is found its data start address and size are returned to
 * the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_hash_get_value(const void *fit, int noffset, uint8_t **value,
				int *value_len)
{
	int len;

	*value = (uint8_t *)fdt_getprop(fit, noffset, FIT_VALUE_PROP, &len);
	if (*value == NULL) {
		fit_get_debug(fit, noffset, FIT_VALUE_PROP, len);
		*value_len = 0;
		return -1;
	}

	*value_len = len;
	return 0;
}

/**
 * fit_image_hash_get_ignore - get hash ignore flag
 * @fit: pointer to the FIT format image header
 * @noffset: hash node offset
 * @ignore: pointer to an int, will hold hash ignore flag
 *
 * fit_image_hash_get_ignore() finds hash ignore property in a given hash node.
 * If the property is found and non-zero, the hash algorithm is not verified by
 * u-boot automatically.
 *
 * returns:
 *     0, on ignore not found
 *     value, on ignore found
 */
static int fit_image_hash_get_ignore(const void *fit, int noffset, int *ignore)
{
	int len;
	int *value;

	value = (int *)fdt_getprop(fit, noffset, FIT_IGNORE_PROP, &len);
	if (value == NULL || len != sizeof(int))
		*ignore = 0;
	else
		*ignore = *value;

	return 0;
}

/**
 * fit_image_cipher_get_algo - get cipher algorithm name
 * @fit: pointer to the FIT format image header
 * @noffset: cipher node offset
 * @algo: double pointer to char, will hold pointer to the algorithm name
 *
 * fit_image_cipher_get_algo() finds cipher algorithm property in a given
 * cipher node. If the property is found its data start address is returned
 * to the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_cipher_get_algo(const void *fit, int noffset, char **algo)
{
	int len;

	*algo = (char *)fdt_getprop(fit, noffset, FIT_ALGO_PROP, &len);
	if (!*algo) {
		fit_get_debug(fit, noffset, FIT_ALGO_PROP, len);
		return -1;
	}

	return 0;
}

ulong fit_get_end(const void *fit)
{
	return map_to_sysmem((void *)(fit + fdt_totalsize(fit)));
}

/**
 * fit_set_timestamp - set node timestamp property
 * @fit: pointer to the FIT format image header
 * @noffset: node offset
 * @timestamp: timestamp value to be set
 *
 * fit_set_timestamp() attempts to set timestamp property in the requested
 * node and returns operation status to the caller.
 *
 * returns:
 *     0, on success
 *     -ENOSPC if no space in device tree, -1 for other error
 */
int fit_set_timestamp(void *fit, int noffset, time_t timestamp)
{
	uint32_t t;
	int ret;

	t = cpu_to_uimage(timestamp);
	ret = fdt_setprop(fit, noffset, FIT_TIMESTAMP_PROP, &t,
				sizeof(uint32_t));
	if (ret) {
		debug("Can't set '%s' property for '%s' node (%s)\n",
		      FIT_TIMESTAMP_PROP, fit_get_name(fit, noffset, NULL),
		      fdt_strerror(ret));
		return ret == -FDT_ERR_NOSPACE ? -ENOSPC : -1;
	}

	return 0;
}

/**
 * calculate_hash - calculate and return hash for provided input data
 * @data: pointer to the input data
 * @data_len: data length
 * @name: requested hash algorithm name
 * @value: pointer to the char, will hold hash value data (caller must
 * allocate enough free space)
 * value_len: length of the calculated hash
 *
 * calculate_hash() computes input data hash according to the requested
 * algorithm.
 * Resulting hash value is placed in caller provided 'value' buffer, length
 * of the calculated hash is returned via value_len pointer argument.
 *
 * returns:
 *     0, on success
 *    -1, when algo is unsupported
 */
int calculate_hash(const void *data, int data_len, const char *name,
			uint8_t *value, int *value_len)
{
#if !defined(USE_HOSTCC) && defined(CONFIG_DM_HASH)
	int rc;
	enum HASH_ALGO hash_algo;
	struct udevice *dev;

	rc = uclass_get_device(UCLASS_HASH, 0, &dev);
	if (rc) {
		debug("failed to get hash device, rc=%d\n", rc);
		return -1;
	}

	hash_algo = hash_algo_lookup_by_name(name);
	if (hash_algo == HASH_ALGO_INVALID) {
		debug("Unsupported hash algorithm\n");
		return -1;
	};

	rc = hash_digest_wd(dev, hash_algo, data, data_len, value, CHUNKSZ);
	if (rc) {
		debug("failed to get hash value, rc=%d\n", rc);
		return -1;
	}

	*value_len = hash_algo_digest_size(hash_algo);
#else
	struct hash_algo *algo;
	int ret;

	ret = hash_lookup_algo(name, &algo);
	if (ret < 0) {
		debug("Unsupported hash alogrithm\n");
		return -1;
	}

	algo->hash_func_ws(data, data_len, value, algo->chunk_size);
	*value_len = algo->digest_size;
#endif

	return 0;
}

static int fit_image_check_hash(const void *fit, int noffset, const void *data,
				size_t size, char **err_msgp)
{
	ALLOC_CACHE_ALIGN_BUFFER(uint8_t, value, FIT_MAX_HASH_LEN);
	int value_len;
	const char *algo;
	uint8_t *fit_value;
	int fit_value_len;
	int ignore;

	*err_msgp = NULL;

	if (fit_image_hash_get_algo(fit, noffset, &algo)) {
		*err_msgp = "Can't get hash algo property";
		return -1;
	}
	printf("%s", algo);

	if (!tools_build()) {
		fit_image_hash_get_ignore(fit, noffset, &ignore);
		if (ignore) {
			printf("-skipped ");
			return 0;
		}
	}

	if (fit_image_hash_get_value(fit, noffset, &fit_value,
				     &fit_value_len)) {
		*err_msgp = "Can't get hash value property";
		return -1;
	}

	if (calculate_hash(data, size, algo, value, &value_len)) {
		*err_msgp = "Unsupported hash algorithm";
		return -1;
	}

	if (value_len != fit_value_len) {
		*err_msgp = "Bad hash value len";
		return -1;
	} else if (memcmp(value, fit_value, value_len) != 0) {
		*err_msgp = "Bad hash value";
		return -1;
	}

	return 0;
}

int fit_image_verify_with_data(const void *fit, int image_noffset,
			       const void *key_blob, const void *data,
			       size_t size)
{
	int		noffset = 0;
	char		*err_msg = "";
	int verify_all = 1;
	int ret;

	/* Verify all required signatures */
	if (FIT_IMAGE_ENABLE_VERIFY &&
	    fit_image_verify_required_sigs(fit, image_noffset, data, size,
					   key_blob, &verify_all)) {
		err_msg = "Unable to verify required signature";
		goto error;
	}

	/* Process all hash subnodes of the component image node */
	fdt_for_each_subnode(noffset, fit, image_noffset) {
		const char *name = fit_get_name(fit, noffset, NULL);

		/*
		 * Check subnode name, must be equal to "hash".
		 * Multiple hash nodes require unique unit node
		 * names, e.g. hash-1, hash-2, etc.
		 */
		if (!strncmp(name, FIT_HASH_NODENAME,
			     strlen(FIT_HASH_NODENAME))) {
			if (fit_image_check_hash(fit, noffset, data, size,
						 &err_msg))
				goto error;
			puts("+ ");
		} else if (FIT_IMAGE_ENABLE_VERIFY && verify_all &&
				!strncmp(name, FIT_SIG_NODENAME,
					strlen(FIT_SIG_NODENAME))) {
			ret = fit_image_check_sig(fit, noffset, data, size,
						  gd_fdt_blob(), -1, &err_msg);

			/*
			 * Show an indication on failure, but do not return
			 * an error. Only keys marked 'required' can cause
			 * an image validation failure. See the call to
			 * fit_image_verify_required_sigs() above.
			 */
			if (ret)
				puts("- ");
			else
				puts("+ ");
		}
	}

	if (noffset == -FDT_ERR_TRUNCATED || noffset == -FDT_ERR_BADSTRUCTURE) {
		err_msg = "Corrupted or truncated tree";
		goto error;
	}

	return 1;

error:
	printf(" error!\n%s for '%s' hash node in '%s' image node\n",
	       err_msg, fit_get_name(fit, noffset, NULL),
	       fit_get_name(fit, image_noffset, NULL));
	return 0;
}

/**
 * fit_image_verify - verify data integrity
 * @fit: pointer to the FIT format image header
 * @image_noffset: component image node offset
 *
 * fit_image_verify() goes over component image hash nodes,
 * re-calculates each data hash and compares with the value stored in hash
 * node.
 *
 * returns:
 *     1, if all hashes are valid
 *     0, otherwise (or on error)
 */
int fit_image_verify(const void *fit, int image_noffset)
{
	const char *name = fit_get_name(fit, image_noffset, NULL);
	const void	*data;
	size_t		size;
	char		*err_msg = "";

	if (CONFIG_IS_ENABLED(FIT_SIGNATURE) && strchr(name, '@')) {
		/*
		 * We don't support this since libfdt considers names with the
		 * name root but different @ suffix to be equal
		 */
		err_msg = "Node name contains @";
		goto err;
	}
	/* Get image data and data length */
	if (fit_image_get_data_and_size(fit, image_noffset, &data, &size)) {
		err_msg = "Can't get image data/size";
		goto err;
	}

	return fit_image_verify_with_data(fit, image_noffset, gd_fdt_blob(),
					  data, size);

err:
	printf("error!\n%s in '%s' image node\n", err_msg,
	       fit_get_name(fit, image_noffset, NULL));
	return 0;
}

/**
 * fit_all_image_verify - verify data integrity for all images
 * @fit: pointer to the FIT format image header
 *
 * fit_all_image_verify() goes over all images in the FIT and
 * for every images checks if all it's hashes are valid.
 *
 * returns:
 *     1, if all hashes of all images are valid
 *     0, otherwise (or on error)
 */
int fit_all_image_verify(const void *fit)
{
	int images_noffset;
	int noffset;
	int ndepth;
	int count;

	/* Find images parent node offset */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images_noffset < 0) {
		printf("Can't find images parent node '%s' (%s)\n",
		       FIT_IMAGES_PATH, fdt_strerror(images_noffset));
		return 0;
	}

	/* Process all image subnodes, check hashes for each */
	printf("## Checking hash(es) for FIT Image at %08lx ...\n",
	       (ulong)fit);
	for (ndepth = 0, count = 0,
	     noffset = fdt_next_node(fit, images_noffset, &ndepth);
			(noffset >= 0) && (ndepth > 0);
			noffset = fdt_next_node(fit, noffset, &ndepth)) {
		if (ndepth == 1) {
			/*
			 * Direct child node of the images parent node,
			 * i.e. component image node.
			 */
			printf("   Hash(es) for Image %u (%s): ", count,
			       fit_get_name(fit, noffset, NULL));
			count++;

			if (!fit_image_verify(fit, noffset))
				return 0;
			printf("\n");
		}
	}
	return 1;
}

int fit_image_uncipher(const void *fit, int image_noffset,
			      void **data, size_t *size)
{
	int cipher_noffset, ret;
	void *dst;
	size_t size_dst;

	cipher_noffset = fdt_subnode_offset(fit, image_noffset,
					    FIT_CIPHER_NODENAME);
	if (cipher_noffset < 0)
		return 0;

	puts("   Decrypting Data ... ");
	ret = fit_image_decrypt_data(fit, image_noffset, cipher_noffset,
				     *data, *size, &dst, &size_dst);
	if (ret) {
		puts("Error\n");
		goto out;
	}

	*data = dst;
	*size = size_dst;
	puts("OK\n");

 out:
	return ret;
}

/**
 * fit_image_check_os - check whether image node is of a given os type
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @os: requested image os
 *
 * fit_image_check_os() reads image os property and compares its numeric
 * id with the requested os. Comparison result is returned to the caller.
 *
 * returns:
 *     1 if image is of given os type
 *     0 otherwise (or on error)
 */
int fit_image_check_os(const void *fit, int noffset, uint8_t os)
{
	uint8_t image_os;

	if (fit_image_get_os(fit, noffset, &image_os))
		return 0;
	return (os == image_os);
}

/**
 * fit_image_check_arch - check whether image node is of a given arch
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @arch: requested imagearch
 *
 * fit_image_check_arch() reads image arch property and compares its numeric
 * id with the requested arch. Comparison result is returned to the caller.
 *
 * returns:
 *     1 if image is of given arch
 *     0 otherwise (or on error)
 */
int fit_image_check_arch(const void *fit, int noffset, uint8_t arch)
{
	uint8_t image_arch;
	int aarch32_support = 0;

	/* Let's assume that sandbox can load any architecture */
	if (IS_ENABLED(CONFIG_SANDBOX))
		return true;

	if (IS_ENABLED(CONFIG_ARM64_SUPPORT_AARCH32))
		aarch32_support = 1;

	if (fit_image_get_arch(fit, noffset, &image_arch))
		return 0;
	return (arch == image_arch) ||
		(arch == IH_ARCH_I386 && image_arch == IH_ARCH_X86_64) ||
		(arch == IH_ARCH_ARM64 && image_arch == IH_ARCH_ARM &&
		 aarch32_support);
}

/**
 * fit_image_check_type - check whether image node is of a given type
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @type: requested image type
 *
 * fit_image_check_type() reads image type property and compares its numeric
 * id with the requested type. Comparison result is returned to the caller.
 *
 * returns:
 *     1 if image is of given type
 *     0 otherwise (or on error)
 */
int fit_image_check_type(const void *fit, int noffset, uint8_t type)
{
	uint8_t image_type;

	if (fit_image_get_type(fit, noffset, &image_type))
		return 0;
	return (type == image_type);
}

/**
 * fit_image_check_comp - check whether image node uses given compression
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @comp: requested image compression type
 *
 * fit_image_check_comp() reads image compression property and compares its
 * numeric id with the requested compression type. Comparison result is
 * returned to the caller.
 *
 * returns:
 *     1 if image uses requested compression
 *     0 otherwise (or on error)
 */
int fit_image_check_comp(const void *fit, int noffset, uint8_t comp)
{
	uint8_t image_comp;

	if (fit_image_get_comp(fit, noffset, &image_comp))
		return 0;
	return (comp == image_comp);
}

/**
 * fdt_check_no_at() - Check for nodes whose names contain '@'
 *
 * This checks the parent node and all subnodes recursively
 *
 * @fit: FIT to check
 * @parent: Parent node to check
 * Return: 0 if OK, -EADDRNOTAVAIL is a node has a name containing '@'
 */
static int fdt_check_no_at(const void *fit, int parent)
{
	const char *name;
	int node;
	int ret;

	name = fdt_get_name(fit, parent, NULL);
	if (!name || strchr(name, '@'))
		return -EADDRNOTAVAIL;

	fdt_for_each_subnode(node, fit, parent) {
		ret = fdt_check_no_at(fit, node);
		if (ret)
			return ret;
	}

	return 0;
}

int fit_check_format(const void *fit, ulong size)
{
	int ret;

	/* A FIT image must be a valid FDT */
	ret = fdt_check_header(fit);
	if (ret) {
		log_debug("Wrong FIT format: not a flattened device tree (err=%d)\n",
			  ret);
		return -ENOEXEC;
	}

	if (CONFIG_IS_ENABLED(FIT_FULL_CHECK)) {
		/*
		 * If we are not given the size, make do wtih calculating it.
		 * This is not as secure, so we should consider a flag to
		 * control this.
		 */
		if (size == IMAGE_SIZE_INVAL)
			size = fdt_totalsize(fit);
		ret = fdt_check_full(fit, size);
		if (ret)
			ret = -EINVAL;

		/*
		 * U-Boot stopped using unit addressed in 2017. Since libfdt
		 * can match nodes ignoring any unit address, signature
		 * verification can see the wrong node if one is inserted with
		 * the same name as a valid node but with a unit address
		 * attached. Protect against this by disallowing unit addresses.
		 */
		if (!ret && CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
			ret = fdt_check_no_at(fit, 0);

			if (ret) {
				log_debug("FIT check error %d\n", ret);
				return ret;
			}
		}
		if (ret) {
			log_debug("FIT check error %d\n", ret);
			return ret;
		}
	}

	/* mandatory / node 'description' property */
	if (!fdt_getprop(fit, 0, FIT_DESC_PROP, NULL)) {
		log_debug("Wrong FIT format: no description\n");
		return -ENOMSG;
	}

	if (IMAGE_ENABLE_TIMESTAMP) {
		/* mandatory / node 'timestamp' property */
		if (!fdt_getprop(fit, 0, FIT_TIMESTAMP_PROP, NULL)) {
			log_debug("Wrong FIT format: no timestamp\n");
			return -EBADMSG;
		}
	}

	/* mandatory subimages parent '/images' node */
	if (fdt_path_offset(fit, FIT_IMAGES_PATH) < 0) {
		log_debug("Wrong FIT format: no images parent node\n");
		return -ENOENT;
	}

	return 0;
}

int fit_conf_find_compat(const void *fit, const void *fdt)
{
	int ndepth = 0;
	int noffset, confs_noffset, images_noffset;
	const void *fdt_compat;
	int fdt_compat_len;
	int best_match_offset = 0;
	int best_match_pos = 0;

	confs_noffset = fdt_path_offset(fit, FIT_CONFS_PATH);
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (confs_noffset < 0 || images_noffset < 0) {
		debug("Can't find configurations or images nodes.\n");
		return -1;
	}

	fdt_compat = fdt_getprop(fdt, 0, "compatible", &fdt_compat_len);
	if (!fdt_compat) {
		debug("Fdt for comparison has no \"compatible\" property.\n");
		return -1;
	}

	/*
	 * Loop over the configurations in the FIT image.
	 */
	for (noffset = fdt_next_node(fit, confs_noffset, &ndepth);
			(noffset >= 0) && (ndepth > 0);
			noffset = fdt_next_node(fit, noffset, &ndepth)) {
		const void *fdt;
		const char *kfdt_name;
		int kfdt_noffset, compat_noffset;
		const char *cur_fdt_compat;
		int len;
		size_t sz;
		int i;

		if (ndepth > 1)
			continue;

		/* If there's a compat property in the config node, use that. */
		if (fdt_getprop(fit, noffset, "compatible", NULL)) {
			fdt = fit;		  /* search in FIT image */
			compat_noffset = noffset; /* search under config node */
		} else {	/* Otherwise extract it from the kernel FDT. */
			kfdt_name = fdt_getprop(fit, noffset, "fdt", &len);
			if (!kfdt_name) {
				debug("No fdt property found.\n");
				continue;
			}
			kfdt_noffset = fdt_subnode_offset(fit, images_noffset,
							  kfdt_name);
			if (kfdt_noffset < 0) {
				debug("No image node named \"%s\" found.\n",
				      kfdt_name);
				continue;
			}

			if (!fit_image_check_comp(fit, kfdt_noffset,
						  IH_COMP_NONE)) {
				debug("Can't extract compat from \"%s\" "
				      "(compressed)\n", kfdt_name);
				continue;
			}

			/* search in this config's kernel FDT */
			if (fit_image_get_data_and_size(fit, kfdt_noffset,
							&fdt, &sz)) {
				debug("Failed to get fdt \"%s\".\n", kfdt_name);
				continue;
			}

			compat_noffset = 0;  /* search kFDT under root node */
		}

		len = fdt_compat_len;
		cur_fdt_compat = fdt_compat;
		/*
		 * Look for a match for each U-Boot compatibility string in
		 * turn in the compat string property.
		 */
		for (i = 0; len > 0 &&
		     (!best_match_offset || best_match_pos > i); i++) {
			int cur_len = strlen(cur_fdt_compat) + 1;

			if (!fdt_node_check_compatible(fdt, compat_noffset,
						       cur_fdt_compat)) {
				best_match_offset = noffset;
				best_match_pos = i;
				break;
			}
			len -= cur_len;
			cur_fdt_compat += cur_len;
		}
	}
	if (!best_match_offset) {
		debug("No match found.\n");
		return -1;
	}

	return best_match_offset;
}

int fit_conf_get_node(const void *fit, const char *conf_uname)
{
	int noffset, confs_noffset;
	int len;
	const char *s;
	char *conf_uname_copy = NULL;

	confs_noffset = fdt_path_offset(fit, FIT_CONFS_PATH);
	if (confs_noffset < 0) {
		debug("Can't find configurations parent node '%s' (%s)\n",
		      FIT_CONFS_PATH, fdt_strerror(confs_noffset));
		return confs_noffset;
	}

	if (conf_uname == NULL) {
		/* get configuration unit name from the default property */
		debug("No configuration specified, trying default...\n");
		if (!tools_build() && IS_ENABLED(CONFIG_MULTI_DTB_FIT)) {
			noffset = fit_find_config_node(fit);
			if (noffset < 0)
				return noffset;
			conf_uname = fdt_get_name(fit, noffset, NULL);
		} else {
			conf_uname = (char *)fdt_getprop(fit, confs_noffset,
							 FIT_DEFAULT_PROP, &len);
			if (conf_uname == NULL) {
				fit_get_debug(fit, confs_noffset, FIT_DEFAULT_PROP,
					      len);
				return len;
			}
		}
		debug("Found default configuration: '%s'\n", conf_uname);
	}

	s = strchr(conf_uname, '#');
	if (s) {
		len = s - conf_uname;
		conf_uname_copy = malloc(len + 1);
		if (!conf_uname_copy) {
			debug("Can't allocate uname copy: '%s'\n",
					conf_uname);
			return -ENOMEM;
		}
		memcpy(conf_uname_copy, conf_uname, len);
		conf_uname_copy[len] = '\0';
		conf_uname = conf_uname_copy;
	}

	noffset = fdt_subnode_offset(fit, confs_noffset, conf_uname);
	if (noffset < 0) {
		debug("Can't get node offset for configuration unit name: '%s' (%s)\n",
		      conf_uname, fdt_strerror(noffset));
	}

	free(conf_uname_copy);

	return noffset;
}

int fit_conf_get_prop_node_count(const void *fit, int noffset,
		const char *prop_name)
{
	return fdt_stringlist_count(fit, noffset, prop_name);
}

int fit_conf_get_prop_node_index(const void *fit, int noffset,
		const char *prop_name, int index)
{
	const char *uname;
	int len;

	/* get kernel image unit name from configuration kernel property */
	uname = fdt_stringlist_get(fit, noffset, prop_name, index, &len);
	if (uname == NULL)
		return len;

	return fit_image_get_node(fit, uname);
}

int fit_conf_get_prop_node(const void *fit, int noffset, const char *prop_name,
			   enum image_phase_t sel_phase)
{
	int i, count;

	if (sel_phase == IH_PHASE_NONE)
		return fit_conf_get_prop_node_index(fit, noffset, prop_name, 0);

	count = fit_conf_get_prop_node_count(fit, noffset, prop_name);
	if (count < 0)
		return count;

	/* check each image in the list */
	for (i = 0; i < count; i++) {
		enum image_phase_t phase;
		int ret, node;

		node = fit_conf_get_prop_node_index(fit, noffset, prop_name, i);
		ret = fit_image_get_phase(fit, node, &phase);

		/* if the image is for any phase, let's use it */
		if (ret == -ENOENT)
			return node;
		else if (ret < 0)
			return ret;

		if (phase == sel_phase)
			return node;
	}

	return -ENOENT;
}

static int fit_get_data_tail(const void *fit, int noffset,
			     const void **data, size_t *size)
{
	char *desc;

	if (noffset < 0)
		return noffset;

	if (!fit_image_verify(fit, noffset))
		return -EINVAL;

	if (fit_image_get_data_and_size(fit, noffset, data, size))
		return -ENOENT;

	if (!fit_get_desc(fit, noffset, &desc))
		printf("%s\n", desc);

	return 0;
}

int fit_get_data_node(const void *fit, const char *image_uname,
		      const void **data, size_t *size)
{
	int noffset = fit_image_get_node(fit, image_uname);

	return fit_get_data_tail(fit, noffset, data, size);
}

int fit_get_data_conf_prop(const void *fit, const char *prop_name,
			   const void **data, size_t *size)
{
	int noffset = fit_conf_get_node(fit, NULL);

	noffset = fit_conf_get_prop_node(fit, noffset, prop_name,
					 IH_PHASE_NONE);
	return fit_get_data_tail(fit, noffset, data, size);
}

static int fit_image_select(const void *fit, int rd_noffset, int verify)
{
	fit_image_print(fit, rd_noffset, "   ");

	if (verify) {
		puts("   Verifying Hash Integrity ... ");
		if (!fit_image_verify(fit, rd_noffset)) {
			puts("Bad Data Hash\n");
			return -EACCES;
		}
		puts("OK\n");
	}

	return 0;
}

int fit_get_node_from_config(struct bootm_headers *images,
			     const char *prop_name, ulong addr)
{
	int cfg_noffset;
	void *fit_hdr;
	int noffset;

	debug("*  %s: using config '%s' from image at 0x%08lx\n",
	      prop_name, images->fit_uname_cfg, addr);

	/* Check whether configuration has this property defined */
	fit_hdr = map_sysmem(addr, 0);
	cfg_noffset = fit_conf_get_node(fit_hdr, images->fit_uname_cfg);
	if (cfg_noffset < 0) {
		debug("*  %s: no such config\n", prop_name);
		return -EINVAL;
	}

	noffset = fit_conf_get_prop_node(fit_hdr, cfg_noffset, prop_name,
					 IH_PHASE_NONE);
	if (noffset < 0) {
		debug("*  %s: no '%s' in config\n", prop_name, prop_name);
		return -ENOENT;
	}

	return noffset;
}

/**
 * fit_get_image_type_property() - get property name for IH_TYPE_...
 *
 * Return: the properly name where we expect to find the image in the
 * config node
 */
static const char *fit_get_image_type_property(int type)
{
	/*
	 * This is sort-of available in the uimage_type[] table in image.c
	 * but we don't have access to the short name, and "fdt" is different
	 * anyway. So let's just keep it here.
	 */
	switch (type) {
	case IH_TYPE_FLATDT:
		return FIT_FDT_PROP;
	case IH_TYPE_KERNEL:
		return FIT_KERNEL_PROP;
	case IH_TYPE_FIRMWARE:
		return FIT_FIRMWARE_PROP;
	case IH_TYPE_RAMDISK:
		return FIT_RAMDISK_PROP;
	case IH_TYPE_X86_SETUP:
		return FIT_SETUP_PROP;
	case IH_TYPE_LOADABLE:
		return FIT_LOADABLE_PROP;
	case IH_TYPE_FPGA:
		return FIT_FPGA_PROP;
	case IH_TYPE_STANDALONE:
		return FIT_STANDALONE_PROP;
	}

	return "unknown";
}

int fit_image_load(struct bootm_headers *images, ulong addr,
		   const char **fit_unamep, const char **fit_uname_configp,
		   int arch, int ph_type, int bootstage_id,
		   enum fit_load_op load_op, ulong *datap, ulong *lenp)
{
	int image_type = image_ph_type(ph_type);
	int cfg_noffset, noffset;
	const char *fit_uname;
	const char *fit_uname_config;
	const char *fit_base_uname_config;
	const void *fit;
	void *buf;
	void *loadbuf;
	size_t size;
	int type_ok, os_ok;
	ulong load, load_end, data, len;
	uint8_t os, comp;
	const char *prop_name;
	int ret;

	fit = map_sysmem(addr, 0);
	fit_uname = fit_unamep ? *fit_unamep : NULL;
	fit_uname_config = fit_uname_configp ? *fit_uname_configp : NULL;
	fit_base_uname_config = NULL;
	prop_name = fit_get_image_type_property(image_type);
	printf("## Loading %s from FIT Image at %08lx ...\n", prop_name, addr);

	bootstage_mark(bootstage_id + BOOTSTAGE_SUB_FORMAT);
	ret = fit_check_format(fit, IMAGE_SIZE_INVAL);
	if (ret) {
		printf("Bad FIT %s image format! (err=%d)\n", prop_name, ret);
		if (CONFIG_IS_ENABLED(FIT_SIGNATURE) && ret == -EADDRNOTAVAIL)
			printf("Signature checking prevents use of unit addresses (@) in nodes\n");
		bootstage_error(bootstage_id + BOOTSTAGE_SUB_FORMAT);
		return ret;
	}
	bootstage_mark(bootstage_id + BOOTSTAGE_SUB_FORMAT_OK);
	if (fit_uname) {
		/* get FIT component image node offset */
		bootstage_mark(bootstage_id + BOOTSTAGE_SUB_UNIT_NAME);
		noffset = fit_image_get_node(fit, fit_uname);
	} else {
		/*
		 * no image node unit name, try to get config
		 * node first. If config unit node name is NULL
		 * fit_conf_get_node() will try to find default config node
		 */
		bootstage_mark(bootstage_id + BOOTSTAGE_SUB_NO_UNIT_NAME);
		if (IS_ENABLED(CONFIG_FIT_BEST_MATCH) && !fit_uname_config) {
			cfg_noffset = fit_conf_find_compat(fit, gd_fdt_blob());
		} else {
			cfg_noffset = fit_conf_get_node(fit, fit_uname_config);
		}
		if (cfg_noffset < 0) {
			puts("Could not find configuration node\n");
			bootstage_error(bootstage_id +
					BOOTSTAGE_SUB_NO_UNIT_NAME);
			return -ENOENT;
		}

		fit_base_uname_config = fdt_get_name(fit, cfg_noffset, NULL);
		printf("   Using '%s' configuration\n", fit_base_uname_config);
		/* Remember this config */
		if (image_type == IH_TYPE_KERNEL)
			images->fit_uname_cfg = fit_base_uname_config;

		if (FIT_IMAGE_ENABLE_VERIFY && images->verify) {
			puts("   Verifying Hash Integrity ... ");
			if (fit_config_verify(fit, cfg_noffset)) {
				puts("Bad Data Hash\n");
				bootstage_error(bootstage_id +
					BOOTSTAGE_SUB_HASH);
				return -EACCES;
			}
			puts("OK\n");
		}

		bootstage_mark(BOOTSTAGE_ID_FIT_CONFIG);

		noffset = fit_conf_get_prop_node(fit, cfg_noffset, prop_name,
						 image_ph_phase(ph_type));
		fit_uname = fit_get_name(fit, noffset, NULL);
	}
	if (noffset < 0) {
		printf("Could not find subimage node type '%s'\n", prop_name);
		bootstage_error(bootstage_id + BOOTSTAGE_SUB_SUBNODE);
		return -ENOENT;
	}

	printf("   Trying '%s' %s subimage\n", fit_uname, prop_name);

	ret = fit_image_select(fit, noffset, images->verify);
	if (ret) {
		bootstage_error(bootstage_id + BOOTSTAGE_SUB_HASH);
		return ret;
	}

	bootstage_mark(bootstage_id + BOOTSTAGE_SUB_CHECK_ARCH);
	if (!tools_build() && IS_ENABLED(CONFIG_SANDBOX)) {
		if (!fit_image_check_target_arch(fit, noffset)) {
			puts("Unsupported Architecture\n");
			bootstage_error(bootstage_id + BOOTSTAGE_SUB_CHECK_ARCH);
			return -ENOEXEC;
		}
	}

#ifndef USE_HOSTCC
	{
	uint8_t os_arch;

	fit_image_get_arch(fit, noffset, &os_arch);
	images->os.arch = os_arch;
	}
#endif

	bootstage_mark(bootstage_id + BOOTSTAGE_SUB_CHECK_ALL);
	type_ok = fit_image_check_type(fit, noffset, image_type) ||
		  fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE) ||
		  fit_image_check_type(fit, noffset, IH_TYPE_TEE) ||
		  (image_type == IH_TYPE_KERNEL &&
		   fit_image_check_type(fit, noffset, IH_TYPE_KERNEL_NOLOAD));

	os_ok = image_type == IH_TYPE_FLATDT ||
		image_type == IH_TYPE_FPGA ||
		fit_image_check_os(fit, noffset, IH_OS_LINUX) ||
		fit_image_check_os(fit, noffset, IH_OS_U_BOOT) ||
		fit_image_check_os(fit, noffset, IH_OS_TEE) ||
		fit_image_check_os(fit, noffset, IH_OS_OPENRTOS) ||
		fit_image_check_os(fit, noffset, IH_OS_EFI) ||
		fit_image_check_os(fit, noffset, IH_OS_VXWORKS);

	/*
	 * If either of the checks fail, we should report an error, but
	 * if the image type is coming from the "loadables" field, we
	 * don't care what it is
	 */
	if ((!type_ok || !os_ok) && image_type != IH_TYPE_LOADABLE) {
		fit_image_get_os(fit, noffset, &os);
		printf("No %s %s %s Image\n",
		       genimg_get_os_name(os),
		       genimg_get_arch_name(arch),
		       genimg_get_type_name(image_type));
		bootstage_error(bootstage_id + BOOTSTAGE_SUB_CHECK_ALL);
		return -EIO;
	}

	bootstage_mark(bootstage_id + BOOTSTAGE_SUB_CHECK_ALL_OK);

	/* get image data address and length */
	if (fit_image_get_data_and_size(fit, noffset,
					(const void **)&buf, &size)) {
		printf("Could not find %s subimage data!\n", prop_name);
		bootstage_error(bootstage_id + BOOTSTAGE_SUB_GET_DATA);
		return -ENOENT;
	}

	/* Decrypt data before uncompress/move */
	if (IS_ENABLED(CONFIG_FIT_CIPHER) && IMAGE_ENABLE_DECRYPT) {
		if (fit_image_uncipher(fit, noffset, &buf, &size)) {
			return -EACCES;
		}
	}

	/* perform any post-processing on the image data */
	if (!tools_build() && IS_ENABLED(CONFIG_FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, noffset, &buf, &size);

	len = (ulong)size;

	bootstage_mark(bootstage_id + BOOTSTAGE_SUB_GET_DATA_OK);

	data = map_to_sysmem(buf);
	load = data;
	if (load_op == FIT_LOAD_IGNORED) {
		/* Don't load */
	} else if (fit_image_get_load(fit, noffset, &load)) {
		if (load_op == FIT_LOAD_REQUIRED) {
			printf("Can't get %s subimage load address!\n",
			       prop_name);
			bootstage_error(bootstage_id + BOOTSTAGE_SUB_LOAD);
			return -EBADF;
		}
	} else if (load_op != FIT_LOAD_OPTIONAL_NON_ZERO || load) {
		ulong image_start, image_end;

		/*
		 * move image data to the load address,
		 * make sure we don't overwrite initial image
		 */
		image_start = addr;
		image_end = addr + fit_get_size(fit);

		load_end = load + len;
		if (image_type != IH_TYPE_KERNEL &&
		    load < image_end && load_end > image_start) {
			printf("Error: %s overwritten\n", prop_name);
			return -EXDEV;
		}

		printf("   Loading %s from 0x%08lx to 0x%08lx\n",
		       prop_name, data, load);
	} else {
		load = data;	/* No load address specified */
	}

	comp = IH_COMP_NONE;
	loadbuf = buf;
	/* Kernel images get decompressed later in bootm_load_os(). */
	if (!fit_image_get_comp(fit, noffset, &comp) &&
	    comp != IH_COMP_NONE &&
	    !(image_type == IH_TYPE_KERNEL ||
	      image_type == IH_TYPE_KERNEL_NOLOAD ||
	      image_type == IH_TYPE_RAMDISK)) {
		ulong max_decomp_len = len * 20;
		if (load == data) {
			loadbuf = malloc(max_decomp_len);
			load = map_to_sysmem(loadbuf);
		} else {
			loadbuf = map_sysmem(load, max_decomp_len);
		}
		if (image_decomp(comp, load, data, image_type,
				loadbuf, buf, len, max_decomp_len, &load_end)) {
			printf("Error decompressing %s\n", prop_name);

			return -ENOEXEC;
		}
		len = load_end - load;
	} else if (load != data) {
		loadbuf = map_sysmem(load, len);
		memcpy(loadbuf, buf, len);
	}

	if (image_type == IH_TYPE_RAMDISK && comp != IH_COMP_NONE)
		puts("WARNING: 'compression' nodes for ramdisks are deprecated,"
		     " please fix your .its file!\n");

	/* verify that image data is a proper FDT blob */
	if (image_type == IH_TYPE_FLATDT && fdt_check_header(loadbuf)) {
		puts("Subimage data is not a FDT");
		return -ENOEXEC;
	}

	bootstage_mark(bootstage_id + BOOTSTAGE_SUB_LOAD);

	*datap = load;
	*lenp = len;
	if (fit_unamep)
		*fit_unamep = (char *)fit_uname;
	if (fit_uname_configp)
		*fit_uname_configp = (char *)(fit_uname_config ? :
					      fit_base_uname_config);

	return noffset;
}

int boot_get_setup_fit(struct bootm_headers *images, uint8_t arch,
		       ulong *setup_start, ulong *setup_len)
{
	int noffset;
	ulong addr;
	ulong len;
	int ret;

	addr = map_to_sysmem(images->fit_hdr_os);
	noffset = fit_get_node_from_config(images, FIT_SETUP_PROP, addr);
	if (noffset < 0)
		return noffset;

	ret = fit_image_load(images, addr, NULL, NULL, arch,
			     IH_TYPE_X86_SETUP, BOOTSTAGE_ID_FIT_SETUP_START,
			     FIT_LOAD_REQUIRED, setup_start, &len);

	return ret;
}

#ifndef USE_HOSTCC
int boot_get_fdt_fit(struct bootm_headers *images, ulong addr,
		     const char **fit_unamep, const char **fit_uname_configp,
		     int arch, ulong *datap, ulong *lenp)
{
	int fdt_noffset, cfg_noffset, count;
	const void *fit;
	const char *fit_uname = NULL;
	const char *fit_uname_config = NULL;
	char *fit_uname_config_copy = NULL;
	char *next_config = NULL;
	ulong load, len;
#ifdef CONFIG_OF_LIBFDT_OVERLAY
	ulong image_start, image_end;
	ulong ovload, ovlen, ovcopylen;
	const char *uconfig;
	const char *uname;
	void *base, *ov, *ovcopy = NULL;
	int i, err, noffset, ov_noffset;
#endif

	fit_uname = fit_unamep ? *fit_unamep : NULL;

	if (fit_uname_configp && *fit_uname_configp) {
		fit_uname_config_copy = strdup(*fit_uname_configp);
		if (!fit_uname_config_copy)
			return -ENOMEM;

		next_config = strchr(fit_uname_config_copy, '#');
		if (next_config)
			*next_config++ = '\0';
		if (next_config - 1 > fit_uname_config_copy)
			fit_uname_config = fit_uname_config_copy;
	}

	fdt_noffset = fit_image_load(images,
		addr, &fit_uname, &fit_uname_config,
		arch, IH_TYPE_FLATDT,
		BOOTSTAGE_ID_FIT_FDT_START,
		FIT_LOAD_OPTIONAL, &load, &len);

	if (fdt_noffset < 0)
		goto out;

	debug("fit_uname=%s, fit_uname_config=%s\n",
			fit_uname ? fit_uname : "<NULL>",
			fit_uname_config ? fit_uname_config : "<NULL>");

	fit = map_sysmem(addr, 0);

	cfg_noffset = fit_conf_get_node(fit, fit_uname_config);

	/* single blob, or error just return as well */
	count = fit_conf_get_prop_node_count(fit, cfg_noffset, FIT_FDT_PROP);
	if (count <= 1 && !next_config)
		goto out;

	/* we need to apply overlays */

#ifdef CONFIG_OF_LIBFDT_OVERLAY
	image_start = addr;
	image_end = addr + fit_get_size(fit);
	/* verify that relocation took place by load address not being in fit */
	if (load >= image_start && load < image_end) {
		/* check is simplified; fit load checks for overlaps */
		printf("Overlayed FDT requires relocation\n");
		fdt_noffset = -EBADF;
		goto out;
	}

	base = map_sysmem(load, len);

	/* apply extra configs in FIT first, followed by args */
	for (i = 1; ; i++) {
		if (i < count) {
			noffset = fit_conf_get_prop_node_index(fit, cfg_noffset,
							       FIT_FDT_PROP, i);
			uname = fit_get_name(fit, noffset, NULL);
			uconfig = NULL;
		} else {
			if (!next_config)
				break;
			uconfig = next_config;
			next_config = strchr(next_config, '#');
			if (next_config)
				*next_config++ = '\0';
			uname = NULL;

			/*
			 * fit_image_load() would load the first FDT from the
			 * extra config only when uconfig is specified.
			 * Check if the extra config contains multiple FDTs and
			 * if so, load them.
			 */
			cfg_noffset = fit_conf_get_node(fit, uconfig);

			i = 0;
			count = fit_conf_get_prop_node_count(fit, cfg_noffset,
							     FIT_FDT_PROP);
		}

		debug("%d: using uname=%s uconfig=%s\n", i, uname, uconfig);

		ov_noffset = fit_image_load(images,
			addr, &uname, &uconfig,
			arch, IH_TYPE_FLATDT,
			BOOTSTAGE_ID_FIT_FDT_START,
			FIT_LOAD_IGNORED, &ovload, &ovlen);
		if (ov_noffset < 0) {
			printf("load of %s failed\n", uname);
			continue;
		}
		debug("%s loaded at 0x%08lx len=0x%08lx\n",
				uname, ovload, ovlen);
		ov = map_sysmem(ovload, ovlen);

		ovcopylen = ALIGN(fdt_totalsize(ov), SZ_4K);
		ovcopy = malloc(ovcopylen);
		if (!ovcopy) {
			printf("failed to duplicate DTO before application\n");
			fdt_noffset = -ENOMEM;
			goto out;
		}

		err = fdt_open_into(ov, ovcopy, ovcopylen);
		if (err < 0) {
			printf("failed on fdt_open_into for DTO\n");
			fdt_noffset = err;
			goto out;
		}

		base = map_sysmem(load, len + ovlen);
		err = fdt_open_into(base, base, len + ovlen);
		if (err < 0) {
			printf("failed on fdt_open_into\n");
			fdt_noffset = err;
			goto out;
		}

		/* the verbose method prints out messages on error */
		err = fdt_overlay_apply_verbose(base, ovcopy);
		if (err < 0) {
			fdt_noffset = err;
			goto out;
		}
		fdt_pack(base);
		len = fdt_totalsize(base);

		free(ovcopy);
		ovcopy = NULL;
	}
#else
	printf("config with overlays but CONFIG_OF_LIBFDT_OVERLAY not set\n");
	fdt_noffset = -EBADF;
#endif

out:
	if (datap)
		*datap = load;
	if (lenp)
		*lenp = len;
	if (fit_unamep)
		*fit_unamep = fit_uname;
	if (fit_uname_configp)
		*fit_uname_configp = fit_uname_config;

#ifdef CONFIG_OF_LIBFDT_OVERLAY
	if (ovcopy)
		free(ovcopy);
#endif
	if (fit_uname_config_copy)
		free(fit_uname_config_copy);
	return fdt_noffset;
}
#endif
