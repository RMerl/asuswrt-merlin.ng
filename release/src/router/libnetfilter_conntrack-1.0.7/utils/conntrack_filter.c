#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack_tcp.h>

static int event_cb(enum nf_conntrack_msg_type type,
		    struct nf_conntrack *ct,
		    void *data)
{
	static int n = 0;
	char buf[1024];

	nfct_snprintf(buf, sizeof(buf), ct, type, NFCT_O_PLAIN, NFCT_OF_TIME);
	printf("%s\n", buf);

	if (++n == 10)
		return NFCT_CB_STOP;

	return NFCT_CB_CONTINUE;
}

int main(void)
{
	int ret;
	struct nfct_handle *h;
	struct nfct_filter *filter;

	h = nfct_open(CONNTRACK, NF_NETLINK_CONNTRACK_NEW | 
				 NF_NETLINK_CONNTRACK_UPDATE);
	if (!h) {
		perror("nfct_open");
		return 0;
	}

	filter = nfct_filter_create();
	if (!filter) {
		perror("nfct_create_filter");
		return 0;
	}

	nfct_filter_add_attr_u32(filter, NFCT_FILTER_L4PROTO, IPPROTO_UDP);
	nfct_filter_add_attr_u32(filter, NFCT_FILTER_L4PROTO, IPPROTO_TCP);

	struct nfct_filter_proto filter_proto = {
		.proto = IPPROTO_TCP,
		.state = TCP_CONNTRACK_ESTABLISHED
	};

	nfct_filter_add_attr(filter, NFCT_FILTER_L4PROTO_STATE, &filter_proto);

	/* BSF always wants data in host-byte order */
	struct nfct_filter_ipv4 filter_ipv4 = {
		.addr = ntohl(inet_addr("127.0.0.1")),
		.mask = 0xffffffff,
	};

	/* ignore whatever that comes from 127.0.0.1 */
	nfct_filter_set_logic(filter,
			      NFCT_FILTER_SRC_IPV4,
			      NFCT_FILTER_LOGIC_NEGATIVE);

	nfct_filter_add_attr(filter, NFCT_FILTER_SRC_IPV4, &filter_ipv4);

	/* BSF always wants data in host-byte order */
	struct nfct_filter_ipv6 filter_ipv6 = {
		.addr = { 0x0, 0x0, 0x0, 0x1 },
		.mask = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff },
	}; 

	/* ignore whatever that comes from ::1 (loopback) */
	nfct_filter_set_logic(filter,
			      NFCT_FILTER_SRC_IPV6,
			      NFCT_FILTER_LOGIC_NEGATIVE);

	nfct_filter_add_attr(filter, NFCT_FILTER_SRC_IPV6, &filter_ipv6);

	if (nfct_filter_attach(nfct_fd(h), filter) == -1) {
		perror("nfct_filter_attach");
		return 0;
	}

	/* release the filter object, this does not detach the filter */
	nfct_filter_destroy(filter);

	nfct_callback_register(h, NFCT_T_ALL, event_cb, NULL);

	printf("TEST: waiting for 10 events...\n");

	ret = nfct_catch(h);

	printf("TEST: conntrack events ");
	if (ret == -1)
		printf("(%d)(%s)\n", ret, strerror(errno));
	else
		printf("(OK)\n");

	nfct_close(h);

	ret == -1 ? exit(EXIT_FAILURE) : exit(EXIT_SUCCESS);
}
