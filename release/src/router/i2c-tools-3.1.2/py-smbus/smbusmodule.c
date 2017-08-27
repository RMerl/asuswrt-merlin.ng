/*
 * smbusmodule.c - Python bindings for Linux SMBus access through i2c-dev
 * Copyright (C) 2005-2007 Mark M. Hoffman <mhoffman@lightlink.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <Python.h>
#include "structmember.h"
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>

/*
** These are required to build this module against Linux older than 2.6.23.
*/
#ifndef I2C_SMBUS_I2C_BLOCK_BROKEN
#undef I2C_SMBUS_I2C_BLOCK_DATA
#define I2C_SMBUS_I2C_BLOCK_BROKEN	6
#define I2C_SMBUS_I2C_BLOCK_DATA	8
#endif

PyDoc_STRVAR(SMBus_module_doc,
	"This module defines an object type that allows SMBus transactions\n"
	"on hosts running the Linux kernel.  The host kernel must have I2C\n"
	"support, I2C device interface support, and a bus adapter driver.\n"
	"All of these can be either built-in to the kernel, or loaded from\n"
	"modules.\n"
	"\n"
	"Because the I2C device interface is opened R/W, users of this\n"
	"module usually must have root permissions.\n");

typedef struct {
	PyObject_HEAD

	int fd;		/* open file descriptor: /dev/i2c-?, or -1 */
	int addr;	/* current client SMBus address */
	int pec;	/* !0 => Packet Error Codes enabled */
} SMBus;

static PyObject *
SMBus_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	SMBus *self;

	if ((self = (SMBus *)type->tp_alloc(type, 0)) == NULL)
		return NULL;

	self->fd = -1;
	self->addr = -1;
	self->pec = 0;

	return (PyObject *)self;
}

PyDoc_STRVAR(SMBus_close_doc,
	"close()\n\n"
	"Disconnects the object from the bus.\n");

static PyObject *
SMBus_close(SMBus *self)
{
	if ((self->fd != -1) && (close(self->fd) == -1)) {
		PyErr_SetFromErrno(PyExc_IOError);
		return NULL;
	}

	self->fd = -1;
	self->addr = -1;
	self->pec = 0;

	Py_INCREF(Py_None);
	return Py_None;
}

static void
SMBus_dealloc(SMBus *self)
{
	PyObject *ref = SMBus_close(self);
	Py_XDECREF(ref);

#if PY_MAJOR_VERSION >= 3
	Py_TYPE(self)->tp_free((PyObject *)self);
#else
	self->ob_type->tp_free((PyObject *)self);
#endif
}

#define MAXPATH 16

PyDoc_STRVAR(SMBus_open_doc,
	"open(bus)\n\n"
	"Connects the object to the specified SMBus.\n");

static PyObject *
SMBus_open(SMBus *self, PyObject *args, PyObject *kwds)
{
	int bus;
	char path[MAXPATH];

	static char *kwlist[] = {"bus", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i:open", kwlist, &bus))
		return NULL;

	if (snprintf(path, MAXPATH, "/dev/i2c-%d", bus) >= MAXPATH) {
		PyErr_SetString(PyExc_OverflowError,
			"Bus number is invalid.");
		return NULL;
	}

	if ((self->fd = open(path, O_RDWR, 0)) == -1) {
		PyErr_SetFromErrno(PyExc_IOError);
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static int
SMBus_init(SMBus *self, PyObject *args, PyObject *kwds)
{
	int bus = -1;

	static char *kwlist[] = {"bus", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:__init__",
			kwlist, &bus))
		return -1;

	if (bus >= 0) {
		SMBus_open(self, args, kwds);
		if (PyErr_Occurred())
			return -1;
	}

	return 0;
}

/*
 * private helper function, 0 => success, !0 => error
 */
static int
SMBus_set_addr(SMBus *self, int addr)
{
	int ret = 0;

	if (self->addr != addr) {
		ret = ioctl(self->fd, I2C_SLAVE, addr);
		self->addr = addr;
	}

	return ret;
}

#define SMBus_SET_ADDR(self, addr) do { \
	if (SMBus_set_addr(self, addr)) { \
		PyErr_SetFromErrno(PyExc_IOError); \
		return NULL; \
	} \
} while(0)

PyDoc_STRVAR(SMBus_write_quick_doc,
	"write_quick(addr)\n\n"
	"Perform SMBus Quick transaction.\n");

static PyObject *
SMBus_write_quick(SMBus *self, PyObject *args)
{
	int addr;
	__s32 result;

	if (!PyArg_ParseTuple(args, "i:write_quick", &addr))
		return NULL;

	SMBus_SET_ADDR(self, addr);

	if ((result = i2c_smbus_write_quick(self->fd, I2C_SMBUS_WRITE))) {
		PyErr_SetFromErrno(PyExc_IOError);
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

PyDoc_STRVAR(SMBus_read_byte_doc,
	"read_byte(addr) -> result\n\n"
	"Perform SMBus Read Byte transaction.\n");

static PyObject *
SMBus_read_byte(SMBus *self, PyObject *args)
{
	int addr;
	__s32 result;

	if (!PyArg_ParseTuple(args, "i:read_byte", &addr))
		return NULL;

	SMBus_SET_ADDR(self, addr);

	if ((result = i2c_smbus_read_byte(self->fd)) == -1) {
		PyErr_SetFromErrno(PyExc_IOError);
		return NULL;
	}

	return Py_BuildValue("l", (long)result);
}

PyDoc_STRVAR(SMBus_write_byte_doc,
	"write_byte(addr, val)\n\n"
	"Perform SMBus Write Byte transaction.\n");

static PyObject *
SMBus_write_byte(SMBus *self, PyObject *args)
{
	int addr, val;
	__s32 result;

	if (!PyArg_ParseTuple(args, "ii:write_byte", &addr, &val))
		return NULL;

	SMBus_SET_ADDR(self, addr);

	if ((result = i2c_smbus_write_byte(self->fd, (__u8)val)) == -1) {
		PyErr_SetFromErrno(PyExc_IOError);
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

PyDoc_STRVAR(SMBus_read_byte_data_doc,
	"read_byte_data(addr, cmd) -> result\n\n"
	"Perform SMBus Read Byte Data transaction.\n");

static PyObject *
SMBus_read_byte_data(SMBus *self, PyObject *args)
{
	int addr, cmd;
	__s32 result;

	if (!PyArg_ParseTuple(args, "ii:read_byte_data", &addr, &cmd))
		return NULL;

	SMBus_SET_ADDR(self, addr);

	if ((result = i2c_smbus_read_byte_data(self->fd, (__u8)cmd)) == -1) {
		PyErr_SetFromErrno(PyExc_IOError);
		return NULL;
	}

	return Py_BuildValue("l", (long)result);
}

PyDoc_STRVAR(SMBus_write_byte_data_doc,
	"write_byte_data(addr, cmd, val)\n\n"
	"Perform SMBus Write Byte Data transaction.\n");

static PyObject *
SMBus_write_byte_data(SMBus *self, PyObject *args)
{
	int addr, cmd, val;
	__s32 result;

	if (!PyArg_ParseTuple(args, "iii:write_byte_data", &addr, &cmd, &val))
		return NULL;

	SMBus_SET_ADDR(self, addr);

	if ((result = i2c_smbus_write_byte_data(self->fd,
				(__u8)cmd, (__u8)val)) == -1) {
		PyErr_SetFromErrno(PyExc_IOError);
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

PyDoc_STRVAR(SMBus_read_word_data_doc,
	"read_word_data(addr, cmd) -> result\n\n"
	"Perform SMBus Read Word Data transaction.\n");

static PyObject *
SMBus_read_word_data(SMBus *self, PyObject *args)
{
	int addr, cmd;
	__s32 result;

	if (!PyArg_ParseTuple(args, "ii:read_word_data", &addr, &cmd))
		return NULL;

	SMBus_SET_ADDR(self, addr);

	if ((result = i2c_smbus_read_word_data(self->fd, (__u8)cmd)) == -1) {
		PyErr_SetFromErrno(PyExc_IOError);
		return NULL;
	}

	return Py_BuildValue("l", (long)result);
}

PyDoc_STRVAR(SMBus_write_word_data_doc,
	"write_word_data(addr, cmd, val)\n\n"
	"Perform SMBus Write Word Data transaction.\n");

static PyObject *
SMBus_write_word_data(SMBus *self, PyObject *args)
{
	int addr, cmd, val;
	__s32 result;

	if (!PyArg_ParseTuple(args, "iii:write_word_data", &addr, &cmd, &val))
		return NULL;

	SMBus_SET_ADDR(self, addr);

	if ((result = i2c_smbus_write_word_data(self->fd,
				(__u8)cmd, (__u16)val)) == -1) {
		PyErr_SetFromErrno(PyExc_IOError);
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

PyDoc_STRVAR(SMBus_process_call_doc,
	"process_call(addr, cmd, val)\n\n"
	"Perform SMBus Process Call transaction.\n");

static PyObject *
SMBus_process_call(SMBus *self, PyObject *args)
{
	int addr, cmd, val;
	__s32 result;

	if (!PyArg_ParseTuple(args, "iii:process_call", &addr, &cmd, &val))
		return NULL;

	SMBus_SET_ADDR(self, addr);

	if ((result = i2c_smbus_process_call(self->fd,
				(__u8)cmd, (__u16)val)) == -1) {
		PyErr_SetFromErrno(PyExc_IOError);
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

/*
 * private helper function; returns a new list of integers
 */
static PyObject *
SMBus_buf_to_list(__u8 const *buf, int len)
{
	PyObject *list = PyList_New(len);
	int ii;

	if (list == NULL)
		return NULL;

	for (ii = 0; ii < len; ii++) {
		PyObject *val = Py_BuildValue("l", (long)buf[ii]);
		PyList_SET_ITEM(list, ii, val);
	}
	return list;
}

PyDoc_STRVAR(SMBus_read_block_data_doc,
	"read_block_data(addr, cmd) -> results\n\n"
	"Perform SMBus Read Block Data transaction.\n");

static PyObject *
SMBus_read_block_data(SMBus *self, PyObject *args)
{
	int addr, cmd;
	union i2c_smbus_data data;

	if (!PyArg_ParseTuple(args, "ii:read_block_data", &addr, &cmd))
		return NULL;

	SMBus_SET_ADDR(self, addr);

	/* save a bit of code by calling the access function directly */
	if (i2c_smbus_access(self->fd, I2C_SMBUS_READ, (__u8)cmd,
				I2C_SMBUS_BLOCK_DATA, &data)) {
		PyErr_SetFromErrno(PyExc_IOError);
		return NULL;
	}

	/* first byte of the block contains (remaining) data length */
	return SMBus_buf_to_list(&data.block[1], data.block[0]);
}

/*
 * private helper function: convert an integer list to union i2c_smbus_data
 */
static int
SMBus_list_to_data(PyObject *list, union i2c_smbus_data *data)
{
	static char *msg = "Third argument must be a list of at least one, "
				"but not more than 32 integers";
	int ii, len;

	if (!PyList_Check(list)) {
		PyErr_SetString(PyExc_TypeError, msg);
		return 0; /* fail */
	}

	if ((len = PyList_GET_SIZE(list)) > 32) {
		PyErr_SetString(PyExc_OverflowError, msg);
		return 0; /* fail */
	}

	/* first byte is the length */
	data->block[0] = (__u8)len;

	for (ii = 0; ii < len; ii++) {
		PyObject *val = PyList_GET_ITEM(list, ii);
#if PY_MAJOR_VERSION >= 3
		if (!PyLong_Check(val)) {
			PyErr_SetString(PyExc_TypeError, msg);
			return 0; /* fail */
		}
		data->block[ii+1] = (__u8)PyLong_AS_LONG(val);
#else
		if (!PyInt_Check(val)) {
			PyErr_SetString(PyExc_TypeError, msg);
			return 0; /* fail */
		}
		data->block[ii+1] = (__u8)PyInt_AS_LONG(val);
#endif
	}

	return 1; /* success */
}

PyDoc_STRVAR(SMBus_write_block_data_doc,
	"write_block_data(addr, cmd, [vals])\n\n"
	"Perform SMBus Write Block Data transaction.\n");

static PyObject *
SMBus_write_block_data(SMBus *self, PyObject *args)
{
	int addr, cmd;
	union i2c_smbus_data data;

	if (!PyArg_ParseTuple(args, "iiO&:write_block_data", &addr, &cmd,
				SMBus_list_to_data, &data))
		return NULL;

	SMBus_SET_ADDR(self, addr);

	/* save a bit of code by calling the access function directly */
	if (i2c_smbus_access(self->fd, I2C_SMBUS_WRITE, (__u8)cmd,
				I2C_SMBUS_BLOCK_DATA, &data)) {
		PyErr_SetFromErrno(PyExc_IOError);
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

PyDoc_STRVAR(SMBus_block_process_call_doc,
	"block_process_call(addr, cmd, [vals]) -> results\n\n"
	"Perform SMBus Block Process Call transaction.\n");

static PyObject *
SMBus_block_process_call(SMBus *self, PyObject *args)
{
	int addr, cmd;
	union i2c_smbus_data data;

	if (!PyArg_ParseTuple(args, "iiO&:block_process_call", &addr, &cmd,
			SMBus_list_to_data, &data))
		return NULL;

	SMBus_SET_ADDR(self, addr);

	/* save a bit of code by calling the access function directly */
	if (i2c_smbus_access(self->fd, I2C_SMBUS_WRITE, (__u8)cmd,
				I2C_SMBUS_BLOCK_PROC_CALL, &data)) {
		PyErr_SetFromErrno(PyExc_IOError);
		return NULL;
	}

	/* first byte of the block contains (remaining) data length */
	return SMBus_buf_to_list(&data.block[1], data.block[0]);
}

PyDoc_STRVAR(SMBus_read_i2c_block_data_doc,
	"read_i2c_block_data(addr, cmd, len=32) -> results\n\n"
	"Perform I2C Block Read transaction.\n");

static PyObject *
SMBus_read_i2c_block_data(SMBus *self, PyObject *args)
{
	int addr, cmd, len=32;
	union i2c_smbus_data data;

	if (!PyArg_ParseTuple(args, "ii|i:read_i2c_block_data", &addr, &cmd,
			&len))
		return NULL;

	SMBus_SET_ADDR(self, addr);

	data.block[0] = len;
	/* save a bit of code by calling the access function directly */
	if (i2c_smbus_access(self->fd, I2C_SMBUS_READ, (__u8)cmd,
				len == 32 ? I2C_SMBUS_I2C_BLOCK_BROKEN:
				I2C_SMBUS_I2C_BLOCK_DATA, &data)) {
		PyErr_SetFromErrno(PyExc_IOError);
		return NULL;
	}

	/* first byte of the block contains (remaining) data length */
	return SMBus_buf_to_list(&data.block[1], data.block[0]);
}

PyDoc_STRVAR(SMBus_write_i2c_block_data_doc,
	"write_i2c_block_data(addr, cmd, [vals])\n\n"
	"Perform I2C Block Write transaction.\n");

static PyObject *
SMBus_write_i2c_block_data(SMBus *self, PyObject *args)
{
	int addr, cmd;
	union i2c_smbus_data data;

	if (!PyArg_ParseTuple(args, "iiO&:write_i2c_block_data", &addr, &cmd,
			SMBus_list_to_data, &data))
		return NULL;

	SMBus_SET_ADDR(self, addr);

	/* save a bit of code by calling the access function directly */
	if (i2c_smbus_access(self->fd, I2C_SMBUS_WRITE, (__u8)cmd,
				I2C_SMBUS_I2C_BLOCK_BROKEN, &data)) {
		PyErr_SetFromErrno(PyExc_IOError);
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

PyDoc_STRVAR(SMBus_type_doc,
	"SMBus([bus]) -> SMBus\n\n"
	"Return a new SMBus object that is (optionally) connected to the\n"
	"specified I2C device interface.\n");

static PyMethodDef SMBus_methods[] = {
	{"open", (PyCFunction)SMBus_open, METH_VARARGS | METH_KEYWORDS,
		SMBus_open_doc},
	{"close", (PyCFunction)SMBus_close, METH_NOARGS,
		SMBus_close_doc},
	{"write_quick", (PyCFunction)SMBus_write_quick, METH_VARARGS,
		SMBus_write_quick_doc},
	{"read_byte", (PyCFunction)SMBus_read_byte, METH_VARARGS,
		SMBus_read_byte_doc},
	{"write_byte", (PyCFunction)SMBus_write_byte, METH_VARARGS,
		SMBus_write_byte_doc},
	{"read_byte_data", (PyCFunction)SMBus_read_byte_data, METH_VARARGS,
		SMBus_read_byte_data_doc},
	{"write_byte_data", (PyCFunction)SMBus_write_byte_data, METH_VARARGS,
		SMBus_write_byte_data_doc},
	{"read_word_data", (PyCFunction)SMBus_read_word_data, METH_VARARGS,
		SMBus_read_word_data_doc},
	{"write_word_data", (PyCFunction)SMBus_write_word_data, METH_VARARGS,
		SMBus_write_word_data_doc},
	{"process_call", (PyCFunction)SMBus_process_call, METH_VARARGS,
		SMBus_process_call_doc},
	{"read_block_data", (PyCFunction)SMBus_read_block_data, METH_VARARGS,
		SMBus_read_block_data_doc},
	{"write_block_data", (PyCFunction)SMBus_write_block_data, METH_VARARGS,
		SMBus_write_block_data_doc},
	{"block_process_call", (PyCFunction)SMBus_block_process_call,
		METH_VARARGS, SMBus_block_process_call_doc},
	{"read_i2c_block_data", (PyCFunction)SMBus_read_i2c_block_data,
		METH_VARARGS, SMBus_read_i2c_block_data_doc},
	{"write_i2c_block_data", (PyCFunction)SMBus_write_i2c_block_data,
		METH_VARARGS, SMBus_write_i2c_block_data_doc},
	{NULL},
};

static PyObject *
SMBus_get_pec(SMBus *self, void *closure)
{
	PyObject *result = self->pec ? Py_True : Py_False;
	Py_INCREF(result);
	return result;
}

static int
SMBus_set_pec(SMBus *self, PyObject *val, void *closure)
{
	int pec;

	pec = PyObject_IsTrue(val);

	if (val == NULL) {
		PyErr_SetString(PyExc_TypeError,
			"Cannot delete attribute");
		return -1;
	}
	else if (pec == -1) {
		PyErr_SetString(PyExc_TypeError, 
			"The pec attribute must be a boolean.");
		return -1;
	}

	if (self->pec != pec) {
		if (ioctl(self->fd, I2C_PEC, pec)) {
			PyErr_SetFromErrno(PyExc_IOError);
			return -1;
		}
		self->pec = pec;
	}

	return 0;
}

static PyGetSetDef SMBus_getset[] = {
	{"pec", (getter)SMBus_get_pec, (setter)SMBus_set_pec,
			"True if Packet Error Codes (PEC) are enabled"},
	{NULL},
};

static PyTypeObject SMBus_type = {
#if PY_MAJOR_VERSION >= 3
	PyVarObject_HEAD_INIT(NULL, 0)
	"SMBus",			/* tp_name */
#else
	PyObject_HEAD_INIT(NULL)
	0,				/* ob_size */
	"smbus.SMBus",			/* tp_name */
#endif
	sizeof(SMBus),			/* tp_basicsize */
	0,				/* tp_itemsize */
	(destructor)SMBus_dealloc,	/* tp_dealloc */
	0,				/* tp_print */
	0,				/* tp_getattr */
	0,				/* tp_setattr */
	0,				/* tp_compare */
	0,				/* tp_repr */
	0,				/* tp_as_number */
	0,				/* tp_as_sequence */
	0,				/* tp_as_mapping */
	0,				/* tp_hash */
	0,				/* tp_call */
	0,				/* tp_str */
	0,				/* tp_getattro */
	0,				/* tp_setattro */
	0,				/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,		/* tp_flags */
	SMBus_type_doc,			/* tp_doc */
	0,				/* tp_traverse */
	0,				/* tp_clear */
	0,				/* tp_richcompare */
	0,				/* tp_weaklistoffset */
	0,				/* tp_iter */
	0,				/* tp_iternext */
	SMBus_methods,			/* tp_methods */
	0,				/* tp_members */
	SMBus_getset,			/* tp_getset */
	0,				/* tp_base */
	0,				/* tp_dict */
	0,				/* tp_descr_get */
	0,				/* tp_descr_set */
	0,				/* tp_dictoffset */
	(initproc)SMBus_init,		/* tp_init */
	0,				/* tp_alloc */
	SMBus_new,			/* tp_new */
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef SMBusModule = {
	PyModuleDef_HEAD_INIT,
	"SMBus",			/* m_name */
	SMBus_module_doc,  		/* m_doc */
	-1,				/* m_size */
	NULL,				/* m_methods */
	NULL,				/* m_reload */
	NULL,				/* m_traverse */
	NULL,				/* m_clear */
	NULL,				/* m_free */
};
#define INIT_RETURN(m)	return m
#define INIT_FNAME	PyInit_smbus
#else
static PyMethodDef SMBus_module_methods[] = {
	{NULL}
};
#define INIT_RETURN(m)	return
#define INIT_FNAME	initsmbus
#endif

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
INIT_FNAME(void)
{
	PyObject* m;

	if (PyType_Ready(&SMBus_type) < 0)
		INIT_RETURN(NULL);

#if PY_MAJOR_VERSION >= 3
	m = PyModule_Create(&SMBusModule);
#else
	m = Py_InitModule3("smbus", SMBus_module_methods, SMBus_module_doc);
#endif
	if (m == NULL)
		INIT_RETURN(NULL);

	Py_INCREF(&SMBus_type);
	PyModule_AddObject(m, "SMBus", (PyObject *)&SMBus_type);

	INIT_RETURN(m);
}

