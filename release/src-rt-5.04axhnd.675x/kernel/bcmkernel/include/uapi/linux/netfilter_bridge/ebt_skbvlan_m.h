#ifndef __LINUX_BRIDGE_EBT_SKBVLAN_H
#define __LINUX_BRIDGE_EBT_SKBVLAN_H

#include <linux/types.h>

#define EBT_SKBVLAN_ID	0x0001
#define EBT_SKBVLAN_PRIO	0x0002
#define EBT_SKBVLAN_ENCAP	0x0004
#define EBT_SKBVLAN_VLAN_TAG_0	0x0008
#define EBT_SKBVLAN_VLAN_TAG_1   0x0010
#define EBT_SKBVLAN_VLAN_TAG_2	0x0020
#define EBT_SKBVLAN_VLAN_TAG_3	0x0040
#define EBT_SKBVLAN_VLAN_TPID_0	0x0080
#define EBT_SKBVLAN_VLAN_TPID_1	0x0100
#define EBT_SKBVLAN_MASK (EBT_SKBVLAN_ID | EBT_SKBVLAN_PRIO | EBT_SKBVLAN_ENCAP | \
		EBT_SKBVLAN_VLAN_TAG_0 | EBT_SKBVLAN_VLAN_TAG_1 | EBT_SKBVLAN_VLAN_TAG_2 | \
		EBT_SKBVLAN_VLAN_TAG_3 |EBT_SKBVLAN_VLAN_TPID_0 | EBT_SKBVLAN_VLAN_TPID_1)



#define EBT_SKBVLAN_MATCH "skbvlan"

struct ebt_skbvlan_m_info {
	__u16 id;		/* VLAN ID {1-4095} */
	__u8 prio;		/* VLAN User Priority {0-7} */
	__be16 encap;		/* VLAN Encapsulated frame code {0-65535} */
	__u16 bitmask;		/* Args bitmask bit 1=1 - ID arg,
				   bit 2=1 User-Priority arg, bit 3=1 encap*/
	__u16 invflags;		/* Inverse bitmask  bit 1=1 - inversed ID arg, 
				   bit 2=1 - inversed Pirority arg */
	__be32 vlantag0[2];
	__be32 vlanmask0;
	__be32 vlantag1[2];
	__be32 vlanmask1;
	__be32 vlantag2[2];
	__be32 vlanmask2;
	__be32 vlantag3[2];
	__be32 vlanmask3;
	__be16 vlantpid0;
	__be16 vlantpid1;
};


#endif /* __LINUX_BRIDGE_EBT_SKBVLAN_H */

