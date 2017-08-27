#if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BRCM_DPI)
#ifndef __DPI_CTK_H__
#define __DPI_CTK_H__

#include<linux/skbuff.h>
#include <net/netfilter/nf_conntrack_acct.h>

#define APPID_STATUS_ONGOING    ((uint16_t) (1 << 0))
#define APPID_STATUS_IDENTIFIED ((uint16_t) (1 << 1))
#define APPID_STATUS_FINAL      ((uint16_t) (1 << 2))
#define APPID_STATUS_NOMORE     ((uint16_t) (1 << 3))
//#define APPID_STATUS_RESYNC     ((uint16_t) (1 << 4))
#define DEVID_STATUS_ONGOING    ((uint16_t) (1 << 5))
#define DEVID_STATUS_IDENTIFIED ((uint16_t) (1 << 6))
#define DEVID_STATUS_FINAL      ((uint16_t) (1 << 7))
#define DEVID_STATUS_NOMORE     ((uint16_t) (1 << 8))

#define CTK_INIT_FROM_WAN      ((uint16_t) (1 << 15))

#define IS_CTK_INIT_FROM_WAN(ct)  \
        ( ((ct)->dpi.flags & CTK_INIT_FROM_WAN) == CTK_INIT_FROM_WAN )

#endif /* __DPI_CTK_H__ */
#endif
