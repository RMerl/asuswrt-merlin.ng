/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012-2013, Xilinx, Michal Simek
 *
 * (C) Copyright 2012
 * Joe Hershberger <joe.hershberger@ni.com>
 */

#ifndef _ZYNQPL_H_
#define _ZYNQPL_H_

#include <xilinx.h>

#ifdef CONFIG_CMD_ZYNQ_AES
int zynq_decrypt_load(u32 srcaddr, u32 dstaddr, u32 srclen, u32 dstlen);
#endif

extern struct xilinx_fpga_op zynq_op;

#define XILINX_ZYNQ_XC7Z007S	0x3
#define XILINX_ZYNQ_XC7Z010	0x2
#define XILINX_ZYNQ_XC7Z012S	0x1c
#define XILINX_ZYNQ_XC7Z014S	0x8
#define XILINX_ZYNQ_XC7Z015	0x1b
#define XILINX_ZYNQ_XC7Z020	0x7
#define XILINX_ZYNQ_XC7Z030	0xc
#define XILINX_ZYNQ_XC7Z035	0x12
#define XILINX_ZYNQ_XC7Z045	0x11
#define XILINX_ZYNQ_XC7Z100	0x16

/* Device Image Sizes */
#define XILINX_XC7Z007S_SIZE	16669920/8
#define XILINX_XC7Z010_SIZE	16669920/8
#define XILINX_XC7Z012S_SIZE	28085344/8
#define XILINX_XC7Z014S_SIZE	32364512/8
#define XILINX_XC7Z015_SIZE	28085344/8
#define XILINX_XC7Z020_SIZE	32364512/8
#define XILINX_XC7Z030_SIZE	47839328/8
#define XILINX_XC7Z035_SIZE	106571232/8
#define XILINX_XC7Z045_SIZE	106571232/8
#define XILINX_XC7Z100_SIZE	139330784/8

/* Device Names */
#define XILINX_XC7Z007S_NAME	"7z007s"
#define XILINX_XC7Z010_NAME	"7z010"
#define XILINX_XC7Z012S_NAME	"7z012s"
#define XILINX_XC7Z014S_NAME	"7z014s"
#define XILINX_XC7Z015_NAME	"7z015"
#define XILINX_XC7Z020_NAME	"7z020"
#define XILINX_XC7Z030_NAME	"7z030"
#define XILINX_XC7Z035_NAME	"7z035"
#define XILINX_XC7Z045_NAME	"7z045"
#define XILINX_XC7Z100_NAME	"7z100"

#if defined(CONFIG_FPGA)
#define ZYNQ_DESC(name) { \
	.idcode = XILINX_ZYNQ_XC##name, \
	.fpga_size = XILINX_XC##name##_SIZE, \
	.devicename = XILINX_XC##name##_NAME \
	}
#else
#define ZYNQ_DESC(name) { \
	.idcode = XILINX_ZYNQ_XC##name, \
	.devicename = XILINX_XC##name##_NAME \
	}
#endif

#endif /* _ZYNQPL_H_ */
