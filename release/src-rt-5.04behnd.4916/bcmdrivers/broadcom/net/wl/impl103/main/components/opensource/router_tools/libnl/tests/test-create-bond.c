#include <netlink/netlink.h>
#include <netlink/route/link.h>
#include <netlink/route/link/bonding.h>

int main(int argc, char *argv[])
{
	struct rtnl_link *link;
	struct nl_sock *sk;
	int err;

	sk = nl_socket_alloc();
	if ((err = nl_connect(sk, NETLINK_ROUTE)) < 0) {
		nl_perror(err, "Unable to connect socket");
		return err;
	}

	link = rtnl_link_bond_alloc();
	rtnl_link_set_name(link, "my_bond");

	if ((err = rtnl_link_add(sk, link, NLM_F_CREATE)) < 0) {
		nl_perror(err, "Unable to add link");
		return err;
	}

	rtnl_link_put(link);
	nl_close(sk);

	return 0;
}
