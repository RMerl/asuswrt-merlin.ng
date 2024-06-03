#ifndef __LINUX_BRIDGE_EBT_REJECT_H
#define __LINUX_BRIDGE_EBT_REJECT_H

enum ebt_reject_with {
	EBT_ICMP6_POLICY_FAIL
};

struct ebt_reject_info {
	int	with;	/* reject type */
};

#endif
