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

#include <sys/socket.h>
#include <sys/stat.h> /* mkdir(2) */

#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"

#define AGREEMENT "https://letsencrypt.org" \
		  "/documents/LE-SA-v1.2-November-15-2017.pdf"
#define SSL_DIR "/etc/ssl/acme"
#define SSL_PRIV_DIR "/etc/ssl/acme/private"
#define ETC_DIR "/etc/acme"
#define WWW_DIR "/var/www/acme"
#define PRIVKEY_FILE "privkey.pem"

/*
 * XXX: I arbitrarily choose a starting descriptor and hope that we
 * haven't opened up til this one.
 * This is a hack, but will suffice as I figure out a better way.
 */
enum	fds {
	FDS_REVOKE = 50,
#define	FDS_FIRST FDS_REVOKE
	FDS_DNS,
	FDS_FILE,
	FDS_CERT,
	FDS_CHALLENGE,
	FDS_ACCOUNT,
	FDS_KEY,
	FDS_NET,
	FDS__MAX
};

static	const char *const subps[COMP__MAX] = {
	"netproc", /* COMP_NET */
	"keyproc", /* COMP_KEY */
	"certproc", /* COMP_CERT */
	"acctproc", /* COMP_ACCOUNT */
	"chngproc", /* COMP_CHALLENGE */
	"fileproc", /* COMP_FILE */
	"dnsproc", /* COMP_DNS */
	"revokeproc", /* COMP_REVOKE */
};

static	void xrun(enum comp, const char **) __attribute__((noreturn));

/*
 * This isn't RFC1035 compliant, but does the bare minimum in making
 * sure that we don't get bogus domain names on the command line, which
 * might otherwise screw up our directory structure.
 * Returns zero on failure, non-zero on success.
 */
static int
domain_valid(const char *cp)
{

	for ( ; '\0' != *cp; cp++)
		if (!('.' == *cp || '-' == *cp ||
		    '_' == *cp || isalnum((int)*cp)))
			return (0);
	return (1);
}

/*
 * XXX: this function is a workaround.
 * It opens the socket pair properly, then makes sure that the file
 * descriptors don't clobber those we've set aside to use for the IPC
 * mechanism.
 */
static void
xsocketpair(int *fds)
{

	if (-1 == socketpair(AF_UNIX, SOCK_STREAM, 0, fds))
		err(EXIT_FAILURE, "socketpair");
	if ((fds[0] >= FDS_FIRST && fds[0] < FDS__MAX) ||
	    (fds[1] >= FDS_FIRST && fds[1] < FDS__MAX))
		errx(EXIT_FAILURE, "file descriptor clobbers "
			"predefined ipc channel");
}

/*
 * Duplicate "outfd" as "infd", then close "infd".
 */
static void
xdup(int infd, int outfd)
{

	if (-1 == dup2(infd, outfd)) 
		err(EXIT_FAILURE, "dup2");
	if (outfd != infd)
		close(infd);
}

/*
 * Execute "newargs", which is already created in main(), as a
 * subprocess of type "comp".
 */
static void
xrun(enum comp comp, const char **newargs)
{

	newargs[2] = subps[comp];
	execvp(newargs[0], (char *const *)newargs);
	warn("%s", newargs[0]);
	exit(EXIT_FAILURE);
	/* NOTREACHED */
}

int
main(int argc, char *argv[])
{
	const char	 *domain, *agreement = AGREEMENT, 
	      		 *challenge = NULL, *sp = NULL;
	const char	**alts = NULL, **newargs = NULL, *modval = NULL;
	char		 *certdir = NULL, *acctkey = NULL, 
			 *chngdir = NULL, *keyfile = NULL,
			 *keydir, *acctdir;
	int		  key_fds[2], acct_fds[2], chng_fds[2],
			  cert_fds[2], file_fds[2], dns_fds[2],
			  rvk_fds[2];
	int		  c, rc, newacct = 0, revocate = 0, force = 0,
			  staging = 0, multidir = 0, newkey = 0, 
			  backup = 0, build_certdir, build_ssldir, 
			  build_acctdir, expand = 0, ocsp = 0;
	pid_t		  pids[COMP__MAX];
	extern int	  verbose;
	extern enum comp  proccomp;
	size_t		  i, j, altsz, ne, newargsz;

	/*
	 * Start by copying over our arguments as if were going to run a
	 * subprocess with the "-x" argument (subprocess name).
	 * This is only used by the master process when invoking
	 * children in a distinct subprocess, but we run it for all
	 * processes in the interests of simplicity.
	 */

	newargsz = (size_t)argc + 5; /* nil ptr, '-x', arg, -X, arg */
	newargs = calloc(newargsz, sizeof(char *));
	if (NULL == newargs)
		err(EXIT_FAILURE, "calloc");
	newargs[0] = argv[0];
	newargs[1] = (char *)"-x";
	/* newargs[2] = the_subprocess */
	newargs[3] = "-X";
	newargs[4] = "";
	for (i = 1, j = 5; i < (size_t)argc; i++, j++)
		newargs[j] = argv[i];

	/* Now parse arguments. */

	while (-1 != (c = getopt(argc, argv, "beFmnNOrsva:f:c:C:k:t:x:X:"))) 
		switch (c) {
		case ('a'):
			agreement = optarg;
			break;
		case ('b'):
			backup = 1;
			break;
		case ('c'):
			free(certdir);
			if (NULL == (certdir = strdup(optarg)))
				err(EXIT_FAILURE, "strdup");
			break;
		case ('C'):
			free(chngdir);
			if (NULL == (chngdir = strdup(optarg)))
				err(EXIT_FAILURE, "strdup");
			break;
		case ('e'):
			expand = 1;
			break;
		case ('f'):
			free(acctkey);
			if (NULL == (acctkey = strdup(optarg)))
				err(EXIT_FAILURE, "strdup");
			break;
		case ('F'):
			force = 1;
			break;
		case ('k'):
			free(keyfile);
			if (NULL == (keyfile = strdup(optarg)))
				err(EXIT_FAILURE, "strdup");
			break;
		case ('m'):
			multidir = 1;
			break;
		case ('n'):
			newacct = 1;
			break;
		case ('N'):
			newkey = 1;
			break;
		case ('O'):
			ocsp = 1;
			break;
		case ('r'):
			revocate = 1;
			break;
		case ('s'):
			staging = 1;
			break;
		case ('t'):
			challenge = optarg;
			break;
		case ('v'):
			verbose = verbose ? 2 : 1;
			break;
		case ('x'):
			/*
			 * XXX Internally used: not to be documented.
			 * This flag dictates which subprocess is
			 * currently running.
			 */
			sp = optarg;
			break;
		case ('X'):
			/*
			 * XXX Internally used: not to be documented.
			 * We ignore these flags.
			 * I use this instead of hacking apart the
			 * argument list, which is foolhardy.
			 */
			modval = optarg;
			break;
		default:
			goto usage;
		}

	argc -= optind;
	argv += optind;

	if (0 == argc)
		goto usage;

	/* Forbidden in parent. */

	if (NULL != modval && NULL == sp) 
		goto usage;

	/* Make sure that the domains are sane. */

	for (i = 0; i < (size_t)argc; i++) {
		if (domain_valid(argv[i]))
			continue;
		errx(EXIT_FAILURE, "%s: bad domain syntax", argv[i]);
	}

	domain = argv[0];
	argc--;
	argv++;

	if ( ! checkprivs())
		errx(EXIT_FAILURE, "must be run as root");

	/* 
	 * Now we allocate our directories and file path buffers IFF we
	 * haven't specified them on the command-line.
	 * If we're in "multidir" (-m) mode, we use our initial domain
	 * name when specifying the prefixes.
	 * Otherwise, we put them all in a known location.
	 * We don't /do/ anything here: just build the paths.
	 */

	build_certdir = (NULL == certdir) && multidir;
	build_ssldir = (NULL == keyfile) && multidir;
	build_acctdir = (NULL == acctkey) && multidir;

	if (NULL == certdir)
		certdir = multidir ?
			doasprintf(SSL_DIR "/%s", domain) :
			strdup(SSL_DIR);
	if (NULL == keyfile)
		keyfile = multidir ?
			doasprintf(SSL_PRIV_DIR "/%s/"
				PRIVKEY_FILE, domain) :
			strdup(SSL_PRIV_DIR "/" PRIVKEY_FILE);
	if (NULL == acctkey)
		acctkey = multidir ?
			doasprintf(ETC_DIR "/%s/"
				PRIVKEY_FILE, domain) :
			strdup(ETC_DIR "/" PRIVKEY_FILE);
	if (NULL == chngdir)
		chngdir = strdup(WWW_DIR);

	keydir = multidir ?
		doasprintf(SSL_PRIV_DIR "/%s", domain) :
		strdup(SSL_PRIV_DIR);
	acctdir = multidir ?
		doasprintf(ETC_DIR "/%s", domain) :
		strdup(ETC_DIR);

	if (NULL == certdir || NULL == keyfile ||
	    NULL == acctkey || NULL == chngdir ||
	    NULL == keydir || NULL == acctdir)
		err(EXIT_FAILURE, "strdup");

	/* Set the zeroth altname as our domain. */

	altsz = argc + 1;
	alts = calloc(altsz, sizeof(char *));
	if (NULL == alts)
		err(EXIT_FAILURE, "calloc");
	alts[0] = domain;
	for (i = 0; i < (size_t)argc; i++)
		alts[i + 1] = argv[i];

	/*
	 * If we're in one of the re-executed child processes, do our
	 * processing here.
	 * If we're not (sp is NULL), then we continue straight into the
	 * forking and exec'ing phase.
	 */

	if (NULL == sp)
		goto main;

	/* Check if we're overriding any given values. */

	if (NULL != modval) {
		if (newacct && NULL != strchr(modval, 'n'))
			newacct = 0;
		if (newkey && NULL != strchr(modval, 'N'))
			newkey = 0;
	}

	if (0 == strcmp(sp, subps[COMP_REVOKE])) {
		proccomp = COMP_REVOKE;
		c = revokeproc(FDS_REVOKE, certdir,
			force, revocate, expand,
			(const char *const *)alts, altsz);
		free(alts);
		exit(c ? EXIT_SUCCESS : EXIT_FAILURE);
	} else if (0 == strcmp(sp, subps[COMP_DNS])) {
		proccomp = COMP_DNS;
		free(alts);
		c = dnsproc(FDS_DNS);
		exit(c ? EXIT_SUCCESS : EXIT_FAILURE);
	} else if (0 == strcmp(sp, subps[COMP_FILE])) {
		proccomp = COMP_FILE;
		free(alts);
		c = fileproc(FDS_FILE, backup, certdir);
		/*
		 * This is different from the other processes in that it
		 * can return 2 if the certificates were updated.
		 */
		exit(c > 1 ? 2 :
		    (c ? EXIT_SUCCESS : EXIT_FAILURE));
	} else if (0 == strcmp(sp, subps[COMP_CERT])) {
		proccomp = COMP_CERT;
		free(alts);
		c = certproc(FDS_CERT, FDS_FILE);
		exit(c ? EXIT_SUCCESS : EXIT_FAILURE);
	} else if (0 == strcmp(sp, subps[COMP_CHALLENGE])) {
		proccomp = COMP_CHALLENGE;
		free(alts);
		c = chngproc(FDS_CHALLENGE, chngdir, challenge);
		exit(c ? EXIT_SUCCESS : EXIT_FAILURE);
	} else if (0 == strcmp(sp, subps[COMP_ACCOUNT])) {
		proccomp = COMP_ACCOUNT;
		free(alts);
		c = acctproc(FDS_ACCOUNT, acctkey, newacct);
		exit(c ? EXIT_SUCCESS : EXIT_FAILURE);
	} else if (0 == strcmp(sp, subps[COMP_KEY])) {
		proccomp = COMP_KEY;
		c = keyproc(FDS_KEY, ocsp, keyfile,
			(const char **)alts, altsz, newkey);
		free(alts);
		exit(c ? EXIT_SUCCESS : EXIT_FAILURE);
	} else if (0 == strcmp(sp, subps[COMP_NET])) {
		proccomp = COMP_NET;
		c = netproc(FDS_KEY, FDS_ACCOUNT,
		    FDS_CHALLENGE, FDS_CERT,
		    FDS_DNS, FDS_REVOKE,
		    newacct, revocate, staging,
		    (const char *const *)alts, altsz,
		    agreement, challenge);
		free(alts);
		exit(c ? EXIT_SUCCESS : EXIT_FAILURE);
	} else if (NULL != sp)
		errx(EXIT_FAILURE, "%s: unknown subprocess", sp);

main:
	/*
	 * Begin by checking to see whether we already have our keys on
	 * disc when we've been ordered to generate them.
	 * If we have them, then append to our "disabled flags" that we
	 * don't want to perform these operations.
	 */

	if (newacct && -1 != access(acctkey, R_OK)) {
		newacct = 0;
		modval = "n";
		dodbg("%s: account key exists "
			"(not creating)", acctkey);
	}

	if (newkey && -1 != access(keyfile, R_OK)) {
		newkey = 0;
		modval = NULL != modval ? "nN" : "N";
		dodbg("%s: domain key exists "
			"(not creating)", keyfile);
	}

	/* Override variables. */

	if (NULL != modval)
		newargs[4] = modval;

	/*
	 * If we're running in multi-mode with default paths, try to
	 * build the domain-specific directory component if not found.
	 * This makes it easier to get started from nothing without
	 * mucking around in directories.
	 */

	if (build_certdir && -1 == access(certdir, R_OK)) {
		dodbg("%s: creating directory", certdir);
		if (-1 == mkdir(certdir, 0755))
			err(EXIT_FAILURE, "%s", certdir);
	}

	if (build_ssldir && -1 == access(keydir, R_OK)) {
		dodbg("%s: creating directory", keydir);
		if (-1 == mkdir(keydir, 0755))
			err(EXIT_FAILURE, "%s", keydir);
	}

	if (build_acctdir && -1 == access(acctdir, R_OK)) {
		dodbg("%s: creating directory", acctdir);
		if (-1 == mkdir(acctdir, 0755)) 
			err(EXIT_FAILURE, "%s", acctdir);
	}

	free(keydir);
	free(acctdir);
	keydir = acctdir = NULL;

	/*
	 * Do some quick checks to see if our paths exist.
	 * Run all of the tests before exiting on possible errors.
	 * We do this in the children, but it's easier to do it now to
	 * report any errors early on.
	 */

	ne = 0;

	if (-1 == access(certdir, R_OK)) {
		warnx("%s: -c directory must exist", certdir);
		ne++;
	}

	if ( ! newkey && -1 == access(keyfile, R_OK)) {
		warnx("%s: -k file must exist", keyfile);
		ne++;
	} 

	if (NULL == challenge && -1 == access(chngdir, R_OK)) {
		warnx("%s: -C directory must exist", chngdir);
		ne++;
	}

	if ( ! newacct && -1 == access(acctkey, R_OK)) {
		warnx("%s: -f file must exist", acctkey);
		ne++;
	} 

	if (ne > 0)
		exit(EXIT_FAILURE);

	/*
	 * Ok, here we go.
	 * Begin by opening channels that we'll use to communicate
	 * between our components.
	 * Then we start each process in sequence, closing out the
	 * descriptors we don't need and and having the child inherit
	 * the ones we do, using the descriptor values we've set aside
	 * (in "enum fds") for them to use.
	 */

	xsocketpair(key_fds);
	xsocketpair(acct_fds);
	xsocketpair(chng_fds);
	xsocketpair(cert_fds);
	xsocketpair(file_fds);
	xsocketpair(dns_fds);
	xsocketpair(rvk_fds);

	if (-1 == (pids[COMP_NET] = fork()))
		err(EXIT_FAILURE, "fork");

	if (0 == pids[COMP_NET]) {
		close(key_fds[0]);
		close(acct_fds[0]);
		close(chng_fds[0]);
		close(cert_fds[0]);
		close(file_fds[0]);
		close(file_fds[1]);
		close(dns_fds[0]);
		close(rvk_fds[0]);
		xdup(key_fds[1], FDS_KEY);
		xdup(acct_fds[1], FDS_ACCOUNT);
		xdup(chng_fds[1], FDS_CHALLENGE);
		xdup(cert_fds[1], FDS_CERT);
		xdup(dns_fds[1], FDS_DNS);
		xdup(rvk_fds[1], FDS_REVOKE);
		xrun(COMP_NET, newargs);
	}

	close(key_fds[1]);
	close(acct_fds[1]);
	close(chng_fds[1]);
	close(cert_fds[1]);
	close(dns_fds[1]);
	close(rvk_fds[1]);

	if (-1 == (pids[COMP_KEY] = fork()))
		err(EXIT_FAILURE, "fork");

	if (0 == pids[COMP_KEY]) {
		close(cert_fds[0]);
		close(dns_fds[0]);
		close(rvk_fds[0]);
		close(acct_fds[0]);
		close(chng_fds[0]);
		close(file_fds[0]);
		close(file_fds[1]);
		xdup(key_fds[0], FDS_KEY);
		xrun(COMP_KEY, newargs);
	}

	close(key_fds[0]);

	if (-1 == (pids[COMP_ACCOUNT] = fork()))
		err(EXIT_FAILURE, "fork");

	if (0 == pids[COMP_ACCOUNT]) {
		close(cert_fds[0]);
		close(dns_fds[0]);
		close(rvk_fds[0]);
		close(chng_fds[0]);
		close(file_fds[0]);
		close(file_fds[1]);
		xdup(acct_fds[0], FDS_ACCOUNT);
		xrun(COMP_ACCOUNT, newargs);
	}

	close(acct_fds[0]);

	if (-1 == (pids[COMP_CHALLENGE] = fork()))
		err(EXIT_FAILURE, "fork");

	if (0 == pids[COMP_CHALLENGE]) {
		close(cert_fds[0]);
		close(dns_fds[0]);
		close(rvk_fds[0]);
		close(file_fds[0]);
		close(file_fds[1]);
		xdup(chng_fds[0], FDS_CHALLENGE);
		xrun(COMP_CHALLENGE, newargs);
	}

	close(chng_fds[0]);

	if (-1 == (pids[COMP_CERT] = fork()))
		err(EXIT_FAILURE, "fork");

	if (0 == pids[COMP_CERT]) {
		close(dns_fds[0]);
		close(rvk_fds[0]);
		close(file_fds[1]);
		xdup(cert_fds[0], FDS_CERT);
		xdup(file_fds[0], FDS_FILE);
		xrun(COMP_CERT, newargs);
	}

	close(cert_fds[0]);
	close(file_fds[0]);

	if (-1 == (pids[COMP_FILE] = fork()))
		err(EXIT_FAILURE, "fork");

	if (0 == pids[COMP_FILE]) {
		close(dns_fds[0]);
		close(rvk_fds[0]);
		xdup(file_fds[1], FDS_FILE);
		xrun(COMP_FILE, newargs);
	}

	close(file_fds[1]);

	if (-1 == (pids[COMP_DNS] = fork()))
		err(EXIT_FAILURE, "fork");

	if (0 == pids[COMP_DNS]) {
		close(rvk_fds[0]);
		xdup(dns_fds[0], FDS_DNS);
		xrun(COMP_DNS, newargs);
	}

	close(dns_fds[0]);

	if (-1 == (pids[COMP_REVOKE] = fork()))
		err(EXIT_FAILURE, "fork");

	if (0 == pids[COMP_REVOKE]) {
		xdup(rvk_fds[0], FDS_REVOKE);
		xrun(COMP_REVOKE, newargs);
	}

	close(rvk_fds[0]);

	/* 
	 * Now all of the components have started.
	 * The main process can now just wait.
	 * Jail: sandbox, file-system, user. 
	 */

	if ( ! sandbox_before())
		exit(EXIT_FAILURE);
	else if ( ! dropfs(PATH_VAR_EMPTY))
		exit(EXIT_FAILURE);
	else if ( ! dropprivs())
		exit(EXIT_FAILURE);
	else if ( ! sandbox_after(0))
		exit(EXIT_FAILURE);

	/*
	 * Collect our subprocesses.
	 * Require that they both have exited cleanly.
	 */

	rc = checkexit(pids[COMP_KEY], COMP_KEY) +
	    checkexit(pids[COMP_CERT], COMP_CERT) +
	    checkexit(pids[COMP_NET], COMP_NET) +
	    checkexit_ext(&c, pids[COMP_FILE], COMP_FILE) +
	    checkexit(pids[COMP_ACCOUNT], COMP_ACCOUNT) +
	    checkexit(pids[COMP_CHALLENGE], COMP_CHALLENGE) +
	    checkexit(pids[COMP_DNS], COMP_DNS) +
	    checkexit(pids[COMP_REVOKE], COMP_REVOKE);

	free(certdir);
	free(keyfile);
	free(acctkey);
	free(chngdir);
	free(alts);
	free(newargs);

	return (COMP__MAX != rc ? EXIT_FAILURE :
	    (2 == c ? EXIT_SUCCESS : 2));
usage:
	fprintf(stderr, "usage: %s "
		"[-beFmnNrsv] "
		"[-a agreement] "
		"[-C challengedir] "
		"[-c certdir] "
		"[-f accountkey] "
		"[-k domainkey] "
		"[-t challenge] "
		"domain [altnames...]\n", 
		getprogname());
	free(certdir);
	free(keyfile);
	free(acctkey);
	free(chngdir);
	return (EXIT_FAILURE);
}
