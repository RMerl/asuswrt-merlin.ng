#ifndef __CFG_LIB_H__
#define __CFG_LIB_H__

#include <rtconfig.h>

extern int igr_wlioctl;

#define MAX_2G_CHANNEL_LIST_NUM	16
#define MAX_5G_CHANNEL_LIST_NUM	32
#if defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_WIFI7)
#define MAX_6G_CHANNEL_LIST_NUM	64
#endif

typedef struct _avbl_chanspec_t
{
	unsigned int bw2g;
	unsigned int bw5g;
	unsigned int bw6g;
	unsigned int channelList2g[MAX_2G_CHANNEL_LIST_NUM];
	unsigned int channelList5g[MAX_5G_CHANNEL_LIST_NUM];
#if defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_WIFI7)
	unsigned int channelList6g[MAX_6G_CHANNEL_LIST_NUM];
#endif
	unsigned int existTribandRe;
	unsigned int existDual5gRe;
}AVBL_CHANSPEC_T;

extern int send_cfgmnt_event(char *msg);
extern int get_chanspec_info(AVBL_CHANSPEC_T *avblChannel);
extern int send_event_to_roamast(char *data);
#if defined(RTCONFIG_NOWL)
extern int is_5g_high_band(int unit);
#endif
extern char *get_rebandtype_chanspc_by_unit (char *mac, int unit ,int reBandNum , char *rebandtype, int rebandtypeLen);
extern char *cap_get_final_paramname(char *mac, char *input_param,int reBandNum , char *finalparamname, int finalparamnamelen);

#ifdef RTCONFIG_ROUTERBOOST
//extern int cm_sendEventToAsusRbd(unsigned char *data, int len);
extern int send_event_to_asusrbd(unsigned char *data, int len);
#endif //RTCONFIG_ROUTERBOOST

extern int cm_isCapSupported(char *reMac, int capType, int capSubtype);

#endif /* __CFG_LIB_H__ */
/* End of cfg_lib.h */
