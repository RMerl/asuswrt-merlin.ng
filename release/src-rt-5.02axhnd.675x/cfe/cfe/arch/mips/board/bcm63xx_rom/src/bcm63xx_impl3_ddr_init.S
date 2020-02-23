#include "sbmips.h"
#include "bsp_config.h"

#include "bcm_cpu.h"
#include "bcm_map.h"

#include "bcm_hwdefs.h"
#include "boardparms.h"
#include "mipsmacros.h"

#define DDR_TEST            1

/* Memory mapping table for different size DRAMs (64Mb, 128Mb, 256Mb, 512Mb) */
/* 6318 mem controller only support 4 banks, 16 bits word? */
    .globl  dram_map_table_x16
dram_map_table_x16:
    //64Mb 1Mwordsx4Banksx16Bits 8MB 12 bit row 8 bit col
  	_LONG_ 0x00000000, 0x00000003  //COL_BITS, sdram_space(total size)

    //128Mb 2Mwordsx4Banksx16Bits 16MB 12 bit row 9 bit col
  	_LONG_ 0x00000001, 0x00000004  //COL_BITS, sdram_space(total size)

    //256Mb 4Mwordsx4Banksx16Bits 32MB 13 bit row 9 bit col
  	_LONG_ 0x00000001, 0x00000005  //COL_BITS, sdram_space(total size)

    //512Mb 8Mwordsx4Banksx16Bits 64MB 13 bit row 10 bit col
  	_LONG_ 0x00000002, 0x00000006  //COL_BITS, sdram_space(total size)
	
#if defined(_BCM963381_)
     /* only support 2 512Mb x8 configuration, total 128MB */	
    .globl  dram_map_table_x8
dram_map_table_x8:
    //512Mb 16Mwordsx4Banksx8Bits 64MB 13 bit row 11 bit col
  	_LONG_ 0x00000003, 0x00000007  //COL_BITS, sdram_space(total size 128MB)
#endif
	
     /* RFC table for different clock strap values and memory size */
     /* calculation: 64ms/# of row/memory clock period/16. for example: */
     /* for 32MB ddr at 200MHz, 64ms/8k/1/200MHz/16=100 */
    .globl  tRFC_table
tRFC_table:
 	 //100MHz, 8MB to 64MB
 	 _LONG_ 0x61, 0x61, 0x30, 0x30

	 //200MHz, 8MB to 64MB
 	 _LONG_ 0xC3, 0xC3, 0x61, 0x61

#if defined(_BCM960333_)
 	 //167MHz, 8MB to 64MB
 	 _LONG_ 0xA2, 0xA2, 0x51, 0x51

 	 /* Not supported in BCM960333 */
 	 //125MHz, 8MB to 64MB
 	 _LONG_ 0x7A, 0x7A, 0x3D, 0x3D
#else
 	 //125MHz, 8MB to 64MB
 	 _LONG_ 0x7A, 0x7A, 0x3D, 0x3D

 	 //167MHz, 8MB to 64MB
 	 _LONG_ 0xA2, 0xA2, 0x51, 0x51
#endif

    .globl  ascii_num_table
ascii_num_table: 
    .byte   0x30    // 0
    .byte   0x31    // 1
    .byte   0x32    // 2
    .byte   0x33    // 3
    .byte   0x34    // 4
    .byte   0x35    // 5
    .byte   0x36    // 6
    .byte   0x37    // 7
    .byte   0x38    // 8
    .byte   0x39    // 9
    .byte   0x41    // A
    .byte   0x42    // B
    .byte   0x43    // C
    .byte   0x44    // D
    .byte   0x45    // E
    .byte   0x46    // F    


#define SETLEDS1(a,b,c,d)                    \
        li      a0,(((a)<<24)|((b)<<16)|((c)<<8)|(d)) ; \
        bal     board_setleds; \
        nop

/*  *********************************************************************
    *  BOARD_DRAMINIT
    *
    *  This routine should activate memory.
    *
    *  Input parameters:
    *       None
    *
    *  Return value:
    *       None
    *
    *  Registers used:
    *       can use all registers.
    ********************************************************************* */
LEAF(board_draminit)
        .set    noreorder

        move    s0,ra

        SETLEDS1('-','-','-', '-')
#if defined(_BCM963381_)
        /* increase 2.5V linear regulator output to 2.6V */
        li      t0, MISC_REG_BASE
        li      t1, 0x40027
        sw      t1, MISC_REGS_2P5V_LDO_CONTROL(t0)
        li      t1, 0x1
        sw      t1, MISC_REGS_2P5V_LDO_CONTROL_ENABLE(t0)

        /* wait 200us: at maximum 600MHz clock, 120000 loops are more than enough */
        li      t1, 120000
        xor     t0, t0
vlt_wait:
        add     t0, 1
        bne     t0, t1, vlt_wait
        nop

        /* turn off slew rate control on DDR DQ pin through PINMUX_PAD_CONTROL_17
           to PINMUX_PAD_CONTROL_20 */
        li      t0, PAD_CTL_BASE
        li      t1, 0x88888888
        sw      t1, 68(t0)
        sw      t1, 72(t0)
        sw      t1, 76(t0)
        sw      t1, 80(t0)
#endif

        SETLEDS1('P','H','Y','S')
        
/***** Load SDRAM Controller Base *************************************/
        li      t0, SDRAM_CTRL_BASE

#if !defined(_BCM963381_)	
        // Set up for arbitration is different for 6318
        // Enable high priority queues for Host and PHY MIPs
#if !defined(_BCM960333_)
        li      t1, (1 << 31) | 0x2
#else
        li      t1, 0
#endif
        sw      t1, SDR_CFG_PRI_CFG(t0) 

        /* Set up Port ID first */
#if !defined(_BCM960333_)
        li      t1, (0xFF << 8) | 0x9
        sw      t1, SDR_CFG_PID_SELECT0(t0) 
        li      t1, (0xFF << 8) | 0xA
        sw      t1, SDR_CFG_PID_SELECT1(t0) 

        /* Really Enable PID */
        li      t1, (1 << 31) | (0xFF << 8) | 0x9 // Host MIPS pid
        sw      t1, SDR_CFG_PID_SELECT0(t0) 
        li      t1, (1 << 31) | (0xFF << 8) | 0xA // Phy Mips pid
        sw      t1, SDR_CFG_PID_SELECT1(t0)         
#endif	
#endif   

/****** Check strap value to figure out SDRAM Clock Frequency and save in a1******/
#if defined(_BCM963381_)
        li      t1, MISC_BASE
        lw      a1, MISC_STRAP_BUS(t1)

        and     a1, MISC_STRAP_BUS_MEMC_FREQ_MASK        // Mask out strap bits
        srl     a1, MISC_STRAP_BUS_MEMC_FREQ_SHIFT


        SETLEDS1('P','H','Y','E')
        li      t1, MISC_BASE
        lw      t7, MISC_STRAP_BUS(t1)
        li      t6, MISC_STRAP_BUS_DDR_N_SDRAM_SELECT
        and     t6, t7                           // Get SDR Strap, 1=DDR1, 0=SDR
        bnez    t6, ddr1_timing                   // Jump if DDR1
        nop
#else
        li      t1, STRAP_BASE
        lw      a1, STRAP_OVERRIDE_BUS(t1)

        and     a1, STRAP_OVERRIDE_BUS_SDRC_MASK        // Mask out strap bits
        srl     a1, STRAP_OVERRIDE_BUS_SDRC_MASK_SHIFT


        SETLEDS1('P','H','Y','E')
        li      t1, STRAP_BASE
        lw      t7, STRAP_OVERRIDE_BUS(t1)
        li      t6, STRAP_OVERRIDE_BUS_SDR_DDR1_SELECT_MASK
        and     t6, t7                           // Get SDR Strap, 1=SDR, 0=DDR1
        beqz    t6, ddr1_timing                   // Jump if DDR1
        nop
#endif
        
sdr_timing:
        // Hardwared to MT48LCXM16A2-6 for now
        //  CKE       = 1
        //  CK_DIS    = 0
        //  DIS_BTERM = 0
        //  CAS_LAT   = 3
        //  tRDWR     = 3
        //  tRFC      = 10
        //  tWTR      = 6
        //  tWR       = 8
        //  tRRD      = 2
        //  tRP       = 2
        //  tRCD      = 2
        //  tRAS      = 6
        // turn on CKE at the same time 
        SETLEDS1('S','D','R',' ')

#if defined(_BCM963381_)
sdr_clk_100:
        bne     a1, 0, sdr_clk_150
        nop
        SETLEDS1('1','0','0','H')
        b       1f
        nop

sdr_clk_150:	
        bne     a1, 1, sdr_clk_133
        nop
        SETLEDS1('1','5','0','H')
        b       1f
        nop

sdr_clk_133:
        bne     a1, 2, sdr_clk_200
        nop
        SETLEDS1('1','3','3','H')
        b       1f
        nop

sdr_clk_200:
        SETLEDS1('2','0','0','H')
	
#else
sdr_clk_100:
        bne     a1, 0, sdr_clk_200
        nop
        SETLEDS1('1','0','0','H')
        b       1f
        nop

sdr_clk_200:
#if defined(_BCM960333_)
        bne     a1, 1, sdr_clk_167
#else
        bne     a1, 1, sdr_clk_125
#endif
        nop
        SETLEDS1('2','0','0','H')
        b       1f
        nop

#if defined(_BCM960333_)
sdr_clk_167:
        bne     a1, 2, sdr_clk_125
        nop
        SETLEDS1('1','6','7','H')
        b       1f
        nop

        /* Not supported in BCM960333 */
sdr_clk_125:
        SETLEDS1('1','2','5','H')

#else
sdr_clk_125:
        bne     a1, 2, sdr_clk_167
        nop
        SETLEDS1('1','2','5','H')
        b       1f
        nop

sdr_clk_167:
        SETLEDS1('1','6','7','H')
#endif
	
#endif
	
1:
        li      t2, 0x85d779b7
        sw      t2, SDR_CFG_TIMING_PARAM(t0)

sdr_init:
       // SDR init sequence. Micron calls for 2 REFRESH commands, but hynix datasheet demands 8
       // [0x00000080] = 0x00000400  :  Perform a PRECHARGE ALL command; and wait tRP clks
       // [0x00000090] = 0x00000400  :  Perform a AUTO REFRESH command; and wait tRP clks
       // [0x00000090] = 0x00000400  :  Perform a AUTO REFRESH command; and wait tRP clks
       // [0x00000090] = 0x00000400  :  Perform a AUTO REFRESH command; and wait tRP clks
       // [0x00000090] = 0x00000400  :  Perform a AUTO REFRESH command; and wait tRP clks
       // [0x00000090] = 0x00000400  :  Perform a AUTO REFRESH command; and wait tRP clks
       // [0x00000090] = 0x00000400  :  Perform a AUTO REFRESH command; and wait tRP clks
       // [0x00000090] = 0x00000400  :  Perform a AUTO REFRESH command; and wait tRP clks
       // [0x00000090] = 0x00000400  :  Perform a AUTO REFRESH command; and wait tRP clks
       // [0x000000a0] = 0x00000032  :  Program LMR reg; and wait tMRD clks
       li       t1, 0x400
       sw       t1, SDR_PRECHARGE_CMD(t0)
       lw       t2, SDR_CFG_SDR_CFG(t0)
       // 8x AUTOREFRESH
       li       t1, 0x400
       sw       t1, SDR_AUTOREFRESH_CMD(t0)
       lw       t2, SDR_CFG_SDR_CFG(t0)
       li       t1, 0x400
       sw       t1, SDR_AUTOREFRESH_CMD(t0)
       lw       t2, SDR_CFG_SDR_CFG(t0)
       li       t1, 0x400
       sw       t1, SDR_AUTOREFRESH_CMD(t0)
       lw       t2, SDR_CFG_SDR_CFG(t0)
       li       t1, 0x400
       sw       t1, SDR_AUTOREFRESH_CMD(t0)
       lw       t2, SDR_CFG_SDR_CFG(t0)
       li       t1, 0x400
       sw       t1, SDR_AUTOREFRESH_CMD(t0)
       lw       t2, SDR_CFG_SDR_CFG(t0)
       li       t1, 0x400
       sw       t1, SDR_AUTOREFRESH_CMD(t0)
       lw       t2, SDR_CFG_SDR_CFG(t0)
       li       t1, 0x400
       sw       t1, SDR_AUTOREFRESH_CMD(t0)
       lw       t2, SDR_CFG_SDR_CFG(t0)
       li       t1, 0x400
       sw       t1, SDR_AUTOREFRESH_CMD(t0)
       lw       t2, SDR_CFG_SDR_CFG(t0)
       
       // hermanl should change to table lookup for value
       // write to LMR
       li       t1, 0x32
       sw       t1, SDR_LMR_CMD(t0)
       lw       t2, SDR_CFG_SDR_CFG(t0)
       b        dram_timing_cont1
       nop
       
ddr1_timing: 
       // CKE       = 1
       // CK_DIS    = 0
       // DIS_BTERM = 0
       // CAS_LAT   = 3
       // tRDWR     = 0
       // tRFC      = 12
       // tWTR      = 2
       // tWR       = 3
       // tRRD      = 2
       // tRP       = 2
       // tRCD      = 2
       // tRAS      = 6
       // turn on CKE at the same time 
       SETLEDS1('D','D','R','1')
#if defined(_BCM963381_)
ddr_clk_200:
        bne     a1, 0, ddr_clk_300
        nop
        SETLEDS1('2','0','0','H')
        b       1f
        nop

ddr_clk_300:
        bne     a1, 1, ddr_clk_266
        nop
        SETLEDS1('3','0','0','H')
        b       1f
        nop

ddr_clk_266:
        bne     a1, 2, ddr_clk_400
        nop
        SETLEDS1('2','6','6','H')
        b       1f
        nop

ddr_clk_400:
        SETLEDS1('4','0','0','H')
#else
ddr_clk_200:
        bne     a1, 0, ddr_clk_400
        nop
        SETLEDS1('2','0','0','H')
        b       1f
        nop

ddr_clk_400:
#if defined(_BCM960333_)
        bne     a1, 1, ddr_clk_333
#else
        bne     a1, 1, ddr_clk_250
#endif
        nop
        SETLEDS1('4','0','0','H')
        b       1f
        nop

#if defined(_BCM960333_)
ddr_clk_333:
        bne     a1, 2, ddr_clk_250
        nop
        SETLEDS1('3','3','3','H')
        b       1f
        nop

        /* Not supported in BCM960333 */
ddr_clk_250:
        SETLEDS1('2','5','0','H')
#else
ddr_clk_250:
        bne     a1, 2, ddr_clk_333
        nop
        SETLEDS1('2','5','0','H')
        b       1f
        nop

ddr_clk_333:
        SETLEDS1('3','3','3','H')
#endif
#endif
1:
       li      t2, 0x846249b7
       sw      t2, SDR_CFG_TIMING_PARAM(t0)
ddr1_init: 
       // DDR1init sequence. hardwired to MT46xxM16-5B, 200MHz
       // [0x00000080] = 0x00000400  :  Perform a PRECHARGE ALL command; and wait tRP clks
       // [0x000000a4] = 0x00000002  :  Program LMR extended mode reg; and wait tMRD clks
       // [0x000000a0] = 0x00000132  :  Program LMR reg; and wait tMRD clks
       // [0x00000080] = 0x00000400  :  Perform a PRECHARGE ALL command; and wait tRP clks
       // [0x00000090] = 0x00000400  :  Perform a AUTO REFRESH command; and wait tRP clks
       // [0x00000090] = 0x00000400  :  Perform a AUTO REFRESH command; and wait tRP clk
       li       t1, 0x400
       sw       t1, SDR_PRECHARGE_CMD(t0)
       lw       t2, SDR_CFG_SDR_CFG(t0)
#if defined(_BCM960333_)
       li       t1, 0
#else
       li       t1, 0x2 //reduced drive strength
#endif
       // hermanl note the LMR and LMRX may need to change to table lookup
       sw       t1, SDR_LMRX_CMD(t0)
       lw       t2, SDR_CFG_SDR_CFG(t0)
       li       t1, 0x132
       sw       t1, SDR_LMR_CMD(t0)
       lw       t2, SDR_CFG_SDR_CFG(t0)
       li       t1, 0x400
       sw       t1, SDR_PRECHARGE_CMD(t0)
       lw       t2, SDR_CFG_SDR_CFG(t0)
       // 2x AUTOREFRESH
       li       t1, 0x400
       sw       t1, SDR_AUTOREFRESH_CMD(t0)
       lw       t2, SDR_CFG_SDR_CFG(t0)
       li       t1, 0x400
       sw       t1, SDR_AUTOREFRESH_CMD(t0)
       lw       t2, SDR_CFG_SDR_CFG(t0)
       b        dram_timing_cont1

        
dram_timing_cont1:      
/*****  Run Read_En and Bit Calibrations, but we need to revert some programming first *****/

/***** Enable Auto refresh for memory size detecting******************************/
        // use worse case 100MHz, 8K row address
        // 64 ms / 8K / (1/100MHz) = 800 cycles = 50 * 16 cycles = 0x32 * 16
        li      t1, (1<<12) | 0x30
        sw      t1, SDR_CFG_AUTO_REFRESH(t0)

#if defined(_BCM963381_)
/* enable the MEMC */
	li      t4, MEMC_BASE
	lw      t5, MEMC_GLB_GCFG(t4)
	or      t5, (MEMC_GLB_GCFG_DRAM_EN_MASK|MEMC_GLB_GCFG_MEMINITDONE_MASK)
	sw      t5, MEMC_GLB_GCFG(t4)
#endif	
	
// Find memory size. a3 keeps the size: 0=64Mb, 1=128Mb, 2=256Mb, 3=512Mb, 4=1Gb
        LOADREL(t7, dram_map_table_x16)
#if defined(_BCM963381_)
        li      a3, 5     // Start from 512Mbx2 device
#else
        li      a3, 4     // Start from 512Mb device
#endif	
ddr_size:
        addi    a3, -1
        move	t1, t7
        move	a0, a3
        li	t7, 0x53495A30 // "SIZ0"
        add	a0, t7
        bal     board_setleds
        nop
        move	t7, t1

        li      t1, 0x8        // size of dram_map_table element
        mult    t1, a3
        mflo    t2              // dram_map_table offset
        add     t2, t7

        /* (size<<4) | (size<<8) | (col_its << 12) */
        lw      t1, 0x00(t2)        // COL_BITS
     	sll     t1, 12
     	lw      t3, 0x04(t2)        // sdram size
     	sll     t3, 4
     	or      t1, t3
#if !defined(_BCM963381_)	
     	lw      t3, 0x04(t2)        // sdram size
     	sll     t3, 8
     	or      t1, t3
#endif
        sw      t1, SDR_CFG_SDR_CFG(t0)

        beqz    a3, tRefi_update
        nop

// Check for memory aliasing
// This assumes that the bank interleaving is below address bits 8.
        li      t1, 8
        li      t2, 22
        add     t2, a3          // Scan up to maximum memory size of 64MB, check alias address at 32MB
        li      t3, 0xA0000000

check_alias:
        li      t4, 1
        sll     t4, t1
        add     t4, t3

        sw      zero, 0(t3)           // Clear address 0
        li      t5, -1
        sw      t5, 0(t4)             // Write the upper address location
        lw      t6, 0(t3)             // Read back address 0 to see whether we wrapped around


        beq     t5, t6, ddr_size
        nop

        bne     t1, t2, check_alias
        addi    t1, 1

tRefi_update:
	move     t3, a3
#if defined(_BCM963381_)
        bne      t3, 4, no_adj    // a3 is total size, for 128MB, we only support 64MBx2, tRFC based on per chip size
	addi     t3, -1
no_adj:
#endif	
        LOADREL(t7, tRFC_table)
        li      t1, 0x10        // size of tRFC_table element
        mult    t1, a1          // a1 is the clock index
        mflo    t2              // tRFC_table offset
        add     t2, t7
        sll     t1, t3, 2       // a3 is the memory size index
        add     t2, t1

        /* update auto refresh period with optimized value */
        lw      t1, 0x00(t2)    // refresh cycle number
        or      t1, (1<<12)
        sw      t1, SDR_CFG_AUTO_REFRESH(t0)

#if 0
/**----- Clear MIPS Async mode bit (for Fast Async Mode) -------**/
        mfc0    t1, C0_BCM_CONFIG, 5
        and     t1, ~(0x3 << 28)
        mtc0    t1, C0_BCM_CONFIG, 5

        SETLEDS1('M','F','A','S')
#endif

#if defined(_BCM963381_)
/* enable LMB full speed */
        li      t0, MIPS_BASE
        lw      t2, MIPS_LMB_CR(t0)
        or      t2, LMB_EN                              // Enable LMB
	and     t2, ~(LMB_SPEED_HALF<<LMB_SPEED_SHFT)
        sw      t2, MIPS_LMB_CR(t0)
        SETLEDS1('L','M','B','E')

/* work around for the LMB issue, use strict mode */
	li      t0, MEMC_BASE
	lw      t2, MEMC_LMBIF_0_REP_ARB_MODE(t0)
	or      t2, MEMC_LMBIF_0_REP_ARB_MODE_FIFO_MODE_STRICT
	sw      t2, MEMC_LMBIF_0_REP_ARB_MODE(t0)	
#endif

/* Make it 64MB only for 6318 */
/**----- Enable RAC -------------------------------------**/
        li      t0, MIPS_BASE
#if defined(_BCM963381_)
        li      t2, 0x7FF << RAC_UPB_SHFT               // Enable prefetch for RAM address range up to 128MB
#else
        li      t2, 0x3FF << RAC_UPB_SHFT               // Enable prefetch for RAM address range up to 64MB
#endif
        sw      t2, MIPS_RAC_ARR(t0)

        lw      t2, MIPS_RAC_CR0(t0)
        or      t2, (RAC_C_INV | RAC_I | RAC_PF_I)
        sw      t2, MIPS_RAC_CR0(t0)
#ifdef MIPS_RAC_CR1
        lw      t2, MIPS_RAC_CR1(t0)
        or      t2, (RAC_C_INV | RAC_I | RAC_PF_I)
        sw      t2, MIPS_RAC_CR1(t0)
#endif

        SETLEDS1('R','A','C','E')

/**----- Enable branch prediction and non-blocking data cache ---**/
#if defined(_BCM963381_)
        mfc0    t1, C0_BCM_CONFIG
        and     t1, ~CP0_BCM_CFG_BTHD
        or      t1, CP0_BCM_CFG_NBK
        or      t1, CP0_BCM_CFG_CLF
        mtc0    t1, C0_BCM_CONFIG	
#else
        mfc0    t1, C0_BCM_CONFIG
#if defined(CP0_BCM_CFG_NBK)        
        or      t1, CP0_BCM_CFG_NBK
#endif        
//        or      t1, CP0_BCM_CFG_CLF // 6318 memory controller cannot support critical line first.
        mtc0    t1, C0_BCM_CONFIG

        mfc0    t1, C0_BCM_CONFIG, 5
        and     t1, ~CP0_BCM_CFG_BTHD
        mtc0    t1, C0_BCM_CONFIG, 5        
#endif

#define BACKGND_DATA            0xa5a5a5a5
#define TEST_DATA               0x5a5a5a5a
#define STARTING_ADDR           0xa0000000
#define TEST_BLOCK_SIZE	        0x2000
#define TEST_SPARSE_SIZE        0x10
#define TEST_SPARSE_SPACE       0x4000

/**----- perform basic memory test ---**/
/* First test a large block at the beginning of the memory. Then test a small trunk of data
in 512 to 4K sparse locations depending on the total memory size */
		/* size in a3 0 to 3 */
        move    t6, zero
        li      t1, TEST_BLOCK_SIZE
        li      t2, STARTING_ADDR
        move    t5, t2
        addu    t1, t2

        /* t2, t5 starting address
           t1  ending address*/
mem_subt:
        li      t3, BACKGND_DATA
fill_bgdata:
        sw      t3, 0(t2)
        addu    t2, 4
        bltu    t2, t1, fill_bgdata
        nop

        move    t2, t5
        li      t3, TEST_DATA
test_loop:
        sw      t3, 0(t2)
        lw      t4, 0(t2)
        bne     t3, t4, test_done
        nop
        addu    t2, 4
        bltu    t2, t1, test_loop
        nop

test_done:
        bltu    t2, t1, test_failed

        /* now test some sparse locations */
        bnez    t6, skip_init
        nop

        addu    t6, a3, 23
        li      t5, 1
        sll     t6, t5, t6
        addu    t6, STARTING_ADDR
        li      t5, STARTING_ADDR-TEST_SPARSE_SPACE

skip_init:
        li      t1, TEST_SPARSE_SIZE
        addu    t5, TEST_SPARSE_SPACE
        bgeu    t5, t6, test_passed
        nop
        move    t2, t5
        addu    t1, t2
        b       mem_subt
        nop

test_passed:
#if defined(_BCM963381_)
        /* setup the MEMC GLB register for size */
        li      t0, SDRAM_CTRL_BASE
        lw      t1, SDR_CFG_SDR_CFG(t0)
        srl     t2, t1, SDR_CFG_SDR_CFG_SDRAM_SPACE_SHIFT
        and     t2, t2, SDR_CFG_SDR_CFG_SDRAM_SPACE_MASK

        li      t0, MEMC_BASE
        lw      t1, MEMC_GLB_GCFG(t0)
        and     t1, t1, ~(MEMC_GLB_GCFG_DRAM_SIZE2_MASK<<MEMC_GLB_GCFG_DRAM_SIZE2_SHIFT)
        and     t1, t1, ~(MEMC_GLB_GCFG_DRAM_SIZE1_MASK<<MEMC_GLB_GCFG_DRAM_SIZE1_SHIFT)
        move    t3, t2
        sll     t2, t2, MEMC_GLB_GCFG_DRAM_SIZE2_SHIFT
        sll     t3, t3, MEMC_GLB_GCFG_DRAM_SIZE1_SHIFT
        or      t2, t2, t3
        or      t1, t1, t2
        sw      t1, MEMC_GLB_GCFG(t0)
#endif
        SETLEDS1('P','A','S','S')
        b   1f
        nop

test_failed:
        SETLEDS1('F','A','I','L')
        b   1f
        nop
1:
        SETLEDS1('-','-','-', '-')

        move    ra,s0
        j       ra
        nop
    
        .set    reorder

END(board_draminit)
