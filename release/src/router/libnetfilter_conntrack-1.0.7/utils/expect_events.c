#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

static int event_cb(enum nf_conntrack_msg_type type,
		    struct nf_expect *exp,
		    void *data)
{
	static int n = 0;
	char buf[1024];

	nfexp_snprintf(buf, 1024, exp, type, NFCT_O_XML, 0);
	printf("%s\n", buf);

	if (++n == 10)
		return NFCT_CB_STOP;

	return NFCT_CB_CONTINUE;
}

int main(void)
{
	int ret;
	struct nfct_handle *h;

	h = nfct_open(EXPECT, NF_NETLINK_CONNTRACK_EXP_NEW |
			      NF_NETLINK_CONNTRACK_EXP_UPDATE |
			      NF_NETLINK_CONNTRACK_EXP_DESTROY);
	if (!h) {
		perror("nfct_open");
		return -1;
	}

	nfexp_callback_register(h, NFCT_T_ALL, event_cb, NULL);

	printf("TEST: waiting for 10 expectation events...\n");

	ret = nfexp_catch(h);

	printf("TEST: expectation events ");
	if (ret == -1)
		printf("(%d)(%s)\n", ret, strerror(errno));
	else
		printf("(OK)\n");

	nfct_close(h);

	ret == -1 ? exit(EXIT_FAILURE) : exit(EXIT_SUCCESS);
}
