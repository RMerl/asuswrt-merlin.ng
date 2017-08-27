#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <linux/mpls.h>

#include "utils.h"

static const char *mpls_ntop1(const struct mpls_label *addr, char *buf, size_t buflen)
{
	size_t destlen = buflen;
	char *dest = buf;
	int count;

	for (count = 0; count < MPLS_MAX_LABELS; count++) {
		uint32_t entry = ntohl(addr[count].entry);
		uint32_t label = (entry & MPLS_LS_LABEL_MASK) >> MPLS_LS_LABEL_SHIFT;
		int len = snprintf(dest, destlen, "%u", label);

		/* Is this the end? */
		if (entry & MPLS_LS_S_MASK)
			return buf;


		dest += len;
		destlen -= len;
		if (destlen) {
			*dest = '/';
			dest++;
			destlen--;
		}
	}
	errno = -E2BIG;
	return NULL;
}

const char *mpls_ntop(int af, const void *addr, char *buf, size_t buflen)
{
	switch(af) {
	case AF_MPLS:
		errno = 0;
		return mpls_ntop1((struct mpls_label *)addr, buf, buflen);
	default:
		errno = EAFNOSUPPORT;
	}

	return NULL;
}
