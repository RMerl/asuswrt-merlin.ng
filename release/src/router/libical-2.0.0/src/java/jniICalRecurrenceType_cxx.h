
/*======================================================================
 FILE: jniICalRecurrenceType_cxx.h
 CREATOR: structConverter
======================================================================*/

#ifndef _jni_ICalRecurrenceType_H_
#define _jni_ICalRecurrenceType_H_
#include <jni.h>

#define ICalRecurrenceType icalrecurrencetype

#ifdef __cplusplus
extern "C" {
#endif

#include "ical.h"

static void initICalRecurrenceTypeFieldIDs(JNIEnv* env, jclass clazz);

void  jni_SetUntil_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void jni_GetUntil_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void  jni_SetFreq_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void jni_GetFreq_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void  jni_SetWeek_start_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void jni_GetWeek_start_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void  jni_SetCount_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void jni_GetCount_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void  jni_SetInterval_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void jni_GetInterval_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void  jni_SetBy_second_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void jni_GetBy_second_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void  jni_SetBy_minute_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void jni_GetBy_minute_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void  jni_SetBy_hour_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void jni_GetBy_hour_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void  jni_SetBy_day_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void jni_GetBy_day_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void  jni_SetBy_month_day_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void jni_GetBy_month_day_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void  jni_SetBy_year_day_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void jni_GetBy_year_day_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void  jni_SetBy_week_no_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void jni_GetBy_week_no_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void  jni_SetBy_month_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void jni_GetBy_month_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void  jni_SetBy_set_pos_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void jni_GetBy_set_pos_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType);
void jni_SetAll_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv* env, jobject thisICalRecurrenceType);

void jni_GetAll_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv* env, jobject thisICalRecurrenceType);
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalRecurrenceType_init__J(JNIEnv* env, jobject thisICalRecurrenceType, jlong data);
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalRecurrenceType_initFIDs(JNIEnv *env, jclass clazz);


#ifdef __cplusplus
}
#endif

#endif
