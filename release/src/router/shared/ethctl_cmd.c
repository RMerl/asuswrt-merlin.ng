/***********************************************************************
 *
 *  Copyright (c) 2004-2010  Broadcom Corporation
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
 * <:copyright-BRCM:2004:proprietary:standard
 * 
 *    Copyright (c) 2004 Broadcom Corporation
 *    All Rights Reserved
 * 
 *  This program is the proprietary software of Broadcom Corporation and/or its
 *  licensors, and may only be used, duplicated, modified or distributed pursuant
 *  to the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied), right
 *  to use, or waiver of any kind with respect to the Software, and Broadcom
 *  expressly reserves all rights in and to the Software and all intellectual
 *  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 *  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 *  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 * 
 *  Except as expressly set forth in the Authorized License,
 * 
 *  1. This program, including its structure, sequence and organization,
 *     constitutes the valuable trade secrets of Broadcom, and you shall use
 *     all reasonable efforts to protect the confidentiality thereof, and to
 *     use this information only in connection with your use of Broadcom
 *     integrated circuit products.
 * 
 *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
 *     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
 *     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
 *     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
 *     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
 *     PERFORMANCE OF THE SOFTWARE.
 * 
 *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 *     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
 *     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
 *     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
 *     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
 *     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
 *     LIMITED REMEDY.
 * :>
 *
************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <errno.h>
#include <asm/param.h>
#include <fcntl.h>
typedef unsigned short u16;
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/sockios.h>
#include "ethctl.h"
#include "boardparms.h"
#include "bcmnet.h"
#include "bcm/bcmswapitypes.h"
#include <rtconfig.h>

#define PORT        "port"
static const char *media_names[] = {
    "10baseT", "10baseT-FD", "100baseTx", "100baseTx-FD", "100baseT4",
    "Flow-control", 0,
};

static struct ethctl_data ethctl;

static int mdio_read(int skfd, struct ifreq *ifr, int phy_id, int location)
{
    struct mii_ioctl_data *mii = (void *)&ifr->ifr_data;

    PHYID_2_MII_IOCTL(phy_id, mii);
    mii->reg_num = location;
    if (ioctl(skfd, SIOCGMIIREG, ifr) < 0) {
        fprintf(stderr, "SIOCGMIIREG on %s failed: %s\n", ifr->ifr_name,
            strerror(errno));
        return 0;
    }
    return mii->val_out;
}

#if 0
/* This structure is used in all SIOCxMIIxxx ioctl calls */
154 struct mii_ioctl_data {
155         __u16           phy_id;
156         __u16           reg_num;
157         __u16           val_in;
158         __u16           val_out;
159 };
160 
#endif

static void mdio_write(int skfd, struct ifreq *ifr, int phy_id, int location, int value)
{
    struct mii_ioctl_data *mii = (void *)&ifr->ifr_data;

    PHYID_2_MII_IOCTL(phy_id, mii);
    mii->reg_num = location;
    mii->val_in = value;

    if (ioctl(skfd, SIOCSMIIREG, ifr) < 0) {
        fprintf(stderr, "SIOCSMIIREG on %s failed: %s\n", ifr->ifr_name,
            strerror(errno));
    }
}

static int et_dev_subports_query(int skfd, struct ifreq *ifr)
{
    int port_list = 0;

    ifr->ifr_data = (char*)&port_list;
    if (ioctl(skfd, SIOCGQUERYNUMPORTS, ifr) < 0) {
        fprintf(stderr, "Error: Interface %s ioctl SIOCGQUERYNUMPORTS error!\n", ifr->ifr_name);
        return -1;
    }
    return port_list;;
}

static int get_bit_count(int bitmap)
{
    int i, j = 0;
    for(i=0; i<32; i++)
        if((bitmap & (1<<i)))
            j++;
    return j;
}
 
static int et_get_phyid2(int skfd, struct ifreq *ifr, int sub_port)
{
    unsigned long phy_id;
    struct mii_ioctl_data *mii = (void *)&ifr->ifr_data;

    mii->val_in = sub_port;

    if (ioctl(skfd, SIOCGMIIPHY, ifr) < 0)
        return -1;

    phy_id = MII_IOCTL_2_PHYID(mii);
    /*
     * returned phy id carries mii->val_out flags if phy is
     * internal/external phy/phy on ext switch.
     * we save it in higher byte to pass to kernel when 
     * phy is accessed.
     */
    return phy_id;
}

static int et_get_phyid(int skfd, struct ifreq *ifr, int sub_port)
{
    int sub_port_map;
#define MAX_SUB_PORT_BITS (sizeof(sub_port_map)*8)

    if ((sub_port_map = et_dev_subports_query(skfd, ifr)) < 0) {
        return -1;
    }

    if (sub_port_map > 0) {
        if (sub_port == -1) {
            if(get_bit_count(sub_port_map) > 1) {
                fprintf(stderr, "Error: Interface %s has sub ports, please specified one of port map: 0x%x\n", 
                        ifr->ifr_name, sub_port_map);
                return -1;
            }
            else if (get_bit_count(sub_port_map) == 1) {
                // get bit position 
                for(sub_port = 0; sub_port < MAX_SUB_PORT_BITS; sub_port++) {
                    if((sub_port_map & (1 << sub_port)))
                        break; 
                } 
            }
        }

        if ((sub_port_map & (1 << sub_port)) == 0) {
            fprintf(stderr, "Specified SubPort %d is not interface %s's member port with map %x\n", 
                    sub_port, ifr->ifr_name, sub_port_map);
            return -1;
        }
    } else {
        if (sub_port != -1) {
            fprintf(stderr, "Interface %s has no sub port\n", ifr->ifr_name);
            return -1;
        }
    }

    return et_get_phyid2(skfd, ifr, sub_port);
}

static int parse_media_options(char *option)
{
    int mode = -1;

    if (strcmp(option, "auto") == 0) {
        mode = MEDIA_TYPE_AUTO;
    } else if (strcmp(option, "100FD") == 0) {
        mode = MEDIA_TYPE_100M_FD;
    } else if (strcmp(option, "100HD") == 0) {
        mode = MEDIA_TYPE_100M_HD;
    } else if (strcmp(option, "10FD") == 0) {
        mode = MEDIA_TYPE_10M_FD;
    } else if (strcmp(option, "10HD") == 0) {
        mode = MEDIA_TYPE_10M_HD;
    } else if (strcmp(option, "1000FD") == 0) {
        mode = MEDIA_TYPE_1000M_FD;
    } else if (strcmp(option, "1000HD") == 0) {
        mode = MEDIA_TYPE_1000M_HD;
    } else if (strcmp(option, "2500FD") == 0) {
        mode = MEDIA_TYPE_2500M_FD;
    }
    return mode;
}

static char *print_speed(int bmcr)
{
    if (bmcr & BMCR_ANENABLE) return "Auto Detection";
    if (bmcr & BMCR_SPEED2500) return "2.5Gps"; 
    if (bmcr & BMCR_SPEED1000) return "1Gps"; 
    if (bmcr & BMCR_SPEED100) return "100Mbps";
    return "10Mbps";
}

static char *print_duplex(int bmcr)
{
    if (bmcr & BMCR_FULLDPLX)
        return "full";

    return "half";
}

static int show_speed_setting(int skfd, struct ifreq *ifr, int phy_id, int sub_port)
{
    int i, err, v16; struct ethswctl_data ifdata;
    int bmcr, bmsr, nway_advert, lkpar, gig_ctrl, gig_status;

    if(ETHCTL_GET_FLAG_FROM_PHYID(phy_id) & ETHCTL_FLAG_ACCESS_SERDES) {
        ifr->ifr_data = (void*) &ifdata;
        ifdata.op = ETHSWPHYMODE;
        ifdata.type = TYPE_GET;
        ifdata.addressing_flag = ETHSW_ADDRESSING_DEV;
        if (sub_port != -1) {
            ifdata.sub_unit = -1; /* Set sub_unit to -1 so that main unit of dev will be used */
            ifdata.sub_port = sub_port;
            ifdata.addressing_flag |= ETHSW_ADDRESSING_SUBPORT;
        }

        if((err = ioctl(skfd, SIOCETHSWCTLOPS, ifr))) {
            fprintf(stderr, "ioctl command return error %d!\n", err);
            return -1;
        }
        printf(" Serdes is Configured at Speed: %s\n", print_speed(ifdata.phycfg));

        if (ifdata.speed == 0) {
            printf("  Link is down.\n");
        }
        else {
            printf("  Link is up at %s, %s duplex\n", 
                print_speed(ifdata.speed), ifdata.duplex? "Full":"Half");
        }
        return 0;
    }

    bmcr = mdio_read(skfd, ifr, phy_id, MII_BMCR);
    bmsr = mdio_read(skfd, ifr, phy_id, MII_BMSR);

    if (bmcr == 0xffff  ||  bmsr == 0x0000) {
        fprintf(stderr, "  No MII transceiver present!.\n");
        return -1;
    }

    nway_advert = mdio_read(skfd, ifr, phy_id, MII_ADVERTISE);
    lkpar = mdio_read(skfd, ifr, phy_id, MII_LPA);

    if (bmcr & BMCR_ANENABLE) {
        printf("Auto-negotiation enabled.\n");

        gig_ctrl = mdio_read(skfd, ifr, phy_id, MII_CTRL1000);
        
        // check ethernet@wirspeed only for PHY support 1G 
        if(gig_ctrl & ADVERTISE_1000FULL || gig_ctrl & ADVERTISE_1000HALF) {
            // check if ethernet@wirespeed is enabled, reg 0x18, shodow 0b'111, bit4
            mdio_write(skfd, ifr, phy_id, 0x18, 0x7007);
            v16 = mdio_read(skfd, ifr, phy_id, 0x18);
            if(v16 & 0x0010) {
                // get link speed from ASR if ethernet@wirespeed is enabled
                v16 = mdio_read(skfd, ifr, phy_id, 0x19);
#define MII_ASR_1000(r) (((r & 0x0700) == 0x0700) || ((r & 0x0700) == 0x0600))
#define MII_ASR_100(r)  (((r & 0x0700) == 0x0500) || ((r & 0x0700) == 0x0300))
#define MII_ASR_10(r)   (((r & 0x0700) == 0x0200) || ((r & 0x0700) == 0x0100))
#define MII_ASR_FDX(r)  (((r & 0x0700) == 0x0700) || ((r & 0x0700) == 0x0500) || ((r & 0x0700) == 0x0200))

                if (MII_ASR_1000(v16) && MII_ASR_FDX(v16))
                    fprintf(stderr, "The autonegotiated media type is 1000BT Full Duplex \n");            
                else if (MII_ASR_1000(v16) && !MII_ASR_FDX(v16))
                    fprintf(stderr, "The autonegotiated media type is 1000BT Half Duplex \n");            
                else if (MII_ASR_100(v16) && MII_ASR_FDX(v16))
                    fprintf(stderr, "The autonegotiated media type is 100BT Full Duplex \n");            
                else if (MII_ASR_100(v16) && !MII_ASR_FDX(v16))
                    fprintf(stderr, "The autonegotiated media type is 100BT Half Duplex \n");            
                else if (MII_ASR_10(v16) && MII_ASR_FDX(v16))
                    fprintf(stderr, "The autonegotiated media type is 10BT Full Duplex \n");            
                else if (MII_ASR_10(v16) && !MII_ASR_FDX(v16))
                    fprintf(stderr, "The autonegotiated media type is 10BT Half Duplex \n");

                return 0;
            }            
        }

        gig_status = mdio_read(skfd, ifr, phy_id, MII_STAT1000);
        if ((gig_ctrl & ADVERTISE_1000FULL) && (gig_status & LPA_1000FULL)) {
            printf("The autonegotiated media type is 1000BT Full Duplex \n");            
        } else if ((gig_ctrl & ADVERTISE_1000HALF) && (gig_status & LPA_1000HALF)) {
            printf("The autonegotiated media type is 1000BT Half Duplex \n");            
        } else if (lkpar & ADVERTISE_LPACK) {
            int negotiated = nway_advert & lkpar & 
                            (ADVERTISE_100BASE4 |
                            ADVERTISE_100FULL |
                            ADVERTISE_100HALF |
                            ADVERTISE_10FULL |
                            ADVERTISE_10HALF );
            int max_capability = 0;
            /* Scan for the highest negotiated capability, highest priority
                (100baseTx-FDX) to lowest (10baseT-HDX). */
            int media_priority[] = {8, 9, 7, 6, 5};     /* media_names[i-5] */
            for (i = 0; media_priority[i]; i++) {
                if (negotiated & (1 << media_priority[i])) {
                    max_capability = media_priority[i];
                    break;
                }
            }
            if (max_capability)
                printf("The autonegotiated media type is %s.\n",
                    media_names[max_capability - 5]);
            else
                fprintf(stderr, "No common media type was autonegotiated!\n"
                    "This is extremely unusual and typically indicates a "
                    "configuration error.\n" "Perhaps the advertised "
                    "capability set was intentionally limited.\n");
        }
    } else {
        printf("Auto-negotiation disabled, with\n"
            " Speed fixed at %s, %s-duplex.\n", print_speed(bmcr), print_duplex(bmcr));

    }
    bmsr = mdio_read(skfd, ifr, phy_id, MII_BMSR);
    bmsr = mdio_read(skfd, ifr, phy_id, MII_BMSR);
    printf("Link is %s\n", (bmsr & BMSR_LSTATUS) ? "up" : "down");
    return 0;
}

static int et_cmd_media_type_op(int skfd, struct ifreq *ifr, cmd_t *cmd, char** argv)
{
    int phy_id = 0, sub_port = -1, err;
    int set = 0;
    int mode = 0;;
    int val = 0;
    int nway_advert, gig_ctrl, gig_cap = 0;
    struct ethswctl_data ifdata;

    for(argv += 3; *argv; argv++) {
        if (!strcmp(*argv, "port") && *(++argv))
            sub_port = strtol(*argv, NULL, 0);
        else {
            if ((mode = parse_media_options(*argv)) < 0)
                goto error;
            set = 1;
        }
    }

    if ((phy_id = et_get_phyid(skfd, ifr, sub_port)) == -1)
        goto error;

    if (set) {
        switch (mode) {
            case MEDIA_TYPE_AUTO:
                val = BMCR_ANENABLE | BMCR_ANRESTART;
                break;
            case MEDIA_TYPE_100M_FD:
                val = BMCR_SPEED100 | BMCR_FULLDPLX;
                break;
            case MEDIA_TYPE_100M_HD:
                val = BMCR_SPEED100;
                break;
            case MEDIA_TYPE_10M_FD:
                val = BMCR_FULLDPLX;
                break;
            case MEDIA_TYPE_10M_HD:
                val = 0;
                break;
            case MEDIA_TYPE_1000M_FD:
                val = BMCR_SPEED1000| BMCR_FULLDPLX;
                if(!(ETHCTL_GET_FLAG_FROM_PHYID(phy_id) & ETHCTL_FLAG_ACCESS_SERDES)) {
                    val |= (BMCR_ANENABLE | BMCR_ANRESTART);
                    gig_cap = ADVERTISE_1000HALF;
                }
                break;                
            case MEDIA_TYPE_1000M_HD:
                val = BMCR_SPEED1000;
                val |= (BMCR_ANENABLE | BMCR_ANRESTART);
                gig_cap = ADVERTISE_1000FULL;                
                break;                                
            case MEDIA_TYPE_2500M_FD:
                val = BMCR_SPEED2500| BMCR_FULLDPLX;
                break;                                
        }

        if(ETHCTL_GET_FLAG_FROM_PHYID(phy_id) & ETHCTL_FLAG_ACCESS_SERDES) {
            ifr->ifr_data = (void*) &ifdata;
            ifdata.op = ETHSWPHYMODE;
            ifdata.type = TYPE_SET;
            ifdata.addressing_flag = ETHSW_ADDRESSING_DEV;
            ifdata.phycfg = val;
            if (sub_port != -1) {
                ifdata.sub_unit = -1; /* Set sub_unit to -1 so that main unit of dev will be used */
                ifdata.sub_port = sub_port;
                ifdata.addressing_flag |= ETHSW_ADDRESSING_SUBPORT;
            }
            if((err = ioctl(skfd, SIOCETHSWCTLOPS, ifr))) {
                fprintf(stderr, "ioctl command return error %d!\n", err);
                return -1;
            }
        } else {
            nway_advert = mdio_read(skfd, ifr, phy_id, MII_ADVERTISE);
            gig_ctrl = mdio_read(skfd, ifr, phy_id, MII_CTRL1000);
            gig_ctrl |= (ADVERTISE_1000FULL | ADVERTISE_1000HALF);

            if (mode == MEDIA_TYPE_1000M_FD || mode == MEDIA_TYPE_1000M_HD) {
                nway_advert &= ~(ADVERTISE_100BASE4 |
                        ADVERTISE_100FULL |
                        ADVERTISE_100HALF |
                        ADVERTISE_10FULL |
                        ADVERTISE_10HALF );
                gig_ctrl &= ~gig_cap; 
            }
            else {
                nway_advert |= (ADVERTISE_100BASE4 |
                        ADVERTISE_100FULL |
                        ADVERTISE_100HALF |
                        ADVERTISE_10FULL |
                        ADVERTISE_10HALF );
            }

            mdio_write(skfd, ifr, phy_id, MII_ADVERTISE, nway_advert);
            mdio_write(skfd, ifr, phy_id, MII_CTRL1000, gig_ctrl);
            mdio_write(skfd, ifr, phy_id, MII_BMCR, val);
            if (mode == MEDIA_TYPE_AUTO)
                sleep(2);

        }
    }
    show_speed_setting(skfd, ifr, phy_id, sub_port);
    return 0;
error:
    return -1;
}

static int et_cmd_phy_reset_op(int skfd, struct ifreq *ifr, cmd_t *cmd, char** argv)
{
    int phy_id = 0, sub_port = -1;

    if (argv[3] && !strcmp(argv[3], "port") && argv[4])
        sub_port = strtol(argv[4], NULL, 0);

    if ((phy_id = et_get_phyid(skfd, ifr, sub_port)) == -1) {
        return -1;
    }

    mdio_write(skfd, ifr, phy_id, MII_BMCR, BMCR_RESET);
    sleep(2);
    show_speed_setting(skfd, ifr, phy_id, sub_port);
    return 0;
}

static int et_cmd_mii_op(int skfd, struct ifreq *ifr, cmd_t *cmd, char** argv)
{
    int phy_id;
    int set = 0;
    int val = 0;
    int reg = -1;
    int sub_port = -1;

    for(argv += 3; *argv; argv++) {
        if (!strcmp(*argv, "port") && *(++argv))
            sub_port = strtol(*argv, NULL, 0);
        else {
            if(reg == -1) {
                reg = strtoul(*argv, NULL, 0);
                if ((reg < 0) || (reg > 31))
                    goto error;
            }
            else {
                val = strtoul(*argv, NULL, 0);
                set = 1;
            }
        }
    }

    if ((phy_id = et_get_phyid(skfd, ifr, sub_port)) == -1)
        goto error;

    if (set)
        mdio_write(skfd, ifr, phy_id, reg, val);
    val = mdio_read(skfd, ifr, phy_id, reg);
    printf("mii (phy addr 0x%x) register %d is 0x%04x\n", phy_id, reg, val);
    return val;
error:
    return -1;
}

static int et_cmd_phy_op(int skfd, struct ifreq *ifr, cmd_t *cmd, char** argv)
{
    int err, phy_id = 0, phy_flag = 0, four_byte;
    unsigned int set = 0, get = 1, val = 0, reg = 0, r, i, dump = 0;

    argv = argv+2;;
    if (*argv) {
        if (strcmp(*argv, "ext") == 0) {
            phy_flag = ETHCTL_FLAG_ACCESS_EXT_PHY;
        } else if (strcmp(*argv, "int") == 0) {
            phy_flag = ETHCTL_FLAG_ACCESS_INT_PHY;
        } else if (strcmp(*argv, "extsw") == 0) { // phy connected to external switch
            phy_flag = ETHCTL_FLAG_ACCESS_EXTSW_PHY;
        } else if (strcmp(*argv, "i2c") == 0) { // phy connected through I2C bus
            phy_flag = ETHCTL_FLAG_ACCESS_I2C_PHY;
        } else if (strcmp(*argv, "serdespower") == 0) { // Serdes power saving mode
            phy_flag = ETHCTL_FLAG_ACCESS_SERDES_POWER_MODE;
        } else if (strcmp(*argv, "ext32") == 0) { // Extended 32bit register access.
            phy_flag = ETHCTL_FLAG_ACCESS_32BIT|ETHCTL_FLAG_ACCESS_EXT_PHY;
        } else {
            goto print_error_and_return;
        }
        argv++;
    } else {
        goto print_error_and_return;
    }

    if (*argv && phy_flag != ETHCTL_FLAG_ACCESS_I2C_PHY) {
        /* parse phy address */
        phy_id = strtol(*argv, NULL, 0);
        if ((phy_id < 0) || (phy_id > 31)) {
            fprintf(stderr, "Invalid Phy Address 0x%02x\n", phy_id);
            return -1;
        }
        argv++;
    } else if (phy_flag != ETHCTL_FLAG_ACCESS_I2C_PHY) {
        goto print_error_and_return;
    }

    if (*argv) {
        reg = strtoul(*argv, NULL, 0);
        
        if(phy_flag == ETHCTL_FLAG_ACCESS_SERDES_POWER_MODE)
        {
            if (reg < 0 || reg > 2)
            {
                fprintf(stderr, "Invalid Serdes Power Mode%02x\n", reg);
                return -1;
            }
            set = 1;
        }
        argv++;
    } else if(phy_flag != ETHCTL_FLAG_ACCESS_SERDES_POWER_MODE) {
        goto print_error_and_return;
    }

    if (*argv) {
        /* parse register setting value */
        val = strtoul(*argv, NULL, 0);
        set = 1;
        argv++;
    }

    if (*argv && phy_flag != ETHCTL_FLAG_ACCESS_SERDES_POWER_MODE) {
        /* parse no read back flag */
        if (strcmp(*argv, "no_read_back") == 0) {
            get = 0;
        } else if (strcmp(*argv, "-d") == 0) {
            dump = 1;
            get = 0;
            set = 0;
        } else {
            fprintf(stderr, "Invalid command %s, expecting no_read_back.\n", *argv);
            return -1;
        }
        argv++;
    }

    ethctl.phy_addr = phy_id;
    ethctl.phy_reg = reg;
    ethctl.flags = phy_flag;

    if (set) {
        ethctl.op = ETHSETMIIREG;
        ethctl.val = val;
        ifr->ifr_data = (void *)&ethctl;
        err = ioctl(skfd, SIOCETHCTLOPS, ifr);
        if (ethctl.ret_val || err) {
            fprintf(stderr, "command return error!\n");
            return err;
        }
        else
            printf("PHY register set successfully\n");
    }

    four_byte = (reg > 0x200000) || (phy_flag & ETHCTL_FLAG_ACCESS_32BIT);
    if (get || dump) {
        for(r = reg, i=0; (dump && r < val) || (get && r == reg); r+=four_byte?4:1, i++)
        {
            ethctl.op = ETHGETMIIREG;
            ethctl.phy_reg = r;
            ifr->ifr_data = (void *)&ethctl;
            err = ioctl(skfd, SIOCETHCTLOPS, ifr);
            if (ethctl.ret_val || err) {
                fprintf(stderr, "command return error!\n");
                return err;
            } 

            if(phy_flag == ETHCTL_FLAG_ACCESS_SERDES_POWER_MODE) {
                static char *mode[] = {"No Power Saving", "Basic Power Saving", "Device Forced Off"};
                printf("Serdes power saving mode: %d-\"%s\"\n\n", 
                        ethctl.val, mode[ethctl.val]);
                break;
            }

            if (get) {
                if (four_byte)
                    printf("mii register 0x%x is 0x%08x\n", reg, ethctl.val);
                else
                    printf("mii register 0x%x is 0x%04x\n", reg, ethctl.val);
            }
            else {  // dump
                if ((i % 8) == 0) {
                    printf("\n");
                    printf("  %04x: ", r);
                }
                else if ((i%4) == 0) {
                    printf("  ");
                }
                if (four_byte)
                    printf(" %08x", ethctl.val);
                else
                    printf("  %04x", ethctl.val);
            }
        }
        printf("\n");
    }

    return err;

print_error_and_return:
    fprintf(stderr, "Invalid syntax\n");
    return -1;
}


#ifdef unused_code
static int mdio_read_shadow(int skfd, struct ifreq *ifr, int phy_id, 
        int shadow_reg) 
{
    int reg = 0x1C;
    int val = (shadow_reg & 0x1F) << 10;
    mdio_write(skfd, ifr, phy_id, reg, val);
    return mdio_read(skfd, ifr, phy_id, reg);
}

static void mdio_write_shadow(int skfd, struct ifreq *ifr, int phy_id, 
        int shadow_reg, int val)
{
    int reg = 0x1C;
    int value = ((shadow_reg & 0x1F) << 10) | (val & 0x3FF) | 0x8000;
    mdio_write(skfd, ifr, phy_id, reg, value);
}
#endif

static int et_cmd_phy_power_op(int skfd, struct ifreq *ifr, cmd_t *cmd, char** argv)
{
    int err = -1;

    if (!argv[3]) {
        return 1;
    }

    if (strcmp(argv[3], "up") == 0) {
#ifdef RTCONFIG_HND_ROUTER_AX
	ethctl.op = ETHSETPHYPWRON;
#else
        ethctl.op = ETHSETSPOWERUP;
#endif
    } else if (strcmp(argv[3], "down") == 0) {
#ifdef RTCONFIG_HND_ROUTER_AX
	ethctl.op = ETHSETPHYPWROFF;
#else
        ethctl.op = ETHSETSPOWERDOWN;
#endif
    } else {
        return 1;
    }

    ifr->ifr_data = (void *)&ethctl;
    err = ioctl(skfd, SIOCETHCTLOPS, ifr);

    if (err) {
        fprintf(stderr, "command return error!\n");
        return err;
    } else {
        printf("Powered %s \n", argv[3]);
    }

    return err;
}

static int et_cmd_vport_enable(int skfd, struct ifreq *ifr)
{
    int err = 0;

    err = ioctl(skfd, SIOCGENABLEVLAN, ifr);

    return err;
}

static int et_cmd_vport_disable(int skfd, struct ifreq *ifr)
{
    int err = 0;

    err = ioctl(skfd, SIOCGDISABLEVLAN, ifr);

    return err;
}

static int et_cmd_vport_query(int skfd, struct ifreq *ifr)
{
    int err = 0;
    int ports = 0;

    ifr->ifr_data = (char*)&ports;
    err = ioctl(skfd, SIOCGQUERYNUMVLANPORTS, ifr);
    if (err == 0)
        printf("%u\n", ports);

    return err;
}

static int et_cmd_vport_op(int skfd, struct ifreq *ifr, cmd_t *cmd, char** argv)
{
    int err = -1;
    char *arg;

    arg = argv[3];
    if (strcmp(arg, "enable") == 0) {
        err = et_cmd_vport_enable(skfd, ifr);
    } else if (strcmp(arg, "disable") == 0) {
        err = et_cmd_vport_disable(skfd, ifr);
    } else if (strcmp(arg, "query") == 0) {
        err = et_cmd_vport_query(skfd, ifr);
    } else {
        return 1;
    }
    if (err)
        fprintf(stderr, "command return error!\n");

    return err;
}

#define MAX_NUM_CHANNELS 4
/* Set/Get number of Tx IUDMA channels */
static int et_cmd_tx_iudma_op(int skfd, struct ifreq *ifr, cmd_t *cmd, 
                              char** argv)
{
    int err = -1;
 
    if (argv[2]) {
        ethctl.num_channels = (int) strtol(argv[2], NULL, 0);
        if ((ethctl.num_channels < 1) || 
            (ethctl.num_channels > MAX_NUM_CHANNELS)) {
            fprintf(stderr, "Invalid number of Tx IUDMA Channels \n");
        }
        ethctl.op = ETHSETNUMTXDMACHANNELS;
    } else {
        ethctl.op = ETHGETNUMTXDMACHANNELS;
    }

    ifr->ifr_data = (void *)&ethctl;
    err = ioctl(skfd, SIOCETHCTLOPS, ifr);

    if (err) {
        fprintf(stderr, "command return error!\n");
        return err;
    } else if (ethctl.op == ETHGETNUMTXDMACHANNELS) {
        printf("The number of Tx DMA channels: %d\n", 
                ethctl.ret_val);
    }

    return err;
}

/* Set/Get number of Rx IUDMA channels */
static int et_cmd_rx_iudma_op(int skfd, struct ifreq *ifr, cmd_t *cmd, 
                              char** argv)
{
    int err = -1;

    if (argv[2]) {
        ethctl.num_channels = (int) strtol(argv[2], NULL, 0);
        if ((ethctl.num_channels < 1) || 
            (ethctl.num_channels > MAX_NUM_CHANNELS)) {
            fprintf(stderr, "Invalid number of Rx IUDMA Channels \n");
        }
        ethctl.op = ETHSETNUMRXDMACHANNELS;
    } else {
        ethctl.op = ETHGETNUMRXDMACHANNELS;
    }

    ifr->ifr_data = (void *)&ethctl;
    err = ioctl(skfd, SIOCETHCTLOPS, ifr);

    if (err) {
        fprintf(stderr, "command return error!\n");
        return err;
    } else if (ethctl.op == ETHGETNUMRXDMACHANNELS) {
        printf("The number of Rx DMA channels: %d\n", 
                ethctl.ret_val);
    }

    return err;
}

/* Display software stats */
static int et_cmd_stats_op(int skfd, struct ifreq *ifr, cmd_t *cmd, 
                              char** argv)
{
    int err = -1;

    ethctl.op = ETHGETSOFTWARESTATS;

    ifr->ifr_data = (void *)&ethctl;
    err = ioctl(skfd, SIOCETHCTLOPS, ifr);

    if (err) {
        fprintf(stderr, "command return error!\n");
        return err;
    }

    return err;
}

/* Enable/Disable ethernet@wirespeed */
static int et_cmd_ethernet_at_wirespeed_op(int skfd, struct ifreq *ifr, cmd_t *cmd, 
                              char** argv)
{
    int phy_id = 0;
    int gig_ctrl, v16, ctrl, sub_port = -1;    

    if (argv[4] && !strcmp(argv[4], "port") && argv[4])
        sub_port = strtol(argv[5], NULL, 0);

    if ((phy_id = et_get_phyid(skfd, ifr, sub_port)) == -1) {        
        return 1;
    }

    if(ETHCTL_GET_FLAG_FROM_PHYID(phy_id) & ETHCTL_FLAG_ACCESS_SERDES) {
        fprintf(stderr, "ethernet@wirespeed is not supported on SERDES interface\n");
        return 1;
    }

    gig_ctrl = mdio_read(skfd, ifr, phy_id, MII_CTRL1000);
       
    // check ethernet@wirspeed only for PHY support 1G 
    if(!(gig_ctrl & ADVERTISE_1000FULL || gig_ctrl & ADVERTISE_1000HALF)) {
        fprintf(stderr, "ethernet@wirespeed is not supported on 10/100Mbps.\n");
        return 1;
    }

    // read current setting
    mdio_write(skfd, ifr, phy_id, 0x18, 0x7007);
    v16 = mdio_read(skfd, ifr, phy_id, 0x18);

    if (strcmp(argv[3], "enable") == 0) {        
        v16 = v16 | 0x8010; // set bit15 for write, bit4 for ethernet@wirespeed
        mdio_write(skfd, ifr, phy_id, 0x18, v16);

        // Restart AN
        ctrl = mdio_read(skfd, ifr, phy_id, MII_BMCR);
        ctrl = BMCR_ANENABLE | BMCR_ANRESTART;
        mdio_write(skfd, ifr, phy_id, MII_BMCR, ctrl);
    } else if (strcmp(argv[3], "disable") == 0) {
        v16 = (v16 & 0xffef) | 0x8000; // set bit15 for write, clear bit4 for ethernet@wirespeed
        mdio_write(skfd, ifr, phy_id, 0x18, v16);       
    } 
    if (v16 & 0x0010) 
        fprintf(stderr, "ethernet@wirespeed is enabled\n");
    else
        fprintf(stderr, "ethernet@wirespeed is disabled\n");
    return 1;
}

static const struct command commands[] = {
    { 0, "media-type", et_cmd_media_type_op, 
      ": Set/Get media type\n"
      "  ethctl <interface> media-type [option] [port <sub_port#> ]\n"
      "    [option]: auto - auto select\n"
      "              1000FD - 1000Mb, Full Duplex\n"
      "              1000HD - 1000Mb, Half Duplex\n"      
      "              100FD - 100Mb, Full Duplex\n"
      "              100HD - 100Mb, Half Duplex\n"
      "              10FD  - 10Mb,  Full Duplex\n"
      "              10HD  - 10Mb,  Half Duplex\n"
      "    [port <sub_port#>]: required if <interface> has Crossbar or Trunk port underneath\n"
    },
    { 0, "phy-reset", et_cmd_phy_reset_op,
      ": Soft reset the transceiver\n"
      "  ethctl <interface> phy-reset [port <sub_port#>]\n"
      "    [port <sub_port#>]: required if <interface> has Crossbar or Trunk port underneath\n"
    },
    { 1, "reg", et_cmd_mii_op,
      ": Set/Get port mii register\n"
      "  ethctl <interface> reg <[0-31]> [0xhhhh] [port <sub_port#>]\n"
      "    [port <sub_port#>]: required if <interface> has Crossbar or Trunk port underneath\n"
    },
    { 1, "phy-power", et_cmd_phy_power_op,
      ": Phy power up/down control\n"
      "  ethctl <interface> phy-power <up|down>"
    },
    { 1, "vport", et_cmd_vport_op,
      ": Enable/disable/query Switch for VLAN port mapping\n"
      "  ethctl <interface> vport <enable|disable|query>"
    },
    { 0, "stats", et_cmd_stats_op,
      ": Display software stats\n"
      "  ethctl <interface> stats"
    },
    { 1, "ethernet@wirespeed", et_cmd_ethernet_at_wirespeed_op,
      ": Enable/Disable ethernet@wirespeed\n"
      "  ethctl <interface> ethernet@wirespeed <show|enable|disable> [port <sub_port#>]\n"
      "    [port <sub_port#>]: required if <interface> has Crossbar or Trunk port underneath\n"
    },
};

cmd_t *command_lookup(const char *cmd)
{
    int i;

    for (i = 0; i < sizeof(commands)/sizeof(commands[0]); i++) {
        if (!strcmp(cmd, commands[i].name))
            return (cmd_t *)&commands[i];
    }

    return NULL;
}

/* These commands don't require interface to be specified */
static const struct command common_commands[] = {
    { 0, "tx_iudma", et_cmd_tx_iudma_op,
      ": Set/Get number of Tx iuDMA channels\n"
      "  ethctl tx_iudma <[1-4]>"
    },
    { 0, "rx_iudma", et_cmd_rx_iudma_op, 
      ": Set/Get number of Rx iuDMA channels\n" 
      "  ethctl rx_iudma <[1-4]>\n"
    },
    { 3, "phy", et_cmd_phy_op, 
      ": Phy Access \n" 
      "  ethctl phy int|ext|extsw|ext32 <phy_addr> <reg> [<value|reg2> [no_read_back]] [-d]\n"
      "  ethctl phy i2c <reg> [<value|reg2> [no_read_back]] [-d] \n"
      "      <reg>: 0-0x1f: CL22 IEEE register; 0x1f-0xffff: Broadcom Extended Registers.\n"
      "             0xffff-0x1fffff: CL45 IEEE Register, DeviceAddress + 2 byte Registers.\n"
      "             0x20ffff-0xffffffff: Broadcom PHY 32bit address.\n"
      "      <ext32>: Force to access Broadcom phy 32bit address.\n" 
      "  ethctl phy serdespower <phy_addr> [<power_mode>]\n"
      "      [<power_mode>]: 0 - Non power saving mode; for loop back, inter connection\n"
      "                      1 - Basic power saving mode; default mode\n"
      "                      2 - Device Forced Off\n"
      "  -d: Dump registers started from <reg> to <reg2>.\n"
    },
};

