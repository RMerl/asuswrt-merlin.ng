/*
	Traffic Limiter usage
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ctype.h>
#include <bcmnvram.h>
#include <time.h>

#include "shutils.h"
#include "shared.h"

#ifdef RTCONFIG_TRAFFIC_LIMITER
static char *traffic_limiter_get_path(const char *type)
{
	if (type == NULL)
		return NULL;
	else if (strcmp(type, "limit") == 0)
		return "/jffs/.sys/tld/tl_limit";
	else if (strcmp(type, "alert") == 0)
		return "/jffs/.sys/tld/tl_alert";
	else if (strcmp(type, "count") == 0)
		return "/jffs/.sys/tld/tl_count";

	return NULL;
}

char *traffic_limtier_count_path()
{
	return traffic_limiter_get_path("count");
}

unsigned int traffic_limiter_read_bit(const char *type)
{
	char *path;
	char buf[sizeof("4294967295")];
	unsigned int val = 0;

	path = traffic_limiter_get_path(type);
	if (path && f_read_string(path, buf, sizeof(buf)) > 0)
		val = strtoul(buf, NULL, 10);

	TL_DBG("path = %s, val=%u\n", path ? : "NULL", val);
	return val;
}

void traffic_limiter_set_bit(const char *type, int unit)
{
	char *path;
	char buf[sizeof("4294967295")];
	unsigned int val = 0;

	path = traffic_limiter_get_path(type);
	if (path) {
		val = traffic_limiter_read_bit(type);
		val |= (1U << unit);
		snprintf(buf, sizeof(buf), "%u", val);
		f_write_string(path, buf, 0, 0);
	}

	TL_DBG("path = %s, val=%u\n", path ? : "NULL", val);
}

void traffic_limiter_clear_bit(const char *type, int unit)
{
	char *path;
	char buf[sizeof("4294967295")];
	unsigned int val = 0;

	path = traffic_limiter_get_path(type);
	if (path) {
		val = traffic_limiter_read_bit(type);
		val &= ~(1U << unit);
		snprintf(buf, sizeof(buf), "%u", val);
		f_write_string(path, buf, 0, 0);
	}

	TL_DBG("path = %s, val=%u\n", path ? : "NULL", val);
}

double traffic_limiter_get_realtime(int unit)
{
	char path[PATH_MAX];
	char buf[32];
	double val = 0;

	snprintf(path, sizeof(path), "/tmp/tl%d_realtime", unit);
	if (f_read_string(path, buf, sizeof(buf)) > 0)
		val = atof(buf);

	return val;
}

int TL_UNIT_S; // traffic limiter dual wan unit start
int TL_UNIT_E; // traffic limiter dual wan unit end

int traffic_limiter_dualwan_check(char *dualwan_mode)
{
	int ret = 1;

	/* check daul wan mode */
	if (!strcmp(dualwan_mode, "lb"))
	{
		// load balance
		TL_UNIT_S = WAN_UNIT_FIRST;
		TL_UNIT_E = WAN_UNIT_MAX;
	}
	else if (!strcmp(dualwan_mode, "fo") || !strcmp(dualwan_mode, "fb"))
	{
		// fail over or fail back
		TL_UNIT_S = wan_primary_ifunit();
		TL_UNIT_E = wan_primary_ifunit() + 1;
	}
	else
	{
		printf("%s : can't identify daulwan_mode\n", __FUNCTION__);
		ret = 0;
	}

	return ret;
}
#endif
