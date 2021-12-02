/* Custom error logging system
 *
 * Copyright (C) 2010-2021  Joachim Wiberg <troglobit@gmail.com>
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

#ifndef INADYN_LOG_H_
#define INADYN_LOG_H_

#include <stdarg.h>
#include "os.h"

#ifndef MAX_LOG_LEVEL
#define MAX_LOG_LEVEL LOG_DEBUG
#endif

void log_init  (char *ident, int log, int bg);
void log_exit  (void);

int  log_level (char *level);

void logitf     (int prio, const char *fmt, ...);
void vlogit    (int prio, const char *fmt, va_list args);

#define logit(p, ...) do if ((p) <= MAX_LOG_LEVEL) logitf((p), __VA_ARGS__); while (0)

#endif /* INADYN_LOG_H_ */

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
