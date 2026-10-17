// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 */

#ifdef CONFIG_GDSYS_LEGACY_DRIVERS

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <malloc.h>
#include <linux/stringify.h>

#include "ch7301.h"
#include "dp501.h"
#include <gdsys_fpga.h>

#define ICS8N3QV01_I2C_ADDR 0x6E
#define ICS8N3QV01_FREF 114285000
#define ICS8N3QV01_FREF_LL 114285000LL
#define ICS8N3QV01_F_DEFAULT_0 156250000LL
#define ICS8N3QV01_F_DEFAULT_1 125000000LL
#define ICS8N3QV01_F_DEFAULT_2 100000000LL
#define ICS8N3QV01_F_DEFAULT_3  25175000LL

#define SIL1178_MASTER_I2C_ADDRESS 0x38
#define SIL1178_SLAVE_I2C_ADDRESS 0x39

#define PIXCLK_640_480_60 25180000
#define MAX_X_CHARS 53
#define MAX_Y_CHARS 26

#ifdef CONFIG_SYS_OSD_DH
#define MAX_OSD_SCREEN 8
#define OSD_DH_BASE 4
#else
#define MAX_OSD_SCREEN 4
#endif

#ifdef CONFIG_SYS_OSD_DH
#define OSD_SET_REG(screen, fld, val) \
	do { \
		if (screen >= OSD_DH_BASE) \
			FPGA_SET_REG(screen - OSD_DH_BASE, osd1.fld, val); \
		else \
			FPGA_SET_REG(screen, osd0.fld, val); \
	} while (0)
#else
#define OSD_SET_REG(screen, fld, val) \
		FPGA_SET_REG(screen, osd0.fld, val)
#endif

#ifdef CONFIG_SYS_OSD_DH
#define OSD_GET_REG(screen, fld, val) \
	do {					\
		if (screen >= OSD_DH_BASE) \
			FPGA_GET_REG(screen - OSD_DH_BASE, osd1.fld, val); \
		else \
			FPGA_GET_REG(screen, osd0.fld, val); \
	} while (0)
#else
#define OSD_GET_REG(screen, fld, val) \
		FPGA_GET_REG(screen, osd0.fld, val)
#endif

unsigned int base_width;
unsigned int base_height;
size_t bufsize;
u16 *buf;

unsigned int osd_screen_mask = 0;

#ifdef CONFIG_SYS_ICS8N3QV01_I2C
int ics8n3qv01_i2c[] = CONFIG_SYS_ICS8N3QV01_I2C;
#endif

#ifdef CONFIG_SYS_SIL1178_I2C
int sil1178_i2c[] = CONFIG_SYS_SIL1178_I2C;
#endif

#ifdef CONFIG_SYS_MPC92469AC
static void mpc92469ac_calc_parameters(unsigned int fout,
	unsigned int *post_div, unsigned int *feedback_div)
{
	unsigned int n = *post_div;
	unsigned int m = *feedback_div;
	unsigned int a;
	unsigned int b = 14745600 / 16;

	if (fout < 50169600)
		n = 8;
	else if (fout < 100339199)
		n = 4;
	else if (fout < 200678399)
		n = 2;
	else
		n = 1;

	a = fout * n + (b / 2); /* add b/2 for proper rounding */

	m = a / b;

	*post_div = n;
	*feedback_div = m;
}

static void mpc92469ac_set(unsigned screen, unsigned int fout)
{
	unsigned int n;
	unsigned int m;
	unsigned int bitval = 0;
	mpc92469ac_calc_parameters(fout, &n, &m);

	switch (n) {
	case 1:
		bitval = 0x00;
		break;
	case 2:
		bitval = 0x01;
		break;
	case 4:
		bitval = 0x02;
		break;
	case 8:
		bitval = 0x03;
		break;
	}

	FPGA_SET_REG(screen, mpc3w_control, (bitval << 9) | m);
}
#endif

#ifdef CONFIG_SYS_ICS8N3QV01_I2C

static unsigned int ics8n3qv01_get_fout_calc(unsigned index)
{
	unsigned long long n;
	unsigned long long mint;
	unsigned long long mfrac;
	u8 reg_a, reg_b, reg_c, reg_d, reg_f;
	unsigned long long fout_calc;

	if (index > 3)
		return 0;

	reg_a = i2c_reg_read(ICS8N3QV01_I2C_ADDR, 0 + index);
	reg_b = i2c_reg_read(ICS8N3QV01_I2C_ADDR, 4 + index);
	reg_c = i2c_reg_read(ICS8N3QV01_I2C_ADDR, 8 + index);
	reg_d = i2c_reg_read(ICS8N3QV01_I2C_ADDR, 12 + index);
	reg_f = i2c_reg_read(ICS8N3QV01_I2C_ADDR, 20 + index);

	mint = ((reg_a >> 1) & 0x1f) | (reg_f & 0x20);
	mfrac = ((reg_a & 0x01) << 17) | (reg_b << 9) | (reg_c << 1)
		| (reg_d >> 7);
	n = reg_d & 0x7f;

	fout_calc = (mint * ICS8N3QV01_FREF_LL
		     + mfrac * ICS8N3QV01_FREF_LL / 262144LL
		     + ICS8N3QV01_FREF_LL / 524288LL
		     + n / 2)
		    / n
		    * 1000000
		    / (1000000 - 100);

	return fout_calc;
}


static void ics8n3qv01_calc_parameters(unsigned int fout,
	unsigned int *_mint, unsigned int *_mfrac,
	unsigned int *_n)
{
	unsigned int n;
	unsigned int foutiic;
	unsigned int fvcoiic;
	unsigned int mint;
	unsigned long long mfrac;

	n = (2215000000U + fout / 2) / fout;
	if ((n & 1) && (n > 5))
		n -= 1;

	foutiic = fout - (fout / 10000);
	fvcoiic = foutiic * n;

	mint = fvcoiic / 114285000;
	if ((mint < 17) || (mint > 63))
		printf("ics8n3qv01_calc_parameters: cannot determine mint\n");

	mfrac = ((unsigned long long)fvcoiic % 114285000LL) * 262144LL
		/ 114285000LL;

	*_mint = mint;
	*_mfrac = mfrac;
	*_n = n;
}

static void ics8n3qv01_set(unsigned int fout)
{
	unsigned int n;
	unsigned int mint;
	unsigned int mfrac;
	unsigned int fout_calc;
	unsigned long long fout_prog;
	long long off_ppm;
	u8 reg0, reg4, reg8, reg12, reg18, reg20;

	fout_calc = ics8n3qv01_get_fout_calc(1);
	off_ppm = (fout_calc - ICS8N3QV01_F_DEFAULT_1) * 1000000
		  / ICS8N3QV01_F_DEFAULT_1;
	printf("       PLL is off by %lld ppm\n", off_ppm);
	fout_prog = (unsigned long long)fout * (unsigned long long)fout_calc
		    / ICS8N3QV01_F_DEFAULT_1;
	ics8n3qv01_calc_parameters(fout_prog, &mint, &mfrac, &n);

	reg0 = i2c_reg_read(ICS8N3QV01_I2C_ADDR, 0) & 0xc0;
	reg0 |= (mint & 0x1f) << 1;
	reg0 |= (mfrac >> 17) & 0x01;
	i2c_reg_write(ICS8N3QV01_I2C_ADDR, 0, reg0);

	reg4 = mfrac >> 9;
	i2c_reg_write(ICS8N3QV01_I2C_ADDR, 4, reg4);

	reg8 = mfrac >> 1;
	i2c_reg_write(ICS8N3QV01_I2C_ADDR, 8, reg8);

	reg12 = mfrac << 7;
	reg12 |= n & 0x7f;
	i2c_reg_write(ICS8N3QV01_I2C_ADDR, 12, reg12);

	reg18 = i2c_reg_read(ICS8N3QV01_I2C_ADDR, 18) & 0x03;
	reg18 |= 0x20;
	i2c_reg_write(ICS8N3QV01_I2C_ADDR, 18, reg18);

	reg20 = i2c_reg_read(ICS8N3QV01_I2C_ADDR, 20) & 0x1f;
	reg20 |= mint & (1 << 5);
	i2c_reg_write(ICS8N3QV01_I2C_ADDR, 20, reg20);
}
#endif

static int osd_write_videomem(unsigned screen, unsigned offset,
	u16 *data, size_t charcount)
{
	unsigned int k;

	for (k = 0; k < charcount; ++k) {
		if (offset + k >= bufsize)
			return -1;
#ifdef CONFIG_SYS_OSD_DH
		if (screen >= OSD_DH_BASE)
			FPGA_SET_REG(screen - OSD_DH_BASE,
				     videomem1[offset + k], data[k]);
		else
			FPGA_SET_REG(screen, videomem0[offset + k], data[k]);
#else
		FPGA_SET_REG(screen, videomem0[offset + k], data[k]);
#endif
	}

	return charcount;
}

static int osd_print(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	unsigned screen;

	if (argc < 5) {
		cmd_usage(cmdtp);
		return 1;
	}

	for (screen = 0; screen < MAX_OSD_SCREEN; ++screen) {
		unsigned x;
		unsigned y;
		unsigned charcount;
		unsigned len;
		u8 color;
		unsigned int k;
		char *text;
		int res;

		if (!(osd_screen_mask & (1 << screen)))
			continue;

		x = hextoul(argv[1], NULL);
		y = hextoul(argv[2], NULL);
		color = hextoul(argv[3], NULL);
		text = argv[4];
		charcount = strlen(text);
		len = (charcount > bufsize) ? bufsize : charcount;

		for (k = 0; k < len; ++k)
			buf[k] = (text[k] << 8) | color;

		res = osd_write_videomem(screen, y * base_width + x, buf, len);
		if (res < 0)
			return res;

		OSD_SET_REG(screen, control, 0x0049);
	}

	return 0;
}

int osd_probe(unsigned screen)
{
	u16 version;
	u16 features;
	int old_bus = i2c_get_bus_num();
	bool pixclock_present = false;
	bool output_driver_present = false;

	OSD_GET_REG(0, version, &version);
	OSD_GET_REG(0, features, &features);

	base_width = ((features & 0x3f00) >> 8) + 1;
	base_height = (features & 0x001f) + 1;
	bufsize = base_width * base_height;
	buf = malloc(sizeof(u16) * bufsize);
	if (!buf)
		return -1;

#ifdef CONFIG_SYS_OSD_DH
	printf("OSD%d-%d: Digital-OSD version %01d.%02d, %d" "x%d characters\n",
	       (screen >= OSD_DH_BASE) ? (screen - OSD_DH_BASE) : screen,
	       (screen > 3) ? 1 : 0, version/100, version%100, base_width,
	       base_height);
#else
	printf("OSD%d:  Digital-OSD version %01d.%02d, %d" "x%d characters\n",
	       screen, version/100, version%100, base_width, base_height);
#endif
	/* setup pixclock */

#ifdef CONFIG_SYS_MPC92469AC
	pixclock_present = true;
	mpc92469ac_set(screen, PIXCLK_640_480_60);
#endif

#ifdef CONFIG_SYS_ICS8N3QV01_I2C
	i2c_set_bus_num(ics8n3qv01_i2c[screen]);
	if (!i2c_probe(ICS8N3QV01_I2C_ADDR)) {
		ics8n3qv01_set(PIXCLK_640_480_60);
		pixclock_present = true;
	}
#endif

	if (!pixclock_present)
		printf("       no pixelclock found\n");

	/* setup output driver */

#ifdef CONFIG_SYS_CH7301_I2C
	if (!ch7301_probe(screen, true))
		output_driver_present = true;
#endif

#ifdef CONFIG_SYS_SIL1178_I2C
	i2c_set_bus_num(sil1178_i2c[screen]);
	if (!i2c_probe(SIL1178_SLAVE_I2C_ADDRESS)) {
		if (i2c_reg_read(SIL1178_SLAVE_I2C_ADDRESS, 0x02) == 0x06) {
			/*
			 * magic initialization sequence,
			 * adapted from datasheet
			 */
			i2c_reg_write(SIL1178_SLAVE_I2C_ADDRESS, 0x08, 0x36);
			i2c_reg_write(SIL1178_MASTER_I2C_ADDRESS, 0x0f, 0x44);
			i2c_reg_write(SIL1178_MASTER_I2C_ADDRESS, 0x0f, 0x4c);
			i2c_reg_write(SIL1178_MASTER_I2C_ADDRESS, 0x0e, 0x10);
			i2c_reg_write(SIL1178_MASTER_I2C_ADDRESS, 0x0a, 0x80);
			i2c_reg_write(SIL1178_MASTER_I2C_ADDRESS, 0x09, 0x30);
			i2c_reg_write(SIL1178_MASTER_I2C_ADDRESS, 0x0c, 0x89);
			i2c_reg_write(SIL1178_MASTER_I2C_ADDRESS, 0x0d, 0x60);
			i2c_reg_write(SIL1178_MASTER_I2C_ADDRESS, 0x08, 0x36);
			i2c_reg_write(SIL1178_MASTER_I2C_ADDRESS, 0x08, 0x37);
			output_driver_present = true;
		}
	}
#endif

#ifdef CONFIG_SYS_DP501_I2C
	if (!dp501_probe(screen, true))
		output_driver_present = true;
#endif

	if (!output_driver_present)
		printf("       no output driver found\n");

	OSD_SET_REG(screen, xy_size, ((32 - 1) << 8) | (16 - 1));
	OSD_SET_REG(screen, x_pos, 0x007f);
	OSD_SET_REG(screen, y_pos, 0x005f);

	if (pixclock_present && output_driver_present)
		osd_screen_mask |= 1 << screen;

	i2c_set_bus_num(old_bus);

	return 0;
}

int osd_write(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned screen;

	if ((argc < 4) || (strlen(argv[3]) % 4)) {
		cmd_usage(cmdtp);
		return 1;
	}

	for (screen = 0; screen < MAX_OSD_SCREEN; ++screen) {
		unsigned x;
		unsigned y;
		unsigned k;
		u16 buffer[base_width];
		char *rp;
		u16 *wp = buffer;
		unsigned count = (argc > 4) ?
			hextoul(argv[4], NULL) : 1;

		if (!(osd_screen_mask & (1 << screen)))
			continue;

		x = hextoul(argv[1], NULL);
		y = hextoul(argv[2], NULL);
		rp = argv[3];


		while (*rp) {
			char substr[5];

			memcpy(substr, rp, 4);
			substr[4] = 0;
			*wp = hextoul(substr, NULL);

			rp += 4;
			wp++;
			if (wp - buffer > base_width)
				break;
		}

		for (k = 0; k < count; ++k) {
			unsigned offset =
				y * base_width + x + k * (wp - buffer);
			osd_write_videomem(screen, offset, buffer,
				wp - buffer);
		}

		OSD_SET_REG(screen, control, 0x0049);
	}

	return 0;
}

int osd_size(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned screen;
	unsigned x;
	unsigned y;

	if (argc < 3) {
		cmd_usage(cmdtp);
		return 1;
	}

	x = hextoul(argv[1], NULL);
	y = hextoul(argv[2], NULL);

	if (!x || (x > 64) || (x > MAX_X_CHARS) ||
	    !y || (y > 32) || (y > MAX_Y_CHARS)) {
		cmd_usage(cmdtp);
		return 1;
	}

	for (screen = 0; screen < MAX_OSD_SCREEN; ++screen) {
		if (!(osd_screen_mask & (1 << screen)))
			continue;

		OSD_SET_REG(screen, xy_size, ((x - 1) << 8) | (y - 1));
		OSD_SET_REG(screen, x_pos, 32767 * (640 - 12 * x) / 65535);
		OSD_SET_REG(screen, y_pos, 32767 * (480 - 18 * y) / 65535);
	}

	return 0;
}

U_BOOT_CMD(
	osdw, 5, 0, osd_write,
	"write 16-bit hex encoded buffer to osd memory",
	"pos_x pos_y buffer count\n"
);

U_BOOT_CMD(
	osdp, 5, 0, osd_print,
	"write ASCII buffer to osd memory",
	"pos_x pos_y color text\n"
);

U_BOOT_CMD(
	osdsize, 3, 0, osd_size,
	"set OSD XY size in characters",
	"size_x(max. " __stringify(MAX_X_CHARS)
	") size_y(max. " __stringify(MAX_Y_CHARS) ")\n"
);

#endif /* CONFIG_GDSYS_LEGACY_DRIVERS */
