/* A very simple skeleton code that implements a daemon that collects
 * conntrack statistics from ctnetlink.
 *
 * This example is placed in the public domain.
 */
#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <errno.h>

#include <libmnl/libmnl.h>
#include <linux/netlink.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_conntrack.h>

#include <sys/queue.h>

struct nstats {
	LIST_ENTRY(nstats)	list;

	uint8_t family;

	union {
		struct in_addr	ip;
		struct in6_addr ip6;
	};
	uint64_t pkts, bytes;
};

static LIST_HEAD(nstats_head, nstats) nstats_head;

static int parse_counters_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_COUNTERS_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case CTA_COUNTERS_PACKETS:
	case CTA_COUNTERS_BYTES:
		if (mnl_attr_validate(attr, MNL_TYPE_U64) < 0) {
			perror("mnl_attr_validate");
			return MNL_CB_ERROR;
		}
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

static void parse_counters(const struct nlattr *nest, struct nstats *ns)
{
	struct nlattr *tb[CTA_COUNTERS_MAX+1] = {};

	mnl_attr_parse_nested(nest, parse_counters_cb, tb);
	if (tb[CTA_COUNTERS_PACKETS])
		ns->pkts += be64toh(mnl_attr_get_u64(tb[CTA_COUNTERS_PACKETS]));

	if (tb[CTA_COUNTERS_BYTES])
		ns->bytes += be64toh(mnl_attr_get_u64(tb[CTA_COUNTERS_BYTES]));
}

static int parse_ip_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_IP_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case CTA_IP_V4_SRC:
	case CTA_IP_V4_DST:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
			perror("mnl_attr_validate");
			return MNL_CB_ERROR;
		}
		break;
	case CTA_IP_V6_SRC:
	case CTA_IP_V6_DST:
		if (mnl_attr_validate2(attr, MNL_TYPE_BINARY,
				       sizeof(struct in6_addr)) < 0) {
			perror("mnl_attr_validate2");
			return MNL_CB_ERROR;
		}
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

static void parse_ip(const struct nlattr *nest, struct nstats *ns)
{
	struct nlattr *tb[CTA_IP_MAX+1] = {};

	mnl_attr_parse_nested(nest, parse_ip_cb, tb);
	if (tb[CTA_IP_V4_SRC]) {
		struct in_addr *in = mnl_attr_get_payload(tb[CTA_IP_V4_SRC]);
		ns->ip = *in;
		ns->family = AF_INET;
	}
	if (tb[CTA_IP_V6_SRC]) {
		struct in6_addr *in = mnl_attr_get_payload(tb[CTA_IP_V6_SRC]);
		ns->ip6 = *in;
		ns->family = AF_INET6;
	}
}

static int parse_tuple_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_TUPLE_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case CTA_TUPLE_IP:
		if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0) {
			perror("mnl_attr_validate");
			return MNL_CB_ERROR;
		}
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

static void parse_tuple(const struct nlattr *nest, struct nstats *ns)
{
	struct nlattr *tb[CTA_TUPLE_MAX+1] = {};

	mnl_attr_parse_nested(nest, parse_tuple_cb, tb);
	if (tb[CTA_TUPLE_IP])
		parse_ip(tb[CTA_TUPLE_IP], ns);
}

static int data_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case CTA_TUPLE_ORIG:
	case CTA_COUNTERS_ORIG:
	case CTA_COUNTERS_REPLY:
		if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0) {
			perror("mnl_attr_validate");
			return MNL_CB_ERROR;
		}
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

static int data_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[CTA_MAX+1] = {};
	struct nfgenmsg *nfg = mnl_nlmsg_get_payload(nlh);
	struct nstats ns = {}, *cur, *new;

	mnl_attr_parse(nlh, sizeof(*nfg), data_attr_cb, tb);
	if (tb[CTA_TUPLE_ORIG])
		parse_tuple(tb[CTA_TUPLE_ORIG], &ns);

	if (tb[CTA_COUNTERS_ORIG])
		parse_counters(tb[CTA_COUNTERS_ORIG], &ns);

	if (tb[CTA_COUNTERS_REPLY])
		parse_counters(tb[CTA_COUNTERS_REPLY], &ns);

	/* Look up for existing statistics object ... */
	LIST_FOREACH(cur, &nstats_head, list) {
		if (memcmp(&ns.ip6, &cur->ip6, sizeof(struct in6_addr)) == 0) {
			/* ... and sum counters */
			cur->pkts += ns.pkts;
			cur->bytes += ns.bytes;
			return MNL_CB_OK;
		}
	}

	/* ... if it does not exist, add new stats object */
	new = calloc(1, sizeof(struct nstats));
	if (!new)
		return MNL_CB_OK;

	new->family = ns.family;
	new->ip6 = ns.ip6;
	new->pkts = ns.pkts;
	new->bytes = ns.bytes;

	LIST_INSERT_HEAD(&nstats_head, new, list);

	return MNL_CB_OK;
}

static int handle(struct mnl_socket *nl)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	int ret;

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	if (ret == -1) {
		/* It only happens if NETLINK_NO_ENOBUFS is not set, it means
		 * we are leaking statistics.
		 */
		if (errno == ENOBUFS) {
			fprintf(stderr, "The daemon has hit ENOBUFS, you can "
					"increase the size of your receiver "
					"buffer to mitigate this or enable "
					"reliable delivery.\n");
		} else {
			perror("mnl_socket_recvfrom");
		}
		return -1;
	}

	ret = mnl_cb_run(buf, ret, 0, 0, data_cb, NULL);
	if (ret == -1) {
		perror("mnl_cb_run");
		return -1;
	} else if (ret <= MNL_CB_STOP)
		return 0;

	return 0;
}

int main(int argc, char *argv[])
{
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	struct nfgenmsg *nfh;
	struct nstats *cur;
	struct timeval tv = {};
	int ret, secs, on = 1, buffersize = (1 << 22);

	if (argc != 2) {
		printf("Usage: %s <poll-secs>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	secs = atoi(argv[1]);

	LIST_INIT(&nstats_head);

	printf("Polling every %d seconds from kernel...\n", secs);

	/* Set high priority for this process, less chances to overrun
	 * the netlink receiver buffer since the scheduler gives this process
	 * more chances to run.
	 */
	nice(-20);

	/* Open netlink socket to operate with netfilter */
	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL) {
		perror("mnl_socket_open");
		exit(EXIT_FAILURE);
	}

	/* Subscribe to destroy events to avoid leaking counters. The same
	 * socket is used to periodically atomically dump and reset counters.
	 */
	if (mnl_socket_bind(nl, NF_NETLINK_CONNTRACK_DESTROY,
				MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		exit(EXIT_FAILURE);
	}

	/* Set netlink receiver buffer to 16 MBytes, to avoid packet drops */
	setsockopt(mnl_socket_get_fd(nl), SOL_SOCKET, SO_RCVBUFFORCE,
		   &buffersize, sizeof(socklen_t));

	/* The two tweaks below enable reliable event delivery, packets may
	 * be dropped if the netlink receiver buffer overruns. This happens ...
	 *
	 * a) if the kernel spams this user-space process until the receiver
	 *    is filled.
	 *
	 * or:
	 *
	 * b) if the user-space process does not pull messages from the
	 *    receiver buffer so often.
	 */
	mnl_socket_setsockopt(nl, NETLINK_BROADCAST_ERROR, &on, sizeof(int));
	mnl_socket_setsockopt(nl, NETLINK_NO_ENOBUFS, &on, sizeof(int));

	nlh = mnl_nlmsg_put_header(buf);
	/* Counters are atomically zeroed in each dump */
	nlh->nlmsg_type = (NFNL_SUBSYS_CTNETLINK << 8) |
			  IPCTNL_MSG_CT_GET_CTRZERO;
	nlh->nlmsg_flags = NLM_F_REQUEST|NLM_F_DUMP;

	nfh = mnl_nlmsg_put_extra_header(nlh, sizeof(struct nfgenmsg));
	nfh->nfgen_family = AF_INET;
	nfh->version = NFNETLINK_V0;
	nfh->res_id = 0;

	/* Filter by mark: We only want to dump entries whose mark is zero */
	mnl_attr_put_u32(nlh, CTA_MARK, htonl(0));
	mnl_attr_put_u32(nlh, CTA_MARK_MASK, htonl(0xffffffff));

	while (1) {
		int fd_max = mnl_socket_get_fd(nl);
		fd_set readfds;

		/* Every N seconds ... */
		if (tv.tv_sec == 0 && tv.tv_usec == 0) {
			/* ... request a fresh dump of the table from kernel */
			ret = mnl_socket_sendto(nl, nlh, nlh->nlmsg_len);
			if (ret == -1) {
				perror("mnl_socket_sendto");
				return -1;
			}
			tv.tv_sec = secs;
			tv.tv_usec = 0;

			/* print the content of the list */
			LIST_FOREACH(cur, &nstats_head, list) {
				char out[INET6_ADDRSTRLEN];

				if (inet_ntop(cur->family, &cur->ip, out, sizeof(out)))
					printf("src=%s ", out);

				printf("counters %"PRIu64" %"PRIu64"\n",
					cur->pkts, cur->bytes);
			}
		}

		FD_ZERO(&readfds);
		FD_SET(mnl_socket_get_fd(nl), &readfds);

		ret = select(fd_max+1, &readfds, NULL, NULL, &tv);
		if (ret < 0) {
			if (errno == EINTR)
				continue;

			perror("select");
			exit(EXIT_FAILURE);
		}

		/* Handled event and periodic atomic-dump-and-reset messages */
		if (FD_ISSET(mnl_socket_get_fd(nl), &readfds)) {
			if (handle(nl) < 0)
				return EXIT_FAILURE;
		}
	}

	mnl_socket_close(nl);

	return 0;
}
