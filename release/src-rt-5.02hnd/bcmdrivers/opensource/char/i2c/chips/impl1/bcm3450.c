/*
    bcm3450.c - BCM3450 I2C client driver

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
#include "bcm3450.h"

#ifdef PROCFS_HOOKS
#include <asm/uaccess.h> /*copy_from_user*/
#include <linux/proc_fs.h>
#define PROC_ENTRY_NAME1 "b3450_reg"
#ifdef MOCA_I2C_TEST
#define PROC_ENTRY_NAME2 "b3450reg"
#endif
#endif

/* I2C client chip addresses */
#define BCM3450_I2C_ADDR 0x70

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0)
/* Addresses to scan */
static unsigned short normal_i2c[] = {BCM3450_I2C_ADDR, I2C_CLIENT_END};
#endif

/* file system */
enum fs_enum {PROC_FS, SYS_FS};

/* Size of client in bytes */
#define DATA_SIZE		      256
#define DWORD_ALIGN	          4
#define WORD_ALIGN            2
#define MAX_TRANSACTION_SIZE  8
#define MAX_REG_OFFSET        0x1C

/* Insmod parameters */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0)
I2C_CLIENT_INSMOD_1(bcm3450);
#endif

/* Each client has this additional data */
struct bcm3450_data {
    struct i2c_client client;
};

static struct bcm3450_data *pclient_data; 

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0)
static int bcm3450_attach_adapter(struct i2c_adapter *adapter);
static int bcm3450_detect(struct i2c_adapter *adapter, int address, int kind);
static int bcm3450_detach_client(struct i2c_client *client);
#else
static int bcm3450_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int bcm3450_remove(struct i2c_client *client);
#endif

/* Check if given 3450 register offset is valid or not */
static inline int check_offset(u8 offset, u8 alignment)
{
    if (offset > MAX_REG_OFFSET) 
    {
        BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Invalid offset. It should be less than "
                      "%X \n", MAX_REG_OFFSET);
        return -EINVAL;
    }
    else if (offset % alignment) 
    {
        BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Invalid offset %d. The offset should be "
                      "%d byte alligned \n", offset, alignment);
        return -EINVAL;
    }
    return 0;
}

/****************************************************************************/
/* Write BCM3450: Writes count number of bytes from buf on to the I2C bus   */
/* Returns:                                                                 */
/*   number of bytes written on success, negative value on failure.         */
/* Notes: 1. The buf[0] should be a DWORD aligned offset where write starts */
/*        2. The LS byte should follow the offset                           */
/* Design Notes: The BCM3450 takes the first byte after the chip address    */
/*  as offset. The BCM6816 can only send/receive upto 8 or 32 bytes         */
/*  depending on I2C_CTLHI_REG.DATA_REG_SIZE configuration in one           */
/*  transaction without using the I2C_IIC_ENABLE NO_STOP functionality.     */
/*  The 6816 algorithm driver currently splits a given transaction larger   */
/*  than DATA_REG_SIZE into multiple transactions. This function is not     */   
/*  expected to be used very rarely and hence a simple approach is          */
/*  taken whereby this function limits the count to 8. This means, we can   */
/*  only write upto 7 bytes using this function.                            */
/****************************************************************************/
ssize_t bcm3450_write(char *buf, size_t count)
{ 
    struct i2c_client *client = &pclient_data->client;
    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if(check_offset(buf[0], DWORD_ALIGN))
        return -1;

    if(count > MAX_TRANSACTION_SIZE)
    {
    	BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "count > %d is not yet supported \n", 
                       MAX_TRANSACTION_SIZE);
        return -1;
    }

    /* BCM3450 requires the offset to be the register number */
    buf[0] = buf[0]/4;
    
    return i2c_master_send(client, buf, count);
}
EXPORT_SYMBOL(bcm3450_write);

/****************************************************************************/
/* Read BCM3450: Reads count number of bytes from BCM3450                   */
/* Returns:                                                                 */
/*   number of bytes read on success, negative value on failure.            */
/* Notes: 1. The buf[0] should be a DWORD aligned offset where read starts  */
/*        2. Limits the count to 8. See the notes of bcm3450_write function */
/****************************************************************************/
ssize_t bcm3450_read(char *buf, size_t count)
{
    struct i2c_msg msg[2];
    struct i2c_client *client = &pclient_data->client;
    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if(check_offset(buf[0], DWORD_ALIGN))
        return -1;

    /* BCM3450 requires the offset to be the register number */
    buf[0] = buf[0]/4;

    if(count > MAX_TRANSACTION_SIZE)
    {
    	BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "count > %d is not yet supported \n", 
                       MAX_TRANSACTION_SIZE);
        return -1;
    }
 
    /* First write the offset  */
    msg[0].addr = msg[1].addr = client->addr;
    msg[0].flags = msg[1].flags = client->flags & I2C_M_TEN;

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
EXPORT_SYMBOL(bcm3450_read);

/****************************************************************************/
/* Write Register: Writes the val into BCM3450 register                     */
/* Returns:                                                                 */
/*   0 on success, negative value on failure.                               */
/* Notes: 1. The offset should be DWORD aligned                             */
/****************************************************************************/
int bcm3450_write_reg(u8 offset, int val)
{
    char buf[5];
    struct i2c_client *client = &pclient_data->client;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if(check_offset(offset, DWORD_ALIGN)) {
        return -EINVAL;
    }

    /* BCM3450 requires the offset to be the register number */
    buf[0] = offset/4;

    /* On the I2C bus, LS Byte should go first */
    val = swab32(val);

    memcpy(&buf[1], (char*)&val, 4);
    if (i2c_master_send(client, buf, 5) == 5)
	{
        return 0;
	}
    return -1;
}
EXPORT_SYMBOL(bcm3450_write_reg);

/****************************************************************************/
/* Read Register: Read the BCM3450 register at given offset                 */
/* Returns:                                                                 */
/*   value on success, negative value on failure.                           */
/* Notes: 1. The offset should be DWORD aligned                             */
/****************************************************************************/
int bcm3450_read_reg(u8 offset)
{
    struct i2c_msg msg[2];
    int val;
    struct i2c_client *client = &pclient_data->client;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if(check_offset(offset, DWORD_ALIGN)) {
        return -EINVAL;
    }

    /* BCM3450 requires the offset to be the register number */
    offset = offset/4;

    msg[0].addr = msg[1].addr = client->addr;
    msg[0].flags = msg[1].flags = client->flags & I2C_M_TEN;

    msg[0].len = 1;
    msg[0].buf = (char *)&offset;

    msg[1].flags |= I2C_M_RD;
    msg[1].len = 4;
    msg[1].buf = (char *)&val;

    /* On I2C bus, we receive LS byte first. So swap bytes as necessary */
    if(i2c_transfer(client->adapter, msg, 2) == 2)
        return swab32(val);

    return -1;
}
EXPORT_SYMBOL(bcm3450_read_reg);

/****************************************************************************/
/* Write Word: Writes the val into LSB 2 bytes of Register                  */ 
/* Returns:                                                                 */
/*   0 on success, negative value on failure.                               */
/* Notes: 1. The offset should be WORD aligned                              */
/*    2. ReadModifyWrite is required because the 3450 requires the register */ 
/* number and not byte offset.                                              */
/****************************************************************************/
int bcm3450_write_word(u8 offset, u16 val)
{
    u8 off;
    int tmp_val;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if(check_offset(offset, WORD_ALIGN))
        return -1;
 
    /* Make the offset WORD aligned */
    off = offset & 0xFC; 
    
    /* Read */
    tmp_val = bcm3450_read_reg(off);
    if (tmp_val < 0) 
    {
        return -1;
    }
    
    /* Modify */
    tmp_val = (tmp_val & (~(0xFFFF << ((offset % 4) * 8)))) 
                      | (val << ((offset % 4) * 8));
    
    /* Write */
    return bcm3450_write_reg(off, tmp_val); 

}
EXPORT_SYMBOL(bcm3450_write_word);

/****************************************************************************/
/* Read Word: Reads the LSB 2 bytes of Register                             */ 
/* Returns:                                                                 */
/*   value on success, negative value on failure.                           */
/* Notes: 1. The offset should be WORD aligned                              */
/****************************************************************************/
u16 bcm3450_read_word(u8 offset)
{
    struct i2c_msg msg[2];
    u8 off, buf[4];
    struct i2c_client *client = &pclient_data->client;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if(check_offset(offset, WORD_ALIGN))
        return -1;

    /* BCM3450 requires the offset to be the register number */
    off = offset/4;

    msg[0].addr = msg[1].addr = client->addr;
    msg[0].flags = msg[1].flags = client->flags & I2C_M_TEN;

    msg[0].len = 1;
    msg[0].buf = (char *)&off;

    msg[1].flags |= I2C_M_RD;
    msg[1].len = 4;
    msg[1].buf = buf;

    if(i2c_transfer(client->adapter, msg, 2) == 2)
    {
        return swab16(*((u16 *)&buf[offset % 4]));
    }

    return -1;
}
EXPORT_SYMBOL(bcm3450_read_word);

/****************************************************************************/
/* Write Byte: Writes the val into LS Byte of Register                      */ 
/* Returns:                                                                 */
/*   0 on success, negative value on failure.                               */
/* Note: ReadModifyWrite is required because the 3450 requires the register */ 
/* number and not byte offset.                                              */
/****************************************************************************/
int bcm3450_write_byte(u8 offset, u8 val)
{
    u8 off;
    int tmp_val;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    /* Make the offset DWORD aligned */
    off = offset & 0xFC; 
    
    /* Read */
    tmp_val = bcm3450_read_reg(off);
    if (tmp_val < 0) 
    {
        return -1;
    }
    
    /* Modify */
    tmp_val = (tmp_val & (~(0xFF << ((offset % 4) * 8)))) 
              | (val << ((offset % 4) * 8));
    
    /* Write */
    return bcm3450_write_reg(off, tmp_val); 
}
EXPORT_SYMBOL(bcm3450_write_byte);

/****************************************************************************/
/* Read Byte: Reads the LS Byte of Register                                 */ 
/* Returns:                                                                 */
/*   value on success, negative value on failure.                           */
/****************************************************************************/
u8 bcm3450_read_byte(u8 offset)
{
    struct i2c_msg msg[2];
    u8 off, buf[4];
    struct i2c_client *client = &pclient_data->client;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if (offset > MAX_REG_OFFSET) 
    {
        BCM_LOG_ERROR(BCM_LOG_ID_I2C, "Invalid offset. It should be less than "
                      "%X \n", MAX_REG_OFFSET);
        return -1;
    }

    /* BCM3450 requires the offset to be the register number */
    off = offset/4;

    msg[0].addr = msg[1].addr = client->addr;
    msg[0].flags = msg[1].flags = client->flags & I2C_M_TEN;

    msg[0].len = 1;
    msg[0].buf = (char *)&off;

    msg[1].flags |= I2C_M_RD;
    msg[1].len = 4;
    msg[1].buf = buf;

    if(i2c_transfer(client->adapter, msg, 2) == 2)
    {
        return buf[offset % 4];
    }

    return -1;
}
EXPORT_SYMBOL(bcm3450_read_byte);

#if defined(SYSFS_HOOKS) || defined(PROCFS_HOOKS)
#ifdef MOCA_I2C_TEST
/* Calls the appropriate function based on user command */
static int exec_command(const char *buf, size_t count, int fs_type)
{
#define MAX_ARGS 2
#define MAX_ARG_SIZE 32
    int i, argc = 0, val = 0;
    char cmd;
    u8 offset;
    char arg[MAX_ARGS][MAX_ARG_SIZE];
#if 0
    char temp_buf[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
#endif
#ifdef PROCFS_HOOKS
#define LOG_WR_KBUF_SIZE 128
    char kbuf[LOG_WR_KBUF_SIZE];

    if(fs_type == PROC_FS)
    {
        if ((count > LOG_WR_KBUF_SIZE-1) || 
            (copy_from_user(kbuf, buf, count) != 0))
            return -EFAULT;
    	kbuf[count]=0;
        argc = sscanf(kbuf, "%c %s %s", &cmd, arg[0], arg[1]);
    }
#endif

#ifdef SYSFS_HOOKS
    if(fs_type == SYS_FS)
        argc = sscanf(buf, "%c %s %s", &cmd, arg[0], arg[1]);
#endif

    if (argc <= 1) {
        BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Need at-least 2 arguments \n");
        return -EFAULT;
    }

    for (i=0; i<MAX_ARGS; ++i) {
        arg[i][MAX_ARG_SIZE-1] = '\0';
    }

    offset = (u8) simple_strtoul(arg[0], NULL, 0);
    if (argc == 3)
        val = (int) simple_strtoul(arg[1], NULL, 0);

    switch (cmd) {

#if 0
    case 'y':
        if (argc == 3) {
            if (val > 7) {
                BCM_LOG_INFO(BCM_LOG_ID_I2C, "Limiting byte count to 7 \n");
                val = 7;
            }
   		 	BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Write Byte Stream: offset = %x, " 
   		 	              "count = %x \n", offset, val);
            for (i=0; i< val; i++) {
                BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "%x, ",temp_buf[i]);
            }
            temp_buf[0] = offset;
            bcm3450_write(temp_buf, val+1);
        }
        break;

    case 'z':
        if (argc == 3) {
            if (val > 8) {
                BCM_LOG_INFO(BCM_LOG_ID_I2C, "This test limits the byte stream" 
                             " count to 8 \n");
                val = 8;
            }
            for (i=0; i< val; i++) {
                BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "%x, ",temp_buf[i]);
            }
            temp_buf[0] = offset;
            bcm3450_read(temp_buf, val);
   		 	BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Read Byte Stream: offset = %x, " 
   		 	              "count = %x \n", offset, val);
            for (i=0; i< val; i++) {
                BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "%x, ",temp_buf[i]);
            }
            BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "\n");
        }
        break;
#endif

    case 'b':
        if (argc == 3) {
   		 	BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Write Byte: offset = %x, " 
   		 	              "val = %x \n", offset, val);
            bcm3450_write_byte(offset, (u8)val);
        }
        else {
            val = bcm3450_read_byte(offset);
   		 	BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Read Byte: offset = %x, " 
   		 	              "val = %x \n", offset, val);
        }
        break;

    case 'w':
        if (argc == 3) {
   		 	BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Write Word: offset = %x, " 
   		 	              "val = %x \n", offset, val);
            bcm3450_write_word(offset, (u16)val);
        }
        else {
            val = bcm3450_read_word(offset);
   		 	BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Read Word: offset = %x, " 
   		 	              "val = %x \n", offset, val);
        }
        break;

    case 'd':	
        if (argc == 3) {
   		 	BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Write Register: offset = %x, " 
   		 	              "val = %x \n", offset, val);
            bcm3450_write_reg(offset, val);
        }
        else {
            val = bcm3450_read_reg(offset);
   		 	BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Read Register: offset = %x, "
   		 	              "val = %x \n", offset, val);
        }
        break;

    default:
        BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Invalid command. \n Valid commands: \n" 
                       "  Write Reg:   d offset val \n" 
                       "  Read Reg:    d offset \n" 
                       "  Write Word:  w offset val \n" 
                       "  Read Word:   w offset \n" 
                       "  Write Byte:  b offset val \n" 
                       "  Read Byte:   b offset \n" 
#if 0
                       "  Write Bytes: y offset count \n" 
                       "  Read Bytes:  z offset count \n"
#endif
                       );
        break;
    }
    return count;
}
#endif
#endif

#ifdef PROCFS_HOOKS
#ifdef MOCA_I2C_TEST
/* Read Function of PROCFS attribute */
static ssize_t b3450_proc_test_read(struct file *f, char *buf, size_t count, 
                               loff_t *pos) 
{
    BCM_LOG_NOTICE(BCM_LOG_ID_I2C, " Usage: echo command > "
                   " /proc/b3450reg \n");
    BCM_LOG_NOTICE(BCM_LOG_ID_I2C, " Supported commands: \n" 
                   "  Write Reg:       d offset val \n" 
                   "  Read Reg:        d offset \n" 
                   "  Write Word:      w offset val \n" 
                   "  Read Word:       w offset \n" 
                   "  Write Byte:      b offset val \n" 
                   "  Read Byte:       b offset \n" 
                   );
    return 0;
}

/* Write Function of PROCFS attribute */
static ssize_t b3450_proc_test_write(struct file *f, const char *buf, 
                                     size_t count, loff_t *pos)
{
    return exec_command(buf, count, PROC_FS);
}
#endif

#define REGISTER_LENGTH 4
/* Read Function of PROCFS attribute "/proc/b3450_reg" */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
static ssize_t b3450_proc_read(char *page, char **start, off_t off, int count,
                               int *eof, void *data)
#else
static ssize_t b3450_proc_read(struct file *filep, char __user *page, size_t count, loff_t *offset)
#endif
{
    int ret_val;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
    int off = (int)(*offset);
#endif

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "The offset is %d; the count is %d \n", 
                  (int)off, (int)count);

    /* Only 4-Byte reads are supported */
    if (count != REGISTER_LENGTH) {
        BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Only 4 Byte reads are supported \n");
        return -EINVAL;
    }

    /*   See comments in the proc_file_read for info on 3 different
    *    ways of returning data. We are following below method.
    *    Set *start = an address within the buffer.
    *    Put the data of the requested offset at *start.
    *    Return the number of bytes of data placed there.
    *    If this number is greater than zero and you
    *    didn't signal eof and the reader is prepared to
    *    take more data you will be called again with the
    *    requested offset advanced by the number of bytes
    *    absorbed. */
    ret_val = bcm3450_read_reg((u8)off);
#ifdef __LITTLE_ENDIAN
    *((int *)page) = ret_val;
#else
    *((int *)page) = swab32(ret_val);
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
    *start = page;
    *eof = 1;
    /* If ret_val is less than 0, return ret_val; else return number
       of bytes read which is 4 */
    return ((ret_val < 0) ? ret_val : REGISTER_LENGTH);
#else
    *offset += REGISTER_LENGTH;

    /* return zero here to indicate EOF */
    return ((ret_val < 0) ? ret_val : 0);
#endif

}

/* Write Function of PROCFS attribute "/proc/b3450_reg" */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
static ssize_t b3450_proc_write(struct file *file, const char __user *buffer,
                                  unsigned long count, void *data)
#else
static ssize_t b3450_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *off)
#endif
{
    int value, ret_val;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
    int offset = (int)file->f_pos;
#else
    int offset = (int)(*off);
#endif

    /* Only 4-Byte writes are supported */
    if (count != REGISTER_LENGTH) {
        BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Only 4 Byte writes are supported \n");
        return -EINVAL;
    }

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "The offset is %d; the count is %ld \n", 
                  offset, (long)count);

#ifdef __LITTLE_ENDIAN
    value = *((int *)buffer);
#else
    value = swab32(*((int *)buffer));
#endif

    ret_val = bcm3450_write_reg(offset, value);

    /* If ret_val is less than 0, return ret_val; else return number
       of bytes read which is 4 */
    return ((ret_val < 0) ? ret_val : REGISTER_LENGTH);
}
#endif

#ifdef SYSFS_HOOKS
/* Read Function of SYSFS attribute */
static ssize_t b3450_read(struct device *dev, struct device_attribute *attr, 
                          char *buf)
{
    return snprintf(buf, PAGE_SIZE, "The bcm3450 reg access attribute \n");
}

/* Write Function of SYSFS attribute */
static ssize_t b3450_write(struct device *dev, struct device_attribute *attr, 
                           const char *buf, size_t count)
{
    return exec_command(buf, count, SYS_FS);
}

static DEVICE_ATTR(reg_access, S_IRWXUGO, b3450_read, b3450_write);

static struct attribute *bcm3450_attributes[] = {
    &dev_attr_reg_access.attr,
    NULL
};

static const struct attribute_group bcm3450_attr_group = {
    .attrs = bcm3450_attributes,
};
#endif

#ifdef PROCFS_HOOKS
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
static struct file_operations b3450Proc_fops = {
    .owner  = THIS_MODULE,
    .read = b3450_proc_read,
    .write = b3450_proc_write
};
#endif
#ifdef MOCA_I2C_TEST
static struct file_operations b3450Test_fops = {
    .owner  = THIS_MODULE,
    .read = b3450_proc_test_read,
    .write = b3450_proc_test_write
};
#endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0)
/* This is the driver that will be inserted */
static struct i2c_driver bcm3450_driver = {
    .driver = {
        .name	= "bcm3450",
    },
    .attach_adapter	= bcm3450_attach_adapter,
    .detach_client	= bcm3450_detach_client,
};
#else
static struct i2c_driver bcm3450_driver = {
    .driver = {
        .name	= "bcm3450",
    },
    .probe  = bcm3450_probe,
    .remove = bcm3450_remove,
};
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0)
static int bcm3450_attach_adapter(struct i2c_adapter *adapter)
{
    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);
    return i2c_probe(adapter, &addr_data, bcm3450_detect);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0)
/* This function is called by i2c_probe */
static int bcm3450_detect(struct i2c_adapter *adapter, int address, int kind)
{
    struct i2c_client *client;
    int err = 0;
#ifdef PROCFS_HOOKS
    struct proc_dir_entry *p;
#endif

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
        goto exit;

    if (!(pclient_data = kzalloc(sizeof(struct bcm3450_data), GFP_KERNEL))) 
    {
        err = -ENOMEM;
        goto exit;
    }

    /* Setup the i2c client data */
    client = &pclient_data->client;
    i2c_set_clientdata(client, pclient_data);
    client->addr = address;
    client->adapter = adapter;
    client->driver = &bcm3450_driver;
    client->flags = 0;
    strlcpy(client->name, "bcm3450", I2C_NAME_SIZE);

    /* Tell the I2C layer a new client has arrived */
    if ((err = i2c_attach_client(client)))
        goto exit_kfree;

#ifdef SYSFS_HOOKS
    /* Register sysfs hooks */
    err = sysfs_create_group(&client->dev.kobj, &bcm3450_attr_group);
    if (err)
        goto exit_detach;
#endif

#ifdef PROCFS_HOOKS
    p = create_proc_entry(PROC_ENTRY_NAME1, 0, 0);
    if (!p) {
        BCM_LOG_ERROR(BCM_LOG_ID_I2C, "bcmlog: unable to create /proc/%s!\n", 
                      PROC_ENTRY_NAME1);
        err = -EIO;
#ifdef SYSFS_HOOKS
        sysfs_remove_group(&client->dev.kobj, &bcm3450_attr_group);
#endif
        goto exit_detach;
    }
    p->read_proc = b3450_proc_read;
    p->write_proc = b3450_proc_write;
    p->data = (void *)pclient_data;
    
#ifdef MOCA_I2C_TEST
        p = create_proc_entry(PROC_ENTRY_NAME2, 0, 0);
        if (p) {
            p->proc_fops = &b3450Test_fops;
        }
#endif
#endif

    return 0;

#if defined(SYSFS_HOOKS) || defined(PROCFS_HOOKS)
exit_detach:
    i2c_detach_client(client);
#endif
exit_kfree:
    kfree(pclient_data);
exit:
    return err;
}
#else
static int bcm3450_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int err = 0;
#ifdef PROCFS_HOOKS
    struct proc_dir_entry *p;
#endif

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
        goto exit;

    if (!(pclient_data = kzalloc(sizeof(struct bcm3450_data), GFP_KERNEL))) 
    {
        err = -ENOMEM;
        goto exit;
    }

    i2c_set_clientdata(client, pclient_data);

#ifdef SYSFS_HOOKS
    /* Register sysfs hooks */
    err = sysfs_create_group(&client->dev.kobj, &bcm3450_attr_group);
    if (err)
        goto exit_kfree;
#endif

#ifdef PROCFS_HOOKS
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
    p = create_proc_entry(PROC_ENTRY_NAME1, 0, 0);
    if (!p) {
        BCM_LOG_ERROR(BCM_LOG_ID_I2C, "bcm3450: unable to create /proc/%s!\n", 
                      PROC_ENTRY_NAME1);
        err = -EIO;
        goto exit_sysfs;
    }
    p->read_proc = b3450_proc_read;
    p->write_proc = b3450_proc_write;
    p->data = (void *)pclient_data;
#ifdef MOCA_I2C_TEST
    p = create_proc_entry(PROC_ENTRY_NAME2, 0, 0);
    if (!p) {
        BCM_LOG_ERROR(BCM_LOG_ID_I2C, "bcm3450: unable to create /proc/%s!\n", 
                      PROC_ENTRY_NAME2);
        err = -EIO;
        goto exit_procfs;
    }
    p->proc_fops = &b3450Test_fops;
#endif
#else
    p = proc_create_data(PROC_ENTRY_NAME1, 0644, NULL, &b3450Proc_fops, pclient_data);
    if (!p) {
        BCM_LOG_ERROR(BCM_LOG_ID_I2C, "bcm3450: unable to create /proc/%s!\n", 
                      PROC_ENTRY_NAME1);
        err = -EIO;
        goto exit_sysfs;
    }
#ifdef MOCA_I2C_TEST
    p = proc_create(PROC_ENTRY_NAME2, 0644, NULL, &b3450Test_fops);
    if (!p) {
        BCM_LOG_ERROR(BCM_LOG_ID_I2C, "bcm3450: unable to create /proc/%s!\n", 
                      PROC_ENTRY_NAME2);
        err = -EIO;
        goto exit_procfs;
    }
#endif
#endif
#endif

    return 0;

exit_procfs:
#if defined(PROCFS_HOOKS) && defined(MOCA_I2C_TEST)
    remove_proc_entry(PROC_ENTRY_NAME1, NULL);
#endif
exit_sysfs:
#ifdef SYSFS_HOOKS
    sysfs_remove_group(&client->dev.kobj, &bcm3450_attr_group);

exit_kfree:
#endif
    kfree(pclient_data);
exit:
    return err;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0)
static int bcm3450_detach_client(struct i2c_client *client)
#else
static int bcm3450_remove(struct i2c_client *client)
#endif
{
    int err = 0;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

#ifdef SYSFS_HOOKS
    sysfs_remove_group(&client->dev.kobj, &bcm3450_attr_group);
#endif

#ifdef PROCFS_HOOKS
    remove_proc_entry(PROC_ENTRY_NAME1, NULL);
#ifdef MOCA_I2C_TEST
    remove_proc_entry(PROC_ENTRY_NAME2, NULL);
#endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0)
    err = i2c_detach_client(client);
    if (err)
        return err;
#endif

    kfree(i2c_get_clientdata(client));

    return err;
}

static int __init bcm3450_init(void)
{
    return i2c_add_driver(&bcm3450_driver);
}

static void __exit bcm3450_exit(void)
{
    i2c_del_driver(&bcm3450_driver);
}

MODULE_AUTHOR("Pratapa Reddy, Vaka <pvaka@broadcom.com>");
MODULE_DESCRIPTION("BCM3450 I2C driver");
MODULE_LICENSE("GPL");

module_init(bcm3450_init);
module_exit(bcm3450_exit);
