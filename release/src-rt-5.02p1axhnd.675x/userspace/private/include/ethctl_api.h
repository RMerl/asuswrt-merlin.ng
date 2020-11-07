/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
 ************************************************************************/

#ifndef _ETHCTL_API_H_
#define _ETHCTL_API_H_

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <bcmnet.h>
#include <bcm/bcmswapitypes.h>
#include <bcm/bcmswapistat.h>

/***************************************************************************
    Function: Get PHY configuration, speed, duplex from ethernet driver
    Input: *ifname
    Output: *speed: current link speed in Mbps unit, if 0, link is down.
            *duplex: current link duplex
            *phycfg: Values are defined in 
                bcmdrivers/opensource/include/bcm963xx/bcm/bcmswapitypes.h, phy_cfg_flag
            *subport: subport with highest current link up speed.
****************************************************************************/
int bcm_get_linkspeed(char *ifname, int *speed, int *duplex, enum phy_cfg_flag *phycfg, int *subport);
int mdio_read(int skfd, struct ifreq *ifr, int phy_id, int location);
void mdio_write(int skfd, struct ifreq *ifr, int phy_id, int location, int value);
int get_link_speed(int skfd, struct ifreq *ifr, int phy_id, int sub_port);

static inline int et_dev_subports_query(int skfd, struct ifreq *ifr)
{
    int port_list = 0;
    void *ifr_data_bk = ifr->ifr_data;

    ifr->ifr_data = (char*)&port_list;
    if (ioctl(skfd, SIOCGQUERYNUMPORTS, ifr) < 0) {
        fprintf(stderr, "Error: Interface %s ioctl SIOCGQUERYNUMPORTS error!\n", ifr->ifr_name);
        ifr->ifr_data = ifr_data_bk;
        return -1;
    }
    ifr->ifr_data = ifr_data_bk;
    return port_list;;
}

static inline int get_bit_count(int bitmap)
{
    int i, j = 0;
    for(i=0; i<32; i++)
        if((bitmap & (1<<i)))
            j++;
    return j;
}

/* 
    ifr->ifr_data contains ethctl pointer 
    Output: ethctl->phy_addr contains PHY id, same as the return value.
            ethctl->flag contains return flag for PHY ID
            The rest fields in ethctl will be protected
*/
static inline int et_get_phyid2(int skfd, struct ifreq *ifr, int sub_port)
{
	struct ethctl_data *_ethctl = ifr->ifr_data;
	struct ethctl_data __ethctl, *ethctl = &__ethctl;

    memset(ethctl, 0, sizeof(*ethctl));
    ifr->ifr_data = ethctl;
    ethctl->sub_port = sub_port;
    ethctl->op = ETHGETPHYID;

    if (ioctl(skfd, SIOCETHCTLOPS, ifr) < 0)
        return -1;

	/* PHY flag is returned and carried in ethctl->flags */
    _ethctl->phy_addr = ethctl->phy_addr;
    _ethctl->flags = ethctl->flags;
	ifr->ifr_data = _ethctl;
	
    return _ethctl->phy_addr;
}

static inline char *print_map_bits(int bmap)
{
    int i, c;
    static char buf[256];
    char *p = buf;
    int cnt = get_bit_count(bmap);

    for(buf[0]=0, i=0, c=0; bmap; i++) {
        if((bmap & (1<<i)) == 0) 
            continue;
        if(p != buf) {
            if (c == (cnt - 1))
                p += sprintf(p, " or ");
            else
                p += sprintf(p, ", ");
        }
        p += sprintf(p, "%d", i);
        c++;
        bmap &= ~(1<<i);
    }
    return buf;
}

/* 
    ifr->ifr_data contains pointer to valid ethctl_data 
    If sub_port == -1 and only single sub port under the crossbar
    The actual sub port will be set back into ethctl_data->sub_port
*/
static inline int et_get_phyid(int skfd, struct ifreq *ifr, int sub_port)
{
    int sub_port_map;
    struct ethctl_data *ethctl = ifr->ifr_data;
#define MAX_SUB_PORT_BITS (sizeof(int)*8)

    if ((sub_port_map = et_dev_subports_query(skfd, ifr)) < 0) {
        return -1;
    }

    if (sub_port_map > 0) {
        if (sub_port == -1) {
            if(get_bit_count(sub_port_map) > 1) {
                fprintf(stderr, "Error: %s has sub ports (%s), specify one of them by \"port <subport>\"\n",
                        ifr->ifr_name, print_map_bits(sub_port_map));
                return -1;
            }
            else if (get_bit_count(sub_port_map) == 1) {
                // get bit position
                for(sub_port = 0; sub_port < (int) MAX_SUB_PORT_BITS; sub_port++) {
                    if((sub_port_map & (1 << sub_port)))
                        break;
                }
            }
        }

        if ((sub_port_map & (1 << sub_port)) == 0) {
            fprintf(stderr, "Specified SubPort %d is not interface %s's member port with map 0x%x\n",
                    sub_port, ifr->ifr_name, sub_port_map);
            return -1;
        }
    } else {
        if (sub_port != -1) {
            fprintf(stderr, "Interface %s has no sub port\n", ifr->ifr_name);
            return -1;
        }
    }
    ethctl->sub_port = sub_port;

    return et_get_phyid2(skfd, ifr, sub_port);
}


#endif
