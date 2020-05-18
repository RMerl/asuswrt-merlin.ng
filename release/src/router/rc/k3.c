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

	if (!check_if_file_exist(ROMCFE) || cfe_nvram_match("bl_version", "1.0.37"))
		return;
	//if (pids("envrams"))
		//sleep(5);

	strcpy(et0mac, cfe_nvram_safe_get("et0macaddr"));
	strcpy(wl1mac, cfe_nvram_safe_get("1:macaddr"));
	strcpy(wl2mac, cfe_nvram_safe_get("2:macaddr"));

	if (pids("envrams")) {
		killall_tk("envrams");
		usleep(100000);
	}

	doSystem("dd if=%s of=/dev/mtdblock0 2>/dev/null", ROMCFE);

	envram_set("et0macaddr", et0mac);
	envram_set("1:macaddr", wl1mac);
	envram_set("2:macaddr", wl2mac);
	envram_commit();

	//if (pids("envrams"))
		//killall_tk("envrams");

	snprintf(buf, sizeof(buf), "Set CFE MAC: LAN=%s, 2.4G=%s, 5G=%s", et0mac, wl1mac, wl2mac);
	//logmessage("K3INIT", "CFE has upgraded to 1.0.37 already!");
	//logmessage("K3INIT", buf);
	_dprintf("**** Upgraded CFE - %s\n", buf);
	usleep(100000);
	reboot(RB_AUTOBOOT);
}

void k3_init()
{
	bool isChange = 0;

	if (!nvram_safe_get("modelname"))
	{
		nvram_set("modelname", "K3");
		isChange = 1;
	}
	if (!nvram_safe_get("screen_timeout"))
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
	_dprintf("**** softcenter: init done\n");
#endif
}

void start_k3screen(void)
{
	char timeout[6];
	int time = nvram_get_int("screen_timeout");
	snprintf(timeout, sizeof(timeout), "-m%d", time);
	char *k3screenctrl_argv[] = {"k3screenctrl", timeout, NULL}; //screen timeout 30sec
	char *k3screenbg_argv[] = {"k3screenbg", NULL};
	pid_t pid1, pid2;

	doSystem("killall -q -9 k3screenctrl k3screenbg 2>/dev/null");

	logmessage("K3INIT", "屏幕控制程序开始启动");
	_dprintf("**** k3screen: start\n");
	doSystem("mkdir -p /tmp/k3screenctrl");
	doSystem("ln -snf /lib/k3screenctrl/* /tmp/k3screenctrl");
	_eval(k3screenctrl_argv, NULL, 0, &pid1);
	_eval(k3screenbg_argv, NULL, 0, &pid2);
	_dprintf("**** k3screen: ok\n");
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
