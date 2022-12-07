/* strongSwan file locations
 * Copyright (C) 2001-2002 Mathieu Lafon - Arkoon Network Security
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

#ifndef _STARTER_FILES_H_
#define _STARTER_FILES_H_

#define CONFIG_FILE     IPSEC_CONFDIR "/ipsec.conf"
#define SECRETS_FILE    IPSEC_CONFDIR "/ipsec.secrets"

#define CHARON_CTL_FILE IPSEC_PIDDIR "/charon.ctl"

extern char *daemon_name;
extern char *cmd;
extern char *pid_file;

#endif /* _STARTER_FILES_H_ */
