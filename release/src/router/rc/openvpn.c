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

/* RC OpenVPN code for Asuswrt-Merlin. */

#include "rc.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>

#include <openvpn_config.h>
#include <openvpn_control.h>

int ovpn_up_main(int argc, char **argv)
{
	int unit;
	char buffer[64];

	if(argc < 3)
		return -1;

	unit = atoi(argv[1]);

	if (!strcmp(argv[2], "server"))
		ovpn_server_up_handler(unit);
	else if (!strcmp(argv[2], "client")) {
		ovpn_client_up_handler(unit);

		update_resolvconf();

		sprintf(buffer, "/etc/openvpn/client%d/client.conf", unit);
		// We got client.conf && update_resolvconf() won't restart it for us
		if ((f_exists(buffer)) && (ovpn_max_dnsmode() != OVPN_DNSMODE_STRICT))
			notify_rc("start_dnsmasq");

	} else
		return -1;

	return 0;
}

int ovpn_down_main(int argc, char **argv)
{
	int unit, restart_dnsmasq = 0;
	char buffer[64];

	if(argc < 3)
		return -1;

	unit = atoi(argv[1]);
	if (!strcmp(argv[2], "server"))
		ovpn_server_down_handler(unit);
	else if (!strcmp(argv[2], "client")) {
		// Check before handler removes client.conf

		sprintf(buffer, "/etc/openvpn/client%d/client.conf", unit);
		// We got client.conf && update_resolvconf() won't restart it for us
		if ((f_exists(buffer)) && (ovpn_max_dnsmode() != OVPN_DNSMODE_STRICT))
			restart_dnsmasq = 1;

		ovpn_client_down_handler(unit);

		update_resolvconf();

		if (restart_dnsmasq)
			notify_rc("start_dnsmasq");

	} else
		return -1;

	return 0;
}

/* Wrappers for library functions */
void inline start_ovpn_eas() {
	ovpn_process_eas(1);
}

void inline stop_ovpn_eas() {
	ovpn_process_eas(0);
}

void inline stop_ovpn_client(int unit) {
	ovpn_stop_client(unit);
}

void inline start_ovpn_client(int unit) {
	ovpn_start_client(unit);
}

void inline stop_ovpn_server(int unit) {
	ovpn_stop_server(unit);
}

void inline start_ovpn_server(int unit) {
	ovpn_start_server(unit);
}
