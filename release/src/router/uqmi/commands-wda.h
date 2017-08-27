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

#define __uqmi_wda_commands \
	__uqmi_command(wda_set_data_format, wda-set-data-format, required, QMI_SERVICE_WDA), \
	__uqmi_command(wda_get_data_format, wda-get-data-format, no, QMI_SERVICE_WDA)


#define wda_helptext \
		"  --wda-set-data-format <type>:     Set data format (type: 802.3|raw-ip)\n" \
		"  --wda-get-data-format:            Get data format\n" \

