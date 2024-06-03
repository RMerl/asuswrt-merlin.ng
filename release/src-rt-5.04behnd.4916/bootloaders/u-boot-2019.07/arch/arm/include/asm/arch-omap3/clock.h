/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2006-2008
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 */
#ifndef _CLOCKS_H_
#define _CLOCKS_H_

#define LDELAY		12000000

#define S12M		12000000
#define S13M		13000000
#define S19_2M		19200000
#define S24M		24000000
#define S26M		26000000
#define S38_4M		38400000

#define FCK_IVA2_ON	0x00000001
#define FCK_CORE1_ON	0x03fffe29
#define ICK_CORE1_ON	0x3ffffffb
#define ICK_CORE2_ON	0x0000001f
#define FCK_WKUP_ON	0x000000e9
#define ICK_WKUP_ON	0x0000003f
#define FCK_DSS_ON	0x00000005
#define ICK_DSS_ON	0x00000001
#define FCK_CAM_ON	0x00000001
#define ICK_CAM_ON	0x00000001

/* Used to index into DPLL parameter tables */
typedef struct {
	unsigned int m;
	unsigned int n;
	unsigned int fsel;
	unsigned int m2;
} dpll_param;

struct dpll_per_36x_param {
	unsigned int sys_clk;
	unsigned int m;
	unsigned int n;
	unsigned int m2;
	unsigned int m3;
	unsigned int m4;
	unsigned int m5;
	unsigned int m6;
	unsigned int m2div;
};

/* Following functions are exported from lowlevel_init.S */
extern dpll_param *get_mpu_dpll_param(void);
extern dpll_param *get_iva_dpll_param(void);
extern dpll_param *get_core_dpll_param(void);
extern dpll_param *get_per_dpll_param(void);
extern dpll_param *get_per2_dpll_param(void);

extern dpll_param *get_36x_mpu_dpll_param(void);
extern dpll_param *get_36x_iva_dpll_param(void);
extern dpll_param *get_36x_core_dpll_param(void);
extern dpll_param *get_36x_per_dpll_param(void);
extern dpll_param *get_36x_per2_dpll_param(void);

#endif
