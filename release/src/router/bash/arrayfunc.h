/* arrayfunc.h -- declarations for miscellaneous array functions in arrayfunc.c */

/* Copyright (C) 2001-2010 Free Software Foundation, Inc.

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

#if !defined (_ARRAYFUNC_H_)
#define _ARRAYFUNC_H_

/* Must include variables.h before including this file. */

#if defined (ARRAY_VARS)

/* Flags for array_value_internal and callers array_value/get_array_value */
#define AV_ALLOWALL	0x001
#define AV_QUOTED	0x002
#define AV_USEIND	0x004
#define AV_USEVAL	0x008	/* XXX - should move this */
#define AV_ASSIGNRHS	0x010	/* no splitting, special case ${a[@]} */

extern SHELL_VAR *convert_var_to_array __P((SHELL_VAR *));
extern SHELL_VAR *convert_var_to_assoc __P((SHELL_VAR *));

extern char *make_array_variable_value __P((SHELL_VAR *, arrayind_t, char *, char *, int));

extern SHELL_VAR *bind_array_variable __P((char *, arrayind_t, char *, int));
extern SHELL_VAR *bind_array_element __P((SHELL_VAR *, arrayind_t, char *, int));
extern SHELL_VAR *assign_array_element __P((char *, char *, int));

extern SHELL_VAR *bind_assoc_variable __P((SHELL_VAR *, char *, char *, char *, int));

extern SHELL_VAR *find_or_make_array_variable __P((char *, int));

extern SHELL_VAR *assign_array_from_string  __P((char *, char *, int));
extern SHELL_VAR *assign_array_var_from_word_list __P((SHELL_VAR *, WORD_LIST *, int));

extern WORD_LIST *expand_compound_array_assignment __P((SHELL_VAR *, char *, int));
extern void assign_compound_array_list __P((SHELL_VAR *, WORD_LIST *, int));
extern SHELL_VAR *assign_array_var_from_string __P((SHELL_VAR *, char *, int));

extern int unbind_array_element __P((SHELL_VAR *, char *));
extern int skipsubscript __P((const char *, int, int));

extern void print_array_assignment __P((SHELL_VAR *, int));
extern void print_assoc_assignment __P((SHELL_VAR *, int));

extern arrayind_t array_expand_index __P((SHELL_VAR *, char *, int));
extern int valid_array_reference __P((const char *, int));
extern char *array_value __P((const char *, int, int, int *, arrayind_t *));
extern char *get_array_value __P((const char *, int, int *, arrayind_t *));

extern char *array_keys __P((char *, int));

extern char *array_variable_name __P((const char *, char **, int *));
extern SHELL_VAR *array_variable_part __P((const char *, char **, int *));

#else

#define AV_ALLOWALL	0
#define AV_QUOTED	0
#define AV_USEIND	0
#define AV_ASSIGNRHS	0

#endif

#endif /* !_ARRAYFUNC_H_ */
