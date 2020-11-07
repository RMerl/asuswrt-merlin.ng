/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

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

 * pmc_lport.h
 *
 *  Created on  Nov 21 2015
 *      Author: yonatani
 */

#ifndef _PMC_LPORT_H_
#define _PMC_LPORT_H_


#include <bcmtypes.h>


//LPORT BPCM Specific registers
#define LPORT_BPCM_Z0_CONTROL         0x20  // offset in bytes: 0x80
#define LPORT_BPCM_Z0_QEGPHY_CTRL     0x21  // offset in bytes: 0x84
#define LPORT_BPCM_Z0_QEGPHY_STATUS   0x22  // offset in bytes: 0x88
#define LPORT_BPCM_Z0_DSERDES0_CTRL   0x23  // offset in bytes: 0x8c
#define LPORT_BPCM_Z0_DSERDES0_STATUS 0x24  // offset in bytes: 0x90
#define LPORT_BPCM_Z0_DSERDES1_CTRL   0x25  // offset in bytes: 0x94
#define LPORT_BPCM_Z0_DSERDES1_STATUS 0x26  // offset in bytes: 0x98
#define LPORT_BPCM_Z1_CONTROL         0x27  // offset in bytes: 0x9c
#define LPORT_BPCM_Z2_CONTROL         0x28  // offset in bytes: 0xa0

typedef struct
{
#ifdef PMC_LITTLE_ENDIAN
    uint32 z0_sw_init_1:1;                   // 0
    uint32 z0_mux_sel:1;                    // 1
    uint32 z0_ubus_dev_clk_en:1;            // 2
    uint32 z0_qegphy_clk_en:1;              // 3
    uint32 z0_pmx_sel:1;                    // 4
    uint32 z0_gport_sel:8;                 // 5:12
    uint32 z0_control_UNUSED:19;             // 31:13
#else
    uint32 z0_control_UNUSED:19;             // 31:13
    uint32 z0_gport_sel:8;                 // 5-12
    uint32 z0_pmx_sel:1;                    // 4
    uint32 z0_qegphy_clk_en:1;              // 3
    uint32 z0_ubus_dev_clk_en:1;            // 2
    uint32 z0_mux_sel:1;                    // 1
    uint32 z0_sw_init_1:1;                   // 0
#endif
}LPORT_BPCM_Z0_CONTROL_REG;


typedef struct
{
#ifdef PMC_LITTLE_ENDIAN
    uint32 z1_sw_init_1:1;                   // 0
    uint32 z1_clk_250_clk_en:1;             // 1
    uint32 z1_tsc_clk_en:1;                 // 2
    uint32 z1_tsc_clk_gated_clk_en:1;       // 3
    uint32 z1_cclk_clk_en:1;                // 4
    uint32 z1_data_path_cclk_clk_en:1;      // 5
    uint32 z1_tsclk_clk_en:1;               // 6
    uint32 z1_control_UNUSED1:25;             // 31:7
#else
    uint32 z1_control_UNUSED1:25;             // 31:7
    uint32 z1_tsclk_clk_en:1;               // 6
    uint32 z1_data_path_cclk_clk_en:1;      // 5
    uint32 z1_cclk_clk_en:1;                // 4
    uint32 z1_tsc_clk_gated_clk_en:1;       // 3
    uint32 z1_tsc_clk_en:1;                 // 2
    uint32 z1_clk_250_clk_en:1;             // 1
    uint32 z1_sw_init_1:1;                   // 0
#endif
}LPORT_BPCM_Z1_CONTROL_REG;

typedef struct
{
#ifdef PMC_LITTLE_ENDIAN
    uint32 z2_sw_init_1:1;                   // 0
    uint32 z2_clk_250_clk_en:1;             // 1
    uint32 z2_tsc_clk_en:1;                 // 2
    uint32 z2_tsc_clk_gated_clk_en:1;       // 3
    uint32 z2_cclk_clk_en:1;                // 4
    uint32 z2_data_path_cclk_clk_en:1;      // 5
    uint32 z2_tsclk_clk_en:1;               // 6
    uint32 z2_control_UNUSED:25;             // 31:7
#else
    uint32 z2_control_UNUSED:25;             // 31:7
    uint32 z2_tsclk_clk_en:1;               // 6
    uint32 z2_data_path_cclk_clk_en:1;      // 5
    uint32 z2_cclk_clk_en:1;                // 4
    uint32 z2_tsc_clk_gated_clk_en:1;       // 3
    uint32 z2_tsc_clk_en:1;                 // 2
    uint32 z2_clk_250_clk_en:1;             // 1
    uint32 z2_sw_init_1:1;                   // 0
#endif
}LPORT_BPCM_Z2_CONTROL_REG;

int pmc_lport_init(void);
int pmc_lport_shutdown(void);
#endif /* _PMC_LPORT_H_ */
