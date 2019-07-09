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

/**
 * @defgroup android_jni android_jni
 * @{ @ingroup libandroidbridge
 */

#ifndef ANDROID_JNI_H_
#define ANDROID_JNI_H_

#include <jni.h>
#include <library.h>

#define JNI_PACKAGE org_strongswan_android_logic
#define JNI_PACKAGE_STRING "org/strongswan/android/logic"

#define JNI_METHOD_PP(pack, klass, name, ret, ...) \
	ret Java_##pack##_##klass##_##name(JNIEnv *env, jobject this, ##__VA_ARGS__)

#define JNI_METHOD_P(pack, klass, name, ret, ...) \
	JNI_METHOD_PP(pack, klass, name, ret, ##__VA_ARGS__)

#define JNI_METHOD(klass, name, ret, ...) \
	JNI_METHOD_P(JNI_PACKAGE, klass, name, ret, ##__VA_ARGS__)

/**
 * Java classes
 * Initialized in JNI_OnLoad()
 */
extern jclass *android_charonvpnservice_class;
extern jclass *android_charonvpnservice_builder_class;
extern jclass *android_simple_fetcher_class;

/**
 * Currently known (supported) SDK versions
 *
 * see android.os.Build.VERSION_CODES for definitions
 */
typedef enum {
	ANDROID_ICE_CREAM_SANDWICH = 14,
	ANDROID_ICE_CREAM_SANDWICH_MR1 = 15,
	ANDROID_JELLY_BEAN = 16,
	ANDROID_JELLY_BEAN_MR1 = 17,
	ANDROID_JELLY_BEAN_MR2 = 18,
	ANDROID_LOLLIPOP = 21,
} android_sdk_version_t;

/**
 * The current SDK version of the Android framework
 *
 * see android.os.Build.VERSION.SDK_INT
 */
extern android_sdk_version_t android_sdk_version;

/**
 * A description of the current Android release
 *
 * see android.os.Build
 */
extern char *android_version_string;

/**
 * A description of the current device
 *
 * see android.os.Build
 */
extern char *android_device_string;

/**
 * Attach the current thread to the JVM
 *
 * As local JNI references are not freed until the thread detaches
 * androidjni_detach_thread() should be called as soon as possible.
 * If it is not called a thread-local destructor ensures that the
 * thread is at least detached as soon as it terminates.
 *
 * @param env		JNIEnv
 */
void androidjni_attach_thread(JNIEnv **env);

/**
 * Detach the current thread from the JVM
 *
 * Call this as soon as possible to ensure that local JNI references are freed.
 */
void androidjni_detach_thread();

/**
 * Handle exceptions thrown by a JNI call
 *
 * @param env		JNIEnv
 * @return			TRUE if an exception was thrown
 */
static inline bool androidjni_exception_occurred(JNIEnv *env)
{
	if ((*env)->ExceptionOccurred(env))
	{	/* clear any exception, otherwise the VM is terminated */
		(*env)->ExceptionDescribe(env);
		(*env)->ExceptionClear(env);
		return TRUE;
	}
	return FALSE;
}

/**
 * Convert a Java string to a C string.  Memory is allocated.
 *
 * @param env		JNIEnv
 * @param jstr		Java string
 * @return			native C string (allocated)
 */
static inline char *androidjni_convert_jstring(JNIEnv *env, jstring jstr)
{
	char *str = NULL;
	jsize bytes, chars;

	if (jstr)
	{
		chars = (*env)->GetStringLength(env, jstr);
		bytes = (*env)->GetStringUTFLength(env, jstr);
		str = malloc(bytes + 1);
		(*env)->GetStringUTFRegion(env, jstr, 0, chars, str);
		str[bytes] = '\0';
	}
	return str;
}

/**
 * Converts the given Java byte array to a chunk
 *
 * @param env			JNIEnv
 * @param jbytearray	Java byte array
 * @return				allocated chunk
 */
static inline chunk_t chunk_from_byte_array(JNIEnv *env, jbyteArray jbytearray)
{
	chunk_t chunk;

	chunk = chunk_alloc((*env)->GetArrayLength(env, jbytearray));
	(*env)->GetByteArrayRegion(env, jbytearray, 0, chunk.len, chunk.ptr);
	return chunk;
}

/**
 * Converts the given chunk to a Java byte array
 *
 * @param env			JNIEnv
 * @param chunk			native chunk
 * @return				allocated Java byte array
 */
static inline jbyteArray byte_array_from_chunk(JNIEnv *env, chunk_t chunk)
{
	jbyteArray jbytearray;

	jbytearray = (*env)->NewByteArray(env, chunk.len);
	(*env)->SetByteArrayRegion(env, jbytearray, 0, chunk.len, chunk.ptr);
	return jbytearray;
}

#endif /** ANDROID_JNI_H_ @}*/
