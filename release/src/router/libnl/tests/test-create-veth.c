#include <netlink/netlink.h>
#include <netlink/route/link.h>
#include <netlink/route/link/veth.h>

int main(int argc, char *argv[])
{
	struct rtnl_link *link;
	struct nl_sock *sk;
	int err;
	struct rtnl_link *peer;

	sk = nl_socket_alloc();
	if ((err = nl_connect(sk, NETLINK_ROUTE)) < 0) {
		nl_perror(err, "Unable to connect socket");
		return err;
	}

#if 0
	rtnl_link_veth_add(sk, "veth2", "veth3", getpid());
#else
	link = rtnl_link_veth_alloc();
	if (!link) {
		nl_perror(err, "Unable to alloc link");
		return err;
	}
		
	rtnl_link_set_name(link, "veth8");
	peer = rtnl_link_veth_get_peer(link);
	rtnl_link_set_name(peer, "veth9");

	if ((err = rtnl_link_add(sk, link, NLM_F_CREATE)) < 0) {
		nl_perror(err, "Unable to add link");
		return err;
	}
	printf("peer is %s\n", rtnl_link_get_name(peer));
	rtnl_link_put(peer);
	rtnl_link_put(link);
#endif
	nl_close(sk);

	return 0;
}
