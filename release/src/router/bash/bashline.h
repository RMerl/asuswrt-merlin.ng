/* bashline.h -- interface to the bash readline functions in bashline.c. */

/* Copyright (C) 1993-2009 Free Software Foundation, Inc.

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

#if !defined (_BASHLINE_H_)
#define _BASHLINE_H_

#include "stdc.h"

extern int bash_readline_initialized;

extern void posix_readline_initialize __P((int));
extern void reset_completer_word_break_chars __P((void));
extern int enable_hostname_completion __P((int));
extern void initialize_readline __P((void));
extern void bashline_reset __P((void));
extern void bashline_reinitialize __P((void));
extern int bash_re_edit __P((char *));

extern void bashline_set_event_hook __P((void));
extern void bashline_reset_event_hook __P((void));

extern int bind_keyseq_to_unix_command __P((char *));
extern int print_unix_command_map __P((void));

extern char **bash_default_completion __P((const char *, int, int, int, int));

void set_directory_hook __P((void));

/* Used by programmable completion code. */
extern char *command_word_completion_function __P((const char *, int));
extern char *bash_groupname_completion_function __P((const char *, int));
extern char *bash_servicename_completion_function __P((const char *, int));

extern char **get_hostname_list __P((void));
extern void clear_hostname_list __P((void));

extern char **bash_directory_completion_matches __P((const char *));
extern char *bash_dequote_text __P((const char *));

#endif /* _BASHLINE_H_ */
