/* strongSwan charon launcher
 * Copyright (C) 2001-2002 Mathieu Lafon - Arkoon Network Security
 * Copyright (C) 2006 Martin Willi - Hochschule fuer Technik Rapperswil
 *
 * Ported from invokepluto.h to fit charons needs.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#ifndef _STARTER_CHARON_H_
#define _STARTER_CHARON_H_

#define CHARON_RESTART_DELAY    5

extern void starter_charon_sigchild (pid_t pid, int status);
extern pid_t starter_charon_pid (void);
extern int starter_stop_charon (void);
extern int starter_start_charon(struct starter_config *cfg, bool no_fork, bool attach_gdb);

#endif /* _STARTER_CHARON_H_ */

