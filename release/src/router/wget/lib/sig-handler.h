/* Convenience declarations when working with <signal.h>.

   Copyright (C) 2008-2024 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef _GL_SIG_HANDLER_H
#define _GL_SIG_HANDLER_H

/* This file uses _GL_INLINE_HEADER_BEGIN, _GL_INLINE, _GL_ATTRIBUTE_PURE.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <signal.h>

_GL_INLINE_HEADER_BEGIN
#ifndef SIG_HANDLER_INLINE
# define SIG_HANDLER_INLINE _GL_INLINE
#endif

/* Convenience type when working with signal handlers.  */
typedef void (*sa_handler_t) (int);

/* Return the handler of a signal, as a sa_handler_t value regardless
   of its true type.  The resulting function can be compared to
   special values like SIG_IGN but it is not portable to call it.  */
SIG_HANDLER_INLINE sa_handler_t _GL_ATTRIBUTE_PURE
get_handler (struct sigaction const *a)
{
  /* POSIX says that special values like SIG_IGN can only occur when
     action.sa_flags does not contain SA_SIGINFO.  But in Linux 2.4,
     for example, sa_sigaction and sa_handler are aliases and a signal
     is ignored if sa_sigaction (after casting) equals SIG_IGN.  In
     this case, this implementation relies on the fact that the two
     are aliases, and simply returns sa_handler.  */
  return a->sa_handler;
}

_GL_INLINE_HEADER_END

#endif /* _GL_SIG_HANDLER_H */
