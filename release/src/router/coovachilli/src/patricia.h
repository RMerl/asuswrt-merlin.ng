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
/*
 * $Id: patricia.h,v 1.6 2005/12/07 20:53:01 dplonka Exp $
 * Dave Plonka <plonka@doit.wisc.edu>
 *
 * This product includes software developed by the University of Michigan,
 * Merit Network, Inc., and their contributors. 
 *
 * This file had been called "radix.h" in the MRT sources.
 *
 * I renamed it to "patricia.h" since it's not an implementation of a general
 * radix trie.  Also, pulled in various requirements from "mrt.h" and added
 * some other things it could be used as a standalone API.
 */

#ifndef _PATRICIA_H
#define _PATRICIA_H

#include "system.h"

#define HAVE_IPV6

typedef void (*void_fn_t)();
#define prefix_touchar(prefix) ((u_char *)&(prefix)->add.sin)
#define MAXLINE 1024
#define BIT_TEST(f, b)  ((f) & (b))

typedef struct _prefix4_t {
  u_short family;		/* AF_INET | AF_INET6 */
  u_short bitlen;		/* same as mask? */
  int ref_count;		/* reference count */
  struct in_addr sin;
} prefix4_t;

typedef struct _prefix_t {
  u_short family;		/* AF_INET | AF_INET6 */
  u_short bitlen;		/* same as mask? */
  int ref_count;		/* reference count */
  union {
    struct in_addr sin;
#ifdef HAVE_IPV6
    struct in6_addr sin6;
#endif /* IPV6 */
  } add;
} prefix_t;

/* } */

typedef struct _patricia_node_t {
  u_int bit;			/* flag if this node used */
  prefix_t *prefix;		/* who we are in patricia tree */
  struct _patricia_node_t *l, *r;	/* left and right children */
  struct _patricia_node_t *parent;/* may be used */
  void *data;			/* pointer to data */
  void	*user1;			/* pointer to usr data (ex. route flap info) */
} patricia_node_t;

typedef struct _patricia_tree_t {
  patricia_node_t 	*head;
  u_int		maxbits;	/* for IP, 32 bit addresses */
  int num_active_node;		/* for debug purpose */
} patricia_tree_t;


patricia_node_t *patricia_search_exact (patricia_tree_t *patricia, 
					prefix_t *prefix);

patricia_node_t *patricia_search_best (patricia_tree_t *patricia, 
				       prefix_t *prefix);

patricia_node_t * patricia_search_best2 (patricia_tree_t *patricia, 
					 prefix_t *prefix, 
					 int inclusive);

patricia_node_t *patricia_lookup (patricia_tree_t *patricia, prefix_t *prefix);
void patricia_remove (patricia_tree_t *patricia, patricia_node_t *node);

patricia_tree_t *patricia_new (int maxbits);
void patricia_clear (patricia_tree_t *patricia, void_fn_t func);
void patricia_destroy (patricia_tree_t *patricia, void_fn_t func);

void patricia_process (patricia_tree_t *patricia, void_fn_t func);

char *prefix_toa (prefix_t * prefix);

prefix_t * patricia_prefix_new2 (int family, void *dest, int bitlen, prefix_t *prefix);
prefix_t * patricia_prefix_new (int family, void *dest, int bitlen);

prefix_t * patricia_prefix_ref (prefix_t * prefix);
void patricia_prefix_deref (prefix_t * prefix);

prefix_t *
ascii2prefix (int family, char *string);

patricia_node_t *
make_and_lookup (patricia_tree_t *tree, char *string);

#define PATRICIA_MAXBITS	(sizeof(struct in6_addr) * 8)
#define PATRICIA_NBIT(x)        (0x80 >> ((x) & 0x7f))
#define PATRICIA_NBYTE(x)       ((x) >> 3)

#define PATRICIA_DATA_GET(node, type) (type *)((node)->data)
#define PATRICIA_DATA_SET(node, value) ((node)->data = (void *)(value))

#define PATRICIA_WALK(Xhead, Xnode) \
    do { \
        patricia_node_t *Xstack[PATRICIA_MAXBITS+1]; \
        patricia_node_t **Xsp = Xstack; \
        patricia_node_t *Xrn = (Xhead); \
        while ((Xnode = Xrn)) { \
            if (Xnode->prefix)

#define PATRICIA_WALK_ALL(Xhead, Xnode) \
do { \
        patricia_node_t *Xstack[PATRICIA_MAXBITS+1]; \
        patricia_node_t **Xsp = Xstack; \
        patricia_node_t *Xrn = (Xhead); \
        while ((Xnode = Xrn)) { \
	    if (1)

#define PATRICIA_WALK_BREAK { \
	    if (Xsp != Xstack) { \
		Xrn = *(--Xsp); \
	     } else { \
		Xrn = (patricia_node_t *) 0; \
	    } \
	    continue; }

#define PATRICIA_WALK_END \
            if (Xrn->l) { \
                if (Xrn->r) { \
                    *Xsp++ = Xrn->r; \
                } \
                Xrn = Xrn->l; \
            } else if (Xrn->r) { \
                Xrn = Xrn->r; \
            } else if (Xsp != Xstack) { \
                Xrn = *(--Xsp); \
            } else { \
                Xrn = (patricia_node_t *) 0; \
            } \
        } \
    } while (0)

/* #define PATRICIA_DEBUG 0 */

#endif /* _PATRICIA_H */
