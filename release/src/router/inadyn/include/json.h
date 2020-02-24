/* JSON helpers
 *
 * Copyright (C) 2019-2020 Simon Pilkington
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, visit the Free Software Foundation
 * website at http://www.gnu.org/licenses/gpl-2.0.html or write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#ifndef INADYN_JSON_H_
#define INADYN_JSON_H_

#define JSMN_HEADER
#include "jsmn.h"

int parse_json(const char *json, jsmntok_t *out_tokens[]);
int jsoneq(const char *json, const jsmntok_t *tok, const char *s);
int json_bool(const char *json, const jsmntok_t *token, int *out_value);

#endif
