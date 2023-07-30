#ifndef __AMAS_OB_H__
#define __AMAS_OB_H__

#define REBOOT_DEF_TIME 180		/* second */
#define CONNECTION_DEF_TIMEOUT 60	/* second */
#define TRAFFIC_DEF_TIMEOUT 60		/* second */
#define MAX_VALUE_REBOOT_TIME	600	/* second */
#define MAX_VALUE_CONNECTION_TIMEOUT	300	/* second */
#define MAX_VALUE_TRAFFIC_TIMEOUT	300	/* second */

#if (defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS))
#define CFG_MNT_FOLDER		"/jffs/.sys/cfg_mnt/"
#else
#define CFG_MNT_FOLDER		"/tmp/cfg_mnt/"
#endif

struct time_mapping_s {
	char *model_name;
	int reboot_time;	/* unit: second, the range is > 0 and <= MAX_VALUE_REBOOT_TIME. */
	int connection_timeout;	/* unit: second, the range is > 0 and <= MAX_VALUE_CONNECTION_TIMEOUT. */
	int traffic_timeout;	/* unit: second, the range is > 0 and <= MAX_VALUE_TRAFFIC_TIMEOUT. */
};

static struct time_mapping_s time_mapping_list[] = {
	{ "RT-AC68U",	120,	60,	60},
	{ "RT-AC68P",	120,	60,	60},
	{ "RT-AC68UF",	120,	60,	60},
	{ "RT-AC68W",	120,	60,	60},
	{ "RT-AC68R",	120,	60,	60},
	{ "RT-AC68U V2",120,	60,	60},
	{ "RT-AC1900",	120,	60,	60},
	{ "RT-AC1900P",	120,	60,	60},
	{ "RT-AC88U",	120,	60,	60},
	{ "RT-AC3100",	120,	60,	60},
	{ "RT-AC5300",	120,	60,	60},
	{ "RT-AC86U",	50,	60,	60},
	{ "GT-AC5300",	50,	60,	60},
	{ "BLUECAVE",	120,	120,	60},
	{ "BLUE_CAVE",	120,	120,	60},
	{ "RT-AX89X",	80,	120,	60},
	{ "Lyra",	80,	60,	60},
	{ "Lyra_Mini",	80,	60,	60},
	{ "Lyra_Trio",		80,	120,	120},
	{ "RT-AC59U_V2",	80,	120,	120},
	{ "RT-AC58U_V3",	80,	120,	120},
	{ "RT-AC57U_V3",	80,	120,	120},
	{ "RT-AC1300G_PLUS_V3",	80,	120,	120},
	{ "ZenWiFi_CD6R",	80,	120,	120},
	{ "ZenWiFi_CD6N",	80,	120,	120},
	{ "RT-AX88U",	50,     120,     60},
	{ "RT-AX92U",	50,     120,     60},
	{ "RT-AX95Q",   50,     120,     60},
	{ "XT8PRO",   50,     120,     60},
	{ "RT-AXE95Q",   50,     120,     60},
	{ "ET8PRO",   50,     120,     60},
	{ "ZenWiFi_XD4",   50,     120,     60},
	{ "ZenWiFi_XD4PRO",   50,     120,     60},
	{ "RT-AX56_XD4",   50,     120,     60},
	{ "CT-AX56_XD4",   50,     60,     60},
	{ "RT-AX55",    50,     120,     60},
	{ "RT-AX58U",   50,     120,     60},
	{ "RT-AX58U_V2",50,     120,     60},
#ifndef TUFAX3000_V2
	{ "TUF-AX3000", 50,     120,     60},
#else
	{ "TUF-AX3000", 90,     120,     90},
#endif
	{ "TUF-AX3000_V2",90,   120,     90},
	{ "TUF-AX5400", 50,     120,     60},
	{ "RT-AX82U",   50,     120,     60},
	{ "GS-AX3000",  50,     60,     60},
	{ "GS-AX5400",  50,     60,     60},
	{ "RT-AX56U",   50,     120,     60},
	{ "RP-AX56",   50,     120,     60},
	{ "RP-AX58",   50,     120,     60},
	{ "GT-AX11000",	50,     120,     60},
	{ "RT-AC85P",	120,	60,	60},
	{ "GT-AXY16000",80,    120,     60},
	{ "GT-AXE11000", 50,    120,     60},
	{ "RT-AX86U",    50,   120,     60},
	{ "RT-AX86S",    50,   100,     60},
	{ "RT-AX68U",    50,   100,     60},
	{ "RT-AC68U_V4", 50,    60,     60},
	{ "GT-AX6000", 50,    120,     60},
	{ "GT-AX11000_PRO", 50,    120,     60},
	{ "GT-AXE16000", 50,    120,     60},
	{ "ET12", 50,    120,     60},
	{ "XT12", 50,    120,     60},
	{ "RT-AXE7800",	90,     120,     90},
	{ "RT-AX86U_PRO",    50,   120,     60},
	{ "XT8_V2",   50,     120,     60},
	{ "GT10", 90,     120,     90},
	{ "GT6",  90,     120,     90},
	{ "RT-AX3000N", 50,     120,     60},
	{ "BR63",       50,     120,     60},
	{ "RT-AX82U_V2",50,     120,     60},
	{ "TUF-AX5400_V2",50,   120,     60},
	{ "RT-AX88U_PRO", 50,    120,     60},
	{ "BM68",   50,     120,     60},
	{ "ET8_V2",   50,     120,     60},
	{ "RT-AX5400",   50,     120,     60},
	{ "RT-AX9000",   50,     120,     60},
	/* END */
	{ NULL, 0, 0, 0 }
};

static void
time_mapping_get(char *model_name, struct time_mapping_s *time_mapping)
{
	if (!time_mapping)
		return;

	time_mapping->reboot_time = REBOOT_DEF_TIME;
	time_mapping->connection_timeout = CONNECTION_DEF_TIMEOUT;
	time_mapping->traffic_timeout = TRAFFIC_DEF_TIMEOUT;

	if (strlen(model_name)) {
		struct time_mapping_s *pTimeout = &time_mapping_list[0];
		for (pTimeout = &time_mapping_list[0]; pTimeout->model_name; pTimeout++) {
			if (!strcmp(pTimeout->model_name, model_name)) {
				/* check reboot time */
				if ((pTimeout->reboot_time > 0) &&
					(pTimeout->reboot_time <= MAX_VALUE_REBOOT_TIME))
					time_mapping->reboot_time = pTimeout->reboot_time;
				else
					time_mapping->reboot_time = 0;

				/* check connection timeout */
				if ((pTimeout->connection_timeout > 0) &&
					(pTimeout->connection_timeout <= MAX_VALUE_CONNECTION_TIMEOUT))
					time_mapping->connection_timeout = pTimeout->connection_timeout;
				else
					time_mapping->connection_timeout = 0;

				/* check traffic timeout */
				if ((pTimeout->traffic_timeout > 0) &&
					(pTimeout->traffic_timeout <= MAX_VALUE_TRAFFIC_TIMEOUT))
					time_mapping->traffic_timeout = pTimeout->traffic_timeout;
				else
					time_mapping->traffic_timeout = 0;

				break;
			}
		}
	}

#ifdef RTCONFIG_HND_ROUTER_AX
	time_mapping->connection_timeout = CONNECTION_DEF_TIMEOUT * 2;
#endif	/* RTCONFIG_HND_ROUTER_AX */

#ifdef RTCONFIG_PRELINK
#if defined(RTCONFIG_QCA)
	if (has_dfs_channel())
#else
#warning ### NEED method to identify is DFS mode or NOT ###
#endif	/* QCA */
	{
		time_mapping->connection_timeout += 60;
	}
#endif	/* PRELINK */
}

#endif // __AMAS_OB_H__
