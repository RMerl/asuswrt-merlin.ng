/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/bitops.h>
#include <linux/device.h>
#include <linux/regmap.h>

/* -----  Sensor Dara Resolution  ----- */
#define CMP201_PRESS_SENSITIVITY (64)    // (unit : count/Pa)
#define CMP201_TEMP_SENSITIVITY (65536)  // (unit : count/Celsius)

// #define CMP201_T_CODE_TO_CELSIUS(tCode) ((float)(tCode) / CMP201_TEMP_SENSITIVITY)
// #define CMP201_P_CODE_TO_PA(pCode) ((float)(pCode) / CMP201_PRESS_SENSITIVITY)

// For FPU-Safe
#define SCALE_X_1000 (1000)

#define CMP201_P_CODE_TO_MPASCAL(pCode)                                 \
  ((s32)(((s64)(pCode) * SCALE_X_1000 + CMP201_PRESS_SENSITIVITY / 2) / \
         CMP201_PRESS_SENSITIVITY))

#define CMP201_T_CODE_TO_MCELSIUS(tCode)                               \
  ((s32)(((s64)(tCode) * SCALE_X_1000 + CMP201_TEMP_SENSITIVITY / 2) / \
         CMP201_TEMP_SENSITIVITY))

/* -----  Registers Address (RAM) ----- */
#define CMP201_REG_CHIP_ID 0x00
#define CMP201_REG_REV_ID 0x01
#define CMP201_REG_ERR_MSG 0x02
#define CMP201_REG_STATUS 0x03
#define CMP201_REG_PRESS_XLSB 0x04
#define CMP201_REG_PRESS_LSB 0x05
#define CMP201_REG_PRESS_MSB 0x06
#define CMP201_REG_TEMP_XLSB 0x07
#define CMP201_REG_TEMP_LSB 0x08
#define CMP201_REG_TEMP_MSB 0x09
#define CMP201_REG_SENSOR_TIME_0 0x0C
#define CMP201_REG_SENSOR_TIME_1 0x0D
#define CMP201_REG_SENSOR_TIME_2 0x0E
#define CMP201_REG_INT_STATUS 0x11
#define CMP201_REG_FIFO_LENGTH_0 0x12
#define CMP201_REG_FIFO_LENGTH_1 0x13
#define CMP201_REG_FIFO_DATA 0x14
#define CMP201_REG_FIFO_WM_0 0x15
#define CMP201_REG_FIFO_WM_1 0x16
#define CMP201_REG_FIFO_CONFIG_0 0x17
#define CMP201_REG_FIFO_CONFIG_1 0x18
#define CMP201_REG_INT_CTRL 0x19
#define CMP201_REG_CONFIG 0x1A
#define CMP201_REG_PWR_CTRL 0x1B
#define CMP201_REG_OSR 0x1C
#define CMP201_REG_ODR 0x1D
#define CMP201_REG_FILTER 0x1F
#define CMP201_REG_PRIMIF 0x22
#define CMP201_REG_RESET 0x7E
/* -----  Sensor PID  ----- */
#define CMP201_CHIP_ID 0xA0

/* -----  Sensor Version  ----- */
#define CMP201_CHIP_Ver 0x80

/* Regmap configurations */
extern const struct regmap_config cmp201_regmap_config;

/* Probe called from different transports */
int cmp201_common_probe(struct device* dev, struct regmap* regmap,
                        unsigned int chip, const char* name, int irq);
int cmp201_common_remove(struct device* dev);

/* PM ops */
extern const struct dev_pm_ops cmp201_dev_pm_ops;
