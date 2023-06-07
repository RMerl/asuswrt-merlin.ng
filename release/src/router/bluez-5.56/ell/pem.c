/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2015  Intel Corporation. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <strings.h>

#include "util.h"
#include "private.h"
#include "key.h"
#include "cert.h"
#include "queue.h"
#include "pem.h"
#include "base64.h"
#include "utf8.h"
#include "asn1-private.h"
#include "cipher.h"
#include "cert-private.h"
#include "missing.h"
#include "pem-private.h"

#define PEM_START_BOUNDARY	"-----BEGIN "
#define PEM_END_BOUNDARY	"-----END "

static const char *is_start_boundary(const void *buf, size_t buf_len,
					size_t *label_len)
{
	const char *start, *end, *ptr;
	int prev_special, special;
	const char *buf_ptr = buf;

	if (buf_len < strlen(PEM_START_BOUNDARY))
		return NULL;

	/* Check we have a "-----BEGIN " (RFC7468 section 2) */
	if (memcmp(buf, PEM_START_BOUNDARY, strlen(PEM_START_BOUNDARY)))
		return NULL;

	/*
	 * Check we have a string of printable characters in which no
	 * two consecutive characters are "special" nor is the first or the
	 * final character "special".  These special characters are space
	 * and hyphen.  (RFC7468 section 3)
	 * The loop will end on the second hyphen of the final "-----" if
	 * no error found earlier.
	 */
	start = buf + strlen(PEM_START_BOUNDARY);
	end = start;
	prev_special = 1;

	while (end < buf_ptr + buf_len && l_ascii_isprint(*end)) {
		special = *end == ' ' || *end == '-';

		if (prev_special && special)
			break;

		end++;
		prev_special = special;
	}

	/* Rewind to the first '-', but handle empty labels */
	if (end != start)
		end--;

	/* Check we have a "-----" (RFC7468 section 2) */
	if (end + 5 > buf_ptr + buf_len || memcmp(end, "-----", 5))
		return NULL;

	/* Check all remaining characters are horizontal whitespace (WSP) */
	for (ptr = end + 5; ptr < buf_ptr + buf_len; ptr++)
		if (*ptr != ' ' && *ptr != '\t')
			return NULL;

	*label_len = end - start;

	return start;
}

static bool is_end_boundary(const void *buf, size_t buf_len,
				const char *label, size_t label_len)
{
	const char *buf_ptr = buf;
	size_t len = strlen(PEM_END_BOUNDARY) + label_len + 5;

	if (buf_len < len)
		return false;

	if (memcmp(buf_ptr, PEM_END_BOUNDARY, strlen(PEM_END_BOUNDARY)) ||
			memcmp(buf_ptr + strlen(PEM_END_BOUNDARY),
				label, label_len) ||
			memcmp(buf_ptr + (len - 5), "-----", 5))
		return false;

	/* Check all remaining characters are horizontal whitespace (WSP) */
	for (; len < buf_len; len++)
		if (buf_ptr[len] != ' ' && buf_ptr[len] != '\t')
			return false;

	return true;
}

const char *pem_next(const void *buf, size_t buf_len, char **type_label,
				size_t *base64_len,
				const char **endp, bool strict)
{
	const char *buf_ptr = buf;
	const char *base64_data = NULL, *eol;
	const char *label = NULL;
	size_t label_len = 0;
	const char *start = NULL;

	/*
	 * The base64 parser uses the RFC7468 laxbase64text grammar but we
	 * do full checks on the encapsulation boundary lines, i.e. no
	 * leading spaces allowed, making sure quoted text and similar
	 * are not confused for actual PEM "textual encoding".
	 */
	while (buf_len) {
		for (eol = buf_ptr; eol < buf_ptr + buf_len; eol++)
			if (*eol == '\r' || *eol == '\n')
				break;

		if (!base64_data) {
			label = is_start_boundary(buf_ptr, eol - buf_ptr,
							&label_len);
			if (label) {
				start = label - strlen("-----BEGIN ");
				base64_data = eol;
			} else if (strict)
				break;
		} else if (start && is_end_boundary(buf_ptr, eol - buf_ptr,
							label, label_len)) {
			if (type_label)
				*type_label = l_strndup(label, label_len);

			if (base64_len)
				*base64_len = buf_ptr - base64_data;

			if (endp) {
				if (eol == buf + buf_len)
					*endp = eol;
				else
					*endp = eol + 1;
			}

			return base64_data;
		}

		if (eol == buf_ptr + buf_len)
			break;

		buf_len -= eol + 1 - buf_ptr;
		buf_ptr = eol + 1;

		if (buf_len && *eol == '\r' && *buf_ptr == '\n') {
			buf_ptr++;
			buf_len--;
		}
	}

	/* If we found no label signal EOF rather than parse error */
	if (!base64_data && endp)
		*endp = NULL;

	return NULL;
}

uint8_t *pem_load_buffer(const void *buf, size_t buf_len,
				char **out_type_label, size_t *out_len,
				char **out_headers, const char **out_endp)
{
	size_t base64_len;
	const char *base64;
	char *label;
	const char *headers = NULL;
	size_t headers_len;
	uint8_t *ret;

	base64 = pem_next(buf, buf_len, &label, &base64_len,
				out_endp, false);
	if (!base64)
		return NULL;

	if (memchr(base64, ':', base64_len)) {
		const char *start;
		const char *end;

		while (base64_len && l_ascii_isspace(*base64)) {
			base64++;
			base64_len--;
		}

		start = base64;

		if (!(end = memmem(start, base64_len, "\n\n", 2)) &&
				!(end = memmem(start, base64_len, "\n\r\n", 3)))
			goto err;

		/* Check that each header line has a key and a colon */
		while (start < end) {
			const char *lf = rawmemchr(start, '\n');
			const char *colon = memchr(start, ':', lf - start);

			if (!colon)
				goto err;

			for (; start < colon; start++)
				if (l_ascii_isalnum(*start))
					break;

			if (start == colon)
				goto err;

			start = lf + 1;
		}

		headers = base64;
		headers_len = end - base64;

		base64_len -= headers_len + 2;
		base64 = end + 2;
	}

	ret = l_base64_decode(base64, base64_len, out_len);
	if (ret) {
		*out_type_label = label;

		if (out_headers) {
			if (headers)
				*out_headers = l_strndup(headers, headers_len);
			else
				*out_headers = NULL;
		}

		return ret;
	}

err:
	l_free(label);

	return NULL;
}

LIB_EXPORT uint8_t *l_pem_load_buffer(const void *buf, size_t buf_len,
					char **type_label, size_t *out_len)
{
	return pem_load_buffer(buf, buf_len, type_label, out_len, NULL, NULL);
}

int pem_file_open(struct pem_file_info *info, const char *filename)
{
	info->fd = open(filename, O_RDONLY);
	if (info->fd < 0)
		return -errno;

	if (fstat(info->fd, &info->st) < 0) {
		int r = -errno;

		close(info->fd);
		return r;
	}

	info->data = mmap(NULL, info->st.st_size,
				PROT_READ, MAP_SHARED, info->fd, 0);
	if (info->data == MAP_FAILED) {
		int r = -errno;

		close(info->fd);
		return r;
	}

	return 0;
}

void pem_file_close(struct pem_file_info *info)
{
	munmap(info->data, info->st.st_size);
	close(info->fd);
}

static uint8_t *pem_load_file(const char *filename, char **out_type_label,
				size_t *out_len, char **out_headers)
{
	struct pem_file_info file;
	uint8_t *result;

	if (unlikely(!filename))
		return NULL;

	if (pem_file_open(&file, filename) < 0)
		return NULL;

	result = pem_load_buffer(file.data, file.st.st_size,
					out_type_label, out_len, out_headers,
					NULL);
	pem_file_close(&file);
	return result;
}

LIB_EXPORT uint8_t *l_pem_load_file(const char *filename,
					char **out_type_label, size_t *out_len)
{
	return pem_load_file(filename, out_type_label, out_len, NULL);
}

static struct l_certchain *pem_list_to_chain(struct l_queue *list)
{
	struct l_certchain *chain;

	if (!list)
		return NULL;

	chain = certchain_new_from_leaf(l_queue_pop_head(list));

	while (!l_queue_isempty(list))
		certchain_link_issuer(chain, l_queue_pop_head(list));

	l_queue_destroy(list, NULL);
	return chain;
}

LIB_EXPORT struct l_certchain *l_pem_load_certificate_chain_from_data(
						const void *buf, size_t len)
{
	struct l_queue *list = l_pem_load_certificate_list_from_data(buf, len);

	if (!list)
		return NULL;

	return pem_list_to_chain(list);
}

LIB_EXPORT struct l_certchain *l_pem_load_certificate_chain(
							const char *filename)
{
	struct l_queue *list = l_pem_load_certificate_list(filename);

	if (!list)
		return NULL;

	return pem_list_to_chain(list);
}

static bool pem_write_one_cert(struct l_cert *cert, void *user_data)
{
	int *fd = user_data;
	const uint8_t *der;
	size_t der_len;
	struct iovec iov[3];
	ssize_t r;

	der = l_cert_get_der_data(cert, &der_len);

	iov[0].iov_base = "-----BEGIN CERTIFICATE-----\n";
	iov[0].iov_len = strlen(iov[0].iov_base);
	iov[1].iov_base = l_base64_encode(der, der_len, 64, &iov[1].iov_len);
	iov[2].iov_base = "\n-----END CERTIFICATE-----\n";
	iov[2].iov_len = strlen(iov[2].iov_base);
	r = L_TFR(writev(*fd, iov, 3));
	l_free(iov[1].iov_base);

	if (r == (ssize_t) (iov[0].iov_len + iov[1].iov_len + iov[2].iov_len))
		return false;

	if (r < 0)
		*fd = -errno;
	else
		*fd = -EIO;

	return true;
}

int pem_write_certificate_chain(const struct l_certchain *chain,
				const char *filename)
{
	int fd = L_TFR(open(filename, O_CREAT | O_WRONLY | O_CLOEXEC, 0600));
	int err = fd;

	if (err < 0)
		return -errno;

	l_certchain_walk_from_leaf((struct l_certchain *) chain,
					pem_write_one_cert, &err);
	close(fd);

	return err < 0 ? err : 0;
}

LIB_EXPORT struct l_queue *l_pem_load_certificate_list_from_data(
						const void *buf, size_t len)
{
	const char *ptr, *end;
	struct l_queue *list = NULL;

	ptr = buf;
	end = buf + len;

	while (ptr && ptr < end) {
		uint8_t *der;
		size_t der_len;
		char *label = NULL;
		struct l_cert *cert;
		const char *base64;
		size_t base64_len;
		bool is_certificate;

		base64 = pem_next(ptr, end - ptr, &label,
					&base64_len, &ptr, false);
		if (!base64) {
			if (!ptr)
				break;

			/* if ptr was not reset to NULL; parse error */
			goto error;
		}

		is_certificate = !strcmp(label, "CERTIFICATE");
		l_free(label);

		if (!is_certificate)
			goto error;

		der = l_base64_decode(base64, base64_len, &der_len);
		if (!der)
			goto error;

		cert = l_cert_new_from_der(der, der_len);
		l_free(der);

		if (!cert)
			goto error;

		if (!list)
			list = l_queue_new();

		l_queue_push_tail(list, cert);
	}

	return list;

error:
	l_queue_destroy(list, (l_queue_destroy_func_t) l_cert_free);
	return NULL;
}

LIB_EXPORT struct l_queue *l_pem_load_certificate_list(const char *filename)
{
	struct pem_file_info file;
	struct l_queue *list = NULL;

	if (unlikely(!filename))
		return NULL;

	if (pem_file_open(&file, filename) < 0)
		return NULL;

	list = l_pem_load_certificate_list_from_data(file.data,
							file.st.st_size);
	pem_file_close(&file);

	return list;
}

#define SKIP_WHITESPACE(str)		\
	while (l_ascii_isspace(*(str)))	\
		(str)++;

static const char *parse_rfc1421_dek_info(char *headers,
						const char **out_params)
{
	const char *proc_type = NULL;
	char *dek_info = NULL;
	char *comma;

	while (headers) {
		char *lf = strchrnul(headers, '\n');
		char *key;

		key = headers;
		SKIP_WHITESPACE(key);
		headers = (*lf == '\n') ? lf + 1 : NULL;

		if (!memcmp(key, "X-", 2))
			key += 2;

		if (!memcmp(key, "Proc-Type:", 10)) {
			if (proc_type)
				return NULL;

			proc_type = key + 10;
			SKIP_WHITESPACE(proc_type);
		} else if (!memcmp(key, "DEK-Info:", 9)) {
			if (dek_info)
				return NULL;

			dek_info = key + 9;
			SKIP_WHITESPACE(dek_info);
		} else
			continue;

		while (l_ascii_isspace(lf[-1]))
			lf--;

		*lf = '\0';
	}

	if (!proc_type || !dek_info)
		return NULL;

	/* Skip the version field (should be 3 or 4) */
	proc_type = strchr(proc_type, ',');
	if (!proc_type)
		return NULL;

	proc_type++;
	SKIP_WHITESPACE(proc_type);

	/* Section 4.6.1.1 */
	if (strcmp(proc_type, "ENCRYPTED"))
		return NULL;

	comma = strchr(dek_info, ',');
	if (comma) {
		*out_params = comma + 1;
		SKIP_WHITESPACE(*out_params);

		while (comma > dek_info && l_ascii_isspace(comma[-1]))
			comma--;

		*comma = '\0';
	} else
		*out_params = NULL;

	return dek_info;
}

static struct l_cipher *cipher_from_dek_info(const char *algid, const char *params,
						const char *passphrase,
						size_t *block_len)
{
	enum l_cipher_type type;
	struct l_cipher *cipher;
	struct l_checksum *md5;
	uint8_t key[32];
	size_t key_len;
	bool ok;
	L_AUTO_FREE_VAR(uint8_t *, iv) = NULL;
	size_t iv_len;

	if (!strcmp(algid, "DES-CBC")) {
		type = L_CIPHER_DES_CBC;
		key_len = 8;
		iv_len = 8;
	} else if (!strcmp(algid, "DES-EDE3-CBC")) {
		type = L_CIPHER_DES3_EDE_CBC;
		key_len = 24;
		iv_len = 8;
	} else if (!strcmp(algid, "AES-128-CBC")) {
		type = L_CIPHER_AES_CBC;
		key_len = 16;
		iv_len = 16;
	} else if (!strcmp(algid, "AES-192-CBC")) {
		type = L_CIPHER_AES_CBC;
		key_len = 24;
		iv_len = 16;
	} else if (!strcmp(algid, "AES-256-CBC")) {
		type = L_CIPHER_AES_CBC;
		key_len = 32;
		iv_len = 16;
	} else
		return NULL;

	if (!params || strlen(params) != 2 * iv_len)
		return NULL;

	*block_len = iv_len;

	iv = l_util_from_hexstring(params, &iv_len);
	if (!iv)
		return NULL;

	/*
	 * The encryption key is the MD5(password | IV[:8]), this comes from
	 * opessl's crypto/evp/evp_key.c:EVP_BytesToKey() and doesn't seem to
	 * be backed by any standard:
	 * https://web.archive.org/web/20190528100132/https://latacora.singles/2018/08/03/the-default-openssh.html
	 */
	md5 = l_checksum_new(L_CHECKSUM_MD5);
	if (!md5)
		return NULL;

	ok = l_checksum_update(md5, passphrase, strlen(passphrase)) &&
		l_checksum_update(md5, iv, 8) &&
		l_checksum_get_digest(md5, key, 16) == 16;

	if (ok && key_len > 16) {
		l_checksum_reset(md5);
		ok = l_checksum_update(md5, key, 16) &&
			l_checksum_update(md5, passphrase, strlen(passphrase)) &&
			l_checksum_update(md5, iv, 8) &&
			l_checksum_get_digest(md5, key + 16, 16) == 16;
	}

	l_checksum_free(md5);

	if (!ok) {
		cipher = NULL;
		goto cleanup;
	}

	cipher  = l_cipher_new(type, key, key_len);
	if (!cipher)
		goto cleanup;

	if (l_cipher_set_iv(cipher, iv, iv_len))
		goto cleanup;

	l_cipher_free(cipher);
	cipher = NULL;

cleanup:
	explicit_bzero(key, sizeof(key));
	return cipher;
}

struct l_key *pem_load_private_key(uint8_t *content, size_t len, char *label,
					const char *passphrase, char *headers,
					bool *encrypted)
{
	struct l_key *pkey;

	/*
	 * RFC7468 Section 10-compatible unencrypted private key label
	 * (also mentioned in PKCS#8/RFC5958 Section 5), encodes
	 * the PKCS#8/RFC5958 PrivateKeyInfo structure -- supported
	 * directly by the pkcs8-key-parser kernel module.
	 */
	if (!strcmp(label, "PRIVATE KEY")) {
		/* RFC822 Headers explicitly disallowed in RFC7468 */
		if (headers)
			goto err;

		pkey = cert_key_from_pkcs8_private_key_info(content, len);
		goto done;
	}

	/*
	 * RFC7468 Section 11-compatible encrypted private key label
	 * (also mentioned in PKCS#8/RFC5958 Section 5), encodes
	 * the PKCS#8/RFC5958 EncryptedPrivateKeyInfo structure.  We
	 * decrypt it into a plain PrivateKeyInfo for the
	 * pkcs8-key-parser module.
	 */
	if (!strcmp(label, "ENCRYPTED PRIVATE KEY")) {
		if (encrypted)
			*encrypted = true;

		if (!passphrase)
			goto err;

		/* RFC822 Headers explicitly disallowed in RFC7468 */
		if (headers)
			goto err;

		pkey = cert_key_from_pkcs8_encrypted_private_key_info(content,
								len,
								passphrase);
		goto done;
	}

	/*
	 * Legacy RSA private key label aka. SSLeay format, understood by
	 * most software but not documented in an RFC.  Encodes the
	 * PKCS#1/RFC8017 RSAPrivateKey structure.  We wrap it in a PKCS#8
	 * PrivateKeyInfo for the pkcs8-key-parser module.
	 */
	if (!strcmp(label, "RSA PRIVATE KEY")) {
		const char *dekalgid;
		const char *dekparameters;

		/*
		 * "openssl rsa ..." can produce encrypted PKCS#1-formatted
		 * keys.  These are incompatible with RFC7468 parsing because
		 * of the RFC822 headers present but the format is the same
		 * as documented in RFC1421.  The encryption algorithms are
		 * supposed to be the ones defined in RFC1423 but that would
		 * be only DES-CBC while openssl allows other algorithms.
		 * When decrypted we get the RSAPrivateKey struct and proceed
		 * like with the unencrypted format.
		 */
		dekalgid = parse_rfc1421_dek_info(headers, &dekparameters);
		if (dekalgid) {
			struct l_cipher *alg;
			bool r;
			size_t block_len;
			uint8_t pad;

			if (encrypted)
				*encrypted = true;

			if (!passphrase)
				goto err;

			alg = cipher_from_dek_info(dekalgid, dekparameters,
							passphrase, &block_len);
			if (!alg)
				goto err;

			if (len % block_len || !len) {
				l_cipher_free(alg);
				goto err;
			}

			r = l_cipher_decrypt(alg, content, content, len);
			l_cipher_free(alg);

			if (!r)
				goto err;

			/* Remove padding like in RFC1423 Section 1.1 */
			pad = content[len - 1];
			if (pad > block_len || pad == 0)
				goto err;

			if (!l_secure_memeq(content + len - pad, pad - 1U, pad))
				goto err;

			len -= pad;
		}

		pkey = cert_key_from_pkcs1_rsa_private_key(content, len);
		goto done;
	}

	/* Label not known */
err:
	pkey = NULL;
done:
	explicit_bzero(content, len);
	l_free(content);
	l_free(label);
	l_free(headers);
	return pkey;
}

LIB_EXPORT struct l_key *l_pem_load_private_key_from_data(const void *buf,
							size_t buf_len,
							const char *passphrase,
							bool *encrypted)
{
	uint8_t *content;
	char *label;
	size_t len;
	char *headers;

	if (encrypted)
		*encrypted = false;

	content = pem_load_buffer(buf, buf_len, &label, &len, &headers, NULL);

	if (!content)
		return NULL;

	return pem_load_private_key(content, len, label, passphrase, headers,
					encrypted);
}

/**
 * l_pem_load_private_key
 * @filename: path string to the PEM file to load
 * @passphrase: private key encryption passphrase or NULL for unencrypted
 * @encrypted: receives indication whether the file was encrypted if non-NULL
 *
 * Load the PEM encoded RSA Private Key file at @filename.  If it is an
 * encrypted private key and @passphrase was non-NULL, the file is
 * decrypted.  If it's unencrypted @passphrase is ignored.  @encrypted
 * stores information of whether the file was encrypted, both in a
 * success case and on error when NULL is returned.  This can be used to
 * check if a passphrase is required without prior information.
 *
 * The passphrase, if given, must have been validated as UTF-8 unless the
 * caller knows that PKCS#12 encryption algorithms are not used.
 * Use l_utf8_validate.
 *
 * Returns: An l_key object to be freed with an l_key_free* function,
 * or NULL.
 **/
LIB_EXPORT struct l_key *l_pem_load_private_key(const char *filename,
						const char *passphrase,
						bool *encrypted)
{
	uint8_t *content;
	char *label;
	size_t len;
	char *headers;

	if (encrypted)
		*encrypted = false;

	content = pem_load_file(filename, &label, &len, &headers);

	if (!content)
		return NULL;

	return pem_load_private_key(content, len, label, passphrase, headers,
					encrypted);
}
