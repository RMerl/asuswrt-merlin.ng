/* SPDX-License-Identifier: GPL-2.0 */
/*
 * arch/arm/cpu/armv7/rmobile/pfc-r8a7790.h
 *
 * Copyright (C) 2013 Renesas Electronics Corporation
 */

#ifndef __PFC_R8A7790_H__
#define __PFC_R8A7790_H__

#include <sh_pfc.h>
#include <asm/gpio.h>

#define CPU_32_PORT(fn, pfx, sfx)				\
	PORT_10(fn, pfx, sfx), PORT_10(fn, pfx##1, sfx),	\
	PORT_10(fn, pfx##2, sfx), PORT_1(fn, pfx##30, sfx),	\
	PORT_1(fn, pfx##31, sfx)

#define CPU_32_PORT2(fn, pfx, sfx)				\
	PORT_10(fn, pfx, sfx), PORT_10(fn, pfx##1, sfx),	\
	PORT_10(fn, pfx##2, sfx)

#if defined(CONFIG_R8A7790)
#define CPU_32_PORT1(fn, pfx, sfx)				\
	PORT_10(fn, pfx, sfx), PORT_10(fn, pfx##1, sfx),	\
	PORT_10(fn, pfx##2, sfx)				\
/* GP_0_0_DATA -> GP_5_31_DATA (except for GP1[30],GP1[31],GP2[30],GP2[31]) */
#define CPU_ALL_PORT(fn, pfx, sfx)				\
	CPU_32_PORT(fn, pfx##_0_, sfx),				\
	CPU_32_PORT1(fn, pfx##_1_, sfx),			\
	CPU_32_PORT2(fn, pfx##_2_, sfx),			\
	CPU_32_PORT(fn, pfx##_3_, sfx),				\
	CPU_32_PORT(fn, pfx##_4_, sfx),				\
	CPU_32_PORT(fn, pfx##_5_, sfx)

#elif defined(CONFIG_R8A7791)
#define CPU_32_PORT1(fn, pfx, sfx)				\
	PORT_10(fn, pfx, sfx), PORT_10(fn, pfx##1, sfx),	\
	PORT_1(fn, pfx##20, sfx), PORT_1(fn, pfx##21, sfx),	\
	PORT_1(fn, pfx##22, sfx), PORT_1(fn, pfx##23, sfx),	\
	PORT_1(fn, pfx##24, sfx), PORT_1(fn, pfx##25, sfx)

/*
 * GP_0_0_DATA -> GP_7_25_DATA
 * (except for GP1[26],GP1[27],GP1[28],GP1[29]),GP1[30]),GP1[31]
 *  GP7[26],GP7[27],GP7[28],GP7[29]),GP7[30]),GP7[31])
 */
#define CPU_ALL_PORT(fn, pfx, sfx)				\
	CPU_32_PORT(fn, pfx##_0_, sfx),				\
	CPU_32_PORT1(fn, pfx##_1_, sfx),			\
	CPU_32_PORT(fn, pfx##_2_, sfx),				\
	CPU_32_PORT(fn, pfx##_3_, sfx),				\
	CPU_32_PORT(fn, pfx##_4_, sfx),				\
	CPU_32_PORT(fn, pfx##_5_, sfx),				\
	CPU_32_PORT(fn, pfx##_6_, sfx),				\
	CPU_32_PORT1(fn, pfx##_7_, sfx)

#elif defined(CONFIG_R8A7792)
/*
 * GP_0_0_DATA -> GP_11_29_DATA
 * (except for GP0[29..31],GP1[23..31],GP3[28..31],GP4[17..31],GP5[17..31]
 *  GP6[17..31],GP7[17..31],GP8[17..31],GP9[17..31],GP11[30..31])
 */
#define CPU_32_PORT0_28(fn, pfx, sfx)				\
	PORT_10(fn, pfx, sfx), PORT_10(fn, pfx##1, sfx),	\
	PORT_1(fn, pfx##20, sfx), PORT_1(fn, pfx##21, sfx),	\
	PORT_1(fn, pfx##22, sfx), PORT_1(fn, pfx##23, sfx),	\
	PORT_1(fn, pfx##24, sfx), PORT_1(fn, pfx##25, sfx),	\
	PORT_1(fn, pfx##26, sfx), PORT_1(fn, pfx##27, sfx),	\
	PORT_1(fn, pfx##28, sfx)

#define CPU_32_PORT0_22(fn, pfx, sfx)				\
	PORT_10(fn, pfx, sfx), PORT_10(fn, pfx##1, sfx),	\
	PORT_1(fn, pfx##20, sfx), PORT_1(fn, pfx##21, sfx),	\
	PORT_1(fn, pfx##22, sfx)

#define CPU_32_PORT0_27(fn, pfx, sfx)				\
	PORT_10(fn, pfx, sfx), PORT_10(fn, pfx##1, sfx),	\
	PORT_1(fn, pfx##20, sfx), PORT_1(fn, pfx##21, sfx),	\
	PORT_1(fn, pfx##22, sfx), PORT_1(fn, pfx##23, sfx),	\
	PORT_1(fn, pfx##24, sfx), PORT_1(fn, pfx##25, sfx),	\
	PORT_1(fn, pfx##26, sfx), PORT_1(fn, pfx##27, sfx)

#define CPU_32_PORT0_16(fn, pfx, sfx)				\
	PORT_10(fn, pfx, sfx), 					\
	PORT_1(fn, pfx##10, sfx),PORT_1(fn, pfx##11, sfx),	\
	PORT_1(fn, pfx##12, sfx), PORT_1(fn, pfx##13, sfx),	\
	PORT_1(fn, pfx##14, sfx), PORT_1(fn, pfx##15, sfx),	\
	PORT_1(fn, pfx##16, sfx)

#define CPU_ALL_PORT(fn, pfx, sfx)				\
	CPU_32_PORT0_28(fn, pfx##_0_, sfx),			\
	CPU_32_PORT0_22(fn, pfx##_1_, sfx),			\
	CPU_32_PORT(fn, pfx##_2_, sfx),				\
	CPU_32_PORT0_27(fn, pfx##_3_, sfx),			\
	CPU_32_PORT0_16(fn, pfx##_4_, sfx),			\
	CPU_32_PORT0_16(fn, pfx##_5_, sfx),			\
	CPU_32_PORT0_16(fn, pfx##_6_, sfx),			\
	CPU_32_PORT0_16(fn, pfx##_7_, sfx),			\
	CPU_32_PORT0_16(fn, pfx##_8_, sfx),			\
	CPU_32_PORT0_16(fn, pfx##_9_, sfx),			\
	CPU_32_PORT(fn, pfx##_10_, sfx),			\
	CPU_32_PORT2(fn, pfx##_11_, sfx)

#else
#error "NO support"
#endif

#define _GP_GPIO(pfx, sfx) PINMUX_GPIO(GPIO_GP##pfx, GP##pfx##_DATA)
#define _GP_DATA(pfx, sfx) PINMUX_DATA(GP##pfx##_DATA, GP##pfx##_FN,	\
				       GP##pfx##_IN, GP##pfx##_OUT)

#define _GP_INOUTSEL(pfx, sfx) GP##pfx##_IN, GP##pfx##_OUT
#define _GP_INDT(pfx, sfx) GP##pfx##_DATA

#define GP_ALL(str)	CPU_ALL_PORT(_PORT_ALL, GP, str)
#define PINMUX_GPIO_GP_ALL()	CPU_ALL_PORT(_GP_GPIO, , unused)
#define PINMUX_DATA_GP_ALL()	CPU_ALL_PORT(_GP_DATA, , unused)

#define PORT_10_REV(fn, pfx, sfx)				\
	PORT_1(fn, pfx##9, sfx), PORT_1(fn, pfx##8, sfx),	\
	PORT_1(fn, pfx##7, sfx), PORT_1(fn, pfx##6, sfx),	\
	PORT_1(fn, pfx##5, sfx), PORT_1(fn, pfx##4, sfx),	\
	PORT_1(fn, pfx##3, sfx), PORT_1(fn, pfx##2, sfx),	\
	PORT_1(fn, pfx##1, sfx), PORT_1(fn, pfx##0, sfx)

#define CPU_32_PORT_REV(fn, pfx, sfx)					\
	PORT_1(fn, pfx##31, sfx), PORT_1(fn, pfx##30, sfx),		\
	PORT_10_REV(fn, pfx##2, sfx), PORT_10_REV(fn, pfx##1, sfx),	\
	PORT_10_REV(fn, pfx, sfx)

#define GP_INOUTSEL(bank) CPU_32_PORT_REV(_GP_INOUTSEL, _##bank##_, unused)
#define GP_INDT(bank) CPU_32_PORT_REV(_GP_INDT, _##bank##_, unused)

#define PINMUX_IPSR_DATA(ipsr, fn) PINMUX_DATA(fn##_MARK, FN_##ipsr, FN_##fn)
#define PINMUX_IPSR_MODSEL_DATA(ipsr, fn, ms) PINMUX_DATA(fn##_MARK, FN_##ms, \
							  FN_##ipsr, FN_##fn)

#endif /* __PFC_R8A7790_H__ */
