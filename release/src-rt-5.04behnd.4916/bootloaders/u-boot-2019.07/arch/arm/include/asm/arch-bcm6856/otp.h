/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2020 Broadcom Ltd.
 */
#ifndef _6856_OTP_H
#define _6856_OTP_H


#define JTAG_OTP_BASE    0xff802800

/* row 8 */
#define OTP_SEC_CHIPVAR_ROW                     8
#define OTP_SEC_CHIPVAR_SHIFT                   24
#define OTP_SEC_CHIPVAR_MASK                    (0xf << OTP_SEC_CHIPVAR_SHIFT)

#define OTP_CPU_CORE_CFG_ROW                    8
#define OTP_CPU_CORE_CFG_SHIFT                  28
#define OTP_CPU_CORE_CFG_MASK                   0x7

/* row 9 */
#define OTP_CPU_CLOCK_FREQ_ROW                  9
#define OTP_CPU_CLOCK_FREQ_SHIFT                0
#define OTP_CPU_CLOCK_FREQ_MASK                 0x7

/* row 17 */
#define OTP_BRCM_BTRM_BOOT_ENABLE_ROW           17
#define OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT         3
#define OTP_BRCM_BTRM_BOOT_ENABLE_MASK          1

/* row 18 */
#define OTP_CUST_BTRM_BOOT_ENABLE_ROW           18
#define OTP_CUST_BTRM_BOOT_ENABLE_SHIFT         15
#define OTP_CUST_BTRM_BOOT_ENABLE_MASK          7

/* row 20 */
#define OTP_JTAG_SER_NUM_ROW                    20
#define OTP_JTAG_SER_NUM_SHIFT                  0x0
#define OTP_JTAG_SER_NUM_MASK                   0xFFFFFFFF

/* row 23 */
#define OTP_CUST_MFG_MRKTID_ROW                 23
#define OTP_CUST_MFG_MRKTID_SHIFT               0
#define OTP_CUST_MFG_MRKTID_MASK                0xffff

/* row 143 */
#define OTP_MFG_PROCESS_ROW                     143
#define OTP_MFG_PROCESS_SHIFT                   0
#define OTP_MFG_PROCESS_MASK                    0xf

#define OTP_MFG_SUBSTRATE_ROW                   143
#define OTP_MFG_SUBSTRATE_SHIFT                 4
#define OTP_MFG_SUBSTRATE_MASK                  0xf

#define OTP_MFG_FOUNDRY_ROW                     143
#define OTP_MFG_FOUNDRY_SHIFT                   8
#define OTP_MFG_FOUNDRY_MASK                    0xf

/* A row initializer that maps actual row number with mask and shift to a feature name;
 * this allows to use features vs. rows for common functionality, 
 * such as secure boot handling frequency, chipid and so on 
 * prevent ifdef dependencies when used outside of arch directories for common among SoCs logic
 * */
#define DEFINE_OTP_MAP_ROW_INITLR(__VV__)                                                                       \
        static otp_hw_cmn_row_t __VV__[ ] = {                                                                   \
        {OTP_MAP_SEC_CHIPVAR, OTP_SEC_CHIPVAR_ROW, OTP_SEC_CHIPVAR_MASK, OTP_SEC_CHIPVAR_SHIFT, 1},             \
        {OTP_MAP_BRCM_BTRM_BOOT_ENABLE, OTP_BRCM_BTRM_BOOT_ENABLE_ROW, OTP_BRCM_BTRM_BOOT_ENABLE_MASK, OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT, 1},\
        {OTP_MAP_CUST_BTRM_BOOT_ENABLE, OTP_CUST_BTRM_BOOT_ENABLE_ROW, OTP_CUST_BTRM_BOOT_ENABLE_MASK, OTP_CUST_BTRM_BOOT_ENABLE_SHIFT, 1},\
        {OTP_MAP_CUST_MFG_MRKTID, OTP_CUST_MFG_MRKTID_ROW, OTP_CUST_MFG_MRKTID_MASK, OTP_CUST_MFG_MRKTID_SHIFT, 1},                             \
        {OTP_MAP_CPU_CLOCK_FREQ, OTP_CPU_CLOCK_FREQ_ROW, OTP_CPU_CLOCK_FREQ_MASK, OTP_CPU_CLOCK_FREQ_SHIFT, 1},                         \
        {OTP_MAP_CPU_CORE_CFG, OTP_CPU_CORE_CFG_ROW, OTP_CPU_CORE_CFG_MASK, OTP_CPU_CORE_CFG_SHIFT, 1},                         \
        {OTP_MAP_MFG_PROCESS, OTP_MFG_PROCESS_ROW, OTP_MFG_PROCESS_MASK, OTP_MFG_PROCESS_SHIFT, 1},                             \
        {OTP_MAP_MFG_SUBSTRATE, OTP_MFG_SUBSTRATE_ROW, OTP_MFG_SUBSTRATE_MASK, OTP_MFG_SUBSTRATE_SHIFT, 1},                             \
        {OTP_MAP_MFG_FOUNDRY, OTP_MFG_FOUNDRY_ROW, OTP_MFG_FOUNDRY_MASK, OTP_MFG_FOUNDRY_SHIFT, 1},                             \
        {OTP_MAP_CSEC_CHIPID, OTP_JTAG_SER_NUM_ROW, OTP_JTAG_SER_NUM_MASK, OTP_JTAG_SER_NUM_SHIFT, 1},                         \
        }

#endif
