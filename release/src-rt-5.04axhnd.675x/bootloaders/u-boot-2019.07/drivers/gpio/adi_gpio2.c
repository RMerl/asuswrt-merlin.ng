/*
 * ADI GPIO2 Abstraction Layer
 * Support BF54x, BF60x and future processors.
 *
 * Copyright 2008-2013 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later
 */

#include <common.h>
#include <linux/errno.h>
#include <asm/gpio.h>

#define RESOURCE_LABEL_SIZE	16

static struct str_ident {
	char name[RESOURCE_LABEL_SIZE];
} str_ident[MAX_RESOURCES];

static void gpio_error(unsigned gpio)
{
	printf("adi_gpio2: GPIO %d wasn't requested!\n", gpio);
}

static void set_label(unsigned short ident, const char *label)
{
	if (label) {
		strncpy(str_ident[ident].name, label,
			RESOURCE_LABEL_SIZE);
		str_ident[ident].name[RESOURCE_LABEL_SIZE - 1] = 0;
	}
}

static char *get_label(unsigned short ident)
{
	return *str_ident[ident].name ? str_ident[ident].name : "UNKNOWN";
}

static int cmp_label(unsigned short ident, const char *label)
{
	if (label == NULL)
		printf("adi_gpio2: please provide none-null label\n");

	if (label)
		return strcmp(str_ident[ident].name, label);
	else
		return -EINVAL;
}

#define map_entry(m, i)      reserved_##m##_map[gpio_bank(i)]
#define is_reserved(m, i, e) (map_entry(m, i) & gpio_bit(i))
#define reserve(m, i)        (map_entry(m, i) |= gpio_bit(i))
#define unreserve(m, i)      (map_entry(m, i) &= ~gpio_bit(i))
#define DECLARE_RESERVED_MAP(m, c) unsigned short reserved_##m##_map[c]

static DECLARE_RESERVED_MAP(gpio, GPIO_BANK_NUM);
static DECLARE_RESERVED_MAP(peri, gpio_bank(MAX_RESOURCES));

inline int check_gpio(unsigned gpio)
{
#if defined(CONFIG_BF54x)
	if (gpio == GPIO_PB15 || gpio == GPIO_PC14 || gpio == GPIO_PC15 ||
		gpio == GPIO_PH14 || gpio == GPIO_PH15 ||
		gpio == GPIO_PJ14 || gpio == GPIO_PJ15)
		return -EINVAL;
#endif
	if (gpio >= MAX_GPIOS)
		return -EINVAL;
	return 0;
}

static void port_setup(unsigned gpio, unsigned short usage)
{
#if defined(CONFIG_BF54x)
	if (usage == GPIO_USAGE)
		gpio_array[gpio_bank(gpio)]->port_fer &= ~gpio_bit(gpio);
	else
		gpio_array[gpio_bank(gpio)]->port_fer |= gpio_bit(gpio);
#else
	if (usage == GPIO_USAGE)
		gpio_array[gpio_bank(gpio)]->port_fer_clear = gpio_bit(gpio);
	else
		gpio_array[gpio_bank(gpio)]->port_fer_set = gpio_bit(gpio);
#endif
}

inline void portmux_setup(unsigned short per)
{
	u32 pmux;
	u16 ident = P_IDENT(per);
	u16 function = P_FUNCT2MUX(per);

	pmux = gpio_array[gpio_bank(ident)]->port_mux;

	pmux &= ~(0x3 << (2 * gpio_sub_n(ident)));
	pmux |= (function & 0x3) << (2 * gpio_sub_n(ident));

	gpio_array[gpio_bank(ident)]->port_mux = pmux;
}

inline u16 get_portmux(unsigned short per)
{
	u32 pmux;
	u16 ident = P_IDENT(per);

	pmux = gpio_array[gpio_bank(ident)]->port_mux;

	return pmux >> (2 * gpio_sub_n(ident)) & 0x3;
}

unsigned short get_gpio_dir(unsigned gpio)
{
	return 0x01 &
		(gpio_array[gpio_bank(gpio)]->dir_clear >> gpio_sub_n(gpio));
}

/***********************************************************
*
* FUNCTIONS:	Peripheral Resource Allocation
*		and PortMux Setup
*
* INPUTS/OUTPUTS:
* per	Peripheral Identifier
* label	String
*
* DESCRIPTION: Peripheral Resource Allocation and Setup API
**************************************************************/

int peripheral_request(unsigned short per, const char *label)
{
	unsigned short ident = P_IDENT(per);

	/*
	 * Don't cares are pins with only one dedicated function
	 */

	if (per & P_DONTCARE)
		return 0;

	if (!(per & P_DEFINED))
		return -EINVAL;

	BUG_ON(ident >= MAX_RESOURCES);

	/* If a pin can be muxed as either GPIO or peripheral, make
	 * sure it is not already a GPIO pin when we request it.
	 */
	if (unlikely(!check_gpio(ident) && is_reserved(gpio, ident, 1))) {
		printf("%s: Peripheral %d is already reserved as GPIO by %s!\n",
		       __func__, ident, get_label(ident));
		return -EBUSY;
	}

	if (unlikely(is_reserved(peri, ident, 1))) {
		/*
		 * Pin functions like AMC address strobes my
		 * be requested and used by several drivers
		 */

		if (!((per & P_MAYSHARE) &&
			get_portmux(per) == P_FUNCT2MUX(per))) {
			/*
			 * Allow that the identical pin function can
			 * be requested from the same driver twice
			 */

			if (cmp_label(ident, label) == 0)
				goto anyway;

			printf("%s: Peripheral %d function %d is already "
				"reserved by %s!\n", __func__, ident,
				P_FUNCT2MUX(per), get_label(ident));
			return -EBUSY;
		}
	}

 anyway:
	reserve(peri, ident);

	portmux_setup(per);
	port_setup(ident, PERIPHERAL_USAGE);

	set_label(ident, label);

	return 0;
}

int peripheral_request_list(const unsigned short per[], const char *label)
{
	u16 cnt;
	int ret;

	for (cnt = 0; per[cnt] != 0; cnt++) {
		ret = peripheral_request(per[cnt], label);

		if (ret < 0) {
			for (; cnt > 0; cnt--)
				peripheral_free(per[cnt - 1]);

			return ret;
		}
	}

	return 0;
}

void peripheral_free(unsigned short per)
{
	unsigned short ident = P_IDENT(per);

	if (per & P_DONTCARE)
		return;

	if (!(per & P_DEFINED))
		return;

	if (unlikely(!is_reserved(peri, ident, 0)))
		return;

	if (!(per & P_MAYSHARE))
		port_setup(ident, GPIO_USAGE);

	unreserve(peri, ident);

	set_label(ident, "free");
}

void peripheral_free_list(const unsigned short per[])
{
	u16 cnt;
	for (cnt = 0; per[cnt] != 0; cnt++)
		peripheral_free(per[cnt]);
}

/***********************************************************
*
* FUNCTIONS: GPIO Driver
*
* INPUTS/OUTPUTS:
* gpio	PIO Number between 0 and MAX_GPIOS
* label	String
*
* DESCRIPTION: GPIO Driver API
**************************************************************/

int gpio_request(unsigned gpio, const char *label)
{
	if (check_gpio(gpio) < 0)
		return -EINVAL;

	/*
	 * Allow that the identical GPIO can
	 * be requested from the same driver twice
	 * Do nothing and return -
	 */

	if (cmp_label(gpio, label) == 0)
		return 0;

	if (unlikely(is_reserved(gpio, gpio, 1))) {
		printf("adi_gpio2: GPIO %d is already reserved by %s!\n",
			gpio, get_label(gpio));
		return -EBUSY;
	}
	if (unlikely(is_reserved(peri, gpio, 1))) {
		printf("adi_gpio2: GPIO %d is already reserved as Peripheral "
			"by %s!\n", gpio, get_label(gpio));
		return -EBUSY;
	}

	reserve(gpio, gpio);
	set_label(gpio, label);

	port_setup(gpio, GPIO_USAGE);

	return 0;
}

int gpio_free(unsigned gpio)
{
	if (check_gpio(gpio) < 0)
		return -1;

	if (unlikely(!is_reserved(gpio, gpio, 0))) {
		gpio_error(gpio);
		return -1;
	}

	unreserve(gpio, gpio);

	set_label(gpio, "free");

	return 0;
}

#ifdef ADI_SPECIAL_GPIO_BANKS
static DECLARE_RESERVED_MAP(special_gpio, gpio_bank(MAX_RESOURCES));

int special_gpio_request(unsigned gpio, const char *label)
{
	/*
	 * Allow that the identical GPIO can
	 * be requested from the same driver twice
	 * Do nothing and return -
	 */

	if (cmp_label(gpio, label) == 0)
		return 0;

	if (unlikely(is_reserved(special_gpio, gpio, 1))) {
		printf("adi_gpio2: GPIO %d is already reserved by %s!\n",
			gpio, get_label(gpio));
		return -EBUSY;
	}
	if (unlikely(is_reserved(peri, gpio, 1))) {
		printf("adi_gpio2: GPIO %d is already reserved as Peripheral "
			"by %s!\n", gpio, get_label(gpio));

		return -EBUSY;
	}

	reserve(special_gpio, gpio);
	reserve(peri, gpio);

	set_label(gpio, label);
	port_setup(gpio, GPIO_USAGE);

	return 0;
}

void special_gpio_free(unsigned gpio)
{
	if (unlikely(!is_reserved(special_gpio, gpio, 0))) {
		gpio_error(gpio);
		return;
	}

	unreserve(special_gpio, gpio);
	unreserve(peri, gpio);
	set_label(gpio, "free");
}
#endif

static inline void __gpio_direction_input(unsigned gpio)
{
	gpio_array[gpio_bank(gpio)]->dir_clear = gpio_bit(gpio);
#if defined(CONFIG_BF54x)
	gpio_array[gpio_bank(gpio)]->inen |= gpio_bit(gpio);
#else
	gpio_array[gpio_bank(gpio)]->inen_set = gpio_bit(gpio);
#endif
}

int gpio_direction_input(unsigned gpio)
{
	unsigned long flags;

	if (!is_reserved(gpio, gpio, 0)) {
		gpio_error(gpio);
		return -EINVAL;
	}

	local_irq_save(flags);
	__gpio_direction_input(gpio);
	local_irq_restore(flags);

	return 0;
}

int gpio_set_value(unsigned gpio, int arg)
{
	if (arg)
		gpio_array[gpio_bank(gpio)]->data_set = gpio_bit(gpio);
	else
		gpio_array[gpio_bank(gpio)]->data_clear = gpio_bit(gpio);

	return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
	unsigned long flags;

	if (!is_reserved(gpio, gpio, 0)) {
		gpio_error(gpio);
		return -EINVAL;
	}

	local_irq_save(flags);

#if defined(CONFIG_BF54x)
	gpio_array[gpio_bank(gpio)]->inen &= ~gpio_bit(gpio);
#else
	gpio_array[gpio_bank(gpio)]->inen_clear = gpio_bit(gpio);
#endif
	gpio_set_value(gpio, value);
	gpio_array[gpio_bank(gpio)]->dir_set = gpio_bit(gpio);

	local_irq_restore(flags);

	return 0;
}

int gpio_get_value(unsigned gpio)
{
	return 1 & (gpio_array[gpio_bank(gpio)]->data >> gpio_sub_n(gpio));
}

void gpio_labels(void)
{
	int c, gpio;

	for (c = 0; c < MAX_RESOURCES; c++) {
		gpio = is_reserved(gpio, c, 1);
		if (!check_gpio(c) && gpio)
			printf("GPIO_%d:\t%s\tGPIO %s\n", c, get_label(c),
				get_gpio_dir(c) ? "OUTPUT" : "INPUT");
		else if (is_reserved(peri, c, 1))
			printf("GPIO_%d:\t%s\tPeripheral\n", c, get_label(c));
		else
			continue;
	}
}
