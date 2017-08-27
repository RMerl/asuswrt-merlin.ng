
/*======================================================================
 FILE: net_cp_jlibical_ICalParameter_cxx.h
 CREATOR: javah 1/11/02
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

#ifndef _Included_net_cp_jlibical_ICalParameter
#define _Included_net_cp_jlibical_ICalParameter

#ifndef JNI_H
#include <jni.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    as_ical_string
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalParameter_as_1ical_1string
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    isa
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalParameter_isa
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    isa_parameter
 * Signature: (Ljava/lang/Object;)Z
 */
JNIEXPORT jboolean JNICALL Java_net_cp_jlibical_ICalParameter_isa_1parameter
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    get_language
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalParameter_get_1language
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    set_language
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalParameter_set_1language
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    get_encoding
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalParameter_get_1encoding
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    set_encoding
 * Signature: (I)V
 */

JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalParameter_set_1encoding
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    get_role
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalParameter_get_1role
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    set_role
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalParameter_set_1role
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    get_partstat
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalParameter_get_1partstat
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    set_partstat
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalParameter_set_1partstat
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalParameter_init__
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    init
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalParameter_init__Ljava_lang_String_2
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    init
 * Signature: (ILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalParameter_init__ILjava_lang_String_2
  (JNIEnv *, jobject, jint, jstring);

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    init
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalParameter_init__I
  (JNIEnv *, jobject, jint);

#ifdef __cplusplus
}
#endif
#endif
