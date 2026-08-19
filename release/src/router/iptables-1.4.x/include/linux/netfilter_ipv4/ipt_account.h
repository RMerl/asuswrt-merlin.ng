/* Header for the ipt_account match (Piotr Gasidlo heritage).
 * Restored: referenced by extensions/libipt_account.c but absent from tree. */
#ifndef _IPT_ACCOUNT_H_
#define _IPT_ACCOUNT_H_

#include <linux/types.h>

#define IPT_ACCOUNT_NAME_LEN 64

struct t_ipt_account_table;

struct t_ipt_account_info {
	__u32 network;
	__u32 netmask;
	char name[IPT_ACCOUNT_NAME_LEN + 1];
	__u16 shortlisting:1;
	/* kernel-internal */
	struct t_ipt_account_table *table;
};

#endif /* _IPT_ACCOUNT_H_ */
