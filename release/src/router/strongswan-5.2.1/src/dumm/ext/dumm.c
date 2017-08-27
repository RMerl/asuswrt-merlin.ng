/*
 * Copyright (C) 2008-2010 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include <library.h>
#include <dumm.h>
#include <utils/debug.h>
#include <collections/linked_list.h>

#undef PACKAGE_NAME
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef PACKAGE_STRING
#undef PACKAGE_BUGREPORT
#undef PACKAGE_URL
/* avoid redefintiion of snprintf etc. */
#define RUBY_DONT_SUBST
/* undef our _GNU_SOURCE, as it gets redefined by <ruby.h> */
#undef _GNU_SOURCE
#include <ruby.h>

static dumm_t *dumm;

static VALUE rbm_dumm;
static VALUE rbc_guest;
static VALUE rbc_bridge;
static VALUE rbc_iface;
static VALUE rbc_template;

/**
 * Guest invocation callback
 */
static pid_t invoke(void *null, guest_t *guest, char *args[], int argc)
{
	pid_t pid;

	pid = fork();
	switch (pid)
	{
		case 0: /* child */
			/* create a new process group in order to prevent signals (e.g.
			 * SIGINT) sent to the parent from terminating the child */
			setpgid(0, 0);
			dup2(open("/dev/null", 0), 1);
			dup2(open("/dev/null", 0), 2);
			execvp(args[0], args);
			/* FALL */
		case -1:
			return 0;
		default:
			return pid;
	}
}

/**
 * SIGCHLD signal handler
 */
static void sigchld_handler(int signal, siginfo_t *info, void* ptr)
{
	enumerator_t *enumerator;
	guest_t *guest;

	enumerator = dumm->create_guest_enumerator(dumm);
	while (enumerator->enumerate(enumerator, &guest))
	{
		if (guest->get_pid(guest) == info->si_pid)
		{
			guest->sigchild(guest);
			break;
		}
	}
	enumerator->destroy(enumerator);
}


/**
 * Global Dumm bindings
 */
static VALUE dumm_add_overlay(VALUE class, VALUE dir)
{
	if (!dumm->add_overlay(dumm, StringValuePtr(dir)))
	{
		rb_raise(rb_eRuntimeError, "loading overlay failed");
	}
	return class;
}

static VALUE dumm_del_overlay(VALUE class, VALUE dir)
{
	return dumm->del_overlay(dumm, StringValuePtr(dir)) ? Qtrue : Qfalse;
}

static VALUE dumm_pop_overlay(VALUE class)
{
	return dumm->pop_overlay(dumm) ? Qtrue : Qfalse;
}

static void dumm_init()
{
	rbm_dumm = rb_define_module("Dumm");

	rb_define_module_function(rbm_dumm, "add_overlay", dumm_add_overlay, 1);
	rb_define_module_function(rbm_dumm, "del_overlay", dumm_del_overlay, 1);
	rb_define_module_function(rbm_dumm, "pop_overlay", dumm_pop_overlay, 0);
}

/**
 * Guest bindings
 */
static VALUE guest_hash_create(VALUE class)
{
	enumerator_t *enumerator;
	guest_t *guest;
	VALUE hash = rb_hash_new();
	enumerator = dumm->create_guest_enumerator(dumm);
	while (enumerator->enumerate(enumerator, &guest))
	{
		rb_hash_aset(hash, rb_str_new2(guest->get_name(guest)),
					 Data_Wrap_Struct(class, NULL, NULL, guest));
	}
	enumerator->destroy(enumerator);
	return hash;
}

static VALUE guest_hash(VALUE class)
{
	ID id = rb_intern("@@guests");
	if (!rb_cvar_defined(class, id))
	{
		VALUE hash = guest_hash_create(class);
#ifdef RB_CVAR_SET_4_ARGS
		rb_cvar_set(class, id, hash, 0);
#else
		rb_cvar_set(class, id, hash);
#endif
		return hash;
	}
	return rb_cvar_get(class, id);
}

static VALUE guest_find(VALUE class, VALUE key)
{
	if (TYPE(key) != T_STRING)
	{
		key = rb_convert_type(key, T_STRING, "String", "to_s");
	}
	return rb_hash_aref(guest_hash(class), key);
}

static VALUE guest_get(VALUE class, VALUE key)
{
	return guest_find(class, key);
}

static VALUE guest_each(int argc, VALUE *argv, VALUE class)
{
	if (!rb_block_given_p())
	{
		rb_raise(rb_eArgError, "must be called with a block");
	}
	rb_block_call(guest_hash(class), rb_intern("each_value"), 0, 0,
				  rb_yield, 0);
	return class;
}

static VALUE guest_new(VALUE class, VALUE name, VALUE kernel,
					   VALUE master, VALUE args)
{
	VALUE self;
	guest_t *guest;
	guest = dumm->create_guest(dumm, StringValuePtr(name),
							   StringValuePtr(kernel), StringValuePtr(master),
							   StringValuePtr(args));
	if (!guest)
	{
		rb_raise(rb_eRuntimeError, "creating guest failed");
	}
	self = Data_Wrap_Struct(class, NULL, NULL, guest);
	rb_hash_aset(guest_hash(class), name, self);
	return self;
}

static VALUE guest_to_s(VALUE self)
{
	guest_t *guest;

	Data_Get_Struct(self, guest_t, guest);
	return rb_str_new2(guest->get_name(guest));
}

static VALUE guest_start(VALUE self)
{
	guest_t *guest;

	Data_Get_Struct(self, guest_t, guest);

	if (!guest->start(guest, invoke, NULL, NULL))
	{
		rb_raise(rb_eRuntimeError, "starting guest failed");
	}
	return self;
}

static VALUE guest_stop(VALUE self)
{
	guest_t *guest;

	Data_Get_Struct(self, guest_t, guest);
	guest->stop(guest, NULL);
	return self;
}

static VALUE guest_running(VALUE self)
{
	guest_t *guest;

	Data_Get_Struct(self, guest_t, guest);
	return guest->get_pid(guest) ? Qtrue : Qfalse;
}

static void exec_cb(void *data, char *buf)
{
	rb_yield(rb_str_new2(buf));
}

static VALUE guest_exec(VALUE self, VALUE cmd)
{
	guest_t *guest;
	bool block;
	int ret;

	block = rb_block_given_p();
	Data_Get_Struct(self, guest_t, guest);
	ret = guest->exec_str(guest, block ? (void*)exec_cb : NULL, TRUE, NULL,
						  "exec %s", StringValuePtr(cmd));
	rb_iv_set(self, "@execstatus", INT2NUM(ret));
	return self;
}

static VALUE guest_mconsole(VALUE self, VALUE cmd)
{
	guest_t *guest;
	bool block;
	int ret;

	block = rb_block_given_p();
	Data_Get_Struct(self, guest_t, guest);
	if ((ret = guest->exec_str(guest, block ? (void*)exec_cb : NULL, TRUE, NULL,
					"%s", StringValuePtr(cmd))) != 0)
	{
		rb_raise(rb_eRuntimeError, "executing command failed (%d)", ret);
	}
	return self;
}

static VALUE guest_add_iface(VALUE self, VALUE name)
{
	guest_t *guest;
	iface_t *iface;

	Data_Get_Struct(self, guest_t, guest);
	iface = guest->create_iface(guest, StringValuePtr(name));
	if (!iface)
	{
		rb_raise(rb_eRuntimeError, "adding interface failed");
	}
	return Data_Wrap_Struct(rbc_iface, NULL, NULL, iface);
}

static VALUE guest_find_iface(VALUE self, VALUE key)
{
	enumerator_t *enumerator;
	iface_t *iface, *found = NULL;
	guest_t *guest;

	if (TYPE(key) == T_SYMBOL)
	{
		key = rb_convert_type(key, T_STRING, "String", "to_s");
	}
	Data_Get_Struct(self, guest_t, guest);
	enumerator = guest->create_iface_enumerator(guest);
	while (enumerator->enumerate(enumerator, &iface))
	{
		if (streq(iface->get_guestif(iface), StringValuePtr(key)))
		{
			found = iface;
			break;
		}
	}
	enumerator->destroy(enumerator);
	if (!found)
	{
		return Qnil;
	}
	return Data_Wrap_Struct(rbc_iface, NULL, NULL, iface);
}

static VALUE guest_get_iface(VALUE self, VALUE key)
{
	VALUE iface = guest_find_iface(self, key);
	if (NIL_P(iface))
	{
		rb_raise(rb_eRuntimeError, "interface not found");
	}
	return iface;
}

static VALUE guest_each_iface(int argc, VALUE *argv, VALUE self)
{
	enumerator_t *enumerator;
	linked_list_t *list;
	guest_t *guest;
	iface_t *iface;

	if (!rb_block_given_p())
	{
		rb_raise(rb_eArgError, "must be called with a block");
	}
	Data_Get_Struct(self, guest_t, guest);
	list = linked_list_create();
	enumerator = guest->create_iface_enumerator(guest);
	while (enumerator->enumerate(enumerator, &iface))
	{
		list->insert_last(list, iface);
	}
	enumerator->destroy(enumerator);
	while (list->remove_first(list, (void**)&iface) == SUCCESS)
	{
		rb_yield(Data_Wrap_Struct(rbc_iface, NULL, NULL, iface));
	}
	list->destroy(list);
	return self;
}

static VALUE guest_delete(VALUE self)
{
	guest_t *guest;

	Data_Get_Struct(self, guest_t, guest);
	if (guest->get_pid(guest))
	{
		rb_raise(rb_eRuntimeError, "guest is running");
	}
	dumm->delete_guest(dumm, guest);
	return Qnil;
}

static VALUE guest_add_overlay(VALUE self, VALUE dir)
{
	guest_t *guest;

	Data_Get_Struct(self, guest_t, guest);
	if (!guest->add_overlay(guest, StringValuePtr(dir)))
	{
		rb_raise(rb_eRuntimeError, "loading overlay failed");
	}
	return self;
}

static VALUE guest_del_overlay(VALUE self, VALUE dir)
{
	guest_t *guest;

	Data_Get_Struct(self, guest_t, guest);
	return guest->del_overlay(guest, StringValuePtr(dir)) ? Qtrue : Qfalse;
}

static VALUE guest_pop_overlay(VALUE self)
{
	guest_t *guest;

	Data_Get_Struct(self, guest_t, guest);
	return guest->pop_overlay(guest) ? Qtrue : Qfalse;
}

static void guest_init()
{
	rbc_guest = rb_define_class_under(rbm_dumm , "Guest", rb_cObject);
	rb_include_module(rb_class_of(rbc_guest), rb_mEnumerable);
	rb_include_module(rbc_guest, rb_mEnumerable);

	rb_define_singleton_method(rbc_guest, "[]", guest_get, 1);
	rb_define_singleton_method(rbc_guest, "each", guest_each, -1);
	rb_define_singleton_method(rbc_guest, "new", guest_new, 4);
	rb_define_singleton_method(rbc_guest, "include?", guest_find, 1);
	rb_define_singleton_method(rbc_guest, "guest?", guest_find, 1);

	rb_define_method(rbc_guest, "to_s", guest_to_s, 0);
	rb_define_method(rbc_guest, "start", guest_start, 0);
	rb_define_method(rbc_guest, "stop", guest_stop, 0);
	rb_define_method(rbc_guest, "running?", guest_running, 0);
	rb_define_method(rbc_guest, "exec", guest_exec, 1);
	rb_define_method(rbc_guest, "mconsole", guest_mconsole, 1);
	rb_define_method(rbc_guest, "add", guest_add_iface, 1);
	rb_define_method(rbc_guest, "[]", guest_get_iface, 1);
	rb_define_method(rbc_guest, "each", guest_each_iface, -1);
	rb_define_method(rbc_guest, "include?", guest_find_iface, 1);
	rb_define_method(rbc_guest, "iface?", guest_find_iface, 1);
	rb_define_method(rbc_guest, "delete", guest_delete, 0);
	rb_define_method(rbc_guest, "add_overlay", guest_add_overlay, 1);
	rb_define_method(rbc_guest, "del_overlay", guest_del_overlay, 1);
	rb_define_method(rbc_guest, "pop_overlay", guest_pop_overlay, 0);

	rb_define_attr(rbc_guest, "execstatus", 1, 0);
}

/**
 * Bridge binding
 */
static VALUE bridge_find(VALUE class, VALUE key)
{
	enumerator_t *enumerator;
	bridge_t *bridge, *found = NULL;

	if (TYPE(key) == T_SYMBOL)
	{
		key = rb_convert_type(key, T_STRING, "String", "to_s");
	}
	enumerator = dumm->create_bridge_enumerator(dumm);
	while (enumerator->enumerate(enumerator, &bridge))
	{
		if (streq(bridge->get_name(bridge), StringValuePtr(key)))
		{
			found = bridge;
			break;
		}
	}
	enumerator->destroy(enumerator);
	if (!found)
	{
		return Qnil;
	}
	return Data_Wrap_Struct(class, NULL, NULL, found);
}

static VALUE bridge_get(VALUE class, VALUE key)
{
	VALUE bridge = bridge_find(class, key);
	if (NIL_P(bridge))
	{
		rb_raise(rb_eRuntimeError, "bridge not found");
	}
	return bridge;
}

static VALUE bridge_each(int argc, VALUE *argv, VALUE class)
{
	enumerator_t *enumerator;
	linked_list_t *list;
	bridge_t *bridge;

	if (!rb_block_given_p())
	{
		rb_raise(rb_eArgError, "must be called with a block");
	}
	list = linked_list_create();
	enumerator = dumm->create_bridge_enumerator(dumm);
	while (enumerator->enumerate(enumerator, &bridge))
	{
		list->insert_last(list, bridge);
	}
	enumerator->destroy(enumerator);
	while (list->remove_first(list, (void**)&bridge) == SUCCESS)
	{
		rb_yield(Data_Wrap_Struct(class, NULL, NULL, bridge));
	}
	list->destroy(list);
	return class;
}

static VALUE bridge_new(VALUE class, VALUE name)

{
	bridge_t *bridge;

	bridge = dumm->create_bridge(dumm, StringValuePtr(name));
	if (!bridge)
	{
		rb_raise(rb_eRuntimeError, "creating bridge failed");
	}
	return Data_Wrap_Struct(class, NULL, NULL, bridge);
}

static VALUE bridge_to_s(VALUE self)
{
	bridge_t *bridge;

	Data_Get_Struct(self, bridge_t, bridge);
	return rb_str_new2(bridge->get_name(bridge));
}

static VALUE bridge_each_iface(int argc, VALUE *argv, VALUE self)
{
	enumerator_t *enumerator;
	linked_list_t *list;
	bridge_t *bridge;
	iface_t *iface;

	if (!rb_block_given_p())
	{
		rb_raise(rb_eArgError, "must be called with a block");
	}
	Data_Get_Struct(self, bridge_t, bridge);
	list = linked_list_create();
	enumerator = bridge->create_iface_enumerator(bridge);
	while (enumerator->enumerate(enumerator, &iface))
	{
		list->insert_last(list, iface);
	}
	enumerator->destroy(enumerator);
	while (list->remove_first(list, (void**)&iface) == SUCCESS)
	{
		rb_yield(Data_Wrap_Struct(rbc_iface, NULL, NULL, iface));
	}
	list->destroy(list);
	return self;
}

static VALUE bridge_delete(VALUE self)
{
	bridge_t *bridge;

	Data_Get_Struct(self, bridge_t, bridge);
	dumm->delete_bridge(dumm, bridge);
	return Qnil;
}

static void bridge_init()
{
	rbc_bridge = rb_define_class_under(rbm_dumm , "Bridge", rb_cObject);
	rb_include_module(rb_class_of(rbc_bridge), rb_mEnumerable);
	rb_include_module(rbc_bridge, rb_mEnumerable);

	rb_define_singleton_method(rbc_bridge, "[]", bridge_get, 1);
	rb_define_singleton_method(rbc_bridge, "each", bridge_each, -1);
	rb_define_singleton_method(rbc_bridge, "new", bridge_new, 1);
	rb_define_singleton_method(rbc_bridge, "include?", bridge_find, 1);
	rb_define_singleton_method(rbc_bridge, "bridge?", bridge_find, 1);

	rb_define_method(rbc_bridge, "to_s", bridge_to_s, 0);
	rb_define_method(rbc_bridge, "each", bridge_each_iface, -1);
	rb_define_method(rbc_bridge, "delete", bridge_delete, 0);
}

/**
 * Iface wrapper
 */
static VALUE iface_to_s(VALUE self)
{
	iface_t *iface;

	Data_Get_Struct(self, iface_t, iface);
	return rb_str_new2(iface->get_hostif(iface));
}

static VALUE iface_connect(VALUE self, VALUE vbridge)
{
	iface_t *iface;
	bridge_t *bridge;

	Data_Get_Struct(self, iface_t, iface);
	Data_Get_Struct(vbridge, bridge_t, bridge);
	if (!bridge->connect_iface(bridge, iface))
	{
		rb_raise(rb_eRuntimeError, "connecting iface failed");
	}
	return self;
}

static VALUE iface_disconnect(VALUE self)
{
	iface_t *iface;
	bridge_t *bridge;

	Data_Get_Struct(self, iface_t, iface);
	bridge = iface->get_bridge(iface);
	if (!bridge || !bridge->disconnect_iface(bridge, iface))
	{
		rb_raise(rb_eRuntimeError, "disconnecting iface failed");
	}
	return self;
}

static VALUE iface_add_addr(VALUE self, VALUE name)
{
	iface_t *iface;
	host_t *addr;
	int bits;

	addr = host_create_from_subnet(StringValuePtr(name), &bits);
	if (!addr)
	{
		rb_raise(rb_eArgError, "invalid IP address");
	}
	Data_Get_Struct(self, iface_t, iface);
	if (!iface->add_address(iface, addr, bits))
	{
		addr->destroy(addr);
		rb_raise(rb_eRuntimeError, "adding address failed");
	}
	if (rb_block_given_p()) {
		rb_yield(self);
		iface->delete_address(iface, addr, bits);
	}
	addr->destroy(addr);
	return self;
}

static VALUE iface_each_addr(int argc, VALUE *argv, VALUE self)
{
	enumerator_t *enumerator;
	linked_list_t *list;
	iface_t *iface;
	host_t *addr;
	char buf[64];

	if (!rb_block_given_p())
	{
		rb_raise(rb_eArgError, "must be called with a block");
	}
	list = linked_list_create();
	Data_Get_Struct(self, iface_t, iface);
	enumerator = iface->create_address_enumerator(iface);
	while (enumerator->enumerate(enumerator, &addr))
	{
		list->insert_last(list, addr->clone(addr));
	}
	enumerator->destroy(enumerator);
	while (list->remove_first(list, (void**)&addr) == SUCCESS)
	{
		snprintf(buf, sizeof(buf), "%H", addr);
		addr->destroy(addr);
		rb_yield(rb_str_new2(buf));
	}
	list->destroy(list);
	return self;
}

static VALUE iface_del_addr(VALUE self, VALUE vaddr)
{
	iface_t *iface;
	host_t *addr;
	int bits;

	addr = host_create_from_subnet(StringValuePtr(vaddr), &bits);
	if (!addr)
	{
		rb_raise(rb_eArgError, "invalid IP address");
	}
	Data_Get_Struct(self, iface_t, iface);
	if (!iface->delete_address(iface, addr, bits))
	{
		addr->destroy(addr);
		rb_raise(rb_eRuntimeError, "address not found");
	}
	if (rb_block_given_p()) {
		rb_yield(self);
		iface->add_address(iface, addr, bits);
	}
	addr->destroy(addr);
	return self;
}

static VALUE iface_delete(VALUE self)
{
	guest_t *guest;
	iface_t *iface;

	Data_Get_Struct(self, iface_t, iface);
	guest = iface->get_guest(iface);
	guest->destroy_iface(guest, iface);
	return Qnil;
}

static void iface_init()
{
	rbc_iface = rb_define_class_under(rbm_dumm , "Iface", rb_cObject);
	rb_include_module(rbc_iface, rb_mEnumerable);

	rb_define_method(rbc_iface, "to_s", iface_to_s, 0);
	rb_define_method(rbc_iface, "connect", iface_connect, 1);
	rb_define_method(rbc_iface, "disconnect", iface_disconnect, 0);
	rb_define_method(rbc_iface, "add", iface_add_addr, 1);
	rb_define_method(rbc_iface, "del", iface_del_addr, 1);
	rb_define_method(rbc_iface, "each", iface_each_addr, -1);
	rb_define_method(rbc_iface, "delete", iface_delete, 0);
}

static VALUE template_load(VALUE class, VALUE dir)
{
	if (!dumm->load_template(dumm, StringValuePtr(dir)))
	{
		rb_raise(rb_eRuntimeError, "loading template failed");
	}
	return class;
}

static VALUE template_unload(VALUE class)
{
	if (!dumm->load_template(dumm, NULL))
	{
		rb_raise(rb_eRuntimeError, "unloading template failed");
	}
	return class;
}

static VALUE template_each(int argc, VALUE *argv, VALUE class)
{
	enumerator_t *enumerator;
	char *template;

	if (!rb_block_given_p())
	{
		rb_raise(rb_eArgError, "must be called with a block");
	}
	enumerator = dumm->create_template_enumerator(dumm);
	while (enumerator->enumerate(enumerator, &template))
	{
		rb_yield(rb_str_new2(template));
	}
	enumerator->destroy(enumerator);
	return class;
}

static void template_init()
{
	rbc_template = rb_define_class_under(rbm_dumm , "Template", rb_cObject);
	rb_include_module(rb_class_of(rbc_template), rb_mEnumerable);

	rb_define_singleton_method(rbc_template, "load", template_load, 1);
	rb_define_singleton_method(rbc_template, "unload", template_unload, 0);
	rb_define_singleton_method(rbc_template, "each", template_each, -1);
}

/**
 * extension finalization
 */
void Final_dumm()
{
	struct sigaction action;

	dumm->destroy(dumm);

	sigemptyset(&action.sa_mask);
	action.sa_handler = SIG_DFL;
	action.sa_flags = 0;
	sigaction(SIGCHLD, &action, NULL);

	library_deinit();
}

/**
 * extension initialization
 */
void Init_dumm()
{
	struct sigaction action;

	/* there are too many to report, rubyruby... */
	setenv("LEAK_DETECTIVE_DISABLE", "1", 1);

	library_init(NULL, "dumm");

	dumm = dumm_create(NULL);

	dumm_init();
	guest_init();
	bridge_init();
	iface_init();
	template_init();

	sigemptyset(&action.sa_mask);
	action.sa_sigaction = sigchld_handler;
	action.sa_flags = SA_SIGINFO;
	sigaction(SIGCHLD, &action, NULL);

	rb_set_end_proc(Final_dumm, 0);
}
