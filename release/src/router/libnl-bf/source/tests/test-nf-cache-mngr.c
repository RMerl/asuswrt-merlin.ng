#include "../src/utils.h"

static void change_cb(struct nl_cache *cache, struct nl_object *obj,
		      int action)
{
	struct nfnl_ct *ct = (struct nfnl_ct *) obj;
	static struct nl_addr *hack = NULL;

	if (!hack)
		hack = nl_addr_parse("194.88.212.233", AF_INET);

	if (!nl_addr_cmp(hack, nfnl_ct_get_src(ct, 1)) ||
	    !nl_addr_cmp(hack, nfnl_ct_get_dst(ct, 1))) {
		struct nl_dump_params dp = {
			.dp_type = NL_DUMP_LINE,
			.dp_fd = stdout,
		};

		printf("UPDATE ");
		nl_object_dump(obj, &dp);
	}
}

int main(int argc, char *argv[])
{
	struct nl_cache_mngr *mngr;
	struct nl_sock *sock;
	struct nl_cache *ct;

	sock = nlt_socket_alloc();

	mngr = nl_cache_mngr_alloc(sock, NETLINK_NETFILTER, NL_AUTO_PROVIDE);
	if (!mngr) {
		nl_perror("nl_cache_mngr_alloc");
		return -1;
	}

	ct = nl_cache_mngr_add(mngr, "netfilter/ct", &change_cb);
	if (ct == NULL) {
		nl_perror("nl_cache_mngr_add(netfilter/ct)");
		return -1;
	}

	for (;;) {
		int err = nl_cache_mngr_poll(mngr, 5000);
		if (err < 0) {
			nl_perror("nl_cache_mngr_poll()");
			return -1;
		}

	}

	nl_cache_mngr_free(mngr);

	return 0;
}
