// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2018 Linaro Limited
 *		Author: AKASHI Takahiro
 */

#include <getopt.h>
#include <pe.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <uuid/uuid.h>

#include <gnutls/gnutls.h>
#include <gnutls/pkcs7.h>
#include <gnutls/abstract.h>

#include "eficapsule.h"

static const char *tool_name = "mkeficapsule";

efi_guid_t efi_guid_fm_capsule = EFI_FIRMWARE_MANAGEMENT_CAPSULE_ID_GUID;
efi_guid_t efi_guid_cert_type_pkcs7 = EFI_CERT_TYPE_PKCS7_GUID;

static const char *opts_short = "g:i:I:v:p:c:m:o:dhARD";

enum {
	CAPSULE_NORMAL_BLOB = 0,
	CAPSULE_ACCEPT,
	CAPSULE_REVERT,
} capsule_type;

static struct option options[] = {
	{"guid", required_argument, NULL, 'g'},
	{"index", required_argument, NULL, 'i'},
	{"instance", required_argument, NULL, 'I'},
	{"fw-version", required_argument, NULL, 'v'},
	{"private-key", required_argument, NULL, 'p'},
	{"certificate", required_argument, NULL, 'c'},
	{"monotonic-count", required_argument, NULL, 'm'},
	{"dump-sig", no_argument, NULL, 'd'},
	{"fw-accept", no_argument, NULL, 'A'},
	{"fw-revert", no_argument, NULL, 'R'},
	{"capoemflag", required_argument, NULL, 'o'},
	{"dump-capsule", no_argument, NULL, 'D'},
	{"help", no_argument, NULL, 'h'},
	{NULL, 0, NULL, 0},
};

static void print_usage(void)
{
	fprintf(stderr, "Usage: %s [options] <image blob> <output file>\n"
		"Options:\n"

		"\t-g, --guid <guid string>    guid for image blob type\n"
		"\t-i, --index <index>         update image index\n"
		"\t-I, --instance <instance>   update hardware instance\n"
		"\t-v, --fw-version <version>  firmware version\n"
		"\t-p, --private-key <privkey file>  private key file\n"
		"\t-c, --certificate <cert file>     signer's certificate file\n"
		"\t-m, --monotonic-count <count>     monotonic count\n"
		"\t-d, --dump_sig              dump signature (*.p7)\n"
		"\t-A, --fw-accept  firmware accept capsule, requires GUID, no image blob\n"
		"\t-R, --fw-revert  firmware revert capsule, takes no GUID, no image blob\n"
		"\t-o, --capoemflag Capsule OEM Flag, an integer between 0x0000 and 0xffff\n"
		"\t-D, --dump-capsule          dump the contents of the capsule headers\n"
		"\t-h, --help                  print a help message\n",
		tool_name);
}

/**
 * auth_context - authentication context
 * @key_file:	Path to a private key file
 * @cert_file:	Path to a certificate file
 * @image_data:	Pointer to firmware data
 * @image_size:	Size of firmware data
 * @auth:	Authentication header
 * @sig_data:	Signature data
 * @sig_size:	Size of signature data
 *
 * Data structure used in create_auth_data(). @key_file through
 * @image_size are input parameters. @auth, @sig_data and @sig_size
 * are filled in by create_auth_data().
 */
struct auth_context {
	char *key_file;
	char *cert_file;
	uint8_t *image_data;
	size_t image_size;
	struct efi_firmware_image_authentication auth;
	uint8_t *sig_data;
	size_t sig_size;
};

static int dump_sig;

/**
 * read_bin_file - read a firmware binary file
 * @bin:	Path to a firmware binary file
 * @data:	Pointer to pointer of allocated buffer
 * @bin_size:	Size of allocated buffer
 *
 * Read out a content of binary, @bin, into @data.
 * A caller should free @data.
 *
 * Return:
 * * 0  - on success
 * * -1 - on failure
 */
static int read_bin_file(char *bin, uint8_t **data, off_t *bin_size)
{
	FILE *g;
	struct stat bin_stat;
	void *buf;
	size_t size;
	int ret = 0;

	g = fopen(bin, "r");
	if (!g) {
		fprintf(stderr, "cannot open %s\n", bin);
		return -1;
	}
	if (stat(bin, &bin_stat) < 0) {
		fprintf(stderr, "cannot determine the size of %s\n", bin);
		ret = -1;
		goto err;
	}
	if (bin_stat.st_size > SIZE_MAX) {
		fprintf(stderr, "file size is too large for malloc: %s\n", bin);
		ret = -1;
		goto err;
	}
	buf = malloc(bin_stat.st_size);
	if (!buf) {
		fprintf(stderr, "cannot allocate memory: %zx\n",
			(size_t)bin_stat.st_size);
		ret = -1;
		goto err;
	}

	size = fread(buf, 1, bin_stat.st_size, g);
	if (size < bin_stat.st_size) {
		fprintf(stderr, "read failed (%zx)\n", size);
		ret = -1;
		goto err;
	}

	*data = buf;
	*bin_size = bin_stat.st_size;
err:
	fclose(g);

	return ret;
}

/**
 * write_capsule_file - write a capsule file
 * @bin:	FILE stream
 * @data:	Pointer to data
 * @bin_size:	Size of data
 *
 * Write out data, @data, with the size @bin_size.
 *
 * Return:
 * * 0  - on success
 * * -1 - on failure
 */
static int write_capsule_file(FILE *f, void *data, size_t size, const char *msg)
{
	size_t size_written;

	size_written = fwrite(data, 1, size, f);
	if (size_written < size) {
		fprintf(stderr, "%s: write failed (%zx != %zx)\n", msg,
			size_written, size);
		return -1;
	}

	return 0;
}

/**
 * create_auth_data - compose authentication data in capsule
 * @auth_context:	Pointer to authentication context
 *
 * Fill up an authentication header (.auth) and signature data (.sig_data)
 * in @auth_context, using library functions from openssl.
 * All the parameters in @auth_context must be filled in by a caller.
 *
 * Return:
 * * 0  - on success
 * * -1 - on failure
 */
static int create_auth_data(struct auth_context *ctx)
{
	gnutls_datum_t cert;
	gnutls_datum_t key;
	off_t file_size;
	gnutls_privkey_t pkey;
	gnutls_x509_crt_t x509;
	gnutls_pkcs7_t pkcs7;
	gnutls_datum_t data;
	gnutls_datum_t signature;
	int ret;

	ret = read_bin_file(ctx->cert_file, &cert.data, &file_size);
	if (ret < 0)
		return -1;
	if (file_size > UINT_MAX)
		return -1;
	cert.size = file_size;

	ret = read_bin_file(ctx->key_file, &key.data, &file_size);
	if (ret < 0)
		return -1;
	if (file_size > UINT_MAX)
		return -1;
	key.size = file_size;

	/*
	 * For debugging,
	 * gnutls_global_set_time_function(mytime);
	 * gnutls_global_set_log_function(tls_log_func);
	 * gnutls_global_set_log_level(6);
	 */

	ret = gnutls_privkey_init(&pkey);
	if (ret < 0) {
		fprintf(stderr, "error in gnutls_privkey_init(): %s\n",
			gnutls_strerror(ret));
		return -1;
	}

	ret = gnutls_x509_crt_init(&x509);
	if (ret < 0) {
		fprintf(stderr, "error in gnutls_x509_crt_init(): %s\n",
			gnutls_strerror(ret));
		return -1;
	}

	/* load a private key */
	ret = gnutls_privkey_import_x509_raw(pkey, &key, GNUTLS_X509_FMT_PEM,
					     0, 0);
	if (ret < 0) {
		fprintf(stderr,
			"error in gnutls_privkey_import_x509_raw(): %s\n",
			gnutls_strerror(ret));
		return -1;
	}

	/* load x509 certificate */
	ret = gnutls_x509_crt_import(x509, &cert, GNUTLS_X509_FMT_PEM);
	if (ret < 0) {
		fprintf(stderr, "error in gnutls_x509_crt_import(): %s\n",
			gnutls_strerror(ret));
		return -1;
	}

	/* generate a PKCS #7 structure */
	ret = gnutls_pkcs7_init(&pkcs7);
	if (ret < 0) {
		fprintf(stderr, "error in gnutls_pkcs7_init(): %s\n",
			gnutls_strerror(ret));
		return -1;
	}

	/* sign */
	/*
	 * Data should have
	 *  * firmware image
	 *  * monotonic count
	 * in this order!
	 * See EDK2's FmpAuthenticatedHandlerRsa2048Sha256()
	 */
	data.size = ctx->image_size + sizeof(ctx->auth.monotonic_count);
	data.data = malloc(data.size);
	if (!data.data) {
		fprintf(stderr, "allocating memory (0x%x) failed\n", data.size);
		return -1;
	}
	memcpy(data.data, ctx->image_data, ctx->image_size);
	memcpy(data.data + ctx->image_size, &ctx->auth.monotonic_count,
	       sizeof(ctx->auth.monotonic_count));

	ret = gnutls_pkcs7_sign(pkcs7, x509, pkey, &data, NULL, NULL,
				GNUTLS_DIG_SHA256,
				/* GNUTLS_PKCS7_EMBED_DATA? */
				GNUTLS_PKCS7_INCLUDE_CERT |
				GNUTLS_PKCS7_INCLUDE_TIME);
	if (ret < 0) {
		fprintf(stderr, "error in gnutls_pkcs7)sign(): %s\n",
			gnutls_strerror(ret));
		return -1;
	}

	/* export */
	ret = gnutls_pkcs7_export2(pkcs7, GNUTLS_X509_FMT_DER, &signature);
	if (ret < 0) {
		fprintf(stderr, "error in gnutls_pkcs7_export2: %s\n",
			gnutls_strerror(ret));
		return -1;
	}
	ctx->sig_data = signature.data;
	ctx->sig_size = signature.size;

	/* fill auth_info */
	ctx->auth.auth_info.hdr.dwLength = sizeof(ctx->auth.auth_info)
						+ ctx->sig_size;
	ctx->auth.auth_info.hdr.wRevision = WIN_CERT_REVISION_2_0;
	ctx->auth.auth_info.hdr.wCertificateType = WIN_CERT_TYPE_EFI_GUID;
	memcpy(&ctx->auth.auth_info.cert_type, &efi_guid_cert_type_pkcs7,
	       sizeof(efi_guid_cert_type_pkcs7));

	/*
	 * For better clean-ups,
	 * gnutls_pkcs7_deinit(pkcs7);
	 * gnutls_privkey_deinit(pkey);
	 * gnutls_x509_crt_deinit(x509);
	 * free(cert.data);
	 * free(key.data);
	 * if error
	 *   gnutls_free(signature.data);
	 */

	return 0;
}

/**
 * dump_signature - dump out a signature
 * @path:	Path to a capsule file
 * @signature:	Signature data
 * @sig_size:	Size of signature data
 *
 * Signature data pointed to by @signature will be saved into
 * a file whose file name is @path with ".p7" suffix.
 *
 * Return:
 * * 0  - on success
 * * -1 - on failure
 */
static int dump_signature(const char *path, uint8_t *signature, size_t sig_size)
{
	char *sig_path;
	FILE *f;
	size_t size;
	int ret = -1;

	sig_path = malloc(strlen(path) + 3 + 1);
	if (!sig_path)
		return ret;

	sprintf(sig_path, "%s.p7", path);
	f = fopen(sig_path, "w");
	if (!f)
		goto err;

	size = fwrite(signature, 1, sig_size, f);
	if (size == sig_size)
		ret = 0;

	fclose(f);
err:
	free(sig_path);
	return ret;
}

/**
 * free_sig_data - free out signature data
 * @ctx:	Pointer to authentication context
 *
 * Free signature data allocated in create_auth_data().
 */
static void free_sig_data(struct auth_context *ctx)
{
	if (ctx->sig_size)
		gnutls_free(ctx->sig_data);
}

/**
 * create_fwbin - create an uefi capsule file
 * @path:	Path to a created capsule file
 * @bin:	Path to a firmware binary to encapsulate
 * @guid:	GUID of related FMP driver
 * @index:	Index number in capsule
 * @instance:	Instance number in capsule
 * @mcount:	Monotonic count in authentication information
 * @private_file:	Path to a private key file
 * @cert_file:	Path to a certificate file
 * @oemflags:  Capsule OEM Flags, bits 0-15
 *
 * This function actually does the job of creating an uefi capsule file.
 * All the arguments must be supplied.
 * If either @private_file ror @cert_file is NULL, the capsule file
 * won't be signed.
 *
 * Return:
 * * 0  - on success
 * * -1 - on failure
 */
static int create_fwbin(char *path, char *bin, efi_guid_t *guid,
			unsigned long index, unsigned long instance,
			struct fmp_payload_header_params *fmp_ph_params,
			uint64_t mcount, char *privkey_file, char *cert_file,
			uint16_t oemflags)
{
	struct efi_capsule_header header;
	struct efi_firmware_management_capsule_header capsule;
	struct efi_firmware_management_capsule_image_header image;
	struct auth_context auth_context;
	FILE *f;
	uint8_t *data, *new_data, *buf;
	off_t bin_size;
	uint64_t offset;
	int ret;
	struct fmp_payload_header payload_header;

#ifdef DEBUG
	fprintf(stderr, "For output: %s\n", path);
	fprintf(stderr, "\tbin: %s\n\ttype: %pUl\n", bin, guid);
	fprintf(stderr, "\tindex: %lu\n\tinstance: %lu\n", index, instance);
#endif
	auth_context.sig_size = 0;
	f = NULL;
	data = NULL;
	new_data = NULL;
	ret = -1;

	/*
	 * read a firmware binary
	 */
	if (read_bin_file(bin, &data, &bin_size))
		goto err;

	buf = data;

	/* insert fmp payload header right before the payload */
	if (fmp_ph_params->have_header) {
		new_data = malloc(bin_size + sizeof(payload_header));
		if (!new_data)
			goto err;

		payload_header.signature = FMP_PAYLOAD_HDR_SIGNATURE;
		payload_header.header_size = sizeof(payload_header);
		payload_header.fw_version = fmp_ph_params->fw_version;
		payload_header.lowest_supported_version = 0; /* not used */
		memcpy(new_data, &payload_header, sizeof(payload_header));
		memcpy(new_data + sizeof(payload_header), data, bin_size);
		buf = new_data;
		bin_size += sizeof(payload_header);
	}

	/* first, calculate signature to determine its size */
	if (privkey_file && cert_file) {
		auth_context.key_file = privkey_file;
		auth_context.cert_file = cert_file;
		auth_context.auth.monotonic_count = mcount;
		auth_context.image_data = buf;
		auth_context.image_size = bin_size;

		if (create_auth_data(&auth_context)) {
			fprintf(stderr, "Signing firmware image failed\n");
			goto err;
		}

		if (dump_sig &&
		    dump_signature(path, auth_context.sig_data,
				   auth_context.sig_size)) {
			fprintf(stderr, "Creating signature file failed\n");
			goto err;
		}
	}

	/*
	 * write a capsule file
	 */
	f = fopen(path, "w");
	if (!f) {
		fprintf(stderr, "cannot open %s\n", path);
		goto err;
	}

	/*
	 * capsule file header
	 */
	header.capsule_guid = efi_guid_fm_capsule;
	header.header_size = sizeof(header);
	/* TODO: The current implementation ignores flags */
	header.flags = CAPSULE_FLAGS_PERSIST_ACROSS_RESET;
	if (oemflags)
		header.flags |= oemflags;
	header.capsule_image_size = sizeof(header)
					+ sizeof(capsule) + sizeof(uint64_t)
					+ sizeof(image)
					+ bin_size;
	if (auth_context.sig_size)
		header.capsule_image_size += sizeof(auth_context.auth)
				+ auth_context.sig_size;
	if (write_capsule_file(f, &header, sizeof(header),
			       "Capsule header"))
		goto err;

	/*
	 * firmware capsule header
	 * This capsule has only one firmware capsule image.
	 */
	capsule.version = 0x00000001;
	capsule.embedded_driver_count = 0;
	capsule.payload_item_count = 1;
	if (write_capsule_file(f, &capsule, sizeof(capsule),
			       "Firmware capsule header"))
		goto err;

	offset = sizeof(capsule) + sizeof(uint64_t);
	if (write_capsule_file(f, &offset, sizeof(offset),
			       "Offset to capsule image"))
		goto err;

	/*
	 * firmware capsule image header
	 */
	image.version = 0x00000003;
	memcpy(&image.update_image_type_id, guid, sizeof(*guid));
	image.update_image_index = index;
	image.reserved[0] = 0;
	image.reserved[1] = 0;
	image.reserved[2] = 0;
	image.update_image_size = bin_size;
	if (auth_context.sig_size)
		image.update_image_size += sizeof(auth_context.auth)
				+ auth_context.sig_size;
	image.update_vendor_code_size = 0; /* none */
	image.update_hardware_instance = instance;
	image.image_capsule_support = 0;
	if (auth_context.sig_size)
		image.image_capsule_support |= CAPSULE_SUPPORT_AUTHENTICATION;
	if (write_capsule_file(f, &image, sizeof(image),
			       "Firmware capsule image header"))
		goto err;

	/*
	 * signature
	 */
	if (auth_context.sig_size) {
		if (write_capsule_file(f, &auth_context.auth,
				       sizeof(auth_context.auth),
				       "Authentication header"))
			goto err;

		if (write_capsule_file(f, auth_context.sig_data,
				       auth_context.sig_size, "Signature"))
			goto err;
	}

	/*
	 * firmware binary
	 */
	if (write_capsule_file(f, buf, bin_size, "Firmware binary"))
		goto err;

	ret = 0;
err:
	if (f)
		fclose(f);
	free_sig_data(&auth_context);
	free(data);
	free(new_data);

	return ret;
}

/**
 * convert_uuid_to_guid() - convert UUID to GUID
 * @buf:	UUID binary
 *
 * UUID and GUID have the same data structure, but their binary
 * formats are different due to the endianness. See lib/uuid.c.
 * Since uuid_parse() can handle only UUID, this function must
 * be called to get correct data for GUID when parsing a string.
 *
 * The correct data will be returned in @buf.
 */
void convert_uuid_to_guid(unsigned char *buf)
{
	unsigned char c;

	c = buf[0];
	buf[0] = buf[3];
	buf[3] = c;
	c = buf[1];
	buf[1] = buf[2];
	buf[2] = c;

	c = buf[4];
	buf[4] = buf[5];
	buf[5] = c;

	c = buf[6];
	buf[6] = buf[7];
	buf[7] = c;
}

static int create_empty_capsule(char *path, efi_guid_t *guid, bool fw_accept)
{
	struct efi_capsule_header header = { 0 };
	FILE *f = NULL;
	int ret = -1;
	efi_guid_t fw_accept_guid = FW_ACCEPT_OS_GUID;
	efi_guid_t fw_revert_guid = FW_REVERT_OS_GUID;
	efi_guid_t capsule_guid;

	f = fopen(path, "w");
	if (!f) {
		fprintf(stderr, "cannot open %s\n", path);
		goto err;
	}

	capsule_guid = fw_accept ? fw_accept_guid : fw_revert_guid;

	memcpy(&header.capsule_guid, &capsule_guid, sizeof(efi_guid_t));
	header.header_size = sizeof(header);
	header.flags = 0;

	header.capsule_image_size = fw_accept ?
		sizeof(header) + sizeof(efi_guid_t) : sizeof(header);

	if (write_capsule_file(f, &header, sizeof(header),
			       "Capsule header"))
		goto err;

	if (fw_accept) {
		if (write_capsule_file(f, guid, sizeof(*guid),
				       "FW Accept Capsule Payload"))
			goto err;
	}

	ret = 0;

err:
	if (f)
		fclose(f);

	return ret;
}

static void print_guid(void *ptr)
{
	int i;
	efi_guid_t *guid = ptr;
	const uint8_t seq[] = {
		3, 2, 1, 0, '-', 5, 4, '-', 7, 6,
		'-', 8, 9, '-', 10, 11, 12, 13, 14, 15 };

	for (i = 0; i < ARRAY_SIZE(seq); i++) {
		if (seq[i] == '-')
			putchar(seq[i]);
		else
			printf("%02X", guid->b[seq[i]]);
	}

	printf("\n");
}

static uint32_t dump_fmp_payload_header(
	struct fmp_payload_header *fmp_payload_hdr)
{
	if (fmp_payload_hdr->signature == FMP_PAYLOAD_HDR_SIGNATURE) {
		printf("--------\n");
		printf("FMP_PAYLOAD_HDR.SIGNATURE\t\t\t: %08X\n",
		       FMP_PAYLOAD_HDR_SIGNATURE);
		printf("FMP_PAYLOAD_HDR.HEADER_SIZE\t\t\t: %08X\n",
		       fmp_payload_hdr->header_size);
		printf("FMP_PAYLOAD_HDR.FW_VERSION\t\t\t: %08X\n",
		       fmp_payload_hdr->fw_version);
		printf("FMP_PAYLOAD_HDR.LOWEST_SUPPORTED_VERSION\t: %08X\n",
		       fmp_payload_hdr->lowest_supported_version);
		return fmp_payload_hdr->header_size;
	}

	return 0;
}

static void dump_capsule_auth_header(
	struct efi_firmware_image_authentication *capsule_auth_hdr)
{
	printf("EFI_FIRMWARE_IMAGE_AUTH.MONOTONIC_COUNT\t\t: %08lX\n",
	       capsule_auth_hdr->monotonic_count);
	printf("EFI_FIRMWARE_IMAGE_AUTH.AUTH_INFO.HDR.dwLENGTH\t: %08X\n",
	       capsule_auth_hdr->auth_info.hdr.dwLength);
	printf("EFI_FIRMWARE_IMAGE_AUTH.AUTH_INFO.HDR.wREVISION\t: %08X\n",
	       capsule_auth_hdr->auth_info.hdr.wRevision);
	printf("EFI_FIRMWARE_IMAGE_AUTH.AUTH_INFO.HDR.wCERTTYPE\t: %08X\n",
	       capsule_auth_hdr->auth_info.hdr.wCertificateType);
	printf("EFI_FIRMWARE_IMAGE_AUTH.AUTH_INFO.CERT_TYPE\t: ");
	print_guid(&capsule_auth_hdr->auth_info.cert_type);
}

static void dump_fmp_capsule_image_header(
	struct efi_firmware_management_capsule_image_header *image_hdr)
{
	void *capsule_auth_hdr;
	void *fmp_payload_hdr;
	uint64_t signature_size = 0;
	uint32_t payload_size = 0;
	uint32_t fmp_payload_hdr_size = 0;
	struct efi_firmware_image_authentication *auth_hdr;

	printf("--------\n");
	printf("FMP_CAPSULE_IMAGE_HDR.VERSION\t\t\t: %08X\n",
	       image_hdr->version);
	printf("FMP_CAPSULE_IMAGE_HDR.UPDATE_IMAGE_TYPE_ID\t: ");
	print_guid(&image_hdr->update_image_type_id);
	printf("FMP_CAPSULE_IMAGE_HDR.UPDATE_IMAGE_INDEX\t: %08X\n",
	       image_hdr->update_image_index);
	printf("FMP_CAPSULE_IMAGE_HDR.UPDATE_IMAGE_SIZE\t\t: %08X\n",
	       image_hdr->update_image_size);
	printf("FMP_CAPSULE_IMAGE_HDR.UPDATE_VENDOR_CODE_SIZE\t: %08X\n",
	       image_hdr->update_vendor_code_size);
	printf("FMP_CAPSULE_IMAGE_HDR.UPDATE_HARDWARE_INSTANCE\t: %08lX\n",
	       image_hdr->update_hardware_instance);
	printf("FMP_CAPSULE_IMAGE_HDR.IMAGE_CAPSULE_SUPPORT\t: %08lX\n",
	       image_hdr->image_capsule_support);

	printf("--------\n");
	if (image_hdr->image_capsule_support & CAPSULE_SUPPORT_AUTHENTICATION) {
		capsule_auth_hdr = (char *)image_hdr + sizeof(*image_hdr);
		dump_capsule_auth_header(capsule_auth_hdr);

		auth_hdr = capsule_auth_hdr;
		signature_size = sizeof(auth_hdr->monotonic_count) +
			auth_hdr->auth_info.hdr.dwLength;
		fmp_payload_hdr = (char *)capsule_auth_hdr + signature_size;
	} else {
		printf("Capsule Authentication Not Enabled\n");
		fmp_payload_hdr = (char *)image_hdr + sizeof(*image_hdr);
	}

	fmp_payload_hdr_size = dump_fmp_payload_header(fmp_payload_hdr);

	payload_size = image_hdr->update_image_size - signature_size -
		fmp_payload_hdr_size;
	printf("--------\n");
	printf("Payload Image Size\t\t\t\t: %08X\n", payload_size);
}

static void dump_fmp_header(
	struct efi_firmware_management_capsule_header *fmp_hdr)
{
	int i;
	void *capsule_image_hdr;

	printf("EFI_FMP_HDR.VERSION\t\t\t\t: %08X\n", fmp_hdr->version);
	printf("EFI_FMP_HDR.EMBEDDED_DRIVER_COUNT\t\t: %08X\n",
	       fmp_hdr->embedded_driver_count);
	printf("EFI_FMP_HDR.PAYLOAD_ITEM_COUNT\t\t\t: %08X\n",
	       fmp_hdr->payload_item_count);

	/*
	 * We currently don't support Embedded Drivers.
	 * Only worry about the payload items.
	 */
	for (i = 0; i < fmp_hdr->payload_item_count; i++) {
		capsule_image_hdr = (char *)fmp_hdr +
			fmp_hdr->item_offset_list[i];
		dump_fmp_capsule_image_header(capsule_image_hdr);
	}
}

static void dump_capsule_header(struct efi_capsule_header *capsule_hdr)
{
	printf("EFI_CAPSULE_HDR.CAPSULE_GUID\t\t\t: ");
	print_guid((void *)&capsule_hdr->capsule_guid);
	printf("EFI_CAPSULE_HDR.HEADER_SIZE\t\t\t: %08X\n",
	       capsule_hdr->header_size);
	printf("EFI_CAPSULE_HDR.FLAGS\t\t\t\t: %08X\n", capsule_hdr->flags);
	printf("EFI_CAPSULE_HDR.CAPSULE_IMAGE_SIZE\t\t: %08X\n",
	       capsule_hdr->capsule_image_size);
}

static void normal_capsule_dump(void *capsule_buf)
{
	void *fmp_hdr;
	struct efi_capsule_header *hdr = capsule_buf;

	dump_capsule_header(hdr);
	printf("--------\n");

	fmp_hdr = (char *)capsule_buf + sizeof(*hdr);
	dump_fmp_header(fmp_hdr);
}

static void empty_capsule_dump(void *capsule_buf)
{
	efi_guid_t *accept_image_guid;
	struct efi_capsule_header *hdr = capsule_buf;
	efi_guid_t efi_empty_accept_capsule = FW_ACCEPT_OS_GUID;

	dump_capsule_header(hdr);

	if (!memcmp(&efi_empty_accept_capsule, &hdr->capsule_guid,
		    sizeof(efi_guid_t))) {
		accept_image_guid = (void *)(char *)capsule_buf +
			sizeof(struct efi_capsule_header);
		printf("--------\n");
		printf("ACCEPT_IMAGE_GUID\t\t\t\t: ");
		print_guid(accept_image_guid);
	}
}

static void dump_capsule_contents(char *capsule_file)
{
	int fd;
	char *ptr;
	efi_guid_t efi_fmp_guid = EFI_FIRMWARE_MANAGEMENT_CAPSULE_ID_GUID;
	efi_guid_t efi_empty_accept_capsule = FW_ACCEPT_OS_GUID;
	efi_guid_t efi_empty_revert_capsule = FW_REVERT_OS_GUID;
	struct stat sbuf;

	if (!capsule_file) {
		fprintf(stderr, "No capsule file provided\n");
		exit(EXIT_FAILURE);
	}

	if ((fd = open(capsule_file, O_RDONLY)) < 0) {
		fprintf(stderr, "Error opening capsule file: %s\n",
			capsule_file);
		exit(EXIT_FAILURE);
	}

	if (fstat(fd, &sbuf) < 0) {
		fprintf(stderr, "Can't stat capsule file: %s\n", capsule_file);
		exit(EXIT_FAILURE);
	}

	if ((ptr = mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, fd, 0))
	    == MAP_FAILED) {
		fprintf(stderr, "Can't mmap capsule file: %s\n", capsule_file);
		exit(EXIT_FAILURE);
	}

	if (!memcmp(&efi_fmp_guid, ptr, sizeof(efi_guid_t))) {
		normal_capsule_dump(ptr);
	} else if (!memcmp(&efi_empty_accept_capsule, ptr,
		   sizeof(efi_guid_t)) ||
		   !memcmp(&efi_empty_revert_capsule, ptr,
			   sizeof(efi_guid_t))) {
		empty_capsule_dump(ptr);
	} else {
		fprintf(stderr, "Unable to decode the capsule file: %s\n",
			capsule_file);
		exit(EXIT_FAILURE);
	}
}

/**
 * main - main entry function of mkeficapsule
 * @argc:	Number of arguments
 * @argv:	Array of pointers to arguments
 *
 * Create an uefi capsule file, optionally signing it.
 * Parse all the arguments and pass them on to create_fwbin().
 *
 * Return:
 * * 0  - on success
 * * -1 - on failure
 */
int main(int argc, char **argv)
{
	efi_guid_t *guid;
	unsigned char uuid_buf[16];
	unsigned long index, instance;
	uint64_t mcount;
	unsigned long oemflags;
	bool capsule_dump;
	char *privkey_file, *cert_file;
	int c, idx;
	struct fmp_payload_header_params fmp_ph_params = { 0 };

	guid = NULL;
	index = 0;
	instance = 0;
	mcount = 0;
	privkey_file = NULL;
	cert_file = NULL;
	capsule_dump = false;
	dump_sig = 0;
	capsule_type = CAPSULE_NORMAL_BLOB;
	oemflags = 0;
	for (;;) {
		c = getopt_long(argc, argv, opts_short, options, &idx);
		if (c == -1)
			break;

		switch (c) {
		case 'g':
			if (guid) {
				fprintf(stderr,
					"Image type already specified\n");
				exit(EXIT_FAILURE);
			}
			if (uuid_parse(optarg, uuid_buf)) {
				fprintf(stderr, "Wrong guid format\n");
				exit(EXIT_FAILURE);
			}
			convert_uuid_to_guid(uuid_buf);
			guid = (efi_guid_t *)uuid_buf;
			break;
		case 'i':
			index = strtoul(optarg, NULL, 0);
			break;
		case 'I':
			instance = strtoul(optarg, NULL, 0);
			break;
		case 'v':
			fmp_ph_params.fw_version = strtoul(optarg, NULL, 0);
			fmp_ph_params.have_header = true;
			break;
		case 'p':
			if (privkey_file) {
				fprintf(stderr,
					"Private Key already specified\n");
				exit(EXIT_FAILURE);
			}
			privkey_file = optarg;
			break;
		case 'c':
			if (cert_file) {
				fprintf(stderr,
					"Certificate file already specified\n");
				exit(EXIT_FAILURE);
			}
			cert_file = optarg;
			break;
		case 'm':
			mcount = strtoul(optarg, NULL, 0);
			break;
		case 'd':
			dump_sig = 1;
			break;
		case 'A':
			if (capsule_type) {
				fprintf(stderr,
					"Select either of Accept or Revert capsule generation\n");
				exit(1);
			}
			capsule_type = CAPSULE_ACCEPT;
			break;
		case 'R':
			if (capsule_type) {
				fprintf(stderr,
					"Select either of Accept or Revert capsule generation\n");
				exit(1);
			}
			capsule_type = CAPSULE_REVERT;
			break;
		case 'o':
			oemflags = strtoul(optarg, NULL, 0);
			if (oemflags > 0xffff) {
				fprintf(stderr,
					"oemflags must be between 0x0 and 0xffff\n");
				exit(1);
			}
			break;
		case 'D':
			capsule_dump = true;
			break;
		default:
			print_usage();
			exit(EXIT_SUCCESS);
		}
	}

	if (capsule_dump) {
		if (argc != optind + 1) {
			fprintf(stderr, "Must provide the capsule file to parse\n");
			exit(EXIT_FAILURE);
		}
		dump_capsule_contents(argv[argc - 1]);
		exit(EXIT_SUCCESS);
	}

	/* check necessary parameters */
	if ((capsule_type == CAPSULE_NORMAL_BLOB &&
	    ((argc != optind + 2) || !guid ||
	     ((privkey_file && !cert_file) ||
	      (!privkey_file && cert_file)))) ||
	    (capsule_type != CAPSULE_NORMAL_BLOB &&
	    ((argc != optind + 1) ||
	     ((capsule_type == CAPSULE_ACCEPT) && !guid) ||
	     ((capsule_type == CAPSULE_REVERT) && guid)))) {
		print_usage();
		exit(EXIT_FAILURE);
	}

	if (capsule_type != CAPSULE_NORMAL_BLOB) {
		if (create_empty_capsule(argv[argc - 1], guid,
					 capsule_type == CAPSULE_ACCEPT) < 0) {
			fprintf(stderr, "Creating empty capsule failed\n");
			exit(EXIT_FAILURE);
		}
	} else 	if (create_fwbin(argv[argc - 1], argv[argc - 2], guid,
				 index, instance, &fmp_ph_params, mcount, privkey_file,
				 cert_file, (uint16_t)oemflags) < 0) {
		fprintf(stderr, "Creating firmware capsule failed\n");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
