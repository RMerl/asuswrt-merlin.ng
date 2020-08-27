#ifndef _XT_MAC_H
#define _XT_MAC_H

/*
oringinal version
*/
/*
struct xt_mac_info {
    unsigned char srcaddr[ETH_ALEN];
    int invert;
};
*/

////////////////////////////////////////

/*
hack version
*/
/*linux-2.6.19*/
#define MAC_SRC                 0x01    /* Match source MAC address */
#define SRC_MASK                0x02    /* Source MAC mask */
#define MAC_DST                 0x04    /* Match destination MAC address */
#define DST_MASK                0x08    /* Destination MAC mask */
#define MAC_SRC_INV             0x10    /* Negate the condition */
#define SRC_MASK_INV            0x20    /* Negate the condition */
#define MAC_DST_INV             0x40    /* Negate the condition */
#define DST_MASK_INV            0x80    /* Negate the condition */

struct xt_mac_info {
    unsigned char srcaddr[ETH_ALEN];
    /*linux-2.6.19*/
    // Kaohj -- add mac mask
    unsigned char srcmask[ETH_ALEN];
    // Added by Mason Yu for dst MAC
    unsigned char dstaddr[ETH_ALEN];
    // Kaohj -- add mac mask
    unsigned char dstmask[ETH_ALEN];
    //int invert;
    /* Flags from above */
    u_int8_t flags;
};

#endif /*_XT_MAC_H*/
