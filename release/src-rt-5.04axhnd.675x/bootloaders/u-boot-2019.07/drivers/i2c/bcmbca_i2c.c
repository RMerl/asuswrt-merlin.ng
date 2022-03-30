// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Broadcom
 */
/*
 *
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <dm/device.h>
#include <linux/io.h>

#define CLK_375K    0
#define CLK_390K    1
#define CLK_187_5K  2 //187.5K
#define CLK_200K    3
#define CLK_93_75K  4 //93.75K
#define CLK_97_5K   5
#define CLK_46_875K 6 //46.875K
#define CLK_50K     7
#define I2C_CTL_REG_SCL_SEL_S             4
#define I2C_CTL_REG_SCL_SEL_M             3
#define I2C_CTL_REG_DIV_CLK_S             7
#define I2C_CTL_REG_DIV_CLK_M             1
#define I2C_CTLHI_REG_DATA_REG_SIZE_SHIFT 6

static int i2c_speed[] = {375000, 390000, 187500,200000, 93750, 97500, 46875, 50000};
static int data_reg_len = 1;

#define TLOW_SEXT  25000
#define TLOW_MEXT  10000
#define MAX_CLKS_PER_TRANSACTION  306; /* 34 * 9 */
#define CLK_375K_TIME 3
#define CLK_187_5K_TIME 6

struct bcmbca_i2c_regs {
  u32        ChipAddress;            /* 0x0 */
#define I2C_CHIP_ADDRESS_MASK           0x000000f7
#define I2C_CHIP_ADDRESS_SHIFT          0x1
  u32        DataIn0;                /* 0x4 */
  u32        DataIn1;                /* 0x8 */
  u32        DataIn2;                /* 0xc */
  u32        DataIn3;                /* 0x10 */
  u32        DataIn4;                /* 0x14 */
  u32        DataIn5;                /* 0x18 */
  u32        DataIn6;                /* 0x1c */
  u32        DataIn7;                /* 0x20 */
  u32        CntReg;                 /* 0x24 */
#define I2C_CNT_REG1_SHIFT              0x0
#define I2C_CNT_REG2_SHIFT              0x6
  u32        CtlReg;                 /* 0x28 */
#define I2C_CTL_REG_DTF_MASK            0x00000003
#define I2C_CTL_REG_DTF_WRITE           0x0
#define I2C_CTL_REG_DTF_READ            0x1
#define I2C_CTL_REG_DTF_READ_AND_WRITE  0x2
#define I2C_CTL_REG_DTF_WRITE_AND_READ  0x3
#define I2C_CTL_REG_DEGLITCH_DISABLE    0x00000004
#define I2C_CTL_REG_DELAY_DISABLE       0x00000008
#define I2C_CTL_REG_SCL_SEL_MASK        0x00000030
#define I2C_CTL_REG_SCL_CLK_375KHZ      0x00000000
#define I2C_CTL_REG_SCL_CLK_390KHZ      0x00000010
#define I2C_CTL_REG_SCL_CLK_187_5KHZ    0x00000020
#define I2C_CTL_REG_SCL_CLK_200KHZ      0x00000030
#define I2C_CTL_REG_INT_ENABLE          0x00000040
#define I2C_CTL_REG_DIV_CLK             0x00000080
  u32        IICEnable;              /* 0x2c */
#define I2C_IIC_ENABLE                  0x00000001
#define I2C_IIC_INTRP                   0x00000002
#define I2C_IIC_NO_ACK                  0x00000004
#define I2C_IIC_NO_STOP                 0x00000010
#define I2C_IIC_NO_START                0x00000020
  u32        DataOut0;               /* 0x30 */
  u32        DataOut1;               /* 0x34 */
  u32        DataOut2;               /* 0x38 */
  u32        DataOut3;               /* 0x3c */
  u32        DataOut4;               /* 0x40 */
  u32        DataOut5;               /* 0x44 */
  u32        DataOut6;               /* 0x48 */
  u32        DataOut7;               /* 0x4c */
  u32        CtlHiReg;               /* 0x50 */
#define I2C_CTLHI_REG_WAIT_DISABLE      0x00000001
#define I2C_CTLHI_REG_IGNORE_ACK        0x00000002
#define I2C_CTLHI_REG_DATA_REG_SIZE     0x00000040
  u32        SclParam;               /* 0x54 */
};

static struct bcmbca_i2c_priv {
    struct bcmbca_i2c_regs *regs;
    int clk_sel;
    int speed;
    int xfer_timeout;
};

static inline int wait_xfer_done(struct bcmbca_i2c_priv* priv)
{
    struct bcmbca_i2c_regs *reg = priv->regs;
    int i, temp = 0;

    for (i = 0; i < priv->xfer_timeout; i++)
    {
        temp = readl((u32*)&reg->IICEnable);
        if (temp & I2C_IIC_NO_ACK)
        {
            debug("%s: No Ack received\n", __func__);
            return -ETIMEDOUT;
        }
        /* The xfer is complete and successful when INTRP is set
           and NOACK is not set */
        if ((temp & I2C_IIC_INTRP) && !(temp & I2C_IIC_NO_ACK))
        {
            return 0;
        }
    }

    return -ETIMEDOUT;
}


static inline void i2c_rd_wr_common(struct bcmbca_i2c_priv* priv, u8 chip_addr, u16 xfer_cnt, u8 dtf)
{
    struct bcmbca_i2c_regs *reg = priv->regs;
    unsigned int data;

    /* Write the Address */
    writel(chip_addr << I2C_CHIP_ADDRESS_SHIFT,  (u32*)&reg->ChipAddress);

    /* Write the Transfer Count */
    writel(xfer_cnt, (u32*)&reg->CntReg);

    /* Set the Control Register Data Transfer Format */
    data = readl((u32*)&reg->CtlReg);
    data &=  ~I2C_CTL_REG_DTF_MASK;
    data |= dtf;
    writel(data, (u32*)&reg->CtlReg);
}

static int i2c_read(struct bcmbca_i2c_priv* priv, struct i2c_msg *p, int fifo_len)
{
    struct bcmbca_i2c_regs *reg = priv->regs;
    int i, j, ret = 0;
    u16 xfer_cnt;
    unsigned int len = p->len;
    unsigned char *buf = p->buf;
    unsigned char addr = p->addr;
    int loops = (len + (fifo_len - 1))/fifo_len;
    int i2c_enable_val = 0;

    for (i = 0; i < loops; i++)
    {

        /* Make sure IIC_Enable bit is zero before start of a new transaction.
           Note that a 0-1 transition of IIC_Enable is required to initiate
           an I2C bus transaction */

        writel(0, (u32*)&reg->IICEnable);

        /* Transfer size for the last iteration will be the remaining size */
        xfer_cnt = (i==loops-1) ? (len - (i * fifo_len)) : fifo_len;

        /* Set the address, transfer count, and data transfer format */
        i2c_rd_wr_common(priv, addr, xfer_cnt, I2C_CTL_REG_DTF_READ);

        /* Start the I2C transaction */
        i2c_enable_val = I2C_IIC_ENABLE;

        if(loops > 1)      /* multiple loops */
        {
           if(i == 0)      /* first loop, with start and without stop */
           {
              i2c_enable_val |= I2C_IIC_NO_STOP;
           }
           else if(i == (loops - 1))     /* last loop, without start[also without phy address, r/w bits], with stop */
           {
              i2c_enable_val |= I2C_IIC_NO_START;
           }
           else            /* middle loops, without start[also without phy address, r/w bits], without stop */
           {
              i2c_enable_val |= I2C_IIC_NO_START | I2C_IIC_NO_STOP;
           }
        }

        writel(i2c_enable_val, (u32*)&reg->IICEnable);

        /* Wait for completion */
        if (wait_xfer_done(priv))
        {
            ret = -ETIMEDOUT;
            goto end;
        }

        /* Read the data */
        if (fifo_len == 8)
        {
            for (j = 0; j < xfer_cnt; j++)
                buf[i*fifo_len + j] = (u8)readl((u32*)&reg->DataOut0 + j);
        }
        else
        {
            int num_dwords = (xfer_cnt)/4;
            int left_over = (xfer_cnt)%4;
            int temp_data=0;
            for (j = 0; j < num_dwords; j++)
            {
                *((int *)&buf[i*fifo_len + j*4]) = readl((u32*)&reg->DataOut0 + j);
            }
            if(left_over)
            {
                temp_data = readl((u32*)&reg->DataOut0 + j);
                memcpy((char*)&buf[i*fifo_len + j*4], (char*)&temp_data, left_over);
            }
        }
    }
end:
    /* I2C bus transaction is complete, so set the IIC_Enable to zero */
    writel(0, (u32*)&reg->IICEnable);
    return ret;
}


static int i2c_write(struct bcmbca_i2c_priv* priv, struct i2c_msg *p, int fifo_len)
{
    struct bcmbca_i2c_regs *reg = priv->regs;
    int i, j, ret = 0;
    u16 xfer_cnt;
    unsigned int len = p->len;
    unsigned char *buf = p->buf;
    unsigned char addr = p->addr;
    int loops = (len + (fifo_len - 1))/fifo_len;
    int i2c_enable_val = 0;

    if (loops == 0)
    {
        i2c_rd_wr_common(priv, addr, 0, I2C_CTL_REG_DTF_WRITE);

        /* Start the I2C transaction */
        writel(I2C_IIC_ENABLE, (u32*)&reg->IICEnable);

        /* Wait for completion */
        ret = wait_xfer_done(priv);

        goto end;
    }


    for (i = 0; i < loops; i++)
    {
        /* Make sure IIC_Enable bit is zero before start of a new transaction.
           Note that a 0-1 transition of IIC_Enable is required to initiate
           an I2C bus transaction */

        writel(0, (u32*)&reg->IICEnable);

        /* Transfer size for the last iteration will be the remaining size */
        xfer_cnt = (i==loops-1) ? (len - (i * fifo_len)) : fifo_len;

        /* Set the address, transfer count, and data transfer format */
        i2c_rd_wr_common(priv, addr, xfer_cnt, I2C_CTL_REG_DTF_WRITE);

        /* Write the data */
        if (fifo_len == 8)
        {
            for (j = 0; j < xfer_cnt; j++)
                writel(buf[i * fifo_len + j], (u32*)&reg->DataIn0 + j);
        }
        else
        {
            int num_dwords = (xfer_cnt + 3)/4;
            for (j = 0; j < num_dwords; j++)
            {
                writel(*((int *)&buf[i * fifo_len + j*4]), (u32*)&reg->DataIn0 + j);
            }
        }

        /* Start the I2C transaction */

        i2c_enable_val = I2C_IIC_ENABLE;

        if(loops > 1)      /* multiple loops */
        {
           if(i == 0)      /* first loop, with start and without stop */
           {
              i2c_enable_val |= I2C_IIC_NO_STOP;
           }
           else if(i == (loops - 1))     /* last loop, without start[also without phy address, r/w bits], with stop */
           {
              i2c_enable_val |= I2C_IIC_NO_START;
           }
           else            /* middle loops, without start[also without phy address, r/w bits], without stop */
           {
              i2c_enable_val |= I2C_IIC_NO_START | I2C_IIC_NO_STOP;
           }
        }

        writel(i2c_enable_val, (u32*)&reg->IICEnable);

        /* Wait for completion */
        if (wait_xfer_done(priv))
        {
            ret = -ETIMEDOUT;
            goto end;
        }
    }

end:
    /* I2C bus transaction is complete, so set the IIC_Enable to zero */
    writel(0, (u32*)&reg->IICEnable);
    return ret;
}


/* read followed by write without a stop in between */
static int i2c_read_then_write(struct bcmbca_i2c_priv* priv, struct i2c_msg *msgs, int fifo_len)
{
    struct bcmbca_i2c_regs *reg = priv->regs;
    int j, ret = 0;
    u16 xfer_cnt;
    struct i2c_msg *p = &msgs[0], *q = &msgs[1];

    /* Make sure IIC_Enable bit is zero before start of a new transaction.
       Note that a 0-1 transition of IIC_Enable is required to initiate
       an I2C bus transaction */
    writel(0, (u32*)&reg->IICEnable);

    /* Set the transfer counts for both the transactions */
    xfer_cnt = (q->len << I2C_CNT_REG2_SHIFT) | p->len;

    /* Set the address, transfer count, and data transfer format */
    i2c_rd_wr_common(priv, (u8)p->addr, xfer_cnt, I2C_CTL_REG_DTF_READ_AND_WRITE);

    /* Write the data */
    if (fifo_len == 8)
    {
        for (j = 0; j < (q->len); j++)
            writel(q->buf[j], (u32*)&reg->DataIn0 + j);
    }
    else
    {
        int num_dwords = ((q->len) + 3)/4;
        for (j = 0; j < num_dwords; j++)
        {
            writel(*((int *)&q->buf[j*4]), (u32*)&reg->DataIn0 +j);
        }
    }

    /* Start the I2C transaction */
    writel(I2C_IIC_ENABLE, (u32*)&reg->IICEnable);

    /* Wait for completion */
    if (wait_xfer_done(priv))
    {
        ret = -ETIMEDOUT;
        goto end;
    }

    /* Read the data */
    if (fifo_len == 8)
    {
        for (j = 0; j < (p->len); j++)
            p->buf[j] = (u8)readl((u32*)&reg->DataOut0 + j);
    }
    else
    {
        int num_dwords = (p->len)/4;
        int left_over = (p->len)%4;
        int temp_data=0;
        for (j = 0; j < num_dwords; j++)
        {
            *((int *)&p->buf[j*4]) = readl((u32*)&reg->DataOut0 + j);
        }
        if(left_over)
        {
            temp_data = readl((u32*)&reg->DataOut0 + j);
            memcpy((char*)&p->buf[j*4], (char*)&temp_data, left_over);
        }
    }

end:
    /* I2C bus transaction is complete, so set the IIC_Enable to zero */
    writel(0, (u32*)&reg->IICEnable);
    return ret;
}


/* write followed by read without a stop in between */
static int i2c_write_then_read(struct bcmbca_i2c_priv* priv, struct i2c_msg *msgs, int fifo_len)
{
    struct bcmbca_i2c_regs *reg = priv->regs;
    int j, ret = 0;
    u16 xfer_cnt;
    struct i2c_msg *p = &msgs[0], *q = &msgs[1];

    /* Make sure IIC_Enable bit is zero before start of a new transaction.
       Note that a 0-1 transition of IIC_Enable is required to initiate
       an I2C bus transaction */
    writel(0, (u32*)&reg->IICEnable);

    /* Set the transfer counts for both the transactions */
    xfer_cnt = (q->len << I2C_CNT_REG2_SHIFT) | p->len;

    /* Set the address, transfer count, and data transfer format */
    i2c_rd_wr_common(priv, (u8)p->addr, xfer_cnt, I2C_CTL_REG_DTF_WRITE_AND_READ);

    /* Write the data */
    if (fifo_len == 8)
    {
        for (j = 0; j < (p->len); j++)
            writel(p->buf[j], (u32*)&reg->DataIn0);
    }
    else
    {
        int num_dwords = ((p->len) + 3)/4;
        for (j = 0; j < num_dwords; j++)
        {
            int data = 0;
            data |= ((j * 4 + 3) < p->len ? p->buf[j * 4 + 3] : 0) << 24;
            data |= ((j * 4 + 2) < p->len ? p->buf[j * 4 + 2] : 0) << 16;
            data |= ((j * 4 + 1) < p->len ? p->buf[j * 4 + 1] : 0) << 8;
            data |= p->buf[j * 4];
            writel(data, (u32*)&reg->DataIn0 + j);
        }
    }

    /* Start the I2C transaction */
    writel(I2C_IIC_ENABLE, (u32*)&reg->IICEnable);

    /* Wait for completion */
    if (wait_xfer_done(priv))
    {
        ret = -ETIMEDOUT;
        goto end;
    }

    /* Read the data */
    if (fifo_len == 8)
    {
        for (j = 0; j < (q->len); j++)
            q->buf[j] = (u8)readl((u32*)&reg->DataOut0 + j);
    }
    else
    {
        int num_dwords = (q->len)/4;
        int left_over = (q->len)%4;
        int temp_data=0;
        for (j = 0; j < num_dwords; j++)
        {
            *((int *)&q->buf[j*4]) =
                readl((u32*)&reg->DataOut0 + j);
        }
        if(left_over)
        {
            temp_data = readl((u32*)&reg->DataOut0 + j);
            memcpy((char*)&q->buf[j*4], (char*)&temp_data, left_over);
        }
    }

end:
    /* I2C bus transaction is complete, so set the IIC_Enable to zero */
    writel(0, (u32*)&reg->IICEnable);
    return ret;
}




static int bcmbca_i2c_xfer(struct udevice *dev, struct i2c_msg *msgs, int nmsgs)
{
    struct bcmbca_i2c_priv *priv = dev_get_priv(dev);
    struct bcmbca_i2c_regs *reg = priv->regs;
    struct i2c_msg *p, *q;
    int i, ret = 0;

    int fifo_len = (readl((u32*)&reg->CtlHiReg) & I2C_CTLHI_REG_DATA_REG_SIZE) ? 32 : 8;

    if (nmsgs == 2)
    {
        p = &msgs[0]; q = &msgs[1];
        /* We may be able to use the read_then_write and write_then_read
        formats only when the length of each message is less than fifo_len */
        if (p->len <= fifo_len && q->len <= fifo_len)
        {
            if ((p->flags == I2C_M_RD) && (!(q->flags & I2C_M_RD)))
            {
                return i2c_read_then_write(priv, msgs, fifo_len);
            }
            else if ((!(p->flags & I2C_M_RD)) && (q->flags == I2C_M_RD))
            {
                return  i2c_write_then_read(priv, msgs, fifo_len);
            }
        }
        else
        {
            debug("%s: msgs len %d %d is too long\n", __func__, p->len, q->len);
            return -EPROTO;
        }
    }

    /*TBD:See if we can use NO_STOP b/w multiple read and write transactions */
    for (i = 0; !ret && i < nmsgs; i++)
    {
        p = &msgs[i];
        if (p->flags & I2C_M_RD)
            ret = i2c_read(priv, p, fifo_len);
        else
            ret = i2c_write(priv, p, fifo_len);
    }

    return ret;
}


static int bcmbca_i2c_set_clk(struct bcmbca_i2c_priv* priv)
{
    struct bcmbca_i2c_regs *reg = priv->regs;
    int clk_sel = priv->clk_sel;
    int val, clk_time = 1000;

    /* Set the clock selection */
    if ( (clk_sel < 0) || (clk_sel > 7)) {
        printf("%s: Invalid clock selection. Setting the clk to 97.5K\n", __func__);
        clk_sel = CLK_97_5K;
    }

    val = (clk_sel & I2C_CTL_REG_SCL_SEL_M) << I2C_CTL_REG_SCL_SEL_S;
    if (clk_sel > 3) {
        val |= (1 << I2C_CTL_REG_DIV_CLK_S);
    }

    writel(val, (u32*)&reg->CtlReg);
    val = readl((u32*)&reg->CtlReg);

    switch (clk_sel) {
       case 0:
       case 1:
       case 4:
       case 5:
            /* approx clock time in uS */
            clk_time = CLK_375K_TIME * ((clk_sel>I2C_CTL_REG_SCL_SEL_M)?4:1);
            break;

       case 2:
       case 3:
       case 6:
       case 7:
            /* approx clock time in uS */
            clk_time = CLK_187_5K_TIME * ((clk_sel>I2C_CTL_REG_SCL_SEL_M)?4:1);
            break;
    }

    /* xfer_timeout in mS */
    priv->xfer_timeout = TLOW_SEXT + TLOW_MEXT + clk_time * MAX_CLKS_PER_TRANSACTION;
    /* Set the Data Register Length */
    writel((data_reg_len << I2C_CTLHI_REG_DATA_REG_SIZE_SHIFT), (u32*)&reg->CtlHiReg);

    return 0;
}

static int bcmbca_i2c_probe(struct udevice *dev)
{
    struct bcmbca_i2c_priv *priv = dev_get_priv(dev);

    priv->speed = i2c_speed[priv->clk_sel];
    bcmbca_i2c_set_clk(priv);

    return 0;
}


static int bcmbca_i2c_set_bus_speed(struct udevice *dev, unsigned int speed)
{
    struct bcmbca_i2c_priv *priv = dev_get_priv(dev);
    int i = 0;

    while (i<8)
    {
        if (i2c_speed[i] == speed)
        {
            priv->speed = speed;
            priv->clk_sel = i;
            break;
        }
        i++;
    }

    if (i==8)
    {
        if (speed == 100000)
        {
            priv->speed = 97500;
            priv->clk_sel = 5;
        }
        else
        {
            printf("%s: Speed %d not supported\n", __func__, speed);
            return -EINVAL;
        }
    }

    bcmbca_i2c_set_clk(priv);

    return 0;
}

int bcmbca_i2c_get_bus_speed(struct udevice *dev)
{
    struct bcmbca_i2c_priv *priv = dev_get_priv(dev);

    return priv->speed;
}

static int bcmbca_i2c_ofdata_to_platdata(struct udevice *dev)
{
    const void *blob = gd->fdt_blob;
    struct bcmbca_i2c_priv *priv = dev_get_priv(dev);
    int node = dev_of_offset(dev);

    priv->regs = (struct bcmbca_i2c_regs *)devfdt_get_addr(dev);
    priv->clk_sel = fdtdec_get_int(blob, node, "clk_sel", 5);

    return 0;
}

static const struct dm_i2c_ops bcmbca_i2c_ops = {
    .xfer       = bcmbca_i2c_xfer,
    .set_bus_speed  = bcmbca_i2c_set_bus_speed,
    .get_bus_speed  = bcmbca_i2c_get_bus_speed,
};

static const struct udevice_id bcmbca_i2c_ids[] = {
{ .compatible = "brcm,bcm63000-i2c", },
{ }
};

U_BOOT_DRIVER(i2c_bcmbca) = {
    .name   = "i2c_bcmbca",
    .id = UCLASS_I2C,
    .of_match = bcmbca_i2c_ids,
    .probe = bcmbca_i2c_probe,
    .ofdata_to_platdata = bcmbca_i2c_ofdata_to_platdata,
    .priv_auto_alloc_size = sizeof(struct bcmbca_i2c_priv),
    .ops    = &bcmbca_i2c_ops,
};
