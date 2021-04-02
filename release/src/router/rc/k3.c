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

	kprintf("Check CFE version...\n", buf);
	if (!check_if_file_exist(ROMCFE) || cfe_nvram_match("bl_version", "1.0.37_mesh2"))
		return;
	//if (pids("envrams"))
	//sleep(5);

	strcpy(et0mac, cfe_nvram_safe_get("et0macaddr"));
	if (!cfe_nvram_get("0:macaddr"))
	{
		strcpy(wl1mac, cfe_nvram_safe_get("1:macaddr"));
		strcpy(wl2mac, cfe_nvram_safe_get("2:macaddr"));
	}
	else
	{
		strcpy(wl1mac, cfe_nvram_safe_get("0:macaddr"));
		strcpy(wl2mac, cfe_nvram_safe_get("1:macaddr"));
	}

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
	envram_set("0:macaddr", et0mac); // 2.4G MAC is the same as LAN MAC in Merlin
	envram_set("1:macaddr", wl2mac);
	envram_set("2:macaddr", "00:11:22:33:44:66"); // For downgrade to 384
	envram_set("secret_code", PIN);
	envram_commit();
	nvram_set("secret_code", PIN);
	nvram_commit();

	//if (pids("envrams"))
	//killall_tk("envrams");

	bzero(buf, sizeof(buf));
	snprintf(buf, sizeof(buf), "Set CFE MAC: LAN & 2.4G=%s, 5G=%s", et0mac, wl2mac);
	//logmessage("K3INIT", "CFE has upgraded to 1.0.37_mesh already!");
	//logmessage("K3INIT", buf);
	kprintf("Upgraded CFE - %s\n", buf);
	kprintf("Upgraded CFE - Set PIN=%s\n", PIN);
	usleep(100000);
	reboot(RB_AUTOBOOT);
}

void k3_init(void)
{
	bool isChange = 0;

	kprintf("k3 init begin.\n");

	if (nvram_match("modelname", ""))
	{
		nvram_set("modelname", "K3");
		isChange = 1;
	}
	if (nvram_match("screen_timeout", ""))
	{
		nvram_set("screen_timeout", "30");
		isChange = 1;
	}
	if (nvram_match("location_code", ""))
	{
		nvram_set("location_code", "AU");
		isChange = 1;
	}
	nvram_set("netease_uu_md5", "");

	if (isChange)
	{
		nvram_commit();
		kprintf("location_code: %s\n", nvram_safe_get("location_code"));
	}
}

void k3_init_done(void)
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

int GetPhyStatus(int verbose, phy_info_list *list)
{
	int port[] = { 3, 1, 0, 2 };
	int i, ret, lret = 0, mask, ret_code = 0;
	char out_buf[21];

	bzero(out_buf, sizeof(out_buf));
	for (i = 0; i < 4; i++)
	{
		mask = 0;
		mask |= 0x0001 << port[i];
		if (list)
		{
			list->count++;
			list->phy_info[i].phy_port_id = port[i];
			if (i == 0)
			{
				snprintf(list->phy_info[i].label_name, sizeof(list->phy_info[i].label_name), "W0");
				snprintf(list->phy_info[i].cap_name, sizeof(list->phy_info[i].cap_name), "wan");
			}
			else
			{
				snprintf(list->phy_info[i].label_name, sizeof(list->phy_info[i].label_name), "L%d", i);
				snprintf(list->phy_info[i].cap_name, sizeof(list->phy_info[i].cap_name), "lan");
			}
			list->phy_info[i].tx_packets = get_phy_mib(port[i], "tx_packets");
			list->phy_info[i].rx_packets = get_phy_mib(port[i], "rx_packets");
			list->phy_info[i].tx_bytes = get_phy_mib(port[i], "tx_bytes");
			list->phy_info[i].rx_bytes = get_phy_mib(port[i], "rx_bytes");
			list->phy_info[i].crc_errors = get_phy_mib(port[i], "crc_errors");
		}
		if (get_phy_status(mask) == 0)
		{ /*Disconnect*/
			if (i == 0)
				snprintf(out_buf, sizeof(out_buf), "W0=X;");
			else
				snprintf(out_buf, sizeof(out_buf), "%sL%d=X;", out_buf, i);

			if (list)
			{
				snprintf(list->phy_info[i].state, sizeof(list->phy_info[i].state), "down");
				snprintf(list->phy_info[i].duplex, sizeof(list->phy_info[i].duplex), "none");
				list->phy_info[i].link_rate = 0;
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
				ret_code |= 0x0001 << i;
				snprintf(out_buf, sizeof(out_buf), "%sL%d=%s;", out_buf, i, (ret & 2) ? "G" : "M");
			}
			if (list)
			{
				snprintf(list->phy_info[i].state, sizeof(list->phy_info[i].state), "up");
				snprintf(list->phy_info[i].duplex, sizeof(list->phy_info[i].duplex), (get_phy_duplex(1 << port[i])) ? "full" : "half");
				list->phy_info[i].link_rate = (ret & 2) ? 1000 : 100;
			}
		}
	}

	if (verbose)
		puts(out_buf);
	if (verbose != 53134 && verbose != 8365)
		lret = ret_code;

	return lret;
}

#ifdef RTCONFIG_UUPLUGIN
void exec_uu_k3(void)
{
	FILE *fpmodel, *fpmac, *fpdir, *fpurl, *fpmd5, *fpcfg;
	char buf[128];
	char *dup, *url, *md5;
	int result;

	if (!nvram_match("netease_uu_enable", "1"))
	{
		system("killall uuplugin_monitor.sh");
		system("killall uuplugin");
	}
	else if (nvram_get_int("ntp_ready") && nvram_get_int("sw_mode") == 1)
	{
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
		if ((fpdir = fopen("/var/uu_plugin_dir", "w")))
		{
			fprintf(fpdir, "/jffs");
			fclose(fpdir);
		}
		mkdir_if_none("/tmp/uu");
		result = system("wget -t 2 -T 30 --dns-timeout=120 --header=Accept:text/plain \
			-q --no-check-certificate 'https://router.uu.163.com/api/script/monitor?type=asuswrt-merlin' \
			-O /tmp/uu/script_url");
		if (!result)
		{
			kprintf("download uuplugin script info successfully\n");
			if ((fpurl = fopen("/tmp/uu/script_url", "r")))
			{
				fgets(buf, 128, fpurl);
				fclose(fpurl);
				unlink("/tmp/uu/script_url");
				dup = strdup(buf);
				url = strsep(&dup, ",");
				md5 = strsep(&dup, ",");
				if (md5 != NULL)
				{
					if (nvram_match("netease_uu_md5", md5))
					{
						kprintf("MD5: %s is the same. skip download.\n", md5);
						if (pids("uuplugin"))
							result = 1;
						else
							result = 0;
					}
					else
					{
						kprintf("URL: %s\n", url);
						kprintf("MD5: %s\n", md5);
						mkdir_if_none("/jffs/uu");
						result = doSystem("wget -t 2 -T 30 --dns-timeout=120 --header=Accept:text/plain \
							-q --no-check-certificate %s -O /jffs/uu/uuplugin_monitor.sh", url);
					}

					if (!result)
					{
						kprintf("download uuplugin script successfully\n");
						nvram_set("netease_uu_md5", md5);
						if ((fpcfg = fopen("/jffs/uu/uuplugin_monitor.config", "w")))
						{
							fprintf(fpcfg, "router=asuswrt-merlin\n");
							fprintf(fpcfg, "model=\n");
							fclose(fpcfg);
						}

						if ((fpmd5=popen("md5sum /jffs/uu/uuplugin_monitor.sh | awk '{print $1}'", "r")))
						{
							bzero(buf, sizeof(buf));
							if((fread(buf, 1, 128, fpmd5)))
							{
								if (!strncasecmp(buf, md5, 32))
								{
									pid_t pid;
									char *uu_argv[] = { "/jffs/uu/uuplugin_monitor.sh", NULL };
									kprintf("prepare to execute uuplugin stript...\n");
									system("killall uuplugin_monitor.sh");
									system("killall uuplugin");
									chmod("/jffs/uu/uuplugin_monitor.sh", 0755);
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
