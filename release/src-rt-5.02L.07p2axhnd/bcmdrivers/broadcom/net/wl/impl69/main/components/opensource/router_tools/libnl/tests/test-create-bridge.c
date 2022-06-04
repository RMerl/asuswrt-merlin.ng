#include <netlink/netlink.h>
#include <netlink/route/link.h>
#include <netlink/route/link/bridge.h>

#define TEST_BRIDGE_NAME "testbridge"
#define TEST_INTERFACE_NAME "testtap1"

int create_bridge(struct nl_sock *sk, struct nl_cache *link_cache, const char *name) {
	struct rtnl_link *link;
	int err;

	link = rtnl_link_alloc();
	if ((err = rtnl_link_set_type(link, "bridge")) < 0) {
		rtnl_link_put(link);
		return err;
	}
	rtnl_link_set_name(link, name);

	if ((err = rtnl_link_add(sk, link, NLM_F_CREATE)) < 0) {
		return err;
	}
	rtnl_link_put(link);

	return 0;
}

int main(int argc, char *argv[])
{
	struct rtnl_link *link;
	struct nl_cache *link_cache;
	struct nl_sock *sk;
	int err;

	sk = nl_socket_alloc();
	if ((err = nl_connect(sk, NETLINK_ROUTE)) < 0) {
		nl_perror(err, "Unable to connect socket");
		return err;
	}

	if ((err = rtnl_link_alloc_cache(sk, AF_UNSPEC, &link_cache)) < 0) {
		nl_perror(err, "Unable to allocate cache");
		return err;
	}

	if ((err = create_bridge(sk, link_cache, TEST_BRIDGE_NAME)) < 0) {
		nl_perror(err, "Unable to allocate testbridge");
		return err;
	}

	nl_cache_refill(sk, link_cache);

	link = rtnl_link_get_by_name(link_cache, TEST_BRIDGE_NAME);
	struct rtnl_link *ltap = rtnl_link_get_by_name(link_cache, TEST_INTERFACE_NAME);
	if (!ltap) {
		fprintf(stderr, "You should create a tap interface before lunch this test (# tunctl -t %s)\n", TEST_INTERFACE_NAME);
		return -1;
	}

	if ((err = rtnl_link_enslave(sk, link, ltap)) < 0) {
		nl_perror(err, "Unable to enslave interface to his bridge\n");
		return err;
	}

	if(rtnl_link_is_bridge(link) == 0) {
		fprintf(stderr, "Link is not a bridge\n");
		return -2;
	}
	if(rtnl_link_get_master(ltap) <= 0) {
		fprintf(stderr, "Interface is not attached to a bridge\n");
		return -3;
	}

	rtnl_link_put(ltap);
	rtnl_link_put(link);

	nl_cache_free(link_cache);
	nl_socket_free(sk);

	return 0;
}
