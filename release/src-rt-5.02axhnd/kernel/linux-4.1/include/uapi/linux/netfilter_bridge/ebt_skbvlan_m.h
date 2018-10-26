#ifndef __LINUX_BRIDGE_EBT_SKBVLAN_H
#define __LINUX_BRIDGE_EBT_SKBVLAN_H

#include <linux/types.h>

#define EBT_SKBVLAN_ID	0x01
#define EBT_SKBVLAN_PRIO	0x02
#define EBT_SKBVLAN_ENCAP	0x04
#define EBT_SKBVLAN_MASK (EBT_SKBVLAN_ID | EBT_SKBVLAN_PRIO | EBT_SKBVLAN_ENCAP)

#define EBT_SKBVLAN_MATCH "skbvlan"

struct ebt_skbvlan_m_info {
	__u16 id;		/* VLAN ID {1-4095} */
	__u8 prio;		/* VLAN User Priority {0-7} */
	__be16 encap;		/* VLAN Encapsulated frame code {0-65535} */
	__u8 bitmask;		/* Args bitmask bit 1=1 - ID arg,
				   bit 2=1 User-Priority arg, bit 3=1 encap*/
	__u8 invflags;		/* Inverse bitmask  bit 1=1 - inversed ID arg, 
				   bit 2=1 - inversed Pirority arg */
};


#endif /* __LINUX_BRIDGE_EBT_SKBVLAN_H */

