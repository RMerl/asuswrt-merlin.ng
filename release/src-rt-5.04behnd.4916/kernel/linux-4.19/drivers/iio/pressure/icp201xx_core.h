/*
 *________________________________________________________________________________________________________
 *Copyright (c) 2017 InvenSense Inc. All rights reserved.
 *
 *This stoftware, related documentation and any modifications thereto (collectively 밪oftware? is stubject
 *to InvenSense and its licensors' intellectual property rights under U.S. and international copyright
 *and other intellectual property rights laws.
 *
 *InvenSense and its licensors retain all intellectual property and proprietary rights in and to the Software
 *and any use, reproduction, disclosure or distribution of the Software without an express license agreement
 *from InvenSense is sttrictly prohibited.
 *
 *EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, THE SOFTWARE IS
 *PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 *TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 *EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, IN NO EVENT SHALL
 *INVENSENSE BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 *DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 *NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 *OF THE SOFTWARE.
 *________________________________________________________________________________________________________
 */
/**@defgroup DriverIcp201xx Icp201xx driver
 *@brief    Low-level driver for Icp201xx devices
 *@ingroup  DriverIcp201xx
 *@{
 */
#ifndef _INV_ICP201XX_CORE_H_
#define _INV_ICP201XX_DCORE_H_


/*Main/ OTP Registers */
#define ICP201XX_REG_TRIM1_MSB			0X05
#define ICP201XX_REG_TRIM2_LSB			0X06
#define ICP201XX_REG_TRIM2_MSB			0X07
#define ICP201XX_REG_DEVICE_ID			0X0C
#define ICP201XX_REG_OTP_MTP_OTP_CFG1		0XAC
#define ICP201XX_REG_OTP_MTP_MR_LSB		0XAD
#define ICP201XX_REG_OTP_MTP_MR_MSB		0XAE
#define ICP201XX_REG_OTP_MTP_MRA_LSB		0XAF
#define ICP201XX_REG_OTP_MTP_MRA_MSB		0XB0
#define ICP201XX_REG_OTP_MTP_MRB_LSB		0XB1
#define ICP201XX_REG_OTP_MTP_MRB_MSB		0XB2
#define ICP201XX_REG_OTP_MTP_OTP_ADDR		0XB5
#define ICP201XX_REG_OTP_MTP_OTP_CMD		0XB6
#define ICP201XX_REG_OTP_MTP_RD_DATA		0XB8
#define ICP201XX_REG_OTP_MTP_OTP_STATUS		0xB9
#define ICP201XX_REG_OTP_DEBUG2			0XBC
#define ICP201XX_REG_MASTER_LOCK		0XBE
#define ICP201XX_REG_OTP_STATUS2		0xBF

#define ICP201XX_REG_MODE_SELECT		0xC0
#define ICP201XX_REG_INTERRUPT_STATUS		0xC1
#define ICP201XX_REG_INTERRUPT_MASK		0xC2
#define ICP201XX_REG_FIFO_CONFIG		0xC3
#define ICP201XX_REG_FIFO_FILL			0xC4
#define ICP201XX_REG_SPI_MODE			0xC5
#define ICP201XX_REG_PRESS_ABS_LSB		0xC7
#define ICP201XX_REG_PRESS_ABS_MSB		0xC8
#define ICP201XX_REG_PRESS_DELTA_LSB		0xC9
#define ICP201XX_REG_PRESS_DELTA_MSB		0xCA
#define ICP201XX_REG_DEVICE_STATUS		0xCD
#define ICP201XX_REG_I3C_INFO			0xCE
#define ICP201XX_REG_VERSION			0XD3
#define ICP201XX_REG_PRESS_DATA_0               0xFA
#define ICP201XX_REG_PRESS_DATA_1               0xFB
#define ICP201XX_REG_PRESS_DATA_2               0xFC
#define ICP201XX_REG_TEMP_DATA_0                0xFD
#define ICP201XX_REG_TEMP_DATA_1                0xFE
#define ICP201XX_REG_TEMP_DATA_2                0xFF

#define ICP201XX_REG_MODE0_OSR_PRESS		0X14
#define ICP201XX_REG_MODE0_CFG1			0X15
#define ICP201XX_REG_MODE0_ODR_LSB		0X16
#define ICP201XX_REG_MODE0_CFG2			0X17
#define ICP201XX_REG_MODE0_BS_VALUE		0X18

#define ICP201XX_REG_MODE4_OSR_PRESS		(st->version == ICP201XX_VERSION_A)? 0x2C:0x3C
#define ICP201XX_REG_MODE4_CONFIG1		(st->version == ICP201XX_VERSION_A)? 0x2D:0x3D
#define ICP201XX_REG_MODE4_ODR_LSB		(st->version == ICP201XX_VERSION_A)? 0x2E:0x3E
#define ICP201XX_REG_MODE4_CONFIG2		(st->version == ICP201XX_VERSION_A)? 0x2F:0x3F
#define ICP201XX_REG_MODE4_BS_VALUE		(st->version == ICP201XX_VERSION_A)? 0x30:0x40
#define ICP201XX_REG_IIR_K_FACTOR_LSB		(st->version == ICP201XX_VERSION_A)? 0x78:0x88
#define ICP201XX_REG_IIR_K_FACTOR_MSB		(st->version == ICP201XX_VERSION_A)? 0x79:0x89

#define ICP201XX_REG_MODE0_PRESS_GAIN_FACTOR_LSB	0x7A
#define ICP201XX_REG_MODE0_PRESS_GAIN_FACTOR_MSG	0x7B
#define ICP201XX_REG_MODE4_PRESS_GAIN_FACTOR_LSB	(st->version == ICP201XX_VERSION_A)? 0x82:0x92
#define ICP201XX_REG_MODE4_PRESS_GAIN_FACTOR_MSB	(st->version == ICP201XX_VERSION_A)? 0x83:0x93

#define ICP201XX_MODE4_CONFIG_PRESS_OSR         0xb1
#define ICP201XX_MODE4_CONFIG_TEMP_OSR          0X0F
/* odr_setting = ( 8000 / ODR in Hz ) -1  : 25 Hz => ODR setting = 320(0x140) **/
#define ICP201XX_MODE4_CONFIG_ODR_SETTING       0x140 

/********************************************
Register Name: MODE4_CONFIG1
Register Type: READ/WRITE
Register Address: 45 (Decimal); 2D (Hex)
********************************************/
#define BIT_MODE4_CONFIG1_OSR_TEMP_MASK         0x1F
#define BIT_MODE4_CONFIG1_FIR_EN_MASK		0x20
#define BIT_MODE4_CONFIG1_IIR_EN_MASK		0x40

#define BIT_MODE4_CONFIG1_FIR_EN_POS		5
#define BIT_MODE4_CONFIG1_IIR_EN_POS		6

/********************************************
Register Name: MODE4_CONFIG2
Register Type: READ/WRITE
Register Address: 47 (Decimal); 2F (Hex)
********************************************/
#define BIT_MODE4_CONFIG2_ODR_MSB_MASK           0x1F
#define BIT_MODE4_CONFIG2_DVDD_ON_MASK		 0x20
#define BIT_MODE4_CONFIG2_HFOSC_ON_MASK		 0x40

#define BIT_MODE4_CONFIG2_DVDD_ON_POS		5
#define BIT_MODE4_CONFIG2_HFOSC_ON_POS		6
/********************************************
Register Name:
Bank         : Main registers
Register Type: READ/WRITE
Register Address: 5(Decimal); 5 (Hex)
********************************************/
#define BIT_PEFE_OFFSET_TRIM_MASK		0x3F


/********************************************
Register Name:
Bank         : Main registers
Register Type: READ/WRITE
Register Address: 48(Decimal); 30(Hex)
********************************************/
#define BIT_MODE4_BS_VALUE_PRESS		0x0F
#define BIT_MODE4_BS_VALUE_TEMP			0XF0


#define ICP201XX_INT_MASK_PRESS_DELTA      (0X01 << 6)
#define ICP201XX_INT_MASK_PRESS_ABS        (0X01 << 5)
#define ICP201XX_INT_MASK_FIFO_WMK_LOW     (0X01 << 3)
#define ICP201XX_INT_MASK_FIFO_WMK_HIGH    (0X01 << 2)
#define ICP201XX_INT_MASK_FIFO_UNDER_FLOW  (0X01 << 1)
#define ICP201XX_INT_MASK_FIFO_OVER_FLOW   (0X01 << 0)	



#define ICP201XX_INT_STATUS_PRESS_DELTA      (0X01 << 6)
#define ICP201XX_INT_STATUS_PRESS_ABS        (0X01 << 5)
#define ICP201XX_INT_STATUS_FIFO_WMK_LOW     (0X01 << 3)
#define ICP201XX_INT_STATUS_FIFO_WMK_HIGH    (0X01 << 2)
#define ICP201XX_INT_STATUS_FIFO_UNDER_FLOW  (0X01 << 1)
#define ICP201XX_INT_STATUS_FIFO_OVER_FLOW   (0X01 << 0)


int icp201xx_wr_pefe_offset_trim(struct icp201xx_state  *st,uint8_t new_value);


/********************************************
Register Name:
Bank         : Main registers
Register Type: READ/WRITE
Register Address: 6(Decimal); 6 (Hex)
********************************************/
#define BIT_HFOSC_OFFSET_TRIM_MASK		0x7F

int icp201xx_wr_hfosc_trim(struct icp201xx_state  *st,uint8_t new_value);

/********************************************
Register Name:
Bank         : Main registers
Register Type: READ/WRITE
Register Address: 7(Decimal); 7 (Hex)
********************************************/
#define BIT_PEFE_GAIN_TRIM_MASK			0x70

#define BIT_PEFE_GAIN_TRIM_POS			4

int icp201xx_wr_pefe_gain_trim(struct icp201xx_state  *st, uint8_t new_value);

/********************************************
Register Name:
Bank         : Main registers
Register Type: READ/WRITE
Register Address: 12(Decimal); 0X0C (Hex)
********************************************/
int icp201xx_rd_a1_who_am_i(struct icp201xx_state  *st, uint8_t *value);

/********************************************
Register Name: IIR_K_FACTOR_LSB
Register Type: READ/WRITE
Register Address: 120 (Decimal); 78 (Hex)
********************************************/
int icp201xx_wr_iir_k_factor_lsb(struct icp201xx_state  *st, uint8_t new_value);
int icp201xx_rd_iir_k_factor_lsb(struct icp201xx_state *st, uint8_t *value);


/********************************************
Register Name: IIR_K_FACTOR_MSB
Register Type: READ/WRITE
Register Address: 121 (Decimal); 79 (Hex)
********************************************/
int icp201xx_wr_iir_k_factor_msb(struct icp201xx_state *st, uint8_t new_value);
int icp201xx_rd_iir_k_factor_msb(struct icp201xx_state *st, uint8_t *value);


/********************************************
Register Name: OTP Config 1
Bank         : otp registers
Register Type: READ/WRITE
Register Address: 172 (Decimal); AC (Hex)
********************************************/
#define BIT_OTP_CONFIG1_WRITE_SWITCH_MASK			0x02
#define BIT_OTP_CONFIG1_OTP_ENABLE_MASK				0x01
#define BIT_OTP_CONFIG1_WRITE_SWITCH_POS			1
int icp201xx_wr_otp_write_switch(struct icp201xx_state  *st,uint8_t new_value);
int icp201xx_wr_otp_enable(struct icp201xx_state  *st,uint8_t new_value);


/********************************************
Register Name: OTP MR LSB
Bank         : otp registers
Register Type: READ/WRITE
Register Address: 173 (Decimal); AD (Hex)
********************************************/
int icp201xx_wr_otp_mr_lsb(struct icp201xx_state  *st,uint8_t new_value);

/********************************************
Register Name: OTP MR MSB
Bank         : otp registers
Register Type: READ/WRITE
Register Address: 174 (Decimal); AE (Hex)
********************************************/
int icp201xx_wr_otp_mr_msb(struct icp201xx_state  *st,uint8_t new_value);
/********************************************
Register Name: OTP MRA LSB
Bank         : otp registers
Register Type: READ/WRITE
Register Address: 175 (Decimal); AF (Hex)
********************************************/
int icp201xx_wr_otp_mra_lsb(struct icp201xx_state  *st,uint8_t new_value);
int icp201xx_rd_otp_mra_lsb(struct icp201xx_state  *st, uint8_t *value);

/********************************************
Register Name: OTP MRA MSB
Bank         : otp registers
Register Type: READ/WRITE
Register Address: 176 (Decimal); B0 (Hex)
********************************************/
int icp201xx_wr_otp_mra_msb(struct icp201xx_state  *st,uint8_t new_value);
int icp201xx_rd_otp_mra_msb(struct icp201xx_state  *st, uint8_t *value);

/********************************************
Register Name: OTP MRB LSB
Bank         : otp registers
Register Type: READ/WRITE
Register Address: 177 (Decimal); B1 (Hex)
********************************************/
int icp201xx_wr_otp_mrb_lsb(struct icp201xx_state  *st,uint8_t new_value);

/********************************************
Register Name: OTP MRB MSB
Bank         : otp registers
Register Type: READ/WRITE
Register Address: 178 (Decimal); B2 (Hex)
********************************************/
int icp201xx_wr_otp_mrb_msb(struct icp201xx_state  *st,uint8_t new_value);

/********************************************
Register Name: OTP ADDR
Bank         : otp registers
Register Type: READ/WRITE
Register Address: 181 (Decimal); B5 (Hex)
********************************************/
int icp201xx_wr_otp_addr(struct icp201xx_state  *st,uint8_t new_value);
int icp201xx_rd_otp_addr(struct icp201xx_state  *st, uint8_t *value);

/********************************************
Register Name: OTP CMD
Bank         : otp register
Register Type: READ/WRITE
Register Address: 182 (Decimal); B6 (Hex)
********************************************/
int icp201xx_wr_otp_cmd(struct icp201xx_state  *st,uint8_t new_value);
int icp201xx_rd_otp_cmd(struct icp201xx_state  *st, uint8_t *value);


/********************************************
Register Name: OTP Read Reg
Bank         : otp registers
Register Type: READ/WRITE
Register Address: 184 (Decimal); B8 (Hex)
********************************************/
int icp201xx_rd_otp_reg_data(struct icp201xx_state  *st, uint8_t *value);


/********************************************
Register Name: OTP sttatus
Bank         : otp registers
Register Type: READ/WRITE
Register Address: 185 (Decimal); B9 (Hex)
********************************************/
int icp201xx_rd_otp_status(struct icp201xx_state  *st, uint8_t *value);

/********************************************
Register Name: OTP Debug2
Bank         : OTP registers
Register Type: READ/WRITE
Register Address:  180(Decimal); BC (Hex)
********************************************/
#define BIT_OTP_DBG2_RESET_MASK   0x80
#define BIT_OTP_DBG2_RESET_POS	  7

int icp201xx_wr_otp_dbg2_reset(struct icp201xx_state  *st,uint8_t new_value);


/********************************************
Register Name: Master lock
Bank         : OTP registers
Register Type: READ/WRITE
Register Address:  190(Decimal); BE (Hex)
********************************************/
int icp201xx_wr_master_lock(struct icp201xx_state  *st,uint8_t new_value);


/********************************************
Register Name: MODE_SELECT
Register Type: READ/WRITE
Register Address: 192 (Decimal); C0 (Hex)
********************************************/
#define BIT_MEAS_CONFIG_MASK          0xE0
#define BIT_FORCED_MEAS_TRIGGER_MASK  0x10
#define BIT_MEAS_MODE_MASK            0x08
#define BIT_POWER_MODE_MASK           0x04
#define BIT_FIFO_READOUT_MODE_MASK    0x03

#define BIT_MEAS_CONFIG_POS		5
#define BIT_FORCED_MEAS_TRIGGER_POS	4
#define BIT_FORCED_MEAS_MODE_POS	3
#define BIT_FORCED_POW_MODE_POS		2

int icp201xx_wr_mode_select(struct icp201xx_state  *st, uint8_t new_value);
int icp201xx_rd_mode_select(struct icp201xx_state *st, uint8_t *value);

int icp201xx_wr_meas_config(struct icp201xx_state  *st, icp201xx_op_mode_t new_value);
int icp201xx_rd_meas_config(struct icp201xx_state *st, icp201xx_op_mode_t *value);

int icp201xx_wr_forced_meas_trigger(struct icp201xx_state  *st, icp201xx_forced_meas_trigger_t new_value);
int icp201xx_rd_forced_meas_trigger(struct icp201xx_state *st, icp201xx_forced_meas_trigger_t *value);

int icp201xx_wr_meas_mode(struct icp201xx_state  *st, icp201xx_meas_mode_t new_value);
int icp201xx_rd_meas_mode(struct icp201xx_state *st, icp201xx_meas_mode_t *value);

int icp201xx_wr_pow_mode(struct icp201xx_state  *st, icp201xx_power_mode_t new_value);
int icp201xx_rd_pow_mode(struct icp201xx_state *st, icp201xx_power_mode_t *value);

int icp201xx_wr_fifo_readout_mode(struct icp201xx_state  *st, icp201xx_FIFO_readout_mode_t new_value);
int icp201xx_rd_fifo_readout_mode(struct icp201xx_state *st, icp201xx_FIFO_readout_mode_t *value);


/********************************************
Register Name: INTERRUPT_STATUS
Register Type: READ/WRITE
Register Address: 193 (Decimal); C1(Hex)
********************************************/
int icp201xx_wr_int_status(struct icp201xx_state *st, uint8_t new_value);

int icp201xx_rd_int_status(struct icp201xx_state *st, uint8_t *value);

/********************************************
Register Name: INTERRUPT_MASK
Register Type: READ/WRITE
Register Address: 194 (Decimal); C2(Hex)
********************************************/
int icp201xx_wr_int_mask(struct icp201xx_state *st, uint8_t new_value);

int icp201xx_rd_int_mask(struct icp201xx_state *st, uint8_t *value);

/********************************************
Register Name: FIFO_CONFIG
Register Type: READ/WRITE
Register Address: 195 (Decimal); C3(Hex)
********************************************/
#define BIT_FIFO_WM_HIGH_MASK    0xF0
#define BIT_FIFO_WM_LOW_MASK     0x0F

#define BIT_FIFO_WM_HIGH_POS     3

int icp201xx_wr_fifo_config(struct icp201xx_state *st, uint8_t new_value);
int icp201xx_rd_fifo_config(struct icp201xx_state *st, uint8_t *value);

int icp201xx_wr_fifo_wm_high(struct icp201xx_state *st, uint8_t new_value);
int icp201xx_rd_fifo_wm_high(struct icp201xx_state *st, uint8_t *value);

int icp201xx_wr_fifo_wm_low(struct icp201xx_state *st, uint8_t new_value);
int icp201xx_rd_fifo_wm_low(struct icp201xx_state *st, uint8_t *value);


/********************************************
Register Name: FIFO_FILL
Register Type: READ/WRITE
Register Address: 196 (Decimal); C4 (Hex)
********************************************/
#define BIT_FIFO_FLUSH_MASK		0x80
#define BIT_FIFO_EMPTY_STATUS_MASK	0x40
#define BIT_FIFO_FULL_STATUS_MASK	0x20
#define BIT_FIFO_LEVEL_MASK		0x1F

#define BIT_FIFO_EMPTY_POS     6
#define BIT_FIFO_FULL_POS      5

int icp201xx_wr_fifo_fill(struct icp201xx_state *st, uint8_t new_value);
int icp201xx_rd_fifo_fill(struct icp201xx_state *st, uint8_t *value);


int icp201xx_wr_flush_fifo(struct icp201xx_state *st);
int icp201xx_rd_fifo_empty_status(struct icp201xx_state *st, uint8_t *value);
int icp201xx_rd_fifo_full_status(struct icp201xx_state *st, uint8_t *value);
int icp201xx_rd_fifo_level(struct icp201xx_state *st, uint8_t *value);

/********************************************
Register Name: SPI_MODE
Register Type: READ/WRITE
Register Address: 197 (Decimal); C5 (Hex)
********************************************/
typedef enum icp201xx_spi_mode {
	ICP201XX_SPI_MODE_4_WIRE = 0,
	ICP201XX_SPI_MODE_3_WIRE,
}icp201xx_spi_mode_t;

#define BIT_FIFO_SPI_MODE_MASK    0x01  
                                 // 0: SPI 4-WIRE
								 // 1: SPI 3-WIRE
int icp201xx_wr_spi_mode(struct icp201xx_state *st, icp201xx_spi_mode_t new_value);
int icp201xx_rd_spi_mode(struct icp201xx_state *st, icp201xx_spi_mode_t *value);

/********************************************
Register Name: PRESS_ABS_LSB
Register Type: READ/WRITE
Register Address: 199 (Decimal); C7 (Hex)
********************************************/
int icp201xx_wr_press_abs_lsb(struct icp201xx_state *st, uint8_t new_value);
int icp201xx_rd_press_abs_lsb(struct icp201xx_state *st, uint8_t *value);
/********************************************
Register Name: PRESS_ABS_MSB
Register Type: READ/WRITE
Register Address: 200 (Decimal); C8 (Hex)
********************************************/
int icp201xx_wr_press_abs_msb(struct icp201xx_state *st, uint8_t new_value);
int icp201xx_rd_press_abs_msb(struct icp201xx_state *st, uint8_t *value);
/********************************************
Register Name: PRESS_DELTA_LSB
Register Type: READ/WRITE
Register Address: 201 (Decimal); C9 (Hex)
********************************************/
int icp201xx_wr_press_delta_lsb(struct icp201xx_state *st, uint8_t new_value);
int icp201xx_rd_press_delta_lsb(struct icp201xx_state *st, uint8_t *value);
/********************************************
Register Name: PRESS_DELTA_MSB
Register Type: READ/WRITE
Register Address: 202 (Decimal); CA (Hex)
********************************************/
int icp201xx_wr_press_delta_msb(struct icp201xx_state *st, uint8_t new_value);
int icp201xx_rd_press_delta_msb(struct icp201xx_state *st, uint8_t *value);
/********************************************
Register Name: DEVICE_STATUS
Register Type: READ
Register Address: 205 (Decimal); CD (Hex)
********************************************/
#define BIT_DEVICE_STATUS_LP_SEQ_STATE_MASK		0X06
#define BIT_DEVICE_STATUS_MODE_SYNC_STATUS_MASK		0x01
// 0 : Mode stync is going on, MODE_SELECT Reg is NOT accessible.
// 1 : MODE_SELECT Reg is accessible.

#define BIT_DEVICE_STATUS_LP_SEQ_STATE_POS	1
int icp201xx_rd_device_status(struct icp201xx_state *st, uint8_t *value);
int icp201xx_rd_mode_sync_status(struct icp201xx_state *st, uint8_t *value);
int icp201xx_rd_lp_seq_sync_status(struct icp201xx_state *st, uint8_t *value);

/********************************************
Register Name: SPI_MODE
Register Type: READ/WRITE
Register Address: 206 (Decimal); CE (Hex)
********************************************/

int icp201xx_rd_fifo(struct icp201xx_state *st, uint8_t len, uint8_t*value, uint8_t fifo_read_offset);

/********************************************
Register Name:
Bank         : Main registers
Register Type: READ/WRITE
Register Address: 211(Decimal); 0XD3 (Hex)
********************************************/

int icp201xx_rd_b1_who_am_i(struct icp201xx_state  *st, uint8_t *value);

int icp201xx_get_fifo_count(struct icp201xx_state *st, uint8_t *fifo_cnt);
int icp201xx_get_fifo_data(struct icp201xx_state *st, uint8_t req_packet_cnt, uint8_t *data);
int icp201xx_read_dummy_data(struct icp201xx_state *st);

int icp201xx_soft_reset(struct icp201xx_state *st);
int icp201xx_config(struct icp201xx_state *st, icp201xx_op_mode_t op_mode, icp201xx_FIFO_readout_mode_t fifo_read_mode);
void inv_run_icp201xx_in_polling(struct icp201xx_state *st, icp201xx_op_mode_t op_mode, uint16_t odr_hz);
int icp201xx_OTP_bootup_cfg(struct icp201xx_state *st);

#endif  /*#ifndef _INV_ICP201XX_CORE_H_ */

