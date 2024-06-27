#ifndef __CFG_LIB_H__
#define __CFG_LIB_H__

#define MAX_2G_CHANNEL_LIST_NUM	16
#define MAX_5G_CHANNEL_LIST_NUM	32
#ifdef RTCONFIG_WIFI6E
#define MAX_6G_CHANNEL_LIST_NUM	64
#endif

typedef struct _avbl_chanspec_t
{
	unsigned int bw2g;
	unsigned int bw5g;
	unsigned int bw6g;
	unsigned int channelList2g[MAX_2G_CHANNEL_LIST_NUM];
	unsigned int channelList5g[MAX_5G_CHANNEL_LIST_NUM];
#ifdef RTCONFIG_WIFI6E
	unsigned int channelList6g[MAX_6G_CHANNEL_LIST_NUM];
#endif
	unsigned int existTribandRe;
	unsigned int existDual5gRe;
	unsigned int existDual6gRe;
}AVBL_CHANSPEC_T;

extern int send_cfgmnt_event(char *msg);
extern int get_chanspec_info(AVBL_CHANSPEC_T *avblChannel);
extern int send_event_to_roamast(char *data);

#endif /* __CFG_LIB_H__ */
/* End of cfg_lib.h */
