/*
   Copyright (c) 2019 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2019:DUAL/GPL:standard

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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <bcmspivoice.h>
#include <boardparms_voice.h>

struct bcm_spidev
{
    struct gpio_desc *reset_gpio;
    uint32_t spi_index;
    uint32_t slaveid;
};

#define MAX_BUS 2
#define MAX_SLAVES 10
static struct device *slaves[MAX_BUS][MAX_SLAVES];

static int insert_slave(int bus, int slave_id, struct device *dev)
{
    if (slaves[bus][slave_id])
        return -EEXIST;

    slaves[bus][slave_id] = dev;
    return 0;
}

static struct device *find_slave_dev(int bus, int slave_id)
{
    return slaves[bus][slave_id];
}

static void find_slave_dev_remove(struct device *dev)
{
    struct spi_device *spi = to_spi_device(dev);
    int bus = spi->master->bus_num;

    slaves[bus][spi->chip_select] = NULL;
}

static ssize_t bcm_sync(struct device *dev, unsigned char *txBuf, unsigned char *rxBuf, int prependcnt, size_t nbytes)
{
    struct spi_transfer xfer[2];
    int status;
    struct spi_message message;
    struct spi_device *spi = to_spi_device(dev);

    spi_message_init(&message);
    memset(xfer, 0, (sizeof xfer));
#if defined(CONFIG_SPI_BCM63XX_HSSPI)
    if (prependcnt > 0) 
    { 
         xfer[0].tx_buf = txBuf;
         xfer[0].len = prependcnt;
         xfer[0].speed_hz = spi->max_speed_hz;
         spi_message_add_tail(&xfer[0], &message);

         xfer[1].rx_buf = rxBuf;
         xfer[1].len = nbytes;
         xfer[1].speed_hz = spi->max_speed_hz;
         spi_message_add_tail(&xfer[1], &message);
     }
     else
     {
         xfer[0].tx_buf = txBuf;
         xfer[0].rx_buf = rxBuf;
         xfer[0].len = nbytes;
         xfer[0].speed_hz = spi->max_speed_hz;
         spi_message_add_tail(&xfer[0], &message);
     }
     status = spi_sync(spi, &message);
#else
     xfer[0].prepend_cnt = prependcnt;
     xfer[0].len = nbytes;
     xfer[0].speed_hz = spi->max_speed_hz;
     xfer[0].rx_buf = rxBuf;
     xfer[0].tx_buf = txBuf;

     spi_message_add_tail(&xfer[0], &message);

    /* the controller does not support asynchronous transfer
       when spi_async returns the transfer will be complete
       don't use spi_sync to avoid the call to schedule */
     status = spi_async(spi, &message);
#endif

    if (status >= 0)
        return SPI_STATUS_OK;

    return SPI_STATUS_ERR;
}

static int spi_voice_probe(struct spi_device *spi)
{
    struct device_node *np = spi->dev.of_node;
    struct bcm_spidev *pdata;
    int bus = spi->master->bus_num;
    int ret;

    if (!np)
        return -ENODEV;

    if (insert_slave(bus, spi->chip_select, &spi->dev))
        return -EEXIST;

    pdata = devm_kzalloc(&spi->dev, sizeof(*pdata), GFP_KERNEL);
    if (!pdata)
        return -ENOMEM;

    dev_set_drvdata(&spi->dev, pdata);

    ret = device_property_read_u32(&spi->dev,"spi-index",&pdata->spi_index);
    if (ret)
    {
        dev_err(&spi->dev, "voice missing index number.");
        return ret;
    }

    pdata->reset_gpio = devm_gpiod_get_optional(&spi->dev, "reset", 0);
    if (IS_ERR(pdata->reset_gpio))
    {
        dev_err(&spi->dev, "voice reset GPIO request failed: %ld\n", PTR_ERR(pdata->reset_gpio));
        goto err_free;
    }

    if (pdata->reset_gpio)
        gpiod_direction_output(pdata->reset_gpio, 0);

    get_device(&spi->dev);
err_free:
    return 0;
}

static int spi_voice_remove(struct spi_device *spi)
{
    find_slave_dev_remove(&spi->dev);
    put_device(&spi->dev);
    return 0;
}

static const struct spi_device_id spi_voice_id_table[] = {
    { "bcm-spi-voice" },
    {},
};
MODULE_DEVICE_TABLE(spi, spi_voice_id_table);

static struct spi_driver spi_voice_driver = {
    .driver = {
        .owner = THIS_MODULE,
        .name = "bcm-spi-voice",
    },
    .probe = spi_voice_probe,
    .remove = spi_voice_remove,
    .id_table = spi_voice_id_table,
};
module_spi_driver(spi_voice_driver);

int bcm_spi_voice_trans(unsigned char *txBuf, unsigned char *rxBuf, int prependcnt, int nbytes, int busNum, int slaveId)
{
    struct device *slave = find_slave_dev(busNum, slaveId);

    if (!slave)
        return SPI_STATUS_ERR;

    if (bcm_sync(slave, txBuf, rxBuf, prependcnt, prependcnt + nbytes) < 0)
        return SPI_STATUS_ERR;

    return SPI_STATUS_OK;
}
EXPORT_SYMBOL(bcm_spi_voice_trans);

int bcm_spi_voice_reserve_dev_ex(int busNum, int slaveId, int maxFreq, int spiMode, int ctrlState)
{
    struct device *slave = find_slave_dev(busNum, slaveId);
    struct spi_device *spi;

    if (!slave)
        return SPI_STATUS_ERR;

    spi = to_spi_device(slave);
    spi->mode = spiMode;
    spi->max_speed_hz = maxFreq;
    spi->controller_data = (void *)(uintptr_t)ctrlState;
    if (spi_setup(spi))
        return SPI_STATUS_ERR;

    return SPI_STATUS_OK;
}
EXPORT_SYMBOL(bcm_spi_voice_reserve_dev_ex);

int bcm_spi_voice_reserve_dev(int busNum, int slaveId, int maxFreq)
{
    return bcm_spi_voice_reserve_dev_ex(busNum, slaveId, maxFreq,
        SPI_MODE_DEFAULT, SPI_CONTROLLER_STATE_DEFAULT);
}
EXPORT_SYMBOL(bcm_spi_voice_reserve_dev);

int bcm_spi_voice_release_dev(int busNum, int slaveId)
{
    struct device *slave = find_slave_dev(busNum, slaveId);

    if (!slave)
        return SPI_STATUS_ERR;

    return SPI_STATUS_OK;
}
EXPORT_SYMBOL(bcm_spi_voice_release_dev);

int bcm_spi_voice_reset_dev(int busNum, int slaveId, int active)
{
    struct device *slave = find_slave_dev(busNum, slaveId);
    struct bcm_spidev *pdata;

    if (!slave)
        return SPI_STATUS_ERR;

    pdata = dev_get_drvdata(slave);
    if (pdata->reset_gpio)
        gpiod_direction_output(pdata->reset_gpio, active);

    return SPI_STATUS_OK;
}
EXPORT_SYMBOL(bcm_spi_voice_reset_dev);

static struct bcm_spidev *bcm_spi_get_priv(int index)
{
    int busNum,slaveid;
    struct device *dev;
    struct bcm_spidev *priv = NULL;

    for (busNum = 0; busNum < MAX_BUS; busNum++) {
        for (slaveid = 0; slaveid < MAX_SLAVES; slaveid++) {
            dev = slaves[busNum][slaveid];
            if (!dev)
                continue;
            priv = dev_get_drvdata(dev);
            if (priv->spi_index == index) {
                priv->slaveid = slaveid;
                return priv;
            }
        }
    }
    return priv;
}

int bcm_spi_get_slaveid(int index)
{
    struct bcm_spidev *priv = bcm_spi_get_priv(index);
    if (priv)
        return priv->slaveid;
    return BP_NOT_DEFINED;
}
EXPORT_SYMBOL(bcm_spi_get_slaveid);

static struct gpio_desc *bcm_spi_get_reset_gpio(int index)
{
    struct bcm_spidev *priv = bcm_spi_get_priv(index);
    if (priv)
        return priv->reset_gpio;
    return NULL;
}

void bcm_spi_voice_device_reset(int index,unsigned int active_delay, unsigned int post_delay)
{
    struct gpio_desc *gpio = bcm_spi_get_reset_gpio(index);
    if (!gpio) 
        return;

    gpiod_direction_output(gpio, 0);
    msleep(5);  
    gpiod_direction_output(gpio, 1);
    msleep(active_delay);
    gpiod_direction_output(gpio, 0);
    msleep(post_delay);
}
EXPORT_SYMBOL(bcm_spi_voice_device_reset);

void bcm_spi_voice_reset_all_dev(int active)
{
    int busNum,slaveid;

    for (busNum = 0; busNum < MAX_BUS; busNum++) {
        for (slaveid = 0; slaveid < MAX_SLAVES; slaveid++) {
          if( bcm_spi_voice_reset_dev(busNum, slaveid, active) == SPI_STATUS_ERR )
             continue;
        }
    }
}
EXPORT_SYMBOL(bcm_spi_voice_reset_all_dev);
