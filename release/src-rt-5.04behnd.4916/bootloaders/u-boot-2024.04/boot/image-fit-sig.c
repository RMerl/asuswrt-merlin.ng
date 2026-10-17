// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013, Google Inc.
 */

#ifdef USE_HOSTCC
#include "mkimage.h"
#include <time.h>
#else
#include <common.h>
#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>
DECLARE_GLOBAL_DATA_PTR;
#endif /* !USE_HOSTCC*/
#include <fdt_region.h>
#include <image.h>
#include <u-boot/rsa.h>
#include <u-boot/hash-checksum.h>

#define IMAGE_MAX_HASHED_NODES		100

/**
 * fit_region_make_list() - Make a list of image regions
 *
 * Given a list of fdt_regions, create a list of image_regions. This is a
 * simple conversion routine since the FDT and image code use different
 * structures.
 *
 * @fit: FIT image
 * @fdt_regions: Pointer to FDT regions
 * @count: Number of FDT regions
 * @region: Pointer to image regions, which must hold @count records. If
 * region is NULL, then (except for an SPL build) the array will be
 * allocated.
 * @return: Pointer to image regions
 */
struct image_region *fit_region_make_list(const void *fit,
					  struct fdt_region *fdt_regions,
					  int count,
					  struct image_region *region)
{
	int i;

	debug("Hash regions:\n");
	debug("%10s %10s\n", "Offset", "Size");

	/*
	 * Use malloc() except in SPL (to save code size). In SPL the caller
	 * must allocate the array.
	 */
	if (!IS_ENABLED(CONFIG_SPL_BUILD) && !region)
		region = calloc(sizeof(*region), count);
	if (!region)
		return NULL;
	for (i = 0; i < count; i++) {
		debug("%10x %10x\n", fdt_regions[i].offset,
		      fdt_regions[i].size);
		region[i].data = fit + fdt_regions[i].offset;
		region[i].size = fdt_regions[i].size;
	}

	return region;
}

static int fit_image_setup_verify(struct image_sign_info *info,
				  const void *fit, int noffset,
				  const void *key_blob, int required_keynode,
				  char **err_msgp)
{
	const char *algo_name;
	const char *padding_name;

	if (fdt_totalsize(fit) > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)) {
		*err_msgp = "Total size too large";
		return 1;
	}
	if (fit_image_hash_get_algo(fit, noffset, &algo_name)) {
		*err_msgp = "Can't get hash algo property";
		return -1;
	}

	padding_name = fdt_getprop(fit, noffset, "padding", NULL);
	if (!padding_name)
		padding_name = RSA_DEFAULT_PADDING_NAME;

	memset(info, '\0', sizeof(*info));
	info->keyname = fdt_getprop(fit, noffset, FIT_KEY_HINT, NULL);
	info->fit = fit;
	info->node_offset = noffset;
	info->name = algo_name;
	info->checksum = image_get_checksum_algo(algo_name);
	info->crypto = image_get_crypto_algo(algo_name);
	info->padding = image_get_padding_algo(padding_name);
	info->fdt_blob = key_blob;
	info->required_keynode = required_keynode;
	printf("%s:%s", algo_name, info->keyname);

	if (!info->checksum || !info->crypto || !info->padding) {
		*err_msgp = "Unknown signature algorithm";
		return -1;
	}

	return 0;
}

int fit_image_check_sig(const void *fit, int noffset, const void *data,
			size_t size, const void *key_blob, int required_keynode,
			char **err_msgp)
{
	struct image_sign_info info;
	struct image_region region;
	uint8_t *fit_value;
	int fit_value_len;

	*err_msgp = NULL;
	if (fit_image_setup_verify(&info, fit, noffset, key_blob,
				   required_keynode, err_msgp))
		return -1;

	if (fit_image_hash_get_value(fit, noffset, &fit_value,
				     &fit_value_len)) {
		*err_msgp = "Can't get hash value property";
		return -1;
	}

	region.data = data;
	region.size = size;

	if (info.crypto->verify(&info, &region, 1, fit_value, fit_value_len)) {
		*err_msgp = "Verification failed";
		return -1;
	}

	return 0;
}

static int fit_image_verify_sig(const void *fit, int image_noffset,
				const char *data, size_t size,
				const void *key_blob, int key_offset)
{
	int noffset;
	char *err_msg = "";
	int verified = 0;
	int ret;

	/* Process all hash subnodes of the component image node */
	fdt_for_each_subnode(noffset, fit, image_noffset) {
		const char *name = fit_get_name(fit, noffset, NULL);

		/*
		 * We don't support this since libfdt considers names with the
		 * name root but different @ suffix to be equal
		 */
		if (strchr(name, '@')) {
			err_msg = "Node name contains @";
			goto error;
		}
		if (!strncmp(name, FIT_SIG_NODENAME,
			     strlen(FIT_SIG_NODENAME))) {
			ret = fit_image_check_sig(fit, noffset, data, size,
						  key_blob, -1, &err_msg);
			if (ret) {
				puts("- ");
			} else {
				puts("+ ");
				verified = 1;
				break;
			}
		}
	}

	if (noffset == -FDT_ERR_TRUNCATED || noffset == -FDT_ERR_BADSTRUCTURE) {
		err_msg = "Corrupted or truncated tree";
		goto error;
	}

	return verified ? 0 : -EPERM;

error:
	printf(" error!\n%s for '%s' hash node in '%s' image node\n",
	       err_msg, fit_get_name(fit, noffset, NULL),
	       fit_get_name(fit, image_noffset, NULL));
	return -1;
}

int fit_image_verify_required_sigs(const void *fit, int image_noffset,
				   const char *data, size_t size,
				   const void *key_blob, int *no_sigsp)
{
	int verify_count = 0;
	int noffset;
	int key_node;

	/* Work out what we need to verify */
	*no_sigsp = 1;
	key_node = fdt_subnode_offset(key_blob, 0, FIT_SIG_NODENAME);
	if (key_node < 0) {
		debug("%s: No signature node found: %s\n", __func__,
		      fdt_strerror(key_node));
		return 0;
	}

	fdt_for_each_subnode(noffset, key_blob, key_node) {
		const char *required;
		int ret;

		required = fdt_getprop(key_blob, noffset, FIT_KEY_REQUIRED,
				       NULL);
		if (!required || strcmp(required, "image"))
			continue;
		ret = fit_image_verify_sig(fit, image_noffset, data, size,
					   key_blob, noffset);
		if (ret) {
			printf("Failed to verify required signature '%s'\n",
			       fit_get_name(key_blob, noffset, NULL));
			return ret;
		}
		verify_count++;
	}

	if (verify_count)
		*no_sigsp = 0;

	return 0;
}

/**
 * fit_config_check_sig() - Check the signature of a config
 *
 * Here we are looking at a particular signature that needs verification (here
 * signature-1):
 *
 *	configurations {
 *		default = "conf-1";
 *		conf-1 {
 *			kernel = "kernel-1";
 *			fdt = "fdt-1";
 *			signature-1 {
 *				algo = "sha1,rsa2048";
 *				value = <...conf 1 signature...>;
 *			};
 *		};
 *
 * @fit: FIT to check
 * @noffset: Offset of the signature node being checked (e.g.
 *	 /configurations/conf-1/signature-1)
 * @conf_noffset: Offset of configuration node (e.g. /configurations/conf-1)
 * @key_blob: Blob containing the keys to check against
 * @required_keynode:	Offset in @key_blob of the required key node,
 *			if any. If this is given, then the configuration wil not
 *			pass verification unless that key is used. If this is
 *			-1 then any signature will do.
 * @err_msgp:		In the event of an error, this will be pointed to a
 *			help error string to display to the user.
 * Return: 0 if all verified ok, <0 on error
 */
static int fit_config_check_sig(const void *fit, int noffset, int conf_noffset,
				const void *key_blob, int required_keynode,
				char **err_msgp)
{
	static char * const exc_prop[] = {
		FIT_DATA_PROP,
		FIT_DATA_SIZE_PROP,
		FIT_DATA_POSITION_PROP,
		FIT_DATA_OFFSET_PROP,
	};

	const char *prop, *end, *name;
	struct image_sign_info info;
	const uint32_t *strings;
	const char *config_name;
	uint8_t *fit_value;
	int fit_value_len;
	bool found_config;
	int max_regions;
	int i, prop_len;
	char path[200];
	int count;

	config_name = fit_get_name(fit, conf_noffset, NULL);
	debug("%s: fdt=%p, conf='%s', sig='%s'\n", __func__, key_blob,
	      fit_get_name(fit, noffset, NULL),
	      fit_get_name(key_blob, required_keynode, NULL));
	*err_msgp = NULL;
	if (fit_image_setup_verify(&info, fit, noffset, key_blob,
				   required_keynode, err_msgp))
		return -1;

	if (fit_image_hash_get_value(fit, noffset, &fit_value,
				     &fit_value_len)) {
		*err_msgp = "Can't get hash value property";
		return -1;
	}

	/* Count the number of strings in the property */
	prop = fdt_getprop(fit, noffset, "hashed-nodes", &prop_len);
	end = prop ? prop + prop_len : prop;
	for (name = prop, count = 0; name < end; name++)
		if (!*name)
			count++;
	if (!count) {
		*err_msgp = "Can't get hashed-nodes property";
		return -1;
	}

	if (prop && prop_len > 0 && prop[prop_len - 1] != '\0') {
		*err_msgp = "hashed-nodes property must be null-terminated";
		return -1;
	}

	/* Add a sanity check here since we are using the stack */
	if (count > IMAGE_MAX_HASHED_NODES) {
		*err_msgp = "Number of hashed nodes exceeds maximum";
		return -1;
	}

	/* Create a list of node names from those strings */
	char *node_inc[count];

	debug("Hash nodes (%d):\n", count);
	found_config = false;
	for (name = prop, i = 0; name < end; name += strlen(name) + 1, i++) {
		debug("   '%s'\n", name);
		node_inc[i] = (char *)name;
		if (!strncmp(FIT_CONFS_PATH, name, strlen(FIT_CONFS_PATH)) &&
		    name[sizeof(FIT_CONFS_PATH) - 1] == '/' &&
		    !strcmp(name + sizeof(FIT_CONFS_PATH), config_name)) {
			debug("      (found config node %s)", config_name);
			found_config = true;
		}
	}
	if (!found_config) {
		*err_msgp = "Selected config not in hashed nodes";
		return -1;
	}

	/*
	 * Each node can generate one region for each sub-node. Allow for
	 * 7 sub-nodes (hash-1, signature-1, etc.) and some extra.
	 */
	max_regions = 20 + count * 7;
	struct fdt_region fdt_regions[max_regions];

	/* Get a list of regions to hash */
	count = fdt_find_regions(fit, node_inc, count,
				 exc_prop, ARRAY_SIZE(exc_prop),
				 fdt_regions, max_regions - 1,
				 path, sizeof(path), 0);
	if (count < 0) {
		*err_msgp = "Failed to hash configuration";
		return -1;
	}
	if (count == 0) {
		*err_msgp = "No data to hash";
		return -1;
	}
	if (count >= max_regions - 1) {
		*err_msgp = "Too many hash regions";
		return -1;
	}

	/* Add the strings */
	strings = fdt_getprop(fit, noffset, "hashed-strings", NULL);
	if (strings) {
		/*
		 * The strings region offset must be a static 0x0.
		 * This is set in tool/image-host.c
		 */
		fdt_regions[count].offset = fdt_off_dt_strings(fit);
		fdt_regions[count].size = fdt32_to_cpu(strings[1]);
		count++;
	}

	/* Allocate the region list on the stack */
	struct image_region region[count];

	fit_region_make_list(fit, fdt_regions, count, region);
	if (info.crypto->verify(&info, region, count, fit_value,
				fit_value_len)) {
		*err_msgp = "Verification failed";
		return -1;
	}

	return 0;
}

/**
 * fit_config_verify_key() - Verify that a configuration is signed with a key
 *
 * Here we are looking at a particular configuration that needs verification:
 *
 *	configurations {
 *		default = "conf-1";
 *		conf-1 {
 *			kernel = "kernel-1";
 *			fdt = "fdt-1";
 *			signature-1 {
 *				algo = "sha1,rsa2048";
 *				value = <...conf 1 signature...>;
 *			};
 *		};
 *
 * We must check each of the signature subnodes of conf-1. Hopefully one of them
 * will match the key at key_offset.
 *
 * @fit: FIT to check
 * @conf_noffset: Offset of the configuration node to check (e.g.
 *	/configurations/conf-1)
 * @key_blob: Blob containing the keys to check against
 * @key_offset: Offset of the key to check within @key_blob
 * @return 0 if OK, -EPERM if any signatures did not verify, or the
 *	configuration node has an invalid name
 */
static int fit_config_verify_key(const void *fit, int conf_noffset,
				 const void *key_blob, int key_offset)
{
	int noffset;
	char *err_msg = "No 'signature' subnode found";
	int verified = 0;
	int ret;

	/* Process all hash subnodes of the component conf node */
	fdt_for_each_subnode(noffset, fit, conf_noffset) {
		const char *name = fit_get_name(fit, noffset, NULL);

		if (!strncmp(name, FIT_SIG_NODENAME,
			     strlen(FIT_SIG_NODENAME))) {
			ret = fit_config_check_sig(fit, noffset, conf_noffset,
						   key_blob, key_offset,
						   &err_msg);
			if (ret) {
				puts("- ");
			} else {
				puts("+ ");
				verified = 1;
				break;
			}
		}
	}

	if (noffset == -FDT_ERR_TRUNCATED || noffset == -FDT_ERR_BADSTRUCTURE) {
		err_msg = "Corrupted or truncated tree";
		goto error;
	}

	if (verified)
		return 0;

error:
	printf(" error!\n%s for '%s' hash node in '%s' config node\n",
	       err_msg, fit_get_name(fit, noffset, NULL),
	       fit_get_name(fit, conf_noffset, NULL));
	return -EPERM;
}

/**
 * fit_config_verify_required_keys() - verify any required signatures for config
 *
 * This looks through all the signatures we expect and verifies that at least
 * all the required ones are valid signatures for the configuration
 *
 * @fit: FIT to check
 * @conf_noffset: Offset of the configuration node to check (e.g.
 *	/configurations/conf-1)
 * @key_blob: Blob containing the keys to check against
 * @return 0 if OK, -EPERM if any signatures did not verify, or the
 *	configuration node has an invalid name
 */
static int fit_config_verify_required_keys(const void *fit, int conf_noffset,
					   const void *key_blob)
{
	const char *name = fit_get_name(fit, conf_noffset, NULL);
	int noffset;
	int key_node;
	int verified = 0;
	int reqd_sigs = 0;
	bool reqd_policy_all = true;
	const char *reqd_mode;

	/*
	 * We don't support this since libfdt considers names with the
	 * name root but different @ suffix to be equal
	 */
	if (strchr(name, '@')) {
		printf("Configuration node '%s' contains '@'\n", name);
		return -EPERM;
	}

	/* Work out what we need to verify */
	key_node = fdt_subnode_offset(key_blob, 0, FIT_SIG_NODENAME);
	if (key_node < 0) {
		debug("%s: No signature node found: %s\n", __func__,
		      fdt_strerror(key_node));
		return 0;
	}

	/* Get required-mode policy property from DTB */
	reqd_mode = fdt_getprop(key_blob, key_node, "required-mode", NULL);
	if (reqd_mode && !strcmp(reqd_mode, "any"))
		reqd_policy_all = false;

	debug("%s: required-mode policy set to '%s'\n", __func__,
	      reqd_policy_all ? "all" : "any");

	/*
	 * The algorithm here is a little convoluted due to how we want it to
	 * work. Here we work through each of the signature nodes in the
	 * public-key area. These are in the U-Boot control devicetree. Each
	 * node was created by signing a configuration, so we check if it is
	 * 'required' and if so, request that it be verified.
	 */
	fdt_for_each_subnode(noffset, key_blob, key_node) {
		const char *required;
		int ret;

		required = fdt_getprop(key_blob, noffset, FIT_KEY_REQUIRED,
				       NULL);
		if (!required || strcmp(required, "conf"))
			continue;

		reqd_sigs++;

		ret = fit_config_verify_key(fit, conf_noffset, key_blob,
					    noffset);
		if (ret) {
			if (reqd_policy_all) {
				printf("Failed to verify required signature '%s'\n",
				       fit_get_name(key_blob, noffset, NULL));
				return ret;
			}
		} else {
			verified++;
			if (!reqd_policy_all)
				break;
		}
	}

	if (reqd_sigs && !verified) {
		printf("Failed to verify 'any' of the required signature(s)\n");
		return -EPERM;
	}

	return 0;
}

int fit_config_verify(const void *fit, int conf_noffset)
{
	return fit_config_verify_required_keys(fit, conf_noffset,
					       gd_fdt_blob());
}
