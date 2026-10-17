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
#include <fdt_region.h>
#include <image.h>
#include <version.h>

#if CONFIG_IS_ENABLED(FIT_SIGNATURE)
#include <openssl/pem.h>
#include <openssl/evp.h>
#endif

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
		fprintf(stderr, "Can't set hash '%s' property for '%s' node(%s)\n",
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
 * a hash of the supplied data and store it in the node.
 *
 * @fit:	pointer to the FIT format image header
 * @image_name:	name of image being processed (used to display errors)
 * @noffset:	subnode offset
 * @data:	data to process
 * @size:	size of data in bytes
 * Return: 0 if ok, -1 on error
 */
static int fit_image_process_hash(void *fit, const char *image_name,
		int noffset, const void *data, size_t size)
{
	uint8_t value[FIT_MAX_HASH_LEN];
	const char *node_name;
	int value_len;
	const char *algo;
	int ret;

	node_name = fit_get_name(fit, noffset, NULL);

	if (fit_image_hash_get_algo(fit, noffset, &algo)) {
		fprintf(stderr,
			"Can't get hash algo property for '%s' hash node in '%s' image node\n",
			node_name, image_name);
		return -ENOENT;
	}

	if (calculate_hash(data, size, algo, value, &value_len)) {
		fprintf(stderr,
			"Unsupported hash algorithm (%s) for '%s' hash node in '%s' image node\n",
			algo, node_name, image_name);
		return -EPROTONOSUPPORT;
	}

	ret = fit_set_hash_value(fit, noffset, value, value_len);
	if (ret) {
		fprintf(stderr, "Can't set hash value for '%s' hash node in '%s' image node\n",
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
		int region_proplen, const char *cmdname, const char *algo_name)
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
		uint32_t t = cpu_to_uimage(timestamp);

		ret = fdt_setprop(fit, noffset, FIT_TIMESTAMP_PROP, &t,
			sizeof(uint32_t));
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
	if (algo_name && !ret)
		ret = fdt_setprop_string(fit, noffset, "algo", algo_name);

	return ret;
}

static int fit_image_setup_sig(struct image_sign_info *info,
		const char *keydir, const char *keyfile, void *fit,
		const char *image_name, int noffset, const char *require_keys,
		const char *engine_id, const char *algo_name)
{
	const char *node_name;
	const char *padding_name;

	node_name = fit_get_name(fit, noffset, NULL);
	if (!algo_name) {
		if (fit_image_hash_get_algo(fit, noffset, &algo_name)) {
			fprintf(stderr,
				"Can't get algo property for '%s' signature node in '%s' image node\n",
				node_name, image_name);
			return -1;
		}
	}

	padding_name = fdt_getprop(fit, noffset, "padding", NULL);

	memset(info, '\0', sizeof(*info));
	info->keydir = keydir;
	info->keyfile = keyfile;
	info->keyname = fdt_getprop(fit, noffset, FIT_KEY_HINT, NULL);
	info->fit = fit;
	info->node_offset = noffset;
	info->name = strdup(algo_name);
	info->checksum = image_get_checksum_algo(algo_name);
	info->crypto = image_get_crypto_algo(algo_name);
	info->padding = image_get_padding_algo(padding_name);
	info->require_keys = require_keys;
	info->engine_id = engine_id;
	if (!info->checksum || !info->crypto) {
		fprintf(stderr,
			"Unsupported signature algorithm (%s) for '%s' signature node in '%s' image node\n",
			algo_name, node_name, image_name);
		return -1;
	}

	return 0;
}

/**
 * fit_image_process_sig- Process a single subnode of the images/ node
 *
 * Check each subnode and process accordingly. For signature nodes we
 * generate a signed hash of the supplied data and store it in the node.
 *
 * @keydir:	Directory containing keys to use for signing
 * @keydest:	Destination FDT blob to write public keys into (NULL if none)
 * @fit:	pointer to the FIT format image header
 * @image_name:	name of image being processed (used to display errors)
 * @noffset:	subnode offset
 * @data:	data to process
 * @size:	size of data in bytes
 * @comment:	Comment to add to signature nodes
 * @require_keys: Mark all keys as 'required'
 * @engine_id:	Engine to use for signing
 * Return: keydest node if @keydest is non-NULL, else 0 if none; -ve error code
 *	on failure
 */
static int fit_image_process_sig(const char *keydir, const char *keyfile,
		void *keydest, void *fit, const char *image_name,
		int noffset, const void *data, size_t size,
		const char *comment, int require_keys, const char *engine_id,
		const char *cmdname, const char *algo_name)
{
	struct image_sign_info info;
	struct image_region region;
	const char *node_name;
	uint8_t *value;
	uint value_len;
	int ret;

	if (fit_image_setup_sig(&info, keydir, keyfile, fit, image_name,
				noffset, require_keys ? "image" : NULL,
				engine_id, algo_name))
		return -1;

	node_name = fit_get_name(fit, noffset, NULL);
	region.data = data;
	region.size = size;
	ret = info.crypto->sign(&info, &region, 1, &value, &value_len);
	if (ret) {
		fprintf(stderr, "Failed to sign '%s' signature node in '%s' image node: %d\n",
			node_name, image_name, ret);

		/* We allow keys to be missing */
		if (ret == -ENOENT)
			return 0;
		return -1;
	}

	ret = fit_image_write_sig(fit, noffset, value, value_len, comment,
			NULL, 0, cmdname, algo_name);
	if (ret) {
		if (ret == -FDT_ERR_NOSPACE)
			return -ENOSPC;
		fprintf(stderr,
			"Can't write signature for '%s' signature node in '%s' conf node: %s\n",
			node_name, image_name, fdt_strerror(ret));
		return -1;
	}
	free(value);

	/* Get keyname again, as FDT has changed and invalidated our pointer */
	info.keyname = fdt_getprop(fit, noffset, FIT_KEY_HINT, NULL);

	/*
	 * Write the public key into the supplied FDT file; this might fail
	 * several times, since we try signing with successively increasing
	 * size values
	 */
	if (keydest) {
		ret = info.crypto->add_verify_data(&info, keydest);
		if (ret < 0) {
			fprintf(stderr,
				"Failed to add verification data for '%s' signature node in '%s' image node\n",
				node_name, image_name);
			return ret;
		}
		/* Return the node that was written to */
		return ret;
	}

	return 0;
}

static int fit_image_read_data(char *filename, unsigned char *data,
			       int expected_size)
{
	struct stat sbuf;
	int fd, ret = -1;
	ssize_t n;

	/* Open file */
	fd = open(filename, O_RDONLY | O_BINARY);
	if (fd < 0) {
		fprintf(stderr, "Can't open file %s (err=%d => %s)\n",
			filename, errno, strerror(errno));
		return -1;
	}

	/* Compute file size */
	if (fstat(fd, &sbuf) < 0) {
		fprintf(stderr, "Can't fstat file %s (err=%d => %s)\n",
			filename, errno, strerror(errno));
		goto err;
	}

	/* Check file size */
	if (sbuf.st_size != expected_size) {
		fprintf(stderr, "File %s don't have the expected size (size=%lld, expected=%d)\n",
			filename, (long long)sbuf.st_size, expected_size);
		goto err;
	}

	/* Read data */
	n = read(fd, data, sbuf.st_size);
	if (n < 0) {
		fprintf(stderr, "Can't read file %s (err=%d => %s)\n",
			filename, errno, strerror(errno));
		goto err;
	}

	/* Check that we have read all the file */
	if (n != sbuf.st_size) {
		fprintf(stderr, "Can't read all file %s (read %zd bytes, expected %lld)\n",
			filename, n, (long long)sbuf.st_size);
		goto err;
	}

	ret = 0;

err:
	close(fd);
	return ret;
}

static int fit_image_read_key_iv_data(const char *keydir, const char *key_iv_name,
				      unsigned char *key_iv_data, int expected_size)
{
	char filename[PATH_MAX];
	int ret = -1;

	ret = snprintf(filename, sizeof(filename), "%s/%s%s",
		       keydir, key_iv_name, ".bin");
	if (ret >= sizeof(filename)) {
		printf("Can't format the key or IV filename when setting up the cipher: insufficient buffer space\n");
		ret = -1;
	}
	if (ret < 0) {
		printf("Can't format the key or IV filename when setting up the cipher: snprintf error\n");
		ret = -1;
	}

	ret = fit_image_read_data(filename, key_iv_data, expected_size);

	return ret;
}

static int get_random_data(void *data, int size)
{
	unsigned char *tmp = data;
	struct timespec date;
	int i, ret;

	if (!tmp) {
		fprintf(stderr, "%s: pointer data is NULL\n", __func__);
		ret = -1;
		goto out;
	}

	ret = clock_gettime(CLOCK_MONOTONIC, &date);
	if (ret) {
		fprintf(stderr, "%s: clock_gettime has failed (%s)\n", __func__,
			strerror(errno));
		goto out;
	}

	srandom(date.tv_nsec);

	for (i = 0; i < size; i++) {
		*tmp = random() & 0xff;
		tmp++;
	}

 out:
	return ret;
}

static int fit_image_setup_cipher(struct image_cipher_info *info,
				  const char *keydir, void *fit,
				  const char *image_name, int image_noffset,
				  int noffset)
{
	char *algo_name;
	int ret = -1;

	if (fit_image_cipher_get_algo(fit, noffset, &algo_name)) {
		fprintf(stderr, "Can't get algo name for cipher in image '%s'\n",
			image_name);
		goto out;
	}

	info->keydir = keydir;

	/* Read the key name */
	info->keyname = fdt_getprop(fit, noffset, FIT_KEY_HINT, NULL);
	if (!info->keyname) {
		fprintf(stderr, "Can't get key name for cipher in image '%s'\n",
			image_name);
		goto out;
	}

	/*
	 * Read the IV name
	 *
	 * If this property is not provided then mkimage will generate
	 * a random IV and store it in the FIT image
	 */
	info->ivname = fdt_getprop(fit, noffset, "iv-name-hint", NULL);

	info->fit = fit;
	info->node_noffset = noffset;
	info->name = algo_name;

	info->cipher = image_get_cipher_algo(algo_name);
	if (!info->cipher) {
		fprintf(stderr, "Can't get algo for cipher '%s'\n", image_name);
		goto out;
	}

	info->key = malloc(info->cipher->key_len);
	if (!info->key) {
		fprintf(stderr, "Can't allocate memory for key\n");
		ret = -1;
		goto out;
	}

	/* Read the key in the file */
	ret = fit_image_read_key_iv_data(info->keydir, info->keyname,
					 (unsigned char *)info->key,
					 info->cipher->key_len);
	if (ret < 0)
		goto out;

	info->iv = malloc(info->cipher->iv_len);
	if (!info->iv) {
		fprintf(stderr, "Can't allocate memory for iv\n");
		ret = -1;
		goto out;
	}

	if (info->ivname) {
		/* Read the IV in the file */
		ret = fit_image_read_key_iv_data(info->keydir, info->ivname,
						 (unsigned char *)info->iv,
						 info->cipher->iv_len);
		if (ret < 0)
			goto out;
	} else {
		/* Generate an ramdom IV */
		ret = get_random_data((void *)info->iv, info->cipher->iv_len);
	}

 out:
	return ret;
}

int fit_image_write_cipher(void *fit, int image_noffset, int noffset,
			   const void *data, size_t size,
			   unsigned char *data_ciphered, int data_ciphered_len)
{
	int ret = -1;

	/* Replace data with ciphered data */
	ret = fdt_setprop(fit, image_noffset, FIT_DATA_PROP,
			  data_ciphered, data_ciphered_len);
	if (ret == -FDT_ERR_NOSPACE) {
		ret = -ENOSPC;
		goto out;
	}
	if (ret) {
		fprintf(stderr, "Can't replace data with ciphered data (err = %d)\n", ret);
		goto out;
	}

	/* add non ciphered data size */
	ret = fdt_setprop_u32(fit, image_noffset, "data-size-unciphered", size);
	if (ret == -FDT_ERR_NOSPACE) {
		ret = -ENOSPC;
		goto out;
	}
	if (ret) {
		fprintf(stderr, "Can't add unciphered data size (err = %d)\n", ret);
		goto out;
	}

 out:
	return ret;
}

static int
fit_image_process_cipher(const char *keydir, void *keydest, void *fit,
			 const char *image_name, int image_noffset,
			 int node_noffset, const void *data, size_t size,
			 const char *cmdname)
{
	struct image_cipher_info info;
	unsigned char *data_ciphered = NULL;
	int data_ciphered_len;
	int ret;

	memset(&info, 0, sizeof(info));

	ret = fit_image_setup_cipher(&info, keydir, fit, image_name,
				     image_noffset, node_noffset);
	if (ret)
		goto out;

	ret = info.cipher->encrypt(&info, data, size,
				    &data_ciphered, &data_ciphered_len);
	if (ret)
		goto out;

	/*
	 * Write the public key into the supplied FDT file; this might fail
	 * several times, since we try signing with successively increasing
	 * size values
	 * And, if needed, write the iv in the FIT file
	 */
	if (keydest) {
		ret = info.cipher->add_cipher_data(&info, keydest, fit, node_noffset);
		if (ret) {
			fprintf(stderr,
				"Failed to add verification data for cipher '%s' in image '%s'\n",
				info.keyname, image_name);
			goto out;
		}
	}

	ret = fit_image_write_cipher(fit, image_noffset, node_noffset,
				     data, size,
				     data_ciphered, data_ciphered_len);

 out:
	free(data_ciphered);
	free((void *)info.key);
	free((void *)info.iv);
	return ret;
}

int fit_image_cipher_data(const char *keydir, void *keydest,
			  void *fit, int image_noffset, const char *comment,
			  int require_keys, const char *engine_id,
			  const char *cmdname)
{
	const char *image_name;
	const void *data;
	size_t size;
	int cipher_node_offset, len;

	/* Get image name */
	image_name = fit_get_name(fit, image_noffset, NULL);
	if (!image_name) {
		fprintf(stderr, "Can't get image name\n");
		return -1;
	}

	/* Get image data and data length */
	if (fit_image_get_data(fit, image_noffset, &data, &size)) {
		fprintf(stderr, "Can't get image data/size\n");
		return -1;
	}

	/*
	 * Don't cipher ciphered data.
	 *
	 * If the data-size-unciphered property is present the data for this
	 * image is already encrypted. This is important as 'mkimage -F' can be
	 * run multiple times on a FIT image.
	 */
	if (fdt_getprop(fit, image_noffset, "data-size-unciphered", &len))
		return 0;
	if (len != -FDT_ERR_NOTFOUND) {
		fprintf(stderr, "Failure testing for data-size-unciphered\n");
		return -1;
	}

	/* Process cipher node if present */
	cipher_node_offset = fdt_subnode_offset(fit, image_noffset,
						FIT_CIPHER_NODENAME);
	if (cipher_node_offset == -FDT_ERR_NOTFOUND)
		return 0;
	if (cipher_node_offset < 0) {
		fprintf(stderr, "Failure getting cipher node\n");
		return -1;
	}
	if (!IMAGE_ENABLE_ENCRYPT || !keydir)
		return 0;
	return fit_image_process_cipher(keydir, keydest, fit, image_name,
		image_noffset, cipher_node_offset, data, size, cmdname);
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
int fit_image_add_verification_data(const char *keydir, const char *keyfile,
		void *keydest, void *fit, int image_noffset,
		const char *comment, int require_keys, const char *engine_id,
		const char *cmdname, const char* algo_name)
{
	const char *image_name;
	const void *data;
	size_t size;
	int noffset;

	/* Get image data and data length */
	if (fit_image_get_data(fit, image_noffset, &data, &size)) {
		fprintf(stderr, "Can't get image data/size\n");
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
		} else if (IMAGE_ENABLE_SIGN && (keydir || keyfile) &&
			   !strncmp(node_name, FIT_SIG_NODENAME,
				strlen(FIT_SIG_NODENAME))) {
			ret = fit_image_process_sig(keydir, keyfile, keydest,
				fit, image_name, noffset, data, size,
				comment, require_keys, engine_id, cmdname,
				algo_name);
		}
		if (ret < 0)
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

static const char *fit_config_get_image_list(const void *fit, int noffset,
					     int *lenp, int *allow_missingp)
{
	static const char default_list[] = FIT_KERNEL_PROP "\0"
			FIT_FDT_PROP;
	const char *prop;

	/* If there is an "sign-image" property, use that */
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

/**
 * fit_config_add_hash() - Add a list of nodes to hash for an image
 *
 * This adds a list of paths to image nodes (as referred to by a particular
 * offset) that need to be hashed, to protect a configuration
 *
 * @fit:	Pointer to the FIT format image header
 * @image_noffset: Offset of image to process (e.g. /images/kernel-1)
 * @node_inc:	List of nodes to add to
 * @conf_name	Configuration-node name, child of /configurations node (only
 *	used for error messages)
 * @sig_name	Signature-node name (only used for error messages)
 * @iname:	Name of image being processed (e.g. "kernel-1" (only used
 *	for error messages)
 */
static int fit_config_add_hash(const void *fit, int image_noffset,
			       struct strlist *node_inc, const char *conf_name,
			       const char *sig_name, const char *iname)
{
	char path[200];
	int noffset;
	int hash_count;
	int ret;

	ret = fdt_get_path(fit, image_noffset, path, sizeof(path));
	if (ret < 0)
		goto err_path;
	if (strlist_add(node_inc, path))
		goto err_mem;

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
		fprintf(stderr,
			"Failed to find any hash nodes in configuration '%s/%s' image '%s' - without these it is not possible to verify this image\n",
			conf_name, sig_name, iname);
		return -ENOMSG;
	}

	/* Add this image's cipher node if present */
	noffset = fdt_subnode_offset(fit, image_noffset,
				     FIT_CIPHER_NODENAME);
	if (noffset != -FDT_ERR_NOTFOUND) {
		if (noffset < 0) {
			fprintf(stderr,
				"Failed to get cipher node in configuration '%s/%s' image '%s': %s\n",
				conf_name, sig_name, iname,
				fdt_strerror(noffset));
			return -EIO;
		}
		ret = fdt_get_path(fit, noffset, path, sizeof(path));
		if (ret < 0)
			goto err_path;
		if (strlist_add(node_inc, path))
			goto err_mem;
	}

	return 0;

err_mem:
	fprintf(stderr, "Out of memory processing configuration '%s/%s'\n", conf_name,
		sig_name);
	return -ENOMEM;

err_path:
	fprintf(stderr, "Failed to get path for image '%s' in configuration '%s/%s': %s\n",
		iname, conf_name, sig_name, fdt_strerror(ret));
	return -ENOENT;
}

/**
 * fit_config_get_hash_list() - Get the regions to sign
 *
 * This calculates a list of nodes to hash for this particular configuration,
 * returning it as a string list (struct strlist, not a devicetree string list)
 *
 * @fit:	Pointer to the FIT format image header
 * @conf_noffset: Offset of configuration node to sign (child of
 *	/configurations node)
 * @sig_offset:	Offset of signature node containing info about how to sign it
 *	(child of 'signatures' node)
 * @return 0 if OK, -ENOENT if an image referred to by the configuration cannot
 *	be found, -ENOMSG if ther were no images in the configuration
 */
static int fit_config_get_hash_list(const void *fit, int conf_noffset,
				    int sig_offset, struct strlist *node_inc)
{
	int allow_missing;
	const char *prop, *iname, *end;
	const char *conf_name, *sig_name;
	char name[200];
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
		int image_noffset;
		int index, max_index;

		max_index = fdt_stringlist_count(fit, conf_noffset, iname);

		for (index = 0; index < max_index; index++) {
			image_noffset = fit_conf_get_prop_node_index(fit, conf_noffset,
								     iname, index);

			if (image_noffset < 0) {
				fprintf(stderr,
					"Failed to find image '%s' in  configuration '%s/%s'\n",
					iname, conf_name, sig_name);
				if (allow_missing)
					continue;

				return -ENOENT;
			}

			ret = fit_config_add_hash(fit, image_noffset, node_inc,
						  conf_name, sig_name, iname);
			if (ret < 0)
				return ret;

			image_count++;
		}
	}

	if (!image_count) {
		fprintf(stderr, "Failed to find any images for configuration '%s/%s'\n",
			conf_name, sig_name);
		return -ENOMSG;
	}

	return 0;

err_mem:
	fprintf(stderr, "Out of memory processing configuration '%s/%s'\n", conf_name,
		sig_name);
	return -ENOMEM;
}

/**
 * fit_config_get_regions() - Get the regions to sign
 *
 * This calculates a list of node to hash for this particular configuration,
 * then finds which regions of the devicetree they correspond to.
 *
 * @fit:	Pointer to the FIT format image header
 * @conf_noffset: Offset of configuration node to sign (child of
 *	/configurations node)
 * @sig_offset:	Offset of signature node containing info about how to sign it
 *	(child of 'signatures' node)
 * @regionp: Returns list of regions that need to be hashed (allocated; must be
 *	freed by the caller)
 * @region_count: Returns number of regions
 * @region_propp: Returns string-list property containing the list of nodes
 *	that correspond to the regions. Each entry is a full path to the node.
 *	This is in devicetree format, i.e. a \0 between each string. This is
 *	allocated and must be freed by the caller.
 * @region_proplen: Returns length of *@@region_propp in bytes
 * @return 0 if OK, -ENOMEM if out of memory, -EIO if the regions to hash could
 * not be found, -EINVAL if no registers were found to hash
 */
static int fit_config_get_regions(const void *fit, int conf_noffset,
				  int sig_offset, struct image_region **regionp,
				  int *region_countp, char **region_propp,
				  int *region_proplen)
{
	char * const exc_prop[] = {
		FIT_DATA_PROP,
		FIT_DATA_SIZE_PROP,
		FIT_DATA_POSITION_PROP,
		FIT_DATA_OFFSET_PROP,
	};
	struct strlist node_inc;
	struct image_region *region;
	struct fdt_region fdt_regions[100];
	const char *conf_name, *sig_name;
	char path[200];
	int count, i;
	char *region_prop;
	int ret, len;

	conf_name = fit_get_name(fit, conf_noffset, NULL);
	sig_name = fit_get_name(fit, sig_offset, NULL);
	debug("%s: conf='%s', sig='%s'\n", __func__, conf_name, sig_name);

	/* Get a list of nodes we want to hash */
	ret = fit_config_get_hash_list(fit, conf_noffset, sig_offset,
				       &node_inc);
	if (ret)
		return ret;

	/* Get a list of regions to hash */
	count = fdt_find_regions(fit, node_inc.strings, node_inc.count,
			exc_prop, ARRAY_SIZE(exc_prop),
			fdt_regions, ARRAY_SIZE(fdt_regions),
			path, sizeof(path), 1);
	if (count < 0) {
		fprintf(stderr, "Failed to hash configuration '%s/%s': %s\n", conf_name,
			sig_name, fdt_strerror(ret));
		return -EIO;
	}
	if (count == 0) {
		fprintf(stderr, "No data to hash for configuration '%s/%s': %s\n",
			conf_name, sig_name, fdt_strerror(ret));
		return -EINVAL;
	}

	/* Build our list of data blocks */
	region = fit_region_make_list(fit, fdt_regions, count, NULL);
	if (!region) {
		fprintf(stderr, "Out of memory hashing configuration '%s/%s'\n",
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
		fprintf(stderr, "Out of memory setting up regions for configuration '%s/%s'\n",
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

/**
 * fit_config_process_sig - Process a single subnode of the configurations/ node
 *
 * Generate a signed hash of the supplied data and store it in the node.
 *
 * @keydir:	Directory containing keys to use for signing
 * @keydest:	Destination FDT blob to write public keys into (NULL if none)
 * @fit:	pointer to the FIT format image header
 * @conf_name	name of config being processed (used to display errors)
 * @conf_noffset: Offset of configuration node, e.g. '/configurations/conf-1'
 * @noffset:	subnode offset, e.g. '/configurations/conf-1/sig-1'
 * @comment:	Comment to add to signature nodes
 * @require_keys: Mark all keys as 'required'
 * @engine_id:	Engine to use for signing
 * @cmdname:	Command name used when reporting errors
 * @return keydest node if @keydest is non-NULL, else 0 if none; -ve error code
 *	on failure
 */
static int fit_config_process_sig(const char *keydir, const char *keyfile,
		void *keydest, void *fit, const char *conf_name,
		int conf_noffset, int noffset, const char *comment,
		int require_keys, const char *engine_id, const char *cmdname,
		const char *algo_name)
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
	if (fit_config_get_regions(fit, conf_noffset, noffset, &region,
				   &region_count, &region_prop,
				   &region_proplen))
		return -1;

	if (fit_image_setup_sig(&info, keydir, keyfile, fit, conf_name, noffset,
				require_keys ? "conf" : NULL, engine_id,
				algo_name))
		return -1;

	ret = info.crypto->sign(&info, region, region_count, &value,
				&value_len);
	free(region);
	if (ret) {
		fprintf(stderr, "Failed to sign '%s' signature node in '%s' conf node\n",
			node_name, conf_name);

		/* We allow keys to be missing */
		if (ret == -ENOENT)
			return 0;
		return -1;
	}

	ret = fit_image_write_sig(fit, noffset, value, value_len, comment,
				  region_prop, region_proplen, cmdname,
				  algo_name);
	if (ret) {
		if (ret == -FDT_ERR_NOSPACE)
			return -ENOSPC;
		fprintf(stderr,
			"Can't write signature for '%s' signature node in '%s' conf node: %s\n",
			node_name, conf_name, fdt_strerror(ret));
		return -1;
	}
	free(value);
	free(region_prop);

	/* Get keyname again, as FDT has changed and invalidated our pointer */
	info.keyname = fdt_getprop(fit, noffset, FIT_KEY_HINT, NULL);

	/* Write the public key into the supplied FDT file */
	if (keydest) {
		ret = info.crypto->add_verify_data(&info, keydest);
		if (ret < 0) {
			fprintf(stderr,
				"Failed to add verification data for '%s' signature node in '%s' configuration node\n",
				node_name, conf_name);
		}
		return ret;
	}

	return 0;
}

static int fit_config_add_verification_data(const char *keydir,
		const char *keyfile, void *keydest, void *fit, int conf_noffset,
		const char *comment, int require_keys, const char *engine_id,
		const char *cmdname, const char *algo_name,
		struct image_summary *summary)
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
			ret = fit_config_process_sig(keydir, keyfile, keydest,
				fit, conf_name, conf_noffset, noffset, comment,
				require_keys, engine_id, cmdname, algo_name);
			if (ret < 0)
				return ret;

			summary->sig_offset = noffset;
			fdt_get_path(fit, noffset, summary->sig_path,
				     sizeof(summary->sig_path));

			if (keydest) {
				summary->keydest_offset = ret;
				fdt_get_path(keydest, ret,
					     summary->keydest_path,
					     sizeof(summary->keydest_path));
			}
		}
	}

	return 0;
}

#if CONFIG_IS_ENABLED(FIT_SIGNATURE)
/*
 * 0) open file (open)
 * 1) read certificate (PEM_read_X509)
 * 2) get public key (X509_get_pubkey)
 * 3) provide der format (d2i_RSAPublicKey)
 */
static int read_pub_key(const char *keydir, const void *name,
			unsigned char **pubkey, int *pubkey_len)
{
	char path[1024];
	EVP_PKEY *key = NULL;
	X509 *cert;
	FILE *f;
	int ret;

	memset(path, 0, 1024);
	snprintf(path, sizeof(path), "%s/%s.crt", keydir, (char *)name);

	/* Open certificate file */
	f = fopen(path, "r");
	if (!f) {
		fprintf(stderr, "Couldn't open RSA certificate: '%s': %s\n",
			path, strerror(errno));
		return -EACCES;
	}

	/* Read the certificate */
	cert = NULL;
	if (!PEM_read_X509(f, &cert, NULL, NULL)) {
		fprintf(stderr, "Couldn't read certificate");
		ret = -EINVAL;
		goto err_cert;
	}

	/* Get the public key from the certificate. */
	key = X509_get_pubkey(cert);
	if (!key) {
		fprintf(stderr, "Couldn't read public key\n");
		ret = -EINVAL;
		goto err_pubkey;
	}

	/* Get DER form */
	ret = i2d_PublicKey(key, pubkey);
	if (ret < 0) {
		fprintf(stderr, "Couldn't get DER form\n");
		ret = -EINVAL;
		goto err_pubkey;
	}

	*pubkey_len = ret;
	ret = 0;

err_pubkey:
	X509_free(cert);
err_cert:
	fclose(f);
	return ret;
}

int fit_pre_load_data(const char *keydir, void *keydest, void *fit)
{
	int pre_load_noffset;
	const void *algo_name;
	const void *key_name;
	unsigned char *pubkey = NULL;
	int ret, pubkey_len;

	if (!keydir || !keydest || !fit)
		return 0;

	/* Search node pre-load sig */
	pre_load_noffset = fdt_path_offset(keydest, IMAGE_PRE_LOAD_PATH);
	if (pre_load_noffset < 0) {
		ret = 0;
		goto out;
	}

	algo_name = fdt_getprop(keydest, pre_load_noffset, "algo-name", NULL);
	key_name  = fdt_getprop(keydest, pre_load_noffset, "key-name", NULL);

	/* Check that all mandatory properties are present */
	if (!algo_name || !key_name) {
		if (!algo_name)
			fprintf(stderr, "The property algo-name is missing in the node %s\n",
				IMAGE_PRE_LOAD_PATH);
		if (!key_name)
			fprintf(stderr, "The property key-name is missing in the node %s\n",
				IMAGE_PRE_LOAD_PATH);
		ret = -EINVAL;
		goto out;
	}

	/* Read public key */
	ret = read_pub_key(keydir, key_name, &pubkey, &pubkey_len);
	if (ret < 0)
		goto out;

	/* Add the public key to the device tree */
	ret = fdt_setprop(keydest, pre_load_noffset, "public-key",
			  pubkey, pubkey_len);
	if (ret)
		fprintf(stderr, "Can't set public-key in node %s (ret = %d)\n",
			IMAGE_PRE_LOAD_PATH, ret);

 out:
	return ret;
}
#endif

int fit_cipher_data(const char *keydir, void *keydest, void *fit,
		    const char *comment, int require_keys,
		    const char *engine_id, const char *cmdname)
{
	int images_noffset;
	int noffset;
	int ret;

	/* Find images parent node offset */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images_noffset < 0) {
		fprintf(stderr, "Can't find images parent node '%s' (%s)\n",
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
		ret = fit_image_cipher_data(keydir, keydest,
					    fit, noffset, comment,
					    require_keys, engine_id,
					    cmdname);
		if (ret)
			return ret;
	}

	return 0;
}

int fit_add_verification_data(const char *keydir, const char *keyfile,
			      void *keydest, void *fit, const char *comment,
			      int require_keys, const char *engine_id,
			      const char *cmdname, const char *algo_name,
			      struct image_summary *summary)
{
	int images_noffset, confs_noffset;
	int noffset;
	int ret;

	/* Find images parent node offset */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images_noffset < 0) {
		fprintf(stderr, "Can't find images parent node '%s' (%s)\n",
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
		ret = fit_image_add_verification_data(keydir, keyfile, keydest,
				fit, noffset, comment, require_keys, engine_id,
				cmdname, algo_name);
		if (ret) {
			fprintf(stderr, "Can't add verification data for node '%s' (%s)\n",
				fdt_get_name(fit, noffset, NULL),
				fdt_strerror(ret));
			return ret;
		}
	}

	/* If there are no keys, we can't sign configurations */
	if (!IMAGE_ENABLE_SIGN || !(keydir || keyfile))
		return 0;

	/* Find configurations parent node offset */
	confs_noffset = fdt_path_offset(fit, FIT_CONFS_PATH);
	if (confs_noffset < 0) {
		fprintf(stderr, "Can't find images parent node '%s' (%s)\n",
			FIT_CONFS_PATH, fdt_strerror(confs_noffset));
		return -ENOENT;
	}

	/* Process its subnodes, print out component images details */
	for (noffset = fdt_first_subnode(fit, confs_noffset);
	     noffset >= 0;
	     noffset = fdt_next_subnode(fit, noffset)) {
		ret = fit_config_add_verification_data(keydir, keyfile, keydest,
						       fit, noffset, comment,
						       require_keys,
						       engine_id, cmdname,
						       algo_name, summary);
		if (ret)
			return ret;
	}

	return 0;
}

#ifdef CONFIG_FIT_SIGNATURE
int fit_check_sign(const void *fit, const void *key,
		   const char *fit_uname_config)
{
	int cfg_noffset;
	int ret;

	cfg_noffset = fit_conf_get_node(fit, fit_uname_config);
	if (!cfg_noffset)
		return -1;

	printf("Verifying Hash Integrity for node '%s'... ",
	       fdt_get_name(fit, cfg_noffset, NULL));
	ret = fit_config_verify(fit, cfg_noffset);
	if (ret)
		return ret;
	printf("Verified OK, loading images\n");
	ret = bootm_host_load_images(fit, cfg_noffset);

	return ret;
}
#endif
