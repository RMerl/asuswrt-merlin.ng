/* Header for the webstr match (Broadcom/tomato heritage).
 * Restored: referenced by extensions/libipt_webstr.c but absent from tree. */
#ifndef _IPT_WEBSTR_H
#define _IPT_WEBSTR_H

#include <linux/types.h>

#ifndef BM_MAX_NLEN
#define BM_MAX_NLEN 256
#endif
#ifndef BM_MAX_HLEN
#define BM_MAX_HLEN 1024
#endif

typedef enum {
	IPT_WEBSTR_HOST,
	IPT_WEBSTR_URL,
	IPT_WEBSTR_CONTENT
} ipt_webstr_type;

struct ipt_webstr_info {
	char string[BM_MAX_NLEN];
	__u16 invert;
	__u16 len;
	__u8 type;
};

#endif /* _IPT_WEBSTR_H */
