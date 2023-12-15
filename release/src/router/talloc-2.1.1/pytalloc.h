/* 
   Unix SMB/CIFS implementation.
   Samba utility functions
   Copyright (C) Jelmer Vernooij <jelmer@samba.org> 2008
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _PYTALLOC_H_
#define _PYTALLOC_H_

#include <Python.h>
#include <talloc.h>

typedef struct {
	PyObject_HEAD
	TALLOC_CTX *talloc_ctx;
	void *ptr;
} pytalloc_Object;

/* Return the PyTypeObject for pytalloc_Object. Returns a new reference. */
PyTypeObject *pytalloc_GetObjectType(void);

/* Check whether a specific object is a talloc Object. */
int pytalloc_Check(PyObject *);

/* Retrieve the pointer for a pytalloc_object. Like talloc_get_type() 
 * but for pytalloc_Objects. */

/* FIXME: Call PyErr_SetString(PyExc_TypeError, "expected " __STR(type) ") 
 * when talloc_get_type() returns NULL. */
#define pytalloc_get_type(py_obj, type) (talloc_get_type(pytalloc_get_ptr(py_obj), type))

#define pytalloc_get_ptr(py_obj) (((pytalloc_Object *)py_obj)->ptr)
#define pytalloc_get_mem_ctx(py_obj)  ((pytalloc_Object *)py_obj)->talloc_ctx

PyObject *pytalloc_steal_ex(PyTypeObject *py_type, TALLOC_CTX *mem_ctx, void *ptr);
PyObject *pytalloc_steal(PyTypeObject *py_type, void *ptr);
PyObject *pytalloc_reference_ex(PyTypeObject *py_type, TALLOC_CTX *mem_ctx, void *ptr);
#define pytalloc_reference(py_type, talloc_ptr) pytalloc_reference_ex(py_type, talloc_ptr, talloc_ptr)

#define pytalloc_new(type, typeobj) pytalloc_steal(typeobj, talloc_zero(NULL, type))

PyObject *pytalloc_CObject_FromTallocPtr(void *);

#endif /* _PYTALLOC_H_ */
