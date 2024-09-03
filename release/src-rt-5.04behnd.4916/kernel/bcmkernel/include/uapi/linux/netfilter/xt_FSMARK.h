#ifndef _XT_FSMARK_H
#define _XT_FSMARK_H

#include <linux/types.h>

#define MAX_NF_FSMARK_QUERIES         128    // FSMARK query ID 0..127  !!update below macro as necessary!!
#define MAX_NF_FSMARK_QUERY_PER_PKT     8    // Max # of FSMARK query per packet/flow

typedef struct {
    __u8       id      : 7;    // FSMARK query Id
    __u8       conds   : 1;    // entry used
} fsmark_cond_t;

typedef union {
    fsmark_cond_t a[MAX_NF_FSMARK_QUERY_PER_PKT];
    __u64         lw;
} fsmark_conds_t;    // in skb

typedef struct {
    __u32       id      : 7;
    __u32       is_mcast: 1;
    __u32       len     :24;
} fsmark_slow_stat_ctxt;

#define DEBUG_FSMARK(fmt, ...) printk("dbg>%s:L%d "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ERROR_FSMARK(fmt, ...) printk("%s:L%d "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

struct xt_fsmark_tginfo {
    __u8    id;
    __u8    flags;
};

#define XT_FSMARK_VALID_ID      0x01

#endif /*_XT_FSMARK_H*/
