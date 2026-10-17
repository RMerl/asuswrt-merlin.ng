#ifndef __GIC_H__
#define __GIC_H__

/* Register offsets for the ARM generic interrupt controller (GIC) */

#define GIC_DIST_OFFSET		0x1000
#define GIC_CPU_OFFSET_A9	0x0100
#define GIC_CPU_OFFSET_A15	0x2000

/* Distributor Registers */
#define GICD_CTLR		0x0000
#define GICD_TYPER		0x0004
#define GICD_IIDR		0x0008
#define GICD_STATUSR		0x0010
#define GICD_SETSPI_NSR		0x0040
#define GICD_CLRSPI_NSR		0x0048
#define GICD_SETSPI_SR		0x0050
#define GICD_CLRSPI_SR		0x0058
#define GICD_SEIR		0x0068
#define GICD_IGROUPRn		0x0080
#define GICD_ISENABLERn		0x0100
#define GICD_ICENABLERn		0x0180
#define GICD_ISPENDRn		0x0200
#define GICD_ICPENDRn		0x0280
#define GICD_ISACTIVERn		0x0300
#define GICD_ICACTIVERn		0x0380
#define GICD_IPRIORITYRn	0x0400
#define GICD_ITARGETSRn		0x0800
#define GICD_ICFGR		0x0c00
#define GICD_IGROUPMODRn	0x0d00
#define GICD_NSACRn		0x0e00
#define GICD_SGIR		0x0f00
#define GICD_CPENDSGIRn		0x0f10
#define GICD_SPENDSGIRn		0x0f20
#define GICD_IROUTERn		0x6000

/* Cpu Interface Memory Mapped Registers */
#define GICC_CTLR		0x0000
#define GICC_PMR		0x0004
#define GICC_BPR		0x0008
#define GICC_IAR		0x000C
#define GICC_EOIR		0x0010
#define GICC_RPR		0x0014
#define GICC_HPPIR		0x0018
#define GICC_ABPR		0x001c
#define GICC_AIAR		0x0020
#define GICC_AEOIR		0x0024
#define GICC_AHPPIR		0x0028
#define GICC_APRn		0x00d0
#define GICC_NSAPRn		0x00e0
#define GICC_IIDR		0x00fc
#define GICC_DIR		0x1000

/* ReDistributor Registers for Control and Physical LPIs */
#define GICR_CTLR		0x0000
#define GICR_IIDR		0x0004
#define GICR_TYPER		0x0008
#define GICR_STATUSR		0x0010
#define GICR_WAKER		0x0014
#define GICR_SETLPIR		0x0040
#define GICR_CLRLPIR		0x0048
#define GICR_SEIR		0x0068
#define GICR_PROPBASER		0x0070
#define GICR_PENDBASER		0x0078
#define GICR_INVLPIR		0x00a0
#define GICR_INVALLR		0x00b0
#define GICR_SYNCR		0x00c0
#define GICR_MOVLPIR		0x0100
#define GICR_MOVALLR		0x0110

/* ReDistributor Registers for SGIs and PPIs */
#define GICR_IGROUPRn		0x0080
#define GICR_ISENABLERn		0x0100
#define GICR_ICENABLERn		0x0180
#define GICR_ISPENDRn		0x0200
#define GICR_ICPENDRn		0x0280
#define GICR_ISACTIVERn		0x0300
#define GICR_ICACTIVERn		0x0380
#define GICR_IPRIORITYRn	0x0400
#define GICR_ICFGR0		0x0c00
#define GICR_ICFGR1		0x0c04
#define GICR_IGROUPMODRn	0x0d00
#define GICR_NSACRn		0x0e00

/* Cpu Interface System Registers */
#define ICC_IAR0_EL1		S3_0_C12_C8_0
#define ICC_IAR1_EL1		S3_0_C12_C12_0
#define ICC_EOIR0_EL1		S3_0_C12_C8_1
#define ICC_EOIR1_EL1		S3_0_C12_C12_1
#define ICC_HPPIR0_EL1		S3_0_C12_C8_2
#define ICC_HPPIR1_EL1		S3_0_C12_C12_2
#define ICC_BPR0_EL1		S3_0_C12_C8_3
#define ICC_BPR1_EL1		S3_0_C12_C12_3
#define ICC_DIR_EL1		S3_0_C12_C11_1
#define ICC_PMR_EL1		S3_0_C4_C6_0
#define ICC_RPR_EL1		S3_0_C12_C11_3
#define ICC_CTLR_EL1		S3_0_C12_C12_4
#define ICC_CTLR_EL3		S3_6_C12_C12_4
#define ICC_SRE_EL1		S3_0_C12_C12_5
#define ICC_SRE_EL2		S3_4_C12_C9_5
#define ICC_SRE_EL3		S3_6_C12_C12_5
#define ICC_IGRPEN0_EL1		S3_0_C12_C12_6
#define ICC_IGRPEN1_EL1		S3_0_C12_C12_7
#define ICC_IGRPEN1_EL3		S3_6_C12_C12_7
#define ICC_SEIEN_EL1		S3_0_C12_C13_0
#define ICC_SGI0R_EL1		S3_0_C12_C11_7
#define ICC_SGI1R_EL1		S3_0_C12_C11_5
#define ICC_ASGI1R_EL1		S3_0_C12_C11_6

#endif /* __GIC_H__ */
