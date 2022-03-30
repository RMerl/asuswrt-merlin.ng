/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000
 * Rob Taylor, Flying Pig Systems. robt@flyingpig.com.
 */

 /* winbond access routines and defines*/

/* from the winbond data sheet -
 The W83C553F SIO controller with PCI arbiter is a multi-function PCI device.
 Function 0 is the ISA bridge, and Function 1 is the bus master IDE controller.
*/

/*ISA bridge configuration space*/

#define W83C553F_VID		0x10AD
#define W83C553F_DID		0x0565

#define WINBOND_PCICONTR	0x40  /*pci control reg*/
#define WINBOND_SGBAR		0x41  /*scatter/gather base address reg*/
#define WINBOND_LBCR		0x42  /*Line Buffer Control reg*/
#define WINBOND_IDEIRCR		0x43  /*IDE Interrupt Routing Control  Reg*/
#define WINBOND_PCIIRCR		0x44  /*PCI Interrupt Routing Control Reg*/
#define WINBOND_BTBAR		0x46  /*BIOS Timer Base Address Register*/
#define WINBOND_IPADCR		0x48  /*ISA to PCI Address Decoder Control Register*/
#define WINBOND_IRADCR		0x49  /*ISA ROM Address Decoder Control Register*/
#define WINBOND_IPMHSAR		0x4a  /*ISA to PCI Memory Hole STart Address Register*/
#define WINBOND_IPMHSR		0x4b  /*ISA to PCI Memory Hols Size Register*/
#define WINBOND_CDR			0x4c  /*Clock Divisor Register*/
#define WINBOND_CSCR		0x4d  /*Chip Select Control Register*/
#define WINBOND_ATSCR		0x4e  /*AT System Control register*/
#define WINBOND_ATBCR		0x4f  /*AT Bus ControL Register*/
#define WINBOND_IRQBEE0R	0x60  /*IRQ Break Event Enable 0 Register*/
#define WINBOND_IRQBEE1R	0x61  /*IRQ Break Event Enable 1 Register*/
#define WINBOND_ABEER		0x62  /*Additional Break Event Enable Register*/
#define WINBOND_DMABEER		0x63  /*DMA Break Event Enable Register*/

#define WINDOND_IDECSR		0x40  /*IDE Control/Status Register, Function 1*/

#define IPADCR_MBE512		0x1
#define IPADCR_MBE640		0x2
#define IPADCR_IPATOM4		0x10
#define IPADCR_IPATOM5		0x20
#define IPADCR_IPATOM6		0x40
#define IPADCR_IPATOM7		0x80

#define CSCR_UBIOSCSE		0x10
#define CSCR_BIOSWP			0x20

#define IDECSR_P0EN			0x01
#define IDECSR_P0F16		0x02
#define IDECSR_P1EN			0x10
#define IDECSR_P1F16		0x20
#define IDECSR_LEGIRQ		0x800

/*
 * Interrupt controller
 */
#define W83C553F_PIC1_ICW1	CONFIG_SYS_ISA_IO + 0x20
#define W83C553F_PIC1_ICW2	CONFIG_SYS_ISA_IO + 0x21
#define W83C553F_PIC1_ICW3	CONFIG_SYS_ISA_IO + 0x21
#define W83C553F_PIC1_ICW4	CONFIG_SYS_ISA_IO + 0x21
#define W83C553F_PIC1_OCW1	CONFIG_SYS_ISA_IO + 0x21
#define W83C553F_PIC1_OCW2	CONFIG_SYS_ISA_IO + 0x20
#define W83C553F_PIC1_OCW3	CONFIG_SYS_ISA_IO + 0x20
#define W83C553F_PIC1_ELC	CONFIG_SYS_ISA_IO + 0x4D0
#define W83C553F_PIC2_ICW1	CONFIG_SYS_ISA_IO + 0xA0
#define W83C553F_PIC2_ICW2	CONFIG_SYS_ISA_IO + 0xA1
#define W83C553F_PIC2_ICW3	CONFIG_SYS_ISA_IO + 0xA1
#define W83C553F_PIC2_ICW4	CONFIG_SYS_ISA_IO + 0xA1
#define W83C553F_PIC2_OCW1	CONFIG_SYS_ISA_IO + 0xA1
#define W83C553F_PIC2_OCW2	CONFIG_SYS_ISA_IO + 0xA0
#define W83C553F_PIC2_OCW3	CONFIG_SYS_ISA_IO + 0xA0
#define W83C553F_PIC2_ELC	CONFIG_SYS_ISA_IO + 0x4D1

#define W83C553F_TMR1_CMOD	CONFIG_SYS_ISA_IO + 0x43

/*
 * DMA controller
 */
#define W83C553F_DMA1	CONFIG_SYS_ISA_IO + 0x000	/* channel 0 - 3 */
#define W83C553F_DMA2	CONFIG_SYS_ISA_IO + 0x0C0	/* channel 4 - 7 */

/* command/status register bit definitions */

#define W83C553F_CS_COM_DACKAL	(1<<7)	/* DACK# assert level */
#define W83C553F_CS_COM_DREQSAL	(1<<6)	/* DREQ sense assert level */
#define W83C553F_CS_COM_GAP	(1<<4)	/* group arbitration priority */
#define W83C553F_CS_COM_CGE	(1<<2)	/* channel group enable */

#define W83C553F_CS_STAT_CH0REQ	(1<<4)	/* channel 0 (4) DREQ status */
#define W83C553F_CS_STAT_CH1REQ	(1<<5)	/* channel 1 (5) DREQ status */
#define W83C553F_CS_STAT_CH2REQ	(1<<6)	/* channel 2 (6) DREQ status */
#define W83C553F_CS_STAT_CH3REQ	(1<<7)	/* channel 3 (7) DREQ status */

#define W83C553F_CS_STAT_CH0TC	(1<<0)	/* channel 0 (4) TC status */
#define W83C553F_CS_STAT_CH1TC	(1<<1)	/* channel 1 (5) TC status */
#define W83C553F_CS_STAT_CH2TC	(1<<2)	/* channel 2 (6) TC status */
#define W83C553F_CS_STAT_CH3TC	(1<<3)	/* channel 3 (7) TC status */

/* mode register bit definitions */

#define W83C553F_MODE_TM_DEMAND	(0<<6)	/* transfer mode - demand */
#define W83C553F_MODE_TM_SINGLE	(1<<6)	/* transfer mode - single */
#define W83C553F_MODE_TM_BLOCK	(2<<6)	/* transfer mode - block */
#define W83C553F_MODE_TM_CASCADE	(3<<6)	/* transfer mode - cascade */
#define W83C553F_MODE_ADDRDEC	(1<<5)	/* address increment/decrement select */
#define W83C553F_MODE_AUTOINIT	(1<<4)	/* autoinitialize enable */
#define W83C553F_MODE_TT_VERIFY	(0<<2)	/* transfer type - verify */
#define W83C553F_MODE_TT_WRITE	(1<<2)	/* transfer type - write */
#define W83C553F_MODE_TT_READ	(2<<2)	/* transfer type - read */
#define W83C553F_MODE_TT_ILLEGAL	(3<<2)	/* transfer type - illegal */
#define W83C553F_MODE_CH0SEL	(0<<0)	/* channel 0 (4) select */
#define W83C553F_MODE_CH1SEL	(1<<0)	/* channel 1 (5) select */
#define W83C553F_MODE_CH2SEL	(2<<0)	/* channel 2 (6) select */
#define W83C553F_MODE_CH3SEL	(3<<0)	/* channel 3 (7) select */

/* request register bit definitions */

#define W83C553F_REQ_CHSERREQ	(1<<2)	/* channel service request */
#define W83C553F_REQ_CH0SEL	(0<<0)	/* channel 0 (4) select */
#define W83C553F_REQ_CH1SEL	(1<<0)	/* channel 1 (5) select */
#define W83C553F_REQ_CH2SEL	(2<<0)	/* channel 2 (6) select */
#define W83C553F_REQ_CH3SEL	(3<<0)	/* channel 3 (7) select */

/* write single mask bit register bit definitions */

#define W83C553F_WSMB_CHMASKSEL	(1<<2)	/* channel mask select */
#define W83C553F_WSMB_CH0SEL	(0<<0)	/* channel 0 (4) select */
#define W83C553F_WSMB_CH1SEL	(1<<0)	/* channel 1 (5) select */
#define W83C553F_WSMB_CH2SEL	(2<<0)	/* channel 2 (6) select */
#define W83C553F_WSMB_CH3SEL	(3<<0)	/* channel 3 (7) select */

/* read/write all mask bits register bit definitions */

#define W83C553F_RWAMB_CH0MASK	(1<<0)	/* channel 0 (4) mask */
#define W83C553F_RWAMB_CH1MASK	(1<<1)	/* channel 1 (5) mask */
#define W83C553F_RWAMB_CH2MASK	(1<<2)	/* channel 2 (6) mask */
#define W83C553F_RWAMB_CH3MASK	(1<<3)	/* channel 3 (7) mask */

/* typedefs */

#define W83C553F_DMA1_CS		0x8
#define W83C553F_DMA1_WR		0x9
#define W83C553F_DMA1_WSMB		0xA
#define W83C553F_DMA1_WM		0xB
#define W83C553F_DMA1_CBP		0xC
#define W83C553F_DMA1_MC		0xD
#define W83C553F_DMA1_CM		0xE
#define W83C553F_DMA1_RWAMB		0xF

#define W83C553F_DMA2_CS		0x10
#define W83C553F_DMA2_WR		0x12
#define W83C553F_DMA2_WSMB		0x14
#define W83C553F_DMA2_WM		0x16
#define W83C553F_DMA2_CBP		0x18
#define W83C553F_DMA2_MC		0x1A
#define W83C553F_DMA2_CM		0x1C
#define W83C553F_DMA2_RWAMB		0x1E

void initialise_w83c553f(void);
