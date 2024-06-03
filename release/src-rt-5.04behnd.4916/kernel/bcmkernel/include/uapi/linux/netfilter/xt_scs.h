#ifndef _XT_SCS_H
#define _XT_SCS_H

#include <linux/types.h>

struct xt_scs_mtinfo {
    __u32 spis[2];  /* Security Parameter Index */
    __u16 udpesp_port;
    __u8  invflags; /* Inverse flags */
    __u8  up_val;
    
    /* Used internally by the kernel */
    void *data __attribute__((aligned(8)));
};

/* Values for "invflags" field in struct xt_scs. */
#define XT_SCS_INV_SPI	    0x01	/* Invert the sense of spi. */
#define XT_SCS_INV_UP_OVRD  0x02
#define XT_SCS_INV_MASK	    0x03	/* All possible flags. */
#define XT_SCS_VALID_UDPESP_L4P 0x08
#define XT_SCS_VALID_SPIS       0x10
#define XT_SCS_VALID_UP_OVRD    0x20
#define XT_SCS_VALID_UP_VAL     0x80

#endif /*_XT_SCS_H*/
