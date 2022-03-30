/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2007 Nobuhiro Iwamatsu
 * Copyright (C) 2008 Yusuke Goda <goda.yusuke@renesas.com>
 *
 * u-boot/board/r7780mp/r7780mp.h
 */

#ifndef _BOARD_R7780MP_R7780MP_H_
#define _BOARD_R7780MP_R7780MP_H_

/* R7780MP's FPGA register map */
#define FPGA_BASE          0xa4000000
#define FPGA_IRLMSK        (FPGA_BASE + 0x00)
#define FPGA_IRLMON        (FPGA_BASE + 0x02)
#define FPGA_IRLPRI1       (FPGA_BASE + 0x04)
#define FPGA_IRLPRI2       (FPGA_BASE + 0x06)
#define FPGA_IRLPRI3       (FPGA_BASE + 0x08)
#define FPGA_IRLPRI4       (FPGA_BASE + 0x0A)
#define FPGA_RSTCTL        (FPGA_BASE + 0x0C)
#define FPGA_PCIBD         (FPGA_BASE + 0x0E)
#define FPGA_PCICD         (FPGA_BASE + 0x10)
#define FPGA_EXTGIO        (FPGA_BASE + 0x16)
#define FPGA_IVDRMON       (FPGA_BASE + 0x18)
#define FPGA_IVDRCR        (FPGA_BASE + 0x1A)
#define FPGA_OBLED         (FPGA_BASE + 0x1C)
#define FPGA_OBSW          (FPGA_BASE + 0x1E)
#define FPGA_TPCTL         (FPGA_BASE + 0x100)
#define FPGA_TPDCKCTL      (FPGA_BASE + 0x102)
#define FPGA_TPCLR         (FPGA_BASE + 0x104)
#define FPGA_TPXPOS        (FPGA_BASE + 0x106)
#define FPGA_TPYPOS        (FPGA_BASE + 0x108)
#define FPGA_DBSW          (FPGA_BASE + 0x200)
#define FPGA_VERSION       (FPGA_BASE + 0x700)
#define FPGA_CFCTL         (FPGA_BASE + 0x300)
#define FPGA_CFPOW         (FPGA_BASE + 0x302)
#define FPGA_CFCDINTCLR    (FPGA_BASE + 0x304)
#define FPGA_PMR           (FPGA_BASE + 0x900)

#endif /* _BOARD_R7780RP_R7780RP_H_ */
