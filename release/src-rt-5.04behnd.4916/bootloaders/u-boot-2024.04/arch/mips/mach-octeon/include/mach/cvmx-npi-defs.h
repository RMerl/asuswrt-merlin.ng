/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon npi.
 */

#ifndef __CVMX_NPI_DEFS_H__
#define __CVMX_NPI_DEFS_H__

#define CVMX_NPI_BASE_ADDR_INPUT0	   CVMX_NPI_BASE_ADDR_INPUTX(0)
#define CVMX_NPI_BASE_ADDR_INPUT1	   CVMX_NPI_BASE_ADDR_INPUTX(1)
#define CVMX_NPI_BASE_ADDR_INPUT2	   CVMX_NPI_BASE_ADDR_INPUTX(2)
#define CVMX_NPI_BASE_ADDR_INPUT3	   CVMX_NPI_BASE_ADDR_INPUTX(3)
#define CVMX_NPI_BASE_ADDR_INPUTX(offset)  (0x00011F0000000070ull + ((offset) & 3) * 16)
#define CVMX_NPI_BASE_ADDR_OUTPUT0	   CVMX_NPI_BASE_ADDR_OUTPUTX(0)
#define CVMX_NPI_BASE_ADDR_OUTPUT1	   CVMX_NPI_BASE_ADDR_OUTPUTX(1)
#define CVMX_NPI_BASE_ADDR_OUTPUT2	   CVMX_NPI_BASE_ADDR_OUTPUTX(2)
#define CVMX_NPI_BASE_ADDR_OUTPUT3	   CVMX_NPI_BASE_ADDR_OUTPUTX(3)
#define CVMX_NPI_BASE_ADDR_OUTPUTX(offset) (0x00011F00000000B8ull + ((offset) & 3) * 8)
#define CVMX_NPI_BIST_STATUS		   (0x00011F00000003F8ull)
#define CVMX_NPI_BUFF_SIZE_OUTPUT0	   CVMX_NPI_BUFF_SIZE_OUTPUTX(0)
#define CVMX_NPI_BUFF_SIZE_OUTPUT1	   CVMX_NPI_BUFF_SIZE_OUTPUTX(1)
#define CVMX_NPI_BUFF_SIZE_OUTPUT2	   CVMX_NPI_BUFF_SIZE_OUTPUTX(2)
#define CVMX_NPI_BUFF_SIZE_OUTPUT3	   CVMX_NPI_BUFF_SIZE_OUTPUTX(3)
#define CVMX_NPI_BUFF_SIZE_OUTPUTX(offset) (0x00011F00000000E0ull + ((offset) & 3) * 8)
#define CVMX_NPI_COMP_CTL		   (0x00011F0000000218ull)
#define CVMX_NPI_CTL_STATUS		   (0x00011F0000000010ull)
#define CVMX_NPI_DBG_SELECT		   (0x00011F0000000008ull)
#define CVMX_NPI_DMA_CONTROL		   (0x00011F0000000128ull)
#define CVMX_NPI_DMA_HIGHP_COUNTS	   (0x00011F0000000148ull)
#define CVMX_NPI_DMA_HIGHP_NADDR	   (0x00011F0000000158ull)
#define CVMX_NPI_DMA_LOWP_COUNTS	   (0x00011F0000000140ull)
#define CVMX_NPI_DMA_LOWP_NADDR		   (0x00011F0000000150ull)
#define CVMX_NPI_HIGHP_DBELL		   (0x00011F0000000120ull)
#define CVMX_NPI_HIGHP_IBUFF_SADDR	   (0x00011F0000000110ull)
#define CVMX_NPI_INPUT_CONTROL		   (0x00011F0000000138ull)
#define CVMX_NPI_INT_ENB		   (0x00011F0000000020ull)
#define CVMX_NPI_INT_SUM		   (0x00011F0000000018ull)
#define CVMX_NPI_LOWP_DBELL		   (0x00011F0000000118ull)
#define CVMX_NPI_LOWP_IBUFF_SADDR	   (0x00011F0000000108ull)
#define CVMX_NPI_MEM_ACCESS_SUBID3	   CVMX_NPI_MEM_ACCESS_SUBIDX(3)
#define CVMX_NPI_MEM_ACCESS_SUBID4	   CVMX_NPI_MEM_ACCESS_SUBIDX(4)
#define CVMX_NPI_MEM_ACCESS_SUBID5	   CVMX_NPI_MEM_ACCESS_SUBIDX(5)
#define CVMX_NPI_MEM_ACCESS_SUBID6	   CVMX_NPI_MEM_ACCESS_SUBIDX(6)
#define CVMX_NPI_MEM_ACCESS_SUBIDX(offset) (0x00011F0000000028ull + ((offset) & 7) * 8 - 8 * 3)
#define CVMX_NPI_MSI_RCV		   (0x0000000000000190ull)
#define CVMX_NPI_NPI_MSI_RCV		   (0x00011F0000001190ull)
#define CVMX_NPI_NUM_DESC_OUTPUT0	   CVMX_NPI_NUM_DESC_OUTPUTX(0)
#define CVMX_NPI_NUM_DESC_OUTPUT1	   CVMX_NPI_NUM_DESC_OUTPUTX(1)
#define CVMX_NPI_NUM_DESC_OUTPUT2	   CVMX_NPI_NUM_DESC_OUTPUTX(2)
#define CVMX_NPI_NUM_DESC_OUTPUT3	   CVMX_NPI_NUM_DESC_OUTPUTX(3)
#define CVMX_NPI_NUM_DESC_OUTPUTX(offset)  (0x00011F0000000050ull + ((offset) & 3) * 8)
#define CVMX_NPI_OUTPUT_CONTROL		   (0x00011F0000000100ull)
#define CVMX_NPI_P0_DBPAIR_ADDR		   CVMX_NPI_PX_DBPAIR_ADDR(0)
#define CVMX_NPI_P0_INSTR_ADDR		   CVMX_NPI_PX_INSTR_ADDR(0)
#define CVMX_NPI_P0_INSTR_CNTS		   CVMX_NPI_PX_INSTR_CNTS(0)
#define CVMX_NPI_P0_PAIR_CNTS		   CVMX_NPI_PX_PAIR_CNTS(0)
#define CVMX_NPI_P1_DBPAIR_ADDR		   CVMX_NPI_PX_DBPAIR_ADDR(1)
#define CVMX_NPI_P1_INSTR_ADDR		   CVMX_NPI_PX_INSTR_ADDR(1)
#define CVMX_NPI_P1_INSTR_CNTS		   CVMX_NPI_PX_INSTR_CNTS(1)
#define CVMX_NPI_P1_PAIR_CNTS		   CVMX_NPI_PX_PAIR_CNTS(1)
#define CVMX_NPI_P2_DBPAIR_ADDR		   CVMX_NPI_PX_DBPAIR_ADDR(2)
#define CVMX_NPI_P2_INSTR_ADDR		   CVMX_NPI_PX_INSTR_ADDR(2)
#define CVMX_NPI_P2_INSTR_CNTS		   CVMX_NPI_PX_INSTR_CNTS(2)
#define CVMX_NPI_P2_PAIR_CNTS		   CVMX_NPI_PX_PAIR_CNTS(2)
#define CVMX_NPI_P3_DBPAIR_ADDR		   CVMX_NPI_PX_DBPAIR_ADDR(3)
#define CVMX_NPI_P3_INSTR_ADDR		   CVMX_NPI_PX_INSTR_ADDR(3)
#define CVMX_NPI_P3_INSTR_CNTS		   CVMX_NPI_PX_INSTR_CNTS(3)
#define CVMX_NPI_P3_PAIR_CNTS		   CVMX_NPI_PX_PAIR_CNTS(3)
#define CVMX_NPI_PCI_BAR1_INDEXX(offset)   (0x00011F0000001100ull + ((offset) & 31) * 4)
#define CVMX_NPI_PCI_BIST_REG		   (0x00011F00000011C0ull)
#define CVMX_NPI_PCI_BURST_SIZE		   (0x00011F00000000D8ull)
#define CVMX_NPI_PCI_CFG00		   (0x00011F0000001800ull)
#define CVMX_NPI_PCI_CFG01		   (0x00011F0000001804ull)
#define CVMX_NPI_PCI_CFG02		   (0x00011F0000001808ull)
#define CVMX_NPI_PCI_CFG03		   (0x00011F000000180Cull)
#define CVMX_NPI_PCI_CFG04		   (0x00011F0000001810ull)
#define CVMX_NPI_PCI_CFG05		   (0x00011F0000001814ull)
#define CVMX_NPI_PCI_CFG06		   (0x00011F0000001818ull)
#define CVMX_NPI_PCI_CFG07		   (0x00011F000000181Cull)
#define CVMX_NPI_PCI_CFG08		   (0x00011F0000001820ull)
#define CVMX_NPI_PCI_CFG09		   (0x00011F0000001824ull)
#define CVMX_NPI_PCI_CFG10		   (0x00011F0000001828ull)
#define CVMX_NPI_PCI_CFG11		   (0x00011F000000182Cull)
#define CVMX_NPI_PCI_CFG12		   (0x00011F0000001830ull)
#define CVMX_NPI_PCI_CFG13		   (0x00011F0000001834ull)
#define CVMX_NPI_PCI_CFG15		   (0x00011F000000183Cull)
#define CVMX_NPI_PCI_CFG16		   (0x00011F0000001840ull)
#define CVMX_NPI_PCI_CFG17		   (0x00011F0000001844ull)
#define CVMX_NPI_PCI_CFG18		   (0x00011F0000001848ull)
#define CVMX_NPI_PCI_CFG19		   (0x00011F000000184Cull)
#define CVMX_NPI_PCI_CFG20		   (0x00011F0000001850ull)
#define CVMX_NPI_PCI_CFG21		   (0x00011F0000001854ull)
#define CVMX_NPI_PCI_CFG22		   (0x00011F0000001858ull)
#define CVMX_NPI_PCI_CFG56		   (0x00011F00000018E0ull)
#define CVMX_NPI_PCI_CFG57		   (0x00011F00000018E4ull)
#define CVMX_NPI_PCI_CFG58		   (0x00011F00000018E8ull)
#define CVMX_NPI_PCI_CFG59		   (0x00011F00000018ECull)
#define CVMX_NPI_PCI_CFG60		   (0x00011F00000018F0ull)
#define CVMX_NPI_PCI_CFG61		   (0x00011F00000018F4ull)
#define CVMX_NPI_PCI_CFG62		   (0x00011F00000018F8ull)
#define CVMX_NPI_PCI_CFG63		   (0x00011F00000018FCull)
#define CVMX_NPI_PCI_CNT_REG		   (0x00011F00000011B8ull)
#define CVMX_NPI_PCI_CTL_STATUS_2	   (0x00011F000000118Cull)
#define CVMX_NPI_PCI_INT_ARB_CFG	   (0x00011F0000000130ull)
#define CVMX_NPI_PCI_INT_ENB2		   (0x00011F00000011A0ull)
#define CVMX_NPI_PCI_INT_SUM2		   (0x00011F0000001198ull)
#define CVMX_NPI_PCI_READ_CMD		   (0x00011F0000000048ull)
#define CVMX_NPI_PCI_READ_CMD_6		   (0x00011F0000001180ull)
#define CVMX_NPI_PCI_READ_CMD_C		   (0x00011F0000001184ull)
#define CVMX_NPI_PCI_READ_CMD_E		   (0x00011F0000001188ull)
#define CVMX_NPI_PCI_SCM_REG		   (0x00011F00000011A8ull)
#define CVMX_NPI_PCI_TSR_REG		   (0x00011F00000011B0ull)
#define CVMX_NPI_PORT32_INSTR_HDR	   (0x00011F00000001F8ull)
#define CVMX_NPI_PORT33_INSTR_HDR	   (0x00011F0000000200ull)
#define CVMX_NPI_PORT34_INSTR_HDR	   (0x00011F0000000208ull)
#define CVMX_NPI_PORT35_INSTR_HDR	   (0x00011F0000000210ull)
#define CVMX_NPI_PORT_BP_CONTROL	   (0x00011F00000001F0ull)
#define CVMX_NPI_PX_DBPAIR_ADDR(offset)	   (0x00011F0000000180ull + ((offset) & 3) * 8)
#define CVMX_NPI_PX_INSTR_ADDR(offset)	   (0x00011F00000001C0ull + ((offset) & 3) * 8)
#define CVMX_NPI_PX_INSTR_CNTS(offset)	   (0x00011F00000001A0ull + ((offset) & 3) * 8)
#define CVMX_NPI_PX_PAIR_CNTS(offset)	   (0x00011F0000000160ull + ((offset) & 3) * 8)
#define CVMX_NPI_RSL_INT_BLOCKS		   (0x00011F0000000000ull)
#define CVMX_NPI_SIZE_INPUT0		   CVMX_NPI_SIZE_INPUTX(0)
#define CVMX_NPI_SIZE_INPUT1		   CVMX_NPI_SIZE_INPUTX(1)
#define CVMX_NPI_SIZE_INPUT2		   CVMX_NPI_SIZE_INPUTX(2)
#define CVMX_NPI_SIZE_INPUT3		   CVMX_NPI_SIZE_INPUTX(3)
#define CVMX_NPI_SIZE_INPUTX(offset)	   (0x00011F0000000078ull + ((offset) & 3) * 16)
#define CVMX_NPI_WIN_READ_TO		   (0x00011F00000001E0ull)

/**
 * cvmx_npi_base_addr_input#
 *
 * NPI_BASE_ADDR_INPUT0 = NPI's Base Address Input 0 Register
 *
 * The address to start reading Instructions from for Input-0.
 */
union cvmx_npi_base_addr_inputx {
	u64 u64;
	struct cvmx_npi_base_addr_inputx_s {
		u64 baddr : 61;
		u64 reserved_0_2 : 3;
	} s;
	struct cvmx_npi_base_addr_inputx_s cn30xx;
	struct cvmx_npi_base_addr_inputx_s cn31xx;
	struct cvmx_npi_base_addr_inputx_s cn38xx;
	struct cvmx_npi_base_addr_inputx_s cn38xxp2;
	struct cvmx_npi_base_addr_inputx_s cn50xx;
	struct cvmx_npi_base_addr_inputx_s cn58xx;
	struct cvmx_npi_base_addr_inputx_s cn58xxp1;
};

typedef union cvmx_npi_base_addr_inputx cvmx_npi_base_addr_inputx_t;

/**
 * cvmx_npi_base_addr_output#
 *
 * NPI_BASE_ADDR_OUTPUT0 = NPI's Base Address Output 0 Register
 *
 * The address to start reading Instructions from for Output-0.
 */
union cvmx_npi_base_addr_outputx {
	u64 u64;
	struct cvmx_npi_base_addr_outputx_s {
		u64 baddr : 61;
		u64 reserved_0_2 : 3;
	} s;
	struct cvmx_npi_base_addr_outputx_s cn30xx;
	struct cvmx_npi_base_addr_outputx_s cn31xx;
	struct cvmx_npi_base_addr_outputx_s cn38xx;
	struct cvmx_npi_base_addr_outputx_s cn38xxp2;
	struct cvmx_npi_base_addr_outputx_s cn50xx;
	struct cvmx_npi_base_addr_outputx_s cn58xx;
	struct cvmx_npi_base_addr_outputx_s cn58xxp1;
};

typedef union cvmx_npi_base_addr_outputx cvmx_npi_base_addr_outputx_t;

/**
 * cvmx_npi_bist_status
 *
 * NPI_BIST_STATUS = NPI's BIST Status Register
 *
 * Results from BIST runs of NPI's memories.
 */
union cvmx_npi_bist_status {
	u64 u64;
	struct cvmx_npi_bist_status_s {
		u64 reserved_20_63 : 44;
		u64 csr_bs : 1;
		u64 dif_bs : 1;
		u64 rdp_bs : 1;
		u64 pcnc_bs : 1;
		u64 pcn_bs : 1;
		u64 rdn_bs : 1;
		u64 pcac_bs : 1;
		u64 pcad_bs : 1;
		u64 rdnl_bs : 1;
		u64 pgf_bs : 1;
		u64 pig_bs : 1;
		u64 pof0_bs : 1;
		u64 pof1_bs : 1;
		u64 pof2_bs : 1;
		u64 pof3_bs : 1;
		u64 pos_bs : 1;
		u64 nus_bs : 1;
		u64 dob_bs : 1;
		u64 pdf_bs : 1;
		u64 dpi_bs : 1;
	} s;
	struct cvmx_npi_bist_status_cn30xx {
		u64 reserved_20_63 : 44;
		u64 csr_bs : 1;
		u64 dif_bs : 1;
		u64 rdp_bs : 1;
		u64 pcnc_bs : 1;
		u64 pcn_bs : 1;
		u64 rdn_bs : 1;
		u64 pcac_bs : 1;
		u64 pcad_bs : 1;
		u64 rdnl_bs : 1;
		u64 pgf_bs : 1;
		u64 pig_bs : 1;
		u64 pof0_bs : 1;
		u64 reserved_5_7 : 3;
		u64 pos_bs : 1;
		u64 nus_bs : 1;
		u64 dob_bs : 1;
		u64 pdf_bs : 1;
		u64 dpi_bs : 1;
	} cn30xx;
	struct cvmx_npi_bist_status_s cn31xx;
	struct cvmx_npi_bist_status_s cn38xx;
	struct cvmx_npi_bist_status_s cn38xxp2;
	struct cvmx_npi_bist_status_cn50xx {
		u64 reserved_20_63 : 44;
		u64 csr_bs : 1;
		u64 dif_bs : 1;
		u64 rdp_bs : 1;
		u64 pcnc_bs : 1;
		u64 pcn_bs : 1;
		u64 rdn_bs : 1;
		u64 pcac_bs : 1;
		u64 pcad_bs : 1;
		u64 rdnl_bs : 1;
		u64 pgf_bs : 1;
		u64 pig_bs : 1;
		u64 pof0_bs : 1;
		u64 pof1_bs : 1;
		u64 reserved_5_6 : 2;
		u64 pos_bs : 1;
		u64 nus_bs : 1;
		u64 dob_bs : 1;
		u64 pdf_bs : 1;
		u64 dpi_bs : 1;
	} cn50xx;
	struct cvmx_npi_bist_status_s cn58xx;
	struct cvmx_npi_bist_status_s cn58xxp1;
};

typedef union cvmx_npi_bist_status cvmx_npi_bist_status_t;

/**
 * cvmx_npi_buff_size_output#
 *
 * NPI_BUFF_SIZE_OUTPUT0 = NPI's D/I Buffer Sizes For Output 0
 *
 * The size in bytes of the Data Bufffer and Information Buffer for output 0.
 */
union cvmx_npi_buff_size_outputx {
	u64 u64;
	struct cvmx_npi_buff_size_outputx_s {
		u64 reserved_23_63 : 41;
		u64 isize : 7;
		u64 bsize : 16;
	} s;
	struct cvmx_npi_buff_size_outputx_s cn30xx;
	struct cvmx_npi_buff_size_outputx_s cn31xx;
	struct cvmx_npi_buff_size_outputx_s cn38xx;
	struct cvmx_npi_buff_size_outputx_s cn38xxp2;
	struct cvmx_npi_buff_size_outputx_s cn50xx;
	struct cvmx_npi_buff_size_outputx_s cn58xx;
	struct cvmx_npi_buff_size_outputx_s cn58xxp1;
};

typedef union cvmx_npi_buff_size_outputx cvmx_npi_buff_size_outputx_t;

/**
 * cvmx_npi_comp_ctl
 *
 * NPI_COMP_CTL = PCI Compensation Control
 *
 * PCI Compensation Control
 */
union cvmx_npi_comp_ctl {
	u64 u64;
	struct cvmx_npi_comp_ctl_s {
		u64 reserved_10_63 : 54;
		u64 pctl : 5;
		u64 nctl : 5;
	} s;
	struct cvmx_npi_comp_ctl_s cn50xx;
	struct cvmx_npi_comp_ctl_s cn58xx;
	struct cvmx_npi_comp_ctl_s cn58xxp1;
};

typedef union cvmx_npi_comp_ctl cvmx_npi_comp_ctl_t;

/**
 * cvmx_npi_ctl_status
 *
 * NPI_CTL_STATUS = NPI's Control Status Register
 *
 * Contains control ans status for NPI.
 * Writes to this register are not ordered with writes/reads to the PCI Memory space.
 * To ensure that a write has completed the user must read the register before
 * making an access(i.e. PCI memory space) that requires the value of this register to be updated.
 */
union cvmx_npi_ctl_status {
	u64 u64;
	struct cvmx_npi_ctl_status_s {
		u64 reserved_63_63 : 1;
		u64 chip_rev : 8;
		u64 dis_pniw : 1;
		u64 out3_enb : 1;
		u64 out2_enb : 1;
		u64 out1_enb : 1;
		u64 out0_enb : 1;
		u64 ins3_enb : 1;
		u64 ins2_enb : 1;
		u64 ins1_enb : 1;
		u64 ins0_enb : 1;
		u64 ins3_64b : 1;
		u64 ins2_64b : 1;
		u64 ins1_64b : 1;
		u64 ins0_64b : 1;
		u64 pci_wdis : 1;
		u64 wait_com : 1;
		u64 reserved_37_39 : 3;
		u64 max_word : 5;
		u64 reserved_10_31 : 22;
		u64 timer : 10;
	} s;
	struct cvmx_npi_ctl_status_cn30xx {
		u64 reserved_63_63 : 1;
		u64 chip_rev : 8;
		u64 dis_pniw : 1;
		u64 reserved_51_53 : 3;
		u64 out0_enb : 1;
		u64 reserved_47_49 : 3;
		u64 ins0_enb : 1;
		u64 reserved_43_45 : 3;
		u64 ins0_64b : 1;
		u64 pci_wdis : 1;
		u64 wait_com : 1;
		u64 reserved_37_39 : 3;
		u64 max_word : 5;
		u64 reserved_10_31 : 22;
		u64 timer : 10;
	} cn30xx;
	struct cvmx_npi_ctl_status_cn31xx {
		u64 reserved_63_63 : 1;
		u64 chip_rev : 8;
		u64 dis_pniw : 1;
		u64 reserved_52_53 : 2;
		u64 out1_enb : 1;
		u64 out0_enb : 1;
		u64 reserved_48_49 : 2;
		u64 ins1_enb : 1;
		u64 ins0_enb : 1;
		u64 reserved_44_45 : 2;
		u64 ins1_64b : 1;
		u64 ins0_64b : 1;
		u64 pci_wdis : 1;
		u64 wait_com : 1;
		u64 reserved_37_39 : 3;
		u64 max_word : 5;
		u64 reserved_10_31 : 22;
		u64 timer : 10;
	} cn31xx;
	struct cvmx_npi_ctl_status_s cn38xx;
	struct cvmx_npi_ctl_status_s cn38xxp2;
	struct cvmx_npi_ctl_status_cn31xx cn50xx;
	struct cvmx_npi_ctl_status_s cn58xx;
	struct cvmx_npi_ctl_status_s cn58xxp1;
};

typedef union cvmx_npi_ctl_status cvmx_npi_ctl_status_t;

/**
 * cvmx_npi_dbg_select
 *
 * NPI_DBG_SELECT = Debug Select Register
 *
 * Contains the debug select value in last written to the RSLs.
 */
union cvmx_npi_dbg_select {
	u64 u64;
	struct cvmx_npi_dbg_select_s {
		u64 reserved_16_63 : 48;
		u64 dbg_sel : 16;
	} s;
	struct cvmx_npi_dbg_select_s cn30xx;
	struct cvmx_npi_dbg_select_s cn31xx;
	struct cvmx_npi_dbg_select_s cn38xx;
	struct cvmx_npi_dbg_select_s cn38xxp2;
	struct cvmx_npi_dbg_select_s cn50xx;
	struct cvmx_npi_dbg_select_s cn58xx;
	struct cvmx_npi_dbg_select_s cn58xxp1;
};

typedef union cvmx_npi_dbg_select cvmx_npi_dbg_select_t;

/**
 * cvmx_npi_dma_control
 *
 * NPI_DMA_CONTROL = DMA Control Register
 *
 * Controls operation of the DMA IN/OUT of the NPI.
 */
union cvmx_npi_dma_control {
	u64 u64;
	struct cvmx_npi_dma_control_s {
		u64 reserved_36_63 : 28;
		u64 b0_lend : 1;
		u64 dwb_denb : 1;
		u64 dwb_ichk : 9;
		u64 fpa_que : 3;
		u64 o_add1 : 1;
		u64 o_ro : 1;
		u64 o_ns : 1;
		u64 o_es : 2;
		u64 o_mode : 1;
		u64 hp_enb : 1;
		u64 lp_enb : 1;
		u64 csize : 14;
	} s;
	struct cvmx_npi_dma_control_s cn30xx;
	struct cvmx_npi_dma_control_s cn31xx;
	struct cvmx_npi_dma_control_s cn38xx;
	struct cvmx_npi_dma_control_s cn38xxp2;
	struct cvmx_npi_dma_control_s cn50xx;
	struct cvmx_npi_dma_control_s cn58xx;
	struct cvmx_npi_dma_control_s cn58xxp1;
};

typedef union cvmx_npi_dma_control cvmx_npi_dma_control_t;

/**
 * cvmx_npi_dma_highp_counts
 *
 * NPI_DMA_HIGHP_COUNTS = NPI's High Priority DMA Counts
 *
 * Values for determing the number of instructions for High Priority DMA in the NPI.
 */
union cvmx_npi_dma_highp_counts {
	u64 u64;
	struct cvmx_npi_dma_highp_counts_s {
		u64 reserved_39_63 : 25;
		u64 fcnt : 7;
		u64 dbell : 32;
	} s;
	struct cvmx_npi_dma_highp_counts_s cn30xx;
	struct cvmx_npi_dma_highp_counts_s cn31xx;
	struct cvmx_npi_dma_highp_counts_s cn38xx;
	struct cvmx_npi_dma_highp_counts_s cn38xxp2;
	struct cvmx_npi_dma_highp_counts_s cn50xx;
	struct cvmx_npi_dma_highp_counts_s cn58xx;
	struct cvmx_npi_dma_highp_counts_s cn58xxp1;
};

typedef union cvmx_npi_dma_highp_counts cvmx_npi_dma_highp_counts_t;

/**
 * cvmx_npi_dma_highp_naddr
 *
 * NPI_DMA_HIGHP_NADDR = NPI's High Priority DMA Next Ichunk Address
 *
 * Place NPI will read the next Ichunk data from. This is valid when state is 0
 */
union cvmx_npi_dma_highp_naddr {
	u64 u64;
	struct cvmx_npi_dma_highp_naddr_s {
		u64 reserved_40_63 : 24;
		u64 state : 4;
		u64 addr : 36;
	} s;
	struct cvmx_npi_dma_highp_naddr_s cn30xx;
	struct cvmx_npi_dma_highp_naddr_s cn31xx;
	struct cvmx_npi_dma_highp_naddr_s cn38xx;
	struct cvmx_npi_dma_highp_naddr_s cn38xxp2;
	struct cvmx_npi_dma_highp_naddr_s cn50xx;
	struct cvmx_npi_dma_highp_naddr_s cn58xx;
	struct cvmx_npi_dma_highp_naddr_s cn58xxp1;
};

typedef union cvmx_npi_dma_highp_naddr cvmx_npi_dma_highp_naddr_t;

/**
 * cvmx_npi_dma_lowp_counts
 *
 * NPI_DMA_LOWP_COUNTS = NPI's Low Priority DMA Counts
 *
 * Values for determing the number of instructions for Low Priority DMA in the NPI.
 */
union cvmx_npi_dma_lowp_counts {
	u64 u64;
	struct cvmx_npi_dma_lowp_counts_s {
		u64 reserved_39_63 : 25;
		u64 fcnt : 7;
		u64 dbell : 32;
	} s;
	struct cvmx_npi_dma_lowp_counts_s cn30xx;
	struct cvmx_npi_dma_lowp_counts_s cn31xx;
	struct cvmx_npi_dma_lowp_counts_s cn38xx;
	struct cvmx_npi_dma_lowp_counts_s cn38xxp2;
	struct cvmx_npi_dma_lowp_counts_s cn50xx;
	struct cvmx_npi_dma_lowp_counts_s cn58xx;
	struct cvmx_npi_dma_lowp_counts_s cn58xxp1;
};

typedef union cvmx_npi_dma_lowp_counts cvmx_npi_dma_lowp_counts_t;

/**
 * cvmx_npi_dma_lowp_naddr
 *
 * NPI_DMA_LOWP_NADDR = NPI's Low Priority DMA Next Ichunk Address
 *
 * Place NPI will read the next Ichunk data from. This is valid when state is 0
 */
union cvmx_npi_dma_lowp_naddr {
	u64 u64;
	struct cvmx_npi_dma_lowp_naddr_s {
		u64 reserved_40_63 : 24;
		u64 state : 4;
		u64 addr : 36;
	} s;
	struct cvmx_npi_dma_lowp_naddr_s cn30xx;
	struct cvmx_npi_dma_lowp_naddr_s cn31xx;
	struct cvmx_npi_dma_lowp_naddr_s cn38xx;
	struct cvmx_npi_dma_lowp_naddr_s cn38xxp2;
	struct cvmx_npi_dma_lowp_naddr_s cn50xx;
	struct cvmx_npi_dma_lowp_naddr_s cn58xx;
	struct cvmx_npi_dma_lowp_naddr_s cn58xxp1;
};

typedef union cvmx_npi_dma_lowp_naddr cvmx_npi_dma_lowp_naddr_t;

/**
 * cvmx_npi_highp_dbell
 *
 * NPI_HIGHP_DBELL = High Priority Door Bell
 *
 * The door bell register for the high priority DMA queue.
 */
union cvmx_npi_highp_dbell {
	u64 u64;
	struct cvmx_npi_highp_dbell_s {
		u64 reserved_16_63 : 48;
		u64 dbell : 16;
	} s;
	struct cvmx_npi_highp_dbell_s cn30xx;
	struct cvmx_npi_highp_dbell_s cn31xx;
	struct cvmx_npi_highp_dbell_s cn38xx;
	struct cvmx_npi_highp_dbell_s cn38xxp2;
	struct cvmx_npi_highp_dbell_s cn50xx;
	struct cvmx_npi_highp_dbell_s cn58xx;
	struct cvmx_npi_highp_dbell_s cn58xxp1;
};

typedef union cvmx_npi_highp_dbell cvmx_npi_highp_dbell_t;

/**
 * cvmx_npi_highp_ibuff_saddr
 *
 * NPI_HIGHP_IBUFF_SADDR = DMA High Priority Instruction Buffer Starting Address
 *
 * The address to start reading Instructions from for HIGHP.
 */
union cvmx_npi_highp_ibuff_saddr {
	u64 u64;
	struct cvmx_npi_highp_ibuff_saddr_s {
		u64 reserved_36_63 : 28;
		u64 saddr : 36;
	} s;
	struct cvmx_npi_highp_ibuff_saddr_s cn30xx;
	struct cvmx_npi_highp_ibuff_saddr_s cn31xx;
	struct cvmx_npi_highp_ibuff_saddr_s cn38xx;
	struct cvmx_npi_highp_ibuff_saddr_s cn38xxp2;
	struct cvmx_npi_highp_ibuff_saddr_s cn50xx;
	struct cvmx_npi_highp_ibuff_saddr_s cn58xx;
	struct cvmx_npi_highp_ibuff_saddr_s cn58xxp1;
};

typedef union cvmx_npi_highp_ibuff_saddr cvmx_npi_highp_ibuff_saddr_t;

/**
 * cvmx_npi_input_control
 *
 * NPI_INPUT_CONTROL = NPI's Input Control Register
 *
 * Control for reads for gather list and instructions.
 */
union cvmx_npi_input_control {
	u64 u64;
	struct cvmx_npi_input_control_s {
		u64 reserved_23_63 : 41;
		u64 pkt_rr : 1;
		u64 pbp_dhi : 13;
		u64 d_nsr : 1;
		u64 d_esr : 2;
		u64 d_ror : 1;
		u64 use_csr : 1;
		u64 nsr : 1;
		u64 esr : 2;
		u64 ror : 1;
	} s;
	struct cvmx_npi_input_control_cn30xx {
		u64 reserved_22_63 : 42;
		u64 pbp_dhi : 13;
		u64 d_nsr : 1;
		u64 d_esr : 2;
		u64 d_ror : 1;
		u64 use_csr : 1;
		u64 nsr : 1;
		u64 esr : 2;
		u64 ror : 1;
	} cn30xx;
	struct cvmx_npi_input_control_cn30xx cn31xx;
	struct cvmx_npi_input_control_s cn38xx;
	struct cvmx_npi_input_control_cn30xx cn38xxp2;
	struct cvmx_npi_input_control_s cn50xx;
	struct cvmx_npi_input_control_s cn58xx;
	struct cvmx_npi_input_control_s cn58xxp1;
};

typedef union cvmx_npi_input_control cvmx_npi_input_control_t;

/**
 * cvmx_npi_int_enb
 *
 * NPI_INTERRUPT_ENB = NPI's Interrupt Enable Register
 *
 * Used to enable the various interrupting conditions of NPI
 */
union cvmx_npi_int_enb {
	u64 u64;
	struct cvmx_npi_int_enb_s {
		u64 reserved_62_63 : 2;
		u64 q1_a_f : 1;
		u64 q1_s_e : 1;
		u64 pdf_p_f : 1;
		u64 pdf_p_e : 1;
		u64 pcf_p_f : 1;
		u64 pcf_p_e : 1;
		u64 rdx_s_e : 1;
		u64 rwx_s_e : 1;
		u64 pnc_a_f : 1;
		u64 pnc_s_e : 1;
		u64 com_a_f : 1;
		u64 com_s_e : 1;
		u64 q3_a_f : 1;
		u64 q3_s_e : 1;
		u64 q2_a_f : 1;
		u64 q2_s_e : 1;
		u64 pcr_a_f : 1;
		u64 pcr_s_e : 1;
		u64 fcr_a_f : 1;
		u64 fcr_s_e : 1;
		u64 iobdma : 1;
		u64 p_dperr : 1;
		u64 win_rto : 1;
		u64 i3_pperr : 1;
		u64 i2_pperr : 1;
		u64 i1_pperr : 1;
		u64 i0_pperr : 1;
		u64 p3_ptout : 1;
		u64 p2_ptout : 1;
		u64 p1_ptout : 1;
		u64 p0_ptout : 1;
		u64 p3_pperr : 1;
		u64 p2_pperr : 1;
		u64 p1_pperr : 1;
		u64 p0_pperr : 1;
		u64 g3_rtout : 1;
		u64 g2_rtout : 1;
		u64 g1_rtout : 1;
		u64 g0_rtout : 1;
		u64 p3_perr : 1;
		u64 p2_perr : 1;
		u64 p1_perr : 1;
		u64 p0_perr : 1;
		u64 p3_rtout : 1;
		u64 p2_rtout : 1;
		u64 p1_rtout : 1;
		u64 p0_rtout : 1;
		u64 i3_overf : 1;
		u64 i2_overf : 1;
		u64 i1_overf : 1;
		u64 i0_overf : 1;
		u64 i3_rtout : 1;
		u64 i2_rtout : 1;
		u64 i1_rtout : 1;
		u64 i0_rtout : 1;
		u64 po3_2sml : 1;
		u64 po2_2sml : 1;
		u64 po1_2sml : 1;
		u64 po0_2sml : 1;
		u64 pci_rsl : 1;
		u64 rml_wto : 1;
		u64 rml_rto : 1;
	} s;
	struct cvmx_npi_int_enb_cn30xx {
		u64 reserved_62_63 : 2;
		u64 q1_a_f : 1;
		u64 q1_s_e : 1;
		u64 pdf_p_f : 1;
		u64 pdf_p_e : 1;
		u64 pcf_p_f : 1;
		u64 pcf_p_e : 1;
		u64 rdx_s_e : 1;
		u64 rwx_s_e : 1;
		u64 pnc_a_f : 1;
		u64 pnc_s_e : 1;
		u64 com_a_f : 1;
		u64 com_s_e : 1;
		u64 q3_a_f : 1;
		u64 q3_s_e : 1;
		u64 q2_a_f : 1;
		u64 q2_s_e : 1;
		u64 pcr_a_f : 1;
		u64 pcr_s_e : 1;
		u64 fcr_a_f : 1;
		u64 fcr_s_e : 1;
		u64 iobdma : 1;
		u64 p_dperr : 1;
		u64 win_rto : 1;
		u64 reserved_36_38 : 3;
		u64 i0_pperr : 1;
		u64 reserved_32_34 : 3;
		u64 p0_ptout : 1;
		u64 reserved_28_30 : 3;
		u64 p0_pperr : 1;
		u64 reserved_24_26 : 3;
		u64 g0_rtout : 1;
		u64 reserved_20_22 : 3;
		u64 p0_perr : 1;
		u64 reserved_16_18 : 3;
		u64 p0_rtout : 1;
		u64 reserved_12_14 : 3;
		u64 i0_overf : 1;
		u64 reserved_8_10 : 3;
		u64 i0_rtout : 1;
		u64 reserved_4_6 : 3;
		u64 po0_2sml : 1;
		u64 pci_rsl : 1;
		u64 rml_wto : 1;
		u64 rml_rto : 1;
	} cn30xx;
	struct cvmx_npi_int_enb_cn31xx {
		u64 reserved_62_63 : 2;
		u64 q1_a_f : 1;
		u64 q1_s_e : 1;
		u64 pdf_p_f : 1;
		u64 pdf_p_e : 1;
		u64 pcf_p_f : 1;
		u64 pcf_p_e : 1;
		u64 rdx_s_e : 1;
		u64 rwx_s_e : 1;
		u64 pnc_a_f : 1;
		u64 pnc_s_e : 1;
		u64 com_a_f : 1;
		u64 com_s_e : 1;
		u64 q3_a_f : 1;
		u64 q3_s_e : 1;
		u64 q2_a_f : 1;
		u64 q2_s_e : 1;
		u64 pcr_a_f : 1;
		u64 pcr_s_e : 1;
		u64 fcr_a_f : 1;
		u64 fcr_s_e : 1;
		u64 iobdma : 1;
		u64 p_dperr : 1;
		u64 win_rto : 1;
		u64 reserved_37_38 : 2;
		u64 i1_pperr : 1;
		u64 i0_pperr : 1;
		u64 reserved_33_34 : 2;
		u64 p1_ptout : 1;
		u64 p0_ptout : 1;
		u64 reserved_29_30 : 2;
		u64 p1_pperr : 1;
		u64 p0_pperr : 1;
		u64 reserved_25_26 : 2;
		u64 g1_rtout : 1;
		u64 g0_rtout : 1;
		u64 reserved_21_22 : 2;
		u64 p1_perr : 1;
		u64 p0_perr : 1;
		u64 reserved_17_18 : 2;
		u64 p1_rtout : 1;
		u64 p0_rtout : 1;
		u64 reserved_13_14 : 2;
		u64 i1_overf : 1;
		u64 i0_overf : 1;
		u64 reserved_9_10 : 2;
		u64 i1_rtout : 1;
		u64 i0_rtout : 1;
		u64 reserved_5_6 : 2;
		u64 po1_2sml : 1;
		u64 po0_2sml : 1;
		u64 pci_rsl : 1;
		u64 rml_wto : 1;
		u64 rml_rto : 1;
	} cn31xx;
	struct cvmx_npi_int_enb_s cn38xx;
	struct cvmx_npi_int_enb_cn38xxp2 {
		u64 reserved_42_63 : 22;
		u64 iobdma : 1;
		u64 p_dperr : 1;
		u64 win_rto : 1;
		u64 i3_pperr : 1;
		u64 i2_pperr : 1;
		u64 i1_pperr : 1;
		u64 i0_pperr : 1;
		u64 p3_ptout : 1;
		u64 p2_ptout : 1;
		u64 p1_ptout : 1;
		u64 p0_ptout : 1;
		u64 p3_pperr : 1;
		u64 p2_pperr : 1;
		u64 p1_pperr : 1;
		u64 p0_pperr : 1;
		u64 g3_rtout : 1;
		u64 g2_rtout : 1;
		u64 g1_rtout : 1;
		u64 g0_rtout : 1;
		u64 p3_perr : 1;
		u64 p2_perr : 1;
		u64 p1_perr : 1;
		u64 p0_perr : 1;
		u64 p3_rtout : 1;
		u64 p2_rtout : 1;
		u64 p1_rtout : 1;
		u64 p0_rtout : 1;
		u64 i3_overf : 1;
		u64 i2_overf : 1;
		u64 i1_overf : 1;
		u64 i0_overf : 1;
		u64 i3_rtout : 1;
		u64 i2_rtout : 1;
		u64 i1_rtout : 1;
		u64 i0_rtout : 1;
		u64 po3_2sml : 1;
		u64 po2_2sml : 1;
		u64 po1_2sml : 1;
		u64 po0_2sml : 1;
		u64 pci_rsl : 1;
		u64 rml_wto : 1;
		u64 rml_rto : 1;
	} cn38xxp2;
	struct cvmx_npi_int_enb_cn31xx cn50xx;
	struct cvmx_npi_int_enb_s cn58xx;
	struct cvmx_npi_int_enb_s cn58xxp1;
};

typedef union cvmx_npi_int_enb cvmx_npi_int_enb_t;

/**
 * cvmx_npi_int_sum
 *
 * NPI_INTERRUPT_SUM = NPI Interrupt Summary Register
 *
 * Set when an interrupt condition occurs, write '1' to clear.
 */
union cvmx_npi_int_sum {
	u64 u64;
	struct cvmx_npi_int_sum_s {
		u64 reserved_62_63 : 2;
		u64 q1_a_f : 1;
		u64 q1_s_e : 1;
		u64 pdf_p_f : 1;
		u64 pdf_p_e : 1;
		u64 pcf_p_f : 1;
		u64 pcf_p_e : 1;
		u64 rdx_s_e : 1;
		u64 rwx_s_e : 1;
		u64 pnc_a_f : 1;
		u64 pnc_s_e : 1;
		u64 com_a_f : 1;
		u64 com_s_e : 1;
		u64 q3_a_f : 1;
		u64 q3_s_e : 1;
		u64 q2_a_f : 1;
		u64 q2_s_e : 1;
		u64 pcr_a_f : 1;
		u64 pcr_s_e : 1;
		u64 fcr_a_f : 1;
		u64 fcr_s_e : 1;
		u64 iobdma : 1;
		u64 p_dperr : 1;
		u64 win_rto : 1;
		u64 i3_pperr : 1;
		u64 i2_pperr : 1;
		u64 i1_pperr : 1;
		u64 i0_pperr : 1;
		u64 p3_ptout : 1;
		u64 p2_ptout : 1;
		u64 p1_ptout : 1;
		u64 p0_ptout : 1;
		u64 p3_pperr : 1;
		u64 p2_pperr : 1;
		u64 p1_pperr : 1;
		u64 p0_pperr : 1;
		u64 g3_rtout : 1;
		u64 g2_rtout : 1;
		u64 g1_rtout : 1;
		u64 g0_rtout : 1;
		u64 p3_perr : 1;
		u64 p2_perr : 1;
		u64 p1_perr : 1;
		u64 p0_perr : 1;
		u64 p3_rtout : 1;
		u64 p2_rtout : 1;
		u64 p1_rtout : 1;
		u64 p0_rtout : 1;
		u64 i3_overf : 1;
		u64 i2_overf : 1;
		u64 i1_overf : 1;
		u64 i0_overf : 1;
		u64 i3_rtout : 1;
		u64 i2_rtout : 1;
		u64 i1_rtout : 1;
		u64 i0_rtout : 1;
		u64 po3_2sml : 1;
		u64 po2_2sml : 1;
		u64 po1_2sml : 1;
		u64 po0_2sml : 1;
		u64 pci_rsl : 1;
		u64 rml_wto : 1;
		u64 rml_rto : 1;
	} s;
	struct cvmx_npi_int_sum_cn30xx {
		u64 reserved_62_63 : 2;
		u64 q1_a_f : 1;
		u64 q1_s_e : 1;
		u64 pdf_p_f : 1;
		u64 pdf_p_e : 1;
		u64 pcf_p_f : 1;
		u64 pcf_p_e : 1;
		u64 rdx_s_e : 1;
		u64 rwx_s_e : 1;
		u64 pnc_a_f : 1;
		u64 pnc_s_e : 1;
		u64 com_a_f : 1;
		u64 com_s_e : 1;
		u64 q3_a_f : 1;
		u64 q3_s_e : 1;
		u64 q2_a_f : 1;
		u64 q2_s_e : 1;
		u64 pcr_a_f : 1;
		u64 pcr_s_e : 1;
		u64 fcr_a_f : 1;
		u64 fcr_s_e : 1;
		u64 iobdma : 1;
		u64 p_dperr : 1;
		u64 win_rto : 1;
		u64 reserved_36_38 : 3;
		u64 i0_pperr : 1;
		u64 reserved_32_34 : 3;
		u64 p0_ptout : 1;
		u64 reserved_28_30 : 3;
		u64 p0_pperr : 1;
		u64 reserved_24_26 : 3;
		u64 g0_rtout : 1;
		u64 reserved_20_22 : 3;
		u64 p0_perr : 1;
		u64 reserved_16_18 : 3;
		u64 p0_rtout : 1;
		u64 reserved_12_14 : 3;
		u64 i0_overf : 1;
		u64 reserved_8_10 : 3;
		u64 i0_rtout : 1;
		u64 reserved_4_6 : 3;
		u64 po0_2sml : 1;
		u64 pci_rsl : 1;
		u64 rml_wto : 1;
		u64 rml_rto : 1;
	} cn30xx;
	struct cvmx_npi_int_sum_cn31xx {
		u64 reserved_62_63 : 2;
		u64 q1_a_f : 1;
		u64 q1_s_e : 1;
		u64 pdf_p_f : 1;
		u64 pdf_p_e : 1;
		u64 pcf_p_f : 1;
		u64 pcf_p_e : 1;
		u64 rdx_s_e : 1;
		u64 rwx_s_e : 1;
		u64 pnc_a_f : 1;
		u64 pnc_s_e : 1;
		u64 com_a_f : 1;
		u64 com_s_e : 1;
		u64 q3_a_f : 1;
		u64 q3_s_e : 1;
		u64 q2_a_f : 1;
		u64 q2_s_e : 1;
		u64 pcr_a_f : 1;
		u64 pcr_s_e : 1;
		u64 fcr_a_f : 1;
		u64 fcr_s_e : 1;
		u64 iobdma : 1;
		u64 p_dperr : 1;
		u64 win_rto : 1;
		u64 reserved_37_38 : 2;
		u64 i1_pperr : 1;
		u64 i0_pperr : 1;
		u64 reserved_33_34 : 2;
		u64 p1_ptout : 1;
		u64 p0_ptout : 1;
		u64 reserved_29_30 : 2;
		u64 p1_pperr : 1;
		u64 p0_pperr : 1;
		u64 reserved_25_26 : 2;
		u64 g1_rtout : 1;
		u64 g0_rtout : 1;
		u64 reserved_21_22 : 2;
		u64 p1_perr : 1;
		u64 p0_perr : 1;
		u64 reserved_17_18 : 2;
		u64 p1_rtout : 1;
		u64 p0_rtout : 1;
		u64 reserved_13_14 : 2;
		u64 i1_overf : 1;
		u64 i0_overf : 1;
		u64 reserved_9_10 : 2;
		u64 i1_rtout : 1;
		u64 i0_rtout : 1;
		u64 reserved_5_6 : 2;
		u64 po1_2sml : 1;
		u64 po0_2sml : 1;
		u64 pci_rsl : 1;
		u64 rml_wto : 1;
		u64 rml_rto : 1;
	} cn31xx;
	struct cvmx_npi_int_sum_s cn38xx;
	struct cvmx_npi_int_sum_cn38xxp2 {
		u64 reserved_42_63 : 22;
		u64 iobdma : 1;
		u64 p_dperr : 1;
		u64 win_rto : 1;
		u64 i3_pperr : 1;
		u64 i2_pperr : 1;
		u64 i1_pperr : 1;
		u64 i0_pperr : 1;
		u64 p3_ptout : 1;
		u64 p2_ptout : 1;
		u64 p1_ptout : 1;
		u64 p0_ptout : 1;
		u64 p3_pperr : 1;
		u64 p2_pperr : 1;
		u64 p1_pperr : 1;
		u64 p0_pperr : 1;
		u64 g3_rtout : 1;
		u64 g2_rtout : 1;
		u64 g1_rtout : 1;
		u64 g0_rtout : 1;
		u64 p3_perr : 1;
		u64 p2_perr : 1;
		u64 p1_perr : 1;
		u64 p0_perr : 1;
		u64 p3_rtout : 1;
		u64 p2_rtout : 1;
		u64 p1_rtout : 1;
		u64 p0_rtout : 1;
		u64 i3_overf : 1;
		u64 i2_overf : 1;
		u64 i1_overf : 1;
		u64 i0_overf : 1;
		u64 i3_rtout : 1;
		u64 i2_rtout : 1;
		u64 i1_rtout : 1;
		u64 i0_rtout : 1;
		u64 po3_2sml : 1;
		u64 po2_2sml : 1;
		u64 po1_2sml : 1;
		u64 po0_2sml : 1;
		u64 pci_rsl : 1;
		u64 rml_wto : 1;
		u64 rml_rto : 1;
	} cn38xxp2;
	struct cvmx_npi_int_sum_cn31xx cn50xx;
	struct cvmx_npi_int_sum_s cn58xx;
	struct cvmx_npi_int_sum_s cn58xxp1;
};

typedef union cvmx_npi_int_sum cvmx_npi_int_sum_t;

/**
 * cvmx_npi_lowp_dbell
 *
 * NPI_LOWP_DBELL = Low Priority Door Bell
 *
 * The door bell register for the low priority DMA queue.
 */
union cvmx_npi_lowp_dbell {
	u64 u64;
	struct cvmx_npi_lowp_dbell_s {
		u64 reserved_16_63 : 48;
		u64 dbell : 16;
	} s;
	struct cvmx_npi_lowp_dbell_s cn30xx;
	struct cvmx_npi_lowp_dbell_s cn31xx;
	struct cvmx_npi_lowp_dbell_s cn38xx;
	struct cvmx_npi_lowp_dbell_s cn38xxp2;
	struct cvmx_npi_lowp_dbell_s cn50xx;
	struct cvmx_npi_lowp_dbell_s cn58xx;
	struct cvmx_npi_lowp_dbell_s cn58xxp1;
};

typedef union cvmx_npi_lowp_dbell cvmx_npi_lowp_dbell_t;

/**
 * cvmx_npi_lowp_ibuff_saddr
 *
 * NPI_LOWP_IBUFF_SADDR = DMA Low Priority's Instruction Buffer Starting Address
 *
 * The address to start reading Instructions from for LOWP.
 */
union cvmx_npi_lowp_ibuff_saddr {
	u64 u64;
	struct cvmx_npi_lowp_ibuff_saddr_s {
		u64 reserved_36_63 : 28;
		u64 saddr : 36;
	} s;
	struct cvmx_npi_lowp_ibuff_saddr_s cn30xx;
	struct cvmx_npi_lowp_ibuff_saddr_s cn31xx;
	struct cvmx_npi_lowp_ibuff_saddr_s cn38xx;
	struct cvmx_npi_lowp_ibuff_saddr_s cn38xxp2;
	struct cvmx_npi_lowp_ibuff_saddr_s cn50xx;
	struct cvmx_npi_lowp_ibuff_saddr_s cn58xx;
	struct cvmx_npi_lowp_ibuff_saddr_s cn58xxp1;
};

typedef union cvmx_npi_lowp_ibuff_saddr cvmx_npi_lowp_ibuff_saddr_t;

/**
 * cvmx_npi_mem_access_subid#
 *
 * NPI_MEM_ACCESS_SUBID3 = Memory Access SubId 3Register
 *
 * Carries Read/Write parameters for PP access to PCI memory that use NPI SubId3.
 * Writes to this register are not ordered with writes/reads to the PCI Memory space.
 * To ensure that a write has completed the user must read the register before
 * making an access(i.e. PCI memory space) that requires the value of this register to be updated.
 */
union cvmx_npi_mem_access_subidx {
	u64 u64;
	struct cvmx_npi_mem_access_subidx_s {
		u64 reserved_38_63 : 26;
		u64 shortl : 1;
		u64 nmerge : 1;
		u64 esr : 2;
		u64 esw : 2;
		u64 nsr : 1;
		u64 nsw : 1;
		u64 ror : 1;
		u64 row : 1;
		u64 ba : 28;
	} s;
	struct cvmx_npi_mem_access_subidx_s cn30xx;
	struct cvmx_npi_mem_access_subidx_cn31xx {
		u64 reserved_36_63 : 28;
		u64 esr : 2;
		u64 esw : 2;
		u64 nsr : 1;
		u64 nsw : 1;
		u64 ror : 1;
		u64 row : 1;
		u64 ba : 28;
	} cn31xx;
	struct cvmx_npi_mem_access_subidx_s cn38xx;
	struct cvmx_npi_mem_access_subidx_cn31xx cn38xxp2;
	struct cvmx_npi_mem_access_subidx_s cn50xx;
	struct cvmx_npi_mem_access_subidx_s cn58xx;
	struct cvmx_npi_mem_access_subidx_s cn58xxp1;
};

typedef union cvmx_npi_mem_access_subidx cvmx_npi_mem_access_subidx_t;

/**
 * cvmx_npi_msi_rcv
 *
 * NPI_MSI_RCV = NPI MSI Receive Vector Register
 *
 * A bit is set in this register relative to the vector received during a MSI. And cleared by a W1 to the register.
 */
union cvmx_npi_msi_rcv {
	u64 u64;
	struct cvmx_npi_msi_rcv_s {
		u64 int_vec : 64;
	} s;
	struct cvmx_npi_msi_rcv_s cn30xx;
	struct cvmx_npi_msi_rcv_s cn31xx;
	struct cvmx_npi_msi_rcv_s cn38xx;
	struct cvmx_npi_msi_rcv_s cn38xxp2;
	struct cvmx_npi_msi_rcv_s cn50xx;
	struct cvmx_npi_msi_rcv_s cn58xx;
	struct cvmx_npi_msi_rcv_s cn58xxp1;
};

typedef union cvmx_npi_msi_rcv cvmx_npi_msi_rcv_t;

/**
 * cvmx_npi_num_desc_output#
 *
 * NUM_DESC_OUTPUT0 = Number Of Descriptors Available For Output 0
 *
 * The size of the Buffer/Info Pointer Pair ring for Output-0.
 */
union cvmx_npi_num_desc_outputx {
	u64 u64;
	struct cvmx_npi_num_desc_outputx_s {
		u64 reserved_32_63 : 32;
		u64 size : 32;
	} s;
	struct cvmx_npi_num_desc_outputx_s cn30xx;
	struct cvmx_npi_num_desc_outputx_s cn31xx;
	struct cvmx_npi_num_desc_outputx_s cn38xx;
	struct cvmx_npi_num_desc_outputx_s cn38xxp2;
	struct cvmx_npi_num_desc_outputx_s cn50xx;
	struct cvmx_npi_num_desc_outputx_s cn58xx;
	struct cvmx_npi_num_desc_outputx_s cn58xxp1;
};

typedef union cvmx_npi_num_desc_outputx cvmx_npi_num_desc_outputx_t;

/**
 * cvmx_npi_output_control
 *
 * NPI_OUTPUT_CONTROL = NPI's Output Control Register
 *
 * The address to start reading Instructions from for Output-3.
 */
union cvmx_npi_output_control {
	u64 u64;
	struct cvmx_npi_output_control_s {
		u64 reserved_49_63 : 15;
		u64 pkt_rr : 1;
		u64 p3_bmode : 1;
		u64 p2_bmode : 1;
		u64 p1_bmode : 1;
		u64 p0_bmode : 1;
		u64 o3_es : 2;
		u64 o3_ns : 1;
		u64 o3_ro : 1;
		u64 o2_es : 2;
		u64 o2_ns : 1;
		u64 o2_ro : 1;
		u64 o1_es : 2;
		u64 o1_ns : 1;
		u64 o1_ro : 1;
		u64 o0_es : 2;
		u64 o0_ns : 1;
		u64 o0_ro : 1;
		u64 o3_csrm : 1;
		u64 o2_csrm : 1;
		u64 o1_csrm : 1;
		u64 o0_csrm : 1;
		u64 reserved_20_23 : 4;
		u64 iptr_o3 : 1;
		u64 iptr_o2 : 1;
		u64 iptr_o1 : 1;
		u64 iptr_o0 : 1;
		u64 esr_sl3 : 2;
		u64 nsr_sl3 : 1;
		u64 ror_sl3 : 1;
		u64 esr_sl2 : 2;
		u64 nsr_sl2 : 1;
		u64 ror_sl2 : 1;
		u64 esr_sl1 : 2;
		u64 nsr_sl1 : 1;
		u64 ror_sl1 : 1;
		u64 esr_sl0 : 2;
		u64 nsr_sl0 : 1;
		u64 ror_sl0 : 1;
	} s;
	struct cvmx_npi_output_control_cn30xx {
		u64 reserved_45_63 : 19;
		u64 p0_bmode : 1;
		u64 reserved_32_43 : 12;
		u64 o0_es : 2;
		u64 o0_ns : 1;
		u64 o0_ro : 1;
		u64 reserved_25_27 : 3;
		u64 o0_csrm : 1;
		u64 reserved_17_23 : 7;
		u64 iptr_o0 : 1;
		u64 reserved_4_15 : 12;
		u64 esr_sl0 : 2;
		u64 nsr_sl0 : 1;
		u64 ror_sl0 : 1;
	} cn30xx;
	struct cvmx_npi_output_control_cn31xx {
		u64 reserved_46_63 : 18;
		u64 p1_bmode : 1;
		u64 p0_bmode : 1;
		u64 reserved_36_43 : 8;
		u64 o1_es : 2;
		u64 o1_ns : 1;
		u64 o1_ro : 1;
		u64 o0_es : 2;
		u64 o0_ns : 1;
		u64 o0_ro : 1;
		u64 reserved_26_27 : 2;
		u64 o1_csrm : 1;
		u64 o0_csrm : 1;
		u64 reserved_18_23 : 6;
		u64 iptr_o1 : 1;
		u64 iptr_o0 : 1;
		u64 reserved_8_15 : 8;
		u64 esr_sl1 : 2;
		u64 nsr_sl1 : 1;
		u64 ror_sl1 : 1;
		u64 esr_sl0 : 2;
		u64 nsr_sl0 : 1;
		u64 ror_sl0 : 1;
	} cn31xx;
	struct cvmx_npi_output_control_s cn38xx;
	struct cvmx_npi_output_control_cn38xxp2 {
		u64 reserved_48_63 : 16;
		u64 p3_bmode : 1;
		u64 p2_bmode : 1;
		u64 p1_bmode : 1;
		u64 p0_bmode : 1;
		u64 o3_es : 2;
		u64 o3_ns : 1;
		u64 o3_ro : 1;
		u64 o2_es : 2;
		u64 o2_ns : 1;
		u64 o2_ro : 1;
		u64 o1_es : 2;
		u64 o1_ns : 1;
		u64 o1_ro : 1;
		u64 o0_es : 2;
		u64 o0_ns : 1;
		u64 o0_ro : 1;
		u64 o3_csrm : 1;
		u64 o2_csrm : 1;
		u64 o1_csrm : 1;
		u64 o0_csrm : 1;
		u64 reserved_20_23 : 4;
		u64 iptr_o3 : 1;
		u64 iptr_o2 : 1;
		u64 iptr_o1 : 1;
		u64 iptr_o0 : 1;
		u64 esr_sl3 : 2;
		u64 nsr_sl3 : 1;
		u64 ror_sl3 : 1;
		u64 esr_sl2 : 2;
		u64 nsr_sl2 : 1;
		u64 ror_sl2 : 1;
		u64 esr_sl1 : 2;
		u64 nsr_sl1 : 1;
		u64 ror_sl1 : 1;
		u64 esr_sl0 : 2;
		u64 nsr_sl0 : 1;
		u64 ror_sl0 : 1;
	} cn38xxp2;
	struct cvmx_npi_output_control_cn50xx {
		u64 reserved_49_63 : 15;
		u64 pkt_rr : 1;
		u64 reserved_46_47 : 2;
		u64 p1_bmode : 1;
		u64 p0_bmode : 1;
		u64 reserved_36_43 : 8;
		u64 o1_es : 2;
		u64 o1_ns : 1;
		u64 o1_ro : 1;
		u64 o0_es : 2;
		u64 o0_ns : 1;
		u64 o0_ro : 1;
		u64 reserved_26_27 : 2;
		u64 o1_csrm : 1;
		u64 o0_csrm : 1;
		u64 reserved_18_23 : 6;
		u64 iptr_o1 : 1;
		u64 iptr_o0 : 1;
		u64 reserved_8_15 : 8;
		u64 esr_sl1 : 2;
		u64 nsr_sl1 : 1;
		u64 ror_sl1 : 1;
		u64 esr_sl0 : 2;
		u64 nsr_sl0 : 1;
		u64 ror_sl0 : 1;
	} cn50xx;
	struct cvmx_npi_output_control_s cn58xx;
	struct cvmx_npi_output_control_s cn58xxp1;
};

typedef union cvmx_npi_output_control cvmx_npi_output_control_t;

/**
 * cvmx_npi_p#_dbpair_addr
 *
 * NPI_P0_DBPAIR_ADDR = NPI's Port-0 DATA-BUFFER Pair Next Read Address.
 *
 * Contains the next address to read for Port's-0 Data/Buffer Pair.
 */
union cvmx_npi_px_dbpair_addr {
	u64 u64;
	struct cvmx_npi_px_dbpair_addr_s {
		u64 reserved_63_63 : 1;
		u64 state : 2;
		u64 naddr : 61;
	} s;
	struct cvmx_npi_px_dbpair_addr_s cn30xx;
	struct cvmx_npi_px_dbpair_addr_s cn31xx;
	struct cvmx_npi_px_dbpair_addr_s cn38xx;
	struct cvmx_npi_px_dbpair_addr_s cn38xxp2;
	struct cvmx_npi_px_dbpair_addr_s cn50xx;
	struct cvmx_npi_px_dbpair_addr_s cn58xx;
	struct cvmx_npi_px_dbpair_addr_s cn58xxp1;
};

typedef union cvmx_npi_px_dbpair_addr cvmx_npi_px_dbpair_addr_t;

/**
 * cvmx_npi_p#_instr_addr
 *
 * NPI_P0_INSTR_ADDR = NPI's Port-0 Instruction Next Read Address.
 *
 * Contains the next address to read for Port's-0 Instructions.
 */
union cvmx_npi_px_instr_addr {
	u64 u64;
	struct cvmx_npi_px_instr_addr_s {
		u64 state : 3;
		u64 naddr : 61;
	} s;
	struct cvmx_npi_px_instr_addr_s cn30xx;
	struct cvmx_npi_px_instr_addr_s cn31xx;
	struct cvmx_npi_px_instr_addr_s cn38xx;
	struct cvmx_npi_px_instr_addr_s cn38xxp2;
	struct cvmx_npi_px_instr_addr_s cn50xx;
	struct cvmx_npi_px_instr_addr_s cn58xx;
	struct cvmx_npi_px_instr_addr_s cn58xxp1;
};

typedef union cvmx_npi_px_instr_addr cvmx_npi_px_instr_addr_t;

/**
 * cvmx_npi_p#_instr_cnts
 *
 * NPI_P0_INSTR_CNTS = NPI's Port-0 Instruction Counts For Packets In.
 *
 * Used to determine the number of instruction in the NPI and to be fetched for Input-Packets.
 */
union cvmx_npi_px_instr_cnts {
	u64 u64;
	struct cvmx_npi_px_instr_cnts_s {
		u64 reserved_38_63 : 26;
		u64 fcnt : 6;
		u64 avail : 32;
	} s;
	struct cvmx_npi_px_instr_cnts_s cn30xx;
	struct cvmx_npi_px_instr_cnts_s cn31xx;
	struct cvmx_npi_px_instr_cnts_s cn38xx;
	struct cvmx_npi_px_instr_cnts_s cn38xxp2;
	struct cvmx_npi_px_instr_cnts_s cn50xx;
	struct cvmx_npi_px_instr_cnts_s cn58xx;
	struct cvmx_npi_px_instr_cnts_s cn58xxp1;
};

typedef union cvmx_npi_px_instr_cnts cvmx_npi_px_instr_cnts_t;

/**
 * cvmx_npi_p#_pair_cnts
 *
 * NPI_P0_PAIR_CNTS = NPI's Port-0 Instruction Counts For Packets Out.
 *
 * Used to determine the number of instruction in the NPI and to be fetched for Output-Packets.
 */
union cvmx_npi_px_pair_cnts {
	u64 u64;
	struct cvmx_npi_px_pair_cnts_s {
		u64 reserved_37_63 : 27;
		u64 fcnt : 5;
		u64 avail : 32;
	} s;
	struct cvmx_npi_px_pair_cnts_s cn30xx;
	struct cvmx_npi_px_pair_cnts_s cn31xx;
	struct cvmx_npi_px_pair_cnts_s cn38xx;
	struct cvmx_npi_px_pair_cnts_s cn38xxp2;
	struct cvmx_npi_px_pair_cnts_s cn50xx;
	struct cvmx_npi_px_pair_cnts_s cn58xx;
	struct cvmx_npi_px_pair_cnts_s cn58xxp1;
};

typedef union cvmx_npi_px_pair_cnts cvmx_npi_px_pair_cnts_t;

/**
 * cvmx_npi_pci_burst_size
 *
 * NPI_PCI_BURST_SIZE = NPI PCI Burst Size Register
 *
 * Control the number of words the NPI will attempt to read / write to/from the PCI.
 */
union cvmx_npi_pci_burst_size {
	u64 u64;
	struct cvmx_npi_pci_burst_size_s {
		u64 reserved_14_63 : 50;
		u64 wr_brst : 7;
		u64 rd_brst : 7;
	} s;
	struct cvmx_npi_pci_burst_size_s cn30xx;
	struct cvmx_npi_pci_burst_size_s cn31xx;
	struct cvmx_npi_pci_burst_size_s cn38xx;
	struct cvmx_npi_pci_burst_size_s cn38xxp2;
	struct cvmx_npi_pci_burst_size_s cn50xx;
	struct cvmx_npi_pci_burst_size_s cn58xx;
	struct cvmx_npi_pci_burst_size_s cn58xxp1;
};

typedef union cvmx_npi_pci_burst_size cvmx_npi_pci_burst_size_t;

/**
 * cvmx_npi_pci_int_arb_cfg
 *
 * NPI_PCI_INT_ARB_CFG = Configuration For PCI Arbiter
 *
 * Controls operation of the Internal PCI Arbiter.  This register should
 * only be written when PRST# is asserted.  NPI_PCI_INT_ARB_CFG[EN] should
 * only be set when Octane is a host.
 */
union cvmx_npi_pci_int_arb_cfg {
	u64 u64;
	struct cvmx_npi_pci_int_arb_cfg_s {
		u64 reserved_13_63 : 51;
		u64 hostmode : 1;
		u64 pci_ovr : 4;
		u64 reserved_5_7 : 3;
		u64 en : 1;
		u64 park_mod : 1;
		u64 park_dev : 3;
	} s;
	struct cvmx_npi_pci_int_arb_cfg_cn30xx {
		u64 reserved_5_63 : 59;
		u64 en : 1;
		u64 park_mod : 1;
		u64 park_dev : 3;
	} cn30xx;
	struct cvmx_npi_pci_int_arb_cfg_cn30xx cn31xx;
	struct cvmx_npi_pci_int_arb_cfg_cn30xx cn38xx;
	struct cvmx_npi_pci_int_arb_cfg_cn30xx cn38xxp2;
	struct cvmx_npi_pci_int_arb_cfg_s cn50xx;
	struct cvmx_npi_pci_int_arb_cfg_s cn58xx;
	struct cvmx_npi_pci_int_arb_cfg_s cn58xxp1;
};

typedef union cvmx_npi_pci_int_arb_cfg cvmx_npi_pci_int_arb_cfg_t;

/**
 * cvmx_npi_pci_read_cmd
 *
 * NPI_PCI_READ_CMD = NPI PCI Read Command Register
 *
 * Controls the type of read command sent.
 * Writes to this register are not ordered with writes/reads to the PCI Memory space.
 * To ensure that a write has completed the user must read the register before
 * making an access(i.e. PCI memory space) that requires the value of this register to be updated.
 * Also any previously issued reads/writes to PCI memory space, still stored in the outbound
 * FIFO will use the value of this register after it has been updated.
 */
union cvmx_npi_pci_read_cmd {
	u64 u64;
	struct cvmx_npi_pci_read_cmd_s {
		u64 reserved_11_63 : 53;
		u64 cmd_size : 11;
	} s;
	struct cvmx_npi_pci_read_cmd_s cn30xx;
	struct cvmx_npi_pci_read_cmd_s cn31xx;
	struct cvmx_npi_pci_read_cmd_s cn38xx;
	struct cvmx_npi_pci_read_cmd_s cn38xxp2;
	struct cvmx_npi_pci_read_cmd_s cn50xx;
	struct cvmx_npi_pci_read_cmd_s cn58xx;
	struct cvmx_npi_pci_read_cmd_s cn58xxp1;
};

typedef union cvmx_npi_pci_read_cmd cvmx_npi_pci_read_cmd_t;

/**
 * cvmx_npi_port32_instr_hdr
 *
 * NPI_PORT32_INSTR_HDR = NPI Port 32 Instruction Header
 *
 * Contains bits [62:42] of the Instruction Header for port 32.
 */
union cvmx_npi_port32_instr_hdr {
	u64 u64;
	struct cvmx_npi_port32_instr_hdr_s {
		u64 reserved_44_63 : 20;
		u64 pbp : 1;
		u64 rsv_f : 5;
		u64 rparmode : 2;
		u64 rsv_e : 1;
		u64 rskp_len : 7;
		u64 rsv_d : 6;
		u64 use_ihdr : 1;
		u64 rsv_c : 5;
		u64 par_mode : 2;
		u64 rsv_b : 1;
		u64 skp_len : 7;
		u64 rsv_a : 6;
	} s;
	struct cvmx_npi_port32_instr_hdr_s cn30xx;
	struct cvmx_npi_port32_instr_hdr_s cn31xx;
	struct cvmx_npi_port32_instr_hdr_s cn38xx;
	struct cvmx_npi_port32_instr_hdr_s cn38xxp2;
	struct cvmx_npi_port32_instr_hdr_s cn50xx;
	struct cvmx_npi_port32_instr_hdr_s cn58xx;
	struct cvmx_npi_port32_instr_hdr_s cn58xxp1;
};

typedef union cvmx_npi_port32_instr_hdr cvmx_npi_port32_instr_hdr_t;

/**
 * cvmx_npi_port33_instr_hdr
 *
 * NPI_PORT33_INSTR_HDR = NPI Port 33 Instruction Header
 *
 * Contains bits [62:42] of the Instruction Header for port 33.
 */
union cvmx_npi_port33_instr_hdr {
	u64 u64;
	struct cvmx_npi_port33_instr_hdr_s {
		u64 reserved_44_63 : 20;
		u64 pbp : 1;
		u64 rsv_f : 5;
		u64 rparmode : 2;
		u64 rsv_e : 1;
		u64 rskp_len : 7;
		u64 rsv_d : 6;
		u64 use_ihdr : 1;
		u64 rsv_c : 5;
		u64 par_mode : 2;
		u64 rsv_b : 1;
		u64 skp_len : 7;
		u64 rsv_a : 6;
	} s;
	struct cvmx_npi_port33_instr_hdr_s cn31xx;
	struct cvmx_npi_port33_instr_hdr_s cn38xx;
	struct cvmx_npi_port33_instr_hdr_s cn38xxp2;
	struct cvmx_npi_port33_instr_hdr_s cn50xx;
	struct cvmx_npi_port33_instr_hdr_s cn58xx;
	struct cvmx_npi_port33_instr_hdr_s cn58xxp1;
};

typedef union cvmx_npi_port33_instr_hdr cvmx_npi_port33_instr_hdr_t;

/**
 * cvmx_npi_port34_instr_hdr
 *
 * NPI_PORT34_INSTR_HDR = NPI Port 34 Instruction Header
 *
 * Contains bits [62:42] of the Instruction Header for port 34. Added for PASS-2.
 */
union cvmx_npi_port34_instr_hdr {
	u64 u64;
	struct cvmx_npi_port34_instr_hdr_s {
		u64 reserved_44_63 : 20;
		u64 pbp : 1;
		u64 rsv_f : 5;
		u64 rparmode : 2;
		u64 rsv_e : 1;
		u64 rskp_len : 7;
		u64 rsv_d : 6;
		u64 use_ihdr : 1;
		u64 rsv_c : 5;
		u64 par_mode : 2;
		u64 rsv_b : 1;
		u64 skp_len : 7;
		u64 rsv_a : 6;
	} s;
	struct cvmx_npi_port34_instr_hdr_s cn38xx;
	struct cvmx_npi_port34_instr_hdr_s cn38xxp2;
	struct cvmx_npi_port34_instr_hdr_s cn58xx;
	struct cvmx_npi_port34_instr_hdr_s cn58xxp1;
};

typedef union cvmx_npi_port34_instr_hdr cvmx_npi_port34_instr_hdr_t;

/**
 * cvmx_npi_port35_instr_hdr
 *
 * NPI_PORT35_INSTR_HDR = NPI Port 35 Instruction Header
 *
 * Contains bits [62:42] of the Instruction Header for port 35. Added for PASS-2.
 */
union cvmx_npi_port35_instr_hdr {
	u64 u64;
	struct cvmx_npi_port35_instr_hdr_s {
		u64 reserved_44_63 : 20;
		u64 pbp : 1;
		u64 rsv_f : 5;
		u64 rparmode : 2;
		u64 rsv_e : 1;
		u64 rskp_len : 7;
		u64 rsv_d : 6;
		u64 use_ihdr : 1;
		u64 rsv_c : 5;
		u64 par_mode : 2;
		u64 rsv_b : 1;
		u64 skp_len : 7;
		u64 rsv_a : 6;
	} s;
	struct cvmx_npi_port35_instr_hdr_s cn38xx;
	struct cvmx_npi_port35_instr_hdr_s cn38xxp2;
	struct cvmx_npi_port35_instr_hdr_s cn58xx;
	struct cvmx_npi_port35_instr_hdr_s cn58xxp1;
};

typedef union cvmx_npi_port35_instr_hdr cvmx_npi_port35_instr_hdr_t;

/**
 * cvmx_npi_port_bp_control
 *
 * NPI_PORT_BP_CONTROL = Port Backpressure Control
 *
 * Enables Port Level Backpressure
 */
union cvmx_npi_port_bp_control {
	u64 u64;
	struct cvmx_npi_port_bp_control_s {
		u64 reserved_8_63 : 56;
		u64 bp_on : 4;
		u64 enb : 4;
	} s;
	struct cvmx_npi_port_bp_control_s cn30xx;
	struct cvmx_npi_port_bp_control_s cn31xx;
	struct cvmx_npi_port_bp_control_s cn38xx;
	struct cvmx_npi_port_bp_control_s cn38xxp2;
	struct cvmx_npi_port_bp_control_s cn50xx;
	struct cvmx_npi_port_bp_control_s cn58xx;
	struct cvmx_npi_port_bp_control_s cn58xxp1;
};

typedef union cvmx_npi_port_bp_control cvmx_npi_port_bp_control_t;

/**
 * cvmx_npi_rsl_int_blocks
 *
 * RSL_INT_BLOCKS = RSL Interrupt Blocks Register
 *
 * Reading this register will return a vector with a bit set '1' for a corresponding RSL block
 * that presently has an interrupt pending. The Field Description below supplies the name of the
 * register that software should read to find out why that intterupt bit is set.
 */
union cvmx_npi_rsl_int_blocks {
	u64 u64;
	struct cvmx_npi_rsl_int_blocks_s {
		u64 reserved_32_63 : 32;
		u64 rint_31 : 1;
		u64 iob : 1;
		u64 reserved_28_29 : 2;
		u64 rint_27 : 1;
		u64 rint_26 : 1;
		u64 rint_25 : 1;
		u64 rint_24 : 1;
		u64 asx1 : 1;
		u64 asx0 : 1;
		u64 rint_21 : 1;
		u64 pip : 1;
		u64 spx1 : 1;
		u64 spx0 : 1;
		u64 lmc : 1;
		u64 l2c : 1;
		u64 rint_15 : 1;
		u64 reserved_13_14 : 2;
		u64 pow : 1;
		u64 tim : 1;
		u64 pko : 1;
		u64 ipd : 1;
		u64 rint_8 : 1;
		u64 zip : 1;
		u64 dfa : 1;
		u64 fpa : 1;
		u64 key : 1;
		u64 npi : 1;
		u64 gmx1 : 1;
		u64 gmx0 : 1;
		u64 mio : 1;
	} s;
	struct cvmx_npi_rsl_int_blocks_cn30xx {
		u64 reserved_32_63 : 32;
		u64 rint_31 : 1;
		u64 iob : 1;
		u64 rint_29 : 1;
		u64 rint_28 : 1;
		u64 rint_27 : 1;
		u64 rint_26 : 1;
		u64 rint_25 : 1;
		u64 rint_24 : 1;
		u64 asx1 : 1;
		u64 asx0 : 1;
		u64 rint_21 : 1;
		u64 pip : 1;
		u64 spx1 : 1;
		u64 spx0 : 1;
		u64 lmc : 1;
		u64 l2c : 1;
		u64 rint_15 : 1;
		u64 rint_14 : 1;
		u64 usb : 1;
		u64 pow : 1;
		u64 tim : 1;
		u64 pko : 1;
		u64 ipd : 1;
		u64 rint_8 : 1;
		u64 zip : 1;
		u64 dfa : 1;
		u64 fpa : 1;
		u64 key : 1;
		u64 npi : 1;
		u64 gmx1 : 1;
		u64 gmx0 : 1;
		u64 mio : 1;
	} cn30xx;
	struct cvmx_npi_rsl_int_blocks_cn30xx cn31xx;
	struct cvmx_npi_rsl_int_blocks_cn38xx {
		u64 reserved_32_63 : 32;
		u64 rint_31 : 1;
		u64 iob : 1;
		u64 rint_29 : 1;
		u64 rint_28 : 1;
		u64 rint_27 : 1;
		u64 rint_26 : 1;
		u64 rint_25 : 1;
		u64 rint_24 : 1;
		u64 asx1 : 1;
		u64 asx0 : 1;
		u64 rint_21 : 1;
		u64 pip : 1;
		u64 spx1 : 1;
		u64 spx0 : 1;
		u64 lmc : 1;
		u64 l2c : 1;
		u64 rint_15 : 1;
		u64 rint_14 : 1;
		u64 rint_13 : 1;
		u64 pow : 1;
		u64 tim : 1;
		u64 pko : 1;
		u64 ipd : 1;
		u64 rint_8 : 1;
		u64 zip : 1;
		u64 dfa : 1;
		u64 fpa : 1;
		u64 key : 1;
		u64 npi : 1;
		u64 gmx1 : 1;
		u64 gmx0 : 1;
		u64 mio : 1;
	} cn38xx;
	struct cvmx_npi_rsl_int_blocks_cn38xx cn38xxp2;
	struct cvmx_npi_rsl_int_blocks_cn50xx {
		u64 reserved_31_63 : 33;
		u64 iob : 1;
		u64 lmc1 : 1;
		u64 agl : 1;
		u64 reserved_24_27 : 4;
		u64 asx1 : 1;
		u64 asx0 : 1;
		u64 reserved_21_21 : 1;
		u64 pip : 1;
		u64 spx1 : 1;
		u64 spx0 : 1;
		u64 lmc : 1;
		u64 l2c : 1;
		u64 reserved_15_15 : 1;
		u64 rad : 1;
		u64 usb : 1;
		u64 pow : 1;
		u64 tim : 1;
		u64 pko : 1;
		u64 ipd : 1;
		u64 reserved_8_8 : 1;
		u64 zip : 1;
		u64 dfa : 1;
		u64 fpa : 1;
		u64 key : 1;
		u64 npi : 1;
		u64 gmx1 : 1;
		u64 gmx0 : 1;
		u64 mio : 1;
	} cn50xx;
	struct cvmx_npi_rsl_int_blocks_cn38xx cn58xx;
	struct cvmx_npi_rsl_int_blocks_cn38xx cn58xxp1;
};

typedef union cvmx_npi_rsl_int_blocks cvmx_npi_rsl_int_blocks_t;

/**
 * cvmx_npi_size_input#
 *
 * NPI_SIZE_INPUT0 = NPI's Size for Input 0 Register
 *
 * The size (in instructions) of Instruction Queue-0.
 */
union cvmx_npi_size_inputx {
	u64 u64;
	struct cvmx_npi_size_inputx_s {
		u64 reserved_32_63 : 32;
		u64 size : 32;
	} s;
	struct cvmx_npi_size_inputx_s cn30xx;
	struct cvmx_npi_size_inputx_s cn31xx;
	struct cvmx_npi_size_inputx_s cn38xx;
	struct cvmx_npi_size_inputx_s cn38xxp2;
	struct cvmx_npi_size_inputx_s cn50xx;
	struct cvmx_npi_size_inputx_s cn58xx;
	struct cvmx_npi_size_inputx_s cn58xxp1;
};

typedef union cvmx_npi_size_inputx cvmx_npi_size_inputx_t;

/**
 * cvmx_npi_win_read_to
 *
 * NPI_WIN_READ_TO = NPI WINDOW READ Timeout Register
 *
 * Number of core clocks to wait before timing out on a WINDOW-READ to the NCB.
 */
union cvmx_npi_win_read_to {
	u64 u64;
	struct cvmx_npi_win_read_to_s {
		u64 reserved_32_63 : 32;
		u64 time : 32;
	} s;
	struct cvmx_npi_win_read_to_s cn30xx;
	struct cvmx_npi_win_read_to_s cn31xx;
	struct cvmx_npi_win_read_to_s cn38xx;
	struct cvmx_npi_win_read_to_s cn38xxp2;
	struct cvmx_npi_win_read_to_s cn50xx;
	struct cvmx_npi_win_read_to_s cn58xx;
	struct cvmx_npi_win_read_to_s cn58xxp1;
};

typedef union cvmx_npi_win_read_to cvmx_npi_win_read_to_t;

#endif
