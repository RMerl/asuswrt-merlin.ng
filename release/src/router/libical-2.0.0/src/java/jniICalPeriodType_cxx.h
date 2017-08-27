
/*======================================================================
 FILE: jniICalPeriodType_cxx.h
 CREATOR: structConverter
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

#ifndef _jni_ICalPeriodType_H
#define _jni_ICalPeriodType_H
#include <jni.h>

// I forgot how to do this using a typedef in c++!!!!
#define ICalPeriodType icalperiodtype


#ifdef __cplusplus
extern "C" {
#endif

#include "ical.h"

static void initICalPeriodTypeFieldIDs(JNIEnv* env, jclass clazz);

void  jni_SetStart_in_ICalPeriodType(struct ICalPeriodType* __ICalPeriodType_, JNIEnv *env, jobject thisICalPeriodType);
void jni_GetStart_from_ICalPeriodType(struct ICalPeriodType* __ICalPeriodType_, JNIEnv *env, jobject thisICalPeriodType);
void  jni_SetEnd_in_ICalPeriodType(struct ICalPeriodType* __ICalPeriodType_, JNIEnv *env, jobject thisICalPeriodType);
void jni_GetEnd_from_ICalPeriodType(struct ICalPeriodType* __ICalPeriodType_, JNIEnv *env, jobject thisICalPeriodType);
void  jni_SetDuration_in_ICalPeriodType(struct ICalPeriodType* __ICalPeriodType_, JNIEnv *env, jobject thisICalPeriodType);
void jni_GetDuration_from_ICalPeriodType(struct ICalPeriodType* __ICalPeriodType_, JNIEnv *env, jobject thisICalPeriodType);

void jni_SetAll_in_ICalPeriodType(struct ICalPeriodType* __ICalPeriodType_, JNIEnv* env, jobject thisICalPeriodType);
void jni_GetAll_from_ICalPeriodType(struct ICalPeriodType* __ICalPeriodType_, JNIEnv* env, jobject thisICalPeriodType);

JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalPeriodType_init__J(JNIEnv* env, jobject thisICalPeriodType, jlong data);
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalPeriodType_initFIDs(JNIEnv *env, jclass clazz);


#ifdef __cplusplus
}
#endif

#endif
