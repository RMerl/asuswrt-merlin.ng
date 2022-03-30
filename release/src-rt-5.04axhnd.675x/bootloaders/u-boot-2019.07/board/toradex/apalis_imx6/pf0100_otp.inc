// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014-2016, Toradex AG
 */

// Register Output for PF0100 programmer
// Customer: Toradex AG
// Program: Apalis iMX6
// Sample marking:
// Date: 12.02.2014
// Time: 17:16:41
// Generated from Spreadsheet Revision: P1.8

/* sed commands to get from programmer script to struct */
/* sed -e 's/^WRITE_I2C:\(..\):\(..\)/\{pmic_i2c, 0x\1, 0x\2\},/g' -e 's/^DELAY:\([0-9]*\)/\{pmic_delay, 0, \1\},/g' pf0100_otp.txt > pf0100_otp.inc
   sed -i -e 's/^VPGM:ON/\{pmic_vpgm, 0, 1},/g' -e 's/^VPGM:OFF/\{pmic_vpgm, 0, 0},/g' pf0100_otp.inc
   sed -i -e 's/^PWRON: HIGH/\{pmic_pwr, 0, 1},/g' -e 's/^PWRON:LOW/\{pmic_pwr, 0, 0},/g' pf0100_otp.inc */

enum { pmic_i2c, pmic_delay, pmic_vpgm, pmic_pwr };
struct pmic_otp_prog_t{
	unsigned char cmd;
	unsigned char reg;
	unsigned short value;
};

struct pmic_otp_prog_t pmic_otp_prog[] = {
{pmic_i2c, 0x7F, 0x01}, // Access FSL EXT Page 1
{pmic_i2c, 0xA0, 0x2B}, // Auto gen from Row94
{pmic_i2c, 0xA1, 0x01}, // Auto gen from Row95
{pmic_i2c, 0xA2, 0x05}, // Auto gen from Row96
{pmic_i2c, 0xA8, 0x2B}, // Auto gen from Row102
{pmic_i2c, 0xA9, 0x02}, // Auto gen from Row103
{pmic_i2c, 0xAA, 0x01}, // Auto gen from Row104
{pmic_i2c, 0xAC, 0x18}, // Auto gen from Row106
{pmic_i2c, 0xAE, 0x01}, // Auto gen from Row108
{pmic_i2c, 0xB0, 0x2C}, // Auto gen from Row110
{pmic_i2c, 0xB1, 0x04}, // Auto gen from Row111
{pmic_i2c, 0xB2, 0x01}, // Auto gen from Row112
{pmic_i2c, 0xB4, 0x2C}, // Auto gen from Row114
{pmic_i2c, 0xB5, 0x04}, // Auto gen from Row115
{pmic_i2c, 0xB6, 0x01}, // Auto gen from Row116
{pmic_i2c, 0xB8, 0x18}, // Auto gen from Row118
{pmic_i2c, 0xBA, 0x01}, // Auto gen from Row120
{pmic_i2c, 0xBD, 0x1F}, // Auto gen from Row123
{pmic_i2c, 0xC0, 0x06}, // Auto gen from Row126
{pmic_i2c, 0xC4, 0x04}, // Auto gen from Row130
{pmic_i2c, 0xC8, 0x0E}, // Auto gen from Row134
{pmic_i2c, 0xC9, 0x08}, // Auto gen from Row135
{pmic_i2c, 0xCC, 0x0E}, // Auto gen from Row138
{pmic_i2c, 0xCD, 0x05}, // Auto gen from Row139
{pmic_i2c, 0xD0, 0x0C}, // Auto gen from Row142
{pmic_i2c, 0xD1, 0x05}, // Auto gen from Row143
{pmic_i2c, 0xD5, 0x07}, // Auto gen from Row147
{pmic_i2c, 0xD8, 0x07}, // Auto gen from Row150
{pmic_i2c, 0xD9, 0x06}, // Auto gen from Row151
{pmic_i2c, 0xDC, 0x0A}, // Auto gen from Row154
{pmic_i2c, 0xDD, 0x03}, // Auto gen from Row155
{pmic_i2c, 0xE0, 0x07}, // Auto gen from Row158

#if 0 /* TBB mode */
{pmic_i2c, 0xE4, 0x80}, // TBB_POR = 1
{pmic_delay, 0, 10},
#else
// Write OTP
{pmic_i2c, 0xE4, 0x02}, // FUSE POR1=1
{pmic_i2c, 0xE5, 0x02}, // FUSE POR2=1
{pmic_i2c, 0xE6, 0x02}, // FUSE POR3=1
{pmic_i2c, 0xF0, 0x1F}, // Enable ECC for fuse banks 1 to 5 by writing to OTP EN ECC0 register
{pmic_i2c, 0xF1, 0x1F}, // Enable ECC for fuse banks 6 to 10 by writing to OTP EN ECC1 register
{pmic_i2c, 0x7F, 0x02}, // Access PF0100 EXT Page2
{pmic_i2c, 0xD0, 0x1F}, // Set Auto ECC for fuse banks 1 to 5 by writing to OTP AUTO ECC0 register
{pmic_i2c, 0xD1, 0x1F}, // Set Auto ECC for fuse banks 6 to 10 by writing to OTP AUTO ECC1 register
//-----------------------------------------------------------------------------------
{pmic_i2c, 0xF1, 0x00}, // Reset Bank 1 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF2, 0x00}, // Reset Bank 2 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF3, 0x00}, // Reset Bank 3 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF4, 0x00}, // Reset Bank 4 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF5, 0x00}, // Reset Bank 5 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF6, 0x00}, // Reset Bank 6 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF7, 0x00}, // Reset Bank 7 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF8, 0x00}, // Reset Bank 8 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF9, 0x00}, // Reset Bank 9 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xFA, 0x00}, // Reset Bank 10 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
//-----------------------------------------------------------------------------------
{pmic_vpgm, 0, 1}, // Turn ON 8V SWBST
//VPGM:DOWN:n
//VPGM:UP:n
{pmic_delay, 0, 500}, // Adds 500msec delay to allow VPGM time to ramp up
//-----------------------------------------------------------------------------------
// PF0100 OTP MANUAL-PROGRAMMING (BANK 1 thru 10)
//-----------------------------------------------------------------------------------
// BANK 1
//-----------------------------------------------------------------------------------
{pmic_i2c, 0xF1, 0x00}, // Reset Bank 1 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF1, 0x03}, // Set Bank 1 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF1, 0x0B}, // Set Bank 1 ANTIFUSE_EN
{pmic_delay, 0, 10}, // Allow time for bank programming to complete
{pmic_i2c, 0xF1, 0x03}, // Reset Bank 1 ANTIFUSE_EN
{pmic_i2c, 0xF1, 0x00}, // Reset Bank 1 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
//-----------------------------------------------------------------------------------
// BANK 2
//-----------------------------------------------------------------------------------
{pmic_i2c, 0xF2, 0x00}, // Reset Bank 2 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF2, 0x03}, // Set Bank 2 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF2, 0x0B}, // Set Bank 2 ANTIFUSE_EN
{pmic_delay, 0, 10}, // Allow time for bank programming to complete
{pmic_i2c, 0xF2, 0x03}, // Reset Bank 2 ANTIFUSE_EN
{pmic_i2c, 0xF2, 0x00}, // Reset Bank 2 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
//-----------------------------------------------------------------------------------
// BANK 3
//-----------------------------------------------------------------------------------
{pmic_i2c, 0xF3, 0x00}, // Reset Bank 3 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF3, 0x03}, // Set Bank 3 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF3, 0x0B}, // Set Bank 3 ANTIFUSE_EN
{pmic_delay, 0, 10}, // Allow time for bank programming to complete
{pmic_i2c, 0xF3, 0x03}, // Reset Bank 3 ANTIFUSE_EN
{pmic_i2c, 0xF3, 0x00}, // Reset Bank 3 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
//-----------------------------------------------------------------------------------
// BANK 4
//-----------------------------------------------------------------------------------
{pmic_i2c, 0xF4, 0x00}, // Reset Bank 4 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF4, 0x03}, // Set Bank 4 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF4, 0x0B}, // Set Bank 4 ANTIFUSE_EN
{pmic_delay, 0, 10}, // Allow time for bank programming to complete
{pmic_i2c, 0xF4, 0x03}, // Reset Bank 4 ANTIFUSE_EN
{pmic_i2c, 0xF4, 0x00}, // Reset Bank 4 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
//-----------------------------------------------------------------------------------
// BANK 5
//-----------------------------------------------------------------------------------
{pmic_i2c, 0xF5, 0x00}, // Reset Bank 5 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF5, 0x03}, // Set Bank 5 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF5, 0x0B}, // Set Bank 5 ANTIFUSE_EN
{pmic_delay, 0, 10}, // Allow time for bank programming to complete
{pmic_i2c, 0xF5, 0x03}, // Reset Bank 5 ANTIFUSE_EN
{pmic_i2c, 0xF5, 0x00}, // Reset Bank 5 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
//-----------------------------------------------------------------------------------
// BANK 6
//-----------------------------------------------------------------------------------
{pmic_i2c, 0xF6, 0x00}, // Reset Bank 6 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF6, 0x03}, // Set Bank 6 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF6, 0x0B}, // Set Bank 6 ANTIFUSE_EN
{pmic_delay, 0, 10}, // Allow time for bank programming to complete
{pmic_i2c, 0xF6, 0x03}, // Reset Bank 6 ANTIFUSE_EN
{pmic_i2c, 0xF6, 0x00}, // Reset Bank 6 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
//-----------------------------------------------------------------------------------
// BANK 7
//-----------------------------------------------------------------------------------
{pmic_i2c, 0xF7, 0x00}, // Reset Bank 7 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF7, 0x03}, // Set Bank 7 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF7, 0x0B}, // Set Bank 7 ANTIFUSE_EN
{pmic_delay, 0, 10}, // Allow time for bank programming to complete
{pmic_i2c, 0xF7, 0x03}, // Reset Bank 7 ANTIFUSE_EN
{pmic_i2c, 0xF7, 0x00}, // Reset Bank 7 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
//-----------------------------------------------------------------------------------
// BANK 8
//-----------------------------------------------------------------------------------
{pmic_i2c, 0xF8, 0x00}, // Reset Bank 8 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF8, 0x03}, // Set Bank 8 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF8, 0x0B}, // Set Bank 8 ANTIFUSE_EN
{pmic_delay, 0, 10}, // Allow time for bank programming to complete
{pmic_i2c, 0xF8, 0x03}, // Reset Bank 8 ANTIFUSE_EN
{pmic_i2c, 0xF8, 0x00}, // Reset Bank 8 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
//-----------------------------------------------------------------------------------
// BANK 9
//-----------------------------------------------------------------------------------
{pmic_i2c, 0xF9, 0x00}, // Reset Bank 9 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF9, 0x03}, // Set Bank 9 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xF9, 0x0B}, // Set Bank 9 ANTIFUSE_EN
{pmic_delay, 0, 10}, // Allow time for bank programming to complete
{pmic_i2c, 0xF9, 0x03}, // Reset Bank 9 ANTIFUSE_EN
{pmic_i2c, 0xF9, 0x00}, // Reset Bank 9 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
//-----------------------------------------------------------------------------------
// BANK 10
//-----------------------------------------------------------------------------------
{pmic_i2c, 0xFA, 0x00}, // Reset Bank 10 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xFA, 0x03}, // Set Bank 10 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
{pmic_i2c, 0xFA, 0x0B}, // Set Bank 10 ANTIFUSE_EN
{pmic_delay, 0, 10}, // Allow time for bank programming to complete
{pmic_i2c, 0xFA, 0x03}, // Reset Bank 10 ANTIFUSE_EN
{pmic_i2c, 0xFA, 0x00}, // Reset Bank 10 ANTIFUSE_RW and ANTIFUSE_BYPASS bits
//-----------------------------------------------------------------------------------
{pmic_vpgm, 0, 0}, // Turn off 8V SWBST
{pmic_delay, 0, 500}, // Adds delay to allow VPGM to bleed off
{pmic_i2c, 0xD0, 0x00}, // Clear
{pmic_i2c, 0xD1, 0x00}, // Clear
{pmic_pwr, 0, 0}, // PWRON LOW to reload new OTP data
{pmic_delay, 0, 500},
{pmic_pwr, 0, 1},
#endif
};