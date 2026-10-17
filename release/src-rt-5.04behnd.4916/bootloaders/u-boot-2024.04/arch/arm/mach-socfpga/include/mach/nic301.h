/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Marek Vasut <marex@denx.de>
 */

#ifndef	_NIC301_REGISTERS_H_
#define	_NIC301_REGISTERS_H_

struct nic301_registers {
	u32	remap;				/* 0x0 */
	/* Security Register Group */
	u32	_pad_0x4_0x8[1];
	u32	l4main;
	u32	l4sp;
	u32	l4mp;				/* 0x10 */
	u32	l4osc1;
	u32	l4spim;
	u32	stm;
	u32	lwhps2fpgaregs;			/* 0x20 */
	u32	_pad_0x24_0x28[1];
	u32	usb1;
	u32	nanddata;
	u32	_pad_0x30_0x80[20];
	u32	usb0;				/* 0x80 */
	u32	nandregs;
	u32	qspidata;
	u32	fpgamgrdata;
	u32	hps2fpgaregs;			/* 0x90 */
	u32	acp;
	u32	rom;
	u32	ocram;
	u32	sdrdata;			/* 0xA0 */
	u32	_pad_0xa4_0x1fd0[1995];
	/* ID Register Group */
	u32	periph_id_4;			/* 0x1FD0 */
	u32	_pad_0x1fd4_0x1fe0[3];
	u32	periph_id_0;			/* 0x1FE0 */
	u32	periph_id_1;
	u32	periph_id_2;
	u32	periph_id_3;
	u32	comp_id_0;			/* 0x1FF0 */
	u32	comp_id_1;
	u32	comp_id_2;
	u32	comp_id_3;
	u32	_pad_0x2000_0x2008[2];
	/* L4 MAIN */
	u32	l4main_fn_mod_bm_iss;
	u32	_pad_0x200c_0x3008[1023];
	/* L4 SP */
	u32	l4sp_fn_mod_bm_iss;
	u32	_pad_0x300c_0x4008[1023];
	/* L4 MP */
	u32	l4mp_fn_mod_bm_iss;
	u32	_pad_0x400c_0x5008[1023];
	/* L4 OSC1 */
	u32	l4osc_fn_mod_bm_iss;
	u32	_pad_0x500c_0x6008[1023];
	/* L4 SPIM */
	u32	l4spim_fn_mod_bm_iss;
	u32	_pad_0x600c_0x7008[1023];
	/* STM */
	u32	stm_fn_mod_bm_iss;
	u32	_pad_0x700c_0x7108[63];
	u32	stm_fn_mod;
	u32	_pad_0x710c_0x8008[959];
	/* LWHPS2FPGA */
	u32	lwhps2fpga_fn_mod_bm_iss;
	u32	_pad_0x800c_0x8108[63];
	u32	lwhps2fpga_fn_mod;
	u32	_pad_0x810c_0xa008[1983];
	/* USB1 */
	u32	usb1_fn_mod_bm_iss;
	u32	_pad_0xa00c_0xa044[14];
	u32	usb1_ahb_cntl;
	u32	_pad_0xa048_0xb008[1008];
	/* NANDDATA */
	u32	nanddata_fn_mod_bm_iss;
	u32	_pad_0xb00c_0xb108[63];
	u32	nanddata_fn_mod;
	u32	_pad_0xb10c_0x20008[21439];
	/* USB0 */
	u32	usb0_fn_mod_bm_iss;
	u32	_pad_0x2000c_0x20044[14];
	u32	usb0_ahb_cntl;
	u32	_pad_0x20048_0x21008[1008];
	/* NANDREGS */
	u32	nandregs_fn_mod_bm_iss;
	u32	_pad_0x2100c_0x21108[63];
	u32	nandregs_fn_mod;
	u32	_pad_0x2110c_0x22008[959];
	/* QSPIDATA */
	u32	qspidata_fn_mod_bm_iss;
	u32	_pad_0x2200c_0x22044[14];
	u32	qspidata_ahb_cntl;
	u32	_pad_0x22048_0x23008[1008];
	/* FPGAMGRDATA */
	u32	fpgamgrdata_fn_mod_bm_iss;
	u32	_pad_0x2300c_0x23040[13];
	u32	fpgamgrdata_wr_tidemark;	/* 0x23040 */
	u32	_pad_0x23044_0x23108[49];
	u32	fn_mod;
	u32	_pad_0x2310c_0x24008[959];
	/* HPS2FPGA */
	u32	hps2fpga_fn_mod_bm_iss;
	u32	_pad_0x2400c_0x24040[13];
	u32	hps2fpga_wr_tidemark;		/* 0x24040 */
	u32	_pad_0x24044_0x24108[49];
	u32	hps2fpga_fn_mod;
	u32	_pad_0x2410c_0x25008[959];
	/* ACP */
	u32	acp_fn_mod_bm_iss;
	u32	_pad_0x2500c_0x25108[63];
	u32	acp_fn_mod;
	u32	_pad_0x2510c_0x26008[959];
	/* Boot ROM */
	u32	bootrom_fn_mod_bm_iss;
	u32	_pad_0x2600c_0x26108[63];
	u32	bootrom_fn_mod;
	u32	_pad_0x2610c_0x27008[959];
	/* On-chip RAM */
	u32	ocram_fn_mod_bm_iss;
	u32	_pad_0x2700c_0x27040[13];
	u32	ocram_wr_tidemark;		/* 0x27040 */
	u32	_pad_0x27044_0x27108[49];
	u32	ocram_fn_mod;
	u32	_pad_0x2710c_0x42024[27590];
	/* DAP */
	u32	dap_fn_mod2;
	u32	dap_fn_mod_ahb;
	u32	_pad_0x4202c_0x42100[53];
	u32	dap_read_qos;			/* 0x42100 */
	u32	dap_write_qos;
	u32	dap_fn_mod;
	u32	_pad_0x4210c_0x43100[1021];
	/* MPU */
	u32	mpu_read_qos;			/* 0x43100 */
	u32	mpu_write_qos;
	u32	mpu_fn_mod;
	u32	_pad_0x4310c_0x44028[967];
	/* SDMMC */
	u32	sdmmc_fn_mod_ahb;
	u32	_pad_0x4402c_0x44100[53];
	u32	sdmmc_read_qos;			/* 0x44100 */
	u32	sdmmc_write_qos;
	u32	sdmmc_fn_mod;
	u32	_pad_0x4410c_0x45100[1021];
	/* DMA */
	u32	dma_read_qos;			/* 0x45100 */
	u32	dma_write_qos;
	u32	dma_fn_mod;
	u32	_pad_0x4510c_0x46040[973];
	/* FPGA2HPS */
	u32	fpga2hps_wr_tidemark;		/* 0x46040 */
	u32	_pad_0x46044_0x46100[47];
	u32	fpga2hps_read_qos;		/* 0x46100 */
	u32	fpga2hps_write_qos;
	u32	fpga2hps_fn_mod;
	u32	_pad_0x4610c_0x47100[1021];
	/* ETR */
	u32	etr_read_qos;			/* 0x47100 */
	u32	etr_write_qos;
	u32	etr_fn_mod;
	u32	_pad_0x4710c_0x48100[1021];
	/* EMAC0 */
	u32	emac0_read_qos;			/* 0x48100 */
	u32	emac0_write_qos;
	u32	emac0_fn_mod;
	u32	_pad_0x4810c_0x49100[1021];
	/* EMAC1 */
	u32	emac1_read_qos;			/* 0x49100 */
	u32	emac1_write_qos;
	u32	emac1_fn_mod;
	u32	_pad_0x4910c_0x4a028[967];
	/* USB0 */
	u32	usb0_fn_mod_ahb;
	u32	_pad_0x4a02c_0x4a100[53];
	u32	usb0_read_qos;			/* 0x4A100 */
	u32	usb0_write_qos;
	u32	usb0_fn_mod;
	u32	_pad_0x4a10c_0x4b100[1021];
	/* NAND */
	u32	nand_read_qos;			/* 0x4B100 */
	u32	nand_write_qos;
	u32	nand_fn_mod;
	u32	_pad_0x4b10c_0x4c028[967];
	/* USB1 */
	u32	usb1_fn_mod_ahb;
	u32	_pad_0x4c02c_0x4c100[53];
	u32	usb1_read_qos;			/* 0x4C100 */
	u32	usb1_write_qos;
	u32	usb1_fn_mod;
};

#endif	/* _NIC301_REGISTERS_H_ */
