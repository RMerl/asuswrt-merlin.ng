/*
    pon_i2c.c - I2C client driver for PON transceiver
    Copyright (C) 2008 Broadcom Corp.
 
* <:copyright-BRCM:2012:DUAL/GPL:standard
* 
*    Copyright (c) 2012 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :>

*/

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/slab.h>  /* kzalloc() */
#include <linux/types.h>
#include <linux/bcm_log.h>
#include "pmd.h"
#include <bcmsfp_i2c.h>

#include <bcm_intr.h>
#include <linux/interrupt.h>
#include <board.h>
#include <bcm_map_part.h>

#ifndef MAX_I2C_BUS 
#define MAX_I2C_BUS          1
#endif

#ifdef PROCFS_HOOKS
#include <asm/uaccess.h> /*copy_from_user*/
#include <linux/proc_fs.h>
#define PROC_DIR_NAME "i2c_bcmsfp_%d"
#ifdef GPON_I2C_TEST
#define PROC_ENTRY_TEST  "bcmsfpTest"
#endif
#endif

/* file system */
enum fs_enum {PROC_FS, SYS_FS};

/* Size of client in bytes */
#define DATA_SIZE             256
#define DWORD_ALIGN           4
#define WORD_ALIGN            2

/* Each client has this additional data */
struct bcmsfp_data {
    struct i2c_client* client[MAX_CLIENT_NUM];
    int cnt;
#ifdef PROCFS_HOOKS
    struct proc_dir_entry *proc_dir;
#endif
#ifdef GPON_I2C_TEST
    struct proc_dir_entry *proc_test_entry;
#endif
};

static struct bcmsfp_data bcmsfp_client[MAX_I2C_BUS];
static int client_init = 0;

#ifdef PROCFS_HOOKS
static ssize_t bcmsfp_proc_read(struct file *filep, char __user *page, size_t count, loff_t *offset);
static ssize_t bcmsfp_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *off);

static struct file_operations bcmsfpProc_fops = {
    .owner  = THIS_MODULE,
    .read = bcmsfp_proc_read,
    .write = bcmsfp_proc_write
};
#endif

#ifdef GPON_I2C_TEST
static ssize_t bcmsfp_proc_test_read(struct file *f, char *buf, size_t count, loff_t *pos);
static ssize_t bcmsfp_proc_test_write(struct file *f, const char *buf, size_t count, loff_t *pos);

static struct file_operations bcmsfpTest_fops = {
    .owner  = THIS_MODULE,
    .read = bcmsfp_proc_test_read,
    .write = bcmsfp_proc_test_write
};
#endif

#ifdef SYSFS_HOOKS
static ssize_t bcmsfp_sys_read(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t bcmsfp_sys_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);

static DEVICE_ATTR(bcmsfp_access, S_IRUGO|S_IWUSR, bcmsfp_sys_read, bcmsfp_sys_write);
static struct attribute *bcmsfp_attributes[] = {
    &dev_attr_bcmsfp_access.attr,
    NULL
};
static const struct attribute_group bcmsfp_attr_group = {
    .attrs = bcmsfp_attributes,
};
#endif

static int bcmsfp_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int bcmsfp_remove(struct i2c_client *client);

/* Check if given offset is valid or not */
static inline int check_offset(u8 offset, u8 alignment)
{
    if (offset % alignment) {
        BCM_LOG_ERROR(BCM_LOG_ID_I2C, "Invalid offset %d. The offset should be"
                      " %d byte alligned \n", offset, alignment);
        return -1;
    }
    return 0;
}

static void init_client(void) {

    if( client_init == 0 ) {
        memset((void*)bcmsfp_client, 0x0, sizeof(bcmsfp_client));
        client_init = 1;
    }

    return;
}
static int get_client(u8 bus_num, u8 client_num, struct i2c_client **client)
{
    if( bus_num >= MAX_I2C_BUS ) {
        BCM_LOG_ERROR(BCM_LOG_ID_I2C, "Invalid bus number %d\n", bus_num);
        return -1;
    }

    if( client_num >= MAX_CLIENT_NUM ) {
        BCM_LOG_ERROR(BCM_LOG_ID_I2C, "Invalid client number %d\n", client_num);
        return -1;
    }
  
    *client = bcmsfp_client[bus_num].client[client_num];
  
    if(*client == NULL)
    {
        BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "bus %d client %d does not exist\n", bus_num, client_num);
        return -1;
    }
    
    return 0;
}

static int get_client_by_addr(u8 bus_num, u16 client_addr, struct i2c_client **client)
{
    int i;
    struct i2c_client* i2c_dev;

    if( bus_num >= MAX_I2C_BUS ) {
        BCM_LOG_ERROR(BCM_LOG_ID_I2C, "Invalid bus number %d\n", bus_num);
        return -1;
    }

    for( i = 0; i < MAX_CLIENT_NUM; i++ ) {
        i2c_dev = bcmsfp_client[bus_num].client[i];
        if( i2c_dev && i2c_dev->addr == client_addr )
            break;
    }
  
    if( i == MAX_CLIENT_NUM) {
        BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "bus %d client addr 0x%x does not exist\n", bus_num, client_addr);
        return -1;
    } else
        *client = i2c_dev;
    
    return 0;
}

static int get_client_num_by_addr(u8 bus_num, u16 client_addr, u8 *client_num)
{
    int i;
    struct i2c_client* i2c_dev;

    if( bus_num >= MAX_I2C_BUS ) {
        BCM_LOG_ERROR(BCM_LOG_ID_I2C, "Invalid bus number %d\n", bus_num);
        return -1;
    }

    for( i = 0; i < MAX_CLIENT_NUM; i++ ) {
        i2c_dev = bcmsfp_client[bus_num].client[i];
        if( i2c_dev && i2c_dev->addr == client_addr )
            break;
    }
  
    if( i == MAX_CLIENT_NUM) {
        BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "bus %d client addr 0x%x does not exist\n", bus_num, client_addr);
        return -1;
    } else
        *client_num = i;
    
    return 0;
}


static int add_client(struct i2c_client* client)
{
    u8 bus_num;
    int client_assigned = 1, i;

    if( (bus_num = client->adapter->nr) >= MAX_I2C_BUS )
        return -1;

    switch(client->addr)
    {
        /* pmd reg and iram client shares the same address of sfp eeprom and diag client */
        case SFP_I2C_EEPROM_ADDR:
            bcmsfp_client[bus_num].client[SFP_CLIENT_EEPROM] = client;
            break;
        case SFP_I2C_DIAG_ADDR:
            bcmsfp_client[bus_num].client[SFP_CLIENT_DIAG] = client;
            break;
        case SFP_I2C_PHY_ADDR:
            bcmsfp_client[bus_num].client[SFP_CLIENT_PHY] = client;
            break;
        case PMD_I2C_DRAM_ADDR:
            bcmsfp_client[bus_num].client[PMD_CLIENT_DRAM] = client;
            break;

        default:
            /* for non-preassigned address, find the first avaialble clinet after the known client */
            for( i = MAX_SFP_I2C_ADDR; i < MAX_CLIENT_NUM; i++ ) {
                if( bcmsfp_client[bus_num].client[i] == NULL ) { 
                    bcmsfp_client[bus_num].client[i] = client; 
                    BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "i2c client bus %d addr 0x%x assigned to client number %d\n", bus_num, client->addr, i);
                    break;
                }
            }
            if( i == MAX_CLIENT_NUM ) {
                client_assigned = 0;
                BCM_LOG_ERROR(BCM_LOG_ID_I2C, "Add i2c client bus %d addr 0x%x no client available\n", bus_num, client->addr);
            }
    }

    if( client_assigned ) {
        bcmsfp_client[bus_num].cnt++;
        return 0;
    } else
        return -1;
}

static int remove_client(struct i2c_client* client)
{
    u8 bus_num, client_num;
  
    if( (bus_num = client->adapter->nr) >= MAX_I2C_BUS )
        return -1;

    if( get_client_num_by_addr(bus_num, client->addr, &client_num) != 0 ) {
         BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "%s client bus %d addr 0x%x does not exist \n", __FUNCTION__, 
         bus_num, client->addr);
         return -1;
    }

    if( bcmsfp_client[bus_num].client[client_num] != client ) {
         BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "%s client %d does not match\n", __FUNCTION__, client_num);
         return -1;

    }

    bcmsfp_client[bus_num].client[client_num] = NULL;
    bcmsfp_client[bus_num].cnt--;
    return 0;
}

static int is_client_pmd(struct i2c_client *client)
{
    return strncmp(client->name, "pmd", 3) == 0 ? 1:0;
}

static int create_procsys_hooks(struct i2c_client *client)
{
    int err = 0;
    u8 bus;
#ifdef PROCFS_HOOKS
    char dir_name[16];
    struct proc_dir_entry *p, *q;
#endif

#ifdef SYSFS_HOOKS
    /* Register sysfs hooks */
    err = sysfs_create_group(&client->dev.kobj, &bcmsfp_attr_group);
    if (err)
        return err;
#endif

#ifdef PROCFS_HOOKS
    bus = client->adapter->nr;
    q = bcmsfp_client[bus].proc_dir;
    if( q == NULL ) {
        snprintf(dir_name, 16, PROC_DIR_NAME, bus); 
        if ( (q = proc_mkdir(dir_name, NULL))  == NULL) {
            BCM_LOG_ERROR(BCM_LOG_ID_I2C, "bcmsfp_i2c: unable to create proc dir %s\n", dir_name);
            err = -ENOMEM;
#ifdef SYSFS_HOOKS
            sysfs_remove_group(&client->dev.kobj, &bcmsfp_attr_group);
#endif
            return err;
        } else {
            bcmsfp_client[bus].proc_dir = q;
        }
    }

    /* do not add proc node for generic i2c device */
    if( strcmp(client->name, "bcmsfp") != 0 ) {
        p = proc_create_data(client->name, 0644, q, &bcmsfpProc_fops, client);
        if (!p) {
            BCM_LOG_ERROR(BCM_LOG_ID_I2C, "bcmsfp_i2c: unable to create proc entry for %s\n", client->name);
            err = -EIO;
#ifdef SYSFS_HOOKS
            sysfs_remove_group(&client->dev.kobj, &bcmsfp_attr_group);
#endif
            return err;
        }
    }

#ifdef GPON_I2C_TEST
    /* Create only once */
    if( bcmsfp_client[bus].proc_test_entry == NULL )
        bcmsfp_client[bus].proc_test_entry = proc_create_data(PROC_ENTRY_TEST, 0644, q, &bcmsfpTest_fops, client);
#endif
#endif
    return 0;
}

static int remove_procsys_hooks(struct i2c_client *client)
{
    int err = 0;
    u8 bus;
#ifdef PROCFS_HOOKS
    char dir_name[16];
    struct proc_dir_entry *q;
#endif

#ifdef SYSFS_HOOKS
    sysfs_remove_group(&client->dev.kobj, &bcmsfp_attr_group);
#endif

#ifdef PROCFS_HOOKS
    bus = client->adapter->nr;
    if( (q = bcmsfp_client[bus].proc_dir) == NULL )
        return -1;
    if( strcmp(client->name, "bcmsfp") != 0 )
        remove_proc_entry(client->name, q);
#endif

    if( bcmsfp_client[bus].cnt == 0 ) {
#ifdef GPON_I2C_TEST
        if ( bcmsfp_client[bus].proc_test_entry != NULL) {
            remove_proc_entry(PROC_ENTRY_TEST, q);
            bcmsfp_client[bus].proc_test_entry = NULL;
        }
#endif
        snprintf(dir_name, 16, PROC_DIR_NAME, bus); 
        remove_proc_entry(dir_name, NULL);
        bcmsfp_client[bus].proc_dir = NULL;
    }

    return err;
}

/****************************************************************************/
/* generic_i2c_access: Provides a way to use i2c bus algorithm driver to    */
/*  access any I2C device on the bus                                        */
/* Inputs: i2c_addr = 7-bit I2C address; offset = 8-bit offset; length =    */
/*  length (limited to 4); val = value to be written; set = indicates write */
/* Returns: None                                                            */
/****************************************************************************/
static void generic_i2c_access(u8 bus_num, u8 i2c_addr, u8 offset, u8 length, 
                               int val, u8 set)
{
    struct i2c_msg msg[2];
    char buf[5];
    int i;
    struct i2c_client *client = NULL;

    if (length > 4)
    {
        BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Read limited to 4 bytes\n");
        return;
    }

    if( get_client_by_addr(bus_num, i2c_addr, &client) != 0 )
    {
        /* try the eeprom address as they all share the same bus adapter */
        if( get_client_by_addr(bus_num, SFP_I2C_EEPROM_ADDR, &client) != 0 )
        {
            BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Invalid client\n");
            return;
        }
    }

    /* First write the offset  */
    msg[0].addr = msg[1].addr = i2c_addr;
    msg[0].flags = msg[1].flags = 0;

    /* if set = 1, do i2c write; otheriwse do i2c read */
    if (set) {
        msg[0].len = length + 1;
        buf[0] = offset;
        /* On the I2C bus, LS Byte should go first */
#ifndef __LITTLE_ENDIAN 
        val = swab32(val);
#endif
        memcpy(&buf[1], (char*)&val, length);
        msg[0].buf = buf;
        if(i2c_transfer(client->adapter, msg, 1) != 1) {
            BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "I2C Write Failed \n");
        }
    } else {
        /* write message */
        msg[0].len = 1;
        buf[0] = offset;
        msg[0].buf = buf;
        /* read message */
        msg[1].flags |= I2C_M_RD;
        msg[1].len = length;
        msg[1].buf = buf;

        /* On I2C bus, we receive LS byte first. So swap bytes as necessary */
        if(i2c_transfer(client->adapter, msg, 2) == 2)
        {
            for (i=0; i < length; i++) {
                printk("0x%2x = 0x%2x \n", offset + i, buf[i] & 0xFF);
            }
            printk("\n");
        } else {
            BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "I2C Read Failed \n");
        }
    }
}

/****************************************************************************/
/* Writes count number of bytes from buf on to the I2C bus                  */
/* Returns:                                                                 */
/*   number of bytes written on success, negative value on failure.         */
/* Notes: 1. The LS byte should follow the offset                           */
/* Design Notes: The ponPhy takes the first byte after the chip address     */
/*  as offset. The BCM6816 can only send/receive upto 8 or 32 bytes         */
/*  depending on I2C_CTLHI_REG.DATA_REG_SIZE configuration in one           */
/*  transaction without using the I2C_IIC_ENABLE NO_STOP functionality.     */
/*  The 6816 algorithm driver currently splits a given transaction larger   */
/*  than DATA_REG_SIZE into multiple transactions. This function is         */   
/*  expected to be used very rarely and hence a simple approach is          */
/*  taken whereby this function limits the count to 32 (Note that the 6816  */
/*  I2C_CTLHI_REG.DATA_REG_SIZE is hard coded in 6816 algorithm driver for  */
/*  32B. This means, we can only write upto 31 bytes using this function.   */
/****************************************************************************/
ssize_t bcmsfp_write(u8 bus_num, u8 client_num, char *buf, size_t count)
{
    struct i2c_client *client = NULL;
    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if(count > MAX_TRANSACTION_SIZE)
    {
        BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "count > %d is not yet supported \n", MAX_TRANSACTION_SIZE);
        return -1;
    }
  
    if(get_client(bus_num, client_num, &client))
    {
        return -1;
    }
    
    return i2c_master_send(client, buf, count);
}
EXPORT_SYMBOL(bcmsfp_write);

/****************************************************************************/
/* Read: Reads count number of bytes from sfp                               */
/* Returns:                                                                 */
/*   number of bytes read on success, negative value on failure.            */
/* Notes: 1. The offset should be provided in buf[0]                        */
/*        2. The count is limited to 32.                                    */
/*        3. The bcmsfp with the serial EEPROM protocol requires the offset*/
/*        be written before reading the data on every I2C transaction       */
/****************************************************************************/
ssize_t bcmsfp_read(u8 bus_num, u8 client_num, char *buf, size_t count)
{
    struct i2c_msg msg[2];
    struct i2c_client *client = NULL;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if(count > MAX_TRANSACTION_SIZE)
    {
        BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "count > %d is not yet supported \n", MAX_TRANSACTION_SIZE);
        return -1;
    }

    if(get_client(bus_num, client_num, &client))
    {
        return -1;
    }

    /* First write the offset  */
    msg[0].addr = msg[1].addr = client->addr;
    msg[0].flags = msg[1].flags = client->flags & I2C_M_TEN;

    if (is_client_pmd(client))
        msg[0].len = PMD_I2C_HEADER;
    else
        msg[0].len = 1;

    msg[0].buf = buf;

    /* Now read the data */
    msg[1].flags |= I2C_M_RD;
    msg[1].len = count;
    msg[1].buf = buf;

    /* On I2C bus, we receive LS byte first. So swap bytes as necessary */
    if(i2c_transfer(client->adapter, msg, 2) == 2)
    {
        return count;
    }

    return -1;
}
EXPORT_SYMBOL(bcmsfp_read);

/****************************************************************************/
/* Write Register: Writes the val into sfp register                         */
/* Returns:                                                                 */
/*   0 on success, negative value on failure.                               */
/* Notes: 1. The offset should be DWORD aligned                             */
/*        2. SFP diagnotic interface in big endian mode. See SFF-8472       */
/****************************************************************************/
int bcmsfp_write_reg(u8 bus_num, u8 client_num, u8 offset, u32 val)
{
    char buf[5];
    struct i2c_client *client = NULL;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if(check_offset(offset, DWORD_ALIGN))
    {
        return -1;
    }

    if(get_client(bus_num, client_num, &client))
    {
        return -1;
    }

    /* Set the buf[0] to be the offset for write operation */
    buf[0] = offset;
    val = cpu_to_be32(val);
    memcpy(&buf[1], (char*)&val, 4);
    if (i2c_master_send(client, buf, 5) == 5)
    {
        return 0;
    }
    return -1;
}
EXPORT_SYMBOL(bcmsfp_write_reg);

/****************************************************************************/
/* Read Register: Read the sfp register                                     */
/* Returns:                                                                 */
/*   0 on success, negative value on failure.                               */
/* Notes: 1. The offset should be DWORD aligned                             */
/*        2. SFP diagnotic interface in big endian mode. See SFF-8472       */
/****************************************************************************/
int bcmsfp_read_reg(u8 bus_num, u8 client_num, u8 offset, u32* val)
{
    struct i2c_msg msg[2];
    u32 reg;
    struct i2c_client *client = NULL;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if(check_offset(offset, DWORD_ALIGN))
    {
        return -1;
    }

    if(get_client(bus_num, client_num, &client))
    {
            return -1;
    }

    msg[0].addr = msg[1].addr = client->addr;
    msg[0].flags = msg[1].flags = client->flags & I2C_M_TEN;

    msg[0].len = 1;
    msg[0].buf = (char *)&offset;

    msg[1].flags |= I2C_M_RD;
    msg[1].len = 4;
    msg[1].buf = (char *)&reg;

    if(i2c_transfer(client->adapter, msg, 2) == 2)
    {
        *val = be32_to_cpu(reg);
        return 0;
    }

    return -1;
}
EXPORT_SYMBOL(bcmsfp_read_reg);

/****************************************************************************/
/* Write Word: Writes the val into the word offset                          */ 
/* Returns:                                                                 */
/*   0 on success, negative value on failure.                               */
/* Notes: 1. The offset should be WORD aligned                              */
/*        2. SFP diagnotic interface in big endian mode. See SFF-8472       */
/****************************************************************************/
int bcmsfp_write_word(u8 bus_num, u8 client_num, u8 offset, u16 val)
{
    char buf[3];
    struct i2c_client *client = NULL;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if(get_client(bus_num, client_num, &client))
    {
        return -1;
    }

    if(client->addr != SFP_I2C_PHY_ADDR) /* SFP use reg number for word access */
    {
        if( check_offset(offset, WORD_ALIGN))
            return -1;
    }


    /* The offset to be written should be the first byte in the I2C write */
    buf[0] = offset;
    val = cpu_to_be16(val);
    memcpy(&buf[1], (char*)&val, 2);
    if (i2c_master_send(client, buf, 3) == 3)
    {
        return 0;
    }
    return -1;
}
EXPORT_SYMBOL(bcmsfp_write_word);

/****************************************************************************/
/* Read Word: Reads the LSB 2 bytes of Register                             */ 
/* Returns:                                                                 */
/*   0 on success, negative value on failure.                               */
/* Notes: 1. The offset should be WORD aligned                              */
/*        2. SFP diagnotic interface in big endian mode. See SFF-8472       */
/****************************************************************************/
int bcmsfp_read_word(u8 bus_num, u8 client_num, u8 offset, u16* val)
{
    struct i2c_msg msg[2];
    u16 reg;
    struct i2c_client *client = NULL;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if(get_client(bus_num, client_num, &client))
    {
        return -1;
    }

    if(client->addr != SFP_I2C_PHY_ADDR) /* SFP use reg number for word access */
    {
        if(check_offset(offset, WORD_ALIGN))
        {
            return -1;
	}
    }

    msg[0].addr = msg[1].addr = client->addr;
    msg[0].flags = msg[1].flags = client->flags & I2C_M_TEN;

    msg[0].len = 1;
    msg[0].buf = (char *)&offset;

    msg[1].flags |= I2C_M_RD;
    msg[1].len = 2;
    msg[1].buf = (char *)&reg;

    if(i2c_transfer(client->adapter, msg, 2) == 2)
    { 
        *val = be16_to_cpu(reg);
        return 0;
    }

    return -1;
}
EXPORT_SYMBOL(bcmsfp_read_word);

/****************************************************************************/
/* Write Byte: Writes the val into LS Byte of Register                      */ 
/* Returns:                                                                 */
/*   0 on success, negative value on failure.                               */
/****************************************************************************/
int bcmsfp_write_byte(u8 bus_num, u8 client_num, u8 offset, u8 val)
{
    char buf[2];
    struct i2c_client *client = NULL;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if(get_client(bus_num, client_num, &client))
    {
        return -1;
    }

    /* BCM3450 requires the offset to be the register number */
    buf[0] = offset;
    buf[1] = val;
    if (i2c_master_send(client, buf, 2) == 2)
    {
        return 0;
    }
    return -1;
}
EXPORT_SYMBOL(bcmsfp_write_byte);

/****************************************************************************/
/* Read Byte: Reads the LS Byte of Register                                 */ 
/* Returns:                                                                 */
/*   0 on success, negative value on failure.                               */
/****************************************************************************/
int bcmsfp_read_byte(u8 bus_num, u8 client_num, u8 offset, u8* val)
{
    struct i2c_msg msg[2];
    u8 reg;
    struct i2c_client *client = NULL;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if(get_client(bus_num, client_num, &client))
    {
        return -1;
    }

    msg[0].addr = msg[1].addr = client->addr;
    msg[0].flags = msg[1].flags = client->flags & I2C_M_TEN;

    msg[0].len = 1;
    msg[0].buf = (char *)&offset;

    msg[1].flags |= I2C_M_RD;
    msg[1].len = 1;
    msg[1].buf = (char *)&reg;

    if(i2c_transfer(client->adapter, msg, 2) == 2)
    {
        *val = reg;
        return 0;
    }

    return -1;
}
EXPORT_SYMBOL(bcmsfp_read_byte);

#if defined(SYSFS_HOOKS) || defined(PROCFS_HOOKS)
#ifdef GPON_I2C_TEST
static int client_num = 0;
static int bus_num = 0;

int bcmsfp_dump_eeprom(void)
{
    int i;
    uint8_t val, client;

    if( get_client_num_by_addr(bus_num, SFP_I2C_EEPROM_ADDR, &client) != 0 )
    {
        BCM_LOG_ERROR(BCM_LOG_ID_I2C, "EEPROM client does not exist for bus %d\n", bus_num);
        return -1;
    }

    printk("dump sfp eeprom on bus %d:", bus_num);
    for( i=0; i < 256; i++ ) {
        if (bcmsfp_read_byte(bus_num, client, i, (u8*)&val) != 0) {
            BCM_LOG_ERROR(BCM_LOG_ID_I2C, "\nFailed to read EEPROM content at offset %d for bus %d\n", i, bus_num);
            return -1;
        }
	if( i % 16 == 0 )
            printk("\noffset %03d: ", i);
        printk("%02x ", val);
    }
    printk("\n");

    return 0;
}

/* Calls the appropriate function based on user command */
static int exec_command(const char *buf, size_t count, int fs_type)
{
#define MAX_ARGS 4
#define MAX_ARG_SIZE 32
    int i, argc = 0, val = 0;
    char cmd;
    u8 offset, i2c_addr, length, set = 0;
    char arg[MAX_ARGS][MAX_ARG_SIZE];
#ifdef PROCFS_HOOKS
#define LOG_WR_KBUF_SIZE 128
    char kbuf[LOG_WR_KBUF_SIZE];

    if(fs_type == PROC_FS)
    {
        if ((count > LOG_WR_KBUF_SIZE-1) || 
            (copy_from_user(kbuf, buf, count) != 0))
            return -EFAULT;
        kbuf[count]=0;
        argc = sscanf(kbuf, "%c %s %s %s %s", &cmd, arg[0], arg[1], 
                      arg[2], arg[3]);
    }
#endif

#ifdef SYSFS_HOOKS
    if(fs_type == SYS_FS)
        argc = sscanf(buf, "%c %s %s %s %s", &cmd, arg[0], arg[1], 
                      arg[2], arg[3]);
#endif

    if (argc < 1) {
        return -EFAULT;
    }

    for (i=0; i<MAX_ARGS; ++i) {
        arg[i][MAX_ARG_SIZE-1] = '\0';
    }

    offset = (u8) 0;
    if (argc >= 2)
    {
        offset = (u8) simple_strtoul(arg[0], NULL, 0);
    }

    if (argc == 3)
    {
        val = (int) simple_strtoul(arg[1], NULL, 0);
    }

    switch (cmd) {
 
       case 'a':
        if (argc >= 4) {
            i2c_addr = (u8) simple_strtoul(arg[0], NULL, 0);
            offset = (u8) simple_strtoul(arg[1], NULL, 0);
            length = (u8) simple_strtoul(arg[2], NULL, 0);
            BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "I2C Access: bus = %d i2c_addr = 0x%x, offset"
                          " = 0x%x, len = %d \n", bus_num, i2c_addr, offset, length);
            if (i2c_addr > 127 || length > 4) {
                BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Invalid I2C addr or len \n");
                return -EFAULT;
            }
            val = 0;
            if (argc > 4) {
                val = (int) simple_strtoul(arg[3], NULL, 0);
                set = 1;
            }
            generic_i2c_access(bus_num, i2c_addr, offset, length, val, set);
        } else {
            BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Need at-least 3 arguments \n");
            return -EFAULT;
        }
        break;

    case 'b':
        if (argc == 3) {
            BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Write Byte: offset = 0x%x, " 
                          "val = 0x%x \n", offset, val);
            if (bcmsfp_write_byte(bus_num, client_num, offset, (u8)val) < 0) {
                BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Write Failed \n"); 
            } else {
                BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Write Successful \n"); 
            }
        }
        else {
            if (bcmsfp_read_byte(bus_num, client_num, offset, (u8*)&val) < 0) {
                BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Read Failed \n"); 
            } else {
                 BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Read Byte: offset = 0x%x, " 
                                "val = 0x%x \n", offset, val);
            }
        }
        break;

    case 'w':
        if (argc == 3) {
            BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Write Word: offset = 0x%x, " 
                              "val = 0x%x \n", offset, val);
            if (bcmsfp_write_word(bus_num, client_num, offset, (u16)val) < 0) {
                BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Write Failed \n"); 
            } else {
                BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Write Successful \n"); 
            }
        }
        else {
            if(bcmsfp_read_word(bus_num, client_num, offset, (u16*)&val) < 0) {
                BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Read Failed \n"); 
            } else {
                BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Read Word: offset = 0x%x, " 
                               "val = 0x%x \n", offset, val);
            }
        }
        break;

    case 'd':    
        if (argc == 3) {
            BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Write Register: offset = 0x%x, " 
                          "val = 0x%x \n", offset, val);
            if (bcmsfp_write_reg(bus_num, client_num, offset, val) < 0) {
                BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Write Failed \n"); 
            } else {
                BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Write Successful \n"); 
            }
        }
        else {
            if (bcmsfp_read_reg(bus_num, client_num, offset, &val) < 0) {
                BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Read Failed \n"); 
            } else {
                BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Read Register: offset = 0x%x,"
                               " val = 0x%x \n", offset, val);
            }
        }
        break;

    case 'c':
        if( get_client_num_by_addr(bus_num, offset, (u8*)&client_num) == 0 ) {
            BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Select client %d at bus %d"
                           " addr = 0x%x \n", client_num, bus_num, offset);

        } else {
            BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Invalid addr = 0x%x in bus %d\n", 
                           offset, bus_num); 
        }
        break;

    case 'e':
        bcmsfp_dump_eeprom();
        break;

    default:
        BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Invalid command. \n Valid commands: \n" 
                       "  Change I2C Addr b/w 0x50 and 0x51: c addr \n" 
                       "  Write Reg:       d offset val \n" 
                       "  Read Reg:        d offset \n" 
                       "  Write Word:      w offset val \n" 
                       "  Read Word:       w offset \n" 
                       "  Write Byte:      b offset val \n" 
                       "  Read Byte:       b offset \n" 
                       "  Dump EEPROM:     e\n" 
                       "  Generic I2C access: a <i2c_addr(7-bit)>" 
                       " <offset> <length(1-4)> [value] \n" 
                       );
        break;
    }
    return count;
}
#endif
#endif

#ifdef PROCFS_HOOKS
#ifdef GPON_I2C_TEST
/* Read Function of PROCFS attribute "bcmsfpTest" */
static ssize_t bcmsfp_proc_test_read(struct file *f, char *buf, size_t count,
                               loff_t *pos) 
{
    BCM_LOG_NOTICE(BCM_LOG_ID_I2C, " Usage: echo command > "
                   " /proc/i2c-bcmsfp-<bus_num>/bcmsfpTest \n");
    BCM_LOG_NOTICE(BCM_LOG_ID_I2C, " supported commands: \n" 
                   "  Change I2C Addr b/w 0x50 and 0x51: c addr \n" 
                   "  Write Reg:       d offset val \n" 
                   "  Read Reg:        d offset \n" 
                   "  Write Word:      w offset val \n" 
                   "  Read Word:       w offset \n" 
                   "  Write Byte:      b offset val \n" 
                   "  Read Byte:       b offset \n" 
                   "  Dump EEPROM:     e\n"
                   "  Generic I2C access: a <i2c_addr(7-bit)>" 
                   " <offset> <length(1-4)> [value] \n" 
                   );
    return 0;
}

/* Write Function of PROCFS attribute "bcmsfpTest" */
static ssize_t bcmsfp_proc_test_write(struct file *f, const char *buf,
                                       size_t count, loff_t *pos)
{
    struct i2c_client *client = (struct i2c_client *)(PDE_DATA(file_inode(f)));
    bus_num = client->adapter->nr;
    return exec_command(buf, count, PROC_FS);
}
#endif

#define PON_PHY_EEPROM_SIZE  256
/* Read Function of PROCFS attribute "bcmsfp_eeprom" */
static ssize_t bcmsfp_proc_read(struct file *filep, char __user *page, size_t count, loff_t *offset)
{
    int max_offset, ret_val = 0;
    u8 client_num = 0, bus_num = 0;
    struct i2c_client *client = (struct i2c_client *)(PDE_DATA(file_inode(filep)));
    int off = (int)(*offset);
    char kbuf[MAX_TRANSACTION_SIZE];

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "The offset is %d; the count is %d \n", 
                  (int)off, (int)count);

    /* Verify that max_offset is below the max_eeprom_size (256 Bytes)*/
    max_offset = (int) (off + count);
    if (max_offset > PON_PHY_EEPROM_SIZE) {
        BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "offset + count must be less than "
                       "Max EEPROM Size of 256\n");
        return -1;
    }
 
    /* Select the eeprom of the 2 eeproms inside bcmsfp */
    bus_num = client->adapter->nr;
    if( get_client_num_by_addr(bus_num, client->addr, &client_num) != 0 )
        return -1;

    /* Set the page[0] to eeprom offset */
    kbuf[0] = (u8)off;
    ret_val = bcmsfp_read(bus_num, client_num, kbuf, count);

    if (ret_val > 0)
        copy_to_user(page, kbuf, ret_val);

    /* return zero here to indicate EOF */
    return ret_val;
}

/* Write Function of PROCFS attribute "bcmsfp_eepromX" */
static ssize_t bcmsfp_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *off)
{
    int max_offset;
    u8 client_num = 0, bus_num = 0;
    char *kbuf;
    int rc;
    int offset = (int)(*off);
    struct i2c_client *client = (struct i2c_client *)(PDE_DATA(file_inode(file)));

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "The offset is %d; the count is %ld \n", 
                  offset, (long)count);

    /* Verify that count is less than 31 bytes */
    if ((count+1) > MAX_TRANSACTION_SIZE)
    {
        BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Writing more than 31 Bytes is not"
                       "yet supported \n");
        return -1;
    }

    kbuf = kzalloc(count, GFP_KERNEL);
    if (!kbuf)
    {
        BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Couldn't allocated kbuf \n");
        return -1;
    }
 
    /* Verify that max_offset is below the max_eeprom_size (256 Bytes)*/
    max_offset = (int) (offset + count);
    if (max_offset > PON_PHY_EEPROM_SIZE)
    {
        BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "offset + count must be less than "
                       "Max EEPROM Size of 256\n");
        rc = -1;
        goto exit;
    }  
   
    /* Select the eeprom of the 2 eeproms inside bcmsfp */
    bus_num = client->adapter->nr;
    if( get_client_num_by_addr(bus_num, client->addr, &client_num) != 0 ) {
        rc =  -1;
        goto exit;
    }

    kbuf[0] = (u8)offset;
    copy_from_user(&kbuf[1], buffer, count);
    /* Return the number of bytes written (exclude the address byte added
       at kbuf[0] */
    rc = bcmsfp_write(bus_num, client_num, kbuf, count+1) - 1;
exit:
    kfree(kbuf);
    return rc;
}
#endif

#ifdef SYSFS_HOOKS
/* Read Function of SYSFS attribute */
static ssize_t bcmsfp_sys_read(struct device *dev, struct device_attribute *attr,
                          char *buf)
{
    return snprintf(buf, PAGE_SIZE, "The bcmsfp access read attribute \n");
}

/* Write Function of SYSFS attribute */
static ssize_t bcmsfp_sys_write(struct device *dev, struct device_attribute *attr,
                           const char *buf, size_t count)
{
    return exec_command(buf, count, SYS_FS);
}
#endif

static int bcmsfp_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int err = 0;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    init_client();

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
        return -1;

    if( add_client(client) != 0 )
        return -1;

    err = create_procsys_hooks(client);

    return err;
   
}

static int bcmsfp_remove(struct i2c_client *client)
{
    int err = 0;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);
    

    if( (err = remove_client(client)) != 0 )
        return err;
    err =  remove_procsys_hooks(client);

    return err;
}

static const struct i2c_device_id bcmsfp_i2c_id_table[] = {
    { "sfp_eeprom", 0 },
    { "sfp_diag", 0 },
    { "sfp_phy", 0 },
    { "pmd_reg", 0 },
    { "pmd_iram", 0 },
    { "pmd_dram", 0 },
    { "bcmsfp", 0 },  /* generic i2c id for any other client */
    { },
};

MODULE_DEVICE_TABLE(i2c, bcmsfp_i2c_id_table);

static struct i2c_driver bcmsfp_i2c_driver = {
    .probe           = bcmsfp_probe,
    .remove          = bcmsfp_remove,
    .driver          = {
    .name            = "bcmsfp-i2c",
    },
    .id_table = bcmsfp_i2c_id_table,
};
module_i2c_driver(bcmsfp_i2c_driver);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom SPF I2C driver");
MODULE_LICENSE("GPL");

