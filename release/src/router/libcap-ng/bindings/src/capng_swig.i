/* capngswig.i --
 * Copyright 2009 Red Hat Inc., Durham, North Carolina.
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
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

