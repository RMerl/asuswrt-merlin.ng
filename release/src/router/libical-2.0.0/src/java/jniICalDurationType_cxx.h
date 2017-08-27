
/*======================================================================
 FILE: jniICalDurationType_cxx.h
 CREATOR: structConverter
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

#ifndef _jni_ICalDurationType_H
#define _jni_ICalDurationType_H
#include <jni.h>

// I forgot how to do this using a typedef in c++!!!!
#define ICalDurationType icaldurationtype


#ifdef __cplusplus
extern "C" {
#endif

#include "ical.h"

static void initICalDurationTypeFieldIDs(JNIEnv* env, jclass clazz);

void  jni_SetIs_neg_in_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType);
void jni_GetIs_neg_from_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType);
void  jni_SetDays_in_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType);
void jni_GetDays_from_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType);
void  jni_SetWeeks_in_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType);
void jni_GetWeeks_from_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType);
void  jni_SetHours_in_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType);
void jni_GetHours_from_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType);
void  jni_SetMinutes_in_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType);
void jni_GetMinutes_from_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType);
void  jni_SetSeconds_in_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType);
void jni_GetSeconds_from_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType);
void jni_SetAll_in_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv* env, jobject thisICalDurationType);

void jni_GetAll_from_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv* env, jobject thisICalDurationType);
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalDurationType_initFIDs(JNIEnv *env, jclass clazz);
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalDurationType_init__J(JNIEnv *env, jobject thisICalDurationType, jlong data);


#ifdef __cplusplus
}
#endif

#endif
