/*
 * Copyright (C) 2012-2014 Tobias Brunner
 * Copyright (C) 2012 Giuliano Grassi
 * Copyright (C) 2012 Ralf Sager
 * HSR Hochschule fuer Technik Rapperswil
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

#include "vpnservice_builder.h"
#include "android_jni.h"

#include <utils/debug.h>
#include <library.h>

typedef struct private_vpnservice_builder_t private_vpnservice_builder_t;

/**
 * private data of vpnservice_builder
 */
struct private_vpnservice_builder_t {

	/**
	 * public interface
	 */
	vpnservice_builder_t public;

	/**
	 * Java object
	 */
	jobject builder;
};

METHOD(vpnservice_builder_t, add_address, bool,
	private_vpnservice_builder_t *this, host_t *addr)
{
	JNIEnv *env;
	jmethodID method_id;
	jstring str;
	char buf[INET6_ADDRSTRLEN];
	int prefix;

	androidjni_attach_thread(&env);

	DBG2(DBG_LIB, "builder: adding interface address %H", addr);

	prefix = addr->get_family(addr) == AF_INET ? 32 : 128;
	if (snprintf(buf, sizeof(buf), "%H", addr) >= sizeof(buf))
	{
		goto failed;
	}

	method_id = (*env)->GetMethodID(env, android_charonvpnservice_builder_class,
									"addAddress", "(Ljava/lang/String;I)Z");
	if (!method_id)
	{
		goto failed;
	}
	str = (*env)->NewStringUTF(env, buf);
	if (!str)
	{
		goto failed;
	}
	if (!(*env)->CallBooleanMethod(env, this->builder, method_id, str, prefix))
	{
		goto failed;
	}
	androidjni_detach_thread();
	return TRUE;

failed:
	DBG1(DBG_LIB, "builder: failed to add address");
	androidjni_exception_occurred(env);
	androidjni_detach_thread();
	return FALSE;
}

METHOD(vpnservice_builder_t, set_mtu, bool,
	private_vpnservice_builder_t *this, int mtu)
{
	JNIEnv *env;
	jmethodID method_id;

	androidjni_attach_thread(&env);

	DBG2(DBG_LIB, "builder: setting MTU to %d", mtu);

	method_id = (*env)->GetMethodID(env, android_charonvpnservice_builder_class,
									"setMtu", "(I)Z");
	if (!method_id)
	{
		goto failed;
	}
	if (!(*env)->CallBooleanMethod(env, this->builder, method_id, mtu))
	{
		goto failed;
	}
	androidjni_detach_thread();
	return TRUE;

failed:
	DBG1(DBG_LIB, "builder: failed to set MTU");
	androidjni_exception_occurred(env);
	androidjni_detach_thread();
	return FALSE;
}

METHOD(vpnservice_builder_t, add_route, bool,
	private_vpnservice_builder_t *this, host_t *net, int prefix)
{
	JNIEnv *env;
	jmethodID method_id;
	jstring str;
	char buf[INET6_ADDRSTRLEN];

	androidjni_attach_thread(&env);

	DBG2(DBG_LIB, "builder: adding route %+H/%d", net, prefix);

	if (snprintf(buf, sizeof(buf), "%+H", net) >= sizeof(buf))
	{
		goto failed;
	}

	method_id = (*env)->GetMethodID(env, android_charonvpnservice_builder_class,
									"addRoute", "(Ljava/lang/String;I)Z");
	if (!method_id)
	{
		goto failed;
	}
	str = (*env)->NewStringUTF(env, buf);
	if (!str)
	{
		goto failed;
	}
	if (!(*env)->CallBooleanMethod(env, this->builder, method_id, str, prefix))
	{
		goto failed;
	}
	androidjni_detach_thread();
	return TRUE;

failed:
	DBG1(DBG_LIB, "builder: failed to add route");
	androidjni_exception_occurred(env);
	androidjni_detach_thread();
	return FALSE;
}

METHOD(vpnservice_builder_t, add_dns, bool,
	private_vpnservice_builder_t *this, host_t *dns)
{
	JNIEnv *env;
	jmethodID method_id;
	jstring str;
	char buf[INET6_ADDRSTRLEN];

	androidjni_attach_thread(&env);

	DBG2(DBG_LIB, "builder: adding DNS server %H", dns);

	if (snprintf(buf, sizeof(buf), "%H", dns) >= sizeof(buf))
	{
		goto failed;
	}

	method_id = (*env)->GetMethodID(env, android_charonvpnservice_builder_class,
									"addDnsServer", "(Ljava/lang/String;)Z");
	if (!method_id)
	{
		goto failed;
	}
	str = (*env)->NewStringUTF(env, buf);
	if (!str)
	{
		goto failed;
	}
	if (!(*env)->CallBooleanMethod(env, this->builder, method_id, str))
	{
		goto failed;
	}
	androidjni_detach_thread();
	return TRUE;

failed:
	DBG1(DBG_LIB, "builder: failed to add DNS server");
	androidjni_exception_occurred(env);
	androidjni_detach_thread();
	return FALSE;
}

/**
 * Establish or reestablish the TUN device
 */
static int establish_internal(private_vpnservice_builder_t *this, char *method)
{
	JNIEnv *env;
	jmethodID method_id;
	int fd;

	androidjni_attach_thread(&env);

	DBG2(DBG_LIB, "builder: building TUN device");

	method_id = (*env)->GetMethodID(env, android_charonvpnservice_builder_class,
									method, "()I");
	if (!method_id)
	{
		goto failed;
	}
	fd = (*env)->CallIntMethod(env, this->builder, method_id);
	if (fd == -1)
	{
		goto failed;
	}
	androidjni_detach_thread();
	return fd;

failed:
	DBG1(DBG_LIB, "builder: failed to build TUN device");
	androidjni_exception_occurred(env);
	androidjni_detach_thread();
	return -1;
}

METHOD(vpnservice_builder_t, establish, int,
	private_vpnservice_builder_t *this)
{
	return establish_internal(this, "establish");
}

METHOD(vpnservice_builder_t, establish_no_dns, int,
	private_vpnservice_builder_t *this)
{
	return establish_internal(this, "establishNoDns");
}

METHOD(vpnservice_builder_t, destroy, void,
	private_vpnservice_builder_t *this)
{
	JNIEnv *env;

	androidjni_attach_thread(&env);
	(*env)->DeleteGlobalRef(env, this->builder);
	androidjni_detach_thread();
	free(this);
}

vpnservice_builder_t *vpnservice_builder_create(jobject builder)
{
	JNIEnv *env;
	private_vpnservice_builder_t *this;

	INIT(this,
		.public = {
			.add_address = _add_address,
			.add_route = _add_route,
			.add_dns = _add_dns,
			.set_mtu = _set_mtu,
			.establish = _establish,
			.establish_no_dns = _establish_no_dns,
			.destroy = _destroy,
		},
	);

	androidjni_attach_thread(&env);
	this->builder = (*env)->NewGlobalRef(env, builder);
	androidjni_detach_thread();

	return &this->public;
}
