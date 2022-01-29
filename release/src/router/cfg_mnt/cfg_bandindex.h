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

#define BANDINDEX_VERSION	BIT(0)

extern void cm_addBandIndex(json_object *outRoot);

#endif /* __CFG_BANDINDEX_H__ */
/* End of cfg_bandindex.h */


