#ifndef __FUNCTION_H__
#define __FUNCTION_H__

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include "networkmap.h"

int mac_str_to_bytes(const char *mac_str, unsigned char mac[6]);

int memcpy_safe(void *dst, size_t dst_size, const void *src, size_t copy_len);

int FindHostname(P_CLIENT_DETAIL_INFO_TABLE p_client_detail_info_tab, int i);

int FindDevice(unsigned char *pIP, unsigned char *pMac, int replaceMac);

static inline void format_conn_time(uint64_t total_sec, char *dest, size_t dest_sz) {
	if (!dest || dest_sz == 0) return;
	unsigned int hr = (unsigned int)(total_sec / 3600);
	unsigned int min = (unsigned int)((total_sec % 3600) / 60);
	unsigned int sec = (unsigned int)(total_sec % 60);
	snprintf(dest, dest_sz, "%02u:%02u:%02u", hr, min, sec);
}

#endif
