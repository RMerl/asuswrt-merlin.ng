
/*======================================================================
 FILE: jniICalTimeType_cxx.h
 CREATOR: structConverter
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

#ifndef _jni_ICalTimeType_H
#define _jni_ICalTimeType_H
#include <jni.h>

// I forgot how to do this using a typedef in c++!!!!
#define ICalTimeType icaltimetype

#ifdef __cplusplus
extern "C" {
#endif

#include "ical.h"

static void initICalTimeTypeFieldIDs(JNIEnv* env, jclass clazz);

void  jni_SetYear_in_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType);
void jni_GetYear_from_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType);
void  jni_SetMonth_in_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType);
void jni_GetMonth_from_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType);
void  jni_SetDay_in_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType);
void jni_GetDay_from_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType);
void  jni_SetHour_in_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType);
void jni_GetHour_from_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType);
void  jni_SetMinute_in_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType);
void jni_GetMinute_from_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType);
void  jni_SetSecond_in_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType);
void jni_GetSecond_from_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType);
void  jni_SetIs_utc_in_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType);
void jni_GetIs_utc_from_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType);
void  jni_SetIs_date_in_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType);
void jni_GetIs_date_from_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType);
void  jni_SetZone_in_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType);
void jni_GetZone_from_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType);
void jni_SetAll_in_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv* env, jobject thisICalTimeType);

void jni_GetAll_from_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv* env, jobject thisICalTimeType);
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalTimeType_initFIDs(JNIEnv *env, jclass clazz);
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalTimeType_init__J(JNIEnv* env, jobject thisICalTimeType, jlong data);


#ifdef __cplusplus
}
#endif

#endif
