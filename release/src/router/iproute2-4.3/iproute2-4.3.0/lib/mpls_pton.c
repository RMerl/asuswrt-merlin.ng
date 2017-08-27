#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <linux/mpls.h>

#include "utils.h"


static int mpls_pton1(const char *name, struct mpls_label *addr)
{
	char *endp;
	unsigned count;

	for (count = 0; count < MPLS_MAX_LABELS; count++) {
		unsigned long label;

		label = strtoul(name, &endp, 0);
		/* Fail when the label value is out or range */
		if (label >= (1 << 20))
			return 0;

		if (endp == name) /* no digits */
			return 0;

		addr->entry = htonl(label << MPLS_LS_LABEL_SHIFT);
		if (*endp == '\0') {
			addr->entry |= htonl(1 << MPLS_LS_S_SHIFT);
			return 1;
		}

		/* Bad character in the address */
		if (*endp != '/')
			return 0;

		name = endp + 1;
		addr += 1;
	}
	/* The address was too long */
	return 0;
}

int mpls_pton(int af, const char *src, void *addr)
{
	int err;

	switch(af) {
	case AF_MPLS:
		errno = 0;
		err = mpls_pton1(src, (struct mpls_label *)addr);
		break;
	default:
		errno = EAFNOSUPPORT;
		err = -1;
	}

	return err;
}
