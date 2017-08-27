#define RAST_COUNT_IDLE 15
#define RAST_COUNT_RSSI 3
#define RAST_POLL_INTV_NORMAL 5
#if defined(RTCONFIG_RALINK)  /* Remove dead STA from assoclist */
#define RAST_TIMEOUT_STA 10
#define RAST_MTK_DATARATE 128 /* 1024=1k, 8bit=1Byte */
#else
#define RAST_TIMEOUT_STA 10
#endif
#define RAST_DFT_IDLE_RATE 	20 	/* Kbps */
#define	MAX_IF_NUM 3
#define MAX_SUBIF_NUM 4
#define MAX_STA_COUNT 128
#define ETHER_ADDR_STR_LEN 18

#define RAST_INFO(fmt, arg...) \
	do {	\
		_dprintf("RAST %lu: "fmt, uptime(), ##arg); \
	} while (0)
#define RAST_DBG(fmt, arg...) \
        do {    \
               if(rast_dbg) \
                _dprintf("RAST %lu: "fmt, uptime(), ##arg); \
        } while (0)

#if defined(RTCONFIG_RALINK)
#elif defined(RTCONFIG_QCA)
#define MACF	"%02x:%02x:%02x:%02x:%02x:%02x"
#define ETHERP_TO_MACF(ea)	((struct ether_addr *) (ea))->ether_addr_octet[0], \
				((struct ether_addr *) (ea))->ether_addr_octet[1], \
				((struct ether_addr *) (ea))->ether_addr_octet[2], \
				((struct ether_addr *) (ea))->ether_addr_octet[3], \
				((struct ether_addr *) (ea))->ether_addr_octet[4], \
				((struct ether_addr *) (ea))->ether_addr_octet[5]
#define ETHER_TO_MACF(ea)	(ea).ether_addr_octet[0], \
				(ea).ether_addr_octet[1], \
				(ea).ether_addr_octet[2], \
				(ea).ether_addr_octet[3], \
				(ea).ether_addr_octet[4], \
				(ea).ether_addr_octet[5]
#else
#define MACF	"%02x:%02x:%02x:%02x:%02x:%02x"
#define ETHERP_TO_MACF(ea)	((struct ether_addr *) (ea))->octet[0], \
				((struct ether_addr *) (ea))->octet[1], \
				((struct ether_addr *) (ea))->octet[2], \
				((struct ether_addr *) (ea))->octet[3], \
				((struct ether_addr *) (ea))->octet[4], \
				((struct ether_addr *) (ea))->octet[5]
#define ETHER_TO_MACF(ea)	(ea).octet[0], \
				(ea).octet[1], \
				(ea).octet[2], \
				(ea).octet[3], \
				(ea).octet[4], \
				(ea).octet[5]
#endif

#if defined(RTCONFIG_RALINK)
#define xR_MAX  4
extern int xTxR;
#elif defined(RTCONFIG_QCA)
typedef struct _WLANCONFIG_LIST {
         char addr[18];
         unsigned int aid;
         unsigned int chan;
         char txrate[6];
         char rxrate[6];
         unsigned int rssi;
         unsigned int idle;
         unsigned int txseq;
         unsigned int rcseq;
         char caps[12];
         char acaps[10];
         char erp[7];
         char state_maxrate[20];
         char wps[4];
         char rsn[4];
         char wme[4];
         char mode[31];
} WLANCONFIG_LIST;
#endif

typedef struct rast_sta_info {
	time_t timestamp;
	time_t active;
	struct ether_addr addr;
	struct rast_sta_info *prev, *next;
	float datarate;	/* Kbps */
	time_t idle_start;
	uint8 idle_state;
	int32 rssi_hit_count;
	int32 rssi;
#if defined(RTCONFIG_RALINK)
	int32 rssi_xR[xR_MAX];
	int32 last_txrx_bytes;  // Kbps
	char mac_addr[18];
#elif defined(RTCONFIG_QCA)
	int32 last_txrx_bytes; /* bytes */
#elif defined(RTCONFIG_REALTEK)
	unsigned long long last_txrx_bytes;
#else
#endif
}rast_sta_info_t;

typedef struct rast_bss_info {
	char wlif_name[32];
	char prefix[32];
	int user_low_rssi;
	int32 idle_rate;
	rast_sta_info_t *assoclist[MAX_SUBIF_NUM];
	int upstream_if;
}rast_bss_info_t;

rast_bss_info_t bssinfo[3];


extern int rast_dbg;


extern rast_sta_info_t *rast_add_to_assoclist(int bssidx, int vifidx, struct ether_addr *addr);

#if defined(RTCONFIG_RALINK)
extern int check_rssi_threshold(int bssidx, int vifidx);
extern void get_stainfo(int bssidx, int vifidx);
#endif

