/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

#ifndef _BCM_SMTC_H_

#include <linux/string.h>
#include "linux/ctype.h"
#include "bcmsfp_i2c.h"

#define TABLE_SEL               0x7F
#define Ax_CRC_EN               0x60
#define EE_UPDATE_DIS           0x1
#define DIG_STATUS_MODE_MASK    0b1
#define CHIP_NAME               "GN28L96_A2"

#define STATUS_CONTROL          0x6E
#define BEN_CTRL                0x87
#define SAFE_MODE_STARTUP       0x88
#define DIG_STATUS              0x89
#define I2C_CONTROL_1           0x8A
#define EEPROM_CTRL_1           0x8D
#define PON_CONTROL             0x6F
#define PW1_0                   0x98
#define PWE_7B                  0x7B
#define PWE_7C                  0x7C
#define PWE_7D                  0x7D
#define PWE_7E                  0x7E
#define APC_BIAS_SEED           0x90
#define ERC_MOD_SEED            0xB0
#define TX_CTRL_3               0x8B

/* compiled-in defaults */
extern const u8 A2_Table_80_file[], A2_Table_81_file[], A2_Lower_file[], A0_Lower_file[], A0_Upper_file[];

enum page_desc_indices {
A0_Lower,
A0_Upper,
A2_Lower,
A2_Table_00,
A2_Table_01,
A2_Table_80,
A2_Table_81,
A2_Table_82,
A2_Table_83,
A2_Table_84,
A2_Table_85,
A2_Table_86,
A2_Table_FF,
};

struct page_desc {
    enum page_desc_indices page_desc_index;
    unsigned char table_num;
    char name[16];
    unsigned char client_num;
    unsigned char i2c_chip_addr;
    unsigned char start_data_addr;
    unsigned char file_data_addr_off; /* A2_Table_*.txt use defines starting from 0x80, but A0_Upper doesn't */
    unsigned char max_rows;
    signed   char num_exceptions;
    unsigned char exceptions[16];
};

static const struct page_desc page_descs[] = {
    [A0_Lower] =    { A0_Lower,       0, "A0_Lower",    SFP_CLIENT_EEPROM, SFP_I2C_EEPROM_ADDR,    0,    0, 128, 1, { 0, }, },
    [A0_Upper] =    { A0_Upper,       0, "A0_Upper",    SFP_CLIENT_EEPROM, SFP_I2C_EEPROM_ADDR, 0x80,    0, 128, 0, {}, },
    [A2_Lower] =    { A2_Lower,       0, "A2_Lower",    SFP_CLIENT_DIAG,   SFP_I2C_DIAG_ADDR,      0,    0, 128, 5, { PWE_7B, PWE_7C, PWE_7D, PWE_7E, TABLE_SEL, }, },
    [A2_Table_00] = { A2_Table_00,    0, "A2_Table_00", SFP_CLIENT_DIAG,   SFP_I2C_DIAG_ADDR,   0x80, 0x80, 128, 0, {}, },
    [A2_Table_01] = { A2_Table_01,    1, "A2_Table_01", SFP_CLIENT_DIAG,   SFP_I2C_DIAG_ADDR,   0x80, 0x80, 128, 0, {}, },
    [A2_Table_80] = { A2_Table_80, 0x80, "A2_Table_80", SFP_CLIENT_DIAG,   SFP_I2C_DIAG_ADDR,   0x80, 0x80, 128, 0, {}, },
    [A2_Table_81] = { A2_Table_81, 0x81, "A2_Table_81", SFP_CLIENT_DIAG,   SFP_I2C_DIAG_ADDR,   0x80, 0x80, 128, 2, {SAFE_MODE_STARTUP, EEPROM_CTRL_1, }, },
    [A2_Table_82] = { A2_Table_82, 0x82, "A2_Table_82", SFP_CLIENT_DIAG,   SFP_I2C_DIAG_ADDR,   0x80, 0x80, 128, 0, {}, },
    [A2_Table_83] = { A2_Table_83, 0x83, "A2_Table_83", SFP_CLIENT_DIAG,   SFP_I2C_DIAG_ADDR,   0x80, 0x80, 128, 0, {}, },
    [A2_Table_84] = { A2_Table_84, 0x84, "A2_Table_84", SFP_CLIENT_DIAG,   SFP_I2C_DIAG_ADDR,   0x80, 0x80, 128, 0, {}, },
    [A2_Table_85] = { A2_Table_85, 0x85, "A2_Table_85", SFP_CLIENT_DIAG,   SFP_I2C_DIAG_ADDR,   0x80, 0x80, 128, 0, {}, },
    [A2_Table_86] = { A2_Table_86, 0x86, "A2_Table_86", SFP_CLIENT_DIAG,   SFP_I2C_DIAG_ADDR,   0x80, 0x80, 128, 0, {}, },
    [A2_Table_FF] = { A2_Table_FF, 0xFF, "A2_Table_FF", SFP_CLIENT_DIAG,   SFP_I2C_DIAG_ADDR,   0x80, 0x80, 128, 0, {}, },
};
// FIXME A0_Lower (3F and 5F) and A2_Lower (5F) may be exceptions, CSUM auto computed.  
// but maybe works only with EEPROM, not soft registers

// uncomment 2 #defines below to use simulator in smtc2.c instead of eeprom 
// #define bcmsfp_read_byte  sim_bcmsfp_read_byte
// #define bcmsfp_write_byte sim_bcmsfp_write_byte
extern int sim_bcmsfp_read_byte(u8 bus_num, u8 client_num, u8 offset, u8* val);
extern int sim_bcmsfp_write_byte(u8 bus_num, u8 client_num, u8 offset, u8 val);
static int textbuf_to_array(const struct page_desc *page_desc_p, char *file_buf, u8 *eeprom_buf);
static int array_to_regs(const struct page_desc *page_desc_p, const u8 *eeprom_buf);

static inline int is_exception(const struct page_desc *page_desc, u8 data_addr)
{
    int i;

    if (!page_desc || data_addr >= 0x80)
         printk("%s.%d.%s: page_desc=%px, data_addr=0x%02x\n", __FILE__, __LINE__, __FUNCTION__, page_desc, data_addr);

    for (i = 0; i < page_desc->num_exceptions; i++)
        if (data_addr == page_desc->exceptions[i])
            return 1;
    return 0;
}

static inline void s_tolower(char *s)
{
    int i;

    for (i = 0; s[i]; i++)
        s[i] = tolower(s[i]);
}

#endif
