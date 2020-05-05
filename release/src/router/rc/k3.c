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
#include "k3.h"

void k3_init()
{
	if (!nvram_get("modelname"))
		nvram_set("modelname", "K3");
	if (!nvram_get("screen_timeout"))
		nvram_set("screen_timeout", "30");
	nvram_commit();
}

void k3_init_done()
{
	start_k3screen();
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
