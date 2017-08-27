#include "../src/utils.h"
#include <signal.h>

static int quit = 0;

static void change_cb(struct nl_cache *cache, struct nl_object *obj,
		      int action)
{
	struct nl_dump_params dp = {
		.dp_type = NL_DUMP_LINE,
		.dp_fd = stdout,
	};

	if (action == NL_ACT_NEW)
		printf("NEW ");
	else if (action == NL_ACT_DEL)
		printf("DEL ");
	else if (action == NL_ACT_CHANGE)
		printf("CHANGE ");

	nl_object_dump(obj, &dp);
}

static void sigint(int arg)
{
	quit = 1;
}

int main(int argc, char *argv[])
{
	struct nl_cache_mngr *mngr;
	struct nl_cache *lc, *nc, *ac, *rc;
	struct nl_sock *sock;
	int err;

	signal(SIGINT, sigint);

	sock = nlt_alloc_socket();
	err = nl_cache_mngr_alloc(sock, NETLINK_ROUTE, NL_AUTO_PROVIDE, &mngr);
	if (err < 0)
		fatal(err, "Unable to allocate cache manager: %s",
		      nl_geterror(err));

	if ((err = nl_cache_mngr_add(mngr, "route/link", &change_cb, &lc)) < 0)
		fatal(err, "Unable to add cache route/link: %s",
		      nl_geterror(err));

	if ((err = nl_cache_mngr_add(mngr, "route/neigh", &change_cb, &nc)) < 0)
		fatal(err, "Unable to add cache route/neigh: %s",
		      nl_geterror(err));

	if ((err = nl_cache_mngr_add(mngr, "route/addr", &change_cb, &ac)) < 0)
		fatal(err, "Unable to add cache route/addr: %s",
		      nl_geterror(err));

	if ((err = nl_cache_mngr_add(mngr, "route/route", &change_cb, &rc)) < 0)
		fatal(err, "Unable to add cache route/route: %s",
		      nl_geterror(err));

	while (!quit) {
		int err = nl_cache_mngr_poll(mngr, 5000);
		if (err < 0 && err != -NLE_INTR)
			fatal(err, "Polling failed: %s", nl_geterror(err));

	}

	nl_cache_mngr_free(mngr);
	nl_socket_free(sock);

	return 0;
}
