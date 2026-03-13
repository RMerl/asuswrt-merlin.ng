// SPDX-License-Identifier: GPL-2.0
#include <linux/device.h>
#include <linux/module.h>
#include <linux/regmap.h>

#include "cmp201.h"

static bool cmp201_is_writeable_reg(struct device* dev, unsigned int reg) {
  switch (reg) {
    case CMP201_REG_RESET:
    case CMP201_REG_PWR_CTRL:
    case CMP201_REG_FIFO_CONFIG_1:
    case CMP201_REG_INT_CTRL:
    case CMP201_REG_OSR:
    case CMP201_REG_ODR:
    case CMP201_REG_FILTER:
      return true;
    default:
      return false;
  }
}

static bool cmp201_is_volatile_reg(struct device* dev, unsigned int reg) {
  switch (reg) {
    case CMP201_REG_PRESS_XLSB:
    case CMP201_REG_PRESS_LSB:
    case CMP201_REG_PRESS_MSB:
    case CMP201_REG_TEMP_XLSB:
    case CMP201_REG_TEMP_LSB:
    case CMP201_REG_TEMP_MSB:
    case CMP201_REG_STATUS:
      return true;
    default:
      return false;
  }
}
const struct regmap_config cmp201_regmap_config = {
    .reg_bits = 8,
    .val_bits = 8,

    .max_register = CMP201_REG_RESET,
    .cache_type = REGCACHE_RBTREE,

    .writeable_reg = cmp201_is_writeable_reg,
    .volatile_reg = cmp201_is_volatile_reg,
};
EXPORT_SYMBOL(cmp201_regmap_config);
