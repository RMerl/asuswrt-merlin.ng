// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2005-2009
 * Jens Scharsig @ BuS Elektronik GmbH & Co. KG, <esw@bus-elektronik.de>
 */

#include <common.h>
#include <bmp_layout.h>
#include <asm/io.h>

vu_char  *vcxk_bws      = ((vu_char *) (CONFIG_SYS_VCXK_BASE));
vu_short *vcxk_bws_word = ((vu_short *)(CONFIG_SYS_VCXK_BASE));
vu_long  *vcxk_bws_long = ((vu_long *) (CONFIG_SYS_VCXK_BASE));

#ifdef CONFIG_AT91RM9200
	#include <asm/arch/hardware.h>
	#include <asm/arch/at91_pio.h>

	#ifndef VCBITMASK
		#define VCBITMASK(bitno)	(0x0001 << (bitno % 16))
	#endif
at91_pio_t *pio = (at91_pio_t *) AT91_PIO_BASE;
#define VCXK_INIT_PIN(PORT, PIN, DDR, I0O1) \
	do { \
		writel(PIN, &pio->PORT.per); \
		writel(PIN, &pio->PORT.DDR); \
		writel(PIN, &pio->PORT.mddr); \
		if (!I0O1) \
			writel(PIN, &pio->PORT.puer); \
	} while (0);

#define VCXK_SET_PIN(PORT, PIN)	writel(PIN, &pio->PORT.sodr);
#define VCXK_CLR_PIN(PORT, PIN)	writel(PIN, &pio->PORT.codr);

#define VCXK_ACKNOWLEDGE	\
	(!(readl(&pio->CONFIG_SYS_VCXK_ACKNOWLEDGE_PORT.pdsr) & \
			CONFIG_SYS_VCXK_ACKNOWLEDGE_PIN))
#elif defined(CONFIG_MCF52x2)
	#include <asm/m5282.h>
	#ifndef VCBITMASK
		#define VCBITMASK(bitno) (0x8000 >> (bitno % 16))
	#endif

	#define VCXK_INIT_PIN(PORT, PIN, DDR, I0O1) \
		if (I0O1) DDR |= PIN; else DDR &= ~PIN;

	#define VCXK_SET_PIN(PORT, PIN)	PORT |= PIN;
	#define VCXK_CLR_PIN(PORT, PIN)	PORT &= ~PIN;

	#define VCXK_ACKNOWLEDGE \
		(!(CONFIG_SYS_VCXK_ACKNOWLEDGE_PORT &	\
			CONFIG_SYS_VCXK_ACKNOWLEDGE_PIN))

#else
	#error no vcxk support for selected ARCH
#endif

#define VCXK_DISABLE\
	VCXK_SET_PIN(CONFIG_SYS_VCXK_ENABLE_PORT, CONFIG_SYS_VCXK_ENABLE_PIN)
#define VCXK_ENABLE\
	VCXK_CLR_PIN(CONFIG_SYS_VCXK_ENABLE_PORT, CONFIG_SYS_VCXK_ENABLE_PIN)

#ifndef CONFIG_SYS_VCXK_DOUBLEBUFFERED
	#define VCXK_BWS(x, data)		vcxk_bws[x] = data;
	#define VCXK_BWS_WORD_SET(x, mask) 	vcxk_bws_word[x] |= mask;
	#define VCXK_BWS_WORD_CLEAR(x, mask) 	vcxk_bws_word[x] &= ~mask;
	#define VCXK_BWS_LONG(x, data)		vcxk_bws_long[x] = data;
#else
	u_char double_bws[16384];
	u_short *double_bws_word;
	u_long  *double_bws_long;
	#define VCXK_BWS(x,data)	\
		double_bws[x] = data; vcxk_bws[x] = data;
	#define VCXK_BWS_WORD_SET(x,mask)	\
		double_bws_word[x] |= mask;	\
		vcxk_bws_word[x] = double_bws_word[x];
	#define VCXK_BWS_WORD_CLEAR(x,mask)	\
		double_bws_word[x] &= ~mask;	\
		vcxk_bws_word[x] = double_bws_word[x];
	#define VCXK_BWS_LONG(x,data) \
		double_bws_long[x] = data; vcxk_bws_long[x] = data;
#endif

#define VC4K16_Bright1	vcxk_bws_word[0x20004 / 2]
#define VC4K16_Bright2 	vcxk_bws_word[0x20006 / 2]
#define VC2K_Bright	vcxk_bws[0x8000]
#define VC8K_BrightH	vcxk_bws[0xC000]
#define VC8K_BrightL	vcxk_bws[0xC001]

vu_char VC4K16;

u_long display_width;
u_long display_height;
u_long display_bwidth;

ulong search_vcxk_driver(void);
void vcxk_cls(void);
void vcxk_setbrightness(unsigned int side, short brightness);
int vcxk_request(void);
int vcxk_acknowledge_wait(void);
void vcxk_clear(void);

/*
 ****f* bus_vcxk/vcxk_init
 * FUNCTION
 * initialalize Video Controller
 * PARAMETERS
 * width	visible display width in pixel
 * height	visible display height  in pixel
 ***
 */

int vcxk_init(unsigned long width, unsigned long height)
{
#ifdef CONFIG_SYS_VCXK_RESET_PORT
	VCXK_INIT_PIN(CONFIG_SYS_VCXK_RESET_PORT,
		CONFIG_SYS_VCXK_RESET_PIN, CONFIG_SYS_VCXK_RESET_DDR, 1)
	VCXK_SET_PIN(CONFIG_SYS_VCXK_RESET_PORT, CONFIG_SYS_VCXK_RESET_PIN);
#endif

#ifdef CONFIG_SYS_VCXK_DOUBLEBUFFERED
	double_bws_word  = (u_short *)double_bws;
	double_bws_long  = (u_long *)double_bws;
	debug("%px %px %px\n", double_bws, double_bws_word, double_bws_long);
#endif
	display_width  = width;
	display_height = height;
#if (CONFIG_SYS_VCXK_DEFAULT_LINEALIGN == 4)
	display_bwidth = ((width + 31) / 8) & ~0x3;
#elif (CONFIG_SYS_VCXK_DEFAULT_LINEALIGN == 2)
	display_bwidth = ((width + 15) / 8) & ~0x1;
#else
	#error CONFIG_SYS_VCXK_DEFAULT_LINEALIGN is invalid
#endif
	debug("linesize ((%ld + 15) / 8 & ~0x1) = %ld\n",
		display_width, display_bwidth);

#ifdef CONFIG_SYS_VCXK_AUTODETECT
	VC4K16 = 0;
	vcxk_bws_long[1] = 0x0;
	vcxk_bws_long[1] = 0x55AAAA55;
	vcxk_bws_long[5] = 0x0;
	if (vcxk_bws_long[1] == 0x55AAAA55)
		VC4K16 = 1;
#else
	VC4K16 = 1;
	debug("No autodetect: use vc4k\n");
#endif

	VCXK_INIT_PIN(CONFIG_SYS_VCXK_INVERT_PORT,
		CONFIG_SYS_VCXK_INVERT_PIN, CONFIG_SYS_VCXK_INVERT_DDR, 1)
	VCXK_SET_PIN(CONFIG_SYS_VCXK_INVERT_PORT, CONFIG_SYS_VCXK_INVERT_PIN)

	VCXK_SET_PIN(CONFIG_SYS_VCXK_REQUEST_PORT, CONFIG_SYS_VCXK_REQUEST_PIN);
	VCXK_INIT_PIN(CONFIG_SYS_VCXK_REQUEST_PORT,
		CONFIG_SYS_VCXK_REQUEST_PIN, CONFIG_SYS_VCXK_REQUEST_DDR, 1)

	VCXK_INIT_PIN(CONFIG_SYS_VCXK_ACKNOWLEDGE_PORT,
		CONFIG_SYS_VCXK_ACKNOWLEDGE_PIN,
		CONFIG_SYS_VCXK_ACKNOWLEDGE_DDR, 0)

	VCXK_DISABLE;
	VCXK_INIT_PIN(CONFIG_SYS_VCXK_ENABLE_PORT,
		CONFIG_SYS_VCXK_ENABLE_PIN, CONFIG_SYS_VCXK_ENABLE_DDR, 1)

	vcxk_cls();
	vcxk_cls();	/* clear second/hidden page */

	vcxk_setbrightness(3, 1000);
	VCXK_ENABLE;
	return 1;
}

/*
 ****f* bus_vcxk/vcxk_setpixel
 * FUNCTION
 * set the pixel[x,y] with the given color
 * PARAMETER
 * x		pixel colum
 * y		pixel row
 * color	<0x40 off/black
 *			>0x40 on
 ***
 */

void vcxk_setpixel(int x, int y, unsigned long color)
{
	vu_short dataptr;

	if ((x < display_width) && (y < display_height)) {
		dataptr = ((x / 16)) + (y * (display_bwidth >> 1));

		color = ((color >> 16) & 0xFF) |
			    ((color >> 8) & 0xFF) | (color & 0xFF);

		if (color > 0x40) {
			VCXK_BWS_WORD_SET(dataptr, VCBITMASK(x));
		} else {
			VCXK_BWS_WORD_CLEAR(dataptr, VCBITMASK(x));
		}
	}
}

/*
 ****f* bus_vcxk/vcxk_loadimage
 * FUNCTION
 * copies a binary image to display memory
 ***
 */

void vcxk_loadimage(ulong source)
{
	int cnt;
	vcxk_acknowledge_wait();
	if (VC4K16) {
		for (cnt = 0; cnt < (16384 / 4); cnt++) {
			VCXK_BWS_LONG(cnt, (*(ulong *) source));
			source = source + 4;
		}
	} else {
		for (cnt = 0; cnt < 16384; cnt++) {
			VCXK_BWS_LONG(cnt*2, (*(vu_char *) source));
			source++;
		}
	}
	vcxk_request();
}

/*
 ****f* bus_vcxk/vcxk_cls
 * FUNCTION
 * clear the display
 ***
 */

void vcxk_cls(void)
{
	vcxk_acknowledge_wait();
	vcxk_clear();
	vcxk_request();
}

/*
 ****f* bus_vcxk/vcxk_clear(void)
 * FUNCTION
 * clear the display memory
 ***
 */

void vcxk_clear(void)
{
	int cnt;

	for (cnt = 0; cnt < (16384 / 4); cnt++) {
		VCXK_BWS_LONG(cnt, 0)
	}
}

/*
 ****f* bus_vcxk/vcxk_setbrightness
 * FUNCTION
 * set the display brightness
 * PARAMETER
 * side	1	set front side brightness
 * 		2	set back  side brightness
 *		3	set brightness for both sides
 * brightness 0..1000
 ***
 */

void vcxk_setbrightness(unsigned int side, short brightness)
{
	if (VC4K16) {
		if ((side == 0) || (side & 0x1))
			VC4K16_Bright1 = brightness + 23;
		if ((side == 0) || (side & 0x2))
			VC4K16_Bright2 = brightness + 23;
	} else 	{
		VC2K_Bright = (brightness >> 4) + 2;
		VC8K_BrightH = (brightness + 23) >> 8;
		VC8K_BrightL = (brightness + 23) & 0xFF;
	}
}

/*
 ****f* bus_vcxk/vcxk_request
 * FUNCTION
 * requests viewing of display memory
 ***
 */

int vcxk_request(void)
{
	VCXK_CLR_PIN(CONFIG_SYS_VCXK_REQUEST_PORT,
		CONFIG_SYS_VCXK_REQUEST_PIN)
	VCXK_SET_PIN(CONFIG_SYS_VCXK_REQUEST_PORT,
		CONFIG_SYS_VCXK_REQUEST_PIN);
	return 1;
}

/*
 ****f* bus_vcxk/vcxk_acknowledge_wait
 * FUNCTION
 * wait for acknowledge viewing requests
 ***
 */

int vcxk_acknowledge_wait(void)
{
	while (VCXK_ACKNOWLEDGE)
		;
	return 1;
}

/*
 ****f* bus_vcxk/vcxk_draw_mono
 * FUNCTION
 * copies a monochrom bitmap (BMP-Format) from given memory
 * PARAMETER
 * dataptr	pointer to bitmap
 * x		output bitmap @ columne
 * y		output bitmap @ row
 ***
 */

void vcxk_draw_mono(unsigned char *dataptr, unsigned long linewidth,
	unsigned long  cp_width, unsigned long cp_height)
{
	unsigned char *lineptr;
	unsigned long xcnt, ycnt;

	for (ycnt = cp_height; ycnt > 0; ycnt--) {
		lineptr	= dataptr;
		for (xcnt = 0; xcnt < cp_width; xcnt++) {
			if ((*lineptr << (xcnt % 8)) & 0x80)
				vcxk_setpixel(xcnt, ycnt - 1, 0xFFFFFF);
			else
				vcxk_setpixel(xcnt, ycnt-1, 0);

			if ((xcnt % 8) == 7)
				lineptr++;
		} /* endfor xcnt */
		dataptr = dataptr + linewidth;
	} /* endfor ycnt */
}

/*
 ****f* bus_vcxk/vcxk_display_bitmap
 * FUNCTION
 * copies a bitmap (BMP-Format) to the given position
 * PARAMETER
 * addr		pointer to bitmap
 * x		output bitmap @ columne
 * y		output bitmap @ row
 ***
 */

int vcxk_display_bitmap(ulong addr, int x, int y)
{
	struct bmp_image *bmp;
	unsigned long width;
	unsigned long height;
	unsigned long bpp;

	unsigned long lw;

	unsigned long c_width;
	unsigned long c_height;
	unsigned char *dataptr;

	bmp = (struct bmp_image *)addr;
	if ((bmp->header.signature[0] == 'B') &&
	    (bmp->header.signature[1] == 'M')) {
		width        = le32_to_cpu(bmp->header.width);
		height       = le32_to_cpu(bmp->header.height);
		bpp          = le16_to_cpu(bmp->header.bit_count);

		dataptr = (unsigned char *) bmp +
				le32_to_cpu(bmp->header.data_offset);

		if (display_width < (width + x))
			c_width = display_width - x;
		else
			c_width = width;
		if (display_height < (height + y))
			c_height = display_height - y;
		else
			c_height = height;

		lw = (((width + 7) / 8) + 3) & ~0x3;

		if (c_height < height)
			dataptr = dataptr + lw * (height - c_height);
		switch (bpp) {
		case 1:
			vcxk_draw_mono(dataptr, lw, c_width, c_height);
			break;
		default:
			printf("Error: %ld bit per pixel "
				"not supported by VCxK\n", bpp);
			return 0;
		}
	} else	{
		printf("Error: no valid bmp at %lx\n", (ulong) bmp);
		return 0;
	}
	return 1;
}

/*
 ****f* bus_vcxk/video_display_bitmap
 ***
 */

int video_display_bitmap(ulong addr, int x, int y)
{
	vcxk_acknowledge_wait();
	if (vcxk_display_bitmap(addr, x, y)) {
		vcxk_request();
		return 0;
	}
	return 1;
}

/* EOF */
