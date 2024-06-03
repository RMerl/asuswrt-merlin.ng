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

#define MERLIN16_PORTING_MACRO
#ifdef MERLIN16_PORTING_MACRO
#define srds_access_s phy_dev_s
#include "linux/types.h"
#include "phy_drv.h"
#include "phy_drv_dsl_serdes.h"
#include "common/srds_api_types.h"
#include "common/srds_api_err_code.h"
#include "merlin16_shortfin_internal.h"
#include "merlin16_shortfin_functions.h"
#include "merlin16_shortfin_config.h"

#define PMD_DEV 1
#define UCODE_MAX_SIZE (84*1024)

typedef struct {char *reg_desc; int dev_addr; int reg_addr; int data_bitEn; int data;} prog_seq_tbl;
typedef struct {char *reg_desc; int dev_addr; int non_reserved_mask; int writeable_mask; int reg_addr; int def_val;} prog_seq_tbl_read;

static inline void timeout_ns(uint32_t ns)
{
    if (ns < 2000)
        ndelay(ns);
    else if (ns < 2000000)
        udelay(ns/1000);
    else if ((ns/1000) < 2000000)
        mdelay(ns/1000/1000);
}

int merlin_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
int merlin_pmi_write16_delay(uint32_t CoreNum, uint32_t LANE, uint32_t DEV_ADDR, uint32_t REG_ADDR, uint16_t DATA, uint16_t MASK, bool delay_acked);
uint16_t merlin_pmi_read16_delay(uint32 CoreNum, uint32 LANE, uint32 DEV_ADDR, uint32 REG_ADDR, bool DELAY_ACKED);
int merlin16_serdes_init(phy_dev_t *phy_dev);
int merlin_core_power_op(phy_dev_t *phy_dev, int power_level);

static inline int merlin_pmi_write16(uint32_t CoreNum, uint32_t LANE, uint32_t DEV_ADDR, uint32_t REG_ADDR, uint16_t DATA, uint16_t MASK)
{
    return merlin_pmi_write16_delay(CoreNum, LANE, DEV_ADDR, REG_ADDR, DATA, MASK, 0);
}

static inline uint16_t merlin_pmi_read16(uint32 CoreNum, uint32 LANE, uint32 DEV_ADDR, uint32 REG_ADDR)
{
    return merlin_pmi_read16_delay(CoreNum, LANE, DEV_ADDR, REG_ADDR, 0);
}

phy_dev_t *phy_drv_serdes146_get_phy_dev(int core_num, int port_num);

static inline phy_serdes_t *phy_drv_serdes146_get_serdes(int core_num, int port_num)
{
    phy_dev_t *phy_dev = phy_drv_serdes146_get_phy_dev(core_num, port_num);
    if (phy_dev == NULL)
        return NULL;
    return (phy_serdes_t *)(phy_dev->priv);
}

void merlin_chk_lane_link_status(phy_dev_t *phy_dev);
int merlin_ext_signal_detected(phy_dev_t *phy_dev);
int merlin16_read_txfir_reg(phy_dev_t *phy_dev, phy_txfir_t *txfir);
int merlin16_write_txfir_reg(phy_dev_t *phy_dev, phy_txfir_t *txfir);
int merlin16_shortfin_set_shared_clock(phy_dev_t *phy_dev);

#endif // MERLIN16_PORTING_MACRO

int phy_drv_serdes146_get_total_cores(void);
#define NUMBER_MERLIN_CORES phy_drv_serdes146_get_total_cores()

/* ln_offset_strap = 0x4 per Figure 16 Multiple Quad Instances doesn't work
   becasue MERLIN_PHY_0_inst.phy_top.pcs_top.ln_offset_strap is tied off to 5'h0
*/

/*
 speed encoding
*/
//CL36      [force GMII to XGMII]x1 digit +[AN]x1 digit +[ENUM]x2 digits
#define MLN_SPD_FORCE_1G                 0x0002     // 1000Base-X
#define MLN_SPD_FORCE_1G_KX1             0x000d     // 1000Base-KX
#define MLN_SPD_FORCE_2P5G               0x0003     // 2500Base-X
#define MLN_SPD_FORCE_5G                 0x0010     // 5000Base-X

#define MLN_SPD_FORCE_1G_XGMII           0x1002
#define MLN_SPD_FORCE_1G_KX1_XGMII       0x100d
#define MLN_SPD_FORCE_2P5G_XGMII         0x1003
#define MLN_SPD_FORCE_5G_XGMII           0x1010

//          [force GMII to XGMII]x1 digit +[AN IEEE: 1; USER: 3]x1 digit +[ENUM]x2 digits
#define MLN_SPD_AN_ALL_SPEEDS_IEEE_CL73  0x9000

#define MLN_SPD_AN_1G_KX_IEEE_CL73       0x010d
#define MLN_SPD_AN_1G_USER_CL73          0x030d
#define MLN_SPD_AN_1G_IEEE_CL37          0x0102
#define MLN_SPD_AN_1G_USER_CL37          0x0302

#define MLN_SPD_AN_1G_KX_IEEE_CL73_XGMII 0x110d
#define MLN_SPD_AN_1G_USER_CL73_XGMII    0x130d
#define MLN_SPD_AN_1G_IEEE_CL37_XGMII    0x1102
#define MLN_SPD_AN_1G_USER_CL37_XGMII    0x1302

//CL49 10GBASE-R       [CL74 FEC]x1 digit +[AN]x1 digit +[ENUM]x2 digits
#define MLN_SPD_FORCE_10G_R              0x000f
#define MLN_SPD_FORCE_10G_R_CL74         0x100f

//         [CL74 FEC + IEEE/USER]x1 digit +[AN]x1 digit +[ENUM]x2 digits
#define MLN_SPD_AN_10G_KR_IEEE_CL73      0x010f
#define MLN_SPD_AN_10G_KR_IEEE_CL73_CL74 0x210f
#define MLN_SPD_AN_10G_USER_CL73         0x110f
#define MLN_SPD_AN_10G_USER_CL73_CL74    0x310f

//CL127 2.5GBASE-X
#define MLN_SPD_FORCE_5G_X               0x0020     // 5GBase-X
#define MLN_SPD_FORCE_2P5G_X             0x0021     // 2.5GBase-X
#define MLN_SPD_FORCE_1G_X               0x0014     // 1GBase-X

//          [force GMII to XGMII]x1 digit +[AN IEEE: 1; USER: 3]x1 digit +[ENUM]x2 digits
#define MLN_SPD_AN_2P5G_KX_IEEE_CL73     0x0121
#define MLN_SPD_AN_2P5G_USER_CL73        0x0321

//CL129 5GBASE-R
#define MLN_SPD_FORCE_5G_R               0x0029
#define MLN_SPD_FORCE_2P5G_R             0x0028
#define MLN_SPD_FORCE_1G_R               0x0027

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
#define MLN_SPD_AN_SGMII_SLAVE_XGMII     0x4108  //+force XGMII for 1Gbps speed

//USXGMII-S [CL74 FEC]x1 digit +[AN]x1 digit +[ENUM]x2 digits
#define MLN_SPD_FORCE_10G_USXGMII        0x0036
#define MLN_SPD_FORCE_10G_CL74_USXGMII   0x1036
#define MLN_SPD_FORCE_5G_USXGMII         0x0035
#define MLN_SPD_FORCE_2P5G_USXGMII       0x0030
#define MLN_SPD_FORCE_1G_USXGMII         0x002f
#define MLN_SPD_FORCE_100M_USXGMII       0x002e

#define MLN_SPD_AN_USXGMII_MASTER        0x0109  //
#define MLN_SPD_AN_USXGMII_SLAVE         0x0108  //
#define MLN_SPD_AN_USXGMII_MP_SLAVE      0x1108  //

#endif //MERLIN_H

