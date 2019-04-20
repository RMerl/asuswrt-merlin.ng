/* This example is placed in the public domain. */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <net/if.h>

#include <libmnl/libmnl.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>

int main(int argc, char *argv[])
{
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	struct rtmsg *rtm;
	uint32_t prefix, seq, portid;
	union {
		in_addr_t ip;
		struct in6_addr ip6;
	} dst;
	union {
		in_addr_t ip;
		struct in6_addr ip6;
	} gw;
	int iface, ret, family = AF_INET;

	if (argc <= 3) {
		printf("Usage: %s iface destination cidr [gateway]\n", argv[0]);
		printf("Example: %s eth0 10.0.1.12 32 10.0.1.11\n", argv[0]);
		printf("	 %s eth0 ffff::10.0.1.12 128 fdff::1\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	iface = if_nametoindex(argv[1]);
	if (iface == 0) {
		perror("if_nametoindex");
		exit(EXIT_FAILURE);
	}

	if (!inet_pton(AF_INET, argv[2], &dst)) {
		if (!inet_pton(AF_INET6, argv[2], &dst)) {
			perror("inet_pton");
			exit(EXIT_FAILURE);
		}
		family = AF_INET6;
	}

	if (sscanf(argv[3], "%u", &prefix) == 0) {
		perror("sscanf");
		exit(EXIT_FAILURE);
	}

	if (argc == 5 && !inet_pton(family, argv[4], &gw)) {
		perror("inet_pton");
		exit(EXIT_FAILURE);
	}

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type	= RTM_NEWROUTE;
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK;
	nlh->nlmsg_seq = seq = time(NULL);

	rtm = mnl_nlmsg_put_extra_header(nlh, sizeof(struct rtmsg));
	rtm->rtm_family = family;
	rtm->rtm_dst_len = prefix;
	rtm->rtm_src_len = 0;
	rtm->rtm_tos = 0;
	rtm->rtm_protocol = RTPROT_STATIC;
	rtm->rtm_table = RT_TABLE_MAIN;
	rtm->rtm_type = RTN_UNICAST;
	/* is there any gateway? */
	rtm->rtm_scope = (argc == 4) ? RT_SCOPE_LINK : RT_SCOPE_UNIVERSE;
	rtm->rtm_flags = 0;

	if (family == AF_INET)
		mnl_attr_put_u32(nlh, RTA_DST, dst.ip);
	else
		mnl_attr_put(nlh, RTA_DST, sizeof(struct in6_addr), &dst);

	mnl_attr_put_u32(nlh, RTA_OIF, iface);
	if (argc == 5) {
		if (family == AF_INET)
			mnl_attr_put_u32(nlh, RTA_GATEWAY, gw.ip);
		else {
			mnl_attr_put(nlh, RTA_GATEWAY, sizeof(struct in6_addr),
					&gw.ip6);
		}
	}

	nl = mnl_socket_open(NETLINK_ROUTE);
	if (nl == NULL) {
		perror("mnl_socket_open");
		exit(EXIT_FAILURE);
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		exit(EXIT_FAILURE);
	}
	portid = mnl_socket_get_portid(nl);

	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
		perror("mnl_socket_sendto");
		exit(EXIT_FAILURE);
	}

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	if (ret < 0) {
		perror("mnl_socket_recvfrom");
		exit(EXIT_FAILURE);
	}

	ret = mnl_cb_run(buf, ret, seq, portid, NULL, NULL);
	if (ret < 0) {
		perror("mnl_cb_run");
		exit(EXIT_FAILURE);
	}

	mnl_socket_close(nl);

	return 0;
}
