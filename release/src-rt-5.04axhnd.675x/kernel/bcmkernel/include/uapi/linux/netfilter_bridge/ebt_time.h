#ifndef __LINUX_BRIDGE_EBT_TIME_H
#define __LINUX_BRIDGE_EBT_TIME_H


struct ebt_time_info {
	u_int8_t  days_match;   /* 1 bit per day. -SMTWTFS                      */
	u_int16_t time_start;   /* 0 < time_start < 23*60+59 = 1439             */
	u_int16_t time_stop;    /* 0:0 < time_stat < 23:59                      */
	u_int8_t  kerneltime;   /* ignore skb time (and use kerneltime) or not. */
};

#define EBT_TIME_MATCH "time"

#endif /* __LINUX_BRIDGE_EBT_TIME_H */
