/*
    i2cmux_i2c.c - I2C client driver for GPON transceiver
    Copyright (C) 2016 Broadcom Corp.

* <:copyright-BRCM:2016:DUAL/GPL:standard
*
*    Copyright (c) 2016 Broadcom
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
#include <boardparms.h>
#include <i2cmux_i2c.h>

#ifdef PROCFS_HOOKS
#include <asm/uaccess.h> /*copy_from_user*/
#include <linux/proc_fs.h>
#define PROC_DIR_NAME "i2c_mux"
#ifdef GPON_I2C_TEST
#define PROC_ENTRY_NAME "i2cmuxOper"
#endif
#endif


/* I2C client chip addresses */
/* Note that these addresses are 7-bit addresses without the LSB bit
   which indicates R/W operation */
#define I2CMUX_I2C_ADDR1 0x72
#define I2CMUX_I2C_ADDR2 0x70

/* Addresses to scan */
static unsigned short normal_i2c[] = {I2CMUX_I2C_ADDR1, I2CMUX_I2C_ADDR2, I2C_CLIENT_END};

/* file system */
enum fs_enum {PROC_FS, SYS_FS};

/* Each client has this additional data */
struct i2cmux_data {
    struct i2c_client client;
};

/* Assumption: The i2c modules will be built-in to the kernel and will not be
   unloaded; otherwise, it is possible for caller modules to call the exported
   functions even when the i2c modules are not loaded unless some registration
   mechanism is built-in to this module.  */
static struct i2cmux_data *pclient1_data;

static int i2cmux_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int i2cmux_remove(struct i2c_client *client);
static int i2cmux_detect(struct i2c_client *, struct i2c_board_info *);

/* Check if given offset is valid or not */
static inline int check_offset(u8 offset, u8 alignment)
{
    if (offset % alignment)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_I2C, "Invalid offset %d. The offset should be"
                      " %d byte alligned \n", offset, alignment);
        return -1;
    }
    return 0;
}

ssize_t i2cmux_write(char *buf, size_t count)
{
    struct i2c_client *client = NULL;
    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if(count > MAX_TRANSACTION_SIZE)
    {
        BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "count > %d is not yet supported \n", MAX_TRANSACTION_SIZE);
        return -1;
    }

    client = &pclient1_data->client;

    return i2c_master_send(client, buf, count);
}
EXPORT_SYMBOL(i2cmux_write);


ssize_t i2cmux_read(char *buf, size_t count)
{
    struct i2c_client *client = NULL;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if(count > MAX_TRANSACTION_SIZE)
    {
        BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "count > %d is not yet supported \n", MAX_TRANSACTION_SIZE);
        return -1;
    }

    client = &pclient1_data->client;

    return i2c_master_recv(client, buf, count);
}
EXPORT_SYMBOL(i2cmux_read);



#if defined(SYSFS_HOOKS) || defined(PROCFS_HOOKS)
#define LOG_WR_KBUF_SIZE 128
/* Calls the appropriate function based on user command */
static int exec_command(char *buf, size_t count, int is_write)
{
#define MAX_ARG_SIZE 32
    int argc = 0, val = 0;
    char temp_buf[MAX_ARG_SIZE];

    argc = sscanf(buf, "w %s", temp_buf);

    if (is_write && argc == 1)
    {
        val = (int) simple_strtoul(temp_buf, NULL, 0);
        temp_buf[0] = val;
        i2cmux_write(temp_buf, 1);
    }
    else if (is_write == 0)
    {
        i2cmux_read(temp_buf, 1);
        count = snprintf(buf, count, "0x%x\n", (u8)temp_buf[0]);
    }
    else
    {
        BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "Invalid command. \n Valid commands: \n"
                " w <Byte> ");
        return -EFAULT;
    }

    return count;
}
#endif

#ifdef PROCFS_HOOKS
/* Read Function of PROCFS attribute "i2cmuxOper" */
static ssize_t i2cmux_proc_oper_read(struct file *f, char *buf, size_t count,
                               loff_t *pos)
{
    char kbuf[LOG_WR_KBUF_SIZE] = {};
    int ret;

    ret = exec_command(kbuf, LOG_WR_KBUF_SIZE, 0);
    if (*pos > LOG_WR_KBUF_SIZE)
        return 0;

    if (*pos + count > LOG_WR_KBUF_SIZE)
        count = LOG_WR_KBUF_SIZE - *pos;

    if (copy_to_user(buf, kbuf + *pos, count) != 0)
        return -EFAULT;

    return count;
}

/* Write Function of PROCFS attribute "i2cmuxOper" */
static ssize_t i2cmux_proc_oper_write(struct file *f, const char *buf,
                                       size_t count, loff_t *pos)
{
    char kbuf[LOG_WR_KBUF_SIZE];

    if ((count > LOG_WR_KBUF_SIZE-1) ||
            (copy_from_user(kbuf, buf, count) != 0))
        return -EFAULT;

    kbuf[count]=0;

    return exec_command(kbuf, count, 1);
}
#endif

#ifdef SYSFS_HOOKS
/* Read Function of SYSFS attribute */
static ssize_t i2cmux_sys_read(struct device *dev, struct device_attribute *attr,
                          char *buf)
{
    return exec_command(buf, PAGE_SIZE, 0);
}

/* Write Function of SYSFS attribute */
static ssize_t i2cmux_sys_write(struct device *dev, struct device_attribute *attr,
                           const char *buf, size_t count)
{
    return exec_command(buf, count, 1);
}

static DEVICE_ATTR(i2cmux_access, S_IRWXUGO, i2cmux_sys_read, i2cmux_sys_write);

static struct attribute *i2cmux_attributes[] = {
    &dev_attr_i2cmux_access.attr,
    NULL
};

static const struct attribute_group i2cmux_attr_group = {
    .attrs = i2cmux_attributes,
};
#endif

#ifdef PROCFS_HOOKS

static struct file_operations i2cmuxOper_fops = {
    .owner  = THIS_MODULE,
    .read = i2cmux_proc_oper_read,
    .write = i2cmux_proc_oper_write
};

static struct proc_dir_entry *q=NULL;
#endif

static const struct i2c_device_id i2cmux_i2c_id_table[] =
{
    { "i2cmux_i2c", 0 },
    { },
};

MODULE_DEVICE_TABLE(i2c, i2cmux_i2c_id_table);

static struct i2c_driver i2cmux_driver = {
    .class = ~0,
    .driver = {
        .name = "i2cmux_i2c",
    },
    .probe  = i2cmux_probe,
    .remove = i2cmux_remove,
    .id_table = i2cmux_i2c_id_table,
    .detect = i2cmux_detect,
    .address_list = normal_i2c
};

static int i2cmux_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int err = 0;
    struct i2cmux_data *pclient_data;
#ifdef PROCFS_HOOKS
    struct proc_dir_entry *p;
#endif

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
        goto exit;

    if (!(pclient_data = kzalloc(sizeof(struct i2cmux_data), GFP_KERNEL)))
    {
        err = -ENOMEM;
        goto exit;
    }

    pclient_data->client.addr = client->addr;
    pclient_data->client.adapter = client->adapter;
    pclient_data->client.flags = client->flags;

    i2c_set_clientdata(client, pclient_data);

    switch(client->addr)
    {
    case I2CMUX_I2C_ADDR1:
    case I2CMUX_I2C_ADDR2:
        pclient1_data = pclient_data;
        break;
    default:
        BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "%s client addr out of range \n", __FUNCTION__);
        goto exit_kfree;
    }

#ifdef SYSFS_HOOKS
    /* Register sysfs hooks */
    err = sysfs_create_group(&client->dev.kobj, &i2cmux_attr_group);
    if (err)
        goto exit_kfree;
#endif

#ifdef PROCFS_HOOKS
    if (!q && (q = proc_mkdir(PROC_DIR_NAME, NULL) ) == NULL) {
        BCM_LOG_ERROR(BCM_LOG_ID_I2C, "i2cmux_i2c: unable to create proc entry\n");
        err = -ENOMEM;
#ifdef SYSFS_HOOKS
        sysfs_remove_group(&client->dev.kobj, &i2cmux_attr_group);
#endif
        goto exit_kfree;
    }

    p = proc_create(PROC_ENTRY_NAME, 0644, q, &i2cmuxOper_fops);
#endif

    return 0;

exit_kfree:
    kfree(pclient_data);
exit:
    return err;

}

static int i2cmux_detect(struct i2c_client *client, struct i2c_board_info *info)
{
    unsigned short mux_addr;
    char temp_buf = 0x1;
    
    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if (BpGetI2cMuxAddr(&mux_addr) || mux_addr != client->addr)
        return -ENODEV;

    if (pclient1_data)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_I2C, "I2C MUX already exists, cannot instantiate another one %s \n", __FUNCTION__);

        return -1;
    }

    /* Write default value */
    i2c_master_send(client, &temp_buf, 1);

    strcpy(info->type, "i2cmux_i2c");
    info->flags = 0;

    return 0;
}

static int i2cmux_remove(struct i2c_client *client)
{
    int err = 0;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

#ifdef SYSFS_HOOKS
    sysfs_remove_group(&client->dev.kobj, &i2cmux_attr_group);
#endif

#ifdef PROCFS_HOOKS
        remove_proc_entry(PROC_ENTRY_NAME, q);
        remove_proc_entry(PROC_DIR_NAME, NULL);
#endif

    kfree(i2c_get_clientdata(client));

    return err;
}

module_i2c_driver(i2cmux_driver);

MODULE_LICENSE("GPL");

