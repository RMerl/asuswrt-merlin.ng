#ifndef __CFG_EVENTNOTIFY_H__
#define __CFG_EVENTNOTIFY_H__

extern void cm_updateStaMacToAmasLib(unsigned char *decodeMsg);
#ifdef RTCONFIG_NOTIFICATION_CENTER
extern void cm_forwardWifiEventToNtCenter(int event, char *mac, char *band);
extern void cm_forwardEthEventToNtCenter(int event, char *mac);
enum eventType {
	WIFI_DEVICE_ONLINE = 1,
	WIFI_DEVICE_OFFLINE,
	ETH_DEVICE_ONLINE,
	ETH_DEVICE_OFFLINE
};
#endif

#endif /* __CFG_EVENTNOTIFY_H__ */
/* End of cfg_eventnotify.h */
