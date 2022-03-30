/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Heungjun Kim <riverful.kim@samsung.com>
 */

#ifndef __ASM_ARM_ARCH_CLOCK_H_
#define __ASM_ARM_ARCH_CLOCK_H_

#ifndef __ASSEMBLY__
struct s5pc100_clock {
	unsigned int	apll_lock;
	unsigned int	mpll_lock;
	unsigned int	epll_lock;
	unsigned int	hpll_lock;
	unsigned char	res1[0xf0];
	unsigned int	apll_con;
	unsigned int	mpll_con;
	unsigned int	epll_con;
	unsigned int	hpll_con;
	unsigned char	res2[0xf0];
	unsigned int	src0;
	unsigned int	src1;
	unsigned int	src2;
	unsigned int	src3;
	unsigned char	res3[0xf0];
	unsigned int	div0;
	unsigned int	div1;
	unsigned int	div2;
	unsigned int	div3;
	unsigned int	div4;
	unsigned char	res4[0x1ec];
	unsigned int	gate_d00;
	unsigned int	gate_d01;
	unsigned int	gate_d02;
	unsigned char	res5[0x54];
	unsigned int	gate_sclk0;
	unsigned int	gate_sclk1;
};

struct s5pc110_clock {
	unsigned int	apll_lock;
	unsigned char	res1[0x4];
	unsigned int	mpll_lock;
	unsigned char	res2[0x4];
	unsigned int	epll_lock;
	unsigned char	res3[0xc];
	unsigned int	vpll_lock;
	unsigned char	res4[0xdc];
	unsigned int	apll_con;
	unsigned char	res5[0x4];
	unsigned int	mpll_con;
	unsigned char	res6[0x4];
	unsigned int	epll_con;
	unsigned char	res7[0xc];
	unsigned int	vpll_con;
	unsigned char	res8[0xdc];
	unsigned int	src0;
	unsigned int	src1;
	unsigned int	src2;
	unsigned int	src3;
	unsigned char	res9[0xf0];
	unsigned int	div0;
	unsigned int	div1;
	unsigned int	div2;
	unsigned int	div3;
	unsigned int	div4;
	unsigned char	res10[0x1ec];
	unsigned int	gate_d00;
	unsigned int	gate_d01;
	unsigned int	gate_d02;
	unsigned char	res11[0x54];
	unsigned int	gate_sclk0;
	unsigned int	gate_sclk1;
};
#endif

#endif
