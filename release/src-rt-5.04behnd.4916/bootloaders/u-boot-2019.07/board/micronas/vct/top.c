// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Stefan Roese <sr@denx.de>, DENX Software Engineering
 *
 * Copyright (C) 2006 Micronas GmbH
 */

#include <common.h>
#include "vct.h"

typedef union _TOP_PINMUX_t
{
	u32 reg;
	struct {
		u32 res		: 24;   /* reserved		*/
		u32 drive	:  2;   /* Driver strength	*/
		u32 slew	:  1;   /* Slew rate		*/
		u32 strig	:  1;   /* Schmitt trigger input*/
		u32 pu_pd	:  2;   /* Pull up/ pull down	*/
		u32 funsel	:  2;   /* Pin function		*/
	} Bits;
} TOP_PINMUX_t;

#if defined(CONFIG_VCT_PREMIUM) || defined(CONFIG_VCT_PLATINUM)

static TOP_PINMUX_t top_read_pin(int pin)
{
	TOP_PINMUX_t reg;

	switch (pin) {
	case 2:
	case 3:
	case 6:
	case 9:
		reg.reg = 0xdeadbeef;
		break;
	case 4:
		reg.reg = reg_read(FWSRAM_TOP_SCL_CFG(FWSRAM_BASE));
		break;
	case 5:
		reg.reg = reg_read(FWSRAM_TOP_SDA_CFG(FWSRAM_BASE));
		break;
	case 7:
		reg.reg = reg_read(FWSRAM_TOP_TDO_CFG(FWSRAM_BASE));
		break;
	case 8:
		reg.reg = reg_read(FWSRAM_TOP_GPIO2_0_CFG(FWSRAM_BASE));
		break;
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
		reg.reg = reg_read(FWSRAM_BASE + FWSRAM_TOP_GPIO2_1_CFG_OFFS +
				   ((pin - 10) * 4));
		break;
	default:
		reg.reg = reg_read(TOP_BASE + (pin * 4));
		break;
	}

	return reg;
}

static void top_write_pin(int pin, TOP_PINMUX_t reg)
{

	switch (pin) {
	case 4:
		reg_write(FWSRAM_TOP_SCL_CFG(FWSRAM_BASE), reg.reg);
		break;
	case 5:
		reg_write(FWSRAM_TOP_SDA_CFG(FWSRAM_BASE), reg.reg);
		break;
	case 7:
		reg_write(FWSRAM_TOP_TDO_CFG(FWSRAM_BASE), reg.reg);
		break;
	case 8:
		reg_write(FWSRAM_TOP_GPIO2_0_CFG(FWSRAM_BASE), reg.reg);
		break;
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
		reg_write(FWSRAM_BASE + FWSRAM_TOP_GPIO2_1_CFG_OFFS +
			  ((pin - 10) * 4), reg.reg);
		break;
	default:
		reg_write(TOP_BASE + (pin * 4), reg.reg);
		break;
	}
}

int top_set_pin(int pin, int func)
{
	TOP_PINMUX_t reg;

	/* check global range */
	if ((pin < 0) || (pin > 170) || (func < 0) || (func > 3))
		return -1;  /* pin number or function out of valid range */

	/* check undefined values; */
	if ((pin == 2) || (pin == 3) || (pin == 6) || (pin == 9))
		return -1;  /* pin number out of valid range */

	reg = top_read_pin(pin);
	reg.Bits.funsel = func;
	top_write_pin(pin, reg);

	return 0;
}

#endif

#if defined(CONFIG_VCT_PLATINUMAVC)

int top_set_pin(int pin, int func)
{
	TOP_PINMUX_t reg;

	/* check global range */
	if ((pin < 0) || (pin > 158))
		return -1;	/* pin number or function out of valid range */

	reg.reg = reg_read(TOP_BASE + (pin * 4));
	reg.Bits.funsel = func;
	reg_write(TOP_BASE + (pin * 4), reg.reg);

	return 0;
}

#endif

void vct_pin_mux_initialize(void)
{
#if defined(CONFIG_VCT_PREMIUM) || defined(CONFIG_VCT_PLATINUM)
	top_set_pin(34, 01);	/* EBI_CS0	*/
	top_set_pin(33, 01);	/* EBI_CS1	*/
	top_set_pin(32, 01);	/* EBI_CS2	*/
	top_set_pin(100, 02);	/* EBI_CS3	*/
	top_set_pin(101, 02);	/* EBI_CS4	*/
	top_set_pin(102, 02);	/* EBI_CS5	*/
	top_set_pin(103, 02);	/* EBI_CS6	*/
	top_set_pin(104, 02);	/* EBI_CS7	top_set_pin(104,03); EBI_GENIO3 */
	top_set_pin(35, 01);	/* EBI_ALE	*/
	top_set_pin(36, 01);	/* EBI_ADDR15	*/
	top_set_pin(37, 01);	/* EBI_ADDR14	top_set_pin(78,03); EBI_ADDR14 */
	top_set_pin(38, 01);	/* EBI_ADDR13	*/
	top_set_pin(39, 01);	/* EBI_ADDR12	*/
	top_set_pin(40, 01);	/* EBI_ADDR11	*/
	top_set_pin(41, 01);	/* EBI_ADDR10	*/
	top_set_pin(42, 01);	/* EBI_ADDR9	*/
	top_set_pin(43, 01);	/* EBI_ADDR8	*/
	top_set_pin(44, 01);	/* EBI_ADDR7	*/
	top_set_pin(45, 01);	/* EBI_ADDR6	*/
	top_set_pin(46, 01);	/* EBI_ADDR5	*/
	top_set_pin(47, 01);	/* EBI_ADDR4	*/
	top_set_pin(48, 01);	/* EBI_ADDR3	*/
	top_set_pin(49, 01);	/* EBI_ADDR2	*/
	top_set_pin(50, 01);	/* EBI_ADDR1	*/
	top_set_pin(51, 01);	/* EBI_ADDR0	*/
	top_set_pin(52, 01);	/* EBI_DIR	*/
	top_set_pin(53, 01);	/* EBI_DAT15	top_set_pin(81,01); EBI_DAT15 */
	top_set_pin(54, 01);	/* EBI_DAT14	top_set_pin(82,01); EBI_DAT14 */
	top_set_pin(55, 01);	/* EBI_DAT13	top_set_pin(83,01); EBI_DAT13 */
	top_set_pin(56, 01);	/* EBI_DAT12	top_set_pin(84,01); EBI_DAT12 */
	top_set_pin(57, 01);	/* EBI_DAT11	top_set_pin(85,01); EBI_DAT11 */
	top_set_pin(58, 01);	/* EBI_DAT10	top_set_pin(86,01); EBI_DAT10 */
	top_set_pin(59, 01);	/* EBI_DAT9	top_set_pin(87,01); EBI_DAT9 */
	top_set_pin(60, 01);	/* EBI_DAT8	top_set_pin(88,01); EBI_DAT8 */
	top_set_pin(61, 01);	/* EBI_DAT7	*/
	top_set_pin(62, 01);	/* EBI_DAT6	*/
	top_set_pin(63, 01);	/* EBI_DAT5	*/
	top_set_pin(64, 01);	/* EBI_DAT4	*/
	top_set_pin(65, 01);	/* EBI_DAT3	*/
	top_set_pin(66, 01);	/* EBI_DAT2	*/
	top_set_pin(67, 01);	/* EBI_DAT1	*/
	top_set_pin(68, 01);	/* EBI_DAT0	*/
	top_set_pin(69, 01);	/* EBI_IORD	*/
	top_set_pin(70, 01);	/* EBI_IOWR	*/
	top_set_pin(71, 01);	/* EBI_WE	*/
	top_set_pin(72, 01);	/* EBI_OE	*/
	top_set_pin(73, 01);	/* EBI_IORDY	*/
	top_set_pin(95, 02);	/* EBI_EBI_DMACK*/
	top_set_pin(112, 02);	/* EBI_IRQ0	*/
	top_set_pin(111, 02);	/* EBI_IRQ1	top_set_pin(111,03); EBI_DMARQ */
	top_set_pin(107, 02);	/* EBI_IRQ2	*/
	top_set_pin(108, 02);	/* EBI_IRQ3	*/
	top_set_pin(30, 01);	/* EBI_GENIO1   top_set_pin(99,03); EBI_GENIO1 */
	top_set_pin(31, 01);	/* EBI_GENIO2   top_set_pin(98,03); EBI_GENIO2 */
	top_set_pin(105, 02);	/* EBI_GENIO3   top_set_pin(104,03); EBI_GENIO3 */
	top_set_pin(106, 02);	/* EBI_GENIO4   top_set_pin(144,02); EBI_GENIO4 */
	top_set_pin(109, 02);	/* EBI_GENIO5   top_set_pin(142,02); EBI_GENIO5 */
	top_set_pin(110, 02);	/* EBI_BURST_CLK	*/
#endif

#if defined(CONFIG_VCT_PLATINUMAVC)
	top_set_pin(19, 01);	/* EBI_CS0	*/
	top_set_pin(18, 01);	/* EBI_CS1	*/
	top_set_pin(17, 01);	/* EBI_CS2	*/
	top_set_pin(92, 02);	/* EBI_CS3	*/
	top_set_pin(93, 02);	/* EBI_CS4	*/
	top_set_pin(95, 02);	/* EBI_CS6	*/
	top_set_pin(96, 02);	/* EBI_CS7	top_set_pin(104,03); EBI_GENIO3 */
	top_set_pin(20, 01);	/* EBI_ALE	*/
	top_set_pin(21, 01);	/* EBI_ADDR15	*/
	top_set_pin(22, 01);	/* EBI_ADDR14	top_set_pin(78,03); EBI_ADDR14 */
	top_set_pin(23, 01);	/* EBI_ADDR13	*/
	top_set_pin(24, 01);	/* EBI_ADDR12	*/
	top_set_pin(25, 01);	/* EBI_ADDR11	*/
	top_set_pin(26, 01);	/* EBI_ADDR10	*/
	top_set_pin(27, 01);	/* EBI_ADDR9	*/
	top_set_pin(28, 01);	/* EBI_ADDR8	*/
	top_set_pin(29, 01);	/* EBI_ADDR7	*/
	top_set_pin(30, 01);	/* EBI_ADDR6	*/
	top_set_pin(31, 01);	/* EBI_ADDR5	*/
	top_set_pin(32, 01);	/* EBI_ADDR4	*/
	top_set_pin(33, 01);	/* EBI_ADDR3	*/
	top_set_pin(34, 01);	/* EBI_ADDR2	*/
	top_set_pin(35, 01);	/* EBI_ADDR1	*/
	top_set_pin(36, 01);	/* EBI_ADDR0	*/
	top_set_pin(37, 01);	/* EBI_DIR	*/
	top_set_pin(38, 01);	/* EBI_DAT15	top_set_pin(81,01); EBI_DAT15 */
	top_set_pin(39, 01);	/* EBI_DAT14	top_set_pin(82,01); EBI_DAT14 */
	top_set_pin(40, 01);	/* EBI_DAT13	top_set_pin(83,01); EBI_DAT13 */
	top_set_pin(41, 01);	/* EBI_DAT12	top_set_pin(84,01); EBI_DAT12 */
	top_set_pin(42, 01);	/* EBI_DAT11	top_set_pin(85,01); EBI_DAT11 */
	top_set_pin(43, 01);	/* EBI_DAT10	top_set_pin(86,01); EBI_DAT10 */
	top_set_pin(44, 01);	/* EBI_DAT9	top_set_pin(87,01); EBI_DAT9 */
	top_set_pin(45, 01);	/* EBI_DAT8	top_set_pin(88,01); EBI_DAT8 */
	top_set_pin(46, 01);	/* EBI_DAT7	*/
	top_set_pin(47, 01);	/* EBI_DAT6	*/
	top_set_pin(48, 01);	/* EBI_DAT5	*/
	top_set_pin(49, 01);	/* EBI_DAT4	*/
	top_set_pin(50, 01);	/* EBI_DAT3	*/
	top_set_pin(51, 01);	/* EBI_DAT2	*/
	top_set_pin(52, 01);	/* EBI_DAT1	*/
	top_set_pin(53, 01);	/* EBI_DAT0	*/
	top_set_pin(54, 01);	/* EBI_IORD	*/
	top_set_pin(55, 01);	/* EBI_IOWR	*/
	top_set_pin(56, 01);	/* EBI_WE	*/
	top_set_pin(57, 01);	/* EBI_OE	*/
	top_set_pin(58, 01);	/* EBI_IORDY	*/
	top_set_pin(87, 02);	/* EBI_EBI_DMACK*/
	top_set_pin(106, 02);	/* EBI_IRQ0	*/
	top_set_pin(105, 02);	/* EBI_IRQ1	top_set_pin(111,03); EBI_DMARQ */
	top_set_pin(101, 02);	/* EBI_IRQ2	*/
	top_set_pin(102, 02);	/* EBI_IRQ3	*/
	top_set_pin(15, 01);	/* EBI_GENIO1   top_set_pin(99,03); EBI_GENIO1 */
	top_set_pin(16, 01);	/* EBI_GENIO2   top_set_pin(98,03); EBI_GENIO2 */
	top_set_pin(99, 02);	/* EBI_GENIO3   top_set_pin(104,03); EBI_GENIO3 */
	top_set_pin(100, 02);	/* EBI_GENIO4   top_set_pin(144,02); EBI_GENIO4 */
	top_set_pin(103, 02);	/* EBI_GENIO5   top_set_pin(142,02); EBI_GENIO5 */
	top_set_pin(104, 02);	/* EBI_BURST_CLK	*/
#endif

	/* I2C: Configure I2C-2 as GPIO to enable soft-i2c */
	top_set_pin(0, 2);	/* SCL2 on GPIO 11 */
	top_set_pin(1, 2);	/* SDA2 on GPIO 10 */

	/* UART pins */
#if defined(CONFIG_VCT_PREMIUM) || defined(CONFIG_VCT_PLATINUM)
	top_set_pin(141, 1);
	top_set_pin(143, 1);
#endif
#if defined(CONFIG_VCT_PLATINUMAVC)
	top_set_pin(107, 1);
	top_set_pin(109, 1);
#endif
}
