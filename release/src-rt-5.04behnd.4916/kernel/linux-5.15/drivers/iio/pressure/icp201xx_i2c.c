/*
 * ________________________________________________________________________________________________________
 * Copyright (c) 2015-2015 InvenSense Inc. All rights reserved.
 *
 * This software, related documentation and any modifications thereto (collectively 밪oftware� is subject
 * to InvenSense and its licensors' intellectual property rights under U.S. and international copyright
 * and other intellectual property rights laws.
 *
 * InvenSense and its licensors retain all intellectual property and proprietary rights in and to the Software
 * and any use, reproduction, disclosure or distribution of the Software without an express license agreement
 * from InvenSense is strictly prohibited.
 *
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, THE SOFTWARE IS
 * PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, IN NO EVENT SHALL
 * INVENSENSE BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THE SOFTWARE.
 * ________________________________________________________________________________________________________
 */

/** @defgroup DriverIcp201xxSerif Icp201xx driver serif
 *  @brief Interface for low-level serial (I2C/I3C/SPI) access
 *  @ingroup  DriverIcp201xx
 *  @{
 */

#include <linux/i2c.h>
#include <linux/module.h>
#include "icp201xx_i2c.h"

static inline int icp201xx_i2c_xfer(struct i2c_adapter *adap,
		struct i2c_msg *msgs, int num)
{
	int ret;

	ret = i2c_transfer(adap, msgs, num);
	if (ret < 0)
		return ret;

	if (ret == num)
		ret = 0;
	else
		ret = -EIO;

	return ret;
}

int icp201xx_reg_write(struct icp201xx_state *st, u8 reg_addr, uint8_t val)
{
	int ret;


	u8 buf[2];

	struct i2c_msg msgs[1] = {
		{
			.addr = st->client->addr,
			.flags = 0,
			.len = 2,
			.buf =(u8*)buf,
		}
	};

	buf[0] = reg_addr & 0xFF;
	buf[1] = val;


	ret = icp201xx_i2c_xfer(st->client->adapter, &msgs[0], 1);
	if (ret)
		return ret;


	return 0;
}
EXPORT_SYMBOL_GPL(icp201xx_reg_write);

int icp201xx_reg_read(struct icp201xx_state *st, u8 reg_addr, u8 *val)
{
	int ret;

	struct i2c_msg msgs[12] = {
		{
			.addr = st->client->addr,
			.flags = 0,
			.len = 1,
			.buf =&reg_addr,
		},
		{
			.addr = st->client->addr,
			.flags = I2C_M_RD,
			.len = 1,
			.buf = (u8*)val,
		},

	};


	ret = icp201xx_i2c_xfer(st->client->adapter, msgs, 2);
	if (ret)
		return ret;


	return 0;
}
EXPORT_SYMBOL_GPL(icp201xx_reg_read);

int icp201xx_reg_read_n(struct icp201xx_state *st, u8 reg_addr, u8 len, u8 *val)
{
        int ret;

        struct i2c_msg msgs[12] = {
                {
                        .addr = st->client->addr,
                        .flags = 0,
                        .len = 1,
                        .buf =&reg_addr,
                },
                {
                        .addr = st->client->addr,
                        .flags = I2C_M_RD,
                        .len = len,
                        .buf = (u8*)val,
                },

        };


        ret = icp201xx_i2c_xfer(st->client->adapter, msgs, 2);
        if (ret)
                return ret;


        return 0;
}
EXPORT_SYMBOL_GPL(icp201xx_reg_read_n);

int icp201xx_reg_update(struct icp201xx_state *st, u8 reg_addr, u8 mask, u8 pos, u8 val)
{
	int ret;
	uint8_t reg_value = 0;

	ret = icp201xx_reg_read(st, reg_addr, &reg_value);

	if(ret)
		return ret;

	reg_value = (reg_value & (~mask)) | (val << pos) ;

	return icp201xx_reg_write(st, reg_addr, reg_value);
}
EXPORT_SYMBOL_GPL(icp201xx_reg_update);

MODULE_AUTHOR("Invensense Corporation");
MODULE_DESCRIPTION("Invensense ICP201XX driver");
MODULE_LICENSE("GPL");
