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
 *
 */

/*
 * OpenVPN utility library for Asuswrt-Merlin
 * Provides some of the functions found in Asuswrt's
 * proprietary libvpn, either re-implemented, or
 * implemented as wrappers around AM's functions.
 * Also includes additional functions developed
 * for Asuswrt-Merlin.
 */

#include <stdio.h>
#include <string.h>

#include <shared.h>
#include "openvpn_config.h"
#include "openvpn_control.h"

int _check_ovpn_enabled(int unit, ovpn_type_t type){
	char tmp[2];
	char* varname = NULL;

	switch (type) {
		case OVPN_TYPE_SERVER:
			if (unit <= OVPN_SERVER_MAX)
				varname = "vpn_serverx_start";
			break;
		case OVPN_TYPE_CLIENT:
			if (unit <= OVPN_CLIENT_MAX)
				varname = "vpn_clientx_eas";
			break;
	}

	if (varname) {
		sprintf(tmp,"%d", unit);
		if (strstr(nvram_safe_get(varname), tmp)) return 1;
	}

	return 0;
}

int check_ovpn_server_enabled(int unit) {
	return _check_ovpn_enabled(unit, OVPN_TYPE_SERVER);
}

int check_ovpn_client_enabled(int unit) {
	return _check_ovpn_enabled(unit, OVPN_TYPE_CLIENT);
}

void update_ovpn_routing(int unit){
	char cmd[56];
	snprintf(cmd, sizeof (cmd), "dev=tun1%d script_type=rmupdate /usr/sbin/vpnrouting.sh", unit);
	system(cmd);
}

