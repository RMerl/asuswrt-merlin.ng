/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 Xilinx Inc.
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

extern void zynq_slcr_lock(void);
extern void zynq_slcr_unlock(void);
extern void zynq_slcr_cpu_reset(void);
extern void zynq_slcr_devcfg_disable(void);
extern void zynq_slcr_devcfg_enable(void);
extern u32 zynq_slcr_get_boot_mode(void);
extern u32 zynq_slcr_get_idcode(void);
extern int zynq_slcr_get_mio_pin_status(const char *periph);
extern void zynq_ddrc_init(void);
extern unsigned int zynq_get_silicon_version(void);

int zynq_board_read_rom_ethaddr(unsigned char *ethaddr);

#endif /* _SYS_PROTO_H_ */
