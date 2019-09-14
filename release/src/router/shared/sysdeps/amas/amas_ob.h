#ifndef __AMAS_OB_H__
#define __AMAS_OB_H__

#define REBOOT_DEF_TIME 180		/* second */
#define CONNECTION_DEF_TIMEOUT 60	/* second */
#define TRAFFIC_DEF_TIMEOUT 60		/* second */
#define MAX_VALUE_REBOOT_TIME	600	/* second */
#define MAX_VALUE_CONNECTION_TIMEOUT	300	/* second */
#define MAX_VALUE_TRAFFIC_TIMEOUT	300	/* second */

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
	{ "Lyra_Trio",	80,	60,	60},
	{ "RT-AX88U",	50,     60,     60},
	{ "RT-AX92U",	50,     60,     60},
	{ "GT-AX11000",	50,     60,     60},
	/* END */
	{ NULL, 0, 0, 0}
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
				return;
			}
		}
	}
}

#endif // __AMAS_OB_H__
