/*
    Copyright (c)  2006, 2007		Dmitry Butskoy
					<buc@citadel.stu.neva.ru>
    License:  GPL v2 or any later

    See COPYING for the status of this software.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/icmp6.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <netdb.h>
#include <errno.h>
#include <locale.h>
#include <sys/utsname.h>
#include <linux/types.h>
#include <linux/errqueue.h>

/*  XXX: Remove this when things will be defined properly in netinet/ ...  */
#include "flowlabel.h"

#include <clif.h>
#include "version.h"
#include "traceroute.h"


#ifndef ICMP6_DST_UNREACH_BEYONDSCOPE
#ifdef ICMP6_DST_UNREACH_NOTNEIGHBOR
#define ICMP6_DST_UNREACH_BEYONDSCOPE ICMP6_DST_UNREACH_NOTNEIGHBOR
#else
#define ICMP6_DST_UNREACH_BEYONDSCOPE 2
#endif
#endif

#ifndef IPV6_RECVHOPLIMIT
#define IPV6_RECVHOPLIMIT IPV6_HOPLIMIT
#endif

#ifndef IP_PMTUDISC_PROBE
#define IP_PMTUDISC_PROBE 3
#endif

#ifndef IPV6_PMTUDISC_PROBE
#define IPV6_PMTUDISC_PROBE 3
#endif

#ifndef AI_IDN
#define AI_IDN	0
#endif

#ifndef NI_IDN
#define NI_IDN	0
#endif


#define MAX_HOPS	255
#define MAX_PROBES	10
#define MAX_GATEWAYS_4	8
#define MAX_GATEWAYS_6	127
#define DEF_HOPS	30
#define DEF_SIM_PROBES	16	/*  including several hops   */
#define DEF_NUM_PROBES	3
#define DEF_WAIT_SECS	5.0
#define DEF_HERE_FACTOR	3
#define DEF_NEAR_FACTOR	10
#ifndef DEF_WAIT_PREC
#define DEF_WAIT_PREC	0.001	/*  +1 ms  to avoid precision issues   */
#endif
#define DEF_SEND_SECS	0
#define DEF_DATA_LEN	40	/*  all but IP header...  */
#define MAX_PACKET_LEN	65000
#ifndef DEF_AF
#define DEF_AF		AF_INET
#endif

#define ttl2hops(X)	(((X) <= 64 ? 65 : ((X) <= 128 ? 129 : 256)) - (X))


static char version_string[] = "Modern traceroute for Linux, "
				"version " _TEXT(VERSION)
				"\nCopyright (c) 2016  Dmitry Butskoy, "
				"  License: GPL v2 or any later";
static int debug = 0;
static unsigned int first_hop = 1;
static unsigned int max_hops = DEF_HOPS;
static unsigned int sim_probes = DEF_SIM_PROBES;
static unsigned int probes_per_hop = DEF_NUM_PROBES;

static char **gateways = NULL;
static int num_gateways = 0;
static unsigned char *rtbuf = NULL;
static size_t rtbuf_len = 0;
static unsigned int ipv6_rthdr_type = 2;	/*  IPV6_RTHDR_TYPE_2   */

static size_t header_len = 0;
static size_t data_len = 0;

static int dontfrag = 0;
static int noresolve = 0;
static int extension = 0;
static int as_lookups = 0;
static unsigned int dst_port_seq = 0;
static unsigned int tos = 0;
static unsigned int flow_label = 0;
static int noroute = 0;
static unsigned int fwmark = 0;
static int packet_len = -1;
static double wait_secs = DEF_WAIT_SECS;
static double here_factor = DEF_HERE_FACTOR;
static double near_factor = DEF_NEAR_FACTOR;
static double send_secs = DEF_SEND_SECS;
static int mtudisc = 0;
static int backward = 0;

static sockaddr_any dst_addr = {{ 0, }, };
static char *dst_name = NULL;
static char *device = NULL;
static sockaddr_any src_addr = {{ 0, }, };
static unsigned int src_port = 0;

static const char *module = "default";
static const tr_module *ops = NULL;

static char *opts[16] = { NULL, };	/*  assume enough   */
static unsigned int opts_idx = 1;	/*  first one reserved...   */


static int af = 0;

static probe *probes = NULL;
static unsigned int num_probes = 0;


static void ex_error (const char *format, ...) {
	va_list ap;

	va_start (ap, format);
	vfprintf (stderr, format, ap);
	va_end (ap);

	fprintf (stderr, "\n");

	exit (2);
}

void error (const char *str) {

	fprintf (stderr, "\n");

	perror (str);

	exit (1);
}

void error_or_perm (const char *str) {

	if (errno == EPERM)
		fprintf (stderr, "You do not have enough privileges to use "
				"this traceroute method.");
	error (str);
}


/*  Set initial parameters according to how we was called   */

static void check_progname (const char *name) {
	const char *p;
	int l;

	p = strrchr (name, '/');
	if (p)  p++;
	else  p = name;

	l = strlen (p);
	if (l <= 0)  return;
	l--;

	if (p[l] == '6')  af = AF_INET6;
	else if (p[l] == '4')  af = AF_INET;

	if (!strncmp (p, "tcp", 3))
		module = "tcp";
	if (!strncmp (p, "tracert", 7))
		module = "icmp";

	return;
}


static int getaddr (const char *name, sockaddr_any *addr) {
	int ret;
	struct addrinfo hints, *ai, *res = NULL;

	memset (&hints, 0, sizeof (hints));
	hints.ai_family = af;
#ifdef AI_FLAGS
	hints.ai_flags = AI_IDN;
#endif

	ret = getaddrinfo (name, NULL, &hints, &res);
	if (ret) {
		fprintf (stderr, "%s: %s\n", name, gai_strerror (ret));
		return -1;
	}

	for (ai = res; ai; ai = ai->ai_next) {
	    if (ai->ai_family == af)  break;
	    /*  when af not specified, choose DEF_AF if present   */
	    if (!af && ai->ai_family == DEF_AF)
		    break;
	}
	if (!ai)  ai = res;	/*  anything...  */

	if (ai->ai_addrlen > sizeof (*addr))
		return -1;	/*  paranoia   */
	memcpy (addr, ai->ai_addr, ai->ai_addrlen);

	freeaddrinfo (res);

	return 0;
}


static void make_fd_used (int fd) {
	int nfd;

	if (fcntl (fd, F_GETFL) != -1)
		return;

	if (errno != EBADF)
		error ("fcntl F_GETFL");

	nfd = open ("/dev/null", O_RDONLY);
	if (nfd < 0)  error ("open /dev/null");

	if (nfd != fd) {
	    dup2 (nfd, fd);
	    close (nfd);
	}

	return;
}


static char addr2str_buf[INET6_ADDRSTRLEN];

static const char *addr2str (const sockaddr_any *addr) {

	getnameinfo (&addr->sa, sizeof (*addr),
		addr2str_buf, sizeof (addr2str_buf), 0, 0, NI_NUMERICHOST);

	return addr2str_buf;
}


/*	IP  options  stuff	    */

static void init_ip_options (void) {
	sockaddr_any *gates;
	int i, max;

	if (!num_gateways)
		return;

	/*  check for TYPE,ADDR,ADDR... form   */
	if (af == AF_INET6 && num_gateways > 1 && gateways[0]) {
	    char *q;
	    unsigned int value = strtoul (gateways[0], &q, 0);

	    if (!*q) {
		ipv6_rthdr_type = value;
		num_gateways--;
		for (i = 0; i < num_gateways; i++)
			gateways[i] = gateways[i + 1];
	    }
	}


	max = af == AF_INET ? MAX_GATEWAYS_4 : MAX_GATEWAYS_6;
	if (num_gateways > max)
	    ex_error ("Too many gateways specified. No more than %d", max);


	gates = alloca (num_gateways * sizeof (*gates));

	for (i = 0; i < num_gateways; i++) {

	    if (!gateways[i])  error ("strdup");

	    if (getaddr (gateways[i], &gates[i]) < 0)
		    ex_error ("");	/*  already reported   */
	    if (gates[i].sa.sa_family != af)
		    ex_error ("IP versions mismatch in gateway addresses");

	    free (gateways[i]);
	}

	free (gateways);
	gateways = NULL;


	if (af == AF_INET) {
	    struct in_addr *in;

	    rtbuf_len = 4 + (num_gateways + 1) * sizeof (*in);
	    rtbuf = malloc (rtbuf_len);
	    if (!rtbuf)  error ("malloc");

	    in = (struct in_addr *) &rtbuf[4];
	    for (i = 0; i < num_gateways; i++)
		    memcpy (&in[i], &gates[i].sin.sin_addr, sizeof (*in));
	    /*  final hop   */
	    memcpy (&in[i], &dst_addr.sin.sin_addr, sizeof (*in));
	    i++;

	    rtbuf[0] = IPOPT_NOP;
	    rtbuf[1] = IPOPT_LSRR;
	    rtbuf[2] = (i * sizeof (*in)) + 3;
	    rtbuf[3] = IPOPT_MINOFF;

	}
	else if (af == AF_INET6) {
	    struct in6_addr *in6;
	    struct ip6_rthdr *rth;

	    /*  IPV6_RTHDR_TYPE_0 length is 8   */
	    rtbuf_len = 8 + num_gateways * sizeof (*in6);
	    rtbuf = malloc (rtbuf_len);
	    if (!rtbuf)  error ("malloc");

	    rth = (struct ip6_rthdr *) rtbuf;
	    rth->ip6r_nxt = 0;
	    rth->ip6r_len = 2 * num_gateways;
	    rth->ip6r_type = ipv6_rthdr_type;
	    rth->ip6r_segleft = num_gateways;

	    *((uint32_t *) (rth + 1)) = 0;

	    in6 = (struct in6_addr *) (rtbuf + 8);
	    for (i = 0; i < num_gateways; i++)
		    memcpy (&in6[i], &gates[i].sin6.sin6_addr, sizeof (*in6));
	}

	return;
}


/*	Command line stuff	    */

static int set_af (CLIF_option *optn, char *arg) {
	int vers = (long) optn->data;

	if (vers == 4)  af = AF_INET;
	else if (vers == 6)  af = AF_INET6;
	else
	    return -1;

	return 0;
}

static int add_gateway (CLIF_option *optn, char *arg) {

	if (num_gateways >= MAX_GATEWAYS_6) {	/*  127 > 8 ... :)   */
		fprintf (stderr, "Too many gateways specified.");
		return -1;
	}

	gateways = realloc (gateways, (num_gateways + 1) * sizeof (*gateways));
	if (!gateways)  error ("malloc");
	gateways[num_gateways++] = strdup (arg);

	return 0;
}

static int set_source (CLIF_option *optn, char *arg) {

	return  getaddr (arg, &src_addr);
}

static int set_port (CLIF_option *optn, char *arg) {
	unsigned int *up = (unsigned int *) optn->data;
	char *q;

	*up = strtoul (arg, &q, 0);
	if (q == arg) {
	    struct servent *s = getservbyname (arg, NULL);

	    if (!s)  return -1;
	    *up = ntohs (s->s_port);
	}

	return 0;
}

static int set_module (CLIF_option *optn, char *arg) {

	module = (char *) optn->data;

	return 0;
}


static int set_mod_option (CLIF_option *optn, char *arg) {

	if (!strcmp (arg, "help")) {
	    const tr_module *mod = tr_get_module (module);

	    if (mod && mod->options) {
		/*  just to set common keyword flag...  */
		CLIF_parse (1, &arg, 0, 0, CLIF_KEYWORD);
		CLIF_print_options (NULL, mod->options);
	    } else
		fprintf (stderr, "No options for module `%s'\n", module);

	    exit (0);
	}

	if (opts_idx >= sizeof (opts) / sizeof (*opts))  {
	    fprintf (stderr, "Too many module options\n");
	    return -1;
	}

	opts[opts_idx] = strdup (arg);
	if (!opts[opts_idx])  error ("strdup");
	opts_idx++;

	return 0;
}


static int set_raw (CLIF_option *optn, char *arg) {
	char buf[1024];

	module = "raw";

	snprintf (buf, sizeof (buf), "protocol=%s", arg);
	return  set_mod_option (optn, buf);
}


static int set_wait_specs (CLIF_option *optn, char *arg) {
	char *p, *q;

	here_factor = near_factor = 0;

	wait_secs = strtod (p = arg, &q);
	if (q == p)  return -1;
	if (!*q++)  return 0;

	here_factor = strtod (p = q, &q);
	if (q == p)  return -1;
	if (!*q++)  return 0;

	near_factor = strtod (p = q, &q);
	if (q == p || *q)  return -1;

	return 0;
}


static int set_host (CLIF_argument *argm, char *arg, int index) {

	if (getaddr (arg, &dst_addr) < 0)
		return -1;

	dst_name = arg;

	/*  i.e., guess it by the addr in cmdline...  */
	if (!af)  af = dst_addr.sa.sa_family;

	return 0;
}


static CLIF_option option_list[] = {
	{ "4", 0, 0, "Use IPv4", set_af, (void *) 4, 0, CLIF_EXTRA },
	{ "6", 0, 0, "Use IPv6", set_af, (void *) 6, 0, 0 },
	{ "d", "debug", 0, "Enable socket level debugging",
			CLIF_set_flag, &debug, 0, 0 },
	{ "F", "dont-fragment", 0, "Do not fragment packets",
			CLIF_set_flag, &dontfrag, 0, CLIF_ABBREV },
	{ "f", "first", "first_ttl", "Start from the %s hop (instead from 1)",
			CLIF_set_uint, &first_hop, 0, 0 },
	{ "g", "gateway", "gate", "Route packets through the specified gateway "
			    "(maximum " _TEXT(MAX_GATEWAYS_4) " for IPv4 and "
			    _TEXT(MAX_GATEWAYS_6) " for IPv6)",
			add_gateway, 0, 0, CLIF_SEVERAL },
	{ "I", "icmp", 0, "Use ICMP ECHO for tracerouting",
			set_module, "icmp", 0, 0 },
	{ "T", "tcp", 0, "Use TCP SYN for tracerouting (default "
			"port is " _TEXT(DEF_TCP_PORT) ")",
			set_module, "tcp", 0, 0 },
	{ "i", "interface", "device", "Specify a network interface "
			    "to operate with",
			CLIF_set_string, &device, 0, 0 },
	{ "m", "max-hops", "max_ttl", "Set the max number of hops (max TTL "
			    "to be reached). Default is " _TEXT(DEF_HOPS) ,
			CLIF_set_uint, &max_hops, 0, 0 },
	{ "N", "sim-queries", "squeries", "Set the number of probes "
			    "to be tried simultaneously (default is "
			    _TEXT(DEF_SIM_PROBES) ")",
			CLIF_set_uint, &sim_probes, 0, 0 },
	{ "n", 0, 0, "Do not resolve IP addresses to their domain names",
			CLIF_set_flag, &noresolve, 0, 0 },
	{ "p", "port", "port", "Set the destination port to use. "
			    "It is either initial udp port value for "
			    "\"default\" method (incremented by each probe, "
			    "default is " _TEXT(DEF_START_PORT) "), "
			    "or initial seq for \"icmp\" (incremented as well, "
			    "default from 1), or some constant destination port"
			    " for other methods (with default of "
			    _TEXT(DEF_TCP_PORT) " for \"tcp\", "
			    _TEXT(DEF_UDP_PORT) " for \"udp\", etc.)",
			    set_port, &dst_port_seq, 0, 0 },
	{ "t", "tos", "tos", "Set the TOS (IPv4 type of service) or TC "
			    "(IPv6 traffic class) value for outgoing packets",
			    CLIF_set_uint, &tos, 0, 0 },
	{ "l", "flowlabel", "flow_label", "Use specified %s for IPv6 packets",
			    CLIF_set_uint, &flow_label, 0, 0 },
	{ "w", "wait", "MAX,HERE,NEAR", "Wait for a probe no more than HERE "
			    "(default " _TEXT(DEF_HERE_FACTOR) ") times longer "
			    "than a response from the same hop, or no more "
			    "than NEAR (default " _TEXT(DEF_NEAR_FACTOR) ") "
			    "times than some next hop, or MAX (default "
			    _TEXT(DEF_WAIT_SECS) ") seconds "
			    "(float point values allowed too)",
			    set_wait_specs, 0, 0, 0 },
	{ "q", "queries", "nqueries", "Set the number of probes per each hop. "
			    "Default is " _TEXT(DEF_NUM_PROBES),
			    CLIF_set_uint, &probes_per_hop, 0, 0 },
	{ "r", 0, 0, "Bypass the normal routing and send directly to a host "
			    "on an attached network",
			    CLIF_set_flag, &noroute, 0, 0 },
	{ "s", "source", "src_addr", "Use source %s for outgoing packets",
			    set_source, 0, 0, 0 },
	{ "z", "sendwait", "sendwait", "Minimal time interval between probes "
			    "(default " _TEXT(DEF_SEND_SECS) "). If the value "
			    "is more than 10, then it specifies a number "
			    "in milliseconds, else it is a number of seconds "
			    "(float point values allowed too)",
			    CLIF_set_double, &send_secs, 0, 0 },
	{ "e", "extensions", 0, "Show ICMP extensions (if present), "
			    "including MPLS",
			    CLIF_set_flag, &extension, 0, CLIF_ABBREV },
	{ "A", "as-path-lookups", 0, "Perform AS path lookups in routing "
			    "registries and print results directly after "
			    "the corresponding addresses",
			    CLIF_set_flag, &as_lookups, 0, 0 },
	{ "M", "module", "name", "Use specified module (either builtin or "
			    "external) for traceroute operations. Most methods "
			    "have their shortcuts (`-I' means `-M icmp' etc.)",
			    CLIF_set_string, &module, 0, CLIF_EXTRA },
	{ "O", "options", "OPTS", "Use module-specific option %s for the "
			    "traceroute module. Several %s allowed, separated "
			    "by comma. If %s is \"help\", print info about "
			    "available options",
			    set_mod_option, 0, 0, CLIF_SEVERAL | CLIF_EXTRA },
	{ 0, "sport", "num", "Use source port %s for outgoing packets. "
			    "Implies `-N 1'",
			    set_port, &src_port, 0, CLIF_EXTRA },
#ifdef SO_MARK
	{ 0, "fwmark", "num", "Set firewall mark for outgoing packets",
			    CLIF_set_uint, &fwmark, 0, 0 },
#endif
	{ "U", "udp", 0, "Use UDP to particular port for tracerouting "
			    "(instead of increasing the port per each probe), "
			    "default port is " _TEXT(DEF_UDP_PORT),
			    set_module, "udp", 0, CLIF_EXTRA },
	{ 0, "UL", 0, "Use UDPLITE for tracerouting (default dest port is "
			    _TEXT(DEF_UDP_PORT) ")",
			    set_module, "udplite", 0, CLIF_ONEDASH|CLIF_EXTRA },
	{ "D", "dccp", 0, "Use DCCP Request for tracerouting (default "
			    "port is " _TEXT(DEF_DCCP_PORT) ")",
			    set_module, "dccp", 0, CLIF_EXTRA },
	{ "P", "protocol", "prot", "Use raw packet of protocol %s "
			    "for tracerouting", 
			    set_raw, 0, 0, CLIF_EXTRA },
	{ 0, "mtu", 0, "Discover MTU along the path being traced. "
			    "Implies `-F -N 1'",
			    CLIF_set_flag, &mtudisc, 0, CLIF_EXTRA },
	{ 0, "back", 0, "Guess the number of hops in the backward path "
			    "and print if it differs",
			    CLIF_set_flag, &backward, 0, CLIF_EXTRA },
	CLIF_VERSION_OPTION (version_string),
	CLIF_HELP_OPTION,
	CLIF_END_OPTION
};

static CLIF_argument arg_list[] = {
        { "host", "The host to traceroute to",
				set_host, 0, CLIF_STRICT },
	{ "packetlen", "The full packet length (default is the length of "
			"an IP header plus " _TEXT(DEF_DATA_LEN) "). Can be "
			"ignored or increased to a minimal allowed value",
				CLIF_arg_int, &packet_len, 0 },
	CLIF_END_ARGUMENT
};


static void do_it (void);

int main (int argc, char *argv[]) {

	setlocale (LC_ALL, "");
	setlocale (LC_NUMERIC, "C");	/*  avoid commas in msec printed  */

	check_progname (argv[0]);


	if (CLIF_parse (argc, argv, option_list, arg_list,
				CLIF_MAY_JOIN_ARG | CLIF_HELP_EMPTY) < 0
	)  exit (2);


	ops = tr_get_module (module);
	if (!ops)  ex_error ("Unknown traceroute module %s", module);


	if (!first_hop || first_hop > max_hops)
		ex_error ("first hop out of range");
	if (max_hops > MAX_HOPS)
		ex_error ("max hops cannot be more than " _TEXT(MAX_HOPS));
	if (!probes_per_hop || probes_per_hop > MAX_PROBES)
		ex_error ("no more than " _TEXT(MAX_PROBES) " probes per hop");
	if (wait_secs < 0 || here_factor < 0 || near_factor < 0)
		ex_error ("bad wait specifications `%g,%g,%g' used",
				    wait_secs, here_factor, near_factor);
	if (packet_len > MAX_PACKET_LEN)
		ex_error ("too big packetlen %d specified", packet_len);
	if (src_addr.sa.sa_family && src_addr.sa.sa_family != af)
		ex_error ("IP version mismatch in addresses specified");
	if (send_secs < 0)
		ex_error ("bad sendtime `%g' specified", send_secs);
	if (send_secs >= 10)	/*  it is milliseconds   */
		send_secs /= 1000;

	if (af == AF_INET6 && (tos || flow_label))
		dst_addr.sin6.sin6_flowinfo =
		    htonl (((tos & 0xff) << 20) | (flow_label & 0x000fffff));

	if (src_port) {
	    src_addr.sin.sin_port = htons ((uint16_t) src_port);
	    src_addr.sa.sa_family = af;
	}

	if (src_port || ops->one_per_time) {
		sim_probes = 1;
		here_factor = near_factor = 0;
	}


	/*  make sure we don't std{in,out,err} to open sockets  */
	make_fd_used (0);
	make_fd_used (1);
	make_fd_used (2);


	init_ip_options ();

	header_len = (af == AF_INET ? sizeof (struct iphdr)
				    : sizeof (struct ip6_hdr)) +
			rtbuf_len + ops->header_len;

	if (mtudisc) {
	    dontfrag = 1;
	    sim_probes = 1;
	    if (packet_len < 0)
		    packet_len = MAX_PACKET_LEN;
	}

	if (packet_len < 0) {
	    if (DEF_DATA_LEN >= ops->header_len)
		    data_len = DEF_DATA_LEN - ops->header_len;
	} else {
	    if (packet_len >= header_len)
		    data_len = packet_len - header_len;
	}


	num_probes = max_hops * probes_per_hop;
	probes = calloc (num_probes, sizeof (*probes));
	if (!probes)  error ("calloc");


	if (ops->options && opts_idx > 1) {
	    opts[0] = strdup (module);	    /*  aka argv[0] ...  */
	    if (CLIF_parse (opts_idx, opts, ops->options, 0, CLIF_KEYWORD) < 0)
		    exit (2);
	}

	if (ops->init (&dst_addr, dst_port_seq, &data_len) < 0)
		ex_error ("trace method's init failed");


	do_it ();

	return 0;
}


/*	PRINT  STUFF	    */

static void print_header (void) {

	/*  Note, without ending new-line!  */
	printf ("traceroute to %s (%s), %u hops max, %zu byte packets",
				dst_name, addr2str (&dst_addr), max_hops,
				header_len + data_len);
	fflush (stdout);
}


static void print_addr (sockaddr_any *res) {
	const char *str;

	if (!res->sa.sa_family)
		return;

	str = addr2str (res);


	if (noresolve)
		printf (" %s", str);
	else {
	    char buf[1024];

	    buf[0] = '\0';
	    getnameinfo (&res->sa, sizeof (*res), buf, sizeof (buf),
							    0, 0, NI_IDN);
	    printf (" %s (%s)", buf[0] ? buf : str, str);
	}

	if (as_lookups)
		printf (" [%s]", get_as_path (str));
}


static void print_probe (probe *pb) {
	unsigned int idx = (pb - probes);
	unsigned int ttl = idx / probes_per_hop + 1;
	unsigned int np = idx % probes_per_hop;

	if (np == 0)
		printf ("\n%2u ", ttl);


	if (!pb->res.sa.sa_family)
		printf (" *");
	else {
	    int prn = !np;	/*  print if the first...  */

	    if (np) {	    /*  ...and if differs with previous   */
		probe *p;

		/*  skip expired   */
		for (p = pb - 1; np && !p->res.sa.sa_family; p--, np--) ;

		if (!np ||
		    !equal_addr (&p->res, &pb->res) ||
		    (p->ext != pb->ext &&
			!(p->ext && pb->ext && !strcmp (p->ext, pb->ext))) ||
		    (backward && p->recv_ttl != pb->recv_ttl)
		)  prn = 1;
	    }

	    if (prn) {
		print_addr (&pb->res);

		if (pb->ext)  printf (" <%s>", pb->ext);

		if (backward && pb->recv_ttl) {
		    int hops = ttl2hops (pb->recv_ttl);
		    if (hops != ttl)  printf (" '-%d'", hops);
		}
	    }
	}


	if (pb->recv_time) {
	    double diff = pb->recv_time - pb->send_time;

	    printf ("  %.3f ms", diff * 1000);
	}

	if (pb->err_str[0])
		printf (" %s", pb->err_str);


	fflush (stdout);

	return;
}


static void print_end (void) {

	printf ("\n");
}


/*	Compute  timeout  stuff		*/

static double get_timeout (probe *pb) {
	double value;

	if (here_factor) {
	    /*  check for already replied from the same hop   */
	    int i, idx = (pb - probes);
	    probe *p = &probes[idx - (idx % probes_per_hop)];

	    for (i = 0; i < probes_per_hop; i++, p++) {
		/*   `p == pb' skipped since  !pb->done   */

		if (p->done && (value = p->recv_time - p->send_time) > 0) {
		    value += DEF_WAIT_PREC;
		    value *= here_factor;
		    return  value < wait_secs ? value : wait_secs;
		}
	    }
	}

	if (near_factor) {
	    /*  check forward for already replied   */
	    probe *p, *endp = probes + num_probes;

	    for (p = pb + 1; p < endp && p->send_time; p++) {

		if (p->done && (value = p->recv_time - p->send_time) > 0) {
		    value += DEF_WAIT_PREC;
		    value *= near_factor;
		    return  value < wait_secs ? value : wait_secs;
		}
	    }
	}

	return wait_secs;
}


/*	Check  expiration  stuff	*/

static void check_expired (probe *pb) {
	int idx = (pb - probes);
	probe *p, *endp = probes + num_probes;
	probe *fp = NULL, *pfp = NULL;

	if (!pb->done)	    /*  an ops method still not release it  */
	    return;


	/*  check all the previous in the same hop   */
	for (p = &probes[idx - (idx % probes_per_hop)]; p < pb; p++) {

	    if (!p->done ||     /*  too early to decide something  */
		!p->final       /*  already ttl-exceeded in the same hop  */
	    )  return;

	    pfp = p;	/*  some of the previous probes is final   */
	}

	/*  check forward all the sent probes   */
	for (p = pb + 1; p < endp && p->send_time; p++) {

	    if (p->done) {	/*  some next probe already done...  */
		if (!p->final)	/*  ...was ttl-exceeded. OK, we are expired.  */
		    return;
		else {
		    fp = p;
		    break;
		}
	    }
	}

	if (!fp)    /*  no any final probe found. Assume expired.   */
	    return;


	/*  Well. There is a situation "*(this) * * * * ... * * final"
	   We cannot guarantee that "final" is in its right place.
	   We've sent "sim_probes" simultaneously, and the final hop
	   can drop some of them and answer only for latest ones.
	   If we can detect/assume that it so, then just put "final"
	   to the (pseudo-expired) "this" place.
	*/

	/*  It seems that the case of "answers for latest ones only"
	   occurs mostly with icmp_unreach error answers ("!H" etc.).
	   Icmp_echoreply, tcp_reset and even icmp_port_unreach looks
	   like going in the right order.
 	*/
	if (!fp->err_str[0])	/*  not an icmp_unreach error report...  */
		return;


	if (pfp ||
	    (idx % probes_per_hop) + (fp - pb) < probes_per_hop
	) {
	    /*  Either some previous (pfp) or some next probe
		in this hop is final. It means that the whole hop is final.
		Do the replace (it also causes further "final"s to be shifted
		here too).
	    */
	    goto  replace_by_final;
	}


	/*  If the final probe is an icmp_unreachable report
	    (either in a case of some error, like "!H", or just port_unreach),
	    it could follow the "time-exceed" report from the *same* hop.
	*/
	for (p = pb - 1; p >= probes; p--) {
	    if (equal_addr (&p->res, &fp->res)) {
		/*  ...Yes. Put "final" to the "this" place.  */
		goto  replace_by_final;
	    }
	}


	if (fp->recv_ttl) {
	    /*  Consider the ttl value of the report packet and guess where
		the "final" should be. If it seems that it should be
		in the same hop as "this", then do replace.
	    */
	    int back_hops, ttl;

	    /*  We assume that the reporting one has an initial ttl value
		of either 64, or 128, or 255. It is most widely used
		in the modern routers and computers.
		The idea comes from tracepath(1) routine.
	    */
	    back_hops = ttl2hops (fp->recv_ttl);

	    /*  It is possible that the back path differs from the forward
		and therefore has different number of hops. To minimize
		such an influence, get the nearest previous time-exceeded
		probe and compare with it.
	    */
	    for (p = pb - 1; p >= probes; p--) {
		if (p->done && !p->final && p->recv_ttl) {
		    int hops = ttl2hops (p->recv_ttl);

		    if (hops < back_hops) {
			ttl = (p - probes) / probes_per_hop + 1;
			back_hops = (back_hops - hops) + ttl;
			break;
		    }
		}
	    }

	    ttl = idx / probes_per_hop + 1;
	    if (back_hops == ttl)
		/*  Yes! It seems that "final" should be at "this" place   */
		goto  replace_by_final;
	    else if (back_hops < ttl)
		/*  Hmmm... Assume better to replace here too...  */
		goto  replace_by_final;

	}


	/*  No idea what to do. Assume expired.  */

	return;


replace_by_final:

	*pb = *fp;

	memset (fp, 0, sizeof (*fp));
	/*  block extra re-send  */
	fp->send_time = 1.;

	return;
}


probe *probe_by_seq (int seq) {
	int n;

	if (seq <= 0)  return NULL;

	for (n = 0; n < num_probes; n++) {
	    if (probes[n].seq == seq)
		    return &probes[n];
	}

	return NULL;
}

probe *probe_by_sk (int sk) {
	int n;

	if (sk <= 0)  return NULL;

	for (n = 0; n < num_probes; n++) {
	    if (probes[n].sk == sk)
		    return &probes[n];
	}

	return NULL;
}


static void poll_callback (int fd, int revents) {

	ops->recv_probe (fd, revents);
}


static void do_it (void) {
	int start = (first_hop - 1) * probes_per_hop;
	int end = num_probes;
	double last_send = 0;

	print_header ();


	while (start < end) {
	    int n, num = 0;
	    double next_time = 0;
	    double now_time = get_time ();


	    for (n = start; n < end; n++) {
		probe *pb = &probes[n];


		if (n == start &&		/*  probably time to print...  */
		    !pb->done && pb->send_time	/*  ...but yet not replied   */
		) {
		    double expire_time = pb->send_time + get_timeout (pb);

		    if (expire_time > now_time)
			    next_time = expire_time;
		    else {
			ops->expire_probe (pb);
			check_expired (pb);
		    }
		}


		if (pb->done) {

		    if (n == start) {	/*  can print it now   */
			print_probe (pb);
			start++;
		    }

		    if (pb->final)
			end = (n / probes_per_hop + 1) * probes_per_hop;

		    continue;
		}


		if (!pb->send_time) {
		    int ttl;
		    double next;

		    if (send_secs && (next = last_send + send_secs) > now_time) {
			next_time = next;
			break;
		    }

		    ttl = n / probes_per_hop + 1;

		    ops->send_probe (pb, ttl);

		    if (!pb->send_time) {
			if (next_time)  break;	/*  have chances later   */
			else  error ("send probe");
		    }

		    last_send = pb->send_time;
		}


		if (!next_time)
		    next_time = pb->send_time + get_timeout (pb);

		num++;
		if (num >= sim_probes)  break;
	    }


	    if (next_time) {
		double timeout = next_time - get_time ();

		if (timeout < 0)  timeout = 0;

		do_poll (timeout, poll_callback);
	    }

	}


	print_end ();

	return;
}


void tune_socket (int sk) {
	int i = 0;

	if (debug) {
	    i = 1;
	    if (setsockopt (sk, SOL_SOCKET, SO_DEBUG, &i, sizeof (i)) < 0)
		    error ("setsockopt SO_DEBUG");
	}


#ifdef SO_MARK
	if (fwmark) {
	    if (setsockopt (sk, SOL_SOCKET, SO_MARK,
					&fwmark, sizeof (fwmark)) < 0
	    )  error ("setsockopt SO_MARK");
	}
#endif


	if (rtbuf && rtbuf_len) {
	    if (af == AF_INET) {
		if (setsockopt (sk, IPPROTO_IP, IP_OPTIONS,
						rtbuf, rtbuf_len) < 0
		)  error ("setsockopt IP_OPTIONS");
	    }
	    else if (af == AF_INET6) {
		if (setsockopt (sk, IPPROTO_IPV6, IPV6_RTHDR,
						rtbuf, rtbuf_len) < 0
		)  error ("setsockopt IPV6_RTHDR");
	    }
	}


	bind_socket (sk);


	if (af == AF_INET) {

	    i = dontfrag ? IP_PMTUDISC_PROBE : IP_PMTUDISC_DONT;
	    if (setsockopt (sk, SOL_IP, IP_MTU_DISCOVER, &i, sizeof(i)) < 0 &&
		(!dontfrag || (i = IP_PMTUDISC_DO,
		 setsockopt (sk, SOL_IP, IP_MTU_DISCOVER, &i, sizeof(i)) < 0))
	    )  error ("setsockopt IP_MTU_DISCOVER");

	    if (tos) {
		i = tos;
		if (setsockopt (sk, SOL_IP, IP_TOS, &i, sizeof (i)) < 0)
			error ("setsockopt IP_TOS");
	    }

	}
	else if (af == AF_INET6) {

	    i = dontfrag ? IPV6_PMTUDISC_PROBE : IPV6_PMTUDISC_DONT;
	    if (setsockopt (sk, SOL_IPV6, IPV6_MTU_DISCOVER,&i,sizeof(i)) < 0 &&
		(!dontfrag || (i = IPV6_PMTUDISC_DO,
		 setsockopt (sk, SOL_IPV6, IPV6_MTU_DISCOVER,&i,sizeof(i)) < 0))
	    )  error ("setsockopt IPV6_MTU_DISCOVER");


	    if (flow_label) {
		struct in6_flowlabel_req flr;

		memset (&flr, 0, sizeof (flr));
		flr.flr_label = htonl (flow_label & 0x000fffff);
                flr.flr_action = IPV6_FL_A_GET;
                flr.flr_flags = IPV6_FL_F_CREATE;
                flr.flr_share = IPV6_FL_S_ANY;
		memcpy (&flr.flr_dst, &dst_addr.sin6.sin6_addr,
						    sizeof (flr.flr_dst));

		if (setsockopt (sk, IPPROTO_IPV6, IPV6_FLOWLABEL_MGR,
						    &flr, sizeof (flr)) < 0
		)  error ("setsockopt IPV6_FLOWLABEL_MGR");
	    }

	    if (tos) {
		i = tos;
		if (setsockopt (sk, IPPROTO_IPV6, IPV6_TCLASS,
						    &i, sizeof (i)) < 0
		)  error ("setsockopt IPV6_TCLASS");
	    }

	    if (tos || flow_label) {
		i = 1;
		if (setsockopt (sk, IPPROTO_IPV6, IPV6_FLOWINFO_SEND,
							&i, sizeof (i)) < 0
		)  error ("setsockopt IPV6_FLOWINFO_SEND");
	    }
	}
  

	if (noroute) {
	    i = noroute;
	    if (setsockopt (sk, SOL_SOCKET, SO_DONTROUTE, &i, sizeof (i)) < 0)
		    error ("setsockopt SO_DONTROUTE");
	}


	use_timestamp (sk);

	use_recv_ttl (sk);

	fcntl (sk, F_SETFL, O_NONBLOCK);

	return;
}


void parse_icmp_res (probe *pb, int type, int code, int info) {
	char *str = NULL;
	char buf[sizeof (pb->err_str)];

	if (af == AF_INET) {

	    if (type == ICMP_TIME_EXCEEDED) {
		if (code == ICMP_EXC_TTL)
			return;
	    }
	    else if (type == ICMP_DEST_UNREACH) {

		switch (code) {
		    case ICMP_UNREACH_NET:
		    case ICMP_UNREACH_NET_UNKNOWN:
		    case ICMP_UNREACH_ISOLATED:
		    case ICMP_UNREACH_TOSNET:
			    str = "!N";
			    break;

		    case ICMP_UNREACH_HOST:
		    case ICMP_UNREACH_HOST_UNKNOWN:
		    case ICMP_UNREACH_TOSHOST:
			    str = "!H";
			    break;

		    case ICMP_UNREACH_NET_PROHIB:
		    case ICMP_UNREACH_HOST_PROHIB:
		    case ICMP_UNREACH_FILTER_PROHIB:
			    str = "!X";
			    break;

		    case ICMP_UNREACH_PORT:
			    /*  dest host is reached   */
			    str = "";
			    break;

		    case ICMP_UNREACH_PROTOCOL:
			    str = "!P";
			    break;

		    case ICMP_UNREACH_NEEDFRAG:
			    snprintf (buf, sizeof (buf), "!F-%d", info);
			    str = buf;
			    break;

		    case ICMP_UNREACH_SRCFAIL:
			    str = "!S";
			    break;

		    case ICMP_UNREACH_HOST_PRECEDENCE:
			    str = "!V";
			    break;

		    case ICMP_UNREACH_PRECEDENCE_CUTOFF:
			    str = "!C";
			    break;

		    default:
			    snprintf (buf, sizeof (buf), "!<%u>", code);
			    str = buf;
			    break;
		}
	    }

	}
	else if (af == AF_INET6) {

	    if (type == ICMP6_TIME_EXCEEDED) {
		if (code == ICMP6_TIME_EXCEED_TRANSIT)
			return;
	    }
	    else if (type == ICMP6_DST_UNREACH) {

		switch (code) {

		    case ICMP6_DST_UNREACH_NOROUTE:
			    str = "!N";
			    break;

		    case ICMP6_DST_UNREACH_BEYONDSCOPE:
		    case ICMP6_DST_UNREACH_ADDR:
			    str = "!H";
			    break;

		    case ICMP6_DST_UNREACH_ADMIN:
			    str = "!X";
			    break;

		    case ICMP6_DST_UNREACH_NOPORT:
			    /*  dest host is reached   */
			    str = "";
			    break;

		    default:
			    snprintf (buf, sizeof (buf), "!<%u>", code);
			    str = buf;
			    break;
		}
	    }
	    else if (type == ICMP6_PACKET_TOO_BIG) {
		snprintf (buf, sizeof (buf), "!F-%d", info);
		str = buf;
	    }
	}


	if (!str) {
	    snprintf (buf, sizeof (buf), "!<%u-%u>", type, code);
	    str = buf;
	}

	if (*str) {
	    strncpy (pb->err_str, str, sizeof (pb->err_str));
	    pb->err_str[sizeof (pb->err_str) - 1] = '\0';
	}

	pb->final = 1;

	return;
}


static void parse_local_res (probe *pb, int ee_errno, int info) {

	if (ee_errno == EMSGSIZE && info != 0) {
	    snprintf (pb->err_str, sizeof(pb->err_str)-1, "!F-%d", info);
	    pb->final = 1;
	    return;
	}

	errno = ee_errno;
	error ("local recverr");
}


void probe_done (probe *pb) {

	if (pb->sk) {
	    del_poll (pb->sk);
	    close (pb->sk);
	    pb->sk = 0;
	}

	pb->seq = 0;

	pb->done = 1;
}


void recv_reply (int sk, int err, check_reply_t check_reply) {
	struct msghdr msg;
	sockaddr_any from;
	struct iovec iov;
	int n;
	probe *pb;
	char buf[1280];		/*  min mtu for ipv6 ( >= 576 for ipv4)  */
	char *bufp = buf;
	char control[1024];
	struct cmsghdr *cm;
	double recv_time = 0;
	int recv_ttl = 0;
	struct sock_extended_err *ee = NULL;


	memset (&msg, 0, sizeof (msg));
	msg.msg_name = &from;
	msg.msg_namelen = sizeof (from);
	msg.msg_control = control;
	msg.msg_controllen = sizeof (control);
	iov.iov_base = buf;
	iov.iov_len = sizeof (buf);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;


	n = recvmsg (sk, &msg, err ? MSG_ERRQUEUE : 0);
	if (n < 0)  return;


	/*  when not MSG_ERRQUEUE, AF_INET returns full ipv4 header
	    on raw sockets...
	*/

	if (!err &&
	    af == AF_INET &&
	    /*  XXX: Assume that the presence of an extra header means
		that it is not a raw socket...
	    */
	    ops->header_len == 0
	) {
	    struct iphdr *ip = (struct iphdr *) bufp;
	    int hlen;

	    if (n < sizeof (struct iphdr))  return;

	    hlen = ip->ihl << 2;
	    if (n < hlen)  return;

	    bufp += hlen;
	    n -= hlen;
	}


	pb = check_reply (sk, err, &from, bufp, n);
	if (!pb) {

	    /*  for `frag needed' case at the local host,
		kernel >= 3.13 sends local error (no more icmp)
	    */
	    if (!n && err && dontfrag) {
		pb = &probes[(first_hop - 1) * probes_per_hop];
		if (pb->done)  return;
	    } else
		return;
	}


	/*  Parse CMSG stuff   */

	for (cm = CMSG_FIRSTHDR (&msg); cm; cm = CMSG_NXTHDR (&msg, cm)) {
	    void *ptr = CMSG_DATA (cm);

	    if (cm->cmsg_level == SOL_SOCKET) {

		if (cm->cmsg_type == SO_TIMESTAMP) {
		    struct timeval *tv = (struct timeval *) ptr;

		    recv_time = tv->tv_sec + tv->tv_usec / 1000000.;
		}
	    }
	    else if (cm->cmsg_level == SOL_IP) {

		if (cm->cmsg_type == IP_TTL)
			recv_ttl = *((int *) ptr);
		else if (cm->cmsg_type == IP_RECVERR) {

		    ee = (struct sock_extended_err *) ptr;

		    if (ee->ee_origin != SO_EE_ORIGIN_ICMP &&
			ee->ee_origin != SO_EE_ORIGIN_LOCAL
		    )  return;

		    /*  dgram icmp sockets might return extra things...  */
		    if (ee->ee_origin == SO_EE_ORIGIN_ICMP &&
			(ee->ee_type == ICMP_SOURCE_QUENCH ||
			 ee->ee_type == ICMP_REDIRECT)
		    )  return;
		}
	    }
	    else if (cm->cmsg_level == SOL_IPV6) {

		if (cm->cmsg_type == IPV6_HOPLIMIT)
			recv_ttl = *((int *) ptr);
		else if (cm->cmsg_type == IPV6_RECVERR) {

		    ee = (struct sock_extended_err *) ptr;

		    if (ee->ee_origin != SO_EE_ORIGIN_ICMP6 &&
			ee->ee_origin != SO_EE_ORIGIN_LOCAL
		    )  return;
		}
	    }
	}

	if (!recv_time)
		recv_time = get_time ();


	if (!err)
	    memcpy (&pb->res, &from, sizeof (pb->res));

	pb->recv_time = recv_time;

	pb->recv_ttl = recv_ttl;

	if (ee && ee->ee_origin != SO_EE_ORIGIN_LOCAL) {    /*  icmp or icmp6   */
	    memcpy (&pb->res, SO_EE_OFFENDER (ee), sizeof(pb->res));
	    parse_icmp_res (pb, ee->ee_type, ee->ee_code, ee->ee_info);
	}

	if (ee && ee->ee_origin == SO_EE_ORIGIN_LOCAL)
		parse_local_res (pb, ee->ee_errno, ee->ee_info);


	if (ee &&
	    mtudisc &&
	    ee->ee_info >= header_len &&
	    ee->ee_info < header_len + data_len
	) {
	    data_len = ee->ee_info - header_len;

	    probe_done (pb);

	    /*  clear this probe (as actually the previous hop answers here)
	      but fill its `err_str' by the info obtained. Ugly, but easy...
	    */
	    memset (pb, 0, sizeof (*pb));
	    snprintf (pb->err_str, sizeof(pb->err_str)-1, "F=%d", ee->ee_info);

	    return;
	}


	if (ee &&
	    extension &&
	    header_len + n >= (128 + 8) &&	/*  at least... (rfc4884)  */
	    header_len <= 128 &&	/*  paranoia   */
	    ((af == AF_INET && (ee->ee_type == ICMP_TIME_EXCEEDED ||
				ee->ee_type == ICMP_DEST_UNREACH ||
				ee->ee_type == ICMP_PARAMETERPROB)) ||
	     (af == AF_INET6 && (ee->ee_type == ICMP6_TIME_EXCEEDED ||
				 ee->ee_type == ICMP6_DST_UNREACH))
	    )
	) {
	    int step;
	    int offs = 128 - header_len;

	    if (n > data_len)  step = 0;	/*  guaranteed at 128 ...  */
	    else
		step = af == AF_INET ? 4 : 8;

	    handle_extensions (pb, bufp + offs, n - offs, step);
	}


	probe_done (pb);
}


int equal_addr (const sockaddr_any *a, const sockaddr_any *b) {

	if (!a->sa.sa_family)
		return 0;

	if (a->sa.sa_family != b->sa.sa_family)
		return 0;

	if (a->sa.sa_family == AF_INET6)
	    return  !memcmp (&a->sin6.sin6_addr, &b->sin6.sin6_addr,
						sizeof (a->sin6.sin6_addr));
	else
	    return  !memcmp (&a->sin.sin_addr, &b->sin.sin_addr,
						sizeof (a->sin.sin_addr));
	return 0;	/*  not reached   */
}


void bind_socket (int sk) {
	sockaddr_any *addr, tmp;

	if (device) {
	    if (setsockopt (sk, SOL_SOCKET, SO_BINDTODEVICE,
					device, strlen (device) + 1) < 0
	    )  error ("setsockopt SO_BINDTODEVICE");
	}

	if (!src_addr.sa.sa_family) {
	    memset (&tmp, 0, sizeof (tmp));
	    tmp.sa.sa_family = af;
	    addr = &tmp;
	} else
	    addr = &src_addr;

	if (bind (sk, &addr->sa, sizeof (*addr)) < 0)
		error ("bind");

	return;
}


void use_timestamp (int sk) {
	int n = 1;

	setsockopt (sk, SOL_SOCKET, SO_TIMESTAMP, &n, sizeof (n));
	/*  foo on errors...  */
}


void use_recv_ttl (int sk) {
	int n = 1;

	if (af == AF_INET)
		setsockopt (sk, SOL_IP, IP_RECVTTL, &n, sizeof (n));
	else if (af == AF_INET6)
		setsockopt (sk, SOL_IPV6, IPV6_RECVHOPLIMIT, &n, sizeof (n));
	/*  foo on errors   */
}


void use_recverr (int sk) {
	int val = 1;

	if (af == AF_INET) {
	    if (setsockopt (sk, SOL_IP, IP_RECVERR, &val, sizeof (val)) < 0)
		    error ("setsockopt IP_RECVERR");
	}
	else if (af == AF_INET6) {
	    if (setsockopt (sk, SOL_IPV6, IPV6_RECVERR, &val, sizeof (val)) < 0)
		    error ("setsockopt IPV6_RECVERR");
	}
}


void set_ttl (int sk, int ttl) {

	if (af == AF_INET) {
	    if (setsockopt (sk, SOL_IP, IP_TTL, &ttl, sizeof (ttl)) < 0)
		    error ("setsockopt IP_TTL");
	}
	else if (af == AF_INET6) {
	    if (setsockopt (sk, SOL_IPV6, IPV6_UNICAST_HOPS,
						&ttl, sizeof (ttl)) < 0
	    )  error ("setsockopt IPV6_UNICAST_HOPS");
	}
}


int do_send (int sk, const void *data, size_t len, const sockaddr_any *addr) {
	int res;

	if (!addr || raw_can_connect ())
		res = send (sk, data, len, 0);
	else
	    res = sendto (sk, data, len, 0, &addr->sa, sizeof (*addr));

	if (res < 0) {
	    if (errno == ENOBUFS || errno == EAGAIN)
		    return res;
	    if (errno == EMSGSIZE)
		    return 0;	/*  recverr will say more...  */
	    error ("send");	/*  not recoverable   */
	}

	return res;
}


/*  There is a bug in the kernel before 2.6.25, which prevents icmp errors
  to be obtained by MSG_ERRQUEUE for ipv6 connected raw sockets.
*/
static int can_connect = -1;

#define VER(A,B,C,D)	(((((((A) << 8) | (B)) << 8) | (C)) << 8) | (D))

int raw_can_connect (void) {

	if (can_connect < 0) {

	    if (af == AF_INET)
		    can_connect = 1;
	    else {	/*  AF_INET6   */
		struct utsname uts;
		int n;
		unsigned int a, b, c, d = 0;

		if (uname (&uts) < 0)
			return 0;

		n = sscanf (uts.release, "%u.%u.%u.%u", &a, &b, &c, &d);
		can_connect = (n >= 3 && VER (a, b, c, d) >= VER (2, 6, 25, 0));
	    }
	}

	return can_connect;
}
