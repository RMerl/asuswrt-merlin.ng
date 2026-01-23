#ifndef __TCODE_H__
#define __TCODE_H__

#ifdef RTCONFIG_TCODE
struct tcode_nvram_s {
	int model;
	char *odmpid;
	char *tcode;
	char *name;
	char *value;
#ifdef RTAC68U
	unsigned int flag;	/* hardware compatibility flag */
#endif
#if defined(RTAX82U) || defined(RTAX82U_V2)  || defined(GT10) || defined(GTAX11000_PRO)
	unsigned cobrand;
#endif
};

struct location_nvram_s {
	int model;
	char *odmpid;
	char *location_code;
	char *name;
	char *value;
};

struct tcode_rc_support_s {
	int model;
	char *tcode;
	char *features;
#ifdef RTAC68U
	unsigned int flag;	/* hardware compatibility flag */
#endif
};

struct tcode_rc_support_by_odmpid_s {
	int model;
	char *odmpid;
	char *tcode;
	char *features;
};

struct tcode_location_s {
	int model;
	char *location;
	char *prefix_fmt;
	int idx_base;
	char *ccode_2g;
	char *regrev_2g;	/* non-BRCM model, this maybe NULL */
	char *ccode_5g;		/* For 2.4G model, this should be NULL */
	char *regrev_5g;	/* For 2.4G model, this should be NULL */
	char *ccode_5g_2;	/* For 2.4G model, this should be NULL */
	char *regrev_5g_2;	/* For 2.4G model, this should be NULL */
#ifdef RTAC68U
	unsigned int flag;	/* hardware compatibility flag */
#endif
#if defined(GTAXE16000) || defined(GTBE98) || defined(GTBE98_PRO) || defined(BQ16) || defined(BQ16_PRO)
	char *ccode_6g;
	char *regrev_6g;
#endif
};

struct tcode_lang_s {
	int model;
	char *odmpid;
	char *tcode;
	char *support_lang;  /* support language list */
	int auto_change;
};

struct tcode_langcode_s {
	int model;
	char *tcode;
	char *lang_list;	/* language list */
	char *location;		/* default location */
};

#ifdef RTAC68U
#define RTAC68U_V1	0x00000001
#define RTAC68U_V2	0x00000002
#define RTAC68U_V1_C0	0x00000004
#define RTAC68U_V2_C0	0x00000008
#define RTAC66U_V2	0x00000010
#define RTAC68U_ALL	(RTAC68U_V1 | RTAC68U_V2 | RTAC68U_V1_C0 | RTAC68U_V2_C0 | RTAC66U_V2 | RTAC68U_V3_C0)
#define RTAC68U_V1_ALL	(RTAC68U_V1 | RTAC68U_V1_C0 | RTAC68U_V3_C0)
#define RTAC68U_V2_ALL	(RTAC68U_V2 | RTAC68U_V2_C0)
#define RT4GAC68U_V1_C0	0x00000020
#define RTAC68U_V3_C0	0x00000040
#endif

extern struct tcode_nvram_s tcode_nvram_list[];
extern struct tcode_nvram_s tcode_init_nvram_list[];
extern struct tcode_nvram_s ate_nvram_list[];
extern struct tcode_rc_support_s tcode_rc_support_list[];
extern struct tcode_location_s tcode_location_list[];
extern struct tcode_location_s tcode_location_list_HwIdA[];
extern struct tcode_location_s tcode_location_list_HwIdB[];
#ifdef RTCONFIG_ASUSCTRL
extern struct tcode_location_s asusctrl_tcode_location_list[];
#endif
#if defined(RTAC88U) || defined(RTAC3100)
extern struct tcode_location_s legacy_tcode_location_list[];
#endif
extern struct tcode_lang_s tcode_lang_list[];
extern struct tcode_langcode_s tcode_langcode_list[];
extern struct location_nvram_s location_init_nvram_list[];

extern char *tcode_default_get(const char *name);
extern int is_CN_sku(void);
extern int is_CN_location(void);
extern int is_EU_sku(void);
extern int noasusddns(void);

#ifdef RTCONFIG_ETH_MONITOR
int is_eth_monitor_support();
#endif

#endif	/* RTCONFIG_TCODE */
#endif	/* !__TCODE_H__ */
