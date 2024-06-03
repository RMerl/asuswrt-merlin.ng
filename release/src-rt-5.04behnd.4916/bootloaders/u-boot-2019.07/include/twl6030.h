/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 */

#ifndef TWL6030_H
#define TWL6030_H

#include <common.h>
#include <i2c.h>

/* I2C chip addresses */
#define TWL6030_CHIP_PM		0x48

#define TWL6030_CHIP_USB	0x49
#define TWL6030_CHIP_ADC	0x49
#define TWL6030_CHIP_CHARGER	0x49
#define TWL6030_CHIP_PWM	0x49

/* Slave Address 0x48 */
#define TWL6030_STS_HW_CONDITIONS	0x21

#define TWL6030_STS_HW_CONDITIONS_PWRON	(1 << 0)

#define TWL6030_PHOENIX_DEV_ON		0x25

#define TWL6030_PHOENIX_APP_DEVOFF	(1 << 0)
#define TWL6030_PHOENIX_CON_DEVOFF	(1 << 1)
#define TWL6030_PHOENIX_MOD_DEVOFF	(1 << 2)

#define TWL6030_PH_STS_BOOT		0x29

#define TWL6030_PH_STS_BOOT0		(1 << 0)
#define TWL6030_PH_STS_BOOT1		(1 << 1)
#define TWL6030_PH_STS_BOOT2		(1 << 2)
#define TWL6030_PH_STS_BOOT3		(1 << 3)

#define TWL6030_VAUX1_CFG_STATE		0x86
#define TWL6030_VAUX1_CFG_VOLTAGE	0x87
#define TWL6030_VMMC_CFG_STATE		0x9A
#define TWL6030_VMMC_CFG_VOLTAGE	0x9B
#define TWL6030_VUSB_CFG_STATE		0xA2
#define TWL6030_VUSB_CFG_VOLTAGE	0xA3

#define TWL6030_CFG_GRP_P1		(1 << 0)
#define TWL6030_CFG_STATE_ON		(1 << 0)
#define TWL6030_CFG_STATE_P1		(TWL6030_CFG_GRP_P1 << 5)
#define TWL6030_CFG_VOLTAGE_18		0x09
#define TWL6030_CFG_VOLTAGE_28		0x13
#define TWL6030_CFG_VOLTAGE_30		0x15
#define TWL6030_CFG_VOLTAGE_33		0x18

#define MISC1			0xE4
#define VAC_MEAS		(1 << 2)
#define VBAT_MEAS		(1 << 1)
#define BB_MEAS			(1 << 0)

#define TWL6030_MISC2			0xE5
#define TWL6030_MISC2_VUSB_IN_PMID	(1 << 3)
#define TWL6030_MISC2_VUSB_IN_VSYS	(1 << 4)

/* Slave Address 0x49 */

#define TWL6030_CONTROLLER_STAT1		0xE3

#define TWL6030_CONTROLLER_STAT1_VAC_DET	(1 << 3)
#define TWL6030_CONTROLLER_STAT1_VBUS_DET	(1 << 2)

/* Battery CHARGER REGISTERS */
#define CONTROLLER_INT_MASK	0xE0
#define CONTROLLER_CTRL1	0xE1
#define CONTROLLER_WDG		0xE2
#define CONTROLLER_STAT1	0xE3
#define CHARGERUSB_INT_STATUS	0xE4
#define CHARGERUSB_INT_MASK	0xE5
#define CHARGERUSB_STATUS_INT1	0xE6
#define CHARGERUSB_STATUS_INT2	0xE7
#define CHARGERUSB_CTRL1	0xE8
#define CHARGERUSB_CTRL2	0xE9
#define CHARGERUSB_CTRL3	0xEA
#define CHARGERUSB_STAT1	0xEB
#define CHARGERUSB_VOREG	0xEC
#define CHARGERUSB_VICHRG	0xED
#define CHARGERUSB_CINLIMIT	0xEE
#define CHARGERUSB_CTRLLIMIT1	0xEF

/* CHARGERUSB_VICHRG */
#define CHARGERUSB_VICHRG_500		0x4
#define CHARGERUSB_VICHRG_1500		0xE
/* CHARGERUSB_CINLIMIT */
#define CHARGERUSB_CIN_LIMIT_100	0x1
#define CHARGERUSB_CIN_LIMIT_300	0x5
#define CHARGERUSB_CIN_LIMIT_500	0x9
#define CHARGERUSB_CIN_LIMIT_NONE	0xF
/* CONTROLLER_INT_MASK */
#define MVAC_FAULT		(1 << 6)
#define MAC_EOC			(1 << 5)
#define MBAT_REMOVED		(1 << 4)
#define MFAULT_WDG		(1 << 3)
#define MBAT_TEMP		(1 << 2)
#define MVBUS_DET		(1 << 1)
#define MVAC_DET		(1 << 0)
/* CHARGERUSB_INT_MASK */
#define MASK_MCURRENT_TERM		(1 << 3)
#define MASK_MCHARGERUSB_STAT		(1 << 2)
#define MASK_MCHARGERUSB_THMREG		(1 << 1)
#define MASK_MCHARGERUSB_FAULT		(1 << 0)
/* CHARGERUSB_VOREG */
#define CHARGERUSB_VOREG_3P52		0x01
#define CHARGERUSB_VOREG_4P0		0x19
#define CHARGERUSB_VOREG_4P2		0x23
#define CHARGERUSB_VOREG_4P76		0x3F
/* CHARGERUSB_CTRL1 */
#define SUSPEND_BOOT		(1 << 7)
#define OPA_MODE		(1 << 6)
#define HZ_MODE			(1 << 5)
#define TERM			(1 << 4)
/* CHARGERUSB_CTRL2 */
#define CHARGERUSB_CTRL2_VITERM_50	(0 << 5)
#define CHARGERUSB_CTRL2_VITERM_100	(1 << 5)
#define CHARGERUSB_CTRL2_VITERM_150	(2 << 5)
#define CHARGERUSB_CTRL2_VITERM_400	(7 << 5)
/* CONTROLLER_CTRL1 */
#define CONTROLLER_CTRL1_EN_CHARGER	(1 << 4)
#define CONTROLLER_CTRL1_SEL_CHARGER	(1 << 3)
/* CONTROLLER_STAT1 */
#define CHRG_EXTCHRG_STATZ	(1 << 7)
#define CHRG_DET_N		(1 << 5)
#define VAC_DET			(1 << 3)
#define VBUS_DET		(1 << 2)

#define FG_REG_10	0xCA
#define FG_REG_11	0xCB

#define TOGGLE1		0x90
#define FGS		(1 << 5)
#define FGR		(1 << 4)
#define GPADCS		(1 << 1)
#define GPADCR		(1 << 0)

#define CTRL_P2		0x34
#define CTRL_P2_SP2	(1 << 2)
#define CTRL_P2_EOCP2	(1 << 1)
#define CTRL_P2_BUSY	(1 << 0)

#define TWL6032_CTRL_P1	0x36
#define CTRL_P1_SP1	(1 << 3)

#define GPCH0_LSB	0x57
#define GPCH0_MSB	0x58

#define TWL6032_GPCH0_LSB	0x3b

#define TWL6032_GPSELECT_ISB	0x35

#define USB_PRODUCT_ID_LSB	0x02

#define TWL6030_GPADC_VBAT_CHNL	0x07
#define TWL6032_GPADC_VBAT_CHNL	0x12

#define TWL6030_GPADC_CTRL	0x2e
#define TWL6032_GPADC_CTRL2	0x2f
#define GPADC_CTRL2_CH18_SCALER_EN	(1 << 2)
#define GPADC_CTRL_SCALER_DIV4		(1 << 3)

#define TWL6030_VBAT_MULT	40 * 1000
#define TWL6032_VBAT_MULT	25 * 1000

#define TWL6030_VBAT_SHIFT	(10 + 3)
#define TWL6032_VBAT_SHIFT	(12 + 2)

enum twl603x_chip_type{
	chip_TWL6030,
	chip_TWL6032,
	chip_TWL603X_cnt
};

struct twl6030_data{
	u8 chip_type;
	u8 adc_rbase;
	u8 adc_ctrl;
	u8 adc_enable;
	int vbat_mult;
	int vbat_shift;
};

/* Functions to read and write from TWL6030 */
#ifndef CONFIG_DM_I2C
static inline int twl6030_i2c_write_u8(u8 chip_no, u8 reg, u8 val)
{
	return i2c_write(chip_no, reg, 1, &val, 1);
}

static inline int twl6030_i2c_read_u8(u8 chip_no, u8 reg, u8 *val)
{
	return i2c_read(chip_no, reg, 1, val, 1);
}
#else
int twl6030_i2c_write_u8(u8 chip_no, u8 reg, u8 val);
int twl6030_i2c_read_u8(u8 chip_no, u8 reg, u8 *val);
#endif

/*
 * Power
 */

void twl6030_power_off(void);
void twl6030_init_battery_charging(void);
void twl6030_usb_device_settings(void);
void twl6030_start_usb_charging(void);
void twl6030_stop_usb_charging(void);
int twl6030_get_battery_voltage(void);
int twl6030_get_battery_current(void);
void twl6030_power_mmc_init(int dev_index);

/*
 * Input
 */

int twl6030_input_power_button(void);
int twl6030_input_charger(void);
int twl6030_input_usb(void);

#endif /* TWL6030_H */
