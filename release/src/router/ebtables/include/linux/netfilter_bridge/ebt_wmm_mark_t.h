#ifndef __LINUX_BRIDGE_EBT_MARK_T_H
#define __LINUX_BRIDGE_EBT_MARK_T_H

#define WMM_MARK_DSCP		1
#define WMM_MARK_8021D		2

#define WMM_MARK_DSCP_STR	"dscp"
#define WMM_MARK_8021D_STR	"vlan"

#define PRIO_LOC_NFMARK		16
#define PRIO_LOC_NFMASK		7	

#define WMM_DSCP_MASK_SHIFT	5
#define WMM_MARK_VALUE_NONE	-1


struct ebt_wmm_mark_t_info
{
	int mark; 
	int markpos;
	int markset;
	/* EBT_ACCEPT, EBT_DROP, EBT_CONTINUE or EBT_RETURN */
	int target;
};
#define EBT_WMM_MARK_TARGET "wmm-mark"

#endif
