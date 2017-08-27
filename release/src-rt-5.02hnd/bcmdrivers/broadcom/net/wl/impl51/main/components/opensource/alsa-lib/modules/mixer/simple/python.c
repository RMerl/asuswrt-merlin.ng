/*
 *  Mixer Interface - python binding simple abstact module
 *  Copyright (c) 2007 by Jaroslav Kysela <perex@perex.cz>
 *
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include "Python.h"
#include <stddef.h>
#include "config.h"
#include "asoundlib.h"
#include "mixer_abst.h"

struct python_priv {
	int py_initialized;
	PyObject *py_event_func;
	PyObject *py_mdict;
	PyObject *py_mixer;
};

#define SCRIPT ALSA_PLUGIN_DIR "/smixer/python/main.py"

struct pymelem {
	PyObject_HEAD
	sm_selem_t selem;
	PyObject *py_mixer;
	snd_mixer_elem_t *melem;
};

struct pymixer {
	PyObject_HEAD
	snd_mixer_class_t *class;
	snd_mixer_t *mixer;
	PyObject *mdict;
	int hctl_count;
	void **hctl;
	int helem_count;
	void **helem;
	int melem_count;
	void **melem;
};

static PyInterpreterState *main_interpreter;

static void *get_C_ptr(PyObject *obj, const char *attr)
{
	PyObject *o;

	o = PyObject_GetAttr(obj, PyString_InternFromString(attr));
	if (!o) {
		PyErr_Format(PyExc_TypeError, "missing '%s' attribute", attr);
		return NULL;
	}
	if (!PyInt_Check(o)) {
		PyErr_Format(PyExc_TypeError, "'%s' attribute is not integer", attr);
		return NULL;
	}
	return (void *)PyInt_AsLong(o);
}

static struct pymelem *melem_to_pymelem(snd_mixer_elem_t *elem)
{
	return (struct pymelem *)((char *)snd_mixer_elem_get_private(elem) - offsetof(struct pymelem, selem));
}

static int pcall(struct pymelem *pymelem, const char *attr, PyObject *args, PyObject **_res)
{
	PyObject *obj = (PyObject *)pymelem, *res;
	int xres = 0;

	if (_res)
		*_res = NULL;
	obj = PyObject_GetAttr(obj, PyString_InternFromString(attr));
	if (!obj) {
		PyErr_Format(PyExc_TypeError, "missing '%s' attribute", attr);
		PyErr_Print();
		PyErr_Clear();
		Py_DECREF(args);
		return -EIO;
	}
	res = PyObject_CallObject(obj, args);
	Py_XDECREF(args);
	if (res == NULL) {
		PyErr_Print();
		PyErr_Clear();
		return -EIO;
	}
	if (_res && PyTuple_Check(res)) {
		*_res = res;
		res = PyTuple_GetItem(res, 0);
	}
	if (PyInt_Check(res)) {
		xres = PyInt_AsLong(res);
	} else if (res == Py_None) {
		xres = 0;
	} else if (PyBool_Check(res)) {
		xres = res == Py_True;
	} else {
		PyErr_Format(PyExc_TypeError, "wrong result from '%s'!", attr);
		PyErr_Print();
		PyErr_Clear();
		Py_DECREF(res);
		if (_res)
			*_res = NULL;
		return -EIO;
	}
	if (_res && *_res)
		return xres;
	Py_DECREF(res);
	return xres;
}

static int is_ops(snd_mixer_elem_t *elem, int dir, int cmd, int val)
{
	PyObject *obj1;
	struct pymelem *pymelem = melem_to_pymelem(elem);
	char *s, fcn[32] = "opsIs";
	int res, xdir = 1, xval = 0;

	switch (cmd) {
	case SM_OPS_IS_ACTIVE: 	s = "Active"; xdir = 0; break;
	case SM_OPS_IS_MONO:	s = "Mono"; break;
	case SM_OPS_IS_CHANNEL:	s = "Channel"; xval = 1; break;
	case SM_OPS_IS_ENUMERATED: s = "Enumerated"; xdir = val == 1; break;
	case SM_OPS_IS_ENUMCNT:	s = "EnumCnt"; break;
	default:
		return 1;
	}
	strcat(fcn, s);

	obj1 = PyTuple_New(xdir + xval);
	if (xdir) {
		PyTuple_SET_ITEM(obj1, 0, PyInt_FromLong(dir));
		if (xval)
			PyTuple_SET_ITEM(obj1, 1, PyInt_FromLong(val));
	}
	res = pcall(pymelem, fcn, obj1, NULL);
	return res < 0 ? 0 : res;
}

static int get_x_range_ops(snd_mixer_elem_t *elem, int dir,
                           long *min, long *max, const char *attr)
{
	PyObject *obj1, *res;
	struct pymelem *pymelem = melem_to_pymelem(elem);
	int err;
	
	obj1 = PyTuple_New(1);
	PyTuple_SET_ITEM(obj1, 0, PyInt_FromLong(dir));
	err = pcall(pymelem, attr, obj1, &res);
	if (err >= 0) {
		err = !PyInt_Check(PyTuple_GetItem(res, 1)) || !PyInt_Check(PyTuple_GetItem(res, 2));
		if (err) {
			err = !PyLong_Check(PyTuple_GetItem(res, 1)) || !PyLong_Check(PyTuple_GetItem(res, 2));
			if (err) {
				PyErr_Format(PyExc_TypeError, "wrong result (invalid tuple)");
				PyErr_Print();
				PyErr_Clear();
				err = -EIO;
			} else {
				*min = PyLong_AsLong(PyTuple_GetItem(res, 1));
				*max = PyLong_AsLong(PyTuple_GetItem(res, 2));
			}
		} else {
			*min = PyInt_AsLong(PyTuple_GetItem(res, 1));
			*max = PyInt_AsLong(PyTuple_GetItem(res, 2));
		}
	}
	Py_XDECREF(res);
	return err;
}

static int get_range_ops(snd_mixer_elem_t *elem, int dir,
                         long *min, long *max)
{
	return get_x_range_ops(elem, dir, min, max, "opsGetRange");
}

static int set_range_ops(snd_mixer_elem_t *elem, int dir,
                         long min, long max)
{
	PyObject *obj1;
	struct pymelem *pymelem = melem_to_pymelem(elem);

	obj1 = PyTuple_New(3);
	PyTuple_SET_ITEM(obj1, 0, PyInt_FromLong(dir));
	PyTuple_SET_ITEM(obj1, 1, PyInt_FromLong(min));
	PyTuple_SET_ITEM(obj1, 2, PyInt_FromLong(max));
	return pcall(pymelem, "opsGetRange", obj1, NULL);
}

static int get_x_ops(snd_mixer_elem_t *elem, int dir,
                     long channel, long *value,
                     const char *attr)
{
	PyObject *obj1, *res;
	struct pymelem *pymelem = melem_to_pymelem(elem);
	int err;
	
	obj1 = PyTuple_New(2);
	PyTuple_SET_ITEM(obj1, 0, PyInt_FromLong(dir));
	PyTuple_SET_ITEM(obj1, 1, PyInt_FromLong(channel));
	err = pcall(pymelem, attr, obj1, &res);
	if (err >= 0) {
		err = !PyInt_Check(PyTuple_GetItem(res, 1));
		if (err) {
			err = !PyLong_Check(PyTuple_GetItem(res, 1));
			if (err) {
				PyErr_Format(PyExc_TypeError, "wrong result (invalid tuple)");
				PyErr_Print();
				PyErr_Clear();
				err = -EIO;
			} else {
				*value = PyLong_AsLong(PyTuple_GetItem(res, 1));
			}
		} else {
			*value = PyInt_AsLong(PyTuple_GetItem(res, 1));
		}
	}
	Py_XDECREF(res);
	return err;
}

static int get_volume_ops(snd_mixer_elem_t *elem, int dir,
			  snd_mixer_selem_channel_id_t channel, long *value)
{
	return get_x_ops(elem, dir, channel, value, "opsGetVolume");
}

static int get_switch_ops(snd_mixer_elem_t *elem, int dir,
                          snd_mixer_selem_channel_id_t channel, int *value)
{
	long value1;
	int res;
	res = get_x_ops(elem, dir, channel, &value1, "opsGetSwitch");
	*value = value1;
	return res;
}

static int ask_vol_dB_ops(snd_mixer_elem_t *elem,
			  int dir,
			  long value,
			  long *dbValue)
{
	return get_x_ops(elem, dir, value, dbValue, "opsGetVolDB");
}

static int ask_dB_vol_ops(snd_mixer_elem_t *elem,
			  int dir,
			  long value,
			  long *dbValue,
			  int xdir)
{
	PyObject *obj1, *res;
	struct pymelem *pymelem = melem_to_pymelem(elem);
	int err;
	
	obj1 = PyTuple_New(3);
	PyTuple_SET_ITEM(obj1, 0, PyInt_FromLong(dir));
	PyTuple_SET_ITEM(obj1, 1, PyInt_FromLong(value));
	PyTuple_SET_ITEM(obj1, 2, PyInt_FromLong(xdir));
	err = pcall(pymelem, "opsGetDBVol", obj1, &res);
	if (err >= 0) {
		err = !PyInt_Check(PyTuple_GetItem(res, 1));
		if (err) {
			err = !PyLong_Check(PyTuple_GetItem(res, 1));
			if (err) {
				PyErr_Format(PyExc_TypeError, "wrong result (invalid tuple)");
				PyErr_Print();
				PyErr_Clear();
				err = -EIO;
			} else {
				*dbValue = PyLong_AsLong(PyTuple_GetItem(res, 1));
			}
		} else {
			*dbValue = PyInt_AsLong(PyTuple_GetItem(res, 1));
		}
	}
	Py_XDECREF(res);
	return err;
}

static int get_dB_ops(snd_mixer_elem_t *elem,
                      int dir,
                      snd_mixer_selem_channel_id_t channel,
                      long *value)
{
	return get_x_ops(elem, dir, channel, value, "opsGetDB");
}

static int get_dB_range_ops(snd_mixer_elem_t *elem, int dir,
                            long *min, long *max)
{
	return get_x_range_ops(elem, dir, min, max, "opsGetDBRange");
}

static int set_volume_ops(snd_mixer_elem_t *elem, int dir,
                          snd_mixer_selem_channel_id_t channel, long value)
{
	PyObject *obj1;
	struct pymelem *pymelem = melem_to_pymelem(elem);

	obj1 = PyTuple_New(3);
	PyTuple_SET_ITEM(obj1, 0, PyInt_FromLong(dir));
	PyTuple_SET_ITEM(obj1, 1, PyInt_FromLong(channel));
	PyTuple_SET_ITEM(obj1, 2, PyInt_FromLong(value));
	return pcall(pymelem, "opsSetVolume", obj1, NULL);
}

static int set_switch_ops(snd_mixer_elem_t *elem, int dir,
                          snd_mixer_selem_channel_id_t channel, int value)
{
	PyObject *obj1;
	struct pymelem *pymelem = melem_to_pymelem(elem);

	obj1 = PyTuple_New(3);
	PyTuple_SET_ITEM(obj1, 0, PyInt_FromLong(dir));
	PyTuple_SET_ITEM(obj1, 1, PyInt_FromLong(channel));
	PyTuple_SET_ITEM(obj1, 2, PyInt_FromLong(value));
	return pcall(pymelem, "opsSetSwitch", obj1, NULL);
}

static int set_dB_ops(snd_mixer_elem_t *elem, int dir,
                      snd_mixer_selem_channel_id_t channel,
                      long db_gain, int xdir)
{
	PyObject *obj1;
	struct pymelem *pymelem = melem_to_pymelem(elem);

	obj1 = PyTuple_New(4);
	PyTuple_SET_ITEM(obj1, 0, PyInt_FromLong(dir));
	PyTuple_SET_ITEM(obj1, 1, PyInt_FromLong(channel));
	PyTuple_SET_ITEM(obj1, 2, PyInt_FromLong(db_gain));
	PyTuple_SET_ITEM(obj1, 3, PyInt_FromLong(xdir));
	return pcall(pymelem, "opsSetDB", obj1, NULL);
}

static int enum_item_name_ops(snd_mixer_elem_t *elem,
                              unsigned int item,
                              size_t maxlen, char *buf)
{
	PyObject *obj1, *res;
	struct pymelem *pymelem = melem_to_pymelem(elem);
	int err;
	unsigned int len;
	char *s;
	
	obj1 = PyTuple_New(1);
	PyTuple_SET_ITEM(obj1, 0, PyInt_FromLong(item));
	err = pcall(pymelem, "opsGetEnumItemName", obj1, &res);
	if (err >= 0) {
		err = !PyString_Check(PyTuple_GetItem(res, 1));
		if (err) {
			PyErr_Format(PyExc_TypeError, "wrong result (invalid tuple)");
			PyErr_Print();
			PyErr_Clear();
			err = -EIO;
		} else {
			s = PyString_AsString(PyTuple_GetItem(res, 1));
			len = strlen(s);
			if (maxlen - 1 > len)
				len = maxlen - 1;
			memcpy(buf, s, len);
			buf[len] = '\0';
		}
	}
	Py_XDECREF(res);
	return err;
}

static int get_enum_item_ops(snd_mixer_elem_t *elem,
                             snd_mixer_selem_channel_id_t channel,
                             unsigned int *itemp)
{
	PyObject *obj1, *res;
	struct pymelem *pymelem = melem_to_pymelem(elem);
	int err;
	
	obj1 = PyTuple_New(1);
	PyTuple_SET_ITEM(obj1, 0, PyInt_FromLong(channel));
	err = pcall(pymelem, "opsGetEnumItem", obj1, &res);
	if (err >= 0) {
		err = !PyInt_Check(PyTuple_GetItem(res, 1));
		if (err) {
			PyErr_Format(PyExc_TypeError, "wrong result (invalid tuple)");
			PyErr_Print();
			PyErr_Clear();
			err = -EIO;
		} else {
			*itemp = PyInt_AsLong(PyTuple_GetItem(res, 1));
		}
	}
	Py_XDECREF(res);
	return err;
}

static int set_enum_item_ops(snd_mixer_elem_t *elem,
                             snd_mixer_selem_channel_id_t channel,
                             unsigned int item)
{
	PyObject *obj1;
	struct pymelem *pymelem = melem_to_pymelem(elem);

	obj1 = PyTuple_New(2);
	PyTuple_SET_ITEM(obj1, 0, PyInt_FromLong(channel));
	PyTuple_SET_ITEM(obj1, 1, PyInt_FromLong(item));
	return pcall(pymelem, "opsSetEnumItem", obj1, NULL);
}

static struct sm_elem_ops simple_python_ops = {
        .is             = is_ops,
        .get_range      = get_range_ops,
        .get_dB_range   = get_dB_range_ops,
        .set_range      = set_range_ops,
        .ask_vol_dB	= ask_vol_dB_ops,
        .ask_dB_vol	= ask_dB_vol_ops,
        .get_volume     = get_volume_ops,
        .get_dB         = get_dB_ops,
        .set_volume     = set_volume_ops,
        .set_dB         = set_dB_ops,
        .get_switch     = get_switch_ops,
        .set_switch     = set_switch_ops,
        .enum_item_name = enum_item_name_ops,
        .get_enum_item  = get_enum_item_ops,
        .set_enum_item  = set_enum_item_ops
};

static void selem_free(snd_mixer_elem_t *elem)
{
	sm_selem_t *simple = snd_mixer_elem_get_private(elem);

	if (simple->id) {
		snd_mixer_selem_id_free(simple->id);
		simple->id = NULL;
	}
}

static PyObject *
pymelem_cap(struct pymelem *pymelem ATTRIBUTE_UNUSED, void *priv)
{
	return PyInt_FromLong((long)priv);
}

static PyObject *
pymelem_get_caps(struct pymelem *pymelem, void *priv ATTRIBUTE_UNUSED)
{
	return PyInt_FromLong(pymelem->selem.caps);
}

static PyObject *
pymelem_get_name(struct pymelem *pymelem, void *priv ATTRIBUTE_UNUSED)
{
	return PyString_FromString(snd_mixer_selem_id_get_name(pymelem->selem.id));
}

static PyObject *
pymelem_get_index(struct pymelem *pymelem, void *priv ATTRIBUTE_UNUSED)
{
	return PyInt_FromLong(snd_mixer_selem_id_get_index(pymelem->selem.id));
}

static int
pymelem_set_caps(struct pymelem *pymelem, PyObject *val, void *priv ATTRIBUTE_UNUSED)
{
	if (!PyInt_Check(val)) {
		PyErr_SetString(PyExc_TypeError, "The last attribute value must be an integer");
		return -1;
	}
	pymelem->selem.caps = PyInt_AsLong(val);
	return 0;
}

static PyObject *
pymelem_ignore(struct pymelem *pymelem ATTRIBUTE_UNUSED, PyObject *args ATTRIBUTE_UNUSED)
{
	Py_RETURN_NONE;
}

static PyObject *
pymelem_ignore1(struct pymelem *pymelem ATTRIBUTE_UNUSED, PyObject *args ATTRIBUTE_UNUSED)
{
	Py_RETURN_TRUE;
}

static PyObject *
pymelem_error(struct pymelem *pymelem ATTRIBUTE_UNUSED, PyObject *args ATTRIBUTE_UNUSED)
{
	return PyInt_FromLong(-EIO);
}

static PyObject *
pymelem_attach(struct pymelem *pymelem, PyObject *args)
{
	PyObject *obj;
	snd_hctl_elem_t *helem;
	int err;
	
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;
	helem = (snd_hctl_elem_t *)get_C_ptr(obj, "get_C_helem");
	if (helem == NULL)
		return NULL;
	err = snd_mixer_elem_attach(pymelem->melem, helem);
	if (err < 0) {
		PyErr_Format(PyExc_RuntimeError, "Cannot attach hcontrol element to mixer element: %s", snd_strerror(err));
		return NULL;		
	}
	Py_RETURN_NONE;
}

static PyObject *
pymelem_detach(struct pymelem *pymelem, PyObject *args)
{
	PyObject *obj;
	snd_hctl_elem_t *helem;
	int err;
	
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;
	helem = (snd_hctl_elem_t *)get_C_ptr(obj, "get_C_helem");
	if (helem == NULL)
		return NULL;
	err = snd_mixer_elem_detach(pymelem->melem, helem);
	if (err < 0) {
		PyErr_Format(PyExc_RuntimeError, "Cannot detach hcontrol element to mixer element: %s", snd_strerror(err));
		return NULL;		
	}
	Py_RETURN_NONE;
}

static PyObject *
pymelem_event_info(struct pymelem *pymelem, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	return PyInt_FromLong(snd_mixer_elem_info(pymelem->melem));
}

static PyObject *
pymelem_event_value(struct pymelem *pymelem, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	return PyInt_FromLong(snd_mixer_elem_value(pymelem->melem));
}

static int
pymelem_init(struct pymelem *pymelem, PyObject *args, PyObject *kwds ATTRIBUTE_UNUSED)
{
	char *name;
	long index, weight;
	snd_mixer_selem_id_t *id;
	int err;

	if (!PyArg_ParseTuple(args, "Osii", &pymelem->py_mixer, &name, &index, &weight))
		return -1;
	memset(&pymelem->selem, 0, sizeof(pymelem->selem));
	if (snd_mixer_selem_id_malloc(&id))
		return -1;
	snd_mixer_selem_id_set_name(id, name);
	snd_mixer_selem_id_set_index(id, index);
	pymelem->selem.id = id;
	pymelem->selem.ops = &simple_python_ops;
	err = snd_mixer_elem_new(&pymelem->melem, SND_MIXER_ELEM_SIMPLE,
				 weight, &pymelem->selem, selem_free);
	if (err < 0) {
		snd_mixer_selem_id_free(id);
		return -1;
	}
	return 0;
}

static void
pymelem_dealloc(struct pymelem *self)
{
	selem_free(self->melem);
        self->ob_type->tp_free(self);
}

static PyGetSetDef pymelem_getseters[] = {
	{"CAP_GVOLUME", (getter)pymelem_cap, NULL, NULL, (void *)SM_CAP_GVOLUME},
	{"CAP_GSWITCH", (getter)pymelem_cap, NULL, NULL, (void *)SM_CAP_GSWITCH},
	{"CAP_PVOLUME", (getter)pymelem_cap, NULL, NULL, (void *)SM_CAP_PVOLUME},
	{"CAP_PVOLUME_JOIN", (getter)pymelem_cap, NULL, NULL, (void *)SM_CAP_PVOLUME_JOIN},
	{"CAP_PSWITCH", (getter)pymelem_cap, NULL, NULL, (void *)SM_CAP_PSWITCH},
	{"CAP_PSWITCH_JOIN", (getter)pymelem_cap, NULL, NULL, (void *)SM_CAP_PSWITCH_JOIN},
	{"CAP_CVOLUME", (getter)pymelem_cap, NULL, NULL, (void *)SM_CAP_CVOLUME},
	{"CAP_CVOLUME_JOIN", (getter)pymelem_cap, NULL, NULL, (void *)SM_CAP_CVOLUME_JOIN},
	{"CAP_CSWITCH", (getter)pymelem_cap, NULL, NULL, (void *)SM_CAP_CSWITCH},
	{"CAP_CSWITCH_JOIN", (getter)pymelem_cap, NULL, NULL, (void *)SM_CAP_CSWITCH_JOIN},
	{"CAP_CSWITCH_EXCL", (getter)pymelem_cap, NULL, NULL, (void *)SM_CAP_CSWITCH_EXCL},
	{"CAP_PENUM", (getter)pymelem_cap, NULL, NULL, (void *)SM_CAP_PENUM},
	{"CAP_CENUM", (getter)pymelem_cap, NULL, NULL, (void *)SM_CAP_CENUM},

	{"caps", (getter)pymelem_get_caps, (setter)pymelem_set_caps, NULL, NULL},

	{"name", (getter)pymelem_get_name, NULL, NULL, NULL},
	{"index", (getter)pymelem_get_index, NULL, NULL, NULL},
	        
	{NULL,NULL,NULL,NULL,NULL}
};

static PyMethodDef pymelem_methods[] = {
	{"attach", (PyCFunction)pymelem_attach, METH_VARARGS, NULL},
	{"detach", (PyCFunction)pymelem_detach, METH_VARARGS, NULL},
	
	/* "default" functions - no functionality */
	{"opsIsActive", (PyCFunction)pymelem_ignore1, METH_VARARGS, NULL},
	{"opsIsMono", (PyCFunction)pymelem_ignore, METH_VARARGS, NULL},
	{"opsIsChannel", (PyCFunction)pymelem_ignore, METH_VARARGS, NULL},
	{"opsIsEnumerated", (PyCFunction)pymelem_ignore, METH_VARARGS, NULL},
	{"opsIsEnumCnt", (PyCFunction)pymelem_ignore, METH_VARARGS, NULL},

	{"opsGetDB", (PyCFunction)pymelem_error, METH_VARARGS, NULL},
	
	{"eventInfo", (PyCFunction)pymelem_event_info, METH_VARARGS, NULL},
	{"eventValue", (PyCFunction)pymelem_event_value, METH_VARARGS, NULL},

	{NULL,NULL,0,NULL}
};

static PyTypeObject pymelem_type = {
        PyObject_HEAD_INIT(0)
        tp_name:        "smixer_python.InternalMElement",
        tp_basicsize:   sizeof(struct pymelem),
        tp_dealloc:     (destructor)pymelem_dealloc,
        tp_flags:       Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        tp_doc:         NULL /* mixerinit__doc__ */,
        tp_getset:      pymelem_getseters,
        tp_init:        (initproc)pymelem_init,
        tp_alloc:       PyType_GenericAlloc,
        tp_new:         PyType_GenericNew,
        tp_free:        PyObject_Del,
        tp_methods:     pymelem_methods,
};

static PyObject *
pymixer_attach_hctl(struct pymixer *pymixer, PyObject *args)
{
	PyObject *obj;
	snd_hctl_t *hctl;
	void **hctls;
	int err;
	
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;
	hctl = (snd_hctl_t *)get_C_ptr(obj, "get_C_hctl");
	if (hctl == NULL)
		return NULL;
	err = snd_mixer_attach_hctl(pymixer->mixer, hctl);
	if (err < 0) {
		PyErr_Format(PyExc_RuntimeError, "Cannot attach hctl: %s", snd_strerror(err));
		return NULL;
	}
	hctls = realloc(pymixer->hctl, sizeof(void *) * (pymixer->hctl_count+1) * 2);
	if (hctls == NULL) {
		PyErr_SetString(PyExc_RuntimeError, "No enough memory");
		return NULL;
	}
	pymixer->hctl = hctls;
	pymixer->hctl[pymixer->hctl_count*2] = (void *)hctl;
	pymixer->hctl[pymixer->hctl_count*2+1] = (void *)obj;
	pymixer->hctl_count++;
	Py_INCREF(obj);
	Py_RETURN_NONE;
}

static PyObject *
pymixer_register(struct pymixer *pymixer, PyObject *args)
{
	int err;
	
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	err = snd_mixer_class_register(pymixer->class, pymixer->mixer);
	if (err < 0) {
		PyErr_Format(PyExc_RuntimeError, "Cannot register mixer: %s", snd_strerror(err));
		return NULL;
	}
	Py_RETURN_NONE;
}

static PyObject *
pymixer_melement_new(struct pymixer *pymixer, PyObject *args)
{
	PyObject *obj, *obj1, *obj2;
	char *class, *name;
	long index, weight;
	
	if (!PyArg_ParseTuple(args, "ssii", &class, &name, &index, &weight))
		return NULL;
	obj = PyDict_GetItemString(pymixer->mdict, class);
	if (obj) {
		obj1 = PyTuple_New(4);
		if (PyTuple_SET_ITEM(obj1, 0, (PyObject *)pymixer))
			Py_INCREF((PyObject *)pymixer);
		PyTuple_SET_ITEM(obj1, 1, PyString_FromString(name));
		PyTuple_SET_ITEM(obj1, 2, PyInt_FromLong(index));
		PyTuple_SET_ITEM(obj1, 3, PyInt_FromLong(weight));
		obj2 = PyObject_CallObject(obj, obj1);
		Py_XDECREF(obj1);
		if (obj2) {
			struct pymelem *pymelem = (struct pymelem *)obj2;
			void **melems = realloc(pymixer->melem, sizeof(void *) * (pymixer->melem_count + 1) * 2);
			if (melems == NULL) {
				Py_DECREF(obj2);
				return NULL;
			}
			melems[pymixer->melem_count*2] = pymelem->melem;
			melems[pymixer->melem_count*2+1] = obj2;
			Py_INCREF(obj2);
			pymixer->melem = melems;
			pymixer->melem_count++;
		}
	} else {
		PyErr_Format(PyExc_RuntimeError, "Cannot find class '%s'", class);
		return NULL;
	}
	return obj2;
}

static PyObject *
pymixer_melement_add(struct pymixer *pymixer, PyObject *args)
{
	PyObject *obj;
	struct pymelem *pymelem;
	int err;
	
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;
	pymelem = (struct pymelem *)obj;
	err = snd_mixer_elem_add(pymelem->melem, pymixer->class);
	if (err < 0) {
		PyErr_Format(PyExc_RuntimeError, "Cannot add mixer element: %s", snd_strerror(err));
		return NULL;		
	}
	Py_RETURN_NONE;
}

static int
pymixer_init(struct pymixer *pymixer, PyObject *args, PyObject *kwds ATTRIBUTE_UNUSED)
{
	long class, mixer;

	if (!PyArg_ParseTuple(args, "iiO", &class, &mixer, &pymixer->mdict))
		return -1;
	pymixer->class = (snd_mixer_class_t *)class;
	pymixer->mixer = (snd_mixer_t *)mixer;
	pymixer->hctl_count = 0;
	pymixer->hctl = NULL;
	pymixer->helem_count = 0;
	pymixer->helem = NULL;
	pymixer->melem_count = 0;
	pymixer->melem = NULL;
	return 0;
}

static void
pymixer_free(struct pymixer *self)
{
	int idx;
	
	for (idx = 0; idx < self->hctl_count; idx++) {
		snd_mixer_detach_hctl(self->mixer, self->hctl[idx*2]);
		Py_DECREF((PyObject *)self->hctl[idx*2+1]);
	}
	if (self->hctl)
		free(self->hctl);
	self->hctl_count = 0;
	self->hctl = NULL;
	for (idx = 0; idx < self->helem_count; idx++)
		Py_DECREF((PyObject *)self->helem[idx*2+1]);
	if (self->helem)
		free(self->helem);
	self->helem_count = 0;
	self->helem = NULL;
	for (idx = 0; idx < self->melem_count; idx++)
		Py_DECREF((PyObject *)self->melem[idx*2+1]);
	if (self->melem)
		free(self->melem);
	self->melem_count = 0;
	self->melem = NULL;
}

static void
pymixer_dealloc(struct pymixer *self)
{
	pymixer_free(self);
        self->ob_type->tp_free(self);
}

static PyGetSetDef pymixer_getseters[] = {
	{NULL,NULL,NULL,NULL,NULL}
};

static PyMethodDef pymixer_methods[] = {
	{"attachHCtl", (PyCFunction)pymixer_attach_hctl, METH_VARARGS, NULL},
	{"register", (PyCFunction)pymixer_register, METH_VARARGS, NULL},
	{"newMElement", (PyCFunction)pymixer_melement_new, METH_VARARGS, NULL},
	{"addMElement", (PyCFunction)pymixer_melement_add, METH_VARARGS, NULL},
	{NULL,NULL,0,NULL}
};

static PyTypeObject pymixer_type = {
        PyObject_HEAD_INIT(0)
        tp_name:        "smixer_python.InternalMixer",
        tp_basicsize:   sizeof(struct pymixer),
        tp_dealloc:     (destructor)pymixer_dealloc,
        tp_flags:       Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        tp_doc:         NULL /* mixerinit__doc__ */,
        tp_getset:      pymixer_getseters,
        tp_init:        (initproc)pymixer_init,
        tp_alloc:       PyType_GenericAlloc,
        tp_new:         PyType_GenericNew,
        tp_free:        PyObject_Del,
        tp_methods:     pymixer_methods,
};

static PyMethodDef python_methods[] = {
	{NULL, NULL, 0, NULL}
};

static PyObject *new_helem(struct python_priv *priv, snd_hctl_elem_t *helem)
{
	PyObject *obj, *py_hctl = NULL, *obj1, *obj2;
	snd_hctl_t *hctl = snd_hctl_elem_get_hctl(helem);
	struct pymixer *pymixer = (struct pymixer *)priv->py_mixer;
	int idx;

	for (idx = 0; idx < pymixer->hctl_count; idx++) {
		if (pymixer->hctl[idx] == hctl) {
			py_hctl = pymixer->hctl[idx*2+1];
			break;
		}
	}
	if (py_hctl == NULL)
		return NULL;
	obj = PyDict_GetItemString(priv->py_mdict, "HElement");
	if (obj) {
		obj1 = PyTuple_New(3);
		if (PyTuple_SET_ITEM(obj1, 0, py_hctl))
			Py_INCREF(py_hctl);
		PyTuple_SET_ITEM(obj1, 1, PyFloat_FromDouble(1));
		PyTuple_SET_ITEM(obj1, 2, PyInt_FromLong((long)helem));
		obj2 = PyObject_CallObject(obj, obj1);
		if (obj2 == NULL) {
			PyErr_Print();
			PyErr_Clear();
		}
		Py_XDECREF(obj1);
	} else {
		SNDERR("Unable to create InternalMixer object");
		return NULL;
	}
	if (obj2) {
		struct pymixer *pymixer = (struct pymixer *)priv->py_mixer;
		void **helems = realloc(pymixer->helem, sizeof(void *) * (pymixer->helem_count + 1) * 2);
		if (helems == NULL) {
			Py_DECREF(obj2);
			return NULL;
		}
		helems[pymixer->helem_count*2] = helem;
		helems[pymixer->helem_count*2+1] = obj2;
		Py_INCREF(obj2);
		pymixer->helem = helems;
		pymixer->helem_count++;
	}
	return obj2;
}

static PyObject *find_helem(struct python_priv *priv, snd_hctl_elem_t *helem)
{
	struct pymixer *pymixer = (struct pymixer *)priv->py_mixer;
	int idx;

	for (idx = 0; idx < pymixer->helem_count; idx++) {
		if (pymixer->helem[idx*2] == helem)
			return (PyObject *)pymixer->helem[idx*2+1];
	}
	return NULL;
}

static PyObject *find_melem(struct python_priv *priv, snd_mixer_elem_t *melem)
{
	struct pymixer *pymixer = (struct pymixer *)priv->py_mixer;
	int idx;

	for (idx = 0; idx < pymixer->melem_count; idx++) {
		if (pymixer->melem[idx*2] == melem)
			return (PyObject *)pymixer->melem[idx*2+1];
	}
	return NULL;
}

int alsa_mixer_simple_event(snd_mixer_class_t *class, unsigned int mask,
			    snd_hctl_elem_t *helem, snd_mixer_elem_t *melem)
{
	struct python_priv *priv = snd_mixer_sbasic_get_private(class);
	PyThreadState *tstate, *origstate;
	PyObject *t, *o, *r;
	int res = -ENOMEM;

	tstate = PyThreadState_New(main_interpreter);
        origstate = PyThreadState_Swap(tstate);
        
        t = PyTuple_New(3);
        if (t) {
        	PyTuple_SET_ITEM(t, 0, (PyObject *)PyInt_FromLong(mask));
        	o = find_helem(priv, helem);
	        if (mask & SND_CTL_EVENT_MASK_ADD) {
	        	if (o == NULL)
        			o = new_helem(priv, helem);
		}
        	if (o == NULL)
        		return 0;
        	if (PyTuple_SET_ITEM(t, 1, o))
        		Py_INCREF(o);
        	o = melem ? find_melem(priv, melem) : Py_None;
        	if (PyTuple_SET_ITEM(t, 2, o))
        		Py_INCREF(o);
		r = PyObject_CallObject(priv->py_event_func, t);
		Py_DECREF(t);
		if (r) {
			if (PyInt_Check(r)) {
				res = PyInt_AsLong(r);
			} else if (r == Py_None) {
				res = 0;
			}
			Py_DECREF(r);
		} else {
			PyErr_Print();
			PyErr_Clear();
			res = -EIO;
		}
	}
	
	return res;
}

static void alsa_mixer_simple_free(snd_mixer_class_t *class)
{
	struct python_priv *priv = snd_mixer_sbasic_get_private(class);

	if (priv->py_mixer) {
		pymixer_free((struct pymixer *)priv->py_mixer);
		Py_DECREF(priv->py_mixer);
	}
	if (priv->py_initialized) {
		Py_XDECREF(priv->py_event_func);
		Py_Finalize();
	}
	free(priv);
}

int alsa_mixer_simple_finit(snd_mixer_class_t *class,
			    snd_mixer_t *mixer,
			    const char *device)
{
	struct python_priv *priv;
	FILE *fp;
	const char *file;
	PyObject *obj, *obj1, *obj2, *py_mod, *mdict;

	priv = calloc(1, sizeof(*priv));
	if (priv == NULL)
		return -ENOMEM;

	snd_mixer_sbasic_set_private(class, priv);
	snd_mixer_sbasic_set_private_free(class, alsa_mixer_simple_free);

	file = getenv("ALSA_MIXER_SIMPLE_MPYTHON");
	if (file == NULL)
		file = SCRIPT;

	fp = fopen(file, "r");
	if (fp == NULL) {
		SNDERR("Unable to find python module '%s'", file);
		return -ENODEV;
	}
	
	Py_Initialize();
	if (PyType_Ready(&pymelem_type) < 0)
		return -EIO;
	if (PyType_Ready(&pymixer_type) < 0)
		return -EIO;
	Py_InitModule("smixer_python", python_methods);
	priv->py_initialized = 1;
	main_interpreter = PyThreadState_Get()->interp;
	obj = PyImport_GetModuleDict();
	py_mod = PyDict_GetItemString(obj, "__main__");
	if (py_mod) {
		mdict = priv->py_mdict = PyModule_GetDict(py_mod);
		obj = PyString_FromString(file);
		if (obj)
			PyDict_SetItemString(mdict, "__file__", obj);
		Py_XDECREF(obj);
		obj = PyString_FromString(device);
		if (obj)
			PyDict_SetItemString(mdict, "device", obj);
		Py_XDECREF(obj);
		Py_INCREF(&pymixer_type);
		PyModule_AddObject(py_mod, "InternalMElement", (PyObject *)&pymelem_type);
		PyModule_AddObject(py_mod, "InternalMixer", (PyObject *)&pymixer_type);
		obj = PyDict_GetItemString(mdict, "InternalMixer");
		if (obj) {
			obj1 = PyTuple_New(3);
			PyTuple_SET_ITEM(obj1, 0, PyInt_FromLong((long)class));
			PyTuple_SET_ITEM(obj1, 1, PyInt_FromLong((long)mixer));
			if (PyTuple_SET_ITEM(obj1, 2, mdict))
				Py_INCREF(mdict);
			obj2 = PyObject_CallObject(obj, obj1);
			Py_XDECREF(obj1);
			PyDict_SetItemString(mdict, "mixer", obj2);
			priv->py_mixer = obj2;
		} else {
			SNDERR("Unable to create InternalMixer object");
			return -EIO;
		}


		obj = PyRun_FileEx(fp, file, Py_file_input, mdict, mdict, 1);
		if (obj == NULL)
			PyErr_Print();
		Py_XDECREF(obj);
		priv->py_event_func = PyDict_GetItemString(mdict, "event");
		if (priv->py_event_func == NULL) {
			SNDERR("Unable to find python function 'event'");
			return -EIO;
		}
	}
	return 0;
}
