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
#include "bcm_map_part.h"

/* I2C client chip addresses */
#define sfp_I2C_ADDR1 0xac    // address of mindspeed chip with BOSA optics

/* Addresses to scan */
static unsigned short normal_i2c[] = {sfp_I2C_ADDR1>>I2C_CHIP_ADDRESS_SHIFT, I2C_CLIENT_END};

struct i2c_client *myclient;

static int sfp_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int sfp_i2c_remove(struct i2c_client *client);
static int sfp_i2c_detect(struct i2c_client *client, struct i2c_board_info *info);

static const struct i2c_device_id sfp_i2c_id_table[] = {
    { "sfp_i2c", 0 },
    { },
};
MODULE_DEVICE_TABLE(i2c, sfp_i2c_id_table);

static struct i2c_driver sfp_i2c_driver = {
    .class = ~0,
    .driver = {
        .name = "sfp_i2c",
    },
    .probe  = sfp_i2c_probe,
    .remove = sfp_i2c_remove,
    .id_table = sfp_i2c_id_table,
    .detect  = sfp_i2c_detect,
    .address_list = normal_i2c

};

static int sfp_i2c_add_driver(void)
{
    int ret;

    ret = i2c_add_driver(&sfp_i2c_driver);
    if (ret)
        return ret;

    /* If no device presented at bus, remove us */
    if (myclient == NULL)
    {
        i2c_del_driver(&sfp_i2c_driver);
        return -1;
    }
    printk("I2C SFP Module Driver Loaded\n");
    return 0;
}

static int __init sfp_i2c_init(void)
{
    sfp_i2c_add_driver();
    return 0;
}

/****************************************************************************/
/* sfp_i2c_write:                                                           */
/*      Writes count number of bytes from buf on to the I2C bus             */
/* Returns:                                                                 */
/*   Successfully written bytes(2) or 0 if no valid SFP modules.            */
/****************************************************************************/
int sfp_i2c_phy_write( int reg, int data)
{
    u8 dbuf[4];
    int ret;

    if (myclient == NULL)
    {
        ret = sfp_i2c_add_driver();
        if (ret)
        {
            return 0;
        }
    }

    dbuf[0] = reg;
    dbuf[1] = (data >> 8) & 0xff;
    dbuf[2] = data & 0xff;

    ret = i2c_master_send(myclient, dbuf, 3);
    return ret == 3;
}
EXPORT_SYMBOL(sfp_i2c_phy_write);

/****************************************************************************/
/* sfp_i2c_read: Reads SFP PHY register                                     */
/* Returns:                                                                 */
/*   Successfully read bytes(2) or 0 if no valid SFP modules.               */
/* Note: the register values are put into *data as Host endianess           */
/****************************************************************************/
int sfp_i2c_phy_read( int reg, int *data)
{
    struct i2c_msg msg[2];
    u8 dbuf[4];
    int ret;

    if (myclient == NULL)
    {
        ret = sfp_i2c_add_driver();
        if (ret)
        {
            return 0;
        }
    }

    /* First write the register  */
    msg[0].addr = msg[1].addr = myclient->addr;
    msg[0].flags = msg[1].flags = 0;
    msg[0].len = 1;
    msg[0].buf = &dbuf[0];
    dbuf[0] = reg;

    /* Now read the data */
    msg[1].flags = I2C_M_RD;
    msg[1].len = 2;
    msg[1].buf = &dbuf[2];

    /* On I2C bus, we receive LS byte first. So swap bytes as necessary */
    ret = i2c_transfer(myclient->adapter, msg, 2);
    *data = (dbuf[2] << 8) | dbuf[3];

    return ret == 2;
}
EXPORT_SYMBOL(sfp_i2c_phy_read);

static int sfp_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int err = 0;


    myclient = client;
    return err;
}

static int sfp_i2c_detect(struct i2c_client *client, struct i2c_board_info *info)
{
    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "\nEntering the function %s \n", __FUNCTION__);
    strcpy(info->type, "sfp_i2c");
    info->flags = 0;
    return 0;
}

static int sfp_i2c_remove(struct i2c_client *client)
{
    int err = 0;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "\nEntering the function %s \n", __FUNCTION__);

    kfree(i2c_get_clientdata(client));

    return err;
}

static void __exit sfp_i2c_exit(void)
{
    i2c_del_driver(&sfp_i2c_driver);
}

module_init(sfp_i2c_init);
module_exit(sfp_i2c_exit);

