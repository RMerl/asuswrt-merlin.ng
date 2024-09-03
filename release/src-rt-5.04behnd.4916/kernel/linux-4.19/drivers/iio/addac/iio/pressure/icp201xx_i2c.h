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
#ifndef _INV_ICP201XX_I2C_H_
#define _INV_ICP201XX_I2C_H_

#include "icp201xx.h"

/** @brief Reads data from a register on mems.
 * @param[in]  st Pointer to the driver context
 * @param[in]  reg_addr    register address
 * @param[out] val   output data from the register
 * @return     0 in case of success, negative value on error. See enum inv_status
 */
int icp201xx_reg_read(struct icp201xx_state *st, u8 reg_addr, u8 *val);

/** @brief Reads data from a register on mems.
 * @param[in]  st Pointer to the driver context
 * @param[in]  reg_addr    register address
 * @param[out] val   output data from the register
 * @return     0 in case of success, negative value on error. See enum inv_status
 */
int icp201xx_reg_read_n(struct icp201xx_state *st, u8 reg_addr, u8 len, u8 *val);

/** @brief Writes data to a register on mems.
 * @param[in]  st Pointer to the driver context
 * @param[in]  reg_addr    register address
 * @param[in]  val intput data to the register
 * @return     0 in case of success, negative value on error. See enum inv_status
 */
int icp201xx_reg_write(struct icp201xx_state *st, u8 reg_addr, uint8_t val);

/** @brief Update data to a register on mems.
 * @param[in]  st Pointer to the driver context
 * @param[in]  reg_addr    register address
 * @param[in]  mask	bit mask for update register
 * @param[in]  pos	bit shift for update register
 * @param[in]  val intput data to the register
 * @return     0 in case of success, negative value on error. See enum inv_status
 */
int icp201xx_reg_update(struct icp201xx_state *st, u8 reg_addr, u8 mask, u8 pos, u8 val);

#endif /* _INV_ICP201XX_INTERFACE_H_ */


