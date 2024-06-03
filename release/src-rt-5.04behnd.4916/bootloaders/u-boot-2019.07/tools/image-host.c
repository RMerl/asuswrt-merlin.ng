// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013, Google Inc.
 *
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include "mkimage.h"
#include <bootm.h>
#include <image.h>
#include <version.h>

/**
 * fit_set_hash_value - set hash value in requested has node
 * @fit: pointer to the FIT format image header
 * @noffset: hash node offset
 * @value: hash value to be set
 * @value_len: hash value length
 *
 * fit_set_hash_value() attempts to set hash value in a node at offset
 * given and returns operation status to the caller.
 *
 * returns
 *     0, on success
 *     -1, on failure
 */
static int fit_set_hash_value(void *fit, int noffset, uint8_t *value,
				int value_len)
{
	int ret;
	(void)fdt_delprop(fit, noffset, FIT_VALUE_PROP);
	ret = fdt_setprop(fit, noffset, FIT_VALUE_PROP, value, value_len);
	if (ret) {
		printf("Can't set hash '%s' property for '%s' node(%s)\n",
		       FIT_VALUE_PROP, fit_get_name(fit, noffset, NULL),
		       fdt_strerror(ret));
		return ret == -FDT_ERR_NOSPACE ? -ENOSPC : -EIO;
	}

	return 0;
}

/**
 * fit_image_process_hash - Process a single subnode of the images/ node
 *
 * Check each subnode and process accordingly. For hash nodes we generate
 * a hash of the supplised data and store it in the node.
 *
 * @fit:	pointer to the FIT format image header
 * @image_name:	name of image being processes (used to display errors)
 * @noffset:	subnode offset
 * @data:	data to process
 * @size:	size of data in bytes
 * @return 0 if ok, -1 on error
 */
static int fit_image_process_hash(void *fit, const char *image_name,
		int noffset, const void *data, size_t size)
{
	uint8_t value[FIT_MAX_HASH_LEN];
	const char *node_name;
	int value_len;
	char *algo;
	int ret;

	node_name = fit_get_name(fit, noffset, NULL);

	if (fit_image_hash_get_algo(fit, noffset, &algo)) {
		printf("Can't get hash algo property for '%s' hash node in '%s' image node\n",
		       node_name, image_name);
		return -ENOENT;
	}

	if (calculate_hash(data, size, algo, value, &value_len)) {
		printf("Unsupported hash algorithm (%s) for '%s' hash node in '%s' image node\n",
		       algo, node_name, image_name);
		return -EPROTONOSUPPORT;
	}

	ret = fit_set_hash_value(fit, noffset, value, value_len);
	if (ret) {
		printf("Can't set hash value for '%s' hash node in '%s' image node\n",
		       node_name, image_name);
		return ret;
	}

	return 0;
}

/**
 * fit_image_write_sig() - write the signature to a FIT
 *
 * This writes the signature and signer data to the FIT.
 *
 * @fit: pointer to the FIT format image header
 * @noffset: hash node offset
 * @value: signature value to be set
 * @value_len: signature value length
 * @comment: Text comment to write (NULL for none)
 *
 * returns
 *     0, on success
 *     -FDT_ERR_..., on failure
 */
static int fit_image_write_sig(void *fit, int noffset, uint8_t *value,
		int value_len, const char *comment, const char *region_prop,
		int region_proplen, const char *cmdname)
{
	int string_size;
	int ret;

	/*
	 * Get the current string size, before we update the FIT and add
	 * more
	 */
	string_size = fdt_size_dt_strings(fit);

	ret = fdt_setprop(fit, noffset, FIT_VALUE_PROP, value, value_len);
	if (!ret) {
		ret = fdt_setprop_string(fit, noffset, "signer-name",
					 "mkimage");
	}
	if (!ret) {
		ret = fdt_setprop_string(fit, noffset, "signer-version",
				  PLAIN_VERSION);
	}
	if (comment && !ret)
		ret = fdt_setprop_string(fit, noffset, "comment", comment);
	if (!ret) {
		time_t timestamp = imagetool_get_source_date(cmdname,
							     time(NULL));

		ret = fit_set_timestamp(fit, noffset, timestamp);
	}
	if (region_prop && !ret) {
		uint32_t strdata[2];

		ret = fdt_setprop(fit, noffset, "hashed-nodes",
				   region_prop, region_proplen);
		/* This is a legacy offset, it is unused, and must remain 0. */
		strdata[0] = 0;
		strdata[1] = cpu_to_fdt32(string_size);
		if (!ret) {
			ret = fdt_setprop(fit, noffset, "hashed-strings",
					  strdata, sizeof(strdata));
		}
	}

	return ret;
}

static int fit_image_setup_sig(struct image_sign_info *info,
		const char *keydir, void *fit, const char *image_name,
		int noffset, const char *require_keys, const char *engine_id)
{
	const char *node_name;
	char *algo_name;
	const char *padding_name;

	node_name = fit_get_name(fit, noffset, NULL);
	if (fit_image_hash_get_algo(fit, noffset, &algo_name)) {
		printf("Can't get algo property for '%s' signature node in '%s' image node\n",
		       node_name, image_name);
		return -1;
	}

	padding_name = fdt_getprop(fit, noffset, "padding", NULL);

	memset(info, '\0', sizeof(*info));
	info->keydir = keydir;
	info->keyname = fdt_getprop(fit, noffset, "key-name-hint", NULL);
	info->fit = fit;
	info->node_offset = noffset;
	info->name = strdup(algo_name);
	info->checksum = image_get_checksum_algo(algo_name);
	info->crypto = image_get_crypto_algo(algo_name);
	info->padding = image_get_padding_algo(padding_name);
	info->require_keys = require_keys;
	info->engine_id = engine_id;
	if (!info->checksum || !info->crypto) {
		printf("Unsupported signature algorithm (%s) for '%s' signature node in '%s' image node\n",
		       algo_name, node_name, image_name);
		return -1;
	}

	return 0;
}

/**
 * fit_image_process_sig- Process a single subnode of the images/ node
 *
 * Check each subnode and process accordingly. For signature nodes we
 * generate a signed hash of the supplised data and store it in the node.
 *
 * @keydir:	Directory containing keys to use for signing
 * @keydest:	Destination FDT blob to write public keys into
 * @fit:	pointer to the FIT format image header
 * @image_name:	name of image being processes (used to display errors)
 * @noffset:	subnode offset
 * @data:	data to process
 * @size:	size of data in bytes
 * @comment:	Comment to add to signature nodes
 * @require_keys: Mark all keys as 'required'
 * @engine_id:	Engine to use for signing
 * @return 0 if ok, -1 on error
 */
static int fit_image_process_sig(const char *keydir, void *keydest,
		void *fit, const char *image_name,
		int noffset, const void *data, size_t size,
		const char *comment, int require_keys, const char *engine_id,
		const char *cmdname)
{
	struct image_sign_info info;
	struct image_region region;
	const char *node_name;
	uint8_t *value;
	uint value_len;
	int ret;

	if (fit_image_setup_sig(&info, keydir, fit, image_name, noffset,
				require_keys ? "image" : NULL, engine_id))
		return -1;

	node_name = fit_get_name(fit, noffset, NULL);
	region.data = data;
	region.size = size;
	ret = info.crypto->sign(&info, &region, 1, &value, &value_len);
	if (ret) {
		printf("Failed to sign '%s' signature node in '%s' image node: %d\n",
		       node_name, image_name, ret);

		/* We allow keys to be missing */
		if (ret == -ENOENT)
			return 0;
		return -1;
	}

	ret = fit_image_write_sig(fit, noffset, value, value_len, comment,
			NULL, 0, cmdname);
	if (ret) {
		if (ret == -FDT_ERR_NOSPACE)
			return -ENOSPC;
		printf("Can't write signature for '%s' signature node in '%s' conf node: %s\n",
		       node_name, image_name, fdt_strerror(ret));
		return -1;
	}
	free(value);

	/* Get keyname again, as FDT has changed and invalidated our pointer */
	info.keyname = fdt_getprop(fit, noffset, "key-name-hint", NULL);

	/*
	 * Write the public key into the supplied FDT file; this might fail
	 * several times, since we try signing with successively increasing
	 * size values
	 */
	if (keydest) {
		ret = info.crypto->add_verify_data(&info, keydest);
		if (ret) {
			printf("Failed to add verification data for '%s' signature node in '%s' image node\n",
			       node_name, image_name);
			return ret;
		}
	}

	return 0;
}

/**
 * fit_image_add_verification_data() - calculate/set verig. data for image node
 *
 * This adds hash and signature values for an component image node.
 *
 * All existing hash subnodes are checked, if algorithm property is set to
 * one of the supported hash algorithms, hash value is computed and
 * corresponding hash node property is set, for example:
 *
 * Input component image node structure:
 *
 * o image-1 (at image_noffset)
 *   | - data = [binary data]
 *   o hash-1
 *     |- algo = "sha1"
 *
 * Output component image node structure:
 *
 * o image-1 (at image_noffset)
 *   | - data = [binary data]
 *   o hash-1
 *     |- algo = "sha1"
 *     |- value = sha1(data)
 *
 * For signature details, please see doc/uImage.FIT/signature.txt
 *
 * @keydir	Directory containing *.key and *.crt files (or NULL)
 * @keydest	FDT Blob to write public keys into (NULL if none)
 * @fit:	Pointer to the FIT format image header
 * @image_noffset: Requested component image node
 * @comment:	Comment to add to signature nodes
 * @require_keys: Mark all keys as 'required'
 * @engine_id:	Engine to use for signing
 * @return: 0 on success, <0 on failure
 */
int fit_image_add_verification_data(const char *keydir, void *keydest,
		void *fit, int image_noffset, const char *comment,
		int require_keys, const char *engine_id, const char *cmdname)
{
	const char *image_name;
	const void *data;
	size_t size;
	int noffset;

	/* Get image data and data length */
	if (fit_image_get_data(fit, image_noffset, &data, &size)) {
		printf("Can't get image data/size\n");
		return -1;
	}

	image_name = fit_get_name(fit, image_noffset, NULL);

	/* Process all hash subnodes of the component image node */
	for (noffset = fdt_first_subnode(fit, image_noffset);
	     noffset >= 0;
	     noffset = fdt_next_subnode(fit, noffset)) {
		const char *node_name;
		int ret = 0;

		/*
		 * Check subnode name, must be equal to "hash" or "signature".
		 * Multiple hash nodes require unique unit node
		 * names, e.g. hash-1, hash-2, signature-1, etc.
		 */
		node_name = fit_get_name(fit, noffset, NULL);
		if (!strncmp(node_name, FIT_HASH_NODENAME,
			     strlen(FIT_HASH_NODENAME))) {
			ret = fit_image_process_hash(fit, image_name, noffset,
						data, size);
		} else if (IMAGE_ENABLE_SIGN && keydir &&
			   !strncmp(node_name, FIT_SIG_NODENAME,
				strlen(FIT_SIG_NODENAME))) {
			ret = fit_image_process_sig(keydir, keydest,
				fit, image_name, noffset, data, size,
				comment, require_keys, engine_id, cmdname);
		}
		if (ret)
			return ret;
	}

	return 0;
}

struct strlist {
	int count;
	char **strings;
};

static void strlist_init(struct strlist *list)
{
	memset(list, '\0', sizeof(*list));
}

static void strlist_free(struct strlist *list)
{
	int i;

	for (i = 0; i < list->count; i++)
		free(list->strings[i]);
	free(list->strings);
}

static int strlist_add(struct strlist *list, const char *str)
{
	char *dup;

	dup = strdup(str);
	list->strings = realloc(list->strings,
				(list->count + 1) * sizeof(char *));
	if (!list || !str)
		return -1;
	list->strings[list->count++] = dup;

	return 0;
}

static const char *fit_config_get_image_list(void *fit, int noffset,
		int *lenp, int *allow_missingp)
{
	static const char default_list[] = FIT_KERNEL_PROP "\0"
			FIT_FDT_PROP;
	const char *prop;

	/* If there is an "image" property, use that */
	prop = fdt_getprop(fit, noffset, "sign-images", lenp);
	if (prop) {
		*allow_missingp = 0;
		return *lenp ? prop : NULL;
	}

	/* Default image list */
	*allow_missingp = 1;
	*lenp = sizeof(default_list);

	return default_list;
}

static int fit_config_get_hash_list(void *fit, int conf_noffset,
				    int sig_offset, struct strlist *node_inc)
{
	int allow_missing;
	const char *prop, *iname, *end;
	const char *conf_name, *sig_name;
	char name[200], path[200];
	int image_count;
	int ret, len;

	conf_name = fit_get_name(fit, conf_noffset, NULL);
	sig_name = fit_get_name(fit, sig_offset, NULL);

	/*
	 * Build a list of nodes we need to hash. We always need the root
	 * node and the configuration.
	 */
	strlist_init(node_inc);
	snprintf(name, sizeof(name), "%s/%s", FIT_CONFS_PATH, conf_name);
	if (strlist_add(node_inc, "/") ||
	    strlist_add(node_inc, name))
		goto err_mem;

	/* Get a list of images that we intend to sign */
	prop = fit_config_get_image_list(fit, sig_offset, &len,
					&allow_missing);
	if (!prop)
		return 0;

	/* Locate the images */
	end = prop + len;
	image_count = 0;
	for (iname = prop; iname < end; iname += strlen(iname) + 1) {
		int noffset;
		int image_noffset;
		int hash_count;

		image_noffset = fit_conf_get_prop_node(fit, conf_noffset,
						       iname);
		if (image_noffset < 0) {
			printf("Failed to find image '%s' in  configuration '%s/%s'\n",
			       iname, conf_name, sig_name);
			if (allow_missing)
				continue;

			return -ENOENT;
		}

		ret = fdt_get_path(fit, image_noffset, path, sizeof(path));
		if (ret < 0)
			goto err_path;
		if (strlist_add(node_inc, path))
			goto err_mem;

		snprintf(name, sizeof(name), "%s/%s", FIT_CONFS_PATH,
			 conf_name);

		/* Add all this image's hashes */
		hash_count = 0;
		for (noffset = fdt_first_subnode(fit, image_noffset);
		     noffset >= 0;
		     noffset = fdt_next_subnode(fit, noffset)) {
			const char *name = fit_get_name(fit, noffset, NULL);

			if (strncmp(name, FIT_HASH_NODENAME,
				    strlen(FIT_HASH_NODENAME)))
				continue;
			ret = fdt_get_path(fit, noffset, path, sizeof(path));
			if (ret < 0)
				goto err_path;
			if (strlist_add(node_inc, path))
				goto err_mem;
			hash_count++;
		}

		if (!hash_count) {
			printf("Failed to find any hash nodes in configuration '%s/%s' image '%s' - without these it is not possible to verify this image\n",
			       conf_name, sig_name, iname);
			return -ENOMSG;
		}

		image_count++;
	}

	if (!image_count) {
		printf("Failed to find any images for configuration '%s/%s'\n",
		       conf_name, sig_name);
		return -ENOMSG;
	}

	return 0;

err_mem:
	printf("Out of memory processing configuration '%s/%s'\n", conf_name,
	       sig_name);
	return -ENOMEM;

err_path:
	printf("Failed to get path for image '%s' in configuration '%s/%s': %s\n",
	       iname, conf_name, sig_name, fdt_strerror(ret));
	return -ENOENT;
}

static int fit_config_get_data(void *fit, int conf_noffset, int noffset,
		struct image_region **regionp, int *region_countp,
		char **region_propp, int *region_proplen)
{
	char * const exc_prop[] = {"data"};
	struct strlist node_inc;
	struct image_region *region;
	struct fdt_region fdt_regions[100];
	const char *conf_name, *sig_name;
	char path[200];
	int count, i;
	char *region_prop;
	int ret, len;

	conf_name = fit_get_name(fit, conf_noffset, NULL);
	sig_name = fit_get_name(fit, noffset, NULL);
	debug("%s: conf='%s', sig='%s'\n", __func__, conf_name, sig_name);

	/* Get a list of nodes we want to hash */
	ret = fit_config_get_hash_list(fit, conf_noffset, noffset, &node_inc);
	if (ret)
		return ret;

	/* Get a list of regions to hash */
	count = fdt_find_regions(fit, node_inc.strings, node_inc.count,
			exc_prop, ARRAY_SIZE(exc_prop),
			fdt_regions, ARRAY_SIZE(fdt_regions),
			path, sizeof(path), 1);
	if (count < 0) {
		printf("Failed to hash configuration '%s/%s': %s\n", conf_name,
		       sig_name, fdt_strerror(ret));
		return -EIO;
	}
	if (count == 0) {
		printf("No data to hash for configuration '%s/%s': %s\n",
		       conf_name, sig_name, fdt_strerror(ret));
		return -EINVAL;
	}

	/* Build our list of data blocks */
	region = fit_region_make_list(fit, fdt_regions, count, NULL);
	if (!region) {
		printf("Out of memory hashing configuration '%s/%s'\n",
		       conf_name, sig_name);
		return -ENOMEM;
	}

	/* Create a list of all hashed properties */
	debug("Hash nodes:\n");
	for (i = len = 0; i < node_inc.count; i++) {
		debug("   %s\n", node_inc.strings[i]);
		len += strlen(node_inc.strings[i]) + 1;
	}
	region_prop = malloc(len);
	if (!region_prop) {
		printf("Out of memory setting up regions for configuration '%s/%s'\n",
		       conf_name, sig_name);
		return -ENOMEM;
	}
	for (i = len = 0; i < node_inc.count;
	     len += strlen(node_inc.strings[i]) + 1, i++)
		strcpy(region_prop + len, node_inc.strings[i]);
	strlist_free(&node_inc);

	*region_countp = count;
	*regionp = region;
	*region_propp = region_prop;
	*region_proplen = len;

	return 0;
}

static int fit_config_process_sig(const char *keydir, void *keydest,
		void *fit, const char *conf_name, int conf_noffset,
		int noffset, const char *comment, int require_keys,
		const char *engine_id, const char *cmdname)
{
	struct image_sign_info info;
	const char *node_name;
	struct image_region *region;
	char *region_prop;
	int region_proplen;
	int region_count;
	uint8_t *value;
	uint value_len;
	int ret;

	node_name = fit_get_name(fit, noffset, NULL);
	if (fit_config_get_data(fit, conf_noffset, noffset, &region,
				&region_count, &region_prop, &region_proplen))
		return -1;

	if (fit_image_setup_sig(&info, keydir, fit, conf_name, noffset,
				require_keys ? "conf" : NULL, engine_id))
		return -1;

	ret = info.crypto->sign(&info, region, region_count, &value,
				&value_len);
	free(region);
	if (ret) {
		printf("Failed to sign '%s' signature node in '%s' conf node\n",
		       node_name, conf_name);

		/* We allow keys to be missing */
		if (ret == -ENOENT)
			return 0;
		return -1;
	}

	ret = fit_image_write_sig(fit, noffset, value, value_len, comment,
				region_prop, region_proplen, cmdname);
	if (ret) {
		if (ret == -FDT_ERR_NOSPACE)
			return -ENOSPC;
		printf("Can't write signature for '%s' signature node in '%s' conf node: %s\n",
		       node_name, conf_name, fdt_strerror(ret));
		return -1;
	}
	free(value);
	free(region_prop);

	/* Get keyname again, as FDT has changed and invalidated our pointer */
	info.keyname = fdt_getprop(fit, noffset, "key-name-hint", NULL);

	/* Write the public key into the supplied FDT file */
	if (keydest) {
		ret = info.crypto->add_verify_data(&info, keydest);
		if (ret) {
			printf("Failed to add verification data for '%s' signature node in '%s' configuration node\n",
			       node_name, conf_name);
		}
		return ret;
	}

	return 0;
}

static int fit_config_add_verification_data(const char *keydir, void *keydest,
		void *fit, int conf_noffset, const char *comment,
		int require_keys, const char *engine_id, const char *cmdname)
{
	const char *conf_name;
	int noffset;

	conf_name = fit_get_name(fit, conf_noffset, NULL);

	/* Process all hash subnodes of the configuration node */
	for (noffset = fdt_first_subnode(fit, conf_noffset);
	     noffset >= 0;
	     noffset = fdt_next_subnode(fit, noffset)) {
		const char *node_name;
		int ret = 0;

		node_name = fit_get_name(fit, noffset, NULL);
		if (!strncmp(node_name, FIT_SIG_NODENAME,
			     strlen(FIT_SIG_NODENAME))) {
			ret = fit_config_process_sig(keydir, keydest,
				fit, conf_name, conf_noffset, noffset, comment,
				require_keys, engine_id, cmdname);
		}
		if (ret)
			return ret;
	}

	return 0;
}

int fit_add_verification_data(const char *keydir, void *keydest, void *fit,
			      const char *comment, int require_keys,
			      const char *engine_id, const char *cmdname)
{
	int images_noffset, confs_noffset;
	int noffset;
	int ret;

	/* Find images parent node offset */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images_noffset < 0) {
		printf("Can't find images parent node '%s' (%s)\n",
		       FIT_IMAGES_PATH, fdt_strerror(images_noffset));
		return images_noffset;
	}

	/* Process its subnodes, print out component images details */
	for (noffset = fdt_first_subnode(fit, images_noffset);
	     noffset >= 0;
	     noffset = fdt_next_subnode(fit, noffset)) {
		/*
		 * Direct child node of the images parent node,
		 * i.e. component image node.
		 */
		ret = fit_image_add_verification_data(keydir, keydest,
				fit, noffset, comment, require_keys, engine_id,
				cmdname);
		if (ret)
			return ret;
	}

	/* If there are no keys, we can't sign configurations */
	if (!IMAGE_ENABLE_SIGN || !keydir)
		return 0;

	/* Find configurations parent node offset */
	confs_noffset = fdt_path_offset(fit, FIT_CONFS_PATH);
	if (confs_noffset < 0) {
		printf("Can't find images parent node '%s' (%s)\n",
		       FIT_CONFS_PATH, fdt_strerror(confs_noffset));
		return -ENOENT;
	}

	/* Process its subnodes, print out component images details */
	for (noffset = fdt_first_subnode(fit, confs_noffset);
	     noffset >= 0;
	     noffset = fdt_next_subnode(fit, noffset)) {
		ret = fit_config_add_verification_data(keydir, keydest,
						       fit, noffset, comment,
						       require_keys,
						       engine_id, cmdname);
		if (ret)
			return ret;
	}

	return 0;
}

#ifdef CONFIG_FIT_SIGNATURE
int fit_check_sign(const void *fit, const void *key)
{
	int cfg_noffset;
	int ret;

	cfg_noffset = fit_conf_get_node(fit, NULL);
	if (!cfg_noffset)
		return -1;

	printf("Verifying Hash Integrity ... ");
	ret = fit_config_verify(fit, cfg_noffset);
	if (ret)
		return ret;
	ret = bootm_host_load_images(fit, cfg_noffset);

	return ret;
}
#endif
