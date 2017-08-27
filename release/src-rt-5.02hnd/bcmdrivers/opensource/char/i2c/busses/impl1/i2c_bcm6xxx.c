/* ------------------------------------------------------------------------- */
/* i2c-algo-bcm.c i2c driver algorithms for bcm63000 Embedded I2C adapter     */
/* ------------------------------------------------------------------------- */
/*   Copyright (C) 2008 Pratapa Reddy, Vaka <pvaka@broadcom.com>

<:label-BRCM:2012:DUAL/GPL:standard

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
/* ------------------------------------------------------------------------- */

/* Referenced and Reused the code form i2c-algo-bcm.c file */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/bcm_log.h>
#include <bcm_pinmux.h>

#include "bcm_map_part.h"
#include <bcmtypes.h>

/* TBD: See if we can use the defines in 63000_map.h */
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

/* Default FIFO size of 32 bytes. */
/* If this length is changed to 8, the write byte stream functions  
   of client drivers may not work properly when writing more than 
   8 bytes as some of the i2c client devices require the first byte of
   each I2C write transaction as the offset */
static int data_reg_len = 1;
/* Module Parameters */
/* Default clock selection of 390K Hz. */
static int clk_sel = CLK_97_5K;
static int delay_dis = 0;
static int deglitch_dis = 0;   
static int xfer_timeout;

/* Read the value from given bcm63000 register */
static inline int reg_read(uint32* offset)
{
    int ret;
    ret = *offset;
    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "reg_read: offset = %lx, val is %x\n", 
                  (long int)offset, ret);
    return ret;
}

/* Write the given value to given bcm63000 register */
static inline void reg_write(uint32* offset, int val)
{
    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "reg_write: offset = %lx; val = %x\n", 
                  (long int)offset, val);
    *offset = val;
}

/*  Keep polling until the bcm63000 completes the I2C transaction or timeout */
static inline int wait_xfer_done(void)
{
    int i, temp = 0;

    for (i = 0; i < xfer_timeout; i++) 
    {
        temp = reg_read((uint32 *)&(I2C->IICEnable)); 
        if (temp & I2C_IIC_NO_ACK)
        {
            BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "No Ack received \n");
            return -1;
        }
        /* The xfer is complete and successful when INTRP is set 
           and NOACK is not set */
        if ((temp & I2C_IIC_INTRP) && !(temp & I2C_IIC_NO_ACK))
        {
            return 0;
        }
        /* TBD: Call the schedule() */
        udelay(1);
    }

    return -ETIMEDOUT;
}

static inline void i2c_rd_wr_common(u8 chip_addr, u16 xfer_cnt, u8 dtf)
{
    /* Write the Address */
    reg_write((uint32 *)&I2C->ChipAddress, 
              chip_addr << I2C_CHIP_ADDRESS_SHIFT);

    /* Write the Transfer Count */
    reg_write((uint32 *)&I2C->CntReg, xfer_cnt);

    /* Set the Control Register Data Transfer Format */
    reg_write((uint32 *)&I2C->CtlReg, (reg_read((uint32 *)&I2C->CtlReg) 
              & ~I2C_CTL_REG_DTF_MASK) | dtf);
}

/* Read data of given length from the slave. Note that 2 or more transactions 
   would be required on the bus if the length is more than fifo_len */
static int i2c_read(struct i2c_msg *p, int fifo_len)
{
    int i, j, ret = 0;
    u16 xfer_cnt;
    unsigned int len = p->len;
    unsigned char *buf = p->buf;
    unsigned char addr = p->addr;
    int loops = (len + (fifo_len - 1))/fifo_len;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);
    /* Make sure IIC_Enable bit is zero before start of a new transaction. 
       Note that a 0-1 transition of IIC_Enable is required to initiate 
       an I2C bus transaction */

    for (i = 0; i < loops; i++) 
    {
        /* Transfer size for the last iteration will be the remaining size */
        xfer_cnt = (i==loops-1) ? (len - (i * fifo_len)) : fifo_len;

        /* Set the address, transfer count, and data transfer format */
        i2c_rd_wr_common(addr, xfer_cnt, I2C_CTL_REG_DTF_READ);

        /* Start the I2C transaction */
        reg_write((uint32 *)&I2C->IICEnable, I2C_IIC_ENABLE);

        /* Wait for completion */
        if (wait_xfer_done())
        {
            ret = -EIO;
            goto end;
        }

        /* Read the data */
        if (fifo_len == 8) 
        {
            for (j = 0; j < xfer_cnt; j++) 
                buf[i*fifo_len + j] = 
                    (u8)reg_read((uint32 *)&I2C->DataOut0 + j);
        }
        else 
        {
            int num_dwords = (xfer_cnt + 3)/4;
            for (j = 0; j < num_dwords; j++) 
            {
#ifdef __LITTLE_ENDIAN
                *((int *)&buf[i*fifo_len + j*4]) = 
                    reg_read((uint32 *)&I2C->DataOut0 + j);
#else
                *((int *)&buf[i*fifo_len + j*4]) = 
                    swab32(reg_read((uint32 *)&I2C->DataOut0 + j));
#endif
            }
        }

    }

end:
    /* I2C bus transaction is complete, so set the IIC_Enable to zero */
    reg_write((uint32 *)&I2C->IICEnable, 0);
    return ret;
}

/* Write data of given length to the slave. Note that 2 or more transactions
   would be required on the bus if the length is more than the fifo_len */
static int i2c_write(struct i2c_msg *p, int fifo_len)
{
    int i, j, ret = 0;
    u16 xfer_cnt;
    unsigned int len = p->len;
    unsigned char *buf = p->buf;
    unsigned char addr = p->addr;
    int loops = (len + (fifo_len - 1))/fifo_len;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);

    /* Make sure IIC_Enable bit is zero before start of a new transaction. 
       Note that a 0-1 transition of IIC_Enable is required to initiate 
       an I2C bus transaction */
    reg_write((uint32 *)&I2C->IICEnable, 0);

    for (i = 0; i < loops; i++)
    {
        /* Transfer size for the last iteration will be the remaining size */
        xfer_cnt = (i==loops-1) ? (len - (i * fifo_len)) : fifo_len;

        /* Set the address, transfer count, and data transfer format */
        i2c_rd_wr_common(addr, xfer_cnt, I2C_CTL_REG_DTF_WRITE);

        /* Write the data */
        if (fifo_len == 8) 
        {
            for (j = 0; j < xfer_cnt; j++) 
                reg_write((uint32 *)&I2C->DataIn0 + j, buf[i * fifo_len + j]);
        }
        else 
        {
            int num_dwords = (xfer_cnt + 3)/4;
            for (j = 0; j < num_dwords; j++) 
            {
#ifdef __LITTLE_ENDIAN
                reg_write((uint32 *)&I2C->DataIn0 + j, 
                          *((int *)&buf[i * fifo_len + j*4]));
#else
                reg_write((uint32 *)&I2C->DataIn0 + j, 
                          swab32(*((int *)&buf[i * fifo_len + j*4])));
#endif
            }
        }

        /* Start the I2C transaction */
        reg_write((uint32 *)&I2C->IICEnable, I2C_IIC_ENABLE);

        /* Wait for completion */
        if (wait_xfer_done())
        {
            ret = -EIO;
            goto end;
        }
    }

end:
    /* I2C bus transaction is complete, so set the IIC_Enable to zero */
    reg_write((uint32 *)&I2C->IICEnable, 0);
    return ret;
}

/* read followed by write without a stop in between */
static int i2c_read_then_write(struct i2c_msg *msgs, int fifo_len)
{
    int j, ret = 0;
    u16 xfer_cnt;
    struct i2c_msg *p = &msgs[0], *q = &msgs[1];

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);
    /* Make sure IIC_Enable bit is zero before start of a new transaction. 
       Note that a 0-1 transition of IIC_Enable is required to initiate 
       an I2C bus transaction */
    reg_write((uint32 *)&I2C->IICEnable, 0);

    /* Set the transfer counts for both the transactions */
    xfer_cnt = (q->len << I2C_CNT_REG2_SHIFT) | p->len;

    /* Set the address, transfer count, and data transfer format */
    i2c_rd_wr_common((u8)p->addr, xfer_cnt, I2C_CTL_REG_DTF_READ_AND_WRITE);

    /* Write the data */
    if (fifo_len == 8) 
    {
        for (j = 0; j < (q->len); j++) 
            reg_write((uint32 *)&I2C->DataIn0 + j, q->buf[j]);
    }
    else 
    {
        int num_dwords = ((q->len) + 3)/4;
        for (j = 0; j < num_dwords; j++) 
        {
#ifdef __LITTLE_ENDIAN
            reg_write((uint32 *)&I2C->DataIn0 + j, 
                      *((int *)&q->buf[j*4]));
#else
            reg_write((uint32 *)&I2C->DataIn0 + j, 
                      swab32(*((int *)&q->buf[j*4])));
#endif
        }
    }

    /* Start the I2C transaction */
    reg_write((uint32 *)&I2C->IICEnable, I2C_IIC_ENABLE);

    /* Wait for completion */
    if (wait_xfer_done())
    {
        ret = -EIO;
        goto end;
    }

    /* Read the data */
    if (fifo_len == 8) 
    {
        for (j = 0; j < (p->len); j++) 
            p->buf[j] = (u8)reg_read((uint32 *)&I2C->DataOut0 + j);
    }
    else 
    {
        int num_dwords = ((p->len) + 3)/4;
        for (j = 0; j < num_dwords; j++) 
        {
#ifdef __LITTLE_ENDIAN
            *((int *)&p->buf[j*4]) = 
                reg_read((uint32 *)&I2C->DataOut0 + j);
#else
            *((int *)&p->buf[j*4]) = 
                swab32(reg_read((uint32 *)&I2C->DataOut0 + j));
#endif
        }
    }

end:
    /* I2C bus transaction is complete, so set the IIC_Enable to zero */
    reg_write((uint32 *)&I2C->IICEnable, 0);
    return ret;
}

/* write followed by read without a stop in between */
static int i2c_write_then_read(struct i2c_msg *msgs, int fifo_len)
{
    int j, ret = 0;
    u16 xfer_cnt;
    struct i2c_msg *p = &msgs[0], *q = &msgs[1];

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);
    /* Make sure IIC_Enable bit is zero before start of a new transaction. 
       Note that a 0-1 transition of IIC_Enable is required to initiate 
       an I2C bus transaction */
    reg_write((uint32 *)&I2C->IICEnable, 0);

    /* Set the transfer counts for both the transactions */
    xfer_cnt = (q->len << I2C_CNT_REG2_SHIFT) | p->len;

    /* Set the address, transfer count, and data transfer format */
    i2c_rd_wr_common((u8)p->addr, xfer_cnt, I2C_CTL_REG_DTF_WRITE_AND_READ);

    /* Write the data */
    if (fifo_len == 8)
    {
        for (j = 0; j < (p->len); j++) 
            reg_write((uint32 *)&I2C->DataIn0 + j, p->buf[j]);
    }
    else
    {
        int num_dwords = ((p->len) + 3)/4;
        for (j = 0; j < num_dwords; j++) 
        {
#ifdef __LITTLE_ENDIAN
            reg_write((uint32 *)&I2C->DataIn0 + j, 
                      *((int *)&p->buf[j*4]));
#else
            reg_write((uint32 *)&I2C->DataIn0 + j, 
                      swab32(*((int *)&p->buf[j*4])));
#endif
        }
    }

    /* Start the I2C transaction */
    reg_write((uint32 *)&I2C->IICEnable, I2C_IIC_ENABLE);

    /* Wait for completion */
    if (wait_xfer_done())
    {
        ret = -EIO;
        goto end;
    }

    /* Read the data */
    if (fifo_len == 8)
    {
        for (j = 0; j < (q->len); j++) 
            q->buf[j] = (u8)reg_read((uint32 *)&I2C->DataOut0 + j);
    }
    else
    {
        int num_dwords = ((q->len) + 3)/4;
        for (j = 0; j < num_dwords; j++) 
        {
#ifdef __LITTLE_ENDIAN
            *((int *)&q->buf[j*4]) = 
                reg_read((uint32 *)&I2C->DataOut0 + j);
#else
            *((int *)&q->buf[j*4]) = 
                swab32(reg_read((uint32 *)&I2C->DataOut0 + j));
#endif
        }
    }

end:
    /* I2C bus transaction is complete, so set the IIC_Enable to zero */
    reg_write((uint32 *)&I2C->IICEnable, 0);
    return ret;
}

/* Transfers the given number of messages */
static int bcm63000_xfer(struct i2c_adapter *adap, 
                        struct i2c_msg *msgs, int num)
{
    struct i2c_msg *p, *q;
    int i, err = 0;
    /* Determine the FIFO length for Read and Write Transactions */
    int fifo_len = (reg_read((uint32 *)&I2C->CtlHiReg) 
                    & I2C_CTLHI_REG_DATA_REG_SIZE) ? 32 : 8;

    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);
    /* The 63000 supports read_then_write and write_then_read formats, 
       take advantage of these formats whenever possible */
    if (num == 2)
    {
     	p = &msgs[0]; q = &msgs[1];
        /* We may be able to use the read_then_write and write_then_read 
        formats only when the length of each message is less than fifo_len */
        if (p->len <= fifo_len && q->len <= fifo_len)
        {
            if ((p->flags == I2C_M_RD) && (!(q->flags & I2C_M_RD)))
            {
                err = i2c_read_then_write(msgs, fifo_len);
                return (err < 0) ? err : 2;
            }
            else if ((!(p->flags & I2C_M_RD)) && (q->flags == I2C_M_RD))
            {
                err = i2c_write_then_read(msgs, fifo_len);
                return (err < 0) ? err : 2;
            }
        }
    }
    
    /*TBD:See if we can use NO_STOP b/w multiple read and write transactions */
    for (i = 0; !err && i < num; i++)
    {
        p = &msgs[i];
        if (p->flags & I2C_M_RD)
            err = i2c_read(p, fifo_len);
        else
            err = i2c_write(p, fifo_len);
    }

    return (err < 0) ? err : i;
}

static u32 bcm63000_func(struct i2c_adapter *adap)
{
    BCM_LOG_DEBUG(BCM_LOG_ID_I2C, "Entering the function %s \n", __FUNCTION__);
    /*The SMBus commands are emulated on the I2C bus using i2c_xfer function*/
    /*TBD:For correct SMBus Emulation, need to use the NOSTOP b/w given 
      messges to the i2c_xfer */
    return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
/*  TBD: The HW supports 10 bit addressing and some protocol magling */
/*	I2C_FUNC_10BIT_ADDR | I2C_FUNC_PROTOCOL_MAGLING; */
}

static const struct i2c_algorithm bcm63000_algo = {
    .master_xfer	= bcm63000_xfer,
    .functionality	= bcm63000_func,
};

static struct i2c_adapter bcm63000_i2c_adapter = {
    .owner		= THIS_MODULE,
    .class		= I2C_CLASS_HWMON,
    .algo		= &bcm63000_algo,
    .name		= "bcm63000 I2C driver",
};


/* Max slave clock stretching per transaction as per SMBus spec */
#define TLOW_SEXT  25000
/* Max master clock stretching per transaction as per SMBus spec */
#define TLOW_MEXT  10000
/* Maximum clock cycles for a 32B transaction */
#define MAX_CLKS_PER_TRANSACTION  306; /* 34 * 9 */ 

/* Time in uS when CLK = 375K or 390K */
#define CLK_375K_TIME 3
/* Time in uS when CLK = 187.5K or 200K */
#define CLK_187_5K_TIME 6

static int __init bcm63000_i2c_init(void)
{
    int val, clk_time = 1000;

    /* Set the clock selection */
    if ((clk_sel < 0) || (clk_sel > 7)) {
        printk("Invalid clock selection. Setting the clk to 97.5K \n");
        clk_sel = CLK_97_5K;
    }
    
    val = (clk_sel & I2C_CTL_REG_SCL_SEL_M) << I2C_CTL_REG_SCL_SEL_S;
    if (clk_sel > 3) {
        val |= (1 << I2C_CTL_REG_DIV_CLK_S);
    }
    reg_write((uint32 *)&I2C->CtlReg, val);
    val = reg_read((uint32*)&I2C->CtlReg);

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
    xfer_timeout = TLOW_SEXT + TLOW_MEXT + clk_time * MAX_CLKS_PER_TRANSACTION; 

    /* Set the Data Register Length */
    reg_write((uint32 *)&I2C->CtlHiReg, 
              data_reg_len << I2C_CTLHI_REG_DATA_REG_SIZE_SHIFT);

    /* Register the Adapter(Bus & Algo drivers combined in this file) Driver*/
    return i2c_add_adapter(&bcm63000_i2c_adapter);
}

static void __exit bcm63000_i2c_exit(void)
{
    i2c_del_adapter(&bcm63000_i2c_adapter);
}

module_init(bcm63000_i2c_init);
module_exit(bcm63000_i2c_exit);

MODULE_AUTHOR("Pratapa Reddy <pratapas@yahoo.com>");
MODULE_DESCRIPTION("I2C-Bus bcm63000 algorithm");
MODULE_LICENSE("GPL");

module_param(clk_sel, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(clk_sel, "clock selection: 0 = 375K, 1 = 390K, 2 = 187.5, " 
                 "3 = 200K, 4 = 93.75K, 5 = 97.5K, 6 = 46.875K, 7 = 50K ");

module_param(delay_dis, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(delay_dis, " When set to 1, removes the 300ns hold time"
                            " requirement on the data relative to the clock");

module_param(deglitch_dis, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(deglitch_dis, " When set to 1, removes the 50ns deglitch"
                               " time requirement on the data.");
