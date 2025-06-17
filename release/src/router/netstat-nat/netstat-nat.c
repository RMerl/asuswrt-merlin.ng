// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2006 by D.Wijsman (danny@tweegy.nl).


#include <arpa/inet.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <search.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NF_CONNTRACK_LOCATION "/proc/net/nf_conntrack"

#define USAGE                                                                                                              \
	"usage: %s [-no] [-x|-X[width]] {[-G] [-S|-D]}|[-L|-R] [-s ource]... [-d estination]... [-g ateway]... [-p rotocol]... " \
	"[-r src|dst|src-port|dst-port|state|gate|gate-port] [nf_conntrack]\n"


static void process_entry(char * line);
static bool check_src_dst(const char * protocol, const char * src_ip, const char * dst_ip, const char * src_port, const char * dst_port, const char * gate_ip,
                          const char * gate_port, const char * state);
static void lookup_hostport(char ** host, char ** port, const char * protocol);
static void nolookup_normalise(char ** host);
static void push_ip_filters(char * arg, struct addrinfo ** ips);
static bool match_ip_filter(const char * ip, const struct addrinfo * ips);
static void * xrealloc(void * oldbuf, size_t newbufsize);
static char * xstrdup(const char * oldstr);
static void local_ip_addresses_add(struct sockaddr * addr);
static void local_ip_addresses_add_forced(char * envvar);
static bool local_ip_address(const char * ip);


static struct {
	struct addrinfo * src_ip;
	struct addrinfo * dst_ip;
	struct addrinfo * gate_ip;
	void * protocol;

	bool snat;
	bool dnat;
	bool local;
	bool routed;
} filter = {.snat = true, .dnat = true};

enum connection_row {
	CONN_PROTOCOL,
	CONN_SRC_IP,
	CONN_DST_IP,
	CONN_SRC_PORT,
	CONN_DST_PORT,
	CONN_STATE,
	CONN_GATE_IP,
	CONN_GATE_PORT,
	CONN_ROWCNT,
};
static const char * const connection_row_names[CONN_ROWCNT] = {
    [CONN_PROTOCOL]  = NULL,         // unpickable, we always partition by protocol
    [CONN_SRC_IP]    = "src",        //
    [CONN_DST_IP]    = "dst",        //
    [CONN_SRC_PORT]  = "src-port",   //
    [CONN_DST_PORT]  = "dst-port",   //
    [CONN_STATE]     = "state",      //
    [CONN_GATE_IP]   = "gate",       //
    [CONN_GATE_PORT] = "gate-port",  //
};
struct connection {
	union {
		struct {
			char * protocol;
			char * src_ip;
			char * dst_ip;
			char * src_port;
			char * dst_port;
			char * state;
			char * gate_ip;
			char * gate_port;
		};
		char * by_row[CONN_ROWCNT];
	};
};
_Static_assert(offsetof(struct connection, by_row[CONN_PROTOCOL]) /***/ == offsetof(struct connection, protocol), /***/ "");
_Static_assert(offsetof(struct connection, by_row[CONN_SRC_IP]) /*****/ == offsetof(struct connection, src_ip), /*****/ "");
_Static_assert(offsetof(struct connection, by_row[CONN_DST_IP]) /*****/ == offsetof(struct connection, dst_ip), /*****/ "");
_Static_assert(offsetof(struct connection, by_row[CONN_SRC_PORT]) /***/ == offsetof(struct connection, src_port), /***/ "");
_Static_assert(offsetof(struct connection, by_row[CONN_DST_PORT]) /***/ == offsetof(struct connection, dst_port), /***/ "");
_Static_assert(offsetof(struct connection, by_row[CONN_STATE]) /******/ == offsetof(struct connection, state), /******/ "");
_Static_assert(offsetof(struct connection, by_row[CONN_GATE_IP]) /****/ == offsetof(struct connection, gate_ip), /****/ "");
_Static_assert(offsetof(struct connection, by_row[CONN_GATE_PORT]) /**/ == offsetof(struct connection, gate_port), /**/ "");
static size_t connection_table_len;
static struct connection * connection_table;

enum connection_row sort_row = CONN_SRC_IP;
int connection_table_cmp(const void * lhs_r, const void * rhs_r) {
	const struct connection * lhs = lhs_r;
	const struct connection * rhs = rhs_r;
	int ret;

	if((ret = strcmp(lhs->protocol, rhs->protocol)))
		return ret;

	if((ret = strcmp(lhs->by_row[sort_row], rhs->by_row[sort_row])))
		return ret;

	for(enum connection_row i = CONN_PROTOCOL + 1; i < CONN_ROWCNT; ++i) {
		if(i == sort_row)
			continue;

		if((ret = strcmp(lhs->by_row[i], rhs->by_row[i])))
			return ret;
	}
	return 0;
}


#if TEST
#include <assert.h>
int main() {
	struct addrinfo * set = NULL;
	push_ip_filters(strdup("127.000.000.001 0:0:0:0:0:0:0:1"), &set);
	assert(match_ip_filter("127.0.0.1", set));
	assert(match_ip_filter("::1", set));


	local_ip_addresses_add((struct sockaddr *)&(struct sockaddr_in){.sin_family = AF_INET, .sin_addr = {0xFFFFFFFF}});
	local_ip_addresses_add((struct sockaddr *)&(struct sockaddr_in6){
	    .sin6_family = AF_INET6, .sin6_addr = {{{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}}});
	local_ip_addresses_add_forced(strdup("127.000.000.001 192.168.001.250 0:0:0:0:0:0:0:1"));
	assert(local_ip_address("255.255.255.255"));
	assert(local_ip_address("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"));
	assert(local_ip_address("127.0.0.1"));
	assert(local_ip_address("192.168.1.250"));
	assert(local_ip_address("::1"));


	char * ip = strdup("127.000.000.001");
	nolookup_normalise(&ip);
	assert(!strcmp(ip, "127.0.0.1"));

	ip = strdup("0:0:0:0:0:0:0:1");
	nolookup_normalise(&ip);
	assert(!strcmp(ip, "::1"));

	char * staticlocal = (char *)"localhost";
	ip                 = staticlocal;
	nolookup_normalise(&ip);
	assert(ip == staticlocal);


	char *host = (char *)"", *port = (char *)"512";
	lookup_hostport(&host, &port, "udplite");
	assert(!*host);
	if(!strcmp(port, "comsat"))
		fprintf(stderr, "netstat-nat test: running under musl, can't test the biff/exec split for port 512\n");
	else
		assert(!strcmp(port, "biff"));

	host = (char *)"127.0.0.1", port = (char *)"512";
	lookup_hostport(&host, &port, "tcp");
	if(strcmp(host, "localhost") && strcmp(host, "localhost.localdomain"))
		fprintf(stderr, "netstat-nat test: looking up 127.0.0.1 yielded %s\n", host);
	assert(!strcmp(port, "exec"));
}
#else
int main(int argc, char * argv[]) {
	int colwidth = 30;
	bool resolve = true;
	bool no_hdr  = false;
	bool nat_hop = false;
	char buf[64 * 1024];
	for(int c; (c = getopt(argc, argv, "np:s:d:g:SDxX::or:LNGR")) != -1;) {
		switch(c) {
			case 'n':
				resolve = false;
				break;
			case 'p':
				tsearch(optarg, &filter.protocol, (int (*)(const void *, const void *))strcasecmp);
				break;
			case 's':
				push_ip_filters(optarg, &filter.src_ip);
				break;
			case 'd':
				push_ip_filters(optarg, &filter.dst_ip);
				break;
			case 'g':
				push_ip_filters(optarg, &filter.gate_ip);
				break;
			case 'S':
				filter.dnat = false;
				filter.snat = true;
				break;
			case 'D':
				filter.dnat = true;
				filter.snat = false;
				break;
			case 'L':
				filter.local  = true;
				filter.routed = false;
				break;
			case 'R':
				filter.local  = false;
				filter.routed = true;
				break;
			case 'x':
				colwidth = 40;
				break;
			case 'X': {
				char * next;
				errno    = 0;
				colwidth = strtol(optarg ?: "", &next, 0);
				if(*next) {
					errno = errno ?: EINVAL;
					err(1, "-X%s", optarg);
				}
				colwidth = colwidth <= 0 ? (int)(sizeof("[0000:0000:0000:0000:0000:0000:0000:0000]:65536") - 1) : colwidth;
			} break;
			case 'o':
				no_hdr = true;
				break;
			case 'N':
			case 'G':
				nat_hop = true;
				break;
			case 'r':
				for(sort_row = 0; sort_row < CONN_ROWCNT; ++sort_row)
					if(connection_row_names[sort_row] && !strcmp(optarg, connection_row_names[sort_row]))
						break;
				if(sort_row == CONN_ROWCNT)
					return fprintf(stderr, USAGE, argv[0]), 1;
				break;
			default:
				return fprintf(stderr, USAGE, argv[0]), 1;
		}
	}
	const char * file_override = *(argv + optind);
	if(file_override && *(argv + optind + 1))
		return fprintf(stderr, USAGE, argv[0]), 1;
	// -L|-R naturally excludes all NATs
	if(filter.local || filter.routed) {
		filter.snat = false;
		filter.dnat = false;
		nat_hop     = false;
	}

	// get local IP addresses for all interfaces
	if(filter.routed || filter.local) {
		char * forced_local_addresses = getenv("NETSTAT_NAT_LOCAL_ADDRS");
		if(forced_local_addresses)
			local_ip_addresses_add_forced(forced_local_addresses);
		else {
			struct ifaddrs * addrs;
			if(!getifaddrs(&addrs)) {
				for(struct ifaddrs * itr = addrs; itr; itr = itr->ifa_next) {
					if (itr->ifa_addr)
						local_ip_addresses_add(itr->ifa_addr);
				}
				freeifaddrs(addrs);
			}
		}
	}

	if((!file_override || strcmp(file_override, "-")) && !freopen(file_override ?: NF_CONNTRACK_LOCATION, "r", stdin)) {
		warn("%s", file_override ?: NF_CONNTRACK_LOCATION);
		if(!file_override && errno == ENOENT)
			warnx("Make sure the kernel is configured with CONFIG_NF_CONNTRACK=y or the nf_conntrack module is loaded, and that /proc is mounted.");
		return 1;
	}
	setvbuf(stdin, buf, _IOFBF, sizeof(buf));

	if(!no_hdr) {
		printf("%-6s%-*s", "Proto", colwidth, (filter.local || filter.routed) ? "Source Address" : "NATed Address");
		if(nat_hop)
			printf(" %-*s", colwidth, "Gateway Address");
		printf(" %-*s %s\n", colwidth, "Destination Address", "State");
	}

	char * line = NULL;
	for(size_t linelen = 0; getline(&line, &linelen, stdin) != -1;)
		process_entry(line);
	if(connection_table_len == 0)  // There are no connections at this moment! We can exit.
		return 0;
	free(line);
	fclose(stdin);


	qsort(connection_table, connection_table_len, sizeof(*connection_table), connection_table_cmp);

#define pa connection_table
	for(size_t i = 0; i < connection_table_len; i++) {
		if(resolve) {
			lookup_hostport(&pa[i].src_ip, &pa[i].src_port, pa[i].protocol);
			lookup_hostport(&pa[i].dst_ip, &pa[i].dst_port, pa[i].protocol);
			lookup_hostport(&pa[i].gate_ip, &pa[i].gate_port, pa[i].protocol);
		} else {
			nolookup_normalise(&pa[i].src_ip);
			nolookup_normalise(&pa[i].dst_ip);
			nolookup_normalise(&pa[i].gate_ip);
		}

#define ADDRESS(ip, port)                    \
	{                                          \
		*buf              = '\0';                \
		size_t extrawidth = 0;                   \
		bool bracket      = false;               \
		if(*port) {                              \
			if(strchr(ip, ':')) {                  \
				strcat(buf, "["), extrawidth += 2;   \
				bracket = true;                      \
			}                                      \
			extrawidth += 1 + strlen(port);        \
		}                                        \
		strncat(buf, ip, colwidth - extrawidth); \
		if(*port) {                              \
			strcat(buf, "]:" + !bracket);          \
			strcat(buf, port);                     \
		}                                        \
	}
		ADDRESS(pa[i].src_ip, pa[i].src_port)
		printf("%-5s %-*s", pa[i].protocol, colwidth, buf);

		if(nat_hop) {
			ADDRESS(pa[i].gate_ip, pa[i].gate_port)
			printf(" %-*s", colwidth, buf);
		}

		ADDRESS(pa[i].dst_ip, pa[i].dst_port)
		printf(" %-*s %s\n", colwidth, buf, pa[i].state);
	}
}
#endif


struct get_protocol_name_map {
	int proto;
	char str[];
};
static void * get_protocol_name_map;
static int get_protocol_name_cmp(const void * lhs_r, const void * rhs_r) {
	const struct get_protocol_name_map * lhs = lhs_r;
	const struct get_protocol_name_map * rhs = rhs_r;

	return lhs->proto - rhs->proto;
}
static const char * get_protocol_name(int protocol_nr) {
	struct get_protocol_name_map k = {.proto = protocol_nr}, **v = tfind(&k, &get_protocol_name_map, get_protocol_name_cmp);
	if(v)
		return (*v)->str;

	char buf[11 + 1], *nm = buf;  // -2147483648
	struct protoent * proto_struct = getprotobynumber(protocol_nr);
	if(proto_struct != NULL)
		nm = proto_struct->p_name;
	else
		sprintf(nm, "%d", protocol_nr);

	struct get_protocol_name_map * kv = xrealloc(NULL, sizeof(struct get_protocol_name_map) + strlen(nm) + 1);
	kv->proto                         = protocol_nr;
	memcpy(kv->str, nm, strlen(nm) + 1);
	tsearch(kv, &get_protocol_name_map, get_protocol_name_cmp);
	return kv->str;
}

static bool an_ok_connection_state(const char * tok, const char ** ret) {
#define EXT(known)                                                     \
	else if(*tok == *known && !strncmp(tok, known, sizeof(known) - 1)) { \
		*ret = known;                                                      \
		return true;                                                       \
	}
	if(!(tok = strpbrk(tok, "ACEFSTU")))
		return false;
	EXT("ESTABLISHED")
	EXT("TIME_WAIT")
	EXT("FIN_WAIT")
	EXT("SYN_RECV")
	EXT("SYN_SENT")
	EXT("UNREPLIED")
	EXT("CLOSE")
	EXT("ASSURED")
	else return false;
}

#if 0
ipv4     2 udp      17 19 src=139.12.34.56 dst=194.0.0.53 sport=55569 dport=53 src=194.0.0.53 dst=139.12.34.56 sport=53 dport=55569 mark=0 zone=0 use=2
ipv4     2 udp      17 1 src=192.168.1.250 dst=185.89.218.12 sport=25718 dport=53 src=185.89.218.12 dst=139.12.34.56 sport=53 dport=25718 mark=0 zone=0 use=2
ipv4     2 tcp      6 431938 ESTABLISHED src=192.168.1.2 dst=34.241.17.166 sport=36486 dport=443 src=34.241.17.166 dst=139.12.34.56 sport=443 dport=36486 [ASSURED] mark=0 zone=0 use=2
ipv4     2 tcp      6 431960 ESTABLISHED src=165.232.32.235 dst=139.12.34.56 sport=33200 dport=443 src=192.168.1.250 dst=165.232.32.235 sport=443 dport=33200 [ASSURED] mark=0 zone=0 use=2
ipv4     2 tcp      6 41 SYN_SENT src=46.29.161.212 dst=139.12.34.56 sport=56618 dport=22 [UNREPLIED] src=192.168.1.250 dst=46.29.161.212 sport=22 dport=56618 mark=0 zone=0 use=2
ipv4     2 tcp      6 50 TIME_WAIT src=118.45.205.44 dst=139.12.34.56 sport=33946 dport=22 src=192.168.1.250 dst=118.45.205.44 sport=22 dport=33946 [ASSURED] mark=0 zone=0 use=2
l3str  num l4str  num
#endif
static void process_entry(char * line) {
	const char * srcip_f   = NULL;  // first src=
	const char * dstip_f   = NULL;  // first dst=
	const char * srcip_s   = NULL;  // second src=
	const char * dstip_s   = NULL;  // second dst=
	const char * srcport   = NULL;  // first sport=
	const char * dstport   = NULL;  // first dport=
	const char * srcport_s = NULL;  // second sport=
	const char * dstport_s = NULL;  // second dport=
	const char * state     = NULL;  // normalised connection state
	const char *protocol, *protocol_num;


	char * sav = NULL;
	strtok_r(line, " \n", &sav);  // L3 str
	strtok_r(NULL, " \n", &sav);  // L3 num

	protocol     = strtok_r(NULL, " \n", &sav);  // L4 str
	protocol_num = strtok_r(NULL, " \n", &sav);  // L4 num
	if(!strcmp(protocol, "unknown"))
		protocol = get_protocol_name(atoi(protocol_num));
	if(filter.protocol && !tfind(protocol, &filter.protocol, (int (*)(const void *, const void *))strcasecmp))
		return;


#define strstarts(what, with) !strncmp(what, with, sizeof(with) - 1)
	for(char * tok; (tok = strtok_r(NULL, " \n", &sav));) {
		if(!state && an_ok_connection_state(tok, &state))
			;
		else if((!srcip_f || !srcip_s) && strstarts(tok, "src="))
			*(srcip_f ? &srcip_s : &srcip_f) = tok + sizeof("src=") - 1;
		else if((!dstip_f || !dstip_s) && strstarts(tok, "dst="))
			*(dstip_f ? &dstip_s : &dstip_f) = tok + sizeof("src=") - 1;
		else if((!srcport || !srcport_s) && strstarts(tok, "sport="))
			*(srcport ? &srcport_s : &srcport) = tok + sizeof("sport=") - 1;
		else if((!dstport || !dstport_s) && strstarts(tok, "dport="))
			*(dstport ? &dstport_s : &dstport) = tok + sizeof("dport=") - 1;

		if(srcip_f && dstip_f && srcip_s && dstip_s && srcport && dstport && srcport_s && dstport_s && state)
			break;
	}


	if(filter.snat)
		if((strcmp(srcip_f, dstip_s) != 0) && (strcmp(dstip_f, srcip_s) == 0))
			check_src_dst(protocol, srcip_f, dstip_f, srcport, dstport, dstip_s, dstport_s, state);

	if(filter.dnat)
		if((strcmp(srcip_f, dstip_s) == 0) && (strcmp(dstip_f, srcip_s) != 0))
			check_src_dst(protocol, srcip_f, srcip_s, srcport, srcport_s, dstip_f, dstport_s, state);

	// bugfix for displaying DNAT over SNAT connections, submitted by Supaflyster (intercepted traffic to DNAT) (2 interfaces)
	if(filter.dnat || filter.snat)
		if((strcmp(srcip_f, srcip_s) != 0) && (strcmp(srcip_f, dstip_s) != 0) && (strcmp(dstip_f, srcip_s) != 0) && (strcmp(dstip_f, dstip_s) != 0))
			check_src_dst(protocol, srcip_f, srcip_s, srcport, srcport_s, dstip_s, dstport_s, state);

	// (DNAT) (1 interface)
	if(filter.dnat)
		if((strcmp(srcip_f, srcip_s) != 0) && (strcmp(srcip_f, dstip_s) != 0) && (strcmp(dstip_f, srcip_s) != 0) && (strcmp(dstip_f, dstip_s) == 0))
			check_src_dst(protocol, srcip_f, srcip_s, srcport, srcport_s, dstip_s, dstport_s, state);

	if(filter.local)
		if((strcmp(srcip_f, dstip_s) == 0) && (strcmp(dstip_f, srcip_s) == 0) &&
		   (local_ip_address(srcip_f) || local_ip_address(srcip_s) || local_ip_address(dstip_f) || local_ip_address(dstip_s)))
			check_src_dst(protocol, srcip_f, srcip_s, srcport, dstport, NULL, NULL, state);

	if(filter.routed)
		if((strcmp(srcip_f, dstip_s) == 0) && (strcmp(dstip_f, srcip_s) == 0) &&  //
		   !local_ip_address(srcip_f) && !local_ip_address(srcip_s) && !local_ip_address(dstip_f) && !local_ip_address(dstip_s))
			check_src_dst(protocol, srcip_f, srcip_s, srcport, dstport, NULL, NULL, state);

	// printf("protocol='%s' srcip_f='%s' dstip_f='%s' srcip_s='%s' dstip_s='%s' srcport='%s' dstport='%s' srcport_s='%s' dstport_s='%s' state='%s'\n",  //
	//        protocol, srcip_f, dstip_f, srcip_s, dstip_s, srcport, dstport, srcport_s, dstport_s, state);
}


static bool check_src_dst(const char * protocol, const char * src_ip, const char * dst_ip, const char * src_port, const char * dst_port, const char * gate_ip,
                          const char * gate_port, const char * state) {
	if(match_ip_filter(dst_ip, filter.dst_ip) && match_ip_filter(src_ip, filter.src_ip) && match_ip_filter(gate_ip, filter.gate_ip)) {
		connection_table                                 = xrealloc(connection_table, (connection_table_len + 1) * sizeof(*connection_table));
		connection_table[connection_table_len].src_port  = xstrdup(src_port ?: "");
		connection_table[connection_table_len].dst_port  = xstrdup(dst_port ?: "");
		connection_table[connection_table_len].src_ip    = xstrdup(src_ip ?: "");
		connection_table[connection_table_len].dst_ip    = xstrdup(dst_ip ?: "");
		connection_table[connection_table_len].protocol  = xstrdup(protocol ?: "");
		connection_table[connection_table_len].state     = (char *)(state ?: "");  // always static data
		connection_table[connection_table_len].gate_ip   = xstrdup(gate_ip ?: "");
		connection_table[connection_table_len].gate_port = xstrdup(gate_port ?: "");
		++connection_table_len;
		return true;
	} else
		return false;
}


struct hostport_map {
	char *key, value[];
};
static int hostport_cmp(const void * lhs_r, const void * rhs_r) {
	const struct hostport_map * lhs = lhs_r;
	const struct hostport_map * rhs = rhs_r;

	return strcmp(lhs->key, rhs->key);
}
static void *hostport_host_, *hostport_port_dgram, *hostport_port_stream;
static void lookup_hostport(char ** host, char ** port, const char * protocol) {
	void ** hostport_host = &hostport_host_;
	void ** hostport_port = strstr(protocol, "udp") ? &hostport_port_dgram : &hostport_port_stream;

#define SANITISE_LOOKUP(field)                                                                \
	if(!**field)                                                                                \
		field = NULL;                                                                             \
	if(field) {                                                                                 \
		struct hostport_map k = {.key = *field}, **v = tfind(&k, hostport_##field, hostport_cmp); \
		if(v) {                                                                                   \
			*field = (*v)->value;                                                                   \
			field  = NULL;                                                                          \
		}                                                                                         \
	}
	SANITISE_LOOKUP(host)
	SANITISE_LOOKUP(port)

	if(!host && !port)
		return;


	struct addrinfo *res, flags = {.ai_flags = AI_NUMERICHOST};
	if(getaddrinfo(host ? *host : NULL, port ? *port : NULL, &flags, &res))
		return;

	char hostbuf[NI_MAXHOST], portbuf[NI_MAXSERV];
	if(getnameinfo(res->ai_addr, res->ai_addrlen, host ? hostbuf : NULL, host ? sizeof(hostbuf) : 0, port ? portbuf : NULL, port ? sizeof(portbuf) : 0,
	               strstr(protocol, "udp") ? NI_DGRAM : 0)) {
		freeaddrinfo(res);
		return;
	}
	freeaddrinfo(res);

#define SAVE(field)                                                                                  \
	if(field) {                                                                                        \
		struct hostport_map * kv = xrealloc(NULL, sizeof(struct hostport_map) + strlen(field##buf) + 1); \
		kv->key                  = *field;                                                               \
		memcpy(kv->value, field##buf, strlen(field##buf) + 1);                                           \
		*field = kv->value;                                                                              \
		tsearch(kv, hostport_##field, hostport_cmp);                                                     \
	}
	SAVE(host)
	SAVE(port)
}

static void nolookup_normalise(char ** host) {
	if(!**host)
		return;

	struct addrinfo *res, flags = {.ai_flags = AI_NUMERICHOST};
	int error;
	if((error = getaddrinfo(*host, NULL, &flags, &res)))
		return;

	char newhost[INET6_ADDRSTRLEN];
	if(!inet_ntop(res->ai_addr->sa_family,
	              res->ai_addr->sa_family == AF_INET ? (void *)&((struct sockaddr_in *)res->ai_addr)->sin_addr
	                                                 : (void *)&((struct sockaddr_in6 *)res->ai_addr)->sin6_addr /*AF_INET6*/,
	              newhost, sizeof(newhost))) {
		freeaddrinfo(res);
		return;
	}
	freeaddrinfo(res);

	*host = xrealloc(*host, strlen(newhost) + 1);
	memcpy(*host, newhost, strlen(newhost) + 1);
}

static void push_ip_filter(const char * hostname, struct addrinfo ** ips) {
	struct addrinfo *res, *itr, *prev = NULL, flags = {};
	int error;
	if((error = getaddrinfo(hostname, NULL, &flags, &res)))
		errx(1, "%s: %s", hostname, gai_strerror(error));

	for(itr = res; itr; itr = itr->ai_next)
		prev = itr;
	prev->ai_next = *ips;
	*ips          = res;
}
static void push_ip_filters(char * arg, struct addrinfo ** ips) {
	char * sav = NULL;
	for(char * addr = strtok_r(arg, ", \t\n", &sav); addr; addr = strtok_r(NULL, ", \t\n", &sav))
		push_ip_filter(addr, ips);
}
static bool match_ip_filter(const char * ip, const struct addrinfo * ips) {
	if(!ips)  // no filters set
		return true;
	if(!ip)  // ip (src=/dst=) not given
		return true;

	struct addrinfo *res, flags = {.ai_flags = AI_NUMERICHOST};
	int error;
	if((error = getaddrinfo(ip, NULL, &flags, &res)))
		return false;

	for(; ips; ips = ips->ai_next)
		if(res->ai_family == ips->ai_family)
			switch(res->ai_family) {
				case AF_INET:
					if(((struct sockaddr_in *)res->ai_addr)->sin_addr.s_addr == ((struct sockaddr_in *)ips->ai_addr)->sin_addr.s_addr)
						goto found;
					break;
				case AF_INET6:
					if(!memcmp(((struct sockaddr_in6 *)res->ai_addr)->sin6_addr.s6_addr, ((struct sockaddr_in6 *)ips->ai_addr)->sin6_addr.s6_addr,
					           sizeof(struct in6_addr)))
						goto found;
					break;
			}

	freeaddrinfo(res);
	return false;
found:
	freeaddrinfo(res);
	return true;
}


static void * xrealloc(void * oldbuf, size_t newbufsize) {
	void * newbuf = realloc(oldbuf, newbufsize);
	if(!newbuf)
		err(1, NULL);
	return newbuf;
}

static char * xstrdup(const char * oldstr) {
	if(!*oldstr)
		return "";
	char * newstr = strdup(oldstr);
	if(!newstr)
		err(1, NULL);
	return newstr;
}

struct local_ip_addresses_data {
	sa_family_t family;
	union {
		struct in_addr sin_addr;
		struct in6_addr sin6_addr;
	};
};
static void local_ip_addresses_data_init(struct local_ip_addresses_data * this, struct sockaddr * addr) {
	switch(this->family = addr->sa_family) {
		case AF_INET:
			this->sin_addr = ((struct sockaddr_in *)addr)->sin_addr;
			return;
		case AF_INET6:
			this->sin6_addr = ((struct sockaddr_in6 *)addr)->sin6_addr;
			return;
		default:
			__builtin_unreachable();
	}
}
static int local_ip_addresses_cmp(const void * lhs_r, const void * rhs_r) {
	const struct local_ip_addresses_data * lhs = lhs_r;
	const struct local_ip_addresses_data * rhs = rhs_r;

	if(lhs->family != rhs->family)
		return lhs->family - rhs->family;
	switch(lhs->family) {
		case AF_INET:
			return memcmp(&lhs->sin_addr, &rhs->sin_addr, sizeof(rhs->sin_addr));
		case AF_INET6:
			return memcmp(&lhs->sin6_addr, &rhs->sin6_addr, sizeof(rhs->sin6_addr));
		default:
			__builtin_unreachable();
	}
}
static void * local_ip_addresses;
static void local_ip_addresses_add(struct sockaddr * addr) {
	if(!(addr->sa_family == AF_INET || addr->sa_family == AF_INET6))
		return;

	struct local_ip_addresses_data * entry = xrealloc(NULL, sizeof(*entry));
	local_ip_addresses_data_init(entry, addr);
	struct local_ip_addresses_data ** sought = tsearch(entry, &local_ip_addresses, local_ip_addresses_cmp);
	if(sought && *sought != entry)
		free(entry);
}
static void local_ip_addresses_add_forced(char * envvar) {
	char * sav = NULL;
	for(char * addr = strtok_r(envvar, ", \t\n", &sav); addr; addr = strtok_r(NULL, ", \t\n", &sav)) {
		struct addrinfo *res, flags = {.ai_flags = AI_NUMERICHOST};
		int error;
		if((error = getaddrinfo(addr, NULL, &flags, &res)))
			errx(1, "%s: %s", addr, gai_strerror(error));

		local_ip_addresses_add(res->ai_addr);

		freeaddrinfo(res);
	}
}
static bool local_ip_address(const char * ip) {
	struct addrinfo *res, flags = {.ai_flags = AI_NUMERICHOST};
	if(getaddrinfo(ip, NULL, &flags, &res))
		return false;

	struct local_ip_addresses_data entry;
	local_ip_addresses_data_init(&entry, res->ai_addr);
	freeaddrinfo(res);

	return tfind(&entry, &local_ip_addresses, local_ip_addresses_cmp);
}
