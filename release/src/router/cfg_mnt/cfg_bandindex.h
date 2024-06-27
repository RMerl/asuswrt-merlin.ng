#ifndef __CFG_BANDINDEX_H__
#define __CFG_BANDINDEX_H__

#if defined(BIT)
#undef BIT
#endif
#define BIT(x)	((1 << x))

/* for WIFI_BAND_TYPE */
#define HAVE_WIFI_2G		BIT(0)
#define HAVE_WIFI_5G		BIT(1)
#define HAVE_WIFI_5GL		BIT(2)
#define HAVE_WIFI_5GH		BIT(3)
#define HAVE_WIFI_6G		BIT(4)
#define HAVE_WIFI_6GL           BIT(5)
#define HAVE_WIFI_6GH           BIT(6)

#define BANDINDEX_VERSION	BIT(0)

enum bandAttribute {
	BAND_ATTR_2G = 0,
	BAND_ATTR_5G = 1,
	BAND_ATTR_5GL = 2,
	BAND_ATTR_5GH = 3,
	BAND_ATTR_6G = 4,
    	BAND_ATTR_6GL = 5,
    	BAND_ATTR_6GH = 6
};

extern void cm_addBandIndex(json_object *outRoot);
extern void cm_updateBandInfoByMac(char *mac, json_object *chanspecObj);

#endif /* __CFG_BANDINDEX_H__ */
/* End of cfg_bandindex.h */


