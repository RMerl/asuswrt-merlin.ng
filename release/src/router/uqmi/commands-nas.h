/*
 * uqmi -- tiny QMI support implementation
 *
 * Copyright (C) 2014-2015 Felix Fietkau <nbd@openwrt.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#define __uqmi_nas_commands \
	__uqmi_command(nas_do_set_system_selection, __set-system-selection, no, QMI_SERVICE_NAS), \
	__uqmi_command(nas_set_network_modes, set-network-modes, required, CMD_TYPE_OPTION), \
	__uqmi_command(nas_initiate_network_register, network-register, no, QMI_SERVICE_NAS), \
	__uqmi_command(nas_set_plmn, set-plmn, no, QMI_SERVICE_NAS), \
	__uqmi_command(nas_set_mcc, mcc, required, CMD_TYPE_OPTION), \
	__uqmi_command(nas_set_mnc, mnc, required, CMD_TYPE_OPTION), \
	__uqmi_command(nas_network_scan, network-scan, no, QMI_SERVICE_NAS), \
	__uqmi_command(nas_get_signal_info, get-signal-info, no, QMI_SERVICE_NAS), \
	__uqmi_command(nas_get_serving_system, get-serving-system, no, QMI_SERVICE_NAS), \
	__uqmi_command(nas_set_network_preference, set-network-preference, required, CMD_TYPE_OPTION), \
	__uqmi_command(nas_set_roaming, set-network-roaming, required, CMD_TYPE_OPTION) \

#define nas_helptext \
		"  --set-network-modes <modes>:      Set usable network modes (Syntax: <mode1>[,<mode2>,...])\n" \
		"                                    Available modes: all, lte, umts, gsm, cdma, td-scdma\n" \
		"  --set-network-preference <mode>:  Set preferred network mode to <mode>\n" \
		"                                    Available modes: auto, gsm, wcdma\n" \
		"  --set-network-roaming <mode>:     Set roaming preference:\n" \
		"                                    Available modes: any, off, only\n" \
		"  --network-scan:                   Initiate network scan\n" \
		"  --network-register:               Initiate network register\n" \
		"  --set-plmn:                       Register at specified network\n" \
		"    --mcc <mcc>:                    Mobile Country Code (0 - auto)\n" \
		"    --mnc <mnc>:                    Mobile Network Code\n" \
		"  --get-signal-info:                Get signal strength info\n" \
		"  --get-serving-system:             Get serving system info\n" \

