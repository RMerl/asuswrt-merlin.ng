#ifndef _XT_CONDITION_H
#define _XT_CONDITION_H

enum {
	CONDITION_NAME_LEN = 31,
};

struct xt_condition_mtinfo {
	char name[CONDITION_NAME_LEN];
	__u8 invert;

	/* Used internally by the kernel */
	void *condvar __attribute__((aligned(8)));
};

#endif /* _XT_CONDITION_H */
