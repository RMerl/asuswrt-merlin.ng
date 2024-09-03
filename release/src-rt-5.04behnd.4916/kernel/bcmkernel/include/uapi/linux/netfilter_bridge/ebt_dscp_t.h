#ifndef __LINUX_BRIDGE_EBT_DSCP_T_H
#define __LINUX_BRIDGE_EBT_DSCP_T_H

struct ebt_dscp_t_info
{
	int dscp;
	// EBT_ACCEPT, EBT_DROP or EBT_CONTINUE or EBT_RETURN
	int target;
};
#define EBT_DSCP_TARGET "dscp"

#define DSCP_TARGET       0x01
#define DSCP_SET      0x02

#endif
