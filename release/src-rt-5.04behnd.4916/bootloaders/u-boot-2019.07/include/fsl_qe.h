/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2006-2009 Freescale Semiconductor, Inc.
 *
 * Dave Liu <daveliu@freescale.com>
 * based on source code of Shlomi Gridish
 */

#ifndef __QE_H__
#define __QE_H__

#include "common.h"
#ifdef CONFIG_U_QE
#include <linux/immap_qe.h>
#endif

#define QE_NUM_OF_BRGS	16
#define UCC_MAX_NUM	8

#define QE_DATAONLY_BASE	0
#define QE_DATAONLY_SIZE	(QE_MURAM_SIZE - QE_DATAONLY_BASE)

/* QE threads SNUM
*/
typedef enum qe_snum_state {
	QE_SNUM_STATE_USED,   /* used */
	QE_SNUM_STATE_FREE    /* free */
} qe_snum_state_e;

typedef struct qe_snum {
	u8		num;   /* snum	*/
	qe_snum_state_e	state; /* state */
} qe_snum_t;

/* QE RISC allocation
*/
#define	QE_RISC_ALLOCATION_RISC1	0x1  /* RISC 1 */
#define	QE_RISC_ALLOCATION_RISC2	0x2  /* RISC 2 */
#define	QE_RISC_ALLOCATION_RISC3	0x4  /* RISC 3 */
#define	QE_RISC_ALLOCATION_RISC4	0x8  /* RISC 4 */
#define	QE_RISC_ALLOCATION_RISC1_AND_RISC2 	(QE_RISC_ALLOCATION_RISC1 | \
						 QE_RISC_ALLOCATION_RISC2)
#define	QE_RISC_ALLOCATION_FOUR_RISCS	(QE_RISC_ALLOCATION_RISC1 | \
					 QE_RISC_ALLOCATION_RISC2 | \
					 QE_RISC_ALLOCATION_RISC3 | \
					 QE_RISC_ALLOCATION_RISC4)

/* QE CECR commands for UCC fast.
*/
#define QE_CR_FLG			0x00010000
#define QE_RESET			0x80000000
#define QE_INIT_TX_RX			0x00000000
#define QE_INIT_RX			0x00000001
#define QE_INIT_TX			0x00000002
#define QE_ENTER_HUNT_MODE		0x00000003
#define QE_STOP_TX			0x00000004
#define QE_GRACEFUL_STOP_TX		0x00000005
#define QE_RESTART_TX			0x00000006
#define QE_SWITCH_COMMAND		0x00000007
#define QE_SET_GROUP_ADDRESS		0x00000008
#define QE_INSERT_CELL			0x00000009
#define QE_ATM_TRANSMIT			0x0000000a
#define QE_CELL_POOL_GET		0x0000000b
#define QE_CELL_POOL_PUT		0x0000000c
#define QE_IMA_HOST_CMD			0x0000000d
#define QE_ATM_MULTI_THREAD_INIT	0x00000011
#define QE_ASSIGN_PAGE			0x00000012
#define QE_START_FLOW_CONTROL		0x00000014
#define QE_STOP_FLOW_CONTROL		0x00000015
#define QE_ASSIGN_PAGE_TO_DEVICE	0x00000016
#define QE_GRACEFUL_STOP_RX		0x0000001a
#define QE_RESTART_RX			0x0000001b

/* QE CECR Sub Block Code - sub block code of QE command.
*/
#define QE_CR_SUBBLOCK_INVALID		0x00000000
#define QE_CR_SUBBLOCK_USB		0x03200000
#define QE_CR_SUBBLOCK_UCCFAST1		0x02000000
#define QE_CR_SUBBLOCK_UCCFAST2		0x02200000
#define QE_CR_SUBBLOCK_UCCFAST3		0x02400000
#define QE_CR_SUBBLOCK_UCCFAST4		0x02600000
#define QE_CR_SUBBLOCK_UCCFAST5		0x02800000
#define QE_CR_SUBBLOCK_UCCFAST6		0x02a00000
#define QE_CR_SUBBLOCK_UCCFAST7		0x02c00000
#define QE_CR_SUBBLOCK_UCCFAST8		0x02e00000
#define QE_CR_SUBBLOCK_UCCSLOW1		0x00000000
#define QE_CR_SUBBLOCK_UCCSLOW2		0x00200000
#define QE_CR_SUBBLOCK_UCCSLOW3		0x00400000
#define QE_CR_SUBBLOCK_UCCSLOW4		0x00600000
#define QE_CR_SUBBLOCK_UCCSLOW5		0x00800000
#define QE_CR_SUBBLOCK_UCCSLOW6		0x00a00000
#define QE_CR_SUBBLOCK_UCCSLOW7		0x00c00000
#define QE_CR_SUBBLOCK_UCCSLOW8		0x00e00000
#define QE_CR_SUBBLOCK_MCC1		0x03800000
#define QE_CR_SUBBLOCK_MCC2		0x03a00000
#define QE_CR_SUBBLOCK_MCC3		0x03000000
#define QE_CR_SUBBLOCK_IDMA1		0x02800000
#define QE_CR_SUBBLOCK_IDMA2		0x02a00000
#define QE_CR_SUBBLOCK_IDMA3		0x02c00000
#define QE_CR_SUBBLOCK_IDMA4		0x02e00000
#define QE_CR_SUBBLOCK_HPAC		0x01e00000
#define QE_CR_SUBBLOCK_SPI1		0x01400000
#define QE_CR_SUBBLOCK_SPI2		0x01600000
#define QE_CR_SUBBLOCK_RAND		0x01c00000
#define QE_CR_SUBBLOCK_TIMER		0x01e00000
#define QE_CR_SUBBLOCK_GENERAL		0x03c00000

/* QE CECR Protocol - For non-MCC, specifies mode for QE CECR command.
*/
#define QE_CR_PROTOCOL_UNSPECIFIED	0x00 /* For all other protocols */
#define QE_CR_PROTOCOL_HDLC_TRANSPARENT	0x00
#define QE_CR_PROTOCOL_ATM_POS		0x0A
#define QE_CR_PROTOCOL_ETHERNET		0x0C
#define QE_CR_PROTOCOL_L2_SWITCH	0x0D
#define QE_CR_PROTOCOL_SHIFT		6

/* QE ASSIGN PAGE command
*/
#define QE_CR_ASSIGN_PAGE_SNUM_SHIFT	17

/* Communication Direction.
*/
typedef enum comm_dir {
	COMM_DIR_NONE		= 0,
	COMM_DIR_RX		= 1,
	COMM_DIR_TX		= 2,
	COMM_DIR_RX_AND_TX	= 3
} comm_dir_e;

/* Clocks and BRG's
*/
typedef enum qe_clock {
	QE_CLK_NONE = 0,
	QE_BRG1,     /* Baud Rate Generator  1 */
	QE_BRG2,     /* Baud Rate Generator  2 */
	QE_BRG3,     /* Baud Rate Generator  3 */
	QE_BRG4,     /* Baud Rate Generator  4 */
	QE_BRG5,     /* Baud Rate Generator  5 */
	QE_BRG6,     /* Baud Rate Generator  6 */
	QE_BRG7,     /* Baud Rate Generator  7 */
	QE_BRG8,     /* Baud Rate Generator  8 */
	QE_BRG9,     /* Baud Rate Generator  9 */
	QE_BRG10,    /* Baud Rate Generator 10 */
	QE_BRG11,    /* Baud Rate Generator 11 */
	QE_BRG12,    /* Baud Rate Generator 12 */
	QE_BRG13,    /* Baud Rate Generator 13 */
	QE_BRG14,    /* Baud Rate Generator 14 */
	QE_BRG15,    /* Baud Rate Generator 15 */
	QE_BRG16,    /* Baud Rate Generator 16 */
	QE_CLK1,     /* Clock  1	       */
	QE_CLK2,     /* Clock  2	       */
	QE_CLK3,     /* Clock  3	       */
	QE_CLK4,     /* Clock  4	       */
	QE_CLK5,     /* Clock  5	       */
	QE_CLK6,     /* Clock  6	       */
	QE_CLK7,     /* Clock  7	       */
	QE_CLK8,     /* Clock  8	       */
	QE_CLK9,     /* Clock  9	       */
	QE_CLK10,    /* Clock 10	       */
	QE_CLK11,    /* Clock 11	       */
	QE_CLK12,    /* Clock 12	       */
	QE_CLK13,    /* Clock 13	       */
	QE_CLK14,    /* Clock 14	       */
	QE_CLK15,    /* Clock 15	       */
	QE_CLK16,    /* Clock 16	       */
	QE_CLK17,    /* Clock 17	       */
	QE_CLK18,    /* Clock 18	       */
	QE_CLK19,    /* Clock 19	       */
	QE_CLK20,    /* Clock 20	       */
	QE_CLK21,    /* Clock 21	       */
	QE_CLK22,    /* Clock 22	       */
	QE_CLK23,    /* Clock 23	       */
	QE_CLK24,    /* Clock 24	       */
	QE_CLK_DUMMY
} qe_clock_e;

/* QE CMXGCR register
*/
#define QE_CMXGCR_MII_ENET_MNG_MASK	0x00007000
#define QE_CMXGCR_MII_ENET_MNG_SHIFT	12

/* QE CMXUCR registers
 */
#define QE_CMXUCR_TX_CLK_SRC_MASK	0x0000000F

/* QE BRG configuration register
*/
#define QE_BRGC_ENABLE			0x00010000
#define QE_BRGC_DIVISOR_SHIFT		1
#define QE_BRGC_DIVISOR_MAX		0xFFF
#define QE_BRGC_DIV16			1

/* QE SDMA registers
*/
#define QE_SDSR_BER1			0x02000000
#define QE_SDSR_BER2			0x01000000

#define QE_SDMR_GLB_1_MSK		0x80000000
#define QE_SDMR_ADR_SEL			0x20000000
#define QE_SDMR_BER1_MSK		0x02000000
#define QE_SDMR_BER2_MSK		0x01000000
#define QE_SDMR_EB1_MSK			0x00800000
#define QE_SDMR_ER1_MSK			0x00080000
#define QE_SDMR_ER2_MSK			0x00040000
#define QE_SDMR_CEN_MASK		0x0000E000
#define QE_SDMR_SBER_1			0x00000200
#define QE_SDMR_SBER_2			0x00000200
#define QE_SDMR_EB1_PR_MASK		0x000000C0
#define QE_SDMR_ER1_PR			0x00000008

#define QE_SDMR_CEN_SHIFT		13
#define QE_SDMR_EB1_PR_SHIFT		6

#define QE_SDTM_MSNUM_SHIFT		24

#define QE_SDEBCR_BA_MASK		0x01FFFFFF

/* Communication Processor */
#define QE_CP_CERCR_MEE		0x8000	/* Multi-user RAM ECC enable */
#define QE_CP_CERCR_IEE		0x4000	/* Instruction RAM ECC enable */
#define QE_CP_CERCR_CIR		0x0800	/* Common instruction RAM */

/* I-RAM */
#define QE_IRAM_IADD_AIE	0x80000000	/* Auto Increment Enable */
#define QE_IRAM_IADD_BADDR	0x00080000	/* Base Address */
#define QE_IRAM_READY		0x80000000

/* Structure that defines QE firmware binary files.
 *
 * See doc/README.qe_firmware for a description of these fields.
 */
struct qe_firmware {
	struct qe_header {
		u32 length;	/* Length of the entire structure, in bytes */
		u8 magic[3];	/* Set to { 'Q', 'E', 'F' } */
		u8 version;	/* Version of this layout. First ver is '1' */
	} header;
	u8 id[62];		/* Null-terminated identifier string */
	u8 split;		/* 0 = shared I-RAM, 1 = split I-RAM */
	u8 count;		/* Number of microcode[] structures */
	struct {
		u16 model;	/* The SOC model  */
		u8 major;	/* The SOC revision major */
		u8 minor;	/* The SOC revision minor */
	} __attribute__ ((packed)) soc;
	u8 padding[4];		/* Reserved, for alignment */
	u64 extended_modes;	/* Extended modes */
	u32 vtraps[8];		/* Virtual trap addresses */
	u8 reserved[4];		/* Reserved, for future expansion */
	struct qe_microcode {
		u8 id[32];	/* Null-terminated identifier */
		u32 traps[16];	/* Trap addresses, 0 == ignore */
		u32 eccr;	/* The value for the ECCR register */
		u32 iram_offset;/* Offset into I-RAM for the code */
		u32 count;	/* Number of 32-bit words of the code */
		u32 code_offset;/* Offset of the actual microcode */
		u8 major;	/* The microcode version major */
		u8 minor;	/* The microcode version minor */
		u8 revision;	/* The microcode version revision */
		u8 padding;	/* Reserved, for alignment */
		u8 reserved[4];	/* Reserved, for future expansion */
	} __attribute__ ((packed)) microcode[1];
	/* All microcode binaries should be located here */
	/* CRC32 should be located here, after the microcode binaries */
} __attribute__ ((packed));

struct qe_firmware_info {
	char id[64];		/* Firmware name */
	u32 vtraps[8];		/* Virtual trap addresses */
	u64 extended_modes;	/* Extended modes */
};

void qe_config_iopin(u8 port, u8 pin, int dir, int open_drain, int assign);
void qe_issue_cmd(uint cmd, uint sbc, u8 mcn, u32 cmd_data);
uint qe_muram_alloc(uint size, uint align);
void *qe_muram_addr(uint offset);
int qe_get_snum(void);
void qe_put_snum(u8 snum);
void qe_init(uint qe_base);
void qe_reset(void);
void qe_assign_page(uint snum, uint para_ram_base);
int qe_set_brg(uint brg, uint rate);
int qe_set_mii_clk_src(int ucc_num);
int qe_upload_firmware(const struct qe_firmware *firmware);
struct qe_firmware_info *qe_get_firmware_info(void);
void ft_qe_setup(void *blob);
void qe_init(uint qe_base);
void qe_reset(void);

#ifdef CONFIG_U_QE
void u_qe_init(void);
int u_qe_upload_firmware(const struct qe_firmware *firmware);
void u_qe_resume(void);
int u_qe_firmware_resume(const struct qe_firmware *firmware,
			 qe_map_t *qe_immrr);
#endif

#endif /* __QE_H__ */
