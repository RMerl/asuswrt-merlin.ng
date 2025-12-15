/* md-internal.h

   Copyright (C) 2001, 2010, 2022 Niels Möller

   This file is part of GNU Nettle.

   GNU Nettle is free software: you can redistribute it and/or
   modify it under the terms of either:

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at your
       option) any later version.

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at your
       option) any later version.

   or both in parallel, as here.

   GNU Nettle is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see http://www.gnu.org/licenses/.
*/

#ifndef NETTLE_MD_INTERNAL_H_INCLUDED
#define NETTLE_MD_INTERNAL_H_INCLUDED

#include <string.h>

/* Internal helper macros for Merkle-Damgård hash functions. Assumes the context
   structs includes the following fields:

     uint8_t block[...];		// Buffer holding one block
     unsigned int index;		// Index into block
*/

#define MD_FILL_OR_RETURN(ctx, length, data)			\
  do {								\
    unsigned __md_left = sizeof((ctx)->block) - (ctx)->index;	\
    if ((length) < __md_left)					\
      {								\
	memcpy((ctx)->block + (ctx)->index, (data), (length));	\
	(ctx)->index += (length);				\
	return;							\
      }								\
    memcpy((ctx)->block + (ctx)->index, (data), __md_left);	\
    (data) += __md_left;					\
    (length) -= __md_left;					\
  } while(0)

#define MD_FILL_OR_RETURN_INDEX(block_size, block, index, length, data)	\
  do {									\
    unsigned __md_left = (block_size) - (index);			\
    if ((length) < __md_left)						\
      {									\
	memcpy(block + (index), (data), (length));			\
	return (index) + (length);					\
      }									\
    memcpy((block) + (index), (data), __md_left);			\
    (data) += __md_left;						\
    (length) -= __md_left;						\
  } while(0)
#endif /* NETTLE_MD_INTERNAL_H_INCLUDED */
