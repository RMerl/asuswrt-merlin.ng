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

#include <sys/stat.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#include "extern.h"
#include "rsa.h"

/*
 * Converts a BIGNUM to the form used in JWK.
 * This is essentially a base64-encoded big-endian binary string
 * representation of the number.
 */
static char *
bn2string(const BIGNUM *bn)
{
	int	 len;
	char	*buf, *bbuf;

	/* Extract big-endian representation of BIGNUM. */

	len = BN_num_bytes(bn);
	if (NULL == (buf = malloc(len))) {
		warn("malloc");
		return (NULL);
	} else if (len != BN_bn2bin(bn, (unsigned char *)buf)) {
		warnx("BN_bn2bin");
		free(buf);
		return (NULL);
	}

	/* Convert to base64url. */

	if (NULL == (bbuf = base64buf_url(buf, len))) {
		warnx("base64buf_url");
		free(buf);
		return (NULL);
	}

	free(buf);
	return (bbuf);
}

/*
 * Extract the relevant RSA components from the key and create the JSON
 * thumbprint from them.
 */
static char *
op_thumb_rsa(EVP_PKEY *pkey)
{
	char	*exp = NULL, *mod = NULL, *json = NULL;
	RSA	*r;

	if (NULL == (r = EVP_PKEY_get1_RSA(pkey)))
		warnx("EVP_PKEY_get1_RSA");
	else if (NULL == (mod = bn2string(r->n)))
		warnx("bn2string");
	else if (NULL == (exp = bn2string(r->e)))
		warnx("bn2string");
	else if (NULL == (json = json_fmt_thumb_rsa(exp, mod)))
		warnx("json_fmt_thumb_rsa");

	free(exp);
	free(mod);
	return (json);
}

/*
 * The thumbprint operation is used for the challenge sequence.
 */
static int
op_thumbprint(int fd, EVP_PKEY *pkey)
{
	char		*thumb = NULL, *dig64 = NULL;
	EVP_MD_CTX	*ctx = NULL;
	unsigned char	*dig = NULL;
	unsigned int	 digsz;
	int		 rc = 0;

	/* Construct the thumbprint input itself. */

	switch (EVP_PKEY_type(pkey->type)) {
	case EVP_PKEY_RSA:
		if (NULL != (thumb = op_thumb_rsa(pkey)))
			break;
		goto out;
	default:
		warnx("EVP_PKEY_type: unknown key type");
		goto out;
	}

	/*
	 * Compute the SHA256 digest of the thumbprint then
	 * base64-encode the digest itself.
	 * If the reader is closed when we write, ignore it (we'll pick
	 * it up in the read loop).
	 */

	if (NULL == (dig = malloc(EVP_MAX_MD_SIZE))) {
		warn("malloc");
		goto out;
	} else if (NULL == (ctx = EVP_MD_CTX_create())) {
		warnx("EVP_MD_CTX_create");
		goto out;
	} else if ( ! EVP_DigestInit_ex(ctx, EVP_sha256(), NULL)) {
		warnx("EVP_SignInit_ex");
		goto out;
	} else if ( ! EVP_DigestUpdate(ctx, thumb, strlen(thumb))) {
		warnx("EVP_SignUpdate");
		goto out;
	} else if ( ! EVP_DigestFinal_ex(ctx, dig, &digsz)) {
		warnx("EVP_SignFinal");
		goto out;
	} else if (NULL == (dig64 = base64buf_url((char *)dig, digsz))) {
		warnx("base64buf_url");
		goto out;
	} else if (writestr(fd, COMM_THUMB, dig64) < 0)
		goto out;

	rc = 1;
out:
	if (NULL != ctx)
		EVP_MD_CTX_destroy(ctx);

	free(thumb);
	free(dig);
	free(dig64);
	return (rc);
}

static int
op_sign_rsa(char **head, char **prot, EVP_PKEY *pkey, const char *nonce)
{
	char	*exp = NULL, *mod = NULL;
	int	rc = 0;
	RSA	*r;

	*head = NULL;
	*prot = NULL;

	/*
	 * First, extract relevant portions of our private key.
	 * Then construct the public header.
	 * Finally, format the header combined with the nonce.
	 */

	if (NULL == (r = EVP_PKEY_get1_RSA(pkey)))
		warnx("EVP_PKEY_get1_RSA");
	else if (NULL == (mod = bn2string(r->n)))
		warnx("bn2string");
	else if (NULL == (exp = bn2string(r->e)))
		warnx("bn2string");
	else if (NULL == (*head = json_fmt_header_rsa(exp, mod)))
		warnx("json_fmt_header_rsa");
	else if (NULL == (*prot = json_fmt_protected_rsa(exp, mod, nonce)))
		warnx("json_fmt_protected_rsa");
	else
		rc = 1;

	free(exp);
	free(mod);
	return (rc);
}

/*
 * Operation to sign a message with the account key.
 * This requires the sender ("fd") to provide the payload and a nonce.
 */
static int
op_sign(int fd, EVP_PKEY *pkey)
{
	char		*nonce = NULL, *pay = NULL, *pay64 = NULL;
	char		*prot = NULL, *prot64 = NULL, *head = NULL;
	char		*sign = NULL, *dig64 = NULL, *fin = NULL;
	unsigned char	*dig = NULL;
	EVP_MD_CTX	*ctx = NULL;
	int		 rc = 0;
	unsigned int	 digsz;

	/* Read our payload and nonce from the requestor. */

	if (NULL == (pay = readstr(fd, COMM_PAY)))
		goto out;
	else if (NULL == (nonce = readstr(fd, COMM_NONCE)))
		goto out;

	/* Base64-encode the payload. */

	if (NULL == (pay64 = base64buf_url(pay, strlen(pay)))) {
		warnx("base64buf_url");
		goto out;
	}

	switch (EVP_PKEY_type(pkey->type)) {
	case EVP_PKEY_RSA:
		if ( ! op_sign_rsa(&head, &prot, pkey, nonce))
			goto out;
		break;
	default:
		warnx("EVP_PKEY_type");
		goto out;
	}

	/* The header combined with the nonce, base64. */

	if (NULL == (prot64 = base64buf_url(prot, strlen(prot)))) {
		warnx("base64buf_url");
		goto out;
	}

	/* Now the signature material. */

	sign = doasprintf("%s.%s", prot64, pay64);
	if (NULL == sign) {
		warn("asprintf");
		goto out;
	}

	if (NULL == (dig = malloc(EVP_PKEY_size(pkey)))) {
		warn("malloc");
		goto out;
	}

	/*
	 * Here we go: using our RSA key as merged into the envelope,
	 * sign a SHA256 digest of our message.
	 */

	if (NULL == (ctx = EVP_MD_CTX_create())) {
		warnx("EVP_MD_CTX_create");
		goto out;
	} else if ( ! EVP_SignInit_ex(ctx, EVP_sha256(), NULL)) {
		warnx("EVP_SignInit_ex");
		goto out;
	} else if ( ! EVP_SignUpdate(ctx, sign, strlen(sign))) {
		warnx("EVP_SignUpdate");
		goto out;
	} else if ( ! EVP_SignFinal(ctx, dig, &digsz, pkey)) {
		warnx("EVP_SignFinal");
		goto out;
	} else if (NULL == (dig64 = base64buf_url((char *)dig, digsz))) {
		warnx("base64buf_url");
		goto out;
	}

	/*
	 * Write back in the correct JSON format.
	 * If the reader is closed, just ignore it (we'll pick it up
	 * when we next enter the read loop).
	 */

	if (NULL == (fin = json_fmt_signed(head, prot64, pay64, dig64))) {
		warnx("json_fmt_signed");
		goto out;
	} else if (writestr(fd, COMM_REQ, fin) < 0)
		goto out;

	rc = 1;
out:
	if (NULL != ctx)
		EVP_MD_CTX_destroy(ctx);

	free(pay);
	free(sign);
	free(pay64);
	free(nonce);
	free(head);
	free(prot);
	free(prot64);
	free(dig);
	free(dig64);
	free(fin);
	return (rc);
}

int
acctproc(int netsock, const char *acctkey, const struct config *cfg)
{
	FILE		*f = NULL;
	EVP_PKEY	*pkey = NULL;
	long		 lval;
	enum acctop	 op;
	unsigned char	 rbuf[64];
	int		 rc = 0, cc;
	mode_t		 prev;

	/*
	 * First, open our private key file read-only or write-only if
	 * we're creating from scratch.
	 * Set our umask to be maximally restrictive.
	 */

	prev = umask((S_IWUSR | S_IXUSR) | S_IRWXG | S_IRWXO);
	f = fopen(acctkey, cfg->newacct ? "wx" : "r");
	umask(prev);

	if (NULL == f) {
		warn("%s", acctkey);
		goto out;
	}

	/* File-system, user, and sandbox jailing. */

	if ( ! sandbox_before())
		goto out;

	ERR_load_crypto_strings();

	if ( ! dropfs(PATH_VAR_EMPTY))
		goto out;
	else if ( ! dropprivs())
		goto out;
	else if ( ! sandbox_after(0))
		goto out;

	/*
	 * Seed our PRNG with data from arc4random().
	 * Do this until we're told it's ok and use increments of 64
	 * bytes (arbitrarily).
	 */

	while (0 == RAND_status()) {
#ifdef __linux__
		FILE *fp;
		if( NULL != (fp = fopen("/dev/urandom", "r")) )
		{
			fread(rbuf, 1, sizeof(rbuf), fp);
			fclose(fp);
		}
#else
		arc4random_buf(rbuf, sizeof(rbuf));
#endif
		RAND_seed(rbuf, sizeof(rbuf));
	}

	if (cfg->newacct) {
		dodbg("%s: generating RSA account key", acctkey);
		if (NULL == (pkey = rsa_key_create(f, acctkey)))
			goto out;
	} else {
		doddbg("%s: loading RSA account key", acctkey);
		if (NULL == (pkey = rsa_key_load(f, acctkey)))
			goto out;
	}

	fclose(f);
	f = NULL;

	/* Notify the netproc that we've started up. */

	if (0 == (cc = writeop(netsock, COMM_ACCT_STAT, ACCT_READY)))
		rc = 1;
	if (cc <= 0)
		goto out;

	/*
	 * Now we wait for requests from the network-facing process.
	 * It might ask us for our thumbprint, for example, or for us to
	 * sign a message.
	 */

	for (;;) {
		op = ACCT__MAX;
		if (0 == (lval = readop(netsock, COMM_ACCT)))
			op = ACCT_STOP;
		else if (ACCT_SIGN == lval || ACCT_THUMBPRINT == lval)
			op = lval;

		if (ACCT__MAX == op) {
			warnx("unknown operation from netproc");
			goto out;
		} else if (ACCT_STOP == op)
			break;

		switch (op) {
		case (ACCT_SIGN):
			if (op_sign(netsock, pkey))
				break;
			warnx("op_sign");
			goto out;
		case (ACCT_THUMBPRINT):
			if (op_thumbprint(netsock, pkey))
				break;
			warnx("op_thumbprint");
			goto out;
		default:
			abort();
		}
	}

	rc = 1;
out:
	close(netsock);
	if (NULL != f)
		fclose(f);
	if (NULL != pkey)
		EVP_PKEY_free(pkey);
	ERR_print_errors_fp(stderr);
	ERR_free_strings();
	return (rc);
}

