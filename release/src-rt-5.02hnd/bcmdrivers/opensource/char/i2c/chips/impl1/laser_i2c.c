/*
<:copyright-BRCM:2012:DUAL/GPL:standard 

   Copyright (c) 2012 Broadcom 
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

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/slab.h>  /* kzalloc() */
#include <linux/types.h>
#include <linux/bcm_log.h>
#include <boardparms.h>

/* I2C client chip addresses */
#define LASER_I2C_ADDR1 0x4e    // address of mindspeed chip with BOSA optics

/* Addresses to scan */
static unsigned short normal_i2c[] = {LASER_I2C_ADDR1, LASER_I2C_ADDR1, I2C_CLIENT_END};

#if defined(CONFIG_I2C_GPIO)
#include <linux/platform_device.h>
#include <linux/i2c-gpio.h>
char *g_platform_info_ptr = NULL;
static void laser_setup_i2c_gpio(void);
#endif

#define RETURN_IF_NULL_CLIENT(C)    if(!client) return(-1)

/* Size of client in bytes */
#define DATA_SIZE		      256
#define DWORD_ALIGN	          4
#define WORD_ALIGN            2
#define MAX_TRANSACTION_SIZE  32
#define MAX_REG_OFFSET        150


/* Each client has this additional data */
struct laser_i2c_data {
    struct i2c_client client;
};

static struct laser_i2c_data *pclient_data; 

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0)
/* Insmod parameters */
I2C_CLIENT_INSMOD_1(laser_i2c);

static int laser_i2c_attach_adapter(struct i2c_adapter *adapter);
static int laser_i2c_detect(struct i2c_adapter *adapter, int address, int kind);
static int laser_i2c_detach_client(struct i2c_client *client);

#else /* !LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0) */
static int laser_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int laser_i2c_remove(struct i2c_client *client);
static int laser_i2c_detect(struct i2c_client *client, struct i2c_board_info *info);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0) */

/* Check if given 3450 register offset is valid or not */
static inline int check_offset(u8 offset, u8 alignment)
{
    if (offset > MAX_REG_OFFSET) 
    {
        BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "\nInvalid offset. It should be less than "
                      "%X \n", MAX_REG_OFFSET);
        return -EINVAL;
    }
    else if (offset % alignment) 
    {
        BCM_LOG_NOTICE(BCM_LOG_ID_I2C, "\nInvalid offset %d. The offset should be "
                      "%d byte alligned \n", offset, alignment);
        return -EINVAL;
    }
    return 0;
}

/****************************************************************************/
/* laser_i2c_write:                                                         */
/*      Writes count number of bytes from buf on to the I2C bus             */
/* Returns:                                                                 */
/*      number of bytes written on success, negative value on failure.      */
/* Notes: 1. The LS byte should follow the offset                           */
/* Design Notes: The gponPhy takes the first byte after the chip address    */
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
ssize_t laser_i2c_write( char *buf, size_t count)
{
    struct i2c_client *client = &pclient_data->client;

    RETURN_IF_NULL_CLIENT(client);
    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    return i2c_master_send(client, buf, count);
}
EXPORT_SYMBOL(laser_i2c_write);

/****************************************************************************/
/* laser_i2c_read: Reads count number of bytes from mindspeed 2098 BOSA optics     */
/* Returns:                                                                 */
/*   number of bytes read on success, negative value on failure.            */
/* Notes: 1. The offset should be provided in buf[0]                        */
/*        2. The count is limited to 32.                                    */
/*        3. The gponPhy with the serial EEPROM protocol requires the offset*/
/*        be written before reading the data on every I2C transaction       */
/****************************************************************************/
ssize_t laser_i2c_read( char *buf, size_t count)
{
    struct i2c_msg msg[2];
    struct i2c_client *client = &pclient_data->client;

    RETURN_IF_NULL_CLIENT(client);
    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

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

EXPORT_SYMBOL(laser_i2c_read);

/****************************************************************************/
/* Write Register: Writes the val into laser optics register                */
/* Returns:                                                                 */
/*   0 on success, negative value on failure.                               */
/* Notes: 1. The offset should be DWORD aligned                             */
/****************************************************************************/
int laser_i2c_write_reg(u8 offset, int val)
{
    char buf[5];
    struct i2c_client *client = &pclient_data->client;

    RETURN_IF_NULL_CLIENT(client);
    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if(check_offset(offset, DWORD_ALIGN)) {
        return -EINVAL;
    }

    /* On the I2C bus, LS Byte should go first */
    val = swab32(val);

    memcpy(&buf[1], (char*)&val, 4);
    if (i2c_master_send(client, buf, 5) == 5)
	{
        return 0;
	}
    return -1;
}
EXPORT_SYMBOL(laser_i2c_write_reg);

/****************************************************************************/
/* Read Register: Read the laser optics register at given offset            */
/* Returns:                                                                 */
/*   value on success, negative value on failure.                           */
/* Notes: 1. The offset should be DWORD aligned                             */
/****************************************************************************/
int laser_i2c_read_reg(u8 offset)
{
    struct i2c_msg msg[2];
    int val;
    struct i2c_client *client = &pclient_data->client;

    RETURN_IF_NULL_CLIENT(client);
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
EXPORT_SYMBOL(laser_i2c_read_reg);

/****************************************************************************/
/* Write Word: Writes the val into LSB 2 bytes of Register                  */ 
/* Returns:                                                                 */
/*   0 on success, negative value on failure.                               */
/* Notes: 1. The offset should be WORD aligned                              */
/*    2. ReadModifyWrite is required because the 3450 requires the register */ 
/* number and not byte offset.                                              */
/****************************************************************************/
int laser_i2c_write_word(u8 offset, u16 val)
{
    char buf[3];
    struct i2c_client *client = &pclient_data->client;

    RETURN_IF_NULL_CLIENT(client);
    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);
    
    if(check_offset(offset, WORD_ALIGN))
    {
        return -1;
    }
    
    /* The offset to be written should be the first byte in the I2C write */
    buf[0] = offset;
    buf[1] = (char)(val&0xFF);
    buf[2] = (char)(val>>8);
    if (i2c_master_send(client, buf, 3) == 3)
    {
        return 0;
    }
    return -1;
}
EXPORT_SYMBOL(laser_i2c_write_word);

/****************************************************************************/
/* Read Word: Reads the LSB 2 bytes of Register                             */ 
/* Returns:                                                                 */
/*   value on success, negative value on failure.                           */
/* Notes: 1. The offset should be WORD aligned                              */
/****************************************************************************/
u16 laser_i2c_read_word(u8 offset)
{
    struct i2c_msg msg[2];
    u16 val;
    struct i2c_client *client = &pclient_data->client;

    RETURN_IF_NULL_CLIENT(client);
    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    if(check_offset(offset, WORD_ALIGN))
    {
        return -1;
    }

    msg[0].addr = msg[1].addr = client->addr;
    msg[0].flags = msg[1].flags = client->flags & I2C_M_TEN;

    msg[0].len = 1;
    msg[0].buf = (char *)&offset;

    msg[1].flags |= I2C_M_RD;
    msg[1].len = 2;
    msg[1].buf = (char *)&val;

    if(i2c_transfer(client->adapter, msg, 2) == 2)
    {
        return swab16(val);
    }

    return -1;
}
EXPORT_SYMBOL(laser_i2c_read_word);


/****************************************************************************/
/* Write Byte: Writes the val into LS Byte of Register                      */ 
/* Returns:                                                                 */
/*   0 on success, negative value on failure.                               */
/* Note: ReadModifyWrite is required because the 3450 requires the register */ 
/* number and not byte offset.                                              */
/****************************************************************************/
int laser_i2c_write_byte(u8 offset, u8 val)
{

    char buf[2];    
    struct i2c_client *client = &pclient_data->client;

    RETURN_IF_NULL_CLIENT(client);
    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "\nEntering the function %s \n", __FUNCTION__);

    buf[0] = offset;
    buf[1] = val;
    
    if (i2c_master_send(client, buf, 2) == 2)     
    {        
        return 0;    
    }    
    
    return -1;

}
EXPORT_SYMBOL(laser_i2c_write_byte);

/****************************************************************************/
/* Read Byte: Reads the LS Byte of Register                                 */ 
/* Returns:                                                                 */
/*   value on success, negative value on failure.                           */
/****************************************************************************/
u8 laser_i2c_read_byte(u8 offset)
{
    struct i2c_msg msg[2];
    char val;
    struct i2c_client *client = &pclient_data->client;

    RETURN_IF_NULL_CLIENT(client);
    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    msg[0].addr = msg[1].addr = client->addr;
    msg[0].flags = msg[1].flags = client->flags & I2C_M_TEN;

    msg[0].len = 1;
    msg[0].buf = (char *)&offset;

    msg[1].flags |= I2C_M_RD;
    msg[1].len = 1;
    msg[1].buf = (char *)&val;

    if(i2c_transfer(client->adapter, msg, 2) == 2)
    {
        return val;
    }

    return -1;

}
EXPORT_SYMBOL(laser_i2c_read_byte);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0)
/* This is the driver that will be inserted */
static struct i2c_driver laser_i2c_driver = {
    .driver = {
        .name	= "laser i2c",
    },
    .attach_adapter	= laser_i2c_attach_adapter,
    .detach_client	= laser_i2c_detach_client,
};

static int laser_i2c_attach_adapter(struct i2c_adapter *adapter)
{

    int ret;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "\nEntering the function %s \n", __FUNCTION__);

    ret = i2c_probe(adapter, &addr_data, laser_i2c_detect);       

    return ret;
}

/* This function is called by i2c_probe */
static int laser_i2c_detect(struct i2c_adapter *adapter, int address, int kind)
{
    struct i2c_client *client;
    int err = 0;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "\nEntering the function %s \n", __FUNCTION__);

    if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
        goto exit;

    if (!(pclient_data = kzalloc(sizeof(struct laser_i2c_data), GFP_KERNEL))) 
    {
        err = -ENOMEM;
        goto exit;
    }

    /* Setup the i2c client data */
    client = &pclient_data->client;
    i2c_set_clientdata(client, pclient_data);
    client->addr = address;
    client->adapter = adapter;
    client->driver = &laser_i2c_driver;
    client->flags = 0;
    strlcpy(client->name, "laser i2c", I2C_NAME_SIZE);

    /* Tell the I2C layer a new client has arrived */
    if ((err = i2c_attach_client(client)))
        goto exit_kfree;

    return 0;

exit_kfree:
    kfree(pclient_data);
    pclient_data = NULL;
exit:
    return err;
}

static int laser_i2c_detach_client(struct i2c_client *client)
{
    int err = 0;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "\nEntering the function %s \n", __FUNCTION__);

    err = i2c_detach_client(client);
    if (err)
        return err;

    kfree(i2c_get_clientdata(client));

    return err;
}

#else /* !LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0) */

static const struct i2c_device_id laser_i2c_id_table[] = {
    { "laser_i2c", 0 },
    { },
};

MODULE_DEVICE_TABLE(i2c, laser_i2c_id_table);

static struct i2c_driver laser_i2c_driver = {
    .class = ~0,
    .driver = {
        .name = "laser_i2c",
    },
    .probe  = laser_i2c_probe,
    .remove = laser_i2c_remove,
    .id_table = laser_i2c_id_table,
    .detect  = laser_i2c_detect,
    .address_list = normal_i2c

};

static int laser_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int err = 0;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "\nEntering the function %s \n", __FUNCTION__);

    if (i2c_check_functionality(client->adapter, I2C_FUNC_I2C) != 0)
    {
        if (!(pclient_data = kzalloc(sizeof(struct laser_i2c_data), GFP_KERNEL))) 
            err = -ENOMEM;
        else
        {
            /* Setup the i2c client data */
            pclient_data->client.addr = client->addr;
            pclient_data->client.adapter = client->adapter;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
            pclient_data->client.driver = client->driver;
#endif
            pclient_data->client.flags = client->flags;

            i2c_set_clientdata(client, pclient_data);
        }
    }

    return err;
}

static int laser_i2c_detect(struct i2c_client *client, struct i2c_board_info *info)
{
    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "\nEntering the function %s \n", __FUNCTION__);
    strcpy(info->type, "laser_i2c");
    info->flags = 0;
    return 0;
}

static int laser_i2c_remove(struct i2c_client *client)
{
    int err = 0;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "\nEntering the function %s \n", __FUNCTION__);

    kfree(i2c_get_clientdata(client));

    return err;
}
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0) */

static int __init laser_i2c_init(void)
{
#if defined(CONFIG_I2C_GPIO)
    laser_setup_i2c_gpio();
#endif
    return i2c_add_driver(&laser_i2c_driver);
}

static void __exit laser_i2c_exit(void)
{
#if defined(CONFIG_I2C_GPIO)
    if( g_platform_info_ptr )
        kfree(g_platform_info_ptr);
#endif
    i2c_del_driver(&laser_i2c_driver);
}

module_init(laser_i2c_init);
module_exit(laser_i2c_exit);

#if defined(CONFIG_I2C_GPIO)
/****************************************************************************
 * If CONFIG_I2C_GPIO is defined, the kernel I2C gpio driver,
 * kernel/linux/drivers/i2c/busses/i2c-gpio.c, is compiled into the image.
 * Register a device with it. Function calls to i2c_transfer and i2c_master_send
 * go to the i2c-gpio driver.
 *
 * If CONFIG_I2C_GPIO is not defined, another I2C driver such as the Broadcom
 * I2C driver, bcmdrivers/broadcom/char/i2c/busses/impl1/i2c_bcm6xxx.c, is
 * compiled into the image.  Function calls to i2c_transfer and i2c_master_send
 * go to that I2C driver.
 ****************************************************************************/
static void laser_setup_i2c_gpio(void)
{
    if( g_platform_info_ptr == NULL )
    {
        g_platform_info_ptr = kzalloc(sizeof(struct platform_device) +
            sizeof(struct i2c_gpio_platform_data), GFP_KERNEL);

        if( g_platform_info_ptr )
        {
            unsigned short bpGpio_scl, bpGpio_sda;
            struct platform_device *pd = 
                (struct platform_device *) g_platform_info_ptr;
            struct i2c_gpio_platform_data *igpd =
                (struct i2c_gpio_platform_data *) (pd + 1);

            pd->name = "i2c-gpio";
            pd->dev.platform_data = (void *)igpd;

            if( BpGetI2cGpios(&bpGpio_scl, &bpGpio_sda) == BP_SUCCESS )
            {
                igpd->sda_pin = bpGpio_sda;
                igpd->scl_pin = bpGpio_scl;
            }
            else
                printk("\nLaser_Dev: error getting SCL and SDA GPIOs\n");

            /* Set udelay and timeout to 0 to take driver default values. */
            igpd->udelay = igpd->timeout = 0;

            if( platform_device_register(pd) < 0 )
                printk("\nLaser_Dev: error registering platform device\n");
        }
        else
            printk("\nLaser_Dev: error allocating platform_device info\n");
    }
    else /* should not happen */
        printk("\nLaser_Dev: platform_device info already allocated\n");
}
#endif

MODULE_AUTHOR("Tim Sharp <tsharp@broadcom.com>");
MODULE_DESCRIPTION("LASER I2C driver");
MODULE_LICENSE("GPL");


