/*
 * (C) 2000-2006 by the netfilter coreteam <coreteam@netfilter.org>:
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "config.h"
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <netdb.h>
#include <spawn.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/ether.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#if defined(HAVE_LINUX_MAGIC_H)
#	include <linux/magic.h> /* for PROC_SUPER_MAGIC */
#elif defined(HAVE_LINUX_PROC_FS_H)
#	include <linux/proc_fs.h>	/* Linux 2.4 */
#else
#	define PROC_SUPER_MAGIC	0x9fa0
#endif

#include <xtables.h>
#include <limits.h> /* INT_MAX in ip_tables.h/ip6_tables.h */
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv6/ip6_tables.h>
#include <libiptc/libxtc.h>
#include <libiptc/linux_list.h>

#ifndef NO_SHARED_LIBS
#include <dlfcn.h>
#endif
#ifndef IPT_SO_GET_REVISION_MATCH /* Old kernel source. */
#	define IPT_SO_GET_REVISION_MATCH	(IPT_BASE_CTL + 2)
#	define IPT_SO_GET_REVISION_TARGET	(IPT_BASE_CTL + 3)
#endif
#ifndef IP6T_SO_GET_REVISION_MATCH /* Old kernel source. */
#	define IP6T_SO_GET_REVISION_MATCH	68
#	define IP6T_SO_GET_REVISION_TARGET	69
#endif
#include <getopt.h>
#include "iptables/internal.h"
#include "xtables_internal.h"

#define NPROTO	255

#ifndef PROC_SYS_MODPROBE
#define PROC_SYS_MODPROBE "/proc/sys/kernel/modprobe"
#endif

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

/* we need this for ip6?tables-restore.  ip6?tables-restore.c sets line to the
 * current line of the input file, in order  to give a more precise error
 * message.  ip6?tables itself doesn't need this, so it is initialized to the
 * magic number of -1 */
int line = -1;

void basic_exit_err(enum xtables_exittype status, const char *msg, ...) __attribute__((noreturn, format(printf,2,3)));

struct xtables_globals *xt_params = NULL;

void basic_exit_err(enum xtables_exittype status, const char *msg, ...)
{
	va_list args;

	va_start(args, msg);
	fprintf(stderr, "%s v%s: ", xt_params->program_name, xt_params->program_version);
	vfprintf(stderr, msg, args);
	va_end(args);
	fprintf(stderr, "\n");
	if (status == PARAMETER_PROBLEM) {
		if (line != -1)
			fprintf(stderr, "Error occurred at line: %d\n", line);
		fprintf(stderr, "Try `%s -h' or '%s --help' for more information.\n",
				xt_params->program_name, xt_params->program_name);
	} else if (status == VERSION_PROBLEM) {
		fprintf(stderr,
			"Perhaps %s or your kernel needs to be upgraded.\n",
			xt_params->program_name);
	}
	/* On error paths, make sure that we don't leak memory */
	xtables_free_opts(1);
	exit(status);
}

void xtables_free_opts(int unused)
{
	free(xt_params->opts);
	xt_params->opts = NULL;
}

struct option *xtables_merge_options(struct option *orig_opts,
				     struct option *oldopts,
				     const struct option *newopts,
				     unsigned int *option_offset)
{
	unsigned int num_oold = 0, num_old = 0, num_new = 0, i;
	struct option *merge, *mp;

	if (newopts == NULL)
		return oldopts;

	for (num_oold = 0; orig_opts[num_oold].name; num_oold++) ;
	if (oldopts != NULL)
		for (num_old = 0; oldopts[num_old].name; num_old++) ;
	for (num_new = 0; newopts[num_new].name; num_new++) ;

	/*
	 * Since @oldopts also has @orig_opts already (and does so at the
	 * start), skip these entries.
	 */
	if (oldopts != NULL) {
		oldopts += num_oold;
		num_old -= num_oold;
	}

	merge = malloc(sizeof(*mp) * (num_oold + num_old + num_new + 1));
	if (merge == NULL)
		return NULL;

	/* Let the base options -[ADI...] have precedence over everything */
	memcpy(merge, orig_opts, sizeof(*mp) * num_oold);
	mp = merge + num_oold;

	/* Second, the new options */
	xt_params->option_offset += XT_OPTION_OFFSET_SCALE;
	*option_offset = xt_params->option_offset;
	memcpy(mp, newopts, sizeof(*mp) * num_new);

	for (i = 0; i < num_new; ++i, ++mp)
		mp->val += *option_offset;

	/* Third, the old options */
	if (oldopts != NULL) {
		memcpy(mp, oldopts, sizeof(*mp) * num_old);
		mp += num_old;
	}
	xtables_free_opts(0);

	/* Clear trailing entry */
	memset(mp, 0, sizeof(*mp));
	return merge;
}

static const struct xtables_afinfo afinfo_ipv4 = {
	.kmod          = "ip_tables",
	.proc_exists   = "/proc/net/ip_tables_names",
	.libprefix     = "libipt_",
	.family	       = NFPROTO_IPV4,
	.ipproto       = IPPROTO_IP,
	.so_rev_match  = IPT_SO_GET_REVISION_MATCH,
	.so_rev_target = IPT_SO_GET_REVISION_TARGET,
};

static const struct xtables_afinfo afinfo_ipv6 = {
	.kmod          = "ip6_tables",
	.proc_exists   = "/proc/net/ip6_tables_names",
	.libprefix     = "libip6t_",
	.family        = NFPROTO_IPV6,
	.ipproto       = IPPROTO_IPV6,
	.so_rev_match  = IP6T_SO_GET_REVISION_MATCH,
	.so_rev_target = IP6T_SO_GET_REVISION_TARGET,
};

/* Dummy families for arptables-compat and ebtables-compat. Leave structure
 * fields that we don't use unset.
 */
static const struct xtables_afinfo afinfo_bridge = {
	.libprefix     = "libebt_",
	.family        = NFPROTO_BRIDGE,
};

static const struct xtables_afinfo afinfo_arp = {
	.libprefix     = "libarpt_",
	.family        = NFPROTO_ARP,
};

const struct xtables_afinfo *afinfo;

/* Search path for Xtables .so files */
static const char *xtables_libdir;

/* the path to command to load kernel module */
const char *xtables_modprobe_program;

/* Keep track of matches/targets pending full registration: linked lists. */
struct xtables_match *xtables_pending_matches;
struct xtables_target *xtables_pending_targets;

/* Keep track of fully registered external matches/targets: linked lists. */
struct xtables_match *xtables_matches;
struct xtables_target *xtables_targets;

/* While true, xtables_compatible_revision() probes the kernel silently and
 * strictly: it emits no diagnostics and does not pretend that revision 0 is
 * supported.
 *
 * It is set only for the probes this file added, that is, those issued after a
 * revision of the same extension has already been registered. That success
 * proves the probe mechanism itself works, so a rejection reported afterwards
 * is a truthful answer and needs neither a warning nor the legacy revision-0
 * fallback. Probes made before the first success are left exactly as upstream
 * had them and keep the flag clear, whether or not the preferred revision was
 * rejected. */
static bool quiet_revision_probe;

/* Fully register a match/target which was previously partially registered. */
static bool xtables_fully_register_pending_match(struct xtables_match *me,
						 struct xtables_match *prev);
static bool xtables_fully_register_pending_target(struct xtables_target *me,
						  struct xtables_target *prev);

#ifndef NO_SHARED_LIBS
/* registry for loaded shared objects to close later */
struct dlreg {
	struct dlreg *next;
	void *handle;
};
static struct dlreg *dlreg = NULL;

static int dlreg_add(void *handle)
{
	struct dlreg *new = malloc(sizeof(*new));

	if (!new)
		return -1;

	new->handle = handle;
	new->next = dlreg;
	dlreg = new;
	return 0;
}

static void dlreg_free(void)
{
	struct dlreg *next;

	while (dlreg) {
		next = dlreg->next;
		dlclose(dlreg->handle);
		free(dlreg);
		dlreg = next;
	}
}
#endif

struct notarget {
	struct hlist_node node;
	char name[];
};

#define NOTARGET_HSIZE	512
static struct hlist_head notargets[NOTARGET_HSIZE];

static void notargets_hlist_init(void)
{
	int i;

	for (i = 0; i < NOTARGET_HSIZE; i++)
		INIT_HLIST_HEAD(&notargets[i]);
}

static void notargets_hlist_free(void)
{
	struct hlist_node *pos, *n;
	struct notarget *cur;
	int i;

	for (i = 0; i < NOTARGET_HSIZE; i++) {
		hlist_for_each_entry_safe(cur, pos, n, &notargets[i], node) {
			hlist_del(&cur->node);
			free(cur);
		}
	}
}

static uint32_t djb_hash(const char *key)
{
	uint32_t i, hash = 5381;

	for (i = 0; i < strlen(key); i++)
		hash = ((hash << 5) + hash) + key[i];

	return hash;
}

static struct notarget *notargets_hlist_lookup(const char *name)
{
	uint32_t key = djb_hash(name) % NOTARGET_HSIZE;
	struct hlist_node *node;
	struct notarget *cur;

	hlist_for_each_entry(cur, node, &notargets[key], node) {
		if (!strcmp(name, cur->name))
			return cur;
	}
	return NULL;
}

static void notargets_hlist_insert(const char *name)
{
	struct notarget *cur;

	if (!name)
		return;

	cur = xtables_malloc(sizeof(*cur) + strlen(name) + 1);
	strcpy(cur->name, name);
	hlist_add_head(&cur->node, &notargets[djb_hash(name) % NOTARGET_HSIZE]);
}

void xtables_announce_chain(const char *name)
{
	if (!notargets_hlist_lookup(name))
		notargets_hlist_insert(name);
}

void xtables_init(void)
{
	/* xtables cannot be used with setuid in a safe way. */
	if (getuid() != geteuid())
		_exit(111);

	xtables_libdir = getenv("XTABLES_LIBDIR");
	if (xtables_libdir != NULL)
		return;
	xtables_libdir = getenv("IPTABLES_LIB_DIR");
	if (xtables_libdir != NULL) {
		fprintf(stderr, "IPTABLES_LIB_DIR is deprecated, "
		        "use XTABLES_LIBDIR.\n");
		return;
	}
	/*
	 * Well yes, IP6TABLES_LIB_DIR is of lower priority over
	 * IPTABLES_LIB_DIR since this moved to libxtables; I think that is ok
	 * for these env vars are deprecated anyhow, and in light of the
	 * (shared) libxt_*.so files, makes less sense to have
	 * IPTABLES_LIB_DIR != IP6TABLES_LIB_DIR.
	 */
	xtables_libdir = getenv("IP6TABLES_LIB_DIR");
	if (xtables_libdir != NULL) {
		fprintf(stderr, "IP6TABLES_LIB_DIR is deprecated, "
		        "use XTABLES_LIBDIR.\n");
		return;
	}
	xtables_libdir = XTABLES_LIBDIR;

	notargets_hlist_init();
}

void xtables_fini(void)
{
#ifndef NO_SHARED_LIBS
	dlreg_free();
#endif
	notargets_hlist_free();
}

void xtables_set_nfproto(uint8_t nfproto)
{
	switch (nfproto) {
	case NFPROTO_IPV4:
		afinfo = &afinfo_ipv4;
		break;
	case NFPROTO_IPV6:
		afinfo = &afinfo_ipv6;
		break;
	case NFPROTO_BRIDGE:
		afinfo = &afinfo_bridge;
		break;
	case NFPROTO_ARP:
		afinfo = &afinfo_arp;
		break;
	default:
		fprintf(stderr, "libxtables: unhandled NFPROTO in %s\n",
		        __func__);
	}
}

/**
 * xtables_set_params - set the global parameters used by xtables
 * @xtp:	input xtables_globals structure
 *
 * The app is expected to pass a valid xtables_globals data-filled
 * with proper values
 * @xtp cannot be NULL
 *
 * Returns -1 on failure to set and 0 on success
 */
int xtables_set_params(struct xtables_globals *xtp)
{
	if (!xtp) {
		fprintf(stderr, "%s: Illegal global params\n",__func__);
		return -1;
	}

	xt_params = xtp;

	if (!xt_params->exit_err)
		xt_params->exit_err = basic_exit_err;

	return 0;
}

int xtables_init_all(struct xtables_globals *xtp, uint8_t nfproto)
{
	xtables_init();
	xtables_set_nfproto(nfproto);
	return xtables_set_params(xtp);
}

/**
 * xtables_*alloc - wrappers that exit on failure
 */
void *xtables_calloc(size_t count, size_t size)
{
	void *p;

	if ((p = calloc(count, size)) == NULL) {
		perror("ip[6]tables: calloc failed");
		exit(1);
	}

	return p;
}

void *xtables_malloc(size_t size)
{
	void *p;

	if ((p = malloc(size)) == NULL) {
		perror("ip[6]tables: malloc failed");
		exit(1);
	}

	return p;
}

void *xtables_realloc(void *ptr, size_t size)
{
	void *p;

	if ((p = realloc(ptr, size)) == NULL) {
		perror("ip[6]tables: realloc failed");
		exit(1);
	}

	return p;
}

char *xtables_strdup(const char *s)
{
	char *dup = strdup(s);

	if (!dup) {
		perror("ip[6]tables: strdup failed");
		exit(1);
	}

	return dup;
}

static char *get_modprobe(void)
{
	int procfile;
	char *ret;
	int count;

	procfile = open(PROC_SYS_MODPROBE, O_RDONLY | O_CLOEXEC);
	if (procfile < 0)
		return NULL;

	ret = malloc(PATH_MAX);
	if (ret) {
		count = read(procfile, ret, PATH_MAX);
		if (count > 0 && count < PATH_MAX)
		{
			if (ret[count - 1] == '\n')
				ret[count - 1] = '\0';
			else
				ret[count] = '\0';
			close(procfile);
			return ret;
		}
	}
	free(ret);
	close(procfile);
	return NULL;
}

int xtables_insmod(const char *modname, const char *modprobe, bool quiet)
{
	char *buf = NULL;
	char *argv[4];
	int status;
	pid_t pid;

	/* If they don't explicitly set it, read out of kernel */
	if (!modprobe) {
		buf = get_modprobe();
		if (!buf)
			return -1;
		modprobe = buf;
	}

	argv[0] = (char *)modprobe;
	argv[1] = (char *)modname;
	argv[2] = quiet ? "-q" : NULL;
	argv[3] = NULL;

	/*
	 * Need to flush the buffer, or the child may output it again
	 * when switching the program thru execv.
	 */
	fflush(stdout);

	if (posix_spawn(&pid, argv[0], NULL, NULL, argv, NULL)) {
		free(buf);
		return -1;
	} else {
		waitpid(pid, &status, 0);
	}

	free(buf);
	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return 0;
	return -1;
}

/* return true if a given file exists within procfs */
static bool proc_file_exists(const char *filename)
{
	struct stat s;
	struct statfs f;

	if (lstat(filename, &s))
		return false;
	if (!S_ISREG(s.st_mode))
		return false;
	if (statfs(filename, &f))
		return false;
	if (f.f_type != PROC_SUPER_MAGIC)
		return false;
	return true;
}

int xtables_load_ko(const char *modprobe, bool quiet)
{
	static bool loaded = false;
	int ret;

	if (loaded)
		return 0;

	if (proc_file_exists(afinfo->proc_exists)) {
		loaded = true;
		return 0;
	};

	ret = xtables_insmod(afinfo->kmod, modprobe, quiet);
	if (ret == 0)
		loaded = true;

	return ret;
}

/**
 * xtables_strtoul_base - string to number conversion
 * @s:	input string
 * @end:	like strtoul's "end" pointer
 * @value:	pointer for result
 * @min:	minimum accepted value
 * @max:	maximum accepted value
 * @base:	assumed base of value
 *
 * If @end is NULL, we assume the caller wants a "strict strtoul", and hence
 * "15a" is rejected.
 * In either case, the value obtained is compared for min-max compliance.
 *
 * Returns true/false whether number was accepted. On failure, *value has
 * undefined contents.
 */
bool xtables_strtoul_base(const char *s, char **end, uintmax_t *value,
			  uintmax_t min, uintmax_t max, unsigned int base)
{
	uintmax_t v;
	const char *p;
	char *my_end;

	errno = 0;
	/* Since strtoul allows leading minus, we have to check for ourself. */
	for (p = s; isspace(*p); ++p)
		;
	if (*p == '-')
		return false;
	v = strtoumax(s, &my_end, base);
	if (my_end == s)
		return false;
	if (end != NULL)
		*end = my_end;

	if (errno != ERANGE && min <= v && (max == 0 || v <= max)) {
		if (value != NULL)
			*value = v;
		if (end == NULL)
			return *my_end == '\0';
		return true;
	}

	return false;
}

bool xtables_strtoul(const char *s, char **end, uintmax_t *value,
		     uintmax_t min, uintmax_t max)
{
	return xtables_strtoul_base(s, end, value, min, max, 0);
}

bool xtables_strtoui(const char *s, char **end, unsigned int *value,
                     unsigned int min, unsigned int max)
{
	uintmax_t v;
	bool ret;

	ret = xtables_strtoul(s, end, &v, min, max);
	if (ret && value != NULL)
		*value = v;
	return ret;
}

int xtables_service_to_port(const char *name, const char *proto)
{
	struct servent *service;

	if ((service = getservbyname(name, proto)) != NULL)
		return ntohs((unsigned short) service->s_port);

	return -1;
}

uint16_t xtables_parse_port(const char *port, const char *proto)
{
	unsigned int portnum;

	if (xtables_strtoui(port, NULL, &portnum, 0, UINT16_MAX) ||
	    (portnum = xtables_service_to_port(port, proto)) != (unsigned)-1)
		return portnum;

	xt_params->exit_err(PARAMETER_PROBLEM,
		   "invalid port/service `%s' specified", port);
}

void xtables_parse_interface(const char *arg, char *vianame,
			     unsigned char *mask)
{
	unsigned int vialen = strlen(arg);
	unsigned int i;

	memset(mask, 0, IFNAMSIZ);
	memset(vianame, 0, IFNAMSIZ);

	if (vialen + 1 > IFNAMSIZ)
		xt_params->exit_err(PARAMETER_PROBLEM,
			   "interface name `%s' must be shorter than IFNAMSIZ"
			   " (%i)", arg, IFNAMSIZ-1);

	strcpy(vianame, arg);
	if (vialen == 0)
		return;
	else if (vianame[vialen - 1] == '+') {
		memset(mask, 0xFF, vialen - 1);
		/* Don't remove `+' here! -HW */
	} else {
		/* Include nul-terminator in match */
		memset(mask, 0xFF, vialen + 1);
	}

	/* Display warning on invalid characters */
	for (i = 0; vianame[i]; i++) {
		if (vianame[i] == '/' || vianame[i] == ' ') {
			fprintf(stderr,	"Warning: weird character in interface"
				" `%s' ('/' and ' ' are not allowed by the kernel).\n",
				vianame);
			break;
		}
	}
}

#ifndef NO_SHARED_LIBS
static void *load_extension(const char *search_path, const char *af_prefix,
    const char *name, bool is_target)
{
	const char *all_prefixes[] = {af_prefix, "libxt_", NULL};
	const char **prefix;
	const char *dir = search_path, *next;
	void *ptr = NULL;
	struct stat sb;
	char path[256];

	do {
		next = strchr(dir, ':');
		if (next == NULL)
			next = dir + strlen(dir);

		for (prefix = all_prefixes; *prefix != NULL; ++prefix) {
			void *handle;

			snprintf(path, sizeof(path), "%.*s/%s%s.so",
			         (unsigned int)(next - dir), dir,
			         *prefix, name);

			if (stat(path, &sb) != 0) {
				if (errno == ENOENT)
					continue;
				fprintf(stderr, "%s: %s\n", path,
					strerror(errno));
				return NULL;
			}
			handle = dlopen(path, RTLD_NOW);
			if (handle == NULL) {
				fprintf(stderr, "%s: %s\n", path, dlerror());
				break;
			}

			dlreg_add(handle);

			if (is_target)
				ptr = xtables_find_target(name, XTF_DONT_LOAD);
			else
				ptr = xtables_find_match(name,
				      XTF_DONT_LOAD, NULL);

			if (ptr != NULL)
				return ptr;

			errno = ENOENT;
			return NULL;
		}
		dir = next + 1;
	} while (*next != '\0');

	return NULL;
}
#endif

static bool extension_cmp(const char *name1, const char *name2, uint32_t family)
{
	if (strcmp(name1, name2) == 0 &&
	    (family == afinfo->family ||
	     family == NFPROTO_UNSPEC))
		return true;

	return false;
}

struct xtables_match *
xtables_find_match(const char *name, enum xtables_tryload tryload,
		   struct xtables_rule_match **matches)
{
	struct xtables_match *prev = NULL;
	struct xtables_match **dptr;
	struct xtables_match *ptr;
	const char *icmp6 = "icmp6";
	bool found = false;
	bool seen = false;

	if (strlen(name) >= XT_EXTENSION_MAXNAMELEN)
		xtables_error(PARAMETER_PROBLEM,
			   "Invalid match name \"%s\" (%u chars max)",
			   name, XT_EXTENSION_MAXNAMELEN - 1);

	/* This is ugly as hell. Nonetheless, there is no way of changing
	 * this without hurting backwards compatibility */
	if ( (strcmp(name,"icmpv6") == 0) ||
	     (strcmp(name,"ipv6-icmp") == 0) ||
	     (strcmp(name,"icmp6") == 0) )
		name = icmp6;

	/* Trigger delayed initialization.
	 *
	 * Register every revision the kernel accepts, not only the preferred
	 * (highest) one. Rules injected by other libiptc users - miniupnpd, for
	 * instance, emits DNAT at revision 0 - can only be rendered when their
	 * exact revision is on the list for xtables_find_match_revision() to find;
	 * otherwise listing degrades to "[unsupported revision]". Preference
	 * order is preserved, so parsing still picks the highest revision.
	 */
	for (dptr = &xtables_pending_matches; *dptr; ) {
		if (extension_cmp(name, (*dptr)->name, (*dptr)->family)) {
			ptr = *dptr;
			*dptr = (*dptr)->next;
			seen = true;
			/* Probe exactly as upstream did until one revision
			 * registers; only the extra probes this loop added are
			 * silenced, since a rejection there is expected and
			 * must not reach the legacy revision-0 fallback. */
			quiet_revision_probe = found;
			if (xtables_fully_register_pending_match(ptr, prev)) {
				found = true;
				prev = ptr;
				continue;
			} else if (prev) {
				/* A revision the kernel refuses is of no use;
				 * drop it so later lookups do not probe it
				 * again. */
				continue;
			}
			*dptr = ptr;
		}
		dptr = &((*dptr)->next);
	}
	quiet_revision_probe = false;

	if (seen && !found)
		fprintf(stderr,
			"Warning: Extension %s is not supported, missing kernel module?\n",
			name);

	for (ptr = xtables_matches; ptr; ptr = ptr->next) {
		if (extension_cmp(name, ptr->name, ptr->family)) {
			struct xtables_match *clone;

			/* First match of this type: */
			if (ptr->m == NULL)
				break;

			/* Second and subsequent clones */
			clone = xtables_malloc(sizeof(struct xtables_match));
			memcpy(clone, ptr, sizeof(struct xtables_match));
			clone->udata = NULL;
			clone->mflags = 0;
			/* This is a clone: */
			clone->next = clone;

			ptr = clone;
			break;
		}
	}

#ifndef NO_SHARED_LIBS
	if (!ptr && tryload != XTF_DONT_LOAD && tryload != XTF_DURING_LOAD) {
		ptr = load_extension(xtables_libdir, afinfo->libprefix,
		      name, false);

		if (ptr == NULL && tryload == XTF_LOAD_MUST_SUCCEED)
			xt_params->exit_err(PARAMETER_PROBLEM,
				   "Couldn't load match `%s':%s\n",
				   name, strerror(errno));
	}
#else
	if (ptr && !ptr->loaded) {
		if (tryload != XTF_DONT_LOAD)
			ptr->loaded = 1;
		else
			ptr = NULL;
	}
	if(!ptr && (tryload == XTF_LOAD_MUST_SUCCEED)) {
		xt_params->exit_err(PARAMETER_PROBLEM,
			   "Couldn't find match `%s'\n", name);
	}
#endif

	if (ptr && matches) {
		struct xtables_rule_match **i;
		struct xtables_rule_match *newentry;

		newentry = xtables_malloc(sizeof(struct xtables_rule_match));

		for (i = matches; *i; i = &(*i)->next) {
			if (extension_cmp(name, (*i)->match->name,
					  (*i)->match->family))
				(*i)->completed = true;
		}
		newentry->match = ptr;
		newentry->completed = false;
		newentry->next = NULL;
		*i = newentry;
	}

	return ptr;
}

struct xtables_match *
xtables_find_match_revision(const char *name, enum xtables_tryload tryload,
			    struct xtables_match *match, int revision)
{
	if (!match) {
		match = xtables_find_match(name, tryload, NULL);
		if (!match)
			return NULL;
	}

	while (1) {
		if (match->revision == revision)
			return match;
		match = match->next;
		if (!match)
			return NULL;
		if (!extension_cmp(name, match->name, match->family))
			return NULL;
	}
}

struct xtables_target *
xtables_find_target(const char *name, enum xtables_tryload tryload)
{
	struct xtables_target *prev = NULL;
	struct xtables_target **dptr;
	struct xtables_target *ptr;
	bool found = false;
	bool seen = false;

	/* Standard target? */
	if (strcmp(name, "") == 0
	    || strcmp(name, XTC_LABEL_ACCEPT) == 0
	    || strcmp(name, XTC_LABEL_DROP) == 0
	    || strcmp(name, XTC_LABEL_QUEUE) == 0
	    || strcmp(name, XTC_LABEL_RETURN) == 0)
		name = "standard";
	/* known non-target? */
	else if (notargets_hlist_lookup(name) &&
		 tryload != XTF_LOAD_MUST_SUCCEED)
		return NULL;

	/* Trigger delayed initialization.
	 *
	 * Register every revision the kernel accepts, not only the preferred
	 * (highest) one. Rules injected by other libiptc users - miniupnpd, for
	 * instance, emits DNAT at revision 0 - can only be rendered when their
	 * exact revision is on the list for xtables_find_target_revision() to find;
	 * otherwise listing degrades to "[unsupported revision]". Preference
	 * order is preserved, so parsing still picks the highest revision.
	 */
	for (dptr = &xtables_pending_targets; *dptr; ) {
		if (extension_cmp(name, (*dptr)->name, (*dptr)->family)) {
			ptr = *dptr;
			*dptr = (*dptr)->next;
			seen = true;
			/* Probe exactly as upstream did until one revision
			 * registers; only the extra probes this loop added are
			 * silenced, since a rejection there is expected and
			 * must not reach the legacy revision-0 fallback. */
			quiet_revision_probe = found;
			if (xtables_fully_register_pending_target(ptr, prev)) {
				found = true;
				prev = ptr;
				continue;
			} else if (prev) {
				/* A revision the kernel refuses is of no use;
				 * drop it so later lookups do not probe it
				 * again. */
				continue;
			}
			*dptr = ptr;
		}
		dptr = &((*dptr)->next);
	}
	quiet_revision_probe = false;

	if (seen && !found)
		fprintf(stderr,
			"Warning: Extension %s is not supported, missing kernel module?\n",
			name);

	for (ptr = xtables_targets; ptr; ptr = ptr->next) {
		if (extension_cmp(name, ptr->name, ptr->family)) {
			struct xtables_target *clone;

			/* First target of this type: */
			if (ptr->t == NULL)
				break;

			/* Second and subsequent clones */
			clone = xtables_malloc(sizeof(struct xtables_target));
			memcpy(clone, ptr, sizeof(struct xtables_target));
			clone->udata = NULL;
			clone->tflags = 0;
			/* This is a clone: */
			clone->next = clone;

			ptr = clone;
			break;
		}
	}

#ifndef NO_SHARED_LIBS
	if (!ptr && tryload != XTF_DONT_LOAD && tryload != XTF_DURING_LOAD) {
		ptr = load_extension(xtables_libdir, afinfo->libprefix,
		      name, true);

		if (ptr == NULL && tryload == XTF_LOAD_MUST_SUCCEED)
			xt_params->exit_err(PARAMETER_PROBLEM,
				   "Couldn't load target `%s':%s\n",
				   name, strerror(errno));
	}
#else
	if (ptr && !ptr->loaded) {
		if (tryload != XTF_DONT_LOAD)
			ptr->loaded = 1;
		else
			ptr = NULL;
	}
	if (ptr == NULL && tryload == XTF_LOAD_MUST_SUCCEED) {
		xt_params->exit_err(PARAMETER_PROBLEM,
			   "Couldn't find target `%s'\n", name);
	}
#endif

	if (ptr)
		ptr->used = 1;
	else
		notargets_hlist_insert(name);

	return ptr;
}

struct xtables_target *
xtables_find_target_revision(const char *name, enum xtables_tryload tryload,
			     struct xtables_target *target, int revision)
{
	if (!target) {
		target = xtables_find_target(name, tryload);
		if (!target)
			return NULL;
	}

	while (1) {
		if (target->revision == revision)
			return target;
		target = target->next;
		if (!target)
			return NULL;
		if (!extension_cmp(name, target->name, target->family))
			return NULL;
	}
}

int xtables_compatible_revision(const char *name, uint8_t revision, int opt)
{
	struct xt_get_revision rev;
	socklen_t s = sizeof(rev);
	int max_rev, sockfd;

	sockfd = socket(afinfo->family, SOCK_RAW | SOCK_CLOEXEC, IPPROTO_RAW);
	if (sockfd < 0) {
		if (errno == EPERM) {
			/* revision 0 is always supported. */
			if (revision != 0 && !quiet_revision_probe)
				fprintf(stderr, "%s: Could not determine whether "
						"revision %u is supported, "
						"assuming it is.\n",
					name, revision);
			return 1;
		}
		fprintf(stderr, "Could not open socket to kernel: %s\n",
			strerror(errno));
		exit(1);
	}

	xtables_load_ko(xtables_modprobe_program, true);

	strncpy(rev.name, name, XT_EXTENSION_MAXNAMELEN - 1);
	rev.name[XT_EXTENSION_MAXNAMELEN - 1] = '\0';
	rev.revision = revision;

	max_rev = getsockopt(sockfd, afinfo->ipproto, opt, &rev, &s);
	if (max_rev < 0) {
		/* Definitely don't support this? */
		if (errno == ENOENT || errno == EPROTONOSUPPORT) {
			close(sockfd);
			if (quiet_revision_probe)
				return 0;
			/* Pretend revision 0 support for better error messaging */
			if (revision == 0)
				fprintf(stderr,
					"Warning: Extension %s revision 0 not supported, missing kernel module?\n",
					name);
			return (revision == 0);
		} else if (errno == ENOPROTOOPT) {
			close(sockfd);
			/* Assume only revision 0 support (old kernel) */
			return (revision == 0);
		} else {
			fprintf(stderr, "getsockopt failed strangely: %s\n",
				strerror(errno));
			exit(1);
		}
	}
	close(sockfd);
	return 1;
}


static int compatible_match_revision(const char *name, uint8_t revision)
{
	return xt_params->compat_rev(name, revision, afinfo->so_rev_match);
}

static int compatible_target_revision(const char *name, uint8_t revision)
{
	return xt_params->compat_rev(name, revision, afinfo->so_rev_target);
}

static void xtables_check_options(const char *name, const struct option *opt)
{
	for (; opt->name != NULL; ++opt)
		if (opt->val < 0 || opt->val >= XT_OPTION_OFFSET_SCALE) {
			fprintf(stderr, "%s: Extension %s uses invalid "
			        "option value %d\n",xt_params->program_name,
			        name, opt->val);
			exit(1);
		}
}

static int xtables_match_prefer(const struct xtables_match *a,
				const struct xtables_match *b);

void xtables_register_match(struct xtables_match *me)
{
	struct xtables_match **pos;
	bool seen_myself = false;

	if (me->next) {
		fprintf(stderr, "%s: match \"%s\" already registered\n",
			xt_params->program_name, me->name);
		exit(1);
	}

	if (me->version == NULL) {
		fprintf(stderr, "%s: match %s<%u> is missing a version\n",
		        xt_params->program_name, me->name, me->revision);
		exit(1);
	}

	if (me->size != XT_ALIGN(me->size)) {
		fprintf(stderr, "%s: match \"%s\" has invalid size %u.\n",
		        xt_params->program_name, me->name,
		        (unsigned int)me->size);
		exit(1);
	}

	if (strcmp(me->version, XTABLES_VERSION) != 0) {
		fprintf(stderr, "%s: match \"%s\" has version \"%s\", "
		        "but \"%s\" is required.\n",
			xt_params->program_name, me->name,
			me->version, XTABLES_VERSION);
		exit(1);
	}

	if (strlen(me->name) >= XT_EXTENSION_MAXNAMELEN) {
		fprintf(stderr, "%s: match `%s' has invalid name\n",
			xt_params->program_name, me->name);
		exit(1);
	}

	if (me->real_name && strlen(me->real_name) >= XT_EXTENSION_MAXNAMELEN) {
		fprintf(stderr, "%s: match `%s' has invalid real name\n",
			xt_params->program_name, me->real_name);
		exit(1);
	}

	if (me->family >= NPROTO) {
		fprintf(stderr,
			"%s: BUG: match %s has invalid protocol family\n",
			xt_params->program_name, me->name);
		exit(1);
	}

	if (me->x6_options != NULL)
		xtables_option_metavalidate(me->name, me->x6_options);
	if (me->extra_opts != NULL)
		xtables_check_options(me->name, me->extra_opts);

	/* order into linked list of matches pending full registration */
	for (pos = &xtables_pending_matches; *pos; pos = &(*pos)->next) {
		/* group by name and family */
		if (strcmp(me->name, (*pos)->name) ||
		    me->family != (*pos)->family) {
			if (seen_myself)
				break; /* end of own group, append to it */
			continue;
		}
		/* found own group */
		seen_myself = true;
		if (xtables_match_prefer(me, *pos) >= 0)
			break; /* put preferred items first in group */
	}
	/* if own group was not found, prepend item */
	if (!*pos && !seen_myself)
		pos = &xtables_pending_matches;

	me->next = *pos;
	*pos = me;
#ifdef DEBUG
#define printmatch(m, sfx)						\
	printf("match %s (", (m)->name);				\
	if ((m)->real_name)						\
		printf("alias %s, ", (m)->real_name);			\
	printf("family %d, revision %d)%s", (m)->family, (m)->revision, sfx);

	{
		int i = 1;

		printf("%s: inserted ", __func__);
		printmatch(me, ":\n");
		for (pos = &xtables_pending_matches; *pos; pos = &(*pos)->next) {
			printf("pos %d:\t", i++);
			printmatch(*pos, "\n");
		}
	}
#endif
}

/**
 * Compare two actions for their preference
 * @a:	one action
 * @b: 	another
 *
 * Like strcmp, returns a negative number if @a is less preferred than @b,
 * positive number if @a is more preferred than @b, or zero if equally
 * preferred.
 */
static int
xtables_mt_prefer(bool a_alias, unsigned int a_rev, unsigned int a_fam,
		  bool b_alias, unsigned int b_rev, unsigned int b_fam)
{
	/*
	 * Alias ranks higher than no alias.
	 * (We want the new action to be used whenever possible.)
	 */
	if (!a_alias && b_alias)
		return -1;
	if (a_alias && !b_alias)
		return 1;

	/* Higher revision ranks higher. */
	if (a_rev < b_rev)
		return -1;
	if (a_rev > b_rev)
		return 1;

	/* NFPROTO_<specific> ranks higher than NFPROTO_UNSPEC. */
	if (a_fam == NFPROTO_UNSPEC && b_fam != NFPROTO_UNSPEC)
		return -1;
	if (a_fam != NFPROTO_UNSPEC && b_fam == NFPROTO_UNSPEC)
		return 1;

	/* Must be the same thing. */
	return 0;
}

static int xtables_match_prefer(const struct xtables_match *a,
				const struct xtables_match *b)
{
	return xtables_mt_prefer(a->real_name != NULL,
				 a->revision, a->family,
				 b->real_name != NULL,
				 b->revision, b->family);
}

static int xtables_target_prefer(const struct xtables_target *a,
				 const struct xtables_target *b)
{
	/*
	 * Note that if x->real_name==NULL, it will be set to x->name in
	 * xtables_register_*; the direct pointer comparison here is therefore
	 * legitimate to detect an alias.
	 */
	return xtables_mt_prefer(a->real_name != NULL,
				 a->revision, a->family,
				 b->real_name != NULL,
				 b->revision, b->family);
}

static bool xtables_fully_register_pending_match(struct xtables_match *me,
						 struct xtables_match *prev)
{
	struct xtables_match **i;
	const char *rn;

	/* See if new match can be used. */
	rn = (me->real_name != NULL) ? me->real_name : me->name;
	if (!compatible_match_revision(rn, me->revision))
		return false;

	if (!prev) {
		/* Append to list. */
		for (i = &xtables_matches; *i; i = &(*i)->next);
	} else {
		/* Append it */
		i = &prev->next;
		prev = prev->next;
	}

	me->next = prev;
	*i = me;

	me->m = NULL;
	me->mflags = 0;

	return true;
}

void xtables_register_matches(struct xtables_match *match, unsigned int n)
{
	int i;

	for (i = 0; i < n; i++)
		xtables_register_match(&match[i]);
}

void xtables_register_target(struct xtables_target *me)
{
	struct xtables_target **pos;
	bool seen_myself = false;

	if (me->next) {
		fprintf(stderr, "%s: target \"%s\" already registered\n",
			xt_params->program_name, me->name);
		exit(1);
	}

	if (me->version == NULL) {
		fprintf(stderr, "%s: target %s<%u> is missing a version\n",
		        xt_params->program_name, me->name, me->revision);
		exit(1);
	}

	if (me->size != XT_ALIGN(me->size)) {
		fprintf(stderr, "%s: target \"%s\" has invalid size %u.\n",
		        xt_params->program_name, me->name,
		        (unsigned int)me->size);
		exit(1);
	}

	if (strcmp(me->version, XTABLES_VERSION) != 0) {
		fprintf(stderr, "%s: target \"%s\" has version \"%s\", "
		        "but \"%s\" is required.\n",
			xt_params->program_name, me->name,
			me->version, XTABLES_VERSION);
		exit(1);
	}

	if (strlen(me->name) >= XT_EXTENSION_MAXNAMELEN) {
		fprintf(stderr, "%s: target `%s' has invalid name\n",
			xt_params->program_name, me->name);
		exit(1);
	}

	if (me->real_name && strlen(me->real_name) >= XT_EXTENSION_MAXNAMELEN) {
		fprintf(stderr, "%s: target `%s' has invalid real name\n",
			xt_params->program_name, me->real_name);
		exit(1);
	}

	if (me->family >= NPROTO) {
		fprintf(stderr,
			"%s: BUG: target %s has invalid protocol family\n",
			xt_params->program_name, me->name);
		exit(1);
	}

	if (me->x6_options != NULL)
		xtables_option_metavalidate(me->name, me->x6_options);
	if (me->extra_opts != NULL)
		xtables_check_options(me->name, me->extra_opts);

	/* ignore not interested target */
	if (me->family != afinfo->family && me->family != AF_UNSPEC)
		return;

	/* order into linked list of targets pending full registration */
	for (pos = &xtables_pending_targets; *pos; pos = &(*pos)->next) {
		/* group by name */
		if (!extension_cmp(me->name, (*pos)->name, (*pos)->family)) {
			if (seen_myself)
				break; /* end of own group, append to it */
			continue;
		}
		/* found own group */
		seen_myself = true;
		if (xtables_target_prefer(me, *pos) >= 0)
			break; /* put preferred items first in group */
	}
	/* if own group was not found, prepend item */
	if (!*pos && !seen_myself)
		pos = &xtables_pending_targets;

	me->next = *pos;
	*pos = me;
#ifdef DEBUG
	printf("%s: inserted target %s (family %d, revision %d):\n",
			__func__, me->name, me->family, me->revision);
	for (pos = &xtables_pending_targets; *pos; pos = &(*pos)->next) {
		printf("%s:\ttarget %s (family %d, revision %d)\n", __func__,
		       (*pos)->name, (*pos)->family, (*pos)->revision);
	}
#endif
}

static bool xtables_fully_register_pending_target(struct xtables_target *me,
						  struct xtables_target *prev)
{
	struct xtables_target **i;
	const char *rn;

	if (strcmp(me->name, "standard") != 0) {
		/* See if new target can be used. */
		rn = (me->real_name != NULL) ? me->real_name : me->name;
		if (!compatible_target_revision(rn, me->revision))
			return false;
	}

	if (!prev) {
		/* Prepend to list. */
		i = &xtables_targets;
		prev = xtables_targets;
	} else {
		/* Append it */
		i = &prev->next;
		prev = prev->next;
	}

	me->next = prev;
	*i = me;

	me->t = NULL;
	me->tflags = 0;

	return true;
}

void xtables_register_targets(struct xtables_target *target, unsigned int n)
{
	int i;

	for (i = 0; i < n; i++)
		xtables_register_target(&target[i]);
}

/* receives a list of xtables_rule_match, release them */
void xtables_rule_matches_free(struct xtables_rule_match **matches)
{
	struct xtables_rule_match *matchp, *tmp;

	for (matchp = *matches; matchp;) {
		tmp = matchp->next;
		if (matchp->match->m) {
			free(matchp->match->m);
			matchp->match->m = NULL;
		}
		if (matchp->match->udata_size) {
			free(matchp->match->udata);
			matchp->match->udata = NULL;
		}
		if (matchp->match == matchp->match->next) {
			free(matchp->match);
			matchp->match = NULL;
		}
		free(matchp);
		matchp = tmp;
	}

	*matches = NULL;
}

/**
 * xtables_param_act - act on condition
 * @status:	a constant from enum xtables_exittype
 *
 * %XTF_ONLY_ONCE: print error message that option may only be used once.
 * @p1:		module name (e.g. "mark")
 * @p2(...):	option in conflict (e.g. "--mark")
 * @p3(...):	condition to match on (see extensions/ for examples)
 *
 * %XTF_NO_INVERT: option does not support inversion
 * @p1:		module name
 * @p2:		option in conflict
 * @p3:		condition to match on
 *
 * %XTF_BAD_VALUE: bad value for option
 * @p1:		module name
 * @p2:		option with which the problem occurred (e.g. "--mark")
 * @p3:		string the user passed in (e.g. "99999999999999")
 *
 * %XTF_ONE_ACTION: two mutually exclusive actions have been specified
 * @p1:		module name
 *
 * Displays an error message and exits the program.
 */
void xtables_param_act(unsigned int status, const char *p1, ...)
{
	const char *p2, *p3;
	va_list args;
	bool b;

	va_start(args, p1);

	switch (status) {
	case XTF_ONLY_ONCE:
		p2 = va_arg(args, const char *);
		b  = va_arg(args, unsigned int);
		if (!b) {
			va_end(args);
			return;
		}
		xt_params->exit_err(PARAMETER_PROBLEM,
		           "%s: \"%s\" option may only be specified once",
		           p1, p2);
		break;
	case XTF_NO_INVERT:
		p2 = va_arg(args, const char *);
		b  = va_arg(args, unsigned int);
		if (!b) {
			va_end(args);
			return;
		}
		xt_params->exit_err(PARAMETER_PROBLEM,
		           "%s: \"%s\" option cannot be inverted", p1, p2);
		break;
	case XTF_BAD_VALUE:
		p2 = va_arg(args, const char *);
		p3 = va_arg(args, const char *);
		xt_params->exit_err(PARAMETER_PROBLEM,
		           "%s: Bad value for \"%s\" option: \"%s\"",
		           p1, p2, p3);
		break;
	case XTF_ONE_ACTION:
		b = va_arg(args, unsigned int);
		if (!b) {
			va_end(args);
			return;
		}
		xt_params->exit_err(PARAMETER_PROBLEM,
		           "%s: At most one action is possible", p1);
		break;
	default:
		xt_params->exit_err(status, p1, args);
		break;
	}

	va_end(args);
}

const char *xtables_ipaddr_to_numeric(const struct in_addr *addrp)
{
	static char buf[INET_ADDRSTRLEN];

	return inet_ntop(AF_INET, addrp, buf, sizeof(buf));
}

static const char *ipaddr_to_host(const struct in_addr *addr)
{
	static char hostname[NI_MAXHOST];
	struct sockaddr_in saddr = {
		.sin_family = AF_INET,
		.sin_addr = *addr,
	};
	int err;


	err = getnameinfo((const void *)&saddr, sizeof(struct sockaddr_in),
		       hostname, sizeof(hostname) - 1, NULL, 0, 0);
	if (err != 0)
		return NULL;

	return hostname;
}

static const char *ipaddr_to_network(const struct in_addr *addr)
{
	struct netent *net;

	if ((net = getnetbyaddr(ntohl(addr->s_addr), AF_INET)) != NULL)
		return net->n_name;

	return NULL;
}

const char *xtables_ipaddr_to_anyname(const struct in_addr *addr)
{
	const char *name;

	if ((name = ipaddr_to_host(addr)) != NULL ||
	    (name = ipaddr_to_network(addr)) != NULL)
		return name;

	return xtables_ipaddr_to_numeric(addr);
}

int xtables_ipmask_to_cidr(const struct in_addr *mask)
{
	uint32_t maskaddr, bits;
	int i;

	maskaddr = ntohl(mask->s_addr);

	for (i = 32, bits = (uint32_t)-1; i >= 0; i--, bits <<= 1) {
		if (bits == maskaddr)
			return i;
	}

	/* this mask cannot be converted to CIDR notation */
	return -1;
}

const char *xtables_ipmask_to_numeric(const struct in_addr *mask)
{
	static char buf[INET_ADDRSTRLEN + 1];
	uint32_t cidr;

	cidr = xtables_ipmask_to_cidr(mask);
	if (cidr == (unsigned int)-1) {
		/* mask was not a decent combination of 1's and 0's */
		buf[0] = '/';
		inet_ntop(AF_INET, mask, buf + 1, sizeof(buf) - 1);
		return buf;
	} else if (cidr == 32) {
		/* we don't want to see "/32" */
		return "";
	}

	sprintf(buf, "/%d", cidr);
	return buf;
}

static struct in_addr *__numeric_to_ipaddr(const char *dotted, bool is_mask)
{
	static struct in_addr addr;
	unsigned char *addrp;
	unsigned int onebyte;
	char buf[20], *p, *q;
	int i;

	/* copy dotted string, because we need to modify it */
	strncpy(buf, dotted, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';
	addrp = (void *)&addr.s_addr;

	p = buf;
	for (i = 0; i < 3; ++i) {
		if ((q = strchr(p, '.')) == NULL) {
			if (is_mask)
				return NULL;

			/* autocomplete, this is a network address */
			if (!xtables_strtoui(p, NULL, &onebyte, 0, UINT8_MAX))
				return NULL;

			addrp[i] = onebyte;
			while (i < 3)
				addrp[++i] = 0;

			return &addr;
		}

		*q = '\0';
		if (!xtables_strtoui(p, NULL, &onebyte, 0, UINT8_MAX))
			return NULL;

		addrp[i] = onebyte;
		p = q + 1;
	}

	/* we have checked 3 bytes, now we check the last one */
	if (!xtables_strtoui(p, NULL, &onebyte, 0, UINT8_MAX))
		return NULL;

	addrp[3] = onebyte;
	return &addr;
}

struct in_addr *xtables_numeric_to_ipaddr(const char *dotted)
{
	return __numeric_to_ipaddr(dotted, false);
}

struct in_addr *xtables_numeric_to_ipmask(const char *dotted)
{
	return __numeric_to_ipaddr(dotted, true);
}

static struct in_addr *network_to_ipaddr(const char *name)
{
	static struct in_addr addr;
	struct netent *net;

	if ((net = getnetbyname(name)) != NULL) {
		if (net->n_addrtype != AF_INET)
			return NULL;
		addr.s_addr = htonl(net->n_net);
		return &addr;
	}

	return NULL;
}

static struct in_addr *host_to_ipaddr(const char *name, unsigned int *naddr)
{
	struct in_addr *addr;
	struct addrinfo hints;
	struct addrinfo *res, *p;
	int err;
	unsigned int i;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family   = AF_INET;
	hints.ai_socktype = SOCK_RAW;

	*naddr = 0;
	err = getaddrinfo(name, NULL, &hints, &res);
	if (err != 0)
		return NULL;
	for (p = res; p != NULL; p = p->ai_next)
		++*naddr;
	addr = xtables_calloc(*naddr, sizeof(struct in_addr));
	for (i = 0, p = res; p != NULL; p = p->ai_next)
		memcpy(&addr[i++],
		       &((const struct sockaddr_in *)p->ai_addr)->sin_addr,
		       sizeof(struct in_addr));
	freeaddrinfo(res);
	return addr;
}

static struct in_addr *
ipparse_hostnetwork(const char *name, unsigned int *naddrs)
{
	struct in_addr *addrptmp, *addrp;

	if ((addrptmp = xtables_numeric_to_ipaddr(name)) != NULL ||
	    (addrptmp = network_to_ipaddr(name)) != NULL) {
		addrp = xtables_malloc(sizeof(struct in_addr));
		memcpy(addrp, addrptmp, sizeof(*addrp));
		*naddrs = 1;
		return addrp;
	}
	if ((addrptmp = host_to_ipaddr(name, naddrs)) != NULL)
		return addrptmp;

	xt_params->exit_err(PARAMETER_PROBLEM, "host/network `%s' not found", name);
}

static struct in_addr *parse_ipmask(const char *mask)
{
	static struct in_addr maskaddr;
	struct in_addr *addrp;
	unsigned int bits;

	if (mask == NULL) {
		/* no mask at all defaults to 32 bits */
		maskaddr.s_addr = 0xFFFFFFFF;
		return &maskaddr;
	}
	if ((addrp = xtables_numeric_to_ipmask(mask)) != NULL)
		/* dotted_to_addr already returns a network byte order addr */
		return addrp;
	if (!xtables_strtoui(mask, NULL, &bits, 0, 32))
		xt_params->exit_err(PARAMETER_PROBLEM,
			   "invalid mask `%s' specified", mask);
	if (bits != 0) {
		maskaddr.s_addr = htonl(0xFFFFFFFF << (32 - bits));
		return &maskaddr;
	}

	maskaddr.s_addr = 0U;
	return &maskaddr;
}

void xtables_ipparse_multiple(const char *name, struct in_addr **addrpp,
                              struct in_addr **maskpp, unsigned int *naddrs)
{
	struct in_addr *addrp;
	char buf[256], *p, *next;
	unsigned int len, i, j, n, count = 1;
	const char *loop = name;

	while ((loop = strchr(loop, ',')) != NULL) {
		++count;
		++loop; /* skip ',' */
	}

	*addrpp = xtables_malloc(sizeof(struct in_addr) * count);
	*maskpp = xtables_malloc(sizeof(struct in_addr) * count);

	loop = name;

	for (i = 0; i < count; ++i) {
		while (isspace(*loop))
			++loop;
		next = strchr(loop, ',');
		if (next != NULL)
			len = next - loop;
		else
			len = strlen(loop);
		if (len > sizeof(buf) - 1)
			xt_params->exit_err(PARAMETER_PROBLEM,
				"Hostname too long");

		strncpy(buf, loop, len);
		buf[len] = '\0';
		if ((p = strrchr(buf, '/')) != NULL) {
			*p = '\0';
			addrp = parse_ipmask(p + 1);
		} else {
			addrp = parse_ipmask(NULL);
		}
		memcpy(*maskpp + i, addrp, sizeof(*addrp));

		/* if a null mask is given, the name is ignored, like in "any/0" */
		if ((*maskpp + i)->s_addr == 0)
			/*
			 * A bit pointless to process multiple addresses
			 * in this case...
			 */
			strcpy(buf, "0.0.0.0");

		addrp = ipparse_hostnetwork(buf, &n);
		if (n > 1) {
			count += n - 1;
			*addrpp = xtables_realloc(*addrpp,
			          sizeof(struct in_addr) * count);
			*maskpp = xtables_realloc(*maskpp,
			          sizeof(struct in_addr) * count);
			for (j = 0; j < n; ++j)
				/* for each new addr */
				memcpy(*addrpp + i + j, addrp + j,
				       sizeof(*addrp));
			for (j = 1; j < n; ++j)
				/* for each new mask */
				memcpy(*maskpp + i + j, *maskpp + i,
				       sizeof(*addrp));
			i += n - 1;
		} else {
			memcpy(*addrpp + i, addrp, sizeof(*addrp));
		}
		/* free what ipparse_hostnetwork had allocated: */
		free(addrp);
		if (next == NULL)
			break;
		loop = next + 1;
	}
	*naddrs = count;
	for (i = 0; i < count; ++i)
		(*addrpp+i)->s_addr &= (*maskpp+i)->s_addr;
}


/**
 * xtables_ipparse_any - transform arbitrary name to in_addr
 *
 * Possible inputs (pseudo regex):
 * 	m{^($hostname|$networkname|$ipaddr)(/$mask)?}
 * "1.2.3.4/5", "1.2.3.4", "hostname", "networkname"
 */
void xtables_ipparse_any(const char *name, struct in_addr **addrpp,
                         struct in_addr *maskp, unsigned int *naddrs)
{
	unsigned int i, j, k, n;
	struct in_addr *addrp;
	char buf[256], *p;

	strncpy(buf, name, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';
	if ((p = strrchr(buf, '/')) != NULL) {
		*p = '\0';
		addrp = parse_ipmask(p + 1);
	} else {
		addrp = parse_ipmask(NULL);
	}
	memcpy(maskp, addrp, sizeof(*maskp));

	/* if a null mask is given, the name is ignored, like in "any/0" */
	if (maskp->s_addr == 0U)
		strcpy(buf, "0.0.0.0");

	addrp = *addrpp = ipparse_hostnetwork(buf, naddrs);
	n = *naddrs;
	for (i = 0, j = 0; i < n; ++i) {
		addrp[j++].s_addr &= maskp->s_addr;
		for (k = 0; k < j - 1; ++k)
			if (addrp[k].s_addr == addrp[j-1].s_addr) {
				/*
				 * Nuke the dup by copying an address from the
				 * tail here, and check the current position
				 * again (--j).
				 */
				memcpy(&addrp[--j], &addrp[--*naddrs],
				       sizeof(struct in_addr));
				break;
			}
	}
}

const char *xtables_ip6addr_to_numeric(const struct in6_addr *addrp)
{
	static char buf[INET6_ADDRSTRLEN];

	return inet_ntop(AF_INET6, addrp, buf, sizeof(buf));
}

static const char *ip6addr_to_host(const struct in6_addr *addr)
{
	static char hostname[NI_MAXHOST];
	struct sockaddr_in6 saddr;
	int err;

	memset(&saddr, 0, sizeof(struct sockaddr_in6));
	memcpy(&saddr.sin6_addr, addr, sizeof(*addr));
	saddr.sin6_family = AF_INET6;

	err = getnameinfo((const void *)&saddr, sizeof(struct sockaddr_in6),
			hostname, sizeof(hostname) - 1, NULL, 0, 0);
	if (err != 0)
		return NULL;

	return hostname;
}

const char *xtables_ip6addr_to_anyname(const struct in6_addr *addr)
{
	const char *name;

	if ((name = ip6addr_to_host(addr)) != NULL)
		return name;

	return xtables_ip6addr_to_numeric(addr);
}

int xtables_ip6mask_to_cidr(const struct in6_addr *k)
{
	unsigned int bits = 0;
	uint32_t a, b, c, d;

	a = ntohl(k->s6_addr32[0]);
	b = ntohl(k->s6_addr32[1]);
	c = ntohl(k->s6_addr32[2]);
	d = ntohl(k->s6_addr32[3]);
	while (a & 0x80000000U) {
		++bits;
		a <<= 1;
		a  |= (b >> 31) & 1;
		b <<= 1;
		b  |= (c >> 31) & 1;
		c <<= 1;
		c  |= (d >> 31) & 1;
		d <<= 1;
	}
	if (a != 0 || b != 0 || c != 0 || d != 0)
		return -1;
	return bits;
}

const char *xtables_ip6mask_to_numeric(const struct in6_addr *addrp)
{
	static char buf[INET6_ADDRSTRLEN + 1];
	int l = xtables_ip6mask_to_cidr(addrp);

	if (l == -1) {
		strcpy(buf, "/");
		inet_ntop(AF_INET6, addrp, buf + 1, sizeof(buf) - 1);
		return buf;
	}
	/* we don't want to see "/128" */
	if (l == 128)
		return "";
	else
		sprintf(buf, "/%d", l);
	return buf;
}

struct in6_addr *xtables_numeric_to_ip6addr(const char *num)
{
	static struct in6_addr ap;

	if (inet_pton(AF_INET6, num, &ap) == 1)
		return &ap;

	return NULL;
}

static struct in6_addr *
host_to_ip6addr(const char *name, unsigned int *naddr)
{
	struct in6_addr *addr;
	struct addrinfo hints;
	struct addrinfo *res, *p;
	int err;
	unsigned int i;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family   = AF_INET6;
	hints.ai_socktype = SOCK_RAW;

	*naddr = 0;
	err = getaddrinfo(name, NULL, &hints, &res);
	if (err != 0)
		return NULL;
	/* Find length of address chain */
	for (p = res; p != NULL; p = p->ai_next)
		++*naddr;
	/* Copy each element of the address chain */
	addr = xtables_calloc(*naddr, sizeof(struct in6_addr));
	for (i = 0, p = res; p != NULL; p = p->ai_next)
		memcpy(&addr[i++],
		       &((const struct sockaddr_in6 *)p->ai_addr)->sin6_addr,
		       sizeof(struct in6_addr));
	freeaddrinfo(res);
	return addr;
}

static struct in6_addr *network_to_ip6addr(const char *name)
{
	/*	abort();*/
	/* TODO: not implemented yet, but the exception breaks the
	 *       name resolvation */
	return NULL;
}

static struct in6_addr *
ip6parse_hostnetwork(const char *name, unsigned int *naddrs)
{
	struct in6_addr *addrp, *addrptmp;

	if ((addrptmp = xtables_numeric_to_ip6addr(name)) != NULL ||
	    (addrptmp = network_to_ip6addr(name)) != NULL) {
		addrp = xtables_malloc(sizeof(struct in6_addr));
		memcpy(addrp, addrptmp, sizeof(*addrp));
		*naddrs = 1;
		return addrp;
	}
	if ((addrp = host_to_ip6addr(name, naddrs)) != NULL)
		return addrp;

	xt_params->exit_err(PARAMETER_PROBLEM, "host/network `%s' not found", name);
}

static struct in6_addr *parse_ip6mask(char *mask)
{
	static struct in6_addr maskaddr;
	struct in6_addr *addrp;
	unsigned int bits;

	if (mask == NULL) {
		/* no mask at all defaults to 128 bits */
		memset(&maskaddr, 0xff, sizeof maskaddr);
		return &maskaddr;
	}
	if ((addrp = xtables_numeric_to_ip6addr(mask)) != NULL)
		return addrp;
	if (!xtables_strtoui(mask, NULL, &bits, 0, 128))
		xt_params->exit_err(PARAMETER_PROBLEM,
			   "invalid mask `%s' specified", mask);
	if (bits != 0) {
		char *p = (void *)&maskaddr;
		memset(p, 0xff, bits / 8);
		memset(p + ((bits + 7) / 8), 0, (128 - bits) / 8);
		if (bits < 128)
			p[bits/8] = 0xff << (8 - (bits & 7));
		return &maskaddr;
	}

	memset(&maskaddr, 0, sizeof(maskaddr));
	return &maskaddr;
}

void
xtables_ip6parse_multiple(const char *name, struct in6_addr **addrpp,
		      struct in6_addr **maskpp, unsigned int *naddrs)
{
	static const struct in6_addr zero_addr;
	struct in6_addr *addrp;
	char buf[256], *p, *next;
	unsigned int len, i, j, n, count = 1;
	const char *loop = name;

	while ((loop = strchr(loop, ',')) != NULL) {
		++count;
		++loop; /* skip ',' */
	}

	*addrpp = xtables_malloc(sizeof(struct in6_addr) * count);
	*maskpp = xtables_malloc(sizeof(struct in6_addr) * count);

	loop = name;

	for (i = 0; i < count /*NB: count can grow*/; ++i) {
		while (isspace(*loop))
			++loop;
		next = strchr(loop, ',');
		if (next != NULL)
			len = next - loop;
		else
			len = strlen(loop);
		if (len > sizeof(buf) - 1)
			xt_params->exit_err(PARAMETER_PROBLEM,
				"Hostname too long");

		strncpy(buf, loop, len);
		buf[len] = '\0';
		if ((p = strrchr(buf, '/')) != NULL) {
			*p = '\0';
			addrp = parse_ip6mask(p + 1);
		} else {
			addrp = parse_ip6mask(NULL);
		}
		memcpy(*maskpp + i, addrp, sizeof(*addrp));

		/* if a null mask is given, the name is ignored, like in "any/0" */
		if (memcmp(*maskpp + i, &zero_addr, sizeof(zero_addr)) == 0)
			strcpy(buf, "::");

		addrp = ip6parse_hostnetwork(buf, &n);
		if (n > 1) {
			count += n - 1;
			*addrpp = xtables_realloc(*addrpp,
			          sizeof(struct in6_addr) * count);
			*maskpp = xtables_realloc(*maskpp,
			          sizeof(struct in6_addr) * count);
			for (j = 0; j < n; ++j)
				/* for each new addr */
				memcpy(*addrpp + i + j, addrp + j,
				       sizeof(*addrp));
			for (j = 1; j < n; ++j)
				/* for each new mask */
				memcpy(*maskpp + i + j, *maskpp + i,
				       sizeof(*addrp));
			i += n - 1;
		} else {
			memcpy(*addrpp + i, addrp, sizeof(*addrp));
		}
		/* free what ip6parse_hostnetwork had allocated: */
		free(addrp);
		if (next == NULL)
			break;
		loop = next + 1;
	}
	*naddrs = count;
	for (i = 0; i < count; ++i)
		for (j = 0; j < 4; ++j)
			(*addrpp+i)->s6_addr32[j] &= (*maskpp+i)->s6_addr32[j];
}

void xtables_ip6parse_any(const char *name, struct in6_addr **addrpp,
                          struct in6_addr *maskp, unsigned int *naddrs)
{
	static const struct in6_addr zero_addr;
	struct in6_addr *addrp;
	unsigned int i, j, k, n;
	char buf[256], *p;

	strncpy(buf, name, sizeof(buf) - 1);
	buf[sizeof(buf)-1] = '\0';
	if ((p = strrchr(buf, '/')) != NULL) {
		*p = '\0';
		addrp = parse_ip6mask(p + 1);
	} else {
		addrp = parse_ip6mask(NULL);
	}
	memcpy(maskp, addrp, sizeof(*maskp));

	/* if a null mask is given, the name is ignored, like in "any/0" */
	if (memcmp(maskp, &zero_addr, sizeof(zero_addr)) == 0)
		strcpy(buf, "::");

	addrp = *addrpp = ip6parse_hostnetwork(buf, naddrs);
	n = *naddrs;
	for (i = 0, j = 0; i < n; ++i) {
		for (k = 0; k < 4; ++k)
			addrp[j].s6_addr32[k] &= maskp->s6_addr32[k];
		++j;
		for (k = 0; k < j - 1; ++k)
			if (IN6_ARE_ADDR_EQUAL(&addrp[k], &addrp[j - 1])) {
				/*
				 * Nuke the dup by copying an address from the
				 * tail here, and check the current position
				 * again (--j).
				 */
				memcpy(&addrp[--j], &addrp[--*naddrs],
				       sizeof(struct in_addr));
				break;
			}
	}
}

void xtables_save_string(const char *value)
{
	static const char no_quote_chars[] = "_-0123456789"
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	static const char escape_chars[] = "\"\\'";
	size_t length;
	const char *p;

	length = strspn(value, no_quote_chars);
	if (length > 0 && value[length] == 0) {
		/* no quoting required */
		putchar(' ');
		fputs(value, stdout);
	} else {
		/* there is at least one dangerous character in the
		   value, which we have to quote.  Write double quotes
		   around the value and escape special characters with
		   a backslash */
		printf(" \"");

		for (p = strpbrk(value, escape_chars); p != NULL;
		     p = strpbrk(value, escape_chars)) {
			if (p > value)
				fwrite(value, 1, p - value, stdout);
			putchar('\\');
			putchar(*p);
			value = p + 1;
		}

		/* print the rest and finish the double quoted
		   string */
		fputs(value, stdout);
		putchar('\"');
	}
}

const struct xtables_pprot xtables_chain_protos[] = {
	{"tcp",       IPPROTO_TCP},
	{"sctp",      IPPROTO_SCTP},
	{"udp",       IPPROTO_UDP},
	{"udplite",   IPPROTO_UDPLITE},
	{"icmp",      IPPROTO_ICMP},
	{"ipv6-icmp", IPPROTO_ICMPV6},
	{"icmpv6",    IPPROTO_ICMPV6},
	{"esp",       IPPROTO_ESP},
	{"ah",        IPPROTO_AH},
	{"mobility-header", IPPROTO_MH},
	{"ipv6-mh",   IPPROTO_MH},
	{"mh",        IPPROTO_MH},
	{"dccp",      IPPROTO_DCCP},
	{"ipcomp",    IPPROTO_COMP},
	{"all",       0},
	{NULL},
};

uint16_t
xtables_parse_protocol(const char *s)
{
	const struct protoent *pent;
	unsigned int proto, i;

	if (xtables_strtoui(s, NULL, &proto, 0, UINT8_MAX))
		return proto;

	for (i = 0; xtables_chain_protos[i].name != NULL; ++i) {
		if (strcmp(s, xtables_chain_protos[i].name) == 0)
			return xtables_chain_protos[i].num;
	}

	pent = getprotobyname(s);
	if (pent != NULL)
		return pent->p_proto;

	xt_params->exit_err(PARAMETER_PROBLEM,
		"unknown protocol \"%s\" specified", s);
	return -1;
}

void xtables_print_num(uint64_t number, unsigned int format)
{
	if (!(format & FMT_KILOMEGAGIGA)) {
		printf(FMT("%8llu ","%llu "), (unsigned long long)number);
		return;
	}
	if (number <= 99999) {
		printf(FMT("%5llu ","%llu "), (unsigned long long)number);
		return;
	}
	number = (number + 500) / 1000;
	if (number <= 9999) {
		printf(FMT("%4lluK ","%lluK "), (unsigned long long)number);
		return;
	}
	number = (number + 500) / 1000;
	if (number <= 9999) {
		printf(FMT("%4lluM ","%lluM "), (unsigned long long)number);
		return;
	}
	number = (number + 500) / 1000;
	if (number <= 9999) {
		printf(FMT("%4lluG ","%lluG "), (unsigned long long)number);
		return;
	}
	number = (number + 500) / 1000;
	printf(FMT("%4lluT ","%lluT "), (unsigned long long)number);
}

static const unsigned char mac_type_unicast[ETH_ALEN] =   {};
static const unsigned char msk_type_unicast[ETH_ALEN] =   {1};
static const unsigned char mac_type_multicast[ETH_ALEN] = {1};
static const unsigned char msk_type_multicast[ETH_ALEN] = {1};
#define ALL_ONE_MAC {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
static const unsigned char mac_type_broadcast[ETH_ALEN] = ALL_ONE_MAC;
static const unsigned char msk_type_broadcast[ETH_ALEN] = ALL_ONE_MAC;
static const unsigned char mac_type_bridge_group[ETH_ALEN] = {0x01, 0x80, 0xc2};
static const unsigned char msk_type_bridge_group[ETH_ALEN] = ALL_ONE_MAC;
#undef ALL_ONE_MAC

int xtables_parse_mac_and_mask(const char *from, void *to, void *mask)
{
	char *p;
	int i;
	struct ether_addr *addr = NULL;

	if (strcasecmp(from, "Unicast") == 0) {
		memcpy(to, mac_type_unicast, ETH_ALEN);
		memcpy(mask, msk_type_unicast, ETH_ALEN);
		return 0;
	}
	if (strcasecmp(from, "Multicast") == 0) {
		memcpy(to, mac_type_multicast, ETH_ALEN);
		memcpy(mask, msk_type_multicast, ETH_ALEN);
		return 0;
	}
	if (strcasecmp(from, "Broadcast") == 0) {
		memcpy(to, mac_type_broadcast, ETH_ALEN);
		memcpy(mask, msk_type_broadcast, ETH_ALEN);
		return 0;
	}
	if (strcasecmp(from, "BGA") == 0) {
		memcpy(to, mac_type_bridge_group, ETH_ALEN);
		memcpy(mask, msk_type_bridge_group, ETH_ALEN);
		return 0;
	}
	if ( (p = strrchr(from, '/')) != NULL) {
		*p = '\0';
		if (!(addr = ether_aton(p + 1)))
			return -1;
		memcpy(mask, addr, ETH_ALEN);
	} else
		memset(mask, 0xff, ETH_ALEN);
	if (!(addr = ether_aton(from)))
		return -1;
	memcpy(to, addr, ETH_ALEN);
	for (i = 0; i < ETH_ALEN; i++)
		((char *)to)[i] &= ((char *)mask)[i];
	return 0;
}

int xtables_print_well_known_mac_and_mask(const void *mac, const void *mask)
{
	if (!memcmp(mac, mac_type_unicast, ETH_ALEN) &&
	    !memcmp(mask, msk_type_unicast, ETH_ALEN))
		printf("Unicast");
	else if (!memcmp(mac, mac_type_multicast, ETH_ALEN) &&
	         !memcmp(mask, msk_type_multicast, ETH_ALEN))
		printf("Multicast");
	else if (!memcmp(mac, mac_type_broadcast, ETH_ALEN) &&
	         !memcmp(mask, msk_type_broadcast, ETH_ALEN))
		printf("Broadcast");
	else if (!memcmp(mac, mac_type_bridge_group, ETH_ALEN) &&
	         !memcmp(mask, msk_type_bridge_group, ETH_ALEN))
		printf("BGA");
	else
		return -1;
	return 0;
}

void xtables_print_mac(const unsigned char *macaddress)
{
	unsigned int i;

	printf("%02x", macaddress[0]);
	for (i = 1; i < 6; ++i)
		printf(":%02x", macaddress[i]);
}

void xtables_print_mac_and_mask(const unsigned char *mac, const unsigned char *mask)
{
	static const char hlpmsk[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	xtables_print_mac(mac);

	if (memcmp(mask, hlpmsk, 6) == 0)
		return;

	printf("/");
	xtables_print_mac(mask);
}

void xtables_parse_val_mask(struct xt_option_call *cb,
			    unsigned int *val, unsigned int *mask,
			    const struct xtables_lmap *lmap)
{
	char *end;

	*mask = ~0U;

	if (!xtables_strtoui(cb->arg, &end, val, 0, UINT32_MAX)) {
		if (lmap)
			goto name2val;
		else
			goto bad_val;
	}

	if (*end == '\0')
		return;

	if (*end != '/') {
		if (lmap)
			goto name2val;
		else
			goto garbage;
	}

	if (!xtables_strtoui(end + 1, &end, mask, 0, UINT32_MAX))
		goto bad_val;

	if (*end == '\0')
		return;

garbage:
	xt_params->exit_err(PARAMETER_PROBLEM,
			"%s: trailing garbage after value "
			"for option \"--%s\".\n",
			cb->ext_name, cb->entry->name);

bad_val:
	xt_params->exit_err(PARAMETER_PROBLEM,
			"%s: bad integer value for option \"--%s\", "
			"or out of range.\n",
			cb->ext_name, cb->entry->name);

name2val:
	*val = xtables_lmap_name2id(lmap, cb->arg);
	if ((int)*val == -1)
		xt_params->exit_err(PARAMETER_PROBLEM,
			"%s: could not map name %s to an integer value "
			"for option \"--%s\".\n",
			cb->ext_name, cb->arg, cb->entry->name);
}

void xtables_print_val_mask(unsigned int val, unsigned int mask,
			    const struct xtables_lmap *lmap)
{
	if (mask != ~0U) {
		printf(" 0x%x/0x%x", val, mask);
		return;
	}

	if (lmap) {
		const char *name = xtables_lmap_id2name(lmap, val);

		if (name) {
			printf(" %s", name);
			return;
		}
	}

	printf(" 0x%x", val);
}

int kernel_version;

void get_kernel_version(void)
{
	static struct utsname uts;
	int x = 0, y = 0, z = 0;

	if (uname(&uts) == -1) {
		fprintf(stderr, "Unable to retrieve kernel version.\n");
		xtables_free_opts(1);
		exit(1);
	}

	sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	kernel_version = LINUX_VERSION(x, y, z);
}

#include <linux/netfilter/nf_tables.h>

enum xt_xlate_type {
	XT_XLATE_RULE = 0,
	XT_XLATE_SET,
	__XT_XLATE_MAX
};

struct xt_xlate {
	struct xt_xlate_buf {
		char	*data;
		int	size;
		int	rem;
		int	off;
	} buf[__XT_XLATE_MAX];
	char comment[NFT_USERDATA_MAXLEN];
	int family;
};

struct xt_xlate *xt_xlate_alloc(int size)
{
	struct xt_xlate *xl = xtables_malloc(sizeof(struct xt_xlate));
	int i;

	for (i = 0; i < __XT_XLATE_MAX; i++) {
		xl->buf[i].data = xtables_malloc(size);
		xl->buf[i].data[0] = '\0';
		xl->buf[i].size = size;
		xl->buf[i].rem = size;
		xl->buf[i].off = 0;
	}
	xl->comment[0] = '\0';

	return xl;
}

void xt_xlate_free(struct xt_xlate *xl)
{
	int i;

	for (i = 0; i < __XT_XLATE_MAX; i++)
		free(xl->buf[i].data);

	free(xl);
}

static bool isbrace(char c)
{
	switch (c) {
	case '(':
	case ')':
	case '{':
	case '}':
	case '[':
	case ']':
		return true;
	}
	return false;
}

static void __xt_xlate_add(struct xt_xlate *xl, enum xt_xlate_type type,
			   bool space, const char *fmt, va_list ap)
{
	struct xt_xlate_buf *buf = &xl->buf[type];
	char tmpbuf[1024] = "";
	int len;

	len = vsnprintf(tmpbuf, 1024, fmt, ap);
	if (len < 0 || len >= buf->rem - 1)
		xtables_error(RESOURCE_PROBLEM, "OOM");

	if (space && buf->off &&
	    !isspace(buf->data[buf->off - 1]) &&
	    (isalnum(tmpbuf[0]) || isbrace(tmpbuf[0]))) {
		buf->data[buf->off] = ' ';
		buf->off++;
		buf->rem--;
	}
	sprintf(buf->data + buf->off, "%s", tmpbuf);
	buf->rem -= len;
	buf->off += len;
}

void xt_xlate_rule_add(struct xt_xlate *xl, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	__xt_xlate_add(xl, XT_XLATE_RULE, true, fmt, ap);
	va_end(ap);
}

void xt_xlate_rule_add_nospc(struct xt_xlate *xl, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	__xt_xlate_add(xl, XT_XLATE_RULE, false, fmt, ap);
	va_end(ap);
}

void xt_xlate_set_add(struct xt_xlate *xl, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	__xt_xlate_add(xl, XT_XLATE_SET, true, fmt, ap);
	va_end(ap);
}

void xt_xlate_set_add_nospc(struct xt_xlate *xl, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	__xt_xlate_add(xl, XT_XLATE_SET, false, fmt, ap);
	va_end(ap);
}

void xt_xlate_add_comment(struct xt_xlate *xl, const char *comment)
{
	strncpy(xl->comment, comment, NFT_USERDATA_MAXLEN - 1);
	xl->comment[NFT_USERDATA_MAXLEN - 1] = '\0';
}

const char *xt_xlate_get_comment(struct xt_xlate *xl)
{
	return xl->comment[0] ? xl->comment : NULL;
}

void xl_xlate_set_family(struct xt_xlate *xl, uint8_t family)
{
	xl->family = family;
}

uint8_t xt_xlate_get_family(struct xt_xlate *xl)
{
	return xl->family;
}

const char *xt_xlate_get(struct xt_xlate *xl)
{
	struct xt_xlate_buf *buf = &xl->buf[XT_XLATE_RULE];

	while (buf->off && isspace(buf->data[buf->off - 1]))
		buf->data[--buf->off] = '\0';

	return buf->data;
}

const char *xt_xlate_set_get(struct xt_xlate *xl)
{
	return xl->buf[XT_XLATE_SET].data;
}
