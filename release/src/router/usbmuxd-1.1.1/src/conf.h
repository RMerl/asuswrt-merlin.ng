/*
 * conf.h
 *
 * Copyright (C) 2013 Nikias Bassen <nikias@gmx.li>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 or version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef CONF_H
#define CONF_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <plist/plist.h>

const char *config_get_config_dir();

void config_get_system_buid(char **system_buid);

int config_has_device_record(const char *udid);
int config_get_device_record(const char *udid, char **record_data, uint64_t *record_size);
int config_set_device_record(const char *udid, char* record_data, uint64_t record_size);
int config_remove_device_record(const char *udid);

void config_device_record_get_host_id(const char *udid, char **host_id);

#endif
