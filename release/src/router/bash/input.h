/* input.h -- Structures and unions used for reading input. */

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

#if !defined (_INPUT_H_)
#define _INPUT_H_

#include "stdc.h"

/* Function pointers can be declared as (Function *)foo. */
#if !defined (_FUNCTION_DEF)
#  define _FUNCTION_DEF
typedef int Function ();
typedef void VFunction ();
typedef char *CPFunction ();		/* no longer used */
typedef char **CPPFunction ();		/* no longer used */
#endif /* _FUNCTION_DEF */

typedef int sh_cget_func_t __P((void));		/* sh_ivoidfunc_t */
typedef int sh_cunget_func_t __P((int));	/* sh_intfunc_t */

enum stream_type {st_none, st_stdin, st_stream, st_string, st_bstream};

#if defined (BUFFERED_INPUT)

/* Possible values for b_flag. */
#undef B_EOF
#undef B_ERROR		/* There are some systems with this define */
#undef B_UNBUFF

#define B_EOF		0x01
#define B_ERROR		0x02
#define B_UNBUFF	0x04
#define B_WASBASHINPUT	0x08
#define B_TEXT		0x10

/* A buffered stream.  Like a FILE *, but with our own buffering and
   synchronization.  Look in input.c for the implementation. */
typedef struct BSTREAM
{
  int	 b_fd;
  char	*b_buffer;		/* The buffer that holds characters read. */
  size_t b_size;		/* How big the buffer is. */
  size_t b_used;		/* How much of the buffer we're using, */
  int	 b_flag;		/* Flag values. */
  size_t b_inputp;		/* The input pointer, index into b_buffer. */
} BUFFERED_STREAM;

#if 0
extern BUFFERED_STREAM **buffers;
#endif

extern int default_buffered_input;

#endif /* BUFFERED_INPUT */

typedef union {
  FILE *file;
  char *string;
#if defined (BUFFERED_INPUT)
  int buffered_fd;
#endif
} INPUT_STREAM;

typedef struct {
  enum stream_type type;
  char *name;
  INPUT_STREAM location;
  sh_cget_func_t *getter;
  sh_cunget_func_t *ungetter;
} BASH_INPUT;

extern BASH_INPUT bash_input;

/* Functions from parse.y whose use directly or indirectly depends on the
   definitions in this file. */
extern void initialize_bash_input __P((void));
extern void init_yy_io __P((sh_cget_func_t *, sh_cunget_func_t *, enum stream_type, const char *, INPUT_STREAM));
extern char *yy_input_name __P((void));
extern void with_input_from_stdin __P((void));
extern void with_input_from_string __P((char *, const char *));
extern void with_input_from_stream __P((FILE *, const char *));
extern void push_stream __P((int));
extern void pop_stream __P((void));
extern int stream_on_stack __P((enum stream_type));
extern char *read_secondary_line __P((int));
extern int find_reserved_word __P((char *));
extern void gather_here_documents __P((void));
extern void execute_variable_command __P((char *, char *));

extern int *save_token_state __P((void));
extern void restore_token_state __P((int *));

/* Functions from input.c */
extern int getc_with_restart __P((FILE *));
extern int ungetc_with_restart __P((int, FILE *));

#if defined (BUFFERED_INPUT)
/* Functions from input.c. */
extern int fd_is_bash_input __P((int));
extern int set_bash_input_fd __P((int));
extern int save_bash_input __P((int, int));
extern int check_bash_input __P((int));
extern int duplicate_buffered_stream __P((int, int));
extern BUFFERED_STREAM *fd_to_buffered_stream __P((int));
extern BUFFERED_STREAM *set_buffered_stream __P((int, BUFFERED_STREAM *));
extern BUFFERED_STREAM *open_buffered_stream __P((char *));
extern void free_buffered_stream __P((BUFFERED_STREAM *));
extern int close_buffered_stream __P((BUFFERED_STREAM *));
extern int close_buffered_fd __P((int));
extern int sync_buffered_stream __P((int));
extern int buffered_getchar __P((void));
extern int buffered_ungetchar __P((int));
extern void with_input_from_buffered_stream __P((int, char *));
#endif /* BUFFERED_INPUT */

#endif /* _INPUT_H_ */
