#include "sbmips.h"
#include "bsp_config.h"

#include "bcm_cpu.h"
#include "bcm_map.h"

#include "bcm_hwdefs.h"
#include "boardparms.h"
#include "mipsmacros.h"

#define DDR_TEST            1
#define UBUS_SYNC_ENABLE    1
#define LMB_SYNC_ENABLE     1
#define MIPS_SYNC_ENABLE    1
#define LMB_ENABLE          1

#define SILICON             1

/* Memory mapping table for different size DRAMs (256Mb, 512Mb, 1Gb, 2Gb) */
      .globl  dram_map_table_x8
dram_map_table_x8:
    // 128Mb 64B Interleaving (x8 Mode) 32MB
    // This is just a place holder. This memory does not exist
    _LONG_  0x0F0E0D0C, 0x13121110, 0x17161514, 0x00000018  // Row00_0 Row00_1 Row01_0 Row01_1
    _LONG_  0x04000000, 0x0A090805, 0x0000000B, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    _LONG_  0x00000706, 0x00000002, 0x00000055              // Bank CS_End Dramsize

    // 256Mb 64B Interleaving (x8 Mode) 64MB
    _LONG_  0x100F0E0D, 0x14131211, 0x18171615, 0x00000019  // Row00_0 Row00_1 Row01_0 Row01_1
    _LONG_  0x04000000, 0x0A090805, 0x00000C0B, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    _LONG_  0x00000706, 0x00000004, 0x00000066              // Bank CS_End Dramsize
    
    // 512Mb 64B Interleaving (x8 Mode) 128MB
    _LONG_  0x100F0E0D, 0x14131211, 0x18171615, 0x00001A19  // Row00_0 Row00_1 Row01_0 Row01_1
    _LONG_  0x04000000, 0x0A090805, 0x00000C0B, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    _LONG_  0x00000706, 0x00000008, 0x00000077              // Bank CS_End Dramsize

    // 1Gb 64B Interleaving (x8 Mode) 256MB
    _LONG_  0x11100F0E, 0x15141312, 0x19181716, 0x00001B1A  // Row00_0 Row00_1 Row01_0 Row01_1
    _LONG_  0x04000000, 0x0B0A0905, 0x00000D0C, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    _LONG_  0x00080706, 0x00000010, 0x00000088              // Bank CS_End Dramsize

    // 2Gb 64B Interleaving (x8 Mode) 512MB
    _LONG_  0x11100F0E, 0x15141312, 0x19181716, 0x001C1B1A  // Row00_0 Row00_1 Row01_0 Row01_1
    _LONG_  0x04000000, 0x0B0A0905, 0x00000D0C, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    _LONG_  0x00080706, 0x00000020, 0x00000099              // Bank CS_End Dramsize

      .globl  dram_map_table_x16
dram_map_table_x16:
    // 256Mb 64B Interleaving (x16 Mode) 32MB
    _LONG_  0x0F0E0D0C, 0x13121110, 0x17161514, 0x00000018  // Row00_0 Row00_1 Row01_0 Row01_1
    _LONG_  0x04000000, 0x0A090805, 0x0000000B, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    _LONG_  0x00000706, 0x00000002, 0x00000055              // Bank CS_End Dramsize
    
    // 512Mb 64B Interleaving (x16 Mode) 64MB
    _LONG_  0x100F0E0D, 0x14131211, 0x18171615, 0x00000019  // Row00_0 Row00_1 Row01_0 Row01_1
    _LONG_  0x04000000, 0x0A090805, 0x00000C0B, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    _LONG_  0x00000706, 0x00000004, 0x00000066              // Bank CS_End Dramsize

    // 1Gb 64B Interleaving (x16 Mode) 128MB
    _LONG_  0x11100F0E, 0x15141312, 0x19181716, 0x0000001A  // Row00_0 Row00_1 Row01_0 Row01_1
    _LONG_  0x04000000, 0x0B0A0905, 0x00000D0C, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    _LONG_  0x00080706, 0x00000008, 0x00000077              // Bank CS_End Dramsize

    // 2Gb 64B Interleaving (x16 Mode) 256MB
    _LONG_  0x11100F0E, 0x15141312, 0x19181716, 0x00001B1A  // Row00_0 Row00_1 Row01_0 Row01_1
    _LONG_  0x04000000, 0x0B0A0905, 0x00000D0C, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    _LONG_  0x00080706, 0x00000010, 0x00000088              // Bank CS_End Dramsize

    // 4Gb 64B Interleaving (x16 Mode) 512MB -- PLACEHOLDER -- NOT TESTED
    _LONG_  0x11100F0E, 0x15141312, 0x19181716, 0x001C1B1A  // Row00_0 Row00_1 Row01_0 Row01_1
    _LONG_  0x04000000, 0x0B0A0905, 0x00000D0C, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    _LONG_  0x00080706, 0x00000020, 0x00000099              // Bank CS_End Dramsize

    .globl  dram_tRefi_table
dram_tRefi_table:       /* Refresh Interval table for different Speed DRAMs (100-200, 300-333, 400) */
    _LONG_  0x23   // 160-266 (MHz)
    _LONG_  0x42   // 300-333 (MHz)
    _LONG_  0x50   // 360-400 (MHz)   // Changed this to cover 360MHz case

    .globl  ddr2_tRFC_table
ddr2_tRFC_table:        /* tRFC table for different size & Speed DRAMs (256Mb, 512Mb, 1Gb)/(200, 333, 400) */
//        160-266 333    400   (MHz)       
    _LONG_  0x14, 0x19,  0x1E // 256Mb (min 75ns)
    _LONG_  0x1C, 0x23,  0x2A // 512Mb (min 105ns)
    _LONG_  0x22, 0x2B,  0x33 // 1Gb (min 127.5ns)
    _LONG_  0x35, 0x42,  0x4F // 2Gb (min 197.5ns)
    _LONG_  0x58, 0x6E,  0x83 // 4Gb (min 327.5ns)

    .globl  ddr3_tRFC_table
ddr3_tRFC_table:        /* tRFC table for different size & Speed DRAMs (256Mb, 512Mb, 1Gb)/(200, 333, 400) */
//        160-266 333    400   (MHz)       
    _LONG_  0xFF, 0xFF, 0xFF   // 256Mb (Do Not Exist!!)
    _LONG_  0x18, 0x1E,  0x24  // 512Mb (min 90ns)    
    _LONG_  0x1E, 0x25,  0x2C  // 1Gb (min 110ns)
    _LONG_  0x2B, 0x36,  0x40  // 2Gb (min 160ns)
    _LONG_  0x50, 0x64,  0x78  // 4Gb (min 300ns)

    .globl  ddr2_timing_table
ddr2_timing_table:      /*  */
//          tRCD  tCL  tWR  tWL  tRP tRRD  tRC tFAW tW2R tR2W tR2R  tAL tRTP tW2W tWTR 
//          --------------------------------------------------------------------------------
    .byte   0x04,0x04,0x04,0x03,0x04,0x03,0x0F,0x0C,0x01,0x01,0x00,0x03,0x02,0x00,0x02,0x00 // 160-266MHz
    .byte   0x05,0x05,0x05,0x04,0x05,0x04,0x13,0x11,0x02,0x01,0x00,0x04,0x03,0x00,0x02,0x00 // 300-333MHz   
    .byte   0x05,0x05,0x06,0x04,0x05,0x05,0x18,0x12,0x02,0x01,0x00,0x04,0x03,0x00,0x02,0x00 // 400MHz
    
    .globl  ddr3_timing_table
ddr3_timing_table:      /*  */
//          tRCD  tCL  tWR  tWL  tRP tRRD  tRC tFAW tW2R tR2W tR2R  tAL tRTP tW2W tWTR 
//          --------------------------------------------------------------------------------
    .byte   0x03,0x04,0x03,0x03,0x03,0x02,0x11,0x00,0x01,0x02,0x00,0x02,0x02,0x00,0x02,0  // 160-266MHz 
    .byte   0x04,0x05,0x05,0x05,0x04,0x04,0x10,0x10,0x02,0x02,0x00,0x04,0x04,0x00,0x04,0  // 300-333MHz 
    .byte   0x06,0x06,0x06,0x05,0x06,0x04,0x15,0x14,0x02,0x02,0x00,0x05,0x04,0x00,0x04,0  // 400MHz (6/6/6)
	// 400MHz: (6-6-6) Micron 25, 25E, 187E, 187, 15F, 15E, 15
	//                 JEDEC 800D/E, 1066E/F/G, 1333G/H/J, 1600G/H/J/K

    .globl  dram_sync_table
dram_sync_table:        /* Bit vector table for Ubus sync modes and Lmb sync modes */
#if defined(_BCM963268_)
    _LONG_  0x0000FFFF, 0x00009690, 0xFFFFFFFF  // UBUS Sync , LMB Sync, Mips Fast Async
#endif
    .globl  dram_speed_table
dram_speed_table:       /* Memory Speed Table for different clock strap values */
                        /* 0=160-266Mhz 1=333MHz 2=400MHz */
#if defined(_BCM963268_)
    .byte   0       // 0x0
    .byte   0       // 0x1
    .byte   0       // 0x2
    .byte   0       // 0x3
    .byte   2       // 0x4
    .byte   2       // 0x5
    .byte   2       // 0x6
    .byte   2       // 0x7   // Make 400's tREFI slow enough for 360MHz
    .byte   1       // 0x8
    .byte   1       // 0x9
    .byte   1       // 0xa
    .byte   1       // 0xb
    .byte   2       // 0xc
    .byte   2       // 0xd
    .byte   2       // 0xe
    .byte   2       // 0xf
#elif defined(_BCM6818_)
    .byte   0   // 0x0
    .byte   1   // 0x1
    .byte   0   // 0x2
    .byte   0   // 0x3
    .byte   0   // 0x4
    .byte   1   // 0x5
    .byte   1   // 0x6
    .byte   1   // 0x7
    .byte   0   // 0x8
    .byte   1   // 0x9
    .byte   1   // 0xa
    .byte   2   // 0xb
    .byte   2   // 0xc
    .byte   2   // 0xd
    .byte   2   // 0xe
    .byte   2   // 0xf
    .byte   2   // 0x10
    .byte   2   // 0x11
    .byte   2   // 0x12
    .byte   2   // 0x13
    .byte   2   // 0x14
    .byte   0   // 0x15
    .byte   0   // 0x16
    .byte   1   // 0x17
    .byte   1   // 0x18
    .byte   1   // 0x19
    .byte   1   // 0x1a
    .byte   1   // 0x1b
    .byte   2   // 0x1c
    .byte   2   // 0x1d
    .byte   2   // 0x1e
    .byte   2   // 0x1f
#endif

#if defined(_BCM963268_)
    .globl strap_table
strap_table: 
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
        mtc0    zero,C0_TLBHI
	nop
        li      t2,0xffff << 13 // 256M pages
	nop
        mtc0    t2,C0_PGMASK          
	nop
	// global, valid, allow writes, non-cachable, low DDR alias
        li      t2,( 1 | 2 | 4 | (2 << 3)) | ((0x20000000 >> 12) << 6)
	nop
        mtc0    t2,C0_TLBLO0
	nop
	// global, valid, allow writes, non-cachable, high DDR
        li      t2,( 1 | 2 | 4 | (2 << 3)) | ((0x30000000 >> 12) << 6)
	nop
        mtc0    t2,C0_TLBLO1
	nop
        mtc0    zero,C0_INX // first TLB entry
        nop
        nop
        tlbwi


        SETLEDS1('P','H','Y','S')
        
/***** Load MEMC Base *************************************/
        li      t0, MEMC_BASE

/***** Disable Auto refresh ******************************/             
        li      t1, 0x1FC00                // Set tRefi to 0xFC for now
        sw      t1, MEMC_CTL_CLKS(t0)

        li      t1, 0x04000000
        sw      t1, PHY_CONTROL_REGS_ZQ_PVT_COMP_CTL(t0)
        li      t1, 0x200
/*
1:
        bnez    t1, 1b
        addi    t1, -1
        sw      zero, PHY_CONTROL_REGS_ZQ_PVT_COMP_CTL(t0)
*/

/****** Hardware calibrate VDL *******/         
        sw      zero, PHY_BYTE_LANE_0_VDL_CALIBRATE(t0)
        sw      zero, PHY_BYTE_LANE_1_VDL_CALIBRATE(t0)

        li      t1, 3
        sw      t1, PHY_BYTE_LANE_0_VDL_CALIBRATE(t0)
        sw      t1, PHY_BYTE_LANE_1_VDL_CALIBRATE(t0)

        li      t1, 0x10   // FIXME: May increase this for faster SPIM CLKs
1:      
        bnez    t1, 1b
        addi    t1, -1

/****** Check strap value to figure out DDR Clock Frequency ******/
        li      t2, MISC_BASE
        lw      t1, MISC_STRAP_BUS(t2)

        and     t1, MISC_STRAP_BUS_MIPS_PLL_FVCO_MASK        // Mask out strap bits
        srl     t1, MISC_STRAP_BUS_MIPS_PLL_FVCO_SHIFT

#if defined(_BCM963268_)
        LOADREL(t4, strap_table)
        add     t4, t1
        lb      a0, 0x00(t4)

        li      t3, 0x53545200 // print <STR><Setting>
        or      a0, t3
        bal board_setleds
	nop
#endif     

        LOADREL(t2, dram_speed_table)
  
        add     t2, t1
        lb      t1, 0x00(t2)
        beq     t1, zero, ddr_clk_266
        li      t2, 1
        beq     t1, t2, ddr_clk_300
	nop

/****** Software override rd_en VDL and ADDRESS VDL ********/
        // a1 keeps the DDR speed value: 0=160-266 1=333 2=400MHz
ddr_clk_400:          
        li      a1, 2           // Set speed to 400MHz (2)
        SETLEDS1('4','0','0','H')        
        b       1f
        nop
	
ddr_clk_300:
        li      a1, 1           // Set speed to 300MHz (1)
        SETLEDS1('3','0','0','H')        

/* At higher frequencies set Read_en VDL value to calibrated VDL value + 10 */          
1:      li      t2, 0x10000 
        lw      t1, PHY_BYTE_LANE_0_VDL_STATUS(t0)
        andi    t1, PHY_BYTE_LANE_0_VDL_STATUS_CALIB_TOTAL_STEP_MASK
        srl     t1, PHY_BYTE_LANE_0_VDL_STATUS_CALIB_TOTAL_STEP_STRT
        addi    t1, 0xA
        li      t3, PHY_BYTE_LANE_0_VDL_STATUS_CALIB_TOTAL_STEP_MAX
        ble     t1, t3, 1f
        nop
        li      t1, PHY_BYTE_LANE_0_VDL_STATUS_CALIB_TOTAL_STEP_MAX
1:
        or      t1, t1, t2
        sw      t1, PHY_BYTE_LANE_0_VDL_OVERRIDE_2(t0)
        lw      t1, PHY_BYTE_LANE_1_VDL_STATUS(t0)
        andi    t1, PHY_BYTE_LANE_0_VDL_STATUS_CALIB_TOTAL_STEP_MASK
        srl     t1, PHY_BYTE_LANE_0_VDL_STATUS_CALIB_TOTAL_STEP_STRT
        addi    t1, 0xA
        ble     t1, t3, 1f
        nop
        li      t1, PHY_BYTE_LANE_0_VDL_STATUS_CALIB_TOTAL_STEP_MAX
1:
        or      t1, t1, t2
        sw      t1, PHY_BYTE_LANE_1_VDL_OVERRIDE_2(t0)
        b       vdl_override_cont
        nop
        
ddr_clk_266:
        li      a1, 0           // Set speed to 266MHz (0)
        SETLEDS1('2','6','6','H')
        li      t3, PHY_BYTE_LANE_0_VDL_STATUS_CALIB_TOTAL_STEP_MAX  // Set maximum VDL step size for 200MHz
        b       1f
        nop

vdl_override_cont:      
        lw      t2, PHY_BYTE_LANE_0_VDL_STATUS(t0)
        andi    t3, t2, PHY_BYTE_LANE_0_VDL_STATUS_CALIB_TOTAL_STEP_MASK
        srl     t3, PHY_BYTE_LANE_0_VDL_STATUS_CALIB_TOTAL_STEP_STRT
        addi    t3, t3, 0x4     // Add Stepsize 4
        li      t1, PHY_BYTE_LANE_0_VDL_STATUS_CALIB_TOTAL_STEP_MAX
        ble     t3, t1, 1f
        nop
        li      t3, PHY_BYTE_LANE_0_VDL_STATUS_CALIB_TOTAL_STEP_MAX

1:      li      t1, 0x110000    // Enable & Force Override
        or      t1, t3          // Fine rise and fine fall are set to 0
        
        sw      t1, PHY_CONTROL_REGS_STATIC_VDL_OVERRIDE(t0)
        li      t1, 0x0c
        sw      t1, MEMC_CTL_DCMD(t0)    // Set VDL

/***** Set arbitor for Burst Round Robin Mode ***/
        lw      t1, MEMC_CTL_ARB(t0)
        or      t1, 4 << 16
        sw      t1, MEMC_CTL_ARB(t0)
        
/* Program MC Timing Registers

 Read each timing parameter based on the speed and then create the
 timing registers and program them. 
*/  
        
/****** Disable byte lane 1 clock ********/
        li      t1, 1
        sw      t1, PHY_BYTE_LANE_1_CLOCK_PAD_DISABLE(t0)

        SETLEDS1('P','H','Y','E')
        li      t1, MISC_BASE
        lw      t7, MISC_STRAP_BUS(t1)
        li      t6, MISC_STRAP_BUS_DDR2_DDR3_N_SELECT
        and     t6, t7                            // Get DDR2 Strap
        beqz    t6, ddr3_timing                   // Jump if DDR3
        nop

ddr2_timing:            
/****** Set control pad strength to half ********/
        lw      t2, PHY_CONTROL_REGS_DRIVE_PAD_CTL(t0)
        or      t2, 0x4
        sw      t2, PHY_CONTROL_REGS_DRIVE_PAD_CTL(t0)

/****** Change the slew and receiver power to full strength in the byte lanes ********/
        li      t1, 0xFFFFFFFC
        lw      t2, PHY_BYTE_LANE_0_DRIVE_PAD_CTL(t0)
        and     t2, t2, t1
        sw      t2, PHY_BYTE_LANE_0_DRIVE_PAD_CTL(t0)
        lw      t2, PHY_BYTE_LANE_1_DRIVE_PAD_CTL(t0)
        and     t2, t2, t1
        sw      t2, PHY_BYTE_LANE_1_DRIVE_PAD_CTL(t0)
        
        
        SETLEDS1('D','D','R','2')
        LOADREL(t2, ddr2_timing_table)
  
        li      t1, 0x10    // size of ddr2_timing_table element
        mult    t1, a1
        mflo    t1          // ddr2_timing_table offset
        add     t2, t1

        move    t3, zero
        lb      t1, 0x00(t2)    // tRCD
        andi    t1, 0xf
        move    t3, t1
        lb      t1, 0x01(t2)    // tCL
        andi    t1, 0xf
    
        move    t4, t1
        and     t4, 0x7
        sll     t4, 20
        sw      t4, MEMC_CTL_DMODE_0 (t0)   // Write tCL to the MRS register holder

        sll     t1, 4
        or      t3, t1
        lb      t1, 0x02(t2)    // tWR
        andi    t1, 0xf
    
        // Here we create the MRS register values
        move    t4, t1
        li      t5, 1
        subu    t4, t5            // tWR written to DRAM is 1 less than real tWR value
        andi    t4, 0x7
        sll     t4, 25
        li      t5, 0x01030000    // Sequential burst mode, burst of 8, reset DLL
        or      t4, t5
        lw      t5, MEMC_CTL_DMODE_0 (t0)  
        or      t5, t4
        sw      t5, MEMC_CTL_DMODE_0 (t0)  // Add tWR to the MRS register holder
    
        sll     t1, 8
        or      t3, t1
        lb      t1, 0x03(t2)    // tWL
        andi    t1, 0xf
        sll     t1, 12
        or      t3, t1
        lb      t1, 0x04(t2)    // tRP
        andi    t1, 0xf
        sll     t1, 16
        or      t3, t1
        lb      t1, 0x05(t2)    // tRRD
        andi    t1, 0xf
        sll     t1, 20
        or      t3, t1
        sw      t3, MEMC_CTL_TIM1_0(t0)  // Program TIM1_0 register

        move    t3, zero
        lb      t1, 0x06(t2)    // tRC
        andi    t1, 0x3f
        or      t3, t1
        li      t1, 0x3f        // Use max value for tFAW initially
        sll     t1, 8
        or      t3, t1
        li      t1, 0xfe    // tRFC = 0xfe (Set to max value first.
        sll     t1, 16      // We'll fix it after we determine dram size)
        or      t3, t1

        lb      t1, 0x08(t2)    // tW2R
        andi    t1, 0x3
        sll     t1, 24
        or      t3, t1
        lb      t1, 0x09(t2)    // tR2W
        andi    t1, 0x7
        sll     t1, 27
        or      t3, t1
        lb      t1, 0x0a(t2)    // tR2R
        andi    t1, 0x3
        sll     t1, 30
        or      t3, t1
        sw      t3, MEMC_CTL_TIM1_1(t0)  // Program TIM1_1 register
    
        move    t3, zero
        lb      t1, 0x0b(t2)    // tAL
        andi    t1, 0xf
    
        // Here we create the EMRS register values
        move    t4, t1
        andi    t4, 0x7
        sll     t4, 3
        li      t5, 0x384   // RTT=75ohm, OCD Enter
        or      t4, t5
        lw      t5, MEMC_CTL_DMODE_0 (t0)  
        or      t5, t4
        sw      t5, MEMC_CTL_DMODE_0 (t0)  // Store required values in EMRS holding register

        or      t3, t1
        lb      t1, 0x0c(t2)    // tRTP
        andi    t1, 0x7
        sll     t1, 4
        or      t3, t1
        lb      t1, 0x0d(t2)    // tW2W
        andi    t1, 0x3
        sll     t1, 8
        or      t3, t1
        lb      t1, 0x0e(t2)    // tWTR
        andi    t1, 0xf
        sll     t1, 12
        or      t3, t1
        sw      t3, MEMC_CTL_TIM2(t0)    // Program TIM2 register
    
/***** Set Page policy to Cas Time check and DRAM Type to DDR2  ************/
        li      t1, 0x100
        sw      t1, MEMC_CTL_DRAM_CFG(t0)
        
// Enable ODT for writes
        li      t1, 0x104
        sw      t1, MEMC_CTL_ODT(t0)
        
/***** DDR Init Portion **********/
ddr2_init:
/***** Turn on CKE ***************/
        li      t1, 0x305
        sw      t1, MEMC_CTL_DCMD(t0)

        li      t1, 0xA0
1:  
        bnez    t1, 1b
        addi    t1, -1
        
/***** Issue Precharge All Banks Command ***/
        li      t1, 0x302
        sw      t1, MEMC_CTL_DCMD(t0)
        
/***** Issue EMRS2 Command ***/
        sw      zero, MEMC_CTL_DMODE_2 (t0)
        li      t1, 0x308
        sw      t1, MEMC_CTL_DCMD(t0)
        
/***** Issue EMRS3 Command ***/
        li      t1, 0x309
        sw      t1, MEMC_CTL_DCMD(t0)
        
/***** Enable DLL by issuing EMRS Command ***/
        li      t1, 0x300
        sw      t1, MEMC_CTL_DCMD(t0)
        
/***** Issue MRS Command. Set Reset DLL bit ***/
        li      t1, 0x301
        sw      t1, MEMC_CTL_DCMD(t0)
        
/***** Issue Precharge All Banks Command ***/
        li      t1, 0x302
        sw      t1, MEMC_CTL_DCMD(t0)
        
/***** Issue Autorefresh Command ***/
        li      t1, 0x303
        sw      t1, MEMC_CTL_DCMD(t0)
        li      t1, 0x303
        sw      t1, MEMC_CTL_DCMD(t0)

        li      t1, 0x80
3:  
        bnez    t1, 3b
        addi    t1, -1
        
/***** Issue MRS Command w/ DLL Reset bit set to 0 ***/
        lw      t1, MEMC_CTL_DMODE_0 (t0)
        li      t2, 0xFEFFFFFF          // Reset DLL reset bit
        and     t1, t2
        sw      t1, MEMC_CTL_DMODE_0 (t0)
        li      t1, 0x301
        sw      t1, MEMC_CTL_DCMD(t0)
    
/***** Issue EMRS Command (Enter OCD Calibration)  75 Ohm, Full strength Drive, tAL=tRCD-1 ***/
        li      t1, 0x300
        sw      t1, MEMC_CTL_DCMD(t0)
        
/***** Issue EMRS Command (Exit OCD Calibration) ***/
        lw      t1, MEMC_CTL_DMODE_0 (t0)
        li      t2, 0xFFFFFC7F          // Reset OCD field for exit mode
        and     t1, t2
        sw      t1, MEMC_CTL_DMODE_0 (t0)
        li      t1, 0x300
        sw      t1, MEMC_CTL_DCMD(t0)
        b       dram_timing_cont1
        nop
        
ddr3_timing:    
/****** Set control pad strength ********/
        lw      t2, PHY_CONTROL_REGS_DRIVE_PAD_CTL(t0)
	and	t2,~(0x0a)
        or      t2, 0x5
        sw      t2, PHY_CONTROL_REGS_DRIVE_PAD_CTL(t0)

/****** Change the slew and receiver power in the byte lanes ********/
        lw      t2, PHY_BYTE_LANE_0_DRIVE_PAD_CTL(t0)
	and	t2,~(0x0b)
        // or      t2, 0x0
        sw      t2, PHY_BYTE_LANE_0_DRIVE_PAD_CTL(t0)
        lw      t2, PHY_BYTE_LANE_1_DRIVE_PAD_CTL(t0)
	and	t2,~(0x0b)
        // or      t2, 0x0
        sw      t2, PHY_BYTE_LANE_1_DRIVE_PAD_CTL(t0)
        
        SETLEDS1('D','D','R','3')
        LOADREL(t2, ddr3_timing_table)

  
        li      t1, 0x10    // size of ddr3_timing_table element
        mult    t1, a1
        mflo    t1          // ddr3_timing_table offset
        add     t2, t1

        move    t3, zero
        lb      t1, 0x00(t2)    // tRCD
        andi    t1, 0xf
        move    t3, t1
        lb      t1, 0x01(t2)    // tCL
        andi    t1, 0xf
    
        move    t4, t1
        sub     t4, 4
        andi    t4, 7           // tCL in DRAM is 3 bits wide
        sll     t4, 20
        sw      t4, MEMC_CTL_DMODE_0 (t0)   // Write tCL to the MRS register holder

        sll     t1, 4
        or      t3, t1
        lb      t1, 0x02(t2)    // tWR
        andi    t1, 0xf
    
        // Here we create the MRS register values
        move    t4, t1
        li      t5, 8
        ble     t4, t5, twr_le_8
        li      t5, 4          // tWR <=8, program twr-4 to DRAM
        
        srl     t4, 2          // tWR 10, 12, 14 16 are programmed as 5, 6, 7, 0
        b       1f            // tWR = 14, program 7 to DRAM (tWR=16 is not supported)
        nop
twr_le_8:               
        subu    t4, t5
1:
        andi    t4, 0x7
        sll     t4, 25
        li      t5, 0x01000000    // Sequential burst mode, burst of 8, reset DLL
        or      t4, t5
        lw      t5, MEMC_CTL_DMODE_0 (t0)  
        or      t5, t4
        sw      t5, MEMC_CTL_DMODE_0 (t0)  // Add tWR to the MRS register holder
    
        sll     t1, 8
        or      t3, t1
        lb      t1, 0x03(t2)    // tWL
        andi    t1, 0xf
        move    t4, t1
        li      t5, 5
        subu    t4, t5         // tCWL is tCL - 5 in DDR programming
        sll     t4, 3          // tCWL starts at bit 3 in DRAM
        ori     t4, (1<<9)     // Enable Dynamic ODT and set RTT_WR to RZQ/4 = 60ohm
        sw      t4, MEMC_CTL_DMODE_2 (t0)  // Write EMRS2 Value to EMRS2 Register DMODE_2[14:0]
        
        move    t9, t4         // Keep EMRS2 value in t9 for now
        sll     t1, 12
        or      t3, t1          // Add tWL to t3
        lb      t1, 0x04(t2)    // tRP
        andi    t1, 0xf
        sll     t1, 16
        or      t3, t1
        lb      t1, 0x05(t2)    // tRRD
        andi    t1, 0xf
        sll     t1, 20
        or      t3, t1
        sw      t3, MEMC_CTL_TIM1_0(t0)  // Program TIM1_0 register

        move    t3, zero
        lb      t1, 0x06(t2)    // tRC
        andi    t1, 0x3f
        or      t3, t1
        li      t1, 0x3f        // Use max value for tFAW initially
        sll     t1, 8
        or      t3, t1
        li      t1, 0xfe    // tRFC = 0xfe (Set to max value first.
        sll     t1, 16      // We'll fix it after we determine dram size)
        or      t3, t1

        lb      t1, 0x08(t2)    // tW2R
        andi    t1, 0x3
        sll     t1, 24
        or      t3, t1
        lb      t1, 0x09(t2)    // tR2W
        andi    t1, 0x7
        sll     t1, 27
        or      t3, t1
        lb      t1, 0x0a(t2)    // tR2R
        andi    t1, 0x3
        sll     t1, 30
        or      t3, t1
        sw      t3, MEMC_CTL_TIM1_1(t0)  // Program TIM1_1 register
    
        move    t3, zero
        lb      t1, 0x0b(t2)    // tAL
        andi    t1, 0xf
    

/**** FIXME:     Add correct RTT values ********/
        // Here we create the EMRS register values
        move    t4, t1
        beqz    t4, 1f         // tAL = 0
        nop
        lb      t5, 0x01(t2)   // tCL
        andi    t5, 0xf
        subu    t5, t4
        li      t7, 1
        beq     t5, t7, 1f     // tAL = tCL - 1
        li      t4, 1
        li      t7, 2
        beq     t5, t7, 1f     // tAL = tCL - 2
        li      t4, 2   
1:      
        sll     t4, 3
        // li      t5, 0x4        // RTT_nom=60ohm
        // or      t4, t5
        lw      t5, MEMC_CTL_DMODE_0 (t0)  
        or      t5, t4
        sw      t5, MEMC_CTL_DMODE_0 (t0)  // Store required values in EMRS holding register

        or      t3, t1
        lb      t1, 0x0c(t2)    // tRTP
        andi    t1, 0x7
        sll     t1, 4
        or      t3, t1
        lb      t1, 0x0d(t2)    // tW2W
        andi    t1, 0x3
        sll     t1, 8
        or      t3, t1
        lb      t1, 0x0e(t2)    // tWTR
        andi    t1, 0xf
        sll     t1, 12
        or      t3, t1
        sw      t3, MEMC_CTL_TIM2(t0)    // Program TIM2 register
       
/***** Set Page policy to Cas Time check and DRAM Type to DDR3  ************/
        li      t1, 0x101
        sw      t1, MEMC_CTL_DRAM_CFG(t0)
        
/*** Use Dynamic ODT  ************/
// Enable ODT for writes
//        li      t1, 0x104
//        sw      t1, MEMC_CTL_ODT(t0)
        
/****** Deassert Reset For DDR3 DRAMs *************/
#if SILICON
        li      t1, 0xA0
#else
        li      t1, 0x10
#endif
1:  
        bnez    t1, 1b                    // Wait before releasing DDR_RST_N
        addi    t1, -1
        
        li      t1, 0x311
        sw      t1, MEMC_CTL_DCMD(t0)       // Deassert DDR_RST_N for DDR3

#if SILICON
        li      t1, 0xd000
#else
        li      t1, 0x10
#endif
1:  
        bnez    t1, 1b
        addi    t1, -1
        
ddr3_init:
/***** Turn on CKE ***************/
2:      
        li      t1, 0x305
        sw      t1, MEMC_CTL_DCMD(t0)

        li      t1, 0x5
1:  
        bnez    t1, 1b
        addi    t1, -1
        
/***** Issue EMRS2 Command ***/
//        lw      t3, MEMC_CTL_DMODE_0 (t0)  // Load previous value to t3 to preserve it
//        nop
//        sw      t9, MEMC_CTL_DMODE_0 (t0)  // Use EMRS2 values stored in t9 register
        li      t1, 0x308
        sw      t1, MEMC_CTL_DCMD(t0)      // Issue EMRS2 command
        
/***** Issue EMRS3 Command***/
//        li      t1, 0x0
//        sw      t1, MEMC_CTL_DMODE_0 (t0)
        li      t1, 0x309
        sw      t1, MEMC_CTL_DCMD(t0)      // Issue EMRS3 command
        
/***** Enable DLL by issuing EMRS Command ***/
//        sw      t3, MEMC_CTL_DMODE_0 (t0)  // Use the saved value back to DMODE_0 register
        li      t1, 0x300
        sw      t1, MEMC_CTL_DCMD(t0)      // Issue EMRS command
        
/***** Issue MRS Command. Set Reset DLL bit ***/
        li      t1, 0x301
        sw      t1, MEMC_CTL_DCMD(t0)      // Issue MRS command
        
/***** Issue ZQL Calibration Command ***/
        li      t1, 0x30d
        sw      t1, MEMC_CTL_DCMD(t0)
        
dram_timing_cont1:      
/*****  Run Read_En and Bit Calibrations, but we need to revert some programming first *****/

/***** Enable Auto refresh ******************************/     
        // li      t1, 0x0FC00
        // sw      t1, MEMC_CTL_CLKS(t0)

// Check x8 or x16 DDR
        li      t1, 0x12345678
        li      t3, 0x00000000
        sw      zero, 0(t3)
        sw      t1, 0(t3)
        lw      t2, 0(t3)
        bne     t1, t2, 3f  // Failed
        nop
// Do the test twice. Just in case random values match...
        li      t1, 0x87654321
        sw      t1, 0(t3)
        lw      t2, 0(t3)
        beq     t1, t2, 1f  // Clock lines are enabled as needed
        nop
        
3:
// Memory test failed. Need to re-enable byte lane 1 clock
        SETLEDS1('D','D','R','8')

// Check to see if we already had the clock re-enabled
        lw      t1, PHY_BYTE_LANE_1_CLOCK_PAD_DISABLE(t0)
	andi	t1, 0x01
	beqz	t1, 1f // we'll fail the rest, but we won't be stuck here

        li      t1, 0x304
        sw      t1, MEMC_CTL_DCMD(t0)    // Put DDR into self-refresh

        li      t1, 0x20
4:  
        bnez    t1, 4b      // Delay after going to self-refresh
        addi    t1, -1

        sw      zero, PHY_BYTE_LANE_1_CLOCK_PAD_DISABLE(t0)
        li      t1, 0x1000
2:  
        bnez    t1, 2b      // Delay after enabling clocks
        addi    t1, -1

        li      t1, 0x0c
        sw      t1, MEMC_CTL_DCMD(t0)    // Issue VDL Update command to clean up PHY problems

        li      t1, MISC_BASE
        lw      t7, MISC_STRAP_BUS(t1)
        li      t6, MISC_STRAP_BUS_DDR2_DDR3_N_SELECT
        and     t6, t7                           // Get DDR2 Strap
        beqz    t6, ddr3_init                    // Jump if DDR3
        nop
	b       ddr2_init
        nop        

1:
// Find memory size. a3 keeps the size: 0=256Mb, 1=512Mb, 2=1Gb, 3=2Gb, 4=4Gb
// Start from 4Gb device
        LOADREL(t7, dram_map_table_x8)
        lw      t1, PHY_BYTE_LANE_1_CLOCK_PAD_DISABLE(t0)
        beqz    t1, 1f
        nop
        LOADREL(t7, dram_map_table_x16)
        
1:
        li      a3, 5
ddr_size:
        addi    a3, -1
        move	t1, t7
	move	a0, a3
	li	t7, 0x53495A30 // "SIZ0"
	add	a0, t7
        bal     board_setleds
        nop
        move	t7, t1
  
        li      t1, 0x2c        // size of dram_map_table element
        mult    t1, a3
        mflo    t2              // dram_map_table offset
        add     t2, t7

        lw      t1, 0x00(t2)        // Row00_0
        sw      t1, MEMC_CTL_ROW00_0(t0)

        lw      t1, 0x04(t2)            // Row00_1
        sw      t1, MEMC_CTL_ROW00_1(t0)

        lw      t1, 0x08(t2)            // Row01_0
        sw      t1, MEMC_CTL_ROW01_0(t0)

        lw      t1, 0x0C(t2)            // Row01_1
        sw      t1, MEMC_CTL_ROW01_1(t0)

        lw      t1, 0x10(t2)            // Col00_0
        sw      t1, MEMC_CTL_COL00_0(t0)

        lw      t1, 0x14(t2)            // Col00_1
        sw      t1, MEMC_CTL_COL00_1(t0)

        lw      t1, 0x18(t2)            // Col01_0
        sw      t1, MEMC_CTL_COL01_0(t0)

        lw      t1, 0x1C(t2)            // Col01_1
        sw      t1, MEMC_CTL_COL01_1(t0)

        lw      t1, 0x20(t2)            // Bank
        sw      t1, MEMC_CTL_BNK10(t0)

        li      t1, 0x0                 // CS_Start
        sw      t1, MEMC_CTL_CSST(t0)

        lw      t1, 0x24(t2)            // CS_End
        sw      t1, MEMC_CTL_CSEND(t0)

        li      t1, 0x0                 // CS Interleaving CFG
        sw      t1, MEMC_CTL_CNFG(t0)

        lw      t3, 0x28(t2)            // Dram Size
        lw      t1, MEMC_CTL_GCFG(t0)     // GCFG
        li      t2, 0xFFFFFF00
        and     t1, t1, t2              // Mask out Dram Size Fields
        or      t1, t3, t1              // insert new DRAM Size value
        sw      t1, MEMC_CTL_GCFG(t0)
        beqz    a3, ddr_speed
        nop

// Check for memory aliasing
// This assumes that the bank interleaving is below address bits 8.
        li      t1, 8
        li      t2, 24
        add     t2, a3          // Scan up to maximum memory size
        li      t3, 0x00000000

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

	// Done with TLB Entry .. invalidate it again
 	li      t2,K0BASE // impossible VA
	nop
        mtc0    t2,C0_TLBHI
	nop
        mtc0    zero,C0_PGMASK          
	nop
        mtc0    zero,C0_TLBLO0
	nop
        mtc0    zero,C0_TLBLO1
	nop
        mtc0    zero,C0_INX // first TLB entry
        nop
        nop
        tlbwi

        
ddr_speed:
        lw      t2, PHY_BYTE_LANE_1_CLOCK_PAD_DISABLE(t0)
        bnez    t2, 1f
        nop
        sub     a3, 1           // Single 8 bit DDR is half the size of total memory
1:
        li      t1, 2
        blt     a3, t1, 1f      // If smaller than 1Gb, then program tFAW to 0
        li      t1, 0
        
        li      t1, MISC_BASE
        lw      t7, MISC_STRAP_BUS(t1)
        li      t6, MISC_STRAP_BUS_DDR2_DDR3_N_SELECT
        and     t6, t7                            // Get DDR2 Strap
        beqz    t6, ddr3_tFAW                     // Jump if DDR3
        nop
ddr2_tFAW:              
        LOADREL(t2, ddr2_timing_table)
  
        li      t1, 0x10    // size of ddrx_timing_table element
        mult    t1, a1
        mflo    t1          // ddrx_timing_table offset
        add     t2, t1
        lb      t1, 0x07(t2)    // tFAW
        b       1f
        nop
ddr3_tFAW:              
        LOADREL(t2, ddr3_timing_table)
  
        li      t1, 0x10    // size of ddrx_timing_table element
        mult    t1, a1
        mflo    t1          // ddrx_timing_table offset
        add     t2, t1
        lb      t1, 0x07(t2)    // tFAW
1:      
        andi    t1, 0x3f
        sll     t1, 8
        li      t2, ~(0x3f << 8)
        lw      t3, MEMC_CTL_TIM1_1(t0)   // Load MEMC_CTL_TIM1_1 register
        and     t3, t2                  // Clear tFAW field
        or      t3, t1
        sw      t3, MEMC_CTL_TIM1_1(t0)   // Store MEMC_CTL_TIM1_1 register
        nop

tRefi_update:   
        LOADREL(t2, dram_tRefi_table)

        li      t1, 0x4                 // size of dram_tRefi_table entry
        mult    t1, a1
        mflo    t1                      // dram_tRefi_table offset
        add     t2, t1

        lw      t1, 0x0(t2)             // tRefi
        sll     t1, 8
        sw      t1, MEMC_CTL_CLKS(t0)
        
        li      t1, MISC_BASE
        lw      t7, MISC_STRAP_BUS(t1)
        li      t6, MISC_STRAP_BUS_DDR2_DDR3_N_SELECT
        and     t6, t7                            // Get DDR2 Strap
        beqz    t6, 1f                            // If !DDR2 load ddr3_tRFC_table
        nop

        LOADREL(t2, ddr2_tRFC_table)
        nop
        b       2f
        nop
1:                      
        LOADREL(t2, ddr3_tRFC_table)
        nop
2:                
        li      t1, 4                   // size of ddrX_tRFC_table entry
        mult    t1, a1
        mflo    t1                      // ddrX_tRefi_table offset
        add     t2, t1                  // Calculate address in the row

        li      t1, 12                  // size of ddrX_tRFC_table row
        mult    t1, a3
        mflo    t1                      // ddrX_tRefi_table offset
        add     t2, t1                  // Calculate address in the column

        lw      t3, 0x0(t2)             // Load tRFC value
        andi    t3, 0xFF                // Mask tRFC to 8-bits
        sll     t3, 16                  // Move it to bit location [23:16]

        lw      t1, MEMC_CTL_TIM1_1(t0)   // Load MEMC_CTL_TIM1_1 register
        li      t2, 0xFF00FFFF
        and     t1, t1, t2              // Mask out tRFC Field
        or      t1, t3, t1              // insert new tRFC value
        sw      t1, MEMC_CTL_TIM1_1(t0)   // Write to TIM1_1 register

        SETLEDS1('D','I','N','T')
        
/***** Enable Other Mips PI Logic ************/
// Enable PHY MIPS PI
        li      t1, 0x00130000
        sw      t1, MEMC_CTL_PI_DSL_MIPS_CTL(t0)
        nop
        
// Enable Security MIPS PI
        li      t1, 0x00130000
        sw      t1, MEMC_CTL_PI_DSL_PHY_CTL(t0)
        nop

#define SKIP_AUTOSYNC_MODE_MASK  1
/************* Run PI Sync Code Based on a2 register *******/
//        li      t1, SKIP_AUTOSYNC_MODE_MASK
//        and     t1, a2
//        bnez    t1, skip_pi_code
        li      t9, 0xc

/**********************************************/    
// Check DRAM Sync Table to check whether to enable PI logic or not
        li      t2, MISC_BASE
        lw      t1, MISC_STRAP_BUS(t2)

        srl     t1, MISC_STRAP_BUS_MIPS_PLL_FVCO_SHIFT
        andi    t1, 0xf         // Mask out strap bits
        
        li      t9, 0                   // Used to keep MC/MIPS and MC/UBUS sync/async mode
        
        LOADREL(t2, dram_sync_table)

        lw      t6, 0x0(t2)             // Ubus Sync vector
        lw      t7, 0x4(t2)             // Lmb Sync vector
        lw      t8, 0x8(t2)             // Mips Sync vector

        srl     t6, t1                  // Get the ubus sync flag
        srl     t7, t1                  // Get the lmb sync flag
        srl     t8, t1                  // Get the mips fast sync flag
        andi    t6, 1                   // Mask out other bits
        andi    t7, 1                   // Mask out other bits
        andi    t8, 1                   // Mask out other bits

        bnez    t6, align_ubus2memc     // Jump if Ubus is in Sync mode
        nop

// set MC/UBUS to ASYNC                  
        or      t9, MISC_MEMC_CONTROL_MC_UBUS_ASYNC_MODE
        b       3f                      // Since Ubus is in Async Mode Skip Over Ubus Sync Code
        nop

/***** UBUS align to MEMC *****/
align_ubus2memc:     
#if UBUS_SYNC_ENABLE
        li      t1, 3                   // Sample every 3 + 1 = 4 cycles
        sll     t4, t1, MEMC_CTL_PI_UBUS_SMPL_SAMPLING_PERIOD_SHIFT  // Update Sampling Period
        li      t2, 4048
        li      t3, 1

        sw      zero, MEMC_CTL_PI_GCF(t0)
        li      t1, 0x00001c0b                  // send feedback command every 11 cycles 
        or      t1, t1, t4
        sw      t1, MEMC_CTL_PI_UBUS_SMPL(t0)
        ori     t1, 0x80                        // trigger stoke signal to latch in new counter value
        sw      t1, MEMC_CTL_PI_UBUS_SMPL(t0)
        li      t1, 0x00040000                  // send new phase value to PLL every 5 cycles
        sw      t1, MEMC_CTL_PI_UBUS_CTL(t0)
        li      t1, 0x00000001                  // enable enable counter that change PLL phase
        sw      t1, MEMC_CTL_PI_GCF(t0)
        nop

1:
        lw      t4, MEMC_CTL_PI_UBUS_SMPL(t0)   // Read a sample value.
        srl     t4, 16                          // The sample is in the upper 16 bits.
        andi    t4, t4, 0x22                    // Look at the 2 outermost bits; if the LSB is 0 and the MSB is 1,
        beq     t4, 0x20, 2f                    // then there is an edge somewhere in the sample.
        nop

        lw      t4, MEMC_CTL_PI_UBUS_CTL(t0)
        and     t4, 0xffff0000
        or      t4, t3
        or      t4, (1<<14)                     // move phase in positive direction
        sw      t4, MEMC_CTL_PI_UBUS_CTL(t0)
        nop
        lw      t4, MEMC_CTL_PI_UBUS_CTL(t0)    // Delay before reading another sample.
        add     t3, 1
        bne     t2, t3, 1b
        nop

        or      t9, MISC_MEMC_CONTROL_MC_UBUS_ASYNC_MODE    // set MC/UBUS to ASYNC                  
        b       3f                              // Did not Lock so run in Async Mode
        nop
2:
        // Success
        lw      t2, MEMC_CTL_PI_UBUS_CTL(t0)    // Turn on auto-PI mode.
        or      t2, (1 << 20)                   // PI_UBUS_CTL_Hw_Cntr_En = 1
        sw      t2, MEMC_CTL_PI_UBUS_CTL(t0)
#endif

3:
        bnez    t7, align_mips2mclk             // Jump if LMB is in Sync mode
        nop
// set MC/MIPS to ASYNC                  
        or      t9, MISC_MEMC_CONTROL_MC_LMB_ASYNC_MODE
        b       3f                              // Since Ubus is in Async Mode Skip Over Ubus Sync Code
        nop

align_mips2mclk:
/***** MIPS align to MCLK *****/
        li      t1, 3
        sll     t4, t1,MEMC_CTL_PI_MIPS_SMPL_SAMPLING_PERIOD_SHIFT
        li      t2, 4048
        li      t3, 1

//        sw      zero, MEMC_CTL_PI_GCF(t0)
        li      t1, 0x00001c0b                  // send feedback command every 11 cycles 
        or      t1, t1, t4
        sw      t1, MEMC_CTL_PI_MIPS_SMPL(t0)
        ori     t1, 0x80                        // trigger stroke signal to latch in new counter value
        sw      t1, MEMC_CTL_PI_MIPS_SMPL(t0)
        li      t1, 0x00040000                  // send new phase value to PLL every 5 cycles
        sw      t1, MEMC_CTL_PI_MIPS_CTL(t0)
        li      t1, 0x00000001                  // enable enable counter that change PLL phase
        sw      t1, MEMC_CTL_PI_GCF(t0)
        nop

1:
        lw      t4, MEMC_CTL_PI_MIPS_SMPL(t0)   // Read a sample value.
        srl     t4, 16                          // The sample is in the upper 16 bits.

        andi    t4, t4, 0x22                    // Look at the 2 outermost bits; if the LSB is 0 and the MSB is 1,
        beq     t4, 0x20, 2f                    // then there is an edge somewhere in the sample.
        nop
        lw      t4, MEMC_CTL_PI_MIPS_CTL(t0)
        and     t4, 0xffff0000
        or      t4, t3
        or      t4, (1<<14)                     // move phase in positive direction
        sw      t4, MEMC_CTL_PI_MIPS_CTL(t0)
        nop
        lw      t4, MEMC_CTL_PI_MIPS_CTL(t0)    // Delay before reading another sample.
        add     t3, 1
        bne     t2, t3, 1b
        nop

        or      t9, MISC_MEMC_CONTROL_MC_LMB_ASYNC_MODE // set MC/MIPS to ASYNC                  
        b       3f                              // Did not Lock so run in Async Mode
        nop
2:      
        // Success
        lw      t2, MEMC_CTL_PI_MIPS_CTL(t0)    // Turn on auto-PI mode.
        or      t2, (1 << 20)                   // PI_MIPS_CTL_Hw_Cntr_En = 1
        sw      t2, MEMC_CTL_PI_MIPS_CTL(t0)

3:
skip_pi_code:

#if UBUS_SYNC_ENABLE
#else
        ori     t9, MISC_MEMC_CONTROL_MC_UBUS_ASYNC_MODE
#endif
#if LMB_SYNC_ENABLE
#else
        ori     t9, MISC_MEMC_CONTROL_MC_LMB_ASYNC_MODE
#endif
        move    t3, t9
        li      t1, MISC_MEMC_CONTROL_MC_UBUS_ASYNC_MODE
        and     t1, t3
        bnez    t1, memc_ubus_async
        nop
        SETLEDS1('U','S','Y','N')
        b       1f
        nop
memc_ubus_async:
        SETLEDS1('U','A','S','Y')

1:
        li      t1, MISC_MEMC_CONTROL_MC_LMB_ASYNC_MODE
        and     t1, t3
        bnez    t1, memc_lmb_async
        nop
        SETLEDS1('L','S','Y','N')
        b       1f
        nop
memc_lmb_async:
        SETLEDS1('L','A','S','Y')
1:
        li      t1, MISC_BASE
        sw      t3, MISC_MEMC_CONTROL(t1)

/**----- Clear MIPS Async mode bit (for Fast Async Mode) -------**/
        mfc0    t1, C0_BCM_CONFIG, 5
        and     t1, ~(0x3 << 28)
        mtc0    t1, C0_BCM_CONFIG, 5

        SETLEDS1('M','F','A','S')

#define LMB_ENABLE_MASK 0x10000
/**----- Enable LMB -------------------------------------**/
//        li      t1, LMB_ENABLE_MASK
//        and     t1, a2
//        bnez    t1, lmb_enable
//        nop
//        SETLEDS1('L','M','B','D')
//        b       1f
//        nop

lmb_enable:
        li      t0, MIPS_BASE
        lw      t2, MIPS_LMB_CR(t0)
#if LMB_ENABLE
        or      t2, LMB_EN                              // Enable LMB
        SETLEDS1('L','M','B','E')
#else
        SETLEDS1('L','M','B','D')
#endif
        sw      t2, MIPS_LMB_CR(t0)
1:
/**----- Enable RAC -------------------------------------**/
        li      t2, 0xFFF << RAC_UPB_SHFT               // Enable prefetch for RAM address range up to 256MB
        sw      t2, MIPS_RAC_ARR(t0)

        lw      t2, MIPS_RAC_CR0(t0)
        or      t2, (RAC_C_INV | RAC_I | RAC_PF_I)
        sw      t2, MIPS_RAC_CR0(t0)

        lw      t2, MIPS_RAC_CR1(t0)
        or      t2, (RAC_C_INV | RAC_I | RAC_PF_I)
        sw      t2, MIPS_RAC_CR1(t0)
        SETLEDS1('R','A','C','E')

/**----- Enable branch prediction and non-blocking data cache ---**/
        mfc0    t1, C0_BCM_CONFIG
        and     t1, ~CP0_BCM_CFG_BTHD
        or      t1, CP0_BCM_CFG_NBK
        or      t1, CP0_BCM_CFG_CLF
        mtc0    t1, C0_BCM_CONFIG

#if DDR_TEST
#######################################
# Run test on DRAM using Test Engine  #
#######################################
#define BACKGND_MODE_DATA       0x0
#define BACKGND_MODE_LFSR       0x400
#define BACKGND_MODE_PRBS       0x800
#define BACKGND_MODE_NOT_PRBS   0xc00
#define BACKGND_MODE_PAT        0x1000
#define BACKGND_MODE_NOT_PAT    0x1400

#define VICT_MODE_PRBS          0
#define VICT_MODE_NOT_PRBS      0x100
#define VICT_MODE_PAT           0x200
#define VICT_MODE_NOT_PAT       0x300

#define VICT_ENABLE             0x8000
#define VICT_SWEEP_ENABLE       0x10000
#define VICT_COUNT              0x0

#define PRBS_ORDER(x)           ((x & 0x3) << 13)

#define TEST_COUNT              0x1000
#define TEST_ADDR               0x0
#define TEST_ADDR_UPDT          0x1
#define TEST_PATTERN0           0x5555
#define TEST_PATTERN1           0xaaaa
#define TEST_PATTERN            (TEST_PATTERN1 << 16 | TEST_PATTERN0)

#define TEST_ENABLE             0x1
#define TEST_DONE               0x2
#define TEST_ERROR              0x4
#define TEST_WRITE              0x10
#define TEST_READ               0x0
#define TEST_WR_RD              0x20    

#define BACKGND_DATA0           0xa5a5a5a5
#define BACKGND_DATA1           0x5a5a5a5a
#define BACKGND_DATA2           0xa5a5a5a5
#define BACKGND_DATA3           0x5a5a5a5a

#define TEST_DATA0              0x02468ace
#define TEST_DATA1              0x13579bdf
#define TEST_DATA2              0x33cccc33
#define TEST_DATA3              0x55aaaa55

/***** Load DDR Base *************************************/
        li      t0, MEMC_BASE
        li      t1, BACKGND_DATA0
        sw      t1, MEMC_CTL_TEST_DATA0_0(t0)  
        li      t1, BACKGND_DATA1
        sw      t1, MEMC_CTL_TEST_DATA0_1(t0)  
        li      t1, BACKGND_DATA2
        sw      t1, MEMC_CTL_TEST_DATA0_2(t0)  
        li      t1, BACKGND_DATA3
        sw      t1, MEMC_CTL_TEST_DATA0_3(t0)  
        li      t1, TEST_COUNT
        li      t2, VICT_COUNT
        or      t1, t2              # add victim count value
        sw      t1, MEMC_CTL_TEST_COUNT(t0)  
        li      t1, TEST_ADDR
        sw      t1, MEMC_CTL_TEST_ADDR(t0)   
        li      t1, TEST_ADDR_UPDT
        sw      t1, MEMC_CTL_TEST_ADDR_UPDT(t0)  
        li      t1, TEST_PATTERN
        sw      t1, MEMC_CTL_TEST_PAT(t0)
        
# Write a background pattern first
        li      t1, BACKGND_MODE_DATA
        li      t2, TEST_WRITE
        or      t1, t2
        li      t2, TEST_ENABLE
        or      t1, t2
        sw      t1, MEMC_CTL_TEST_CFG1(t0)

        li      t2, 0x2
        li      t3, MISC_BASE

        li      t5, 0x10000
2:      li      t1, 0x100
1:      addiu   t1, -1
        bnez    t1, 1b
        nop

        lw      t4, MISC_MEMC_CONTROL(t3)
        and     t4, 0x2
        beq     t4, t2, backgnd_write_done
        nop
        addiu   t5, -1
        bnez    t5, 2b
        nop

# Background write operation is finished

backgnd_write_done:
        li      t1, TEST_DATA0
        sw      t1, MEMC_CTL_TEST_DATA0_0(t0)  
        li      t1, TEST_DATA1
        sw      t1, MEMC_CTL_TEST_DATA0_1(t0)  
        li      t1, TEST_DATA2
        sw      t1, MEMC_CTL_TEST_DATA0_2(t0)  
        li      t1, TEST_DATA3
        sw      t1, MEMC_CTL_TEST_DATA0_3(t0)  
        li      t1, TEST_COUNT
        li      t2, VICT_COUNT
        or      t1, t2              # add victim count value
        sw      t1, MEMC_CTL_TEST_COUNT(t0)  
        li      t1, TEST_ADDR
        sw      t1, MEMC_CTL_TEST_ADDR(t0)   
        li      t1, TEST_ADDR_UPDT
        sw      t1, MEMC_CTL_TEST_ADDR_UPDT(t0)  
        li      t1, TEST_PATTERN
        sw      t1, MEMC_CTL_TEST_PAT(t0)

#       li      t1, BACKGND_MODE_DATA
#       li      t1, BACKGND_MODE_PAT
#       li      t2, VICT_MODE_NOT_PAT
        li      t1, BACKGND_MODE_PRBS
        li      t2, VICT_MODE_NOT_PRBS
        
        or      t1, t2
        li      t2, VICT_ENABLE
        or      t1, t2
        li      t2, VICT_SWEEP_ENABLE
        or      t1, t2
        li      t2, PRBS_ORDER(0)
        or      t1, t2
        li      t2, TEST_WRITE
        or      t1, t2
        li      t2, TEST_ENABLE
        or      t1, t2
        sw      t1, MEMC_CTL_TEST_CFG1(t0)

        li      t2, 0x2
        li      t3, MISC_BASE

        li      t5, 0x10000
2:      li      t1, 0x100
1:      addiu   t1, -1
        bnez    t1, 1b
        nop

        lw      t4, MISC_MEMC_CONTROL(t3)
        and     t4, 0x2
        beq     t4, t2, test_write_done
        nop
        addiu   t5, -1
        bnez    t5, 2b
        nop

# Test write operation is finished

test_write_done:
        li      t1, TEST_DATA0
        sw      t1, MEMC_CTL_TEST_DATA0_0(t0)  
        li      t1, TEST_DATA1
        sw      t1, MEMC_CTL_TEST_DATA0_1(t0)  
        li      t1, TEST_DATA2
        sw      t1, MEMC_CTL_TEST_DATA0_2(t0)  
        li      t1, TEST_DATA3
        sw      t1, MEMC_CTL_TEST_DATA0_3(t0)  
        li      t1, TEST_COUNT
        li      t2, VICT_COUNT
        or      t1, t2              # add victim count value
        sw      t1, MEMC_CTL_TEST_COUNT(t0)  
        li      t1, TEST_ADDR
        sw      t1, MEMC_CTL_TEST_ADDR(t0)   
        li      t1, TEST_ADDR_UPDT
        sw      t1, MEMC_CTL_TEST_ADDR_UPDT(t0)  
        li      t1, TEST_PATTERN
        sw      t1, MEMC_CTL_TEST_PAT(t0)

#       li      t1, BACKGND_MODE_DATA
#       li      t1, BACKGND_MODE_PAT
#       li      t2, VICT_MODE_NOT_PAT
        li      t1, BACKGND_MODE_PRBS
        li      t2, VICT_MODE_NOT_PRBS
        
        or      t1, t2
        li      t2, VICT_ENABLE
        or      t1, t2
        li      t2, VICT_SWEEP_ENABLE
        or      t1, t2
        li      t2, PRBS_ORDER(0)
        or      t1, t2
        li      t2, TEST_READ
        or      t1, t2
        li      t2, TEST_ENABLE
        or      t1, t2
        sw      t1, MEMC_CTL_TEST_CFG1(t0)

        li      t3, MISC_BASE
        li      t2, 0x2

        li      t5, 0x10000
2:      li      t1, 0x100
1:      addiu   t1, -1
        bnez    t1, 1b
        nop

        lw      t4, MISC_MEMC_CONTROL(t3)
        and     t4, 0x2
        beq     t4, t2, test_read_done
        nop
        addiu   t5, -1
        bnez    t5, 2b
        nop

# Test read operation is finished

test_read_done:
        lw      t1, MEMC_CTL_TEST_CFG1(t0)
        srl     t1, 2
        and     t1, 1
        bnez    t1, test_failed
        nop
        
# Second test (Write/write/Read/Read test)
# First write background data
        li      t1, BACKGND_DATA0
        sw      t1, MEMC_CTL_TEST_DATA0_0(t0)  
        li      t1, BACKGND_DATA1
        sw      t1, MEMC_CTL_TEST_DATA0_1(t0)  
        li      t1, BACKGND_DATA2
        sw      t1, MEMC_CTL_TEST_DATA0_2(t0)  
        li      t1, BACKGND_DATA3
        sw      t1, MEMC_CTL_TEST_DATA0_3(t0)  
        li      t1, TEST_COUNT
        li      t2, VICT_COUNT
        or      t1, t2              # add victim count value
        sw      t1, MEMC_CTL_TEST_COUNT(t0)  
        li      t1, TEST_ADDR
        sw      t1, MEMC_CTL_TEST_ADDR(t0)   
        li      t1, TEST_ADDR_UPDT
        sw      t1, MEMC_CTL_TEST_ADDR_UPDT(t0)  
        li      t1, TEST_PATTERN
        sw      t1, MEMC_CTL_TEST_PAT(t0)
        
# Write a background pattern first
        li      t1, BACKGND_MODE_DATA
        li      t2, TEST_WRITE
        or      t1, t2
        li      t2, TEST_ENABLE
        or      t1, t2
        sw      t1, MEMC_CTL_TEST_CFG1(t0)

        li      t2, 0x2
        li      t3, MISC_BASE

        li      t5, 0x10000
2:      li      t1, 0x100
1:      addiu   t1, -1
        bnez    t1, 1b
        nop

        lw      t4, MISC_MEMC_CONTROL(t3)
        and     t4, 0x2
        beq     t4, t2, backgnd_write_done2
        nop
        addiu   t5, -1
        bnez    t5, 2b
        nop

# Background write operation is finished

backgnd_write_done2:
        li      t1, TEST_DATA0
        sw      t1, MEMC_CTL_TEST_DATA0_0(t0)  
        li      t1, TEST_DATA1
        sw      t1, MEMC_CTL_TEST_DATA0_1(t0)  
        li      t1, TEST_DATA2
        sw      t1, MEMC_CTL_TEST_DATA0_2(t0)  
        li      t1, TEST_DATA3
        sw      t1, MEMC_CTL_TEST_DATA0_3(t0)  
        li      t1, TEST_COUNT
        li      t2, VICT_COUNT
        or      t1, t2              # add victim count value
        sw      t1, MEMC_CTL_TEST_COUNT(t0)  
        li      t1, TEST_ADDR
        sw      t1, MEMC_CTL_TEST_ADDR(t0)   
        li      t1, TEST_ADDR_UPDT
        sw      t1, MEMC_CTL_TEST_ADDR_UPDT(t0)  
        li      t1, TEST_PATTERN
        sw      t1, MEMC_CTL_TEST_PAT(t0)

#       li      t1, BACKGND_MODE_DATA
#       li      t1, BACKGND_MODE_PAT
#       li      t2, VICT_MODE_NOT_PAT
        li      t1, BACKGND_MODE_LFSR
        li      t2, VICT_MODE_NOT_PRBS
        
        or      t1, t2
        li      t2, VICT_ENABLE
        or      t1, t2
        li      t2, VICT_SWEEP_ENABLE
        or      t1, t2
        li      t2, PRBS_ORDER(0)
        or      t1, t2
        li      t2, TEST_WR_RD
        or      t1, t2
        li      t2, TEST_ENABLE
        or      t1, t2
        sw      t1, MEMC_CTL_TEST_CFG1(t0)

        li      t2, 0x2
        li      t3, MISC_BASE

        li      t5, 0x10000
2:      li      t1, 0x100
1:      addiu   t1, -1
        bnez    t1, 1b
        nop

        lw      t4, MISC_MEMC_CONTROL(t3)
        and     t4, 0x2
        beq     t4, t2, test_wr_rd_done
        nop
        addiu   t5, -1
        bnez    t5, 2b
        nop

test_wr_rd_done:
        lw      t1, MEMC_CTL_TEST_CFG1(t0)
        srl     t1, 2
        and     t1, 1
        beqz    t1, test_passed
        nop
# Test write/write/read/read operation is finished

test_failed:
        SETLEDS1('F','A','I','L')
        b   1f
        nop
test_passed:
        SETLEDS1('P','A','S','S')
1:
        SETLEDS1('-','-','-', '-')
#endif

memc_init_end:
        li      t0, MEMC_BASE
        li      t1, MEMC_CTL_GCFG_MEMINITDONE
        lw      t2, MEMC_CTL_GCFG(t0)
        or      t2, t1                          // Set MemInitDone bit
        sw      t2, MEMC_CTL_GCFG(t0)           // Write to MC Global Config Register

        move    ra,s0
        j       ra
        nop
    
        .set    reorder

END(board_draminit)
