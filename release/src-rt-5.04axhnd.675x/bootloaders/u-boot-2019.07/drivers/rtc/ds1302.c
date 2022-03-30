/*
 * ds1302.c - Support for the Dallas Semiconductor DS1302 Timekeeping Chip
 *
 * Rex G. Feany <rfeany@zumanetworks.com>
 *
 */

#include <common.h>
#include <command.h>
#include <rtc.h>

/* GPP Pins */
#define DATA		0x200
#define SCLK		0x400
#define RST		0x800

/* Happy Fun Defines(tm) */
#define RESET		rtc_go_low(RST), rtc_go_low(SCLK)
#define N_RESET		rtc_go_high(RST), rtc_go_low(SCLK)

#define CLOCK_HIGH	rtc_go_high(SCLK)
#define CLOCK_LOW	rtc_go_low(SCLK)

#define DATA_HIGH	rtc_go_high(DATA)
#define DATA_LOW	rtc_go_low(DATA)
#define DATA_READ	(GTREGREAD(GPP_VALUE) & DATA)

#undef RTC_DEBUG

#ifdef RTC_DEBUG
#  define DPRINTF(x,args...)	printf("ds1302: " x , ##args)
static inline void DUMP(const char *ptr, int num)
{
	while (num--) printf("%x ", *ptr++);
	printf("]\n");
}
#else
#  define DPRINTF(x,args...)
#  define DUMP(ptr, num)
#endif

/* time data format for DS1302 */
struct ds1302_st
{
	unsigned char CH:1;		/* clock halt 1=stop 0=start */
	unsigned char sec10:3;
	unsigned char sec:4;

	unsigned char zero0:1;
	unsigned char min10:3;
	unsigned char min:4;

	unsigned char fmt:1;		/* 1=12 hour 0=24 hour */
	unsigned char zero1:1;
	unsigned char hr10:2;	/* 10 (0-2) or am/pm (am/pm, 0-1) */
	unsigned char hr:4;

	unsigned char zero2:2;
	unsigned char date10:2;
	unsigned char date:4;

	unsigned char zero3:3;
	unsigned char month10:1;
	unsigned char month:4;

	unsigned char zero4:5;
	unsigned char day:3;		/* day of week */

	unsigned char year10:4;
	unsigned char year:4;

	unsigned char WP:1;		/* write protect 1=protect 0=unprot */
	unsigned char zero5:7;
};

static int ds1302_initted=0;

/* Pin control */
static inline void
rtc_go_high(unsigned int mask)
{
	unsigned int f = GTREGREAD(GPP_VALUE) | mask;

	GT_REG_WRITE(GPP_VALUE, f);
}

static inline void
rtc_go_low(unsigned int mask)
{
	unsigned int f = GTREGREAD(GPP_VALUE) & ~mask;

	GT_REG_WRITE(GPP_VALUE, f);
}

static inline void
rtc_go_input(unsigned int mask)
{
	unsigned int f = GTREGREAD(GPP_IO_CONTROL) & ~mask;

	GT_REG_WRITE(GPP_IO_CONTROL, f);
}

static inline void
rtc_go_output(unsigned int mask)
{
	unsigned int f = GTREGREAD(GPP_IO_CONTROL) | mask;

	GT_REG_WRITE(GPP_IO_CONTROL, f);
}

/* Access data in RTC */

static void
write_byte(unsigned char b)
{
	int i;
	unsigned char mask=1;

	for(i=0;i<8;i++) {
		CLOCK_LOW;			/* Lower clock */
		(b&mask)?DATA_HIGH:DATA_LOW;	/* set data */
		udelay(1);
		CLOCK_HIGH;		/* latch data with rising clock */
		udelay(1);
		mask=mask<<1;
	}
}

static unsigned char
read_byte(void)
{
	int i;
	unsigned char mask=1;
	unsigned char b=0;

	for(i=0;i<8;i++) {
		CLOCK_LOW;
		udelay(1);
		if (DATA_READ) b|=mask;	/* if this bit is high, set in b */
		CLOCK_HIGH;		/* clock out next bit */
		udelay(1);
		mask=mask<<1;
	}
	return b;
}

static void
read_ser_drv(unsigned char addr, unsigned char *buf, int count)
{
	int i;
#ifdef RTC_DEBUG
	char *foo = buf;
#endif

	DPRINTF("READ 0x%x bytes @ 0x%x [ ", count, addr);

	addr|=1;	/* READ */
	N_RESET;
	udelay(4);
	write_byte(addr);
	rtc_go_input(DATA); /* Put gpp pin into input mode */
	udelay(1);
	for(i=0;i<count;i++) *(buf++)=read_byte();
	RESET;
	rtc_go_output(DATA);/* Reset gpp for output */
	udelay(4);

	DUMP(foo, count);
}

static void
write_ser_drv(unsigned char addr, unsigned char *buf, int count)
{
	int i;

	DPRINTF("WRITE 0x%x bytes @ 0x%x [ ", count, addr);
	DUMP(buf, count);

	addr&=~1;	/* WRITE */
	N_RESET;
	udelay(4);
	write_byte(addr);
	for(i=0;i<count;i++) write_byte(*(buf++));
	RESET;
	udelay(4);

}

void
rtc_init(void)
{
	struct ds1302_st bbclk;
	unsigned char b;
	int mod;

	DPRINTF("init\n");

	rtc_go_output(DATA|SCLK|RST);

	/* disable write protect */
	b = 0;
	write_ser_drv(0x8e,&b,1);

	/* enable trickle */
	b = 0xa5;	/* 1010.0101 */
	write_ser_drv(0x90,&b,1);

	/* read burst */
	read_ser_drv(0xbe, (unsigned char *)&bbclk, 8);

	/* Sanity checks */
	mod = 0;
	if (bbclk.CH) {
		printf("ds1302: Clock was halted, starting clock\n");
		bbclk.CH=0;
		mod=1;
	}

	if (bbclk.fmt) {
		printf("ds1302: Clock was in 12 hour mode, fixing\n");
		bbclk.fmt=0;
		mod=1;
	}

	if (bbclk.year>9) {
		printf("ds1302: Year was corrupted, fixing\n");
		bbclk.year10=100/10;	/* 2000 - why not? ;) */
		bbclk.year=0;
		mod=1;
	}

	/* Write out the changes if needed */
	if (mod) {
		/* enable write protect */
		bbclk.WP = 1;
		write_ser_drv(0xbe,(unsigned char *)&bbclk,8);
	} else {
		/* Else just turn write protect on */
		b = 0x80;
		write_ser_drv(0x8e,&b,1);
	}
	DPRINTF("init done\n");

	ds1302_initted=1;
}

void
rtc_reset(void)
{
	if(!ds1302_initted) rtc_init();
	/* TODO */
}

int
rtc_get(struct rtc_time *tmp)
{
	int rel = 0;
	struct ds1302_st bbclk;

	if(!ds1302_initted) rtc_init();

	read_ser_drv(0xbe,(unsigned char *)&bbclk, 8);      /* read burst */

	if (bbclk.CH) {
		printf("ds1302: rtc_get: Clock was halted, clock probably "
			"corrupt\n");
		rel = -1;
	}

	tmp->tm_sec=10*bbclk.sec10+bbclk.sec;
	tmp->tm_min=10*bbclk.min10+bbclk.min;
	tmp->tm_hour=10*bbclk.hr10+bbclk.hr;
	tmp->tm_wday=bbclk.day;
	tmp->tm_mday=10*bbclk.date10+bbclk.date;
	tmp->tm_mon=10*bbclk.month10+bbclk.month;
	tmp->tm_year=10*bbclk.year10+bbclk.year + 1900;

	tmp->tm_yday = 0;
	tmp->tm_isdst= 0;

	DPRINTF("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec );

	return rel;
}

int rtc_set(struct rtc_time *tmp)
{
	struct ds1302_st bbclk;
	unsigned char b=0;

	if(!ds1302_initted) rtc_init();

	DPRINTF("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	memset(&bbclk,0,sizeof(bbclk));
	bbclk.CH=0; /* dont halt */
	bbclk.WP=1; /* write protect when we're done */

	bbclk.sec10=tmp->tm_sec/10;
	bbclk.sec=tmp->tm_sec%10;

	bbclk.min10=tmp->tm_min/10;
	bbclk.min=tmp->tm_min%10;

	bbclk.hr10=tmp->tm_hour/10;
	bbclk.hr=tmp->tm_hour%10;

	bbclk.day=tmp->tm_wday;

	bbclk.date10=tmp->tm_mday/10;
	bbclk.date=tmp->tm_mday%10;

	bbclk.month10=tmp->tm_mon/10;
	bbclk.month=tmp->tm_mon%10;

	tmp->tm_year -= 1900;
	bbclk.year10=tmp->tm_year/10;
	bbclk.year=tmp->tm_year%10;

	write_ser_drv(0x8e,&b,1);           /* disable write protect */
	write_ser_drv(0xbe,(unsigned char *)&bbclk, 8);     /* write burst */

	return 0;
}
