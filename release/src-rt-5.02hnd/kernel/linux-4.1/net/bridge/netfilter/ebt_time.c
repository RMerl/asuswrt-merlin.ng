#if defined(CONFIG_BCM_KF_NETFILTER)

/*
*    Copyright (c) 2003-2012 Broadcom Corporation
*    All Rights Reserved
*
<:label-BRCM:2012:GPL/GPL:standard

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

/*
  Description: EBTables time match extension kernelspace module.
  Authors:  Song Wang <songw@broadcom.com>, ported from netfilter/iptables
            The following is the original disclaimer.

  This is a module which is used for time matching
  It is using some modified code from dietlibc (localtime() function)
  that you can find at http://www.fefe.de/dietlibc/
  This file is distributed under the terms of the GNU General Public
  License (GPL). Copies of the GPL can be obtained from: ftp://prep.ai.mit.edu/pub/gnu/GPL
  2001-05-04 Fabrice MARIE <fabrice@netfilter.org> : initial development.
  2001-21-05 Fabrice MARIE <fabrice@netfilter.org> : bug fix in the match code,
     thanks to "Zeng Yu" <zengy@capitel.com.cn> for bug report.
  2001-26-09 Fabrice MARIE <fabrice@netfilter.org> : force the match to be in LOCAL_IN or PRE_ROUTING only.
  2001-30-11 Fabrice : added the possibility to use the match in FORWARD/OUTPUT with a little hack,
     added Nguyen Dang Phuoc Dong <dongnd@tlnet.com.vn> patch to support timezones.
*/

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_time.h>
#include <linux/time.h>

//static unsigned char debug;
//MODULE_PARM(debug, "0-1b");
static int debug;
module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "debug=1 is turn on debug messages");
MODULE_AUTHOR("Song Wang <songw@broadcom.com>");
MODULE_DESCRIPTION("Match timestamp");
MODULE_LICENSE("GPL");

#define DEBUG_MSG(...) if (debug) printk (KERN_DEBUG "ebt_time: " __VA_ARGS__)

void localtime(const time_t *timepr, struct tm *r);
bool (*match)(const struct sk_buff *skb, struct xt_action_param *);

static bool ebt_time_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	//const struct ebt_time_info *info = (struct ebt_time_info *)data;   /* match info for rule */
	const struct ebt_time_info *info = par->matchinfo;
	struct tm currenttime;                          /* time human readable */
	u_int8_t days_of_week[7] = {64, 32, 16, 8, 4, 2, 1};
	u_int16_t packet_time;
	struct timeval kerneltimeval;
	time_t packet_local_time;

	/* if kerneltime=1, we don't read the skb->timestamp but kernel time instead */
	if (info->kerneltime)
	{
		do_gettimeofday(&kerneltimeval);
		packet_local_time = kerneltimeval.tv_sec;
	}
	else
	{
		struct timespec ts;
		ts = ktime_to_timespec(skb->tstamp);
		packet_local_time = ts.tv_sec;
	}

	/* Transform the timestamp of the packet, in a human readable form */
	localtime(&packet_local_time, &currenttime);
	DEBUG_MSG("currenttime: Y-%ld M-%d D-%d H-%d M-%d S-%d, Day: W-%d\n",
		currenttime.tm_year, currenttime.tm_mon, currenttime.tm_mday,
		currenttime.tm_hour, currenttime.tm_min, currenttime.tm_sec,
		currenttime.tm_wday);

	/* check if we match this timestamp, we start by the days... */
	if (info->days_match != 0) {
		if ((days_of_week[currenttime.tm_wday] & info->days_match) != days_of_week[currenttime.tm_wday])
		{
			DEBUG_MSG("the day doesn't match\n");
			return false; /* the day doesn't match */
		}
	}
	/* ... check the time now */
	packet_time = (currenttime.tm_hour * 60) + currenttime.tm_min;
	if ((packet_time < info->time_start) || (packet_time > info->time_stop))
	{
		DEBUG_MSG("the time doesn't match\n");
		return false;
	}
	
	/* here we match ! */
	DEBUG_MSG("the time match!!!!!!!!\n");
	return true;
}

static int ebt_time_mt_check(const struct xt_mtchk_param *par)
{
	//struct ebt_time_info *info = (struct ebt_time_info *)data;   /* match info for rule */
	struct ebt_time_info *info = par->matchinfo;

	/* First, check that we are in the correct hook */
	/* PRE_ROUTING, LOCAL_IN or FROWARD */
#if 0
	if (hookmask
            & ~((1 << NF_BR_PRE_ROUTING) | (1 << NF_BR_LOCAL_IN) | (1 << NF_BR_FORWARD) | (1 << NF_BR_LOCAL_OUT)))
	{
		printk("ebt_time: error, only valid for PRE_ROUTING, LOCAL_IN, FORWARD and OUTPUT)\n");
		return -EINVAL;
	}
#endif
	/* we use the kerneltime if we are in forward or output */
	info->kerneltime = 1;
#if 0
	if (hookmask & ~((1 << NF_BR_FORWARD) | (1 << NF_BR_LOCAL_OUT))) 
		/* if not, we use the skb time */
		info->kerneltime = 0;
#endif

	/* Check the size */
	//if (datalen != sizeof(struct ebt_time_info))
	//	return -EINVAL;
	/* Now check the coherence of the data ... */
	if ((info->time_start > 1439) ||        /* 23*60+59 = 1439*/
	    (info->time_stop  > 1439))
	{
		printk(KERN_WARNING "ebt_time: invalid argument\n");
		return -EINVAL;
	}

	return 0;
}

static struct xt_match ebt_time_mt_reg = {
	.name		= EBT_TIME_MATCH,
	.revision	= 0,
	.family		= NFPROTO_BRIDGE,
	.match		= ebt_time_mt,
	.checkentry	= ebt_time_mt_check,
	.matchsize	= XT_ALIGN(sizeof(struct ebt_time_info)),
	.me		= THIS_MODULE,
};

static int __init ebt_time_init(void)
{
	int ret;
	ret = xt_register_match(&ebt_time_mt_reg);

	if(ret == 0)
		printk(KERN_INFO "ebt_time registered\n");

	return ret;
}

static void __exit ebt_time_fini(void)
{
	xt_unregister_match(&ebt_time_mt_reg);
}



module_init(ebt_time_init);
module_exit(ebt_time_fini);


/* The part below is borowed and modified from dietlibc */

/* seconds per day */
#define SPD 24*60*60

void localtime(const time_t *timepr, struct tm *r) {
	time_t i;
	time_t timep;
	extern struct timezone sys_tz;
	const unsigned int __spm[12] =
		{ 0,
		  (31),
		  (31+28),
		  (31+28+31),
		  (31+28+31+30),
		  (31+28+31+30+31),
		  (31+28+31+30+31+30),
		  (31+28+31+30+31+30+31),
		  (31+28+31+30+31+30+31+31),
		  (31+28+31+30+31+30+31+31+30),
		  (31+28+31+30+31+30+31+31+30+31),
		  (31+28+31+30+31+30+31+31+30+31+30),
		};
	register time_t work;

	timep = (*timepr) - (sys_tz.tz_minuteswest * 60);
	work=timep%(SPD);
	r->tm_sec=work%60; work/=60;
	r->tm_min=work%60; r->tm_hour=work/60;
	work=timep/(SPD);
	r->tm_wday=(4+work)%7;
	for (i=1970; ; ++i) {
		register time_t k= (!(i%4) && ((i%100) || !(i%400)))?366:365;
		if (work>k)
			work-=k;
		else
			break;
	}
	r->tm_year=i-1900;
	for (i=11; i && __spm[i]>work; --i) ;
	r->tm_mon=i;
	r->tm_mday=work-__spm[i]+1;
}
#endif