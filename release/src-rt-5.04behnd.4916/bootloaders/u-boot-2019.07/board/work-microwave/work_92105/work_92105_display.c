// SPDX-License-Identifier: GPL-2.0+
/*
 * work_92105 display support
 *
 * (C) Copyright 2014  DENX Software Engineering GmbH
 * Written-by: Albert ARIBAUD <albert.aribaud@3adev.fr>
 *
 * The work_92105 display is a HD44780-compatible module
 * controlled through a MAX6957AAX SPI port expander, two
 * MAX518 I2C DACs and native LPC32xx GPO 15.
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/cpu.h>
#include <asm/arch/emc.h>
#include <asm/gpio.h>
#include <spi.h>
#include <i2c.h>
#include <version.h>
#include <vsprintf.h>

/*
 * GPO 15 in port 3 is gpio 3*32+15 = 111
 */

#define GPO_15 111

/**
 * MAX6957AAX registers that we will be using
 */

#define MAX6957_CONF		0x04

#define MAX6957_CONF_08_11	0x0A
#define MAX6957_CONF_12_15	0x0B
#define MAX6957_CONF_16_19	0x0C

/**
 * Individual gpio ports (one per gpio) to HD44780
 */

#define MAX6957AAX_HD44780_RS	0x29
#define MAX6957AAX_HD44780_R_W	0x2A
#define MAX6957AAX_HD44780_EN	0x2B
#define MAX6957AAX_HD44780_DATA	0x4C

/**
 * Display controller instructions
 */

/* Function set: eight bits, two lines, 8-dot font */
#define HD44780_FUNCTION_SET		0x38

/* Display ON / OFF: turn display on */
#define HD44780_DISPLAY_ON_OFF_CONTROL	0x0C

/* Entry mode: increment */
#define HD44780_ENTRY_MODE_SET		0x06

/* Clear */
#define HD44780_CLEAR_DISPLAY		0x01

/* Set DDRAM addr (to be ORed with exact address) */
#define HD44780_SET_DDRAM_ADDR		0x80

/* Set CGRAM addr (to be ORed with exact address) */
#define HD44780_SET_CGRAM_ADDR		0x40

/**
 * Default value for contrats
 */

#define CONTRAST_DEFAULT  25

/**
 * Define slave as a module-wide local to save passing it around,
 * plus we will need it after init for the "hd44780" command.
 */

static struct spi_slave *slave;

/*
 * Write a value into a MAX6957AAX register.
 */

static void max6957aax_write(uint8_t reg, uint8_t value)
{
	uint8_t dout[2];

	dout[0] = reg;
	dout[1] = value;
	gpio_set_value(GPO_15, 0);
	/* do SPI read/write (passing din==dout is OK) */
	spi_xfer(slave, 16, dout, dout, SPI_XFER_BEGIN | SPI_XFER_END);
	gpio_set_value(GPO_15, 1);
}

/*
 * Read a value from a MAX6957AAX register.
 *
 * According to the MAX6957AAX datasheet, we should release the chip
 * select halfway through the read sequence, when the actual register
 * value is read; but the WORK_92105 hardware prevents the MAX6957AAX
 * SPI OUT from reaching the LPC32XX SIP MISO if chip is not selected.
 * so let's release the CS an hold it again while reading the result.
 */

static uint8_t max6957aax_read(uint8_t reg)
{
	uint8_t dout[2], din[2];

	/* send read command */
	dout[0] = reg | 0x80; /* set bit 7 to indicate read */
	dout[1] = 0;
	gpio_set_value(GPO_15, 0);
	/* do SPI read/write (passing din==dout is OK) */
	spi_xfer(slave, 16, dout, dout, SPI_XFER_BEGIN | SPI_XFER_END);
	/* latch read command */
	gpio_set_value(GPO_15, 1);
	/* read register -- din = noop on xmit, din[1] = reg on recv */
	din[0] = 0;
	din[1] = 0;
	gpio_set_value(GPO_15, 0);
	/* do SPI read/write (passing din==dout is OK) */
	spi_xfer(slave, 16, din, din, SPI_XFER_BEGIN | SPI_XFER_END);
	/* end of read. */
	gpio_set_value(GPO_15, 1);
	return din[1];
}

static void hd44780_instruction(unsigned long instruction)
{
	max6957aax_write(MAX6957AAX_HD44780_RS, 0);
	max6957aax_write(MAX6957AAX_HD44780_R_W, 0);
	max6957aax_write(MAX6957AAX_HD44780_EN, 1);
	max6957aax_write(MAX6957AAX_HD44780_DATA, instruction);
	max6957aax_write(MAX6957AAX_HD44780_EN, 0);
	/* HD44780 takes 37 us for most instructions, 1520 for clear */
	if (instruction == HD44780_CLEAR_DISPLAY)
		udelay(2000);
	else
		udelay(100);
}

static void hd44780_write_char(char c)
{
	max6957aax_write(MAX6957AAX_HD44780_RS, 1);
	max6957aax_write(MAX6957AAX_HD44780_R_W, 0);
	max6957aax_write(MAX6957AAX_HD44780_EN, 1);
	max6957aax_write(MAX6957AAX_HD44780_DATA, c);
	max6957aax_write(MAX6957AAX_HD44780_EN, 0);
	/* HD44780 takes 37 us to write to DDRAM or CGRAM */
	udelay(100);
}

static void hd44780_write_str(char *s)
{
	max6957aax_write(MAX6957AAX_HD44780_RS, 1);
	max6957aax_write(MAX6957AAX_HD44780_R_W, 0);
	while (*s) {
		max6957aax_write(MAX6957AAX_HD44780_EN, 1);
		max6957aax_write(MAX6957AAX_HD44780_DATA, *s);
		max6957aax_write(MAX6957AAX_HD44780_EN, 0);
		s++;
		/* HD44780 takes 37 us to write to DDRAM or CGRAM */
		udelay(100);
	}
}

/*
 * Existing user code might expect these custom characters to be
 * recognized and displayed on the LCD
 */

static u8 char_gen_chars[] = {
	/* #8, empty rectangle */
	0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F,
	/* #9, filled right arrow */
	0x10, 0x18, 0x1C, 0x1E, 0x1C, 0x18, 0x10, 0x00,
	/* #10, filled left arrow */
	0x01, 0x03, 0x07, 0x0F, 0x07, 0x03, 0x01, 0x00,
	/* #11, up and down arrow */
	0x04, 0x0E, 0x1F, 0x00, 0x00, 0x1F, 0x0E, 0x04,
	/* #12, plus/minus */
	0x04, 0x04, 0x1F, 0x04, 0x04, 0x00, 0x1F, 0x00,
	/* #13, fat exclamation mark */
	0x06, 0x06, 0x06, 0x06, 0x00, 0x06, 0x06, 0x00,
	/* #14, empty square */
	0x00, 0x1F, 0x11, 0x11, 0x11, 0x1F, 0x00, 0x00,
	/* #15, struck out square */
	0x00, 0x1F, 0x19, 0x15, 0x13, 0x1F, 0x00, 0x00,
};

static void hd44780_init_char_gen(void)
{
	int i;

	hd44780_instruction(HD44780_SET_CGRAM_ADDR);

	for (i = 0; i < sizeof(char_gen_chars); i++)
		hd44780_write_char(char_gen_chars[i]);

	hd44780_instruction(HD44780_SET_DDRAM_ADDR);
}

void work_92105_display_init(void)
{
	int claim_err;
	char *display_contrast_str;
	uint8_t display_contrast = CONTRAST_DEFAULT;
	uint8_t enable_backlight = 0x96;

	slave = spi_setup_slave(0, 0, 500000, 0);

	if (!slave) {
		printf("Failed to set up SPI slave\n");
		return;
	}

	claim_err = spi_claim_bus(slave);

	if (claim_err)
		debug("Failed to claim SPI bus: %d\n", claim_err);

	/* enable backlight */
	i2c_write(0x2c, 0x01, 1, &enable_backlight, 1);

	/* set display contrast */
	display_contrast_str = env_get("fwopt_dispcontrast");
	if (display_contrast_str)
		display_contrast = simple_strtoul(display_contrast_str,
			NULL, 10);
	i2c_write(0x2c, 0x00, 1, &display_contrast, 1);

	/* request GPO_15 as an output initially set to 1 */
	gpio_request(GPO_15, "MAX6957_nCS");
	gpio_direction_output(GPO_15, 1);

	/* enable MAX6957 portexpander */
	max6957aax_write(MAX6957_CONF, 0x01);
	/* configure pin 8 as input, pins 9..19 as outputs */
	max6957aax_write(MAX6957_CONF_08_11, 0x56);
	max6957aax_write(MAX6957_CONF_12_15, 0x55);
	max6957aax_write(MAX6957_CONF_16_19, 0x55);

	/* initialize HD44780 */
	max6957aax_write(MAX6957AAX_HD44780_EN, 0);
	hd44780_instruction(HD44780_FUNCTION_SET);
	hd44780_instruction(HD44780_DISPLAY_ON_OFF_CONTROL);
	hd44780_instruction(HD44780_ENTRY_MODE_SET);

	/* write custom character glyphs */
	hd44780_init_char_gen();

	/* Show U-Boot version, date and time as a sign-of-life */
	hd44780_instruction(HD44780_CLEAR_DISPLAY);
	hd44780_instruction(HD44780_SET_DDRAM_ADDR | 0);
	hd44780_write_str(U_BOOT_VERSION);
	hd44780_instruction(HD44780_SET_DDRAM_ADDR | 64);
	hd44780_write_str(U_BOOT_DATE);
	hd44780_instruction(HD44780_SET_DDRAM_ADDR | 64 | 20);
	hd44780_write_str(U_BOOT_TIME);
}

#ifdef CONFIG_CMD_MAX6957

static int do_max6957aax(cmd_tbl_t *cmdtp, int flag, int argc,
			 char *const argv[])
{
	int reg, val;

	if (argc != 3)
		return CMD_RET_USAGE;
	switch (argv[1][0]) {
	case 'r':
	case 'R':
		reg = simple_strtoul(argv[2], NULL, 0);
		val = max6957aax_read(reg);
		printf("MAX6957 reg 0x%02x read 0x%02x\n", reg, val);
		return 0;
	default:
		reg = simple_strtoul(argv[1], NULL, 0);
		val = simple_strtoul(argv[2], NULL, 0);
		max6957aax_write(reg, val);
		printf("MAX6957 reg 0x%02x wrote 0x%02x\n", reg, val);
		return 0;
	}
	return 1;
}

#ifdef CONFIG_SYS_LONGHELP
static char max6957aax_help_text[] =
	"max6957aax - write or read display register:\n"
		"\tmax6957aax R|r reg - read display register;\n"
		"\tmax6957aax reg val - write display register.";
#endif

U_BOOT_CMD(
	max6957aax, 6, 1, do_max6957aax,
	"SPI MAX6957 display write/read",
	max6957aax_help_text
);
#endif /* CONFIG_CMD_MAX6957 */

#ifdef CONFIG_CMD_HD44760

/*
 * We need the HUSH parser because we need string arguments, and
 * only HUSH can understand them.
 */

#if !defined(CONFIG_HUSH_PARSER)
#error CONFIG_CMD_HD44760 requires CONFIG_HUSH_PARSER
#endif

static int do_hd44780(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	char *cmd;

	if (argc != 3)
		return CMD_RET_USAGE;

	cmd = argv[1];

	if (strcasecmp(cmd, "cmd") == 0)
		hd44780_instruction(simple_strtol(argv[2], NULL, 0));
	else if (strcasecmp(cmd, "data") == 0)
		hd44780_write_char(simple_strtol(argv[2], NULL, 0));
	else if (strcasecmp(cmd, "str") == 0)
		hd44780_write_str(argv[2]);
	return 0;
}

#ifdef CONFIG_SYS_LONGHELP
static char hd44780_help_text[] =
	"hd44780 - control LCD driver:\n"
		"\thd44780 cmd <val> - send command <val> to driver;\n"
		"\thd44780 data <val> - send data <val> to driver;\n"
		"\thd44780 str \"<text>\" - send \"<text>\" to driver.";
#endif

U_BOOT_CMD(
	hd44780, 6, 1, do_hd44780,
	"HD44780 LCD driver control",
	hd44780_help_text
);
#endif /* CONFIG_CMD_HD44760 */
