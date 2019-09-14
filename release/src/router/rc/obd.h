#ifndef __OBD_H__
#define __OBD_H__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/time.h>
//#include <unistd.h>
//#include <time.h>
//#include <bcmnvram.h>
//#include <bcmutils.h>
//#include <wlutils.h>
//#include <shutils.h>
#include <shared.h>
#include <proto/ethernet.h>
#include <rc.h>

#if defined(RTCONFIG_AMAS)

//#define OUI_LEN 3
//#define VS_ID 221

#if 1
#define ETHER_ADDRESS_LEN 6
#if defined(RTCONFIG_LANTIQ) || defined(RTCONFIG_QCA) || defined(RTCONFIG_REALTEK)
#define OUI_LEN 3
#define VS_ID 221
#else
#define OUI_LEN DOT11_OUI_LEN
#define VS_ID DOT11_MNG_VS_ID
#endif
#endif

#define NORMAL_PERIOD		1		/* second */
#define RUSHURGENT_PERIOD	50 * 1000	/* microsecond */
#define NVRAM_BUFSIZE   100

#define WPS_TIMEOUT			240000000 /* microsecond (240 seconds) */
#define OBD_TIMEOUT			300

/* Debug Print */
#define OBD_DEBUG_ERROR		0x000001
#define OBD_DEBUG_WARNING	0x000002
#define OBD_DEBUG_INFO		0x000004
#define OBD_DEBUG_EVENT		0x000008
#define OBD_DEBUG_DETAIL	0x000010

#define OBD_DEBUG "/tmp/OBD_DEBUG"
#define NEW_RSSI_INFO	1

static int msglevel = 0;//OBD_DEBUG_ERROR | OBD_DEBUG_INFO | OBD_DEBUG_EVENT | OBD_DEBUG_DETAIL;

#define OBD_ERROR(fmt, arg...) \
	do { if ((msglevel & OBD_DEBUG_ERROR) || f_exists(OBD_DEBUG) > 0) \
		dbg("OBD %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define OBD_WARNING(fmt, arg...) \
	do { if ((msglevel & OBD_DEBUG_WARNING) || f_exists(OBD_DEBUG) > 0) \
		dbg("OBD %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define OBD_INFO(fmt, arg...) \
	do { if ((msglevel & OBD_DEBUG_INFO) || f_exists(OBD_DEBUG) > 0) \
		dbg("OBD %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define OBD_EVENT(fmt, arg...) \
	do { if ((msglevel & OBD_DEBUG_EVENT) || f_exists(OBD_DEBUG) > 0) \
		dbg("OBD %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define OBD_DBG(fmt, arg...) \
	do { if ((msglevel & OBD_DEBUG_DETAIL) || f_exists(OBD_DEBUG) > 0) \
		dbg("OBD %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define OBD_PRINT(fmt, arg...) \
	do { dbg("OBD %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define MAX_VSIE_LEN 1024

/* Use to store scanned bss which contains vsie information. */
struct scanned_bss {
	struct scanned_bss *next;
	int idx;
	uint8 channel;
	struct ether_addr BSSID;
	uint8 vsie_len;
	uint8 vsie[MAX_VSIE_LEN];
	unsigned char RSSI;
};


// sysdeps prototype
int obd_init();
void obd_final();
void obd_save_para();
void obd_start_active_scan();
struct scanned_bss *obd_get_bss_scan_result();
void obd_start_wps_enrollee();
void obd_add_probe_req_vsie(int unit, int len, unsigned char *ie_data);
void obd_del_probe_req_vsie(int unit, int len, unsigned char *ie_data);

void obd_led_blink();
void obd_led_off();

#endif //#if defined(RTCONFIG_AMAS)

#endif
