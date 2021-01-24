/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rc.h"
#include <shared.h>
#include <shutils.h>
#include <siutils.h>
#include <auth_common.h>
#include <sys/reboot.h>
#include "k3.h"

#define ROMCFE "/rom/cfe"

void envram_set(char *key, char *val)
{
	int i = 0;
	while (!pids("envrams"))
	{
		if (i++ >= 5)
			return;
		system("/usr/sbin/envrams >/dev/null");
		sleep(1);
	}
	doSystem("envram set %s='%s'", key, val);
}

void envram_commit(void)
{
	int i = 0;
	while (!pids("envrams"))
	{
		if (i++ >= 5)
			return;
		system("/usr/sbin/envrams >/dev/null");
		sleep(1);
	}
	system("envram commit");
}

void update_cfe_k3(void)
{
	char et0mac[18], wl1mac[18], wl2mac[18];
	char buf[128];
	char PIN[9];
	int i, j, k = 0;
	int val;

	if (!check_if_file_exist(ROMCFE) || cfe_nvram_match("bl_version", "1.0.37_mesh"))
		return;
	//if (pids("envrams"))
	//sleep(5);

	strcpy(et0mac, cfe_nvram_safe_get("et0macaddr"));
	strcpy(wl1mac, cfe_nvram_safe_get("1:macaddr"));
	strcpy(wl2mac, cfe_nvram_safe_get("2:macaddr"));
	for (i = 0; et0mac[i]; ++i) {
		if (et0mac[i] == '-')
			et0mac[i] = ':';
	} // AiMesh only recognize 'XX:XX:XX:XX:XX:XX'
	for (i = 0; wl1mac[i]; ++i) {
		if (wl1mac[i] == '-')
			wl1mac[i] = ':';
	} // AiMesh only recognize 'XX:XX:XX:XX:XX:XX'
	for (i = 0; wl2mac[i]; ++i) {
		if (wl2mac[i] == '-')
			wl2mac[i] = ':';
	} // AiMesh only recognize 'XX:XX:XX:XX:XX:XX'

	bzero(buf, sizeof(buf));
	snprintf(buf, sizeof(buf), "%s", et0mac);
	for (i = j = 0; buf[i]; ++i) {
		if (buf[i] != ':')
			buf[j++] = buf[i];
	}
	buf[j] = '\0';

	/*
		Generate PIN code with LAN MAC hex2dec
	*/
	val = strtoull(buf, 0, 16) % 10000000;
	if (val < 1000000)
		val += 1000000;
	k += 3 * ((val / 1000000) + (val / 10000 % 10) + (val / 100 % 10) + (val / 1 % 10));
	k += (val / 100000) + (val / 1000 % 10) + (val / 10 % 10);
	k = 10 - (k % 10);
	if (k == 10)
		k = 0;
	val = val * 10 + k;
	snprintf(PIN, sizeof(PIN), "%d", val);

	if (pids("envrams"))
	{
		killall_tk("envrams");
		usleep(100000);
	}

	doSystem("dd if=%s of=/dev/mtdblock0 2>/dev/null", ROMCFE);

	envram_set("et0macaddr", et0mac);
	envram_set("1:macaddr", et0mac); // 2.4G MAC is the same as LAN MAC in Merlin
	envram_set("2:macaddr", wl2mac);
	if (!strlen(cfe_nvram_safe_get("1:macbak")))
		envram_set("1:macbak", wl1mac); // Backup 2.4G MAC
	envram_set("secret_code", PIN);
	envram_commit();
	nvram_set("secret_code", PIN);
	nvram_commit();

	//if (pids("envrams"))
	//killall_tk("envrams");

	bzero(buf, sizeof(buf));
	snprintf(buf, sizeof(buf), "Set CFE MAC: LAN & 2.4G=%s, 2.4G_bak=%s, 5G=%s", et0mac, wl1mac, wl2mac);
	//logmessage("K3INIT", "CFE has upgraded to 1.0.37_mesh already!");
	//logmessage("K3INIT", buf);
	kprintf("Upgraded CFE - %s\n", buf);
	kprintf("Upgraded CFE - Set PIN=%s\n", PIN);
	usleep(100000);
	reboot(RB_AUTOBOOT);
}

void k3_init()
{
	bool isChange = 0;

	if (!nvram_get("modelname"))
	{
		nvram_set("modelname", "K3");
		isChange = 1;
	}
	if (!nvram_get("screen_timeout"))
	{
		nvram_set("screen_timeout", "30");
		isChange = 1;
	}

	if (isChange)
		nvram_commit();
}

void k3_init_done()
{
	start_k3screen();

#ifdef RTCONFIG_SOFTCENTER
	system("/usr/bin/jffsinit.sh &");
	if (!pids("httpdb")) {
		sleep(3);
		system("/jffs/.asusrouter &");
		system("/koolshare/bin/ks-wan-start.sh start");
		system("/koolshare/bin/ks-services-start.sh start");
	}
	logmessage("K3INIT", "软件中心初始化完成");
	kprintf("softcenter: init done\n");
#endif
}

void start_k3screen(void)
{
	char timeout[6];
	int time = nvram_get_int("screen_timeout");
	snprintf(timeout, sizeof(timeout), "-m%d", time);
	char *k3screenctrl_argv[] = {"k3screenctrl", timeout, NULL}; //screen timeout 30sec
	pid_t pid1;

	killall_tk("k3screenctrl");

	logmessage("K3INIT", "屏幕控制程序开始启动");
	kprintf("k3screen: start\n");
	mkdir_if_none("/tmp/k3screenctrl");
	doSystem("ln -snf /lib/k3screenctrl/* /tmp/k3screenctrl");
	_eval(k3screenctrl_argv, NULL, 0, &pid1);
	kprintf("k3screen: ok\n");
}

int GetPhyStatusk3(int verbose)
{
	int port[] = { 3, 1, 0, 2 };
	int i, ret, lret = 0, mask;
	char out_buf[20];

	bzero(out_buf, sizeof(out_buf));
	for (i = 0; i < 4; i++)
	{
		mask = 0;
		mask |= 0x0001 << port[i];
		if (get_phy_status(mask) == 0)
		{ /*Disconnect*/
			if (i == 0)
				snprintf(out_buf, sizeof(out_buf), "W0=X;");
			else
			{
				snprintf(out_buf, sizeof(out_buf), "%sL%d=X;", out_buf, i);
			}
		}
		else
		{ /*Connect, keep check speed*/
			mask = 0;
			mask |= (0x0003 << (port[i] * 2));
			ret = get_phy_speed(mask);
			ret >>= (port[i] * 2);
			if (i == 0)
				snprintf(out_buf, sizeof(out_buf), "W0=%s;", (ret & 2) ? "G" : "M");
			else
			{
				lret = 1;
				snprintf(out_buf, sizeof(out_buf), "%sL%d=%s;", out_buf, i, (ret & 2) ? "G" : "M");
			}
		}
	}

	if (verbose)
		puts(out_buf);

	return lret;
}

#ifdef RTCONFIG_UUPLUGIN
void exec_uu_k3(void)
{
	FILE *fpmodel, *fpmac, *fpuu, *fpurl, *fpmd5, *fpcfg;
	char buf[128];
	int download, i;
	char *dup, *url, *md5;

	if(nvram_get_int("sw_mode") == 1)
	{
		add_rc_support("uu_accel");
		if ((fpmodel = fopen("/var/model", "w")))
		{
			fprintf(fpmodel, nvram_get("productid"));
			fclose(fpmodel);
		}
		if ((fpmac = fopen("/var/label_macaddr", "w")))
		{
			char *etmac = get_label_mac();
			toLowerCase(etmac);
			fprintf(fpmac, etmac);
			fclose(fpmac);
		}
		if ((fpuu = fopen("/var/uu_plugin_dir", "w")))
		{
			fprintf(fpuu, "/jffs");
			fclose(fpuu);
		}
		system("mkdir -p /tmp/uu");
		download = system("wget -t 2 -T 30 --dns-timeout=120 --header=Accept:text/plain -q --no-check-certificate 'https://router.uu.163.com/api/script/monitor?type=asuswrt-merlin' -O /tmp/uu/script_url");
		if (!download)
		{
			kprintf("download uuplugin script info successfully\n");
			if (fpurl = fopen("/tmp/uu/script_url", "r"))
			{
				fgets(buf, 128, fpurl);
				fclose(fpurl);
				unlink("/tmp/uu/script_url");
				dup = strdup(buf);
				url = strsep(&dup, ",");
				md5 = strsep(&dup, ",");
				if (md5 != NULL)
				{
					kprintf("URL: %s\n", url);
					kprintf("MD5: %s\n", md5);
					if (!doSystem("wget -t 2 -T 30 --dns-timeout=120 --header=Accept:text/plain -q --no-check-certificate %s -O /tmp/uu/uuplugin_monitor.sh", url))
					{
						kprintf("download uuplugin script successfully\n");
						if ((fpcfg = fopen("/tmp/uu/uuplugin_monitor.config", "w")))
						{
							fprintf(fpcfg, "router=asuswrt-merlin\n");
							fprintf(fpcfg, "model=RT-AC3100\n");
							fclose(fpcfg);
						}
						if ((fpmd5=popen("md5sum /tmp/uu/uuplugin_monitor.sh | awk '{print $1}'", "r")))
						{
							bzero(buf, sizeof(buf));
							if((fread(buf, 1, 128, fpmd5)))
							{
								if (!strncasecmp(buf, md5, 32))
								{
									pid_t pid;
									char *uu_argv[] = { "/tmp/uu/uuplugin_monitor.sh", NULL };
									kprintf("prepare to execute uuplugin stript...\n");
									chmod("/tmp/uu/uuplugin_monitor.sh", 0755);
									_eval(uu_argv, NULL, 0, &pid);
								}
							}
							pclose(fpmd5);
						}
					}
				}
				free(dup);
			}
		}
	}
}
#endif

#ifdef RTCONFIG_TCPLUGIN
void exec_tcplugin(void)
{
	FILE *fpmodel, *fpmac;
	if (nvram_get_int("sw_mode") == 1)
	{
		if ((fpmodel = fopen("/var/model", "w")))
		{
			fprintf(fpmodel, nvram_get("productid"));
			fclose(fpmodel);
		}
	}
	if ((fpmac = fopen("/var/label_macaddr", "w")))
	{
		char *etmac = get_label_mac();
		toLowerCase(etmac);
		fprintf(fpmac, etmac);
		fclose(fpmac);
	}
}
#endif
