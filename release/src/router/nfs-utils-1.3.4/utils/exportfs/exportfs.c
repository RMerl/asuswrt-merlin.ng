/*
 * utils/exportfs/exportfs.c
 *
 * Export file systems to knfsd
 *
 * Copyright (C) 1995, 1996, 1997 Olaf Kirch <okir@monad.swb.de>
 *
 * Extensive changes, 1999, Neil Brown <neilb@cse.unsw.edu.au>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <getopt.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>

#define INT_TO_LONG_THRESHOLD_SECS (INT_MAX - (60 * 60 * 24))

#include "sockaddr.h"
#include "misc.h"
#include "nfslib.h"
#include "exportfs.h"
#include "xlog.h"

static void	export_all(int verbose);
static void	exportfs(char *arg, char *options, int verbose);
static void	unexportfs(char *arg, int verbose);
static void	exports_update(int verbose);
static void	dump(int verbose, int export_format);
static void	error(nfs_export *exp, int err);
static void	usage(const char *progname, int n);
static void	validate_export(nfs_export *exp);
static int	matchhostname(const char *hostname1, const char *hostname2);
static int	export_d_read(const char *dname);
static void grab_lockfile(void);
static void release_lockfile(void);

static const char *lockfile = EXP_LOCKFILE;
static int _lockfd = -1;

/*
 * If we aren't careful, changes made by exportfs can be lost
 * when multiple exports process run at once:
 *
 *	exportfs process 1	exportfs process 2
 *	------------------------------------------
 *	reads etab version A	reads etab version A
 *	adds new export B	adds new export C
 *	writes A+B		writes A+C
 *
 * The locking in support/export/xtab.c will prevent mountd from
 * seeing a partially written version of etab, and will prevent
 * the two writers above from writing simultaneously and
 * corrupting etab, but to prevent problems like the above we
 * need these additional lockfile() routines.
 */
static void
grab_lockfile()
{
	_lockfd = open(lockfile, O_CREAT|O_RDWR, 0666);
	if (_lockfd != -1)
		lockf(_lockfd, F_LOCK, 0);
}
static void
release_lockfile()
{
	if (_lockfd != -1)
		lockf(_lockfd, F_ULOCK, 0);
}

int
main(int argc, char **argv)
{
	char	*options = NULL;
	char	*progname = NULL;
	int	f_export = 1;
	int	f_all = 0;
	int	f_verbose = 0;
	int	f_export_format = 0;
	int	f_reexport = 0;
	int	f_ignore = 0;
	int	i, c;
	int	new_cache = 0;
	int	force_flush = 0;

	if ((progname = strrchr(argv[0], '/')) != NULL)
		progname++;
	else
		progname = argv[0];

	xlog_open(progname);
	xlog_stderr(1);
	xlog_syslog(0);

	while ((c = getopt(argc, argv, "ad:fhio:ruvs")) != EOF) {
		switch(c) {
		case 'a':
			f_all = 1;
			break;
		case 'd':
			xlog_sconfig(optarg, 1);
			break;
		case 'f':
			force_flush = 1;
			break;
		case 'h':
			usage(progname, 0);
			break;
		case 'i':
			f_ignore = 1;
			break;
		case 'o':
			options = optarg;
			break;
		case 'r':
			f_reexport = 1;
			f_all = 1;
			break;
		case 'u':
			f_export = 0;
			break;
		case 'v':
			f_verbose = 1;
			break;
		case 's':
			f_export_format = 1;
			break;
		default:
			usage(progname, 1);
			break;
		}
	}

	if (optind != argc && f_all) {
		xlog(L_ERROR, "extra arguments are not permitted with -a or -r");
		return 1;
	}
	if (f_ignore && (f_all || ! f_export)) {
		xlog(L_ERROR, "-i not meaningful with -a, -r or -u");
		return 1;
	}
	if (f_reexport && ! f_export) {
		xlog(L_ERROR, "-r and -u are incompatible");
		return 1;
	}
	new_cache = check_new_cache();
	if (optind == argc && ! f_all) {
		if (force_flush) {
			if (new_cache)
				cache_flush(1);
			else {
				xlog(L_ERROR, "-f is available only "
					"with new cache controls. "
					"Mount /proc/fs/nfsd first");
				return 1;
			}
			return 0;
		} else {
			xtab_export_read();
			dump(f_verbose, f_export_format);
			return 0;
		}
	}

	/*
	 * Serialize things as best we can
	 */
	grab_lockfile();
	atexit(release_lockfile);

	if (f_export && ! f_ignore) {
		if (! (export_read(_PATH_EXPORTS) +
		       export_d_read(_PATH_EXPORTS_D))) {
			if (f_verbose)
				xlog(L_WARNING, "No file systems exported!");
		}
	}
	if (f_export) {
		if (f_all)
			export_all(f_verbose);
		else
			for (i = optind; i < argc ; i++)
				exportfs(argv[i], options, f_verbose);
	}
	/* If we are unexporting everything, then
	 * don't care about what should be exported, as that
	 * may require DNS lookups..
	 */
	if (! ( !f_export && f_all)) {
		/* note: xtab_*_read does not update entries if they already exist,
		 * so this will not lose new options
		 */
		if (!f_reexport)
			xtab_export_read();
		if (!f_export)
			for (i = optind ; i < argc ; i++)
				unexportfs(argv[i], f_verbose);
		if (!new_cache)
			rmtab_read();
	}
	if (!new_cache) {
		xtab_mount_read();
		exports_update(f_verbose);
	}
	xtab_export_write();
	if (new_cache)
		cache_flush(force_flush);
	if (!new_cache)
		xtab_mount_write();

	return export_errno;
}

static void
exports_update_one(nfs_export *exp, int verbose)
{
		/* check mountpoint option */
	if (exp->m_mayexport &&
	    exp->m_export.e_mountpoint &&
	    !is_mountpoint(exp->m_export.e_mountpoint[0]?
			   exp->m_export.e_mountpoint:
			   exp->m_export.e_path)) {
		printf("%s not exported as %s not a mountpoint.\n",
		       exp->m_export.e_path, exp->m_export.e_mountpoint);
		exp->m_mayexport = 0;
	}
	if (exp->m_mayexport && ((exp->m_exported<1) || exp->m_changed)) {
		if (verbose)
			printf("%sexporting %s:%s to kernel\n",
			       exp->m_exported ?"re":"",
			       exp->m_client->m_hostname,
			       exp->m_export.e_path);
		if (!export_export(exp))
			error(exp, errno);
	}
	if (exp->m_exported && ! exp->m_mayexport) {
		if (verbose)
			printf("unexporting %s:%s from kernel\n",
			       exp->m_client->m_hostname,
			       exp->m_export.e_path);
		if (!export_unexport(exp))
			error(exp, errno);
	}
}


/* we synchronise intention with reality.
 * entries with m_mayexport get exported
 * entries with m_exported but not m_mayexport get unexported
 * looking at m_client->m_type == MCL_FQDN and m_client->m_type == MCL_GSS only
 */
static void
exports_update(int verbose)
{
	nfs_export 	*exp;

	for (exp = exportlist[MCL_FQDN].p_head; exp; exp=exp->m_next) {
		exports_update_one(exp, verbose);
	}
	for (exp = exportlist[MCL_GSS].p_head; exp; exp=exp->m_next) {
		exports_update_one(exp, verbose);
	}
}

/*
 * export_all finds all entries and
 *    marks them xtabent and mayexport so that they get exported
 */
static void
export_all(int verbose)
{
	nfs_export	*exp;
	int		i;

	for (i = 0; i < MCL_MAXTYPES; i++) {
		for (exp = exportlist[i].p_head; exp; exp = exp->m_next) {
			if (verbose)
				printf("exporting %s:%s\n",
				       exp->m_client->m_hostname,
				       exp->m_export.e_path);
			exp->m_xtabent = 1;
			exp->m_mayexport = 1;
			exp->m_changed = 1;
			exp->m_warned = 0;
			validate_export(exp);
		}
	}
}


static void
exportfs_parsed(char *hname, char *path, char *options, int verbose)
{
	struct exportent *eep;
	nfs_export	*exp = NULL;
	struct addrinfo	*ai = NULL;
	int		htype;

	if ((htype = client_gettype(hname)) == MCL_FQDN) {
		ai = host_addrinfo(hname);
		if (ai != NULL) {
			exp = export_find(ai, path);
			hname = ai->ai_canonname;
		}
	} else
		exp = export_lookup(hname, path, 0);

	if (!exp) {
		if (!(eep = mkexportent(hname, path, options)) ||
		    !(exp = export_create(eep, 0)))
			goto out;
	} else if (!updateexportent(&exp->m_export, options))
		goto out;

	if (verbose)
		printf("exporting %s:%s\n", exp->m_client->m_hostname,
			exp->m_export.e_path);
	exp->m_xtabent = 1;
	exp->m_mayexport = 1;
	exp->m_changed = 1;
	exp->m_warned = 0;
	validate_export(exp);

out:
	freeaddrinfo(ai);
}

static int exportfs_generic(char *arg, char *options, int verbose)
{
	char *path;

	if ((path = strchr(arg, ':')) != NULL)
		*path++ = '\0';

	if (!path || *path != '/')
		return 1;

	exportfs_parsed(arg, path, options, verbose);
	return 0;
}

static int exportfs_ipv6(char *arg, char *options, int verbose)
{
	char *path, *c;

	arg++;
	c = strchr(arg, ']');
	if (c == NULL)
		return 1;

	/* no colon means this is a wildcarded DNS hostname */
	if (memchr(arg, ':', c - arg) == NULL)
		return exportfs_generic(--arg, options, verbose);

	path = strstr(c, ":/");
	if (path == NULL)
		return 1;
	*path++ = '\0';

	/* if there's anything between the closing brace and the
	 * path separator, it's probably a prefix length */
	memmove(c, c + 1, path - c);

	exportfs_parsed(arg, path, options, verbose);
	return 0;
}

static void
exportfs(char *arg, char *options, int verbose)
{
	int failed;

	if (*arg == '[')
		failed = exportfs_ipv6(arg, options, verbose);
	else
		failed = exportfs_generic(arg, options, verbose);
	if (failed)
		xlog(L_ERROR, "Invalid export syntax: %s", arg);
}

static void
unexportfs_parsed(char *hname, char *path, int verbose)
{
	nfs_export	*exp;
	struct addrinfo *ai = NULL;
	int		htype;
	int		success = 0;

	if ((htype = client_gettype(hname)) == MCL_FQDN) {
		ai = host_addrinfo(hname);
		if (ai)
			hname = ai->ai_canonname;
	}

	/*
	 * It's possible the specified path ends with a '/'. But
	 * the entry from exportlist won't has the trailing '/',
	 * so need to deal with it.
	*/
	size_t nlen = strlen(path);
	while (path[nlen - 1] == '/')
		nlen--;

	for (exp = exportlist[htype].p_head; exp; exp = exp->m_next) {
		if (path && strncmp(path, exp->m_export.e_path, nlen))
			continue;
		if (htype != exp->m_client->m_type)
			continue;
		if (htype == MCL_FQDN
		    && !matchhostname(exp->m_export.e_hostname,
					  hname))
			continue;
		if (htype != MCL_FQDN
		    && strcasecmp(exp->m_export.e_hostname, hname))
			continue;
		if (verbose) {
#if 0
			if (exp->m_exported) {
				printf("unexporting %s:%s from kernel\n",
				       exp->m_client->m_hostname,
				       exp->m_export.e_path);
			}
			else
#endif
				printf("unexporting %s:%s\n",
					exp->m_client->m_hostname,
					exp->m_export.e_path);
		}
#if 0
		if (exp->m_exported && !export_unexport(exp))
			error(exp, errno);
#endif
		exp->m_xtabent = 0;
		exp->m_mayexport = 0;
		success = 1;
	}
	if (!success)
		xlog(L_ERROR, "Could not find '%s:%s' to unexport.", hname, path);

	freeaddrinfo(ai);
}

static int unexportfs_generic(char *arg, int verbose)
{
	char *path;

	if ((path = strchr(arg, ':')) != NULL)
		*path++ = '\0';

	if (!path || *path != '/')
		return 1;

	unexportfs_parsed(arg, path, verbose);
	return 0;
}

static int unexportfs_ipv6(char *arg, int verbose)
{
	char *path, *c;

	arg++;
	c = strchr(arg, ']');
	if (c == NULL)
		return 1;

	/* no colon means this is a wildcarded DNS hostname */
	if (memchr(arg, ':', c - arg) == NULL)
		return unexportfs_generic(--arg, verbose);

	path = strstr(c, ":/");
	if (path == NULL)
		return 1;
	*path++ = '\0';

	/* if there's anything between the closing brace and the
	 * path separator, it's probably a prefix length */
	memmove(c, c + 1, path - c);

	unexportfs_parsed(arg, path, verbose);
	return 0;
}

static void
unexportfs(char *arg, int verbose)
{
	int failed;

	if (*arg == '[')
		failed = unexportfs_ipv6(arg, verbose);
	else
		failed = unexportfs_generic(arg, verbose);
	if (failed)
		xlog(L_ERROR, "Invalid export syntax: %s", arg);
}

static int can_test(void)
{
	char buf[1024] = { 0 };
	int fd;
	int n;
	size_t bufsiz = sizeof(buf);

	fd = open("/proc/net/rpc/auth.unix.ip/channel", O_WRONLY);
	if (fd < 0)
		return 0;

	/*
	 * We introduce tolerance of 1 day to ensure that we use a
	 * LONG_MAX for the expiry timestamp before it is actually
	 * needed. To use LONG_MAX, the kernel code must have
	 * commit 2f74f972  (sunrpc: prepare NFS for 2038).
	 */
	if (time(NULL) > INT_TO_LONG_THRESHOLD_SECS)
		snprintf(buf, bufsiz-1, "nfsd 0.0.0.0 %ld -test-client-\n", LONG_MAX);
	else
		snprintf(buf, bufsiz-1, "nfsd 0.0.0.0 %d -test-client-\n", INT_MAX);

	n = write(fd, buf, strlen(buf));
	close(fd);
	if (n < 0)
		return 0;

	fd = open("/proc/net/rpc/nfsd.export/channel", O_WRONLY);
	if (fd < 0)
		return 0;
	close(fd);
	return 1;
}

static int test_export(char *path, int with_fsid)
{
	/* beside max path, buf size should take protocol str into account */
	char buf[NFS_MAXPATHLEN+1+64] = { 0 };
	char *bp = buf;
	int len = sizeof(buf);
	int fd, n;

	n = snprintf(buf, len, "-test-client- ");
	bp += n;
	len -= n;
	qword_add(&bp, &len, path);
	if (len < 1)
		return 0;
	snprintf(bp, len, " 3 %d 65534 65534 0\n", with_fsid ? NFSEXP_FSID : 0);
	fd = open("/proc/net/rpc/nfsd.export/channel", O_WRONLY);
	if (fd < 0)
		return 0;
	n = write(fd, buf, strlen(buf));
	close(fd);
	if (n < 0)
		return 0;
	return 1;
}

static void
validate_export(nfs_export *exp)
{
	/* Check that the given export point is potentially exportable.
	 * We just give warnings here, don't cause anything to fail.
	 * If a path doesn't exist, or is not a dir or file, give an warning
	 * otherwise trial-export to '-test-client-' and check for failure.
	 */
	struct stat stb;
	char *path = exp->m_export.e_path;
	struct statfs64 stf;
	int fs_has_fsid = 0;

	if (stat(path, &stb) < 0) {
		xlog(L_ERROR, "Failed to stat %s: %m", path);
		return;
	}
	if (!S_ISDIR(stb.st_mode)) {
		xlog(L_ERROR, "%s is not a directory. "
			"Remote access will fail", path);
		return;
	}
	if (!can_test())
		return;

	if (!statfs64(path, &stf) &&
	    (stf.f_fsid.__val[0] || stf.f_fsid.__val[1]))
		fs_has_fsid = 1;

	if ((exp->m_export.e_flags & NFSEXP_FSID) || exp->m_export.e_uuid ||
	    fs_has_fsid) {
		if ( !test_export(path, 1)) {
			xlog(L_ERROR, "%s does not support NFS export", path);
			return;
		}
	} else if ( ! test_export(path, 0)) {
		if (test_export(path, 1))
			xlog(L_ERROR, "%s requires fsid= for NFS export", path);
		else
			xlog(L_ERROR, "%s does not support NFS export", path);
		return;

	}
}

static _Bool
is_hostname(const char *sp)
{
	if (*sp == '\0' || *sp == '@')
		return false;

	for (; *sp != '\0'; sp++) {
		if (*sp == '*' || *sp == '?' || *sp == '[' || *sp == '/')
			return false;
		if (*sp == '\\' && sp[1] != '\0')
			sp++;
	}

	return true;
}

/*
 * Take care to perform an explicit reverse lookup on presentation
 * addresses.  Otherwise we don't get a real canonical name or a
 * complete list of addresses.
 */
static struct addrinfo *
address_list(const char *hostname)
{
	struct addrinfo *ai;
	char *cname;

	ai = host_pton(hostname);
	if (ai != NULL) {
		/* @hostname was a presentation address */
		cname = host_canonname(ai->ai_addr);
		freeaddrinfo(ai);
		if (cname != NULL)
			goto out;
	}
	/* @hostname was a hostname or had no reverse mapping */
	cname = strdup(hostname);
	if (cname == NULL)
		return NULL;

out:
	ai = host_addrinfo(cname);
	free(cname);
	return ai;
}

static int
matchhostname(const char *hostname1, const char *hostname2)
{
	struct addrinfo *results1 = NULL, *results2 = NULL;
	struct addrinfo *ai1, *ai2;
	int result = 0;

	if (strcasecmp(hostname1, hostname2) == 0)
		return 1;

	/*
	 * Don't pass export wildcards or netgroup names to DNS
	 */
	if (!is_hostname(hostname1) || !is_hostname(hostname2))
		return 0;

	results1 = address_list(hostname1);
	if (results1 == NULL)
		goto out;
	results2 = address_list(hostname2);
	if (results2 == NULL)
		goto out;

	if (strcasecmp(results1->ai_canonname, results2->ai_canonname) == 0) {
		result = 1;
		goto out;
	}

	for (ai1 = results1; ai1 != NULL; ai1 = ai1->ai_next)
		for (ai2 = results2; ai2 != NULL; ai2 = ai2->ai_next)
			if (nfs_compare_sockaddr(ai1->ai_addr, ai2->ai_addr)) {
				result = 1;
				break;
			}

out:
	freeaddrinfo(results1);
	freeaddrinfo(results2);
	return result;
}

/* Based on mnt_table_parse_dir() in
   util-linux-ng/shlibs/mount/src/tab_parse.c */
static int
export_d_read(const char *dname)
{
	int n = 0, i;
	struct dirent **namelist = NULL;
	int volumes = 0;


	n = scandir(dname, &namelist, NULL, versionsort);
	if (n < 0) {
		if (errno == ENOENT)
			/* Silently return */
			return volumes;
		xlog(L_NOTICE, "scandir %s: %s", dname, strerror(errno));
	} else if (n == 0)
		return volumes;

	for (i = 0; i < n; i++) {
		struct dirent *d = namelist[i];
		size_t namesz;
		char fname[PATH_MAX + 1];
		int fname_len;


		if (d->d_type != DT_UNKNOWN
		    && d->d_type != DT_REG
		    && d->d_type != DT_LNK)
			continue;
		if (*d->d_name == '.')
			continue;

#define _EXT_EXPORT_SIZ   (sizeof(_EXT_EXPORT) - 1)
		namesz = strlen(d->d_name);
		if (!namesz
		    || namesz < _EXT_EXPORT_SIZ + 1
		    || strcmp(d->d_name + (namesz - _EXT_EXPORT_SIZ),
			      _EXT_EXPORT))
			continue;

		fname_len = snprintf(fname, PATH_MAX +1, "%s/%s", dname, d->d_name);
		if (fname_len > PATH_MAX) {
			xlog(L_WARNING, "Too long file name: %s in %s", d->d_name, dname);
			continue;
		}

		volumes += export_read(fname);
	}

	for (i = 0; i < n; i++)
		free(namelist[i]);
	free(namelist);

	return volumes;
}

static char
dumpopt(char c, char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	printf("%c", c);
	vprintf(fmt, ap);
	va_end(ap);
	return ',';
}

static void
dump(int verbose, int export_format)
{
	/* buf[] size should >= sizeof(struct exportent->e_path) */
	char buf[NFS_MAXPATHLEN+1] = { 0 };
	char *bp;
	int len;
	nfs_export	*exp;
	struct exportent *ep;
	int		htype;
	char		*hname, c;

	for (htype = 0; htype < MCL_MAXTYPES; htype++) {
		for (exp = exportlist[htype].p_head; exp; exp = exp->m_next) {
			ep = &exp->m_export;
			if (!exp->m_xtabent)
			    continue; /* neilb */
			if (htype == MCL_ANONYMOUS)
				hname = (export_format) ? "*" : "<world>";
			else
				hname = ep->e_hostname;
			if (strlen(ep->e_path) > 14 && !export_format)
				printf("%-14s\n\t\t%s", ep->e_path, hname);
			else
				if (export_format) {
					bp = buf;
					len = sizeof(buf) - 1;
					qword_add(&bp, &len, ep->e_path);
					*bp = '\0';
					printf("%s %s", buf, hname);
				} else {
					printf("%-14s\t%s", ep->e_path, hname);
				}

			if (!verbose && !export_format) {
				printf("\n");
				continue;
			}
			c = '(';
			if (ep->e_flags & NFSEXP_READONLY)
				c = dumpopt(c, "ro");
			else
				c = dumpopt(c, "rw");
			if (ep->e_flags & NFSEXP_ASYNC)
				c = dumpopt(c, "async");
			if (ep->e_flags & NFSEXP_GATHERED_WRITES)
				c = dumpopt(c, "wdelay");
			if (ep->e_flags & NFSEXP_NOHIDE)
				c = dumpopt(c, "nohide");
			if (ep->e_flags & NFSEXP_CROSSMOUNT)
				c = dumpopt(c, "crossmnt");
			if (ep->e_flags & NFSEXP_INSECURE_PORT)
				c = dumpopt(c, "insecure");
			if (ep->e_flags & NFSEXP_ROOTSQUASH)
				c = dumpopt(c, "root_squash");
			else
				c = dumpopt(c, "no_root_squash");
			if (ep->e_flags & NFSEXP_ALLSQUASH)
				c = dumpopt(c, "all_squash");
			if (ep->e_flags & NFSEXP_NOSUBTREECHECK)
				c = dumpopt(c, "no_subtree_check");
			if (ep->e_flags & NFSEXP_NOAUTHNLM)
				c = dumpopt(c, "insecure_locks");
			if (ep->e_flags & NFSEXP_NOREADDIRPLUS)
				c = dumpopt(c, "nordirplus");
			if (ep->e_flags & NFSEXP_NOACL)
				c = dumpopt(c, "no_acl");
			if (ep->e_flags & NFSEXP_PNFS)
				c = dumpopt(c, "pnfs");
			if (ep->e_flags & NFSEXP_FSID)
				c = dumpopt(c, "fsid=%d", ep->e_fsid);
			if (ep->e_uuid)
				c = dumpopt(c, "fsid=%s", ep->e_uuid);
			if (ep->e_mountpoint)
				c = dumpopt(c, "mountpoint%s%s",
					    ep->e_mountpoint[0]?"=":"",
					    ep->e_mountpoint);
			if (ep->e_anonuid != 65534)
				c = dumpopt(c, "anonuid=%d", ep->e_anonuid);
			if (ep->e_anongid != 65534)
				c = dumpopt(c, "anongid=%d", ep->e_anongid);
			switch(ep->e_fslocmethod) {
			case FSLOC_NONE:
				break;
			case FSLOC_REFER:
				c = dumpopt(c, "refer=%s", ep->e_fslocdata);
				break;
			case FSLOC_REPLICA:
				c = dumpopt(c, "replicas=%s", ep->e_fslocdata);
				break;
#ifdef DEBUG
			case FSLOC_STUB:
				c = dumpopt(c, "fsloc=stub");
				break;
#endif
			}
			secinfo_show(stdout, ep);
			printf("%c\n", (c != '(')? ')' : ' ');
		}
	}
}

static void
error(nfs_export *exp, int err)
{
	xlog(L_ERROR, "%s:%s: %s", exp->m_client->m_hostname,
		exp->m_export.e_path, strerror(err));
}

static void
usage(const char *progname, int n)
{
	fprintf(stderr, "usage: %s [-adfhioruvs] [host:/path]\n", progname);
	exit(n);
}
