#ifndef _RTK_WIFI_DRVMIB_H_
#define _RTK_WIFI_DRVMIB_H_

int drvmib_init_rtkapi();
int drvmib_release_rtkapi();
int drvmib_apply_rtkapi();
struct wifi_mib* drvmib_get_pmib(char* ifname);

#endif
