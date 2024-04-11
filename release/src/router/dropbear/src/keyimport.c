/*
 * Based on PuTTY's import.c for importing/exporting OpenSSH and SSH.com
 * keyfiles.
 *
 * Modifications copyright 2003-2022 Matt Johnston
 *
 * PuTTY is copyright 1997-2003 Simon Tatham.
 * 
 * Portions copyright Robert de Bath, Joris van Rantwijk, Delian
 * Delchev, Andreas Schultz, Jeroen Massar, Wez Furlong, Nicolas Barry,
 * Justin Bradford, and CORE SDI S.A.
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "keyimport.h"
#include "bignum.h"
#include "buffer.h"
#include "dbutil.h"
#include "ecc.h"
#include "ssh.h"
#include "rsa.h"
#include "dss.h"
#include "ed25519.h"
#include "ecdsa.h"
#include "signkey_ossh.h"

static const unsigned char OSSH_PKEY_BLOB[] =
	"openssh-key-v1\0"			/* AUTH_MAGIC */
	"\0\0\0\4none"				/* cipher name*/
	"\0\0\0\4none"				/* kdf name */
	"\0\0\0\0"				/* kdf */
	"\0\0\0\1";				/* key num */
#define OSSH_PKEY_BLOBLEN (sizeof(OSSH_PKEY_BLOB) - 1)
#if DROPBEAR_ECDSA
static const unsigned char OID_SEC256R1_BLOB[] = {0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07};
static const unsigned char OID_SEC384R1_BLOB[] = {0x2b, 0x81, 0x04, 0x00, 0x22};
static const unsigned char OID_SEC521R1_BLOB[] = {0x2b, 0x81, 0x04, 0x00, 0x23};
#endif

#define PUT_32BIT(cp, value) do { \
  (cp)[3] = (unsigned char)(value); \
  (cp)[2] = (unsigned char)((value) >> 8); \
  (cp)[1] = (unsigned char)((value) >> 16); \
  (cp)[0] = (unsigned char)((value) >> 24); } while (0)

#define GET_32BIT(cp) \
	(((unsigned long)(unsigned char)(cp)[0] << 24) | \
	((unsigned long)(unsigned char)(cp)[1] << 16) | \
	((unsigned long)(unsigned char)(cp)[2] << 8) | \
	((unsigned long)(unsigned char)(cp)[3]))

static int openssh_encrypted(const char *filename);
static sign_key *openssh_read(const char *filename, const char *passphrase);
static int openssh_write(const char *filename, sign_key *key,
				  const char *passphrase);

static int dropbear_write(const char*filename, sign_key * key);
static sign_key *dropbear_read(const char* filename);

static int toint(unsigned u);

#if 0
static int sshcom_encrypted(const char *filename, char **comment);
static struct ssh2_userkey *sshcom_read(const char *filename, char *passphrase);
static int sshcom_write(const char *filename, struct ssh2_userkey *key,
				 char *passphrase);
#endif

int import_encrypted(const char* filename, int filetype) {

	if (filetype == KEYFILE_OPENSSH) {
		return openssh_encrypted(filename);
#if 0
	} else if (filetype == KEYFILE_SSHCOM) {
		return sshcom_encrypted(filename, NULL);
#endif
	}
	return 0;
}

sign_key *import_read(const char *filename, const char *passphrase, int filetype) {

	if (filetype == KEYFILE_OPENSSH) {
		return openssh_read(filename, passphrase);
	} else if (filetype == KEYFILE_DROPBEAR) {
		return dropbear_read(filename);
#if 0
	} else if (filetype == KEYFILE_SSHCOM) {
		return sshcom_read(filename, passphrase);
#endif
	}
	return NULL;
}

int import_write(const char *filename, sign_key *key, const char *passphrase,
		int filetype) {

	if (filetype == KEYFILE_OPENSSH) {
		return openssh_write(filename, key, passphrase);
	} else if (filetype == KEYFILE_DROPBEAR) {
		return dropbear_write(filename, key);
#if 0
	} else if (filetype == KEYFILE_SSHCOM) {
		return sshcom_write(filename, key, passphrase);
#endif
	}
	return 0;
}

static sign_key *dropbear_read(const char* filename) {

	buffer * buf = NULL;
	sign_key *ret = NULL;
	enum signkey_type type;

	buf = buf_new(MAX_PRIVKEY_SIZE);
	if (buf_readfile(buf, filename) == DROPBEAR_FAILURE) {
		goto error;
	}

	buf_setpos(buf, 0);
	ret = new_sign_key();

	type = DROPBEAR_SIGNKEY_ANY;
	if (buf_get_priv_key(buf, ret, &type) == DROPBEAR_FAILURE){
		goto error;
	}
	buf_free(buf);

	ret->type = type;

	return ret;

error:
	if (buf) {
		buf_free(buf);
	}
	if (ret) {
		sign_key_free(ret);
	}
	return NULL;
}

/* returns 0 on fail, 1 on success */
static int dropbear_write(const char*filename, sign_key * key) {

	buffer * buf;
	FILE*fp;
	int len;
	int ret;

	buf = buf_new(MAX_PRIVKEY_SIZE);
	buf_put_priv_key(buf, key, key->type);

	fp = fopen(filename, "w");
	if (!fp) {
		ret = 0;
		goto out;
	}

	buf_setpos(buf, 0);
	do {
		len = fwrite(buf_getptr(buf, buf->len - buf->pos),
				1, buf->len - buf->pos, fp);
		buf_incrpos(buf, len);
	} while (len > 0 && buf->len != buf->pos);

	fclose(fp);

	if (buf->pos != buf->len) {
		ret = 0;
	} else {
		ret = 1;
	}
out:
	buf_free(buf);
	return ret;
}


/* ----------------------------------------------------------------------
 * Helper routines. (The base64 ones are defined in sshpubk.c.)
 */

#define isbase64(c) (	((c) >= 'A' && (c) <= 'Z') || \
						 ((c) >= 'a' && (c) <= 'z') || \
						 ((c) >= '0' && (c) <= '9') || \
						 (c) == '+' || (c) == '/' || (c) == '=' \
						 )

/* cpl has to be less than 100 */
static void base64_encode_fp(FILE * fp, const unsigned char *data,
		int datalen, int cpl)
{
	unsigned char out[100];
	int n;
	unsigned long outlen;
	int rawcpl;
	rawcpl = cpl * 3 / 4;
	dropbear_assert((unsigned int)cpl < sizeof(out));

	while (datalen > 0) {
		n = (datalen < rawcpl ? datalen : rawcpl);
		outlen = sizeof(out);
		base64_encode(data, n, out, &outlen);
		data += n;
		datalen -= n;
		fwrite(out, 1, outlen, fp);
		fputc('\n', fp);
	}
}
/*
 * Read an ASN.1/BER identifier and length pair.
 * 
 * Flags are a combination of the #defines listed below.
 * 
 * Returns -1 if unsuccessful; otherwise returns the number of
 * bytes used out of the source data.
 */

/* ASN.1 tag classes. */
#define ASN1_CLASS_UNIVERSAL		(0 << 6)
#define ASN1_CLASS_APPLICATION	  (1 << 6)
#define ASN1_CLASS_CONTEXT_SPECIFIC (2 << 6)
#define ASN1_CLASS_PRIVATE		  (3 << 6)
#define ASN1_CLASS_MASK			 (3 << 6)

/* Primitive versus constructed bit. */
#define ASN1_CONSTRUCTED			(1 << 5)

static int ber_read_id_len(void *source, int sourcelen,
						   int *id, int *length, int *flags)
{
	unsigned char *p = (unsigned char *) source;

	if (sourcelen == 0)
		return -1;

	*flags = (*p & 0xE0);
	if ((*p & 0x1F) == 0x1F) {
		*id = 0;
		while (*p & 0x80) {
			p++, sourcelen--;
			if (sourcelen == 0)
				return -1;
			*id = (*id << 7) | (*p & 0x7F);
		}
		p++, sourcelen--;
	} else {
		*id = *p & 0x1F;
		p++, sourcelen--;
	}

	if (sourcelen == 0)
		return -1;

	if (*p & 0x80) {
		unsigned len;
		int n = *p & 0x7F;
		p++, sourcelen--;
		if (sourcelen < n)
			return -1;
		len = 0;
		while (n--)
			len = (len << 8) | (*p++);
		sourcelen -= n;
		*length = toint(len);
	} else {
		*length = *p;
		p++, sourcelen--;
	}

	if (*length < 0) {
		printf("Negative ASN.1 length\n");
		return -1;
	}

	return p - (unsigned char *) source;
}

/*
 * Write an ASN.1/BER identifier and length pair. Returns the
 * number of bytes consumed. Assumes dest contains enough space.
 * Will avoid writing anything if dest is NULL, but still return
 * amount of space required.
 */
#if DROPBEAR_DSS
static int ber_write_id_len(void *dest, int id, int length, int flags)
{
	unsigned char *d = (unsigned char *)dest;
	int len = 0;

	if (id <= 30) {
		/*
		 * Identifier is one byte.
		 */
		len++;
		if (d) *d++ = id | flags;
	} else {
		int n;
		/*
		 * Identifier is multiple bytes: the first byte is 11111
		 * plus the flags, and subsequent bytes encode the value of
		 * the identifier, 7 bits at a time, with the top bit of
		 * each byte 1 except the last one which is 0.
		 */
		len++;
		if (d) *d++ = 0x1F | flags;
		for (n = 1; (id >> (7*n)) > 0; n++)
			continue;					   /* count the bytes */
		while (n--) {
			len++;
			if (d) *d++ = (n ? 0x80 : 0) | ((id >> (7*n)) & 0x7F);
		}
	}

	if (length < 128) {
		/*
		 * Length is one byte.
		 */
		len++;
		if (d) *d++ = length;
	} else {
		int n;
		/*
		 * Length is multiple bytes. The first is 0x80 plus the
		 * number of subsequent bytes, and the subsequent bytes
		 * encode the actual length.
		 */
		for (n = 1; (length >> (8*n)) > 0; n++)
			continue;					   /* count the bytes */
		len++;
		if (d) *d++ = 0x80 | n;
		while (n--) {
			len++;
			if (d) *d++ = (length >> (8*n)) & 0xFF;
		}
	}

	return len;
}
#endif /* DROPBEAR_DSS */


/* Simple structure to point to an mp-int within a blob. */
struct mpint_pos { void *start; int bytes; };

/* ----------------------------------------------------------------------
 * Code to read and write OpenSSH private keys.
 */

enum { OSSH_DSA, OSSH_RSA, OSSH_EC, OSSH_PKEY };
struct openssh_key {
	int type;
	int encrypted;
	char iv[32];
	/* keyblob is publickey1 onwards (ref OpenSSH PROTOCOL.key) */
	unsigned char *keyblob;
	unsigned int keyblob_len, keyblob_size;
};

static struct openssh_key *load_openssh_key(const char *filename)
{
	struct openssh_key *ret;
	buffer *buf = NULL;
	FILE *fp = NULL;
	char buffer[256];
	char *errmsg = NULL, *p = NULL;
	int headers_done;
	unsigned long len;

	ret = (struct openssh_key*)m_malloc(sizeof(struct openssh_key));
	ret->keyblob = NULL;
	ret->keyblob_len = ret->keyblob_size = 0;
	ret->encrypted = 0;
	memset(ret->iv, 0, sizeof(ret->iv));

	if (strlen(filename) == 1 && filename[0] == '-') {
		fp = stdin;
	} else {
		fp = fopen(filename, "r");
	}
	if (!fp) {
		errmsg = "Unable to open key file";
		goto error;
	}
	if (!fgets(buffer, sizeof(buffer), fp) ||
		0 != strncmp(buffer, "-----BEGIN ", 11) ||
		0 != strcmp(buffer+strlen(buffer)-17, "PRIVATE KEY-----\n")) {
		errmsg = "File does not begin with OpenSSH key header";
		goto error;
	}
	if (!strcmp(buffer, "-----BEGIN RSA PRIVATE KEY-----\n"))
		ret->type = OSSH_RSA;
	else if (!strcmp(buffer, "-----BEGIN DSA PRIVATE KEY-----\n"))
		ret->type = OSSH_DSA;
	else if (!strcmp(buffer, "-----BEGIN EC PRIVATE KEY-----\n"))
		ret->type = OSSH_EC;
	else if (!strcmp(buffer, "-----BEGIN OPENSSH PRIVATE KEY-----\n"))
		ret->type = OSSH_PKEY;
	else {
		errmsg = "Unrecognised key type";
		goto error;
	}

	headers_done = 0;
	buf = buf_new(0);
	while (1) {
		if (!fgets(buffer, sizeof(buffer), fp)) {
			errmsg = "Unexpected end of file";
			goto error;
		}
		if (0 == strncmp(buffer, "-----END ", 9) &&
			0 == strcmp(buffer+strlen(buffer)-17, "PRIVATE KEY-----\n"))
			break;					   /* done */
		if ((p = strchr(buffer, ':')) != NULL) {
			if (headers_done) {
				errmsg = "Header found in body of key data";
				goto error;
			}
			*p++ = '\0';
			while (*p && isspace((unsigned char)*p)) p++;
			if (!strcmp(buffer, "Proc-Type")) {
				if (p[0] != '4' || p[1] != ',') {
					errmsg = "Proc-Type is not 4 (only 4 is supported)";
					goto error;
				}
				p += 2;
				if (!strcmp(p, "ENCRYPTED\n"))
					ret->encrypted = 1;
			} else if (!strcmp(buffer, "DEK-Info")) {
				int i, j;

				if (strncmp(p, "DES-EDE3-CBC,", 13)) {
					errmsg = "Ciphers other than DES-EDE3-CBC not supported";
					goto error;
				}
				p += 13;
				for (i = 0; i < 8; i++) {
					if (1 != sscanf(p, "%2x", &j))
						break;
					ret->iv[i] = j;
					p += 2;
				}
				if (i < 8) {
					errmsg = "Expected 16-digit iv in DEK-Info";
					goto error;
				}
			}
		} else {
			headers_done = 1;
			len = strlen(buffer);
			buf = buf_resize(buf, buf->size + len);
			buf_putbytes(buf, buffer, len);
		}
	}

	if (buf && buf->len) {
		ret->keyblob_size = ret->keyblob_len + buf->len*4/3 + 256;
		ret->keyblob = (unsigned char*)m_realloc(ret->keyblob, ret->keyblob_size);
		len = ret->keyblob_size;
		if (base64_decode((const unsigned char *)buf->data, buf->len,
					ret->keyblob, &len) != CRYPT_OK){
			errmsg = "Error decoding base64";
			goto error;
		}
		ret->keyblob_len = len;
	}

	if (ret->type == OSSH_PKEY) {
		if (ret->keyblob_len < OSSH_PKEY_BLOBLEN ||
				memcmp(ret->keyblob, OSSH_PKEY_BLOB, OSSH_PKEY_BLOBLEN)) {
			errmsg = "Error decoding OpenSSH key";
			goto error;
		}
		ret->keyblob_len -= OSSH_PKEY_BLOBLEN;
		memmove(ret->keyblob, ret->keyblob + OSSH_PKEY_BLOBLEN, ret->keyblob_len);
	}

	if (ret->keyblob_len == 0 || !ret->keyblob) {
		errmsg = "Key body not present";
		goto error;
	}

	if (ret->encrypted && ret->keyblob_len % 8 != 0) {
		errmsg = "Encrypted key blob is not a multiple of cipher block size";
		goto error;
	}

	if (buf) {
		buf_burn_free(buf);
	}
	m_burn(buffer, sizeof(buffer));
	return ret;

error:
	if (buf) {
		buf_burn_free(buf);
	}
	m_burn(buffer, sizeof(buffer));
	if (ret) {
		if (ret->keyblob) {
			m_burn(ret->keyblob, ret->keyblob_size);
			m_free(ret->keyblob);
		}
		m_free(ret);
	}
	if (fp) {
		fclose(fp);
	}
	if (errmsg) {
		fprintf(stderr, "Error: %s\n", errmsg);
	}
	return NULL;
}

static int openssh_encrypted(const char *filename)
{
	struct openssh_key *key = load_openssh_key(filename);
	int ret;

	if (!key)
		return 0;
	ret = key->encrypted;
	m_burn(key->keyblob, key->keyblob_size);
	m_free(key->keyblob);
	m_free(key);
	return ret;
}

static sign_key *openssh_read(const char *filename, const char * UNUSED(passphrase))
{
	struct openssh_key *key;
	unsigned char *p;
	int ret, id, len, flags;
	int i, num_integers = 0;
	sign_key *retval = NULL;
	char *errmsg;
	unsigned char *modptr = NULL;
	int modlen = -9999;
	enum signkey_type type;

	sign_key *retkey;
	buffer * blobbuf = NULL;

	retkey = new_sign_key();

	key = load_openssh_key(filename);

	if (!key)
		return NULL;

	if (key->encrypted) {
		errmsg = "Encrypted keys are not supported. Please convert with ssh-keygen first";
		goto error;
	}

	/*
	 * Now we have a decrypted key blob, which contains OpenSSH
	 * encoded private key. We must now untangle the OpenSSH format.
	 */
	if (key->type == OSSH_PKEY) {
		blobbuf = buf_new(key->keyblob_len);
		buf_putbytes(blobbuf, key->keyblob, key->keyblob_len);
		buf_setpos(blobbuf, 0);

		/* limit length of public key blob */
		len = buf_getint(blobbuf);

		type = DROPBEAR_SIGNKEY_ANY;
		if (buf_get_pub_key(blobbuf, retkey, &type)
				!= DROPBEAR_SUCCESS) {
			errmsg = "Error parsing OpenSSH key";
			goto ossh_error;
		}

		/* restore full length */
		buf_setlen(blobbuf, key->keyblob_len);

		/* length of private key part. we can discard it */
		buf_getint(blobbuf);

		/* discard checkkey1 */
		buf_getint(blobbuf);
		/* discard checkkey2 */
		buf_getint(blobbuf);

		errmsg = "Unsupported OpenSSH key type";
		retkey->type = type;
		ret = DROPBEAR_FAILURE;
		/* Parse private key part */
#if DROPBEAR_RSA
		if (type == DROPBEAR_SIGNKEY_RSA) {
			errmsg = "Error parsing OpenSSH RSA key";
			ret = buf_get_rsa_priv_ossh(blobbuf, retkey);
		}
#endif
#if DROPBEAR_ED25519
		if (type == DROPBEAR_SIGNKEY_ED25519) {
			errmsg = "Error parsing OpenSSH ed25519 key";
			ret = buf_get_ed25519_priv_ossh(blobbuf, retkey);
		}
#endif
#if DROPBEAR_ECDSA
		if (signkey_is_ecdsa(type)) {
			errmsg = "Error parsing OpenSSH ecdsa key";
			ret = buf_get_ecdsa_priv_ossh(blobbuf, retkey);
		}
#endif
		if (ret == DROPBEAR_SUCCESS) {
				errmsg = NULL;
				retval = retkey;
				goto error;
		}

ossh_error:
		sign_key_free(retkey);
		retkey = NULL;
		goto error;
	}

	/*
	 * Now we have a decrypted key blob, which contains an ASN.1
	 * encoded private key. We must now untangle the ASN.1.
	 *
	 * We expect the whole key blob to be formatted as a SEQUENCE
	 * (0x30 followed by a length code indicating that the rest of
	 * the blob is part of the sequence). Within that SEQUENCE we
	 * expect to see a bunch of INTEGERs. What those integers mean
	 * depends on the key type:
	 *
	 *  - For RSA, we expect the integers to be 0, n, e, d, p, q,
	 *	dmp1, dmq1, iqmp in that order. (The last three are d mod
	 *	(p-1), d mod (q-1), inverse of q mod p respectively.)
	 *
	 *  - For DSA, we expect them to be 0, p, q, g, y, x in that
	 *	order.
	 */
	
	p = key->keyblob;

	/* Expect the SEQUENCE header. Take its absence as a failure to decrypt. */
	ret = ber_read_id_len(p, key->keyblob_len, &id, &len, &flags);
	p += ret;
	if (ret < 0 || id != 16 || len < 0 ||
		key->keyblob+key->keyblob_len-p < len) {
				errmsg = "ASN.1 decoding failure";
		goto error;
	}

	/* Expect a load of INTEGERs. */
	if (key->type == OSSH_RSA)
		num_integers = 9;
	else if (key->type == OSSH_DSA)
		num_integers = 6;
	else if (key->type == OSSH_EC)
		num_integers = 1;

	/*
	 * Space to create key blob in.
	 */
	blobbuf = buf_new(3000);

#if DROPBEAR_DSS
	if (key->type == OSSH_DSA) {
		buf_putstring(blobbuf, "ssh-dss", 7);
		retkey->type = DROPBEAR_SIGNKEY_DSS;
	} 
#endif
#if DROPBEAR_RSA
	if (key->type == OSSH_RSA) {
		buf_putstring(blobbuf, "ssh-rsa", 7);
		retkey->type = DROPBEAR_SIGNKEY_RSA;
	}
#endif

	for (i = 0; i < num_integers; i++) {
		ret = ber_read_id_len(p, key->keyblob+key->keyblob_len-p,
							  &id, &len, &flags);
		p += ret;
		if (ret < 0 || id != 2 || len < 0 ||
			key->keyblob+key->keyblob_len-p < len) {
			errmsg = "ASN.1 decoding failure";
			goto error;
		}

		if (i == 0) {
			/* First integer is a version indicator */
			int expected = -1;
			switch (key->type) {
				case OSSH_RSA:
				case OSSH_DSA:
					expected = 0;
					break;
				case OSSH_EC:
					expected = 1;
					break;
			}
			if (len != 1 || p[0] != expected) {
				errmsg = "Version number mismatch";
				goto error;
			}
		} else if (key->type == OSSH_RSA) {
			/*
			 * OpenSSH key order is n, e, d, p, q, dmp1, dmq1, iqmp
			 * but we want e, n, d, p, q
			 */
			if (i == 1) {
				/* Save the details for after we deal with number 2. */
				modptr = p;
				modlen = len;
			} else if (i >= 2 && i <= 5) {
				buf_putstring(blobbuf, (const char*)p, len);
				if (i == 2) {
					buf_putstring(blobbuf, (const char*)modptr, modlen);
				}
			}
		} else if (key->type == OSSH_DSA) {
			/*
			 * OpenSSH key order is p, q, g, y, x,
			 * we want the same.
			 */
			buf_putstring(blobbuf, (const char*)p, len);
		}

		/* Skip past the number. */
		p += len;
	}

#if DROPBEAR_ECDSA
	if (key->type == OSSH_EC) {
		unsigned char* private_key_bytes = NULL;
		int private_key_len = 0;
		unsigned char* public_key_bytes = NULL;
		int public_key_len = 0;
		ecc_key *ecc = NULL;
		const struct dropbear_ecc_curve *curve = NULL;

		/* See SEC1 v2, Appendix C.4 */
		/* OpenSSL (so OpenSSH) seems to include the optional parts. */

		/* privateKey OCTET STRING, */
		ret = ber_read_id_len(p, key->keyblob+key->keyblob_len-p,
							  &id, &len, &flags);
		p += ret;
		/* id==4 for octet string */
		if (ret < 0 || id != 4 || len < 0 ||
			key->keyblob+key->keyblob_len-p < len) {
			errmsg = "ASN.1 decoding failure";
			goto error;
		}
		private_key_bytes = p;
		private_key_len = len;
		p += len;

		/* parameters [0] ECDomainParameters {{ SECGCurveNames }} OPTIONAL, */
		ret = ber_read_id_len(p, key->keyblob+key->keyblob_len-p,
							  &id, &len, &flags);
		p += ret;
		/* id==0 */
		if (ret < 0 || id != 0 || len < 0) {
			errmsg = "ASN.1 decoding failure";
			goto error;
		}

		ret = ber_read_id_len(p, key->keyblob+key->keyblob_len-p,
							  &id, &len, &flags);
		p += ret;
		/* id==6 for object */
		if (ret < 0 || id != 6 || len < 0 ||
			key->keyblob+key->keyblob_len-p < len) {
			errmsg = "ASN.1 decoding failure";
			goto error;
		}

		if (0) {}
#if DROPBEAR_ECC_256
		else if (len == sizeof(OID_SEC256R1_BLOB) 
			&& memcmp(p, OID_SEC256R1_BLOB, len) == 0) {
			retkey->type = DROPBEAR_SIGNKEY_ECDSA_NISTP256;
			curve = &ecc_curve_nistp256;
		} 
#endif
#if DROPBEAR_ECC_384
		else if (len == sizeof(OID_SEC384R1_BLOB)
			&& memcmp(p, OID_SEC384R1_BLOB, len) == 0) {
			retkey->type = DROPBEAR_SIGNKEY_ECDSA_NISTP384;
			curve = &ecc_curve_nistp384;
		} 
#endif
#if DROPBEAR_ECC_521
		else if (len == sizeof(OID_SEC521R1_BLOB)
			&& memcmp(p, OID_SEC521R1_BLOB, len) == 0) {
			retkey->type = DROPBEAR_SIGNKEY_ECDSA_NISTP521;
			curve = &ecc_curve_nistp521;
		} 
#endif
		else {
			errmsg = "Unknown ECC key type";
			goto error;
		}
		p += len;

		/* publicKey [1] BIT STRING OPTIONAL */
		ret = ber_read_id_len(p, key->keyblob+key->keyblob_len-p,
							  &id, &len, &flags);
		p += ret;
		/* id==1 */
		if (ret < 0 || id != 1 || len < 0) {
			errmsg = "ASN.1 decoding failure";
			goto error;
		}

		ret = ber_read_id_len(p, key->keyblob+key->keyblob_len-p,
							  &id, &len, &flags);
		p += ret;
		/* id==3 for bit string */
		if (ret < 0 || id != 3 || len < 0 ||
			key->keyblob+key->keyblob_len-p < len) {
			errmsg = "ASN.1 decoding failure";
			goto error;
		}
		public_key_bytes = p+1;
		public_key_len = len-1;
		p += len;

		buf_putbytes(blobbuf, public_key_bytes, public_key_len);
		ecc = buf_get_ecc_raw_pubkey(blobbuf, curve);
		if (!ecc) {
			errmsg = "Error parsing ECC key";
			goto error;
		}
		m_mp_alloc_init_multi((mp_int**)&ecc->k, NULL);
		if (mp_from_ubin(ecc->k, private_key_bytes, private_key_len)
			!= MP_OKAY) {
			errmsg = "Error parsing ECC key";
			goto error;
		}

		*signkey_key_ptr(retkey, retkey->type) = ecc;
	}
#endif /* DROPBEAR_ECDSA */

	/*
	 * Now put together the actual key. Simplest way to do this is
	 * to assemble our own key blobs and feed them to the createkey
	 * functions; this is a bit faffy but it does mean we get all
	 * the sanity checks for free.
	 */
	if (key->type == OSSH_RSA || key->type == OSSH_DSA) {
		buf_setpos(blobbuf, 0);
		type = DROPBEAR_SIGNKEY_ANY;
		if (buf_get_priv_key(blobbuf, retkey, &type)
				!= DROPBEAR_SUCCESS) {
			errmsg = "unable to create key structure";
			sign_key_free(retkey);
			retkey = NULL;
			goto error;
		}
	}

	errmsg = NULL;					 /* no error */
	retval = retkey;

	error:
	if (blobbuf) {
		buf_burn_free(blobbuf);
	}
	m_burn(key->keyblob, key->keyblob_size);
	m_free(key->keyblob);
	m_burn(key, sizeof(*key));
	m_free(key);
	if (errmsg) {
		fprintf(stderr, "Error: %s\n", errmsg);
	}
	return retval;
}

static int openssh_write(const char *filename, sign_key *key,
				  const char *passphrase)
{
	buffer * keyblob = NULL;
	buffer * extrablob = NULL; /* used for calculated values to write */
	unsigned char *outblob = NULL;
	int outlen = -9999;
	int pos = 0, len = 0, i;
	char *header = NULL, *footer = NULL;
	int ret = 0;
	FILE *fp;

#if DROPBEAR_DSS
	if (key->type == DROPBEAR_SIGNKEY_DSS) {
		char zero[1];
		struct mpint_pos numbers[9];
		int nnumbers = -1, seqlen;
		/*
		 * Fetch the key blobs.
		 */
		keyblob = buf_new(3000);
		buf_put_priv_key(keyblob, key, key->type);

		buf_setpos(keyblob, 0);
		/* skip the "ssh-rsa" or "ssh-dss" header */
		buf_incrpos(keyblob, buf_getint(keyblob));

		/*
		 * Find the sequence of integers to be encoded into the OpenSSH
		 * key blob, and also decide on the header line.
		 */
		numbers[0].start = zero; numbers[0].bytes = 1; zero[0] = '\0';

		if (key->type == DROPBEAR_SIGNKEY_DSS) {

			/* p */
			numbers[1].bytes = buf_getint(keyblob);
			numbers[1].start = buf_getptr(keyblob, numbers[1].bytes);
			buf_incrpos(keyblob, numbers[1].bytes);

			/* q */
			numbers[2].bytes = buf_getint(keyblob);
			numbers[2].start = buf_getptr(keyblob, numbers[2].bytes);
			buf_incrpos(keyblob, numbers[2].bytes);

			/* g */
			numbers[3].bytes = buf_getint(keyblob);
			numbers[3].start = buf_getptr(keyblob, numbers[3].bytes);
			buf_incrpos(keyblob, numbers[3].bytes);

			/* y */
			numbers[4].bytes = buf_getint(keyblob);
			numbers[4].start = buf_getptr(keyblob, numbers[4].bytes);
			buf_incrpos(keyblob, numbers[4].bytes);

			/* x */
			numbers[5].bytes = buf_getint(keyblob);
			numbers[5].start = buf_getptr(keyblob, numbers[5].bytes);
			buf_incrpos(keyblob, numbers[5].bytes);

			nnumbers = 6;
			header = "-----BEGIN DSA PRIVATE KEY-----\n";
			footer = "-----END DSA PRIVATE KEY-----\n";
		}

		/*
		 * Now count up the total size of the ASN.1 encoded integers,
		 * so as to determine the length of the containing SEQUENCE.
		 */
		len = 0;
		for (i = 0; i < nnumbers; i++) {
			len += ber_write_id_len(NULL, 2, numbers[i].bytes, 0);
			len += numbers[i].bytes;
		}
		seqlen = len;
		/* Now add on the SEQUENCE header. */
		len += ber_write_id_len(NULL, 16, seqlen, ASN1_CONSTRUCTED);
		/* Round up to the cipher block size, ensuring we have at least one
		 * byte of padding (see below). */
		outlen = len;
		if (passphrase)
			outlen = (outlen+8) &~ 7;

		/*
		 * Now we know how big outblob needs to be. Allocate it.
		 */
		outblob = (unsigned char*)m_malloc(outlen);

		/*
		 * And write the data into it.
		 */
		pos = 0;
		pos += ber_write_id_len(outblob+pos, 16, seqlen, ASN1_CONSTRUCTED);
		for (i = 0; i < nnumbers; i++) {
			pos += ber_write_id_len(outblob+pos, 2, numbers[i].bytes, 0);
			memcpy(outblob+pos, numbers[i].start, numbers[i].bytes);
			pos += numbers[i].bytes;
		}
	} /* end DSS handling */
#endif /* DROPBEAR_DSS */

	if (0
#if DROPBEAR_RSA
		|| key->type == DROPBEAR_SIGNKEY_RSA
#endif
#if DROPBEAR_ED25519
		|| key->type == DROPBEAR_SIGNKEY_ED25519
#endif
#if DROPBEAR_ECDSA
		|| signkey_is_ecdsa(key->type)
#endif
		) {
		buffer *buf = buf_new(3200);
		keyblob = buf_new(3000);
		extrablob = buf_new(3100);

		/* private key blob w/o header */
#if DROPBEAR_RSA
		if (key->type == DROPBEAR_SIGNKEY_RSA) {
			buf_put_rsa_priv_ossh(keyblob, key);
		}
#endif
#if DROPBEAR_ED25519
		if (key->type == DROPBEAR_SIGNKEY_ED25519) {
			buf_put_ed25519_priv_ossh(keyblob, key);
		}
#endif
#if DROPBEAR_ECDSA
		if (signkey_is_ecdsa(key->type)) {
			buf_put_ecdsa_priv_ossh(keyblob, key);
		}
#endif

		/* header */
		buf_putbytes(buf, OSSH_PKEY_BLOB, OSSH_PKEY_BLOBLEN);

		/* public key */
		buf_put_pub_key(buf, key, key->type);

		/* private key */
		buf_putint(extrablob, 0); /* checkint 1 */
		buf_putint(extrablob, 0); /* checkint 2 */
		/* raw openssh private key */
		buf_putbytes(extrablob, keyblob->data, keyblob->len);
		/* comment */
		buf_putstring(extrablob, "", 0);
		/* padding to cipher block length */
		len = (extrablob->len+8) & ~7;
		for (i = 1; len - extrablob->len > 0; i++)
			buf_putbyte(extrablob, i);
		buf_setpos(extrablob, 0);
		buf_putbytes(extrablob, "\0\0\0\0\0\0\0\0", 8);
		buf_putbufstring(buf, extrablob);

		outlen = len = pos = buf->len;
		outblob = (unsigned char*)m_malloc(outlen);
		memcpy(outblob, buf->data, buf->len);

		buf_burn_free(buf);
		buf = NULL;

		header = "-----BEGIN OPENSSH PRIVATE KEY-----\n";
		footer = "-----END OPENSSH PRIVATE KEY-----\n";
	}

	/*
	 * Padding on OpenSSH keys is deterministic. The number of
	 * padding bytes is always more than zero, and always at most
	 * the cipher block length. The value of each padding byte is
	 * equal to the number of padding bytes. So a plaintext that's
	 * an exact multiple of the block size will be padded with 08
	 * 08 08 08 08 08 08 08 (assuming a 64-bit block cipher); a
	 * plaintext one byte less than a multiple of the block size
	 * will be padded with just 01.
	 * 
	 * This enables the OpenSSL key decryption function to strip
	 * off the padding algorithmically and return the unpadded
	 * plaintext to the next layer: it looks at the final byte, and
	 * then expects to find that many bytes at the end of the data
	 * with the same value. Those are all removed and the rest is
	 * returned.
	 */
	dropbear_assert(pos == len);
	while (pos < outlen) {
		outblob[pos++] = outlen - len;
	}

	/*
	 * Encrypt the key.
	 */
	if (passphrase) {
		fprintf(stderr, "Encrypted keys aren't supported currently\n");
		goto error;
	}

	/*
	 * And save it. We'll use Unix line endings just in case it's
	 * subsequently transferred in binary mode.
	 */
	if (strlen(filename) == 1 && filename[0] == '-') {
		fp = stdout;
	} else {
		fp = fopen(filename, "wb");	  /* ensure Unix line endings */
	}
	if (!fp) {
		fprintf(stderr, "Failed opening output file\n");
		goto error;
	}
	fputs(header, fp);
	base64_encode_fp(fp, outblob, outlen, 64);
	fputs(footer, fp);
	fclose(fp);
	ret = 1;

	error:
	if (outblob) {
		memset(outblob, 0, outlen);
		m_free(outblob);
	}
	if (keyblob) {
		buf_burn_free(keyblob);
	}
	if (extrablob) {
		buf_burn_free(extrablob);
	}
	return ret;
}

/* From PuTTY misc.c */
static int toint(unsigned u)
{
	/*
	 * Convert an unsigned to an int, without running into the
	 * undefined behaviour which happens by the strict C standard if
	 * the value overflows. You'd hope that sensible compilers would
	 * do the sensible thing in response to a cast, but actually I
	 * don't trust modern compilers not to do silly things like
	 * assuming that _obviously_ you wouldn't have caused an overflow
	 * and so they can elide an 'if (i < 0)' test immediately after
	 * the cast.
	 *
	 * Sensible compilers ought of course to optimise this entire
	 * function into 'just return the input value'!
	 */
	if (u <= (unsigned)INT_MAX)
		return (int)u;
	else if (u >= (unsigned)INT_MIN)   /* wrap in cast _to_ unsigned is OK */
		return INT_MIN + (int)(u - (unsigned)INT_MIN);
	else
		return INT_MIN; /* fallback; should never occur on binary machines */
}
