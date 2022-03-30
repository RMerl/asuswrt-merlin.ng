// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002 SIXNET, dge@sixnetio.com.
 *
 * (C) Copyright 2004, Li-Pro.Net <www.li-pro.net>
 * Stephan Linz <linz@li-pro.net>
 */

/*
 * Date & Time support for DS1306 RTC using SPI:
 *
 *    - SXNI855T:    it uses its own soft SPI here in this file
 *    - all other:   use the external spi_xfer() function
 *                   (see include/spi.h)
 */

#include <common.h>
#include <command.h>
#include <rtc.h>
#include <spi.h>

#define	RTC_SECONDS		0x00
#define	RTC_MINUTES		0x01
#define	RTC_HOURS		0x02
#define	RTC_DAY_OF_WEEK		0x03
#define	RTC_DATE_OF_MONTH	0x04
#define	RTC_MONTH		0x05
#define	RTC_YEAR		0x06

#define	RTC_SECONDS_ALARM0	0x07
#define	RTC_MINUTES_ALARM0	0x08
#define	RTC_HOURS_ALARM0	0x09
#define	RTC_DAY_OF_WEEK_ALARM0	0x0a

#define	RTC_SECONDS_ALARM1	0x0b
#define	RTC_MINUTES_ALARM1	0x0c
#define	RTC_HOURS_ALARM1	0x0d
#define	RTC_DAY_OF_WEEK_ALARM1	0x0e

#define	RTC_CONTROL		0x0f
#define	RTC_STATUS		0x10
#define	RTC_TRICKLE_CHARGER	0x11

#define	RTC_USER_RAM_BASE	0x20

/* ************************************************************************* */
#ifdef CONFIG_SXNI855T		/* !!! SHOULD BE CHANGED TO NEW CODE !!! */

static void soft_spi_send (unsigned char n);
static unsigned char soft_spi_read (void);
static void init_spi (void);

/*-----------------------------------------------------------------------
 * Definitions
 */

#define	PB_SPISCK	0x00000002	/* PB 30 */
#define PB_SPIMOSI	0x00000004	/* PB 29 */
#define PB_SPIMISO	0x00000008	/* PB 28 */
#define PB_SPI_CE	0x00010000	/* PB 15 */

/* ------------------------------------------------------------------------- */

/* read clock time from DS1306 and return it in *tmp */
int rtc_get (struct rtc_time *tmp)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	unsigned char spi_byte;	/* Data Byte */

	init_spi ();		/* set port B for software SPI */

	/* Now we can enable the DS1306 RTC */
	immap->im_cpm.cp_pbdat |= PB_SPI_CE;
	udelay (10);

	/* Shift out the address (0) of the time in the Clock Chip */
	soft_spi_send (0);

	/* Put the clock readings into the rtc_time structure */
	tmp->tm_sec = bcd2bin (soft_spi_read ());	/* Read seconds */
	tmp->tm_min = bcd2bin (soft_spi_read ());	/* Read minutes */

	/* Hours are trickier */
	spi_byte = soft_spi_read ();	/* Read Hours into temporary value */
	if (spi_byte & 0x40) {
		/* 12 hour mode bit is set (time is in 1-12 format) */
		if (spi_byte & 0x20) {
			/* since PM we add 11 to get 0-23 for hours */
			tmp->tm_hour = (bcd2bin (spi_byte & 0x1F)) + 11;
		} else {
			/* since AM we subtract 1 to get 0-23 for hours */
			tmp->tm_hour = (bcd2bin (spi_byte & 0x1F)) - 1;
		}
	} else {
		/* Otherwise, 0-23 hour format */
		tmp->tm_hour = (bcd2bin (spi_byte & 0x3F));
	}

	soft_spi_read ();	/* Read and discard Day of week */
	tmp->tm_mday = bcd2bin (soft_spi_read ());	/* Read Day of the Month */
	tmp->tm_mon = bcd2bin (soft_spi_read ());	/* Read Month */

	/* Read Year and convert to this century */
	tmp->tm_year = bcd2bin (soft_spi_read ()) + 2000;

	/* Now we can disable the DS1306 RTC */
	immap->im_cpm.cp_pbdat &= ~PB_SPI_CE;	/* Disable DS1306 Chip */
	udelay (10);

	rtc_calc_weekday(tmp);	/* Determine the day of week */

	debug ("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	       tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	       tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	return 0;
}

/* ------------------------------------------------------------------------- */

/* set clock time in DS1306 RTC and in MPC8xx RTC */
int rtc_set (struct rtc_time *tmp)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	init_spi ();		/* set port B for software SPI */

	/* Now we can enable the DS1306 RTC */
	immap->im_cpm.cp_pbdat |= PB_SPI_CE;	/* Enable DS1306 Chip */
	udelay (10);

	/* First disable write protect in the clock chip control register */
	soft_spi_send (0x8F);	/* send address of the control register */
	soft_spi_send (0x00);	/* send control register contents */

	/* Now disable the DS1306 to terminate the write */
	immap->im_cpm.cp_pbdat &= ~PB_SPI_CE;
	udelay (10);

	/* Now enable the DS1306 to initiate a new write */
	immap->im_cpm.cp_pbdat |= PB_SPI_CE;
	udelay (10);

	/* Next, send the address of the clock time write registers */
	soft_spi_send (0x80);	/* send address of the first time register */

	/* Use Burst Mode to send all of the time data to the clock */
	bin2bcd (tmp->tm_sec);
	soft_spi_send (bin2bcd (tmp->tm_sec));	/* Send Seconds */
	soft_spi_send (bin2bcd (tmp->tm_min));	/* Send Minutes */
	soft_spi_send (bin2bcd (tmp->tm_hour));	/* Send Hour */
	soft_spi_send (bin2bcd (tmp->tm_wday));	/* Send Day of the Week */
	soft_spi_send (bin2bcd (tmp->tm_mday));	/* Send Day of Month */
	soft_spi_send (bin2bcd (tmp->tm_mon));	/* Send Month */
	soft_spi_send (bin2bcd (tmp->tm_year - 2000));	/* Send Year */

	/* Now we can disable the Clock chip to terminate the burst write */
	immap->im_cpm.cp_pbdat &= ~PB_SPI_CE;	/* Disable DS1306 Chip */
	udelay (10);

	/* Now we can enable the Clock chip to initiate a new write */
	immap->im_cpm.cp_pbdat |= PB_SPI_CE;	/* Enable DS1306 Chip */
	udelay (10);

	/* First we Enable write protect in the clock chip control register */
	soft_spi_send (0x8F);	/* send address of the control register */
	soft_spi_send (0x40);	/* send out Control Register contents */

	/* Now disable the DS1306 */
	immap->im_cpm.cp_pbdat &= ~PB_SPI_CE;	/*  Disable DS1306 Chip */
	udelay (10);

	/* Set standard MPC8xx clock to the same time so Linux will
	 * see the time even if it doesn't have a DS1306 clock driver.
	 * This helps with experimenting with standard kernels.
	 */
	{
		ulong tim;

		tim = rtc_mktime(tmp);

		immap->im_sitk.sitk_rtck = KAPWR_KEY;
		immap->im_sit.sit_rtc = tim;
	}

	debug ("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	       tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	       tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	return 0;
}

/* ------------------------------------------------------------------------- */

/* Initialize Port B for software SPI */
static void init_spi (void)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	/* Force output pins to begin at logic 0 */
	immap->im_cpm.cp_pbdat &= ~(PB_SPI_CE | PB_SPIMOSI | PB_SPISCK);

	/* Set these 3 signals as outputs */
	immap->im_cpm.cp_pbdir |= (PB_SPIMOSI | PB_SPI_CE | PB_SPISCK);

	immap->im_cpm.cp_pbdir &= ~PB_SPIMISO;	/* Make MISO pin an input */
	udelay (10);
}

/* ------------------------------------------------------------------------- */

/* NOTE: soft_spi_send() assumes that the I/O lines are configured already */
static void soft_spi_send (unsigned char n)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	unsigned char bitpos;	/* bit position to receive */
	unsigned char i;	/* Loop Control */

	/* bit position to send, start with most significant bit */
	bitpos = 0x80;

	/* Send 8 bits to software SPI */
	for (i = 0; i < 8; i++) {	/* Loop for 8 bits */
		immap->im_cpm.cp_pbdat |= PB_SPISCK;	/* Raise SCK */

		if (n & bitpos)
			immap->im_cpm.cp_pbdat |= PB_SPIMOSI;	/* Set MOSI to 1 */
		else
			immap->im_cpm.cp_pbdat &= ~PB_SPIMOSI;	/* Set MOSI to 0 */
		udelay (10);

		immap->im_cpm.cp_pbdat &= ~PB_SPISCK;	/* Lower SCK */
		udelay (10);

		bitpos >>= 1;	/* Shift for next bit position */
	}
}

/* ------------------------------------------------------------------------- */

/* NOTE: soft_spi_read() assumes that the I/O lines are configured already */
static unsigned char soft_spi_read (void)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	unsigned char spi_byte = 0;	/* Return value, assume success */
	unsigned char bitpos;	/* bit position to receive */
	unsigned char i;	/* Loop Control */

	/* bit position to receive, start with most significant bit */
	bitpos = 0x80;

	/* Read 8 bits here */
	for (i = 0; i < 8; i++) {	/* Do 8 bits in loop */
		immap->im_cpm.cp_pbdat |= PB_SPISCK;	/* Raise SCK */
		udelay (10);
		if (immap->im_cpm.cp_pbdat & PB_SPIMISO)	/* Get a bit of data */
			spi_byte |= bitpos;	/* Set data accordingly */
		immap->im_cpm.cp_pbdat &= ~PB_SPISCK;	/* Lower SCK */
		udelay (10);
		bitpos >>= 1;	/* Shift for next bit position */
	}

	return spi_byte;	/* Return the byte read */
}

/* ------------------------------------------------------------------------- */

void rtc_reset (void)
{
	return;			/* nothing to do */
}

#else  /* not CONFIG_SXNI855T */
/* ************************************************************************* */

static unsigned char rtc_read (unsigned char reg);
static void rtc_write (unsigned char reg, unsigned char val);

static struct spi_slave *slave;

/* read clock time from DS1306 and return it in *tmp */
int rtc_get (struct rtc_time *tmp)
{
	unsigned char sec, min, hour, mday, wday, mon, year;

	/*
	 * Assuming Vcc = 2.0V (lowest speed)
	 *
	 * REVISIT: If we add an rtc_init() function we can do this
	 * step just once.
	 */
	if (!slave) {
		slave = spi_setup_slave(0, CONFIG_SYS_SPI_RTC_DEVID, 600000,
				SPI_MODE_3 | SPI_CS_HIGH);
		if (!slave)
			return;
	}

	if (spi_claim_bus(slave))
		return;

	sec = rtc_read (RTC_SECONDS);
	min = rtc_read (RTC_MINUTES);
	hour = rtc_read (RTC_HOURS);
	mday = rtc_read (RTC_DATE_OF_MONTH);
	wday = rtc_read (RTC_DAY_OF_WEEK);
	mon = rtc_read (RTC_MONTH);
	year = rtc_read (RTC_YEAR);

	spi_release_bus(slave);

	debug ("Get RTC year: %02x mon: %02x mday: %02x wday: %02x "
	       "hr: %02x min: %02x sec: %02x\n",
	       year, mon, mday, wday, hour, min, sec);
	debug ("Alarms[0]: wday: %02x hour: %02x min: %02x sec: %02x\n",
	       rtc_read (RTC_DAY_OF_WEEK_ALARM0),
	       rtc_read (RTC_HOURS_ALARM0),
	       rtc_read (RTC_MINUTES_ALARM0), rtc_read (RTC_SECONDS_ALARM0));
	debug ("Alarms[1]: wday: %02x hour: %02x min: %02x sec: %02x\n",
	       rtc_read (RTC_DAY_OF_WEEK_ALARM1),
	       rtc_read (RTC_HOURS_ALARM1),
	       rtc_read (RTC_MINUTES_ALARM1), rtc_read (RTC_SECONDS_ALARM1));

	tmp->tm_sec = bcd2bin (sec & 0x7F);	/* convert Seconds */
	tmp->tm_min = bcd2bin (min & 0x7F);	/* convert Minutes */

	/* convert Hours */
	tmp->tm_hour = (hour & 0x40)
		? ((hour & 0x20)	/* 12 hour mode */
		   ? bcd2bin (hour & 0x1F) + 11	/* PM */
		   : bcd2bin (hour & 0x1F) - 1	/* AM */
		)
		: bcd2bin (hour & 0x3F);	/* 24 hour mode */

	tmp->tm_mday = bcd2bin (mday & 0x3F);	/* convert Day of the Month */
	tmp->tm_mon = bcd2bin (mon & 0x1F);	/* convert Month */
	tmp->tm_year = bcd2bin (year) + 2000;	/* convert Year */
	tmp->tm_wday = bcd2bin (wday & 0x07) - 1;	/* convert Day of the Week */
	tmp->tm_yday = 0;
	tmp->tm_isdst = 0;

	debug ("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	       tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	       tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	return 0;
}

/* ------------------------------------------------------------------------- */

/* set clock time from *tmp in DS1306 RTC */
int rtc_set (struct rtc_time *tmp)
{
	/* Assuming Vcc = 2.0V (lowest speed) */
	if (!slave) {
		slave = spi_setup_slave(0, CONFIG_SYS_SPI_RTC_DEVID, 600000,
				SPI_MODE_3 | SPI_CS_HIGH);
		if (!slave)
			return;
	}

	if (spi_claim_bus(slave))
		return;

	debug ("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	       tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	       tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	rtc_write (RTC_SECONDS, bin2bcd (tmp->tm_sec));
	rtc_write (RTC_MINUTES, bin2bcd (tmp->tm_min));
	rtc_write (RTC_HOURS, bin2bcd (tmp->tm_hour));
	rtc_write (RTC_DAY_OF_WEEK, bin2bcd (tmp->tm_wday + 1));
	rtc_write (RTC_DATE_OF_MONTH, bin2bcd (tmp->tm_mday));
	rtc_write (RTC_MONTH, bin2bcd (tmp->tm_mon));
	rtc_write (RTC_YEAR, bin2bcd (tmp->tm_year - 2000));

	spi_release_bus(slave);
}

/* ------------------------------------------------------------------------- */

/* reset the DS1306 */
void rtc_reset (void)
{
	/* Assuming Vcc = 2.0V (lowest speed) */
	if (!slave) {
		slave = spi_setup_slave(0, CONFIG_SYS_SPI_RTC_DEVID, 600000,
				SPI_MODE_3 | SPI_CS_HIGH);
		if (!slave)
			return;
	}

	if (spi_claim_bus(slave))
		return;

	/* clear the control register */
	rtc_write (RTC_CONTROL, 0x00);	/* 1st step: reset WP */
	rtc_write (RTC_CONTROL, 0x00);	/* 2nd step: reset 1Hz, AIE1, AIE0 */

	/* reset all alarms */
	rtc_write (RTC_SECONDS_ALARM0, 0x00);
	rtc_write (RTC_SECONDS_ALARM1, 0x00);
	rtc_write (RTC_MINUTES_ALARM0, 0x00);
	rtc_write (RTC_MINUTES_ALARM1, 0x00);
	rtc_write (RTC_HOURS_ALARM0, 0x00);
	rtc_write (RTC_HOURS_ALARM1, 0x00);
	rtc_write (RTC_DAY_OF_WEEK_ALARM0, 0x00);
	rtc_write (RTC_DAY_OF_WEEK_ALARM1, 0x00);

	spi_release_bus(slave);
}

/* ------------------------------------------------------------------------- */

static unsigned char rtc_read (unsigned char reg)
{
	int ret;

	ret = spi_w8r8(slave, reg);
	return ret < 0 ? 0 : ret;
}

/* ------------------------------------------------------------------------- */

static void rtc_write (unsigned char reg, unsigned char val)
{
	unsigned char dout[2];	/* SPI Output Data Bytes */
	unsigned char din[2];	/* SPI Input Data Bytes */

	dout[0] = 0x80 | reg;
	dout[1] = val;

	spi_xfer (slave, 16, dout, din, SPI_XFER_BEGIN | SPI_XFER_END);
}

#endif /* end of code exclusion (see #ifdef CONFIG_SXNI855T above) */
