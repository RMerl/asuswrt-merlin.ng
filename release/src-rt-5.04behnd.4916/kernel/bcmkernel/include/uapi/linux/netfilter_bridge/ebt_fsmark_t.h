#ifndef __LINUX_BRIDGE_EBT_FSMARK_T_H
#define __LINUX_BRIDGE_EBT_FSMARK_T_H

#include <linux/types.h>

struct ebt_fsmark_t_info {
    __u8    id;
    int     target;     // EBT_ACCEPT, EBT_DROP, EBT_CONTINUE or EBT_RETURN
};

#endif /*__LINUX_BRIDGE_EBT_FSMARK_T_H*/
