#ifndef _LINUX_NETFILTER_XT_PSD_H
#define _LINUX_NETFILTER_XT_PSD_H 1

#include <linux/param.h>
#include <linux/types.h>

/*
 * High port numbers have a lower weight to reduce the frequency of false
 * positives, such as from passive mode FTP transfers.
 */
#define PORT_WEIGHT_PRIV		3
#define PORT_WEIGHT_HIGH		1
#define	PSD_MAX_RATE			10000

/*
 * Port scan detection thresholds: at least COUNT ports need to be scanned
 * from the same source, with no longer than DELAY ticks between ports.
 */
#define SCAN_MIN_COUNT			7
#define SCAN_MAX_COUNT			(SCAN_MIN_COUNT * PORT_WEIGHT_PRIV)
#define SCAN_WEIGHT_THRESHOLD		SCAN_MAX_COUNT

struct xt_psd_info {
	__u32 weight_threshold;
	__u32 delay_threshold;
	__u16 lo_ports_weight;
	__u16 hi_ports_weight;
};

#endif /*_LINUX_NETFILTER_XT_PSD_H*/
