/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom
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
#ifndef BPCM_6765_H
#define BPCM_6765_H

#define BPCM_ZONE_HAS_STATUS
// types of PMB devices
enum {
	kPMB_NODEV = 0,
	kPMB_BPCM = 1,
	kPMB_CLKGEN = 2,
	kPMB_PVTMON = 3,
	kPMB_TMON = 4,
	kPMB_ARS = 5,
	kPMB_BMC = 6,
	kPMB_SECCFG = 7,
	kPMB_REG = 0xf,
};

typedef union {
	struct {
		uint32_t pmbAddr:12;            // [11:00]
		uint32_t map_rev:4;             // [15:12] always non-zero for AVS3 devices 
		uint32_t island:4;              // [19:16]
		uint32_t devType:4;             // [23:20] see enum above
		uint32_t hw_rev:8;              // [31:24]
	} Bits;
	uint32_t Reg32;
} BPCM_ID_REG;

typedef union {
	struct {
		uint32_t pmbAddr:8;             // [07:00]
		uint32_t hw_rev:8;              // [15:08]
		uint32_t strap:16;              // [31:16]
	} Bits;
	uint32_t Reg32;
} CLKRST_MODULE_ID_REG;

typedef union {
	struct {
		uint32_t num_zones:7;           // [06:00]
		uint32_t reserved1:1;           // [07:07]
		uint32_t num_sr_bits:8;         // [15:08]
		uint32_t sw_strap:10;           // [25:16]
		uint32_t reserved2:5;           // [30:26]
		uint32_t sync_pm_clcks:1;       // [31:31]
	} Bits;
	uint32_t Reg32;
} BPCM_CAPABILITES_REG;

typedef union {
	struct {
		uint32_t reserved:16;           // [15:00]
		uint32_t devType:4;             // [19:16]
		uint32_t reserved1:12;          // [31:20]
	} Bits;
	uint32_t Reg32;
} CLKRST_CAPABILITES_REG;

typedef union {
	struct {
		uint32_t link_addr:20;          // [19:00]
		uint32_t link_bus:3;            // [22:20]
		uint32_t link_valid:1;          // [23:23]
		uint32_t reserved1:8;           // [31:24]
	} Bits;
	uint32_t Reg32;
} BPCM_LINK_ADDR_REG;

typedef union {
	struct {
		uint32_t sr:8;                  // [09:00]
		uint32_t gp:24;                 // [31:10]
	} Bits;
	uint32_t Reg32;
} BPCM_SR_CONTROL;

typedef union {
	struct {
		uint32_t egphy_xport:4;         // [03:00]
		uint32_t reserved:4;            // [07:04]
		uint32_t orion:4;               // [11:08]
		uint32_t usb:4;                 // [15:12]
		uint32_t memc:4;                // [19:16]
		uint32_t pcie:4;                // [23:20]
		uint32_t periph:4;              // [27:24]
		uint32_t crypto:4;              // [31:28]
	} Bits;
	uint32_t Reg32;
} CLKRST_OBSERVE_CNTL;

typedef union {
	struct {
		uint32_t egphy0:1;              // [00:00]
		uint32_t egphy1:1;              // [01:01]
		uint32_t egphy2:1;              // [02:02]
		uint32_t egphy4:1;              // [03:03]
		uint32_t orion:1;               // [04:04]
		uint32_t usb:1;                 // [05:05]
		uint32_t memc:1;                // [06:06]
		uint32_t pcie:1;                // [07:07]
		uint32_t periph1:1;             // [08:08]
		uint32_t periph2:1;             // [09:09]
		uint32_t crypto:1;              // [10:10]
		uint32_t reserved:21;           // [31:11]
	} Bits;
	uint32_t Reg32;
} CLKRST_OBSERVE_DIV_EN;

typedef union {
	struct {
		uint32_t cpu_reset_n:8;         // [07:00]
		uint32_t c0l2_reset:1;          // [08:08]
		uint32_t c1l2_reset:1;          // [09:09]
		uint32_t reserved0:6;           // [15:10]
		uint32_t cpu_bpcm_init_on:8;    // [23:16]
		uint32_t c0l2_bpcm_init_on:1;   // [24:24]
		uint32_t c1l2_bpcm_init_on:1;   // [25:25]
		uint32_t ubus_sr:1;             // [26:26]
		uint32_t cci_sr:1;              // [27:27]
		uint32_t webcores_sr:1;         // [28:28]
		uint32_t hw_done:1;             // [29:29]
		uint32_t sw_done:1;             // [30:30]
		uint32_t start:1;               // [31:31]
	} Bits;
	uint32_t Reg32;
} ARM_CONTROL_REG;

typedef union {
	struct {
		uint32_t trace_div_ratio:4;     // [03:00]
		uint32_t reserved1:3;           // [06:04]
		uint32_t trace_enclk:1;         // [07:07]
		uint32_t atb_div_ratio:4;       // [11:08]
		uint32_t reserved2:3;           // [14:04]
		uint32_t atb_enclk:1;           // [15:15]
		uint32_t apb_div_ratio:4;       // [19:16]
		uint32_t reserved3:3;           // [22:20]
		uint32_t apb_enclk:1;           // [23:23]
		uint32_t axi_div_ratio:4;       // [27:24]
		uint32_t reserved4:3;           // [30:28]
		uint32_t axi_enclk:1;           // [31:31]
	} Bits;
	uint32_t Reg32;
} BIU_CLK_CONTROL0_REG;

typedef union {
	struct {
		uint32_t evento:1;              // [00:00]
		uint32_t eventi:1;              // [01:01]
		uint32_t reserved:30;           // [31:02]
	} Bits;
	uint32_t Reg32;
} BIU_EVENT_REG;

typedef union {
	struct {
		uint32_t C0_iso_out:1;          // [00:00]
		uint32_t C1_iso_out:1;          // [01:01]
		uint32_t C0_iso_in:1;           // [02:02]
		uint32_t C1_iso_in:1;           // [03:03]
		uint32_t reserved:28;           // [31:04]
	} Bits;
	uint32_t Reg32;
} ARM_PWR_CTRL_REG;

typedef union {
	struct {
		uint32_t ctrl:5;                // [04:00]
		uint32_t reserved:26;           // [30:05]
		uint32_t rstn:1;                // [31:31]
	} Bits;
	uint32_t Reg32;
} BIU_CX_CLK_CTRL_REG;

typedef union {
	struct {
		uint32_t resetb:1;              // [00:00]
		uint32_t post_resetb:1;         // [01:01]
		uint32_t pwrdwn:1;              // [02:02]
		uint32_t master_reset:1;        // [03:03]
		uint32_t pwrdwn_ldo:1;          // [04:04]
		uint32_t iso:1;                 // [05:05]
		uint32_t reserved0:2;           // [07:06]
		uint32_t ldo_ctrl:6;            // [13:08]
		uint32_t reserved1:1;           // [14:14]
		uint32_t hold_ch_all:1;         // [15:15]
		uint32_t reserved2:4;           // [16:19]
		uint32_t byp_wait:1;            // [20:20]
		uint32_t reserved3:11;          // [21:31]
	} Bits;
	uint32_t Reg32;
} PLL_CTRL_REG;

typedef union {
	struct {
		uint32_t ndiv_int:10;           // [09:00]
		uint32_t ndiv_frac:20;          // [29:10]
		uint32_t ndiv_frac_mod_sel:1;   // [30:30]
		uint32_t ndiv_override:1;       // [31:31]
	} Bits;
	uint32_t Reg32;
} PLL_NDIV_REG;

typedef union {
	struct {
		uint32_t pdiv:4;                // [03:00]
		uint32_t reserved0:27;          // [30:04]
		uint32_t ndiv_pdiv_override:1;  // [31:31]
	} Bits;
	uint32_t Reg32;
} PLL_PDIV_REG;

typedef union {
	struct {
		uint32_t mdiv0:9;               // [08:00]
		uint32_t enableb_ch0:1;         // [09:09]
		uint32_t hold_ch0:1;            // [10:10]
		uint32_t mdel0:1;               // [11:11]
		uint32_t reserved0:3;           // [14:12]
		uint32_t mdiv_override0:1;      // [15:15]
		uint32_t mdiv1:9;               // [24:16]
		uint32_t enableb_ch1:1;         // [25:25]
		uint32_t hold_ch1:1;            // [26:26]
		uint32_t mdel1:1;               // [27:27]
		uint32_t reserved1:3;           // [30:28]
		uint32_t mdiv_override1:1;      // [31:31]
	} Bits;
	uint32_t Reg32;
} PLL_CHCFG_REG;

typedef union {
	struct {
		uint32_t reserved0:4;           // [03:00]
		uint32_t ss_ka:4;               // [07:04]
		uint32_t ss_ki:3;               // [10:08]
		uint32_t reserved1:1;           // [11:11]
		uint32_t ss_kp:4;               // [15:12]
		uint32_t ssc_step:16;           // [31:16]
	} Bits;
	uint32_t Reg32;
} PLL_LOOP0_REG;

typedef union {
	struct {
		uint32_t ssc_limit:22;          // [21:00]
		uint32_t reserved0:8;           // [29:22]
		uint32_t ssc_downspread:1;      // [30:30]
		uint32_t ssc_mode:1;            // [31:31]
	} Bits;
	uint32_t Reg32;
} PLL_LOOP1_REG;

typedef union {
	struct {
		uint32_t out:12;                // [11:00]
		uint32_t fb_lock:1;             // [12:12]
		uint32_t post_held:1;           // [13:13]
		uint32_t reserved:16;           // [29:14]
		uint32_t lock_lost:1;           // [30:30]
		uint32_t lock:1;                // [31:31]
	} Bits;
	uint32_t Reg32;
} PLL_STAT_REG;

typedef union {
	struct {
		uint32_t ndiv_int:10;           // [09:00]
		uint32_t reserved:2;            // [11:10]
		uint32_t ndiv_frac:20;          // [31:12]
	} Bits;
	uint32_t Reg32;
} PLL_DECNDIV_REG;

typedef union {
	struct {
		uint32_t pdiv:4;                // [03:00]
		uint32_t reserved:12;           // [15:04]
		uint32_t mdiv0:8;               // [23:16]
		uint32_t mdiv1:8;               // [31:24]
	} Bits;
	uint32_t Reg32;
} PLL_DECPDIV_REG;

typedef union {
	struct {
		uint32_t mdiv2:8;               // [07:00]
		uint32_t mdiv3:8;               // [15:08]
		uint32_t mdiv4:8;               // [23:16]
		uint32_t mdiv5:8;               // [31:24]
	} Bits;
	uint32_t Reg32;
} PLL_DECCH25_REG;

typedef union {
	struct {
		uint32_t manual_clk_en:1;       // [00:00]
		uint32_t manual_reset_ctl:1;    // [01:01]
		uint32_t clk_en_phase:1;        // [02:02]
		uint32_t manual_bisr_en:1;      // [03:03]
		uint32_t manual_mem_pwr:1;      // [04:04]
		uint32_t manual_amem_pwr:1;     // [05:05]
		uint32_t manual_iso_ctl:1;      // [06:06]
		uint32_t manual_ctl:1;          // [07:07]
		uint32_t dpg_ctl_en:1;          // [08:08]
		uint32_t pwr_dn_req:1;          // [09:09]
		uint32_t pwr_up_req:1;          // [10:10]
		uint32_t mem_pwr_ctl_en:1;      // [11:11]
		uint32_t blk_reset_assert:1;    // [12:12]
		uint32_t mem_stby:1;            // [13:13]
		uint32_t manual_dpg1_ctrl:1;    // [14:14]
		uint32_t manual_dpgn_ctrl:1;    // [15:15]
		uint32_t bist_shift_en:1;       // [16:16]
		uint32_t reseved:15;            // [31:17]
	} Bits;
	uint32_t Reg32;
} BPCM_PWR_ZONE_N_CONTROL;

typedef union {
	struct {
		uint32_t pwr_ok_delay_sel:3;    // [02:00]
		uint32_t pwk_ok_thresh:2;       // [04:03]
		uint32_t reserved:3;            // [07:05]
		uint32_t iso_on_delay:4;        // [11:08]
		uint32_t iso_off_delay:4;       // [15:12]
		uint32_t clock_on_delay:4;      // [19:16]
		uint32_t clock_off_delay:4;     // [23:20]
		uint32_t reset_on_delay:4;      // [27:24]
		uint32_t reset_off_delay:4;     // [31:28]
	} Bits;
	uint32_t Reg32;
} BPCM_PWR_ZONE_N_CONFIG1;

typedef union {
	struct {
		uint32_t delay_prescale_sel:3;  // [02:00]
		uint32_t slew_prescale_sel:3;   // [05:03]
		uint32_t reserved:2;            // [07:06]
		uint32_t bist_thresh:4;         // [11:08]
		uint32_t dpgn_on_delay:4;       // [15:12]
		uint32_t dpg1_on_delay:4;       // [19:16]
		uint32_t dpg_off_delay:4;       // [23:20]
		uint32_t mem_on_delay:4;        // [27:24]
		uint32_t mem_off_delay:4;       // [31:28]
	} Bits;
	uint32_t Reg32;
} BPCM_PWR_ZONE_N_CONFIG2;

typedef union {
	struct {
		uint32_t pwr_cntl_state:5;      // [04:00]
		uint32_t reserved0:3;           // [07:05]
		uint32_t dpg_capable:1;         // [08:08]
		uint32_t bist_capable:1;        // [09:09]
		uint32_t bist_timeout:1;        // [10:10]
		uint32_t mem_apon_state:1;      // [11:11]
		uint32_t mem_apok_state:1;      // [12:12]
		uint32_t mem_pon_state:1;       // [13:13]
		uint32_t pwr_off_state:1;       // [14:14]
		uint32_t pwr_on_state:1;        // [15:15]
		uint32_t pwr_good:1;            // [16:16]
		uint32_t dpg_pwr_state:1;       // [17:17]
		uint32_t mem_pwr_state:1;       // [18:18]
		uint32_t iso_state:1;           // [19:19]
		uint32_t reset_state:1;         // [20:20]
		uint32_t reserved1:11;          // [31:21]
	} Bits;
	uint32_t Reg32;
} BPCM_PWR_ZONE_N_STATUS;

typedef struct {
	BPCM_PWR_ZONE_N_CONTROL control;    // offset = 0x00, actual offset 0
	BPCM_PWR_ZONE_N_CONFIG1 config1;    // offset = 0x04, actual offset 1
	BPCM_PWR_ZONE_N_CONFIG2 config2;    // offset = 0x08, actual offset 2
	BPCM_PWR_ZONE_N_STATUS status;      // offset = 0x0c, actual offset 3
	uint32_t timer_control;             // offset = 0x10, actual offset 4
	uint32_t timer_status;              // offset = 0x14, actual offset 5
	uint32_t reserved1[2];              // offset = 0x18, actual offset 6
} BPCM_ZONE;

#define BPCMZoneOffset(reg)	            offsetof(BPCM_ZONE,reg)
#define BPCMZoneRegOffset(reg)	        (BPCMZoneOffset(reg) >> 2)

#define BPCMZoneCtrlOffset(zone)        BPCMOffset(zones[zone].control)
#define BPCMZoneCtrlRegOffset(zone)     (BPCMZoneCtrlOffset(zone) >> 2)

#define BPCMZoneStsOffset(zone)	        BPCMOffset(zones[zone].status)
#define BPCMZoneStsRegOffset(zone)	    (BPCMZoneStsOffset(zone) >> 2)

typedef union {
	struct {
		uint32_t pmb_Addr:8;            // [07:00]
		uint32_t hw_rev:8;              // [15:08]
		uint32_t module_id:16;          // [31:16]
	} Bits;
	uint32_t Reg32;
} BPCM_UBUS_ID_REG;

typedef union {
	struct {
		uint32_t num_zones:8;           // [07:00]
		uint32_t sr_reg_bits:8;         // [15:08]
		uint32_t pllType:2;             // [17:16]
		uint32_t reserved0:1;           // [18:18]
		uint32_t ubus:1;                // [19:19]
		uint32_t reserved1:12;          // [31:20]
	} Bits;
	uint32_t Reg32;
} BPCM_UBUS_CAPABILITES_REG;

typedef union {
	struct {
		uint32_t ctrl_eswap:4;          // [03:00]
		uint32_t reserved0:4;           // [07:04]
		uint32_t ctrl_cd:4;             // [11:08]
		uint32_t reserved1:4;           // [15:12]
		uint32_t ctrl_seclev:8;         // [23:16]
		uint32_t reqout_seclev:8;       // [31:24]
	} Bits;
	uint32_t Reg32;
} BPCM_UBUS_CTRL_REG;

typedef union {
	struct {
		uint64_t addr_in:24;            // [23:00]
		uint64_t addr_out:24;           // [47:24]
		uint64_t pid:8;                 // [55:48]
		uint64_t size:5;                // [60:56]
		uint64_t cmddta:1;              // [61:61]
		uint64_t en:2;                  // [63:62]
	} Bits;
	struct {
		uint32_t word0;
		uint32_t word1;
	} Regs32;
	uint64_t Reg64;
} BPCM_UBUS_CFG_REG;

typedef union {
	struct {
		uint32_t ubus_soft_reset:1;     // [00:00]
		uint32_t alt_ubus_clk_sel:1;    // [01:01]
		uint32_t observe_clk_sw_init:1; // [02:02]
		uint32_t reserved:6;            // [08:03]
		uint32_t enable:1;              // [09:09]
		uint32_t counter:8;             // [17:10]
		uint32_t wol_rst_scheme_sel:1;  // [18:18]
		uint32_t reserved2:13;          // [31:19]
	} Bits;
	uint32_t Reg32;
} BPCM_CLKRST_VREG_CONTROL;

// There is a 20-bit address used to access any given BPCM register.  The upper 8-bits
// is the device address and the lower 12-bits is used to represent the BPCM register
// set for that device.  32-bit registers are allocated on 4-byte boundaries
// (i.e. 0, 1, 2, 3...) rather than on byte boundaries (0x00, 0x04, 0x08, 0x0c...)
// Thus, to get the actual address of any given register within the device's address
// space, I'll use the "C" offsetof macro and divide the result by 4
// e.g.:
// int regOffset = offsetof(BPCM_REGS,BPCM_AVS_PWD_CONTROL);    // yields the byte offset of the target register
// int regAddress = regOffset/4;                                // yields the 32-bit word offset of the target register
// The ReadBPCMReg and WriteBPCMReg functions will always take a device address
// (address of the BPCM device) and register offset (like regOffset above).  The offset
// will be divided by 4 and used as the lower 12-bits of the actual target address, while the
// device address will serve as the upper 8-bits of the actual address.
typedef struct {
	// PMB-slave:
	BPCM_ID_REG id_reg;                     // offset = 0x00
	BPCM_CAPABILITES_REG capabilities;      // offset = 0x04
	BPCM_LINK_ADDR_REG link_address;        // offset = 0x08
	uint32_t reserved0;                     // offset = 0x0c
	// BPCM
	uint32_t dpg_zones;                     // offset = 0x10
	BPCM_SR_CONTROL sr_control;             // offset = 0x14
	uint32_t reserved1[2];                  // offset = 0x18
	// Client-specific registers
	uint32_t client_specific[24];           // offset = 0x20
	// Zones
	BPCM_ZONE zones[];                    // offset = 0x80
} BPCM_REGS;
#define BPCMOffset(reg)		offsetof(BPCM_REGS,reg)
#define BPCMRegOffset(reg)	(BPCMOffset(reg) >> 2)

typedef struct {
	BPCM_ID_REG id_reg;                     // offset = 0x00
	BPCM_CAPABILITES_REG capabilities;      // offset = 0x04
	BPCM_LINK_ADDR_REG link_address;        // offset = 0x08
	uint32_t reserved0;                     // offset = 0x0c
	uint32_t dpg_zones;                     // offset = 0x10
	BPCM_SR_CONTROL sr_control;             // offset = 0x14
	uint32_t reserved1[2];                  // offset = 0x18
	ARM_CONTROL_REG arm_control;            // offset = 0x20
	BIU_CLK_CONTROL0_REG biu_clk_control;   // offset = 0x24
	uint32_t reserved2;                     // offset = 0x28
	BIU_EVENT_REG biu_event;                // offset = 0x2c
	ARM_PWR_CTRL_REG arm_pwr_ctrl;          // offset = 0x30
	uint32_t reserved4[3];                  // offset = 0x34
	BIU_CX_CLK_CTRL_REG c0_clk_control;     // offset = 0x40
	uint32_t c0_clk_ramp;                   // offset = 0x44
	uint32_t c0_clk_pattern;                // offset = 0x48
	uint32_t reserved5;                     // offset = 0x4c
	BIU_CX_CLK_CTRL_REG c1_clk_control;     // offset = 0x50
	uint32_t c1_clk_ramp;                   // offset = 0x54
	uint32_t c1_clk_pattern;                // offset = 0x58
	uint32_t reserved6[9];                  // offset = 0x5c
	BPCM_ZONE zones;                        // offset = 0x80
} ARM_BPCM_REGS;
#define ARMBPCMOffset(reg)  offsetof(ARM_BPCM_REGS,reg)
#define ARMBPCMRegOffset(reg)   (ARMBPCMOffset(reg) >> 2)

typedef struct {
	/* BIU PLL BCPM definition */
	BPCM_ID_REG id_reg;                     // offset = 0x00
	BPCM_CAPABILITES_REG capabilities;      // offset = 0x04
	BPCM_LINK_ADDR_REG link_address;        // offset = 0x08
	uint32_t reserved0;                     // offset = 0x0c
	PLL_CTRL_REG resets;                    // offset = 0x10
	uint32_t reserved1[2];                  // offset = 0x14
	PLL_NDIV_REG ndiv;                      // offset = 0x1c
	PLL_PDIV_REG pdiv;                      // offset = 0x20
	PLL_LOOP0_REG loop0;                    // offset = 0x24
	PLL_LOOP1_REG loop1;                    // offset = 0x28
	PLL_CHCFG_REG ch01_cfg;                 // offset = 0x2c
	PLL_CHCFG_REG ch23_cfg;                 // offset = 0x30
	PLL_CHCFG_REG ch45_cfg;                 // offset = 0x34
	uint32_t reserved2;                     // offset = 0x38
	PLL_STAT_REG stat;                      // offset = 0x3c
	uint32_t strap;                         // offset = 0x40
	PLL_DECNDIV_REG decndiv;                // offset = 0x44
	PLL_DECPDIV_REG decpdiv;                // offset = 0x48
	PLL_DECCH25_REG decch25;                // offset = 0x4c
} PLL_BPCM_REGS;

#define PLLBPCMOffset(reg)	offsetof(PLL_BPCM_REGS,reg)
#define PLLBPCMRegOffset(reg)	(PLLBPCMOffset(reg) >> 2)

typedef struct {
	BPCM_UBUS_ID_REG id_reg;                // offset = 0x00
	BPCM_UBUS_CAPABILITES_REG capabilities; // offset = 0x04
	uint32_t reserved0;                     // offset = 0x08
	BPCM_UBUS_CTRL_REG ctrl;                // offset = 0x0c
	BPCM_UBUS_CFG_REG cfg[4];               // offset = 0x10
} BPCM_UBUS_REG;

#define UBUSBPCMOffset(reg)	offsetof(BPCM_UBUS_REG,reg)
#define UBUSBPCMRegOffset(reg)	(UBUSBPCMOffset(reg) >> 2)

typedef struct {
	CLKRST_MODULE_ID_REG id_reg;            // offset = 0x00
	CLKRST_CAPABILITES_REG capabilities;    // offset = 0x04
	uint32_t reserved0[7];                  // offset = 0x08
	uint32_t control;                       // offset = 0x24
	CLKRST_OBSERVE_CNTL observe_cntrol;     // offset = 0x28
	CLKRST_OBSERVE_DIV_EN observe_div_en;   // offset = 0x2c
	CLKRST_OBSERVE_DIV_EN observe_en;       // offset = 0x30
	BPCM_CLKRST_VREG_CONTROL vreg_control;  // offset = 0x34
	uint32_t xtal_control0;                 // offset = 0x38
	uint32_t xtal_control1;                 // offset = 0x3c
	uint32_t reserved1[18];                 // offset = 0x40
	uint32_t clkrst_stat;                   // offset = 0x84
	uint32_t clkrst_test_obs_sel_start;     // offset = 0x88
	uint32_t clkrst_test_ref_count_th;      // offset = 0x8c
	uint32_t clkrst_test_test_low_th;       // offset = 0x90
	uint32_t clkrst_test_test_high_th;      // offset = 0x94
	uint32_t clkrst_test_test_status0;      // offset = 0x98
	uint32_t clkrst_test_test_status1;      // offset = 0x9c
	uint32_t clkrst_obsrv_ctrl_int;         // offset = 0xa0
} BPCM_CLKRST_REGS;

#define CLKRSTBPCMOffset(reg)  offsetof(BPCM_CLKRST_REGS, reg)
#define CLKRSTBPCMRegOffset(reg)   (CLKRSTBPCMOffset(reg) >> 2)

// *************************** macros ******************************
#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((unsigned long) &((TYPE *)0)->MEMBER)
#endif

#endif
