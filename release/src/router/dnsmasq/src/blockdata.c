/* dnsmasq is Copyright (c) 2000-2021 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991, or
   (at your option) version 3 dated 29 June, 2007.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
     
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dnsmasq.h"

static struct blockdata *keyblock_free;
static unsigned int blockdata_count, blockdata_hwm, blockdata_alloced;

static void blockdata_expand(int n)
{
  struct blockdata *new = whine_malloc(n * sizeof(struct blockdata));
  
  if (new)
    {
      int i;
      
      new[n-1].next = keyblock_free;
      keyblock_free = new;

      for (i = 0; i < n - 1; i++)
	new[i].next = &new[i+1];

      blockdata_alloced += n;
    }
}

/* Preallocate some blocks, proportional to cachesize, to reduce heap fragmentation. */
void blockdata_init(void)
{
  keyblock_free = NULL;
  blockdata_alloced = 0;
  blockdata_count = 0;
  blockdata_hwm = 0;

  /* Note that daemon->cachesize is enforced to have non-zero size if OPT_DNSSEC_VALID is set */  
  if (option_bool(OPT_DNSSEC_VALID))
    blockdata_expand(daemon->cachesize);
}

void blockdata_report(void)
{
  my_syslog(LOG_INFO, _("pool memory in use %u, max %u, allocated %u"), 
	    blockdata_count * sizeof(struct blockdata),  
	    blockdata_hwm * sizeof(struct blockdata),  
	    blockdata_alloced * sizeof(struct blockdata));
} 

static struct blockdata *blockdata_alloc_real(int fd, char *data, size_t len)
{
  struct blockdata *block, *ret = NULL;
  struct blockdata **prev = &ret;
  size_t blen;

  while (len > 0)
    {
      if (!keyblock_free)
	blockdata_expand(50);
      
      if (keyblock_free)
	{
	  block = keyblock_free;
	  keyblock_free = block->next;
	  blockdata_count++; 
	}
      else
	{
	  /* failed to alloc, free partial chain */
	  blockdata_free(ret);
	  return NULL;
	}
       
      if (blockdata_hwm < blockdata_count)
	blockdata_hwm = blockdata_count; 
      
      blen = len > KEYBLOCK_LEN ? KEYBLOCK_LEN : len;
      if (data)
	{
	  memcpy(block->key, data, blen);
	  data += blen;
	}
      else if (!read_write(fd, block->key, blen, 1))
	{
	  /* failed read free partial chain */
	  blockdata_free(ret);
	  return NULL;
	}
      len -= blen;
      *prev = block;
      prev = &block->next;
      block->next = NULL;
    }
  
  return ret;
}

struct blockdata *blockdata_alloc(char *data, size_t len)
{
  return blockdata_alloc_real(0, data, len);
}

void blockdata_free(struct blockdata *blocks)
{
  struct blockdata *tmp;
  
  if (blocks)
    {
      for (tmp = blocks; tmp->next; tmp = tmp->next)
	blockdata_count--;
      tmp->next = keyblock_free;
      keyblock_free = blocks; 
      blockdata_count--;
    }
}

/* if data == NULL, return pointer to static block of sufficient size */
void *blockdata_retrieve(struct blockdata *block, size_t len, void *data)
{
  size_t blen;
  struct  blockdata *b;
  void *new, *d;
  
  static unsigned int buff_len = 0;
  static unsigned char *buff = NULL;
   
  if (!data)
    {
      if (len > buff_len)
	{
	  if (!(new = whine_malloc(len)))
	    return NULL;
	  if (buff)
	    free(buff);
	  buff = new;
	}
      data = buff;
    }
  
  for (d = data, b = block; len > 0 && b;  b = b->next)
    {
      blen = len > KEYBLOCK_LEN ? KEYBLOCK_LEN : len;
      memcpy(d, b->key, blen);
      d += blen;
      len -= blen;
    }

  return data;
}


void blockdata_write(struct blockdata *block, size_t len, int fd)
{
  for (; len > 0 && block; block = block->next)
    {
      size_t blen = len > KEYBLOCK_LEN ? KEYBLOCK_LEN : len;
      read_write(fd, block->key, blen, 0);
      len -= blen;
    }
}

struct blockdata *blockdata_read(int fd, size_t len)
{
  return blockdata_alloc_real(fd, NULL, len);
}
