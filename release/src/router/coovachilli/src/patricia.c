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
 * $Id: patricia.c,v 1.7 2005/12/07 20:46:41 dplonka Exp $
 * Dave Plonka <plonka@doit.wisc.edu>
 *
 * This product includes software developed by the University of Michigan,
 * Merit Network, Inc., and their contributors. 
 *
 * This file had been called "radix.c" in the MRT sources.
 *
 * I renamed it to "patricia.c" since it's not an implementation of a general
 * radix trie.  Also I pulled in various requirements from "prefix.c" and
 * "demo.c" so that it could be used as a standalone API.
 */

#include "chilli.h"
#include <assert.h>

u_char *
prefix_tochar (prefix_t * prefix)
{
  if (prefix == NULL)
    return (NULL);
  
  return ((u_char *) & prefix->add.sin);
}

int 
comp_with_mask (void *addr, void *dest, u_int mask)
{
  
  if ( /* mask/8 == 0 || */ memcmp (addr, dest, mask / 8) == 0) {
    int n = mask / 8;
    int m = ((-1) << (8 - (mask % 8)));
    
    if (mask % 8 == 0 || (((u_char *)addr)[n] & m) == (((u_char *)dest)[n] & m))
      return (1);
  }
  return (0);
}

/* this allows imcomplete prefix */
int
my_inet_pton (int af, const char *src, void *dst)
{
  if (af == AF_INET) {
    int i, c, val;
    u_char xp[sizeof(struct in_addr)] = {0, 0, 0, 0};
    
    for (i = 0; ; i++) {
      c = *src++;
      if (!isdigit (c))
	return (-1);
      val = 0;
      do {
	val = val * 10 + c - '0';
	if (val > 255)
	  return (0);
	c = *src++;
      } while (c && isdigit (c));
      xp[i] = val;
      if (c == '\0')
	break;
      if (c != '.')
	return (0);
      if (i >= 3)
	return (0);
    }
    memcpy (dst, xp, sizeof(struct in_addr));
    return (1);
#ifdef HAVE_IPV6
  } else if (af == AF_INET6) {
    return (inet_pton (af, src, dst));
#endif /* HAVE_IPV6 */
  } else {
#ifndef NT
    errno = EAFNOSUPPORT;
#endif /* NT */
    return -1;
  }
}

#define PATRICIA_MAX_THREADS		16

/* 
 * convert prefix information to ascii string with length
 * thread safe and (almost) re-entrant implementation
 */
char *
prefix_toa2x (prefix_t *prefix, char *buff, int with_len)
{
  if (prefix == NULL)
    return ("(Null)");
  assert (prefix->ref_count >= 0);
  if (buff == NULL) {
    
    struct buffer {
      char buffs[PATRICIA_MAX_THREADS][48+5];
      u_int i;
    } *buffp;
    
#    if 0
    THREAD_SPECIFIC_DATA (struct buffer, buffp, 1);
#    else
    { /* for scope only */
      static struct buffer local_buff;
      buffp = &local_buff;
    }
#    endif
    if (buffp == NULL) {
      /* XXX should we report an error? */
      return (NULL);
    }
    
    buff = buffp->buffs[buffp->i++%PATRICIA_MAX_THREADS];
  }
  if (prefix->family == AF_INET) {
    u_char *a;
    assert (prefix->bitlen <= sizeof(struct in_addr) * 8);
    a = prefix_touchar (prefix);
    if (with_len) {
      sprintf (buff, "%d.%d.%d.%d/%d", a[0], a[1], a[2], a[3],
	       prefix->bitlen);
    }
    else {
      sprintf (buff, "%d.%d.%d.%d", a[0], a[1], a[2], a[3]);
    }
    return (buff);
  }
#ifdef HAVE_IPV6
  else if (prefix->family == AF_INET6) {
    char *r;
    r = (char *) inet_ntop (AF_INET6, &prefix->add.sin6, buff, 48 /* a guess value */ );
    if (r && with_len) {
      assert (prefix->bitlen <= sizeof(struct in6_addr) * 8);
      sprintf (buff + strlen (buff), "/%d", prefix->bitlen);
    }
    return (buff);
  }
#endif /* HAVE_IPV6 */
  else
    return (NULL);
}

/* prefix_toa2
 * convert prefix information to ascii string
 */
char *
prefix_toa2 (prefix_t *prefix, char *buff)
{
  return (prefix_toa2x (prefix, buff, 0));
}

/* prefix_toa
 */
char *
prefix_toa (prefix_t * prefix)
{
  return (prefix_toa2 (prefix, (char *) NULL));
}

prefix_t *
patricia_prefix_new2 (int family, void *dest, int bitlen, prefix_t *prefix)
{
  int dynamic_allocated = 0;
  int default_bitlen = sizeof(struct in_addr) * 8;
  
#ifdef HAVE_IPV6
  if (family == AF_INET6) {
    default_bitlen = sizeof(struct in6_addr) * 8;
    if (prefix == NULL) {
      prefix = calloc(1, sizeof (prefix_t));
      dynamic_allocated++;
    }
    memcpy (&prefix->add.sin6, dest, sizeof(struct in6_addr));
  }
  else
#endif /* HAVE_IPV6 */
    if (family == AF_INET) {
      if (prefix == NULL) {
#ifndef NT
	prefix = calloc(1, sizeof (prefix4_t));
#else
	//for some reason, compiler is getting
	//prefix4_t size incorrect on NT
	prefix = calloc(1, sizeof (prefix_t)); 
#endif /* NT */
	
	dynamic_allocated++;
      }
      memcpy (&prefix->add.sin, dest, sizeof(struct in_addr));
    }
    else {
      return (NULL);
    }
  
  prefix->bitlen = (bitlen >= 0)? bitlen: default_bitlen;
  prefix->family = family;
  prefix->ref_count = 0;
  if (dynamic_allocated) {
    prefix->ref_count++;
  }
  /* fprintf(stderr, "[C %s, %d]", prefix_toa (prefix), prefix->ref_count); */
  return (prefix);
}

prefix_t *
patricia_prefix_new (int family, void *dest, int bitlen)
{
  return (patricia_prefix_new2 (family, dest, bitlen, NULL));
}

/* ascii2prefix
 */
prefix_t *
ascii2prefix (int family, char *string)
{
  u_long bitlen, maxbitlen = 0;
  char *cp;
  struct in_addr sin;
#ifdef HAVE_IPV6
  struct in6_addr sin6;
#endif /* HAVE_IPV6 */
  int result;
  char save[MAXLINE];
  
  if (string == NULL)
    return (NULL);
  
  /* easy way to handle both families */
  if (family == 0) {
    family = AF_INET;
#ifdef HAVE_IPV6
    if (strchr (string, ':')) family = AF_INET6;
#endif /* HAVE_IPV6 */
  }
  
  if (family == AF_INET) {
    maxbitlen = sizeof(struct in_addr) * 8;
  }
#ifdef HAVE_IPV6
  else if (family == AF_INET6) {
    maxbitlen = sizeof(struct in6_addr) * 8;
  }
#endif /* HAVE_IPV6 */
  
  if ((cp = strchr (string, '/')) != NULL) {
    bitlen = atol (cp + 1);
    /* *cp = '\0'; */
    /* copy the string to save. Avoid destroying the string */
    assert (cp - string < MAXLINE);
    memcpy (save, string, cp - string);
    save[cp - string] = '\0';
    string = save;
    if (bitlen < 0 || bitlen > maxbitlen)
      bitlen = maxbitlen;
  }
  else {
    bitlen = maxbitlen;
  }
  
  if (family == AF_INET) {
    if ((result = my_inet_pton (AF_INET, string, &sin)) <= 0)
      return (NULL);
    return (patricia_prefix_new (AF_INET, &sin, bitlen));
  }
  
#ifdef HAVE_IPV6
  else if (family == AF_INET6) {
    // Get rid of this with next IPv6 upgrade
#if defined(NT) && !defined(HAVE_INET_NTOP)
    inet6_addr(string, &sin6);
    return (patricia_prefix_new (AF_INET6, &sin6, bitlen));
#else
    if ((result = inet_pton (AF_INET6, string, &sin6)) <= 0)
      return (NULL);
#endif /* NT */
    return (patricia_prefix_new (AF_INET6, &sin6, bitlen));
  }
#endif /* HAVE_IPV6 */
  else
    return (NULL);
}

prefix_t *
patricia_prefix_ref (prefix_t * prefix)
{
  if (prefix == NULL)
    return (NULL);
  if (prefix->ref_count == 0) {
    /* make a copy in case of a static prefix */
    return (patricia_prefix_new2 (prefix->family, &prefix->add, prefix->bitlen, NULL));
  }
  prefix->ref_count++;
  /* fprintf(stderr, "[A %s, %d]", prefix_toa (prefix), prefix->ref_count); */
  return (prefix);
}

void 
patricia_prefix_deref (prefix_t * prefix)
{
  if (prefix == NULL)
    return;
  /* for secure programming, raise an assert. no static prefix can call this */
  assert (prefix->ref_count > 0);
  
  prefix->ref_count--;
  assert (prefix->ref_count >= 0);
  if (prefix->ref_count <= 0) {
    free (prefix);
    return;
  }
}

/* #define PATRICIA_DEBUG 1 */

static int num_active_patricia = 0;

/* these routines support continuous mask only */

patricia_tree_t *
patricia_new (int maxbits)
{
  patricia_tree_t *patricia = calloc(1, sizeof *patricia);
  
  patricia->maxbits = maxbits;
  patricia->head = NULL;
  patricia->num_active_node = 0;
  assert (maxbits <= PATRICIA_MAXBITS); /* XXX */
  num_active_patricia++;
  return (patricia);
}

/*
 * if func is supplied, it will be called as func(node->data)
 * before deleting the node
 */

void
patricia_clear (patricia_tree_t *patricia, void_fn_t func)
{
  assert (patricia);
  if (patricia->head) {
    
    patricia_node_t *Xstack[PATRICIA_MAXBITS+1];
    patricia_node_t **Xsp = Xstack;
    patricia_node_t *Xrn = patricia->head;
    
    while (Xrn) {
      patricia_node_t *l = Xrn->l;
      patricia_node_t *r = Xrn->r;
      
      if (Xrn->prefix) {
	patricia_prefix_deref (Xrn->prefix);
	if (Xrn->data && func)
	  func (Xrn->data);
      }
      else {
	assert (Xrn->data == NULL);
      }
      free (Xrn);
      patricia->num_active_node--;
      
      if (l) {
	if (r) {
	  *Xsp++ = r;
	}
	Xrn = l;
      } else if (r) {
	Xrn = r;
      } else if (Xsp != Xstack) {
	Xrn = *(--Xsp);
      } else {
	Xrn = NULL;
      }
    }
  }
  assert (patricia->num_active_node == 0);
  /* free (patricia); */
}

void
patricia_destroy (patricia_tree_t *patricia, void_fn_t func)
{
  patricia_clear (patricia, func);
  free (patricia);
  num_active_patricia--;
}

/*
 * if func is supplied, it will be called as func(node->prefix, node->data)
 */
void
patricia_process (patricia_tree_t *patricia, void_fn_t func)
{
  patricia_node_t *node;
  assert (func);
  
  PATRICIA_WALK (patricia->head, node) {
    func (node->prefix, node->data);
  } PATRICIA_WALK_END;
}

size_t
patricia_walk_inorder(patricia_node_t *node, void_fn_t func)
{
  size_t n = 0;
  assert(func);
  
  if (node->l) {
    n += patricia_walk_inorder(node->l, func);
  }
  
  if (node->prefix) {
    func(node->prefix, node->data);
    n++;
  }
  
  if (node->r) {
    n += patricia_walk_inorder(node->r, func);
  }
  
  return n;
}


patricia_node_t *
patricia_search_exact (patricia_tree_t *patricia, prefix_t *prefix)
{
  patricia_node_t *node;
  u_char *addr;
  u_int bitlen;
  
  assert (patricia);
  assert (prefix);
  assert (prefix->bitlen <= patricia->maxbits);
  
  if (patricia->head == NULL)
    return (NULL);
  
  node = patricia->head;
  addr = prefix_touchar (prefix);
  bitlen = prefix->bitlen;
  
  while (node->bit < bitlen) {
    
    if (BIT_TEST (addr[node->bit >> 3], 0x80 >> (node->bit & 0x07))) {
#ifdef PATRICIA_DEBUG
      if (node->prefix) {
	log_dbg( "patricia_search_exact: take right %s/%d", 
		 prefix_toa (node->prefix), node->prefix->bitlen);
      } else {
	log_dbg( "patricia_search_exact: take right at %u", 
		 node->bit);
      }
#endif /* PATRICIA_DEBUG */
      node = node->r;
	}
    else {
#ifdef PATRICIA_DEBUG
      if (node->prefix) {
	log_dbg( "patricia_search_exact: take left %s/%d", 
		 prefix_toa (node->prefix), node->prefix->bitlen);
      } else { 
	log_dbg( "patricia_search_exact: take left at %u", 
		 node->bit);
      }
#endif /* PATRICIA_DEBUG */
      node = node->l;
    }
    
    if (node == NULL)
      return (NULL);
  }
  
#ifdef PATRICIA_DEBUG
  if (node->prefix) {
    log_dbg( "patricia_search_exact: stop at %s/%d", 
	     prefix_toa (node->prefix), node->prefix->bitlen);
  } else {
    log_dbg( "patricia_search_exact: stop at %u", node->bit);
  }
#endif /* PATRICIA_DEBUG */
  if (node->bit > bitlen || node->prefix == NULL)
    return (NULL);
  assert (node->bit == bitlen);
  assert (node->bit == node->prefix->bitlen);
  if (comp_with_mask (prefix_tochar (node->prefix), prefix_tochar (prefix),
		      bitlen)) {
#ifdef PATRICIA_DEBUG
    log_dbg( "patricia_search_exact: found %s/%d", 
	     prefix_toa (node->prefix), node->prefix->bitlen);
#endif /* PATRICIA_DEBUG */
    return (node);
  }
  return (NULL);
}


/* if inclusive != 0, "best" may be the given prefix itself */
patricia_node_t *
patricia_search_best2 (patricia_tree_t *patricia, prefix_t *prefix, int inclusive)
{
  patricia_node_t *node;
  patricia_node_t *stack[PATRICIA_MAXBITS + 1];
  u_char *addr;
  u_int bitlen;
  int cnt = 0;
  
  assert (patricia);
  assert (prefix);
  assert (prefix->bitlen <= patricia->maxbits);
  
  if (patricia->head == NULL)
    return (NULL);
  
  node = patricia->head;
  addr = prefix_touchar (prefix);
  bitlen = prefix->bitlen;
  
  while (node->bit < bitlen) {
    
    if (node->prefix) {
#ifdef PATRICIA_DEBUG
      log_dbg( "patricia_search_best: push %s/%d", 
	       prefix_toa (node->prefix), node->prefix->bitlen);
#endif /* PATRICIA_DEBUG */
      stack[cnt++] = node;
    }
    
    if (BIT_TEST (addr[node->bit >> 3], 0x80 >> (node->bit & 0x07))) {
#ifdef PATRICIA_DEBUG
      if (node->prefix) {
	log_dbg( "patricia_search_best: take right %s/%d", 
		 prefix_toa (node->prefix), node->prefix->bitlen);
      } else {
	log_dbg( "patricia_search_best: take right at %u", 
		 node->bit);
      }
#endif /* PATRICIA_DEBUG */
      node = node->r;
    }
    else {
#ifdef PATRICIA_DEBUG
      if (node->prefix) {
	log_dbg( "patricia_search_best: take left %s/%d", 
		 prefix_toa (node->prefix), node->prefix->bitlen);
      } else {
	log_dbg( "patricia_search_best: take left at %u", 
		 node->bit);
      }
#endif /* PATRICIA_DEBUG */
      node = node->l;
    }
    
    if (node == NULL)
      break;
  }
  
  if (inclusive && node && node->prefix)
    stack[cnt++] = node;
  
#ifdef PATRICIA_DEBUG
  if (node == NULL) {
    log_dbg( "patricia_search_best: stop at null");
  } else if (node->prefix) {
    log_dbg( "patricia_search_best: stop at %s/%d", 
	     prefix_toa (node->prefix), node->prefix->bitlen);
  } else {
    log_dbg( "patricia_search_best: stop at %u", node->bit);
  }
#endif /* PATRICIA_DEBUG */
  
  if (cnt <= 0)
    return (NULL);
  
  while (--cnt >= 0) {
    node = stack[cnt];
#ifdef PATRICIA_DEBUG
    log_dbg( "patricia_search_best: pop %s/%d", 
	     prefix_toa (node->prefix), node->prefix->bitlen);
#endif /* PATRICIA_DEBUG */
    if (comp_with_mask (prefix_tochar (node->prefix), 
			prefix_tochar (prefix),
			node->prefix->bitlen) && node->prefix->bitlen <= bitlen) {
#ifdef PATRICIA_DEBUG
      log_dbg( "patricia_search_best: found %s/%d", 
	       prefix_toa (node->prefix), node->prefix->bitlen);
#endif /* PATRICIA_DEBUG */
      return (node);
    }
  }
  return (NULL);
}


patricia_node_t *
patricia_search_best (patricia_tree_t *patricia, prefix_t *prefix)
{
  return (patricia_search_best2 (patricia, prefix, 1));
}


patricia_node_t *
patricia_lookup (patricia_tree_t *patricia, prefix_t *prefix)
{
  patricia_node_t *node, *new_node, *parent, *glue;
  u_char *addr, *test_addr;
  u_int bitlen, check_bit, differ_bit;
  int i, j, r;
  
  assert (patricia);
  assert (prefix);
  assert (prefix->bitlen <= patricia->maxbits);
  
  if (patricia->head == NULL) {
    node = calloc(1, sizeof *node);
    node->bit = prefix->bitlen;
    node->prefix = patricia_prefix_ref (prefix);
    node->parent = NULL;
    node->l = node->r = NULL;
    node->data = NULL;
    patricia->head = node;
#ifdef PATRICIA_DEBUG
    log_dbg( "patricia_lookup: new_node #0 %s/%d (head)", 
	     prefix_toa (prefix), prefix->bitlen);
#endif /* PATRICIA_DEBUG */
    patricia->num_active_node++;
    return (node);
  }
  
  addr = prefix_touchar (prefix);
  bitlen = prefix->bitlen;
  node = patricia->head;
  
  while (node->bit < bitlen || node->prefix == NULL) {
    
    if (node->bit < patricia->maxbits &&
	BIT_TEST (addr[node->bit >> 3], 0x80 >> (node->bit & 0x07))) {
      if (node->r == NULL)
	break;
#ifdef PATRICIA_DEBUG
      if (node->prefix) {
	log_dbg( "patricia_lookup: take right %s/%d", 
		 prefix_toa (node->prefix), node->prefix->bitlen);
      } else {
	log_dbg( "patricia_lookup: take right at %u", node->bit);
      }
#endif /* PATRICIA_DEBUG */
      node = node->r;
    }
    else {
      if (node->l == NULL)
	break;
#ifdef PATRICIA_DEBUG
      if (node->prefix) {
	log_dbg( "patricia_lookup: take left %s/%d", 
		 prefix_toa (node->prefix), node->prefix->bitlen);
      } else {
	log_dbg( "patricia_lookup: take left at %u", node->bit);
      }
#endif /* PATRICIA_DEBUG */
      node = node->l;
    }
    
    assert (node);
  }
  
  assert (node->prefix);
#ifdef PATRICIA_DEBUG
  log_dbg( "patricia_lookup: stop at %s/%d", 
	   prefix_toa (node->prefix), node->prefix->bitlen);
#endif /* PATRICIA_DEBUG */
  
  test_addr = prefix_touchar (node->prefix);
  /* find the first bit different */
  check_bit = (node->bit < bitlen)? node->bit: bitlen;
  differ_bit = 0;
  for (i = 0; i*8 < check_bit; i++) {
    if ((r = (addr[i] ^ test_addr[i])) == 0) {
      differ_bit = (i + 1) * 8;
      continue;
    }
    /* I know the better way, but for now */
    for (j = 0; j < 8; j++) {
      if (BIT_TEST (r, (0x80 >> j)))
	break;
    }
    /* must be found */
    assert (j < 8);
    differ_bit = i * 8 + j;
    break;
  }
  if (differ_bit > check_bit)
    differ_bit = check_bit;
#ifdef PATRICIA_DEBUG
  log_dbg( "patricia_lookup: differ_bit %d", differ_bit);
#endif /* PATRICIA_DEBUG */
  
  parent = node->parent;
  while (parent && parent->bit >= differ_bit) {
    node = parent;
    parent = node->parent;
#ifdef PATRICIA_DEBUG
    if (node->prefix) {
      log_dbg( "patricia_lookup: up to %s/%d", 
	       prefix_toa (node->prefix), node->prefix->bitlen);
    } else {
      log_dbg( "patricia_lookup: up to %u", node->bit);
    }
#endif /* PATRICIA_DEBUG */
  }
  
  if (differ_bit == bitlen && node->bit == bitlen) {
    if (node->prefix) {
#ifdef PATRICIA_DEBUG 
      log_dbg( "patricia_lookup: found %s/%d", 
	       prefix_toa (node->prefix), node->prefix->bitlen);
#endif /* PATRICIA_DEBUG */
      return (node);
	}
    node->prefix = patricia_prefix_ref (prefix);
#ifdef PATRICIA_DEBUG
    log_dbg( "patricia_lookup: new node #1 %s/%d (glue mod)",
	     prefix_toa (prefix), prefix->bitlen);
#endif /* PATRICIA_DEBUG */
    assert (node->data == NULL);
	return (node);
  }
  
  new_node = calloc(1, sizeof *new_node);
  new_node->bit = prefix->bitlen;
  new_node->prefix = patricia_prefix_ref (prefix);
  new_node->parent = NULL;
  new_node->l = new_node->r = NULL;
  new_node->data = NULL;
  patricia->num_active_node++;

  if (node->bit == differ_bit) {
    new_node->parent = node;
    if (node->bit < patricia->maxbits &&
	BIT_TEST (addr[node->bit >> 3], 0x80 >> (node->bit & 0x07))) {
      assert (node->r == NULL);
      node->r = new_node;
    }
    else {
      assert (node->l == NULL);
      node->l = new_node;
    }
#ifdef PATRICIA_DEBUG
    log_dbg( "patricia_lookup: new_node #2 %s/%d (child)", 
	     prefix_toa (prefix), prefix->bitlen);
#endif /* PATRICIA_DEBUG */
    return (new_node);
  }
  
  if (bitlen == differ_bit) {
    if (bitlen < patricia->maxbits &&
	BIT_TEST (test_addr[bitlen >> 3], 0x80 >> (bitlen & 0x07))) {
      new_node->r = node;
    }
    else {
      new_node->l = node;
    }
    new_node->parent = node->parent;
    if (node->parent == NULL) {
      assert (patricia->head == node);
      patricia->head = new_node;
    }
    else if (node->parent->r == node) {
      node->parent->r = new_node;
    }
    else {
      node->parent->l = new_node;
    }
    node->parent = new_node;
#ifdef PATRICIA_DEBUG
    log_dbg( "patricia_lookup: new_node #3 %s/%d (parent)", 
	     prefix_toa (prefix), prefix->bitlen);
#endif /* PATRICIA_DEBUG */
  }
  else {
    glue = calloc(1, sizeof *glue);
    glue->bit = differ_bit;
    glue->prefix = NULL;
    glue->parent = node->parent;
    glue->data = NULL;
    patricia->num_active_node++;
    if (differ_bit < patricia->maxbits &&
	BIT_TEST (addr[differ_bit >> 3], 0x80 >> (differ_bit & 0x07))) {
      glue->r = new_node;
      glue->l = node;
    }
    else {
      glue->r = node;
      glue->l = new_node;
    }
    new_node->parent = glue;
    
    if (node->parent == NULL) {
      assert (patricia->head == node);
      patricia->head = glue;
    }
    else if (node->parent->r == node) {
      node->parent->r = glue;
    }
    else {
      node->parent->l = glue;
    }
    node->parent = glue;
#ifdef PATRICIA_DEBUG
    log_dbg( "patricia_lookup: new_node #4 %s/%d (glue+node)", 
	     prefix_toa (prefix), prefix->bitlen);
#endif /* PATRICIA_DEBUG */
  }
  return (new_node);
}


void
patricia_remove (patricia_tree_t *patricia, patricia_node_t *node)
{
  patricia_node_t *parent, *child;
  
  assert (patricia);
  assert (node);
  
  if (node->r && node->l) {
#ifdef PATRICIA_DEBUG
    log_dbg( "patricia_remove: #0 %s/%d (r & l)", 
	     prefix_toa (node->prefix), node->prefix->bitlen);
#endif /* PATRICIA_DEBUG */
    
    /* this might be a placeholder node -- have to check and make sure
     * there is a prefix aossciated with it ! */
    if (node->prefix != NULL) 
      patricia_prefix_deref (node->prefix);
    node->prefix = NULL;
    /* Also I needed to clear data pointer -- masaki */
    node->data = NULL;
    return;
  }
  
  if (node->r == NULL && node->l == NULL) {
#ifdef PATRICIA_DEBUG
    log_dbg( "patricia_remove: #1 %s/%d (!r & !l)", 
	     prefix_toa (node->prefix), node->prefix->bitlen);
#endif /* PATRICIA_DEBUG */
    parent = node->parent;
    patricia_prefix_deref (node->prefix);
    free (node);
    patricia->num_active_node--;
    
    if (parent == NULL) {
      assert (patricia->head == node);
      patricia->head = NULL;
      return;
    }
    
    if (parent->r == node) {
      parent->r = NULL;
      child = parent->l;
    }
    else {
      assert (parent->l == node);
      parent->l = NULL;
      child = parent->r;
    }
    
    if (parent->prefix)
      return;
    
    /* we need to remove parent too */
    
    if (parent->parent == NULL) {
      assert (patricia->head == parent);
      patricia->head = child;
    }
    else if (parent->parent->r == parent) {
      parent->parent->r = child;
    }
    else {
      assert (parent->parent->l == parent);
      parent->parent->l = child;
    }
    child->parent = parent->parent;
    free (parent);
    patricia->num_active_node--;
    return;
  }
  
#ifdef PATRICIA_DEBUG
  log_dbg( "patricia_remove: #2 %s/%d (r ^ l)", 
	   prefix_toa (node->prefix), node->prefix->bitlen);
#endif /* PATRICIA_DEBUG */
  if (node->r) {
    child = node->r;
  }
  else {
    assert (node->l);
    child = node->l;
  }
  parent = node->parent;
  child->parent = parent;
  
  patricia_prefix_deref (node->prefix);
  free (node);
  patricia->num_active_node--;
  
  if (parent == NULL) {
    assert (patricia->head == node);
    patricia->head = child;
    return;
  }
  
  if (parent->r == node) {
    parent->r = child;
  }
  else {
    assert (parent->l == node);
    parent->l = child;
  }
}

patricia_node_t *
make_and_lookup (patricia_tree_t *tree, char *string)
{
  prefix_t *prefix;
  patricia_node_t *node;
  
  prefix = ascii2prefix (AF_INET, string);
  printf ("make_and_lookup: %s/%d", prefix_toa (prefix), prefix->bitlen);
  node = patricia_lookup (tree, prefix);
  patricia_prefix_deref (prefix);
  return (node);
}

patricia_node_t *
try_search_exact (patricia_tree_t *tree, char *string)
{
  prefix_t *prefix;
  patricia_node_t *node;
  
  prefix = ascii2prefix (AF_INET, string);
  printf ("try_search_exact: %s/%d", prefix_toa (prefix), prefix->bitlen);
  if ((node = patricia_search_exact (tree, prefix)) == NULL) {
    printf ("try_search_exact: not found");
  }
  else {
    printf ("try_search_exact: %s/%d found", 
	    prefix_toa (node->prefix), node->prefix->bitlen);
  }
  patricia_prefix_deref (prefix);
  return (node);
}

void
lookup_then_remove (patricia_tree_t *tree, char *string)
{
  patricia_node_t *node;
  
  if ((node = try_search_exact (tree, string)))
    patricia_remove (tree, node);
}

patricia_node_t *
try_search_best (patricia_tree_t *tree, char *string)
{
  prefix_t *prefix;
  patricia_node_t *node;
  
  prefix = ascii2prefix (AF_INET, string);
  printf ("try_search_best: %s/%d", prefix_toa (prefix), prefix->bitlen);
  if ((node = patricia_search_best (tree, prefix)) == NULL)
    printf ("try_search_best: not found");
  else
    printf ("try_search_best: %s/%d found", 
	    prefix_toa (node->prefix), node->prefix->bitlen);
  patricia_prefix_deref (prefix);
  return (node);
}
