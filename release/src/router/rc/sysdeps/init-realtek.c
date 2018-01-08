/*

	Copyright 2005, Broadcom Corporation
	All Rights Reserved.

	THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
	KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
	SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
	FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.

*/

#include "rc.h"
#include "shared.h"
#include "interface.h"

#include <termios.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <time.h>
#include <errno.h>
#include <paths.h>
#include <sys/wait.h>
#include <sys/reboot.h>
#include <sys/klog.h>
#ifdef LINUX26
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#endif
#include <wlutils.h>
#include <bcmdevs.h>

#define MKNOD(name,mode,dev)	if(mknod(name,mode,dev)) perror("## mknod " name)

void init_devs(void)
{
//	system("mknod /dev/nvram ");
	MKNOD("/dev/nvram", S_IFCHR | 0x666, makedev(228, 0));
	{
		int status;
		if((status = WEXITSTATUS(modprobe("nvram_linux"))))	printf("## modprove(nvram_linux) fail status(%d)\n", status);
	}
}

void rtl_configRps(void)
{
	system("mount -t sysfs sysfs /sys");
	system("echo 2 > /sys/class/net/eth0/queues/rx-0/rps_cpus");
	system("echo 2 > /sys/class/net/eth1/queues/rx-0/rps_cpus");
	system("echo 2 > /sys/class/net/eth2/queues/rx-0/rps_cpus");
	system("echo 2 > /sys/class/net/eth3/queues/rx-0/rps_cpus");
	system("echo 2 > /sys/class/net/eth4/queues/rx-0/rps_cpus");
	system("echo 2 > /sys/class/net/wl0/queues/rx-0/rps_cpus");
	system("echo 2 > /sys/class/net/wl0.0/queues/rx-0/rps_cpus");		
	system("echo 2 > /sys/class/net/wl0.1/queues/rx-0/rps_cpus");		
	system("echo 2 > /sys/class/net/wl0.2/queues/rx-0/rps_cpus");	
	system("echo 2 > /sys/class/net/wl0.3/queues/rx-0/rps_cpus");
	system("echo 2 > /sys/class/net/wl1/queues/rx-0/rps_cpus");
	system("echo 2 > /sys/class/net/wl1.0/queues/rx-0/rps_cpus");
	system("echo 2 > /sys/class/net/wl1.1/queues/rx-0/rps_cpus");
	system("echo 2 > /sys/class/net/wl1.2/queues/rx-0/rps_cpus");
	system("echo 2 > /sys/class/net/wl1.3/queues/rx-0/rps_cpus");
	system("echo 4096 > /sys/class/net/eth0/queues/rx-0/rps_flow_cnt");
	system("echo 4096 > /sys/class/net/eth1/queues/rx-0/rps_flow_cnt");
	system("echo 4096 > /sys/class/net/eth2/queues/rx-0/rps_flow_cnt");
	system("echo 4096 > /sys/class/net/eth3/queues/rx-0/rps_flow_cnt");
	system("echo 4096 > /sys/class/net/eth4/queues/rx-0/rps_flow_cnt");
	system("echo 4096 > /proc/sys/net/core/rps_sock_flow_entries");
}


#if defined(RTCONFIG_REALTEK)
void init_igmpsnooping()
{
    char command[32] = {0};
    //snprintf(command, sizeof(command), "echo %d > /proc/br_igmpsnoop", nvram_get_int("emf_enable"));
    snprintf(command, sizeof(command), "echo 1 > /proc/br_igmpsnoop");
    //_dprintf("init_igmpsnooping: command = %s\n", command);
    system(command);
}
#endif

void init_switch()
{
	char *mac_addr;
	char word[PATH_MAX], *next_word;
	mac_addr = nvram_safe_get("et0macaddr");

	foreach(word, nvram_safe_get("lan_ifnames"), next_word) {
		if (!strncmp(word, "eth", 3)) {
			doSystem("ifconfig %s hw ether %s", word, mac_addr);
		}
	}

	mac_addr = nvram_safe_get("et1macaddr");

	if(strlen(nvram_safe_get("wan0_ifname")))
		doSystem("ifconfig %s hw ether %s", nvram_safe_get("wan0_ifname"), mac_addr);

	if (!is_router_mode())
		doSystem("echo \"2\" > /proc/hw_nat");
#if defined(RTCONFIG_REALTEK)
	init_igmpsnooping();
#endif
}

char *get_lan_hwaddr(void)
{
	/* TODO: handle exceptional model */
        return nvram_safe_get("et0macaddr");
}

void init_syspara(void)
{

#ifdef RTCONFIG_ODMPID
	char modelname[16];
	getflashMN(modelname, sizeof(modelname));

	if (modelname[0] != 0 && (unsigned char)(modelname[0]) != 0xff
	    && is_valid_hostname(modelname)
	    && strcmp(modelname, "ASUS")) {
		nvram_set("odmpid", modelname);
	} else
#endif
		nvram_unset("odmpid");

	nvram_set("firmver", rt_version);
	nvram_set("productid", rt_buildname);
	set_basic_fw_name();

	set_country_code();
#ifdef RTCONFIG_TCODE
	set_territory_code();
#endif
}


void init_wl(void)
{
}

void fini_wl(void)
{
}

void config_switch()
{
}

int switch_exist(void)
{
	FILE *fp;
	char out[64];

	system("echo 'physt 1' > /proc/asus_ate");
	fp = popen("cat /proc/asus_ate", "r");
	if (fp) {
		fgets(out, sizeof(out),fp);
		pclose(fp);
		if (strlen(out))
			return 1;
	}
	return 0;
}

int wl_exist(char *ifname, int band)
{
	char file[64];
	int ret = 1;
	
	memset(file, 0, sizeof(file));
	snprintf(file, sizeof(file), "/proc/%s/stats", ifname);

	if(!f_exists(file))
		ret = 0;

	return ret; 
}

char *get_wlifname(int unit, int subunit, int subunit_x, char *buf)
{
	sprintf(buf, "wl%d.%d", unit, subunit);
	return buf;
}

void generate_wl_para(int unit, int subunit)
{
}

void stop_wds_rtk(const char* lan_ifname, const char* wif)
{
}




