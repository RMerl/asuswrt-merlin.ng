#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <shared.h>

#if defined(RTCONFIG_HNS)
int check_hns_switch()
{
	if ((HNS_FULL & HNS_MALS) == 0 && (HNS_FULL & HNS_VP) == 0 && (HNS_FULL & HNS_CC) == 0)
		return 0;
	else
		return 1;
}

int check_hns_setting()
{
	int enabled = 1;

	if (((HNS_FULL & HNS_MALS) == 0 && (HNS_FULL & HNS_VP) == 0 && (HNS_FULL & HNS_CC) == 0) &&
		WEB_FILTER == 0 &&
		WEB_HISTORY == 0
		)
		enabled = 0;

	return enabled;
}
#endif
