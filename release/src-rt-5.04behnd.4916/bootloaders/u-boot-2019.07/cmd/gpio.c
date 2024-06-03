/*
 * Control GPIO pins on the fly
 *
 * Copyright (c) 2008-2011 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <command.h>
#include <errno.h>
#include <dm.h>
#include <asm/gpio.h>

__weak int name_to_gpio(const char *name)
{
	return simple_strtoul(name, NULL, 10);
}

enum gpio_cmd {
	GPIOC_INPUT,
	GPIOC_SET,
	GPIOC_CLEAR,
	GPIOC_TOGGLE,
};

#if defined(CONFIG_DM_GPIO) && !defined(gpio_status)

/* A few flags used by show_gpio() */
enum {
	FLAG_SHOW_ALL		= 1 << 0,
	FLAG_SHOW_BANK		= 1 << 1,
	FLAG_SHOW_NEWLINE	= 1 << 2,
};

static void gpio_get_description(struct udevice *dev, const char *bank_name,
				 int offset, int *flagsp, bool show_all)
{
	char buf[80];
	int ret;

	ret = gpio_get_function(dev, offset, NULL);
	if (ret < 0)
		goto err;
	if (!show_all && !(*flagsp & FLAG_SHOW_ALL) && ret == GPIOF_UNUSED)
		return;
	if ((*flagsp & FLAG_SHOW_BANK) && bank_name) {
		if (*flagsp & FLAG_SHOW_NEWLINE) {
			putc('\n');
			*flagsp &= ~FLAG_SHOW_NEWLINE;
		}
		printf("Bank %s:\n", bank_name);
		*flagsp &= ~FLAG_SHOW_BANK;
	}

	ret = gpio_get_status(dev, offset, buf, sizeof(buf));
	if (ret)
		goto err;

	printf("%s\n", buf);
	return;
err:
	printf("Error %d\n", ret);
}

static int do_gpio_status(bool all, const char *gpio_name)
{
	struct udevice *dev;
	int banklen;
	int flags;
	int ret;

	flags = 0;
	if (gpio_name && !*gpio_name)
		gpio_name = NULL;
	for (ret = uclass_first_device(UCLASS_GPIO, &dev);
	     dev;
	     ret = uclass_next_device(&dev)) {
		const char *bank_name;
		int num_bits;

		flags |= FLAG_SHOW_BANK;
		if (all)
			flags |= FLAG_SHOW_ALL;
		bank_name = gpio_get_bank_info(dev, &num_bits);
		if (!num_bits) {
			debug("GPIO device %s has no bits\n", dev->name);
			continue;
		}
		banklen = bank_name ? strlen(bank_name) : 0;

		if (!gpio_name || !bank_name ||
		    !strncasecmp(gpio_name, bank_name, banklen)) {
			const char *p = NULL;
			int offset;

			p = gpio_name + banklen;
			if (gpio_name && *p) {
				offset = simple_strtoul(p, NULL, 10);
				gpio_get_description(dev, bank_name, offset,
						     &flags, true);
			} else {
				for (offset = 0; offset < num_bits; offset++) {
					gpio_get_description(dev, bank_name,
						     offset, &flags, false);
				}
			}
		}
		/* Add a newline between bank names */
		if (!(flags & FLAG_SHOW_BANK))
			flags |= FLAG_SHOW_NEWLINE;
	}

	return ret;
}
#endif

static int do_gpio(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int gpio;
	enum gpio_cmd sub_cmd;
	int value;
	const char *str_cmd, *str_gpio = NULL;
	int ret;
#ifdef CONFIG_DM_GPIO
	bool all = false;
#endif

	if (argc < 2)
 show_usage:
		return CMD_RET_USAGE;
	str_cmd = argv[1];
	argc -= 2;
	argv += 2;
#ifdef CONFIG_DM_GPIO
	if (argc > 0 && !strcmp(*argv, "-a")) {
		all = true;
		argc--;
		argv++;
	}
#endif
	if (argc > 0)
		str_gpio = *argv;
	if (!strncmp(str_cmd, "status", 2)) {
		/* Support deprecated gpio_status() */
#ifdef gpio_status
		gpio_status();
		return 0;
#elif defined(CONFIG_DM_GPIO)
		return cmd_process_error(cmdtp, do_gpio_status(all, str_gpio));
#else
		goto show_usage;
#endif
	}

	if (!str_gpio)
		goto show_usage;

	/* parse the behavior */
	switch (*str_cmd) {
	case 'i':
		sub_cmd = GPIOC_INPUT;
		break;
	case 's':
		sub_cmd = GPIOC_SET;
		break;
	case 'c':
		sub_cmd = GPIOC_CLEAR;
		break;
	case 't':
		sub_cmd = GPIOC_TOGGLE;
		break;
	default:
		goto show_usage;
	}

#if defined(CONFIG_DM_GPIO)
	/*
	 * TODO(sjg@chromium.org): For now we must fit into the existing GPIO
	 * framework, so we look up the name here and convert it to a GPIO number.
	 * Once all GPIO drivers are converted to driver model, we can change the
	 * code here to use the GPIO uclass interface instead of the numbered
	 * GPIO compatibility layer.
	 */
	ret = gpio_lookup_name(str_gpio, NULL, NULL, &gpio);
	if (ret) {
		printf("GPIO: '%s' not found\n", str_gpio);
		return cmd_process_error(cmdtp, ret);
	}
#else
	/* turn the gpio name into a gpio number */
	gpio = name_to_gpio(str_gpio);
	if (gpio < 0)
		goto show_usage;
#endif
	/* grab the pin before we tweak it */
	ret = gpio_request(gpio, "cmd_gpio");
	if (ret && ret != -EBUSY) {
		printf("gpio: requesting pin %u failed\n", gpio);
		return -1;
	}

	/* finally, let's do it: set direction and exec command */
	if (sub_cmd == GPIOC_INPUT) {
		gpio_direction_input(gpio);
		value = gpio_get_value(gpio);
	} else {
		switch (sub_cmd) {
		case GPIOC_SET:
			value = 1;
			break;
		case GPIOC_CLEAR:
			value = 0;
			break;
		case GPIOC_TOGGLE:
			value = gpio_get_value(gpio);
			if (!IS_ERR_VALUE(value))
				value = !value;
			break;
		default:
			goto show_usage;
		}
		gpio_direction_output(gpio, value);
	}
	printf("gpio: pin %s (gpio %u) value is ", str_gpio, gpio);
	if (IS_ERR_VALUE(value))
		printf("unknown (ret=%d)\n", value);
	else
		printf("%d\n", value);
	if (sub_cmd != GPIOC_INPUT && !IS_ERR_VALUE(value)) {
		int nval = gpio_get_value(gpio);

		if (IS_ERR_VALUE(nval))
			printf("   Warning: no access to GPIO output value\n");
		else if (nval != value)
			printf("   Warning: value of pin is still %d\n", nval);
	}

	if (ret != -EBUSY)
		gpio_free(gpio);

	return value;
}

U_BOOT_CMD(gpio, 4, 0, do_gpio,
	   "query and control gpio pins",
	   "<input|set|clear|toggle> <pin>\n"
	   "    - input/set/clear/toggle the specified pin\n"
	   "gpio status [-a] [<bank> | <pin>]  - show [all/claimed] GPIOs");
