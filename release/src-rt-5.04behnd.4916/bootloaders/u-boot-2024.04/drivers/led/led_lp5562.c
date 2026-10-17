// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Doug Zobel <douglas.zobel@climate.com>
 *
 * Driver for TI lp5562 4 channel LED driver.  There are only 3
 * engines available for the 4 LEDs, so white and blue LEDs share
 * the same engine.  This means that the blink period is shared
 * between them.  Changing the period of blue blink will affect
 * the white period (and vice-versa).  Blue and white On/Off
 * states remain independent (as would PWM brightness if that's
 * ever added to the LED core).
 */

#include <dm.h>
#include <errno.h>
#include <led.h>
#include <i2c.h>
#include <asm/gpio.h>
#include <linux/delay.h>

#define DEFAULT_CURRENT			100  /* 10 mA */
#define MIN_BLINK_PERIOD		32   /* ms */
#define MAX_BLINK_PERIOD		2248 /* ms */

/* Register Map */
#define REG_ENABLE			0x00
#define REG_OP_MODE			0x01
#define REG_B_PWM			0x02
#define REG_G_PWM			0x03
#define REG_R_PWM			0x04
#define REG_B_CUR			0x05
#define REG_G_CUR			0x06
#define REG_R_CUR			0x07
#define REG_CONFIG			0x08
#define REG_ENG1_PC			0x09
#define REG_ENG2_PC			0x0A
#define REG_ENG3_PC			0x0B
#define REG_STATUS			0x0C
#define REG_RESET			0x0D
#define REG_W_PWM			0x0E
#define REG_W_CUR			0x0F
#define REG_ENG1_MEM_BEGIN		0x10
#define REG_ENG2_MEM_BEGIN		0x30
#define REG_ENG3_MEM_BEGIN		0x50
#define REG_LED_MAP			0x70

/* LED Register Values */
/* 0x00  ENABLE */
#define REG_ENABLE_CHIP_ENABLE		(0x1 << 6)
#define REG_ENABLE_ENG_EXEC_HOLD	0x0
#define REG_ENABLE_ENG_EXEC_RUN		0x2
#define REG_ENABLE_ENG_EXEC_MASK	0x3

/* 0x01  OP MODE */
#define REG_OP_MODE_DISABLED		0x0
#define REG_OP_MODE_LOAD_SRAM		0x1
#define REG_OP_MODE_RUN			0x2
#define REG_OP_MODE_MASK		0x3

/* 0x02, 0x03, 0x04, 0x0E  PWM */
#define REG_PWM_MIN_VALUE		0
#define REG_PWM_MAX_VALUE		0xFF

/* 0x08  CONFIG */
#define REG_CONFIG_EXT_CLK		0x0
#define REG_CONFIG_INT_CLK		0x1
#define REG_CONFIG_AUTO_CLK		0x2
#define REG_CONFIG_CLK_MASK		0x3

/* 0x0D  RESET */
#define REG_RESET_RESET			0xFF

/* 0x70  LED MAP */
#define REG_LED_MAP_ENG_MASK		0x03
#define REG_LED_MAP_W_ENG_SHIFT		6
#define REG_LED_MAP_R_ENG_SHIFT		4
#define REG_LED_MAP_G_ENG_SHIFT		2
#define REG_LED_MAP_B_ENG_SHIFT		0

/* Engine program related */
#define REG_ENGINE_MEM_SIZE		0x20
#define LED_PGRM_RAMP_INCREMENT_SHIFT	0
#define LED_PGRM_RAMP_SIGN_SHIFT	7
#define LED_PGRM_RAMP_STEP_SHIFT	8
#define LED_PGRM_RAMP_PRESCALE_SHIFT	14

struct lp5562_led_wrap_priv {
	struct gpio_desc enable_gpio;
};

struct lp5562_led_priv {
	u8 reg_pwm;
	u8 reg_current;
	u8 map_shift;
	u8 enginenum;
};

/* enum values map to LED_MAP (0x70) values */
enum lp5562_led_ctl_mode {
	I2C = 0x0,
#ifdef CONFIG_LED_BLINK
	ENGINE1 = 0x1,
	ENGINE2 = 0x2,
	ENGINE3 = 0x3
#endif
};

/*
 * Update a register value
 *  dev     - I2C udevice (parent of led)
 *  regnum  - register number to update
 *  value   - value to write to register
 *  mask    - mask of bits that should be changed
 */
static int lp5562_led_reg_update(struct udevice *dev, int regnum,
				 u8 value, u8 mask)
{
	int ret;

	if (mask == 0xFF)
		ret = dm_i2c_reg_write(dev, regnum, value);
	else
		ret = dm_i2c_reg_clrset(dev, regnum, mask, value);


	/*
	 * Data sheet says "Delay between consecutive I2C writes to
	 * ENABLE register (00h) need to be longer than 488 μs
	 * (typical)." and "Delay between consecutive I2C writes to
	 * OP_MODE register need to be longer than 153 μs (typ)."
	 *
	 * The linux driver does usleep_range(500, 600) and
	 * usleep_range(200, 300), respectively.
	 */
	switch (regnum) {
	case REG_ENABLE:
		udelay(600);
		break;
	case REG_OP_MODE:
		udelay(300);
		break;
	}

	return ret;
}

#ifdef CONFIG_LED_BLINK
/*
 * Program the lp5562 engine
 *  dev     - I2C udevice (parent of led)
 *  program - array of commands
 *  size    - number of commands in program array (1-16)
 *  engine  - engine number (1-3)
 */
static int lp5562_led_program_engine(struct udevice *dev, u16 *program,
				     u8 size, u8 engine)
{
	int ret, cmd;
	u8 engine_reg = REG_ENG1_MEM_BEGIN +
			     ((engine - 1) * REG_ENGINE_MEM_SIZE);
	u8 shift = (3 - engine) * 2;
	__be16 prog_be[16];

	if (size < 1 || size > 16 || engine < 1 || engine > 3)
		return -EINVAL;

	for (cmd = 0; cmd < size; cmd++)
		prog_be[cmd] = cpu_to_be16(program[cmd]);

	/* set engine mode to 'disabled' */
	ret = lp5562_led_reg_update(dev, REG_OP_MODE,
				    REG_OP_MODE_DISABLED << shift,
				    REG_OP_MODE_MASK << shift);
	if (ret != 0)
		goto done;

	/* set exec mode to 'hold' */
	ret = lp5562_led_reg_update(dev, REG_ENABLE,
				    REG_ENABLE_ENG_EXEC_HOLD << shift,
				    REG_ENABLE_ENG_EXEC_MASK << shift);
	if (ret != 0)
		goto done;

	/* set engine mode to 'load SRAM' */
	ret = lp5562_led_reg_update(dev, REG_OP_MODE,
				    REG_OP_MODE_LOAD_SRAM << shift,
				    REG_OP_MODE_MASK << shift);
	if (ret != 0)
		goto done;

	/* send the re-ordered program sequence */
	ret = dm_i2c_write(dev, engine_reg, (uchar *)prog_be, sizeof(u16) * size);
	if (ret != 0)
		goto done;

	/* set engine mode to 'run' */
	ret = lp5562_led_reg_update(dev, REG_OP_MODE,
				    REG_OP_MODE_RUN << shift,
				    REG_OP_MODE_MASK << shift);
	if (ret != 0)
		goto done;

	/* set engine exec to 'run' */
	ret = lp5562_led_reg_update(dev, REG_ENABLE,
				    REG_ENABLE_ENG_EXEC_RUN << shift,
				    REG_ENABLE_ENG_EXEC_MASK << shift);

done:
	return ret;
}

/*
 * Get the LED's current control mode (I2C or ENGINE[1-3])
 *  dev       - led udevice (child udevice)
 */
static enum lp5562_led_ctl_mode lp5562_led_get_control_mode(struct udevice *dev)
{
	struct lp5562_led_priv *priv = dev_get_priv(dev);
	u8 data;
	enum lp5562_led_ctl_mode mode = I2C;

	if (dm_i2c_read(dev_get_parent(dev), REG_LED_MAP, &data, 1) == 0)
		mode = (data & (REG_LED_MAP_ENG_MASK << priv->map_shift))
			>> priv->map_shift;

	return mode;
}
#endif

/*
 * Set the LED's control mode to I2C or ENGINE[1-3]
 *  dev       - led udevice (child udevice)
 *  mode      - mode to change to
 */
static int lp5562_led_set_control_mode(struct udevice *dev,
				       enum lp5562_led_ctl_mode mode)
{
	struct lp5562_led_priv *priv = dev_get_priv(dev);

	return (lp5562_led_reg_update(dev_get_parent(dev), REG_LED_MAP,
				      mode << priv->map_shift,
				      REG_LED_MAP_ENG_MASK << priv->map_shift));
}

/*
 * Return the LED's PWM value;  If LED is in BLINK state, then it is
 * under engine control mode which doesn't use this PWM value.
 *  dev       - led udevice (child udevice)
 */
static int lp5562_led_get_pwm(struct udevice *dev)
{
	struct lp5562_led_priv *priv = dev_get_priv(dev);
	u8 data;

	if (dm_i2c_read(dev_get_parent(dev), priv->reg_pwm, &data, 1) != 0)
		return -EINVAL;

	return data;
}

/*
 * Set the LED's PWM value and configure it to use this (I2C mode).
 *  dev       - led udevice (child udevice)
 *  value     - PWM value (0 - 255)
 */
static int lp5562_led_set_pwm(struct udevice *dev, u8 value)
{
	struct lp5562_led_priv *priv = dev_get_priv(dev);

	if (lp5562_led_reg_update(dev_get_parent(dev), priv->reg_pwm,
				  value, 0xff) != 0)
		return -EINVAL;

	/* set LED to I2C register mode */
	return lp5562_led_set_control_mode(dev, I2C);
}

/*
 * Return the led's current state
 *  dev     - led udevice (child udevice)
 *
 */
static enum led_state_t lp5562_led_get_state(struct udevice *dev)
{
	enum led_state_t state = LEDST_ON;

	if (lp5562_led_get_pwm(dev) == REG_PWM_MIN_VALUE)
		state = LEDST_OFF;

#ifdef CONFIG_LED_BLINK
	if (lp5562_led_get_control_mode(dev) != I2C)
		state = LEDST_BLINK;
#endif

	return state;
}

/*
 * Set the led state
 *  dev     - led udevice (child udevice)
 *  state   - State to set the LED to
 */
static int lp5562_led_set_state(struct udevice *dev, enum led_state_t state)
{
#ifdef CONFIG_LED_BLINK
	struct lp5562_led_priv *priv = dev_get_priv(dev);
#endif

	switch (state) {
	case LEDST_OFF:
		return lp5562_led_set_pwm(dev, REG_PWM_MIN_VALUE);
	case LEDST_ON:
		return lp5562_led_set_pwm(dev, REG_PWM_MAX_VALUE);
#ifdef CONFIG_LED_BLINK
	case LEDST_BLINK:
		return lp5562_led_set_control_mode(dev, priv->enginenum);
#endif
	case LEDST_TOGGLE:
		if (lp5562_led_get_state(dev) == LEDST_OFF)
			return lp5562_led_set_state(dev, LEDST_ON);
		else
			return lp5562_led_set_state(dev, LEDST_OFF);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

#ifdef CONFIG_LED_BLINK
/*
 * Set the blink period of an LED; note blue and white share the same
 * engine so changing the period of one affects the other.
 *  dev       - led udevice (child udevice)
 *  period_ms - blink period in ms
 */
static int lp5562_led_set_period(struct udevice *dev, int period_ms)
{
	struct lp5562_led_priv *priv = dev_get_priv(dev);
	u8 opcode = 0;
	u16 program[7];
	u16 wait_time;

	/* Blink is implemented as an engine program.  Simple on/off
	 * for short periods, or fade in/fade out for longer periods:
	 *
	 *  if (period_ms < 500):
	 *    set PWM to 100%
	 *    pause for period / 2
	 *    set PWM to 0%
	 *    pause for period / 2
	 *    goto start
	 *
	 *  else
	 *    raise PWM 0% -> 50% in 62.7 ms
	 *    raise PWM 50% -> 100% in 62.7 ms
	 *    pause for (period - 4 * 62.7) / 2
	 *    lower PWM 100% -> 50% in 62.7 ms
	 *    lower PWM 50% -> 0% in 62.7 ms
	 *    pause for (period - 4 * 62.7) / 2
	 *    goto start
	 */

	if (period_ms < MIN_BLINK_PERIOD)
		period_ms = MIN_BLINK_PERIOD;
	else if (period_ms > MAX_BLINK_PERIOD)
		period_ms = MAX_BLINK_PERIOD;

	if (period_ms < 500) {
		/* Simple on/off blink */
		wait_time = period_ms / 2;

		/* 1st command is full brightness */
		program[opcode++] =
			(1 << LED_PGRM_RAMP_PRESCALE_SHIFT) |
			REG_PWM_MAX_VALUE;

		/* 2nd command is wait (period / 2) using 15.6ms steps */
		program[opcode++] =
			(1 << LED_PGRM_RAMP_PRESCALE_SHIFT) |
			(((wait_time * 10) / 156) << LED_PGRM_RAMP_STEP_SHIFT) |
			(0 << LED_PGRM_RAMP_INCREMENT_SHIFT);

		/* 3rd command is 0% brightness */
		program[opcode++] =
			(1 << LED_PGRM_RAMP_PRESCALE_SHIFT);

		/* 4th command is wait (period / 2) using 15.6ms steps */
		program[opcode++] =
			(1 << LED_PGRM_RAMP_PRESCALE_SHIFT) |
			(((wait_time * 10) / 156) << LED_PGRM_RAMP_STEP_SHIFT) |
			(0 << LED_PGRM_RAMP_INCREMENT_SHIFT);

		/* 5th command: repeat */
		program[opcode++] = 0x00;
	} else {
		/* fade-in / fade-out blink */
		wait_time = ((period_ms - 251) / 2);

		/* ramp up time is 256 * 0.49ms (125.4ms) done in 2 steps */
		/* 1st command is ramp up 1/2 way */
		program[opcode++] =
			(0 << LED_PGRM_RAMP_PRESCALE_SHIFT) |
			(1 << LED_PGRM_RAMP_STEP_SHIFT) |
			(127 << LED_PGRM_RAMP_INCREMENT_SHIFT);

		/* 2nd command is ramp up rest of the way */
		program[opcode++] =
			(0 << LED_PGRM_RAMP_PRESCALE_SHIFT) |
			(1 << LED_PGRM_RAMP_STEP_SHIFT) |
			(127 << LED_PGRM_RAMP_INCREMENT_SHIFT);

		/* 3rd: wait ((period - 2 * ramp_time) / 2) (15.6ms steps) */
		program[opcode++] =
			(1 << LED_PGRM_RAMP_PRESCALE_SHIFT) |
			(((wait_time * 10) / 156) << LED_PGRM_RAMP_STEP_SHIFT) |
			(0 << LED_PGRM_RAMP_INCREMENT_SHIFT);

		/* ramp down is same as ramp up with sign bit set */
		/* 4th command is ramp down 1/2 way */
		program[opcode++] =
			(0 << LED_PGRM_RAMP_PRESCALE_SHIFT) |
			(1 << LED_PGRM_RAMP_STEP_SHIFT) |
			(1 << LED_PGRM_RAMP_SIGN_SHIFT) |
			(127 << LED_PGRM_RAMP_INCREMENT_SHIFT);

		/* 5th command is ramp down rest of the way */
		program[opcode++] =
			(0 << LED_PGRM_RAMP_PRESCALE_SHIFT) |
			(1 << LED_PGRM_RAMP_STEP_SHIFT) |
			(1 << LED_PGRM_RAMP_SIGN_SHIFT) |
			(127 << LED_PGRM_RAMP_INCREMENT_SHIFT);

		/* 6th: wait ((period - 2 * ramp_time) / 2) (15.6ms steps) */
		program[opcode++] =
			(1 << LED_PGRM_RAMP_PRESCALE_SHIFT) |
			(((wait_time * 10) / 156) << LED_PGRM_RAMP_STEP_SHIFT) |
			(0 << LED_PGRM_RAMP_INCREMENT_SHIFT);

		/* 7th command: repeat */
		program[opcode++] = 0x00;
	}

	return lp5562_led_program_engine(dev_get_parent(dev), program,
					 opcode, priv->enginenum);
}
#endif

static const struct led_ops lp5562_led_ops = {
	.get_state = lp5562_led_get_state,
	.set_state = lp5562_led_set_state,
#ifdef CONFIG_LED_BLINK
	.set_period = lp5562_led_set_period,
#endif
};

static int lp5562_led_probe(struct udevice *dev)
{
	struct lp5562_led_priv *priv = dev_get_priv(dev);
	u8 current;
	int ret = 0;

	/* Child LED nodes */
	switch (dev_read_addr(dev)) {
	case 0:
		priv->reg_current = REG_R_CUR;
		priv->reg_pwm = REG_R_PWM;
		priv->map_shift = REG_LED_MAP_R_ENG_SHIFT;
		priv->enginenum = 1;
		break;
	case 1:
		priv->reg_current = REG_G_CUR;
		priv->reg_pwm = REG_G_PWM;
		priv->map_shift = REG_LED_MAP_G_ENG_SHIFT;
		priv->enginenum = 2;
		break;
	case 2:
		priv->reg_current = REG_B_CUR;
		priv->reg_pwm = REG_B_PWM;
		priv->map_shift = REG_LED_MAP_B_ENG_SHIFT;
		priv->enginenum = 3; /* shared with white */
		break;
	case 3:
		priv->reg_current = REG_W_CUR;
		priv->map_shift = REG_LED_MAP_W_ENG_SHIFT;
		priv->enginenum = 3; /* shared with blue */
		break;
	default:
		return -EINVAL;
	}

	current = dev_read_u8_default(dev, "max-cur", DEFAULT_CURRENT);

	ret = lp5562_led_reg_update(dev_get_parent(dev), priv->reg_current,
				    current, 0xff);

	return ret;
}

static int lp5562_led_bind(struct udevice *dev)
{
	struct led_uc_plat *uc_plat = dev_get_uclass_plat(dev);

	/*
	 * For the child nodes, parse a "chan-name" property, since
	 * the DT bindings for this device use that instead of
	 * "label".
	 */
	uc_plat->label = dev_read_string(dev, "chan-name");

	return 0;
}

U_BOOT_DRIVER(lp5562_led) = {
	.name = "lp5562-led",
	.id = UCLASS_LED,
	.bind = lp5562_led_bind,
	.probe = lp5562_led_probe,
	.priv_auto = sizeof(struct lp5562_led_priv),
	.ops = &lp5562_led_ops,
};


static int lp5562_led_wrap_probe(struct udevice *dev)
{
	struct lp5562_led_wrap_priv *priv = dev_get_priv(dev);
	u8 clock_mode;
	int ret;

	/* Enable gpio if needed */
	if (gpio_request_by_name(dev, "enabled-gpios", 0,
				 &priv->enable_gpio, GPIOD_IS_OUT) == 0) {
		dm_gpio_set_value(&priv->enable_gpio, 1);
		udelay(1000);
	}

	/* Ensure all registers have default values. */
	ret = lp5562_led_reg_update(dev, REG_RESET, REG_RESET_RESET, 0xff);
	if (ret)
		return ret;
	udelay(10000);

	/* Enable the chip */
	ret = lp5562_led_reg_update(dev, REG_ENABLE, REG_ENABLE_CHIP_ENABLE, 0xff);
	if (ret)
		return ret;

	/*
	 * The DT bindings say 0=auto, 1=internal, 2=external, while
	 * the register[0:1] values are 0=external, 1=internal,
	 * 2=auto.
	 */
	clock_mode = dev_read_u8_default(dev, "clock-mode", 0);
	ret = lp5562_led_reg_update(dev, REG_CONFIG, 2 - clock_mode, REG_CONFIG_CLK_MASK);

	return ret;
}

static int lp5562_led_wrap_bind(struct udevice *dev)
{
	return led_bind_generic(dev, "lp5562-led");
}

static const struct udevice_id lp5562_led_ids[] = {
	{ .compatible = "ti,lp5562" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(lp5562_led_wrap) = {
	.name = "lp5562-led-wrap",
	.id = UCLASS_NOP,
	.of_match = lp5562_led_ids,
	.bind = lp5562_led_wrap_bind,
	.probe = lp5562_led_wrap_probe,
	.priv_auto = sizeof(struct lp5562_led_wrap_priv),
};
