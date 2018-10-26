#include "mipsmacros.h"
#include "sbmips.h"
#include "bcm_map.h"

// uncomment following line to double DDR self-refresh rate to support operating tempretures up to 95C
//#define DDR_HI_TEMP

#define DDR3_LETTER_GRADE       0x00030000      # bit 17-16
#define DDR3_SPEED_GRADE        0x00000300      # bit 12-8
#define DDR3_EXACT_SPEED        0x00000003      # bit 4-0
        
#define DDR3_ODS_34             1               # Set Output Drive to 34Ohms
       
#if RUN_IN_SIMULATOR
#define RD_EN_OVR               0
#define DDR_TEST                0
#define DDRDEBUG                0
#define SILICON                 0
#else
#define MEM_SIZE_DETECT			1
#define RD_EN_OVR               1
#define DDR_TEST                1
#define DDRDEBUG                1
#define SILICON                 1
#endif
#define JEDEC_DEFAULT           12              # Default (12): sg187E JEDEC DDR3-1066F (7-7-7)

//#define LOADREL( reg, label )   la reg, label
//#define LEAF( label )           .ent label;.globl label; label:
//#define END( label )            .end label

    .globl  ddr3_333MHz
ddr3_333MHz:
    .word  0x0    /* DDR3 333Mhz enable*/

/* Memory mapping table for different size DRAMs (256Mb, 512Mb, 1Gb, 2Gb) */
    .globl  dram_map_table_x8
dram_map_table_x8:
    // 128Mb 64B Interleaving (x8 Mode) 32MB
    // This is just a place holder. This memory does not exist
    .word  0x0F0E0D0C, 0x13121110, 0x17161514, 0x00000018  // Row00_0 Row00_1 Row01_0 Row01_1
    .word  0x04000000, 0x0A090805, 0x0000000B, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    .word  0x00080706, 0x00000002, 0x00000055              // Bank CS_End Dramsize

    // 256Mb 64B Interleaving (x8 Mode) 64MB
    .word  0x100F0E0D, 0x14131211, 0x18171615, 0x00000019  // Row00_0 Row00_1 Row01_0 Row01_1
    .word  0x04000000, 0x0A090805, 0x00000C0B, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    .word  0x00000706, 0x00000004, 0x00000066              // Bank CS_End Dramsize
    
    // 512Mb 64B Interleaving (x8 Mode) 128MB
    .word  0x100F0E0D, 0x14131211, 0x18171615, 0x00001A19  // Row00_0 Row00_1 Row01_0 Row01_1
    .word  0x04000000, 0x0A090805, 0x00000C0B, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    .word  0x00000706, 0x00000008, 0x00000077              // Bank CS_End Dramsize

    // 1Gb 64B Interleaving (x8 Mode) 256MB
    .word  0x11100F0E, 0x15141312, 0x19181716, 0x00001B1A  // Row00_0 Row00_1 Row01_0 Row01_1
    .word  0x04000000, 0x0B0A0905, 0x00000D0C, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    .word  0x00080706, 0x00000010, 0x00000088              // Bank CS_End Dramsize

    // 2Gb 64B Interleaving (x8 Mode) 512MB
    .word  0x11100F0E, 0x15141312, 0x19181716, 0x001C1B1A  // Row00_0 Row00_1 Row01_0 Row01_1
    .word  0x04000000, 0x0B0A0905, 0x00000D0C, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    .word  0x00080706, 0x00000020, 0x00000099              // Bank CS_End Dramsize

    // 4Gb 64B Interleaving (x8 Mode) 1GB
    .word  0x11100F0E, 0x15141312, 0x19181716, 0x1D1C1B1A  // Row00_0 Row00_1 Row01_0 Row01_1
    .word  0x04000000, 0x0B0A0905, 0x00000D0C, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    .word  0x00080706, 0x00000040, 0x000000AA              // Bank CS_End Dramsize

    .globl  dram_map_table_x16
dram_map_table_x16:
    // 256Mb 64B Interleaving (x16 Mode) 32MB
    .word  0x0F0E0D0C, 0x13121110, 0x17161514, 0x00000018  // Row00_0 Row00_1 Row01_0 Row01_1
    .word  0x04000000, 0x0A090805, 0x0000000B, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    .word  0x00000706, 0x00000002, 0x00000055              // Bank CS_End Dramsize
    
    // 512Mb 64B Interleaving (x16 Mode) 64MB
    .word  0x11100F0E, 0x15141312, 0x19181716, 0x00000000  // Row00_0 Row00_1 Row01_0 Row01_1
    .word  0x04000000, 0x0B0A0905, 0x00000D0C, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    .word  0x00080706, 0x00000004, 0x00000066              // Bank CS_End Dramsize

    // 1Gb 64B Interleaving (x16 Mode) 128MB
    .word  0x11100F0E, 0x15141312, 0x19181716, 0x0000001A  // Row00_0 Row00_1 Row01_0 Row01_1
    .word  0x04000000, 0x0B0A0905, 0x00000D0C, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    .word  0x00080706, 0x00000008, 0x00000077              // Bank CS_End Dramsize

    // 2Gb 64B Interleaving (x16 Mode) 256MB
    .word  0x11100F0E, 0x15141312, 0x19181716, 0x00001B1A  // Row00_0 Row00_1 Row01_0 Row01_1
    .word  0x04000000, 0x0B0A0905, 0x00000D0C, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    .word  0x00080706, 0x00000010, 0x00000088              // Bank CS_End Dramsize

    // 4Gb 64B Interleaving (x16 Mode) 512MB
    .word  0x11100F0E, 0x15141312, 0x19181716, 0x001C1B1A  // Row00_0 Row00_1 Row01_0 Row01_1
    .word  0x04000000, 0x0B0A0905, 0x00000D0C, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    .word  0x00080706, 0x00000020, 0x00000099              // Bank CS_End Dramsize
        
    // 8Gb 64B Interleaving (x16 Mode) 1GB
    .word  0x11100F0E, 0x15141312, 0x19181716, 0x1D1C1B1A  // Row00_0 Row00_1 Row01_0 Row01_1
    .word  0x04000000, 0x0B0A0905, 0x00000D0C, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    .word  0x00080706, 0x00000040, 0x000000AA              // Bank CS_End Dramsize

ddr3_512_map_table_x8:
    // 512Mb 64B Interleaving (x8 Mode) 128MB
    .word  0x11100F0E, 0x15141312, 0x19181716, 0x0000001A  // Row00_0 Row00_1 Row01_0 Row01_1
    .word  0x04000000, 0x0B0A0905, 0x00000D0C, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    .word  0x00080706, 0x00000008, 0x00000077              // Bank CS_End Dramsize
    
ddr3_512_map_table_x16:
    // 512Mb 64B Interleaving (x16 Mode) 64MB
    .word  0x11100F0E, 0x15141312, 0x19181716, 0x00000000  // Row00_0 Row00_1 Row01_0 Row01_1
    .word  0x04000000, 0x0B0A0905, 0x00000D0C, 0x00000000  // Col00_0 Col00_1 Col01_0 Col01_1
    .word  0x00080706, 0x00000004, 0x00000066              // Bank CS_End Dramsize
    
/*
bit 26: cke
bit 25: rst_n
bit 24: ras
bit 23: cas
bit 22: we
bit 21: aux[2]
bit 20: aux[1]
bit 19: aux[0]
bit 18: ba[2]
bit 17: ba[1]
bit 16: ba[0]
bit 15: ad[15]
bit 14: ad[14]
bit 13: ad[13]
bit 12: ad[12]
bit 11: ad[11]
bit 10: ad[10]
bit 9: ad[9]
bit 8: ad[8]
bit 7: ad[7]
bit 6: ad[6]
bit 5: ad[5]
bit 4: ad[4]
bit 3: ad[3]
bit 2: ad[2]
bit 1: ad[1]
bit 0: ad[0]
*/
        
#define Virtual_VTT_Context     0x01C7FFFF
#define Virtual_VTT_Override    0x0007FFFF
        
        /*
ddr3_Virtual_VTT_x16:
    .word  0x01C70FFF,  // 512Mbx16
    .word  0x01C71FFF   // 1Gbx16
    .word  0x01C73FFF   // 2Gbx16
    .word  0x01C77FFF   // 4Gbx16
    .word  0x01C7FFFF   // 8Gbx16

ddr3_Virtual_VTT_x8:
    .word  0x01C71FFF   // 512Mbx16
    .word  0x01C73FFF   // 1Gbx16
    .word  0x01C77FFF   // 2Gbx16
    .word  0x01C7FFFF   // 4Gbx16
    .word  0x01C7FFFF   // 8Gbx16
        
ddr2_Virtual_VTT_x16:
    .word  0x01C71FFF   // 216Mbx16
    .word  0x01C71FFF   // 512Mbx16
    .word  0x01C71FFF   // 1Gbx16
    .word  0x01C73FFF   // 2Gbx16
    .word  0x01C77FFF   // 4Gbx16

ddr2_Virtual_VTT_x8:
    .word  0x01C71FFF   // 216Mbx16
    .word  0x01C73FFF   // 512Mbx16
    .word  0x01C73FFF   // 1Gbx16
    .word  0x01C77FFF   // 2Gbx16
    .word  0x01C7FFFF   // 4Gbx16
*/

    .globl  ddr_tRefi_table
ddr_tRefi_table:       /* Refresh Interval table for different Speed DRAMs */
#if defined(DDR_HI_TEMP)
    .word  0x0B   // 200-266 (MHz)
    .word  0x23   // 300-333 (MHz)
    .word  0x2E   // 400     (MHz)
    .word  0x3A   // 533     (MHz)  
    .word  0x4A   // 667     (MHz)
    .word  0x5D   // 800     (MHz)
#else
    .word  0x16   // 200-266 (MHz)
    .word  0x49   // 300-333 (MHz)
    .word  0x5D   // 400     (MHz)
    .word  0x78   // 533     (MHz)  
    .word  0x98   // 667     (MHz)
    .word  0xC0   // 800     (MHz)
#endif

    .globl  ddr2_tRFC_table
ddr2_tRFC_table:        /* tRFC table for different size & Speed DRAMs (256Mb, 512Mb, 1Gb)/(200, 333, 400) */
//         200    333    400   533   667   800(MHz)       
    .word  0xF,  0x19,  0x1E, 0x28, 0xFE, 0xFE   // 256Mb 
    .word  0x15, 0x23,  0x2A, 0x38, 0xFE, 0xFE   // 512Mb        
    .word  0x1A, 0x2B,  0x33, 0x44, 0xFE, 0xFE   // 1Gb      
    .word  0x28, 0x42,  0x4F, 0x6A, 0xFE, 0xFE   // 2Gb  

    .globl  ddr3_tRFC_table
ddr3_tRFC_table:        /* tRFC table for different size & Speed DRAMs (256Mb, 512Mb, 1Gb)/(200, 333, 400) */
//         200    333    400   533   667   800(MHz)       
    .word  0x24, 0x24,  0x24, 0x30, 0x3C, 0x48   // 256Mb (Do Not Exist!!)
    .word  0x24, 0x24,  0x24, 0x30, 0x3C, 0x48   // 512Mb        
    .word  0x2C, 0x2C,  0x2C, 0x3B, 0x4A, 0x58   // 1Gb      
    .word  0x40, 0x40,  0x40, 0x56, 0x6B, 0x80   // 2Gb  
    .word  0x78, 0x78,  0x78, 0xA0, 0xC8, 0xF0   // 4Gb  

    .globl  ddr2_timing_table
ddr2_timing_table:      /*  */
//          tRCD  tCL  tWR  tWL  tRP tRRD  tRC tFAW tW2R tR2W tR2R  tAL tRTP tW2W tWTR Jedec
//          --------------------------------------------------------------------------------
    .byte   0x04,0x04,0x04,0x03,0x04,0x03,0x10,0x0D,0x01,0x01,0x00,0x03,0x02,0x00,0x00,0x00 // 200-266MHz DDR2-400B/C, DDR2-533B/C (4-4-4)
    .byte   0x05,0x05,0x05,0x04,0x05,0x04,0x14,0x11,0x02,0x01,0x00,0x04,0x03,0x00,0x00,0x00 // 300-333MHz DDR2-667D (5-5-5)  
    .byte   0x05,0x05,0x06,0x04,0x05,0x04,0x17,0x12,0x02,0x01,0x00,0x04,0x03,0x00,0x00,0x00 // 400MHz     DDR2-800D, DDR2-800C (5-5-5)
    .byte   0x07,0x07,0x08,0x06,0x07,0x06,0x1D,0x19,0x02,0x01,0x00,0x06,0x05,0x00,0x04,0x00 // 533MHz     DDR2-1066E, DDR2-1066F (7-7-7)
//    .byte   0x06,0x06,0x08,0x05,0x06,0x06,0x1D,0x00,0x02,0x01,0x00,0x05,0x05,0x00,0x05,0x00 // 533MHz     DDR2-1066E, DDR2-1066F (6-6-6)
    
    .globl  ddr3_timing_table
ddr3_timing_table:      /*  */
//          tRCD  tCL  tWR  tWL  tRP tRRD  tRC tFAW tW2R tR2W tR2R  tAL tRTP tW2W tWTR Jedec
//          --------------------------------------------------------------------------------
    .byte   0x05,0x05,0x06,0x05,0x06,0x05,0x18,0x10,0x02,0x03,0x00,0x04,0x03,0x00,0x04,9  // 400MHz Micron -25E   JEDEC DDR3-800D
    .byte   0x07,0x07,0x08,0x06,0x07,0x06,0x1C,0x14,0x02,0x03,0x00,0x05,0x04,0x00,0x04,12 // 533MHz Micron -1875E JEDEC DDR3-1066F
    .byte   0x09,0x09,0x0A,0x07,0x09,0x05,0x22,0x14,0x02,0x03,0x00,0x07,0x05,0x00,0x05,16 // 667MHz Micron -15E   JEDEC DDR3-1333H
    .byte   0x0A,0x0A,0x0C,0x08,0x0A,0x06,0x26,0x18,0x02,0x03,0x00,0x08,0x06,0x00,0x06,20 // 800MHz Micron -125E  JEDEC DDR3-1600J
    
#define JEDEC_LETTER_GRADE_BITS  0x30     // bits[5:4]
#define JEDEC_SPEED_GRADE_BITS   0x03     // bits[1:0]

.globl jedec_table
jedec_table:
//   LetterGrade_SpeedGrade  // PHY SpeedGrade Number
.byte   0x00    // 0    JEDEC DDR2-533B (3-3-3)
.byte   0x00    // 1    JEDEC DDR2-533C (3-3-3)
.byte   0x00    // 2    JEDEC DDR2-667C (4-4-4)
.byte   0x00    // 3    JEDEC DDR2-667D (5-5-5)
.byte   0x00    // 4    JEDEC DDR2-800C (4-4-4)
.byte   0x00    // 5    JEDEC DDR2-800D (5-5-5)
.byte   0x00    // 6    JEDEC DDR2-800E (6-6-6)
.byte   0x00    // 7    JEDEC DDR2-1066E (5-5-5)
.byte   0x00    // 8    JEDEC DDR2-1066F (6-6-6)
.byte   0x00    // 9    JEDEC DDR3-800D (5-5-5)
.byte   0x10    // 10   JEDEC DDR3-800E (6-6-6)
.byte   0x01    // 11   JEDEC DDR3-1066E (6-6-6)
.byte   0x11    // 12   JEDEC DDR3-1066F (7-7-7)
.byte   0x21    // 13   JEDEC DDR3-1066G (8-8-8)
.byte   0x02    // 14   JEDEC DDR3-1333F (7-7-7)
.byte   0x12    // 15   JEDEC DDR3-1333G (8-8-8)
.byte   0x22    // 16   JEDEC DDR3-1333H (9-9-9)
.byte   0x32    // 17   JEDEC DDR3-1333J (10-10-10)
.byte   0x03    // 18   JEDEC DDR3-1600G (8-8-8)
.byte   0x13    // 19   JEDEC DDR3-1600H (9-9-9)
.byte   0x23    // 20   JEDEC DDR3-1600J (10-10-10)
.byte   0x33    // 21   JEDEC DDR3-1600K (11-11-11)
        
.align 2
        
.globl ddr3_new_config_table
ddr3_new_config_table:      /*  */
#       tCL  tCWL  tAL Jedec /* tCL=tRP=tRCD */
#       -----------------------------------------------------------------------------------------
.globl letter_grade_0
letter_grade_0: 
.globl base_800_0
base_800_0:  // 400MHz Micron -25E   JEDEC DDR3-800D (5-5-5)
.byte   0x05,0x05,0x04,9    # 400-400
.byte   0xff,0xff,0xff,0xff # 400-533
.byte   0xff,0xff,0xff,0xff # 400-667
.byte   0xff,0xff,0xff,0xff # 400-800
        
.globl base_1066_0
base_1066_0:  // 533MHz Micron -1875X JEDEC DDR3-1066E (6-6-6)
.byte   0x05,0x05,0x04,9    # 533-400
.byte   0x06,0x06,0x04,11   # 533-533
.byte   0xff,0xff,0xff,0xff # 533-667
.byte   0xff,0xff,0xff,0xff # 533-800

.globl base_1333_0
base_1333_0:  // 667MHz Micron -15X   JEDEC DDR3-1333F (7-7-7)
.byte   0x05,0x05,0x04,9    # 667-400
.byte   0x06,0x06,0x04,11   # 667-533
.byte   0x07,0x07,0x05,14   # 667-667
.byte   0xff,0xff,0xff,0xff # 667-800

.globl base_1600_0
base_1600_0:  // 800MHz Micron -125X  JEDEC DDR3-1600G (8-8-8)
.byte    0x05,0x05,0x04,9  # 800-400
.byte    0x06,0x06,0x04,11 # 800-533
.byte    0x07,0x07,0x05,14 # 800-667
.byte    0x08,0x08,0x06,18 # 800-800

.globl letter_grade_1   
letter_grade_1: 
.globl base_800_1
base_800_1:  // 400MHz Micron -25   JEDEC DDR3-800E (6-6-6)
.byte   0x06,0x05,0x05,10   # 400-400
.byte   0xff,0xff,0xff,0xff # 400-533
.byte   0xff,0xff,0xff,0xff # 400-667
.byte   0xff,0xff,0xff,0xff # 400-800
        
.globl base_1066_1
base_1066_1:  // 533MHz Micron -1875E JEDEC DDR3-1066F (7-7-7)
.byte   0x06,0x05,0x05,10   # 533-400
.byte   0x07,0x06,0x05,12   # 533-533
.byte   0xff,0xff,0xff,0xff # 533-667
.byte   0xff,0xff,0xff,0xff # 533-800

.globl base_1333_1
base_1333_1:  // 667MHz Micron -15F   JEDEC DDR3-1333G (8-8-8)
.byte   0x05,0x05,0x04,9    # 667-400
.byte   0x07,0x06,0x05,12   # 667-533
.byte   0x08,0x07,0x06,15   # 667-667
.byte   0xff,0xff,0xff,0xff # 667-800

.globl base_1600_1
base_1600_1:  // 800MHz Micron -125F  JEDEC DDR3-1600H (9-9-9)
.byte    0x05,0x05,0x04,9  # 800-400
.byte    0x06,0x06,0x04,11 # 800-533
.byte    0x08,0x07,0x06,15 # 800-667
.byte    0x09,0x08,0x07,19 # 800-800

.globl letter_grade_2
letter_grade_2: 
.globl base_800_2
base_800_2:  // Unused
.byte   0xff,0xff,0xff,0xff # 400-400
.byte   0xff,0xff,0xff,0xff # 400-533
.byte   0xff,0xff,0xff,0xff # 400-667
.byte   0xff,0xff,0xff,0xff # 400-800
        
.globl base_1066_2
base_1066_2:  // 533MHz Micron -1875 JEDEC DDR3-1066G (8-8-8)
.byte   0x06,0x05,0x05,10   # 533-400
.byte   0x08,0x06,0x06,13   # 533-533
.byte   0xff,0xff,0xff,0xff # 533-667
.byte   0xff,0xff,0xff,0xff # 533-800

.globl base_1333_2
base_1333_2:  // 667MHz Micron -15E   JEDEC DDR3-1333H (9-9-9)
.byte   0x06,0x05,0x05,10   # 667-400
.byte   0x07,0x06,0x05,12   # 667-533
.byte   0x09,0x07,0x07,16   # 667-667
.byte   0xff,0xff,0xff,0xff # 667-800

.globl base_1600_2
base_1600_2:  // 800MHz Micron -125E  JEDEC DDR3-1600J (10-10-10)
.byte    0x05,0x05,0x04,9  # 800-400
.byte    0x07,0x06,0x05,12 # 800-533
.byte    0x09,0x07,0x07,16 # 800-667
.byte    0x0A,0x08,0x08,20 # 800-800

.globl letter_grade_3   
letter_grade_3:         
.globl base_800_3
base_800_3:  // Unused
.byte   0xff,0xff,0xff,0xff # 400-400
.byte   0xff,0xff,0xff,0xff # 400-533
.byte   0xff,0xff,0xff,0xff # 400-667
.byte   0xff,0xff,0xff,0xff # 400-800
        
.globl base_1066_3
base_1066_3:  // Unused
.byte   0xff,0xff,0xff,0xff # 533-400
.byte   0xff,0xff,0xff,0xff # 533-533
.byte   0xff,0xff,0xff,0xff # 533-667
.byte   0xff,0xff,0xff,0xff # 533-800

.globl base_1333_3
base_1333_3:  // 667MHz Micron -15   JEDEC DDR3-1333J (10-10-10)
.byte   0x06,0x05,0x05,10   # 667-400
.byte   0x08,0x06,0x06,13   # 667-533
.byte   0x0A,0x07,0x08,17   # 667-667
.byte   0xff,0xff,0xff,0xff # 667-800

.globl base_1600_3
base_1600_3:  // 800MHz Micron -125  JEDEC DDR3-1600K (11-11-11)
.byte    0x06,0x05,0x05,10 # 800-400
.byte    0x07,0x06,0x05,12 # 800-533
.byte    0x09,0x07,0x07,16 # 800-667
.byte    0x0B,0x08,0x09,21 # 800-800

.globl speed_table
speed_table:    
.word   1, 133  // FTM Mode
.word   2, 333  // 2x mode
.word   3, 400  // 2x mode
.word   5, 667  // 4x mode
.word   6, 800  // 4x mode
.word   7, 933  // 933 4x mode (Not supported)
.word   8, 1066  // 1066 4x mode (Not supported)
.word   4, 533  // 2x mode (default)


#define MC_SPEED        400
#define ADD_WIDTH       0       // A[12:0] Assume small memory
#define DUAL_RANK       0
#define DDR_BUS16       1
#define DDR_BUS8        0
#define DDR_X16         1       // Assume we have x16 device during init
#define VDDQ            1       // 1.5V
#define CHIP_SIZE       3       // 512Mb Assume a small DRAM during Init
#define JEDEC_TYPE      5       // DDR2-800D
        
#define TRACE_DELAY     200

#define PHY_STRAP_REG_VAL(a)    ( \
        (a & 0xFFF) << PHY_CONTROL_REGS_STRAP_CONTROL_MHZ_STRT \
        ) 
/*      \
        ((ADD_WIDTH << PHY_CONTROL_REGS_STRAP_CONTROL_AD_WIDTH_STRT) & PHY_CONTROL_REGS_STRAP_CONTROL_AD_WIDTH_MASK) | \
        (DUAL_RANK << PHY_CONTROL_REGS_STRAP_CONTROL_DUAL_RANK_STRT) | \
        (DDR_BUS16 << PHY_CONTROL_REGS_STRAP_CONTROL_BUS16_STRT) | \
        (DDR_BUS8 << PHY_CONTROL_REGS_STRAP_CONTROL_BUS8_STRT) | \
        (DDR_X16 << PHY_CONTROL_REGS_STRAP_CONTROL_CHIP_WIDTH_STRT) | \
        ((VDDQ << PHY_CONTROL_REGS_STRAP_CONTROL_VDDQ_STRT) & PHY_CONTROL_REGS_STRAP_CONTROL_VDDQ_MASK) | \
        ((CHIP_SIZE << PHY_CONTROL_REGS_STRAP_CONTROL_CHIP_SIZE_STRT) & PHY_CONTROL_REGS_STRAP_CONTROL_CHIP_SIZE_MASK) | \
        ((JEDEC_TYPE << PHY_CONTROL_REGS_STRAP_CONTROL_JEDEC_TYPE_STRT) & PHY_CONTROL_REGS_STRAP_CONTROL_JEDEC_TYPE_MASK) | 1 
*/      
#define PHY_STRAP_REG_VAL_FIXED  \
        ((MC_SPEED << PHY_CONTROL_REGS_STRAP_CONTROL_MHZ_STRT) & PHY_CONTROL_REGS_STRAP_CONTROL_MHZ_MASK) |  \
        ((ADD_WIDTH << PHY_CONTROL_REGS_STRAP_CONTROL_AD_WIDTH_STRT) & PHY_CONTROL_REGS_STRAP_CONTROL_AD_WIDTH_MASK) | \
        (DUAL_RANK << PHY_CONTROL_REGS_STRAP_CONTROL_DUAL_RANK_STRT) | \
        (DDR_BUS16 << PHY_CONTROL_REGS_STRAP_CONTROL_BUS16_STRT) | \
        (DDR_BUS8 << PHY_CONTROL_REGS_STRAP_CONTROL_BUS8_STRT) | \
        (DDR_X16 << PHY_CONTROL_REGS_STRAP_CONTROL_CHIP_WIDTH_STRT) | \
        ((VDDQ << PHY_CONTROL_REGS_STRAP_CONTROL_VDDQ_STRT) & PHY_CONTROL_REGS_STRAP_CONTROL_VDDQ_MASK) | \
        ((CHIP_SIZE << PHY_CONTROL_REGS_STRAP_CONTROL_CHIP_SIZE_STRT) & PHY_CONTROL_REGS_STRAP_CONTROL_CHIP_SIZE_MASK) | \
        ((JEDEC_TYPE << PHY_CONTROL_REGS_STRAP_CONTROL_JEDEC_TYPE_STRT) & PHY_CONTROL_REGS_STRAP_CONTROL_JEDEC_TYPE_MASK) | \
        1

#if DDRDEBUG
#define SETLEDS1(a,b,c,d)                    \
        li      a0,(((a)<<24)|((b)<<16)|((c)<<8)|(d)) ; \
        bal     board_setleds; \
        nop
#else
#define SETLEDS1(a,b,c,d)
#endif
        
                
#define DDR3_LOOKUP1(a,b,c)                    \
        li      a0,(((a)<<16)|((b)<<8)|(c)) ; \
        bal     ddr3_look_up; \
        nop


/*  *********************************************************************
        *  BOARD_DRAMINIT
        *
        *  This routine should activate memory.
        *
        *  Input parameters:
        *               None
        *
        *  Return value:
        *               None
        *
        *  Registers used:
        *               can use all registers.
        ********************************************************************* */
LEAF(board_draminit)

        .set    noreorder

        move    s0,ra
        move	s2,a2

        SETLEDS1('-','-','-', '-')

/*
        mtc0    zero,C0_TLBHI
        nop
        li      t2,0xffff << 13 // 256M pages
        nop
        mtc0    t2,C0_PGMASK          
        nop
        // global, valid, allow writes, cachable, low DDR alias
        li      t2,( 1 | 2 | 4 | (3 << 3)) | ((0x00000000 >> 12) << 6)
        nop
        mtc0    t2,C0_TLBLO0
        nop
        // global, valid, allow writes, cachable, high DDR
        li      t2,( 1 | 2 | 4 | (3 << 3)) | ((0x20000000 >> 12) << 6)
        nop
        mtc0    t2,C0_TLBLO1
        nop
        mtc0    zero,C0_INX // first TLB entry
        nop
        nop
        tlbwi
        sync
*/      

/*        
        li      t1, 0x10000000  
        mtc0    t1, C0_TLBHI   // TLB HI Register
        nop
        li      t2, 0x7fff << 13 // 128M pages
        nop
        mtc0    t2, C0_PGMASK          
        nop
        // global, valid, allow writes, cachable, low DDR alias
        li      t3, (1 | 2 | 4 | (3 << 3)) | ((0x20000000 >> 12) << 6)
        nop
        mtc0    t3, C0_TLBLO0  // first TLB entry
        nop
        li      t4, (1 | 2 | 4 | (3 << 3)) | ((0x28000000 >> 12) << 6)
        mtc0    t4, C0_TLBLO1  // second TLB entry
	nop
        mtc0    zero, C0_INX // second TLB entry
	nop
        tlbwi
        sync
	
	mfc0    t1, t4
	and    	t1, t1, 0xfffffffb
	mtc0   	t1, t4	
*/


        li	s3, 10
retry_draminit:
        move	a2,s2
		
        SETLEDS1('P','H','Y','S')

/***** Load MEMC Base *************************************/
        li      t0, MEMC_BASE

/***** Disable Auto refresh ******************************/     
        li      t1, 0x1FC00                 // Set the tREFI to 0xFF
        sw      t1, MC_CSR_CLKS(t0)

/***** Assert DDR_RST_N For DDR3 DRAMs*********************/     
        li      t1, 0x310
        sw      t1, MC_CSR_DCMD(t0)

/***** Wait after Reset for 500us for DDR3. DDR2 requires 200us after clocks are stable*****/
#if     SILICON
        li      t1, 0x50000              // Wait loop count
1:      bne     t1, zero, 1b
        sub     t1, 1
#endif  
	
/****** Wait for PHY to get out of Reset ****************/
#if     SILICON
        li      t2, 0x10000              // Timeout value
#else
        li      t2, 0x1000               // Timeout value
#endif  
        li      t3, 1

wait_phy:
        beq     t2, zero, phy_timeout
        nop

        lw      t4, MC_CSR_PHY_ST(t0)
        andi    t4, (MC_CSR_PHY_ST_SW_RESET_MASK \
                     | MC_CSR_PHY_ST_HW_RESET_MASK  \
                     | MC_CSR_PHY_ST_POR_RDY_MASK)
        bne     t4, t3, wait_phy
        sub     t2, t3

        b       phy_ready
        nop

/****** Error Case Quit!! What do we do on the real system *****/
phy_timeout:
        SETLEDS1('P','H','Y','T')
        b       memc_init_end_error
        nop

phy_ready:
/***** Detect DDR2/3 from Jedec Straps ******/
        sw      zero, PHY_CONTROL_REGS_STRAP_CONTROL(t0)  // Clear Strap Control Register
        sw      zero, PHY_CONTROL_REGS_STRAP_CONTROL2(t0) // Clear Strap Control2 Register
		bnez    a2, 1f                                    // If a2 != 0, it contains Strap Status from Mips
	    move    t1, a2
//      lw      t1, PHY_CONTROL_REGS_STRAP_STATUS(t0)
1:      
        move    t2, t1
        li      t3, PHY_CONTROL_REGS_STRAP_STATUS_STRAPS_VLD_MASK
        and     t2, t3
        beqz    t2, 2f          // If Straps not valid, jump
        nop
        
// DQ Straps are valid
        move    t2, t1
        lui     t3, 0x8000
        and     t2, t3
        LOADREL(t3, ddr3_333MHz)
        sw      t2, 0(t3)      // Mark if it is 333MHz DDR3
        move    t2, t1
        li      t3, (PHY_CONTROL_REGS_STRAP_STATUS_JEDEC_TYPE_MASK | \
                     PHY_CONTROL_REGS_STRAP_STATUS_SPEED_LO_MASK   | \
                     PHY_CONTROL_REGS_STRAP_STATUS_SPEED_HI_MASK)
        and     t2, t3         // Keep only Jedec and Speed fields
        move    t7, t1
        li      t3, PHY_CONTROL_REGS_STRAP_STATUS_JEDEC_TYPE_MASK
        and     t7, t3
        srl     t7, PHY_CONTROL_REGS_STRAP_STATUS_JEDEC_TYPE_STRT  // t7 keeps JEDEC Type 
        li      t3, 21
        bgt     t7, t3, jedec_err       // IF Jedec value > 21, Error
        nop
        li      t3, 9
        bge     t7, t3, 1f      // IF Jedec value >= 9, It is DDR3
        li      t6, 1
        
        li      t6, 0           // It is DDR2
1:      
        move    t2, t1
        and     t2, PHY_CONTROL_REGS_STRAP_STATUS_SPEED_LO_MASK    // Bits [1:0]
        srl     t2, PHY_CONTROL_REGS_STRAP_STATUS_SPEED_LO_STRT
        move    t3, t1
        and     t3, PHY_CONTROL_REGS_STRAP_STATUS_SPEED_HI_MASK    // Bit [2]
        srl     t3, PHY_CONTROL_REGS_STRAP_STATUS_SPEED_HI_STRT
        sll     t3, 2
        or      t2, t3
        move    t9, t2          // t9 keeps Speed Strap Value
        b       3f
        nop

2:      // DQ Straps are not set. Use default: DDR3, 533MHz
        li      t6, 1                   // DDR3
        li      t9, 7                   // 7: 533MHz
        li      t7, JEDEC_DEFAULT       // Default (12): sg187E JEDEC DDR3-1066F (7-7-7)
3:      
        
        move    t2, t9
        and     t2, 3
        sll     t2, PHY_CONTROL_REGS_STRAP_STATUS_SPEED_LO_STRT 
        move    t1, t9
        and     t1, 4
        sll     t1, PHY_CONTROL_REGS_STRAP_STATUS_SPEED_HI_STRT-2
        or      t1, t2                                             // Add Speed Field
        move    t2, t7
        sll     t2, PHY_CONTROL_REGS_STRAP_STATUS_JEDEC_TYPE_STRT
        or      t1, t2                                             // Add JEDEC Field
        or      t1, PHY_CONTROL_REGS_STRAP_STATUS_STRAPS_VLD_MASK  // Set Valid bit
        sw      t1, PHY_CONTROL_REGS_STRAP_CONTROL(t0)             // Copy to Control
        
        beqz    t6, 1f
        li      t1, 0x101       // DDR3 Type
        b       2f
        nop
1:
        li      t1, 0x100       // DDR2 Type
2:   
/***** Set Page policy to Cas Time check and DRAM Type to DDR2/DDR3 ***/
        sw      t1, MC_CSR_DRAM_CFG(t0)
        b       find_speed
        nop
        
jedec_err:
        li      t1, MC_CSR_STAT_ERR_JEDEC_MASK | MC_CSR_STAT_ST_JEDEC_MASK
        lw      t2, MC_CSR_STAT(t0)
        or      t2, t1
        sw      t2, MC_CSR_STAT(t0)     // Mark Status Error for JEDEC Strap error
        SETLEDS1('J','D','C','E')
        b       memc_init_end_error
        nop
        
find_speed:     
        LOADREL(t2, speed_table)
        li      t1, 8
        mult    t1, t9
        mflo    t1
        add     t2, t1
        lw      a1, 0(t2)
        lw      t8, 4(t2)
        
/***  a1 keeps the DDR speed pointer to tables ****/
/***  a3 keeps the DDR size value or 0xff for auto-detect ****/
/***  t6 == 0 => DDR2 Type, t6 == 1 => DDR3 Type ****/
/***  t7 temporarily stores Jedec Type ****/
/***  t8 temporarily Stores DDR MHz ****/
/***  t9 temporarily Stores Clock Strap value: 0 value is for FTM mode.  ****/

        move    t2, t8
        sll     t2, PHY_CONTROL_REGS_STRAP_CONTROL_MHZ_STRT   // Move MHz value to correct bit location
        lw      t1, PHY_CONTROL_REGS_STRAP_CONTROL(t0)
        li      t3, ~PHY_CONTROL_REGS_STRAP_STATUS_MHZ_MASK
        and     t1, t3                                        // Clear MHz field
        or      t2, t1                                        // Add new MHz value
        or      t2, PHY_CONTROL_REGS_STRAP_STATUS_STRAPS_VLD_MASK  // Set Valid bit
        sw      t2, PHY_CONTROL_REGS_STRAP_CONTROL(t0)        // Update the strap control register with MHz value from speed table
        
        bnez    t9, 1f                                        // If not FTM mode, (clock strap != 0) continue w/ normal init
        nop
        sw      zero, PHY_CONTROL_REGS_VDL_CALIBRATE(t0)        // Clear VDL Calibration Register
        li      t1, (PHY_CONTROL_REGS_VDL_CALIBRATE_FTM_MASK)
        sw      t1, PHY_CONTROL_REGS_VDL_CALIBRATE(t0)          // Set FTM Mode
        b       pll_lock
1:      
        
pll_setup:      
        beqz    t6, 1f                  // If DDR2, jump
        nop
//      sub     a1, 2                   // DDR3 speed table starts at 667 (333MHz)
        sub     a1, 3                   // DDR3 speed table starts at 800 (400MHz)
1:      
        li      t1, 250
        bltu    t8, t1, slow_speed
        nop
        li      t1, 500
        bltu    t8, t1, medium_speed
        nop

high_speed:             
        li      t1, 1
        li      t2, 16
        b       1f
        nop

medium_speed:   
        li      t1, 2
        li      t2, 32
        b       1f
        nop

slow_speed:     
        li      t1, 4
        li      t2, 64
        

/******  Update PLL Dividers based on the DDR Frequency ********/       
1:      
        sll     t1, PHY_CONTROL_REGS_PLL_DIVIDERS_POST_DIV_STRT   // Move Post_div value to correct bit location
        sll     t2, PHY_CONTROL_REGS_PLL_DIVIDERS_NDIV_STRT       // Move n_div value to correct bit location
        or      t1, t2                                            // Add both fields together
        lw      t3, PHY_CONTROL_REGS_PLL_DIVIDERS(t0)             // Load original PLL Divider register value
        li      t4, ~PHY_CONTROL_REGS_PLL_DIVIDERS_NDIV_MASK      
        li      t5, ~PHY_CONTROL_REGS_PLL_DIVIDERS_POST_DIV_MASK  
        and     t3, t4                                            // Clear ndiv field
        and     t3, t5                                            // Clear post_div field
        or      t1, t3                                            // Add new post_div and ndiv values to the old value
        sw      t1, PHY_CONTROL_REGS_PLL_DIVIDERS(t0)             // Update the register

/****** Bring PLL out of reset ************/

        lw      t3, PHY_CONTROL_REGS_PLL_CONFIG(t0)
        li      t4, ~PHY_CONTROL_REGS_PLL_CONFIG_RESET_MASK
        and     t3, t4
        sw      t3, PHY_CONTROL_REGS_PLL_CONFIG(t0)               // Enable PLL

#if SKIP_PLL_VDL_INIT
#else
/****** Wait for PLL to Lock ****************/
        li      t2, 0x10000              // Timeout value
        li      t3, 1

wait_pll_lock:
        beq     t2, zero, pll_lock_timeout
        nop

        lw      t4, PHY_CONTROL_REGS_PLL_STATUS(t0)
        andi    t4, PHY_CONTROL_REGS_PLL_STATUS_LOCK_MASK
        bne     t4, t3, wait_pll_lock
        sub     t2, t3

        b       pll_lock
        nop

/****** Error Case Quit!! What do we do on the real system *****/
pll_lock_timeout:
        SETLEDS1('P','L','L','T')
        b       memc_init_end_error
        nop

#endif
pll_lock:
/****** Start ZQ Calibration ****************/

        sw      zero, PHY_CONTROL_REGS_ZQ_PVT_COMP_CTL(t0)
        li      t1, ZQ_PVT_SAMPLE_EN
        sw      t1, PHY_CONTROL_REGS_ZQ_PVT_COMP_CTL(t0)       // Start ZQ Calibration

#if SKIP_PLL_VDL_INIT
#else
        bnez    t9, 1f
        nop
        li      a1, 0                                          // FTM mode
        b       vdl_lock
        nop
1:      

/****** Start PHY VDL Calibration ****************/
        sw      zero, PHY_CONTROL_REGS_VDL_CALIBRATE(t0)        // Clear VDL Calibration Register
        li      t1, (PHY_CONTROL_REGS_VDL_CALIBRATE_AUTO_MASK | \
                     PHY_CONTROL_REGS_VDL_CALIBRATE_ONCE_MASK | \
                     PHY_CONTROL_REGS_VDL_CALIBRATE_FAST_MASK)
        sw      t1, PHY_CONTROL_REGS_VDL_CALIBRATE(t0)          // Start a VDL Calibration

        li      t1, 0x100                                       // timeout_ns 1000
1:      bne     t1, zero, 1b
        sub     t1, 1
        
        li      t2, 0x100                                        // Loop count value
        li      t3, 1
        li      t4, (PHY_CONTROL_REGS_VDL_CALIB_STATUS_IDLE_MASK)
        lw      t1, PHY_CONTROL_REGS_VDL_CALIB_STATUS(t0)      // Load VDL Status Register
        and     t1, t4
        beq     t1, t4, 2f                                     // VDL locked and Idle?
        nop

vdl_loop:
        beq     t2, zero, vdl_lock_timeout
        nop

        li      t1, 0x100                                       // timeout_ns 1000
1:      bne     t1, zero, 1b
        sub     t1, 1
        
        lw      t1, PHY_CONTROL_REGS_VDL_CALIB_STATUS(t0)      // Load VDL Status Register
        and     t1, t4
        bne     t1, t4, vdl_loop                               // VDL locked and Idle?
        sub     t2, t3

2:      sw      zero, PHY_CONTROL_REGS_VDL_CALIBRATE(t0)       // Clear VDL Calibration Register
        b       vdl_lock
        nop


vdl_lock_timeout:
        sw      zero, PHY_CONTROL_REGS_VDL_CALIBRATE(t0)        // Clear VDL Calibration Register
        li      t1, MC_CSR_STAT_ERR_VDL_MASK | MC_CSR_STAT_ST_VDL_MASK
        lw      t2, MC_CSR_STAT(t0)
        or      t2, t1
        sw      t2, MC_CSR_STAT(t0)             // Mark Status Error for VDL error
        
        SETLEDS1('V','D','L','1')
        b       memc_init_end_error
        nop

#endif
vdl_lock:
        li      t1, MC_CSR_STAT_ST_VDL_MASK     // Mark Status register for VDL done
        lw      t2, MC_CSR_STAT(t0)
        or      t2, t1
        sw      t2, MC_CSR_STAT(t0)
        
/****** Wait for ZQ Calibration to Finish ****************/
        li      t2, 0x100              // Timeout value
        li      t3, 1
        li      t5, ZQ_PVT_SAMPLE_DONE
        lw      t4, PHY_CONTROL_REGS_ZQ_PVT_COMP_CTL(t0)
        and     t4, t5
        bne     t4, zero, 2f
        nop

wait_zq_done:
        beq     t2, zero, zq_timeout
        nop

        li      t1, 0x100                                       // timeout_ns 100
1:      bne     t1, zero, 1b
        sub     t1, 1
        
        lw      t4, PHY_CONTROL_REGS_ZQ_PVT_COMP_CTL(t0)
        and     t4, t5
        beq     t4, zero, wait_zq_done
        sub     t2, t3

2:
        li      t1, (PHY_CONTROL_REGS_ZQ_PVT_COMP_CTL_ADDR_OVR_EN_MASK | \
                     PHY_CONTROL_REGS_ZQ_PVT_COMP_CTL_DQ_OVR_EN_MASK   | \
                     0x3F)  // ADDR_PD/ND and DQ_PD/ND max value
        sw      t1, PHY_CONTROL_REGS_ZQ_PVT_COMP_CTL(t0)
        
        li      t1, MC_CSR_STAT_ST_ZQ_MASK      // Mark Status register for ZQ done
        lw      t2, MC_CSR_STAT(t0)
        or      t2, t1
        sw      t2, MC_CSR_STAT(t0)
        
        SETLEDS1('Z','Q','D','N')
        b       zq_done
        nop

zq_timeout:
        li      t1, MC_CSR_STAT_ERR_ZQ_MASK | MC_CSR_STAT_ST_ZQ_MASK
        lw      t2, MC_CSR_STAT(t0)
        or      t2, t1
        sw      t2, MC_CSR_STAT(t0)             // Mark Status Error for ZQ error
        SETLEDS1('Z','Q','T','O')
        b       memc_init_end_error
        nop

zq_done:
/****** ZQ Calibration is Done ****************/
        
        lw      t9, PHY_CONTROL_REGS_VDL_CALIB_STATUS(t0)        // Load VDL Status Register
        move    t7, t9
        li      t1, PHY_CONTROL_REGS_VDL_CALIB_STATUS_TOTAL_MASK
        and     t9, t1
        srl     t9, PHY_CONTROL_REGS_VDL_CALIB_STATUS_TOTAL_STRT // Get the Calib_Total value
        srl     t9, 4                                            // t9 = calib_steps variable Calib_Total[9:4]
        li      t1, PHY_CONTROL_REGS_VDL_CALIB_STATUS_BYTESEL_MASK // Determine whether it is in byte or bit mode
        and     t7, t1
        
/****** Start PHY VDL Steps Calibration ****************/

        beqz    t6, ddr2_calib                             // If DDR2, skip
        nop

#if RD_EN_OVR
        li      t1, (PHY_CONTROL_REGS_VDL_CALIBRATE_STEPS_MASK)
        sw      t1, PHY_CONTROL_REGS_VDL_CALIBRATE(t0)          // Start a VDL Steps Calibration

        li      t1, 0x100                                       // timeout_ns 1000
1:      bne     t1, zero, 1b
        sub     t1, 1
        
        li      t2, 0x100                                        // Loop count value
        li      t3, 1
        li      t4, (PHY_CONTROL_REGS_VDL_CALIB_STATUS_IDLE_MASK)
        lw      t1, PHY_CONTROL_REGS_VDL_CALIB_STATUS(t0)       // Load VDL Status Register
        and     t1, t4
        bne     t1, zero, 2f                                    // VDL locked and Idle?
        nop

#if SKIP_PLL_VDL_INIT
#else
vdl_loop_2:
        beq     t2, zero, vdl_lock_timeout_2
        nop

        li      t1, 0x100                                       // timeout_ns 1000
1:      bne     t1, zero, 1b
        sub     t1, 1
        
        lw      t1, PHY_CONTROL_REGS_VDL_CALIB_STATUS(t0)       // Load VDL Status Register
        and     t1, t4
        bne     t1, t4, vdl_loop_2                              // VDL locked and Idle?
        sub     t2, t3

2:      sw      zero, PHY_CONTROL_REGS_VDL_CALIBRATE(t0)        // Clear VDL Calibration Register
        b       vdl_lock_2
        nop


vdl_lock_timeout_2:
        sw      zero, PHY_CONTROL_REGS_VDL_CALIBRATE(t0)        // Clear VDL Calibration Register
        li      t1, MC_CSR_STAT_ERR_VDL_STEPS_MASK | MC_CSR_STAT_ST_VDL_STEPS_MASK
        lw      t2, MC_CSR_STAT(t0)
        or      t2, t1
        sw      t2, MC_CSR_STAT(t0)                             // Mark Status Error for VDL error
        
        SETLEDS1('V','D','L','2')
        b       memc_init_end_error
        nop

#endif
vdl_lock_2:
        li      t1, MC_CSR_STAT_ST_VDL_STEPS_MASK               // Mark Status register for VDL done
        lw      t2, MC_CSR_STAT(t0)
        or      t2, t1
        sw      t2, MC_CSR_STAT(t0)

/* SW workaround for Read_En Calibration bug  ********/
        lw      t1, PHY_CONTROL_REGS_VDL_CALIB_STATUS(t0)          // Load VDL Status Register
        li      t4, (PHY_CONTROL_REGS_VDL_CALIB_STATUS_TOTAL_MASK)
        and     t1, t4                                             
        srl     t1, PHY_CONTROL_REGS_VDL_CALIB_STATUS_TOTAL_STRT   // Get Calib_Total Value
        srl     t1, 4                                              // t1 = Steps_in_cycle is Calib_Total[9:4]
        
        lw      t8, PHY_CONTROL_REGS_STRAP_CONTROL(t0)             // Read Straps Control Register
        li      t3, PHY_CONTROL_REGS_STRAP_CONTROL_MHZ_MASK
        and     t8, t3
        srl     t8, PHY_CONTROL_REGS_STRAP_CONTROL_MHZ_STRT        // Get MHz value

        li      t4, 1000000
        divu    t4, t8                                             // t8 = ddr_speed
        mflo    t4                                                 // t4 = 1000000/ddr_speed
        divu    t4, t1                                             // t4 = step_size = (1000000/ddr_speed)/steps_in_cycle
        mflo    t4                                                 // t4 = step_size

        li      t2, TRACE_DELAY
        divu    t2, t4
        mflo    t2                                                 // t2 = trace_steps = TRACE_DELAY / step_size (can reuse t4)
        
//      bnez    t7, vdl_byte_mode                                  // t7 keeps VDL auto_calib calib_byte_sel bit (1 <<22 is byte mode)
        li      t3, 550
        ble     t8, t3, vdl_byte_mode
        nop

vdl_bit_mode:
        // Program Bit RD_EN OVRD Register (for DDR_SPEED > 550MHz)
        // ovr_step = calib_steps + trace_steps + 8;
        addi    t2, 8
        add     t2, t9
        li      t4, 63
        ble     t2, t4, 1f
        nop
        li      t2, 63                                             // If ovr_step > 63, then use 63
1:      
        li      t4, PHY_WORD_LANE_VDL_OVRIDE_OVR_EN_MASK
        or      t2, t4
        li      t4, PHY_WORD_LANE_VDL_OVRIDE_OVR_FORCE_MASK
        or      t2, t4                                             // t2 = over_step
        sw      t2, PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT_RD_EN(t0)
        sw      t2, PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_BIT_RD_EN(t0)
        b       4f
        nop

vdl_byte_mode:
        // Program Byte RD_EN OVRD Register (for DDR_SPEED <= 550MHz)
        // ovr_step = calib_steps + ((trace_steps + 8) / 2);
        addi    t2, 8
        srl     t2, 1
        add     t2, t9
        li      t4, 63
        ble     t2, t4, 1f
        nop
        li      t2, 63                                             // If ovr_step > 63, then use 63
1:      
        li      t4, PHY_WORD_LANE_VDL_OVRIDE_OVR_EN_MASK
        or      t2, t4
        li      t4, PHY_WORD_LANE_VDL_OVRIDE_OVR_FORCE_MASK
        or      t2, t4                                             // t2 = over_step
        sw      t2, PHY_WORD_LANE_0_VDL_OVRIDE_BYTE_RD_EN(t0)
//      sw      t2, PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_RD_EN(t0)

4:
        
#endif

        b       calib_done
        nop

ddr2_calib:
        li      t2, 20
        li      t4, PHY_WORD_LANE_VDL_OVRIDE_OVR_EN_MASK
        or      t2, t4
        li      t4, PHY_WORD_LANE_VDL_OVRIDE_OVR_FORCE_MASK
        or      t2, t4                                             // t2 = over_step
        sw      t2, PHY_WORD_LANE_0_VDL_OVRIDE_BYTE_RD_EN(t0)
//      sw      t2, PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_RD_EN(t0)

        li      t2, 0
        li      t4, PHY_WORD_LANE_VDL_OVRIDE_OVR_EN_MASK
        or      t2, t4
        li      t4, PHY_WORD_LANE_VDL_OVRIDE_OVR_FORCE_MASK
        or      t2, t4                                             // t2 = over_step
        li      t4, PHY_WORD_LANE_VDL_OVRIDE_BYTE_SEL_MASK
        or      t2, t4                                             // Set Byte_sel mode
        sw      t2, PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT_RD_EN(t0)
        sw      t2, PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_BIT_RD_EN(t0)
        
calib_done:

/****** Calibrations are done *****************/

/****** Enable Virtual VTT *****************/
        li      t2, Virtual_VTT_Context
        sw      t2, PHY_CONTROL_REGS_VIRTUAL_VTT_CONNECTIONS(t0) // Program VTT connections
        li      t2, Virtual_VTT_Override
        sw      t2, PHY_CONTROL_REGS_VIRTUAL_VTT_OVERRIDE(t0)    // Program VTT override
        
        li      t2, PHY_CONTROL_REGS_VIRTUAL_VTT_CONTROL_ENABLE_ERROR_RESET_MASK
        sw      t2, PHY_CONTROL_REGS_VIRTUAL_VTT_CONTROL(t0)     // Turn Off Virtual VTT

        li      t2, (PHY_CONTROL_REGS_VIRTUAL_VTT_CONTROL_ENABLE_CKE_IDLE_MASK | \
                     PHY_CONTROL_REGS_VIRTUAL_VTT_CONTROL_ENABLE_CS_IDLE_MASK | \
                     PHY_CONTROL_REGS_VIRTUAL_VTT_CONTROL_ENABLE_CTL_IDLE_MASK)
        sw      t2, PHY_CONTROL_REGS_VIRTUAL_VTT_CONTROL(t0)     // Turn On Virtual VTT

        beqz    t6, 1f                          // If DDR2 DRAM, jump
        li      t5, 2                           // DDR2 uses 1.8V

/*** DDR3 Related Programming ****/     
/*** t5: keeps Voltage value ****/      
        li      t5, 1                                              // DDR3 uses 1.5V
        li      t1, 1
        sw      t1, PHY_WORD_LANE_0_WR_PREAMBLE_MODE(t0)           // Enable Pulse DQS Preamble for Wordlane0
1:      
        
/*** Reset dq_odt_te_adj bit (drives ODT for 1 extra cycle after the last DQ/DQS) FIXME: May be different at high speeds!! ****/
        lw      t1, PHY_WORD_LANE_0_READ_CONTROL(t0)
        li      t2, ~PHY_WORD_LANE_0_READ_CONTROL_DQ_ODT_TE_ADJ_MASK
        and     t1, t2
        sw      t1, PHY_WORD_LANE_0_READ_CONTROL(t0)

/****** Store Voltage value in the Strap Control register **************/
        lw      t1, PHY_CONTROL_REGS_STRAP_CONTROL(t0)                // Load old control value
        li      t3, ~PHY_CONTROL_REGS_STRAP_CONTROL_VDDQ_MASK
        and     t1, t3                                                // Clear Voltage Field
        sll     t2, t5, PHY_CONTROL_REGS_STRAP_CONTROL_VDDQ_STRT      // Move Voltage value to correct field
        andi    t2, PHY_CONTROL_REGS_STRAP_CONTROL_VDDQ_MASK          // Make sure the there is no overflow
        or      t1, t2                                                // Add the new field
        sw      t1, PHY_CONTROL_REGS_STRAP_CONTROL(t0)

/****** Set control pad strength to half ********/
        lw      t2, PHY_CONTROL_REGS_DRIVE_PAD_CTL(t0)
//        or      t2, (PHY_CONTROL_REGS_DRIVE_PAD_CTL_HALF_STRENGTH_MASK)  // FIXME: Program Drive strength to Half-Strength (unterminated)
        li      t1, ~PHY_CONTROL_REGS_DRIVE_PAD_CTL_VDDO_VOLTS_MASK
        and     t2, t1
        sll     t4, t5, PHY_CONTROL_REGS_DRIVE_PAD_CTL_VDDO_VOLTS_STRT
        andi    t4, PHY_CONTROL_REGS_DRIVE_PAD_CTL_VDDO_VOLTS_MASK     // Make sure there is no overflow
        or      t2, t4                                                 // Add the new voltage field
        sw      t2, PHY_CONTROL_REGS_DRIVE_PAD_CTL(t0)                 // Set Voltage and drive strength
        
/****** Set wordlane 0 Voltage ********/
        lw      t2, PHY_WORD_LANE_0_DRIVE_PAD_CTL(t0)
        li      t1, ~PHY_WORD_LANE_0_DRIVE_PAD_CTL_VDDO_VOLTS_MASK
        and     t2, t1
        sll     t4, t5, PHY_WORD_LANE_0_DRIVE_PAD_CTL_VDDO_VOLTS_STRT
        andi    t4, PHY_WORD_LANE_0_DRIVE_PAD_CTL_VDDO_VOLTS_MASK      // Make sure there is no overflow
        or      t2, t4                                                 // Add the new voltage field
        li      t1, ~PHY_WORD_LANE_0_DRIVE_PAD_CTL_DQS_ALWAYS_ON_MASK
        and     t2, t1                                                 // Reset DQS toggling mode
        sw      t2, PHY_WORD_LANE_0_DRIVE_PAD_CTL(t0)                  // Set Voltage for Wordlane 0

        li      t1, MC_CSR_STAT_ST_PHYE_MASK    // Mark Status register for PHY Init Ended (PHYE)
        lw      t2, MC_CSR_STAT(t0)
        or      t2, t1
        sw      t2, MC_CSR_STAT(t0)
        


        SETLEDS1('P','H','Y','E')
    

/***** Set arbitor for Burst Round Robin Mode ***/
        lw      t1, MC_CSR_ARB(t0)
        or      t1, 4 << 16
        sw      t1, MC_CSR_ARB(t0)


/* Program MC Timing Registers

 Read each timing parameter based on the speed and then create the
 timing registers and program them. 
*/  
        bnez    t6, ddr3_timing             // Jump if DDR3
        nop

ddr2_timing:            
        
        lw      t8, PHY_CONTROL_REGS_STRAP_CONTROL(t0)         // Read Straps Control Register
        li      t3, PHY_CONTROL_REGS_STRAP_CONTROL_MHZ_MASK
        and     t8, t3
        srl     t8, PHY_CONTROL_REGS_STRAP_CONTROL_MHZ_STRT    // Get MHz value
// Determine the pointer for dram_speed_table by checking the speed
        li      t1, 267
        bgeu    t1, t8, 1f 
        li      a1, 0
        li      t1, 333
        bgeu    t1, t8, 1f
        li      a1, 1
        li      t1, 400
        bgeu    t1, t8, 1f
        li      a1, 2
        li      a1, 3
1:
        LOADREL(t2, ddr2_timing_table)
  
        li      t1, 0x10    // size of ddr2_timing_table element
        mult    t1, a1
        mflo    t1          // ddr2_timing_table offset
        add     t2, t1

        move    t3, zero
        lbu     t1, 0x00(t2)    // tRCD
        andi    t1, 0xf
        move    t3, t1
        lbu     t1, 0x01(t2)    // tCL
        andi    t1, 0xf
    
        move    t4, t1
        and     t4, 0x7
        sll     t4, 20
        sw      t4, MC_CSR_DMODE_0 (t0)   // Write tCL to the MRS register holder

        sll     t1, 4
        or      t3, t1
        lbu     t1, 0x02(t2)    // tWR
        andi    t1, 0xf
    
// Here we create the MRS register values
        move    t4, t1
        li      t5, 1
        subu    t4, t5            // tWR written to DRAM is 1 less than real tWR value
        andi    t4, 0x7
        sll     t4, 25
        li      t5, 0x01030000    // Sequential burst mode, burst of 8, reset DLL
        or      t4, t5
        lw      t5, MC_CSR_DMODE_0 (t0)  
        or      t5, t4
        sw      t5, MC_CSR_DMODE_0 (t0)  // Add tWR to the MRS register holder
    
        sll     t1, 8
        or      t3, t1
        lbu     t1, 0x03(t2)    // tWL
        andi    t1, 0xf
        sll     t1, 12
        or      t3, t1
        lbu     t1, 0x04(t2)    // tRP
        andi    t1, 0xf
        sll     t1, 16
        or      t3, t1
        lbu     t1, 0x05(t2)    // tRRD
        andi    t1, 0xf
        sll     t1, 20
        or      t3, t1
        sw      t3, MC_CSR_TIM1_0(t0)  // Program TIM1_0 register

        move    t3, zero
        lbu     t1, 0x06(t2)    // tRC
        andi    t1, 0x3f
        or      t3, t1
        li      t1, 0x3f        // Use max value for tFAW initially
        sll     t1, 8
        or      t3, t1
        li      t1, 0xfe    // tRFC = 0xfe (Set to max value first.
        sll     t1, 16      // We'll fix it after we determine dram size)
        or      t3, t1

        lbu     t1, 0x08(t2)    // tW2R
        andi    t1, 0x7
        sll     t1, 24
        or      t3, t1
        lbu     t1, 0x09(t2)    // tR2W
        andi    t1, 0x7
        sll     t1, 27
        or      t3, t1
        lbu     t1, 0x0a(t2)    // tR2R
        andi    t1, 0x3
        sll     t1, 30
        or      t3, t1
        sw      t3, MC_CSR_TIM1_1(t0)  // Program TIM1_1 register
    
        move    t3, zero
        lbu     t1, 0x0b(t2)    // tAL
        andi    t1, 0xf
    
        // Here we create the EMRS register values
        move    t4, t1
        andi    t4, 0x7
        sll     t4, 3
        li      t5, 0x384   // RTT=75ohm, OCD Enter
        or      t4, t5
        lw      t5, MC_CSR_DMODE_0 (t0)  
        or      t5, t4
        sw      t5, MC_CSR_DMODE_0 (t0)  // Store required values in EMRS holding register

        or      t3, t1
        lbu     t1, 0x0c(t2)    // tRTP
        andi    t1, 0x7
        sll     t1, 4
        or      t3, t1
        lbu     t1, 0x0d(t2)    // tW2W
        andi    t1, 0x7
        sll     t1, 8
        or      t3, t1
        lbu     t1, 0x0e(t2)    // tWTR
        andi    t1, 0xf
        sll     t1, 12
        or      t3, t1
        sw      t3, MC_CSR_TIM2(t0)    // Program TIM2 register
    
/***** Add the Jedec Field to the Strap Control register ******
        lbu     t1, 0x0f(t2)    // Jedec Type
        andi    t1, 0x1f
        sll     t1, PHY_CONTROL_REGS_STRAP_CONTROL_JEDEC_TYPE_STRT    // Shift Jedec to its location
        andi    t1, PHY_CONTROL_REGS_STRAP_CONTROL_JEDEC_TYPE_MASK    // Make sure it is not too big
        lw      t3, PHY_CONTROL_REGS_STRAP_CONTROL(t0)                // Load old control value
        li      t4, ~PHY_CONTROL_REGS_STRAP_CONTROL_JEDEC_TYPE_MASK
        and     t3, t4                                                // Clear Jedec Field
        or      t3, t1                                                // Add the new field
        sw      t3, PHY_CONTROL_REGS_STRAP_CONTROL(t0)                // Store the new value

*/
        
/***** Set Page policy to Cas Time check and DRAM Type to DDR2  ************/
//        li      t1, 0x100
//        sw      t1, MC_CSR_DRAM_CFG(t0)
        
/***** Enable ODT for writes *************/
        li      t1, 0x104
        sw      t1, MC_CSR_ODT(t0)
        
/***** Wait after Reset for 200us for DDR2. */
#if     SILICON
        li      t1, 0x10000              // Wait loop count
1:      bne     t1, zero, 1b
        sub     t1, 1
#else
        li      t1, 0x10
1:  
        bnez    t1, 1b
        addi    t1, -1
#endif  
	
/***** Turn on CKE ***************/
        li      t1, 0x305
        sw      t1, MC_CSR_DCMD(t0)

        li      t1, 0x200
1:  
        bnez    t1, 1b
        addi    t1, -1
        
/***** Issue Precharge All Banks Command ***/
        li      t1, 0x302
        sw      t1, MC_CSR_DCMD(t0)
        
/***** Issue EMRS2 Command ***/
        sw      zero, MC_CSR_DMODE_2 (t0)
        li      t1, 0x308
        sw      t1, MC_CSR_DCMD(t0)
        
/***** Issue EMRS3 Command ***/
        li      t1, 0x309
        sw      t1, MC_CSR_DCMD(t0)
        
/***** Enable DLL by issuing EMRS Command ***/
        li      t1, 0x300
        sw      t1, MC_CSR_DCMD(t0)
        
/***** Issue MRS Command. Set Reset DLL bit ***/
        li      t1, 0x301
        sw      t1, MC_CSR_DCMD(t0)
        
/***** Issue Precharge All Banks Command ***/
        li      t1, 0x302
        sw      t1, MC_CSR_DCMD(t0)
        
/***** Issue Autorefresh Command ***/
        li      t1, 0x303
        sw      t1, MC_CSR_DCMD(t0)
        li      t1, 0x303
        sw      t1, MC_CSR_DCMD(t0)

#if     SILICON
        li      t1, 0x180
#else
        li      t1, 0x40
#endif
3:  
        bnez    t1, 3b
        addi    t1, -1
        
/***** Issue MRS Command w/ DLL Reset bit set to 0 ***/
        lw      t1, MC_CSR_DMODE_0 (t0)
        li      t2, 0xFEFFFFFF          // Reset DLL reset bit
        and     t1, t2
        sw      t1, MC_CSR_DMODE_0 (t0)
        li      t1, 0x301
        sw      t1, MC_CSR_DCMD(t0)
    
/***** Issue EMRS Command (Enter OCD Calibration)  75 Ohm, Full strength Drive, tAL=tRCD-1 ***/
        li      t1, 0x300
        sw      t1, MC_CSR_DCMD(t0)
        
/***** Issue EMRS Command (Exit OCD Calibration) ***/
        lw      t1, MC_CSR_DMODE_0 (t0)
        li      t2, 0xFFFFFC7F          // Reset OCD field for exit mode
        and     t1, t2
        sw      t1, MC_CSR_DMODE_0 (t0)
        li      t1, 0x300
        sw      t1, MC_CSR_DCMD(t0)
        b       dram_timing_cont1
        nop
        
ddr3_timing:    
        
//   Input:     a2 =    letter grade (bit 16-17),
//                      speed grade (bit 8-12),
//                      exact speed (bit 4-0)

        lw      t1, PHY_CONTROL_REGS_STRAP_CONTROL(t0)
        move    t2, t1
        li      a2, 0
        and     t1, PHY_CONTROL_REGS_STRAP_STATUS_JEDEC_TYPE_MASK
        srl     t1, PHY_CONTROL_REGS_STRAP_STATUS_JEDEC_TYPE_STRT

read_jedec_table:       
        
//      sub     t1, 9                   // DDR3 speed grades start at 9
        LOADREL(t2, jedec_table)
        add     t2, t1
        lbu     t1, 0(t2)               // Read Jedec Letter grade[5:4] and Speed grade[1:0]
        move    a2, zero
        andi    a2, t1, JEDEC_LETTER_GRADE_BITS
        sll     a2, 16-4                // Move letter grade to bit location 16
        andi    t1, JEDEC_SPEED_GRADE_BITS
        sll     t1, 8
        or      a2, t1                  // Add Speed Grade to bit location 8
        or      a2, a1                  // Add DDR Speed to bit location 0
        
        bal     ddr3_look_up_v2 // v0 returns pointer for the reset of the parameters
        nop                     // v1 returns pointer for tCL/tCWL/tAL
        
        li      t1, 0xFFFFFFFF 
        beq     v1, t1, jedec_err
        nop
        
        move    t3, zero
        lbu     t1, 0x00(v1)    // tRCD = tCL
        andi    t1, 0xf
        move    t3, t1
        lbu     t1, 0x00(v1)    // tCL
        andi    t1, 0xf      
        
        sw      t1, PHY_CONTROL_REGS_STRAP_CONTROL2(t0) // Store tCL

        move    t4, t1
        sub     t4, 4
        andi    t4, 7           // tCL in DRAM is 3 bits wide
        sll     t4, 20
        sw      t4, MC_CSR_DMODE_0 (t0)   // Write tCL to the MRS register holder

        sll     t1, 4
        or      t3, t1
        lbu     t1, 0x02(v0)    // tWR
        andi    t1, 0xf
    
        lw      t4, PHY_CONTROL_REGS_STRAP_CONTROL2(t0)
        li      t2, ~PHY_CONTROL_REGS_STRAP_CONTROL2_WR_MASK
        and     t4, t2
        move    t2, t1
        sll     t2, PHY_CONTROL_REGS_STRAP_CONTROL2_WR_STRT
        or      t4, t2
        li      t2, PHY_CONTROL_REGS_STRAP_CONTROL2_DDR3_MASK   // Set DDR3 bit
        or      t4, t2
        sw      t4, PHY_CONTROL_REGS_STRAP_CONTROL2(t0)

        // Here we create the MRS register values
        move    t4, t1
        li      t5, 8
        ble     t4, t5, twr_le_8
        li      t5, 4          // tWR <=8, program twr-4 to DRAM
        
        li      t5, 10
        beq     t4, t5, 1f
        li      t4, 5         // tWR = 10, program 5 to DRAM

        li      t4, 6         // tWR = 12, program 6 to DRAM
        b       1f
        nop
twr_le_8:               
        subu    t4, t5
1:
        andi    t4, 0x7
        sll     t4, 25
        li      t5, 0x01000000    // Sequential burst mode, burst of 8, reset DLL
        or      t4, t5
        lw      t5, MC_CSR_DMODE_0 (t0)  
        or      t5, t4
        sw      t5, MC_CSR_DMODE_0 (t0)  // Add tWR to the MRS register holder
    
        sll     t1, 8
        or      t3, t1
        lbu     t1, 0x01(v1)    // tCWL
        andi    t1, 0xf
        
        lw      t4, PHY_CONTROL_REGS_STRAP_CONTROL2(t0)
        li      t2, ~PHY_CONTROL_REGS_STRAP_CONTROL2_CWL_MASK
        and     t4, t2
        move    t2, t1
        sll     t2, PHY_CONTROL_REGS_STRAP_CONTROL2_CWL_STRT
        or      t4, t2
        sw      t4, PHY_CONTROL_REGS_STRAP_CONTROL2(t0)

        move    t4, t1
        li      t5, 5
        subu    t4, t5         // tCWL is tCL - 5 in DDR programming
        sll     t4, 3          // tCWL starts at bit 3 in DRAM
        ori     t4, (1<<9)     // Enable Dynamic ODT and set RTT_WR to RZQ/4 = 60ohm
#if defined(DDR_HI_TEMP)
        ori     t4, (1<<7)
#endif
        sw      t4, MC_CSR_DMODE_2 (t0)  // Write EMRS2 Value to EMRS2 Register DMODE_2[14:0]
        
        move    t9, t4         // Keep EMRS2 value in t9 for now
        sll     t1, 12
        or      t3, t1          // Add tCWL to t3
        lbu     t1, 0x00(v1)    // tRP = tCL
        andi    t1, 0xf
        sll     t1, 16
        or      t3, t1
        lbu     t1, 0x05(v0)    // tRRD
        andi    t1, 0xf
        sll     t1, 20
        or      t3, t1
        sw      t3, MC_CSR_TIM1_0(t0)  // Program TIM1_0 register

        move    t3, zero
        lbu     t1, 0x06(v0)    // tRC
        andi    t1, 0x3f
        or      t3, t1
        li      t1, 0x3f        // Use max value for tFAW initially
        sll     t1, 8
        or      t3, t1
        li      t1, 0xfe    // tRFC = 0xfe (Set to max value first.
        sll     t1, 16      // We'll fix it after we determine dram size)
        or      t3, t1

        lbu     t1, 0x08(v0)    // tW2R
        andi    t1, 0x3
        sll     t1, 24
        or      t3, t1
        lbu     t1, 0x09(v0)    // tR2W
        andi    t1, 0x7
        sll     t1, 27
        or      t3, t1
        lbu     t1, 0x0a(v0)    // tR2R
        andi    t1, 0x3
        sll     t1, 30
        or      t3, t1
        sw      t3, MC_CSR_TIM1_1(t0)  // Program TIM1_1 register
    
        move    t3, zero
        lbu     t1, 0x02(v1)    // tAL
        andi    t1, 0xf
        or      t3, t1
    
//**** FIXME:    Add correct RTT values *******/
        // Here we create the EMRS register values
        move    t4, t1
        beqz    t4, 1f         // tAL = 0
        nop
        lbu     t5, 0x00(v1)   // tCL
        andi    t5, 0xf
        subu    t5, t4
        li      t7, 1
        beq     t5, t7, 1f     // tAL = tCL - 1
        li      t4, 1
        li      t7, 2
        beq     t5, t7, 1f     // tAL = tCL - 2
        li      t4, 2   
1:      
        lw      t1, PHY_CONTROL_REGS_STRAP_CONTROL2(t0)
        li      t2, ~PHY_CONTROL_REGS_STRAP_CONTROL2_AL_MASK
        and     t1, t2
        move    t2, t4
        sll     t2, PHY_CONTROL_REGS_STRAP_CONTROL2_AL_STRT
        or      t1, t2
        sw      t1, PHY_CONTROL_REGS_STRAP_CONTROL2(t0)

        sll     t4, 3
// RTT_nom is 0. RTT_wr is used.
//        li      t5, 0x4        // RTT_nom=60ohm
//        or      t4, t5
#if     defined(DDR3_ODS_34)
        li      t5, 2            // Set Bit 1 to 1 if 34Ohm Output Drive is Selected
        or      t4, t5
#endif
        lw      t5, MC_CSR_DMODE_0 (t0)  
        or      t5, t4
        sw      t5, MC_CSR_DMODE_0 (t0)  // Store required values in EMRS holding register

        lbu     t1, 0x0c(v0)    // tRTP
        andi    t1, 0x7
        sll     t1, 4
        or      t3, t1
        lbu     t1, 0x0d(v0)    // tW2W
        andi    t1, 0x3
        sll     t1, 8
        or      t3, t1
        lbu     t1, 0x0e(v0)    // tWTR
        andi    t1, 0xf
        sll     t1, 12
        or      t3, t1
        sw      t3, MC_CSR_TIM2(t0)    // Program TIM2 register


/***** Set Page policy to Cas Time check and DRAM Type to DDR3  ************/
//        li      t1, 0x101
//        sw      t1, MC_CSR_DRAM_CFG(t0)
        
/***** Enable ODT for writes and Enable Dynamic ODT ********/
//        li      t1, 0x304
//        sw      t1, MC_CSR_ODT(t0)
        
/****** Deassert Reset For DDR3 DRAMs *************/
        li      t1, 0x10
1:  
        bnez    t1, 1b                    // Wait before releasing DDR_RST_N
        addi    t1, -1
        
        li      t1, 0x311
        sw      t1, MC_CSR_DCMD(t0)       // Deassert DDR_RST_N for DDR3

/***** Wait after Reset for 500us for DDR3. DDR2 requires 200us after clocks are stable*****/
#if     SILICON
        li      t1, 0x50000              // Wait loop count
1:      bne     t1, zero, 1b
        sub     t1, 1
#else
        li      t1, 0x10
1:  
        bnez    t1, 1b
        addi    t1, -1
#endif  
        
/***** Turn on CKE ***************/
2:      
        li      t1, 0x305
        sw      t1, MC_CSR_DCMD(t0)

        li      t1, 0x200
1:  
        bnez    t1, 1b
        addi    t1, -1
        
/***** Issue EMRS2 Command ***/
        li      t1, 0x308
        sw      t1, MC_CSR_DCMD(t0)      // Issue EMRS2 command
        
/***** Issue EMRS3 Command***/
        li      t1, 0x309
        sw      t1, MC_CSR_DCMD(t0)      // Issue EMRS3 command
        
/***** Enable DLL by issuing EMRS Command ***/
        li      t1, 0x300
        sw      t1, MC_CSR_DCMD(t0)      // Issue EMRS command
        
/***** Issue MRS Command. Set Reset DLL bit ***/
        li      t1, 0x301
        sw      t1, MC_CSR_DCMD(t0)      // Issue MRS command
        
/***** Issue ZQL Calibration Command ***/
        li      t1, 0x30d
        sw      t1, MC_CSR_DCMD(t0)
        
        li      t1, 0xC0
1:  
        bnez    t1, 1b
        addi    t1, -1
        
dram_timing_cont1:      
        

        beqz    t6, read_data_dly  // If DDR2 DRAM, skip
        nop

#if RD_EN_OVR
/*****  Run Read_En and Bit Calibrations, but we need to revert some programming first *****/
        

/*-------------------------------------------------------------------------------------------*/

/*      
        lw      t1, PHY_CONTROL_REGS_STRAP_CONTROL(t0)            // Read the Strap Status Reg
        // Program strap control register
        li      t5, ((ADD_WIDTH<< PHY_CONTROL_REGS_STRAP_CONTROL_AD_WIDTH_STRT) & PHY_CONTROL_REGS_STRAP_CONTROL_AD_WIDTH_MASK) | \
                    (DUAL_RANK << PHY_CONTROL_REGS_STRAP_CONTROL_DUAL_RANK_STRT) | \
                    (DDR_BUS16 << PHY_CONTROL_REGS_STRAP_CONTROL_BUS16_STRT) | \
                    (DDR_BUS8 << PHY_CONTROL_REGS_STRAP_CONTROL_BUS8_STRT) | \
                    (DDR_X16 << PHY_CONTROL_REGS_STRAP_CONTROL_CHIP_WIDTH_STRT) | \
                    ((CHIP_SIZE << PHY_CONTROL_REGS_STRAP_CONTROL_CHIP_SIZE_STRT) & PHY_CONTROL_REGS_STRAP_CONTROL_CHIP_SIZE_MASK) | \
                    1 

        or      t1, t5
        sw      t1, PHY_CONTROL_REGS_STRAP_CONTROL(t0)
*/

        lw      t5, MC_CSR_DMODE_0(t0)          // do no reset DLL again w/ MRS command
        move    t9, t5
        li      t2, ~(1 << (8 + 16))
        and     t5, t2
        sw      t5, MC_CSR_DMODE_0 (t0)

        bnez    t6, ddr3                        // Jump if DDR3
        nop

ddr2:   li      t2, ~(0x7 << 16)                // For DDR2 set the burstlen to 2 temporarily
        and     t5, t2
        li      t2, (2 << 16)
        or      t5, t2

ddr3:
//      li      t2, ~(0x7 << 3)                 // Reset tAL to 0
//      and     t5, t2

        sw      t5, MC_CSR_DMODE_0 (t0)

        li      t1, 0x301                       // issue MRS command
        sw      t1, MC_CSR_DCMD(t0)
        li      t1, 0x300                       // issue EMRS command
        sw      t1, MC_CSR_DCMD(t0)

        li      t2, 0x80                        // timeout_ns 100ns 
to_100: bne     t2, zero, to_100
        sub     t2, 1
                                                // clear VDL Calibration register */
        sw      zero, PHY_CONTROL_REGS_VDL_CALIBRATE(t0)        

        /* --------------------------- */
        /* Start Read_En Calibrations. */

        bnez    t6, ddr3_1                      // Jump if DDR3
        nop
ddr2_1:
        li      t2, (PHY_CONTROL_REGS_VDL_CALIBRATE_RD_EN_CAL_MASK | \
                     PHY_CONTROL_REGS_VDL_CALIBRATE_RD_DLY_CAL_MASK | \
                     PHY_CONTROL_REGS_VDL_CALIBRATE_BIT_REFRESH_MASK)
        b       cont_1
        nop
                
ddr3_1:
        li      t2, (PHY_CONTROL_REGS_VDL_CALIBRATE_RD_EN_CAL_MASK | \
                     PHY_CONTROL_REGS_VDL_CALIBRATE_SET_MR_MPR_MASK | \
                     PHY_CONTROL_REGS_VDL_CALIBRATE_RD_DLY_CAL_MASK | \
                     PHY_CONTROL_REGS_VDL_CALIBRATE_BIT_REFRESH_MASK)

cont_1: sw      t2, PHY_CONTROL_REGS_VDL_CALIBRATE(t0)          // start Read_En Calibration

        li      t2, 0x100                                       // timeout_ns 1000
1:      bne     t2, zero, 1b
        sub     t2, 1

                                                                // Read VDL Calibration Status.
        li      t5, 0x100                                       // Loop count value
        li      t3, 1
        li      t4, (PHY_CONTROL_REGS_VDL_CALIB_STATUS_IDLE_MASK)
        lw      t1, PHY_CONTROL_REGS_VDL_CALIB_STATUS(t0)       // Load VDL Status Register
        and     t1, t4
        bne     t1, zero, vdl_exit_3                            // VDL locked and Idle?
        nop

vdl_loop_3: beq     t5, zero, vdl_exit_3
        nop

        li      t2, 0x100                                       // timeout_ns(200)
2:      bne     t2, zero, 2b
        sub     t2, 1

        lw      t1, PHY_CONTROL_REGS_VDL_CALIB_STATUS(t0)       // Load VDL Status Register
        and     t1, t4
        bne     t1, t4, vdl_loop_3                              // VDL locked and Idle?
        sub     t5, t3
vdl_exit_3:

        /* Clear VDL Calibration Register */
        sw      zero, PHY_CONTROL_REGS_VDL_CALIBRATE(t0)        

        lw      t8, PHY_CONTROL_REGS_STRAP_CONTROL(t0)         // Read Straps Control Register
        li      t3, PHY_CONTROL_REGS_STRAP_CONTROL_MHZ_MASK
        and     t8, t3
        srl     t8, PHY_CONTROL_REGS_STRAP_CONTROL_MHZ_STRT    // Get MHz value
        
        li      t1, 550                                        // if ddr_speed > 550
        bltu    t8, t1, pre_chrg_banks 
        nop

speed_gt_553:
/*      DDR2 will not work above 533MHz
        bnez    t6, ddr3_2                      // Jump if DDR3
        nop
ddr2_2:
        li      t2, (PHY_CONTROL_REGS_VDL_CALIBRATE_SET_WR_DQ_MASK | \
                     PHY_CONTROL_REGS_VDL_CALIBRATE_BIT_REFRESH_MASK)
        b       cont_2
        nop
ddr3_2:
*/
        li      t2, (PHY_CONTROL_REGS_VDL_CALIBRATE_BIT_CAL_MASK | \
                     PHY_CONTROL_REGS_VDL_CALIBRATE_SET_MR_MPR_MASK | \
                     PHY_CONTROL_REGS_VDL_CALIBRATE_SET_WR_DQ_MASK | \
                     PHY_CONTROL_REGS_VDL_CALIBRATE_BIT_REFRESH_MASK)
cont_2:
        /* Clear VDL Calibration Register */
        sw      t2, PHY_CONTROL_REGS_VDL_CALIBRATE(t0)  
        
        li      t2, 0x100                       // timeout_ns(1000)
1:      bne     t2, zero, 1b
        sub     t2, 1

        /* ------------------------------- */
        /* PHY VDL Bit Calibration Status. */

        li      t5, 1000                                 // Loop count value
        li      t3, 1
        li      t4, (PHY_CONTROL_REGS_VDL_CALIB_STATUS_IDLE_MASK)
        lw      t1, PHY_CONTROL_REGS_VDL_CALIB_STATUS(t0)       // Load VDL Status Register
        and     t1, t4
        bne     t1, zero, vdl_exit_4                            // VDL locked and Idle?
        nop
vdl_loop_4:
        beq     t5, zero, vdl_exit_4
        nop

        li      t2, 0x100                       // timeout_ns(100)
2:      bne     t2, zero, 2b
        sub     t2, 1

        lw      t1, PHY_CONTROL_REGS_VDL_CALIB_STATUS(t0)       // Load VDL Status Register
        and     t1, t4
        bne     t1, t4, vdl_loop_4                              // VDL locked and Idle?
        sub     t5, t3
vdl_exit_4:
                                                                // end (ddr_speed > 550)

        /* ----------------------------------------------------------------- */
        /* Precharge all banks of DRAM  and revert to the normal programming */
pre_chrg_banks:

        sw      t9, MC_CSR_DMODE_0(t0)          // Restore previous values to MC_CSR_DMODE_0
        li      t1, 0x302                       // issue Precharge All command
        sw      t1, MC_CSR_DCMD(t0)
        li      t1, 0x301                       // issue MRS  command to revert back to original
        sw      t1, MC_CSR_DCMD(t0)
        li      t1, 0x300                       // issue EMRS command to revert back to original
        sw      t1, MC_CSR_DCMD(t0)

#endif

read_data_dly:
        li      t1, 3                           // set the delay on read data valid (FIXME: Update based on layout)
        sw      t1, PHY_WORD_LANE_0_READ_DATA_DLY(t0)
        
/*----------------------------------------------------------------------------------------*/


/***** Enable Auto refresh ******************************/     
        li      t1, 0x0FC00
        sw      t1, MC_CSR_CLKS(t0)

/***** Enable ODT for writes and Enable Dynamic ODT ********/
        beqz    t6, 1f                         // if DDR2 - skip enabling Dynamic ODT
        li      t1, 0x304
        sw      t1, MC_CSR_ODT(t0)
1:
// If a3 is 0xff, then detect memory size.
// Otherwise, skip detection and use the given value as the memory size
// Find memory size. a3 keeps the size: 0=256Mb (not supported),  1=512Mb (not supported), 2=1Gb, 3=2Gb,  4=4Gb 

#if MEM_SIZE_DETECT
        li      a3, 0xff
		
       /***** choose Error level to get access to 2GB physical memory space *****/
        mfc0	t2, C0_SR
        ori		t2, M_SR_ERL
        mtc0	t2, C0_SR
       /***********************/
#else
        li      a3, 2   
#endif /*MEM_SIZE_DETECT */
        li      t1, 0xff
        bne     t1, a3, 1f           // If a3 != 0xff, then set flag t4=0 so we skip memory size detection
        li      t4, 0          
        
        li      t4, 1                // Reset flag to 1. Do memory size detection
        li      a3, 4                // Start from 4Gb 
        b       1f
        nop
	
ddr_size:
        addi    a3, -1
1:      
        LOADREL(t2, dram_map_table_x16)
  
        li      t1, 0x2c        // size of dram_map_table element
        mult    t1, a3
        mflo    t1              // dram_map_table offset
        add     t2, t1

        lw      t1, 0x00(t2)        // Row00_0
        sw      t1, MC_CSR_ROW00_0(t0)

        lw      t1, 0x04(t2)        // Row00_1
        sw      t1, MC_CSR_ROW00_1(t0)

        lw      t1, 0x08(t2)        // Row01_0
        sw      t1, MC_CSR_ROW01_0(t0)

        lw      t1, 0x0C(t2)        // Row01_1
        sw      t1, MC_CSR_ROW01_1(t0)

        lw      t1, 0x10(t2)        // Col00_0
        sw      t1, MC_CSR_COL00_0(t0)

        lw      t1, 0x14(t2)        // Col00_1
        sw      t1, MC_CSR_COL00_1(t0)

        lw      t1, 0x18(t2)        // Col01_0
        sw      t1, MC_CSR_COL01_0(t0)

        lw      t1, 0x1C(t2)        // Col01_1
        sw      t1, MC_CSR_COL01_1(t0)

        lw      t1, 0x20(t2)        // Bank
        sw      t1, MC_CSR_BNK10(t0)

        li      t1, 0x0             // CS_Start
        sw      t1, MC_CSR_CSST(t0)

        lw      t1, 0x24(t2)        // CS_End
        sw      t1, MC_CSR_CSEND(t0)

        li      t1, 0x0             // CS Interleaving CFG
        sw      t1, MC_CSR_CNFG(t0)

        lw      t3, 0x28(t2)        // Dram Size
        lw      t1, MC_CSR_GCFG(t0) // GCFG
        li      t2, 0xFFFFFF00
        and     t1, t1, t2          // Mask out Dram Size Fields
        or      t1, t3, t1          // insert new DRAM Size value
        li      t2, 0x80000000		// set GCFG_AliasDisabled bit to enable DDR of 512MB size(could be problem with more then 768Mb)
        or      t1, t2          
        sw      t1, MC_CSR_GCFG(t0)
		
        li      t1, 0               // minimum supported memory size is 256Mb , skip check aliasing
        beq     t1, a3, ddr_speed
        nop
		
        beqz    t4, ddr_speed       // no need check aliasing, static defined memory size
        nop

// Check for memory aliasing
// This assumes that the bank interleaving is below address bits 8.
        li      t1, 8
        li      t2, 24
        add     t2, a3          
        li      t3, 0x00000000
        li      t5, -1 
        sw      zero, 0(t3)           // Clear address 0
	
check_alias:
        li      t4, 1
        sll     t4, t1
        add     t4, t3
		
          /* skip perif */
        li      t6, 28
        blt     t1, t6, 1f
        nop
        add     t4, 0x10000000
        
  1:    sw      t5, 0(t4)             // Write the upper address location
        lw      t6, 0(t3)             // Read back address 0 to see whether we wrapped around


        beq     t5, t6, ddr_size
        nop

        bne     t1, t2, check_alias
        addi    t1, 1

 
ddr_speed:
#if MEM_SIZE_DETECT
        /***** clear Error level  ***************/
        mfc0	t2, C0_SR
        and		t2, ~M_SR_ERL
        mtc0	t2, C0_SR
		/********************/

#endif
			
        li      t1, 2
        blt     a3, t1, 1f     // If smaller than 1Gb, then program tFAW to 0
        li      t1, 0
        
        lw      t6, MC_CSR_DRAM_CFG(t0)
        andi    t6, MC_DRAM_CFG_DDR3
        bnez    t6, ddr3_tFAW               // Jump if DDR3
        nop
ddr2_tFAW:              
        LOADREL(t2, ddr2_timing_table)
  
        li      t1, 0x10    // size of ddrx_timing_table element
        mult    t1, a1
        mflo    t1          // ddrx_timing_table offset
        add     t2, t1
        lbu     t1, 0x07(t2)    // tFAW
        b       1f
        nop
ddr3_tFAW:              
//        LOADREL(t2, ddr3_timing_table)
  
//        li      t1, 0x10    // size of ddrx_timing_table element
//        mult    t1, a1
//        mflo    t1          // ddrx_timing_table offset
//        add     t2, t1
//        lbu     t1, 0x07(t2)    // tFAW
        lbu     t1, 0x07(v0)    // tFAW
1:      
        andi    t1, 0x3f
        sll     t1, 8
        li      t2, ~(0x3f << 8)
        lw      t3, MC_CSR_TIM1_1(t0)   // Load MC_CSR_TIM1_1 register
        and     t3, t2                  // Clear tFAW field
        or      t3, t1
        sw      t3, MC_CSR_TIM1_1(t0)   // Store MC_CSR_TIM1_1 register
        nop

tRefi_update:   
        LOADREL(t2, ddr_tRefi_table)

        beqz    t6, 1f                  // If DDR2, use a1 as is in t3
        move    t3, a1
        addi    t3, 2                   // Otherwise, use a1 + 2 in t3
1:
        LOADREL(t1, ddr3_333MHz)
        lw      t1, 0(t1)
        beqz    t1, 1f                 // If it is 333Mhz DDR3 use apropriate refresh rate
        nop
        li      t3, 1
1:
        li      t1, 0x4                 // size of ddr_tRefi_table entry
        mult    t1, t3
        mflo    t1                      // ddr_tRefi_table offset
        add     t2, t1

        lw      t1, 0x0(t2)             // tRefi
        sll     t1, 8
        sw      t1, MC_CSR_CLKS(t0)
    
        bnez    t6, 1f                  // If DDR3, load ddr3_tRFC_table
        nop

        LOADREL(t2, ddr2_tRFC_table)
        nop
        b       2f
        nop
1:                      
        LOADREL(t2, ddr3_tRFC_table)
        nop
2:                
        li      t1, 4           // size of ddrX_tRFC_table entry
        mult    t1, t3
        mflo    t1              // ddrX_tRefi_table offset
        add     t2, t1          // Calculate address in the row

        li      t1, 24          // size of ddrX_tRFC_table row
        mult    t1, a3
        mflo    t1              // ddrX_tRefi_table offset
        add     t2, t1          // Calculate address in the column

        lw      t3, 0x0(t2)     // Load tRFC value
        andi    t3, 0xFF        // Mask tRFC to 8-bits
        sll     t3, 16          // Move it to bit location [23:16]

        lw      t1, MC_CSR_TIM1_1(t0)  // Load MC_CSR_TIM1_1 register
        li      t2, 0xFF00FFFF
        and     t1, t1, t2      // Mask out tRFC Field
        or      t1, t3, t1      // insert new tRFC value
        sw      t1, MC_CSR_TIM1_1(t0)  // Write to TIM1_1 register

        SETLEDS1('D','I','N','T')
    

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
#define TEST_ADDR               0x80000
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

/***** Load MEMC Base *************************************/
        li      t0, MEMC_BASE
        li      t1, BACKGND_DATA0
        sw      t1, MC_CSR_TEST_DATA0_0(t0)  
        li      t1, BACKGND_DATA1
        sw      t1, MC_CSR_TEST_DATA0_1(t0)  
        li      t1, BACKGND_DATA2
        sw      t1, MC_CSR_TEST_DATA0_2(t0)  
        li      t1, BACKGND_DATA3
        sw      t1, MC_CSR_TEST_DATA0_3(t0)  
        li      t1, TEST_COUNT
        li      t2, VICT_COUNT
        or      t1, t2              # add victim count value
        sw      t1, MC_CSR_TEST_COUNT(t0)  
        li      t1, TEST_ADDR
        sw      t1, MC_CSR_TEST_ADDR(t0)   
        li      t1, TEST_ADDR_UPDT
        sw      t1, MC_CSR_TEST_ADDR_UPDT(t0)  
        li      t1, TEST_PATTERN
        sw      t1, MC_CSR_TEST_PAT(t0)
        
# Write a background pattern first
        li      t1, BACKGND_MODE_DATA
        li      t2, TEST_WRITE
        or      t1, t2
        li      t2, TEST_ENABLE
        or      t1, t2
        sw      t1, MC_CSR_TEST_CFG1(t0)

//        li      t2, 0x2
        li      t3, GPIO_BASE

        li      t5, 0x10000
2:      li      t1, 0x40
1:      addiu   t1, -1
        bnez    t1, 1b
        nop

        lw      t4, GPIO_MEMC_CONTROL(t3)
        andi    t4, GPIO_MEMC_CONTROL_MC_TEST_DONE_MASK
        bnez    t4, backgnd_write_done
        nop
        addiu   t5, -1
        bnez    t5, 2b
        nop

# Background write operation is finished

backgnd_write_done:
        li      t1, TEST_DATA0
        sw      t1, MC_CSR_TEST_DATA0_0(t0)  
        li      t1, TEST_DATA1
        sw      t1, MC_CSR_TEST_DATA0_1(t0)  
        li      t1, TEST_DATA2
        sw      t1, MC_CSR_TEST_DATA0_2(t0)  
        li      t1, TEST_DATA3
        sw      t1, MC_CSR_TEST_DATA0_3(t0)  
        li      t1, TEST_COUNT
        li      t2, VICT_COUNT
        or      t1, t2              # add victim count value
        sw      t1, MC_CSR_TEST_COUNT(t0)  
        li      t1, TEST_ADDR
        sw      t1, MC_CSR_TEST_ADDR(t0)   
        li      t1, TEST_ADDR_UPDT
        sw      t1, MC_CSR_TEST_ADDR_UPDT(t0)  
        li      t1, TEST_PATTERN
        sw      t1, MC_CSR_TEST_PAT(t0)

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
        sw      t1, MC_CSR_TEST_CFG1(t0)

//        li      t2, 0x2
        li      t3, GPIO_BASE

        li      t5, 0x10000
2:      li      t1, 0x40
1:      addiu   t1, -1
        bnez    t1, 1b
        nop

        lw      t4, GPIO_MEMC_CONTROL(t3)
        andi    t4, GPIO_MEMC_CONTROL_MC_TEST_DONE_MASK
        bnez    t4, test_write_done
        nop
        addiu   t5, -1
        bnez    t5, 2b
        nop

# Test write operation is finished

test_write_done:
        li      t1, TEST_DATA0
        sw      t1, MC_CSR_TEST_DATA0_0(t0)  
        li      t1, TEST_DATA1
        sw      t1, MC_CSR_TEST_DATA0_1(t0)  
        li      t1, TEST_DATA2
        sw      t1, MC_CSR_TEST_DATA0_2(t0)  
        li      t1, TEST_DATA3
        sw      t1, MC_CSR_TEST_DATA0_3(t0)  
        li      t1, TEST_COUNT
        li      t2, VICT_COUNT
        or      t1, t2              # add victim count value
        sw      t1, MC_CSR_TEST_COUNT(t0)  
        li      t1, TEST_ADDR
        sw      t1, MC_CSR_TEST_ADDR(t0)   
        li      t1, TEST_ADDR_UPDT
        sw      t1, MC_CSR_TEST_ADDR_UPDT(t0)  
        li      t1, TEST_PATTERN
        sw      t1, MC_CSR_TEST_PAT(t0)

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
        sw      t1, MC_CSR_TEST_CFG1(t0)

        li      t3, GPIO_BASE
//        li      t2, 0x2

        li      t5, 0x10000
2:      li      t1, 0x40
1:      addiu   t1, -1
        bnez    t1, 1b
        nop

        lw      t4, GPIO_MEMC_CONTROL(t3)
        andi    t4, GPIO_MEMC_CONTROL_MC_TEST_DONE_MASK
        bnez    t4, test_read_done
        nop
        addiu   t5, -1
        bnez    t5, 2b
        nop

# Test read operation is finished

test_read_done:
        lw      t1, MC_CSR_TEST_CFG1(t0)
        srl     t1, 2
        and     t1, 1
        SETLEDS1('T','S','T','1')
        bne     t1, zero, test_failed
        nop
        
/***** Load MEMC Base *************************************/
        li      t0, MEMC_BASE
        li      t1, BACKGND_DATA0
        sw      t1, MC_CSR_TEST_DATA0_0(t0)  
        li      t1, BACKGND_DATA1
        sw      t1, MC_CSR_TEST_DATA0_1(t0)  
        li      t1, BACKGND_DATA2
        sw      t1, MC_CSR_TEST_DATA0_2(t0)  
        li      t1, BACKGND_DATA3
        sw      t1, MC_CSR_TEST_DATA0_3(t0)  
        li      t1, TEST_COUNT
        li      t2, VICT_COUNT
        or      t1, t2              # add victim count value
        sw      t1, MC_CSR_TEST_COUNT(t0)  
        li      t1, TEST_ADDR
        sw      t1, MC_CSR_TEST_ADDR(t0)   
        li      t1, TEST_ADDR_UPDT
        sw      t1, MC_CSR_TEST_ADDR_UPDT(t0)  
        li      t1, TEST_PATTERN
        sw      t1, MC_CSR_TEST_PAT(t0)
        
# Write a background pattern first
        li      t1, BACKGND_MODE_DATA
        li      t2, TEST_WRITE
        or      t1, t2
        li      t2, TEST_ENABLE
        or      t1, t2
        sw      t1, MC_CSR_TEST_CFG1(t0)

//        li      t2, 0x2
        li      t3, GPIO_BASE

        li      t5, 0x10000
2:      li      t1, 0x40
1:      addiu   t1, -1
        bnez    t1, 1b
        nop

        lw      t4, GPIO_MEMC_CONTROL(t3)
        andi    t4, GPIO_MEMC_CONTROL_MC_TEST_DONE_MASK
        bnez    t4, backgnd_write_done1
        nop
        addiu   t5, -1
        bnez    t5, 2b
        nop

# Background write operation is finished

backgnd_write_done1:
        li      t1, TEST_DATA0
        sw      t1, MC_CSR_TEST_DATA0_0(t0)  
        li      t1, TEST_DATA1
        sw      t1, MC_CSR_TEST_DATA0_1(t0)  
        li      t1, TEST_DATA2
        sw      t1, MC_CSR_TEST_DATA0_2(t0)  
        li      t1, TEST_DATA3
        sw      t1, MC_CSR_TEST_DATA0_3(t0)  
        li      t1, TEST_COUNT
        li      t2, VICT_COUNT
        or      t1, t2              # add victim count value
        sw      t1, MC_CSR_TEST_COUNT(t0)  
        li      t1, TEST_ADDR
        sw      t1, MC_CSR_TEST_ADDR(t0)   
        li      t1, TEST_ADDR_UPDT
        sw      t1, MC_CSR_TEST_ADDR_UPDT(t0)  
        li      t1, TEST_PATTERN
        sw      t1, MC_CSR_TEST_PAT(t0)

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
        li      t2, TEST_WR_RD
        or      t1, t2
        li      t2, TEST_ENABLE
        or      t1, t2
        sw      t1, MC_CSR_TEST_CFG1(t0)

//        li      t2, 0x2
        li      t3, GPIO_BASE

        li      t5, 0x10000
2:      li      t1, 0x40
1:      addiu   t1, -1
        bnez    t1, 1b
        nop

        lw      t4, GPIO_MEMC_CONTROL(t3)
        andi    t4, GPIO_MEMC_CONTROL_MC_TEST_DONE_MASK
        bnez    t4, test_wr_rd_done
        nop
        addiu   t5, -1
        bnez    t5, 2b
        nop

# Test write_read operation is finished

test_wr_rd_done:
        lw      t1, MC_CSR_TEST_CFG1(t0)
        srl     t1, 2
        and     t1, 1
        SETLEDS1('T','S','T','2')
        bne     t1, zero, test_failed
        nop

test_passed:
        SETLEDS1('P','A','S','S')
        b       1f
        nop
test_failed:
        SETLEDS1('F','A','I','L')
        bnez	s3, retry_draminit
        sub	s3,1
stall_on_fail:
        b stall_on_fail
        nop
1:
        SETLEDS1('-','-','-', '-')
#endif

memc_init_end:
        li      t1, MC_CSR_GCFG_MEMINITDONE_MSK   
        lw      t2, MC_CSR_GCFG(t0)
        or      t2, t1                            // Set MemInitDone bit
        sw      t2, MC_CSR_GCFG(t0)               // Write to MC Global Config Register

memc_init_end_error:
        move    ra,s0
        j       ra
        nop
    
        .set    reorder

END(board_draminit)


/* ********************************************************************************
   Name:        ddr3_look_up_v2(a0) 
   Description: return the configuration for memc for a perticular DDR3

   Input:       a2 =    letter grade (bit 16-17),
                        speed grade (bit 9-8),
                        exact speed (bit 1-0)

   Output:      v1 = pointer to 16 Byte configuration field for speed grade and exact speed dependant timing values
                   = 0xFFFF_FFFF                if speed grade is not supported.
                                                OR if exact speed > speed grade
                v0 = pointer to 16 Byte DDR3 configuration field for speed dependant timing values

   ******************************************************************************** */
SLEAF(ddr3_look_up_v2)
        .set    noreorder

        /* get letter grade */
        li      t2, DDR3_LETTER_GRADE
        and     t5, a2, t2                      
        srl     t5, 16                          # t5 contain letter grade

        /* get speed grade */
        li      t2, DDR3_SPEED_GRADE
        and     t3, a2, t2                      
        srl     t3, 8                           # t3 contain speed grade

        /* get exact speed */
        li      t2, DDR3_EXACT_SPEED
        and     t1, a2, t2                      # t1 contain exact speed

        /* CHECK for valid exact speed, must be less/equal to speed grade */
        bgt     t1, t3, error
        nop
        
        li      t2, 4*4*4                       # compute index using letter grade
        mult    t5, t2
        mflo    t5

        li      t2, 4*4                         # compute index using speed grade
        mult    t3, t2
        mflo    t4

        add     t5, t4                          # combine letter grade & speed grade indices into t5
        
        li      t2, 4
        mult    t1, t2
        mflo    t4
        add     t5, t4
        
        move    s1, ra
        LOADREL (t2, ddr3_new_config_table)
        add     v1, t2, t5
        
        LOADREL(t3, ddr3_timing_table)
  
        li      t2, 16      // size of ddr3_timing_table element
        mult    t2, t1
        mflo    t2          // ddr3_timing_table offset
        add     v0, t2, t3

        move    ra, s1
        jr      ra
        nop
        
error:  li      v0, 0xffffffff
        li      v1, 0xffffffff                  # found a non-supported speed grade
        jr      ra
        nop
        
        .set    reorder
SEND(ddr3_look_up_v2)

#if DDRDEBUG
/*  *********************************************************************
    *  BOARD_SETLEDS(x)
    *
    *  Set LEDs for boot-time progress indication.  Not used if
    *  the board does not have progress LEDs.  This routine
    *  must not call any other routines, since it may be invoked
    *  either from KSEG0 or KSEG1 and it may be invoked
    *  whether or not the icache is operational.
    *
    *  Input parameters:
    *       a0 - LED value (8 bits per character, 4 characters)
    *
    *  Return value:
    *       nothing
    *
    *  Registers used:
    *       t7,t8,t9
    ********************************************************************* */
SLEAF(board_setleds)
        li      t7, UART_BASE
        li      t8, TXFIFOEMT

1:      lh      t9, UART0INTSTAT(t7)
        and     t9, t8
        bne     t9, t8, 1b

        srl     t8, a0, 24
        sb      t8, UART0DATA(t7)
        srl     t8, a0, 16
        sb      t8, UART0DATA(t7)
        srl     t8, a0, 8
        sb      t8, UART0DATA(t7)
        sb      a0, UART0DATA(t7)
        li      a0, '\r'
        sb      a0, UART0DATA(t7)
        li      a0, '\n'
        sb      a0, UART0DATA(t7)
        j       ra
        nop
SEND(board_setleds)
#endif

