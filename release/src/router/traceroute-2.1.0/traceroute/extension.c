#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "traceroute.h"


struct icmp_ext_header {
#if __BYTE_ORDER == __BIG_ENDIAN
	unsigned int version:4;
	unsigned int reserved:4;
#else
	unsigned int reserved:4;
	unsigned int version:4;
#endif
	uint8_t reserved1;
	uint16_t checksum;
} __attribute__ ((packed));


struct icmp_ext_object {
	uint16_t length;
	uint8_t class;
	uint8_t c_type;
	uint8_t data[0];
};

#define MPLS_CLASS 1
#define MPLS_C_TYPE 1


#define do_snprintf(CURR, END, FMT, ARGS...)	\
	do {					\
	    CURR += snprintf (CURR, END - CURR, (FMT), ## ARGS);\
	    if (CURR > END)  CURR = END;			\
	} while (0)


static int try_extension (probe *pb, char *buf, size_t len) {
	struct icmp_ext_header *iext = (struct icmp_ext_header *) buf;
	char str[1024];
	char *curr = str;
	char *end = str + sizeof (str) / sizeof (*str);
	

	/*  a check for len >= 8 already done for all cases   */

	if (iext->version != 2)  return -1;

	if (iext->checksum &&
	    in_csum (iext, len) != (uint16_t) ~0
	)  return -1;

	buf += sizeof (*iext);
	len -= sizeof (*iext);


	while (len >= sizeof (struct icmp_ext_object)) {
	    struct icmp_ext_object *obj = (struct icmp_ext_object *) buf;
	    size_t objlen = ntohs (obj->length);
	    size_t data_len;
	    uint32_t *ui = (uint32_t *) obj->data;
	    int i, n;

	    if (objlen < sizeof (*obj) ||
		objlen > len
	    )  return -1;

	    data_len = objlen - sizeof (*obj);
	    if (data_len % sizeof (uint32_t))
		    return -1;	/*  must be 32bit rounded...  */

	    n = data_len / sizeof (*ui);


	    if (curr > (char *) str && curr < end)
		    *curr++ = ';';	/*  a separator   */

	    if (obj->class == MPLS_CLASS &&
		obj->c_type == MPLS_C_TYPE &&
		n >= 1
	    ) {    /*  people prefer MPLS to be parsed...  */

		do_snprintf (curr, end, "MPLS:");

		for (i = 0; i < n; i++, ui++) {
		    uint32_t mpls = ntohl (*ui);

		    do_snprintf (curr, end, "%sL=%u,E=%u,S=%u,T=%u",
					i ? "/" : "",
					mpls >> 12,
					(mpls >> 9) & 0x7,
					(mpls >> 8) & 0x1,
					mpls & 0xff);
		}

	    }
	    else {	/*  common case...  */

		do_snprintf (curr, end, "%u/%u:", obj->class, obj->c_type);

		for (i = 0; i < n && curr < end; i++, ui++)
		    do_snprintf (curr, end, "%s%08x", i ? "," : "", ntohl(*ui));
	    }

	    buf += objlen;
	    len -= objlen;
	}

	if (len)  return -1;


	pb->ext = strdup (str);

	return 0;
}


void handle_extensions (probe *pb, char *buf, int len, int step) {

	if (!step)
	    try_extension (pb, buf, len);
	else {
	    for ( ; len >= 8; buf += step, len -= step)
		if (try_extension (pb, buf, len) == 0)
			break;
	}

	return;
}

