#include <netlink/route/link/ip6tnl.h>
#include <netlink-private/netlink.h>

int main(int argc, char *argv[])
{
	struct nl_cache *link_cache;
	struct rtnl_link *link;
        struct in6_addr addr;
	struct nl_sock *sk;
	int err, if_index;

	sk = nl_socket_alloc();
	if ((err = nl_connect(sk, NETLINK_ROUTE)) < 0) {
		nl_perror(err, "Unable to connect socket");
		return err;
	}

	err = rtnl_link_alloc_cache(sk, AF_UNSPEC, &link_cache);
	if ( err < 0) {
		nl_perror(err, "Unable to allocate cache");
		return err;
	}

	if_index = rtnl_link_name2i(link_cache, "ens33");
	if (!if_index) {
		fprintf(stderr, "Unable to lookup ens33");
		return -1;
	}

	link = rtnl_link_ip6_tnl_alloc();
	if(!link) {
		nl_perror(err, "Unable to allocate link");
		return -1;

	}
	rtnl_link_set_name(link, "ip6tnl-tun");
	rtnl_link_ip6_tnl_set_link(link, if_index);

	inet_pton(AF_INET6, "2607:f0d0:1002:51::4", &addr);
	rtnl_link_ip6_tnl_set_local(link, &addr);

	inet_pton(AF_INET6, "2607:f0d0:1002:52::5", &addr);
	rtnl_link_ip6_tnl_set_remote(link, &addr);

	err = rtnl_link_add(sk, link, NLM_F_CREATE);
	if (err < 0) {
		nl_perror(err, "Unable to add link");
		return err;
	}

	rtnl_link_put(link);
	nl_close(sk);

	return 0;
}
