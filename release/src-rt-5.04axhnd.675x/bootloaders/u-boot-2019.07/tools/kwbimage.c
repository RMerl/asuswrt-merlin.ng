// SPDX-License-Identifier: GPL-2.0+
/*
 * Image manipulator for Marvell SoCs
 *  supports Kirkwood, Dove, Armada 370, Armada XP, and Armada 38x
 *
 * (C) Copyright 2013 Thomas Petazzoni
 * <thomas.petazzoni@free-electrons.com>
 *
 * Not implemented: support for the register headers in v1 images
 */

#include "imagetool.h"
#include <limits.h>
#include <image.h>
#include <stdarg.h>
#include <stdint.h>
#include "kwbimage.h"

#ifdef CONFIG_KWB_SECURE
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/evp.h>

#if OPENSSL_VERSION_NUMBER < 0x10100000L || \
    (defined(LIBRESSL_VERSION_NUMBER) && LIBRESSL_VERSION_NUMBER < 0x2070000fL)
static void RSA_get0_key(const RSA *r,
                 const BIGNUM **n, const BIGNUM **e, const BIGNUM **d)
{
   if (n != NULL)
       *n = r->n;
   if (e != NULL)
       *e = r->e;
   if (d != NULL)
       *d = r->d;
}

#elif !defined(LIBRESSL_VERSION_NUMBER)
void EVP_MD_CTX_cleanup(EVP_MD_CTX *ctx)
{
	EVP_MD_CTX_reset(ctx);
}
#endif
#endif

static struct image_cfg_element *image_cfg;
static int cfgn;
#ifdef CONFIG_KWB_SECURE
static int verbose_mode;
#endif

struct boot_mode {
	unsigned int id;
	const char *name;
};

/*
 * SHA2-256 hash
 */
struct hash_v1 {
	uint8_t hash[32];
};

struct boot_mode boot_modes[] = {
	{ 0x4D, "i2c"  },
	{ 0x5A, "spi"  },
	{ 0x8B, "nand" },
	{ 0x78, "sata" },
	{ 0x9C, "pex"  },
	{ 0x69, "uart" },
	{ 0xAE, "sdio" },
	{},
};

struct nand_ecc_mode {
	unsigned int id;
	const char *name;
};

struct nand_ecc_mode nand_ecc_modes[] = {
	{ 0x00, "default" },
	{ 0x01, "hamming" },
	{ 0x02, "rs" },
	{ 0x03, "disabled" },
	{},
};

/* Used to identify an undefined execution or destination address */
#define ADDR_INVALID ((uint32_t)-1)

#define BINARY_MAX_ARGS 8

/* In-memory representation of a line of the configuration file */

enum image_cfg_type {
	IMAGE_CFG_VERSION = 0x1,
	IMAGE_CFG_BOOT_FROM,
	IMAGE_CFG_DEST_ADDR,
	IMAGE_CFG_EXEC_ADDR,
	IMAGE_CFG_NAND_BLKSZ,
	IMAGE_CFG_NAND_BADBLK_LOCATION,
	IMAGE_CFG_NAND_ECC_MODE,
	IMAGE_CFG_NAND_PAGESZ,
	IMAGE_CFG_BINARY,
	IMAGE_CFG_PAYLOAD,
	IMAGE_CFG_DATA,
	IMAGE_CFG_BAUDRATE,
	IMAGE_CFG_DEBUG,
	IMAGE_CFG_KAK,
	IMAGE_CFG_CSK,
	IMAGE_CFG_CSK_INDEX,
	IMAGE_CFG_JTAG_DELAY,
	IMAGE_CFG_BOX_ID,
	IMAGE_CFG_FLASH_ID,
	IMAGE_CFG_SEC_COMMON_IMG,
	IMAGE_CFG_SEC_SPECIALIZED_IMG,
	IMAGE_CFG_SEC_BOOT_DEV,
	IMAGE_CFG_SEC_FUSE_DUMP,

	IMAGE_CFG_COUNT
} type;

static const char * const id_strs[] = {
	[IMAGE_CFG_VERSION] = "VERSION",
	[IMAGE_CFG_BOOT_FROM] = "BOOT_FROM",
	[IMAGE_CFG_DEST_ADDR] = "DEST_ADDR",
	[IMAGE_CFG_EXEC_ADDR] = "EXEC_ADDR",
	[IMAGE_CFG_NAND_BLKSZ] = "NAND_BLKSZ",
	[IMAGE_CFG_NAND_BADBLK_LOCATION] = "NAND_BADBLK_LOCATION",
	[IMAGE_CFG_NAND_ECC_MODE] = "NAND_ECC_MODE",
	[IMAGE_CFG_NAND_PAGESZ] = "NAND_PAGE_SIZE",
	[IMAGE_CFG_BINARY] = "BINARY",
	[IMAGE_CFG_PAYLOAD] = "PAYLOAD",
	[IMAGE_CFG_DATA] = "DATA",
	[IMAGE_CFG_BAUDRATE] = "BAUDRATE",
	[IMAGE_CFG_DEBUG] = "DEBUG",
	[IMAGE_CFG_KAK] = "KAK",
	[IMAGE_CFG_CSK] = "CSK",
	[IMAGE_CFG_CSK_INDEX] = "CSK_INDEX",
	[IMAGE_CFG_JTAG_DELAY] = "JTAG_DELAY",
	[IMAGE_CFG_BOX_ID] = "BOX_ID",
	[IMAGE_CFG_FLASH_ID] = "FLASH_ID",
	[IMAGE_CFG_SEC_COMMON_IMG] = "SEC_COMMON_IMG",
	[IMAGE_CFG_SEC_SPECIALIZED_IMG] = "SEC_SPECIALIZED_IMG",
	[IMAGE_CFG_SEC_BOOT_DEV] = "SEC_BOOT_DEV",
	[IMAGE_CFG_SEC_FUSE_DUMP] = "SEC_FUSE_DUMP"
};

struct image_cfg_element {
	enum image_cfg_type type;
	union {
		unsigned int version;
		unsigned int bootfrom;
		struct {
			const char *file;
			unsigned int args[BINARY_MAX_ARGS];
			unsigned int nargs;
		} binary;
		const char *payload;
		unsigned int dstaddr;
		unsigned int execaddr;
		unsigned int nandblksz;
		unsigned int nandbadblklocation;
		unsigned int nandeccmode;
		unsigned int nandpagesz;
		struct ext_hdr_v0_reg regdata;
		unsigned int baudrate;
		unsigned int debug;
		const char *key_name;
		int csk_idx;
		uint8_t jtag_delay;
		uint32_t boxid;
		uint32_t flashid;
		bool sec_specialized_img;
		unsigned int sec_boot_dev;
		const char *name;
	};
};

#define IMAGE_CFG_ELEMENT_MAX 256

/*
 * Utility functions to manipulate boot mode and ecc modes (convert
 * them back and forth between description strings and the
 * corresponding numerical identifiers).
 */

static const char *image_boot_mode_name(unsigned int id)
{
	int i;

	for (i = 0; boot_modes[i].name; i++)
		if (boot_modes[i].id == id)
			return boot_modes[i].name;
	return NULL;
}

int image_boot_mode_id(const char *boot_mode_name)
{
	int i;

	for (i = 0; boot_modes[i].name; i++)
		if (!strcmp(boot_modes[i].name, boot_mode_name))
			return boot_modes[i].id;

	return -1;
}

int image_nand_ecc_mode_id(const char *nand_ecc_mode_name)
{
	int i;

	for (i = 0; nand_ecc_modes[i].name; i++)
		if (!strcmp(nand_ecc_modes[i].name, nand_ecc_mode_name))
			return nand_ecc_modes[i].id;
	return -1;
}

static struct image_cfg_element *
image_find_option(unsigned int optiontype)
{
	int i;

	for (i = 0; i < cfgn; i++) {
		if (image_cfg[i].type == optiontype)
			return &image_cfg[i];
	}

	return NULL;
}

static unsigned int
image_count_options(unsigned int optiontype)
{
	int i;
	unsigned int count = 0;

	for (i = 0; i < cfgn; i++)
		if (image_cfg[i].type == optiontype)
			count++;

	return count;
}

#if defined(CONFIG_KWB_SECURE)

static int image_get_csk_index(void)
{
	struct image_cfg_element *e;

	e = image_find_option(IMAGE_CFG_CSK_INDEX);
	if (!e)
		return -1;

	return e->csk_idx;
}

static bool image_get_spezialized_img(void)
{
	struct image_cfg_element *e;

	e = image_find_option(IMAGE_CFG_SEC_SPECIALIZED_IMG);
	if (!e)
		return false;

	return e->sec_specialized_img;
}

#endif

/*
 * Compute a 8-bit checksum of a memory area. This algorithm follows
 * the requirements of the Marvell SoC BootROM specifications.
 */
static uint8_t image_checksum8(void *start, uint32_t len)
{
	uint8_t csum = 0;
	uint8_t *p = start;

	/* check len and return zero checksum if invalid */
	if (!len)
		return 0;

	do {
		csum += *p;
		p++;
	} while (--len);

	return csum;
}

size_t kwbimage_header_size(unsigned char *ptr)
{
	if (image_version((void *)ptr) == 0)
		return sizeof(struct main_hdr_v0);
	else
		return KWBHEADER_V1_SIZE((struct main_hdr_v1 *)ptr);
}

/*
 * Verify checksum over a complete header that includes the checksum field.
 * Return 1 when OK, otherwise 0.
 */
static int main_hdr_checksum_ok(void *hdr)
{
	/* Offsets of checksum in v0 and v1 headers are the same */
	struct main_hdr_v0 *main_hdr = (struct main_hdr_v0 *)hdr;
	uint8_t checksum;

	checksum = image_checksum8(hdr, kwbimage_header_size(hdr));
	/* Calculated checksum includes the header checksum field. Compensate
	 * for that.
	 */
	checksum -= main_hdr->checksum;

	return checksum == main_hdr->checksum;
}

static uint32_t image_checksum32(void *start, uint32_t len)
{
	uint32_t csum = 0;
	uint32_t *p = start;

	/* check len and return zero checksum if invalid */
	if (!len)
		return 0;

	if (len % sizeof(uint32_t)) {
		fprintf(stderr, "Length %d is not in multiple of %zu\n",
			len, sizeof(uint32_t));
		return 0;
	}

	do {
		csum += *p;
		p++;
		len -= sizeof(uint32_t);
	} while (len > 0);

	return csum;
}

static uint8_t baudrate_to_option(unsigned int baudrate)
{
	switch (baudrate) {
	case 2400:
		return MAIN_HDR_V1_OPT_BAUD_2400;
	case 4800:
		return MAIN_HDR_V1_OPT_BAUD_4800;
	case 9600:
		return MAIN_HDR_V1_OPT_BAUD_9600;
	case 19200:
		return MAIN_HDR_V1_OPT_BAUD_19200;
	case 38400:
		return MAIN_HDR_V1_OPT_BAUD_38400;
	case 57600:
		return MAIN_HDR_V1_OPT_BAUD_57600;
	case 115200:
		return MAIN_HDR_V1_OPT_BAUD_115200;
	default:
		return MAIN_HDR_V1_OPT_BAUD_DEFAULT;
	}
}

#if defined(CONFIG_KWB_SECURE)
static void kwb_msg(const char *fmt, ...)
{
	if (verbose_mode) {
		va_list ap;

		va_start(ap, fmt);
		vfprintf(stdout, fmt, ap);
		va_end(ap);
	}
}

static int openssl_err(const char *msg)
{
	unsigned long ssl_err = ERR_get_error();

	fprintf(stderr, "%s", msg);
	fprintf(stderr, ": %s\n",
		ERR_error_string(ssl_err, 0));

	return -1;
}

static int kwb_load_rsa_key(const char *keydir, const char *name, RSA **p_rsa)
{
	char path[PATH_MAX];
	RSA *rsa;
	FILE *f;

	if (!keydir)
		keydir = ".";

	snprintf(path, sizeof(path), "%s/%s.key", keydir, name);
	f = fopen(path, "r");
	if (!f) {
		fprintf(stderr, "Couldn't open RSA private key: '%s': %s\n",
			path, strerror(errno));
		return -ENOENT;
	}

	rsa = PEM_read_RSAPrivateKey(f, 0, NULL, "");
	if (!rsa) {
		openssl_err("Failure reading private key");
		fclose(f);
		return -EPROTO;
	}
	fclose(f);
	*p_rsa = rsa;

	return 0;
}

static int kwb_load_cfg_key(struct image_tool_params *params,
			    unsigned int cfg_option, const char *key_name,
			    RSA **p_key)
{
	struct image_cfg_element *e_key;
	RSA *key;
	int res;

	*p_key = NULL;

	e_key = image_find_option(cfg_option);
	if (!e_key) {
		fprintf(stderr, "%s not configured\n", key_name);
		return -ENOENT;
	}

	res = kwb_load_rsa_key(params->keydir, e_key->key_name, &key);
	if (res < 0) {
		fprintf(stderr, "Failed to load %s\n", key_name);
		return -ENOENT;
	}

	*p_key = key;

	return 0;
}

static int kwb_load_kak(struct image_tool_params *params, RSA **p_kak)
{
	return kwb_load_cfg_key(params, IMAGE_CFG_KAK, "KAK", p_kak);
}

static int kwb_load_csk(struct image_tool_params *params, RSA **p_csk)
{
	return kwb_load_cfg_key(params, IMAGE_CFG_CSK, "CSK", p_csk);
}

static int kwb_compute_pubkey_hash(struct pubkey_der_v1 *pk,
				   struct hash_v1 *hash)
{
	EVP_MD_CTX *ctx;
	unsigned int key_size;
	unsigned int hash_size;
	int ret = 0;

	if (!pk || !hash || pk->key[0] != 0x30 || pk->key[1] != 0x82)
		return -EINVAL;

	key_size = (pk->key[2] << 8) + pk->key[3] + 4;

	ctx = EVP_MD_CTX_create();
	if (!ctx)
		return openssl_err("EVP context creation failed");

	EVP_MD_CTX_init(ctx);
	if (!EVP_DigestInit(ctx, EVP_sha256())) {
		ret = openssl_err("Digest setup failed");
		goto hash_err_ctx;
	}

	if (!EVP_DigestUpdate(ctx, pk->key, key_size)) {
		ret = openssl_err("Hashing data failed");
		goto hash_err_ctx;
	}

	if (!EVP_DigestFinal(ctx, hash->hash, &hash_size)) {
		ret = openssl_err("Could not obtain hash");
		goto hash_err_ctx;
	}

	EVP_MD_CTX_cleanup(ctx);

hash_err_ctx:
	EVP_MD_CTX_destroy(ctx);
	return ret;
}

static int kwb_import_pubkey(RSA **key, struct pubkey_der_v1 *src, char *keyname)
{
	RSA *rsa;
	const unsigned char *ptr;

	if (!key || !src)
		goto fail;

	ptr = src->key;
	rsa = d2i_RSAPublicKey(key, &ptr, sizeof(src->key));
	if (!rsa) {
		openssl_err("error decoding public key");
		goto fail;
	}

	return 0;
fail:
	fprintf(stderr, "Failed to decode %s pubkey\n", keyname);
	return -EINVAL;
}

static int kwb_export_pubkey(RSA *key, struct pubkey_der_v1 *dst, FILE *hashf,
			     char *keyname)
{
	int size_exp, size_mod, size_seq;
	const BIGNUM *key_e, *key_n;
	uint8_t *cur;
	char *errmsg = "Failed to encode %s\n";

	RSA_get0_key(key, NULL, &key_e, NULL);
	RSA_get0_key(key, &key_n, NULL, NULL);

	if (!key || !key_e || !key_n || !dst) {
		fprintf(stderr, "export pk failed: (%p, %p, %p, %p)",
			key, key_e, key_n, dst);
		fprintf(stderr, errmsg, keyname);
		return -EINVAL;
	}

	/*
	 * According to the specs, the key should be PKCS#1 DER encoded.
	 * But unfortunately the really required encoding seems to be different;
	 * it violates DER...! (But it still conformes to BER.)
	 * (Length always in long form w/ 2 byte length code; no leading zero
	 * when MSB of first byte is set...)
	 * So we cannot use the encoding func provided by OpenSSL and have to
	 * do the encoding manually.
	 */

	size_exp = BN_num_bytes(key_e);
	size_mod = BN_num_bytes(key_n);
	size_seq = 4 + size_mod + 4 + size_exp;

	if (size_mod > 256) {
		fprintf(stderr, "export pk failed: wrong mod size: %d\n",
			size_mod);
		fprintf(stderr, errmsg, keyname);
		return -EINVAL;
	}

	if (4 + size_seq > sizeof(dst->key)) {
		fprintf(stderr, "export pk failed: seq too large (%d, %lu)\n",
			4 + size_seq, sizeof(dst->key));
		fprintf(stderr, errmsg, keyname);
		return -ENOBUFS;
	}

	cur = dst->key;

	/* PKCS#1 (RFC3447) RSAPublicKey structure */
	*cur++ = 0x30;		/* SEQUENCE */
	*cur++ = 0x82;
	*cur++ = (size_seq >> 8) & 0xFF;
	*cur++ = size_seq & 0xFF;
	/* Modulus */
	*cur++ = 0x02;		/* INTEGER */
	*cur++ = 0x82;
	*cur++ = (size_mod >> 8) & 0xFF;
	*cur++ = size_mod & 0xFF;
	BN_bn2bin(key_n, cur);
	cur += size_mod;
	/* Exponent */
	*cur++ = 0x02;		/* INTEGER */
	*cur++ = 0x82;
	*cur++ = (size_exp >> 8) & 0xFF;
	*cur++ = size_exp & 0xFF;
	BN_bn2bin(key_e, cur);

	if (hashf) {
		struct hash_v1 pk_hash;
		int i;
		int ret = 0;

		ret = kwb_compute_pubkey_hash(dst, &pk_hash);
		if (ret < 0) {
			fprintf(stderr, errmsg, keyname);
			return ret;
		}

		fprintf(hashf, "SHA256 = ");
		for (i = 0 ; i < sizeof(pk_hash.hash); ++i)
			fprintf(hashf, "%02X", pk_hash.hash[i]);
		fprintf(hashf, "\n");
	}

	return 0;
}

int kwb_sign(RSA *key, void *data, int datasz, struct sig_v1 *sig, char *signame)
{
	EVP_PKEY *evp_key;
	EVP_MD_CTX *ctx;
	unsigned int sig_size;
	int size;
	int ret = 0;

	evp_key = EVP_PKEY_new();
	if (!evp_key)
		return openssl_err("EVP_PKEY object creation failed");

	if (!EVP_PKEY_set1_RSA(evp_key, key)) {
		ret = openssl_err("EVP key setup failed");
		goto err_key;
	}

	size = EVP_PKEY_size(evp_key);
	if (size > sizeof(sig->sig)) {
		fprintf(stderr, "Buffer to small for signature (%d bytes)\n",
			size);
		ret = -ENOBUFS;
		goto err_key;
	}

	ctx = EVP_MD_CTX_create();
	if (!ctx) {
		ret = openssl_err("EVP context creation failed");
		goto err_key;
	}
	EVP_MD_CTX_init(ctx);
	if (!EVP_SignInit(ctx, EVP_sha256())) {
		ret = openssl_err("Signer setup failed");
		goto err_ctx;
	}

	if (!EVP_SignUpdate(ctx, data, datasz)) {
		ret = openssl_err("Signing data failed");
		goto err_ctx;
	}

	if (!EVP_SignFinal(ctx, sig->sig, &sig_size, evp_key)) {
		ret = openssl_err("Could not obtain signature");
		goto err_ctx;
	}

	EVP_MD_CTX_cleanup(ctx);
	EVP_MD_CTX_destroy(ctx);
	EVP_PKEY_free(evp_key);

	return 0;

err_ctx:
	EVP_MD_CTX_destroy(ctx);
err_key:
	EVP_PKEY_free(evp_key);
	fprintf(stderr, "Failed to create %s signature\n", signame);
	return ret;
}

int kwb_verify(RSA *key, void *data, int datasz, struct sig_v1 *sig,
	       char *signame)
{
	EVP_PKEY *evp_key;
	EVP_MD_CTX *ctx;
	int size;
	int ret = 0;

	evp_key = EVP_PKEY_new();
	if (!evp_key)
		return openssl_err("EVP_PKEY object creation failed");

	if (!EVP_PKEY_set1_RSA(evp_key, key)) {
		ret = openssl_err("EVP key setup failed");
		goto err_key;
	}

	size = EVP_PKEY_size(evp_key);
	if (size > sizeof(sig->sig)) {
		fprintf(stderr, "Invalid signature size (%d bytes)\n",
			size);
		ret = -EINVAL;
		goto err_key;
	}

	ctx = EVP_MD_CTX_create();
	if (!ctx) {
		ret = openssl_err("EVP context creation failed");
		goto err_key;
	}
	EVP_MD_CTX_init(ctx);
	if (!EVP_VerifyInit(ctx, EVP_sha256())) {
		ret = openssl_err("Verifier setup failed");
		goto err_ctx;
	}

	if (!EVP_VerifyUpdate(ctx, data, datasz)) {
		ret = openssl_err("Hashing data failed");
		goto err_ctx;
	}

	if (EVP_VerifyFinal(ctx, sig->sig, sizeof(sig->sig), evp_key) != 1) {
		ret = openssl_err("Could not verify signature");
		goto err_ctx;
	}

	EVP_MD_CTX_cleanup(ctx);
	EVP_MD_CTX_destroy(ctx);
	EVP_PKEY_free(evp_key);

	return 0;

err_ctx:
	EVP_MD_CTX_destroy(ctx);
err_key:
	EVP_PKEY_free(evp_key);
	fprintf(stderr, "Failed to verify %s signature\n", signame);
	return ret;
}

int kwb_sign_and_verify(RSA *key, void *data, int datasz, struct sig_v1 *sig,
			char *signame)
{
	if (kwb_sign(key, data, datasz, sig, signame) < 0)
		return -1;

	if (kwb_verify(key, data, datasz, sig, signame) < 0)
		return -1;

	return 0;
}


int kwb_dump_fuse_cmds_38x(FILE *out, struct secure_hdr_v1 *sec_hdr)
{
	struct hash_v1 kak_pub_hash;
	struct image_cfg_element *e;
	unsigned int fuse_line;
	int i, idx;
	uint8_t *ptr;
	uint32_t val;
	int ret = 0;

	if (!out || !sec_hdr)
		return -EINVAL;

	ret = kwb_compute_pubkey_hash(&sec_hdr->kak, &kak_pub_hash);
	if (ret < 0)
		goto done;

	fprintf(out, "# burn KAK pub key hash\n");
	ptr = kak_pub_hash.hash;
	for (fuse_line = 26; fuse_line <= 30; ++fuse_line) {
		fprintf(out, "fuse prog -y %u 0 ", fuse_line);

		for (i = 4; i-- > 0;)
			fprintf(out, "%02hx", (ushort)ptr[i]);
		ptr += 4;
		fprintf(out, " 00");

		if (fuse_line < 30) {
			for (i = 3; i-- > 0;)
				fprintf(out, "%02hx", (ushort)ptr[i]);
			ptr += 3;
		} else {
			fprintf(out, "000000");
		}

		fprintf(out, " 1\n");
	}

	fprintf(out, "# burn CSK selection\n");

	idx = image_get_csk_index();
	if (idx < 0 || idx > 15) {
		ret = -EINVAL;
		goto done;
	}
	if (idx > 0) {
		for (fuse_line = 31; fuse_line < 31 + idx; ++fuse_line)
			fprintf(out, "fuse prog -y %u 0 00000001 00000000 1\n",
				fuse_line);
	} else {
		fprintf(out, "# CSK index is 0; no mods needed\n");
	}

	e = image_find_option(IMAGE_CFG_BOX_ID);
	if (e) {
		fprintf(out, "# set box ID\n");
		fprintf(out, "fuse prog -y 48 0 %08x 00000000 1\n", e->boxid);
	}

	e = image_find_option(IMAGE_CFG_FLASH_ID);
	if (e) {
		fprintf(out, "# set flash ID\n");
		fprintf(out, "fuse prog -y 47 0 %08x 00000000 1\n", e->flashid);
	}

	fprintf(out, "# enable secure mode ");
	fprintf(out, "(must be the last fuse line written)\n");

	val = 1;
	e = image_find_option(IMAGE_CFG_SEC_BOOT_DEV);
	if (!e) {
		fprintf(stderr, "ERROR: secured mode boot device not given\n");
		ret = -EINVAL;
		goto done;
	}

	if (e->sec_boot_dev > 0xff) {
		fprintf(stderr, "ERROR: secured mode boot device invalid\n");
		ret = -EINVAL;
		goto done;
	}

	val |= (e->sec_boot_dev << 8);

	fprintf(out, "fuse prog -y 24 0 %08x 0103e0a9 1\n", val);

	fprintf(out, "# lock (unused) fuse lines (0-23)s\n");
	for (fuse_line = 0; fuse_line < 24; ++fuse_line)
		fprintf(out, "fuse prog -y %u 2 1\n", fuse_line);

	fprintf(out, "# OK, that's all :-)\n");

done:
	return ret;
}

static int kwb_dump_fuse_cmds(struct secure_hdr_v1 *sec_hdr)
{
	int ret = 0;
	struct image_cfg_element *e;

	e = image_find_option(IMAGE_CFG_SEC_FUSE_DUMP);
	if (!e)
		return 0;

	if (!strcmp(e->name, "a38x")) {
		FILE *out = fopen("kwb_fuses_a38x.txt", "w+");

		kwb_dump_fuse_cmds_38x(out, sec_hdr);
		fclose(out);
		goto done;
	}

	ret = -ENOSYS;

done:
	return ret;
}

#endif

static void *image_create_v0(size_t *imagesz, struct image_tool_params *params,
			     int payloadsz)
{
	struct image_cfg_element *e;
	size_t headersz;
	struct main_hdr_v0 *main_hdr;
	uint8_t *image;
	int has_ext = 0;

	/*
	 * Calculate the size of the header and the size of the
	 * payload
	 */
	headersz  = sizeof(struct main_hdr_v0);

	if (image_count_options(IMAGE_CFG_DATA) > 0) {
		has_ext = 1;
		headersz += sizeof(struct ext_hdr_v0);
	}

	if (image_count_options(IMAGE_CFG_PAYLOAD) > 1) {
		fprintf(stderr, "More than one payload, not possible\n");
		return NULL;
	}

	image = malloc(headersz);
	if (!image) {
		fprintf(stderr, "Cannot allocate memory for image\n");
		return NULL;
	}

	memset(image, 0, headersz);

	main_hdr = (struct main_hdr_v0 *)image;

	/* Fill in the main header */
	main_hdr->blocksize =
		cpu_to_le32(payloadsz + sizeof(uint32_t) - headersz);
	main_hdr->srcaddr   = cpu_to_le32(headersz);
	main_hdr->ext       = has_ext;
	main_hdr->destaddr  = cpu_to_le32(params->addr);
	main_hdr->execaddr  = cpu_to_le32(params->ep);

	e = image_find_option(IMAGE_CFG_BOOT_FROM);
	if (e)
		main_hdr->blockid = e->bootfrom;
	e = image_find_option(IMAGE_CFG_NAND_ECC_MODE);
	if (e)
		main_hdr->nandeccmode = e->nandeccmode;
	e = image_find_option(IMAGE_CFG_NAND_PAGESZ);
	if (e)
		main_hdr->nandpagesize = cpu_to_le16(e->nandpagesz);
	main_hdr->checksum = image_checksum8(image,
					     sizeof(struct main_hdr_v0));

	/* Generate the ext header */
	if (has_ext) {
		struct ext_hdr_v0 *ext_hdr;
		int cfgi, datai;

		ext_hdr = (struct ext_hdr_v0 *)
				(image + sizeof(struct main_hdr_v0));
		ext_hdr->offset = cpu_to_le32(0x40);

		for (cfgi = 0, datai = 0; cfgi < cfgn; cfgi++) {
			e = &image_cfg[cfgi];
			if (e->type != IMAGE_CFG_DATA)
				continue;

			ext_hdr->rcfg[datai].raddr =
				cpu_to_le32(e->regdata.raddr);
			ext_hdr->rcfg[datai].rdata =
				cpu_to_le32(e->regdata.rdata);
			datai++;
		}

		ext_hdr->checksum = image_checksum8(ext_hdr,
						    sizeof(struct ext_hdr_v0));
	}

	*imagesz = headersz;
	return image;
}

static size_t image_headersz_v1(int *hasext)
{
	struct image_cfg_element *binarye;
	size_t headersz;

	/*
	 * Calculate the size of the header and the size of the
	 * payload
	 */
	headersz = sizeof(struct main_hdr_v1);

	if (image_count_options(IMAGE_CFG_BINARY) > 1) {
		fprintf(stderr, "More than one binary blob, not supported\n");
		return 0;
	}

	if (image_count_options(IMAGE_CFG_PAYLOAD) > 1) {
		fprintf(stderr, "More than one payload, not possible\n");
		return 0;
	}

	binarye = image_find_option(IMAGE_CFG_BINARY);
	if (binarye) {
		int ret;
		struct stat s;

		ret = stat(binarye->binary.file, &s);
		if (ret < 0) {
			char cwd[PATH_MAX];
			char *dir = cwd;

			memset(cwd, 0, sizeof(cwd));
			if (!getcwd(cwd, sizeof(cwd))) {
				dir = "current working directory";
				perror("getcwd() failed");
			}

			fprintf(stderr,
				"Didn't find the file '%s' in '%s' which is mandatory to generate the image\n"
				"This file generally contains the DDR3 training code, and should be extracted from an existing bootable\n"
				"image for your board. See 'kwbimage -x' to extract it from an existing image.\n",
				binarye->binary.file, dir);
			return 0;
		}

		headersz += sizeof(struct opt_hdr_v1) +
			s.st_size +
			(binarye->binary.nargs + 2) * sizeof(uint32_t);
		if (hasext)
			*hasext = 1;
	}

#if defined(CONFIG_KWB_SECURE)
	if (image_get_csk_index() >= 0) {
		headersz += sizeof(struct secure_hdr_v1);
		if (hasext)
			*hasext = 1;
	}
#endif

#if defined(CONFIG_SYS_U_BOOT_OFFS)
	if (headersz > CONFIG_SYS_U_BOOT_OFFS) {
		fprintf(stderr,
			"Error: Image header (incl. SPL image) too big!\n");
		fprintf(stderr, "header=0x%x CONFIG_SYS_U_BOOT_OFFS=0x%x!\n",
			(int)headersz, CONFIG_SYS_U_BOOT_OFFS);
		fprintf(stderr, "Increase CONFIG_SYS_U_BOOT_OFFS!\n");
		return 0;
	}

	headersz = CONFIG_SYS_U_BOOT_OFFS;
#endif

	/*
	 * The payload should be aligned on some reasonable
	 * boundary
	 */
	return ALIGN_SUP(headersz, 4096);
}

int add_binary_header_v1(uint8_t *cur)
{
	struct image_cfg_element *binarye;
	struct opt_hdr_v1 *hdr = (struct opt_hdr_v1 *)cur;
	uint32_t *args;
	size_t binhdrsz;
	struct stat s;
	int argi;
	FILE *bin;
	int ret;

	binarye = image_find_option(IMAGE_CFG_BINARY);

	if (!binarye)
		return 0;

	hdr->headertype = OPT_HDR_V1_BINARY_TYPE;

	bin = fopen(binarye->binary.file, "r");
	if (!bin) {
		fprintf(stderr, "Cannot open binary file %s\n",
			binarye->binary.file);
		return -1;
	}

	if (fstat(fileno(bin), &s)) {
		fprintf(stderr, "Cannot stat binary file %s\n",
			binarye->binary.file);
		goto err_close;
	}

	binhdrsz = sizeof(struct opt_hdr_v1) +
		(binarye->binary.nargs + 2) * sizeof(uint32_t) +
		s.st_size;

	/*
	 * The size includes the binary image size, rounded
	 * up to a 4-byte boundary. Plus 4 bytes for the
	 * next-header byte and 3-byte alignment at the end.
	 */
	binhdrsz = ALIGN_SUP(binhdrsz, 4) + 4;
	hdr->headersz_lsb = cpu_to_le16(binhdrsz & 0xFFFF);
	hdr->headersz_msb = (binhdrsz & 0xFFFF0000) >> 16;

	cur += sizeof(struct opt_hdr_v1);

	args = (uint32_t *)cur;
	*args = cpu_to_le32(binarye->binary.nargs);
	args++;
	for (argi = 0; argi < binarye->binary.nargs; argi++)
		args[argi] = cpu_to_le32(binarye->binary.args[argi]);

	cur += (binarye->binary.nargs + 1) * sizeof(uint32_t);

	ret = fread(cur, s.st_size, 1, bin);
	if (ret != 1) {
		fprintf(stderr,
			"Could not read binary image %s\n",
			binarye->binary.file);
		goto err_close;
	}

	fclose(bin);

	cur += ALIGN_SUP(s.st_size, 4);

	/*
	 * For now, we don't support more than one binary
	 * header, and no other header types are
	 * supported. So, the binary header is necessarily the
	 * last one
	 */
	*((uint32_t *)cur) = 0x00000000;

	cur += sizeof(uint32_t);

	return 0;

err_close:
	fclose(bin);

	return -1;
}

#if defined(CONFIG_KWB_SECURE)

int export_pub_kak_hash(RSA *kak, struct secure_hdr_v1 *secure_hdr)
{
	FILE *hashf;
	int res;

	hashf = fopen("pub_kak_hash.txt", "w");

	res = kwb_export_pubkey(kak, &secure_hdr->kak, hashf, "KAK");

	fclose(hashf);

	return res < 0 ? 1 : 0;
}

int kwb_sign_csk_with_kak(struct image_tool_params *params,
			  struct secure_hdr_v1 *secure_hdr, RSA *csk)
{
	RSA *kak = NULL;
	RSA *kak_pub = NULL;
	int csk_idx = image_get_csk_index();
	struct sig_v1 tmp_sig;

	if (csk_idx >= 16) {
		fprintf(stderr, "Invalid CSK index %d\n", csk_idx);
		return 1;
	}

	if (kwb_load_kak(params, &kak) < 0)
		return 1;

	if (export_pub_kak_hash(kak, secure_hdr))
		return 1;

	if (kwb_import_pubkey(&kak_pub, &secure_hdr->kak, "KAK") < 0)
		return 1;

	if (kwb_export_pubkey(csk, &secure_hdr->csk[csk_idx], NULL, "CSK") < 0)
		return 1;

	if (kwb_sign_and_verify(kak, &secure_hdr->csk,
				sizeof(secure_hdr->csk) +
				sizeof(secure_hdr->csksig),
				&tmp_sig, "CSK") < 0)
		return 1;

	if (kwb_verify(kak_pub, &secure_hdr->csk,
		       sizeof(secure_hdr->csk) +
		       sizeof(secure_hdr->csksig),
		       &tmp_sig, "CSK (2)") < 0)
		return 1;

	secure_hdr->csksig = tmp_sig;

	return 0;
}

int add_secure_header_v1(struct image_tool_params *params, uint8_t *ptr,
			 int payloadsz, size_t headersz, uint8_t *image,
			 struct secure_hdr_v1 *secure_hdr)
{
	struct image_cfg_element *e_jtagdelay;
	struct image_cfg_element *e_boxid;
	struct image_cfg_element *e_flashid;
	RSA *csk = NULL;
	unsigned char *image_ptr;
	size_t image_size;
	struct sig_v1 tmp_sig;
	bool specialized_img = image_get_spezialized_img();

	kwb_msg("Create secure header content\n");

	e_jtagdelay = image_find_option(IMAGE_CFG_JTAG_DELAY);
	e_boxid = image_find_option(IMAGE_CFG_BOX_ID);
	e_flashid = image_find_option(IMAGE_CFG_FLASH_ID);

	if (kwb_load_csk(params, &csk) < 0)
		return 1;

	secure_hdr->headertype = OPT_HDR_V1_SECURE_TYPE;
	secure_hdr->headersz_msb = 0;
	secure_hdr->headersz_lsb = cpu_to_le16(sizeof(struct secure_hdr_v1));
	if (e_jtagdelay)
		secure_hdr->jtag_delay = e_jtagdelay->jtag_delay;
	if (e_boxid && specialized_img)
		secure_hdr->boxid = cpu_to_le32(e_boxid->boxid);
	if (e_flashid && specialized_img)
		secure_hdr->flashid = cpu_to_le32(e_flashid->flashid);

	if (kwb_sign_csk_with_kak(params, secure_hdr, csk))
		return 1;

	image_ptr = ptr + headersz;
	image_size = payloadsz - headersz;

	if (kwb_sign_and_verify(csk, image_ptr, image_size,
				&secure_hdr->imgsig, "image") < 0)
		return 1;

	if (kwb_sign_and_verify(csk, image, headersz, &tmp_sig, "header") < 0)
		return 1;

	secure_hdr->hdrsig = tmp_sig;

	kwb_dump_fuse_cmds(secure_hdr);

	return 0;
}
#endif

static void *image_create_v1(size_t *imagesz, struct image_tool_params *params,
			     uint8_t *ptr, int payloadsz)
{
	struct image_cfg_element *e;
	struct main_hdr_v1 *main_hdr;
#if defined(CONFIG_KWB_SECURE)
	struct secure_hdr_v1 *secure_hdr = NULL;
#endif
	size_t headersz;
	uint8_t *image, *cur;
	int hasext = 0;
	uint8_t *next_ext = NULL;

	/*
	 * Calculate the size of the header and the size of the
	 * payload
	 */
	headersz = image_headersz_v1(&hasext);
	if (headersz == 0)
		return NULL;

	image = malloc(headersz);
	if (!image) {
		fprintf(stderr, "Cannot allocate memory for image\n");
		return NULL;
	}

	memset(image, 0, headersz);

	main_hdr = (struct main_hdr_v1 *)image;
	cur = image;
	cur += sizeof(struct main_hdr_v1);
	next_ext = &main_hdr->ext;

	/* Fill the main header */
	main_hdr->blocksize    =
		cpu_to_le32(payloadsz - headersz + sizeof(uint32_t));
	main_hdr->headersz_lsb = cpu_to_le16(headersz & 0xFFFF);
	main_hdr->headersz_msb = (headersz & 0xFFFF0000) >> 16;
	main_hdr->destaddr     = cpu_to_le32(params->addr)
				 - sizeof(image_header_t);
	main_hdr->execaddr     = cpu_to_le32(params->ep);
	main_hdr->srcaddr      = cpu_to_le32(headersz);
	main_hdr->ext          = hasext;
	main_hdr->version      = 1;
	e = image_find_option(IMAGE_CFG_BOOT_FROM);
	if (e)
		main_hdr->blockid = e->bootfrom;
	e = image_find_option(IMAGE_CFG_NAND_BLKSZ);
	if (e)
		main_hdr->nandblocksize = e->nandblksz / (64 * 1024);
	e = image_find_option(IMAGE_CFG_NAND_BADBLK_LOCATION);
	if (e)
		main_hdr->nandbadblklocation = e->nandbadblklocation;
	e = image_find_option(IMAGE_CFG_BAUDRATE);
	if (e)
		main_hdr->options = baudrate_to_option(e->baudrate);
	e = image_find_option(IMAGE_CFG_DEBUG);
	if (e)
		main_hdr->flags = e->debug ? 0x1 : 0;
	e = image_find_option(IMAGE_CFG_BINARY);
	if (e) {
		char *s = strrchr(e->binary.file, '/');

		if (strcmp(s, "/binary.0") == 0)
			main_hdr->destaddr = cpu_to_le32(params->addr);
	}

#if defined(CONFIG_KWB_SECURE)
	if (image_get_csk_index() >= 0) {
		/*
		 * only reserve the space here; we fill the header later since
		 * we need the header to be complete to compute the signatures
		 */
		secure_hdr = (struct secure_hdr_v1 *)cur;
		cur += sizeof(struct secure_hdr_v1);
		next_ext = &secure_hdr->next;
	}
#endif
	*next_ext = 1;

	if (add_binary_header_v1(cur))
		return NULL;

#if defined(CONFIG_KWB_SECURE)
	if (secure_hdr && add_secure_header_v1(params, ptr, payloadsz,
					       headersz, image, secure_hdr))
		return NULL;
#endif

	/* Calculate and set the header checksum */
	main_hdr->checksum = image_checksum8(main_hdr, headersz);

	*imagesz = headersz;
	return image;
}

int recognize_keyword(char *keyword)
{
	int kw_id;

	for (kw_id = 1; kw_id < IMAGE_CFG_COUNT; ++kw_id)
		if (!strcmp(keyword, id_strs[kw_id]))
			return kw_id;

	return 0;
}

static int image_create_config_parse_oneline(char *line,
					     struct image_cfg_element *el)
{
	char *keyword, *saveptr, *value1, *value2;
	char delimiters[] = " \t";
	int keyword_id, ret, argi;
	char *unknown_msg = "Ignoring unknown line '%s'\n";

	keyword = strtok_r(line, delimiters, &saveptr);
	keyword_id = recognize_keyword(keyword);

	if (!keyword_id) {
		fprintf(stderr, unknown_msg, line);
		return 0;
	}

	el->type = keyword_id;

	value1 = strtok_r(NULL, delimiters, &saveptr);

	if (!value1) {
		fprintf(stderr, "Parameter missing in line '%s'\n", line);
		return -1;
	}

	switch (keyword_id) {
	case IMAGE_CFG_VERSION:
		el->version = atoi(value1);
		break;
	case IMAGE_CFG_BOOT_FROM:
		ret = image_boot_mode_id(value1);

		if (ret < 0) {
			fprintf(stderr, "Invalid boot media '%s'\n", value1);
			return -1;
		}
		el->bootfrom = ret;
		break;
	case IMAGE_CFG_NAND_BLKSZ:
		el->nandblksz = strtoul(value1, NULL, 16);
		break;
	case IMAGE_CFG_NAND_BADBLK_LOCATION:
		el->nandbadblklocation = strtoul(value1, NULL, 16);
		break;
	case IMAGE_CFG_NAND_ECC_MODE:
		ret = image_nand_ecc_mode_id(value1);

		if (ret < 0) {
			fprintf(stderr, "Invalid NAND ECC mode '%s'\n", value1);
			return -1;
		}
		el->nandeccmode = ret;
		break;
	case IMAGE_CFG_NAND_PAGESZ:
		el->nandpagesz = strtoul(value1, NULL, 16);
		break;
	case IMAGE_CFG_BINARY:
		argi = 0;

		el->binary.file = strdup(value1);
		while (1) {
			char *value = strtok_r(NULL, delimiters, &saveptr);

			if (!value)
				break;
			el->binary.args[argi] = strtoul(value, NULL, 16);
			argi++;
			if (argi >= BINARY_MAX_ARGS) {
				fprintf(stderr,
					"Too many arguments for BINARY\n");
				return -1;
			}
		}
		el->binary.nargs = argi;
		break;
	case IMAGE_CFG_DATA:
		value2 = strtok_r(NULL, delimiters, &saveptr);

		if (!value1 || !value2) {
			fprintf(stderr,
				"Invalid number of arguments for DATA\n");
			return -1;
		}

		el->regdata.raddr = strtoul(value1, NULL, 16);
		el->regdata.rdata = strtoul(value2, NULL, 16);
		break;
	case IMAGE_CFG_BAUDRATE:
		el->baudrate = strtoul(value1, NULL, 10);
		break;
	case IMAGE_CFG_DEBUG:
		el->debug = strtoul(value1, NULL, 10);
		break;
	case IMAGE_CFG_KAK:
		el->key_name = strdup(value1);
		break;
	case IMAGE_CFG_CSK:
		el->key_name = strdup(value1);
		break;
	case IMAGE_CFG_CSK_INDEX:
		el->csk_idx = strtol(value1, NULL, 0);
		break;
	case IMAGE_CFG_JTAG_DELAY:
		el->jtag_delay = strtoul(value1, NULL, 0);
		break;
	case IMAGE_CFG_BOX_ID:
		el->boxid = strtoul(value1, NULL, 0);
		break;
	case IMAGE_CFG_FLASH_ID:
		el->flashid = strtoul(value1, NULL, 0);
		break;
	case IMAGE_CFG_SEC_SPECIALIZED_IMG:
		el->sec_specialized_img = true;
		break;
	case IMAGE_CFG_SEC_COMMON_IMG:
		el->sec_specialized_img = false;
		break;
	case IMAGE_CFG_SEC_BOOT_DEV:
		el->sec_boot_dev = strtoul(value1, NULL, 0);
		break;
	case IMAGE_CFG_SEC_FUSE_DUMP:
		el->name = strdup(value1);
		break;
	default:
		fprintf(stderr, unknown_msg, line);
	}

	return 0;
}

/*
 * Parse the configuration file 'fcfg' into the array of configuration
 * elements 'image_cfg', and return the number of configuration
 * elements in 'cfgn'.
 */
static int image_create_config_parse(FILE *fcfg)
{
	int ret;
	int cfgi = 0;

	/* Parse the configuration file */
	while (!feof(fcfg)) {
		char *line;
		char buf[256];

		/* Read the current line */
		memset(buf, 0, sizeof(buf));
		line = fgets(buf, sizeof(buf), fcfg);
		if (!line)
			break;

		/* Ignore useless lines */
		if (line[0] == '\n' || line[0] == '#')
			continue;

		/* Strip final newline */
		if (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = 0;

		/* Parse the current line */
		ret = image_create_config_parse_oneline(line,
							&image_cfg[cfgi]);
		if (ret)
			return ret;

		cfgi++;

		if (cfgi >= IMAGE_CFG_ELEMENT_MAX) {
			fprintf(stderr,
				"Too many configuration elements in .cfg file\n");
			return -1;
		}
	}

	cfgn = cfgi;
	return 0;
}

static int image_get_version(void)
{
	struct image_cfg_element *e;

	e = image_find_option(IMAGE_CFG_VERSION);
	if (!e)
		return -1;

	return e->version;
}

static void kwbimage_set_header(void *ptr, struct stat *sbuf, int ifd,
				struct image_tool_params *params)
{
	FILE *fcfg;
	void *image = NULL;
	int version;
	size_t headersz = 0;
	uint32_t checksum;
	int ret;
	int size;

	fcfg = fopen(params->imagename, "r");
	if (!fcfg) {
		fprintf(stderr, "Could not open input file %s\n",
			params->imagename);
		exit(EXIT_FAILURE);
	}

	image_cfg = malloc(IMAGE_CFG_ELEMENT_MAX *
			   sizeof(struct image_cfg_element));
	if (!image_cfg) {
		fprintf(stderr, "Cannot allocate memory\n");
		fclose(fcfg);
		exit(EXIT_FAILURE);
	}

	memset(image_cfg, 0,
	       IMAGE_CFG_ELEMENT_MAX * sizeof(struct image_cfg_element));
	rewind(fcfg);

	ret = image_create_config_parse(fcfg);
	fclose(fcfg);
	if (ret) {
		free(image_cfg);
		exit(EXIT_FAILURE);
	}

	/* The MVEBU BootROM does not allow non word aligned payloads */
	sbuf->st_size = ALIGN_SUP(sbuf->st_size, 4);

	version = image_get_version();
	switch (version) {
		/*
		 * Fallback to version 0 if no version is provided in the
		 * cfg file
		 */
	case -1:
	case 0:
		image = image_create_v0(&headersz, params, sbuf->st_size);
		break;

	case 1:
		image = image_create_v1(&headersz, params, ptr, sbuf->st_size);
		break;

	default:
		fprintf(stderr, "Unsupported version %d\n", version);
		free(image_cfg);
		exit(EXIT_FAILURE);
	}

	if (!image) {
		fprintf(stderr, "Could not create image\n");
		free(image_cfg);
		exit(EXIT_FAILURE);
	}

	free(image_cfg);

	/* Build and add image checksum header */
	checksum =
		cpu_to_le32(image_checksum32((uint32_t *)ptr, sbuf->st_size));
	size = write(ifd, &checksum, sizeof(uint32_t));
	if (size != sizeof(uint32_t)) {
		fprintf(stderr, "Error:%s - Checksum write %d bytes %s\n",
			params->cmdname, size, params->imagefile);
		exit(EXIT_FAILURE);
	}

	sbuf->st_size += sizeof(uint32_t);

	/* Finally copy the header into the image area */
	memcpy(ptr, image, headersz);

	free(image);
}

static void kwbimage_print_header(const void *ptr)
{
	struct main_hdr_v0 *mhdr = (struct main_hdr_v0 *)ptr;

	printf("Image Type:   MVEBU Boot from %s Image\n",
	       image_boot_mode_name(mhdr->blockid));
	printf("Image version:%d\n", image_version((void *)ptr));
	printf("Data Size:    ");
	genimg_print_size(mhdr->blocksize - sizeof(uint32_t));
	printf("Load Address: %08x\n", mhdr->destaddr);
	printf("Entry Point:  %08x\n", mhdr->execaddr);
}

static int kwbimage_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_KWBIMAGE)
		return EXIT_SUCCESS;

	return EXIT_FAILURE;
}

static int kwbimage_verify_header(unsigned char *ptr, int image_size,
				  struct image_tool_params *params)
{
	uint8_t checksum;
	size_t header_size = kwbimage_header_size(ptr);

	if (header_size > image_size)
		return -FDT_ERR_BADSTRUCTURE;

	if (!main_hdr_checksum_ok(ptr))
		return -FDT_ERR_BADSTRUCTURE;

	/* Only version 0 extended header has checksum */
	if (image_version((void *)ptr) == 0) {
		struct ext_hdr_v0 *ext_hdr;

		ext_hdr = (struct ext_hdr_v0 *)
				(ptr + sizeof(struct main_hdr_v0));
		checksum = image_checksum8(ext_hdr,
					   sizeof(struct ext_hdr_v0)
					   - sizeof(uint8_t));
		if (checksum != ext_hdr->checksum)
			return -FDT_ERR_BADSTRUCTURE;
	}

	return 0;
}

static int kwbimage_generate(struct image_tool_params *params,
			     struct image_type_params *tparams)
{
	FILE *fcfg;
	int alloc_len;
	int version;
	void *hdr;
	int ret;

	fcfg = fopen(params->imagename, "r");
	if (!fcfg) {
		fprintf(stderr, "Could not open input file %s\n",
			params->imagename);
		exit(EXIT_FAILURE);
	}

	image_cfg = malloc(IMAGE_CFG_ELEMENT_MAX *
			   sizeof(struct image_cfg_element));
	if (!image_cfg) {
		fprintf(stderr, "Cannot allocate memory\n");
		fclose(fcfg);
		exit(EXIT_FAILURE);
	}

	memset(image_cfg, 0,
	       IMAGE_CFG_ELEMENT_MAX * sizeof(struct image_cfg_element));
	rewind(fcfg);

	ret = image_create_config_parse(fcfg);
	fclose(fcfg);
	if (ret) {
		free(image_cfg);
		exit(EXIT_FAILURE);
	}

	version = image_get_version();
	switch (version) {
		/*
		 * Fallback to version 0 if no version is provided in the
		 * cfg file
		 */
	case -1:
	case 0:
		alloc_len = sizeof(struct main_hdr_v0) +
			sizeof(struct ext_hdr_v0);
		break;

	case 1:
		alloc_len = image_headersz_v1(NULL);
		break;

	default:
		fprintf(stderr, "Unsupported version %d\n", version);
		free(image_cfg);
		exit(EXIT_FAILURE);
	}

	free(image_cfg);

	hdr = malloc(alloc_len);
	if (!hdr) {
		fprintf(stderr, "%s: malloc return failure: %s\n",
			params->cmdname, strerror(errno));
		exit(EXIT_FAILURE);
	}

	memset(hdr, 0, alloc_len);
	tparams->header_size = alloc_len;
	tparams->hdr = hdr;

	/*
	 * The resulting image needs to be 4-byte aligned. At least
	 * the Marvell hdrparser tool complains if its unaligned.
	 * By returning 1 here in this function, called via
	 * tparams->vrec_header() in mkimage.c, mkimage will
	 * automatically pad the the resulting image to a 4-byte
	 * size if necessary.
	 */
	return 1;
}

/*
 * Report Error if xflag is set in addition to default
 */
static int kwbimage_check_params(struct image_tool_params *params)
{
	if (!strlen(params->imagename)) {
		char *msg = "Configuration file for kwbimage creation omitted";

		fprintf(stderr, "Error:%s - %s\n", params->cmdname, msg);
		return CFG_INVALID;
	}

	return (params->dflag && (params->fflag || params->lflag)) ||
		(params->fflag && (params->dflag || params->lflag)) ||
		(params->lflag && (params->dflag || params->fflag)) ||
		(params->xflag) || !(strlen(params->imagename));
}

/*
 * kwbimage type parameters definition
 */
U_BOOT_IMAGE_TYPE(
	kwbimage,
	"Marvell MVEBU Boot Image support",
	0,
	NULL,
	kwbimage_check_params,
	kwbimage_verify_header,
	kwbimage_print_header,
	kwbimage_set_header,
	NULL,
	kwbimage_check_image_types,
	NULL,
	kwbimage_generate
);
