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

/**
 * lookup()
 **/

#include "system.h"
#include <assert.h>

#ifdef HAVE_SFHASH
  extern uint32_t SuperFastHash(const char * data, int len, uint32_t hash);
#elif HAVE_LOOKUP3
#if LITTLE_ENDIAN
  extern uint32_t hashlittle(const void *key, size_t length, uint32_t initval);
#elif BIG_ENDIAN
  extern uint32_t hashbig(const void *key, size_t length, uint32_t initval);
#endif
#else
#error No hashing function found.
#endif

uint32_t lookup(uint8_t *k,  uint32_t length,  uint32_t initval)
{
#ifdef HAVE_SFHASH
  return SuperFastHash((const char*)k, length, initval);
#elif HAVE_LOOKUP3
#if LITTLE_ENDIAN
  return hashlittle(k, length, initval);
#elif BIG_ENDIAN
  return hashbig(k, length, initval);
#endif
#endif
}

