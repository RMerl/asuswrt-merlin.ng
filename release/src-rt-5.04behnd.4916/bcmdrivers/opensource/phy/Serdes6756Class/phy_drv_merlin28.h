/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2017:DUAL/GPL:standard

   Unless you and Broadcom execute a separate written software license
   agreement governing use of this software, this software is licensed
   to you under the terms of the GNU General Public License version 2
   (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
   with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

   Not withstanding the above, under no circumstances may you combine
   this software in any way with any other Broadcom software provided
   under a license other than the GPL, without Broadcom's express prior
   written consent.

   :>
 */

/*
 *  Created on: Sep 2017
 *      Author: li.xu@broadcom.com
 */

#ifndef MERLIN_H
#define MERLIN_H

#define merlin28_PORTING_MACRO
#ifdef merlin28_PORTING_MACRO
#define srds_access_s phy_serdes_s
#include "linux/types.h"
#include "phy_drv.h"
#include "phy_drv_dsl_serdes.h"
#include "phy_drv_ethtop_merlin28.h"
#include "serdes_access.h"

#define PMD_DEV 1

typedef struct {char *reg_desc; int dev_addr; int reg_addr; int data_bitEn; int data;} prog_seq_tbl;
static inline void timeout_ns(uint32_t ns)
{
    if (ns < 2000)
        ndelay(ns);
    else if (ns < 2000000)
        udelay(ns/1000);
    else if ((ns/1000) < 2000000)
        mdelay(ns/1000/1000);
}


void serdes_access_read_raw(uint32_t reg, uint32_t *val);
void serdes_access_write_raw(uint32_t reg, uint32_t val);

int merlin28_enable_an(phy_dev_t *phy_dev);
int merlin28_enable_multi_speed_an(phy_dev_t *phy_dev);
int merlin28_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
int merlin28_pmi_write16_delay(uint32_t CoreNum, uint32_t LANE, uint32_t DEV_ADDR, uint32_t REG_ADDR, uint16_t DATA, uint16_t MASK, bool delay_acked);
uint32_t merlin28_pmi_read16_delay(uint32 CoreNum, uint32 LANE, uint32 DEV_ADDR, uint32 REG_ADDR, bool DELAY_ACKED);
int merlin28_serdes_init(phy_dev_t *phy_dev);
int merlin28_lane_power_op(phy_dev_t *phy_dev, int power_level);
static inline void host_reg_write(uint32_t eth_phy_top_offset, uint32_t data)
{
    serdes_access_write_raw(eth_phy_top_offset, data);
}
static inline uint32_t host_reg_read(uint32_t eth_phy_top_offset)
{
    uint32_t data;
    serdes_access_read_raw(eth_phy_top_offset, &data);
    return data;
}
static inline int merlin28_pmi_write16(uint32_t CoreNum, uint32_t LANE, uint32_t DEV_ADDR, uint32_t REG_ADDR, uint16_t DATA, uint16_t MASK)
{
    return merlin28_pmi_write16_delay(CoreNum, LANE, DEV_ADDR, REG_ADDR, DATA, MASK, 0);
}

static inline uint32 merlin28_pmi_read16(uint32 CoreNum, uint32 LANE, uint32 DEV_ADDR, uint32 REG_ADDR)
{
    return merlin28_pmi_read16_delay(CoreNum, LANE, DEV_ADDR, REG_ADDR, 0);
}

phy_dev_t *phy_drv_serdes6756_get_phy_dev(int core_num, int lane_num);
static inline int phy_drv_serdes6756_get_mdio_addr(int core_num, int lane_num)
{
    return phy_drv_serdes6756_get_phy_dev(core_num, lane_num)->addr;
}
static inline phy_serdes_t *phy_drv_serdes6756_get_serdes(int core_num, int lane_num)
{
    return (phy_serdes_t *)(phy_drv_serdes6756_get_phy_dev(core_num, lane_num)->priv);
}
#define HOST_MDIO_PHY_ADDR(core_num, lane_num) phy_drv_serdes6756_get_mdio_addr(core_num, lane_num)

void merlin28_chk_lane_link_status(phy_dev_t *phy_dev);

#endif // merlin28_PORTING_MACRO

#define HOST_BASE                 0x80400000

#define PMI_BC_ADDRESS            0x1f

#ifdef CHIPTOP
  #ifdef MERLIN_DUAL_DUT
    #define NUMBER_MERLIN_CORES      0x2   //only two Merlin cores in block level bench
  #else
    #define NUMBER_MERLIN_CORES      0x1   //only one Merlin cores in block level bench
  #endif
#else
  #ifdef MERLIN_DUAL_DUT
    #define NUMBER_MERLIN_CORES      0x2   //only two Merlin cores in block level bench
  #else
    #define NUMBER_MERLIN_CORES      0x1   //only one Merlin cores in block level bench
  #endif
#endif

#define NUMBER_MERLIN_LANES      0x2 // MERLIN 28 PHY 2;

/* ln_offset_strap = 0x4 per Figure 16 Multiple Quad Instances doesn't work
   becasue MERLIN_PHY_0_inst.phy_top.pcs_top.ln_offset_strap is tied off to 5'h0
*/
#define MERLINE_LANE_OFFSET       0x0

/*
 speed encoding
*/
//CL36      [force GMII to XGMII]x1 digit +[AN]x1 digit +[ENUM]x2 digits
#define MLN_SPD_FORCE_1G                 0x0002
#define MLN_SPD_FORCE_2P5G               0x0003

//          [force GMII to XGMII]x1 digit +[AN IEEE: 1; USER: 3]x1 digit +[ENUM]x2 digits
#define MLN_SPD_AN_1G_KX_IEEE_CL73       0x010d
#define MLN_SPD_AN_1G_USER_CL73          0x030d
#define MLN_SPD_AN_1G_IEEE_CL37          0x0102
#define MLN_SPD_AN_1G_USER_CL37          0x0302

//CL49 10GBASE-R       [CL74 FEC]x1 digit +[AN]x1 digit +[ENUM]x2 digits
#define MLN_SPD_FORCE_10G_R              0x000f
#define MLN_SPD_FORCE_10G_R_CL74         0x100f

//         [CL74 FEC + IEEE/USER]x1 digit +[AN]x1 digit +[ENUM]x2 digits
#define MLN_SPD_AN_10G_KR_IEEE_CL73      0x010f
#define MLN_SPD_AN_10G_KR_IEEE_CL73_CL74 0x210f
#define MLN_SPD_AN_10G_USER_CL73         0x110f
#define MLN_SPD_AN_10G_USER_CL73_CL74    0x310f

//CL129 5GBASE-R
#define MLN_SPD_FORCE_5G_R               0x0029
#define MLN_SPD_FORCE_2P5G_R             0x0028

//                   [IEEE/USER]x1 digit +[AN]x1 digit +[ENUM]x2 digits
#define MLN_SPD_AN_5G_KR_IEEE_CL73      0x0129
#define MLN_SPD_AN_5G_USER_CL73         0x1129

//SGMII      [MASTER=3; SLAVE=2]x1 digit +[AN]x1 digit +[ENUM]x2 digits
#define MLN_SPD_FORCE_10M                0x0000
#define MLN_SPD_FORCE_100M               0x0001
//#define MLN_SPD_FORCE_1G                 0x0002  //already defined in the CL36
#define MLN_SPD_AN_1G_SGMII              0x3102       
#define MLN_SPD_AN_100M_SGMII            0x3101
#define MLN_SPD_AN_10M_SGMII             0x3100
#define MLN_SPD_AN_SGMII_SLAVE           0x2108  //

//CL73
//                   [all speed]x1 digit +[AN IEEE: 1; USER: 3]x1 digit +[ENUM]x2 digits
#define MLN_SPD_AN_IEEE_CL73             0x1100
#define MLN_SPD_AN_USER_CL73             0x1300

#endif //MERLIN_H

