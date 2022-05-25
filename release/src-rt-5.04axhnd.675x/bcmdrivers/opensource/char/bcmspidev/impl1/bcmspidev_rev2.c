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

struct rev_fn
{
    int (*init)(struct device *dev);
    int (*read)(struct device *dev, unsigned int addr, unsigned int *buf, size_t len);
    int (*write)(void);
    int (*write_buf)(struct device *dev, unsigned int addr, unsigned int *buf, size_t len, int unit_size);
};

struct bcm_spidev
{
    struct rev_fn *rev;
    struct gpio_desc *reset_gpio;
};

#define LEGACY
#ifdef LEGACY
#include <bcmSpiRes.h>

#define MAX_SLAVES 10
static struct device *slaves[MAX_SLAVES];

static struct device *insert_slave(struct device *dev)
{
    int i;

    for (i = 0; i < MAX_SLAVES; i++)
    {
        if (slaves[i])
            continue;

        break;
    }

    if (slaves[i])
        return NULL;

    slaves[i] = dev;

    return dev;
}

static void find_slave_dev_remove(struct device *dev)
{
    int i;

    for (i = 0; i < MAX_SLAVES; i++)
    {
        if (slaves[i] == dev)
            slaves[i] = NULL;
    }
}

static struct device *find_slave_dev(int slave_id)
{
    return slaves[slave_id];
}

int compat_kerSysBcmSpiSlaveInit(int dev)
{
    struct device *slave_dev = find_slave_dev(dev);
    struct bcm_spidev *pdata;

    if (!slave_dev)
        return SPI_STATUS_ERR;

    pdata = dev_get_drvdata(slave_dev);
    if (pdata->rev->init && pdata->rev->init(slave_dev))
        return SPI_STATUS_ERR;

    return SPI_STATUS_OK;
}

int compat_kerSysBcmSpiSlaveRead(int dev, unsigned int addr, unsigned int *data, unsigned int len)
{
    struct device *slave_dev = find_slave_dev(dev);
    struct bcm_spidev *pdata;

    if (!slave_dev)
        return SPI_STATUS_ERR;

    pdata = dev_get_drvdata(slave_dev);
    if (pdata->rev->read && pdata->rev->read(slave_dev, addr, data, len))
    {
        dev_err(slave_dev, "rev_read failed\n");
        return SPI_STATUS_ERR;
    }

    return SPI_STATUS_OK;
}

int compat_kerSysBcmSpiSlaveWrite(int dev, unsigned int addr, unsigned int data, unsigned int len)
{
    return SPI_STATUS_ERR;
}

int compat_kerSysBcmSpiSlaveWriteBuf(int dev, unsigned int addr, unsigned int *data, unsigned int len, unsigned int unitSize)
{
    struct device *slave_dev = find_slave_dev(dev);
    struct bcm_spidev *pdata;

    if (!slave_dev)
        return SPI_STATUS_ERR;

    pdata = dev_get_drvdata(slave_dev);
    if (pdata->rev->write_buf && pdata->rev->write_buf(slave_dev, addr, data, len, unitSize))
    {
        dev_err(slave_dev, "rev_write_buf failed\n");
        return SPI_STATUS_ERR;
    }

    return SPI_STATUS_OK;
}
#endif

ssize_t bcm_sync(struct device *dev, unsigned char *txBuf, unsigned char *rxBuf, int prependcnt, size_t nbytes)
{
    struct spi_transfer xfer[2];
    int                 status;
    struct spi_message  message;
    struct spi_device   *spi = to_spi_device(dev);

    spi_message_init(&message);
    memset(xfer, 0, (sizeof xfer));
#if defined(CONFIG_SPI_BCM63XX_HSSPI)
    if( prependcnt > 0 ) 
    { 
         xfer[0].tx_buf = txBuf;
         xfer[0].len = prependcnt;
         xfer[0].speed_hz = spi->max_speed_hz;
         spi_message_add_tail(&xfer[0], &message);

         xfer[1].rx_buf = rxBuf;
         xfer[1].len = nbytes;
         xfer[1].speed_hz  = spi->max_speed_hz;
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
     xfer[0].len         = nbytes;
     xfer[0].speed_hz    = spi->max_speed_hz;
     xfer[0].rx_buf      = rxBuf;
     xfer[0].tx_buf      = txBuf;

     spi_message_add_tail(&xfer[0], &message);

    /* the controller does not support asynchronous transfer
       when spi_async returns the transfer will be complete
       don't use spi_sync to avoid the call to schedule */
     status = spi_async(spi, &message);
#endif

    if (status >= 0)
        status = SPI_STATUS_OK;
    else
        status = SPI_STATUS_ERR;

    return( status );
}

static void rev2_reset(struct device *dev)
{
    struct bcm_spidev *pdata = dev_get_drvdata(dev);

    if (!pdata->reset_gpio)
        return;

    gpiod_direction_output(pdata->reset_gpio, 1);
    mdelay(100);
    gpiod_direction_output(pdata->reset_gpio, 0);
    mdelay(100);
}

static int rev2_init(struct device *dev)
{
    rev2_reset(dev);
    return 0;
}

typedef enum {
    SPI_WRITE = 0x02,
    SPI_READ = 0x03,
    SPI_FAST_READ = 0x0b
} spi_command_rev2;

// Add spi command and address to a buffer, return prepend count.
// In case of write, prepend count is not used by hardware.
static int rev2_add_command_and_address_to_buffer(uint8_t *buf, spi_command_rev2 cmd, unsigned int addr)
{
    buf[0] = cmd;
    buf[1] = addr >> 8;
    buf[2] = addr & 0xff;
    return (cmd == SPI_FAST_READ? 4: 3);
}

int rev2_read(struct device *dev, unsigned int addr, unsigned int *data, size_t len)
{
    uint8_t buf[8] = {0, 0, 0, 0, 0, 0, 0};
    int prependcnt;

    prependcnt = rev2_add_command_and_address_to_buffer(buf, SPI_READ, addr);
    if (bcm_sync(dev, buf, buf, prependcnt, prependcnt + len) < 0)
    {
        dev_err(dev, "rev2_read failed\n");
        return -1;
    }

    *data = (uint32_t) le32_to_cpu(*((uint32_t *)buf));

    return 0;
}

int rev2_write_buf(struct device *dev, unsigned int addr, unsigned int *data, size_t len, int unit_size)
{
    uint8_t buf[7] = {0, 0, 0, 0, 0, 0, 0};
    uint32_t le32_data = cpu_to_le32(*data);
    int prependcnt = rev2_add_command_and_address_to_buffer(buf, SPI_WRITE, addr);

    buf[prependcnt+0] = (uint8_t) ((le32_data >>  0) & 0xff);
    buf[prependcnt+1] = (uint8_t) ((le32_data >>  8) & 0xff);
    buf[prependcnt+2] = (uint8_t) ((le32_data >> 16) & 0xff);
    buf[prependcnt+3] = (uint8_t) ((le32_data >> 24) & 0xff);
    return bcm_sync(dev, buf, NULL, 0, len + prependcnt) < 0;
}

static struct rev_fn rev2 =
{
    .init = rev2_init,
    .read = rev2_read,
    .write_buf = rev2_write_buf,
};

static int spi_rev_probe(struct spi_device *spi)
{
    struct device_node *np = spi->dev.of_node;
    struct bcm_spidev *pdata;
    int err, rev = 2;
    unsigned long ctrl_state = SPI_CONTROLLER_STATE_SET | SPI_CONTROLLER_STATE_GATE_CLK_SSOFF;

    if (!np)
        return -ENODEV;

    if (!insert_slave(&spi->dev))
        return -EEXIST;

    spi->controller_data = (void*)ctrl_state;

    spi_setup(spi);

    pdata = devm_kzalloc(&spi->dev, sizeof(*pdata), GFP_KERNEL);
    if (!pdata)
        return -ENOMEM;

    switch (rev)
    {
        case 2:
            pdata->rev = &rev2;
            break;
        default:
            dev_err(&spi->dev, "missing rev property\n");
            return -ENODEV;
    }

    dev_set_drvdata(&spi->dev, pdata);

    pdata->reset_gpio = gpiod_get_optional(&spi->dev, "reset", 0);
    if (IS_ERR(pdata->reset_gpio))
    {
        dev_err(&spi->dev, "reset GPIO request failed: %ld\n", PTR_ERR(pdata->reset_gpio));
        goto err_free;
    }

    if (pdata->reset_gpio)
    {
        err = gpiod_export(pdata->reset_gpio, 0);
        if (err)
            return err;

        err = gpiod_export_link(&spi->dev, "spi-reset", pdata->reset_gpio);
        if (err)
            return err;
    }

    get_device(&spi->dev);
err_free:
    return 0;
}

static int spi_rev_remove(struct spi_device *spi)
{
    // release gpio
    find_slave_dev_remove(&spi->dev);
    put_device(&spi->dev);
    return 0;
}

static const struct spi_device_id spi_rev_id_table[] = {
    { "bcm-spi-rev2" },
    {},
};
MODULE_DEVICE_TABLE(spi, spi_rev_id_table);

static struct spi_driver spi_rev_driver = {
    .driver = {
        .owner = THIS_MODULE,
        .name = "bcm-spi-rev",
    },
    .probe = spi_rev_probe,
    .remove = spi_rev_remove,
    .id_table = spi_rev_id_table,
};
module_spi_driver(spi_rev_driver);

