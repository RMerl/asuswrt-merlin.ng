#if defined(CONFIG_BCM_KF_NETFILTER)
#ifndef __LINUX_QOS_MAP_H
#define __LINUX_QOS_MAP_H

struct ebt_qos_map_info
{
	int dscp2pbit; 
	int dscp2q;
};
#endif
#endif
