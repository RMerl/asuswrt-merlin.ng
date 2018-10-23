#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

static int events = 0;
static int new, update, destroy;

static int event_cb(enum nf_conntrack_msg_type type,
		    struct nf_conntrack *ct,
		    void *data)
{
	if (type == NFCT_T_NEW)
		new++;
	else if (type == NFCT_T_UPDATE)
		update++;
	else if (type == NFCT_T_DESTROY)
		destroy++;

	if ((++events % 10000) == 0)
		printf("%d events received (%d new, %d update, %d destroy)\n",
			events, new, update, destroy);

	return NFCT_CB_CONTINUE;
}

static void sighandler(int foo)
{
	printf("%d events received (%d new, %d update, %d destroy)\n",
		events, new, update, destroy);
	exit(EXIT_SUCCESS);
}

int main(void)
{
	int ret;
	struct nfct_handle *h;
	int on = 1;

	signal(SIGINT, sighandler);

	h = nfct_open(CONNTRACK, NFCT_ALL_CT_GROUPS);
	if (!h) {
		perror("nfct_open");
		return 0;
	}

	setsockopt(nfct_fd(h), SOL_NETLINK,
			NETLINK_BROADCAST_SEND_ERROR, &on, sizeof(int));
	setsockopt(nfct_fd(h), SOL_NETLINK,
			NETLINK_NO_ENOBUFS, &on, sizeof(int));

	nfct_callback_register(h, NFCT_T_ALL, event_cb, NULL);

	printf("TEST: waiting for events...\n");

	ret = nfct_catch(h);

	printf("TEST: conntrack events ");
	if (ret == -1)
		printf("(%d)(%s)\n", ret, strerror(errno));
	else
		printf("(OK)\n");

	nfct_close(h);

	ret == -1 ? exit(EXIT_FAILURE) : exit(EXIT_SUCCESS);
}
