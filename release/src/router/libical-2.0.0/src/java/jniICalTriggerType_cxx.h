
/*======================================================================
 FILE: jniICalTriggerType_cxx.h
 CREATOR: structConverter
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

#ifndef _jni_ICalTriggerType_H
#define _jni_ICalTriggerType_H
#include <jni.h>

// I forgot how to do this using a typedef in c++!!!!
#define ICalTriggerType icaltriggertype


#ifdef __cplusplus
extern "C" {
#endif

#include "ical.h"

static void initICalTriggerTypeFieldIDs(JNIEnv* env, jclass clazz);

void  jni_SetTime_in_ICalTriggerType(struct ICalTriggerType* __ICalTriggerType_, JNIEnv *env, jobject thisICalTriggerType);
void jni_GetTime_from_ICalTriggerType(struct ICalTriggerType* __ICalTriggerType_, JNIEnv *env, jobject thisICalTriggerType);
void  jni_SetDuration_in_ICalTriggerType(struct ICalTriggerType* __ICalTriggerType_, JNIEnv *env, jobject thisICalTriggerType);
void jni_GetDuration_from_ICalTriggerType(struct ICalTriggerType* __ICalTriggerType_, JNIEnv *env, jobject thisICalTriggerType);
void jni_SetAll_in_ICalTriggerType(struct ICalTriggerType* __ICalTriggerType_, JNIEnv* env, jobject thisICalTriggerType);

void jni_GetAll_from_ICalTriggerType(struct ICalTriggerType* __ICalTriggerType_, JNIEnv* env, jobject thisICalTriggerType);
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalTriggerType_init__J(JNIEnv* env, jobject thisICalTriggerType, jlong data);
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalTriggerType_initFIDs(JNIEnv *env, jclass clazz);


#ifdef __cplusplus
}
#endif

#endif
