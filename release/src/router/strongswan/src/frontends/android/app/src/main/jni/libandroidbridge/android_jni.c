/*
 * Copyright (C) 2012-2017 Tobias Brunner
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

#include <dlfcn.h>

#include "android_jni.h"

#include <library.h>
#include <threading/thread_value.h>

/**
 * JVM
 */
static JavaVM *android_jvm;

static struct {
	char name[32];
	void *handle;
} libs[] = {
	{ "libstrongswan.so", NULL },
#ifdef USE_BYOD
	{ "libtpmtss.so", NULL },
	{ "libtncif.so", NULL },
	{ "libtnccs.so", NULL },
	{ "libimcv.so", NULL },
#endif
	{ "libcharon.so", NULL },
	{ "libipsec.so", NULL },
};

jclass *android_charonvpnservice_class;
jclass *android_charonvpnservice_builder_class;
jclass *android_simple_fetcher_class;
android_sdk_version_t android_sdk_version;
char *android_version_string;
char *android_device_string;

/**
 * Thread-local variable. Only used because of the destructor
 */
static thread_value_t *androidjni_threadlocal;

/**
 * Thread-local destructor to ensure that a native thread is detached
 * from the JVM even if androidjni_detach_thread() is not called.
 */
static void attached_thread_cleanup(void *arg)
{
	(*android_jvm)->DetachCurrentThread(android_jvm);
}

/*
 * Described in header
 */
void androidjni_attach_thread(JNIEnv **env)
{
	if ((*android_jvm)->GetEnv(android_jvm, (void**)env,
							   JNI_VERSION_1_6) == JNI_OK)
	{	/* already attached or even a Java thread */
		return;
	}
	(*android_jvm)->AttachCurrentThread(android_jvm, env, NULL);
	/* use a thread-local value with a destructor that automatically detaches
	 * the thread from the JVM before it terminates, if not done manually */
	androidjni_threadlocal->set(androidjni_threadlocal, (void*)*env);
}

/*
 * Described in header
 */
void androidjni_detach_thread()
{
	if (androidjni_threadlocal->get(androidjni_threadlocal))
	{	/* only do this if we actually attached this thread */
		androidjni_threadlocal->set(androidjni_threadlocal, NULL);
		(*android_jvm)->DetachCurrentThread(android_jvm);
	}
}

/**
 * Called when this library is loaded by the JVM
 */
jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
	JNIEnv *env;
	jclass jversion;
	jfieldID jsdk_int;
	jmethodID method_id;
	jstring jstr;
	int i;

	android_jvm = vm;

	if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK)
	{
		return -1;
	}

	for (i = 0; i < countof(libs); i++)
	{
		libs[i].handle = dlopen(libs[i].name, RTLD_GLOBAL);
		if (!libs[i].handle)
		{
			return -1;
		}
	}

	androidjni_threadlocal = thread_value_create(attached_thread_cleanup);

	android_charonvpnservice_class =
				(*env)->NewGlobalRef(env, (*env)->FindClass(env,
						JNI_PACKAGE_STRING "/CharonVpnService"));
	android_charonvpnservice_builder_class =
				(*env)->NewGlobalRef(env, (*env)->FindClass(env,
						JNI_PACKAGE_STRING "/CharonVpnService$BuilderAdapter"));
	android_simple_fetcher_class =
				(*env)->NewGlobalRef(env, (*env)->FindClass(env,
						JNI_PACKAGE_STRING "/SimpleFetcher"));

	jversion = (*env)->FindClass(env, "android/os/Build$VERSION");
	jsdk_int = (*env)->GetStaticFieldID(env, jversion, "SDK_INT", "I");
	android_sdk_version = (*env)->GetStaticIntField(env, jversion, jsdk_int);

	method_id = (*env)->GetStaticMethodID(env, android_charonvpnservice_class,
									"getAndroidVersion", "()Ljava/lang/String;");
	jstr = (*env)->CallStaticObjectMethod(env,
									android_charonvpnservice_class, method_id);
	if (jstr)
	{
		android_version_string = androidjni_convert_jstring(env, jstr);
	}
	method_id = (*env)->GetStaticMethodID(env, android_charonvpnservice_class,
									"getDeviceString", "()Ljava/lang/String;");
	jstr = (*env)->CallStaticObjectMethod(env,
									android_charonvpnservice_class, method_id);
	if (jstr)
	{
		android_device_string = androidjni_convert_jstring(env, jstr);
	}
	return JNI_VERSION_1_6;
}

/**
 * Called when this library is unloaded by the JVM (which never happens on
 * Android)
 */
void JNI_OnUnload(JavaVM *vm, void *reserved)
{
	int i;

	androidjni_threadlocal->destroy(androidjni_threadlocal);

	for (i = countof(libs) - 1; i >= 0; i--)
	{
		if (libs[i].handle)
		{
			dlclose(libs[i].handle);
		}
	}
	free(android_version_string);
	free(android_device_string);
}
