#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

static int event_cb(enum nf_conntrack_msg_type type,
		    struct nf_conntrack *ct,
		    void *data)
{
	static int n = 0;
	char buf[1024];

	nfct_snprintf(buf, sizeof(buf), ct, type, NFCT_O_XML, NFCT_OF_TIME | NFCT_OF_TIMESTAMP);
	printf("%s\n", buf);

	if (++n == 10)
		return NFCT_CB_STOP;

	return NFCT_CB_CONTINUE;
}

int main(void)
{
	int ret;
	struct nfct_handle *h;

	h = nfct_open(CONNTRACK, NFCT_ALL_CT_GROUPS);
	if (!h) {
		perror("nfct_open");
		return 0;
	}

	nfct_callback_register(h, NFCT_T_ALL, event_cb, NULL);

	printf("TEST: waiting for 10 events...\n");

	ret = nfct_catch(h);

	printf("TEST: conntrack events ");
	if (ret == -1)
		printf("(%d)(%s)\n", ret, strerror(errno));
	else
		printf("(OK)\n");

	nfct_close(h);

	ret == -1 ? exit(EXIT_FAILURE) : exit(EXIT_SUCCESS);
}
