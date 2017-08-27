
/*======================================================================
 FILE: net_cp_jlibical_ICalValue_cxx.h
 CREATOR: javah 1/11/02
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

#ifndef _Included_net_cp_jlibical_ICalValue
#define _Included_net_cp_jlibical_ICalValue

#ifndef JNI_H
#include <jni.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    as_ical_string
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalValue_as_1ical_1string
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    isa
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalValue_isa
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    isa_value
 * Signature: (Ljava/lang/Object;)Z
 */
JNIEXPORT jboolean JNICALL Java_net_cp_jlibical_ICalValue_isa_1value
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    set_trigger
 * Signature: (Lnet/cp/jlibical/ICalTriggerType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalValue_set_1trigger
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    get_trigger
 * Signature: ()Lnet/cp/jlibical/ICalTriggerType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalValue_get_1trigger
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    get_method
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalValue_get_1method
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    set_method
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalValue_set_1method
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    get_text
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalValue_get_1text
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    set_text
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalValue_set_1text
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    get_duration
 * Signature: ()Lnet/cp/jlibical/ICalDurationType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalValue_get_1duration
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    set_duration
 * Signature: (Lnet/cp/jlibical/ICalDurationType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalValue_set_1duration
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    get_query
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalValue_get_1query
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    set_query
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalValue_set_1query
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    get_datetime
 * Signature: ()Lnet/cp/jlibical/ICalTimeType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalValue_get_1datetime
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    set_datetime
 * Signature: (Lnet/cp/jlibical/ICalTimeType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalValue_set_1datetime
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    get_action
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalValue_get_1action
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    set_action
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalValue_set_1action
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalValue_init__
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    init
 * Signature: (ILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalValue_init__ILjava_lang_String_2
  (JNIEnv *, jobject, jint, jstring);

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    init
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalValue_init__I
  (JNIEnv *, jobject, jint);

#ifdef __cplusplus
}
#endif
#endif
