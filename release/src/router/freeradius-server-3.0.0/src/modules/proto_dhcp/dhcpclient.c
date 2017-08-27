/*
 * dhcpclient.c	General radius packet debug tool.
 *
 * Version:	$Id$
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2000,2006  The FreeRADIUS server project
 * Copyright 2000  Miquel van Smoorenburg <miquels@cistron.nl>
 * Copyright 2010  Alan DeKok <aland@ox.org>
 */

RCSID("$Id$")

#include <freeradius-devel/libradius.h>
#include <freeradius-devel/conf.h>
#include <freeradius-devel/radpaths.h>
#include <freeradius-devel/dhcp.h>

#ifdef WITH_DHCP

#include <ctype.h>

#ifdef HAVE_GETOPT_H
#	include <getopt.h>
#endif

#include <assert.h>

static int success = 0;
static int retries = 3;
static float timeout = 5;

static int server_port = 0;
static int packet_code = 0;
static fr_ipaddr_t server_ipaddr;

static fr_ipaddr_t client_ipaddr;
static int client_port = 0;

static int sockfd;

static RADIUS_PACKET *request = NULL;
static RADIUS_PACKET *reply = NULL;

#define DHCP_CHADDR_LEN	(16)
#define DHCP_SNAME_LEN	(64)
#define DHCP_FILE_LEN	(128)
#define DHCP_VEND_LEN	(308)
#define DHCP_OPTION_MAGIC_NUMBER (0x63825363)

char const *dhcpclient_version = "dhcpclient version " RADIUSD_VERSION_STRING
#ifdef RADIUSD_VERSION_COMMIT
" (git #" RADIUSD_VERSION_COMMIT ")"
#endif
", built on " __DATE__ " at " __TIME__;

static void NEVER_RETURNS usage(void)
{
	fprintf(stderr, "Usage: dhcpclient [options] server[:port] <command>\n");

	fprintf(stderr, "  <command>    One of discover, request, offer\n");
	fprintf(stderr, "  -c count    Send each packet 'count' times.\n");
	fprintf(stderr, "  -d raddb    Set dictionary directory.\n");
	fprintf(stderr, "  -f file     Read packets from file, not stdin.\n");
	fprintf(stderr, "  -r retries  If timeout, retry sending the packet 'retries' times.\n");
	fprintf(stderr, "  -v	  Show program version information.\n");
	fprintf(stderr, "  -x	  Debugging mode.\n");

	exit(1);
}


/*
 *	Initialize the request.
 */
static int request_init(char const *filename)
{
	FILE *fp;
	vp_cursor_t cursor;
	VALUE_PAIR *vp;
	int filedone = 0;

	/*
	 *	Determine where to read the VP's from.
	 */
	if (filename) {
		fp = fopen(filename, "r");
		if (!fp) {
			fprintf(stderr, "dhcpclient: Error opening %s: %s\n",
				filename, strerror(errno));
			return 0;
		}
	} else {
		fp = stdin;
	}

	request = rad_alloc(NULL, 0);

	/*
	 *	Read the VP's.
	 */
	request->vps = readvp2(NULL, fp, &filedone, "dhcpclient:");
	if (!request->vps) {
		rad_free(&request);
		if (fp != stdin) fclose(fp);
		return 1;
	}

	/*
	 *	Fix / set various options
	 */
	for (vp = paircursor(&cursor, &request->vps); vp; vp = pairnext(&cursor)) {
		switch (vp->da->attr) {
		default:
			break;

			/*
			 *	Allow it to set the packet type in
			 *	the attributes read from the file.
			 */
		case PW_PACKET_TYPE:
			request->code = vp->vp_integer;
			break;

		case PW_PACKET_DST_PORT:
			request->dst_port = (vp->vp_integer & 0xffff);
			break;

		case PW_PACKET_DST_IP_ADDRESS:
			request->dst_ipaddr.af = AF_INET;
			request->dst_ipaddr.ipaddr.ip4addr.s_addr = vp->vp_ipaddr;
			break;

		case PW_PACKET_DST_IPV6_ADDRESS:
			request->dst_ipaddr.af = AF_INET6;
			request->dst_ipaddr.ipaddr.ip6addr = vp->vp_ipv6addr;
			break;

		case PW_PACKET_SRC_PORT:
			request->src_port = (vp->vp_integer & 0xffff);
			break;

		case PW_PACKET_SRC_IP_ADDRESS:
			request->src_ipaddr.af = AF_INET;
			request->src_ipaddr.ipaddr.ip4addr.s_addr = vp->vp_ipaddr;
			break;

		case PW_PACKET_SRC_IPV6_ADDRESS:
			request->src_ipaddr.af = AF_INET6;
			request->src_ipaddr.ipaddr.ip6addr = vp->vp_ipv6addr;
			break;
		} /* switch over the attribute */

	} /* loop over the VP's we read in */

	if (fp != stdin) fclose(fp);

	/*
	 *	And we're done.
	 */
	return 1;
}

static char const *dhcp_header_names[] = {
	"DHCP-Opcode",
	"DHCP-Hardware-Type",
	"DHCP-Hardware-Address-Length",
	"DHCP-Hop-Count",
	"DHCP-Transaction-Id",
	"DHCP-Number-of-Seconds",
	"DHCP-Flags",
	"DHCP-Client-IP-Address",
	"DHCP-Your-IP-Address",
	"DHCP-Server-IP-Address",
	"DHCP-Gateway-IP-Address",
	"DHCP-Client-Hardware-Address",
	"DHCP-Server-Host-Name",
	"DHCP-Boot-Filename",

	NULL
};

static int dhcp_header_sizes[] = {
	1, 1, 1, 1,
	4, 2, 2, 4,
	4, 4, 4,
	DHCP_CHADDR_LEN,
	DHCP_SNAME_LEN,
	DHCP_FILE_LEN
};


static void print_hex(RADIUS_PACKET *packet)
{
	int i, j;
	uint8_t const *p, *a;

	if (!packet->data) return;

	if (packet->data_len < 244) {
		printf("Huh?\n");
		return;
	}

	printf("----------------------------------------------------------------------\n");
	fflush(stdout);

	p = packet->data;
	for (i = 0; i < 14; i++) {
		printf("%s = 0x", dhcp_header_names[i]);
		for (j = 0; j < dhcp_header_sizes[i]; j++) {
			printf("%02x", p[j]);

		}
		printf("\n");
		p += dhcp_header_sizes[i];
	}

	/*
	 *	Magic number
	 */
	printf("%02x %02x %02x %02x\n",
	       p[0], p[1], p[2], p[3]);
	p += 4;

	while (p < (packet->data + packet->data_len)) {

		if (*p == 0) break;
		if (*p == 255) break; /* end of options signifier */
		if ((p + 2) > (packet->data + packet->data_len)) break;

		printf("%02x  %02x  ", p[0], p[1]);
		a = p + 2;

		for (i = 0; i < p[1]; i++) {
			if ((i > 0) && ((i & 0x0f) == 0x00))
				printf("\t\t");
			printf("%02x ", a[i]);
			if ((i & 0x0f) == 0x0f) printf("\n");
		}

		if ((p[1] & 0x0f) != 0x00) printf("\n");

		p += p[1] + 2;
	}
	printf("\n----------------------------------------------------------------------\n");
	fflush(stdout);
}

int main(int argc, char **argv)
{
	char *p;
	int c;
	char const *radius_dir = RADDBDIR;
	char const *filename = NULL;

	fr_debug_flag = 0;

	while ((c = getopt(argc, argv, "d:f:hr:t:vx")) != EOF) switch(c) {
		case 'd':
			radius_dir = optarg;
			break;
		case 'f':
			filename = optarg;
			break;
		case 'r':
			if (!isdigit((int) *optarg))
				usage();
			retries = atoi(optarg);
			if ((retries == 0) || (retries > 1000)) usage();
			break;
		case 't':
			if (!isdigit((int) *optarg))
				usage();
			timeout = atof(optarg);
			break;
		case 'v':
			printf("%s\n", dhcpclient_version);
			exit(0);
			break;
		case 'x':
			fr_debug_flag++;
			fr_log_fp = stdout;
			break;
		case 'h':
		default:
			usage();
			break;
	}
	argc -= (optind - 1);
	argv += (optind - 1);

	if (argc < 2) usage();

	if (dict_init(radius_dir, RADIUS_DICTIONARY) < 0) {
		fr_perror("dhcpclient");
		return 1;
	}

	/*
	 *	Resolve hostname.
	 */
	server_ipaddr.af = AF_INET;
	if (strcmp(argv[1], "-") != 0) {
		char const *hostname = argv[1];
		char const *portname = argv[1];
		char buffer[256];

		if (*argv[1] == '[') { /* IPv6 URL encoded */
			p = strchr(argv[1], ']');
			if ((size_t) (p - argv[1]) >= sizeof(buffer)) {
				usage();
			}

			memcpy(buffer, argv[1] + 1, p - argv[1] - 1);
			buffer[p - argv[1] - 1] = '\0';

			hostname = buffer;
			portname = p + 1;

		}
		p = strchr(portname, ':');
		if (p && (strchr(p + 1, ':') == NULL)) {
			*p = '\0';
			portname = p + 1;
		} else {
			portname = NULL;
		}

		if (ip_hton(hostname, AF_INET, &server_ipaddr) < 0) {
			fprintf(stderr, "dhcpclient: Failed to find IP address for host %s: %s\n", hostname, strerror(errno));
			exit(1);
		}

		/*
		 *	Strip port from hostname if needed.
		 */
		if (portname) server_port = atoi(portname);
	}

	/*
	 *	See what kind of request we want to send.
	 */
	if (strcmp(argv[2], "discover") == 0) {
		if (server_port == 0) server_port = 67;
		packet_code = PW_DHCP_DISCOVER;

	} else if (strcmp(argv[2], "request") == 0) {
		if (server_port == 0) server_port = 67;
		packet_code = PW_DHCP_REQUEST;

	} else if (strcmp(argv[2], "offer") == 0) {
		if (server_port == 0) server_port = 67;
		packet_code = PW_DHCP_OFFER;

	} else if (isdigit((int) argv[2][0])) {
		if (server_port == 0) server_port = 67;
		packet_code = atoi(argv[2]);
	} else {
		fprintf(stderr, "Unknown packet type %s\n", argv[2]);
		usage();
	}

	request_init(filename);

	/*
	 *	No data read.  Die.
	 */
	if (!request || !request->vps) {
		fprintf(stderr, "dhcpclient: Nothing to send.\n");
		exit(1);
	}
	request->code = packet_code;

	/*
	 *	Bind to the first specified IP address and port.
	 *	This means we ignore later ones.
	 */
	if (request->src_ipaddr.af == AF_UNSPEC) {
		memset(&client_ipaddr, 0, sizeof(client_ipaddr));
		client_ipaddr.af = server_ipaddr.af;
		client_port = 0;
	} else {
		client_ipaddr = request->src_ipaddr;
		client_port = request->src_port;
	}
	sockfd = fr_socket(&client_ipaddr, client_port);
	if (sockfd < 0) {
		fprintf(stderr, "dhcpclient: socket: %s\n", fr_strerror());
		exit(1);
	}

	request->sockfd = sockfd;
	if (request->src_ipaddr.af == AF_UNSPEC) {
		request->src_ipaddr = client_ipaddr;
		request->src_port = client_port;
	}
	if (request->dst_ipaddr.af == AF_UNSPEC) {
		request->dst_ipaddr = server_ipaddr;
		request->dst_port = server_port;
	}

	/*
	 *	Encode the packet
	 */
	if (fr_dhcp_encode(request) < 0) {
		fprintf(stderr, "dhcpclient: failed encoding: %s\n",
			fr_strerror());
		exit(1);
	}
	if (fr_debug_flag) print_hex(request);

	if (fr_dhcp_send(request) < 0) {
		fprintf(stderr, "dhcpclient: failed sending: %s\n",
			strerror(errno));
		exit(1);
	}

	reply = fr_dhcp_recv(sockfd);
	if (!reply) {
		fprintf(stderr, "dhcpclient: no reply\n");
		exit(1);
	}
	if (fr_debug_flag) print_hex(reply);

	if (fr_dhcp_decode(reply) < 0) {
		fprintf(stderr, "dhcpclient: failed decoding\n");
		return 1;
	}

	dict_free();

	if (success) return 0;

	return 1;
}

#endif	/* WITH_DHCP */
