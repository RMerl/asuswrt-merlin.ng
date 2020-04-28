/* assoc.h -- definitions for the interface exported by assoc.c that allows
   the rest of the shell to manipulate associative array variables. */

/* Copyright (C) 2008,2009 Free Software Foundation, Inc.

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

#ifndef _ASSOC_H_
#define _ASSOC_H_

#include "stdc.h"
#include "hashlib.h"

#define assoc_empty(h)		((h)->nentries == 0)
#define assoc_num_elements(h)	((h)->nentries)

#define assoc_create(n)		(hash_create((n)))

#define assoc_copy(h)		(hash_copy((h), 0))

#define assoc_walk(h, f)	(hash_walk((h), (f))

extern void assoc_dispose __P((HASH_TABLE *));
extern void assoc_flush __P((HASH_TABLE *));

extern int assoc_insert __P((HASH_TABLE *, char *, char *));
extern PTR_T assoc_replace __P((HASH_TABLE *, char *, char *));
extern void assoc_remove __P((HASH_TABLE *, char *));

extern char *assoc_reference __P((HASH_TABLE *, char *));

extern char *assoc_subrange __P((HASH_TABLE *, arrayind_t, arrayind_t, int, int));
extern char *assoc_patsub __P((HASH_TABLE *, char *, char *, int));
extern char *assoc_modcase __P((HASH_TABLE *, char *, int, int));

extern HASH_TABLE *assoc_quote __P((HASH_TABLE *));
extern HASH_TABLE *assoc_quote_escapes __P((HASH_TABLE *));
extern HASH_TABLE *assoc_dequote __P((HASH_TABLE *));
extern HASH_TABLE *assoc_dequote_escapes __P((HASH_TABLE *));
extern HASH_TABLE *assoc_remove_quoted_nulls __P((HASH_TABLE *));

extern char *assoc_to_assign __P((HASH_TABLE *, int));

extern WORD_LIST *assoc_to_word_list __P((HASH_TABLE *));
extern WORD_LIST *assoc_keys_to_word_list __P((HASH_TABLE *));

extern char *assoc_to_string __P((HASH_TABLE *, char *, int));
#endif /* _ASSOC_H_ */
