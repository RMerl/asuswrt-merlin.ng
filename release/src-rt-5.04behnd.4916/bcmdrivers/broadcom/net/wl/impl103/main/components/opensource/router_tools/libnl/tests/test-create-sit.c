#include <netlink/route/link/sit.h>
#include <netlink-private/netlink.h>

int main(int argc, char *argv[])
{
	struct nl_cache *link_cache;
	struct rtnl_link *link;
	struct in_addr addr;
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

	if_index = rtnl_link_name2i(link_cache, "eno16777736");
	if (!if_index) {
		fprintf(stderr, "Unable to lookup eno16777736");
		return -1;
	}

	link = rtnl_link_sit_alloc();
	if(!link) {
		nl_perror(err, "Unable to allocate link");
		return -1;

	}
	rtnl_link_set_name(link, "sit-tun");
	rtnl_link_sit_set_link(link, if_index);

	inet_pton(AF_INET, "192.168.254.12", &addr.s_addr);
	rtnl_link_sit_set_local(link, addr.s_addr);

	inet_pton(AF_INET, "192.168.254.13", &addr.s_addr);
	rtnl_link_sit_set_remote(link, addr.s_addr);

	rtnl_link_sit_set_ttl(link, 64);
	err = rtnl_link_add(sk, link, NLM_F_CREATE);
	if (err < 0) {
		nl_perror(err, "Unable to add link");
		return err;
	}

	rtnl_link_put(link);
	nl_close(sk);

	return 0;
}
