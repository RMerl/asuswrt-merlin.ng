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

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/version.h>
#include "linux/delay.h"
#include "linux/kthread.h"
#include "linux/reboot.h"
#include <bcm_smtc.h>
#include "board.h"
#include "bcm_pinmux.h"
#include "bcm_gpio.h"

#define KEEP_EEPROM 1
/* SEEDS should be set in per-board calibration */
#define PER_BOARD_APC_BIAS_SEED         0x019B // works both for 6858SMTC and 6856CHGU
#define PER_BOARD_ERC_MOD_SEED          0x01D4 // works 6856CHGU BUT MAY NOT WORK for 6858SMTC
/* to obtain SEED values (if Rx works but Tx doesn't work and can't get O5)...
   first do prbs for 1 second: serdesctrl prbs 1 1 0 10; sleep 1; serdesctrl prbs 0 1 0 10
   then replace seed values APC_BIAS_SEED and ERC_MOD_SEED above with output of (change bus number as needed):
   i2cget -f -y 0 0x51 0x7f > /tmp/0 ; i2cset -f -y 0 0x51 0x7f 0x80 ; sleep 1 ; i2cget -f -y 0 0x51 0x92 w ; i2cget -f -y 0 0x51 0xb4 w ; i2cset -f -y 0 0x51 0x7f `cat /tmp/0`

   to set seed values without recompiling driver:
   i2cget -f -y 0 0x51 0x7f > /tmp/0 ; i2cset -f -y 0 0x51 0x7f 0x80 ; sleep 1 ;
   i2cget -f -y 0 0x51 0x92 w > /tmp/1 ; i2cset -f -y 0 0x51 0x90 `cat /tmp/1` w ;sleep 1;
   i2cget -f -y 0 0x51 0xb4 w > /tmp/2 ; i2cset -f -y 0 0x51 0xb0 `cat /tmp/2` w; sleep 1;
   i2cset -f -y 0 0x51 0x7f `cat /tmp/0` */

#define PRINTK(format, ...) printk("%s.%d.%s: " format, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);

extern int kerSysFsFileGet(const char *filename, char *buf, int len);

enum smtc_board_mode {
    unknown,
    eeprom_less,
    eeprom_blank,
    eeprom_programmed,
} smtc_board_mode = unknown;

struct task_struct *smtc_thread = NULL;
int  smtc_thread_arg = 0;
u8 smtc_bus_num = 0xFF;


static void smtc_tx_en(void)
{
    unsigned short gpio;
    int rc;

    if (((rc = BpGetPonSMTCTxDisGpio(&gpio)) != BP_SUCCESS) || (gpio == BP_GPIO_NONE))
    {
        PRINTK("Could not get GPIO defined for TX_DIS, signal should be low, rc=%d\n", rc);
        return;
    }

    kerSysSetGpioDir(gpio);
    kerSysSetGpioState(gpio, kGpioInactive);
}

/* macros to read/write from register, print error, set ret and execute cmd if fail.  do not set A2:7F */
#define GET_BYTE_FROM_A2_TABLE(offset, var, fail, cmd)                                  \
    {                                                                                   \
        int rc;                                                                         \
                                                                                        \
        if ((rc = bcmsfp_read_byte(smtc_bus_num, SFP_CLIENT_DIAG, offset, &var)))       \
        {                                                                               \
            PRINTK("bcmsfp_read_byte failed I2C bus=%d, client=0x%x, addr=0x%02x, rc=%d\n", smtc_bus_num, SFP_CLIENT_DIAG, offset, rc);                   \
            ret = fail;                                                                 \
            cmd;                                                                        \
        }                                                                               \
    }

#define SET_BYTE_FROM_A2_TABLE(offset, val, fail, cmd)                                  \
    {                                                                                   \
        int rc;                                                                         \
                                                                                        \
        if ((rc = bcmsfp_write_byte(smtc_bus_num, SFP_CLIENT_DIAG, offset, val)))       \
        {                                                                               \
            PRINTK("bcmsfp_write_byte failed I2C bus=%d, client=0x%x, addr=0x%02x, rc=%d\n", smtc_bus_num, SFP_CLIENT_DIAG, offset, rc);                  \
            ret = fail;                                                                 \
            cmd;                                                                        \
        }                                                                               \
        msleep(10); /* o/w get EIO */                                                   \
    }

#define GET_WORD_FROM_A2_TABLE(offset, var, fail, cmd)                                  \
    {                                                                                   \
        int rc;                                                                         \
                                                                                        \
        if ((rc = bcmsfp_read_word(smtc_bus_num, SFP_CLIENT_DIAG, offset, &var)))       \
        {                                                                               \
            PRINTK("bcmsfp_read_word failed I2C bus=%d, client=0x%x, addr=0x%02x, rc=%d\n", smtc_bus_num, SFP_CLIENT_DIAG, offset, rc);                   \
            ret = fail;                                                                 \
            cmd;                                                                        \
        }                                                                               \
    }

#define SET_WORD_FROM_A2_TABLE(offset, val, fail, cmd)                                  \
    {                                                                                   \
        int rc;                                                                         \
        if ((rc = bcmsfp_write_word(smtc_bus_num, SFP_CLIENT_DIAG, offset, val)))       \
        {                                                                               \
            PRINTK("bcmsfp_write_word failed I2C bus=%d, client=0x%x, addr=0x%02x, rc=%d\n", smtc_bus_num, SFP_CLIENT_DIAG, offset, rc);                  \
            ret = fail;                                                                 \
            cmd;                                                                        \
        }                                                                               \
        msleep(10); /* o/w get EIO */                                                   \
    }

#define GET_BYTE_FROM_A0(offset, var, fail, cmd)                                        \
    {                                                                                   \
        int rc;                                                                         \
                                                                                        \
        if ((rc = bcmsfp_read_byte(smtc_bus_num, SFP_CLIENT_EEPROM, offset, &var)))     \
        {                                                                               \
            PRINTK("bcmsfp_read_byte failed I2C bus=%d, client=0x%x, addr=0x%02x, rc=%d\n", smtc_bus_num, SFP_CLIENT_EEPROM, offset, rc);                 \
            ret = fail;                                                                 \
            cmd;                                                                        \
        }                                                                               \
    }

#define SET_BYTE_FROM_A0(offset, val, fail, cmd)                                        \
    {                                                                                   \
        int rc;                                                                         \
                                                                                        \
        if ((rc = bcmsfp_write_byte(smtc_bus_num, SFP_CLIENT_EEPROM, offset, val)))     \
        {                                                                               \
            PRINTK("bcmsfp_write_byte failed I2C bus=%d, client=0x%x, addr=0x%02x, rc=%d\n", smtc_bus_num, SFP_CLIENT_EEPROM, offset, rc);                \
            ret = fail;                                                                 \
            cmd;                                                                        \
        }                                                                               \
        msleep(10); /* o/w get EIO */                                                   \
    }

/* find the desc for a filename A[02]_{Low,Upp}er* A2_Table_xx* */
__attribute__((unused)) static const struct page_desc *filename_to_desc(char *fn)
{
    unsigned int A, matched, table;
    char *basename, *pc, mid[16];
    const struct page_desc *ret;

    if ( (pc = strrchr(fn, '/')) == NULL )
        basename = fn;
    else
        basename = pc + 1;

    s_tolower(basename);

    matched = sscanf(basename, "a%d_%15[^_.]_%x", &A, mid, &table);

    if ( (matched < 2) || (matched > 3)
         || ((matched == 3) && (A != 2))
         || ((matched == 2) && strcmp(mid, "lower") && strcmp(mid, "upper"))
         || ((matched == 3) && ( strcmp(mid, "table")
            || ( ((table > 0x02) && (table < 0x80)) || ((table > 0x86) && (table < 0xFF)) || (table > 0xFF) ) ) ) )
    {
        PRINTK("malformed file name %s, matched==%d, A==%d, mid=='%s', table==0x%x\n", basename, matched, A, mid, table);
        return NULL;
    }
    printk("file name %s, matched==%d, A==%d, mid=='%s', table==0x%x\n", basename, matched, A, mid, table);

    if (A == 0)
        ret = &page_descs[ strcmp(mid, "lower") ? A0_Upper : A0_Lower ];
    else if (!strcmp(mid, "lower"))
        ret = &page_descs[ A2_Lower ];
    else if (!strcmp(mid, "upper"))
        ret = &page_descs[ A2_Table_00 ];
    else
    {
        if (table <= 0x02)
            ret = &page_descs[ A2_Table_00 + table ];
        else if (table <= 0x86)
            ret = &page_descs[ A2_Table_80 + table - 0x80 ];
        else
            ret = &page_descs[ A2_Table_FF ];
    }

    printk("page is %s\n", ret->name);
    return ret;
}

/* parse a buffer of text (e.g. from a file) for a given desc, check format and boundaries, and save in array
   each line in file_buf should be <= 15, sufficient for worst case line '0xDE 0xAD \r\n\0' */
static int textbuf_to_array(const struct page_desc *page_desc_p, char *file_buf, u8 *eeprom_buf)
{
    unsigned int l_num, i, j, n, data_addr, val;
    char set[0x80] = {}, temp_buf[0x80] = { [0 ... 0x7F] = 0xA5, };

    if (!page_desc_p || !file_buf || !eeprom_buf)
        return 1;

    PRINTK("desc: %d, %x, %s, %d, 0x%02x, 0x%02x, 0x%02x, %d, %d\n", page_desc_p->page_desc_index, page_desc_p->table_num, page_desc_p->name, page_desc_p->client_num, page_desc_p->i2c_chip_addr, page_desc_p->start_data_addr, page_desc_p->file_data_addr_off, page_desc_p->max_rows, page_desc_p->num_exceptions);
    for (l_num = 1 , i = 0; file_buf[i]; l_num++ , i += n)
    {
        j = sscanf(file_buf + i, "0x%x 0x%x %n", &data_addr, &val, &n);
        if (j != 2)
        {
            PRINTK("malformed line %d, sscanf returned %d\n", l_num, j);
            return 2;
        }
        data_addr -= page_desc_p->file_data_addr_off; /* from here all is 0..7F */

        if (data_addr > 0x7F)
        {
            PRINTK("data_addr==0x%02x > 0x7F\n", data_addr);
            return 3;
        }
        if (l_num > page_desc_p->max_rows)
        {
            PRINTK("more than %d lines\n", page_desc_p->max_rows);
            return 4;
        }
        if (is_exception(page_desc_p, data_addr))
        {
            PRINTK("data_addr==0x%x is an exception and will not be programmed\n", data_addr);
        }
        else
        {
            set[data_addr]++;
            temp_buf[data_addr] = val;
        }
    }

    for (j = 0; j < 0x80; j++)
    {
        if (!set[j] && !is_exception(page_desc_p, j))
        {
            PRINTK("data_addr==0x%x was not set\n", j);
            return 5;
        }
    }

    memcpy(eeprom_buf, temp_buf, sizeof(temp_buf));
    return 0;
}

/* write values from an array for a given desc to i2c registers.  set A2:7F and EE_UPDATE before call */
static int array_to_regs(u8 bus_num, const struct page_desc *page_desc_p, const u8 *eeprom_buf)
{
    unsigned int ret = 0, i;

    for (i = 0; i < 0x80; i++)
    {
        if (!is_exception(page_desc_p, i))
        {
            /* these seed values must be written as 16bit */
            if ((page_desc_p->page_desc_index == A2_Table_80) && (i + page_desc_p->start_data_addr == APC_BIAS_SEED))
            {
                SET_WORD_FROM_A2_TABLE(APC_BIAS_SEED, (eeprom_buf[i] << 8) | eeprom_buf[i+1], 1, return ret);
            }
            if ((page_desc_p->page_desc_index == A2_Table_80) && (i + page_desc_p->start_data_addr == ERC_MOD_SEED))
            {
                SET_WORD_FROM_A2_TABLE(ERC_MOD_SEED,  (eeprom_buf[i] << 8) | eeprom_buf[i+1], 2, return ret);
            }
            if ((page_desc_p->page_desc_index == A2_Table_80) && ((i + page_desc_p->start_data_addr == ERC_MOD_SEED + 1) || (i + page_desc_p->start_data_addr == APC_BIAS_SEED + 1)))
                continue;

            if ((ret = bcmsfp_write_byte(bus_num, page_desc_p->client_num, i + page_desc_p->start_data_addr, eeprom_buf[i])))
            {
                PRINTK("Failed to write i2c value at bus_num=%d, client=0x%02x, addr 0x%02x, val=0x%02x, ret=%d\n", bus_num, page_desc_p->client_num, i + page_desc_p->start_data_addr, eeprom_buf[i], ret);
                return ret;
            }
            msleep(10);
        }
    }

    return 0;
}


/* save current indirect access pointer to A2_Upper from 0x7F, and set new value (unless it's the same) */
static int set_A2_table(u8 bus_num, u8 new_table, u8 *curr_table)
{
    int rc;
    static u8 latched_table = 0xAA;
    static int depth = 0;

    if (depth++ != 0)
    {
        printk("Unexpected Error.  %s should not reenter, depth==%d\n", __FUNCTION__, depth);
        dump_stack();
    }

    if (new_table == latched_table)
    {
        if (curr_table)
            *curr_table = latched_table;
    }
    else
    {
        if (curr_table && (rc = bcmsfp_read_byte(bus_num, SFP_CLIENT_DIAG, TABLE_SEL, curr_table)))
        {
            PRINTK("Failed to read i2c value at bus_num=%d, addr 0x%02x, rc=%d\n", bus_num, TABLE_SEL, rc);
            depth--;
            return 1;
        }
        if ((rc = bcmsfp_write_byte(bus_num, SFP_CLIENT_DIAG, TABLE_SEL, new_table)))
        {
            PRINTK("Failed to write i2c value at bus_num=%d, addr 0x%02x, rc=%d\n", bus_num, TABLE_SEL, rc);
            depth--;
            // we assume that if the write failed, there is no need to restore the table number
            return 1;
        }
        msleep(10);
        latched_table = new_table;
    }
    depth--;
    return 0;
}

/* dump functions yield output similar to i2cdump cli */
static void i2cdump_title(void)
{
    int i;

    printk("%3s", "");
    for (i=0; i<16; i++)
        printk(" %2x", i);
    printk("%4s", "");
    for (i=0; i<16; i++)
        printk("%x", i);
    printk("\n");
}

static void i2cdump_print16(u8 *buf)
{
    int i;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
#error // FIXME 4.19 requires \n at the end of every printk
#endif
    for (i = 0; i < 16; i++)
    {
        printk(" %02x", buf[i]);
    }
    printk("%4s", "");
    for (i = 0; i < 16; i++)
    {
        printk("%c", (isprint(buf[i]) && buf[i] >= 0x20 && buf[i] < 0x7F) ? buf[i] : '?');
    }
    printk("\n");
}

/* saves and restores A2:7F if required */
static __attribute__((unused)) void i2cdump_256(u8 bus_num, u8 client_num, u8 table)
{
    int i, j, rc, addr;
    u8 buf[16], saved_table;

    if (client_num == SFP_CLIENT_DIAG)
        if (set_A2_table(bus_num, table, &saved_table))
            return;

    i2cdump_title();
    for (j = 0; j < 16; j++)
    {
        printk("%02x:", 16*j);
        for (i = 0; i < 16; i++)
        {
            addr = 16*j + i;
            if ((rc = bcmsfp_read_byte(bus_num, client_num, addr, &buf[i])))
            {
               PRINTK("bcmsfp_read_byte failed I2C bus=%d, client=0x%x, addr=0x%02x, rc=%d\n", bus_num, client_num, addr, rc);
               buf[i] = 'X';
            }
        }
        i2cdump_print16(buf);
    }

    if (client_num == SFP_CLIENT_DIAG)
        set_A2_table(bus_num, saved_table, NULL);
}

static __attribute__((unused)) void i2cdump_desc(u8 bus_num, struct page_desc *page_desc)
{
    int i, j, rc, addr;
    u8 buf[16], saved_table;
    const bool is_A0 = page_desc->page_desc_index <= A0_Upper;
    const u8 client_num = is_A0 ? SFP_CLIENT_EEPROM : SFP_CLIENT_DIAG;

    if (!is_A0)
        if (set_A2_table(bus_num, page_desc->table_num, &saved_table))
            return;

    i2cdump_title();
    for (j = 0; j < 8; j++)
    {
        printk("%02x:", 16*j);
        for (i = 0; i < 16; i++)
        {
            addr = 16*j + i;
            if ((rc = bcmsfp_read_byte(bus_num, client_num, addr, &buf[i])))
            {
               PRINTK("bcmsfp_read_byte failed I2C bus=%d, client=0x%x, addr=0x%02x, rc=%d\n", bus_num, client_num, addr, rc);
               buf[i] = 'X';
            }
        }
        i2cdump_print16(buf);
    }

    if (client_num == SFP_CLIENT_DIAG)
        set_A2_table(bus_num, saved_table, NULL);
}

static int setPW(void)
{
    int ret = 0;
#if !KEEP_EEPROM
#if 0
    u8 i, pw[] = { 0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78, };

    for (i = 0; i < 8; i++)
        SET_BYTE_FROM_A2_TABLE(PW1_0 + i, pw[i], 1, goto out);

out:
#endif
#endif
    return ret;
}

__attribute__((unused)) static int set_seeds(void)
{
    int ret = 0;
    u8 saved_table;

    if (set_A2_table(smtc_bus_num, 0x80, &saved_table))
    {
        ret = 1;
        goto out;
    }

    SET_WORD_FROM_A2_TABLE(APC_BIAS_SEED, PER_BOARD_APC_BIAS_SEED, 2, goto restore_A2_Table_and_out);
    SET_WORD_FROM_A2_TABLE(ERC_MOD_SEED,  PER_BOARD_ERC_MOD_SEED, 3, goto restore_A2_Table_and_out);

restore_A2_Table_and_out:
    if (set_A2_table(smtc_bus_num, saved_table, NULL))
        ret = ret ?: 4;

out:
    return ret;
}

/* if eeprom_less design, use A2 values from non-volatile fs, otherwise use compiled-in defaults */
static int _eeprom_less_init(void)
{
    int ret = 0;
    char file_buf[128*15] = {}; /* 15 is sufficient for worst case line '0xDE 0xAD \r\n\0' */
    u8 val;
    static u8 eeprom_buf[0x80]; /* doesn't fit activation frame */

    GET_BYTE_FROM_A2_TABLE(DIG_STATUS, val, 2, return ret);

    if ( ! (val & DIG_STATUS_MODE_MASK) )
    {
        PRINTK("DIG_STATUS MODE should be 1 (MCU MODE) when EEPROM absent, dig_status=0x%02x\n", val);
        return 3;
    }

    GET_BYTE_FROM_A2_TABLE(EEPROM_CTRL_1, val, 4, return ret);
    val |= Ax_CRC_EN;
    SET_BYTE_FROM_A2_TABLE(EEPROM_CTRL_1, val, 5, return ret);

    /* set PW even if EEPROM less */
    if (setPW())
        return 6;

    if (kerSysFsFileGet("/data/smtc/A2_Lower_calib.txt", file_buf, sizeof(file_buf)) < 0)
    {
        PRINTK("could not read %s, trying defaults\n", "/data/smtc/A2_Lower_calib.txt");
        memcpy(eeprom_buf, A2_Lower_file, 0x80);
    }
    else
    {
        if (textbuf_to_array(&page_descs[A2_Lower], file_buf, eeprom_buf))
            return 10;
    }
    PRINTK("saving %s\n", page_descs[A2_Lower].name);
    if (array_to_regs(smtc_bus_num, &page_descs[A2_Lower], eeprom_buf))
        return 11;

    if (set_A2_table(smtc_bus_num, 0x80, NULL))
        return 12;
    if (kerSysFsFileGet("/data/smtc/A2_Table_80_calib.txt", file_buf, sizeof(file_buf)) < 0)
    {
        PRINTK("could not read %s, trying defaults\n", "/data/smtc/A2_Table_80_calib.txt");
        memcpy(eeprom_buf, A2_Table_80_file, 0x80);
    }
    else
    {
        if (textbuf_to_array(&page_descs[A2_Table_80], file_buf, eeprom_buf))
            return 13;
    }
    PRINTK("saving %s\n", page_descs[A2_Table_80].name);
    if (array_to_regs(smtc_bus_num, &page_descs[A2_Table_80], eeprom_buf))
        return 14;

    if (set_A2_table(smtc_bus_num, 0x81, NULL))
        return 15;
    if (kerSysFsFileGet("/data/smtc/A2_Table_81_calib.txt", file_buf, sizeof(file_buf)) < 0)
    {
        PRINTK("could not read %s, trying defaults\n", "/data/smtc/A2_Table_81_calib.txt");
        memcpy(eeprom_buf, A2_Table_81_file, 0x80);
    }
    else
    {
        if (textbuf_to_array(&page_descs[A2_Table_81], file_buf, eeprom_buf))
            return 16;
    }
    PRINTK("saving %s\n", page_descs[A2_Table_81].name);
    if (array_to_regs(smtc_bus_num, &page_descs[A2_Table_81], eeprom_buf))
        return 17;

    PRINTK("saving %s\n", page_descs[A0_Lower].name);
    if (array_to_regs(smtc_bus_num, &page_descs[A0_Lower], A0_Lower_file))
        return 18;

    PRINTK("saving %s\n", page_descs[A0_Upper].name);
    if (array_to_regs(smtc_bus_num, &page_descs[A0_Upper], A0_Upper_file))
        return 19;

    /* check if EEPROMless device responds to AA instead of A0.  if it does, program A0 values through AA, then enable LOW_PAGE.
       otherwise, enable LOW_PAGE first and program through A0 */

    GET_BYTE_FROM_A2_TABLE(I2C_CONTROL_1, val, 22, return ret);
    val &= ~0x02; /* clear LOW_PAGE_DIS */
    SET_BYTE_FROM_A2_TABLE(I2C_CONTROL_1, val, 23, return ret);

    return 0;
}

static int eeprom_less_init(void)
{
    int ret = 0;
    u8 saved_table;

    if (set_A2_table(smtc_bus_num, 0x81, &saved_table))
        return 40;

    ret = _eeprom_less_init();

    if (set_A2_table(smtc_bus_num, saved_table, NULL))
        ret = ret ?: 41;

    return ret;
}

/* write compiled-in defaults */
static int _eeprom_blank_init(void)
{
    int ret = 0;
    __attribute__((unused)) char file_buf[128*15];
    u8 val;

    GET_BYTE_FROM_A2_TABLE(DIG_STATUS, val, 2, return ret);

    if (val & DIG_STATUS_MODE_MASK)
    {
        PRINTK("DIG_STATUS MODE should be 0 (EEPROM MODE) when EEPROM blank, DIG_STATUS=0x%02x\n", val);
        return 3;
    }

    GET_BYTE_FROM_A2_TABLE(EEPROM_CTRL_1, val, 6, return ret);
    val |= Ax_CRC_EN;
    SET_BYTE_FROM_A2_TABLE(EEPROM_CTRL_1, val, 7, return ret);

#if !KEEP_EEPROM
    if (setPW())
        return 8;

    PRINTK("saving %s\n", page_descs[A2_Lower].name);
    if (array_to_regs(smtc_bus_num, &page_descs[A2_Lower], A2_Lower_file))
        return 9;

    if (set_A2_table(smtc_bus_num, 0x80, NULL))
        return 10;
    PRINTK("saving %s\n", page_descs[A2_Table_80].name);
    if (array_to_regs(smtc_bus_num, &page_descs[A2_Table_80], A2_Table_80_file))
        return 12;

    if (set_A2_table(smtc_bus_num, 0x81, NULL))
        return 11;
    PRINTK("saving %s\n", page_descs[A2_Table_81].name);
    if (array_to_regs(smtc_bus_num, &page_descs[A2_Table_81], A2_Table_81_file))
        return 13;
#endif

    PRINTK("saving %s\n", page_descs[A0_Lower].name);
    if (array_to_regs(smtc_bus_num, &page_descs[A0_Lower], A0_Lower_file))
        return 14;

    PRINTK("saving %s\n", page_descs[A0_Upper].name);
    if (array_to_regs(smtc_bus_num, &page_descs[A0_Upper], A0_Upper_file))
        return 15;

    return 0;
}

static int eeprom_blank_init(void)
{
    int ret = 0;
    u8 saved_table;

    if (set_A2_table(smtc_bus_num, 0x81, &saved_table))
        return 40;

    ret = _eeprom_blank_init();

    if (set_A2_table(smtc_bus_num, saved_table, NULL))
        ret = ret ?: 41;

    return ret;
}

/* Voltage and Tx Bias calibration were swapped in early boards */
static int fix_voltage_bias_calib(void)
{
    u8 val, val_c8, val_c9, val_cc, val_cd, val_ae, val_af, ret = 0;

    GET_BYTE_FROM_A2_TABLE(0xc8, val_c8, 10, goto out);
    GET_BYTE_FROM_A2_TABLE(0xc9, val_c9, 11, goto out);
    GET_BYTE_FROM_A2_TABLE(0xcc, val_cc, 12, goto out);
    GET_BYTE_FROM_A2_TABLE(0xcd, val_cd, 13, goto out);
    GET_BYTE_FROM_A2_TABLE(0xae, val_ae, 14, goto out);
    GET_BYTE_FROM_A2_TABLE(0xaf, val_af, 15, goto out);

    if ((val_c8 == 0x8b) && (val_c9 == 0xa6) && (val_cc == 0x03) && (val_cd == 0xe8) && (val_ae == 0x70) && (val_af == 0xf0))
    {
        GET_BYTE_FROM_A2_TABLE(EEPROM_CTRL_1, val, 16, goto out);
        val &= ~EE_UPDATE_DIS;
        SET_BYTE_FROM_A2_TABLE(EEPROM_CTRL_1, val, 17, goto out);

        SET_BYTE_FROM_A2_TABLE(0xc8, 0x4e, 18, goto out);
        SET_BYTE_FROM_A2_TABLE(0xc9, 0x20, 19, goto out);
        SET_BYTE_FROM_A2_TABLE(0xcc, 0xdf, 20, goto out);
        SET_BYTE_FROM_A2_TABLE(0xcd, 0x70, 21, goto out);
        SET_BYTE_FROM_A2_TABLE(0xae, 0xf0, 22, goto out);
        SET_BYTE_FROM_A2_TABLE(0xaf, 0xc0, 23, goto out);

        val |= EE_UPDATE_DIS;
        SET_BYTE_FROM_A2_TABLE(EEPROM_CTRL_1, val, 24, goto out);
    }

out:
    return ret;
}

/* if eeprom already programmed, little is needed, transceiver will be detected as is */
static int eeprom_programmed_init(void)
{
    int ret = 0;
    u8 saved_table, val;

    if (set_A2_table(smtc_bus_num, 0x81, &saved_table))
        return 1;

    GET_BYTE_FROM_A2_TABLE(DIG_STATUS, val, 2, goto restore_A2_Table_and_out);
    if (val & DIG_STATUS_MODE_MASK)
    {
        PRINTK("DIG_STATUS MODE should be 0 (EEPROM MODE) when EEPROM programmed, DIG_STATUS=0x%02x\n", val);
        ret = 3;
        goto restore_A2_Table_and_out;
    }

    GET_BYTE_FROM_A2_TABLE(SAFE_MODE_STARTUP, val, 4, goto restore_A2_Table_and_out);
    if (val != 0x6A)
    {
        PRINTK("SAFE_MODE_STARTUP should be 0x6A when EEPROM programmed, SAFE_MODE_STARTUP=0x%02x\n", val);
        ret = 5;
        goto restore_A2_Table_and_out;
    }

    ret = fix_voltage_bias_calib();

restore_A2_Table_and_out:
    if (set_A2_table(smtc_bus_num, saved_table, NULL))
        ret = ret ?: 6;

    return ret;
}


/* This performs the reset state machine sequence.  o/w SMTC is only reset at power up */
static int smtc_reboot_callback(struct notifier_block *self, unsigned long val, void *data)
{
    int ret;

    if (smtc_bus_num == 0xFF) goto out;
    SET_BYTE_FROM_A2_TABLE(PWE_7B, 0x5D, 0, goto out);
    SET_BYTE_FROM_A2_TABLE(PWE_7C, 0x2C, 0, goto out);
    SET_BYTE_FROM_A2_TABLE(PWE_7D, 0x6A, 0, goto out);
    SET_BYTE_FROM_A2_TABLE(PWE_7E, 0xC9, 0, goto out);
    set_A2_table(smtc_bus_num, 0x8B, NULL);
out:
    return NOTIFY_DONE;
}

static struct notifier_block smtc_RebootNotifier = {
    .notifier_call = smtc_reboot_callback,
};


/* scan for chip once on each possible i2c bus.  if found check if EEPROM absent/uninitialized/initalized */
static int driver_thread(void *arg)
{
    u8 bus_num, val, found = 0, addr, saved_table, Type, buf[16];
    int i, rc, ret = 0;
    int len = strlen(CHIP_NAME);

    msleep(1000);
    printk("SMTC PMD driver thread started.\n");

    for (bus_num = 0; !found && bus_num < MAX_I2C_BUS_INCLUDE_MUX; bus_num++)
    {
        if (set_A2_table(bus_num, 0xFF, &saved_table))
            continue;

        memset(buf, 0, sizeof(buf));
        for (i = 0, addr = 0x80; i < len; i++, addr++)
        {
            if ((rc = bcmsfp_read_byte(bus_num, SFP_CLIENT_DIAG, addr, &buf[i])))
            {
                PRINTK("bcmsfp_read_byte failed I2C bus=%d, client=0x%x, addr=0x%02x, rc=%d\n", bus_num, SFP_CLIENT_DIAG, addr, rc);
                break;
            }
        }
        i2cdump_print16(buf);

        if ( (i == len) && !strncmp(buf, CHIP_NAME, len - 3) )
        {
            found++;
            smtc_bus_num = bus_num;
            PRINTK("SMTC GN28L96 found on I2C bus=%d, client=0x%x, Table FF\n", bus_num, SFP_CLIENT_DIAG);
            if (strncmp(buf, CHIP_NAME, len))
                PRINTK("unknown chip step\n");
        }
        else
            set_A2_table(bus_num, saved_table, NULL);
    }
    if (!found)
        goto out;

    register_reboot_notifier(&smtc_RebootNotifier);
#if KEEP_EEPROM
    if (set_A2_table(smtc_bus_num, 0x81, NULL))
    {
        ret = 2;
        goto restore_A2_Table_and_out;
    }
    GET_BYTE_FROM_A2_TABLE(EEPROM_CTRL_1, val, 3, goto restore_A2_Table_and_out);
    val |= EE_UPDATE_DIS;
    SET_BYTE_FROM_A2_TABLE(EEPROM_CTRL_1, val, 4, goto restore_A2_Table_and_out);
#endif

    if (set_A2_table(smtc_bus_num, 0x80, NULL))
    {
        ret = 5;
        goto restore_A2_Table_and_out;
    }
    GET_BYTE_FROM_A2_TABLE(BEN_CTRL, val, 8, goto restore_A2_Table_and_out);
    val &= ~0x60; /* disable TX_FORCE_BEN_ON, BEN_POL_INV */
    SET_BYTE_FROM_A2_TABLE(BEN_CTRL, val, 9, goto restore_A2_Table_and_out);

    GET_BYTE_FROM_A0(STATUS_CONTROL, val, 12, goto restore_A2_Table_and_out);
    val &= ~0x40; /* clear SOFT_TX_DISABLE */
    SET_BYTE_FROM_A0(STATUS_CONTROL, val, 13, goto restore_A2_Table_and_out);

    if ((rc = bcmsfp_read_byte(smtc_bus_num, SFP_CLIENT_EEPROM, TRANSCEIVER_IDENTIFIER_OFFSET, &Type)))
    {
        PRINTK("bcmsfp_read_byte failed I2C bus=%d, client=0x%x, addr=0x%02x, rc=%d\nAssuming EEPROMless\n", smtc_bus_num, SFP_CLIENT_EEPROM, TRANSCEIVER_IDENTIFIER_OFFSET, rc);
        smtc_board_mode = eeprom_less;
    }
    else if (Type == TRANSCEIVER_IDENTIFIER_SFF)
    {
        PRINTK("Transceiver type==%d.  Assuming EEPROM present and programmed\n", Type);
        smtc_board_mode = eeprom_programmed;
    }
    else
    {
        PRINTK("Transceiver type==%d.  Assuming EEPROM present and not programmed\n", Type);
        smtc_board_mode = eeprom_blank;
    }

    switch (smtc_board_mode)
    {
        case eeprom_less:
            rc = eeprom_less_init();
            break;
        case eeprom_programmed:
            rc = eeprom_programmed_init();
            break;
        case eeprom_blank:
            rc = eeprom_blank_init();
            break;
        default:
            printk("unexpected smtc_board_mode==%d\n", smtc_board_mode);
            break;
    }
    if (rc)
    {
        PRINTK("eeprom_*_init() failed, rc=%d\n", rc);
        return 1;
    }
    else
        PRINTK("eeprom_*_init() succeeded\n");

    if (set_A2_table(smtc_bus_num, 0x81, NULL))
    {
        ret = 10;
        goto restore_A2_Table_and_out;
    }

#if !KEEP_EEPROM
    GET_BYTE_FROM_A2_TABLE(EEPROM_CTRL_1, val, 3, goto restore_A2_Table_and_out);
    val |= EE_UPDATE_DIS;
    SET_BYTE_FROM_A2_TABLE(EEPROM_CTRL_1, val, 4, goto restore_A2_Table_and_out);
#endif

    /* FIXME: optionally check 3 checksums: A0:3F, A0:5F, A2:5F */
    /* if tx_wan_rate < 5Gbps, set TX_CTRL_3 accordingly */

    GET_BYTE_FROM_A2_TABLE(PON_CONTROL, val, 20, goto restore_A2_Table_and_out);
    val &= ~0b11;  /* MAX TX POWER*/
    SET_BYTE_FROM_A2_TABLE(PON_CONTROL, val, 21, goto restore_A2_Table_and_out);

    smtc_tx_en();

    SET_BYTE_FROM_A2_TABLE(SAFE_MODE_STARTUP, 0x6A, 18, goto restore_A2_Table_and_out);

restore_A2_Table_and_out:
    if (set_A2_table(smtc_bus_num, saved_table, NULL))
        ret = ret ?: 19;

    if (!ret && (smtc_board_mode != eeprom_programmed) )
    {
        msleep(1500);
        bcm_i2c_sfp_rescan(smtc_bus_num); /* eeprom absent or blank, need to rescan to detect sff */
    }

out:
    while(!kthread_should_stop())
        msleep(1000);
    return ret;
}


int __init detect_init(void)
{
    printk("\nSMTC PMD driver loaded.\n");
    if (IS_ERR(smtc_thread = kthread_run(driver_thread, &smtc_thread_arg, "SMTC driver thread")))
    {
        PRINTK("Thread creation failed, rc=0x%lx\n", PTR_ERR(smtc_thread));
        return -1;
    }
    return 0;
}
module_init(detect_init);

static void __exit detect_exit(void)
{
    printk("\nSMTC PMD driver unloading.\n");
    unregister_reboot_notifier(&smtc_RebootNotifier);
    if (!IS_ERR(smtc_thread) && smtc_thread)
        kthread_stop(smtc_thread);
    msleep(1500);
}
module_exit(detect_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("SMTC PMD device driver");
MODULE_LICENSE("GPL");
