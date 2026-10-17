#define	PALMAS		0x0
#define TPS659038	0x1
#define TPS65917	0x2

struct palmas_priv {
	struct udevice *chip2;
};

/* I2C device address for pmic palmas */
#define PALMAS_I2C_ADDR	(0x12 >> 1)
#define PALMAS_LDO_NUM		11
#define PALMAS_SMPS_NUM	8

/* Drivers name */
#define PALMAS_LDO_DRIVER     "palmas_ldo"
#define PALMAS_SMPS_DRIVER    "palmas_smps"
#define PALMAS_RST_DRIVER     "palmas_rst"
#define PALMAS_GPIO_DRIVER    "palmas_gpio"

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

#define PALMAS_DEV_CTRL			0xA0
#define   SW_RST			BIT(1)
#define   DEV_OFF			0x00
#define PALMAS_INT3_MASK		0x1B
#define   MASK_VBUS			BIT(7)

/* second chip */
#define PALMAS_GPIO_DATA_IN		0x80
#define PALMAS_GPIO_DATA_DIR		0x81
#define PALMAS_GPIO_DATA_OUT		0x82
#define PALMAS_GPIO_DEBOUNCE_EN		0x83
#define PALMAS_GPIO_CLEAR_DATA_OUT	0x84
#define PALMAS_GPIO_SET_DATA_OUT	0x85
#define PALMAS_PU_PD_GPIO_CTRL1		0x86
#define PALMAS_PU_PD_GPIO_CTRL2		0x87
#define PALMAS_OD_OUTPUT_GPIO_CTRL	0x88
