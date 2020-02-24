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

#include <string.h>

#include "log.h"
#include "json.h"

int parse_json(const char *json, jsmntok_t *out_tokens[])
{
	int num_tokens;

	jsmn_parser parser;
	jsmn_init(&parser);
	num_tokens = jsmn_parse(&parser, json, strlen(json), NULL, 0);

	if (num_tokens < 0) {
		logit(LOG_ERR, "Failed to parse JSON.");
		return -1;
	}

	if (num_tokens == 0) {
		logit(LOG_WARNING, "No JSON found in string.");
		return -1;
	}

	*out_tokens = malloc(num_tokens * sizeof(jsmntok_t));

	if (!(*out_tokens)) {
		logit(LOG_ERR, "Couldn't allocate memory to parse JSON.");
		return -1;
	}

	jsmn_init(&parser);
	jsmn_parse(&parser, json, strlen(json), *out_tokens, num_tokens);

	return num_tokens;
}

int jsoneq(const char *json, const jsmntok_t *tok, const char *s)
{
	if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
		strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}

	return -1;
}

int json_bool(const char *json, const jsmntok_t *token, int *out_value)
{
	if (token->type == JSMN_PRIMITIVE) {
		*out_value = json[token->start] == 't';
		return 0;
	}

	return -1;
}
