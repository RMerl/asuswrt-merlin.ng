#include <config.h>
#include <ctype.h>
#include <getopt.h>
#include <errno.h>
#include <libgen.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <xtables.h>
#include <math.h>
#include <signal.h>
#include "xshared.h"

/* a few arp opcode names */
char *arp_opcodes[] =
{
	"Request",
	"Reply",
	"Request_Reverse",
	"Reply_Reverse",
	"DRARP_Request",
	"DRARP_Reply",
	"DRARP_Error",
	"InARP_Request",
	"ARP_NAK",
};

/*
 * Print out any special helps. A user might like to be able to add a --help
 * to the commandline, and see expected results. So we call help for all
 * specified matches and targets.
 */
static void print_extension_helps(const struct xtables_target *t,
				  const struct xtables_rule_match *m)
{
	for (; t != NULL; t = t->next) {
		if (t->used) {
			printf("\n");
			if (t->help == NULL)
				printf("%s does not take any options\n",
				       t->name);
			else
				t->help();
		}
	}
	for (; m != NULL; m = m->next) {
		printf("\n");
		if (m->match->help == NULL)
			printf("%s does not take any options\n",
			       m->match->name);
		else
			m->match->help();
	}
}

const char *
proto_to_name(uint16_t proto, int nolookup)
{
	unsigned int i;

	for (i = 0; xtables_chain_protos[i].name != NULL; ++i)
		if (xtables_chain_protos[i].num == proto)
			return xtables_chain_protos[i].name;

	if (proto && !nolookup) {
		struct protoent *pent = getprotobynumber(proto);
		if (pent)
			return pent->p_name;
	}

	return NULL;
}

static struct xtables_match *
find_proto(const char *pname, enum xtables_tryload tryload,
	   int nolookup, struct xtables_rule_match **matches)
{
	unsigned int proto;

	if (xtables_strtoui(pname, NULL, &proto, 0, UINT8_MAX)) {
		const char *protoname = proto_to_name(proto, nolookup);

		if (protoname)
			return xtables_find_match(protoname, tryload, matches);
	} else
		return xtables_find_match(pname, tryload, matches);

	return NULL;
}

/*
 * Some explanations (after four different bugs in 3 different releases): If
 * we encounter a parameter, that has not been parsed yet, it's not an option
 * of an explicitly loaded match or a target. However, we support implicit
 * loading of the protocol match extension. '-p tcp' means 'l4 proto 6' and at
 * the same time 'load tcp protocol match on demand if we specify --dport'.
 *
 * To make this work, we need to make sure:
 * - the parameter has not been parsed by a match (m above)
 * - a protocol has been specified
 * - the protocol extension has not been loaded yet, or is loaded and unused
 *   [think of ip6tables-restore!]
 * - the protocol extension can be successively loaded
 */
static struct xtables_match *load_proto(struct iptables_command_state *cs)
{
	if (cs->protocol == NULL)
		return NULL;
	if (cs->proto_used)
		return NULL;
	cs->proto_used = true;
	return find_proto(cs->protocol, XTF_TRY_LOAD,
			  cs->options & OPT_NUMERIC, &cs->matches);
}

int command_default(struct iptables_command_state *cs,
		    struct xtables_globals *gl, bool invert)
{
	struct xtables_rule_match *matchp;
	struct xtables_match *m;

	if (cs->target != NULL &&
	    (cs->target->parse != NULL || cs->target->x6_parse != NULL) &&
	    cs->c >= cs->target->option_offset &&
	    cs->c < cs->target->option_offset + XT_OPTION_OFFSET_SCALE) {
		xtables_option_tpcall(cs->c, cs->argv, invert,
				      cs->target, &cs->fw);
		return 0;
	}

	for (matchp = cs->matches; matchp; matchp = matchp->next) {
		m = matchp->match;

		if (matchp->completed ||
		    (m->x6_parse == NULL && m->parse == NULL))
			continue;
		if (cs->c < matchp->match->option_offset ||
		    cs->c >= matchp->match->option_offset + XT_OPTION_OFFSET_SCALE)
			continue;
		xtables_option_mpcall(cs->c, cs->argv, invert, m, &cs->fw);
		return 0;
	}

	m = load_proto(cs);
	if (m != NULL) {
		size_t size;

		size = XT_ALIGN(sizeof(struct xt_entry_match)) + m->size;

		m->m = xtables_calloc(1, size);
		m->m->u.match_size = size;
		strcpy(m->m->u.user.name, m->name);
		m->m->u.user.revision = m->revision;
		xs_init_match(m);

		if (m->x6_options != NULL)
			gl->opts = xtables_options_xfrm(gl->orig_opts,
							gl->opts,
							m->x6_options,
							&m->option_offset);
		else
			gl->opts = xtables_merge_options(gl->orig_opts,
							 gl->opts,
							 m->extra_opts,
							 &m->option_offset);
		if (gl->opts == NULL)
			xtables_error(OTHER_PROBLEM, "can't alloc memory!");
		optind--;
		/* Indicate to rerun getopt *immediately* */
 		return 1;
	}

	if (cs->c == ':')
		xtables_error(PARAMETER_PROBLEM, "option \"%s\" "
		              "requires an argument", cs->argv[optind-1]);
	if (cs->c == '?') {
		char optoptstr[3] = {'-', optopt, '\0'};

		xtables_error(PARAMETER_PROBLEM, "unknown option \"%s\"",
			      optopt ? optoptstr : cs->argv[optind - 1]);
	}
	xtables_error(PARAMETER_PROBLEM, "Unknown arg \"%s\"", optarg);
}

static mainfunc_t subcmd_get(const char *cmd, const struct subcommand *cb)
{
	for (; cb->name != NULL; ++cb)
		if (strcmp(cb->name, cmd) == 0)
			return cb->main;
	return NULL;
}

int subcmd_main(int argc, char **argv, const struct subcommand *cb)
{
	const char *cmd = basename(*argv);
	mainfunc_t f = subcmd_get(cmd, cb);

	if (f == NULL && argc > 1) {
		/*
		 * Unable to find a main method for our command name?
		 * Let's try again with the first argument!
		 */
		++argv;
		--argc;
		f = subcmd_get(*argv, cb);
	}

	/* now we should have a valid function pointer */
	if (f != NULL)
		return f(argc, argv);

	fprintf(stderr, "ERROR: No valid subcommand given.\nValid subcommands:\n");
	for (; cb->name != NULL; ++cb)
		fprintf(stderr, " * %s\n", cb->name);
	exit(EXIT_FAILURE);
}

void xs_init_target(struct xtables_target *target)
{
	if (target->udata_size != 0) {
		free(target->udata);
		target->udata = xtables_calloc(1, target->udata_size);
	}
	if (target->init != NULL)
		target->init(target->t);
}

void xs_init_match(struct xtables_match *match)
{
	if (match->udata_size != 0) {
		/*
		 * As soon as a subsequent instance of the same match
		 * is used, e.g. "-m time -m time", the first instance
		 * is no longer reachable anyway, so we can free udata.
		 * Same goes for target.
		 */
		free(match->udata);
		match->udata = xtables_calloc(1, match->udata_size);
	}
	if (match->init != NULL)
		match->init(match->m);
}

static void alarm_ignore(int i) {
}

static int xtables_lock(int wait)
{
	struct sigaction sigact_alarm;
	const char *lock_file;
	int fd;

	lock_file = getenv("XTABLES_LOCKFILE");
	if (lock_file == NULL || lock_file[0] == '\0')
		lock_file = XT_LOCK_NAME;

	fd = open(lock_file, O_CREAT, 0600);
	if (fd < 0) {
		fprintf(stderr, "Fatal: can't open lock file %s: %s\n",
			lock_file, strerror(errno));
		return XT_LOCK_FAILED;
	}

	if (wait > 0) {
		sigact_alarm.sa_handler = alarm_ignore;
		sigact_alarm.sa_flags = SA_RESETHAND;
		sigemptyset(&sigact_alarm.sa_mask);
		sigaction(SIGALRM, &sigact_alarm, NULL);
		alarm(wait);
	}

	if (flock(fd, LOCK_EX | (wait ? 0 : LOCK_NB)) == 0)
		return fd;

	if (errno == EINTR) {
		errno = EWOULDBLOCK;
	}

	fprintf(stderr, "Can't lock %s: %s\n", lock_file,
		strerror(errno));
	return XT_LOCK_BUSY;
}

void xtables_unlock(int lock)
{
	if (lock >= 0)
		close(lock);
}

int xtables_lock_or_exit(int wait)
{
	int lock = xtables_lock(wait);

	if (lock == XT_LOCK_FAILED) {
		xtables_free_opts(1);
		exit(RESOURCE_PROBLEM);
	}

	if (lock == XT_LOCK_BUSY) {
		fprintf(stderr, "Another app is currently holding the xtables lock. ");
		if (wait == 0)
			fprintf(stderr, "Perhaps you want to use the -w option?\n");
		else
			fprintf(stderr, "Stopped waiting after %ds.\n", wait);
		xtables_free_opts(1);
		exit(RESOURCE_PROBLEM);
	}

	return lock;
}

int parse_wait_time(int argc, char *argv[])
{
	int wait = -1;

	if (optarg) {
		if (sscanf(optarg, "%i", &wait) != 1)
			xtables_error(PARAMETER_PROBLEM,
				"wait seconds not numeric");
	} else if (xs_has_arg(argc, argv))
		if (sscanf(argv[optind++], "%i", &wait) != 1)
			xtables_error(PARAMETER_PROBLEM,
				"wait seconds not numeric");

	return wait;
}

void parse_wait_interval(int argc, char *argv[])
{
	const char *arg;
	unsigned int usec;
	int ret;

	if (optarg)
		arg = optarg;
	else if (xs_has_arg(argc, argv))
		arg = argv[optind++];
	else
		xtables_error(PARAMETER_PROBLEM, "wait interval value required");

	ret = sscanf(arg, "%u", &usec);
	if (ret == 1) {
		if (usec > 999999)
			xtables_error(PARAMETER_PROBLEM,
				      "too long usec wait %u > 999999 usec",
				      usec);

		fprintf(stderr, "Ignoring deprecated --wait-interval option.\n");
		return;
	}
	xtables_error(PARAMETER_PROBLEM, "wait interval not numeric");
}

int parse_counters(const char *string, struct xt_counters *ctr)
{
	int ret;

	if (!string)
		return 0;

	ret = sscanf(string, "[%llu:%llu]",
		     (unsigned long long *)&ctr->pcnt,
		     (unsigned long long *)&ctr->bcnt);

	return ret == 2;
}

/* Tokenize counters argument of typical iptables-restore format rule.
 *
 * If *bufferp contains counters, update *pcntp and *bcntp to point at them,
 * change bytes after counters in *bufferp to nul-bytes, update *bufferp to
 * point to after the counters and return true.
 * If *bufferp does not contain counters, return false.
 * If syntax is wrong in *bufferp, call xtables_error() and hence exit().
 * */
bool tokenize_rule_counters(char **bufferp, char **pcntp, char **bcntp, int line)
{
	char *ptr, *buffer = *bufferp, *pcnt, *bcnt;

	if (buffer[0] != '[')
		return false;

	/* we have counters in our input */

	ptr = strchr(buffer, ']');
	if (!ptr)
		xtables_error(PARAMETER_PROBLEM, "Bad line %u: need ]", line);

	pcnt = strtok(buffer+1, ":");
	if (!pcnt)
		xtables_error(PARAMETER_PROBLEM, "Bad line %u: need :", line);

	bcnt = strtok(NULL, "]");
	if (!bcnt)
		xtables_error(PARAMETER_PROBLEM, "Bad line %u: need ]", line);

	*pcntp = pcnt;
	*bcntp = bcnt;
	/* start command parsing after counter */
	*bufferp = ptr + 1;

	return true;
}

inline bool xs_has_arg(int argc, char *argv[])
{
	return optind < argc &&
	       argv[optind][0] != '-' &&
	       argv[optind][0] != '!';
}

/* function adding one argument to store, updating argc
 * returns if argument added, does not return otherwise */
void add_argv(struct argv_store *store, const char *what, int quoted)
{
	DEBUGP("add_argv: %s\n", what);

	if (store->argc + 1 >= MAX_ARGC)
		xtables_error(PARAMETER_PROBLEM,
			      "Parser cannot handle more arguments");
	if (!what)
		xtables_error(PARAMETER_PROBLEM,
			      "Trying to store NULL argument");

	store->argv[store->argc] = xtables_strdup(what);
	store->argvattr[store->argc] = quoted;
	store->argv[++store->argc] = NULL;
}

void free_argv(struct argv_store *store)
{
	while (store->argc) {
		store->argc--;
		free(store->argv[store->argc]);
		store->argvattr[store->argc] = 0;
	}
}

/* Save parsed rule for comparison with next rule to perform action aggregation
 * on duplicate conditions.
 */
void save_argv(struct argv_store *dst, struct argv_store *src)
{
	int i;

	free_argv(dst);
	for (i = 0; i < src->argc; i++) {
		dst->argvattr[i] = src->argvattr[i];
		dst->argv[i] = src->argv[i];
		src->argv[i] = NULL;
	}
	dst->argc = src->argc;
	src->argc = 0;
}

struct xt_param_buf {
	char	buffer[1024];
	int 	len;
};

static void add_param(struct xt_param_buf *param, const char *curchar)
{
	param->buffer[param->len++] = *curchar;
	if (param->len >= sizeof(param->buffer))
		xtables_error(PARAMETER_PROBLEM,
			      "Parameter too long!");
}

void add_param_to_argv(struct argv_store *store, char *parsestart, int line)
{
	int quote_open = 0, escaped = 0, quoted = 0;
	struct xt_param_buf param = {};
	char *curchar;

	/* After fighting with strtok enough, here's now
	 * a 'real' parser. According to Rusty I'm now no
	 * longer a real hacker, but I can live with that */

	for (curchar = parsestart; *curchar; curchar++) {
		if (quote_open) {
			if (escaped) {
				add_param(&param, curchar);
				escaped = 0;
				continue;
			} else if (*curchar == '\\') {
				escaped = 1;
				continue;
			} else if (*curchar == '"') {
				quote_open = 0;
			} else {
				add_param(&param, curchar);
				continue;
			}
		} else {
			if (*curchar == '"') {
				quote_open = 1;
				quoted = 1;
				continue;
			}
		}

		switch (*curchar) {
		case '"':
			break;
		case ' ':
		case '\t':
		case '\n':
			if (!param.len) {
				/* two spaces? */
				continue;
			}
			break;
		default:
			/* regular character, copy to buffer */
			add_param(&param, curchar);
			continue;
		}

		param.buffer[param.len] = '\0';
		add_argv(store, param.buffer, quoted);
		param.len = 0;
		quoted = 0;
	}
	if (param.len) {
		param.buffer[param.len] = '\0';
		add_argv(store, param.buffer, 0);
	}
}

#ifdef DEBUG
void debug_print_argv(struct argv_store *store)
{
	int i;

	for (i = 0; i < store->argc; i++)
		fprintf(stderr, "argv[%d]: %s\n", i, store->argv[i]);
}
#endif

void print_header(unsigned int format, const char *chain, const char *pol,
		  const struct xt_counters *counters,
		  int refs, uint32_t entries)
{
	printf("Chain %s", chain);
	if (pol) {
		printf(" (policy %s", pol);
		if (!(format & FMT_NOCOUNTS)) {
			fputc(' ', stdout);
			xtables_print_num(counters->pcnt, (format|FMT_NOTABLE));
			fputs("packets, ", stdout);
			xtables_print_num(counters->bcnt, (format|FMT_NOTABLE));
			fputs("bytes", stdout);
		}
		printf(")\n");
	} else if (refs < 0) {
		printf(" (ERROR obtaining refs)\n");
	} else {
		printf(" (%d references)\n", refs);
	}

	if (format & FMT_LINENUMBERS)
		printf(FMT("%-4s ", "%s "), "num");
	if (!(format & FMT_NOCOUNTS)) {
		if (format & FMT_KILOMEGAGIGA) {
			printf(FMT("%5s ","%s "), "pkts");
			printf(FMT("%5s ","%s "), "bytes");
		} else {
			printf(FMT("%8s ","%s "), "pkts");
			printf(FMT("%10s ","%s "), "bytes");
		}
	}
	if (!(format & FMT_NOTARGET))
		printf(FMT("%-9s ","%s "), "target");
	fputs(" prot ", stdout);
	if (format & FMT_OPTIONS)
		fputs("opt", stdout);
	if (format & FMT_VIA) {
		printf(FMT(" %-6s ","%s "), "in");
		printf(FMT("%-6s ","%s "), "out");
	}
	printf(FMT(" %-19s ","%s "), "source");
	printf(FMT(" %-19s "," %s "), "destination");
	printf("\n");
}

const char *ipv4_addr_to_string(const struct in_addr *addr,
				const struct in_addr *mask,
				unsigned int format)
{
	static char buf[BUFSIZ];

	if (!mask->s_addr && !(format & FMT_NUMERIC))
		return "anywhere";

	if (format & FMT_NUMERIC)
		strncpy(buf, xtables_ipaddr_to_numeric(addr), BUFSIZ - 1);
	else
		strncpy(buf, xtables_ipaddr_to_anyname(addr), BUFSIZ - 1);
	buf[BUFSIZ - 1] = '\0';

	strncat(buf, xtables_ipmask_to_numeric(mask),
		BUFSIZ - strlen(buf) - 1);

	return buf;
}

void print_ipv4_addresses(const struct ipt_entry *fw, unsigned int format)
{
	fputc(fw->ip.invflags & IPT_INV_SRCIP ? '!' : ' ', stdout);
	printf(FMT("%-19s ", "%s "),
	       ipv4_addr_to_string(&fw->ip.src, &fw->ip.smsk, format));

	fputc(fw->ip.invflags & IPT_INV_DSTIP ? '!' : ' ', stdout);
	printf(FMT("%-19s ", "-> %s"),
	       ipv4_addr_to_string(&fw->ip.dst, &fw->ip.dmsk, format));
}

static const char *mask_to_str(const struct in_addr *mask)
{
	uint32_t bits, hmask = ntohl(mask->s_addr);
	static char mask_str[INET_ADDRSTRLEN];
	int i;

	if (mask->s_addr == 0xFFFFFFFFU) {
		sprintf(mask_str, "32");
		return mask_str;
	}

	i    = 32;
	bits = 0xFFFFFFFEU;
	while (--i >= 0 && hmask != bits)
		bits <<= 1;
	if (i >= 0)
		sprintf(mask_str, "%u", i);
	else
		inet_ntop(AF_INET, mask, mask_str, sizeof(mask_str));

	return mask_str;
}

void save_ipv4_addr(char letter, const struct in_addr *addr,
		    const struct in_addr *mask, int invert)
{
	char addrbuf[INET_ADDRSTRLEN];

	if (!mask->s_addr && !invert && !addr->s_addr)
		return;

	printf("%s -%c %s/%s", invert ? " !" : "", letter,
	       inet_ntop(AF_INET, addr, addrbuf, sizeof(addrbuf)),
	       mask_to_str(mask));
}

static const char *ipv6_addr_to_string(const struct in6_addr *addr,
				       const struct in6_addr *mask,
				       unsigned int format)
{
	static char buf[BUFSIZ];

	if (IN6_IS_ADDR_UNSPECIFIED(addr) && !(format & FMT_NUMERIC))
		return "anywhere";

	if (format & FMT_NUMERIC)
		strncpy(buf, xtables_ip6addr_to_numeric(addr), BUFSIZ - 1);
	else
		strncpy(buf, xtables_ip6addr_to_anyname(addr), BUFSIZ - 1);
	buf[BUFSIZ - 1] = '\0';

	strncat(buf, xtables_ip6mask_to_numeric(mask),
		BUFSIZ - strlen(buf) - 1);

	return buf;
}

void print_ipv6_addresses(const struct ip6t_entry *fw6, unsigned int format)
{
	fputc(fw6->ipv6.invflags & IP6T_INV_SRCIP ? '!' : ' ', stdout);
	printf(FMT("%-19s ", "%s "),
	       ipv6_addr_to_string(&fw6->ipv6.src,
				   &fw6->ipv6.smsk, format));

	fputc(fw6->ipv6.invflags & IP6T_INV_DSTIP ? '!' : ' ', stdout);
	printf(FMT("%-19s ", "-> %s"),
	       ipv6_addr_to_string(&fw6->ipv6.dst,
				   &fw6->ipv6.dmsk, format));
}

void save_ipv6_addr(char letter, const struct in6_addr *addr,
		    const struct in6_addr *mask, int invert)
{
	int l = xtables_ip6mask_to_cidr(mask);
	char addr_str[INET6_ADDRSTRLEN];

	if (!invert && l == 0)
		return;

	printf("%s -%c %s",
		invert ? " !" : "", letter,
		inet_ntop(AF_INET6, addr, addr_str, sizeof(addr_str)));

	if (l == -1)
		printf("/%s", inet_ntop(AF_INET6, mask,
					addr_str, sizeof(addr_str)));
	else
		printf("/%d", l);
}

void print_fragment(unsigned int flags, unsigned int invflags,
		    unsigned int format, bool fake)
{
	if (!(format & FMT_OPTIONS))
		return;

	if (format & FMT_NOTABLE)
		fputs("opt ", stdout);

	if (fake) {
		fputs("--", stdout);
	} else {
		fputc(invflags & IPT_INV_FRAG ? '!' : '-', stdout);
		fputc(flags & IPT_F_FRAG ? 'f' : '-', stdout);
	}
	fputc(' ', stdout);
}

/* Luckily, IPT_INV_VIA_IN and IPT_INV_VIA_OUT
 * have the same values as IP6T_INV_VIA_IN and IP6T_INV_VIA_OUT
 * so this function serves for both iptables and ip6tables */
void print_ifaces(const char *iniface, const char *outiface, uint8_t invflags,
		  unsigned int format)
{
	const char *anyname = format & FMT_NUMERIC ? "*" : "any";
	char iface[IFNAMSIZ + 2];

	if (!(format & FMT_VIA))
		return;

	snprintf(iface, IFNAMSIZ + 2, "%s%s",
		 invflags & IPT_INV_VIA_IN ? "!" : "",
		 iniface[0] != '\0' ? iniface : anyname);

	printf(FMT(" %-6s ", "in %s "), iface);

	snprintf(iface, IFNAMSIZ + 2, "%s%s",
		 invflags & IPT_INV_VIA_OUT ? "!" : "",
		 outiface[0] != '\0' ? outiface : anyname);

	printf(FMT("%-6s ", "out %s "), iface);
}

static void save_iface(char letter, const char *iface, int invert)
{
	if (iface && strlen(iface) && (strcmp(iface, "+") || invert))
		printf("%s -%c %s", invert ? " !" : "", letter, iface);
}

static void command_match(struct iptables_command_state *cs, bool invert)
{
	struct option *opts = xt_params->opts;
	struct xtables_match *m;
	size_t size;

	if (invert)
		xtables_error(PARAMETER_PROBLEM,
			   "unexpected ! flag before --match");

	m = xtables_find_match(optarg, XTF_LOAD_MUST_SUCCEED, &cs->matches);
	size = XT_ALIGN(sizeof(struct xt_entry_match)) + m->size;
	m->m = xtables_calloc(1, size);
	m->m->u.match_size = size;
	if (m->real_name == NULL) {
		strcpy(m->m->u.user.name, m->name);
	} else {
		strcpy(m->m->u.user.name, m->real_name);
		if (!(m->ext_flags & XTABLES_EXT_ALIAS))
			fprintf(stderr, "Notice: the %s match is converted into %s match "
				"in rule listing and saving.\n", m->name, m->real_name);
	}
	m->m->u.user.revision = m->revision;
	xs_init_match(m);
	if (m == m->next)
		return;
	/* Merge options for non-cloned matches */
	if (m->x6_options != NULL)
		opts = xtables_options_xfrm(xt_params->orig_opts, opts,
					    m->x6_options, &m->option_offset);
	else if (m->extra_opts != NULL)
		opts = xtables_merge_options(xt_params->orig_opts, opts,
					     m->extra_opts, &m->option_offset);
	else
		return;

	if (opts == NULL)
		xtables_error(OTHER_PROBLEM, "can't alloc memory!");
	xt_params->opts = opts;
}

static const char *xt_parse_target(const char *targetname)
{
	const char *ptr;

	if (strlen(targetname) < 1)
		xtables_error(PARAMETER_PROBLEM,
			   "Invalid target name (too short)");

	if (strlen(targetname) >= XT_EXTENSION_MAXNAMELEN)
		xtables_error(PARAMETER_PROBLEM,
			   "Invalid target name `%s' (%u chars max)",
			   targetname, XT_EXTENSION_MAXNAMELEN - 1);

	for (ptr = targetname; *ptr; ptr++)
		if (isspace(*ptr))
			xtables_error(PARAMETER_PROBLEM,
				   "Invalid target name `%s'", targetname);
	return targetname;
}

void command_jump(struct iptables_command_state *cs, const char *jumpto)
{
	struct option *opts = xt_params->opts;
	size_t size;

	cs->jumpto = xt_parse_target(jumpto);
	/* TRY_LOAD (may be chain name) */
	cs->target = xtables_find_target(cs->jumpto, XTF_TRY_LOAD);

	if (cs->target == NULL)
		return;

	size = XT_ALIGN(sizeof(struct xt_entry_target)) + cs->target->size;

	cs->target->t = xtables_calloc(1, size);
	cs->target->t->u.target_size = size;
	if (cs->target->real_name == NULL) {
		strcpy(cs->target->t->u.user.name, cs->jumpto);
	} else {
		/* Alias support for userspace side */
		strcpy(cs->target->t->u.user.name, cs->target->real_name);
		if (!(cs->target->ext_flags & XTABLES_EXT_ALIAS))
			fprintf(stderr, "Notice: The %s target is converted into %s target "
				"in rule listing and saving.\n",
				cs->jumpto, cs->target->real_name);
	}
	cs->target->t->u.user.revision = cs->target->revision;
	xs_init_target(cs->target);

	if (cs->target->x6_options != NULL)
		opts = xtables_options_xfrm(xt_params->orig_opts, opts,
					    cs->target->x6_options,
					    &cs->target->option_offset);
	else if (cs->target->extra_opts != NULL)
		opts = xtables_merge_options(xt_params->orig_opts, opts,
					     cs->target->extra_opts,
					     &cs->target->option_offset);
	else
		return;

	if (opts == NULL)
		xtables_error(OTHER_PROBLEM, "can't alloc memory!");
	xt_params->opts = opts;
}

static char cmd2char(int option)
{
	/* cmdflags index corresponds with position of bit in CMD_* values */
	static const char cmdflags[] = { 'I', 'D', 'D', 'R', 'A', 'L', 'F', 'Z',
					 'N', 'X', 'P', 'E', 'S', 'Z', 'C' };
	int i;

	for (i = 0; option > 1; option >>= 1, i++)
		;
	if (i >= ARRAY_SIZE(cmdflags))
		xtables_error(OTHER_PROBLEM,
			      "cmd2char(): Invalid command number %u.", 1 << i);
	return cmdflags[i];
}

static void add_command(unsigned int *cmd, const int newcmd,
			const int othercmds, int invert)
{
	if (invert)
		xtables_error(PARAMETER_PROBLEM, "unexpected '!' flag");
	if (*cmd & (~othercmds))
		xtables_error(PARAMETER_PROBLEM, "Cannot use -%c with -%c",
			      cmd2char(newcmd), cmd2char(*cmd & (~othercmds)));
	*cmd |= newcmd;
}

/* Can't be zero. */
static int parse_rulenumber(const char *rule)
{
	unsigned int rulenum;

	if (!xtables_strtoui(rule, NULL, &rulenum, 1, INT_MAX))
		xtables_error(PARAMETER_PROBLEM,
			   "Invalid rule number `%s'", rule);

	return rulenum;
}

static void parse_rule_range(struct xt_cmd_parse *p, const char *argv)
{
	char *colon = strchr(argv, ':'), *buffer;

	if (colon) {
		if (!p->rule_ranges)
			xtables_error(PARAMETER_PROBLEM,
				      "Rule ranges are not supported");

		*colon = '\0';
		if (*(colon + 1) == '\0')
			p->rulenum_end = -1; /* Until the last rule */
		else {
			p->rulenum_end = strtol(colon + 1, &buffer, 10);
			if (*buffer != '\0' || p->rulenum_end == 0)
				xtables_error(PARAMETER_PROBLEM,
					      "Invalid rule range end`%s'",
					      colon + 1);
		}
	}
	if (colon == argv)
		p->rulenum = 1; /* Beginning with the first rule */
	else {
		p->rulenum = strtol(argv, &buffer, 10);
		if (*buffer != '\0' || p->rulenum == 0)
			xtables_error(PARAMETER_PROBLEM,
				      "Invalid rule number `%s'", argv);
	}
	if (!colon)
		p->rulenum_end = p->rulenum;
}

/* list the commands an option is allowed with */
#define CMD_IDRAC	CMD_INSERT | CMD_DELETE | CMD_REPLACE | \
			CMD_APPEND | CMD_CHECK | CMD_CHANGE_COUNTERS
static const unsigned int options_v_commands[NUMBER_OF_OPT] = {
/*OPT_NUMERIC*/		CMD_LIST,
/*OPT_SOURCE*/		CMD_IDRAC,
/*OPT_DESTINATION*/	CMD_IDRAC,
/*OPT_PROTOCOL*/	CMD_IDRAC,
/*OPT_JUMP*/		CMD_IDRAC,
/*OPT_VERBOSE*/		UINT_MAX,
/*OPT_EXPANDED*/	CMD_LIST,
/*OPT_VIANAMEIN*/	CMD_IDRAC,
/*OPT_VIANAMEOUT*/	CMD_IDRAC,
/*OPT_LINENUMBERS*/	CMD_LIST,
/*OPT_COUNTERS*/	CMD_INSERT | CMD_REPLACE | CMD_APPEND | CMD_SET_POLICY,
/*OPT_FRAGMENT*/	CMD_IDRAC,
/*OPT_S_MAC*/		CMD_IDRAC,
/*OPT_D_MAC*/		CMD_IDRAC,
/*OPT_H_LENGTH*/	CMD_IDRAC,
/*OPT_OPCODE*/		CMD_IDRAC,
/*OPT_H_TYPE*/		CMD_IDRAC,
/*OPT_P_TYPE*/		CMD_IDRAC,
/*OPT_LOGICALIN*/	CMD_IDRAC,
/*OPT_LOGICALOUT*/	CMD_IDRAC,
/*OPT_LIST_C*/		CMD_LIST,
/*OPT_LIST_X*/		CMD_LIST,
/*OPT_LIST_MAC2*/	CMD_LIST,
};
#undef CMD_IDRAC

static void generic_opt_check(struct xt_cmd_parse_ops *ops,
			      int command, int options)
{
	int i, optval;

	/* Check that commands are valid with options. Complicated by the
	 * fact that if an option is legal with *any* command given, it is
	 * legal overall (ie. -z and -l).
	 */
	for (i = 0, optval = 1; i < NUMBER_OF_OPT; optval = (1 << ++i)) {
		if ((options & optval) &&
		    (options_v_commands[i] & command) != command)
			xtables_error(PARAMETER_PROBLEM,
				      "Illegal option `%s' with this command",
				      ops->option_name(optval));
	}
}

const char *ip46t_option_name(int option)
{
	switch (option) {
	case OPT_NUMERIC:	return "--numeric";
	case OPT_SOURCE:	return "--source";
	case OPT_DESTINATION:	return "--destination";
	case OPT_PROTOCOL:	return "--protocol";
	case OPT_JUMP:		return "--jump";
	case OPT_VERBOSE:	return "--verbose";
	case OPT_EXPANDED:	return "--exact";
	case OPT_VIANAMEIN:	return "--in-interface";
	case OPT_VIANAMEOUT:	return "--out-interface";
	case OPT_LINENUMBERS:	return "--line-numbers";
	case OPT_COUNTERS:	return "--set-counters";
	case OPT_FRAGMENT:	return "--fragments";
	default:		return "unknown option";
	}
}

int ip46t_option_invert(int option)
{
	switch (option) {
	case OPT_SOURCE:	return IPT_INV_SRCIP;
	case OPT_DESTINATION:	return IPT_INV_DSTIP;
	case OPT_PROTOCOL:	return XT_INV_PROTO;
	case OPT_VIANAMEIN:	return IPT_INV_VIA_IN;
	case OPT_VIANAMEOUT:	return IPT_INV_VIA_OUT;
	case OPT_FRAGMENT:	return IPT_INV_FRAG;
	default:		return -1;
	}
}

static void
set_option(struct xt_cmd_parse_ops *ops,
	   unsigned int *options, unsigned int option,
	   uint16_t *invflg, bool invert)
{
	if (*options & option)
		xtables_error(PARAMETER_PROBLEM,
			      "multiple %s options not allowed",
			      ops->option_name(option));
	*options |= option;

	if (invert) {
		int invopt = ops->option_invert(option);

		if (invopt < 0)
			xtables_error(PARAMETER_PROBLEM,
				      "cannot have ! before %s",
				      ops->option_name(option));
		*invflg |= invopt;
	}
}

void assert_valid_chain_name(const char *chainname)
{
	const char *ptr;

	if (strlen(chainname) >= XT_EXTENSION_MAXNAMELEN)
		xtables_error(PARAMETER_PROBLEM,
			      "chain name `%s' too long (must be under %u chars)",
			      chainname, XT_EXTENSION_MAXNAMELEN);

	if (*chainname == '-' || *chainname == '!')
		xtables_error(PARAMETER_PROBLEM,
			      "chain name not allowed to start with `%c'",
			      *chainname);

	if (xtables_find_target(chainname, XTF_TRY_LOAD))
		xtables_error(PARAMETER_PROBLEM,
			      "chain name may not clash with target name");

	for (ptr = chainname; *ptr; ptr++)
		if (isspace(*ptr))
			xtables_error(PARAMETER_PROBLEM,
				      "Invalid chain name `%s'", chainname);
}

void print_rule_details(unsigned int linenum, const struct xt_counters *ctrs,
			const char *targname, uint8_t proto, uint8_t flags,
			uint8_t invflags, unsigned int format)
{
	const char *pname = proto_to_name(proto, format&FMT_NUMERIC);

	if (format & FMT_LINENUMBERS)
		printf(FMT("%-4u ", "%u "), linenum);

	if (!(format & FMT_NOCOUNTS)) {
		xtables_print_num(ctrs->pcnt, format);
		xtables_print_num(ctrs->bcnt, format);
	}

	if (!(format & FMT_NOTARGET))
		printf(FMT("%-9s ", "%s "), targname ? targname : "");

	fputc(invflags & XT_INV_PROTO ? '!' : ' ', stdout);

	if (pname)
		printf(FMT("%-4s ", "%s "), pname);
	else
		printf(FMT("%-4hu ", "%hu "), proto);
}

void save_rule_details(const char *iniface, const char *outiface,
		       uint16_t proto, int frag, uint8_t invflags)
{
	save_iface('i', iniface, invflags & IPT_INV_VIA_IN);
	save_iface('o', outiface, invflags & IPT_INV_VIA_OUT);

	if (proto > 0) {
		const char *pname = proto_to_name(proto, true);

		if (invflags & XT_INV_PROTO)
			printf(" !");

		if (pname)
			printf(" -p %s", pname);
		else
			printf(" -p %u", proto);
	}

	if (frag) {
		if (invflags & IPT_INV_FRAG)
			printf(" !");
		printf(" -f");
	}
}

int print_match_save(const struct xt_entry_match *e, const void *ip)
{
	const char *name = e->u.user.name;
	const int revision = e->u.user.revision;
	struct xtables_match *match, *mt, *mt2;

	match = xtables_find_match(name, XTF_TRY_LOAD, NULL);
	if (match) {
		mt = mt2 = xtables_find_match_revision(name, XTF_TRY_LOAD,
						       match, revision);
		if (!mt2)
			mt2 = match;
		printf(" -m %s", mt2->alias ? mt2->alias(e) : name);

		/* some matches don't provide a save function */
		if (mt && mt->save)
			mt->save(ip, e);
		else if (match->save)
			printf(" [unsupported revision]");
	} else {
		if (e->u.match_size) {
			fprintf(stderr,
				"Can't find library for match `%s'\n",
				name);
			exit(1);
		}
	}
	return 0;
}

void xtables_printhelp(struct iptables_command_state *cs)
{
	const struct xtables_rule_match *matches = cs->matches;
	const char *prog_name = xt_params->program_name;
	const char *prog_vers = xt_params->program_version;

	printf("%s v%s\n\n"
"Usage: %s -[ACD] chain rule-specification [options]\n"
"       %s -I chain [rulenum] rule-specification [options]\n"
"       %s -R chain rulenum rule-specification [options]\n"
"       %s -D chain rulenum [options]\n"
"       %s -[LS] [chain [rulenum]] [options]\n"
"       %s -[FZ] [chain] [options]\n"
"       %s -[NX] chain\n"
"       %s -E old-chain-name new-chain-name\n"
"       %s -P chain target [options]\n"
"       %s -h (print this help information)\n\n",
	       prog_name, prog_vers, prog_name, prog_name,
	       prog_name, prog_name, prog_name, prog_name,
	       prog_name, prog_name, prog_name, prog_name);

	printf(
"Commands:\n"
"Either long or short options are allowed.\n"
"  --append  -A chain		Append to chain\n"
"  --check   -C chain		Check for the existence of a rule\n"
"  --delete  -D chain		Delete matching rule from chain\n"
"  --delete  -D chain rulenum\n"
"				Delete rule rulenum (1 = first) from chain\n"
"  --insert  -I chain [rulenum]\n"
"				Insert in chain as rulenum (default 1=first)\n"
"  --replace -R chain rulenum\n"
"				Replace rule rulenum (1 = first) in chain\n"
"  --list    -L [chain [rulenum]]\n"
"				List the rules in a chain or all chains\n"
"  --list-rules -S [chain [rulenum]]\n"
"				Print the rules in a chain or all chains\n"
"  --flush   -F [chain]		Delete all rules in  chain or all chains\n"
"  --zero    -Z [chain [rulenum]]\n"
"				Zero counters in chain or all chains\n"
"  --new     -N chain		Create a new user-defined chain\n"
"  --delete-chain\n"
"            -X [chain]		Delete a user-defined chain\n"
"  --policy  -P chain target\n"
"				Change policy on chain to target\n"
"  --rename-chain\n"
"            -E old-chain new-chain\n"
"				Change chain name, (moving any references)\n"
"\n"
"Options:\n");

	if (afinfo->family == NFPROTO_ARP) {
		printf(
"[!] --source-ip	-s address[/mask]\n"
"				source specification\n"
"[!] --destination-ip -d address[/mask]\n"
"				destination specification\n"
"[!] --source-mac address[/mask]\n"
"[!] --destination-mac address[/mask]\n"
"    --h-length   -l   length[/mask] hardware length (nr of bytes)\n"
"    --opcode code[/mask] operation code (2 bytes)\n"
"    --h-type   type[/mask]  hardware type (2 bytes, hexadecimal)\n"
"    --proto-type   type[/mask]  protocol type (2 bytes)\n");
	} else {
		printf(
"    --ipv4	-4		%s (line is ignored by ip6tables-restore)\n"
"    --ipv6	-6		%s (line is ignored by iptables-restore)\n"
"[!] --protocol	-p proto	protocol: by number or name, eg. `tcp'\n"
"[!] --source	-s address[/mask][...]\n"
"				source specification\n"
"[!] --destination -d address[/mask][...]\n"
"				destination specification\n",
		afinfo->family == NFPROTO_IPV4 ? "Nothing" : "Error",
		afinfo->family == NFPROTO_IPV4 ? "Error" : "Nothing");
	}

	printf(
"[!] --in-interface -i input name[+]\n"
"				network interface name ([+] for wildcard)\n"
" --jump	-j target\n"
"				target for rule (may load target extension)\n");

	if (0
#ifdef IPT_F_GOTO
	    || afinfo->family == NFPROTO_IPV4
#endif
#ifdef IP6T_F_GOTO
	    || afinfo->family == NFPROTO_IPV6
#endif
	   )
		printf(
"  --goto      -g chain\n"
"			       jump to chain with no return\n");
	printf(
"  --match	-m match\n"
"				extended match (may load extension)\n"
"  --numeric	-n		numeric output of addresses and ports\n"
"[!] --out-interface -o output name[+]\n"
"				network interface name ([+] for wildcard)\n"
"  --table	-t table	table to manipulate (default: `filter')\n"
"  --verbose	-v		verbose mode\n"
"  --wait	-w [seconds]	maximum wait to acquire xtables lock before give up\n"
"  --line-numbers		print line numbers when listing\n"
"  --exact	-x		expand numbers (display exact values)\n");

	if (afinfo->family == NFPROTO_IPV4)
		printf(
"[!] --fragment	-f		match second or further fragments only\n");

	printf(
"  --modprobe=<command>		try to insert modules using this command\n"
"  --set-counters -c PKTS BYTES	set the counter during insert/append\n"
"[!] --version	-V		print package version.\n");

	if (afinfo->family == NFPROTO_ARP) {
		int i;

		printf(" opcode strings: \n");
		for (i = 0; i < ARP_NUMOPCODES; i++)
			printf(" %d = %s\n", i + 1, arp_opcodes[i]);
		printf(
	" hardware type string: 1 = Ethernet\n"
	" protocol type string: 0x800 = IPv4\n");

		xtables_find_target("standard", XTF_TRY_LOAD);
		xtables_find_target("mangle", XTF_TRY_LOAD);
		xtables_find_target("CLASSIFY", XTF_TRY_LOAD);
		xtables_find_target("MARK", XTF_TRY_LOAD);
	}

	print_extension_helps(xtables_targets, matches);
}

void exit_tryhelp(int status, int line)
{
	if (line != -1)
		fprintf(stderr, "Error occurred at line: %d\n", line);
	fprintf(stderr, "Try `%s -h' or '%s --help' for more information.\n",
			xt_params->program_name, xt_params->program_name);
	xtables_free_opts(1);
	exit(status);
}

static void check_empty_interface(struct xtables_args *args, const char *arg)
{
	const char *msg = "Empty interface is likely to be undesired";

	if (*arg != '\0')
		return;

	if (args->family != NFPROTO_ARP)
		xtables_error(PARAMETER_PROBLEM, "%s", msg);

	fprintf(stderr, "%s", msg);
}

static void check_inverse(struct xtables_args *args, const char option[],
			  bool *invert, int argc, char **argv)
{
	switch (args->family) {
	case NFPROTO_ARP:
	case NFPROTO_BRIDGE:
		break;
	default:
		return;
	}

	if (!option || strcmp(option, "!"))
		return;

	fprintf(stderr, "Using intrapositioned negation (`--option ! this`) "
		"is deprecated in favor of extrapositioned (`! --option this`).\n");

	if (*invert)
		xtables_error(PARAMETER_PROBLEM,
			      "Multiple `!' flags not allowed");
	*invert = true;
	optind++;
	if (optind > argc)
		xtables_error(PARAMETER_PROBLEM, "no argument following `!'");

	optarg = argv[optind - 1];
}

static const char *optstring_lookup(int family)
{
	switch (family) {
	case AF_INET:
	case AF_INET6:
		return IPT_OPTSTRING;
	case NFPROTO_ARP:
		return ARPT_OPTSTRING;
	case NFPROTO_BRIDGE:
		return EBT_OPTSTRING;
	}
	return "";
}

void xtables_clear_iptables_command_state(struct iptables_command_state *cs)
{
	xtables_rule_matches_free(&cs->matches);
	if (cs->target) {
		free(cs->target->t);
		cs->target->t = NULL;

		free(cs->target->udata);
		cs->target->udata = NULL;

		if (cs->target == cs->target->next) {
			free(cs->target);
			cs->target = NULL;
		}
	}
}

void iface_to_mask(const char *iface, unsigned char *mask)
{
	unsigned int len = strlen(iface);

	memset(mask, 0, IFNAMSIZ);

	if (!len) {
		return;
	} else if (iface[len - 1] == '+') {
		memset(mask, 0xff, len - 1);
		/* Don't remove `+' here! -HW */
	} else {
		/* Include nul-terminator in match */
		memset(mask, 0xff, len + 1);
	}
}

static void parse_interface(const char *arg, char *iface)
{
	unsigned int len = strlen(arg);

	memset(iface, 0, IFNAMSIZ);

	if (!len)
		return;
	if (len >= IFNAMSIZ)
		xtables_error(PARAMETER_PROBLEM,
			      "interface name `%s' must be shorter than %d characters",
			      arg, IFNAMSIZ);

	if (strchr(arg, '/') || strchr(arg, ' '))
		fprintf(stderr,
			"Warning: weird character in interface `%s' ('/' and ' ' are not allowed by the kernel).\n",
			arg);

	strcpy(iface, arg);
}

static bool
parse_signed_counter(char *argv, unsigned long long *val, uint8_t *ctr_op,
		     uint8_t flag_inc, uint8_t flag_dec)
{
	char *endptr, *p = argv;

	switch (*p) {
	case '+':
		*ctr_op |= flag_inc;
		p++;
		break;
	case '-':
		*ctr_op |= flag_dec;
		p++;
		break;
	}
	*val = strtoull(p, &endptr, 10);
	return *endptr == '\0';
}

static void parse_change_counters_rule(int argc, char **argv,
				       struct xt_cmd_parse *p,
				       struct xtables_args *args)
{
	if (optind + 1 >= argc ||
	    (argv[optind][0] == '-' && !isdigit(argv[optind][1])) ||
	    (argv[optind + 1][0] == '-' && !isdigit(argv[optind + 1][1])))
		xtables_error(PARAMETER_PROBLEM,
			      "The command -C needs at least 2 arguments");
	if (optind + 2 < argc &&
	    (argv[optind + 2][0] != '-' || isdigit(argv[optind + 2][1]))) {
		if (optind + 3 != argc)
			xtables_error(PARAMETER_PROBLEM,
				      "No extra options allowed with -C start_nr[:end_nr] pcnt bcnt");
		parse_rule_range(p, argv[optind++]);
	}

	if (!parse_signed_counter(argv[optind++], &args->pcnt_cnt,
				  &args->counter_op,
				  CTR_OP_INC_PKTS, CTR_OP_DEC_PKTS) ||
	    !parse_signed_counter(argv[optind++], &args->bcnt_cnt,
				  &args->counter_op,
				  CTR_OP_INC_BYTES, CTR_OP_DEC_BYTES))
		xtables_error(PARAMETER_PROBLEM,
			      "Packet counter '%s' invalid", argv[optind - 1]);
}

static void option_test_and_reject(struct xt_cmd_parse *p,
				   struct iptables_command_state *cs,
				   unsigned int option)
{
	if (cs->options & option)
		xtables_error(PARAMETER_PROBLEM, "Can't use %s with %s",
			      p->ops->option_name(option), p->chain);
}

void do_parse(int argc, char *argv[],
	      struct xt_cmd_parse *p, struct iptables_command_state *cs,
	      struct xtables_args *args)
{
	bool family_is_bridge = args->family == NFPROTO_BRIDGE;
	struct xtables_match *m;
	struct xtables_rule_match *matchp;
	bool wait_interval_set = false;
	struct xtables_target *t;
	bool table_set = false;
	bool invert = false;

	/* re-set optind to 0 in case do_command4 gets called
	 * a second time */
	optind = 0;

	/* clear mflags in case do_command4 gets called a second time
	 * (we clear the global list of all matches for security)*/
	for (m = xtables_matches; m; m = m->next)
		m->mflags = 0;

	for (t = xtables_targets; t; t = t->next) {
		t->tflags = 0;
		t->used = 0;
	}

	/* Suppress error messages: we may add new options if we
	   demand-load a protocol. */
	opterr = 0;

	while ((cs->c = getopt_long(argc, argv,
				    optstring_lookup(afinfo->family),
				    xt_params->opts ?: xt_params->orig_opts,
				    NULL)) != -1) {
		switch (cs->c) {
			/*
			 * Command selection
			 */
		case 'A':
			add_command(&p->command, CMD_APPEND, CMD_NONE, invert);
			p->chain = optarg;
			break;

		case 'C':
			if (family_is_bridge) {
				add_command(&p->command, CMD_CHANGE_COUNTERS,
					    CMD_NONE, invert);
				p->chain = optarg;
				parse_change_counters_rule(argc, argv, p, args);
				break;
			}
			/* fall through */
		case 14: /* ebtables --check */
			add_command(&p->command, CMD_CHECK, CMD_NONE, invert);
			p->chain = optarg;
			break;

		case 'D':
			add_command(&p->command, CMD_DELETE, CMD_NONE, invert);
			p->chain = optarg;
			if (xs_has_arg(argc, argv)) {
				parse_rule_range(p, argv[optind++]);
				p->command = CMD_DELETE_NUM;
			}
			break;

		case 'R':
			add_command(&p->command, CMD_REPLACE, CMD_NONE, invert);
			p->chain = optarg;
			if (xs_has_arg(argc, argv))
				p->rulenum = parse_rulenumber(argv[optind++]);
			else
				xtables_error(PARAMETER_PROBLEM,
					   "-%c requires a rule number",
					   cmd2char(CMD_REPLACE));
			break;

		case 'I':
			add_command(&p->command, CMD_INSERT, CMD_NONE, invert);
			p->chain = optarg;
			if (xs_has_arg(argc, argv))
				p->rulenum = parse_rulenumber(argv[optind++]);
			else
				p->rulenum = 1;
			break;

		case 'L':
			add_command(&p->command, CMD_LIST,
				    CMD_ZERO | CMD_ZERO_NUM, invert);
			if (optarg)
				p->chain = optarg;
			else if (xs_has_arg(argc, argv))
				p->chain = argv[optind++];
			if (xs_has_arg(argc, argv))
				p->rulenum = parse_rulenumber(argv[optind++]);
			break;

		case 'S':
			add_command(&p->command, CMD_LIST_RULES,
				    CMD_ZERO|CMD_ZERO_NUM, invert);
			if (optarg)
				p->chain = optarg;
			else if (xs_has_arg(argc, argv))
				p->chain = argv[optind++];
			if (xs_has_arg(argc, argv))
				p->rulenum = parse_rulenumber(argv[optind++]);
			break;

		case 'F':
			add_command(&p->command, CMD_FLUSH, CMD_NONE, invert);
			if (optarg)
				p->chain = optarg;
			else if (xs_has_arg(argc, argv))
				p->chain = argv[optind++];
			break;

		case 'Z':
			add_command(&p->command, CMD_ZERO,
				    CMD_LIST|CMD_LIST_RULES, invert);
			if (optarg)
				p->chain = optarg;
			else if (xs_has_arg(argc, argv))
				p->chain = argv[optind++];
			if (xs_has_arg(argc, argv)) {
				p->rulenum = parse_rulenumber(argv[optind++]);
				p->command = CMD_ZERO_NUM;
			}
			break;

		case 'N':
			assert_valid_chain_name(optarg);
			add_command(&p->command, CMD_NEW_CHAIN, CMD_NONE,
				    invert);
			p->chain = optarg;
			break;

		case 'X':
			add_command(&p->command, CMD_DELETE_CHAIN, CMD_NONE,
				    invert);
			if (optarg)
				p->chain = optarg;
			else if (xs_has_arg(argc, argv))
				p->chain = argv[optind++];
			break;

		case 'E':
			add_command(&p->command, CMD_RENAME_CHAIN, CMD_NONE,
				    invert);
			p->chain = optarg;
			if (xs_has_arg(argc, argv))
				p->newname = argv[optind++];
			else
				xtables_error(PARAMETER_PROBLEM,
					   "-%c requires old-chain-name and "
					   "new-chain-name",
					    cmd2char(CMD_RENAME_CHAIN));
			assert_valid_chain_name(p->newname);
			break;

		case 'P':
			add_command(&p->command, CMD_SET_POLICY,
				    family_is_bridge ? CMD_NEW_CHAIN : CMD_NONE,
				    invert);
			if (p->command & CMD_NEW_CHAIN) {
				p->policy = optarg;
			} else if (xs_has_arg(argc, argv)) {
				p->chain = optarg;
				p->policy = argv[optind++];
			} else {
				xtables_error(PARAMETER_PROBLEM,
					   "-%c requires a chain and a policy",
					   cmd2char(CMD_SET_POLICY));
			}
			break;

		case 'h':
			/* iptables -p icmp -h */
			if (!cs->matches && cs->protocol)
				xtables_find_match(cs->protocol,
					XTF_TRY_LOAD, &cs->matches);

			p->ops->print_help(cs);
			xtables_clear_iptables_command_state(cs);
			xtables_free_opts(1);
			xtables_fini();
			exit(0);

			/*
			 * Option selection
			 */
		case 'p':
			check_inverse(args, optarg, &invert, argc, argv);
			set_option(p->ops, &cs->options, OPT_PROTOCOL,
				   &args->invflags, invert);

			/* Canonicalize into lower case */
			for (cs->protocol = optarg;
			     *cs->protocol; cs->protocol++)
				*cs->protocol = tolower(*cs->protocol);

			cs->protocol = optarg;

			/* This needs to happen here to parse extensions */
			if (p->ops->proto_parse)
				p->ops->proto_parse(cs, args);
			break;

		case 's':
			check_inverse(args, optarg, &invert, argc, argv);
			set_option(p->ops, &cs->options, OPT_SOURCE,
				   &args->invflags, invert);
			args->shostnetworkmask = optarg;
			break;

		case 'd':
			check_inverse(args, optarg, &invert, argc, argv);
			set_option(p->ops, &cs->options, OPT_DESTINATION,
				   &args->invflags, invert);
			args->dhostnetworkmask = optarg;
			break;

#ifdef IPT_F_GOTO
		case 'g':
			set_option(p->ops, &cs->options, OPT_JUMP,
				   &args->invflags, invert);
			args->goto_set = true;
			cs->jumpto = xt_parse_target(optarg);
			break;
#endif

		case 2:/* src-mac */
			check_inverse(args, optarg, &invert, argc, argv);
			set_option(p->ops, &cs->options, OPT_S_MAC,
				   &args->invflags, invert);
			args->src_mac = optarg;
			break;

		case 3:/* dst-mac */
			check_inverse(args, optarg, &invert, argc, argv);
			set_option(p->ops, &cs->options, OPT_D_MAC,
				   &args->invflags, invert);
			args->dst_mac = optarg;
			break;

		case 'l':/* hardware length */
			check_inverse(args, optarg, &invert, argc, argv);
			set_option(p->ops, &cs->options, OPT_H_LENGTH,
				   &args->invflags, invert);
			args->arp_hlen = optarg;
			break;

		case 8: /* was never supported, not even in arptables-legacy */
			xtables_error(PARAMETER_PROBLEM, "not supported");
		case 4:/* opcode */
			check_inverse(args, optarg, &invert, argc, argv);
			set_option(p->ops, &cs->options, OPT_OPCODE,
				   &args->invflags, invert);
			args->arp_opcode = optarg;
			break;

		case 5:/* h-type */
			check_inverse(args, optarg, &invert, argc, argv);
			set_option(p->ops, &cs->options, OPT_H_TYPE,
				   &args->invflags, invert);
			args->arp_htype = optarg;
			break;

		case 6:/* proto-type */
			check_inverse(args, optarg, &invert, argc, argv);
			set_option(p->ops, &cs->options, OPT_P_TYPE,
				   &args->invflags, invert);
			args->arp_ptype = optarg;
			break;

		case 11: /* ebtables --init-table */
			if (p->restore)
				xtables_error(PARAMETER_PROBLEM,
					      "--init-table is not supported in daemon mode");
			add_command(&p->command, CMD_INIT_TABLE, CMD_NONE, invert);
			break;

		case 12 : /* ebtables --Lmac2 */
			set_option(p->ops, &cs->options, OPT_LIST_MAC2,
				   &args->invflags, invert);
			break;

		case 13 : /* ebtables --concurrent */
			break;

		case 15 : /* ebtables --logical-in */
			check_inverse(args, optarg, &invert, argc, argv);
			set_option(p->ops, &cs->options, OPT_LOGICALIN,
				   &args->invflags, invert);
			parse_interface(optarg, args->bri_iniface);
			break;

		case 16 : /* ebtables --logical-out */
			check_inverse(args, optarg, &invert, argc, argv);
			set_option(p->ops, &cs->options, OPT_LOGICALOUT,
				   &args->invflags, invert);
			parse_interface(optarg, args->bri_outiface);
			break;

		case 17 : /* ebtables --Lc */
			set_option(p->ops, &cs->options, OPT_LIST_C,
				   &args->invflags, invert);
			break;

		case 19 : /* ebtables --Lx */
			set_option(p->ops, &cs->options, OPT_LIST_X,
				   &args->invflags, invert);
			break;

		case 'j':
			set_option(p->ops, &cs->options, OPT_JUMP,
				   &args->invflags, invert);
			if (strcmp(optarg, "CONTINUE"))
				command_jump(cs, optarg);
			break;

		case 'i':
			check_empty_interface(args, optarg);
			check_inverse(args, optarg, &invert, argc, argv);
			set_option(p->ops, &cs->options, OPT_VIANAMEIN,
				   &args->invflags, invert);
			parse_interface(optarg, args->iniface);
			break;

		case 'o':
			check_empty_interface(args, optarg);
			check_inverse(args, optarg, &invert, argc, argv);
			set_option(p->ops, &cs->options, OPT_VIANAMEOUT,
				   &args->invflags, invert);
			parse_interface(optarg, args->outiface);
			break;

		case 'f':
			if (args->family == AF_INET6) {
				xtables_error(PARAMETER_PROBLEM,
					"`-f' is not supported in IPv6, "
					"use -m frag instead");
			}
			set_option(p->ops, &cs->options, OPT_FRAGMENT,
				   &args->invflags, invert);
			args->flags |= IPT_F_FRAG;
			break;

		case 'v':
			if (!p->verbose)
				set_option(p->ops, &cs->options, OPT_VERBOSE,
					   &args->invflags, invert);
			p->verbose++;
			break;

		case 'm':
			command_match(cs, invert);
			break;

		case 'n':
			set_option(p->ops, &cs->options, OPT_NUMERIC,
				   &args->invflags, invert);
			break;

		case 't':
			if (invert)
				xtables_error(PARAMETER_PROBLEM,
					   "unexpected ! flag before --table");
			if (p->restore && table_set)
				xtables_error(PARAMETER_PROBLEM,
					      "The -t option cannot be used in %s.\n",
					      xt_params->program_name);
			p->table = optarg;
			table_set = true;
			break;

		case 'x':
			set_option(p->ops, &cs->options, OPT_EXPANDED,
				   &args->invflags, invert);
			break;

		case 'V':
			if (invert)
				printf("Not %s ;-)\n",
				       xt_params->program_version);
			else
				printf("%s v%s\n",
				       xt_params->program_name,
				       xt_params->program_version);
			exit(0);

		case 'w':
			if (p->restore) {
				xtables_error(PARAMETER_PROBLEM,
					      "You cannot use `-w' from "
					      "iptables-restore");
			}

			args->wait = parse_wait_time(argc, argv);
			break;

		case 'W':
			if (p->restore) {
				xtables_error(PARAMETER_PROBLEM,
					      "You cannot use `-W' from "
					      "iptables-restore");
			}

			parse_wait_interval(argc, argv);
			wait_interval_set = true;
			break;

		case '0':
		case 18 : /* ebtables --Ln */
			set_option(p->ops, &cs->options, OPT_LINENUMBERS,
				   &args->invflags, invert);
			break;

		case 'M':
			xtables_modprobe_program = optarg;
			break;

		case 'c':
			set_option(p->ops, &cs->options, OPT_COUNTERS,
				   &args->invflags, invert);
			args->pcnt = optarg;
			args->bcnt = strchr(args->pcnt, ',');
			if (args->bcnt)
			    args->bcnt++;
			if (!args->bcnt && xs_has_arg(argc, argv))
				args->bcnt = argv[optind++];
			if (!args->bcnt)
				xtables_error(PARAMETER_PROBLEM,
					      "%s requires packet and byte counter",
					      p->ops->option_name(OPT_COUNTERS));

			if (sscanf(args->pcnt, "%llu", &args->pcnt_cnt) != 1)
				xtables_error(PARAMETER_PROBLEM,
					      "%s packet counter not numeric",
					      p->ops->option_name(OPT_COUNTERS));

			if (sscanf(args->bcnt, "%llu", &args->bcnt_cnt) != 1)
				xtables_error(PARAMETER_PROBLEM,
					      "%s byte counter not numeric",
					      p->ops->option_name(OPT_COUNTERS));
			break;

		case '4':
			if (args->family == AF_INET)
				break;

			if (p->restore && args->family == AF_INET6)
				return;

			exit_tryhelp(2, p->line);

		case '6':
			if (args->family == AF_INET6)
				break;

			if (p->restore && args->family == AF_INET)
				return;

			exit_tryhelp(2, p->line);

		case 1: /* non option */
			if (optarg[0] == '!' && optarg[1] == '\0') {
				if (invert)
					xtables_error(PARAMETER_PROBLEM,
						   "multiple consecutive ! not"
						   " allowed");
				invert = true;
				optarg[0] = '\0';
				continue;
			}
			fprintf(stderr, "Bad argument `%s'\n", optarg);
			exit_tryhelp(2, p->line);

		default:
			check_inverse(args, optarg, &invert, argc, argv);
			if (p->ops->command_default(cs, xt_params, invert))
				/* cf. ip6tables.c */
				continue;
			break;
		}
		invert = false;
	}

	if (!family_is_bridge &&
	    strcmp(p->table, "nat") == 0 &&
	    ((p->policy != NULL && strcmp(p->policy, "DROP") == 0) ||
	    (cs->jumpto != NULL && strcmp(cs->jumpto, "DROP") == 0)))
		xtables_error(PARAMETER_PROBLEM,
			"\nThe \"nat\" table is not intended for filtering, "
			"the use of DROP is therefore inhibited.\n\n");

	if (!args->wait && wait_interval_set)
		xtables_error(PARAMETER_PROBLEM,
			      "--wait-interval only makes sense with --wait\n");

	for (matchp = cs->matches; matchp; matchp = matchp->next)
		xtables_option_mfcall(matchp->match);
	if (cs->target != NULL)
		xtables_option_tfcall(cs->target);

	/* Fix me: must put inverse options checking here --MN */

	if (optind < argc)
		xtables_error(PARAMETER_PROBLEM,
			   "unknown arguments found on commandline");
	if (!p->command)
		xtables_error(PARAMETER_PROBLEM, "no command specified");
	if (invert)
		xtables_error(PARAMETER_PROBLEM,
			   "nothing appropriate following !");

	if (p->ops->post_parse)
		p->ops->post_parse(p->command, cs, args);

	generic_opt_check(p->ops, p->command, cs->options);

	if (p->chain != NULL && strlen(p->chain) >= XT_EXTENSION_MAXNAMELEN)
		xtables_error(PARAMETER_PROBLEM,
			   "chain name `%s' too long (must be under %u chars)",
			   p->chain, XT_EXTENSION_MAXNAMELEN);

	if (p->command == CMD_APPEND ||
	    p->command == CMD_DELETE ||
	    p->command == CMD_CHECK ||
	    p->command == CMD_INSERT ||
	    p->command == CMD_REPLACE ||
	    p->command == CMD_CHANGE_COUNTERS) {
		if (strcmp(p->chain, "PREROUTING") == 0
		    || strcmp(p->chain, "INPUT") == 0) {
			/* -o not valid with incoming packets. */
			option_test_and_reject(p, cs, OPT_VIANAMEOUT);
			/* same with --logical-out */
			option_test_and_reject(p, cs, OPT_LOGICALOUT);
		}

		if (strcmp(p->chain, "POSTROUTING") == 0
		    || strcmp(p->chain, "OUTPUT") == 0) {
			/* -i not valid with outgoing packets */
			option_test_and_reject(p, cs, OPT_VIANAMEIN);
			/* same with --logical-in */
			option_test_and_reject(p, cs, OPT_LOGICALIN);
		}
	}
}

void ipv4_proto_parse(struct iptables_command_state *cs,
		      struct xtables_args *args)
{
	cs->fw.ip.proto = xtables_parse_protocol(cs->protocol);

	if (cs->fw.ip.proto == 0 &&
	    (args->invflags & XT_INV_PROTO))
		xtables_error(PARAMETER_PROBLEM,
			      "rule would never match protocol");

	cs->fw.ip.invflags = args->invflags;
}

/* These are invalid numbers as upper layer protocol */
static int is_exthdr(uint16_t proto)
{
	return (proto == IPPROTO_ROUTING ||
		proto == IPPROTO_FRAGMENT ||
		proto == IPPROTO_AH ||
		proto == IPPROTO_DSTOPTS);
}

void ipv6_proto_parse(struct iptables_command_state *cs,
		      struct xtables_args *args)
{
	cs->fw6.ipv6.proto = xtables_parse_protocol(cs->protocol);

	if (cs->fw6.ipv6.proto == 0 &&
	    (args->invflags & XT_INV_PROTO))
		xtables_error(PARAMETER_PROBLEM,
			      "rule would never match protocol");

	cs->fw6.ipv6.invflags = args->invflags;

	/* this is needed for ip6tables-legacy only */
	args->flags |= IP6T_F_PROTO;
	cs->fw6.ipv6.flags |= IP6T_F_PROTO;

	if (is_exthdr(cs->fw6.ipv6.proto)
	    && (cs->fw6.ipv6.invflags & XT_INV_PROTO) == 0)
		fprintf(stderr,
			"Warning: never matched protocol: %s. "
			"use extension match instead.\n",
			cs->protocol);
}

void ipv4_post_parse(int command, struct iptables_command_state *cs,
		     struct xtables_args *args)
{
	cs->fw.ip.flags = args->flags;
	/* We already set invflags in proto_parse, but we need to refresh it
	 * to include new parsed options.
	 */
	cs->fw.ip.invflags = args->invflags;

	memcpy(cs->fw.ip.iniface, args->iniface, IFNAMSIZ);
	memcpy(cs->fw.ip.outiface, args->outiface, IFNAMSIZ);

	if (args->goto_set)
		cs->fw.ip.flags |= IPT_F_GOTO;

	/* nft-variants use cs->counters, legacy uses cs->fw.counters */
	cs->counters.pcnt = args->pcnt_cnt;
	cs->counters.bcnt = args->bcnt_cnt;
	cs->fw.counters.pcnt = args->pcnt_cnt;
	cs->fw.counters.bcnt = args->bcnt_cnt;

	if (command & (CMD_REPLACE | CMD_INSERT |
			CMD_DELETE | CMD_APPEND | CMD_CHECK)) {
		if (!(cs->options & OPT_DESTINATION))
			args->dhostnetworkmask = "0.0.0.0/0";
		if (!(cs->options & OPT_SOURCE))
			args->shostnetworkmask = "0.0.0.0/0";
	}

	if (args->shostnetworkmask)
		xtables_ipparse_multiple(args->shostnetworkmask,
					 &args->s.addr.v4, &args->s.mask.v4,
					 &args->s.naddrs);
	if (args->dhostnetworkmask)
		xtables_ipparse_multiple(args->dhostnetworkmask,
					 &args->d.addr.v4, &args->d.mask.v4,
					 &args->d.naddrs);

	if ((args->s.naddrs > 1 || args->d.naddrs > 1) &&
	    (cs->fw.ip.invflags & (IPT_INV_SRCIP | IPT_INV_DSTIP)))
		xtables_error(PARAMETER_PROBLEM,
			      "! not allowed with multiple"
			      " source or destination IP addresses");
}

void ipv6_post_parse(int command, struct iptables_command_state *cs,
		     struct xtables_args *args)
{
	cs->fw6.ipv6.flags = args->flags;
	/* We already set invflags in proto_parse, but we need to refresh it
	 * to include new parsed options.
	 */
	cs->fw6.ipv6.invflags = args->invflags;

	memcpy(cs->fw6.ipv6.iniface, args->iniface, IFNAMSIZ);
	memcpy(cs->fw6.ipv6.iniface_mask,
	       args->iniface_mask, IFNAMSIZ*sizeof(unsigned char));

	memcpy(cs->fw6.ipv6.outiface, args->outiface, IFNAMSIZ);
	memcpy(cs->fw6.ipv6.outiface_mask,
	       args->outiface_mask, IFNAMSIZ*sizeof(unsigned char));

	if (args->goto_set)
		cs->fw6.ipv6.flags |= IP6T_F_GOTO;

	/* nft-variants use cs->counters, legacy uses cs->fw6.counters */
	cs->counters.pcnt = args->pcnt_cnt;
	cs->counters.bcnt = args->bcnt_cnt;
	cs->fw6.counters.pcnt = args->pcnt_cnt;
	cs->fw6.counters.bcnt = args->bcnt_cnt;

	if (command & (CMD_REPLACE | CMD_INSERT |
			CMD_DELETE | CMD_APPEND | CMD_CHECK)) {
		if (!(cs->options & OPT_DESTINATION))
			args->dhostnetworkmask = "::0/0";
		if (!(cs->options & OPT_SOURCE))
			args->shostnetworkmask = "::0/0";
	}

	if (args->shostnetworkmask)
		xtables_ip6parse_multiple(args->shostnetworkmask,
					  &args->s.addr.v6,
					  &args->s.mask.v6,
					  &args->s.naddrs);
	if (args->dhostnetworkmask)
		xtables_ip6parse_multiple(args->dhostnetworkmask,
					  &args->d.addr.v6,
					  &args->d.mask.v6,
					  &args->d.naddrs);

	if ((args->s.naddrs > 1 || args->d.naddrs > 1) &&
	    (cs->fw6.ipv6.invflags & (IP6T_INV_SRCIP | IP6T_INV_DSTIP)))
		xtables_error(PARAMETER_PROBLEM,
			      "! not allowed with multiple"
			      " source or destination IP addresses");
}

unsigned char *
make_delete_mask(const struct xtables_rule_match *matches,
		 const struct xtables_target *target,
		 size_t entry_size)
{
	/* Establish mask for comparison */
	unsigned int size = entry_size;
	const struct xtables_rule_match *matchp;
	unsigned char *mask, *mptr;

	for (matchp = matches; matchp; matchp = matchp->next)
		size += XT_ALIGN(sizeof(struct xt_entry_match)) + matchp->match->size;

	mask = xtables_calloc(1, size
			 + XT_ALIGN(sizeof(struct xt_entry_target))
			 + target->size);

	memset(mask, 0xFF, entry_size);
	mptr = mask + entry_size;

	for (matchp = matches; matchp; matchp = matchp->next) {
		memset(mptr, 0xFF,
		       XT_ALIGN(sizeof(struct xt_entry_match))
		       + matchp->match->userspacesize);
		mptr += XT_ALIGN(sizeof(struct xt_entry_match)) + matchp->match->size;
	}

	memset(mptr, 0xFF,
	       XT_ALIGN(sizeof(struct xt_entry_target))
	       + target->userspacesize);

	return mask;
}

void xtables_clear_args(struct xtables_args *args)
{
	free(args->s.addr.ptr);
	free(args->s.mask.ptr);
	free(args->d.addr.ptr);
	free(args->d.mask.ptr);
}
