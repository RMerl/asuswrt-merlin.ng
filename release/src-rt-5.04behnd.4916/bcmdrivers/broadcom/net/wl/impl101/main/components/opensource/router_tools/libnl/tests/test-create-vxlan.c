#include <netlink/netlink.h>
#include <netlink/route/link.h>
#include <netlink/route/link/vxlan.h>

int main(int argc, char *argv[])
{
	struct rtnl_link *link;
	struct nl_addr *addr;
	struct nl_sock *sk;
	int err;

	sk = nl_socket_alloc();
	if ((err = nl_connect(sk, NETLINK_ROUTE)) < 0) {
		nl_perror(err, "Unable to connect socket");
		return err;
	}

	link = rtnl_link_vxlan_alloc();

	rtnl_link_set_name(link, "vxlan128");

	if ((err = rtnl_link_vxlan_set_id(link, 128)) < 0) {
		nl_perror(err, "Unable to set VXLAN network identifier");
		return err;
	}

	if ((err = nl_addr_parse("239.0.0.1", AF_INET, &addr)) < 0) {
		nl_perror(err, "Unable to parse IP address");
		return err;
	}

	if ((err = rtnl_link_vxlan_set_group(link, addr)) < 0) {
		nl_perror(err, "Unable to set multicast IP address");
		return err;
	}
	nl_addr_put(addr);

	if ((err = rtnl_link_add(sk, link, NLM_F_CREATE)) < 0) {
		nl_perror(err, "Unable to add link");
		return err;
	}

	rtnl_link_put(link);
	nl_close(sk);

	return 0;
}
