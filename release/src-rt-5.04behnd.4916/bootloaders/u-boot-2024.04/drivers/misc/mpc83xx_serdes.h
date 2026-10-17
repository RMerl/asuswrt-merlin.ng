/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

/**
 * enum srdscr0_mask - Bit masks for SRDSCR0 (SerDes Control Register 0)
 * @SRDSCR0_DPPA:       Bitmask for the DPPA (diff pk-pk swing for lane A)
 *			field of the SRCSCR0
 * @SRDSCR0_DPPE:       Bitmask for the DPPE (diff pk-pk swing for lane E)
 *			field of the SRCSCR0
 * @SRDSCR0_DPP_1V2:    Combined bitmask to set diff pk-pk swing for both lanes
 * @SRDSCR0_TXEQA_MASK: Bitmask for the TXEQA (transmit equalization for
 *			lane A) field of the SRCSCR0
 * @SRDSCR0_TXEQA_SATA: Bitmask to set the TXEQA to the value used for SATA
 * @SRDSCR0_TXEQE_MASK: Bitmask for the TXEQE (transmit equalization for
 *			lane E) field of the SRCSCR0
 * @SRDSCR0_TXEQE_SATA: Bitmask to set the TXEQE to the value used for SATA
 */
#include <linux/bitops.h>
enum srdscr0_mask {
	SRDSCR0_DPPA        = BIT(31 - 16),
	SRDSCR0_DPPE        = BIT(31 - 20),
	SRDSCR0_DPP_1V2    = SRDSCR0_DPPE | SRDSCR0_DPPA,

	SRDSCR0_TXEQA_MASK = 0x00007000,
	SRDSCR0_TXEQA_SATA = 0x00001000,
	SRDSCR0_TXEQE_MASK = 0x00000700,
	SRDSCR0_TXEQE_SATA = 0x00000100,
};

/**
 * enum srdscr1_mask - Bit masks for SRDSCR1 (SerDes Control Register 1)
 * @SRDSCR1_PLLBW: Bitmask for the PLLBW (PLL bandwidth) field of SRDSCR1
 */
enum srdscr1_mask {
	SRDSCR1_PLLBW = BIT(31 - 25),
};

/**
 * enum srdscr2_mask - Bit masks for SRDSCR2 (SerDes Control Register 2)
 * @SRDSCR2_VDD_1V2:     Bit mask to to set the VDD field of the SCRSCR2
 * @SRDSCR2_SEICA_MASK:  Bitmask for the SEICA (Receiver electrical idle
 *			 detection control for lane A) field of the SRCSCR2
 * @SRDSCR2_SEICE_MASK:  Bitmask for the SEICE (Receiver electrical idle
 *			 detection control for lane E) field of the SRCSCR2
 * @SRDSCR2_SEIC_MASK:   Combined bitmask to set the receiver electrical idle
 *			 detection control for both lanes
 * @SRDSCR2_SEICA_SATA:  Bitmask to set the SEICA field to the value used for
 *			 SATA
 * @SRDSCR2_SEICE_SATA:  Bitmask to set the SEICE field to the value used for
 *			 SATA
 * @SRDSCR2_SEIC_SATA:   Combined bitmask to set the value of both SEIC fields
 *			 to the value used for SATA
 * @SRDSCR2_SEICA_PEX:   Bitmask to set the SEICA field to the value used for
 *			 PCI Express
 * @SRDSCR2_SEICE_PEX:   Bitmask to set the SEICE field to the value used for
 *			 PCI Express
 * @SRDSCR2_SEIC_PEX:    Combined bitmask to set the value of both SEIC fields
 *			 to the value used for PCI Express
 * @SRDSCR2_SEICA_SGMII: Bitmask to set the SEICA field to the value used for
 *			 SGMII
 * @SRDSCR2_SEICE_SGMII: Bitmask to set the SEICE field to the value used for
 *			 SGMII
 * @SRDSCR2_SEIC_SGMII:  Combined bitmask to set the value of both SEIC fields
 *			 to the value used for SGMII
 */
enum srdscr2_mask {
	SRDSCR2_VDD_1V2     = 0x00800000,

	SRDSCR2_SEICA_MASK  = 0x00001c00,
	SRDSCR2_SEICE_MASK  = 0x0000001c,
	SRDSCR2_SEIC_MASK   = SRDSCR2_SEICA_MASK | SRDSCR2_SEICE_MASK,

	SRDSCR2_SEICA_SATA  = 0x00001400,
	SRDSCR2_SEICE_SATA  = 0x00000014,
	SRDSCR2_SEIC_SATA   = SRDSCR2_SEICA_SATA | SRDSCR2_SEICE_SATA,

	SRDSCR2_SEICA_PEX   = 0x00001000,
	SRDSCR2_SEICE_PEX   = 0x00000010,
	SRDSCR2_SEIC_PEX    = SRDSCR2_SEICA_PEX | SRDSCR2_SEICE_PEX,

	SRDSCR2_SEICA_SGMII = 0x00000100,
	SRDSCR2_SEICE_SGMII = 0x00000001,
	SRDSCR2_SEIC_SGMII  = SRDSCR2_SEICA_SGMII | SRDSCR2_SEICE_SGMII,
};

/**
 * enum srdscr3_mask - Bit masks for SRDSCR3 (SerDes Control Register 3)
 * @SRDSCR3_KFRA_SATA:      Bitmask to set the KFRA field of SRDSCR3 to the
 *			    value used by SATA
 * @SRDSCR3_KFRE_SATA:      Bitmask to set the KFRE field of SRDSCR3 to the
 *			    value used by SATA
 * @SRDSCR3_KFR_SATA:       Combined bitmask to set both KFR fields to the
 *			    value used by SATA
 * @SRDSCR3_KPHA_SATA:      Bitmask to set the KPHA field of SRDSCR3 to the
 *			    value used by SATA
 * @SRDSCR3_KPHE_SATA:      Bitmask to set the KPHE field of SRDSCR3 to the
 *			    value used by SATA
 * @SRDSCR3_KPH_SATA:       Combined bitmask to set both KPH fields to the
 *			    value used by SATA
 * @SRDSCR3_SDFMA_SATA_PEX: Bitmask to set the SDFMA field of SRDSCR3 to the
 *			    value used by SATA and PCI Express
 * @SRDSCR3_SDFME_SATA_PEX: Bitmask to set the SDFME field of SRDSCR3 to the
 *			    value used by SATA and PCI Express
 * @SRDSCR3_SDFM_SATA_PEX:  Combined bitmask to set both SDFM fields to the
 *			    value used by SATA and PCI Express
 * @SRDSCR3_SDTXLA_SATA:    Bitmask to set the SDTXLA field of SRDSCR3 to the
 *			    value used by SATA
 * @SRDSCR3_SDTXLE_SATA:    Bitmask to set the SDTXLE field of SRDSCR3 to the
 *			    value used by SATA
 * @SRDSCR3_SDTXL_SATA:     Combined bitmask to set both SDTXL fields to the
 *			    value used by SATA
 *
 * KFRA = 'Kfr' gain selection in the CDR for lane A
 * KFRE = 'Kfr' gain selection in the CDR for lane E
 * SDFMA = Bandwidth of digital filter for lane A
 * SDFME = Bandwidth of digital filter for lane E
 * SDTXLA = Lane A transmitter amplitude levels
 * SDTXLE = Lane E transmitter amplitude levels
 */
enum srdscr3_mask {
	SRDSCR3_KFRA_SATA      = 0x10000000,
	SRDSCR3_KFRE_SATA      = 0x00100000,
	SRDSCR3_KFR_SATA       = SRDSCR3_KFRA_SATA | SRDSCR3_KFRE_SATA,

	SRDSCR3_KPHA_SATA      = 0x04000000,
	SRDSCR3_KPHE_SATA      = 0x00040000,
	SRDSCR3_KPH_SATA       = SRDSCR3_KPHA_SATA | SRDSCR3_KPHE_SATA,

	SRDSCR3_SDFMA_SATA_PEX = 0x01000000,
	SRDSCR3_SDFME_SATA_PEX = 0x00010000,
	SRDSCR3_SDFM_SATA_PEX  = SRDSCR3_SDFMA_SATA_PEX | SRDSCR3_SDFME_SATA_PEX,

	SRDSCR3_SDTXLA_SATA    = 0x00000500,
	SRDSCR3_SDTXLE_SATA    = 0x00000005,
	SRDSCR3_SDTXL_SATA     = SRDSCR3_SDTXLA_SATA | SRDSCR3_SDTXLE_SATA,
};

/**
 * enum srdscr4_mask - Bit masks for SRDSCR4 (SerDes Control Register 4)
 * @SRDSCR4_PROTA_SATA:  Bitmask to set the PROTA field of SRDSCR4 to the
 *			 value used by SATA
 * @SRDSCR4_PROTE_SATA:  Bitmask to set the PROTE field of SRDSCR4 to the
 *			 value used by SATA
 * @SRDSCR4_PROT_SATA:   Combined bitmask to set both PROT fields to the
 *			 value used by SATA
 * @SRDSCR4_PROTA_PEX:   Bitmask to set the PROTA field of SRDSCR4 to the
 *			 value used by PCI Express
 * @SRDSCR4_PROTE_PEX:   Bitmask to set the PROTE field of SRDSCR4 to the
 *			 value used by PCI Express
 * @SRDSCR4_PROT_PEX:    Combined bitmask to set both PROT fields to the
 *			 value used by PCI Express
 * @SRDSCR4_PROTA_SGMII: Bitmask to set the PROTA field of SRDSCR4 to the
 *			 value used by SGMII
 * @SRDSCR4_PROTE_SGMII: Bitmask to set the PROTE field of SRDSCR4 to the
 *			 value used by SGMII
 * @SRDSCR4_PROT_SGMII:  Combined bitmask to set both PROT fields to the
 *			 value used by SGMII
 * @SRDSCR4_PLANE_X2:    Bitmask to set the PLANE field of SRDSCR4
 * @SRDSCR4_RFCKS_100:   Bitmask to set the RFCKS field of SRDSCR4 to the
 *			 value 100Mhz
 * @SRDSCR4_RFCKS_125:   Bitmask to set the RFCKS field of SRDSCR4 to the
 *			 value 125Mhz
 * @SRDSCR4_RFCKS_150:   Bitmask to set the RFCKS field of SRDSCR4 to the
 *			 value 150Mhz
 *
 * PROTA = Lane A protocol select
 * PROTE = Lane E protocol select
 * PLAME = Number of PCI Express lanes
 */
enum srdscr4_mask {
	SRDSCR4_PROTA_SATA  = 0x00000800,
	SRDSCR4_PROTE_SATA  = 0x00000008,
	SRDSCR4_PROT_SATA   = SRDSCR4_PROTA_SATA | SRDSCR4_PROTE_SATA,

	SRDSCR4_PROTA_PEX   = 0x00000100,
	SRDSCR4_PROTE_PEX   = 0x00000001,
	SRDSCR4_PROT_PEX    = SRDSCR4_PROTA_PEX | SRDSCR4_PROTE_PEX,

	SRDSCR4_PROTA_SGMII = 0x00000500,
	SRDSCR4_PROTE_SGMII = 0x00000005,
	SRDSCR4_PROT_SGMII  = SRDSCR4_PROTA_SGMII | SRDSCR4_PROTE_SGMII,

	SRDSCR4_PLANE_X2    = 0x01000000,

	SRDSCR4_RFCKS_100 = (0 << 28),
	SRDSCR4_RFCKS_125 = (1 << 28),
	SRDSCR4_RFCKS_150 = (3 << 28),
};

/**
 * enum srdsrstctl_mask - Bit masks for SRDSRSTCTL (SerDes Reset Control Register)
 * @SRDSRSTCTL_RST:        Bitmask for the RST (Software reset) field of the
 *			   SRDSRSTCTL
 * @SRDSRSTCTL_SATA_RESET: Bitmask for the SATA_RESET (SATA reset) field of the
 *			   SRDSRSTCTL
 */
enum srdsrstctl_mask {
	SRDSRSTCTL_RST        = 0x80000000,
	SRDSRSTCTL_SATA_RESET = 0xf,
};

/**
 * struct mpc83xx_serdes_regs - Register map of the SerDes controller
 * @srdscr0:    SerDes Control Register 0
 * @srdscr1:    SerDes Control Register 1
 * @srdscr2:    SerDes Control Register 2
 * @srdscr3:    SerDes Control Register 3
 * @srdscr4:    SerDes Control Register 4
 * @fill0:      Reserved space in the register map
 * @srdsrstctl: SerDes Reset Control Register
 */
struct mpc83xx_serdes_regs {
	u32 srdscr0;
	u32 srdscr1;
	u32 srdscr2;
	u32 srdscr3;
	u32 srdscr4;
	u8 fill0[12];
	u32 srdsrstctl;
};

/**
 * enum pex_type - Types of PCI Express
 * @PEX_X1: PCI Express in x1 mode
 * @PEX_X2: PCI Express in x2 mode
 */
enum pex_type {
	PEX_X1,
	PEX_X2,
};
