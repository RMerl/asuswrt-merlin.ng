#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

static int cb(enum nf_conntrack_msg_type type,
	      struct nf_expect *exp,
	      void *data)
{
	char buf[1024];

	nfexp_snprintf(buf, 1024, exp, NFCT_T_UNKNOWN, NFCT_O_DEFAULT, 0);
	printf("%s\n", buf);

	return NFCT_CB_CONTINUE;
}

int main(void)
{
	int ret;
	uint8_t family = AF_INET;
	struct nfct_handle *h;

	h = nfct_open(EXPECT, 0);
	if (!h) {
		perror("nfct_open");
		return -1;
	}

	nfexp_callback_register(h, NFCT_T_ALL, cb, NULL);
	ret = nfexp_query(h, NFCT_Q_DUMP, &family);

	printf("TEST: dumo expectation ");
	if (ret == -1)
		printf("(%d)(%s)\n", ret, strerror(errno));
	else
		printf("(OK)\n");

	nfct_close(h);

	ret == -1 ? exit(EXIT_FAILURE) : exit(EXIT_SUCCESS);
}
