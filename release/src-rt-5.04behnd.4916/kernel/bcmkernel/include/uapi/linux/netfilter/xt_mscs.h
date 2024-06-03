#ifndef _XT_MSCS_H
#define _XT_MSCS_H

#include <linux/types.h>

struct xt_mscs_mtinfo {
    __u8 up_bitmap;
    __u8 up_limit;
    __u8 is_global;
    
    /* Used internally by the kernel */
    void *data __attribute__((aligned(8)));
};

#endif /*_XT_MSCS_H*/
