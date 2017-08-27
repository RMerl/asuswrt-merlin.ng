/* simple tool to generate random of flow entries to fill hard the
   conntrack table. Early drop will not save our day then, because
   the table will be plenty of assured flows. If things go well,
   we hit ENOMEM at some point.

   You have to use conntrack_events_reliable together with this tool.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack_tcp.h>

int main(int argc, char *argv[])
{
	time_t t;
	int ret, i, j, r;
	struct nfct_handle *h;
	struct nf_conntrack *ct;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s [ct_table_size]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	time(&t);
	srandom(t);
	r = random();

	ct = nfct_new();
	if (!ct) {
		perror("nfct_new");
		return 0;
	}

	h = nfct_open(CONNTRACK, 0);
	if (!h) {
		perror("nfct_open");
		nfct_destroy(ct);
		return -1;
	}

	for (i = r, j = 0;i < (r + atoi(argv[1]) * 2); i++, j++) {
		nfct_set_attr_u8(ct, ATTR_L3PROTO, AF_INET);
		nfct_set_attr_u32(ct, ATTR_IPV4_SRC, inet_addr("1.1.1.1") + i);
		nfct_set_attr_u32(ct, ATTR_IPV4_DST, inet_addr("2.2.2.2") + i);

		nfct_set_attr_u8(ct, ATTR_L4PROTO, IPPROTO_TCP);
		nfct_set_attr_u16(ct, ATTR_PORT_SRC, htons(10));
		nfct_set_attr_u16(ct, ATTR_PORT_DST, htons(20));

		nfct_setobjopt(ct, NFCT_SOPT_SETUP_REPLY);

		nfct_set_attr_u8(ct, ATTR_TCP_STATE, TCP_CONNTRACK_ESTABLISHED);
		nfct_set_attr_u32(ct, ATTR_TIMEOUT, 1000);
		nfct_set_attr_u32(ct, ATTR_STATUS, IPS_ASSURED);

		if (i % 10000 == 0)
			printf("added %d flow entries\n", j);

		ret = nfct_query(h, NFCT_Q_CREATE, ct);
		if (ret == -1)
			perror("nfct_query: ");
	}
	nfct_close(h);

	nfct_destroy(ct);

	exit(EXIT_SUCCESS);
}
