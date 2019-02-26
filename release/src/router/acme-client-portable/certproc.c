/*	$Id$ */
/*
 * Copyright (c) 2016 Kristaps Dzonsons <kristaps@bsd.lv>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHORS DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>

#include "compat-openssl.h"

#include "extern.h"

#define MARKER "-----BEGIN CERTIFICATE-----"

/*
 * Convert an X509 certificate to a buffer of "sz".
 * We don't guarantee that it's nil-terminated.
 * Returns NULL on failure.
 */
static char *
x509buf(X509 *x, size_t *sz)
{
	BIO	*bio;
	char	*p;
	int	 ssz;

	/* Convert X509 to PEM in BIO. */

	if (NULL == (bio = BIO_new(BIO_s_mem()))) {
		warnx("BIO_new");
		return (NULL);
	} else if ( ! PEM_write_bio_X509(bio, x)) {
		warnx("PEM_write_bio_X509");
		BIO_free(bio);
		return (NULL);
	}

	/*
	 * Now convert bio to string.
	 * Make into nil-terminated, just in case.
	 */

	if (NULL == (p = calloc(1, BIO_number_written(bio) + 1))) {
		warn("calloc");
		BIO_free(bio);
		return (NULL);
	}

	ssz = BIO_read(bio, p, BIO_number_written(bio));
	if (ssz < 0 || (unsigned)ssz != BIO_number_written(bio)) {
		warnx("BIO_read");
		BIO_free(bio);
		return (NULL);
	}

	*sz = ssz;
	BIO_free(bio);
	return (p);
}

int
certproc(int netsock, int filesock)
{
	char		*csr = NULL, *chain = NULL, *url = NULL;
	unsigned char	*csrcp, *chaincp;
	size_t		 csrsz, chainsz;
	int		 i, rc = 0, idx = -1, cc;
	enum certop	 op;
	long		 lval;
	X509		*x = NULL, *chainx = NULL;
	X509_EXTENSION	*ext = NULL;
	X509V3_EXT_METHOD *method = NULL;
	void		*entries;
	STACK_OF(CONF_VALUE) *val;
	CONF_VALUE	*nval;

	/* File-system and sandbox jailing. */

	if ( ! sandbox_before())
		goto out;

	ERR_load_crypto_strings();

	if ( ! dropfs(PATH_VAR_EMPTY))
		goto out;
	else if ( ! dropprivs())
		goto out;
	else if ( ! sandbox_after(0))
		goto out;

	/* Read what the netproc wants us to do. */

	op = CERT__MAX;
	if (0 == (lval = readop(netsock, COMM_CSR_OP)))
		op = CERT_STOP;
	else if (CERT_REVOKE == lval || CERT_UPDATE == lval)
		op = lval;

	if (CERT_STOP == op) {
		rc = 1;
		goto out;
	} else if (CERT__MAX == op) {
		warnx("unknown operation from netproc");
		goto out;
	}

	/*
	 * Pass revocation right through to fileproc.
	 * If the reader is terminated, ignore it.
	 */

	if (CERT_REVOKE == op) {
		if (writeop(filesock, COMM_CHAIN_OP, FILE_REMOVE) >= 0)
			rc = 1;
		goto out;
	}

	/*
	 * Wait until we receive the DER encoded (signed) certificate
	 * from the network process.
	 * Then convert the DER encoding into an X509 certificate.
	 */

	if (NULL == (csr = readbuf(netsock, COMM_CSR, &csrsz)))
		goto out;

	csrcp = (u_char *)csr;
	x = d2i_X509(NULL, (const u_char **)&csrcp, csrsz);
	if (NULL == x) {
		warnx("d2i_X509");
		goto out;
	}

	/*
	 * Extract the CA Issuers from its NID.
	 * TODO: I have no idea what I'm doing.
	 */

	idx = X509_get_ext_by_NID(x, NID_info_access, idx);
	if (idx >= 0 && NULL != (ext = X509_get_ext(x, idx)))
		method = (X509V3_EXT_METHOD *)X509V3_EXT_get(ext);

	entries = X509_get_ext_d2i(x, NID_info_access, 0, 0);
	if (NULL != method && NULL != entries) {
		val = method->i2v(method, entries, 0);
		for (i = 0; i < sk_CONF_VALUE_num(val); i++) {
			nval = sk_CONF_VALUE_value(val, i);
			if (strcmp(nval->name, "CA Issuers - URI"))
				continue;
			url = strdup(nval->value);
			if (NULL == url) {
				warn("strdup");
				goto out;
			}
			break;
		}
	}

	if (NULL == url) {
		warnx("no CA issuer registered with certificate");
		goto out;
	}

	/* Write the CA issuer to the netsock. */

	if (writestr(netsock, COMM_ISSUER, url) <= 0)
		goto out;

	/* Read the full-chain back from the netsock. */

	if (NULL == (chain = readbuf(netsock, COMM_CHAIN, &chainsz)))
		goto out;

	/*
	 * Then check if the chain is PEM-encoded by looking to see if
	 * it begins with the PEM marker.
	 * If so, ship it as-is; otherwise, convert to a PEM encoded
	 * buffer and ship that.
	 * FIXME: if PEM, re-parse it.
	 */

	if (chainsz <= strlen(MARKER) ||
	    strncmp(chain, MARKER, strlen(MARKER))) {
		chaincp = (u_char *)chain;
		chainx = d2i_X509(NULL,
			(const u_char **)&chaincp, chainsz);
		if (NULL == chainx) {
			warnx("d2i_X509");
			goto out;
		}
		free(chain);
		if (NULL == (chain = x509buf(chainx, &chainsz)))
			goto out;
	}

	/* Allow reader termination to just push us out. */

	if (0 == (cc = writeop(filesock, COMM_CHAIN_OP, FILE_CREATE)))
		rc = 1;
	if (cc <= 0)
		goto out;
	if (0 == (cc = writebuf(filesock, COMM_CHAIN, chain, chainsz)))
		rc = 1;
	if (cc <= 0)
		goto out;

	/*
	 * Next, convert the X509 to a buffer and send that.
	 * Reader failure doesn't change anything.
	 */

	free(chain);
	if (NULL == (chain = x509buf(x, &chainsz)))
		goto out;
	if (writebuf(filesock, COMM_CSR, chain, chainsz) < 0)
		goto out;

	rc = 1;
out:
	close(netsock);
	close(filesock);
	if (NULL != x)
		X509_free(x);
	if (NULL != chainx)
		X509_free(chainx);
	free(csr);
	free(url);
	free(chain);
	ERR_print_errors_fp(stderr);
	ERR_free_strings();
	return (rc);
}
