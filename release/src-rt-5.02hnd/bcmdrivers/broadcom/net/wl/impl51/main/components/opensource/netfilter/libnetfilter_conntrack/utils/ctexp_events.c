#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

static int n = 0;

static int
ct_event_cb(enum nf_conntrack_msg_type type,struct nf_conntrack *ct, void *data)
{
	char buf[1024];

	nfct_snprintf(buf, sizeof(buf), ct, type, NFCT_O_PLAIN, NFCT_OF_TIME);
	printf("[CT] %s\n", buf);

	if (++n == 20)
		return NFCT_CB_STOP;

	return NFCT_CB_CONTINUE;
}

static int
exp_event_cb(enum nf_conntrack_msg_type type,struct nf_expect *exp, void *data)
{
	char buf[1024];

	nfexp_snprintf(buf, 1024, exp, type, NFCT_O_DEFAULT, 0);
	printf("[EXP] %s\n", buf);

	if (++n == 20)
		return NFCT_CB_STOP;

	return NFCT_CB_CONTINUE;
}

int main(void)
{
	int ret = 0;
	struct nfct_handle *h;

	h = nfct_open(NFNL_SUBSYS_NONE, NF_NETLINK_CONNTRACK_EXP_NEW |
					NF_NETLINK_CONNTRACK_EXP_UPDATE |
					NF_NETLINK_CONNTRACK_EXP_DESTROY |
					NF_NETLINK_CONNTRACK_NEW |
					NF_NETLINK_CONNTRACK_UPDATE |
					NF_NETLINK_CONNTRACK_DESTROY);
	if (h == NULL) {
		perror("nfct_open");
		return -1;
	}

	nfexp_callback_register(h, NFCT_T_ALL, exp_event_cb, NULL);
	nfct_callback_register(h, NFCT_T_ALL, ct_event_cb, NULL);

	printf("TEST: waiting for 20 expectation events...\n");

	/* we may use nfexp_catch() instead, it would also work. */
	ret = nfct_catch(h);

	printf("TEST: expectation events ");
	if (ret == -1)
		printf("(%d)(%s)\n", ret, strerror(errno));
	else
		printf("(OK)\n");

	nfct_close(h);

	ret == -1 ? exit(EXIT_FAILURE) : exit(EXIT_SUCCESS);
}
