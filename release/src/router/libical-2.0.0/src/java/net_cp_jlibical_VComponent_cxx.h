
/*======================================================================
 FILE: net_cp_jlibical_VComponent_cxx.h
 CREATOR: javah 1/11/02
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

#ifndef _Included_net_cp_jlibical_VComponent
#define _Included_net_cp_jlibical_VComponent

#ifndef JNI_H
#include <jni.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    as_ical_string
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_VComponent_as_1ical_1string
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    isa
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_VComponent_isa
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    isa_component
 * Signature: (Ljava/lang/Object;)Z
 */
JNIEXPORT jboolean JNICALL Java_net_cp_jlibical_VComponent_isa_1component
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    add_property
 * Signature: (Lnet/cp/jlibical/ICalProperty;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_add_1property
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    remove_property
 * Signature: (Lnet/cp/jlibical/ICalProperty;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_remove_1property
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    count_properties
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_VComponent_count_1properties
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_current_property
 * Signature: ()Lnet/cp/jlibical/ICalProperty;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1current_1property
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_first_property
 * Signature: (I)Lnet/cp/jlibical/ICalProperty;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1first_1property
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_next_property
 * Signature: (I)Lnet/cp/jlibical/ICalProperty;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1next_1property
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_inner
 * Signature: ()Lnet/cp/jlibical/VComponent;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1inner
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    add_component
 * Signature: (Lnet/cp/jlibical/VComponent;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_add_1component
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    remove_component
 * Signature: (Lnet/cp/jlibical/VComponent;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_remove_1component
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    count_components
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_VComponent_count_1components
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_current_component
 * Signature: ()Lnet/cp/jlibical/VComponent;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1current_1component
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_first_component
 * Signature: (I)Lnet/cp/jlibical/VComponent;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1first_1component
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_next_component
 * Signature: (I)Lnet/cp/jlibical/VComponent;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1next_1component
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_dtstart
 * Signature: ()Lnet/cp/jlibical/ICalTimeType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1dtstart
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    set_dtstart
 * Signature: (Lnet/cp/jlibical/ICalTimeType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_set_1dtstart
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_dtend
 * Signature: ()Lnet/cp/jlibical/ICalTimeType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1dtend
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    set_dtend
 * Signature: (Lnet/cp/jlibical/ICalTimeType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_set_1dtend
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_duration
 * Signature: ()Lnet/cp/jlibical/ICalDurationType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1duration
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    set_duration
 * Signature: (Lnet/cp/jlibical/ICalDurationType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_set_1duration
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_method
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_VComponent_get_1method
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    set_method
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_set_1method
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_summary
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_VComponent_get_1summary
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    set_summary
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_set_1summary
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_dtstamp
 * Signature: ()Lnet/cp/jlibical/ICalTimeType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1dtstamp
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    set_dtstamp
 * Signature: (Lnet/cp/jlibical/ICalTimeType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_set_1dtstamp
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_location
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_VComponent_get_1location
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    set_location
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_set_1location
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_description
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_VComponent_get_1description
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    set_description
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_set_1description
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_uid
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_VComponent_get_1uid
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    set_uid
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_set_1uid
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_first_real_component
 * Signature: ()Lnet/cp/jlibical/VComponent;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1first_1real_1component
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_init__
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    init
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_init__Ljava_lang_String_2
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    init
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_init__I
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_relcalid
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_VComponent_get_1relcalid
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    set_relcalid
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_set_1relcalid
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_recurrenceid
 * Signature: ()Lnet/cp/jlibical/ICalTimeType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1recurrenceid
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    set_recurrenceid
 * Signature: (Lnet/cp/jlibical/ICalTimeType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_set_1recurrenceid
  (JNIEnv *, jobject, jobject);

#ifdef __cplusplus
}
#endif
#endif
