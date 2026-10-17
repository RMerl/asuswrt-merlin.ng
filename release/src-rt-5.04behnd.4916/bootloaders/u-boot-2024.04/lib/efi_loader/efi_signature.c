// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2018 Patrick Wildt <patrick@blueri.se>
 * Copyright (c) 2019 Linaro Limited, Author: AKASHI Takahiro
 */

#include <charset.h>
#include <efi_loader.h>
#include <efi_variable.h>
#include <image.h>
#include <hexdump.h>
#include <malloc.h>
#include <crypto/pkcs7.h>
#include <crypto/pkcs7_parser.h>
#include <crypto/public_key.h>
#include <linux/compat.h>
#include <linux/oid_registry.h>
#include <u-boot/hash-checksum.h>
#include <u-boot/rsa.h>
#include <u-boot/sha256.h>

const efi_guid_t efi_guid_sha256 = EFI_CERT_SHA256_GUID;
const efi_guid_t efi_guid_cert_rsa2048 = EFI_CERT_RSA2048_GUID;
const efi_guid_t efi_guid_cert_x509 = EFI_CERT_X509_GUID;
const efi_guid_t efi_guid_cert_x509_sha256 = EFI_CERT_X509_SHA256_GUID;
const efi_guid_t efi_guid_cert_x509_sha384 = EFI_CERT_X509_SHA384_GUID;
const efi_guid_t efi_guid_cert_x509_sha512 = EFI_CERT_X509_SHA512_GUID;
const efi_guid_t efi_guid_cert_type_pkcs7 = EFI_CERT_TYPE_PKCS7_GUID;

static u8 pkcs7_hdr[] = {
	/* SEQUENCE */
	0x30, 0x82, 0x05, 0xc7,
	/* OID: pkcs7-signedData */
	0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x02,
	/* Context Structured? */
	0xa0, 0x82, 0x05, 0xb8,
};

/**
 * efi_parse_pkcs7_header - parse a signature in payload
 * @buf:	Pointer to payload's value
 * @buflen:	Length of @buf
 * @tmpbuf:	Pointer to temporary buffer
 *
 * Parse a signature embedded in payload's value and instantiate
 * a pkcs7_message structure. Since pkcs7_parse_message() accepts only
 * pkcs7's signedData, some header needed be prepended for correctly
 * parsing authentication data
 * A temporary buffer will be allocated if needed, and it should be
 * kept valid during the authentication because some data in the buffer
 * will be referenced by efi_signature_verify().
 *
 * Return:	Pointer to pkcs7_message structure on success, NULL on error
 */
struct pkcs7_message *efi_parse_pkcs7_header(const void *buf,
					     size_t buflen,
					     u8 **tmpbuf)
{
	u8 *ebuf;
	size_t ebuflen, len;
	struct pkcs7_message *msg;

	/*
	 * This is the best assumption to check if the binary is
	 * already in a form of pkcs7's signedData.
	 */
	if (buflen > sizeof(pkcs7_hdr) &&
	    !memcmp(&((u8 *)buf)[4], &pkcs7_hdr[4], 11)) {
		msg = pkcs7_parse_message(buf, buflen);
		if (IS_ERR(msg))
			return NULL;
		return msg;
	}

	/*
	 * Otherwise, we should add a dummy prefix sequence for pkcs7
	 * message parser to be able to process.
	 * NOTE: EDK2 also uses similar hack in WrapPkcs7Data()
	 * in CryptoPkg/Library/BaseCryptLib/Pk/CryptPkcs7VerifyCommon.c
	 * TODO:
	 * The header should be composed in a more refined manner.
	 */
	EFI_PRINT("Makeshift prefix added to authentication data\n");
	ebuflen = sizeof(pkcs7_hdr) + buflen;
	if (ebuflen <= 0x7f) {
		EFI_PRINT("Data is too short\n");
		return NULL;
	}

	ebuf = malloc(ebuflen);
	if (!ebuf) {
		EFI_PRINT("Out of memory\n");
		return NULL;
	}

	memcpy(ebuf, pkcs7_hdr, sizeof(pkcs7_hdr));
	memcpy(ebuf + sizeof(pkcs7_hdr), buf, buflen);
	len = ebuflen - 4;
	ebuf[2] = (len >> 8) & 0xff;
	ebuf[3] = len & 0xff;
	len = ebuflen - 0x13;
	ebuf[0x11] = (len >> 8) & 0xff;
	ebuf[0x12] = len & 0xff;

	msg = pkcs7_parse_message(ebuf, ebuflen);

	if (IS_ERR(msg)) {
		free(ebuf);
		return NULL;
	}

	*tmpbuf = ebuf;
	return msg;
}

/**
 * efi_hash_regions - calculate a hash value
 * @regs:	Array of regions
 * @count:	Number of regions
 * @hash:	Pointer to a pointer to buffer holding a hash value
 * @size:	Size of buffer to be returned
 *
 * Calculate a sha256 value of @regs and return a value in @hash.
 *
 * Return:	true on success, false on error
 */
bool efi_hash_regions(struct image_region *regs, int count,
		      void **hash, const char *hash_algo, int *len)
{
	int ret, hash_len;

	if (!hash_algo)
		return false;

	hash_len = algo_to_len(hash_algo);
	if (!hash_len)
		return false;

	if (!*hash) {
		*hash = calloc(1, hash_len);
		if (!*hash) {
			EFI_PRINT("Out of memory\n");
			return false;
		}
	}

	ret = hash_calculate(hash_algo, regs, count, *hash);
	if (ret)
		return false;

	if (len)
		*len = hash_len;
#ifdef DEBUG
	EFI_PRINT("hash calculated:\n");
	print_hex_dump("    ", DUMP_PREFIX_OFFSET, 16, 1,
		       *hash, hash_len, false);
#endif

	return true;
}

/**
 * hash_algo_supported - check if the requested hash algorithm is supported
 * @guid: guid of the algorithm
 *
 * Return: true if supported false otherwise
 */
static bool hash_algo_supported(const efi_guid_t guid)
{
	int i;
	const efi_guid_t unsupported_hashes[] = {
		 EFI_CERT_SHA1_GUID,
		 EFI_CERT_SHA224_GUID,
		 EFI_CERT_SHA384_GUID,
		 EFI_CERT_SHA512_GUID,
	};

	for (i = 0; i < ARRAY_SIZE(unsupported_hashes); i++) {
		if (!guidcmp(&unsupported_hashes[i], &guid))
			return false;
	}

	return true;
}

/**
 * efi_signature_lookup_digest - search for an image's digest in sigdb
 * @regs:	List of regions to be authenticated
 * @db:		Signature database for trusted certificates
 * @dbx		Caller needs to set this to true if he is searching dbx
 *
 * A message digest of image pointed to by @regs is calculated and
 * its hash value is compared to entries in signature database pointed
 * to by @db.
 *
 * Return:	true if found, false if not
 */
bool efi_signature_lookup_digest(struct efi_image_regions *regs,
				 struct efi_signature_store *db,
				 bool dbx)

{
	struct efi_signature_store *siglist;
	struct efi_sig_data *sig_data;
	void *hash = NULL;
	bool found = false;
	bool hash_done = false;

	EFI_PRINT("%s: Enter, %p, %p\n", __func__, regs, db);

	if (!regs || !db || !db->sig_data_list)
		goto out;

	for (siglist = db; siglist; siglist = siglist->next) {
		int len = 0;
		const char *hash_algo = NULL;
		/*
		 * if the hash algorithm is unsupported and we get an entry in
		 * dbx reject the image
		 */
		if (dbx && !hash_algo_supported(siglist->sig_type)) {
			found = true;
			continue;
		};
		/*
		 * Only support sha256 for now, that's what
		 * hash-to-efi-sig-list produces
		 */
		if (guidcmp(&siglist->sig_type, &efi_guid_sha256))
			continue;

		hash_algo = guid_to_sha_str(&efi_guid_sha256);
		/*
		 * We could check size and hash_algo but efi_hash_regions()
		 * will do that for us
		 */
		if (!hash_done &&
		    !efi_hash_regions(regs->reg, regs->num, &hash, hash_algo,
				      &len)) {
			EFI_PRINT("Digesting an image failed\n");
			break;
		}
		hash_done = true;

		for (sig_data = siglist->sig_data_list; sig_data;
		     sig_data = sig_data->next) {
#ifdef DEBUG
			EFI_PRINT("Msg digest in database:\n");
			print_hex_dump("    ", DUMP_PREFIX_OFFSET, 16, 1,
				       sig_data->data, sig_data->size, false);
#endif
			if (sig_data->size == len &&
			    !memcmp(sig_data->data, hash, len)) {
				found = true;
				free(hash);
				goto out;
			}
		}

		free(hash);
		hash = NULL;
	}

out:
	EFI_PRINT("%s: Exit, found: %d\n", __func__, found);
	return found;
}

/**
 * efi_lookup_certificate - find a certificate within db
 * @msg:	Signature
 * @db:		Signature database
 *
 * Search signature database pointed to by @db and find a certificate
 * pointed to by @cert.
 *
 * Return:	true if found, false otherwise.
 */
static bool efi_lookup_certificate(struct x509_certificate *cert,
				   struct efi_signature_store *db)
{
	struct efi_signature_store *siglist;
	struct efi_sig_data *sig_data;
	struct image_region reg[1];
	void *hash = NULL, *hash_tmp = NULL;
	int len = 0;
	bool found = false;
	const char *hash_algo = NULL;

	EFI_PRINT("%s: Enter, %p, %p\n", __func__, cert, db);

	if (!cert || !db || !db->sig_data_list)
		goto out;

	/*
	 * TODO: identify a certificate using sha256 digest
	 * Is there any better way?
	 */
	/* calculate hash of TBSCertificate */
	reg[0].data = cert->tbs;
	reg[0].size = cert->tbs_size;

	/* We just need any sha256 algo to start the matching */
	hash_algo = guid_to_sha_str(&efi_guid_sha256);
	if (!efi_hash_regions(reg, 1, &hash, hash_algo, &len))
		goto out;

	EFI_PRINT("%s: searching for %s\n", __func__, cert->subject);
	for (siglist = db; siglist; siglist = siglist->next) {
		/* only with x509 certificate */
		if (guidcmp(&siglist->sig_type, &efi_guid_cert_x509))
			continue;

		for (sig_data = siglist->sig_data_list; sig_data;
		     sig_data = sig_data->next) {
			struct x509_certificate *cert_tmp;

			cert_tmp = x509_cert_parse(sig_data->data,
						   sig_data->size);
			if (IS_ERR_OR_NULL(cert_tmp))
				continue;

			EFI_PRINT("%s: against %s\n", __func__,
				  cert_tmp->subject);
			reg[0].data = cert_tmp->tbs;
			reg[0].size = cert_tmp->tbs_size;
			if (!efi_hash_regions(reg, 1, &hash_tmp, hash_algo,
					      NULL))
				goto out;

			x509_free_certificate(cert_tmp);

			if (!memcmp(hash, hash_tmp, len)) {
				found = true;
				goto out;
			}
		}
	}
out:
	free(hash);
	free(hash_tmp);

	EFI_PRINT("%s: Exit, found: %d\n", __func__, found);
	return found;
}

/**
 * efi_verify_certificate - verify certificate's signature with database
 * @signer:	Certificate
 * @db:		Signature database
 * @root:	Certificate to verify @signer
 *
 * Determine if certificate pointed to by @signer may be verified
 * by one of certificates in signature database pointed to by @db.
 *
 * Return:	true if certificate is verified, false otherwise.
 */
static bool efi_verify_certificate(struct x509_certificate *signer,
				   struct efi_signature_store *db,
				   struct x509_certificate **root)
{
	struct efi_signature_store *siglist;
	struct efi_sig_data *sig_data;
	struct x509_certificate *cert;
	bool verified = false;
	int ret;

	EFI_PRINT("%s: Enter, %p, %p\n", __func__, signer, db);

	if (!signer || !db || !db->sig_data_list)
		goto out;

	for (siglist = db; siglist; siglist = siglist->next) {
		/* only with x509 certificate */
		if (guidcmp(&siglist->sig_type, &efi_guid_cert_x509))
			continue;

		for (sig_data = siglist->sig_data_list; sig_data;
		     sig_data = sig_data->next) {
			cert = x509_cert_parse(sig_data->data, sig_data->size);
			if (IS_ERR_OR_NULL(cert)) {
				EFI_PRINT("Cannot parse x509 certificate\n");
				continue;
			}

			ret = public_key_verify_signature(cert->pub,
							  signer->sig);
			if (!ret) {
				verified = true;
				if (root)
					*root = cert;
				else
					x509_free_certificate(cert);
				goto out;
			}
			x509_free_certificate(cert);
		}
	}

out:
	EFI_PRINT("%s: Exit, verified: %d\n", __func__, verified);
	return verified;
}

/**
 * efi_signature_check_revocation - check revocation with dbx
 * @sinfo:	Signer's info
 * @cert:	x509 certificate
 * @dbx:	Revocation signature database
 *
 * Search revocation signature database pointed to by @dbx and find
 * an entry matching to certificate pointed to by @cert.
 *
 * While this entry contains revocation time, we don't support timestamp
 * protocol at this time and any image will be unconditionally revoked
 * when this match occurs.
 *
 * Return:	true if check passed (not found), false otherwise.
 */
static bool efi_signature_check_revocation(struct pkcs7_signed_info *sinfo,
					   struct x509_certificate *cert,
					   struct efi_signature_store *dbx)
{
	struct efi_signature_store *siglist;
	struct efi_sig_data *sig_data;
	struct image_region reg[1];
	void *hash = NULL;
	int len = 0;
	time64_t revoc_time;
	bool revoked = false;
	const char *hash_algo = NULL;

	EFI_PRINT("%s: Enter, %p, %p, %p\n", __func__, sinfo, cert, dbx);

	if (!sinfo || !cert || !dbx || !dbx->sig_data_list)
		goto out;

	EFI_PRINT("Checking revocation against %s\n", cert->subject);
	for (siglist = dbx; siglist; siglist = siglist->next) {
		hash_algo = guid_to_sha_str(&siglist->sig_type);
		if (!hash_algo)
			continue;

		/* calculate hash of TBSCertificate */
		reg[0].data = cert->tbs;
		reg[0].size = cert->tbs_size;
		if (!efi_hash_regions(reg, 1, &hash, hash_algo, &len))
			goto out;

		for (sig_data = siglist->sig_data_list; sig_data;
		     sig_data = sig_data->next) {
			/*
			 * struct efi_cert_x509_sha256 {
			 *	u8 tbs_hash[256/8];
			 *	time64_t revocation_time;
			 * };
			 */
#ifdef DEBUG
			if (sig_data->size >= len) {
				EFI_PRINT("hash in db:\n");
				print_hex_dump("    ", DUMP_PREFIX_OFFSET,
					       16, 1,
					       sig_data->data, len, false);
			}
#endif
			if ((sig_data->size < len + sizeof(time64_t)) ||
			    memcmp(sig_data->data, hash, len))
				continue;

			memcpy(&revoc_time, sig_data->data + len,
			       sizeof(revoc_time));
			EFI_PRINT("revocation time: 0x%llx\n", revoc_time);
			/*
			 * TODO: compare signing timestamp in sinfo
			 * with revocation time
			 */

			revoked = true;
			free(hash);
			goto out;
		}
		free(hash);
		hash = NULL;
	}
out:
	EFI_PRINT("%s: Exit, revoked: %d\n", __func__, revoked);
	return !revoked;
}

/*
 * efi_signature_verify - verify signatures with db and dbx
 * @regs:	List of regions to be authenticated
 * @msg:	Signature
 * @db:		Signature database for trusted certificates
 * @dbx:	Revocation signature database
 *
 * All the signature pointed to by @msg against image pointed to by @regs
 * will be verified by signature database pointed to by @db and @dbx.
 *
 * Return:	true if verification for all signatures passed, false otherwise
 */
bool efi_signature_verify(struct efi_image_regions *regs,
			  struct pkcs7_message *msg,
			  struct efi_signature_store *db,
			  struct efi_signature_store *dbx)
{
	struct pkcs7_signed_info *sinfo;
	struct x509_certificate *signer, *root;
	bool verified = false;
	int ret;

	EFI_PRINT("%s: Enter, %p, %p, %p, %p\n", __func__, regs, msg, db, dbx);

	if (!regs || !msg || !db || !db->sig_data_list)
		goto out;

	for (sinfo = msg->signed_infos; sinfo; sinfo = sinfo->next) {
		EFI_PRINT("Signed Info: digest algo: %s, pkey algo: %s\n",
			  sinfo->sig->hash_algo, sinfo->sig->pkey_algo);

		/*
		 * only for authenticated variable.
		 *
		 * If this function is called for image,
		 * hash calculation will be done in
		 * pkcs7_verify_one().
		 */
		if (!msg->data &&
		    !efi_hash_regions(regs->reg, regs->num,
				      (void **)&sinfo->sig->digest,
				      guid_to_sha_str(&efi_guid_sha256),
				      NULL)) {
			EFI_PRINT("Digesting an image failed\n");
			goto out;
		}

		EFI_PRINT("Verifying certificate chain\n");
		signer = NULL;
		ret = pkcs7_verify_one(msg, sinfo, &signer);
		if (ret == -ENOPKG)
			continue;

		if (ret < 0 || !signer)
			goto out;

		if (sinfo->blacklisted)
			goto out;

		EFI_PRINT("Verifying last certificate in chain\n");
		if (efi_lookup_certificate(signer, db))
			if (efi_signature_check_revocation(sinfo, signer, dbx))
				break;
		if (!signer->self_signed &&
		    efi_verify_certificate(signer, db, &root)) {
			bool check;

			check = efi_signature_check_revocation(sinfo, root,
							       dbx);
			x509_free_certificate(root);
			if (check)
				break;
		}

		EFI_PRINT("Certificate chain didn't reach trusted CA\n");
	}
	if (sinfo)
		verified = true;
out:
	EFI_PRINT("%s: Exit, verified: %d\n", __func__, verified);
	return verified;
}

/**
 * efi_signature_check_signers - check revocation against all signers with dbx
 * @msg:	Signature
 * @dbx:	Revocation signature database
 *
 * Determine if none of signers' certificates in @msg are revoked
 * by signature database pointed to by @dbx.
 *
 * Return:	true if all signers passed, false otherwise.
 */
bool efi_signature_check_signers(struct pkcs7_message *msg,
				 struct efi_signature_store *dbx)
{
	struct pkcs7_signed_info *sinfo;
	bool revoked = false;

	EFI_PRINT("%s: Enter, %p, %p\n", __func__, msg, dbx);

	if (!msg || !dbx)
		goto out;

	for (sinfo = msg->signed_infos; sinfo; sinfo = sinfo->next) {
		if (sinfo->signer &&
		    !efi_signature_check_revocation(sinfo, sinfo->signer,
						    dbx)) {
			revoked = true;
			break;
		}
	}
out:
	EFI_PRINT("%s: Exit, revoked: %d\n", __func__, revoked);
	return !revoked;
}

/**
 * efi_sigstore_free - free signature store
 * @sigstore:	Pointer to signature store structure
 *
 * Feee all the memories held in signature store and itself,
 * which were allocated by efi_sigstore_parse_sigdb().
 */
void efi_sigstore_free(struct efi_signature_store *sigstore)
{
	struct efi_signature_store *sigstore_next;
	struct efi_sig_data *sig_data, *sig_data_next;

	while (sigstore) {
		sigstore_next = sigstore->next;

		sig_data = sigstore->sig_data_list;
		while (sig_data) {
			sig_data_next = sig_data->next;
			free(sig_data->data);
			free(sig_data);
			sig_data = sig_data_next;
		}

		free(sigstore);
		sigstore = sigstore_next;
	}
}

/**
 * efi_sigstore_parse_siglist - parse a signature list
 * @name:	Pointer to signature list
 *
 * Parse signature list and instantiate a signature store structure.
 * Signature database is a simple concatenation of one or more
 * signature list(s).
 *
 * Return:	Pointer to signature store on success, NULL on error
 */
static struct efi_signature_store *
efi_sigstore_parse_siglist(struct efi_signature_list *esl)
{
	struct efi_signature_store *siglist = NULL;
	struct efi_sig_data *sig_data, *sig_data_next;
	struct efi_signature_data *esd;
	size_t left;

	/*
	 * UEFI specification defines certificate types:
	 *   for non-signed images,
	 *	EFI_CERT_SHA256_GUID
	 *	EFI_CERT_RSA2048_GUID
	 *	EFI_CERT_RSA2048_SHA256_GUID
	 *	EFI_CERT_SHA1_GUID
	 *	EFI_CERT_RSA2048_SHA_GUID
	 *	EFI_CERT_SHA224_GUID
	 *	EFI_CERT_SHA384_GUID
	 *	EFI_CERT_SHA512_GUID
	 *
	 *   for signed images,
	 *	EFI_CERT_X509_GUID
	 *	NOTE: Each certificate will normally be in a separate
	 *	EFI_SIGNATURE_LIST as the size may vary depending on
	 *	its algo's.
	 *
	 *   for timestamp revocation of certificate,
	 *	EFI_CERT_X509_SHA512_GUID
	 *	EFI_CERT_X509_SHA256_GUID
	 *	EFI_CERT_X509_SHA384_GUID
	 */

	if (esl->signature_list_size
			<= (sizeof(*esl) + esl->signature_header_size)) {
		EFI_PRINT("Siglist in wrong format\n");
		return NULL;
	}

	/* Create a head */
	siglist = calloc(sizeof(*siglist), 1);
	if (!siglist) {
		EFI_PRINT("Out of memory\n");
		goto err;
	}
	memcpy(&siglist->sig_type, &esl->signature_type, sizeof(efi_guid_t));

	/* Go through the list */
	sig_data_next = NULL;
	left = esl->signature_list_size
			- (sizeof(*esl) + esl->signature_header_size);
	esd = (struct efi_signature_data *)
			((u8 *)esl + sizeof(*esl) + esl->signature_header_size);

	while (left > 0) {
		/* Signature must exist if there is remaining data. */
		if (left < esl->signature_size) {
			EFI_PRINT("Certificate is too small\n");
			goto err;
		}

		sig_data = calloc(esl->signature_size
					- sizeof(esd->signature_owner), 1);
		if (!sig_data) {
			EFI_PRINT("Out of memory\n");
			goto err;
		}

		/* Append signature data */
		memcpy(&sig_data->owner, &esd->signature_owner,
		       sizeof(efi_guid_t));
		sig_data->size = esl->signature_size
					- sizeof(esd->signature_owner);
		sig_data->data = malloc(sig_data->size);
		if (!sig_data->data) {
			EFI_PRINT("Out of memory\n");
			goto err;
		}
		memcpy(sig_data->data, esd->signature_data, sig_data->size);

		sig_data->next = sig_data_next;
		sig_data_next = sig_data;

		/* Next */
		esd = (struct efi_signature_data *)
				((u8 *)esd + esl->signature_size);
		left -= esl->signature_size;
	}
	siglist->sig_data_list = sig_data_next;

	return siglist;

err:
	efi_sigstore_free(siglist);

	return NULL;
}

/**
 * efi_sigstore_parse_sigdb - parse the signature list and populate
 * the signature store
 *
 * @sig_list:	Pointer to the signature list
 * @size:	Size of the signature list
 *
 * Parse the efi signature list and instantiate a signature store
 * structure.
 *
 * Return:	Pointer to signature store on success, NULL on error
 */
struct efi_signature_store *efi_build_signature_store(void *sig_list,
						      efi_uintn_t size)
{
	struct efi_signature_list *esl;
	struct efi_signature_store *sigstore = NULL, *siglist;

	esl = sig_list;
	while (size > 0) {
		/* List must exist if there is remaining data. */
		if (size < sizeof(*esl)) {
			EFI_PRINT("Signature list in wrong format\n");
			goto err;
		}

		if (size < esl->signature_list_size) {
			EFI_PRINT("Signature list in wrong format\n");
			goto err;
		}

		/* Parse a single siglist. */
		siglist = efi_sigstore_parse_siglist(esl);
		if (!siglist) {
			EFI_PRINT("Parsing of signature list of failed\n");
			goto err;
		}

		/* Append siglist */
		siglist->next = sigstore;
		sigstore = siglist;

		/* Next */
		size -= esl->signature_list_size;
		esl = (void *)esl + esl->signature_list_size;
	}
	free(sig_list);

	return sigstore;

err:
	efi_sigstore_free(sigstore);
	free(sig_list);

	return NULL;
}

/**
 * efi_sigstore_parse_sigdb - parse a signature database variable
 * @name:	Variable's name
 *
 * Read in a value of signature database variable pointed to by
 * @name, parse it and instantiate a signature store structure.
 *
 * Return:	Pointer to signature store on success, NULL on error
 */
struct efi_signature_store *efi_sigstore_parse_sigdb(u16 *name)
{
	const efi_guid_t *vendor;
	void *db;
	efi_uintn_t db_size;

	vendor = efi_auth_var_get_guid(name);
	db = efi_get_var(name, vendor, &db_size);
	if (!db) {
		EFI_PRINT("variable, %ls, not found\n", name);
		return calloc(sizeof(struct efi_signature_store), 1);
	}

	return efi_build_signature_store(db, db_size);
}
