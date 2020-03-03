/*  *********************************************************************
    *
    <:copyright-BRCM:2012:proprietary:standard
    
       Copyright (c) 2012 Broadcom 
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

#ifndef MEMSYS_TOP_API_H__
#define MEMSYS_TOP_API_H__

/* 
 * Note the following data types need to be specified ahead of this header.
 *    uint32_t   - 32-bit unsigned integer type
 *    physaddr_t - physical address type, carries "phys_" prefix
 *                 Note currently only lower 32-bits are used.
 *
 * For 32-bit Memsys Library, physaddr_t should be declared as 32-bit unsigned
 * integer type. 
 */

typedef uint32_t  physaddr_t;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Memsys Library Version
 *
 * This is a 32-bit word to identify Memsys Library version. Fields are:
 *
 * Bits [31:24] MEMSYS_FW_VERSION_MAJOR : Major version number
 *                                        (incompatible changes)
 * Bits [23:16] MEMSYS_FW_VERSION_MINOR : Minor version number
 *                                        (backwards-compatible new features)
 * Bits [15:08] MEMSYS_FW_VERSION_PATCH : Patch number
 *                                        (backwards-compatible bug fixes)
 * Bits [07:00] MEMSYS_FW_VERSION_ENG   : Engineering drop number
 *                                        (internal debug, 0=non-engineering)
 *
 * The Memsys Library Version can be used to track API changes.
 * There will be more version changes than actual API version changes.
 * Please read the release notes to see which version changed API, and whether
 * new MCBs are required or not.
 *
 * Minor version number tracks changes that are compatible with earlier
 * versions. For instance, a previously unused option bit now has meaning,
 * or an additional error code is added. The patch number is reset to 0 when
 * minor version changes. If minor version changes, but not major, both of
 * the following conditions are met:
 *   1. Existing user code can use either the old or the new API, without
 *      breaking any existing functionalities. This means code written for
 *      v1.0.0.0 will continue to work the same way with v1.3.0.0 library.
 *   2. New user code that utilizes the new API will also work using the old
 *      API, except new functionalities will not be available. This means code
 *      written for v1.3.0.0 will still work with v1.0.0.0 library,
 *      however features introduced after v1.0.0.0 will not be available.
 * 
 * Major version number tracks changes that are incompatible with earlier
 * versions. For example, the meaning of an existing option bit is changed,
 * or adding a new field to a struct. Both minor and patch numbers set to 0
 * when major version changes.
 *
 * Note major version may change without any changes to the API, such as when
 * MCB become incompatible, and user need to get new MCBs. Incompatible API
 * change will always trigger the library's major version number to change,
 * and whenever there is compatible API change, the library's minor version
 * number will also change.
 *
 * Patch number tracks backward compatible bug fixes.
 *
 * A non-zero engineering drop number means the library is for internal
 * debug and testing purposes only.
 */
#define MEMSYS_FW_VERSION_MAJOR_SHIFT       24
#define MEMSYS_FW_VERSION_MINOR_SHIFT       16
#define MEMSYS_FW_VERSION_PATCH_SHIFT       8
#define MEMSYS_FW_VERSION_ENG_SHIFT         0
    
#define MEMSYS_FW_VERSION_MAJOR             1
#define MEMSYS_FW_VERSION_MINOR             2
#define MEMSYS_FW_VERSION_PATCH             0
#define MEMSYS_FW_VERSION_ENG               2
    
#define MEMSYS_FW_VERSION                                         \
    ((MEMSYS_FW_VERSION_MAJOR << MEMSYS_FW_VERSION_MAJOR_SHIFT) | \
     (MEMSYS_FW_VERSION_MINOR << MEMSYS_FW_VERSION_MINOR_SHIFT) | \
     (MEMSYS_FW_VERSION_PATCH << MEMSYS_FW_VERSION_PATCH_SHIFT) | \
     (MEMSYS_FW_VERSION_ENG   << MEMSYS_FW_VERSION_ENG_SHIFT))

/*
 * Helper for Memsys Library users to perform API feature checks. e.g.,
 * if API feature Z is only available on version >= major.minor.patch.eng
 *
 * #if (MEMSYS_VERSION_IN_API_FILE_IS_THIS_OR_MORE(major, minor, patch, eng))
 *  enable_API_feature_Z
 * #endif
 */
#define MEMSYS_VERSION_IN_API_FILE_IS_THIS_OR_MORE(major, minor, patch, eng) \
    (MEMSYS_FW_VERSION >=                                               \
     (((major) << MEMSYS_FW_VERSION_MAJOR_SHIFT) |                      \
      ((minor) << MEMSYS_FW_VERSION_MINOR_SHIFT) |                      \
      ((patch) << MEMSYS_FW_VERSION_PATCH_SHIFT) |                      \
      ((eng) << MEMSYS_FW_VERSION_ENG_SHIFT)))
     
/*
 * memsys_version_t
 *   Memsys Library version data structure
 *
 *   version - Memsys Library version, same as MEMSYS_FW_VERSION.
 *   build_ver - A string showing internal build information such as source
 *       code revision. Note this is a pointer to a global address. This
 *       pointer may be NULL.
 *   hw_ver_str - A string showing the HW combo (PHY + MEMC) this library
 *       supports, for example, "hpf0_b1p3". Note this is a pointer to a
 *       global address. This pointer may be NULL.
 *   ver_str - Version string. This is a human readable string of the library
 *       version, e.g. "1.0.0.0". Note this is a pointer to a global address.
 *       This pointer may be NULL to prevent external output of version info.
 */
typedef struct memsys_version
{
    uint32_t version;           // MEMSYS_FW_VERSION
    const char * build_ver;     // Internal build version string
    const char * hw_ver_str;    // HW version this library supports (string)
    const char * ver_str;       // library version string
} memsys_version_t;
     
/*
 * Memsys EDIS interrupt enable bits
 *   bit  [0]   1 = Enable the EDIS DONE bit interrupt
 *   bit  [1]   1 = Enable the EDIS STALLED bit interrupt
 *   bit  [2]   1 = Enable the EDIS READ_ERROR bit interrupt
 */
typedef enum memsys_edis_int_enable
{
    MEMSYS_EDIS_INT_ENABLE_DONE             = (1 << 0),    // 0x00000001
    MEMSYS_EDIS_INT_ENABLE_STALLED          = (1 << 1),    // 0x00000002
    MEMSYS_EDIS_INT_ENABLE_READ_ERROR       = (1 << 2),    // 0x00000004
} memsys_edis_int_enable_e;

/*
 * memsys_system_callbacks_t
 *   Helper functions provided by the system (i.e., boot loader)
 *
 * delay_us - This function provides a blocking delay in units of microseconds.
 *     This function is required for Memsys Library to work properly. The
 *     delay should be accurate, as the delay impact timing that could be
 *     checked for standards conformance. Memsys Library will never delay
 *     above the uint32_t limit. This function should return 0 on success and
 *     non-zero on failure.
 *
 * putchar - Output a single character to console. This will be used to display
 *     debug information (shmoo output) and should be implemented. The output
 *     should have as little buffering delay as possible. This function may be
 *     NULL to prevent any output. This function should return 0 on success
 *     and non-zero on failure.
 *
 * getchar - Get a single character from console. If blocking_on_non_zero
 *     parameter is set to a non-zero value, this function is blocking and
 *     should wait forever for user input, a return value of 0 or less will be
 *     interpreted as error. If blocking_on_non_zero is set to zero, this
 *     function is non-blocking, and it will return user input if it exist, or
 *     zero if no input. This is an optional debug function and can be NULL.
 *
 * get_time_us - Get global clock count in microsecond units. The wrap around
 *     is the full 32 bits. This is for debug usage and can be NULL.
 */
typedef struct memsys_system_callbacks
{
    int      (*delay_us)(uint32_t us);      // required
    int      (*putchar)(char c);            // required for debug
    char     (*getchar)(int blocking_on_non_zero); // optional
    uint32_t (*get_time_us)(void);          // optional
} memsys_system_callbacks_t;
     
/*
 * Memsys run-time option flags.
 *   bit  [0]   1 = Normal console print disabled
 *   bit  [1]   Reserved: set to 0
 *   bit  [2]   1 = Warm boot, 0 = Cold boot
 *   bit  [3]   1 = PHY selection for lpe0e2_custom, PHY is LP E0 instead of LP E2
 *   bit  [4]   1 = Skip shmoo
 *   bit  [5]   1 = Skip program rts
 *   bit  [6]   1 = Skip MEMC init
 *   bit  [7]   1 = Skip PHY init
 *   bit  [8]   1 = Skip PHY DRAM init
 *   bit  [9]   1 = Skip PHY PLL init
 *   bit [10]   1 = Save PHY state at end of memsys_top
 *   bit [11]   1 = Prepare PHY for standby entry
 *   bit [12]   1 = PHY placed in low power mode when in standby
 */
typedef enum memsys_top_options
{
    MEMSYS_OPTION_CONSOLE_OUTPUT_DISABLED   = (1 << 0),    // 0x00000001
    MEMSYS_OPTION_WARM_BOOT                 = (1 << 2),    // 0x00000004
    MEMSYS_OPTION_PHY_IS_LP_E0              = (1 << 3),    // 0x00000008
    MEMSYS_OPTION_SKIP_SHMOO                = (1 << 4),    // 0x00000010
    MEMSYS_OPTION_SKIP_PROGRAM_RTS          = (1 << 5),    // 0x00000020
    MEMSYS_OPTION_SKIP_MEMC_INIT            = (1 << 6),    // 0x00000040
    MEMSYS_OPTION_SKIP_PHY_INIT             = (1 << 7),    // 0x00000080
    MEMSYS_OPTION_SKIP_PHY_DRAM_INIT        = (1 << 8),    // 0x00000100
    MEMSYS_OPTION_SKIP_PHY_PLL_INIT         = (1 << 9),    // 0x00000200
    MEMSYS_OPTION_SAVE_PHY_STATE            = (1 << 10),   // 0x00000400
    MEMSYS_OPTION_PREP_PHY_FOR_STANDBY      = (1 << 11),   // 0x00000800
    MEMSYS_OPTION_PHY_LOW_POWER_AT_STANDBY  = (1 << 12),   // 0x00001000
} memsys_top_options_e;
     
/* Number of bits to shift EDIS HW block register offset into edis_info */
#define EDIS_HW_SPACING_SHIFT               4

/* Size in number of bytes needed to save PHY state */
#define MEMSYS_SAVE_STATE_SIZE_BYTES        320

/*
 * Memsys Library Error Codes and Error Data Structure
 */
/*
 * List of memc error codes
 */
typedef enum memsys_error_memc
{
    MEMSYS_ERROR_MEMC_NONE              = 0,
    MEMSYS_ERROR_MEMC_POWER_UP_TIMEOUT,
    MEMSYS_ERROR_MEMC_MAX,
} memsys_error_memc_e;

/*
 * List of phy error codes
 */
typedef enum memsys_error_phy
{
    MEMSYS_ERROR_PHY_NONE = 0,
    MEMSYS_ERROR_PHY_VDL_CALIB_NOLOCK,
    MEMSYS_ERROR_PHY_PLL_NOLOCK,
    MEMSYS_ERROR_PHY_MEMC_NODFIACK,
    MEMSYS_ERROR_PHY_MEMC_NODFIRELEASE,
    MEMSYS_ERROR_PHY_MAX
} memsys_error_phy_e;

/*
 * List of shmoo/memsys error codes
 */
typedef enum memsys_error_shmoo
{
    MEMSYS_ERROR_SHMOO_NONE    =    0,
    MEMSYS_ERROR_OVERALL_MCB_INVALID,                       // word 0: 0x00000002
    MEMSYS_ERROR_OVERALL_MCB_BAD_CHECKSUM,                  // word 0: 0x00000004
    MEMSYS_ERROR_OVERALL_PHY_MCB_INVALID,                   // word 0: 0x00000008
    MEMSYS_ERROR_OVERALL_PHY_MCB_BAD_VERSION,               // word 0: 0x00000010
    MEMSYS_ERROR_OVERALL_MEMC_MCB_INVALID,                  // word 0: 0x00000020
    MEMSYS_ERROR_OVERALL_MEMC_BAD_REG_BASE,                 // word 0: 0x00000040
    MEMSYS_ERROR_OVERALL_MEMC_MCB_BAD_VERSION,              // word 0: 0x00000080
    MEMSYS_ERROR_PSSR_BIG_BIT_LENGTH,                       // word 0: 0x00000100
    MEMSYS_ERROR_PSSR_NULL_PTR,                             // word 0: 0x00000200
    MEMSYS_ERROR_PSSR_BAD_ALT_ID,                           // word 0: 0x00000400
    MEMSYS_ERROR_PSSR_CHECKSUM,                             // word 0: 0x00000800
    MEMSYS_ERROR_PSSR_GLOBAL_COUNTER,                       // word 0: 0x00001000
    MEMSYS_ERROR_PSSR_UNEXPECTED_ALT_ENTRY,                 // word 0: 0x00002000
    MEMSYS_ERROR_PSSR_UNEXPECTED_FUNC_INDEX,                // word 0: 0x00004000
    MEMSYS_ERROR_PSSR_MEMC_TIMEOUT_ON_WARM_BOOT,            // word 0: 0x00008000
    MEMSYS_ERROR_SHMOO_ZQCAL_NO_P_LOW,                      // word 0: 0x00010000
    MEMSYS_ERROR_SHMOO_ZQCAL_NO_P_HIGH,                     // word 0: 0x00020000
    MEMSYS_ERROR_SHMOO_ZQCAL_NO_N_LOW,                      // word 0: 0x00040000
    MEMSYS_ERROR_SHMOO_ZQCAL_NO_N_HIGH,                     // word 0: 0x00080000
    MEMSYS_ERROR_SHMOO_ZQCAL_NO_SOLUTION,                   // word 0: 0x00100000         
    MEMSYS_ERROR_SHMOO_WR_LEVEL_TO_1,                       // word 0: 0x00200000        
    MEMSYS_ERROR_SHMOO_WR_LEVEL_NOT_CONVERGED,              // word 0: 0x00400000
    MEMSYS_ERROR_SHMOO_READ_ENABLE_FAILED,                  // word 0: 0x00800000
    MEMSYS_ERROR_SHMOO_READ_ENABLE_MEM_TEST_FAILED,         // word 0: 0x01000000
    MEMSYS_ERROR_SHMOO_READ_ENABLE_PEGGED_VDL,              // word 0: 0x02000000   
    MEMSYS_ERROR_SHMOO_RD_DQS_MEM_TEST_FAILED,              // word 0: 0x04000000     
    MEMSYS_ERROR_SHMOO_RD_DQS_NO_BYTE_PASS,                 // word 0: 0x08000000
    MEMSYS_ERROR_SHMOO_READ_DQS_PEGGED_VDL,                 // word 0: 0x10000000       
    MEMSYS_ERROR_SHMOO_WR_DQS_DQ_0,                         // word 0: 0x20000000       
    MEMSYS_ERROR_SHMOO_WR_DQS_DQ_1,                         // word 0: 0x40000000       
    MEMSYS_ERROR_SHMOO_WR_DQS_DQ_2,                         // word 0: 0x80000000       
    MEMSYS_ERROR_SHMOO_WR_DQS_MEM_TEST_FAILED,                  // word 1: 0x00000001    
    MEMSYS_ERROR_SHMOO_WR_DQS_PEGGED_VDL,                       // word 1: 0x00000002      
    MEMSYS_ERROR_SHMOO_WR_DM_TO_0,                              // word 1: 0x00000004      
    MEMSYS_ERROR_SHMOO_WR_DM_TO_1,                              // word 1: 0x00000008      
    MEMSYS_ERROR_SHMOO_WR_DM_TO_2,                              // word 1: 0x00000010
    MEMSYS_ERROR_SHMOO_WR_DM_TO_3,                              // word 1: 0x00000020       
    MEMSYS_ERROR_SHMOO_WR_DM_MEM_TEST_FAILED,                   // word 1: 0x00000040       
    MEMSYS_ERROR_SHMOO_WR_DM_PEGGED_VDL,                        // word 1: 0x00000080       
    MEMSYS_ERROR_SHMOO_PRINT_REPORT,                            // word 1: 0x00000100       
    MEMSYS_ERROR_SHMOO_FILTER,                                  // word 1: 0x00000200      
    MEMSYS_ERROR_SHMOO_ADDX_SHMOO_PEGGED_VDL,                   // word 1: 0x00000400     
    MEMSYS_ERROR_SHMOO_ADDX_SHMOO_MEM_TEST_FAILED,              // word 1: 0x00000800
    MEMSYS_ERROR_SHMOO_CA_TRAINING_FAILED,                      // word 1: 0x00001000
    MEMSYS_ERROR_SHMOO_GDDR5_ADDX_ALIGN_VDL_NOT_AT_CALIB,       // word 1: 0x00002000
    MEMSYS_ERROR_SHMOO_GDDR5_ADDX_ALIGN_PEGGED_VDL,             // word 1: 0x00004000  
    MEMSYS_ERROR_SHMOO_GDDR5_VENDOR_ID_ERROR,                   // word 1: 0x00008000 
    MEMSYS_ERROR_SHMOO_GDDR5_WCK2CK_ALIGN_SAMPLE_TO,            // word 1: 0x00010000 
    MEMSYS_ERROR_SHMOO_GDDR5_ADDX_TRAIN_ERROR,                  // word 1: 0x00020000   
    MEMSYS_ERROR_SHMOO_GDDR5_WCK2CK_INVALID_INITIAL_STATE,      // word 1: 0x00040000  
    MEMSYS_ERROR_SHMOO_GDDR5_WCK2CK_ALIGN_FAIL_CONVERGE,        // word 1: 0x00080000  
    MEMSYS_ERROR_SHMOO_GDDR5_WCK2CK_ALIGN_PEGGED_VDL,           // word 1: 0x00100000
    MEMSYS_ERROR_SHMOO_GDDR5_WCK2CK_FAILED_TO_DETECT_POLARITY,  // word 1: 0x00200000
    MEMSYS_ERROR_SHMOO_GDDR5_RD_EN_SHMOO_NO_LEFT_WINDOW,        // word 1: 0x00400000 
    MEMSYS_ERROR_SHMOO_GDDR5_RD_EN_SHMOO_NO_RIGHT_WINDOW,       // word 1: 0x00800000  
    MEMSYS_ERROR_SHMOO_GDDR5_READ_ENABLE_SHMOO_PEGGED_VDL,      // word 1: 0x01000000 
    MEMSYS_ERROR_SHMOO_GDDR5_INVALID_MRE_OPTIONS,               // word 1: 0x02000000    
    MEMSYS_ERROR_SHMOO_HP_RX_TRIM_TO_ERROR,                     // word 1: 0x04000000           
    MEMSYS_ERROR_SHMOO_HP_RX_TRIM_NO_RESULT,                    // word 1: 0x08000000          
    MEMSYS_ERROR_SHMOO_HP_RX_TRIM_INVALID_DDR_TYPE,             // word 1: 0x10000000
    MEMSYS_ERROR_SHMOO_DDR4_ADDX_ALIGN_PEGGED_VDL,              // word 1: 0x20000000
    MEMSYS_ERROR_OVERALL_INCOMPATIBLE_FW_VERSION,               // word 1: 0x40000000 
    MEMSYS_ERROR_OVERALL_PARAM_INVALID,                         // word 1: 0x80000000
    MEMSYS_ERROR_SHMOO_VTT_NOT_READY,                       // word 2: 0x00000001    
    MEMSYS_ERROR_FLOW_CONTROL_REGISTER_CALLBACKS,           // word 2: 0x00000002    
    MEMSYS_ERROR_FLOW_CONTROL_MEMSYS_BEGIN,                 // word 2: 0x00000004    
    MEMSYS_ERROR_FLOW_CONTROL_PRE_MEMC_INIT,                // word 2: 0x00000008    
    MEMSYS_ERROR_FLOW_CONTROL_PRE_SHMOO,                    // word 2: 0x00000010 
    MEMSYS_ERROR_FLOW_CONTROL_MEMSYS_END,                   // word 2: 0x00000020 
    MEMSYS_ERROR_SHMOO_DDR4_CS_SHMOO_PEGGED_VDL,            // word 2: 0x00000040 
    MEMSYS_ERROR_SHMOO_RD_DATA_DLY_WRONG_MODE,              // word 2: 0x00000080 
    MEMSYS_ERROR_SHMOO_RD_DATA_DLY_ERROR,                   // word 2: 0x00000100 
    MEMSYS_ERROR_OVERALL_INCOMPATIBLE_PHY_HW,               // word 2: 0x00000200 
    MEMSYS_ERROR_OVERALL_INCOMPATIBLE_MEMC_HW,              // word 2: 0x00000400 
    MEMSYS_ERROR_OVERALL_MCB_INVALID_DDR_TYPE,              // word 2: 0x00000800 
    MEMSYS_ERROR_OVERALL_UNSUPPORTED_DDR_WIDTH,             // word 2: 0x00001000 
    MEMSYS_ERROR_SHMOO_INVALID_CLK_SHIFT,                   // word 2: 0x00002000 
    MEMSYS_ERROR_SHMOO_MAX,
} memsys_error_shmoo_e;

/* Number of bits per word */
#define MEMSYS_ERROR_BITS_PER_WORD   (sizeof(unsigned int) * 8)

/* Maximum number of error words needed */
#define MEMSYS_ERROR_MEMC_MAX_WORDS       \
    ((MEMSYS_ERROR_MEMC_MAX / MEMSYS_ERROR_BITS_PER_WORD) + 1)
#define MEMSYS_ERROR_PHY_MAX_WORDS       \
    ((MEMSYS_ERROR_PHY_MAX / MEMSYS_ERROR_BITS_PER_WORD) + 1)
#define MEMSYS_ERROR_SHMOO_MAX_WORDS       \
    ((MEMSYS_ERROR_SHMOO_MAX / MEMSYS_ERROR_BITS_PER_WORD) + 1)

/*
 * Memsys error data structure.
 *   memc  - an array of memc error words, each bit reflects one error code
 *   phy   - an array of phy error words, each bit reflects one error code
 *   shmoo - an array of shmoo error words, each bit reflects one error code
 */
typedef struct memsys_error
{
    unsigned int memc [ MEMSYS_ERROR_MEMC_MAX_WORDS ]; 
    unsigned int phy  [ MEMSYS_ERROR_PHY_MAX_WORDS ]; 
    unsigned int shmoo[ MEMSYS_ERROR_SHMOO_MAX_WORDS ]; 
} memsys_error_t;

/*
 * memsys_top_params_t
 *   Memsys Library top level function parameters
 *
 *   version - Memsys Library version, user should pass in MEMSYS_FW_VERSION.
 *   options - Runtime options and flags. This is a bit field.
 *       See memsys_top_options_e. All other bits not defined are reserved and
 *       should be set to 0. In general, the value 1 does what the option says,
 *       and the value 0 does the opposite of the enum. 
 *   edis_info - EDIS HW block information.
 *       bits[03:0] Number of EDIS HW blocks per PHY (cannot be 0).
 *       bits[31:4] Register offset between EDIS HW blocks (0 if only one EDIS).
 *                  Can use EDIS_HW_SPACING_SHIFT to shift in the offset.
 *   mem_test_size_bytes - Memory test size in bytes for Shmoo. Test size
 *       starting at physical memory space as specified by phys_mem_test_base
 *       will be thrashed. The larger the region, the longer Shmoo will run.
 *       Test size should be aligned on 32 bytes boundary. This parameter can
 *       be set to 0 if not running Shmoo.
 *   phys_mem_test_base - Memory test base address for Shmoo. This is a
 *       physical address in the global memory address space. This parameter
 *       can be set to 0 if not running Shmoo.
 *   phys_memc_reg_base - MEMC register base. This is a physical address.
 *       Memsys Library will directly access this address. This parameter
 *       cannot be 0.
 *   phys_phy_reg_base - PHY register base. This is a physical address.
 *       Memsys Library will directly access this address. This parameter
 *       cannot be 0.
 *   phys_shim_reg_base - SHIMPHY register base. This is a physical address.
 *       Memsys Library will directly access this address. This parameter can
 *       be set to 0 if not using Andover MEMC.
 *   phys_edis_reg_base - EDIS register base. This is the physical address of
 *       the first EDIS. Memsys Library will directly access this address.
 *       This parameter cannot be 0.
 *   mcb_addr - Physical or virtual address pointing to MCB. Memsys Library
 *       will directly access this address. This parameter cannot be 0. MCB
 *       is read only data. This should be aligned on 32-bit boundary. 
 *   saved_state_base - This is a pointer to a physical or virtual address,
 *       where PHY state will be saved. Memsys Library will access this
 *       memory space 32-bits at a time. This should be aligned on 32-bit
 *       boundary. This parameter should be valid if S3 functionality is
 *       required. The size of saved state in bytes is specified by
 *       MEMSYS_SAVE_STATE_SIZE_BYTES.
 *   callbacks - Pointer to system provided helper functions data structure.
 *   error - Memsys error data structure. Memsys Library will fill in this
 *       data structure. 
 */
typedef struct memsys_top_params 
{
    uint32_t version;
    uint32_t options;
    uint32_t edis_info;
    uint32_t mem_test_size_bytes;
    physaddr_t phys_mem_test_base;
    physaddr_t phys_memc_reg_base;
    physaddr_t phys_phy_reg_base;
    physaddr_t phys_shim_reg_base;
    physaddr_t phys_edis_reg_base;
    const uint32_t * mcb_addr;
    uint32_t * saved_state_base;
    memsys_system_callbacks_t * callbacks;
    memsys_error_t error;
} memsys_top_params_t;

/* 
 * Generic callback function pointer supplied by user to Memsys Library
 *
 * Params:
 *   memsys_top_params_t * params [IN] -
 *       Pointer to Memsys Library top level function parameters structure. 
 *   void * args [I/O] -
 *       Pointer to data structure used by the function.
 *
 * Returns:
 *   0 - No error.
 *   1 - Error has occured.
 */                      
typedef int (*memsys_user_callback_func)(memsys_top_params_t * params, void * args);
 
/*
 * memsys_flow_control_user_callbacks_t    
 *   User helper functions that are used in Memsys Library flow control
 *
 * All functions shall return 0 on success and 1 on failure unless otherwise
 * noted below.
 *
 * Memsys Library will call memsys_register_flow_control_user_callbacks() to
 * register this data structure. This function shall be provided by the user
 * that supplies these flow control callback functions. The prototype is:
 *     extern int memsys_register_flow_control_user_callbacks(
 *              memsys_flow_control_user_callbacks_t * memsys_flow_cb )
 *
 * fp_memsys_begin - This function is called by Memsys Library after top level
 *     entry, prior to any other flow control stages. This function is optional
 *     and can be set to NULL.
 *
 * fp_pre_memc_init - This function is called by Memsys Library prior to
 *     calling MEMC init. This function is optional and can be set to NULL.
 *
 * fp_pre_shmoo - This function is called by Memsys Library prior to calling
 *     Shmoo. This function is optional and can be set to NULL.
 *
 * fp_memsys_end - This function is called by Memsys Library after all other
 *     flow control stages and prior to the exit of Memsys top level function.
 *     This function is called when MEMSYS HW block has been initialized
 *     correctly without errors. This function is optional and can be set to NULL.
 *
 * fp_disable_dram_refresh - This function is called by Memsys Library during
 *     Shmoo to disable issuing refresh command to DRAM. This is a required
 *     function if not using Andover MEMC, otherwise can be set to NULL. 
 *
 * fp_enable_dram_refresh - This function is called by Memsys Library during
 *     Shmoo to enable issuing refresh command to DRAM. This is a required
 *     function if not using Andover MEMC, otherwise can be set to NULL. 
 *
 * fp_edis_int_enable - This function is called by Memsys Library during Shmoo
 *     It will set the appropriate EDIS L1 (or L2 if applicable) interrupt
 *     status such as DONE, STALLED, and READ_ERROR. This function is required
 *     if Memsys Library cannot poll EDIS status register directly (63138,
 *     63148, 68620) due to design limitations. Otherwise set to NULL.
 *     The arguments (void * args) for this function is as follows:
 *         int enable - a bit field, see memsys_edis_int_enable_e, where the
 *                      value 1 means allow interrupts to be generated and
 *                      the value 0 means do not generate interrupt.
 *
 * fp_edis_int_ready - This function is called by Memsys Library during Shmoo.
 *     It should return 1 if L1 interrupt status is set due to EDIS internal
 *     status. Return 0 otherwise. This function is required if Memsys Library
 *     cannot poll EDIS status register directly (63138, 63148, 68620) due to
 *     design limitations. Otherwise set to NULL.
 *
 * fp_edis_int_clear - This function is called by Memsys Library during Shmoo
 *     It should clear all sources of EDIS interrupt status in L2 (and L1 if
 *     applicable). This function is required if Memsys Library cannot poll
 *     EDIS status register directly (63138, 63148, 68620) due to design
 *     limitations. Otherwise set to NULL.
 */
typedef struct memsys_flow_control_user_callbacks
{                              
    memsys_user_callback_func  fp_memsys_begin;
    memsys_user_callback_func  fp_pre_memc_init;
    memsys_user_callback_func  fp_pre_shmoo;
    memsys_user_callback_func  fp_memsys_end;
    
    // the following two functions are required if using non-Andover MEMC
    memsys_user_callback_func  fp_disable_dram_refresh;
    memsys_user_callback_func  fp_enable_dram_refresh;
    
    // the following functions are required if EDIS interrupts are used
    memsys_user_callback_func  fp_edis_int_enable;
    memsys_user_callback_func  fp_edis_int_ready;
    memsys_user_callback_func  fp_edis_int_clear;
    
} memsys_flow_control_user_callbacks_t;

/************************************************************************
 * Function: memsys_get_version
 *   Get Memsys Library version information.
 *
 * Params:
 *   memsys_version_t *version [I/O] -
 *       Pointer to Memsys Library version data structure, allocated by caller.
 *
 * Returns:
 *   0 - No error, data structure is filled.
 *   1 - Error, data inside the structure cannot be used.
 *
 * Notes:
 *   This function may set the strings to NULL to prevent caller from
 *   outputting the version string.
 ************************************************************************/
int memsys_get_version(memsys_version_t * version);

/************************************************************************
 * Function: memsys_top
 *   Initialize MEMSYS HW block.
 *
 * Params:
 *   memsys_top_params_t *params [I/O] -
 *       Pointer to Memsys Library top level parameters data structure.
 *
 * Returns:
 *   0 - No error.
 *   1 - Error has occured, the memsys_error_t structure inside the params
 *       will be filled in.
 *
 * Notes:
 *   If this function returns non-zero number, error has occurred, and the
 *   caller should print all words in the error structure to console output.
 *   Printing of error codes is required even when the Memsys Library is the
 *   "no print" version.
 *   If MEMSYS_OPTION_WARM_BOOT option is used, PHY state will be recovered
 *   from saved_state_base.
 *   If MEMSYS_OPTION_SAVE_PHY_STATE option is used, PHY state will be saved
 *   to saved_state_base.
 ************************************************************************/
int memsys_top(memsys_top_params_t * params);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MEMSYS_TOP_API_H__ */

