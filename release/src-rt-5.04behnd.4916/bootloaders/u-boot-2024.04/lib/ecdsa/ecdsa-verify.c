// SPDX-License-Identifier: GPL-2.0+
/*
 * ECDSA signature verification for u-boot
 *
 * This implements the firmware-side wrapper for ECDSA verification. It bridges
 * the struct crypto_algo API to the ECDSA uclass implementations.
 *
 * Copyright (c) 2020, Alexandru Gagniuc <mr.nuke.me@gmail.com>
 */

#include <crypto/ecdsa-uclass.h>
#include <dm/uclass.h>
#include <u-boot/ecdsa.h>

/*
 * Derive size of an ECDSA key from the curve name
 *
 * While it's possible to extract the key size by using string manipulation,
 * use a list of known curves for the time being.
 */
static int ecdsa_key_size(const char *curve_name)
{
	if (!strcmp(curve_name, "prime256v1"))
		return 256;
	else
		return 0;
}

static int fdt_get_key(struct ecdsa_public_key *key, const void *fdt, int node)
{
	int x_len, y_len;

	key->curve_name = fdt_getprop(fdt, node, "ecdsa,curve", NULL);
	key->size_bits = ecdsa_key_size(key->curve_name);
	if (key->size_bits == 0) {
		debug("Unknown ECDSA curve '%s'", key->curve_name);
		return -EINVAL;
	}

	key->x = fdt_getprop(fdt, node, "ecdsa,x-point", &x_len);
	key->y = fdt_getprop(fdt, node, "ecdsa,y-point", &y_len);

	if (!key->x || !key->y)
		return -EINVAL;

	if (x_len != (key->size_bits / 8) || y_len != (key->size_bits / 8)) {
		printf("%s: node=%d, curve@%p x@%p+%i y@%p+%i\n", __func__,
		       node, key->curve_name, key->x, x_len, key->y, y_len);
		return -EINVAL;
	}

	return 0;
}

static int ecdsa_verify_hash(struct udevice *dev,
			     const struct image_sign_info *info,
			     const void *hash, const void *sig, uint sig_len)
{
	const struct ecdsa_ops *ops = device_get_ops(dev);
	const struct checksum_algo *algo = info->checksum;
	struct ecdsa_public_key key;
	int sig_node, key_node, ret;

	if (!ops || !ops->verify)
		return -ENODEV;

	if (info->required_keynode > 0) {
		ret = fdt_get_key(&key, info->fdt_blob, info->required_keynode);
		if (ret < 0)
			return ret;

		return ops->verify(dev, &key, hash, algo->checksum_len,
				   sig, sig_len);
	}

	sig_node = fdt_subnode_offset(info->fdt_blob, 0, FIT_SIG_NODENAME);
	if (sig_node < 0)
		return -ENOENT;

	/* Try all possible keys under the "/signature" node */
	fdt_for_each_subnode(key_node, info->fdt_blob, sig_node) {
		ret = fdt_get_key(&key, info->fdt_blob, key_node);
		if (ret < 0)
			continue;

		ret = ops->verify(dev, &key, hash, algo->checksum_len,
				  sig, sig_len);

		/* On success, don't worry about remaining keys */
		if (!ret)
			return 0;
	}

	return -EPERM;
}

int ecdsa_verify(struct image_sign_info *info,
		 const struct image_region region[], int region_count,
		 uint8_t *sig, uint sig_len)
{
	const struct checksum_algo *algo = info->checksum;
	uint8_t hash[algo->checksum_len];
	struct udevice *dev;
	int ret;

	ret = uclass_first_device_err(UCLASS_ECDSA, &dev);
	if (ret) {
		debug("ECDSA: Could not find ECDSA implementation: %d\n", ret);
		return ret;
	}

	ret = algo->calculate(algo->name, region, region_count, hash);
	if (ret < 0)
		return -EINVAL;

	return ecdsa_verify_hash(dev, info, hash, sig, sig_len);
}

U_BOOT_CRYPTO_ALGO(ecdsa) = {
	.name = "ecdsa256",
	.key_len = ECDSA256_BYTES,
	.verify = ecdsa_verify,
};

/*
 * uclass definition for ECDSA API
 *
 * We don't implement any wrappers around ecdsa_ops->verify() because it's
 * trivial to call ops->verify().
 */
UCLASS_DRIVER(ecdsa) = {
	.id		= UCLASS_ECDSA,
	.name		= "ecdsa_verifier",
};
