/* capngswig.i --
 * Copyright 2009 Red Hat Inc.
 * All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor
 * Boston, MA 02110-1335, USA.
 *
 * Authors:
 *   Steve Grubb <sgrubb@redhat.com>
 */

%module capng
%{
        #include "./capng.h"
%}

#if defined(SWIGPYTHON)

%varargs(16, signed capability = 0) capng_updatev;

%except(python) {
  $action
  if (result < 0) {
    PyErr_SetFromErrno(PyExc_OSError);
    return NULL;
  }
}
#endif

%define __signed__
signed
%enddef
#define __attribute(X) /*nothing*/
typedef unsigned __u32;
#define __extension__ /*nothing*/
%include "./caps.h"
%include "./capng.h"

