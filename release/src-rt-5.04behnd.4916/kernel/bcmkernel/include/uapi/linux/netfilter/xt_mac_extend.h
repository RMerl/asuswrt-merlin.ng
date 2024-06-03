#ifndef _XT_MAC_EXTEND_H
#define _XT_MAC_EXTEND_H

/* extend from xt_mac.h for MAC address extend match operations,
 * i.e, MAC/mask. 
 * BRCM, Jan, 31. 2019.
 */


struct xt_mac_info_extend {
    unsigned char srcaddr[ETH_ALEN];
    unsigned char msk[ETH_ALEN];
    int invert;
};
#endif /*_XT_MAC_EXTEND_H*/
