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
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/of_platform.h>
#include <linux/gpio/consumer.h>
#include "board.h"
#include "bcm_pinmux.h"
#include "bcm_gpio.h"
#include "wan_drv.h"
#include "bcmsfp.h"
#include "bcm_smtc.h"

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
extern struct device *bcm_i2c_legacy_sfp_get_dev(int bus);

enum smtc_board_mode {
    unknown,
    eeprom_less,
    eeprom_blank,
    eeprom_programmed,
} smtc_board_mode = unknown;

struct task_struct *smtc_thread = NULL;
int  smtc_thread_arg = 0;
u8 smtc_bus_num = 0xFF;
struct i2c_adapter *smtc_bus_i2c = NULL;

enum
{
    GPIO_SMTC_TX_DIS,
    GPIO_MAX,
};

static const enum gpiod_flags gpio_flags[] =
{
    GPIOD_ASIS,
};

static const char *gpio_of_names[] =
{
    "smtc-tx-dis",
};

struct smtc_data
{
    struct device *dev;
    struct gpio_desc *gpio[GPIO_MAX];
    int tx_dis_state;
    struct i2c_adapter *i2c;
    struct mutex st_mutex;
} smtc_data;

static const struct of_device_id of_platform_smtc_table[] = {
    { .compatible = "brcm,smtc", },
    { /* end of list */ },
};

static void _smtc_module_gpio_set(struct smtc_data *psmtc, int gpio, int value)
{
    if (psmtc->gpio[gpio])
        gpiod_direction_output(psmtc->gpio[gpio], psmtc->tx_dis_state = value);
}

static void smtc_module_smtc_tx_dis_set(struct smtc_data *psmtc, int value)
{
    _smtc_module_gpio_set(psmtc, GPIO_SMTC_TX_DIS, value);
}

static void smtc_cleanup(void *data)
{
    return;
}

static struct smtc_data *smtc_data_init(struct device *dev)
{
    struct smtc_data *psmtc = &smtc_data;

    psmtc->dev = dev;
    mutex_init(&psmtc->st_mutex);

    return psmtc;
}

static int i2c_read(struct i2c_adapter *i2c, u8 addr, u8 offset, u8 *buf, int count)
{
    struct i2c_msg msg[2];
    int rc;

    /* First write the offset  */
    msg[0].addr = addr;
    msg[0].flags = 0;
    msg[0].len = 1;
    buf[0] = offset;
    msg[0].buf = buf;

    /* Now read the data */
    msg[1].addr = addr;
    msg[1].flags = I2C_M_RD;
    msg[1].len = count;
    msg[1].buf = buf;

    if ((rc = i2c_transfer(i2c, msg, ARRAY_SIZE(msg))) == ARRAY_SIZE(msg))
        return count;

    PRINTK("i2c_transfer(%d, msg, %zu) failed, rc=%d\n", i2c->nr, ARRAY_SIZE(msg), rc);
    return -1;
}

static inline int i2c_read_word(struct i2c_adapter *i2c, u8 addr, u8 offset, u16 *val)
{
    u16 reg;
    int rc;

    if ((rc = i2c_read(i2c, addr, offset, (u8 *)&reg, 2)) == 2)
    {
        *val = be16_to_cpu(reg);
        return 0;
    }

    return rc;
}

static inline int i2c_read_byte(struct i2c_adapter *i2c, u8 addr, u8 offset, u8 *val)
{
    int rc;

    rc = i2c_read(i2c, addr, offset, val, 1);
    return (rc == 1) ? 0 : rc;
}


static int i2c_write(struct i2c_adapter *i2c, u8 addr, u8 offset, const u8 *buf, int count)
{
    u8 _buf[count + 1];
    int rc;
    struct i2c_msg msg = {
        .addr = addr,
        .flags = 0,
        .len = count + 1,
        .buf = _buf,
    };

    _buf[0] = offset;
    memcpy(&_buf[1], buf, count);

    if ((rc = i2c_transfer(i2c, &msg, 1)) == 1)
        return count;

    PRINTK("i2c_transfer(i2c, msg, %d) failed, rc=%d\n", 1, rc);
    return -1;
}

static inline int i2c_write_word(struct i2c_adapter *i2c, u8 addr, u8 offset, u16 val)
{
    int rc;

    val = cpu_to_be16(val);
    rc = i2c_write(i2c, addr, offset, (u8 *)&val, 2);
    return (rc == 2) ? 0 : rc;
}

static inline int i2c_write_byte(struct i2c_adapter *i2c, u8 addr, u8 offset, u8 val)
{
    int rc;

    rc = i2c_write(i2c, addr, offset, &val, 1);
    return (rc == 1) ? 0 : rc;
}

static inline u8 sfp_client2addr(u8 client)
{
    switch (client)
    {
        case SFP_CLIENT_EEPROM:
            return SFP_I2C_EEPROM_ADDR;

        case SFP_CLIENT_DIAG:
            return SFP_I2C_DIAG_ADDR;

        default:
            PRINTK("unexpected client=%d\n", client);
            return 0;
    }
}

#define bcmsfp_read_byte   _bcmsfp_read_byte
#define bcmsfp_write_byte  _bcmsfp_write_byte
#define bcmsfp_read_word   _bcmsfp_read_word
#define bcmsfp_write_word  _bcmsfp_write_word

static int _bcmsfp_read_byte(u8 bus, u8 client, u8 offset, u8 *var)
{
    u8 addr = sfp_client2addr(client);
    int rc;
    struct i2c_adapter *i2c;

    i2c = (bus != smtc_bus_num) ? i2c_get_adapter(bus) : smtc_bus_i2c;
    rc = i2c_read_byte(i2c, addr, offset, var);

    if (bus != smtc_bus_num)
        i2c_put_adapter(i2c);
    return rc;
}

static int _bcmsfp_write_byte(u8 bus, u8 client, u8 offset, u8 val)
{
    u8 addr = sfp_client2addr(client);
    int rc;
    struct i2c_adapter *i2c;

    i2c = (bus != smtc_bus_num) ? i2c_get_adapter(bus) : smtc_bus_i2c;
    rc = i2c_write_byte(i2c, addr, offset, val);

    if (bus != smtc_bus_num)
        i2c_put_adapter(i2c);
    return rc;
}

__attribute__((unused)) static int _bcmsfp_read_word(u8 bus, u8 client, u8 offset, u16 *var)
{
    u8 addr = sfp_client2addr(client);
    int rc;
    struct i2c_adapter *i2c;

    i2c = (bus != smtc_bus_num) ? i2c_get_adapter(bus) : smtc_bus_i2c;
    rc = i2c_read_word(i2c, addr, offset, var);

    if (bus != smtc_bus_num)
        i2c_put_adapter(i2c);
    return rc;
}

static int _bcmsfp_write_word(u8 bus, u8 client, u8 offset, u16 val)
{
    u8 addr = sfp_client2addr(client);
    int rc;
    struct i2c_adapter *i2c;

    i2c = (bus != smtc_bus_num) ? i2c_get_adapter(bus) : smtc_bus_i2c;
    rc = i2c_write_word(i2c, addr, offset, val);

    if (bus != smtc_bus_num)
        i2c_put_adapter(i2c);
    return rc;
}

static void smtc_tx_en(void)
{
    smtc_module_smtc_tx_dis_set(&smtc_data, 0);
}

static void smtc_tx_dis(void)
{
    smtc_module_smtc_tx_dis_set(&smtc_data, 1);
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
static int array_to_regs(const struct page_desc *page_desc_p, const u8 *eeprom_buf)
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

            if ((ret = bcmsfp_write_byte(smtc_bus_num, page_desc_p->client_num, i + page_desc_p->start_data_addr, eeprom_buf[i])))
            {
                PRINTK("Failed to write i2c value at bus_num=%d, client=0x%02x, addr 0x%02x, val=0x%02x, ret=%d\n", smtc_bus_num, page_desc_p->client_num, i + page_desc_p->start_data_addr, eeprom_buf[i], ret);
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
    int rc, ret = 0;
    static u8 latched_table = 0xAA;
    static int depth = 0;

    if (depth++ != 0)
    {
        PRINTK("Unexpected Error.  %s should not reenter, depth==%d\n", __FUNCTION__, depth);
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
            // no error message, as this would be printed at boot for each bus on non-SMTC boards
            ret = 1;
            goto out;
        }

        if ((rc = bcmsfp_write_byte(bus_num, SFP_CLIENT_DIAG, TABLE_SEL, new_table)))
        {
            PRINTK("Failed to write i2c value at bus_num=%d, addr 0x%02x, rc=%d\n", bus_num, TABLE_SEL, rc);
            // we assume that if the write failed, there is no need to restore the table number
            ret = 1;
            goto out;
        }

        msleep(10);
        latched_table = new_table;
    }

out:
    depth--;
    return ret;
}

/* dump functions yield output similar to i2cdump cli */
static void i2cdump_title(void)
{
    int i, j = 0;
    char line[5*16] = {};

    j += sprintf(&line[j], "%3s", "");
    for (i=0; i<16; i++)
        j += sprintf(&line[j], " %2x", i);
    j += sprintf(&line[j], "%4s", "");
    for (i=0; i<16; i++)
        j += sprintf(&line[j], "%x", i);
    printk("%s\n", line);
}

static void i2cdump_print16(int header, u8 *buf)
{
    int i, j = 0;
    char line[5*16] = {};

    j += sprintf(&line[j], "%02x:", header);
    for (i = 0; i < 16; i++)
    {
        j += sprintf(&line[j], " %02x", buf[i]);
    }
    j += sprintf(&line[j], "%4s", "");
    for (i = 0; i < 16; i++)
    {
        j += sprintf(&line[j], "%c", (isprint(buf[i]) && buf[i] >= 0x20 && buf[i] < 0x7F) ? buf[i] : '?');
    }
    printk("%s\n", line);
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
        for (i = 0; i < 16; i++)
        {
            addr = 16*j + i;
            if ((rc = bcmsfp_read_byte(bus_num, client_num, addr, &buf[i])))
            {
               PRINTK("bcmsfp_read_byte failed I2C bus=%d, client=0x%x, addr=0x%02x, rc=%d\n", bus_num, client_num, addr, rc);
               buf[i] = 'X';
            }
        }
        i2cdump_print16(16*j, buf);
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
        for (i = 0; i < 16; i++)
        {
            addr = 16*j + i;
            if ((rc = bcmsfp_read_byte(bus_num, client_num, addr, &buf[i])))
            {
               PRINTK("bcmsfp_read_byte failed I2C bus=%d, client=0x%x, addr=0x%02x, rc=%d\n", bus_num, client_num, addr, rc);
               buf[i] = 'X';
            }
        }
        i2cdump_print16(16*j, buf);
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
    if (array_to_regs(&page_descs[A2_Lower], eeprom_buf))
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
    if (array_to_regs(&page_descs[A2_Table_80], eeprom_buf))
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
    if (array_to_regs(&page_descs[A2_Table_81], eeprom_buf))
        return 17;

    PRINTK("saving %s\n", page_descs[A0_Upper].name);
    if (array_to_regs(&page_descs[A0_Upper], A0_Upper_file))
        return 19;

    PRINTK("saving %s\n", page_descs[A0_Lower].name);
    if (array_to_regs(&page_descs[A0_Lower], A0_Lower_file))
        return 18;

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
    if (array_to_regs(&page_descs[A2_Lower], A2_Lower_file))
        return 9;

    if (set_A2_table(smtc_bus_num, 0x80, NULL))
        return 10;
    PRINTK("saving %s\n", page_descs[A2_Table_80].name);
    if (array_to_regs(&page_descs[A2_Table_80], A2_Table_80_file))
        return 12;

    if (set_A2_table(smtc_bus_num, 0x81, NULL))
        return 11;
    PRINTK("saving %s\n", page_descs[A2_Table_81].name);
    if (array_to_regs(&page_descs[A2_Table_81], A2_Table_81_file))
        return 13;
#endif

    PRINTK("saving %s\n", page_descs[A0_Upper].name);
    if (array_to_regs(&page_descs[A0_Upper], A0_Upper_file))
        return 15;

    PRINTK("saving %s\n", page_descs[A0_Lower].name);
    if (array_to_regs(&page_descs[A0_Lower], A0_Lower_file))
        return 14;

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


/* check for chip once on i2c bus specified in DT.  if found check if EEPROM absent/uninitialized/initalized */
static int driver_thread(void *arg)
{
    u8 val, addr, saved_table, Type, buf[16] = {};
    int i, rc, ret = 0;
    const int len = strlen(CHIP_NAME);

    printk("SMTC PMD driver thread started.\n");

    if (set_A2_table(smtc_bus_num, 0xFF, &saved_table))
    {
        PRINTK("set_A2_table failed, bus=%d\n", smtc_bus_num);
        goto out;
    }

    for (i = 0, addr = 0x80; i < len; i++, addr++)
    {
        if ((rc = bcmsfp_read_byte(smtc_bus_num, SFP_CLIENT_DIAG, addr, &buf[i])))
        {
            PRINTK("bcmsfp_read_byte failed I2C bus=%d, client=0x%x, addr=0x%02x, rc=%d\n", smtc_bus_num, SFP_CLIENT_DIAG, addr, rc);
            goto out;
        }
    }
    i2cdump_print16(0, buf);

    if ( (i == len) && !strncmp(buf, CHIP_NAME, len - 3) )
    {
        PRINTK("SMTC GN28L96 found on I2C bus=%d, client=0x%x, Table FF\n", smtc_bus_num, SFP_CLIENT_DIAG);
        if (strncmp(buf, CHIP_NAME, len))
            PRINTK("unknown chip step\n");
    }
    else
    {
        PRINTK("SMTC GN28L96 NOT found on I2C bus=%d, client=0x%x, Table FF\n", smtc_bus_num, SFP_CLIENT_DIAG);
        set_A2_table(smtc_bus_num, saved_table, NULL);
        goto out;
    }

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
        ret = 23;
        goto restore_A2_Table_and_out;
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

    SET_BYTE_FROM_A2_TABLE(SAFE_MODE_STARTUP, 0x6A, 18, goto restore_A2_Table_and_out);

restore_A2_Table_and_out:
    if (set_A2_table(smtc_bus_num, saved_table, NULL))
        ret = ret ?: 19;

    // FINAL INIT!!!  no need to rescan, bcmsfp polling will notice Type change at 0x50, 0
    smtc_tx_en();
    SET_BYTE_FROM_A0(TRANSCEIVER_IDENTIFIER_OFFSET, TRANSCEIVER_IDENTIFIER_SFF, 22, goto out);

out:
    i2c_put_adapter(smtc_bus_i2c);
    while(!kthread_should_stop())
        msleep(1000);
    return ret;
}


/* This performs the reset state machine sequence.  o/w SMTC is only reset at power up */
static int smtc_reboot_callback(struct notifier_block *self, unsigned long val, void *data)
{
    int ret;

    if (smtc_bus_num != 0xFF)
    {
        smtc_tx_dis();
        udelay(15);             /* TX_DIS assert time */
        smtc_bus_i2c = i2c_get_adapter(smtc_bus_num);
        SET_BYTE_FROM_A2_TABLE(PWE_7B, 0x5D, 0, goto out);
        SET_BYTE_FROM_A2_TABLE(PWE_7C, 0x2C, 0, goto out);
        SET_BYTE_FROM_A2_TABLE(PWE_7D, 0x6A, 0, goto out);
        SET_BYTE_FROM_A2_TABLE(PWE_7E, 0xC9, 0, goto out);
        set_A2_table(smtc_bus_num, 0x8B, NULL);
out:
        i2c_put_adapter(smtc_bus_i2c);
    }
    return NOTIFY_DONE;
}

static struct notifier_block smtc_RebootNotifier = {
    .notifier_call = smtc_reboot_callback,
};


static int _probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct device_node *np = dev->of_node;
    struct smtc_data *psmtc;
    struct device_node *i2c_np;
    const struct of_device_id *of_id;
    int ret = 0, i;

    if (!np)
        return -ENODEV;
    psmtc = smtc_data_init(dev);

    platform_set_drvdata(pdev, psmtc);
    ret = devm_add_action(psmtc->dev, smtc_cleanup, psmtc);
    if (ret < 0)
        return ret;

    of_id = of_match_node(of_platform_smtc_table, np);
    if (WARN_ON(!of_id))
        return -EINVAL;

    mutex_lock(&psmtc->st_mutex);
    for (i = 0; i < GPIO_MAX; i++)
    {
        psmtc->gpio[i] = devm_gpiod_get_optional(dev, gpio_of_names[i], gpio_flags[i]);
        if (IS_ERR(psmtc->gpio[i]))
        {
            ret = PTR_ERR(psmtc->gpio[i]);
            goto label_unlock_return;
        }

        if (psmtc->gpio[i] && gpio_flags[i] == GPIOD_IN)
        {
            gpiod_direction_input(psmtc->gpio[i]);
        }
    }

    i2c_np = of_parse_phandle(np, "i2c-bus", 0);
    if (!i2c_np)
    {
        dev_err(psmtc->dev, "missing 'i2c-bus' property\n");
    }
    else 
    {
        psmtc->i2c = of_find_i2c_adapter_by_node(i2c_np);
    }
    of_node_put(i2c_np);
    if (!psmtc->i2c)
    {
        dev_err(psmtc->dev, "cannot get i2c_adapter\n");
        ret = -ENOENT;
        goto label_unlock_return;
    }
    else
    {
        smtc_bus_i2c = psmtc->i2c;
        smtc_bus_num = psmtc->i2c->nr;
    }

    register_reboot_notifier(&smtc_RebootNotifier);
    if (IS_ERR(smtc_thread = kthread_run(driver_thread, &smtc_thread_arg, "SMTC driver thread")))
    {
        ret = PTR_ERR(smtc_thread);
        PRINTK("Thread creation failed, rc=0x%x\n", ret);
        goto label_unlock_return;
    }

label_unlock_return:
    mutex_unlock(&psmtc->st_mutex);
    return ret;
}

struct platform_driver of_platform_smtc_driver = {
    .driver = {
        .name = "brcm_smtc",
        .of_match_table = of_platform_smtc_table,
    },
    .probe = _probe,
};

static int __init detect_init(void)
{
    int rc; 

    printk("\nSMTC PMD driver loaded.\n");
    rc = platform_driver_register(&of_platform_smtc_driver);
    PRINTK("platform_driver_register() rc=%d\n", rc);
    return rc;
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
