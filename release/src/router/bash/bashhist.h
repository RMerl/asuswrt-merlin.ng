/* bashhist.h -- interface to the bash history functions in bashhist.c. */

/* Copyright (C) 1993-2009 Free Software Foundation,  Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#if !defined (_BASHHIST_H_)
#define _BASHHIST_H_

#include "stdc.h"

/* Flag values for history_control */
#define HC_IGNSPACE	0x01
#define HC_IGNDUPS	0x02
#define HC_ERASEDUPS	0x04

#define HC_IGNBOTH	(HC_IGNSPACE|HC_IGNDUPS)

extern int remember_on_history;
extern int enable_history_list;		/* value for `set -o history' */
extern int literal_history;		/* controlled by `shopt lithist' */
extern int force_append_history;
extern int history_lines_this_session;
extern int history_lines_in_file;
extern int history_expansion;
extern int history_control;
extern int command_oriented_history;
extern int current_command_first_line_saved;
extern int hist_last_line_added;
extern int hist_last_line_pushed;

#  if defined (BANG_HISTORY)
extern int history_expansion_inhibited;
#  endif /* BANG_HISTORY */

extern void bash_initialize_history __P((void));
extern void bash_history_reinit __P((int));
extern void bash_history_disable __P((void));
extern void bash_history_enable __P((void));
extern void bash_clear_history __P((void));
extern int bash_delete_histent __P((int));
extern int bash_delete_last_history __P((void));
extern void load_history __P((void));
extern void save_history __P((void));
extern int maybe_append_history __P((char *));
extern int maybe_save_shell_history __P((void));
extern char *pre_process_line __P((char *, int, int));
extern void maybe_add_history __P((char *));
extern void bash_add_history __P((char *));
extern int check_add_history __P((char *, int));
extern int history_number __P((void));

extern void setup_history_ignore __P((char *));

extern char *last_history_line __P((void));

#endif /* _BASHHIST_H_ */
