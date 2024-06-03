#define	PALMAS		0x0
#define TPS659038	0x1
#define TPS65917	0x2

/* I2C device address for pmic palmas */
#define PALMAS_I2C_ADDR	(0x12 >> 1)
#define PALMAS_LDO_NUM		11
#define PALMAS_SMPS_NUM	8

/* Drivers name */
#define PALMAS_LDO_DRIVER     "palmas_ldo"
#define PALMAS_SMPS_DRIVER    "palmas_smps"

#define PALMAS_SMPS_VOLT_MASK		0x7F
#define PALMAS_SMPS_RANGE_MASK		0x80
#define PALMAS_SMPS_VOLT_MAX_HEX	0x7F
#define PALMAS_SMPS_VOLT_MAX		3300000
#define PALMAS_SMPS_MODE_MASK		0x3
#define	PALMAS_SMPS_STATUS_MASK		0x30

#define PALMAS_LDO_VOLT_MASK    0x3F
#define PALMAS_LDO_VOLT_MAX_HEX 0x3F
#define PALMAS_LDO_VOLT_MAX     3300000
#define PALMAS_LDO_MODE_MASK	0x1
#define PALMAS_LDO_STATUS_MASK	0x10
#define PALMAS_LDO_BYPASS_EN	0x40
