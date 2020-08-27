#ifndef _XT_STRING_H
#define _XT_STRING_H

#include <linux/types.h>

#define XT_STRING_MAX_PATTERN_SIZE 128
#define XT_STRING_MAX_ALGO_NAME_SIZE 16

enum {
	XT_STRING_FLAG_INVERT		= 0x01,
	XT_STRING_FLAG_IGNORECASE	= 0x02,
};

// Magician: Extended to URL, Domain, and URLALW
enum {
	XT_STRING_FLAG_STRING = 1,
	XT_STRING_FLAG_HEX_STRING,
	XT_STRING_FLAG_URL,
	XT_STRING_FLAG_DOMAIN,
	XT_STRING_FLAG_URLALW
};

struct xt_string_info {
	__u16 from_offset;
	__u16 to_offset;
	char algo[XT_STRING_MAX_ALGO_NAME_SIZE];
	char pattern[XT_STRING_MAX_PATTERN_SIZE];
	__u8 patlen;
	__u8 str_flag; // Magician: Extended to URL, Domain, and URLALW

	union {
		struct {
			__u8 invert;
		} v0;

		struct {
			__u8 flags;
		} v1;
	} u;

	/* Used internally by the kernel */
	struct ts_config __attribute__((aligned(8))) *config;
};

#endif /*_XT_STRING_H*/
