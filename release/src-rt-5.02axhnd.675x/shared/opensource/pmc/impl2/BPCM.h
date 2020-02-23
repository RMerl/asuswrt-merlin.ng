#ifndef BPCM_H
#define BPCM_H

#include "bcmtypes.h"

#ifndef NULL
	#define NULL ((void *)0)
#endif

typedef union
{
	struct {
		uint32	sw_strap : 16;
		uint32	hw_rev	 : 8;
		uint32	pmb_Addr : 8;
	} Bits;
	uint32 Reg32;
} BPCM_ID_REG;

// types of PMB devices
enum {
	kPMB_BPCM = 0,
	kPMB_MIPS_PLL = 1,
	kPMB_GEN_PLL = 2,
	kPMB_LC_PLL = 3,
    // 4..15 reserved
};

typedef union
{
	struct {
		uint32	reserved1: 12;
		uint32  devType  :  4;	// see enum above
        uint32  reserved2:  8;
		uint32	num_zones:  8;
	} Bits;
	uint32 Reg32;
} BPCM_CAPABILITES_REG;

typedef union
{
	struct {
		uint32	reserved : 31;
		uint32	pwd_alert:  1;
	} Bits;
	uint32 Reg32;
} BPCM_STATUS_REG;

typedef union
{
	struct {
		uint32	interval	 : 16;
		uint32	alert_h		 :  1;
		uint32	valid_h		 :  1;
		uint32	alert_s		 :  1;
		uint32	valid_s		 :  1;
		uint32	reserved	 :  4;
		uint32	continuous_h :  1;
		uint32	continuous_s :  1;
		uint32	thresh_en_h	 :  1;
		uint32	thresh_en_s	 :  1;
		uint32	ectr_en_h	 :  1;
		uint32	ectr_en_s	 :  1;
		uint32	ro_en_h	 	 :  1;
		uint32	ro_en_s	 	 :  1;
	} Bits;
	uint32 Reg32;
} BPCM_AVS_ROSC_CONTROL_REG;

typedef union
{
	struct {
		uint32	thresh_hi	: 16;
		uint32	thresh_lo	: 16;
	} Bits;
	uint32 Reg32;
} BPCM_AVS_ROSC_THRESHOLD;

typedef union
{
	struct {
		uint32	count_h	: 16;
		uint32	count_s	: 16;
	} Bits;
	uint32 Reg32;
} BPCM_AVS_ROSC_COUNT;

typedef union
{
	struct {
		uint32	reserved1		:  2;
		uint32	clr_cfg			:  3;
		uint32	rsel			:  3;
		uint32	ccfg			:  8;
		uint32	alert			:  1;
		uint32  reserved2		:  6;
		uint32	pwd_tm_en		:  1;
		uint32	start			:  6;
		uint32	pwd_alert_sel	:  1;
		uint32	pwd_en			:  1;
	} Bits;
	uint32 Reg32;
} BPCM_AVS_PWD_CONTROL;

typedef union
{
	struct {
		uint32	reset_state			:  1;	// R/O
		uint32	iso_state			:  1;	// R/O
		uint32	mem_pwr_state		:  1;	// R/O
		uint32	dpg_pwr_state		:  1;	// R/O
		uint32	pwr_good			:  1;	// R/O
		uint32  pwr_on_state		:  1;	// R/O
		uint32	pwr_off_state		:  1;	// R/O
		uint32	freq_scalar_dyn_sel	:  1;	// R/O
		uint32	reserved			: 11;
		uint32	blk_reset_assert	:  1;
		uint32	mem_pwr_ctl_en		:  1;
		uint32	pwr_up_req			:  1;
		uint32	pwr_dn_req			:  1;
		uint32	dpg_ctl_en			:  1;
		uint32	manual_ctl			:  1;
		uint32	manual_iso_ctl		:  1;
		uint32	manual_mem_pwr		:  2;
		uint32	dpg_capable			:  1;	// R/O
		uint32	freq_scale_used		:  1;	// R/O
		uint32	manual_reset_ctl	:  1;
		uint32	manual_clk_en		:  1;
	} Bits;
	uint32 Reg32;
} BPCM_PWR_ZONE_N_CONTROL;

typedef union
{
	struct {
		uint32	reset_off_delay		:  4;
		uint32	reset_on_delay		:  4;
		uint32	clock_off_delay		:  4;
		uint32	clock_on_delay		:  4;
		uint32	iso_off_delay		:  4;
		uint32  iso_on_delay		:  4;
		uint32	reserved			:  3;
		uint32	pwk_ok_thresh		:  2;
		uint32	pwr_ok_delay_sel	:  3;
	} Bits;
	uint32 Reg32;
} BPCM_PWR_ZONE_N_CONFIG1;

typedef union
{
	struct {
		uint32	mem_off_delay		:  4;
		uint32	mem_on_delay		:  4;
		uint32	dpg_off_delay		:  4;
		uint32	dpg1_on_delay		:  4;
		uint32	dpg2_on_delay		:  4;
		uint32	dpgn_on_delay		:  4;
		uint32	reserved			:  6;
		uint32	slew_prescale_sel	:  3;
		uint32	delay_prescale_sel	:  3;
	} Bits;
	uint32 Reg32;
} BPCM_PWR_ZONE_N_CONFIG2;

typedef union
{
	struct {
		uint32	reserved			: 22;
		uint32	high_gear_div		:  3;
		uint32	low_gear_div		:  3;
		uint32	reserved2			:  1;
		uint32	use_dyn_gear_sel	:  1;
		uint32	gear_sel			:  1;
		uint32	fs_bypass_en		:  1;
	} Bits;
	uint32 Reg32;
} BPCM_ZONE_N_FREQ_SCALAR_CONTROL;

typedef struct
{
	BPCM_PWR_ZONE_N_CONTROL			control;
	BPCM_PWR_ZONE_N_CONFIG1			config1;
	BPCM_PWR_ZONE_N_CONFIG2 		config2;
	BPCM_ZONE_N_FREQ_SCALAR_CONTROL	freq_scalar_control;
} BPCM_ZONE;

// There is a 20-bit address used to access any given BPCM register.  The upper 8-bits 
// is the device address and the lower 12-bits is used to represent the BPCM register 
// set for that device.  32-bit registers are allocated on 4-byte boundaries 
// (i.e. 0, 1, 2, 3...) rather than on byte boundaries (0x00, 0x04, 0x08, 0x0c...)
// Thus, to get the actual address of any given register within the device's address
// space, I'll use the "C" offsetof macro and divide the result by 4
// e.g.:
// int regOffset = offsetof(BPCM_REGS,BPCM_AVS_PWD_CONTROL); // yeilds the byte offset of the target register
// int regAddress = regOffset/4;							 // yeilds the 32-bit offset of the target register
// The ReadBPCMReg and WriteBPCMReg functions will always take a device address 
// (address of the BPCM device) and register offset (like regOffset above).  The offset 
// will be divided by 4 and used as the lower 12-bits of the actual target address, while the
// device address will serve as the upper 8-bits of the actual address.
typedef struct 
{
	BPCM_ID_REG					id_reg;			// offset = 0x00, actual offset = 0
	BPCM_CAPABILITES_REG		capabilities;	// offset = 0x04, actual offset = 1
	uint32						control;		// offset = 0x08, actual offset = 2
	BPCM_STATUS_REG				status;			// offset = 0x0c, actual offset = 3
	BPCM_AVS_ROSC_CONTROL_REG	rosc_control;	// offset = 0x10, actual offset = 4
	BPCM_AVS_ROSC_THRESHOLD		rosc_threshold;	// offset = 0x14, actual offset = 5
	BPCM_AVS_ROSC_COUNT			rosc_count;		// offset = 0x18, actual offset = 6
	BPCM_AVS_PWD_CONTROL		pwd_control;	// offset = 0x1c, actual offset = 7
	uint32						reseved[8];		// offset = 0x20, actual offset = 8..15					
	BPCM_ZONE					zones[1020];	// offset = 0x40..0x3FFC, actual offset = 16..4095 (1020 * 4 = 4080 + 16 = 4096)
} BPCM_REGS;									// total offset space = 4096

// *************************** macros ******************************
#define BPCMOffset(reg) offsetof(BPCM_REGS,reg)

#endif
