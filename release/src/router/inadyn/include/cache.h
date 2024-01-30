/* Interface to file chache API
 *
 * Copyright (C) 2014-2021  Joachim Wiberg <troglobit@gmail.com>
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

#ifndef INADYN_CACHE_H_
#define INADYN_CACHE_H_

#include "ddns.h"

extern char *cache_dir;

char *cache_file       (char *name, const char *sysname, char *buf, size_t len);
int   read_cache_file  (ddns_t *ctx);
int   write_cache_file (ddns_alias_t *alias, const char *name);

#endif /* INADYN_CACHE_H_ */

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
