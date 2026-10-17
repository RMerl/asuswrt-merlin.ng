// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/*
 
*/

#ifndef BPCM_H
#define BPCM_H

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
		uint32_t hw_rev:8;	            // [15:08]
		uint32_t strap:16;	            // [31:16]
	} Bits;
	uint32_t Reg32;
} CLKRST_MODULE_ID_REG;

typedef union {
	struct {
		uint32_t num_zones:8;           // [07:00]
		uint32_t num_sr_bits:8;         // [15:08]
		uint32_t sw_strap:10;	        // [25:16]
		uint32_t reserved1:5;           // [30:26]
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
		uint32_t link_valid:1;	        // [23:23]
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
		uint32_t xrdp_mpm:4;            // [07:04]
		uint32_t orion:4;               // [11:08]
		uint32_t usb:4;                 // [15:12]
		uint32_t memc:4;                // [19:16]
		uint32_t pcie:4;                // [23:20]
		uint32_t periph:4;              // [27:24]
		uint32_t reserved:4;            // [31:28]
	} Bits;
	uint32_t Reg32;
} CLKRST_OBSERVE_CNTL;

typedef union {
	struct {
		uint32_t egphy0:1;              // [00:00]
		uint32_t egphy1:1;              // [01:01]
		uint32_t egphy2:1;              // [02:02]
		uint32_t egphy4:1;              // [03:03]
		uint32_t xport0:1;              // [04:04]
		uint32_t xport1:1;              // [05:05]
		uint32_t xport2:1;              // [06:06]
		uint32_t xrdp:1;                // [07:07]
		uint32_t rctop:1;               // [08:08]
		uint32_t orion:1;               // [09:09]
		uint32_t usb:1;                 // [10:10]
		uint32_t memc:1;                // [11:11]
		uint32_t pcie0:1;               // [12:12]
		uint32_t pcie1:1;               // [13:13]
		uint32_t periph1:1;             // [14:14]
		uint32_t periph2:1;             // [15:15]
		uint32_t clkrst:1;              // [16:16]
		uint32_t crypto:1;              // [17:17]
		uint32_t wan:1;                 // [18:17]
		uint32_t reserved:13;           // [31:19]
	} Bits;
	uint32_t Reg32;
} CLKRST_OBSERVE_DIV_EN;

typedef union {
	struct {
		uint32_t cpu_reset_n:8;         // [07:00]
		uint32_t c0l2_reset:1;          // [08:08]
		uint32_t c1l2_reset:1;          // [09:09]
		uint32_t reserved0:6;	        // [15:10]
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
		uint32_t reserved:28;    	    // [31:04]
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
		uint32_t resetb:1;	            // [00:00]
		uint32_t post_resetb:1;         // [01:01]
		uint32_t pwrdwn:1;	            // [02:02]
		uint32_t master_reset:1;	    // [03:03]
		uint32_t pwrdwn_ldo:1;	        // [04:04]
		uint32_t iso:1;                 // [05:05]
		uint32_t reserved0:2;	        // [07:06]
		uint32_t ldo_ctrl:6;	        // [13:08]
		uint32_t reserved1:1;	        // [14:14]
		uint32_t hold_ch_all:1;	        // [15:15]
		uint32_t reserved2:4;	        // [16:19]
		uint32_t byp_wait:1;	        // [20:20]
		uint32_t reserved3:11;	        // [21:31]
	} Bits;
	uint32_t Reg32;
} PLL_CTRL_REG;

typedef union {
	struct {
		uint32_t fb_offset:12;          // [11:00]
		uint32_t fb_phase_en:1;         // [12:12]
		uint32_t _8phase_en:1;          // [13:13]
		uint32_t sr:18;                 // [31:14]
	} Bits;
	uint32_t Reg32;
} PLL_PHASE_REG;

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
		uint32_t reserved0:27;	        // [30:04]
		uint32_t ndiv_pdiv_override:1;  // [31:31]
	} Bits;
	uint32_t Reg32;
} PLL_PDIV_REG;

typedef union {
	struct {
		uint32_t mdiv0:8;               // [07:00]
		uint32_t enableb_ch0:1;         // [08:08]
		uint32_t hold_ch0:1;            // [09:09]
        uint32_t reserved0:1;           // [10:10]
		uint32_t mdel0:1;               // [11:11]
		uint32_t reserved1:3;           // [14:12]
		uint32_t mdiv_override0:1;      // [15:15]
		uint32_t mdiv1:8;               // [23:16]
		uint32_t enableb_ch1:1;         // [24:24]
		uint32_t hold_ch1:1;            // [25:25]
        uint32_t reserved2:1;           // [26:26]
		uint32_t mdel1:1;               // [27:27]
		uint32_t reserved3:3;           // [30:28]
		uint32_t mdiv_override1:1;      // [31:31]
	} Bits;
	uint32_t Reg32;
} PLL_CLASSIC_CHCFG_REG;

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
		uint32_t ss_ka:3;               // [06:04]
		uint32_t reserved1:1;           // [07:07]
		uint32_t ss_ki:3;               // [10:08]
		uint32_t reserved2:1;           // [11:11]
		uint32_t ss_kp:4;               // [15:12]
		uint32_t ssc_step:16;           // [31:16]
	} Bits;
	uint32_t Reg32;
} PLL_LOOP0_REG;

typedef union {
	struct {
		uint32_t ssc_limit:22;          // [21:00]
		uint32_t reserved0:9;           // [30:22]
		uint32_t ssc_mode:1;            // [31:31]
	} Bits;
	uint32_t Reg32;
} PLL_LOOP1_REG;

typedef union {
	struct {
		uint32_t dco_ctrl_bypass:12;    // [11:00]
		uint32_t dco_bypass_en:1;       // [12:12]
		uint32_t state_reset:1;         // [13:13]
		uint32_t state_sel:3;           // [16:14]
		uint32_t state_update:1;        // [17:17]
        uint32_t reserved0:2;           // [19:18]
        uint32_t state_mode:2;          // [21:20]
        uint32_t pwm_rate:2;            // [23:22]
		uint32_t post_resetb_sel:2;     // [25:24]
		uint32_t vco_fb_div2:1;         // [26:26]
		uint32_t fast_phase_lock:1;     // [27:27]
        uint32_t ndiv_relock:1;         // [28:28]
        uint32_t reserved1:1;           // [29:29]
		uint32_t vco_range:2;           // [31:30]
	} Bits;
	uint32_t Reg32;
} PLL_CFG0_REG;

typedef union {
	struct {
		uint32_t ldo:2;	                // [01:00]
		uint32_t reserved:30;           // [31:02]
	} Bits;
	uint32_t Reg32;
} PLL_CFG1_REG;

typedef union {
	struct {
		uint32_t en_cml:3;              // [02:00]
		uint32_t tri_en:1;              // [03:03]
		uint32_t test_sel:3;            // [06:04]
		uint32_t test_en:1;             // [07:07]
		uint32_t reserved0:24;          // [31:08]
	} Bits;
	uint32_t Reg32;
} PLL_OCTRL_REG;

typedef union {
	struct {
		uint32_t out:12;                // [11:00]
		uint32_t reserved:18;           // [29:12]
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
		uint32_t mdiv5:9;               // [12:04]
		uint32_t mdiv0:9;               // [21:13]
		uint32_t mdiv1:9;               // [30:22]
		uint32_t reserved0:1;           // [31:31]
	} Bits;
	uint32_t Reg32;
} RDPPLL_DECPDIV_REG;

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
		uint32_t mdiv2:9;               // [08:00]
		uint32_t mdiv3:9;               // [17:09]
		uint32_t mdiv4:9;               // [26:18]
		uint32_t reserved0:5;           // [31:27]
	} Bits;
	uint32_t Reg32;
} RDPPLL_DECCH25_REG;

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
		uint32_t clk_en_phase:1;	    // [02:02]
		uint32_t dpg_capable:1;	        // [03:03]
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
		uint32_t mem_apon_state:1;      // [16:16]
		uint32_t mem_apok_state:1;      // [17:17]
		uint32_t mem_pon_state:1;       // [18:18]
		uint32_t pwr_cntl_state:5;      // [23:19]
		uint32_t reserved0:1;	        // [24:24]
		uint32_t pwr_off_state:1;	    // [25:25]
		uint32_t pwr_on_state:1;	    // [26:26]
		uint32_t pwr_good:1;	        // [27:27]
		uint32_t dpg_pwr_state:1;	    // [28:28]
		uint32_t mem_pwr_state:1;	    // [29:29]
		uint32_t iso_state:1;	        // [30:30]
		uint32_t reset_state:1;	        // [31:31]
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
		uint32_t reserved:6;            // [11:06]
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
		uint32_t fs_bypass_en:1;        // [00:00]
		uint32_t gear_sel:1;            // [01:01]
		uint32_t use_dyn_gear_sel:1;    // [02:02]
		uint32_t reserved2:1;           // [03:03]
		uint32_t low_gear_div:3;        // [06:04]
		uint32_t high_gear_div:3;       // [09:07]
		uint32_t reserved:22;           // [31:10]
	} Bits;
	uint32_t Reg32;
} BPCM_ZONE_N_FREQ_SCALAR_CONTROL;

typedef struct {
	BPCM_PWR_ZONE_N_CONTROL control;    // offset = 0x00, actual offset 0
	BPCM_PWR_ZONE_N_CONFIG1 config1;    // offset = 0x04, actual offset 1
	BPCM_PWR_ZONE_N_CONFIG2 config2;    // offset = 0x08, actual offset 2
	uint32_t reserved0;                 // offset = 0x0c, actual offset 3
	uint32_t timer_control;             // offset = 0x10, actual offset 4
	uint32_t timer_status;              // offset = 0x14, actual offset 5
	uint32_t reserved1[2];              // offset = 0x18, actual offset 6
} BPCM_ZONE;

#define BPCMZoneOffset(reg)	offsetof(BPCM_ZONE,reg)
#define BPCMZoneRegOffset(reg)	(BPCMZoneOffset(reg) >> 2)

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
		uint32_t alt_emmc_clk_sel:1;    // [03:03]
		uint32_t reserved:5;            // [08:04]
		uint32_t enable:1;              // [09:09]
		uint32_t counter:8;             // [17:10]
		uint32_t reserved2:14;          // [31:18]
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
	BPCM_ID_REG id_reg;	                // offset 0x00, PMB reg index 0
	BPCM_CAPABILITES_REG capabilities;	// offset 0x04, PMB reg index 1
	BPCM_LINK_ADDR_REG link_address;	// offset 0x08, PMB reg index 2
	uint32_t reserved0;  	            // offset 0x0c, PMB reg index 3
	// BPCM
	uint32_t dpg_zones;	                // offset 0x10, PMB reg index 4
	uint32_t soft_reset;	            // offset 0x14, PMB reg index 5
	uint32_t reserved1[2];	            // offset 0x18, PMB reg index 6/7
	// Client-specific registers
	uint32_t client_specific[24];	    // offset 0x20, PMB reg index 8..31
	// Zones
	BPCM_ZONE zones[];	                // offset 0x80..(0x20 + MAX_ZONES*32)), PMB reg index 32..(32+(MAX_ZONES*8-1))
} BPCM_REGS;			// total offset space = 4096

#define BPCM_OFFSET(reg) (offsetof(BPCM_REGS,reg)>>2)
#define BPCMOffset(reg)		offsetof(BPCM_REGS,reg)
#define BPCMRegOffset(reg)	(BPCMOffset(reg) >> 2)

typedef struct {
	BPCM_ID_REG id_reg;                     // offset = 0x00, actual offset = 0
	BPCM_CAPABILITES_REG capabilities;      // offset = 0x04, actual offset = 1
	uint32_t reserved0[2];                  // offset = 0x08, actual offset = 2
	uint32_t cfg_control;                   // offset = 0x10, actual offset = 4
	BPCM_SR_CONTROL sr_control;             // offset = 0x14, actual offset = 5
	uint32_t reserved1[2];                  // offset = 0x18, actual offset = 6
	ARM_CONTROL_REG arm_control;            // offset = 0x20, actual offset = 8
    BIU_CLK_CONTROL0_REG biu_clk_control;   // offset = 0x24, actual offset = 9
    uint32_t reserved2;                     // offset = 0x28, actual offset = 10
    BIU_EVENT_REG biu_event;                // offset = 0x2c, actual offset = 11
    ARM_PWR_CTRL_REG arm_pwr_ctrl;          // offset = 0x30, actual offset = 12
    uint32_t reserved4[3];                  // offset = 0x34, actual offset = 13
    BIU_CX_CLK_CTRL_REG c0_clk_control;     // offset = 0x40, actual offset = 16
    uint32_t c0_clk_ramp;                   // offset = 0x44, actual offset = 17
    uint32_t c0_clk_pattern;                // offset = 0x48, actual offset = 18
    uint32_t reserved5;                     // offset = 0x4c, actual offset = 19
    BIU_CX_CLK_CTRL_REG c1_clk_control;     // offset = 0x50, actual offset = 20
    uint32_t c1_clk_ramp;                   // offset = 0x54, actual offset = 21
    uint32_t c1_clk_pattern;                // offset = 0x58, actual offset = 22
    uint32_t reserved6;                     // offset = 0x5c, actual offset = 23
} ARM_BPCM_REGS;
#define ARMBPCMOffset(reg)  offsetof(ARM_BPCM_REGS,reg)
#define ARMBPCMRegOffset(reg)   (ARMBPCMOffset(reg) >> 2)

typedef struct {
	BPCM_ID_REG id_reg;                     // offset = 0x00, 
	BPCM_CAPABILITES_REG capabilities;      // offset = 0x04, 
	uint32_t reserved0[6];                  // offset = 0x08, 
	PLL_CTRL_REG resets;                    // offset = 0x20, 
	PLL_CFG0_REG cfg0;                      // offset = 0x24, 
	PLL_CFG1_REG cfg1;                      // offset = 0x28, 
	PLL_NDIV_REG ndiv;                      // offset = 0x2c, 
	PLL_PDIV_REG pdiv;                      // offset = 0x30, 
	PLL_LOOP0_REG loop0;                    // offset = 0x34, 
	uint32_t reserved1;                     // offset = 0x38, 
	PLL_LOOP1_REG loop1;                    // offset = 0x3c, 
	PLL_CLASSIC_CHCFG_REG ch01_cfg;         // offset = 0x40, 
	PLL_CLASSIC_CHCFG_REG ch23_cfg;         // offset = 0x44, 
	PLL_CLASSIC_CHCFG_REG ch45_cfg;         // offset = 0x48, 
	PLL_STAT_REG stat;                      // offset = 0x4c, 
	uint32_t strap;                         // offset = 0x50, 
	PLL_DECNDIV_REG decndiv;                // offset = 0x54, 
	PLL_DECPDIV_REG decpdiv;                // offset = 0x58, 
	PLL_DECCH25_REG decch25;                // offset = 0x5c, 
} PLL_CLASSIC_BPCM_REGS;

#define PLLCLASSICBPCMOffset(reg)  offsetof(PLL_CLASSIC_BPCM_REGS,reg)
#define PLLCLASSICBPCMRegOffset(reg)   (PLLCLASSICBPCMOffset(reg) >> 2)

typedef struct {
	/* BIU PLL BCPM definition */  
	BPCM_ID_REG id_reg;                     // offset = 0x00 
	BPCM_CAPABILITES_REG capabilities;      // offset = 0x04
	uint32_t reserved0[2];                  // offset = 0x08..0x0c
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
	BPCM_ID_REG id_reg;                     // offset = 0x00
	BPCM_CAPABILITES_REG capabilities;	    // offset = 0x04
	uint32_t reserved0[2];                  // offset = 0x08..0x0c
	PLL_CTRL_REG resets;                    // offset = 0x10
	uint32_t reserved1[5];                  // offset = 0x14
	PLL_NDIV_REG ndiv;                      // offset = 0x28
	PLL_PDIV_REG pdiv;                      // offset = 0x2c
	PLL_CHCFG_REG ch01_cfg;                 // offset = 0x30
	PLL_CHCFG_REG ch23_cfg;                 // offset = 0x34
	PLL_CHCFG_REG ch45_cfg;                 // offset = 0x38
	PLL_STAT_REG stat;                      // offset = 0x3c
	uint32_t strap;                         // offset = 0x40
	PLL_DECNDIV_REG decndiv;                // offset = 0x44
	RDPPLL_DECPDIV_REG decpdiv;             // offset = 0x48
	RDPPLL_DECCH25_REG decch25;             // offset = 0x4c
} RDPPLL_BPCM_REGS;

#define RDPPLLBPCMOffset(reg)		offsetof(RDPPLL_BPCM_REGS,reg)
#define RDPPLLBPCMRegOffset(reg)	(RDPPLLBPCMOffset(reg) >> 2)

typedef union {
    struct {
        uint32_t mdiv1_ch1:2;               //[01:00]
        uint32_t mdiv2_ch1:2;               //[03:02]
        uint32_t reserved1:5;               //[08:04]
        uint32_t en_ch1:1;                  //[09:09]
        uint32_t hold_ch1:1;                //[10:10]
        uint32_t load_en_ch1:1;             //[11:11]
        uint32_t mdel0:1;                   //[12:12]
        uint32_t reserved2:2;               //[14:13]
        uint32_t mdiv_ch1_over:1;           //[15:15]
        uint32_t mdiv1_ch2:2;               //[17:16]
        uint32_t mdiv2_ch2:2;               //[19:18]
        uint32_t reserved3:5;               //[24:20]
        uint32_t en_ch2:1;                  //[25:25]
        uint32_t hold_ch2:1;                //[26:26]
        uint32_t load_en_ch2:1;             //[27:27]
        uint32_t mdel2:1;                   //[28:28]
        uint32_t reserved4:2;               //[30:29]
        uint32_t mdiv_ch2_over:1;           //[31:31]
    } Bits;
    uint32_t Reg32;
} PLL_ORION_CHCFG_REG;

typedef union {
    struct {
        uint32_t ph0_cfg_rate:6;            //[05:00]
        uint32_t ph0_cfg_mdiv2:2;           //[07:06]
        uint32_t ph90_cfg_rate:6;           //[13:08]
        uint32_t ph90_cfg_mdiv2:2;          //[15:14]
        uint32_t ph180_cfg_rate:6;          //[21:16]
        uint32_t ph180_cfg_mdiv2:2;         //[23:22]
        uint32_t ph270_cfg_rate:6;          //[29:24]
        uint32_t ph270_cfg_mdiv2:2;         //[31:30]
    } Bits;
    uint32_t Reg32;
} PLL_ORION_4PHASE_CFG0_REG;

typedef union {
    struct {
        uint32_t phase_slew_up:3;          //[02:00]
        uint32_t reserved1:1;               //[03:03]
        uint32_t phase_slew_down:3;        //[06:04]
        uint32_t reserved2:1;               //[07:07]
        uint32_t phase_bup_en:4;           //[11:08]
        uint32_t phase_slow:4;             //[15:12]
        uint32_t phase_hold:4;             //[19:16]
        uint32_t phase_enableb:1;          //[20:20]
        uint32_t phase_rate_mode_sel:1;    //[21:21]
        uint32_t phase_slow_rate:6;        //[27:22]
        uint32_t phase_ph0_override:1;     //[28:28]
        uint32_t phase_ph90_override:1;    //[29:29]
        uint32_t phase_ph180_override:1;   //[30:30]
        uint32_t phase_ph270_override:1;   //[31:31]
    } Bits;
    uint32_t Reg32;
} PLL_ORION_4PHASE_CFG1_REG;

typedef struct {
	BPCM_ID_REG id_reg;                     // offset = 0x00
	BPCM_CAPABILITES_REG capabilities;	    // offset = 0x04
	uint32_t reserved0[2];                  // offset = 0x08..0x0c
	PLL_CTRL_REG resets;                    // offset = 0x10
	uint32_t reserved1[2];                  // offset = 0x14..0x18
	PLL_NDIV_REG ndiv;                      // offset = 0x1c
	PLL_PDIV_REG pdiv;                      // offset = 0x20
	PLL_LOOP0_REG loop0;                    // offset = 0x24
	PLL_LOOP1_REG loop1;                    // offset = 0x28
	PLL_ORION_CHCFG_REG ch12_cfg;           // offset = 0x2c
    PLL_ORION_4PHASE_CFG0_REG phase_cfg0;   // offset = 0x30
    PLL_ORION_4PHASE_CFG1_REG phase_cfg1;   // offset = 0x34
    uint32_t reserved2;                     // offset = 0x38
    uint32_t pll_stat;                      // offset = 0x3c
    uint32_t pll_strap;                     // offset = 0x40
    uint32_t pll_decndiv;                   // offset = 0x44
    uint32_t pll_dec4ph0;                   // offset = 0x48
    uint32_t pll_dec4ph1_pdiv;              // offset = 0x4c
} ORION_PLL_BPCM_REGS;
#define ORIONPLLBPCMOffset(reg)		offsetof(ORION_PLL_BPCM_REGS,reg)
#define ORIONPLLBPCMRegOffset(reg)	(ORIONPLLBPCMOffset(reg) >> 2)

typedef struct {
	BPCM_UBUS_ID_REG id_reg;                // offset = 0x00, actual offset = 0
	BPCM_UBUS_CAPABILITES_REG capabilities; // offset = 0x04, actual offset = 1
	uint32_t reserved0;                     // offset = 0x08, actual offset = 2
	BPCM_UBUS_CTRL_REG ctrl;                // offset = 0x0c, actual offset = 3
	BPCM_UBUS_CFG_REG cfg[4];               // offset = 0x10..0x2c, actual offset = 4..11
} BPCM_UBUS_REG;

#define UBUSBPCMOffset(reg)	offsetof(BPCM_UBUS_REG,reg)
#define UBUSBPCMRegOffset(reg)	(UBUSBPCMOffset(reg) >> 2)

typedef struct {
	CLKRST_MODULE_ID_REG id_reg;            // offset = 0x00, actual offset = 0
	CLKRST_CAPABILITES_REG capabilities;    // offset = 0x04, actual offset = 1
	uint32_t reserved0[7];                  // offset = 0x08, actual offset = 2
	uint32_t control;                       // offset = 0x24, actual offset = 9
	CLKRST_OBSERVE_CNTL observe_cntrol;	    // offset = 0x28, actual offset = 10
	CLKRST_OBSERVE_DIV_EN observe_div_en;   // offset = 0x2c, actual offset = 11
	CLKRST_OBSERVE_DIV_EN observe_en;       // offset = 0x30, actual offset = 12
	BPCM_CLKRST_VREG_CONTROL vreg_control;  // offset = 0x34, actual offset = 13
	uint32_t xtal_control0;                 // offset = 0x38, actual offset = 14
	uint32_t xtal_control1;                 // offset = 0x3c, actual offset = 15
    uint32_t reserved1[18];                 // offset = 0x40, actual offset = 16
	uint32_t clkrst_stat;	                // offset = 0x84, actual offset = 33 
    uint32_t clkrst_test_obs_sel_start;     // offset = 0x88, actual offset = 34
    uint32_t clkrst_test_ref_count_th;      // offset = 0x8c, actual offset = 35
    uint32_t clkrst_test_test_low_th;       // offset = 0x90, actual offset = 36
    uint32_t clkrst_test_test_high_th;      // offset = 0x94, actual offset = 37
    uint32_t clkrst_test_test_status0;      // offset = 0x98, actual offset = 38
    uint32_t clkrst_test_test_status1;      // offset = 0x9c, actual offset = 39
    uint32_t clkrst_obsrv_ctrl_int;         // offset = 0xa0, actual offset = 40
} BPCM_CLKRST_REGS;

#define CLKRSTBPCMOffset(reg)  offsetof(BPCM_CLKRST_REGS, reg)
#define CLKRSTBPCMRegOffset(reg)   (CLKRSTBPCMOffset(reg) >> 2)

typedef struct {
    // ETH_PMB
	BPCM_ID_REG id_reg;                     // offset 0x00, PMB reg index 0
	BPCM_CAPABILITES_REG capabilities;      // offset 0x04, PMB reg index 1
	uint32_t reserved0[2];                  // offset 0x08, PMB reg index 2/3
	// ETH_CFG
	uint32_t dpg_zones;                     // offset 0x10, PMB reg index 4
	uint32_t soft_rst;                      // offset 0x14, PMB reg index 5
	uint32_t reserved1[2];                  // offset 0x18, PMB reg index 6/7
    // ETH_CORE
	uint32_t qphy_cntrl;                    // offset 0x20
	uint32_t qphy_status;                   // offset 0x24
	uint32_t reserved2[2];                  // offset 0x28
	uint32_t serdes0_cntrl;                 // offset 0x30
	uint32_t serdes0_status;                // offset 0x34
	uint32_t serdes_apd_st;                 // offset 0x38
	uint32_t serdes1_cntrl;                 // offset 0x3c
	uint32_t serdes1_status;                // offset 0x40
	uint32_t xphy0_status;                  // offset 0x44
	uint32_t serdes2_cntrl;                 // offset 0x48
	uint32_t serdes2_status;                // offset 0x4c
	uint32_t xphy1_status;                  // offset 0x50
	uint32_t rgmii_cntrl;                   // offset 0x54
	uint32_t xport0_cntrl;                  // offset 0x58
	uint32_t xport1_cntrl;                  // offset 0x5c
	uint32_t xport2_cntrl;                  // offset 0x60
	uint32_t xphy0_ctrl;                    // offset 0x64
	uint32_t xphy1_ctrl;                    // offset 0x68
	uint32_t serdes0_pwr_cntrl;             // offset 0x6c
	uint32_t serdes1_pwr_cntrl;             // offset 0x70
	uint32_t serdes2_pwr_cntrl;             // offset 0x74
	uint32_t qphy_test_cntrl;               // offset 0x78
} BPCM_ETH_REGS;

#define BPCMETHOffset(reg)		offsetof(BPCM_ETH_REGS,reg)
#define BPCMETHRegOffset(reg)	(BPCMETHOffset(reg) >> 2)

// *************************** macros ******************************
#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((unsigned long) &((TYPE *)0)->MEMBER)
#endif
#endif


