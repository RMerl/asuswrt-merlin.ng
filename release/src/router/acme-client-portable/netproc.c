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

#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "http.h"
#include "extern.h"

#define URL_REAL_CA "https://acme-v01.api.letsencrypt.org/directory"
#define URL_STAGE_CA "https://acme-staging.api.letsencrypt.org/directory"

#define	RETRY_DELAY 5
#define RETRY_MAX 10

/*
 * Buffer used when collecting the results of a CURL transfer.
 */
struct	buf {
	char	*buf;	/* binary buffer */
	size_t	 sz;	/* length of buffer */
};

/*
 * Used for CURL communications.
 */
struct	conn {
	const char	*na;	/* nonce authority */
	int		 fd;	/* acctproc handle */
	int		 dfd;	/* dnsproc handle */
	struct buf	 buf;	/* transfer buffer */
	struct httpcfg	*cfg;	/* http configuration */
};

/*
 * If something goes wrong (or we're tracing output), we dump the
 * current transfer's data as a debug message.
 * Make sure that print all non-printable characters as question marks
 * so that we don't spam the console.
 * Also, consolidate white-space.
 * This of course will ruin string literals, but the intent here is just
 * to show the message, not to replicate it.
 */
static void
buf_dump(const struct buf *buf)
{
	size_t	 i;
	int	 j;
	char	*nbuf;

	if (0 == buf->sz)
		return;
	if (NULL == (nbuf = malloc(buf->sz)))
		err(EXIT_FAILURE, "malloc");

	for (j = 0, i = 0; i < buf->sz; i++)
		if (isspace((int)buf->buf[i])) {
			nbuf[j++] = ' ';
			while (isspace((int)buf->buf[i]))
				i++;
			i--;
		} else
			nbuf[j++] = isprint((int)buf->buf[i]) ?
				buf->buf[i] : '?';
	warnx("transfer buffer: [%.*s] (%zu bytes)", j, nbuf, buf->sz);
	free(nbuf);
}

/*
 * Extract the domain and port from a URL.
 * The url must be formatted as schema://address[/stuff].
 * This returns NULL on failure.
 */
static char *
url2host(const char *host, short *port, char **path)
{
	char	*url, *ep;

	/* We only understand HTTP and HTTPS. */

	if (0 == strncmp(host, "https://", 8)) {
		*port = 443;
		if (NULL == (url = strdup(host + 8))) {
			warn("strdup");
			return (NULL);
		}
	} else if (0 == strncmp(host, "http://", 7)) {
		*port = 80;
		if (NULL == (url = strdup(host + 7))) {
			warn("strdup");
			return (NULL);
		}
	} else {
		warnx("%s: unknown schema", host);
		return (NULL);
	}

	/* Terminate path part. */

	if (NULL != (ep = strchr(url, '/'))) {
		*path = strdup(ep);
		*ep = '\0';
	} else
		*path = strdup("");

	if (NULL == *path) {
		warn("strdup");
		free(url);
		return (NULL);
	}

	return (url);
}

/*
 * Contact dnsproc and resolve a host.
 * Place the answers in "v" and return the number of answers, which can
 * be at most MAX_SERVERS_DNS.
 * Return <0 on failure.
 */
static ssize_t
urlresolve(int fd, const char *host, struct source *v)
{
	char		*addr;
	size_t		 i, sz;
	long		 lval;

	if (writeop(fd, COMM_DNS, DNS_LOOKUP) <= 0)
		return (-1);
	else if (writestr(fd, COMM_DNSQ, host) <= 0)
		return (-1);
	else if ((lval = readop(fd, COMM_DNSLEN)) < 0)
		return (-1);

	sz = lval;
	assert(sz <= MAX_SERVERS_DNS);

	for (i = 0; i < sz; i++) {
		memset(&v[i], 0, sizeof(struct source));
		if ((lval = readop(fd, COMM_DNSF)) < 0)
			goto err;
		else if (4 != lval && 6 != lval)
			goto err;
		else if (NULL == (addr = readstr(fd, COMM_DNSA)))
			goto err;
		v[i].family = lval;
		v[i].ip = addr;
	}

	return (sz);
err:
	for (i = 0; i < sz; i++)
		free(v[i].ip);
	return (-1);
}

/*
 * Send a "regular" HTTP GET message to "addr" and stuff the response
 * into the connection buffer.
 * Return the HTTP error code or <0 on failure.
 */
static long
nreq(struct conn *c, const char *addr)
{
	struct httpget	*g;
	struct source	 src[MAX_SERVERS_DNS];
	char		*host, *path;
	short		 port;
	size_t		 srcsz;
	ssize_t		 ssz;
	long		 code;

	if (NULL == (host = url2host(addr, &port, &path)))
		return (-1);

	if ((ssz = urlresolve(c->dfd, host, src)) < 0) {
		free(host);
		free(path);
		return (-1);
	}
	srcsz = ssz;

	g = http_get(c->cfg, src, srcsz, host, port, path, NULL, 0);
	free(host);
	free(path);
	if (NULL == g)
		return (-1);

	code = g->code;

	/* Copy the body part into our buffer. */

	free(c->buf.buf);
	c->buf.sz = g->bodypartsz;
	c->buf.buf = malloc(c->buf.sz);
	memcpy(c->buf.buf, g->bodypart, c->buf.sz);
	http_get_free(g);
	if (NULL == c->buf.buf) {
		warn("malloc");
		return (-1);
	}
	return (code);
}

/*
 * Create and send a signed communication to the ACME server.
 * Stuff the response into the communication buffer.
 * Return <0 on failure on the HTTP error code otherwise.
 */
static long
sreq(struct conn *c, const char *addr, const char *req)
{
	struct httpget	*g;
	struct source	 src[MAX_SERVERS_DNS];
	char		*host, *path, *nonce, *reqsn;
	short		 port;
	struct httphead	*h;
	ssize_t		 ssz;
	long		 code;

	if (NULL == (host = url2host(c->na, &port, &path)))
		return (-1);

	if ((ssz = urlresolve(c->dfd, host, src)) < 0) {
		free(host);
		free(path);
		return (-1);
	}

	g = http_get(c->cfg, src, (size_t)ssz, 
		host, port, path, NULL, 0);
	free(host);
	free(path);
	if (NULL == g)
		return (-1);

	h = http_head_get("Replay-Nonce", g->head, g->headsz);
	if (NULL == h) {
		warnx("%s: no replay nonce", c->na);
		http_get_free(g);
		return (-1);
	} else if (NULL == (nonce = strdup(h->val))) {
		warn("strdup");
		http_get_free(g);
		return (-1);
	}
	http_get_free(g);

	/*
	 * Send the nonce and request payload to the acctproc.
	 * This will create the proper JSON object we need.
	 */

	if (writeop(c->fd, COMM_ACCT, ACCT_SIGN) <= 0) {
		free(nonce);
		return (-1);
	} else if (writestr(c->fd, COMM_PAY, req) <= 0) {
		free(nonce);
		return (-1);
	} else if (writestr(c->fd, COMM_NONCE, nonce) <= 0) {
		free(nonce);
		return (-1);
	}
	free(nonce);

	/* Now read back the signed payload. */

	if (NULL == (reqsn = readstr(c->fd, COMM_REQ)))
		return (-1);

	/* Now send the signed payload to the CA. */

	if (NULL == (host = url2host(addr, &port, &path))) {
		free(reqsn);
		return (-1);
	} else if ((ssz = urlresolve(c->dfd, host, src)) < 0) {
		free(host);
		free(path);
		free(reqsn);
		return (-1);
	}

	g = http_get(c->cfg, src, (size_t)ssz, host,
		port, path, reqsn, strlen(reqsn));

	free(host);
	free(path);
	free(reqsn);
	if (NULL == g)
		return (-1);

	/* Stuff response into parse buffer. */

	code = g->code;

	free(c->buf.buf);
	c->buf.sz = g->bodypartsz;
	c->buf.buf = malloc(c->buf.sz);
	memcpy(c->buf.buf, g->bodypart, c->buf.sz);
	http_get_free(g);
	if (NULL == c->buf.buf) {
		warn("malloc");
		return (-1);
	}
	return (code);
}

/*
 * Send to the CA that we want to authorise a new account.
 * This only happens once for a new account key.
 * Returns non-zero on success.
 */
static int
donewreg(struct conn *c, const char *agreement,
	const struct capaths *p)
{
	int		 rc = 0;
	char		*req;
	long		 lc;
	char	agreement_buf[128];
	int		new_agreement = 0;
	char	*cp = NULL;

	dodbg("%s: new-reg", p->newreg);
	strlcpy(agreement_buf, agreement, sizeof(agreement_buf));
reg:
	if (NULL == (req = json_fmt_newreg(agreement_buf)))
		warnx("json_fmt_newreg");
	else if ((lc = sreq(c, p->newreg, req)) < 0)
		warnx("%s: bad comm", p->newreg);
	else if (200 != lc && 201 != lc && 409 != lc)	//409, Registration key is already in use
	{
		warnx("%s: bad HTTP: %ld", p->newreg, lc);
		if(lc == 400 && (cp = strstr(c->buf.buf, "current agreement URL")) && new_agreement == 0)
		{
			//parse new agreement url
			cp = strchr(cp, '[');
			strlcpy(agreement_buf, cp+1, sizeof(agreement_buf));
			cp = strchr(agreement_buf, ']');
			*cp = '\0';
			new_agreement = 1;
			warnx("try new agreement: %s", agreement_buf);
			goto reg;
		}
	}
	else if (NULL == c->buf.buf || 0 == c->buf.sz)
		warnx("%s: empty response", p->newreg);
	else
		rc = 1;

	if (0 == rc || verbose > 1)
		buf_dump(&c->buf);
	free(req);
	return (rc);
}

/*
 * Request a challenge for the given domain name.
 * This must happen for each name "alt".
 * On non-zero exit, fills in "chng" with the challenge.
 */
static int
dochngreq(struct conn *c, const char *alt, struct chng *chng, 
	const struct capaths *p, const char *challenge)
{
	int		 rc = 0;
	char		*req;
	long		 lc;
	struct jsmnn	*j = NULL;

	dodbg("%s: req-auth: %s", p->newauthz, alt);

	if (NULL == (req = json_fmt_newauthz(alt)))
		warnx("json_fmt_newauthz");
	else if ((lc = sreq(c, p->newauthz, req)) < 0)
		warnx("%s: bad comm", p->newauthz);
	else if (200 != lc && 201 != lc)
		warnx("%s: bad HTTP: %ld", p->newauthz, lc);
	else if (NULL == (j = json_parse(c->buf.buf, c->buf.sz)))
		warnx("%s: bad JSON object", p->newauthz);
	else if ( ! json_parse_challenge(j, chng, challenge)) 
		warnx("%s: bad challenge", p->newauthz);
	else
		rc = 1;

	if (0 == rc || verbose > 1)
		buf_dump(&c->buf);
	json_free(j);
	free(req);
	return (rc);
}

/*
 * Note to the CA that a challenge response is in place.
 */
static int
dochngresp(struct conn *c, const struct chng *chng, const char *th)
{
	int	 rc = 0;
	long	 lc;
	char	*req;

	dodbg("%s: challenge", chng->uri);

	if (NULL == (req = json_fmt_challenge(chng->token, th)))
		warnx("json_fmt_challenge");
	else if ((lc = sreq(c, chng->uri, req)) < 0)
		warnx("%s: bad comm", chng->uri);
	else if (200 != lc && 201 != lc && 202 != lc)
		warnx("%s: bad HTTP: %ld", chng->uri, lc);
	else
		rc = 1;

	if (0 == rc || verbose > 1)
		buf_dump(&c->buf);
	free(req);
	return (rc);
}

/*
 * Check with the CA whether a challenge has been processed.
 * Note: we'll only do this a limited number of times, and pause for a
 * time between checks, but this happens in the caller.
 */
static int
dochngcheck(struct conn *c, struct chng *chng)
{
	int		 cc;
	long		 lc;
	struct jsmnn	*j;

	dodbg("%s: status", chng->uri);

	if ((lc = nreq(c, chng->uri)) < 0) {
		warnx("%s: bad comm", chng->uri);
		return (0);
	} else if (200 != lc && 201 != lc && 202 != lc) {
		warnx("%s: bad HTTP: %ld", chng->uri, lc);
		buf_dump(&c->buf);
		return (0);
	} else if (NULL == (j = json_parse(c->buf.buf, c->buf.sz))) {
		warnx("%s: bad JSON object", chng->uri);
		buf_dump(&c->buf);
		return (0);
	} else if (-1 == (cc = json_parse_response(j))) {
		warnx("%s: bad response", chng->uri);
		buf_dump(&c->buf);
		json_free(j);
		return (0);
	} else if (cc > 0)
		chng->status = 1;

	json_free(j);
	return (1);
}

static int
dorevoke(struct conn *c, const char *addr, const char *cert)
{
	char		*req;
	int		 rc = 0;
	long		 lc = 0;

	dodbg("%s: revocation", addr);

	if (NULL == (req = json_fmt_revokecert(cert)))
		warnx("json_fmt_revokecert");
	else if ((lc = sreq(c, addr, req)) < 0)
		warnx("%s: bad comm", addr);
	else if (200 != lc && 201 != lc && 409 != lc)
		warnx("%s: bad HTTP: %ld", addr, lc);
	else
		rc = 1;

	if (409 == lc)
		warnx("%s: already revoked", addr);

	if (0 == rc || verbose > 1)
		buf_dump(&c->buf);
	free(req);
	return (rc);
}

/*
 * Submit our certificate to the CA.
 * This, upon success, will return the signed CA.
 */
static int
docert(struct conn *c, const char *addr, const char *cert)
{
	char	*req;
	int	 rc = 0;
	long	 lc;

	dodbg("%s: certificate", addr);

	if (NULL == (req = json_fmt_newcert(cert)))
		warnx("json_fmt_newcert");
	else if ((lc = sreq(c, addr, req)) < 0)
		warnx("%s: bad comm", addr);
	else if (200 != lc && 201 != lc)
		warnx("%s: bad HTTP: %ld", addr, lc);
	else if (0 == c->buf.sz || NULL == c->buf.buf)
		warnx("%s: empty response", addr);
	else
		rc = 1;

	if (0 == rc || verbose > 1)
		buf_dump(&c->buf);
	free(req);
	return (rc);
}

/*
 * Look up directories from the certificate authority.
 */
static int
dodirs(struct conn *c, const char *addr, struct capaths *paths)
{
	struct jsmnn	*j = NULL;
	long		 lc;
	int		 rc = 0;

	dodbg("%s: directories", addr);

	if ((lc = nreq(c, addr)) < 0)
		warnx("%s: bad comm", addr);
	else if (200 != lc && 201 != lc)
		warnx("%s: bad HTTP: %ld", addr, lc);
	else if (NULL == (j = json_parse(c->buf.buf, c->buf.sz)))
		warnx("json_parse");
	else if ( ! json_parse_capaths(j, paths))
		warnx("%s: bad CA paths", addr);
	else
		rc = 1;

	if (0 == rc || verbose > 1)
		buf_dump(&c->buf);
	json_free(j);
	return (rc);
}

/*
 * Request the full chain certificate.
 */
static int
dofullchain(struct conn *c, const char *addr)
{
	int	 rc = 0;
	long	 lc;

	dodbg("%s: full chain", addr);

	if ((lc = nreq(c, addr)) < 0)
		warnx("%s: bad comm", addr);
	else if (200 != lc && 201 != lc)
		warnx("%s: bad HTTP: %ld", addr, lc);
	else
		rc = 1;

	if (0 == rc || verbose > 1)
		buf_dump(&c->buf);
	return (rc);
}

/*
 * Here we communicate with the ACME server.
 * For this, we'll need the certificate we want to upload and our
 * account key information.
 */
int
netproc(int kfd, int afd, int Cfd, int cfd, int dfd, int rfd,
	int newacct, int revocate, int staging, 
	const char *const *alts, size_t altsz, const char *agreement,
	const char *challenge)
{
	int		 rc = 0;
	size_t		 i;
	char		*cert = NULL, *thumb = NULL, *url = NULL;
	struct conn	 c;
	struct capaths	 paths;
	struct chng	*chngs = NULL;
	long		 lval;

	memset(&paths, 0, sizeof(struct capaths));
	memset(&c, 0, sizeof(struct conn));

	/*
	 * FIXME: we do this before pledge()ing because our pledge (and
	 * chroot) will prevent reading the default certificate.
	 * However, we should do a pledge before it with "rpath", as in
	 * downstream OpenBSD, but the current framework is too rigid
	 * for doing so.
	 * Need to rethink.
	 */

	c.cfg = http_init();

	/* File-system, user, and sandbox jail. */

	if ( ! sandbox_before()) 
		goto out;
	else if ( ! dropfs(PATH_VAR_EMPTY))
		goto out;
	else if ( ! dropprivs())
		goto out;
	else if ( ! sandbox_after(0))
		goto out;

	/*
	 * Wait until the acctproc, keyproc, and revokeproc have started
	 * up and are ready to serve us data.
	 * There's no point in running if these don't work.
	 * Then check whether revokeproc indicates that the certificate
	 * on file (if any) can be updated.
	 */

	if (0 == (lval = readop(afd, COMM_ACCT_STAT))) {
		rc = 1;
		goto out;
	} else if (ACCT_READY != lval) {
		warnx("unknown operation from acctproc");
		goto out;
	}

	if (0 == (lval = readop(kfd, COMM_KEY_STAT))) {
		rc = 1;
		goto out;
	} else if (KEY_READY != lval) {
		warnx("unknown operation from keyproc");
		goto out;
	}

	if (0 == (lval = readop(rfd, COMM_REVOKE_RESP))) {
		rc = 1;
		goto out;
	} else if (REVOKE_EXP != lval && REVOKE_OK != lval) {
		warnx("unknown operation from revokeproc");
		goto out;
	}

	/* If our certificate is up-to-date, return now. */

	if (REVOKE_OK == lval) {
		rc = 1;
		goto out;
	}

	/* Allocate main state. */

	chngs = calloc(altsz, sizeof(struct chng));
	if (NULL == chngs) {
		warn("calloc");
		goto out;
	}

	c.dfd = dfd;
	c.fd = afd;
	c.na = staging ? URL_STAGE_CA : URL_REAL_CA;

	if (NULL == c.cfg)
		goto out;

	/*
	 * Look up the domain of the ACME server.
	 * We'll use this ourselves instead of having libcurl do the DNS
	 * resolution itself.
	 */
	if ( ! dodirs(&c, c.na, &paths))
		goto out;

	/*
	 * If we're meant to revoke, then wait for revokeproc to send us
	 * the certificate (if it's found at all).
	 * Following that, submit the request to the CA then notify the
	 * certproc, which will in turn notify the fileproc.
	 */

	if (revocate) {
		if (NULL == (cert = readstr(rfd, COMM_CSR)))
			goto out;
		if ( ! dorevoke(&c, paths.revokecert, cert))
			goto out;
		else if (writeop(cfd, COMM_CSR_OP, CERT_REVOKE) > 0)
			rc = 1;
		goto out;
	}

	/* If new, register with the CA server. */
	// do reg, if alreay registered, continue to do authorise

	if ( ! donewreg(&c, agreement, &paths) && newacct)
		goto out;

	/* Pre-authorise all domains with CA server. */

	for (i = 0; i < altsz; i++)
		if ( ! dochngreq(&c, alts[i], 
		    &chngs[i], &paths, challenge))
			goto out;

	/*
	 * We now have our challenges.
	 * We need to ask the acctproc for the thumbprint.
	 * We'll combine this to the challenge to create our response,
	 * which will be orchestrated by the chngproc.
	 */

	if (writeop(afd, COMM_ACCT, ACCT_THUMBPRINT) <= 0)
		goto out;
	else if (NULL == (thumb = readstr(afd, COMM_THUMB)))
		goto out;

	/* We'll now ask chngproc to build the challenge. */

	for (i = 0; i < altsz; i++) {
		if (writeop(Cfd, COMM_CHNG_OP, CHNG_SYN) <= 0)
			goto out;
		else if (writestr(Cfd, COMM_DNSA, alts[i]) <= 0)
			goto out;
		else if (writestr(Cfd, COMM_THUMB, thumb) <= 0)
			goto out;
		else if (writestr(Cfd, COMM_TOK, chngs[i].token) <= 0)
			goto out;

		/* Read that the challenge has been made. */

		if (CHNG_ACK != readop(Cfd, COMM_CHNG_ACK))
			goto out;

		/* Write to the CA that it's ready. */

		if ( ! dochngresp(&c, &chngs[i], thumb))
			goto out;
	}

	/*
	 * We now wait on the ACME server for each domain.
	 * Connect to the server (assume it's the same server) once
	 * every five seconds.
	 */

	for (i = 0; i < altsz; i++) {
		if (1 == chngs[i].status)
			continue;

		if (chngs[i].retry++ >= RETRY_MAX) {
			warnx("%s: too many tries", chngs[i].uri);
			goto out;
		}

		/* Sleep before every attempt. */
		sleep(RETRY_DELAY);
		if ( ! dochngcheck(&c, &chngs[i]))
			goto out;
	}

	/*
	 * Write our acknowledgement that the challenges are over.
	 * The challenge process will remove all of the files.
	 */

	if (writeop(Cfd, COMM_CHNG_OP, CHNG_STOP) <= 0)
		goto out;

	/* Wait to receive the certificate itself. */

	if (NULL == (cert = readstr(kfd, COMM_CERT)))
		goto out;

	/*
	 * Otherwise, submit the CA for signing, download the signed
	 * copy, and ship that into the certificate process for copying.
	 */

	if ( ! docert(&c, paths.newcert, cert))
		goto out;
	else if (writeop(cfd, COMM_CSR_OP, CERT_UPDATE) <= 0)
		goto out;
	else if (writebuf(cfd, COMM_CSR, c.buf.buf, c.buf.sz) <= 0)
		goto out;

	/*
	 * Read back the issuer from the certproc.
	 * Then contact the issuer to get the certificate chain.
	 * Write this chain directly back to the certproc.
	 */

	if (NULL == (url = readstr(cfd, COMM_ISSUER)))
		goto out;
	else if ( ! dofullchain(&c, url))
		goto out;
	else if (writebuf(cfd, COMM_CHAIN, c.buf.buf, c.buf.sz) <= 0)
		goto out;

	rc = 1;
out:
	close(cfd);
	close(kfd);
	close(afd);
	close(Cfd);
	close(dfd);
	close(rfd);
	free(cert);
	free(url);
	free(thumb);
	http_uninit(c.cfg);
	free(c.buf.buf);
	if (NULL != chngs)
		for (i = 0; i < altsz; i++)
			json_free_challenge(&chngs[i]);
	free(chngs);
	json_free_capaths(&paths);
	return (rc);
}
