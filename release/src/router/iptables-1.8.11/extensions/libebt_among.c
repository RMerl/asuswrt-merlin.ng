/* ebt_among
 *
 * Authors:
 * Grzegorz Borowiak <grzes@gnu.univ.gda.pl>
 *
 * August, 2003
 */

#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <xtables.h>
#include <arpa/inet.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <linux/if_ether.h>
#include <linux/netfilter_bridge/ebt_among.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include "iptables/nft.h"
#include "iptables/nft-bridge.h"

#define AMONG_DST '1'
#define AMONG_SRC '2'
#define AMONG_DST_F '3'
#define AMONG_SRC_F '4'

static const struct option bramong_opts[] = {
	{"among-dst", required_argument, 0, AMONG_DST},
	{"among-src", required_argument, 0, AMONG_SRC},
	{"among-dst-file", required_argument, 0, AMONG_DST_F},
	{"among-src-file", required_argument, 0, AMONG_SRC_F},
	{0}
};

static void bramong_print_help(void)
{
	printf(
"`among' options:\n"
"[!] --among-dst      list      : matches if ether dst is in list\n"
"[!] --among-src      list      : matches if ether src is in list\n"
"[!] --among-dst-file file      : obtain dst list from file\n"
"[!] --among-src-file file      : obtain src list from file\n"
"list has form:\n"
" xx:xx:xx:xx:xx:xx[=ip.ip.ip.ip],yy:yy:yy:yy:yy:yy[=ip.ip.ip.ip]"
",...,zz:zz:zz:zz:zz:zz[=ip.ip.ip.ip][,]\n"
"Things in brackets are optional.\n"
"If you want to allow two (or more) IP addresses to one MAC address, you\n"
"can specify two (or more) pairs with the same MAC, e.g.\n"
" 00:00:00:fa:eb:fe=153.19.120.250,00:00:00:fa:eb:fe=192.168.0.1\n"
	);
}

static void
parse_nft_among_pair(char *buf, struct nft_among_pair *pair, bool have_ip)
{
	char *sep = index(buf, '=');
	struct ether_addr *ether;

	if (sep) {
		*sep = '\0';

		if (!inet_pton(AF_INET, sep + 1, &pair->in))
			xtables_error(PARAMETER_PROBLEM,
				      "Invalid IP address '%s'", sep + 1);
	}
	ether = ether_aton(buf);
	if (!ether)
		xtables_error(PARAMETER_PROBLEM,
			      "Invalid MAC address '%s'", buf);
	memcpy(&pair->ether, ether, sizeof(*ether));
}

static void
parse_nft_among_pairs(struct nft_among_pair *pairs, char *buf,
		      size_t cnt, bool have_ip)
{
	size_t tmpcnt = 0;

	buf = strtok(buf, ",");
	while (buf) {
		struct nft_among_pair pair = {};

		parse_nft_among_pair(buf, &pair, have_ip);
		nft_among_insert_pair(pairs, &tmpcnt, &pair);
		buf = strtok(NULL, ",");
	}
}

static size_t count_nft_among_pairs(char *buf)
{
	size_t cnt = 0;
	char *p = buf;

	if (!*buf)
		return 0;

	do {
		cnt++;
		p = index(++p, ',');
	} while (p);

	return cnt;
}

static bool nft_among_pairs_have_ip(char *buf)
{
	return !!index(buf, '=');
}

static int bramong_parse(int c, char **argv, int invert,
		 unsigned int *flags, const void *entry,
		 struct xt_entry_match **match)
{
	struct nft_among_data *data = (struct nft_among_data *)(*match)->data;
	bool have_ip, dst = false;
	size_t new_size, cnt;
	struct stat stats;
	int fd = -1, poff;
	long flen = 0;

	switch (c) {
	case AMONG_DST_F:
		dst = true;
		/* fall through */
	case AMONG_SRC_F:
		if ((fd = open(optarg, O_RDONLY)) == -1)
			xtables_error(PARAMETER_PROBLEM,
				      "Couldn't open file '%s'", optarg);
		if (fstat(fd, &stats) < 0)
			xtables_error(PARAMETER_PROBLEM,
				      "fstat(%s) failed: '%s'",
				      optarg, strerror(errno));
		flen = stats.st_size;
		/* use mmap because the file will probably be big */
		optarg = mmap(0, flen, PROT_READ | PROT_WRITE,
			      MAP_PRIVATE, fd, 0);
		if (optarg == MAP_FAILED)
			xtables_error(PARAMETER_PROBLEM,
				      "Couldn't map file to memory");
		if (optarg[flen-1] != '\n')
			xtables_error(PARAMETER_PROBLEM,
				      "File should end with a newline");
		if (strchr(optarg, '\n') != optarg+flen-1)
			xtables_error(PARAMETER_PROBLEM,
				      "File should only contain one line");
		optarg[flen-1] = '\0';
		break;
	case AMONG_DST:
		dst = true;
		/* fall through */
	case AMONG_SRC:
		break;
	default:
		return 0;
	}

	cnt = count_nft_among_pairs(optarg);
	if (cnt == 0)
		return 0;

	new_size = data->src.cnt + data->dst.cnt + cnt;
	new_size *= sizeof(struct nft_among_pair);
	new_size += XT_ALIGN(sizeof(struct xt_entry_match)) +
			sizeof(struct nft_among_data);

	if (new_size > (*match)->u.match_size) {
		*match = xtables_realloc(*match, new_size);
		(*match)->u.match_size = new_size;
		data = (struct nft_among_data *)(*match)->data;
	}

	have_ip = nft_among_pairs_have_ip(optarg);
	poff = nft_among_prepare_data(data, dst, cnt, invert, have_ip);
	parse_nft_among_pairs(data->pairs + poff, optarg, cnt, have_ip);

	if (c == AMONG_DST_F || c == AMONG_SRC_F) {
		munmap(argv, flen);
		close(fd);
	}
	return 1;
}

static void __bramong_print(struct nft_among_pair *pairs,
			    int cnt, bool have_ip)
{
	char abuf[INET_ADDRSTRLEN];
	const char *isep = "";
	int i;

	for (i = 0; i < cnt; i++) {
		printf("%s", isep);
		isep = ",";

		printf("%s", ether_ntoa(&pairs[i].ether));
		if (pairs[i].in.s_addr != INADDR_ANY)
			printf("=%s", inet_ntop(AF_INET, &pairs[i].in,
						abuf, sizeof(abuf)));
	}
	printf(" ");
}

static void bramong_print(const void *ip, const struct xt_entry_match *match,
			  int numeric)
{
	struct nft_among_data *data = (struct nft_among_data *)match->data;

	if (data->src.cnt) {
		printf("%s--among-src ", data->src.inv ? "! " : "");
		__bramong_print(data->pairs, data->src.cnt, data->src.ip);
	}
	if (data->dst.cnt) {
		printf("%s--among-dst ", data->dst.inv ? "! " : "");
		__bramong_print(data->pairs + data->src.cnt,
				data->dst.cnt, data->dst.ip);
	}
}

static struct xtables_match bramong_match = {
	.name		= "among",
	.revision	= 0,
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= XT_ALIGN(sizeof(struct nft_among_data)),
	.userspacesize	= XT_ALIGN(sizeof(struct nft_among_data)),
	.help		= bramong_print_help,
	.parse		= bramong_parse,
	.print		= bramong_print,
	.extra_opts	= bramong_opts,
};

void _init(void)
{
	xtables_register_match(&bramong_match);
}
