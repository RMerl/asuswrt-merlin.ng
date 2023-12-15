/* -*- mode: c; c-basic-offset: 2 -*- */
/* 
 * Copyright (C) 2003, 2004, 2005 Mondru AB.
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

extern int chilli_main(int argc, char **argv);

#if defined(__linux__)
#include <unistd.h>
#include <sys/ptrace.h>
#include <asm/unistd.h>
#include <sys/syscall.h>

#include <sys/time.h>
#include <sys/resource.h>

extern int sys_ioprio_set(int, int, int);
extern int sys_ioprio_get(int, int);

enum {
        IOPRIO_CLASS_NONE,
        IOPRIO_CLASS_RT,
        IOPRIO_CLASS_BE,
        IOPRIO_CLASS_IDLE,
};

enum {
        IOPRIO_WHO_PROCESS = 1,
        IOPRIO_WHO_PGRP,
        IOPRIO_WHO_USER,
};

#define IOPRIO_CLASS_SHIFT      13

#ifndef __NR_ioprio_set
#if defined(__i386__)
#define __NR_ioprio_set         289
#define __NR_ioprio_get         290
#elif defined(__ppc__)
#define __NR_ioprio_set         273
#define __NR_ioprio_get         274
#elif defined(__x86_64__)
#define __NR_ioprio_set         251
#define __NR_ioprio_get         252
#elif defined(__ia64__)
#define __NR_ioprio_set         1274
#define __NR_ioprio_get         1275
#endif
#endif
#endif

int main(int argc, char **argv)
{
  int ret;
#if defined(__linux__)
  char *ev;
#endif

#ifdef MTRACE
  mtrace();  /* Turn on mtrace function */
#endif

#if defined(__linux__)
  if ((ev = getenv("CHILLI_PRIORITY")) != NULL) {
    if (setpriority(PRIO_PROCESS, getpid(), atoi(ev))) {
      perror("setpriority");
    }
  }    

#ifdef __NR_ioprio_set
  if ((ev = getenv("CHILLI_IOPRIO_RT")) != NULL) {
    if (syscall(__NR_ioprio_set, IOPRIO_WHO_PROCESS, getpid(), atoi(ev) | IOPRIO_CLASS_RT << IOPRIO_CLASS_SHIFT) == -1) {
      perror("ioprio_set");
    }
  }
#endif
#endif

  ret = chilli_main(argc, argv);
#ifdef MTRACE
  muntrace();  /* Turn off mtrace function */
#endif
  return ret;
}
