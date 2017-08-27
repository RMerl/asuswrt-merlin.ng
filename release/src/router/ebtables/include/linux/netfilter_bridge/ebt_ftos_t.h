#ifndef __LINUX_BRIDGE_EBT_FTOS_T_H
#define __LINUX_BRIDGE_EBT_FTOS_T_H

struct ebt_ftos_t_info
{
   int           ftos_set;
	unsigned char ftos;
	// EBT_ACCEPT, EBT_DROP or EBT_CONTINUE or EBT_RETURN
	int target;
};
#define EBT_FTOS_TARGET "ftos"

#define FTOS_TARGET       0x01
#define FTOS_SETFTOS      0x02
#define FTOS_WMMFTOS      0x04
#define FTOS_8021QFTOS    0x08

#endif
