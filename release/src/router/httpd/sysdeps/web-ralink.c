/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*
 * ASUS Home Gateway Reference Design
 * Web Page Configuration Support Routines
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 */

#ifdef WEBS
#include <webs.h>
#include <uemf.h>
#include <ej.h>
#else /* !WEBS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <httpd.h>
#endif /* WEBS */
#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <shutils.h>
#include <ralink.h>
#include <iwlib.h>
#include <stapriv.h>
#include <ethutils.h>
#include <shared.h>
#include <sys/mman.h>
#ifndef O_BINARY
#define O_BINARY 	0
#endif
#ifndef MAP_FAILED
#define MAP_FAILED (-1)
#endif

#define wan_prefix(unit, prefix)	snprintf(prefix, sizeof(prefix), "wan%d_", unit)
//static char * rfctime(const time_t *timep);
//static char * reltime(unsigned int seconds);
void reltime(unsigned int seconds, char *buf);
static int wl_status(int eid, webs_t wp, int argc, char_t **argv, int unit);

#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <net/if_arp.h>

#include <dirent.h>


/************************ CONSTANTS & MACROS ************************/

/*
 * Constants fof WE-9->15
 */
#define IW15_MAX_FREQUENCIES	16
#define IW15_MAX_BITRATES	8
#define IW15_MAX_TXPOWER	8
#define IW15_MAX_ENCODING_SIZES	8
#define IW15_MAX_SPY		8
#define IW15_MAX_AP		8

/****************************** TYPES ******************************/

/*
 *	Struct iw_range up to WE-15
 */
struct	iw15_range
{
	__u32		throughput;
	__u32		min_nwid;
	__u32		max_nwid;
	__u16		num_channels;
	__u8		num_frequency;
	struct iw_freq	freq[IW15_MAX_FREQUENCIES];
	__s32		sensitivity;
	struct iw_quality	max_qual;
	__u8		num_bitrates;
	__s32		bitrate[IW15_MAX_BITRATES];
	__s32		min_rts;
	__s32		max_rts;
	__s32		min_frag;
	__s32		max_frag;
	__s32		min_pmp;
	__s32		max_pmp;
	__s32		min_pmt;
	__s32		max_pmt;
	__u16		pmp_flags;
	__u16		pmt_flags;
	__u16		pm_capa;
	__u16		encoding_size[IW15_MAX_ENCODING_SIZES];
	__u8		num_encoding_sizes;
	__u8		max_encoding_tokens;
	__u16		txpower_capa;
	__u8		num_txpower;
	__s32		txpower[IW15_MAX_TXPOWER];
	__u8		we_version_compiled;
	__u8		we_version_source;
	__u16		retry_capa;
	__u16		retry_flags;
	__u16		r_time_flags;
	__s32		min_retry;
	__s32		max_retry;
	__s32		min_r_time;
	__s32		max_r_time;
	struct iw_quality	avg_qual;
};

/*
 * Union for all the versions of iwrange.
 * Fortunately, I mostly only add fields at the end, and big-bang
 * reorganisations are few.
 */
union	iw_range_raw
{
	struct iw15_range	range15;	/* WE 9->15 */
	struct iw_range		range;		/* WE 16->current */
};

/*
 * Offsets in iw_range struct
 */
#define iwr15_off(f)	( ((char *) &(((struct iw15_range *) NULL)->f)) - \
			  (char *) NULL)
#define iwr_off(f)	( ((char *) &(((struct iw_range *) NULL)->f)) - \
			  (char *) NULL)

/* Disable runtime version warning in ralink_get_range_info() */
int	iw_ignore_version_sp = 0;

/*------------------------------------------------------------------*/
/*
 * Get the range information out of the driver
 */
int
ralink_get_range_info(iwrange *	range, char* buffer, int length)
{
  union iw_range_raw *	range_raw;

  /* Point to the buffer */
  range_raw = (union iw_range_raw *) buffer;

  /* For new versions, we can check the version directly, for old versions
   * we use magic. 300 bytes is a also magic number, don't touch... */
  if (length < 300)
    {
      /* That's v10 or earlier. Ouch ! Let's make a guess...*/
      range_raw->range.we_version_compiled = 9;
    }

  /* Check how it needs to be processed */
  if (range_raw->range.we_version_compiled > 15)
    {
      /* This is our native format, that's easy... */
      /* Copy stuff at the right place, ignore extra */
      memcpy((char *) range, buffer, sizeof(iwrange));
    }
  else
    {
      /* Zero unknown fields */
      bzero((char *) range, sizeof(struct iw_range));

      /* Initial part unmoved */
      memcpy((char *) range,
	     buffer,
	     iwr15_off(num_channels));
      /* Frequencies pushed futher down towards the end */
      memcpy((char *) range + iwr_off(num_channels),
	     buffer + iwr15_off(num_channels),
	     iwr15_off(sensitivity) - iwr15_off(num_channels));
      /* This one moved up */
      memcpy((char *) range + iwr_off(sensitivity),
	     buffer + iwr15_off(sensitivity),
	     iwr15_off(num_bitrates) - iwr15_off(sensitivity));
      /* This one goes after avg_qual */
      memcpy((char *) range + iwr_off(num_bitrates),
	     buffer + iwr15_off(num_bitrates),
	     iwr15_off(min_rts) - iwr15_off(num_bitrates));
      /* Number of bitrates has changed, put it after */
      memcpy((char *) range + iwr_off(min_rts),
	     buffer + iwr15_off(min_rts),
	     iwr15_off(txpower_capa) - iwr15_off(min_rts));
      /* Added encoding_login_index, put it after */
      memcpy((char *) range + iwr_off(txpower_capa),
	     buffer + iwr15_off(txpower_capa),
	     iwr15_off(txpower) - iwr15_off(txpower_capa));
      /* Hum... That's an unexpected glitch. Bummer. */
      memcpy((char *) range + iwr_off(txpower),
	     buffer + iwr15_off(txpower),
	     iwr15_off(avg_qual) - iwr15_off(txpower));
      /* Avg qual moved up next to max_qual */
      memcpy((char *) range + iwr_off(avg_qual),
	     buffer + iwr15_off(avg_qual),
	     sizeof(struct iw_quality));
    }

  /* We are now checking much less than we used to do, because we can
   * accomodate more WE version. But, there are still cases where things
   * will break... */
  if (!iw_ignore_version_sp)
    {
      /* We don't like very old version (unfortunately kernel 2.2.X) */
      if (range->we_version_compiled <= 10)
	{
	  fprintf(stderr, "Warning: Driver for device %s has been compiled with an ancient version\n", "raxx");
	  fprintf(stderr, "of Wireless Extension, while this program support version 11 and later.\n");
	  fprintf(stderr, "Some things may be broken...\n\n");
	}

      /* We don't like future versions of WE, because we can't cope with
       * the unknown */
      if (range->we_version_compiled > WE_MAX_VERSION)
	{
	  fprintf(stderr, "Warning: Driver for device %s has been compiled with version %d\n", "raxx", range->we_version_compiled);
	  fprintf(stderr, "of Wireless Extension, while this program supports up to version %d.\n", WE_VERSION);
	  fprintf(stderr, "Some things may be broken...\n\n");
	}

      /* Driver version verification */
      if ((range->we_version_compiled > 10) &&
	 (range->we_version_compiled < range->we_version_source))
	{
	  fprintf(stderr, "Warning: Driver for device %s recommend version %d of Wireless Extension,\n", "raxx", range->we_version_source);
	  fprintf(stderr, "but has been compiled with version %d, therefore some driver features\n", range->we_version_compiled);
	  fprintf(stderr, "may not be available...\n\n");
	}
      /* Note : we are only trying to catch compile difference, not source.
       * If the driver source has not been updated to the latest, it doesn't
       * matter because the new fields are set to zero */
    }

  /* Don't complain twice.
   * In theory, the test apply to each individual driver, but usually
   * all drivers are compiled from the same kernel. */
  iw_ignore_version_sp = 1;

  return (0);
}


char* GetBW(int BW)
{
	switch(BW)
	{
		case BW_10:
			return "10M";

		case BW_20:
			return "20M";

		case BW_40:
			return "40M";

#if defined(RTAC52U) || defined(RTAC51U) || defined(RTN54U) || defined(RTAC1200HP) || defined(RTAC54U) || defined(RTAC1200) || defined(RTAC1200V2) || defined(RTAC1200GA1) || defined(RTAC1200GU)
		case BW_80:
			return "80M";
#endif

		default:
			return "N/A";
	}
}

char* GetPhyMode(int Mode)
{
	switch(Mode)
	{
		case MODE_CCK:
			return "CCK";

		case MODE_OFDM:
			return "OFDM";
		case MODE_HTMIX:
			return "HTMIX";

		case MODE_HTGREENFIELD:
			return "GREEN";

#if defined(RTAC52U) || defined(RTAC51U)  || defined(RTN54U) || defined(RTAC1200HP) || defined(RTAC54U) || defined(RTAC1200) || defined(RTAC1200V2) || defined(RTAC1200GA1) || defined(RTAC1200GU)
		case MODE_VHT:
			return "VHT";
#endif

		default:
			return "N/A";
	}
}

int MCSMappingRateTable[] =
	{2,  4,   11,  22, // CCK
	12, 18,   24,  36, 48, 72, 96, 108, // OFDM
	13, 26,   39,  52,  78, 104, 117, 130, 26,  52,  78, 104, 156, 208, 234, 260, // 20MHz, 800ns GI, MCS: 0 ~ 15
	39, 78,  117, 156, 234, 312, 351, 390,										  // 20MHz, 800ns GI, MCS: 16 ~ 23
	27, 54,   81, 108, 162, 216, 243, 270, 54, 108, 162, 216, 324, 432, 486, 540, // 40MHz, 800ns GI, MCS: 0 ~ 15
	81, 162, 243, 324, 486, 648, 729, 810,										  // 40MHz, 800ns GI, MCS: 16 ~ 23
	14, 29,   43,  57,  87, 115, 130, 144, 29, 59,   87, 115, 173, 230, 260, 288, // 20MHz, 400ns GI, MCS: 0 ~ 15
	43, 87,  130, 173, 260, 317, 390, 433,										  // 20MHz, 400ns GI, MCS: 16 ~ 23
	30, 60,   90, 120, 180, 240, 270, 300, 60, 120, 180, 240, 360, 480, 540, 600, // 40MHz, 400ns GI, MCS: 0 ~ 15
	90, 180, 270, 360, 540, 720, 810, 900,
	13, 26,   39,  52,  78, 104, 117, 130, 156, /* 11ac: 20Mhz, 800ns GI, MCS: 0~8 */
	27, 54,   81, 108, 162, 216, 243, 270, 324, 360, /*11ac: 40Mhz, 800ns GI, MCS: 0~9 */
	59, 117, 176, 234, 351, 468, 527, 585, 702, 780, /*11ac: 80Mhz, 800ns GI, MCS: 0~9 */
	14, 29,   43,  57,  87, 115, 130, 144, 173, /* 11ac: 20Mhz, 400ns GI, MCS: 0~8 */
	30, 60,   90, 120, 180, 240, 270, 300, 360, 400, /*11ac: 40Mhz, 400ns GI, MCS: 0~9 */
	65, 130, 195, 260, 390, 520, 585, 650, 780, 867 /*11ac: 80Mhz, 400ns GI, MCS: 0~9 */
	};

#if defined(RTCONFIG_WLMODULE_MT7663E_AP)
int MCSMappingRateTable_5G[] = {
	2,  4, 11, 22, 12,  18,  24,  36, 48,  72,  96, 108, 109, 110, 111, 112,/* CCK and OFDM */
	13, 26, 39, 52, 78, 104, 117, 130, 26,  52,  78, 104, 156, 208, 234, 260,
	39, 78, 117, 156, 234, 312, 351, 390, /* BW 20, 800ns GI, MCS 0~23 */
	27, 54, 81, 108, 162, 216, 243, 270, 54, 108, 162, 216, 324, 432, 486, 540,
	81, 162, 243, 324, 486, 648, 729, 810, /* BW 40, 800ns GI, MCS 0~23 */
	14, 29, 43, 57, 87, 115, 130, 144, 29, 59,   87, 115, 173, 230, 260, 288,
	43, 87, 130, 173, 260, 317, 390, 433, /* BW 20, 400ns GI, MCS 0~23 */
	30, 60, 90, 120, 180, 240, 270, 300, 60, 120, 180, 240, 360, 480, 540, 600,
	90, 180, 270, 360, 540, 720, 810, 900, /* BW 40, 400ns GI, MCS 0~23 */

	/*for 11ac:20 Mhz 800ns GI*/
	6,  13, 19, 26,  39,  52,  58,  65,  78,  0,     /*1ss mcs 0~8*/
	13, 26, 39, 52,  78,  104, 117, 130, 156, 0,     /*2ss mcs 0~8*/
	19, 39, 58, 78,  117, 156, 175, 195, 234, 260,   /*3ss mcs 0~9*/
	26, 52, 78, 104, 156, 208, 234, 260, 312, 0,     /*4ss mcs 0~8*/

	/*for 11ac:40 Mhz 800ns GI*/
	13,	27,	40,	54,	 81,  108, 121, 135, 162, 180,   /*1ss mcs 0~9*/
	27,	54,	81,	108, 162, 216, 243, 270, 324, 360,   /*2ss mcs 0~9*/
	40,	81,	121, 162, 243, 324, 364, 405, 486, 540,  /*3ss mcs 0~9*/
	54,	108, 162, 216, 324, 432, 486, 540, 648, 720, /*4ss mcs 0~9*/

	/*for 11ac:80 Mhz 800ns GI*/
	29,	58,	87,	117, 175, 234, 263, 292, 351, 390,   /*1ss mcs 0~9*/
	58,	117, 175, 243, 351, 468, 526, 585, 702, 780, /*2ss mcs 0~9*/
	87,	175, 263, 351, 526, 702, 0,	877, 1053, 1170, /*3ss mcs 0~9*/
	117, 234, 351, 468, 702, 936, 1053, 1170, 1404, 1560, /*4ss mcs 0~9*/

	/*for 11ac:160 Mhz 800ns GI*/
	58,	117, 175, 234, 351, 468, 526, 585, 702, 780, /*1ss mcs 0~9*/
	117, 234, 351, 468, 702, 936, 1053, 1170, 1404, 1560, /*2ss mcs 0~9*/
	175, 351, 526, 702, 1053, 1404, 1579, 1755, 2160, 0, /*3ss mcs 0~8*/
	234, 468, 702, 936, 1404, 1872, 2106, 2340, 2808, 3120, /*4ss mcs 0~9*/

	/*for 11ac:20 Mhz 400ns GI*/
	7,	14,	21,	28,  43,  57,   65,	 72,  86,  0,    /*1ss mcs 0~8*/
	14,	28,	43,	57,	 86,  115,  130, 144, 173, 0,    /*2ss mcs 0~8*/
	21,	43,	65,	86,	 130, 173,  195, 216, 260, 288,  /*3ss mcs 0~9*/
	28,	57,	86,	115, 173, 231,  260, 288, 346, 0,    /*4ss mcs 0~8*/

	/*for 11ac:40 Mhz 400ns GI*/
	15,	30,	45,	60,	 90,  120,  135, 150, 180, 200,  /*1ss mcs 0~9*/
	30,	60,	90,	120, 180, 240,  270, 300, 360, 400,  /*2ss mcs 0~9*/
	45,	90,	135, 180, 270, 360,  405, 450, 540, 600, /*3ss mcs 0~9*/
	60,	120, 180, 240, 360, 480,  540, 600, 720, 800, /*4ss mcs 0~9*/

	/*for 11ac:80 Mhz 400ns GI*/
	32,	65,	97,	130, 195, 260,  292, 325, 390, 433,  /*1ss mcs 0~9*/
	65,	130, 195, 260, 390, 520,  585, 650, 780, 866, /*2ss mcs 0~9*/
	97,	195, 292, 390, 585, 780,  0,	 975, 1170, 1300, /*3ss mcs 0~9*/
	130, 260, 390, 520, 780, 1040,	1170, 1300, 1560, 1733, /*4ss mcs 0~9*/

	/*for 11ac:160 Mhz 400ns GI*/
	65,	130, 195, 260, 390, 520,  585, 650, 780, 866, /*1ss mcs 0~9*/
	130, 260, 390, 520, 780, 1040,	1170, 1300, 1560, 1733, /*2ss mcs 0~9*/
	195, 390, 585, 780, 1170, 1560,	1755, 1950, 2340, 0, /*3ss mcs 0~8*/
	260, 520, 780, 1040, 1560, 2080,	2340, 2600, 3120, 3466, /*4ss mcs 0~9*/

	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37
}; /* 3*3 */

#define FN_GETRATE(_fn_, _st_, _if, _mcstbl)						\
_fn_(_st_ HTSetting)							\
{									\
	unsigned char Antenna = 0;	\
	unsigned char MCS = HTSetting.field.MCS;	\
	int rate_count = sizeof(_mcstbl)/sizeof(int);	\
	int rate_index = 0;						\
	int value = 0;	\
									\
	if (HTSetting.field.MODE >= MODE_VHT)				\
	{								\
		if(_if == 1) {	\
			MCS = HTSetting.field.MCS & 0xf;	\
			Antenna = (HTSetting.field.MCS >> 4) + 1;	\
														\
			if (HTSetting.field.BW == BW_20) {	\
				rate_index = 112 + ((Antenna - 1) * 10) + ((unsigned char)HTSetting.field.ShortGI * 160) + ((unsigned char)MCS);	\
			} else if (HTSetting.field.BW == BW_40) {	\
				rate_index = 152 + ((Antenna - 1) * 10) + ((unsigned char)HTSetting.field.ShortGI * 160) + ((unsigned char)MCS);	\
			} else if (HTSetting.field.BW == BW_80) {	\
				rate_index = 192 + ((Antenna - 1) * 10) + ((unsigned char)HTSetting.field.ShortGI * 160) + ((unsigned char)MCS);	\
			} else if (HTSetting.field.BW == BW_160) {	\
				rate_index = 232 + ((Antenna - 1) * 10) + ((unsigned char)HTSetting.field.ShortGI * 160) + ((unsigned char)MCS);	\
			}	\
		}	\
		else	\
		if (HTSetting.field.BW == BW_20) {			\
			rate_index = 108 +				\
			((unsigned char)HTSetting.field.ShortGI * 29) +	\
			((unsigned char)HTSetting.field.MCS);		\
		}							\
		else if (HTSetting.field.BW == BW_40) {			\
			rate_index = 117 +				\
			((unsigned char)HTSetting.field.ShortGI * 29) +	\
			((unsigned char)HTSetting.field.MCS);		\
		}							\
		else if (HTSetting.field.BW == BW_80) {			\
			rate_index = 127 +				\
			((unsigned char)HTSetting.field.ShortGI * 29) +	\
			((unsigned char)HTSetting.field.MCS);		\
		}							\
	}								\
	else								\
	if (HTSetting.field.MODE >= MODE_HTMIX)				\
	{								\
		if(_if == 1)	\
		{	\
			MCS = HTSetting.field.MCS;	\
			\
			if ((HTSetting.field.MODE == MODE_HTMIX) || (HTSetting.field.MODE == MODE_HTGREENFIELD))	\
				Antenna = (MCS >> 3) + 1;	\
			\
			/* map back to 1SS MCS , multiply by antenna numbers later */		\
			if (MCS > 7)		\
				MCS %= 8;		\
			\
			rate_index = 16 + ((unsigned char)HTSetting.field.BW * 24) + ((unsigned char)HTSetting.field.ShortGI * 48) + ((unsigned char)MCS);		\
		}else	\
			rate_index = 12 + ((unsigned char)HTSetting.field.BW *24) + ((unsigned char)HTSetting.field.ShortGI *48) + ((unsigned char)HTSetting.field.MCS);	\
	}								\
	else								\
		if (HTSetting.field.MODE == MODE_OFDM)				\
			rate_index = (unsigned char)(HTSetting.field.MCS) + 4;	\
		else if (HTSetting.field.MODE == MODE_CCK)			\
			rate_index = (unsigned char)(HTSetting.field.MCS);	\
									\
	if (rate_index < 0)						\
		rate_index = 0;						\
									\
	if (rate_index >= rate_count)					\
		rate_index = rate_count-1;				\
	\
	if(_if == 1)	{		\
		if (HTSetting.field.MODE != MODE_VHT)	\
			value = (_mcstbl[rate_index] * 5) / 10;	\
		else	\
			value =  _mcstbl[rate_index];	\
	}else		\
		value = (_mcstbl[rate_index] * 5) / 10;	\
	\
	return value;		\
}


#else

#define FN_GETRATE(_fn_, _st_, _if, _mcstbl)						\
_fn_(_st_ HTSetting)							\
{									\
	int rate_count = sizeof(_mcstbl)/sizeof(int);	\
	int rate_index = 0;						\
	int value = 0;	\
									\
	if (HTSetting.field.MODE >= MODE_VHT)				\
	{								\
		if (HTSetting.field.BW == BW_20) {			\
			rate_index = 108 +				\
			((unsigned char)HTSetting.field.ShortGI * 29) +	\
			((unsigned char)HTSetting.field.MCS);		\
		}							\
		else if (HTSetting.field.BW == BW_40) {			\
			rate_index = 117 +				\
			((unsigned char)HTSetting.field.ShortGI * 29) +	\
			((unsigned char)HTSetting.field.MCS);		\
		}							\
		else if (HTSetting.field.BW == BW_80) {			\
			rate_index = 127 +				\
			((unsigned char)HTSetting.field.ShortGI * 29) +	\
			((unsigned char)HTSetting.field.MCS);		\
		}							\
	}								\
	else								\
	if (HTSetting.field.MODE >= MODE_HTMIX)				\
	{								\
		rate_index = 12 + ((unsigned char)HTSetting.field.BW *24) + ((unsigned char)HTSetting.field.ShortGI *48) + ((unsigned char)HTSetting.field.MCS);	\
	}								\
	else								\
	if (HTSetting.field.MODE == MODE_OFDM)				\
		rate_index = (unsigned char)(HTSetting.field.MCS) + 4;	\
	else if (HTSetting.field.MODE == MODE_CCK)			\
		rate_index = (unsigned char)(HTSetting.field.MCS);	\
									\
	if (rate_index < 0)						\
		rate_index = 0;						\
									\
	if (rate_index >= rate_count)					\
		rate_index = rate_count-1;				\
	\
	if (HTSetting.field.MODE != MODE_VHT)	\
		value = (_mcstbl[rate_index] * 5) / 10;	\
	else	\
		value =  _mcstbl[rate_index];	\
	return value;		\
}

#endif

#if defined(RTCONFIG_HAS_5G)
#if defined(RTCONFIG_WLMODULE_MT7663E_AP) 
int FN_GETRATE(getRate,      MACHTTRANSMIT_SETTING_for_5G, 1, MCSMappingRateTable_5G)		//getRate   (MACHTTRANSMIT_SETTING_for_5G)
#else
int FN_GETRATE(getRate,      MACHTTRANSMIT_SETTING_for_5G, 1, MCSMappingRateTable)		//getRate   (MACHTTRANSMIT_SETTING_for_5G)
#endif
#endif	/* RTCONFIG_HAS_5G */
int FN_GETRATE(getRate_2g,   MACHTTRANSMIT_SETTING_for_2G, 0, MCSMappingRateTable)		//getRate_2g(MACHTTRANSMIT_SETTING_for_2G)



int
ej_wl_status(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int retval = 0;
	int ii = 0;
	char word[256], *next;

	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(ii);
		retval += wl_status(eid, wp, argc, argv, ii);
		retval += websWrite(wp, "\n");

		ii++;
	}

	return retval;
}

int
ej_wl_status_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_status(eid, wp, argc, argv, 0);
}

static int
wl_status(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int ret = 0;
	int channel;
	struct iw_range	range;
	double freq;
	struct iwreq wrq0;
	struct iwreq wrq1;
	struct iwreq wrq2;
	struct iwreq wrq3;
	unsigned long phy_mode;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_", *ifname;
	int wl_mode_x;
	int r;

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	ifname = nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)));

#if 0
	if (nvram_match(strlcat_r(prefix, "radio", tmp, sizeof(tmp)), "0"))
	{
		ret+=websWrite(wp, "%s radio is disabled\n",
			wl_nband_name(nvram_pf_get(prefix, "nband")));
		return ret;
	}
#else
	if (!get_radio_status(ifname))
	{
#if defined(BAND_2G_ONLY)
		ret+=websWrite(wp, "2.4 GHz radio is disabled\n");
#else
		ret+=websWrite(wp, "%s radio is disabled\n",
			wl_nband_name(nvram_pf_get(prefix, "nband")));
#endif
		return ret;
	}
#endif

	if (wl_ioctl(ifname, SIOCGIWAP, &wrq0) < 0)
	{
#if defined(BAND_2G_ONLY)
		ret+=websWrite(wp, "2.4 GHz radio is disabled\n");
#else
		ret+=websWrite(wp, "%s radio is disabled\n",
			wl_nband_name(nvram_pf_get(prefix, "nband")));
#endif
		return ret;
	}

	wrq0.u.ap_addr.sa_family = ARPHRD_ETHER;
	ret+=websWrite(wp, "MAC address	: %02X:%02X:%02X:%02X:%02X:%02X\n",
			(unsigned char)wrq0.u.ap_addr.sa_data[0],
			(unsigned char)wrq0.u.ap_addr.sa_data[1],
			(unsigned char)wrq0.u.ap_addr.sa_data[2],
			(unsigned char)wrq0.u.ap_addr.sa_data[3],
			(unsigned char)wrq0.u.ap_addr.sa_data[4],
			(unsigned char)wrq0.u.ap_addr.sa_data[5]);

	if (wl_ioctl(ifname, SIOCGIWFREQ, &wrq1) < 0)
		return ret;

	char buffer[sizeof(iwrange) * 2];
	bzero(buffer, sizeof(buffer));
	wrq2.u.data.pointer = (caddr_t) buffer;
	wrq2.u.data.length = sizeof(buffer);
	wrq2.u.data.flags = 0;

	if (wl_ioctl(ifname, SIOCGIWRANGE, &wrq2) < 0)
		return ret;

	if (ralink_get_range_info(&range, buffer, wrq2.u.data.length) < 0)
		return ret;

#if defined(RTN65U)
	if (unit == 0 && get_model() == MODEL_RTN65U)
	{
		FILE *fp;
		phy_mode = 0;
		if((fp = fopen("/etc/Wireless/iNIC/iNIC_ap.dat", "r")) != NULL)
		{
			while(fgets(tmp, sizeof(tmp), fp) != NULL)
			{
				if(strncmp(tmp, "WirelessMode=", 13) == 0)
				{
					phy_mode = atoi(tmp + 13);
					break;
				}
			}
			fclose(fp);
		}
	}
	else
	{
#endif	/* RTN65U */
	bzero(buffer, sizeof(unsigned long));
	wrq2.u.data.length = sizeof(unsigned long);
	wrq2.u.data.pointer = (caddr_t) buffer;
	wrq2.u.data.flags = RT_OID_GET_PHY_MODE;

	if (wl_ioctl(ifname, RT_PRIV_IOCTL, &wrq2) < 0)
		return ret;

	if(wrq2.u.mode == (__u32) buffer) //.u.mode is at the same location as u.data.pointer
	{ //new wifi driver
		phy_mode = 0;
		memcpy(&phy_mode, wrq2.u.data.pointer, wrq2.u.data.length);
	}
	else
		phy_mode=wrq2.u.mode;
#if defined(RTN65U)
	}
#endif	/* RTN65U */

	freq = iw_freq2float(&(wrq1.u.freq));
	if (freq < KILO)
		channel = (int) freq;
	else
	{
		channel = iw_freq_to_channel(freq, &range);
		if (channel < 0)
			return ret;
	}

	wl_mode_x = nvram_get_int(strlcat_r(prefix, "mode_x", tmp, sizeof(tmp)));
	if	(wl_mode_x == 1)
		ret+=websWrite(wp, "OP Mode		: WDS Only\n");
	else if (wl_mode_x == 2)
		ret+=websWrite(wp, "OP Mode		: Hybrid\n");
	else
		ret+=websWrite(wp, "OP Mode		: AP\n");

#if defined(RTAC52U) || defined(RTAC51U) || defined(RTN54U)  || defined(RTAC1200HP) || defined(RTAC54U) || defined(RTAC1200) || defined(RTAC1200GA1) || defined(RTAC1200GU) || defined(RTAC85U) || defined(RTAC85P) || defined(MTK_REP) || defined(RTACRH26) || defined(TUFAC1750)
	if (unit == 1)
	{
		char *p = tmp;
		size_t len = sizeof(tmp);

		*tmp = '\0';
		if(phy_mode & WMODE_A) {
			strlcat(p, "/a", len);
			p += 2;
			len -= 2;
		}
		if(phy_mode & WMODE_B) {
			strlcat(p, "/b", len);
			p += 2;
			len -= 2;
		}
		if(phy_mode & WMODE_G) {
			strlcat(p, "/g", len);
			p += 2;
			len -= 2;
		}
		if(phy_mode & WMODE_GN) {
			strlcat(p, "/n", len);	//N in 2G
			p += 2;
			len -= 2;
		}
		if(phy_mode & WMODE_AN) {
			strlcat(p, "/n", len);	//N in 5G
			p += 2;
			len -= 2;
		}
		if(phy_mode & WMODE_AC) {
			strlcat(p, "/ac", len);
			p += 3;
			len -= 3;
		}
		if(*tmp != '\0')
			ret+=websWrite(wp, "Phy Mode	: 11%s\n", tmp+1); // skip first '/'
	}
	else
#else if defined(RTCONFIG_WLMODULE_MT7663E_AP)
	if (unit == 1 || unit == 0)
	{
		char *p = tmp;
		size_t len = sizeof(tmp);

		*tmp = '\0';
		if(phy_mode & WMODE_A) {
			strlcat(p, "/a", len);
			p += 2;
			len -= 2;
		}
		if(phy_mode & WMODE_B) {
			strlcat(p, "/b", len);
			p += 2;
			len -= 2;
		}
		if(phy_mode & WMODE_G) {
			strlcat(p, "/g", len);
			p += 2;
			len -= 2;
		}
		if(phy_mode & WMODE_GN) {
			strlcat(p, "/n", len);	//N in 2G
			p += 2;
			len -= 2;
		}
		if(phy_mode & WMODE_AN) {
			strlcat(p, "/n", len);	//N in 5G
			p += 2;
			len -= 2;
		}
		if(phy_mode & WMODE_AC) {
			strlcat(p, "/ac", len);
			p += 3;
			len -= 3;
		}
		if(*tmp != '\0')
			ret+=websWrite(wp, "Phy Mode	: 11%s\n", tmp+1); // skip first '/'
	}
	else
#endif
	if (phy_mode==PHY_11BG_MIXED)
		ret+=websWrite(wp, "Phy Mode	: 11b/g\n");
	else if (phy_mode==PHY_11B)
		ret+=websWrite(wp, "Phy Mode	: 11b\n");
	else if (phy_mode==PHY_11A)
		ret+=websWrite(wp, "Phy Mode	: 11a\n");
	else if (phy_mode==PHY_11ABG_MIXED)
		ret+=websWrite(wp, "Phy Mode	: 11a/b/g\n");
	else if (phy_mode==PHY_11G)
		ret+=websWrite(wp, "Phy Mode	: 11g\n");
	else if (phy_mode==PHY_11ABGN_MIXED)
		ret+=websWrite(wp, "Phy Mode	: 11a/b/g/n\n");
	else if (phy_mode==PHY_11N)
		ret+=websWrite(wp, "Phy Mode	: 11n\n");
	else if (phy_mode==PHY_11GN_MIXED)
		ret+=websWrite(wp, "Phy Mode	: 11g/n\n");
	else if (phy_mode==PHY_11AN_MIXED)
		ret+=websWrite(wp, "Phy Mode	: 11a/n\n");
	else if (phy_mode==PHY_11BGN_MIXED)
		ret+=websWrite(wp, "Phy Mode	: 11b/g/n\n");
	else if (phy_mode==PHY_11AGN_MIXED)
		ret+=websWrite(wp, "Phy Mode	: 11a/g/n\n");

	ret+=websWrite(wp, "Channel		: %d\n", channel);

	char data[16384];
	memset(data, 0, sizeof(data));
	wrq3.u.data.pointer = data;
	wrq3.u.data.length = sizeof(data);
	wrq3.u.data.flags = 0;

	if ((r = wl_ioctl(ifname, RTPRIV_IOCTL_GET_MAC_TABLE, &wrq3)) < 0) {
		_dprintf("%s: Take MAC table from i/f %s fail! ret %d, errno %d (%s)\n",
			__func__, r, errno, strerror(errno));
		return ret;
	}

	RT_802_11_MAC_TABLE_5G* mp =(RT_802_11_MAC_TABLE_5G*)wrq3.u.data.pointer;
	RT_802_11_MAC_TABLE_2G* mp2=(RT_802_11_MAC_TABLE_2G*)wrq3.u.data.pointer;
	int i;

	ret+=websWrite(wp, "\nStations List			   \n");
	ret+=websWrite(wp, "----------------------------------------\n");
	ret+=websWrite(wp, "%-18s%-4s%-8s%-4s%-4s%-4s%-5s%-5s%-12s\n",
			   "MAC", "PSM", "PhyMode", "BW", "MCS", "SGI", "STBC", "Rate", "Connect Time");

#define SHOW_STA_INFO(_p,_i,_st, _gr) {											\
		int hr, min, sec;											\
		_st *Entry = ((_st *)(_p)) + _i;									\
		hr = Entry->ConnectedTime/3600;										\
		min = (Entry->ConnectedTime % 3600)/60;									\
		sec = Entry->ConnectedTime - hr*3600 - min*60;						\
		ret+=websWrite(wp, "%02X:%02X:%02X:%02X:%02X:%02X %s %-7s %s %3d %s %s  %3dM %02d:%02d:%02d\n",		\
				Entry->Addr[0], Entry->Addr[1],								\
				Entry->Addr[2], Entry->Addr[3],								\
				Entry->Addr[4], Entry->Addr[5],								\
				Entry->Psm ? "Yes" : "NO ",								\
				GetPhyMode(Entry->TxRate.field.MODE),							\
				GetBW(Entry->TxRate.field.BW),								\
				Entry->TxRate.field.MCS,								\
				Entry->TxRate.field.ShortGI ? "Yes" : "NO ",						\
				Entry->TxRate.field.STBC ? "Yes" : "NO ",						\
				_gr(Entry->TxRate),									\
				hr, min, sec										\
		);													\
	}

	if (!strcmp(ifname, WIF_2G)) {
		for (i=0;i<mp->Num;i++) {
			SHOW_STA_INFO(mp2->Entry, i, RT_802_11_MAC_ENTRY_for_2G, getRate_2g);
		}
	}
#if defined(RTCONFIG_HAS_5G)
	else {
		for (i=0;i<mp->Num;i++) {
			SHOW_STA_INFO(mp->Entry, i, RT_802_11_MAC_ENTRY_for_5G, getRate);
		}
	}
#endif	/* RTCONFIG_HAS_5G */

	return ret;
}

typedef struct PACKED _WSC_CONFIGURED_VALUE {
    unsigned short WscConfigured;	// 1 un-configured; 2 configured
    unsigned char WscSsid[32 + 1];
    unsigned short WscAuthMode;		// mandatory, 0x01: open, 0x02: wpa-psk, 0x04: shared, 0x08:wpa, 0x10: wpa2, 0x
    unsigned short WscEncrypType;	// 0x01: none, 0x02: wep, 0x04: tkip, 0x08: aes
    unsigned char DefaultKeyIdx;
    unsigned char WscWPAKey[64 + 1];
} WSC_CONFIGURED_VALUE;

void getWPSAuthMode(WSC_CONFIGURED_VALUE *result, char *ret_str, int len)
{
	if (result->WscAuthMode & 0x1)
		strlcat(ret_str, "Open System", len);
	if (result->WscAuthMode & 0x2)
		strlcat(ret_str, "WPA-Personal", len);
	if (result->WscAuthMode & 0x4)
		strlcat(ret_str, "Shared Key", len);
	if (result->WscAuthMode & 0x8)
		strlcat(ret_str, "WPA-Enterprise", len);
	if (result->WscAuthMode & 0x10)
		strlcat(ret_str, "WPA2-Enterprise", len);
	if (result->WscAuthMode & 0x20)
		strlcat(ret_str, "WPA2-Personal", len);
}

void getWPSEncrypType(WSC_CONFIGURED_VALUE *result, char *ret_str, int len)
{
	if (result->WscEncrypType & 0x1)
		strlcat(ret_str, "None", len);
	if (result->WscEncrypType & 0x2)
		strlcat(ret_str, "WEP", len);
	if (result->WscEncrypType & 0x4)
		strlcat(ret_str, "TKIP", len);
	if (result->WscEncrypType & 0x8)
		strlcat(ret_str, "AES", len);
}

/*
 * these definitions are from rt2860v2 driver include/wsc.h
 */
char *getWscStatusStr(int status)
{
	switch(status) {
	case 0:
		return "Not used";
	case 1:
		return "Idle";
	case 2:
#if 0
		return "WPS Fail(Ignore this if Intel/Marvell registrar used)";
#else
		return "Idle";
#endif
	case 3:
		return "Start WPS Process";
	case 4:
		return "Received EAPOL-Start";
	case 5:
		return "Sending EAP-Req(ID)";
	case 6:
		return "Receive EAP-Rsp(ID)";
	case 7:
		return "Receive EAP-Req with wrong WPS SMI Vendor Id";
	case 8:
		return "Receive EAP-Req with wrong WPS Vendor Type";
	case 9:
		return "Sending EAP-Req(WPS_START)";
	case 10:
		return "Send M1";
	case 11:
		return "Received M1";
	case 12:
		return "Send M2";
	case 13:
		return "Received M2";
	case 14:
		return "Received M2D";
	case 15:
		return "Send M3";
	case 16:
		return "Received M3";
	case 17:
		return "Send M4";
	case 18:
		return "Received M4";
	case 19:
		return "Send M5";
	case 20:
		return "Received M5";
	case 21:
		return "Send M6";
	case 22:
		return "Received M6";
	case 23:
		return "Send M7";
	case 24:
		return "Received M7";
	case 25:
		return "Send M8";
	case 26:
		return "Received M8";
	case 27:
		return "Processing EAP Response (ACK)";
	case 28:
		return "Processing EAP Request (Done)";
	case 29:
		return "Processing EAP Response (Done)";
	case 30:
		return "Sending EAP-Fail";
	case 31:
		return "WPS_ERROR_HASH_FAIL";
	case 32:
		return "WPS_ERROR_HMAC_FAIL";
	case 33:
		return "WPS_ERROR_DEV_PWD_AUTH_FAIL";
	case 34:
//		return "Configured";
		return "Success";
	case 35:
		return "SCAN AP";
	case 36:
		return "EAPOL START SENT";
	case 37:
		return "WPS_EAP_RSP_DONE_SENT";
	case 38:
		return "WAIT PINCODE";
	case 39:
		return "WSC_START_ASSOC";
	case 0x101:
		return "PBC:TOO MANY AP";
	case 0x102:
		return "PBC:NO AP";
	case 0x103:
		return "EAP_FAIL_RECEIVED";
	case 0x104:
		return "EAP_NONCE_MISMATCH";
	case 0x105:
		return "EAP_INVALID_DATA";
	case 0x106:
		return "PASSWORD_MISMATCH";
	case 0x107:
		return "EAP_REQ_WRONG_SMI";
	case 0x108:
		return "EAP_REQ_WRONG_VENDOR_TYPE";
	case 0x109:
		return "PBC_SESSION_OVERLAP";
	default:
		return "Unknown";
	}
}

int getWscStatus(int unit)
{
	int data = 0;
	struct iwreq wrq;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	wrq.u.data.length = sizeof(data);
	wrq.u.data.pointer = (caddr_t) &data;
	wrq.u.data.flags = RT_OID_WSC_QUERY_STATUS;

	if (wl_ioctl(nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp))), RT_PRIV_IOCTL, &wrq) < 0)
		fprintf(stderr, "errors in getting WSC status\n");

	return data;
}

unsigned int getAPPIN(int unit)
{
	unsigned int data = 0;
	struct iwreq wrq;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	wrq.u.data.length = sizeof(data);
	wrq.u.data.pointer = (caddr_t) &data;
	wrq.u.data.flags = RT_OID_WSC_PIN_CODE;

	if (wl_ioctl(nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp))), RT_PRIV_IOCTL, &wrq) < 0)
		fprintf(stderr, "errors in getting AP PIN\n");

	return data;
}

int
wl_wps_info(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int i, j = -1, u = unit;
	char tmpstr[128], tmpstr2[256];
	WSC_CONFIGURED_VALUE result;
	int retval=0;
	struct iwreq wrq;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *wps_sta_pin;
	char tag1[] = "<wps_infoXXXXXX>", tag2[] = "</wps_infoXXXXXX>";

#if defined(RTCONFIG_WPSMULTIBAND)
	for (j = -1; j < MAX_NR_WL_IF; ++j) {
		SKIP_ABSENT_BAND(unit);
#endif
		switch (j) {
		case WL_2G_BAND:	/* fall through */
		case WL_5G_BAND:	/* fall through */
		case WL_5G_2_BAND:	/* fall through */
		case WL_60G_BAND:	/* fall through */
			u = j;
			snprintf(tag1, sizeof(tag1), "<wps_info%d>", j);
			snprintf(tag2, sizeof(tag2), "</wps_info%d>", j);
			break;
		case -1: /* fall through */
		default:
			u = unit;
			strlcpy(tag1, "<wps_info>", sizeof(tag1));
			strlcpy(tag2, "</wps_info>", sizeof(tag2));
		}

		snprintf(prefix, sizeof(prefix), "wl%d_", u);
		wrq.u.data.length = sizeof(WSC_CONFIGURED_VALUE);
		wrq.u.data.pointer = (caddr_t) &result;
		wrq.u.data.flags = 0;
		strlcpy((char *)&result, "get_wsc_profile", sizeof(result));	/* FIXME */

#if defined(RTCONFIG_WPSMULTIBAND)
		if (!nvram_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp))))
			continue;
#endif

		if (wl_ioctl(nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp))), RTPRIV_IOCTL_WSC_PROFILE, &wrq) < 0) {
			fprintf(stderr, "errors in getting WSC profile\n");
			return 0;
		}

		if (j == -1)
			retval += websWrite(wp, "<wps>\n");

		//0. WSC Status
		retval += websWrite(wp, "%s%s%s\n", tag1, getWscStatusStr(getWscStatus(u)), tag2);

		//1. WPSConfigured
		if (result.WscConfigured==2)
			retval += websWrite(wp, "%s%s%s\n", tag1, "Yes", tag2);
		else
			retval += websWrite(wp, "%s%s%s\n", tag1, "No", tag2);

		//2. WPSSSID
		memset(tmpstr, 0, sizeof(tmpstr));
		char_to_ascii(tmpstr, result.WscSsid);
		retval += websWrite(wp, "%s%s%s\n", tag1, tmpstr, tag2);

		//3. WPSAuthMode
		memset(tmpstr, 0, sizeof(tmpstr));
		getWPSAuthMode(&result, tmpstr, sizeof(tmpstr));
		retval += websWrite(wp, "%s%s%s\n", tag1, tmpstr, tag2);

		//4. EncrypType
		memset(tmpstr, 0, sizeof(tmpstr));
		getWPSEncrypType(&result, tmpstr, sizeof(tmpstr));
		retval += websWrite(wp, "%s%s%s\n", tag1, tmpstr, tag2);

		//5. DefaultKeyIdx
		memset(tmpstr, 0, sizeof(tmpstr));
		snprintf(tmpstr, sizeof(tmpstr), "%d", result.DefaultKeyIdx);
		retval += websWrite(wp, "%s%s%s\n", tag1, tmpstr, tag2);

		//6. WPAKey
#if 0	//hide for security
		memset(tmpstr, 0, sizeof(tmpstr));
		for (i=0; i<64; i++)	// WPA key default length is 64 (defined & hardcode in driver)
		{
			snprintf(tmpstr, sizeof(tmpstr), "%s%c", tmpstr, result.WscWPAKey[i]);
		}
		if (!strlen(tmpstr))
			retval += websWrite(wp, "%sNone%s\n", tag1, tag2);
		else
		{
			char_to_ascii(tmpstr2, tmpstr);
			retval += websWrite(wp, "%s%s%s\n", tag1, tmpstr2, tag2);
		}
#else
		retval += websWrite(wp, "%s%s\n", tag1, tag2);
#endif
		//7. AP PIN Code
		retval += websWrite(wp, "%s%08d%s\n", tag1, getAPPIN(u), tag2);

		//8. Saved WPAKey
#if 0	//hide for security
		if (!strlen(nvram_safe_get(strlcat_r(prefix, "wpa_psk", tmp, sizeof(tmp)))))
			retval += websWrite(wp, "%s%s%s\n", tag1, "None", tag2);
		else
		{
			char_to_ascii(tmpstr, nvram_safe_get(strlcat_r(prefix, "wpa_psk", tmp, sizeof(tmp))));
			retval += websWrite(wp, "%s%s%s\n", tag1, tmpstr, tag2);
		}
#else
		retval += websWrite(wp, "%s%s\n", tag1, tag2);
#endif
		//9. WPS enable?
		if (!strcmp(nvram_safe_get(strlcat_r(prefix, "wps_mode", tmp, sizeof(tmp))), "enabled"))
			retval += websWrite(wp, "%s%s%s\n", tag1, "None", tag2);
		else
			retval += websWrite(wp, "%s%s%s\n", tag1, nvram_safe_get("wps_enable"), tag2);

		//A. WPS mode
		wps_sta_pin = nvram_safe_get("wps_sta_pin");
		if (strlen(wps_sta_pin) && strcmp(wps_sta_pin, "00000000"))
			retval += websWrite(wp, "%s%s%s\n", tag1, "1", tag2);
		else
			retval += websWrite(wp, "%s%s%s\n", tag1, "2", tag2);

		//B. current auth mode
		if (!strlen(nvram_safe_get(strlcat_r(prefix, "auth_mode_x", tmp, sizeof(tmp)))))
			retval += websWrite(wp, "%s%s%s\n", tag1, "None", tag2);
		else
			retval += websWrite(wp, "%s%s%s\n", tag1, nvram_safe_get(strlcat_r(prefix, "auth_mode_x", tmp, sizeof(tmp))), tag2);

		//C. WPS band
		retval += websWrite(wp, "%s%d%s\n", tag1, u, tag2);
#if defined(RTCONFIG_WPSMULTIBAND)
	}
#endif

	retval += websWrite(wp, "</wps>");

	return retval;
}

int
ej_wps_info(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_wps_info(eid, wp, argc, argv, 1);
}

int
ej_wps_info_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_wps_info(eid, wp, argc, argv, 0);
}

// Wireless Client List		 /* Start --Alicia, 08.09.23 */

int ej_wl_sta_list_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	struct iwreq wrq;
	int i, firstRow;
	char data[16384];
	char mac[ETHER_ADDR_STR_LEN];
	RT_802_11_MAC_TABLE_2G *mp2;
	char *value;
	int rssi, cnt;
	int from_app = 0;

	from_app = check_user_agent(user_agent);

	memset(mac, 0, sizeof(mac));

#if defined(RTAC85U) || defined(RTAC85P) || defined(RTACRH26) || defined(TUFAC1750)
	if (!nvram_get_int("wlready"))
		goto exit;
#endif

	/* query wl for authenticated sta list */
	memset(data, 0, sizeof(data));
	wrq.u.data.pointer = data;
	wrq.u.data.length = sizeof(data);
	wrq.u.data.flags = 0;
	if (wl_ioctl(WIF_2G, RTPRIV_IOCTL_GET_MAC_TABLE, &wrq) < 0)
		goto exit;

	/* build wireless sta list */
	firstRow = 1;
	mp2 = (RT_802_11_MAC_TABLE_2G *)wrq.u.data.pointer;
	for (i = 0; i<mp2->Num; i++)
	{
		rssi = cnt = 0;
		if (mp2->Entry[i].AvgRssi0) {
			rssi += mp2->Entry[i].AvgRssi0;
			cnt++;
		}
		if (mp2->Entry[i].AvgRssi1) {
			rssi += mp2->Entry[i].AvgRssi1;
			cnt++;
		}
		if (mp2->Entry[i].AvgRssi2) {
			rssi += mp2->Entry[i].AvgRssi2;
			cnt++;
		}
		if (cnt == 0)
			continue;	//skip this sta info

		if (firstRow == 1)
			firstRow = 0;
		else
			websWrite(wp, ", ");

		if (from_app == 0)
			websWrite(wp, "[");

		snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
				mp2->Entry[i].Addr[0], mp2->Entry[i].Addr[1],
				mp2->Entry[i].Addr[2], mp2->Entry[i].Addr[3],
				mp2->Entry[i].Addr[4], mp2->Entry[i].Addr[5]);
		websWrite(wp, "\"%s\"", mac);

		if (from_app != 0) {
			websWrite(wp, ":{");
			websWrite(wp, "\"isWL\":");
		}

		value = "Yes";
		if (from_app == 0)
			websWrite(wp, ", \"%s\"", value);
		else
			websWrite(wp, "\"%s\"", value);

		value = "";
		if (from_app == 0)
			websWrite(wp, ", \"%s\"", value);

		if (from_app != 0)
			websWrite(wp, ",\"rssi\":");

		rssi = rssi / cnt;
		if (from_app == 0)
			websWrite(wp, ", \"%d\"", rssi);
		else
			websWrite(wp, "\"%d\"", rssi);


		if (from_app == 0)
			websWrite(wp, "]");
		else
			websWrite(wp, "}");
	}

	/* error/exit */
exit:
	return 0;
}

int ej_wl_sta_list_5g(int eid, webs_t wp, int argc, char_t **argv)
{
#if defined(RTCONFIG_HAS_5G)
	struct iwreq wrq;
	int i, firstRow;
	char data[16384];
	char mac[ETHER_ADDR_STR_LEN];
	RT_802_11_MAC_TABLE_5G *mp;
	char *value;
	int rssi, cnt;
	int from_app = 0;

	from_app = check_user_agent(user_agent);

	memset(mac, 0, sizeof(mac));

#if defined(RTAC85U) || defined(RTAC85P) || defined(RTACRH26) || defined(TUFAC1750)
	if (!nvram_get_int("wlready"))
		goto exit;
#endif

	/* query wl for authenticated sta list */
	memset(data, 0, sizeof(data));
	wrq.u.data.pointer = data;
	wrq.u.data.length = sizeof(data);
	wrq.u.data.flags = 0;
	if (wl_ioctl(WIF_5G, RTPRIV_IOCTL_GET_MAC_TABLE, &wrq) < 0)
		goto exit;

	/* build wireless sta list */
	firstRow = 1;
	mp = (RT_802_11_MAC_TABLE_5G *)wrq.u.data.pointer;
	for (i = 0; i<mp->Num; i++)
	{
		rssi = cnt = 0;
		if (mp->Entry[i].AvgRssi0) {
			rssi += mp->Entry[i].AvgRssi0;
			cnt++;
		}
		if (mp->Entry[i].AvgRssi1) {
			rssi += mp->Entry[i].AvgRssi1;
			cnt++;
		}
		if (mp->Entry[i].AvgRssi2) {
			rssi += mp->Entry[i].AvgRssi2;
			cnt++;
		}
		if (cnt == 0)
			continue;	//skip this sta info

		if (firstRow == 1)
			firstRow = 0;
		else
			websWrite(wp, ", ");

		if (from_app == 0)
			websWrite(wp, "[");

		snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
				mp->Entry[i].Addr[0], mp->Entry[i].Addr[1],
				mp->Entry[i].Addr[2], mp->Entry[i].Addr[3],
				mp->Entry[i].Addr[4], mp->Entry[i].Addr[5]);
		websWrite(wp, "\"%s\"", mac);

		if (from_app != 0) {
			websWrite(wp, ":{");
			websWrite(wp, "\"isWL\":");
		}

		value = "Yes";
		if (from_app == 0)
			websWrite(wp, ", \"%s\"", value);
		else
			websWrite(wp, "\"%s\"", value);

		value = "";
		if (from_app == 0)
			websWrite(wp, ", \"%s\"", value);

		if (from_app != 0)
			websWrite(wp, ",\"rssi\":");

		rssi = rssi / cnt;
		if (from_app == 0)
			websWrite(wp, ", \"%d\"", rssi);
		else
			websWrite(wp, "\"%d\"", rssi);


		if (from_app == 0)
			websWrite(wp, "]");
		else
			websWrite(wp, "}");
	}

	/* error/exit */
#endif	/* RTCONFIG_HAS_5G */
exit:
	return 0;
}

int ej_get_wlstainfo_list(int eid, webs_t wp, int argc, char_t **argv)
{
	char word[64], *next;
	int unit = 0;
	int haveInfo = 0;

	websWrite(wp, "{");

	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		char tmp[128], prefix[] = "wlXXXXXXXXXX_";
		struct iwreq wrq;
		int i, j, s, firstRow;
		char data[16384];
		char mac[ETHER_ADDR_STR_LEN];
		RT_802_11_MAC_TABLE_5G *mp;
		RT_802_11_MAC_TABLE_2G *mp2;
		char *name;
		int rssi, cnt;
		char alias[16];

		SKIP_ABSENT_BAND_AND_INC_UNIT(unit);
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		name = nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)));

		/* query wl for authenticated sta list */
		memset(data, 0, sizeof(data));
		wrq.u.data.pointer = data;
		wrq.u.data.length = sizeof(data);
		wrq.u.data.flags = 0;
		if (wl_ioctl(name, RTPRIV_IOCTL_GET_MAC_TABLE, &wrq) < 0)
			goto exit;

		/* build wireless sta list */
		if (unit == 0) {
			mp2 = (RT_802_11_MAC_TABLE_2G *)wrq.u.data.pointer;

			for (i = 0; i < MAX_NO_MSSID; i++) {
				firstRow = 1;
				memset(alias, 0, sizeof(alias));

				if (i == 0)
					snprintf(alias, sizeof(alias), "%s", unit ? (unit == 2 ? "5G1" : "5G") : "2G");
				else
					snprintf(alias, sizeof(alias), "%s_%d", unit ? (unit == 2 ? "5G1" : "5G") : "2G", i);

				for (j = 0; j<mp2->Num; j++) {
					s = mp2->Entry[j].ApIdx;
					if (i != s)
						continue;

					rssi = cnt = 0;
					if (mp2->Entry[j].AvgRssi0) {
						rssi += mp2->Entry[j].AvgRssi0;
						cnt++;
					}
					if (mp2->Entry[j].AvgRssi1) {
						rssi += mp2->Entry[j].AvgRssi1;
						cnt++;
					}
					if (mp2->Entry[j].AvgRssi2) {
						rssi += mp2->Entry[j].AvgRssi2;
						cnt++;
					}

					if (firstRow == 1) {
						if (haveInfo)
							websWrite(wp, ",");
						websWrite(wp, "\"%s\":[", alias);
						firstRow = 0;
						haveInfo = 1;
					}
					else
						websWrite(wp, ",");

					memset(mac, 0, sizeof(mac));
					snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
							mp2->Entry[j].Addr[0], mp2->Entry[j].Addr[1],
							mp2->Entry[j].Addr[2], mp2->Entry[j].Addr[3],
							mp2->Entry[j].Addr[4], mp2->Entry[j].Addr[5]);
					rssi = rssi / cnt;
					websWrite(wp, "{\"mac\":\"%s\",\"rssi\":%d}", mac, rssi);
				}

				if (!firstRow)
					websWrite(wp, "]");
			}
		}
		else
		{
			mp = (RT_802_11_MAC_TABLE_5G *)wrq.u.data.pointer;

			for (i = 0; i < MAX_NO_MSSID; i++) {
				firstRow = 1;
				memset(alias, 0, sizeof(alias));

				if (i == 0)
					snprintf(alias, sizeof(alias), "%s", unit ? (unit == 2 ? "5G1" : "5G") : "2G");
				else
					snprintf(alias, sizeof(alias), "%s_%d", unit ? (unit == 2 ? "5G1" : "5G") : "2G", i);

				for (j = 0; j<mp->Num; j++) {
					s = mp->Entry[j].ApIdx;
					if (i != s)
						continue;

					rssi = cnt = 0;
					if (mp->Entry[j].AvgRssi0) {
						rssi += mp->Entry[j].AvgRssi0;
						cnt++;
					}
					if (mp->Entry[j].AvgRssi1) {
						rssi += mp->Entry[j].AvgRssi1;
						cnt++;
					}
					if (mp->Entry[j].AvgRssi2) {
						rssi += mp->Entry[j].AvgRssi2;
						cnt++;
					}

					if (firstRow == 1) {
						if (haveInfo)
							websWrite(wp, ",");
						websWrite(wp, "\"%s\":[", alias);
						firstRow = 0;
						haveInfo = 1;
					}
					else
						websWrite(wp, ",");

					memset(mac, 0, sizeof(mac));
					snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
							mp->Entry[j].Addr[0], mp->Entry[j].Addr[1],
							mp->Entry[j].Addr[2], mp->Entry[j].Addr[3],
							mp->Entry[j].Addr[4], mp->Entry[j].Addr[5]);
					rssi = rssi / cnt;
					websWrite(wp, "{\"mac\":\"%s\",\"rssi\":%d}", mac, rssi);
				}

				if (!firstRow)
					websWrite(wp, "]");
			}
		}

		unit++;
	}

	/* error/exit */
exit:

	websWrite(wp, "}");

	return 0;
}

int ej_wl_auth_list(int eid, webs_t wp, int argc, char_t **argv)
{
	struct iwreq wrq;
	int i, firstRow;
	char data[16384];
	char mac[ETHER_ADDR_STR_LEN];
	RT_802_11_MAC_TABLE_5G *mp;
	RT_802_11_MAC_TABLE_2G *mp2;
	char *value;

	memset(mac, 0, sizeof(mac));

	/* query wl for authenticated sta list */
	memset(data, 0, sizeof(data));
	wrq.u.data.pointer = data;
	wrq.u.data.length = sizeof(data);
	wrq.u.data.flags = 0;
	if (wl_ioctl(WIF_2G, RTPRIV_IOCTL_GET_MAC_TABLE, &wrq) < 0)
		goto exit;

	/* build wireless sta list */
	firstRow = 1;
	mp2 = (RT_802_11_MAC_TABLE_2G *)wrq.u.data.pointer;
	for (i = 0; i<mp2->Num; i++)
	{
		if (firstRow == 1)
			firstRow = 0;
		else
			websWrite(wp, ", ");
		websWrite(wp, "[");

		snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
				mp2->Entry[i].Addr[0], mp2->Entry[i].Addr[1],
				mp2->Entry[i].Addr[2], mp2->Entry[i].Addr[3],
				mp2->Entry[i].Addr[4], mp2->Entry[i].Addr[5]);
		websWrite(wp, "\"%s\"", mac);

		value = "Yes";
		websWrite(wp, ", \"%s\"", value);

		value = "";
		websWrite(wp, ", \"%s\"", value);

		websWrite(wp, "]");
	}

#if defined(RTCONFIG_HAS_5G)
	/* query wl for authenticated sta list */
	memset(data, 0, sizeof(data));
	wrq.u.data.pointer = data;
	wrq.u.data.length = sizeof(data);
	wrq.u.data.flags = 0;
	if (wl_ioctl(WIF_5G, RTPRIV_IOCTL_GET_MAC_TABLE, &wrq) < 0)
		goto exit;

	/* build wireless sta list */
	mp = (RT_802_11_MAC_TABLE_5G *)wrq.u.data.pointer;
	for (i = 0; i<mp->Num; i++)
	{
		if (firstRow == 1)
			firstRow = 0;
		else
			websWrite(wp, ", ");
		websWrite(wp, "[");

		snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
				mp->Entry[i].Addr[0], mp->Entry[i].Addr[1],
				mp->Entry[i].Addr[2], mp->Entry[i].Addr[3],
				mp->Entry[i].Addr[4], mp->Entry[i].Addr[5]);
		websWrite(wp, "\"%s\"", mac);

		value = "Yes";
		websWrite(wp, ", \"%s\"", value);

		value = "";
		websWrite(wp, ", \"%s\"", value);

		websWrite(wp, "]");
	}
#endif	/* RTCONFIG_HAS_5G */

	/* error/exit */
exit:
	return 0;
}
#if defined(RTN65U)
static void convertToUpper(char *str)
{
	if(str == NULL)
		return;
	while(*str)
	{
		if(*str >= 'a' && *str <= 'z')
		{
			*str &= (unsigned char)~0x20;
		}
		str++;
	}
}
#endif
static int wl_scan(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int retval = 0, i = 0, apCount = 0;
	char data[8192];
	char ssid_str[256];
	char header[128];
	struct iwreq wrq;
	SSA *ssap;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	int lock;

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	memset(data, 0x00, 255);
	strlcpy(data, "SiteSurvey=1", sizeof(data));
	wrq.u.data.length = strlen(data)+1;
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;

	lock = file_lock("nvramcommit");
	if (wl_ioctl(nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp))), RTPRIV_IOCTL_SET, &wrq) < 0)
	{
		file_unlock(lock);
		dbg("Site Survey fails\n");
		return 0;
	}
	file_unlock(lock);
	dbg("Please wait");
	sleep(1);
	dbg(".");
	sleep(1);
	dbg(".");
	sleep(1);
	dbg(".");
	sleep(1);
	dbg(".\n\n");
	memset(data, 0, 8192);
	strlcpy(data, "", sizeof(data));
	wrq.u.data.length = 8192;
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;
	if (wl_ioctl(nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp))), RTPRIV_IOCTL_GSITESURVEY, &wrq) < 0)
	{
		dbg("errors in getting site survey result\n");
		return 0;
	}
	memset(header, 0, sizeof(header));
	//snprintf(header, sizeof(header), "%-3s%-33s%-18s%-8s%-15s%-9s%-8s%-2s\n", "Ch", "SSID", "BSSID", "Enc", "Auth", "Siganl(%)", "W-Mode", "NT");
#if 0// defined(RTN14U)
	snprintf(header, sizeof(header), "%-4s%-33s%-18s%-9s%-16s%-9s%-8s%-4s%-5s\n", "Ch", "SSID", "BSSID", "Enc", "Auth", "Siganl(%)", "W-Mode"," WPS", " DPID");
#else
	snprintf(header, sizeof(header), "%-4s%-33s%-18s%-9s%-16s%-9s%-8s\n", "Ch", "SSID", "BSSID", "Enc", "Auth", "Siganl(%)", "W-Mode");
#endif
	dbg("\n%s", header);
	if (wrq.u.data.length > 0)
	{
#if defined(RTN65U)
		if (unit == 0 && get_model() == MODEL_RTN65U)
		{
			char *encryption;
			SITE_SURVEY_RT3352_iNIC *pSsap, *ssAP;

			pSsap = ssAP = (SITE_SURVEY_RT3352_iNIC *) (1 /* '\n' */ + wrq.u.data.pointer +  sizeof(SITE_SURVEY_RT3352_iNIC) /* header */);
			while(((unsigned int)wrq.u.data.pointer + wrq.u.data.length) > (unsigned int) ssAP)
			{
				ssAP->channel   [sizeof(ssAP->channel)    -1] = '\0';
				ssAP->ssid      [32                         ] = '\0';
				ssAP->bssid     [17                         ] = '\0';
				ssAP->encryption[sizeof(ssAP->encryption) -1] = '\0';
				if((encryption = strchr(ssAP->authmode, '/')) != NULL)
				{
					memmove(ssAP->encryption, encryption +1, sizeof(ssAP->encryption) -1);
					memset(encryption, ' ', sizeof(ssAP->authmode) - (encryption - ssAP->authmode));
					*encryption = '\0';
				}
				ssAP->authmode  [sizeof(ssAP->authmode)   -1] = '\0';
				ssAP->signal    [sizeof(ssAP->signal)     -1] = '\0';
				ssAP->wmode     [sizeof(ssAP->wmode)      -1] = '\0';
				ssAP->extch     [sizeof(ssAP->extch)      -1] = '\0';
				ssAP->nt        [sizeof(ssAP->nt)         -1] = '\0';
				ssAP->wps       [sizeof(ssAP->wps)        -1] = '\0';
				ssAP->dpid      [sizeof(ssAP->dpid)       -1] = '\0';

				convertToUpper(ssAP->bssid);
				ssAP++;
				apCount++;
			}

			if (apCount)
			{
				retval += websWrite(wp, "[");
				for (i = 0; i < apCount; i++)
				{
					dbg("%-4s%-33s%-18s%-9s%-16s%-9s%-8s\n",
						pSsap[i].channel,
						pSsap[i].ssid,
						pSsap[i].bssid,
						pSsap[i].encryption,
						pSsap[i].authmode,
						pSsap[i].signal,
						pSsap[i].wmode
					);

					memset(ssid_str, 0, sizeof(ssid_str));
					char_to_ascii(ssid_str, trim_r(pSsap[i].ssid));

					if (!i)
						retval += websWrite(wp, "[\"%s\", \"%s\"]", ssid_str, pSsap[i].bssid);
					else
						retval += websWrite(wp, ", [\"%s\", \"%s\"]", ssid_str, pSsap[i].bssid);
				}
				retval += websWrite(wp, "]");
				dbg("\n");
			}
			else
				retval += websWrite(wp, "[]");
			return retval;
		}
#endif
		ssap=(SSA *)(wrq.u.data.pointer+strlen(header)+1);
		int len = strlen(wrq.u.data.pointer+strlen(header))-1;
		char *sp, *op;
 		op = sp = wrq.u.data.pointer+strlen(header)+1;
		while (*sp && ((len - (sp-op)) >= 0))
		{
			ssap->SiteSurvey[i].channel[3] = '\0';
			ssap->SiteSurvey[i].ssid[32] = '\0';
			ssap->SiteSurvey[i].bssid[17] = '\0';
			ssap->SiteSurvey[i].encryption[8] = '\0';
			ssap->SiteSurvey[i].authmode[15] = '\0';
			ssap->SiteSurvey[i].signal[8] = '\0';
			ssap->SiteSurvey[i].wmode[7] = '\0';
#if 0//defined(RTN14U)
			ssap->SiteSurvey[i].wps[3] = '\0';
			ssap->SiteSurvey[i].dpid[4] = '\0';
#endif
			sp+=strlen(header);
			apCount=++i;
		}
		if (apCount)
		{
			retval += websWrite(wp, "[");
			for (i = 0; i < apCount; i++)
			{
			   	dbg("\napCount=%d\n",i);
				dbg(
#if 0//defined(RTN14U)
				"%-4s%-33s%-18s%-9s%-16s%-9s%-8s%-4s%-5s\n",
#else
				"%-4s%-33s%-18s%-9s%-16s%-9s%-8s\n",
#endif
					ssap->SiteSurvey[i].channel,
					(char*)ssap->SiteSurvey[i].ssid,
					ssap->SiteSurvey[i].bssid,
					ssap->SiteSurvey[i].encryption,
					ssap->SiteSurvey[i].authmode,
					ssap->SiteSurvey[i].signal,
					ssap->SiteSurvey[i].wmode
#if 0//defined(RTN14U)
					, ssap->SiteSurvey[i].wps
					, ssap->SiteSurvey[i].dpid
#endif
				);

				memset(ssid_str, 0, sizeof(ssid_str));
				char_to_ascii(ssid_str, trim_r(ssap->SiteSurvey[i].ssid));

				if (!i)
//					retval += websWrite(wp, "\"%s\"", ssap->SiteSurvey[i].bssid);
					retval += websWrite(wp, "[\"%s\", \"%s\"]", ssid_str, ssap->SiteSurvey[i].bssid);
				else
//					retval += websWrite(wp, ", \"%s\"", ssap->SiteSurvey[i].bssid);
					retval += websWrite(wp, ", [\"%s\", \"%s\"]", ssid_str, ssap->SiteSurvey[i].bssid);
			}
			retval += websWrite(wp, "]");
			dbg("\n");
		}
		else
			retval += websWrite(wp, "[]");
	}
	return retval;
}

int
ej_wl_scan(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_scan(eid, wp, argc, argv, 0);
}

int
ej_wl_scan_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_scan(eid, wp, argc, argv, 0);
}

int
ej_wl_scan_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_scan(eid, wp, argc, argv, 1);
}

const char *
get_wifname(int band)
{
	if (band)
		return WIF_5G;
	else
		return WIF_2G;
}
int
getChannel(int band)
{
	int channel;
	struct iw_range	range;
	double freq;
	struct iwreq wrq1;
	struct iwreq wrq2;
	if (wl_ioctl(get_wifname(band), SIOCGIWFREQ, &wrq1) < 0)
		return 0;
	char buffer[sizeof(iwrange) * 2];
	bzero(buffer, sizeof(buffer));
	wrq2.u.data.pointer = (caddr_t) buffer;
	wrq2.u.data.length = sizeof(buffer);
	wrq2.u.data.flags = 0;

	if (wl_ioctl(get_wifname(band), SIOCGIWRANGE, &wrq2) < 0)
		return 0;

	if (ralink_get_range_info(&range, buffer, wrq2.u.data.length) < 0)
		return 0;

	freq = iw_freq2float(&(wrq1.u.freq));
	if (freq < KILO)
		channel = (int) freq;
	else
	{
		channel = iw_freq_to_channel(freq, &range);
		if (channel < 0)
			return 0;
	}
	return channel;
}
int
ej_wl_control_channel(int eid, webs_t wp, int argc, char_t **argv)
{
	int ret = 0;
	int channel_24 = 0, channel_50 = 0;
	if (!(channel_24 = getChannel(0)))
	{
		ret = websWrite(wp, "[\"0\"]");
		return ret;
	}
	if (!(channel_50 = getChannel(1)))
		ret = websWrite(wp, "[\"%d\", \"%d\"]", channel_24, 0);
	else
		ret = websWrite(wp, "[\"%d\", \"%d\"]", channel_24, channel_50);

	return ret;
}


static int ej_wl_channel_list(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int retval = 0;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *country_code;
	char chList[256];
	int band;

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	country_code = nvram_get(strlcat_r(prefix, "country_code", tmp, sizeof(tmp)));
	band = unit;

	if (country_code == NULL || strlen(country_code) != 2) return retval;

	if (band != 0 && band != 1) return retval;

#if defined(RTAC85U) || defined(RTAC85P) || defined(RTACRH26) || defined(TUFAC1750)
	if (!nvram_get_int("wlready")) return retval;
#endif

	//try getting channel list via wifi driver first
	if(get_channel_list_via_driver(unit, chList, sizeof(chList)) > 0)
	{
		retval += websWrite(wp, "[%s]", chList);
	}
	else if(get_channel_list_via_country(unit, country_code, chList, sizeof(chList)) > 0)
	{
		retval += websWrite(wp, "[%s]", chList);
	}
	return retval;
}


int
ej_wl_channel_list_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_channel_list(eid, wp, argc, argv, 0);
}

int
ej_wl_channel_list_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_channel_list(eid, wp, argc, argv, 1);
}

int
ej_wl_channel_list_5g_2(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_channel_list(eid, wp, argc, argv, 2);
}

int
ej_wl_channel_list_60g(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_channel_list(eid, wp, argc, argv, 3);
}


static int ej_wl_rate(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	struct iwreq wrq;
	int retval = 0;
	char tmp[256], prefix[] = "wlXXXXXXXXXX_";
	char *name;
	char word[256], *next;
	int rate=0;
	int status;
	char rate_buf[32];
	int sw_mode = sw_mode();
	int wlc_band = nvram_get_int("wlc_band");
	int from_app = 0;

	from_app = check_user_agent(user_agent);

#ifdef RTCONFIG_CONCURRENTREPEATER
	strlcpy(rate_buf, "auto", sizeof(rate_buf));
#else
	strlcpy(rate_buf, "0 Mbps", sizeof(rate_buf));	
#endif

	if (absent_band(unit))
		goto ERROR;
#ifdef RTCONFIG_CONCURRENTREPEATER
	if (sw_mode == SW_MODE_REPEATER || sw_mode == SW_MODE_HOTSPOT)
#else	
	if (wlc_band == unit && (sw_mode == SW_MODE_REPEATER || sw_mode == SW_MODE_HOTSPOT))
#endif		
		snprintf(prefix, sizeof(prefix), "wl%d.1_", unit);
	else
#if 0
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	name = nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)));

	wrq.u.bitrate.value=-1;
	if (wl_ioctl(name, SIOCGIWRATE, &wrq))
	{
		dbG("can not get rate info of %s\n", name);
		goto ERROR;
	}

	rate = wrq.u.bitrate.value;
	if ((rate == -1) || (rate == 0))
		strlcpy(rate_buf, "auto", sizeof(rate_buf));
	else
		snprintf(rate_buf, sizeof(rate_buf), "%d Mbps", (rate / 1000000));
#else
		goto ERROR;
	name = nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)));

	memset(tmp, 0x00, sizeof(tmp));
	wrq.u.data.length = sizeof(tmp);
	wrq.u.data.pointer = (caddr_t) tmp;
	wrq.u.data.flags = ASUS_SUBCMD_CONN_STATUS;

	if (wl_ioctl(name, RTPRIV_IOCTL_ASUSCMD, &wrq) < 0)
	{
		dbg("%s: errors in getting %s CONN_STATUS result\n", __func__, name);
		goto ERROR;
	}
	status = ((int*)tmp)[0];
	rate   = ((int*)tmp)[1];

	if(status == 6)
		snprintf(rate_buf, sizeof(rate_buf), "%d Mbps", rate);
#endif

ERROR:
	if(from_app == 0)
		retval += websWrite(wp, "%s", rate_buf);
	else
		retval += websWrite(wp, "\"%s\"", rate_buf);
	return retval;
}


int
ej_wl_rate_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	if(sw_mode() == SW_MODE_REPEATER)
		return ej_wl_rate(eid, wp, argc, argv, 0);
	else
	   	return 0;
}

int
ej_wl_rate_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	if(sw_mode() == SW_MODE_REPEATER)
		return ej_wl_rate(eid, wp, argc, argv, 1);
	else
	   	return 0;
}

int
ej_wl_rate_5g_2(int eid, webs_t wp, int argc, char_t **argv)
{
	if(sw_mode() == SW_MODE_REPEATER)
		return ej_wl_rate(eid, wp, argc, argv, 2);
	else
		return 0;
}

int
ej_wl_rate_6g(int eid, webs_t wp, int argc, char_t **argv)
{
	if(sw_mode() == SW_MODE_REPEATER)
		return ej_wl_rate(eid, wp, argc, argv, 3);
	else
		return 0;
}

int
ej_nat_accel_status(int eid, webs_t wp, int argc, char_t **argv)
{
	int retval = 0;

	retval += websWrite(wp, "%d", module_loaded("hw_nat"));

	return retval;
}

#if defined(RTCONFIG_MT798X) || defined(RTCONFIG_WLMODULE_MT7915D_AP)
/* Hook validate_apply().
 * Sync wl[0~2]_yyy with wlx_yyy if yyy in global_params[].
 */
static const char *global_params[] = { "atf", "plcphdr", "frameburst", "PktAggregate", "wme_no_ack", "mumimo", NULL };
void __validate_apply_set_wl_var(char *nv, char *val)
{
	const char **p;
	int band;
	char prefix[sizeof("wlxxx_")];

	if (!nv || (strncmp(nv, "wl0_", 4) && strncmp(nv, "wl1_", 4) && strncmp(nv, "wl2_", 4)))
		return;

	for (p = &global_params[0]; *p != NULL; ++p) {
		if (strcmp(nv + 4, *p))
			continue;

		for (band = WL_2G_BAND; band < min(MAX_NR_WL_IF, WL_5G_2_BAND + 1); ++band) {
			snprintf(prefix, sizeof(prefix), "wl%d_", band);
			if (!strncmp(nv, prefix, strlen(prefix)))
				continue;
			nvram_pf_set(prefix, *p, val);
			_dprintf("%s: set %s%s=%s\n", __func__, prefix, *p, val? : "NULL");
		}
		break;
	}
}
#endif

#ifdef RTCONFIG_PROXYSTA
int
ej_wl_auth_psta(int eid, webs_t wp, int argc, char_t **argv)
{
	int retval = 0;
	int psta = 0, psta_auth = 0;

	if(nvram_match("wlc_state", "2")){	//connected
		psta = 1;
		psta_auth = 0;
	//else if(?)				//authorization failed
	//	retval += websWrite(wp, "wlc_state=2;wlc_state_auth=1;");
	}else{					//disconnected
		psta = 0;
		psta_auth = 0;
	}

	if(json_support){
		retval += websWrite(wp, "{");
		retval += websWrite(wp, "\"wlc_state\":\"%d\"", psta);
		retval += websWrite(wp, ",\"wlc_state_auth\":\"%d\"", psta_auth);
		retval += websWrite(wp, "}");
	}else{
		retval += websWrite(wp, "wlc_state=%d;", psta);
		retval += websWrite(wp, "wlc_state_auth=%d;", psta_auth);
	}

	return retval;
}
#endif
