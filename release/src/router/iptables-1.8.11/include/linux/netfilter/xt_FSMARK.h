#ifndef _XT_FSMARK_H
#define _XT_FSMARK_H

#include <linux/types.h>
// Note: multiple files have following defintion, see bcmdrivers/broadcom/include/bcm963xx/flwstats.h for locations
#define MAX_NF_FSMARK_QUERIES         128

struct xt_fsmark_tginfo {
    __u16    id;
    __u16    flags;
};

#define XT_FSMARK_VALID_ID      0x01

#endif /*_XT_FSMARK_H*/
