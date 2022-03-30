/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2008 Stefan Roese <sr@denx.de>, DENX Software Engineering
 */

#define GPIO1_BASE		0x00044000
#define GPIO2_BASE		0x00048000

/* Instances */
#define GPIO_INSTANCES		2

/*  Relative offsets of the register adresses */
#define GPIO_SWPORTA_DR_OFFS	0x00000000
#define GPIO_SWPORTA_DR(base)	((base) + GPIO_SWPORTA_DR_OFFS)
#define GPIO_SWPORTA_DDR_OFFS	0x00000004
#define GPIO_SWPORTA_DDR(base)	((base) + GPIO_SWPORTA_DDR_OFFS)
#define GPIO_EXT_PORTA_OFFS	0x00000050
#define GPIO_EXT_PORTA(base)	((base) + GPIO_EXT_PORTA_OFFS)
