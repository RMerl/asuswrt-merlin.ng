/* -*- mode: c; c-basic-offset: 2 -*- */
/* 
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#define MAIN_FILE

#include "chilli.h"

struct options_t _options;

#ifndef CHILLI_USER
#define CHILLI_USER "chilli"
#endif
#ifndef CHILLI_GROUP
#define CHILLI_GROUP "chilli"
#endif

static void usage(char *prog) {
  fprintf(stderr,"%s chilli-bin script-path [script-arguments]\n", prog);
  exit(-1);
}

int main(int argc, char **argv) {
  struct stat statbuf;
  uid_t uid = getuid();
  uid_t gid = getgid();
  uid_t euid = geteuid();
  uid_t egid = getegid();
  
  struct passwd * pwd = getpwuid(uid);
  struct group * grp = getgrgid(gid);

  if (argc < 3)
    usage(argv[0]);
  
  options_init();

  openlog(PACKAGE, LOG_PID, LOG_DAEMON);
  
  memset(&statbuf, 0, sizeof(statbuf));
  
  if (!options_binload(argv[1])) {
    log_err(0, "invalid binary config file %s", argv[1]);
    usage(argv[0]);
  }

  if (uid != 0) {
    if (strcmp(pwd->pw_name, CHILLI_USER)) {
      log_err(0, "has to run as user %s or root", CHILLI_USER);
      usage(argv[0]);
    }
    
    if (strcmp(grp->gr_name, CHILLI_GROUP)) {
      log_err(0, "has to run as group %s or root", CHILLI_GROUP);
      usage(argv[0]);
    }
  }
  
  log_dbg("USER %s(%d/%d), GROUP %s(%d/%d) CHILLI[UID %d, GID %d]", 
	  pwd->pw_name, uid, euid, grp->gr_name, gid, egid,
	  _options.uid, _options.gid);
  
  if (stat(argv[2], &statbuf)) { 
    log_err(errno, "%s does not exist", argv[2]); 
    usage(argv[0]);
  }
  
  if (_options.uid &&                        /* chilli is running as non-root */
      _options.uid == euid &&                /* current euid same as chilli uid */
      _options.gid == egid &&                /* current egid same as chilli gid */
      statbuf.st_uid == 0 &&                 /* script owned by root */
      statbuf.st_gid == _options.gid &&      /* script group same as chilli gid */
      (statbuf.st_mode & 0400) == 0400) {
    
    if (setuid(0))
      log_err(errno, "setuid %s", argv[0]);
  }
  
  log_info("Running %s (%d/%d)", argv[2], getuid(), geteuid());

  if (execv(argv[2], &argv[2])) {
    log_err(errno, "exec %s", argv[2]);
    usage(argv[0]);
  }
  
  return 0;
}
