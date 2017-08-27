/*
 *  ALSA lisp implementation - sound related commands
 *  Copyright (c) 2003 by Jaroslav Kysela <perex@perex.cz>
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

struct acall_table {
	const char *name;
	struct alisp_object * (*func) (struct alisp_instance *instance, struct acall_table * item, struct alisp_object * args);
	void * xfunc;
	const char *prefix;
};

/*
 *  helper functions
 */

static inline int get_integer(struct alisp_object * obj)
{
	if (alisp_compare_type(obj, ALISP_OBJ_INTEGER))
		return obj->value.i;
	return 0;
}

static inline const void *get_pointer(struct alisp_object * obj)
{
	if (alisp_compare_type(obj, ALISP_OBJ_POINTER))
		return obj->value.ptr;
	return NULL;
}

static const char *get_string(struct alisp_object * obj, const char * deflt)
{
	if (obj == &alsa_lisp_t)
		return "true";
	if (alisp_compare_type(obj, ALISP_OBJ_STRING) ||
	    alisp_compare_type(obj, ALISP_OBJ_IDENTIFIER))
		return obj->value.s;
	return deflt;
}

struct flags {
	const char *key;
	unsigned int mask;
}; 

static unsigned int get_flags(struct alisp_instance * instance,
			      struct alisp_object * obj,
			      const struct flags * flags,
			      unsigned int deflt)
{
	const char *key;
	int invert;
	unsigned int result;
	const struct flags *ptr;
	struct alisp_object *n;

	if (obj == &alsa_lisp_nil)
		return deflt;
	result = deflt;
	do {
		key = get_string(obj, NULL);
		if (key) {
			invert = key[0] == '!';
			key += invert;
			ptr = flags;
			while (ptr->key) {
				if (!strcmp(ptr->key, key)) {
					if (invert)
						result &= ~ptr->mask;
					else
						result |= ptr->mask;
					break;
				}
				ptr++;
			}
		}
		delete_tree(instance, car(obj));
		obj = cdr(n = obj);
		delete_object(instance, n);
	} while (obj != &alsa_lisp_nil);
	return result;
}

static const void *get_ptr(struct alisp_instance * instance,
			   struct alisp_object * obj,
			   const char *_ptr_id)
{
	const char *ptr_id;
	const void *ptr;
	
	ptr_id = get_string(car(obj), NULL);
	if (ptr_id == NULL) {
		delete_tree(instance, obj);
		return NULL;
	}
	if (strcmp(ptr_id, _ptr_id)) {
		delete_tree(instance, obj);
		return NULL;
	}
	ptr = get_pointer(cdr(obj));
	delete_tree(instance, obj);
	return ptr;
}

static struct alisp_object * new_lexpr(struct alisp_instance * instance, int err)
{
	struct alisp_object * lexpr;

	lexpr = new_object(instance, ALISP_OBJ_CONS);
	if (lexpr == NULL)
		return NULL;
	lexpr->value.c.car = new_integer(instance, err);
	if (lexpr->value.c.car == NULL) {
		delete_object(instance, lexpr);
		return NULL;
	}
	lexpr->value.c.cdr = new_object(instance, ALISP_OBJ_CONS);
	if (lexpr->value.c.cdr == NULL) {
		delete_object(instance, lexpr->value.c.car);
		delete_object(instance, lexpr);
		return NULL;
	}
	return lexpr;
}

static struct alisp_object * add_cons(struct alisp_instance * instance,
				      struct alisp_object *lexpr,
				      int cdr, const char *id,
				      struct alisp_object *obj)
{
	struct alisp_object * p1, * p2;

	if (lexpr == NULL || obj == NULL) {
		delete_tree(instance, obj);
		return NULL;
	}
	if (cdr) {
		p1 = lexpr->value.c.cdr = new_object(instance, ALISP_OBJ_CONS);
	} else {
		p1 = lexpr->value.c.car = new_object(instance, ALISP_OBJ_CONS);
	}
	lexpr = p1;
	if (p1 == NULL) {
		delete_tree(instance, obj);
		return NULL;
	}
	p1->value.c.car = new_object(instance, ALISP_OBJ_CONS);
	if ((p2 = p1->value.c.car) == NULL)
		goto __err;
	p2->value.c.car = new_string(instance, id);
	if (p2->value.c.car == NULL) {
	      __err:
		if (cdr)
			lexpr->value.c.cdr = NULL;
		else
			lexpr->value.c.car = NULL;
		delete_tree(instance, p1);
		delete_tree(instance, obj);
		return NULL;
	}
	p2->value.c.cdr = obj;
	return lexpr;
}

static struct alisp_object * add_cons2(struct alisp_instance * instance,
				       struct alisp_object *lexpr,
				       int cdr, struct alisp_object *obj)
{
	struct alisp_object * p1;

	if (lexpr == NULL || obj == NULL) {
		delete_tree(instance, obj);
		return NULL;
	}
	if (cdr) {
		p1 = lexpr->value.c.cdr = new_object(instance, ALISP_OBJ_CONS);
	} else {
		p1 = lexpr->value.c.car = new_object(instance, ALISP_OBJ_CONS);
	}
	lexpr = p1;
	if (p1 == NULL) {
		delete_tree(instance, obj);
		return NULL;
	}
	p1->value.c.car = obj;
	return lexpr;
}

static struct alisp_object * new_result1(struct alisp_instance * instance,
					 int err, const char *ptr_id, void *ptr)
{
	struct alisp_object * lexpr, * p1;

	if (err < 0)
		ptr = NULL;
	lexpr = new_object(instance, ALISP_OBJ_CONS);
	if (lexpr == NULL)
		return NULL;
	lexpr->value.c.car = new_integer(instance, err);
	if (lexpr->value.c.car == NULL) {
		delete_object(instance, lexpr);
		return NULL;
	}
	p1 = add_cons(instance, lexpr, 1, ptr_id, new_pointer(instance, ptr));
	if (p1 == NULL) {
		delete_object(instance, lexpr);
		return NULL;
	}
	return lexpr;
}

static struct alisp_object * new_result2(struct alisp_instance * instance,
					 int err, int val)
{
	struct alisp_object * lexpr, * p1;

	if (err < 0)
		val = 0;
	lexpr = new_lexpr(instance, err);
	if (lexpr == NULL)
		return NULL;
	p1 = lexpr->value.c.cdr;
	p1->value.c.car = new_integer(instance, val);
	if (p1->value.c.car == NULL) {
		delete_object(instance, lexpr);
		return NULL;
	}
	return lexpr;
}

static struct alisp_object * new_result3(struct alisp_instance * instance,
					 int err, const char *str)
{
	struct alisp_object * lexpr, * p1;

	if (err < 0)
		str = "";
	lexpr = new_lexpr(instance, err);
	if (lexpr == NULL)
		return NULL;
	p1 = lexpr->value.c.cdr;
	p1->value.c.car = new_string(instance, str);
	if (p1->value.c.car == NULL) {
		delete_object(instance, lexpr);
		return NULL;
	}
	return lexpr;
}

/*
 *  macros
 */

/*
 *  HCTL functions
 */

typedef int (*snd_int_pp_strp_int_t)(void **rctl, const char *name, int mode);
typedef int (*snd_int_pp_p_t)(void **rctl, void *handle);
typedef int (*snd_int_p_t)(void *rctl);
typedef char * (*snd_str_p_t)(void *rctl);
typedef int (*snd_int_intp_t)(int *val);
typedef int (*snd_int_str_t)(const char *str);
typedef int (*snd_int_int_strp_t)(int val, char **str);
typedef void *(*snd_p_p_t)(void *handle);

static struct alisp_object * FA_int_pp_strp_int(struct alisp_instance * instance, struct acall_table * item, struct alisp_object * args)
{
	const char *name;
	int err, mode;
	void *handle;
	struct alisp_object *p1, *p2;
	static const struct flags flags[] = {
		{ "nonblock", SND_CTL_NONBLOCK },
		{ "async", SND_CTL_ASYNC },
		{ "readonly", SND_CTL_READONLY },
		{ NULL, 0 }
	};

	name = get_string(p1 = eval(instance, car(args)), NULL);
	if (name == NULL)
		return &alsa_lisp_nil;
	mode = get_flags(instance, p2 = eval(instance, car(cdr(args))), flags, 0);
	delete_tree(instance, cdr(cdr(args)));
	delete_object(instance, cdr(args));
	delete_object(instance, args);
	delete_tree(instance, p2);
	err = ((snd_int_pp_strp_int_t)item->xfunc)(&handle, name, mode);
	delete_tree(instance, p1);
	return new_result1(instance, err, item->prefix, handle);
}

static struct alisp_object * FA_int_pp_p(struct alisp_instance * instance, struct acall_table * item, struct alisp_object * args)
{
	int err;
	void *handle;
	const char *prefix1;
	struct alisp_object *p1;

	if (item->xfunc == &snd_hctl_open_ctl)
		prefix1 = "ctl";
	else {
		delete_tree(instance, args);
		return &alsa_lisp_nil;
	}
	p1 = eval(instance, car(args));
	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	handle = (void *)get_ptr(instance, p1, prefix1);
	if (handle == NULL)
		return &alsa_lisp_nil;
	err = ((snd_int_pp_p_t)item->xfunc)(&handle, handle);
	return new_result1(instance, err, item->prefix, handle);
}

static struct alisp_object * FA_p_p(struct alisp_instance * instance, struct acall_table * item, struct alisp_object * args)
{
	void *handle;
	const char *prefix1;
	struct alisp_object * p1;

	if (item->xfunc == &snd_hctl_first_elem ||
	    item->xfunc == &snd_hctl_last_elem ||
	    item->xfunc == &snd_hctl_elem_next ||
	    item->xfunc == &snd_hctl_elem_prev)
		prefix1 = "hctl_elem";
	else if (item->xfunc == &snd_hctl_ctl)
		prefix1 = "ctl";
	else {
		delete_tree(instance, args);
		return &alsa_lisp_nil;
	}
	p1 = eval(instance, car(args));
	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	handle = (void *)get_ptr(instance, p1, item->prefix);
	if (handle == NULL)
		return &alsa_lisp_nil;
	handle = ((snd_p_p_t)item->xfunc)(handle);
	return new_cons_pointer(instance, prefix1, handle);
}

static struct alisp_object * FA_int_p(struct alisp_instance * instance, struct acall_table * item, struct alisp_object * args)
{
	void *handle;
	struct alisp_object * p1;

	p1 = eval(instance, car(args));
	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	handle = (void *)get_ptr(instance, p1, item->prefix);
	if (handle == NULL)
		return &alsa_lisp_nil;
	return new_integer(instance, ((snd_int_p_t)item->xfunc)(handle));
}

static struct alisp_object * FA_str_p(struct alisp_instance * instance, struct acall_table * item, struct alisp_object * args)
{
	void *handle;
	struct alisp_object * p1;

	p1 = eval(instance, car(args));
	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	handle = (void *)get_ptr(instance, p1, item->prefix);
	if (handle == NULL)
		return &alsa_lisp_nil;
	return new_string(instance, ((snd_str_p_t)item->xfunc)(handle));
}

static struct alisp_object * FA_int_intp(struct alisp_instance * instance, struct acall_table * item, struct alisp_object * args)
{
	int val, err;
	struct alisp_object * p1;

	p1 = eval(instance, car(args));
	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	if (!alisp_compare_type(p1, ALISP_OBJ_INTEGER)) {
		delete_tree(instance, p1);
		return &alsa_lisp_nil;
	}
	val = p1->value.i;
	delete_tree(instance, p1);
	err = ((snd_int_intp_t)item->xfunc)(&val);
	return new_result2(instance, err, val);
}

static struct alisp_object * FA_int_str(struct alisp_instance * instance, struct acall_table * item, struct alisp_object * args)
{
	int err;
	struct alisp_object * p1;

	p1 = eval(instance, car(args));
	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	if (!alisp_compare_type(p1, ALISP_OBJ_STRING) &&
	    !alisp_compare_type(p1, ALISP_OBJ_IDENTIFIER)) {
		delete_tree(instance, p1);
		return &alsa_lisp_nil;
	}
	err = ((snd_int_str_t)item->xfunc)(p1->value.s);
	delete_tree(instance, p1);
	return new_integer(instance, err);
}

static struct alisp_object * FA_int_int_strp(struct alisp_instance * instance, struct acall_table * item, struct alisp_object * args)
{
	int err;
	char *str;
	long val;
	struct alisp_object * p1;

	p1 = eval(instance, car(args));
	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	if (!alisp_compare_type(p1, ALISP_OBJ_INTEGER)) {
		delete_tree(instance, p1);
		return &alsa_lisp_nil;
	}
	val = p1->value.i;
	delete_tree(instance, p1);
	err = ((snd_int_int_strp_t)item->xfunc)(val, &str);
	return new_result3(instance, err, str);
}

static struct alisp_object * FA_card_info(struct alisp_instance * instance, struct acall_table * item, struct alisp_object * args)
{
	snd_ctl_t *handle;
	struct alisp_object * lexpr, * p1;
	snd_ctl_card_info_t * info;
	int err;

	p1 = eval(instance, car(args));
	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	handle = (snd_ctl_t *)get_ptr(instance, p1, item->prefix);
	if (handle == NULL)
		return &alsa_lisp_nil;
	snd_ctl_card_info_alloca(&info);
	err = snd_ctl_card_info(handle, info);
	lexpr = new_lexpr(instance, err);
	if (err < 0)
		return lexpr;
	p1 = add_cons(instance, lexpr->value.c.cdr, 0, "id", new_string(instance, snd_ctl_card_info_get_id(info)));
	p1 = add_cons(instance, p1, 1, "driver", new_string(instance, snd_ctl_card_info_get_driver(info)));
	p1 = add_cons(instance, p1, 1, "name", new_string(instance, snd_ctl_card_info_get_name(info)));
	p1 = add_cons(instance, p1, 1, "longname", new_string(instance, snd_ctl_card_info_get_longname(info)));
	p1 = add_cons(instance, p1, 1, "mixername", new_string(instance, snd_ctl_card_info_get_mixername(info)));
	p1 = add_cons(instance, p1, 1, "components", new_string(instance, snd_ctl_card_info_get_components(info)));
	if (p1 == NULL) {
		delete_tree(instance, lexpr);
		return NULL;
	}
	return lexpr;
}

static struct alisp_object * create_ctl_elem_id(struct alisp_instance * instance, snd_ctl_elem_id_t * id, struct alisp_object * cons)
{
	cons = add_cons(instance, cons, 0, "numid", new_integer(instance, snd_ctl_elem_id_get_numid(id)));
	cons = add_cons(instance, cons, 1, "iface", new_string(instance, snd_ctl_elem_iface_name(snd_ctl_elem_id_get_interface(id))));
	cons = add_cons(instance, cons, 1, "dev", new_integer(instance, snd_ctl_elem_id_get_device(id)));
	cons = add_cons(instance, cons, 1, "subdev", new_integer(instance, snd_ctl_elem_id_get_subdevice(id)));
	cons = add_cons(instance, cons, 1, "name", new_string(instance, snd_ctl_elem_id_get_name(id)));
	cons = add_cons(instance, cons, 1, "index", new_integer(instance, snd_ctl_elem_id_get_index(id)));
	return cons;
}

static int parse_ctl_elem_id(struct alisp_instance * instance,
			     struct alisp_object * cons,
			     snd_ctl_elem_id_t * id)
{
	struct alisp_object *p1;
	const char *xid;

	if (cons == NULL)
		return -ENOMEM;
	snd_ctl_elem_id_clear(id);
	id->numid = 0;
	do {
		p1 = car(cons);
		if (alisp_compare_type(p1, ALISP_OBJ_CONS)) {
			xid = get_string(p1->value.c.car, NULL);
			if (xid == NULL) {
				/* noop */
			} else if (!strcmp(xid, "numid")) {
				snd_ctl_elem_id_set_numid(id, get_integer(p1->value.c.cdr));
			} else if (!strcmp(xid, "iface")) {
				snd_ctl_elem_id_set_interface(id, snd_config_get_ctl_iface_ascii(get_string(p1->value.c.cdr, "0")));
			} else if (!strcmp(xid, "dev")) {
				snd_ctl_elem_id_set_device(id, get_integer(p1->value.c.cdr));
			} else if (!strcmp(xid, "subdev")) {
				snd_ctl_elem_id_set_subdevice(id, get_integer(p1->value.c.cdr));
			} else if (!strcmp(xid, "name")) {
				snd_ctl_elem_id_set_name(id, get_string(p1->value.c.cdr, "?"));
			} else if (!strcmp(xid, "index")) {
				snd_ctl_elem_id_set_index(id, get_integer(p1->value.c.cdr));
			}
		}
		delete_tree(instance, p1);
	        cons = cdr(p1 = cons);
	        delete_object(instance, p1);
	} while (cons != &alsa_lisp_nil);
	return 0;
}

static struct alisp_object * FA_hctl_find_elem(struct alisp_instance * instance, struct acall_table * item, struct alisp_object * args)
{
	snd_hctl_t *handle;
	snd_ctl_elem_id_t *id;
	struct alisp_object *p1;

	handle = (snd_hctl_t *)get_ptr(instance, car(args), item->prefix);
	if (handle == NULL) {
		delete_tree(instance, cdr(args));
		delete_object(instance, args);
		return &alsa_lisp_nil;
	}
	snd_ctl_elem_id_alloca(&id);
	p1 = car(cdr(args));
	delete_tree(instance, cdr(cdr(args)));
	delete_object(instance, cdr(args));
	delete_object(instance, args);
	if (parse_ctl_elem_id(instance, eval(instance, p1), id) < 0)
		return &alsa_lisp_nil;
	return new_cons_pointer(instance, "hctl_elem", snd_hctl_find_elem(handle, id));
}

static struct alisp_object * FA_hctl_elem_info(struct alisp_instance * instance, struct acall_table * item, struct alisp_object * args)
{
	snd_hctl_elem_t *handle;
	struct alisp_object * lexpr, * p1, * p2;
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_type_t type;
	int err;

	p1 = eval(instance, car(args));
	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	handle = (snd_hctl_elem_t *)get_ptr(instance, p1, item->prefix);
	if (handle == NULL)
		return &alsa_lisp_nil;
	snd_ctl_elem_info_alloca(&info);
	snd_ctl_elem_id_alloca(&id);
	err = snd_hctl_elem_info(handle, info);
	lexpr = new_lexpr(instance, err);
	if (err < 0)
		return lexpr;
	type = snd_ctl_elem_info_get_type(info);
	p1 = add_cons(instance, lexpr->value.c.cdr, 0, "id", p2 = new_object(instance, ALISP_OBJ_CONS));
	snd_ctl_elem_info_get_id(info, id);
	if (create_ctl_elem_id(instance, id, p2) == NULL) {
		delete_tree(instance, lexpr);
		return NULL;
	}
	p1 = add_cons(instance, p1, 1, "type", new_string(instance, snd_ctl_elem_type_name(type)));
	p1 = add_cons(instance, p1, 1, "readable", new_integer(instance, snd_ctl_elem_info_is_readable(info)));
	p1 = add_cons(instance, p1, 1, "writeable", new_integer(instance, snd_ctl_elem_info_is_writable(info)));
	p1 = add_cons(instance, p1, 1, "volatile", new_integer(instance, snd_ctl_elem_info_is_volatile(info)));
	p1 = add_cons(instance, p1, 1, "inactive", new_integer(instance, snd_ctl_elem_info_is_inactive(info)));
	p1 = add_cons(instance, p1, 1, "locked", new_integer(instance, snd_ctl_elem_info_is_locked(info)));
	p1 = add_cons(instance, p1, 1, "isowner", new_integer(instance, snd_ctl_elem_info_is_owner(info)));
	p1 = add_cons(instance, p1, 1, "owner", new_integer(instance, snd_ctl_elem_info_get_owner(info)));
	p1 = add_cons(instance, p1, 1, "count", new_integer(instance, snd_ctl_elem_info_get_count(info)));
	err = snd_ctl_elem_info_get_dimensions(info);
	if (err > 0) {
		int idx;
		p1 = add_cons(instance, p1, 1, "dimensions", p2 = new_object(instance, ALISP_OBJ_CONS));
		for (idx = 0; idx < err; idx++)
			p2 = add_cons2(instance, p2, idx > 0, new_integer(instance, snd_ctl_elem_info_get_dimension(info, idx)));
	}
	switch (type) {
	case SND_CTL_ELEM_TYPE_ENUMERATED: {
		unsigned int items, item;
		items = snd_ctl_elem_info_get_items(info);
		p1 = add_cons(instance, p1, 1, "items", p2 = new_object(instance, ALISP_OBJ_CONS));
		for (item = 0; item < items; item++) {
			snd_ctl_elem_info_set_item(info, item);
			err = snd_hctl_elem_info(handle, info);
			if (err < 0) {
				p2 = add_cons2(instance, p2, item, &alsa_lisp_nil);
			} else {
				p2 = add_cons2(instance, p2, item, new_string(instance, snd_ctl_elem_info_get_item_name(info)));
			}
		}
		break;
	}
	case SND_CTL_ELEM_TYPE_INTEGER:
		p1 = add_cons(instance, p1, 1, "min", new_integer(instance, snd_ctl_elem_info_get_min(info)));
		p1 = add_cons(instance, p1, 1, "max", new_integer(instance, snd_ctl_elem_info_get_max(info)));
		p1 = add_cons(instance, p1, 1, "step", new_integer(instance, snd_ctl_elem_info_get_step(info)));
		break;
	case SND_CTL_ELEM_TYPE_INTEGER64:
		p1 = add_cons(instance, p1, 1, "min64", new_float(instance, snd_ctl_elem_info_get_min64(info)));
		p1 = add_cons(instance, p1, 1, "max64", new_float(instance, snd_ctl_elem_info_get_max64(info)));
		p1 = add_cons(instance, p1, 1, "step64", new_float(instance, snd_ctl_elem_info_get_step64(info)));
		break;
	default:
		break;
	}
	if (p1 == NULL) {
		delete_tree(instance, lexpr);
		return NULL;
	}
	return lexpr;
}

static struct alisp_object * FA_hctl_elem_read(struct alisp_instance * instance, struct acall_table * item, struct alisp_object * args)
{
	snd_hctl_elem_t *handle;
	struct alisp_object * lexpr, * p1 = NULL, * obj;
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_value_t *value;
	snd_ctl_elem_type_t type;
	unsigned int idx, count;
	int err;

	p1 = eval(instance, car(args));
	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	handle = (snd_hctl_elem_t *)get_ptr(instance, p1, item->prefix);
	if (handle == NULL)
		return &alsa_lisp_nil;
	snd_ctl_elem_info_alloca(&info);
	snd_ctl_elem_value_alloca(&value);
	err = snd_hctl_elem_info(handle, info);
	if (err >= 0)
		err = snd_hctl_elem_read(handle, value);
	lexpr = new_lexpr(instance, err);
	if (err < 0)
		return lexpr;
	type = snd_ctl_elem_info_get_type(info);
	count = snd_ctl_elem_info_get_count(info);
	if (type == SND_CTL_ELEM_TYPE_IEC958) {
		count = sizeof(snd_aes_iec958_t);
		type = SND_CTL_ELEM_TYPE_BYTES;
	}
	for (idx = 0; idx < count; idx++) {
		switch (type) {
		case SND_CTL_ELEM_TYPE_BOOLEAN:
			obj = new_integer(instance, snd_ctl_elem_value_get_boolean(value, idx));
			break;
		case SND_CTL_ELEM_TYPE_INTEGER:
			obj = new_integer(instance, snd_ctl_elem_value_get_integer(value, idx));
			break;
		case SND_CTL_ELEM_TYPE_INTEGER64:
			obj = new_integer(instance, snd_ctl_elem_value_get_integer64(value, idx));
			break;
		case SND_CTL_ELEM_TYPE_ENUMERATED:
			obj = new_integer(instance, snd_ctl_elem_value_get_enumerated(value, idx));
			break;
		case SND_CTL_ELEM_TYPE_BYTES:
			obj = new_integer(instance, snd_ctl_elem_value_get_byte(value, idx));
			break;
		default:
			obj = NULL;
			break;
		}
		if (idx == 0) {
			p1 = add_cons2(instance, lexpr->value.c.cdr, 0, obj);
		} else {
			p1 = add_cons2(instance, p1, 1, obj);
		}
	}
	if (p1 == NULL) {
		delete_tree(instance, lexpr);
		return &alsa_lisp_nil;
	}
	return lexpr;
}

static struct alisp_object * FA_hctl_elem_write(struct alisp_instance * instance, struct acall_table * item, struct alisp_object * args)
{
	snd_hctl_elem_t *handle;
	struct alisp_object * p1 = NULL, * obj;
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_value_t *value;
	snd_ctl_elem_type_t type;
	unsigned int idx, count;
	int err;

	p1 = car(cdr(args));
	obj = eval(instance, car(args));
	delete_tree(instance, cdr(cdr(args)));
	delete_object(instance, cdr(args));
	delete_object(instance, args);
	handle = (snd_hctl_elem_t *)get_ptr(instance, obj, item->prefix);
	if (handle == NULL) {
		delete_tree(instance, p1);
		return &alsa_lisp_nil;
	}
	snd_ctl_elem_info_alloca(&info);
	snd_ctl_elem_value_alloca(&value);
	err = snd_hctl_elem_info(handle, info);
	if (err < 0) {
		delete_tree(instance, p1);
		return new_integer(instance, err);
	}
	type = snd_ctl_elem_info_get_type(info);
	count = snd_ctl_elem_info_get_count(info);
	if (type == SND_CTL_ELEM_TYPE_IEC958) {
		count = sizeof(snd_aes_iec958_t);
		type = SND_CTL_ELEM_TYPE_BYTES;
	}
	idx = -1;
	do {
		if (++idx >= count) {
			delete_tree(instance, p1);
			break;
		}
		obj = car(p1);
		switch (type) {
		case SND_CTL_ELEM_TYPE_BOOLEAN:
			snd_ctl_elem_value_set_boolean(value, idx, get_integer(obj));
			break;
		case SND_CTL_ELEM_TYPE_INTEGER:
			snd_ctl_elem_value_set_integer(value, idx, get_integer(obj));
			break;
		case SND_CTL_ELEM_TYPE_INTEGER64:
			snd_ctl_elem_value_set_integer64(value, idx, get_integer(obj));
			break;
		case SND_CTL_ELEM_TYPE_ENUMERATED:
			snd_ctl_elem_value_set_enumerated(value, idx, get_integer(obj));
			break;
		case SND_CTL_ELEM_TYPE_BYTES:
			snd_ctl_elem_value_set_byte(value, idx, get_integer(obj));
			break;
		default:
			break;
		}
		delete_tree(instance, obj);
		p1 = cdr(obj = p1);
		delete_object(instance, obj);
	} while (p1 != &alsa_lisp_nil);
	err = snd_hctl_elem_write(handle, value);
	return new_integer(instance, err);
}

static struct alisp_object * FA_pcm_info(struct alisp_instance * instance, struct acall_table * item, struct alisp_object * args)
{
	snd_pcm_t *handle;
	struct alisp_object * lexpr, * p1;
	snd_pcm_info_t *info;
	int err;

	p1 = eval(instance, car(args));
	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	handle = (snd_pcm_t *)get_ptr(instance, p1, item->prefix);
	if (handle == NULL)
		return &alsa_lisp_nil;
	snd_pcm_info_alloca(&info);
	err = snd_pcm_info(handle, info);
	lexpr = new_lexpr(instance, err);
	if (err < 0)
		return lexpr;
	p1 = add_cons(instance, lexpr->value.c.cdr, 0, "card", new_integer(instance, snd_pcm_info_get_card(info)));
	p1 = add_cons(instance, p1, 1, "device", new_integer(instance, snd_pcm_info_get_device(info)));
	p1 = add_cons(instance, p1, 1, "subdevice", new_integer(instance, snd_pcm_info_get_subdevice(info)));
	p1 = add_cons(instance, p1, 1, "id", new_string(instance, snd_pcm_info_get_id(info)));
	p1 = add_cons(instance, p1, 1, "name", new_string(instance, snd_pcm_info_get_name(info)));
	p1 = add_cons(instance, p1, 1, "subdevice_name", new_string(instance, snd_pcm_info_get_subdevice_name(info)));
	p1 = add_cons(instance, p1, 1, "class", new_integer(instance, snd_pcm_info_get_class(info)));
	p1 = add_cons(instance, p1, 1, "subclass", new_integer(instance, snd_pcm_info_get_subclass(info)));
	p1 = add_cons(instance, p1, 1, "subdevices_count", new_integer(instance, snd_pcm_info_get_subdevices_count(info)));
	p1 = add_cons(instance, p1, 1, "subdevices_avail", new_integer(instance, snd_pcm_info_get_subdevices_avail(info)));
	//p1 = add_cons(instance, p1, 1, "sync", new_string(instance, snd_pcm_info_get_sync(info)));
	return lexpr;
}

/*
 *  main code
 */

static const struct acall_table acall_table[] = {
	{ "card_get_index", &FA_int_str, (void *)snd_card_get_index, NULL },
	{ "card_get_longname", &FA_int_int_strp, (void *)snd_card_get_longname, NULL },
	{ "card_get_name", &FA_int_int_strp, (void *)snd_card_get_name, NULL },
	{ "card_next", &FA_int_intp, (void *)&snd_card_next, NULL },
	{ "ctl_card_info", &FA_card_info, NULL, "ctl" },
	{ "ctl_close", &FA_int_p, (void *)&snd_ctl_close, "ctl" },
	{ "ctl_open", &FA_int_pp_strp_int, (void *)&snd_ctl_open, "ctl" },
	{ "hctl_close", &FA_int_p, (void *)&snd_hctl_close, "hctl" },
	{ "hctl_ctl", &FA_p_p, (void *)&snd_hctl_ctl, "hctl" },
	{ "hctl_elem_info", &FA_hctl_elem_info, (void *)&snd_hctl_elem_info, "hctl_elem" },
	{ "hctl_elem_next", &FA_p_p, (void *)&snd_hctl_elem_next, "hctl_elem" },
	{ "hctl_elem_prev", &FA_p_p, (void *)&snd_hctl_elem_prev, "hctl_elem" },
	{ "hctl_elem_read", &FA_hctl_elem_read, (void *)&snd_hctl_elem_read, "hctl_elem" },
	{ "hctl_elem_write", &FA_hctl_elem_write, (void *)&snd_hctl_elem_write, "hctl_elem" },
	{ "hctl_find_elem", &FA_hctl_find_elem, (void *)&snd_hctl_find_elem, "hctl" },
	{ "hctl_first_elem", &FA_p_p, (void *)&snd_hctl_first_elem, "hctl" },
	{ "hctl_free", &FA_int_p, (void *)&snd_hctl_free, "hctl" },
	{ "hctl_last_elem", &FA_p_p, (void *)&snd_hctl_last_elem, "hctl" },
	{ "hctl_load", &FA_int_p, (void *)&snd_hctl_load, "hctl" },
	{ "hctl_open", &FA_int_pp_strp_int, (void *)&snd_hctl_open, "hctl" },
	{ "hctl_open_ctl", &FA_int_pp_p, (void *)&snd_hctl_open_ctl, "hctl" },
	{ "pcm_info", &FA_pcm_info, NULL, "pcm" },
	{ "pcm_name", &FA_str_p, (void *)&snd_pcm_name, "pcm" },
};

static int acall_compar(const void *p1, const void *p2)
{
	return strcmp(((struct acall_table *)p1)->name,
        	      ((struct acall_table *)p2)->name);
}

static struct alisp_object * F_acall(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p1, *p2;
	struct acall_table key, *item;

	p1 = eval(instance, car(args));
	p2 = cdr(args);
	delete_object(instance, args);
	if (!alisp_compare_type(p1, ALISP_OBJ_IDENTIFIER) &&
	    !alisp_compare_type(p1, ALISP_OBJ_STRING)) {
	    	delete_tree(instance, p2);
		return &alsa_lisp_nil;
	}
	key.name = p1->value.s;
	if ((item = bsearch(&key, acall_table,
			    sizeof acall_table / sizeof acall_table[0],
			    sizeof acall_table[0], acall_compar)) != NULL) {
		delete_tree(instance, p1);
		return item->func(instance, item, p2);
	}
	delete_tree(instance, p1);
	delete_tree(instance, p2);
	lisp_warn(instance, "acall function %s' is undefined", p1->value.s);
	return &alsa_lisp_nil;
}

static struct alisp_object * F_ahandle(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object *p1;

	p1 = eval(instance, car(args));
	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	args = car(cdr(p1));
	delete_tree(instance, cdr(cdr(p1)));
	delete_object(instance, cdr(p1));
	delete_tree(instance, car(p1));
	delete_object(instance, p1);
	return args;
}

static struct alisp_object * F_aerror(struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object *p1;

	p1 = eval(instance, car(args));
	delete_tree(instance, cdr(args));
	delete_object(instance, args);
	args = car(p1);
	if (args == &alsa_lisp_nil) {
		delete_tree(instance, p1);
		return new_integer(instance, SND_ERROR_ALISP_NIL);
	} else {
		delete_tree(instance, cdr(p1));
		delete_object(instance, p1);
	}
	return args;
}

static int common_error(snd_output_t **rout, struct alisp_instance *instance, struct alisp_object * args)
{
	struct alisp_object * p = args, * p1;
	snd_output_t *out;
	int err;
	
	err = snd_output_buffer_open(&out);
	if (err < 0) {
		delete_tree(instance, args);
		return err;
	}

	do {
		p1 = eval(instance, car(p));
		if (alisp_compare_type(p1, ALISP_OBJ_STRING))
			snd_output_printf(out, "%s", p1->value.s);
		else
			princ_object(out, p1);
		delete_tree(instance, p1);
		p = cdr(p1 = p);
		delete_object(instance, p1);
	} while (p != &alsa_lisp_nil);

	*rout = out;
	return 0;
}

static struct alisp_object * F_snderr(struct alisp_instance *instance, struct alisp_object * args)
{
	snd_output_t *out;
	char *str;

	if (common_error(&out, instance, args) < 0)
		return &alsa_lisp_nil;
	snd_output_buffer_string(out, &str);
	SNDERR(str);
	snd_output_close(out);
	return &alsa_lisp_t;
}

static struct alisp_object * F_syserr(struct alisp_instance *instance, struct alisp_object * args)
{
	snd_output_t *out;
	char *str;

	if (common_error(&out, instance, args) < 0)
		return &alsa_lisp_nil;
	snd_output_buffer_string(out, &str);
	SYSERR(str);
	snd_output_close(out);
	return &alsa_lisp_t;
}

static const struct intrinsic snd_intrinsics[] = {
	{ "Acall", F_acall },
	{ "Aerror", F_aerror },
	{ "Ahandle", F_ahandle },
	{ "Aresult", F_ahandle },
	{ "Asnderr", F_snderr },
	{ "Asyserr", F_syserr }
};
