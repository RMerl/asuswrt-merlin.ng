/*  *********************************************************************
    *
    <:copyright-BRCM:2015:proprietary:standard
    
       Copyright (c) 2015 Broadcom 
       All Rights Reserved
    
     This program is the proprietary software of Broadcom and/or its
     licensors, and may only be used, duplicated, modified or distributed pursuant
     to the terms and conditions of a separate, written license agreement executed
     between you and Broadcom (an "Authorized License").  Except as set forth in
     an Authorized License, Broadcom grants no license (express or implied), right
     to use, or waiver of any kind with respect to the Software, and Broadcom
     expressly reserves all rights in and to the Software and all intellectual
     property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
     NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
     BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
    
     Except as expressly set forth in the Authorized License,
    
     1. This program, including its structure, sequence and organization,
        constitutes the valuable trade secrets of Broadcom, and you shall use
        all reasonable efforts to protect the confidentiality thereof, and to
        use this information only in connection with your use of Broadcom
        integrated circuit products.
    
     2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
        AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
        WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
        RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
        ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
        FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
        COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
        TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
        PERFORMANCE OF THE SOFTWARE.
    
     3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
        ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
        INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
        WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
        IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
        OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
        SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
        SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
        LIMITED REMEDY.
    :>
    ********************************************************************* */

#include "arm.h"
#include "armmacros.h"
#include "bcm_map.h"


#define SI_ENUM_BASE		0x18000000    /* Enumeration space base */
#define	CC_EROMPTR		0xfc          /* little-endian */
#define CMEM_CORE_ID		0x846         /* CNDS DDR2/3 memory controller */

/* Erom entries */
#define	ER_TAG			0xe
#define	ER_TAG1			0x6
#define	ER_VALID		1
#define	ER_CI			0
#define	ER_MP			2
#define	ER_ADD			4
#define	ER_END			0xe
#define	ER_BAD			0xffffffff

/* EROM CompIdentA */
#define	CIA_MFG_MASK		0xfff00000
#define	CIA_MFG_SHIFT		20
#define	CIA_CID_MASK		0x000fff00
#define	CIA_CID_SHIFT		8
#define	CIA_CCL_MASK		0x000000f0
#define	CIA_CCL_SHIFT		4

/* EROM AddrDesc */
#define	AD_ADDR_MASK		0xfffff000
#define	AD_SP_MASK		0x00000f00
#define	AD_SP_SHIFT		8
#define	AD_ST_MASK		0x000000c0
#define	AD_ST_SHIFT		6
#define	AD_ST_SLAVE		0x00000000
#define	AD_ST_BRIDGE		0x00000040
#define	AD_ST_SWRAP		0x00000080
#define	AD_ST_MWRAP		0x000000c0
#define	AD_SZ_MASK		0x00000030
#define	AD_SZ_SHIFT		4
#define	AD_SZ_4K	   	0x00000000
#define	AD_SZ_8K	  	0x00000010
#define	AD_SZ_16K	  	0x00000020
#define	AD_SZ_SZD	  	0x00000030
#define	AD_AG32		  	0x00000008
#define	AD_ADDR_ALIGN		0x00000fff
#define	AD_SZ_BASE		0x00001000    /* 4KB */

/* chipid */
#define	CID_ID_MASK	0x0000ffff    /* Chip Id mask */
#define	CID_REV_MASK	0x000f0000    /* Chip Revision mask */
#define	CID_REV_SHIFT	16            /* Chip Revision shift */
#define	CID_PKG_MASK	0x00f00000    /* Package Option mask */
#define	CID_PKG_SHIFT	20            /* Package Option shift */
#define	CID_CC_MASK	0x0f000000    /* CoreCount (corerev >= 4) */
#define CID_CC_SHIFT	24
#define	CID_TYPE_MASK	0xf0000000    /* Chip Type */
#define CID_TYPE_SHIFT	28

/* From ddr_core.h */
#define	DDRC_CONTROL00 0x000
#define	DDRC_CONTROL25 0x064
#define	DDRC_CONTROL26 0x068
#define	DDRC_CONTROL60 0x0f0
#define	DDRC_CONTROL65 0x104

#define DDR_TABLE_END  0xffffffff
#define DDRC00_START   0x00000001


/* From hndsoc.h */
#define SI_BCM53573_NANDFLASH		0x30000000	/* 53573 NAND flash base */
#define SI_BCM53573_NORFLASH		0x1c000000	/* 53573 NOR flash base */

#define	SI_BCM53573_BOOTDEV_MASK    0x3
#define	SI_BCM53573_BOOTDEV_NOR	    0x0

/* A boot/binary may have an embedded block that describes its size  */
#define	BISZ_OFFSET                 0x3e0

#define	SI_BCM53573_DDRTYPE_MASK    0x10
#define	SI_BCM53573_DDRTYPE_DDR3    0x10

/*From bcmnvram.h */
#define NVRAM_MAGIC                 0x48534C46    /* 'FLSH' */

/*for cms NVRAM ulMemoryConfig offset 0x580+732*/
#define CMS_NVRAM_MEM_CFG_OFFSET 0x85c

/* from cms boardparams.h */
/*ulMemoryConfig*/
/*ddr device width, shift 5*/
#define CMS_DDR_DEVICE_WIDTH_16 1
#define CMS_DDR_DEVICE_WIDTH_32 2
/*ddr total device width, shift 18*/  
#define CMS_DDR_TOTAL_WIDTH_32BIT 1
/*enable half tREF,shift 20*/
#define CMS_DDR_HALF_TREF    0x100000   /*1 <<20*/
#define ENABLE_HALF_TREF     0x10000000
/*tRFC,shift 21*/
#define CMS_DDR_TRFC_MASK    0xe00000
#define CMS_DDR_TRFC_90NS    0x200000   /*1<<21*/
#define CMS_DDR_TRFC_110NS   0x400000   /*2<<21*/
#define CMS_DDR_TRFC_160NS   0x600000   /*3<<21*/
#define CMS_DDR_TRFC_260NS   0x800000   /*4<<21*/
#define CMS_DDR_TRFC_350NS   0xa00000   /*5<<21*/

/*caculate tRFC ,if want to set tRFC=160ns 
47189 is 533MHZ 
1/533M=1.87ns 
160/1.87=86=0x56   
register value is 0x56
*/

#define TRFC_90NS   0x310000            /*90ns 0x31 << 16*/
#define TRFC_110NS  0x3b0000            /*110ns 0x3b << 16*/
#define TRFC_160NS  0x560000            /*160ns 0x56 << 16*/
#define TRFC_260NS  0x8c0000            /*260ns 0x8c << 16*/
#define TRFC_350NS  0xbc0000            /*350ns 0xbc << 16*/


/*  *********************************************************************
    *  ddr_init
    *
    *  Initializes and configures the 47189 DDR memory controller.
    *
    *  Changed registers:
    *  	   r0-r12
    ********************************************************************* */
FUNC(ddr_init)
	/* Save lr */
	mov	r11, lr

	/* load CMEM_CORE_ID if CA7 */
	ldr	r4,=(CMEM_CORE_ID << CIA_CID_SHIFT)

	ldr	r0,=SI_ENUM_BASE		/* r0:	core regs SI base address */
	ldr	r1,=CC_EROMPTR
	ldr	r1,[r0,r1]			/* r1:	erom pointer */
1:
	ldr	r2,[r1]				/* r2:	erom entry */
	ldr	r3,=(ER_TAG | ER_VALID)
	and	r3,r3,r2
	cmp	r3,#(ER_END | ER_VALID)		/* Is this the END marker? */
	beq	panic				/* Yes: done and did not find ddr */

	cmp	r3,#(ER_CI | ER_VALID)
	beq	2f

	add	r1,r1,#4			/* Next erom entry */
	b	1b

panic: b	.

	/* Found a CIA */
2:
	add	r1,r1,#8			/* Skip both CIA & CIB */
	ldr	r3,=CIA_CID_MASK
	and	r3,r3,r2
	cmp	r3,r4
	bne	1b

	/* Found ddr, get regs and wrapper and save them */
	bl	ai_get_slave
	mov	r8,r0
	bl	ai_get_wrapper
	mov	r9,r0

	/* Initialize memory controller and size memory */
	mov	r0,r8
	mov	r1,r9

	ldr	r7, =0x18007400
	ldr	r12, [r7]		@ saved flag indicating DDR inited or not
	b	ddr_init_ca7

ret_tag:
	ldr	r0, =DDRC00_START
	and	r1, r12, r0
	cmp	r1, r0
	beq	ca7_cpu_boost_done	@ don't boost CPU again in 2nd init DDR round

ca7_cpu_boost_600MHz:
	SETLEDS('B','T','7','6')
	ldr	r0,=0x18003000			@ switch ClockRatioARM 1:1
	ldr	r1,[r0]
	bic	r1,r1,#(0x7<<9)
	str	r1,[r0]

	ldr	r0,=0x18003000			@ switch ClockRatioCCI 1:1
	ldr	r1,[r0]
	bic	r1,r1,#(0x3<<12)
	str	r1,[r0]

	ldr	r0,=0x18012660			@ set CPU Fvco to 1.8G
	ldr	r1,=0xe
	str	r1,[r0]
	ldr	r0,=0x18012664
	ldr	r1,=0x01680000
	str	r1,[r0]

	ldr	r0,=0x18012660			@ set CPU channels 2,1,0 mdiv to 12:6:3, and pdiv to 1
	ldr	r1,=0xd
	str	r1,[r0]
	ldr	r0,=0x18012664
	ldr	r1,=0x010c0603
	str	r1,[r0]

	ldr	r0,=0x18012660			@ set load_en for 3 channels
	ldr	r1,=0xc
	str	r1,[r0]
	ldr	r0,=0x18012664
	ldr	r1,[r0]
	orr	r1,r1,#0x7000
	str	r1,[r0]

	ldr	r0,=0x18012600			@ update PLL with Fvco settings
	ldr	r1,[r0]
	orr	r1,r1,#0x400
	str	r1,[r0]

	ldr	r0,=0x1800002c			@ wait until CPU PLL being locked
	ldr	r1,=0x1
1:	ldr	r2,[r0]
	and	r2,r2,r1
	cmp	r2,r1
	bne	1b

	ldr	r0,=0x18012660			@ clear load_en for 3 channels
	ldr	r1,=0xc
	str	r1,[r0]
	ldr	r0,=0x18012664
	ldr	r1,[r0]
	bic	r1,r1,#0x7000
	str	r1,[r0]

	ldr	r0,=0x18012600			@ set PLL control update bit
	ldr	r1,[r0]
	orr	r1,r1,#0x400
	str	r1,[r0]

	ldr	r0,=0x1800002c			@ wait until CPU PLL being locked
	ldr	r1,=0x1
1:	ldr	r2,[r0]
	and	r2,r2,r1
	cmp	r2,r1
	bne	1b

	ldr	r0,=0x18003000			@ boost CPU:CCI:NIC clock to 600:300:150
	ldr	r1,[r0]
	orr	r1,r1,#(0x1<<12)		@ switch ClockRatioCCI 2:1
	str	r1,[r0]
	ldr	r1,[r0]
	orr	r1,r1,#(0x1<<9)			@ switch ClockRatioArm 2:1
	str	r1,[r0]

ca7_cpu_boost_900MHz:
	SETLEDS('B','T','7','9')
	ldr	r0,=0x18003000			@ switch ClockRatioARM 1:1
	ldr	r1,[r0]
	bic	r1,r1,#(0x7<<9)
	str	r1,[r0]

	ldr	r0,=0x18003000			@ switch ClockRatioCCI 1:1
	ldr	r1,[r0]
	bic	r1,r1,#(0x3<<12)
	str	r1,[r0]

	ldr	r0,=0x18012660			@ set CPU channels 2,1,0 mdiv to 12:6:2, and pdiv to 1
	ldr	r1,=0xd
	str	r1,[r0]
	ldr	r0,=0x18012664
	ldr	r1,[r0]
	bic	r1,r1,#0xff
	orr	r1,r1,#0x2
	str	r1,[r0]

	ldr	r0,=0x18012660			@ set load_en_ch for ch0
	ldr	r1,=0xc
	str	r1,[r0]
	ldr	r0,=0x18012664
	ldr	r1,[r0]
	orr	r1,r1,#0x1000
	str	r1,[r0]

	ldr	r0,=0x18012600			@ set PLL control update bit
	ldr	r1,[r0]
	orr	r1,r1,#0x400
	str	r1,[r0]

	ldr	r0,=0x1800002c			@ wait until CPU PLL being locked
	ldr	r1,=0x1
1:	ldr	r2,[r0]
	and	r2,r2,r1
	cmp	r2,r1
	bne	1b

	ldr	r0,=SI_ENUM_BASE
	ldr	r1,[r0]
	ldr	r2,=CID_REV_MASK
	and	r1,r1,r2
	lsr	r1,r1,#CID_REV_SHIFT
	cmp	r1,#0x2				@ 53573B0 revid is 2
	bge	ca7_cpu_set_ratio		@ don't clear load_en if 53573B0

	ldr	r0,=0x18012660			@ clear load_en for ch0
	ldr	r1,=0xc
	str	r1,[r0]
	ldr	r0,=0x18012664
	ldr	r1,[r0]
	bic	r1,r1,#0x1000
	str	r1,[r0]

	ldr	r0,=0x18012600			@ set PLL control update bit
	ldr	r1,[r0]
	orr	r1,r1,#0x400
	str	r1,[r0]

	ldr	r0,=0x1800002c			@ wait until CPU PLL being locked
	ldr	r1,=0x1
1:	ldr	r2,[r0]
	and	r2,r2,r1
	cmp	r2,r1
	bne	1b

ca7_cpu_set_ratio:
	SETLEDS('R','T','O','7')
	ldr	r0,=0x18003000			@ boost CPU:CCI:NIC clock to 900:300:150
	ldr	r1,[r0]
	orr	r1,r1,#(0x1<<12)		@ switch ClockRatioCCI 2:1
	str	r1,[r0]
	ldr	r1,[r0]
	orr	r1,r1,#(0x2<<9)			@ switch ClockRatioArm 3:1
	str	r1,[r0]

ca7_cpu_boost_done:
	SETLEDS('B','T','7','D')

	SETLEDS('-','-','-','-')
	mov	lr, r11
	mov	pc, lr

ddr_init_ca7:
	mov	r9,r0			@ shift register base to 0x18007400
	add	r9,r9,#0x400		@ since 53573 DDR controller address shift 1K

chk_bootdev_53573:			@ to determine embedded nvram address
	SETLEDS('C','H','K','B')
	ldr	r1,=0x18010c40		@ get ddr type from bit[4] of
	ldr	r2,=0x7			@ gci chipstatus reg 7
	str	r2,[r1]
	ldr	r1,=0x18010e04
	ldr	r7,[r1]			@ save bootdev in r7

	ldr	r8,=SI_BCM53573_NANDFLASH	@ r0 = 0x30000000
	ldr	r3,=SI_BCM53573_BOOTDEV_MASK
	and	r4,r3,r7
	ldr	r3,=SI_BCM53573_BOOTDEV_NOR
	cmp	r4,r3
	bne	embedded_nv_53573

	SETLEDS('N','O','R',' ')
1:
	b	no_sdram_params_53573

	lsl	r1,r1,#1
	cmp	r1,r2
	ble	1b

	ldr	r8,=SI_BCM53573_NORFLASH	@ r0 = 0x1c000000

	/* Try embedded NVRAM at 4KB and 1KB as last resorts */
embedded_nv_53573:
	SETLEDS('N','A','N','D')
no_sdram_params_53573:
	/*No nvram, check CMS NVRAM*/
	SETLEDS('C','H','C','N')
	ldr	r3,=CMS_NVRAM_MEM_CFG_OFFSET
	add	r4,r8,r3
	ldr	r5,[r4]
	cmp	r5,#0xffffffff
	bne	cms_has_memcfg
	SETLEDS('D','R','N','S')/* sdram configuration not set*/
	mov	r4,#0
	b	cms_nv_default_sdram
cms_has_memcfg:	
	/*save r5 to r4,this should be useless if only 16bit and 32 bit is supported*/
	mov	r4,r5
	ldr	r3,=CMS_DDR_DEVICE_WIDTH_32
	lsl	r3,r3,#5
	and	r5,r5,r3
	cmp	r5,#0
	bne	cms_nv_32bit_sdram
	/*save r5 to r4,this should be useless if only 16bit and 32 bit is supported*/
	ldr	r3,=CMS_DDR_DEVICE_WIDTH_16
	lsl	r3,r3,#5
	mov	r5,r4
	and	r5,r5,r3
	cmp	r5,#0
	bne	cms_nv_16b_check_total_width
	/* No nvram, pick up some defaults */
	SETLEDS('D','R','D','F')
cms_nv_default_sdram:	
	SETLEDS('D','R','1','6')
	ldr	r6,=0x1c7		@ setup sdram_config to 0x1c7
	b	cms_nv_16bit_sdram
cms_nv_16b_check_total_width: /*16bit ddr, check total size*/
	SETLEDS('C','H','T','S')
	ldr	r3,=CMS_DDR_TOTAL_WIDTH_32BIT
	lsl	r3,r3,#18    
	mov	r5,r4
	and	r5,r5,r3
	cmp	r5,#0
	beq	cms_nv_default_sdram

cms_nv_32bit_sdram:
	SETLEDS('D','R','3','2')
	ldr	r6,=0x147		@ setup sdram_config to 0x147 32-bit-width

cms_nv_16bit_sdram:
	ldr	r3,=CMS_DDR_HALF_TREF
	and	r5,r4,r3
	cmp	r5,r3
	bne	cms_check_trfc_set
	SETLEDS('D','H','R','F')
	ldr	r3,=ENABLE_HALF_TREF
	add	r6,r3

cms_check_trfc_set:
	ldr	r3,=CMS_DDR_TRFC_MASK
	and	r5,r4,r3
	ldr	r3,=CMS_DDR_TRFC_90NS
	cmp	r5,r3
	bne	cms_check_trfc_110ns
	SETLEDS('R','F','9','0')
	ldr	r3,=TRFC_90NS
	add	r6,r3
	b	cms_check_trfc_end
cms_check_trfc_110ns:
	ldr	r3,=CMS_DDR_TRFC_110NS
	cmp	r5,r3
	bne	cms_check_trfc_160ns
	SETLEDS('R','1','1','0')
	ldr	r3,=TRFC_110NS
	add	r6,r3
	b	cms_check_trfc_end
cms_check_trfc_160ns:
	ldr	r3,=CMS_DDR_TRFC_160NS
	cmp	r5,r3
	bne	cms_check_trfc_260ns
	SETLEDS('R','1','6','0')
	ldr	r3,=TRFC_160NS
	add	r6,r3
	b	cms_check_trfc_end
cms_check_trfc_260ns:
	ldr	r3,=CMS_DDR_TRFC_260NS
	cmp	r5,r3
	bne	cms_check_trfc_350ns
	SETLEDS('R','2','6','0')
	ldr	r3,=TRFC_260NS
	add	r6,r3
	b	cms_check_trfc_end
cms_check_trfc_350ns:
	ldr	r3,=CMS_DDR_TRFC_350NS
	cmp	r5,r3
	bne	set_default_trfc_160ns
	SETLEDS('R','3','5','0')
	ldr	r3,=TRFC_350NS
	add	r6,r3
	b	cms_check_trfc_end
set_default_trfc_160ns:
	SETLEDS('R','1','6','0')
	ldr	r3,=TRFC_160NS
	add	r6,r3
cms_check_trfc_end:
	ldr	r5,=0x0
get_ddr_clock_53573:
	SETLEDS('G','C','L','K')
	ldr	r0,=DDRC00_START
	ldr	r1,[r9,#DDRC_CONTROL00]
	and	r1,r1,r0
	cmp	r1,r0
	bne	set_ddr_clock_default_53573	@ The first time

	SETLEDS('M','C','L','K')
	ldr	r1,=0x1000
	ldr	r0,=BISZ_OFFSET
	add	r1,r1,r0
	sub	r1,r1,#4
	ldr	r0,=0x0
	ldr	r2,[r1]			@ retrieve ddr clk from memory

	cmp	r2,r0
	beq	set_ddr_clock_default_53573	/* No ddrclk specified */
2:
	ldr	r3,=DDR_TABLE_END
	adrl	r4,ddr_clk_tab_53573
1:	ldr	r1,[r4]			 @r4 now hold ddr clock sequence entry
	cmp	r1,r3
	beq	set_ddr_clock_default_53573
	cmp	r1,r2
	beq	chg_ddr_pll_53573
	add	r4,r4,#16		@ table entry 16 bytes
	b	1b
set_ddr_clock_default_53573:
	SETLEDS('S','C','L','1')
	ldr	r2,=392			@ set r2 to 392MHz for ddr2

	ldr	r3,=533
	and	r0,r7,#SI_BCM53573_DDRTYPE_MASK		@ gci chipstatus 7 already in r7
	ldr	r1,=SI_BCM53573_DDRTYPE_DDR3
	cmp	r0,r1
	moveq	r2,r3			@ set r2 to 533MHz for ddr3
	b	2b			@ to find the clock entry

chg_ddr_pll_53573:
	mov	r10,r2			@ save DDR clock in r10
	add	r4,r4,#4		@ shift r4 to next entry

Adjust_Fvco:
	SETLEDS('A','F','V','C')
	ldr	r0,=0x18012660		@ switch to PMU PLL control 27 for setting nfrac
	ldr	r1,=27
	str	r1,[r0]
	ldr	r0,=0x18012664
	ldr	r1,[r4]
	str	r1,[r0]
	add	r4,r4,#4

	ldr	r0,=0x18012660		@ switch to PMU PLL control 26 for setting ndiv
	ldr	r1,=26
	str	r1,[r0]
	ldr	r0,=0x18012664
	ldr	r1,[r0]
	ldr	r2,=0x000fffff
	and	r1,r1,r2
	ldr	r2,[r4]
	orr	r1,r1,r2
	str	r1,[r0]
	add	r4,r4,#4

	ldr	r0,=0x18012600		@update PLL with Fvco settings
	ldr	r1,[r0]
	orr	r1,r1,#0x400
	str	r1,[r0]

	ldr	r0,=0x1800002c		@ wait until DDR PLL being locked
	ldr	r1,=0x4
1:	ldr	r2,[r0]
	and	r2,r2,r1
	cmp	r2,r1
	bne	1b

adjust_post_divisor:
	ldr	r0,=0x18012660		@ switch to PMU PLL control 25 for setting post-divisor
	ldr	r1,=25
	str	r1,[r0]
	ldr	r0,=0x18012664
	ldr	r1,[r0]
	ldr	r2,=0x0000ffff
	and	r1,r1,r2
	ldr	r2,[r4]
	orr	r1,r1,r2
	str	r1,[r0]

	ldr	r0,=0x18012664		@ load post-divisor setting
	ldr	r1,[r0]
	orr	r1,r1,#0x300
	str	r1,[r0]

	ldr	r0,=0x18012600		@ update PLL with Fvco settings
	ldr	r1,[r0]
	orr	r1,r1,#0x400
	str	r1,[r0]

	ldr	r0,=0x0			@ wait 7.5us for clock switch
	ldr	r1,=0x2000
1:	add	r0,r0,#1
	cmp	r0,r1
	ble	1b

	ldr	r0,=0x18012664		@ lock post-divisor setting
	ldr	r1,[r0]
	bic	r1,r1,#0x300
	str	r1,[r0]

	ldr	r0,=0x18012600		@ update PLL with post-divisor settings
	ldr	r1,[r0]
	orr	r1,r1,#0x400
	str	r1,[r0]

	ldr	r0,=0x1800002c		@ wait until DDR PLL being locked
	ldr	r1,=0x4
1:	ldr	r2,[r0]
	and	r2,r2,r1
	cmp	r2,r1
	bne	1b

reset_ddr_core_53573:
	SETLEDS('R','S','T','D')
	ldr	r0,=0x18107800		@ reset DDR controller and PHY for new frequency
	ldr	r1,=0x1
	str	r1,[r0]

	ldr	r0,=0			@ wait 10 cycles for reset complete
	ldr	r1,=100
1:	add	r0,r0,#1
	cmp	r0,r1
	ble	1b

	ldr	r0,=0x18107800		@ out of reset DDR controller and PHY
	ldr	r1,=0
	str	r1,[r0]

	ldr	r0,=0x4			@ test mem_rst_valid in 0x18007004[2] equal
	ldr	r2,=0x18007004		@ to 0, for reset completed
1:
	ldr	r1,[r2]
	and	r1,r1,r0
	cmp	r1,#0x0
	bne	1b

set_pad_on:				@ enable DDR read pad
	SETLEDS('P','D','O','N')
	ldr	r0,=0x18007048
	ldr	r1,=0x0
	str	r1,[r0]
	ldr	r0,=0x1800704c		@ enable pad, ODT resistor matching settings
	ldr	r1,=0x10101010
	str	r1,[r0]
	ldr	r0,=0x18007048		@ ODT resistor matching settings
	ldr	r1,=0x00000800
	str	r1,[r0]
	ldr	r0,=0x18007040		@ Turn on PVT controller
	ldr	r1,=0x80002001
	str	r1,[r0]

set_ddr_reg_53573:
	and	r8, r6,#0xf		@ cas latency saved in r8

	adrl	r0,ddr_init_tab_53573	@ call generic DDR ctrlr init
	bl	ddr_init_regs

	and	r0,r7,#SI_BCM53573_DDRTYPE_MASK		@ gci chipstatus 7 already in r7
	ldr	r1,=SI_BCM53573_DDRTYPE_DDR3
	cmp	r0,r1
	beq	set_ddr3_reg_53573
set_ddr2_reg_53573:
	adrl	r0,ddr2_init_tab_53573
	bl	ddr_init_regs

	ldr	r0,=392
	cmp	r0,r10			@ DDR clock already saved in r10
	beq	set_ddr2_392_reg_53573
	ldr	r0,=358
	cmp	r0,r10			@ DDR clock already saved in r10
	beq	set_ddr2_358_reg_53573
	ldr	r0,=318
	cmp	r0,r10			@ DDR clock already saved in r10
	beq	set_ddr2_318_reg_53573

set_ddr2_286_reg_53573:
	adrl	r0,ddr2_286_init_tab_53573
	bl	ddr_init_regs

	adrl	r0,ddr2_286_cl5_init_tab_53573
	adrl	r1,ddr2_286_cl6_init_tab_53573
	cmp	r8,#0x6
	moveq	r0,r1			@ r0 will point to either cl5/cl6 table eventually
	bl	ddr_init_regs

	b set_half_bus_width_53573
set_ddr2_318_reg_53573:
	adrl	r0,ddr2_318_init_tab_53573
	bl	ddr_init_regs

	adrl	r0,ddr2_318_cl5_init_tab_53573
	adrl	r1,ddr2_318_cl6_init_tab_53573
	cmp	r8,#0x6
	moveq	r0,r1			@ r0 will point to either cl5/cl6 table eventually
	bl	ddr_init_regs

	b set_half_bus_width_53573
set_ddr2_358_reg_53573:
	adrl	r0,ddr2_358_init_tab_53573	@ ddr2 358 support cl6. merge table together
	bl	ddr_init_regs

	b set_half_bus_width_53573
set_ddr2_392_reg_53573:
	adrl	r0,ddr2_392_init_tab_53573	@ ddr2 392 support cl6. merge table together
	bl	ddr_init_regs

	b set_half_bus_width_53573
set_ddr3_reg_53573:
	adrl	r0,ddr3_init_tab_53573
	bl	ddr_init_regs

	ldr	r0,=533
	cmp	r0,r10			@ DDR clock already saved in r10
	beq	set_ddr3_533_reg_53573
	ldr	r0,=392
	cmp	r0,r10			@ DDR clock already saved in r10
	beq	set_ddr3_392_reg_53573
	ldr	r0,=358
	cmp	r0,r10			@ DDR clock already saved in r10
	beq	set_ddr3_358_reg_53573

set_ddr3_318_reg_53573:
	adrl	r0,ddr3_318_init_tab_53573
	bl	ddr_init_regs

	adrl	r0,ddr3_318_cl5_init_tab_53573
	adrl	r1,ddr3_318_cl6_init_tab_53573
	cmp	r8,#0x6
	moveq	r0,r1			@ r0 will point to either cl5/cl6 table eventually
	bl	ddr_init_regs

	b set_half_bus_width_53573
set_ddr3_358_reg_53573:
	adrl	r0,ddr3_358_init_tab_53573	@ ddr3 358 only support cl6.  merge table together
	bl	ddr_init_regs

	b set_half_bus_width_53573
set_ddr3_392_reg_53573:
	adrl	r0,ddr3_392_init_tab_53573	@ ddr3 392 only support cl6.  merge table together
	bl	ddr_init_regs

	b set_half_bus_width_53573
set_ddr3_533_reg_53573:
	adrl	r0,ddr3_533_init_tab_53573
	bl	ddr_init_regs

	adrl	r0,ddr3_533_cl7_init_tab_53573
	adrl	r1,ddr3_533_cl8_init_tab_53573
	cmp	r8,#0x8
	moveq	r0,r1			@ r0 will point to either cl7/cl8 table eventually
	bl	ddr_init_regs

set_half_bus_width_53573:
	ldr	r1,[r9,#DDRC_CONTROL65]
	bic	r1,r1,#0x1			@ clear bit [0] by default(32bit)
	ldr	r2,=0x80
	and	r3,r6,r2
	cmp	r3,r2
	orreq	r1,r1,#0x1			@ set bit [0] for half width(16bit)
	str	r1,[r9,#DDRC_CONTROL65]
set_bank_53573:
	ldr	r1,[r9,#DDRC_CONTROL60]
	bic	r1,r1,#0x300			@ clear bit [9:8] for 8 banks
	ldr	r2,=0x40
	and	r3,r6,r2
	cmp	r3,r2
	orrne	r1,r1,#0x100			@ set bit [8] for 4 banks
	str	r1,[r9,#DDRC_CONTROL60]

set_col_53573:
	ldr	r1,[r9,#DDRC_CONTROL60]
	bic	r1,r1,#0x0f000000
	ldr	r2,=0x700
	and	r3,r6,r2
	lsl	r0,r3,#16
	orr	r1,r1,r0
	str	r1,[r9,#DDRC_CONTROL60]

set_tref_53573:
	ldr	r2,=0x10000000
	and	r2,r6,r2
	cmp	r2,#0
	beq	set_trfc_53573
	ldr	r1,[r9,#DDRC_CONTROL26]		@ half tref cycle
	ldr	r3,=0x3fff
	and	r2,r1,r3
	bic	r1,r1,r3
	lsr	r2,r2,#1
	sub	r3,r2,#3
	add	r1,r1,r3
	str	r1,[r9,#DDRC_CONTROL26]
set_trfc_53573:
	ldr	r2,=0x3ff0000
	and	r2,r6,r2
	cmp	r2,#0
	beq	start_ddr_53573
	ldr	r1,[r9,#DDRC_CONTROL25]
	ldr	r3,=0x3ff0000
	bic	r1,r1,r3
	add	r1,r1,r2
	str	r1,[r9,#DDRC_CONTROL25]		@ add counts to trfc

start_ddr_53573:
	ldr	r1,[r9,#DDRC_CONTROL00]
	orr	r1,r1,#DDRC00_START
	str	r1,[r9,#DDRC_CONTROL00]

wait_for_ddr_ready_53573:
	/* check 0x18007004 bit 7(mc_rdy_for_cmd) for AXI ready to start transaction */
	ldr	r0,=0x80
	ldr	r2,=0x18007004
1:
	ldr	r1,[r2]
	and	r1,r1,r0
	cmp	r1,r0
	bne	1b

turn_off_ODT_on_16bit_width:
	ldr	r2,=0x80
	and	r3,r6,r2
	cmp	r3,r2			@ value 1 means half width
	bne	ddr_init_completed_53573
	ldr	r0,=0x18007a0c		@ turning off ODT
	ldr	r1,=0x000dd000
	str	r1,[r0]

ddr_init_completed_53573:
	b ret_tag


	.ltorg

	.align 6
ddr_clk_tab_53573:	/* pllcontrol 27(nfrac[19:1]), 26(ndiv, nfrac[0]), 25(mdiv) */
	.word	286,	0x00054ccd,	0x01c00000,	0x10080000
	.word	318,	0x000714fe,	0x01700000,	0x0c060000
	.word	358,	0x0006f333,	0x81a00000,	0x0c060000
	.word	392,	0x00040000,	0x01800000,	0x0a050000
	.word	533,	0x00055555,	0x01a00000,	0x08040000
	.word	DDR_TABLE_END

	.align 6
ddr_init_tab_53573:
	.word	1,	0x00000000
	.word	2,	0x00000000
	.word	3,	0x00000000
	.word	4,	0x00000000
	.word	5,	0x00000000
	.word	22,	0x01000202
	.word	34,	0x00010000
	.word	37,	0x00000000
	.word	38,	0x00000000
	.word	40,	0x00200400
	.word	41,	0x01000400
	.word	42,	0x00000180
	.word	43,	0x00000000
	.word	44,	0x00000001
	.word	45,	0x00000000
	.word	46,	0x00000000
	.word	51,	0x00000000
	.word	52,	0x00010100
	.word	53,	0x00000000
	.word	54,	0x00000000
	.word	55,	0x00020000
	.word	56,	0x00400100
	.word	57,	0x01000200
	.word	58,	0x02000040
	.word	59,	0x00000040
	.word	60,	0x01000000
	.word	61,	0x01ffff0a
	.word	62,	0x01010101
	.word	63,	0x03010101
	.word	64,	0x0c000001
	.word	65,	0x00000000
	.word	66,	0x00010000
	.word	67,	0x00000000
	.word	68,	0x00000000
	.word	69,	0x00000000
	.word	70,	0x00000000
	.word	71,	0x00000000
	.word	72,	0x00000000
	.word	73,	0x00000000
	.word	74,	0x00000000
	.word	75,	0x00000000
	.word	76,	0x00000000
	.word	77,	0x00000000
	.word	81,	0x00000000
	.word	82,	0x00000000
	.word	83,	0x00000000
	.word	84,	0x0d000000
	.word	85,	0x00000028
	.word	86,	0x00000000
	.word	87,	0x00000000
	.word	88,	0x00000000
	.word	89,	0x00000000
	.word	90,	0x00000000
	.word	91,	0x00000000
	.word	92,	0x00000000
	.word	93,	0x00000000
	.word	94,	0x00000000
	.word	95,	0x00000000
	.word	96,	0x00000000
	.word	97,	0x00000000
	.word	98,	0x00000000
	.word	99,	0x00000000
	.word	100,	0x00000202
	.word	101,	0x00000000
	.word	102,	0x00000000
	.word	103,	0x00000000
	.word	104,	0x00000000
	.word	105,	0x00000000
	.word	106,	0xff03ff00
	.word	108,	0x00000000
	.word	110,	0x02000200
	.word	114,	0x02000200
	.word	115,	0x02000200
	.word	119,	0x000a0301
	.word	120,	0x0000000d
	.word	121,	0x00000000
	.word	122,	0x00000000
	.word	123,	0x00000000
	.word	124,	0x00000a00
	.word	125,	0x00000000
	.word	126,	0x000d0000
	.word	127,	0x00000000
	.word	128,	0x00000000
	.word	129,	0x00000000
	.word	130,	0x00000000
	.word	131,	0x00000000
	.word	132,	0x00000000
	.word	133,	0x00000001
	.word	135,	0x00000101
	.word	136,	0x00000000
	.word	137,	0x00000000
	.word	138,	0x00000000
	.word	139,	0x00000000
	.word	140,	0x00000000
	.word	141,	0x00000000
	.word	142,	0x00000000
	.word	143,	0x00000000
	.word	144,	0x00000000
	.word	145,	0x00000000
	.word	146,	0x00000000
	.word	147,	0x00000000
	.word	148,	0x00000000
	.word	149,	0x00000000
	.word	150,	0x00000000
	.word	151,	0x00000000
	.word	153,	0x00010000
	.word	154,	0x00000100
	.word	155,	0x00000101
	.word	156,	0x00010000
	.word	157,	0x00000100
	.word	158,	0x00000000
	.word	159,	0x0003ffff
	.word	160,	0x00000000
	.word	161,	0x0003ffff
	.word	162,	0x00000000
	.word	163,	0x0003ffff
	.word	164,	0x00000000
	.word	165,	0x0003ffff
	.word	166,	0x00000000
	.word	167,	0x0003ffff
	.word	168,	0x00000000
	.word	169,	0x0003ffff
	.word	170,	0x00000000
	.word	171,	0x0003ffff
	.word	172,	0x00000000
	.word	173,	0x0003ffff
	.word	174,	0x00000000
	.word	175,	0x0003ffff
	.word	176,	0x00000000
	.word	177,	0x0003ffff
	.word	178,	0x00000000
	.word	179,	0x0003ffff
	.word	180,	0x00000000
	.word	181,	0x0003ffff
	.word	182,	0x00000000
	.word	183,	0x0003ffff
	.word	184,	0x00000000
	.word	185,	0x0003ffff
	.word	186,	0x00000000
	.word	187,	0x0003ffff
	.word	188,	0x00000000
	.word	189,	0x0303ffff
	.word	190,	0xffffffff
	.word	191,	0x000f000f
	.word	192,	0x00ffff03
	.word	193,	0x000fffff
	.word	194,	0x0003000f
	.word	195,	0xffffffff
	.word	196,	0x000f000f
	.word	197,	0x00ffff03
	.word	198,	0x000fffff
	.word	199,	0x0003000f
	.word	200,	0xffffffff
	.word	201,	0x000f000f
	.word	202,	0x00ffff03
	.word	203,	0x000fffff
	.word	204,	0x0003000f
	.word	205,	0xffffffff
	.word	206,	0x000f000f
	.word	207,	0x00ffff03
	.word	208,	0x000fffff
	.word	209,	0x0003000f
	.word	210,	0xffffffff
	.word	211,	0x000f000f
	.word	212,	0x00ffff03
	.word	213,	0x000fffff
	.word	214,	0x0003000f
	.word	215,	0xffffffff
	.word	216,	0x000f000f
	.word	217,	0x00ffff03
	.word	218,	0x000fffff
	.word	219,	0x0003000f
	.word	220,	0xffffffff
	.word	221,	0x000f000f
	.word	222,	0x00ffff03
	.word	223,	0x000fffff
	.word	224,	0x0003000f
	.word	225,	0xffffffff
	.word	226,	0x000f000f
	.word	227,	0x00ffff03
	.word	228,	0x000fffff
	.word	256,	0x00000413
	.word	257,	0x00000415
	.word	258,	0x80010088
	.word	261,	0x00004040
	.word	262,	0x00000413
	.word	263,	0x00000415
	.word	264,	0x80010060
	.word	265,	0x00000003
	.word	266,	0x0912007f
	.word	267,	0x00004040
	.word	268,	0x00000000
	.word	269,	0x00000000
	.word	270,	0x00000000
	.word	271,	0x00000000
	.word	272,	0x00000000
	.word	273,	0x0000818a
	.word	274,	0x0000818a
	.word	275,	0x00000000
	.word	276,	0x00000000
	.word	277,	0x4240f000
	.word	278,	0x4240f000
	.word	279,	0x00000000
	.word	280,	0x00000000
	.word	281,	0x00000000
	.word	282,	0x00000000
	.word	283,	0x00000000
	.word	284,	0x00000000
	.word	285,	0x00000000
	.word	286,	0x00000000
	.word	287,	0x00000000
	.word	288,	0x00000413
	.word	289,	0x00000415
	.word	290,	0x80010088
	.word	293,	0x00004040
	.word	294,	0x00000413
	.word	295,	0x00000415
	.word	296,	0x80010060
	.word	297,	0x00000003
	.word	298,	0x0912007f
	.word	299,	0x00004040
	.word	300,	0x00000000
	.word	301,	0x00000000
	.word	302,	0x00000000
	.word	303,	0x00000000
	.word	304,	0x00000000
	.word	305,	0x0000818a
	.word	306,	0x0000818a
	.word	307,	0x00000000
	.word	308,	0x00000000
	.word	309,	0x4240f000
	.word	310,	0x4240f000
	.word	311,	0x00000000
	.word	312,	0x00000000
	.word	313,	0x00000000
	.word	314,	0x00000000
	.word	315,	0x00000000
	.word	316,	0x00000000
	.word	317,	0x00000000
	.word	318,	0x00000000
	.word	319,	0x00000000
	.word	320,	0x00000413
	.word	321,	0x00000415
	.word	322,	0x80010088
	.word	325,	0x00004040
	.word	326,	0x00000413
	.word	327,	0x00000415
	.word	328,	0x80010060
	.word	329,	0x00000003
	.word	330,	0x0912007f
	.word	331,	0x00004040
	.word	332,	0x00000000
	.word	333,	0x00000000
	.word	334,	0x00000000
	.word	335,	0x00000000
	.word	336,	0x00000000
	.word	337,	0x0000818a
	.word	338,	0x0000818a
	.word	339,	0x00000000
	.word	340,	0x00000000
	.word	341,	0x4240f000
	.word	342,	0x4240f000
	.word	343,	0x00000000
	.word	344,	0x00000000
	.word	345,	0x00000000
	.word	346,	0x00000000
	.word	347,	0x00000000
	.word	348,	0x00000000
	.word	349,	0x00000000
	.word	350,	0x00000000
	.word	351,	0x00000000
	.word	352,	0x00000413
	.word	353,	0x00000415
	.word	354,	0x80010088
	.word	357,	0x00004040
	.word	358,	0x00000413
	.word	359,	0x00000415
	.word	360,	0x80010060
	.word	361,	0x00000003
	.word	362,	0x0912007f
	.word	363,	0x00004040
	.word	364,	0x00000000
	.word	365,	0x00000000
	.word	366,	0x00000000
	.word	367,	0x00000000
	.word	368,	0x00000000
	.word	369,	0x0000818a
	.word	370,	0x0000818a
	.word	371,	0x00000000
	.word	372,	0x00000000
	.word	373,	0x4240f000
	.word	374,	0x4240f000
	.word	375,	0x00000000
	.word	376,	0x00000000
	.word	377,	0x00000000
	.word	378,	0x00000000
	.word	379,	0x00000000
	.word	380,	0x00000000
	.word	381,	0x00000000
	.word	382,	0x00000000
	.word	383,	0x00000000
	.word	384,	0x00000000
	.word	387,	0x00000000
	.word	DDR_TABLE_END

	.align 6
ddr2_init_tab_53573:
	.word	0,	0x00000400
	.word	7,	0x0000682b
	.word	11,	0x00000036
	.word	16,	0x02020608
	.word	19,	0x02020203
	.word	20,	0x03002475
	.word	27,	0x00000409
	.word	28,	0x00020002
	.word	29,	0x000a000a
	.word	30,	0x00080002
	.word	31,	0x00080002
	.word	33,	0x001300c8
	.word	35,	0x03030300
	.word	36,	0x00000003
	.word	39,	0x20000100
	.word	48,	0x00000040
	.word	50,	0x00000000
	.word	78,	0x03000000
	.word	116,	0x00000812
	.word	117,	0x0000285a
	.word	134,	0x00c800c8
	.word	152,	0x02020101
	.word	386,	0x00000005
	.word	DDR_TABLE_END

	.align 6
ddr2_286_init_tab_53573:
	.word	6,	0x0000dfdb
	.word	8,	0x0000003a
	.word	9,	0x00000090
	.word	10,	0x00007302
	.word	14,	0x0d120302
	.word	15,	0x020d0503
	.word	17,	0x04020306
	.word	18,	0x03004e59
	.word	21,	0x05050003
	.word	23,	0x03040a01
	.word	24,	0x00000306
	.word	25,	0x00250100
	.word	26,	0x001108b6
	.word	32,	0x002800c8
	.word	107,	0x0c0c0000
	.word	109,	0x0200116c
	.word	111,	0x116c0200
	.word	112,	0x0000571c
	.word	259,	0x00000003
	.word	260,	0x08120033
	.word	291,	0x00000003
	.word	292,	0x08120033
	.word	323,	0x00000003
	.word	324,	0x08120033
	.word	355,	0x00000003
	.word	356,	0x08120033
	.word	385,	0x00000005
	.word	DDR_TABLE_END

	.align 6
ddr2_286_cl5_init_tab_53573:
	.word	12,	0x00040a00
	.word	13,	0x0400040a
	.word	47,	0x00085300
	.word	49,	0x00400253
	.word	79,	0x01000503
	.word	80,	0x00000001
	.word	113,	0x08120405
	.word	118,	0x02020405
	.word	229,	0x0304000f
	.word	230,	0x00000304
	.word	DDR_TABLE_END

	.align 6
ddr2_286_cl6_init_tab_53573:
	.word	12,	0x00050c00
	.word	13,	0x0400050c
	.word	47,	0x00086300
	.word	49,	0x00400263
	.word	79,	0x02000503
	.word	80,	0x00000002
	.word	113,	0x08120506
	.word	118,	0x02020506
	.word	229,	0x0405000f
	.word	230,	0x00000405
	.word	DDR_TABLE_END

	.align 6
ddr2_318_init_tab_53573:
	.word	6,	0x0000f805
	.word	8,	0x00000040
	.word	9,	0x0000009f
	.word	10,	0x00007f02
	.word	14,	0x0f140402
	.word	15,	0x020f0503
	.word	17,	0x04020306
	.word	18,	0x030056ce
	.word	21,	0x05050003
	.word	23,	0x03040a01
	.word	24,	0x00000306
	.word	25,	0x00290100
	.word	26,	0x001109a8
	.word	32,	0x002c00c8
	.word	107,	0x0c0d0000
	.word	109,	0x02001350
	.word	111,	0x13500200
	.word	112,	0x00006090
	.word	259,	0x00000004
	.word	260,	0x0a12002d
	.word	291,	0x00000004
	.word	292,	0x0a12002d
	.word	323,	0x00000004
	.word	324,	0x0a12002d
	.word	355,	0x00000004
	.word	356,	0x0a12002d
	.word	385,	0x00000006
	.word	DDR_TABLE_END

	.align 6
ddr2_318_cl5_init_tab_53573:
	.word	12,	0x00040a00
	.word	13,	0x0400040a
	.word	47,	0x00085300
	.word	49,	0x00400253
	.word	79,	0x01000503
	.word	80,	0x00000001
	.word	113,	0x08120405
	.word	118,	0x02020405
	.word	229,	0x0304000f
	.word	230,	0x00000304
	.word	DDR_TABLE_END

	.align 6
ddr2_318_cl6_init_tab_53573:
	.word	12,	0x00050c00
	.word	13,	0x0400050c
	.word	47,	0x00086300
	.word	49,	0x00400263
	.word	79,	0x02000503
	.word	80,	0x00000002
	.word	113,	0x08120506
	.word	118,	0x02020506
	.word	229,	0x0405000f
	.word	230,	0x00000405
	.word	DDR_TABLE_END


	.align 6
ddr2_358_init_tab_53573:
	.word	6,	0x00011705
	.word	8,	0x00000048
	.word	9,	0x000000b3
	.word	10,	0x00008f02
	.word	12,	0x00050c00
	.word	13,	0x0400050c
	.word	14,	0x11160402
	.word	15,	0x02110603
	.word	17,	0x05020306
	.word	18,	0x030061a8
	.word	21,	0x06060003
	.word	23,	0x03040c01
	.word	24,	0x00000307
	.word	25,	0x002e0100
	.word	26,	0x00110ade
	.word	32,	0x003200c8
	.word	47,	0x000a6300
	.word	49,	0x00400263
	.word	79,	0x02000503
	.word	80,	0x00000002
	.word	107,	0x0c0d0000
	.word	109,	0x020015bc
	.word	111,	0x15bc0200
	.word	112,	0x00006cac
	.word	113,	0x08120506
	.word	118,	0x02020506
	.word	229,	0x0405000f
	.word	230,	0x00000405
	.word	259,	0x00000004
	.word	260,	0x0a120027
	.word	291,	0x00000004
	.word	292,	0x0a120027
	.word	323,	0x00000004
	.word	324,	0x0a120027
	.word	355,	0x00000004
	.word	356,	0x0a120027
	.word	385,	0x00000006
	.word	DDR_TABLE_END

	.align 6
ddr2_392_init_tab_53573:
	.word	6,	0x0001312d
	.word	8,	0x0000004f
	.word	9,	0x000000c4
	.word	10,	0x00009d02
	.word	12,	0x00050c00
	.word	13,	0x0400050c
	.word	14,	0x12180402
	.word	15,	0x02120603
	.word	17,	0x05020306
	.word	18,	0x03006acf
	.word	21,	0x06060003
	.word	23,	0x03040c01
	.word	24,	0x00000307
	.word	25,	0x00320100
	.word	26,	0x00110be3
	.word	32,	0x003600c8
	.word	47,	0x000a6300
	.word	49,	0x00400263
	.word	79,	0x02000503
	.word	80,	0x00000002
	.word	107,	0x0c0d0000
	.word	109,	0x020017c6
	.word	111,	0x17c60200
	.word	112,	0x000076de
	.word	113,	0x08120506
	.word	118,	0x02020506
	.word	229,	0x0405000f
	.word	230,	0x00000405
	.word	259,	0x00000004
	.word	260,	0x0a120024
	.word	291,	0x00000004
	.word	292,	0x0a120024
	.word	323,	0x00000004
	.word	324,	0x0a120024
	.word	355,	0x00000004
	.word	356,	0x0a120024
	.word	385,	0x00000006
	.word	DDR_TABLE_END

	.align 6
ddr3_init_tab_53573:
	.word	0,	0x00000600
	.word	7,	0x00000002
	.word	10,	0x00000000
	.word	11,	0x00000000
	.word	13,	0x0400060a
	.word	16,	0x02040507
	.word	17,	0x0c040407
	.word	19,	0x0c040403
	.word	20,	0x03002247
	.word	27,	0x000003c7
	.word	30,	0x00000000
	.word	31,	0x00000000
	.word	33,	0x000f0200
	.word	36,	0x00000005
	.word	39,	0x20020100
	.word	49,	0x00030220
	.word	50,	0x00000008
	.word	79,	0x00000605
	.word	80,	0x00000000
	.word	107,	0x0c0d0000
	.word	116,	0x0000078e
	.word	117,	0x000025c6
	.word	118,	0x02020605
	.word	134,	0x02000200
	.word	152,	0x03020101
	.word	230,	0x00000505
	.word	259,	0x00000004
	.word	291,	0x00000004
	.word	323,	0x00000004
	.word	355,	0x00000004
	.word	385,	0x00001006
	.word	386,	0x00001005
	.word	DDR_TABLE_END

	.align 6
ddr3_318_init_tab_53573:
	.word	6,	0x00000004
	.word	8,	0x0000f805
	.word	9,	0x00026c0b
	.word	14,	0x0c110404
	.word	15,	0x04100504
	.word	18,	0x0300570d
	.word	21,	0x05050003
	.word	23,	0x03070a01
	.word	24,	0x00000205
	.word	25,	0x00230100
	.word	26,	0x000e09a4
	.word	28,	0x00030003
	.word	29,	0x000a000a
	.word	32,	0x00270200
	.word	35,	0x05050500
	.word	48,	0x00000046
	.word	78,	0x04000000
	.word	109,	0x02001348
	.word	111,	0x13480200
	.word	112,	0x00006068
	.word	260,	0x0a12002d
	.word	292,	0x0a12002d
	.word	324,	0x0a12002d
	.word	356,	0x0a12002d
	.word	DDR_TABLE_END

	.align 6
ddr3_318_cl5_init_tab_53573:
	.word	12,	0x00050a00
	.word	47,	0x00021000
	.word	113,	0x078e0505
	.word	229,	0x0404000f
	.word	DDR_TABLE_END

	.align 6
ddr3_318_cl6_init_tab_53573:
	.word	12,	0x00050c00
	.word	47,	0x00022000
	.word	113,	0x078e0506
	.word	229,	0x0405000f
	.word	DDR_TABLE_END

	.align 6
ddr3_358_init_tab_53573:
	.word	6,	0x00000004
	.word	8,	0x00011705
	.word	9,	0x0002b98c
	.word	12,	0x00050c00
	.word	14,	0x0e130404
	.word	15,	0x04120504
	.word	18,	0x030061ef
	.word	21,	0x06050003
	.word	23,	0x03070b01
	.word	24,	0x00000205
	.word	25,	0x00280100
	.word	26,	0x000e0ad9
	.word	28,	0x00030003
	.word	29,	0x000a000a
	.word	32,	0x002b0200
	.word	35,	0x05050500
	.word	47,	0x00042000
	.word	48,	0x00000046
	.word	78,	0x04000000
	.word	109,	0x020015b2
	.word	111,	0x15b20200
	.word	112,	0x00006c7a
	.word	113,	0x078e0506
	.word	229,	0x0405000f
	.word	260,	0x0a120027
	.word	292,	0x0a120027
	.word	324,	0x0a120027
	.word	356,	0x0a120027
	.word	DDR_TABLE_END

	.align 6
ddr3_392_init_tab_53573:
	.word	6,	0x00000004
	.word	8,	0x0001312d
	.word	9,	0x0002faf1
	.word	12,	0x00050c00
	.word	14,	0x0f140404
	.word	15,	0x04140604
	.word	18,	0x03006b1d
	.word	21,	0x06060003
	.word	23,	0x03070c01
	.word	24,	0x00000206
	.word	25,	0x002b0100
	.word	26,	0x000e0bde
	.word	28,	0x00030003
	.word	29,	0x000a000a
	.word	32,	0x002f0200
	.word	35,	0x05050500
	.word	47,	0x00042000
	.word	48,	0x00000046
	.word	78,	0x04000000
	.word	109,	0x020017bc
	.word	111,	0x17bc0200
	.word	112,	0x000076ac
	.word	113,	0x078e0506
	.word	229,	0x0405000f
	.word	260,	0x0a120024
	.word	292,	0x0a120024
	.word	324,	0x0a120024
	.word	356,	0x0a120024
	.word	DDR_TABLE_END

	.align 6
ddr3_533_init_tab_53573:
	.word	6,	0x00000006
	.word	8,	0x00019f8f
	.word	9,	0x00040ee6
	.word	14,	0x141b0604
	.word	15,	0x041b0704
	.word	18,	0x030091dc
	.word	21,	0x08070003
	.word	23,	0x03070f01
	.word	24,	0x00000207
	.word	25,	0x003b0100
	.word	26,	0x000e102c
	.word	28,	0x00030004
	.word	29,	0x000a000d
	.word	32,	0x00400200
	.word	35,	0x05060600
	.word	48,	0x00080044
	.word	78,	0x05000000
	.word	109,	0x02002058
	.word	111,	0x20580200
	.word	112,	0x0000a1b8
	.word	258,	0x800100a8
	.word	260,	0x0a12001b
	.word	290,	0x800100a8
	.word	292,	0x0a12001b
	.word	322,	0x800100a8
	.word	324,	0x0a12001b
	.word	354,	0x800100a8
	.word	356,	0x0a12001b
	.word	DDR_TABLE_END

	.align 6
ddr3_533_cl7_init_tab_53573:
	.word	12,	0x00060e00
	.word	47,	0x00083000
	.word	113,	0x078e0607
	.word	229,	0x0506000f
	.word	DDR_TABLE_END

	.align 6
ddr3_533_cl8_init_tab_53573:
	.word	12,	0x00061000
	.word	47,	0x00084000
	.word	113,	0x078e0608
	.word	229,	0x0507000f
	.word	DDR_TABLE_END
END(ddr_init)

FUNC(ddr_init_regs)
	mov	r2,r0
	ldr	r3,=DDR_TABLE_END
1:	ldr	r4,[r2]
	cmp	r4,r3
	beq	2f
	ldr	r5,[r2,#4]
	lsl	r4,r4,#2
	add	r1,r9,r4
	str	r5,[r1]
	add	r2,r2,#8
	b	1b
2:
	mov	pc,lr
END(ddr_init_regs)

/*
 * Find the first slave address for a core in the AI EROM
 * Input:
 *	r1 - pointer to EROM after CIB
 * Output:
 *	r0 - slave address
 * Changed:
 *	r0, r1, r2
 */
FUNC(ai_get_slave)
1:	ldr	r0,[r1]
	ldr	r2,=(ER_TAG | ER_VALID)
	add	r1,r1,#4			/* Next erom entry */
	and	r2,r2,r0
	cmp	r2,#(ER_ADD | ER_VALID)
	bne	1b

2:	ldr	r2,=AD_ADDR_MASK
	and	r0,r0,r2
	mov	pc, lr
END(ai_get_slave)

/*
 * Find the first wrapper address for a core in the AI EROM
 * Input:
 *	r1 - pointer to EROM after first slave ADD
 * Output:
 *	r0 - wrapper address
 * Changed:
 *	r0, r1, r2, r3
 */
FUNC(ai_get_wrapper)
1:	ldr	r0,[r1]
	ldr	r2,=(ER_TAG | ER_VALID)
	add	r1,r1,#4			/* Next erom entry */
	and	r2,r2,r0
	cmp	r2,#(ER_ADD | ER_VALID)
	bne	1b

	/* An address descriptor, is it a wrapper? */
	ldr	r2,=AD_ST_SWRAP			/* We test just for the SWRAP bit set, */
	tst	r0,r2				/*  that gets both Master and Slave */
	beq	1b				/*  wrappers. */

	/* It is, isolate address and return */
	ldr	r2,=AD_ADDR_MASK
	and	r0,r0,r2
	mov	pc, lr
END(ai_get_wrapper)
