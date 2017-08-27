/* -*- Mode: C -*- */
/*======================================================================
 FILE: jniICalTriggerType_cxx.cpp
 CREATOR: structConverter
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

#include <jni.h>

#include "jniICalTriggerType_cxx.h"
#include "jniICalTimeType_cxx.h"
#include "jniICalDurationType_cxx.h"

static jfieldID ICalTriggerType_Time_FID;
static jfieldID ICalTriggerType_Duration_FID;


void initICalTriggerTypeFieldIDs(JNIEnv* env, jclass clazz)
{
	ICalTriggerType_Time_FID = env->GetFieldID(clazz, "time", "Lnet/cp/jlibical/ICalTimeType;");
	ICalTriggerType_Duration_FID = env->GetFieldID(clazz, "duration", "Lnet/cp/jlibical/ICalDurationType;");
}

void  jni_SetTime_in_ICalTriggerType(struct ICalTriggerType* __ICalTriggerType_, JNIEnv *env, jobject thisICalTriggerType)
{
	jobject	lcl_jobj0;
	lcl_jobj0 = env->GetObjectField(thisICalTriggerType, ICalTriggerType_Time_FID);
	jni_SetYear_in_ICalTimeType(&(__ICalTriggerType_->time), env, lcl_jobj0);
	jni_SetMonth_in_ICalTimeType(&(__ICalTriggerType_->time), env, lcl_jobj0);
	jni_SetDay_in_ICalTimeType(&(__ICalTriggerType_->time), env, lcl_jobj0);
	jni_SetHour_in_ICalTimeType(&(__ICalTriggerType_->time), env, lcl_jobj0);
	jni_SetMinute_in_ICalTimeType(&(__ICalTriggerType_->time), env, lcl_jobj0);
	jni_SetSecond_in_ICalTimeType(&(__ICalTriggerType_->time), env, lcl_jobj0);
	jni_SetIs_utc_in_ICalTimeType(&(__ICalTriggerType_->time), env, lcl_jobj0);
	jni_SetIs_date_in_ICalTimeType(&(__ICalTriggerType_->time), env, lcl_jobj0);
	jni_SetZone_in_ICalTimeType(&(__ICalTriggerType_->time), env, lcl_jobj0);
}

void jni_GetTime_from_ICalTriggerType(struct ICalTriggerType* __ICalTriggerType_, JNIEnv *env, jobject thisICalTriggerType)
{
	jobject	lcl_jobj0;
	lcl_jobj0 = env->GetObjectField(thisICalTriggerType, ICalTriggerType_Time_FID);
	jni_GetYear_from_ICalTimeType(&(__ICalTriggerType_->time), env, lcl_jobj0);
	jni_GetMonth_from_ICalTimeType(&(__ICalTriggerType_->time), env, lcl_jobj0);
	jni_GetDay_from_ICalTimeType(&(__ICalTriggerType_->time), env, lcl_jobj0);
	jni_GetHour_from_ICalTimeType(&(__ICalTriggerType_->time), env, lcl_jobj0);
	jni_GetMinute_from_ICalTimeType(&(__ICalTriggerType_->time), env, lcl_jobj0);
	jni_GetSecond_from_ICalTimeType(&(__ICalTriggerType_->time), env, lcl_jobj0);
	jni_GetIs_utc_from_ICalTimeType(&(__ICalTriggerType_->time), env, lcl_jobj0);
	jni_GetIs_date_from_ICalTimeType(&(__ICalTriggerType_->time), env, lcl_jobj0);
	jni_GetZone_from_ICalTimeType(&(__ICalTriggerType_->time), env, lcl_jobj0);
}

void  jni_SetDuration_in_ICalTriggerType(struct ICalTriggerType* __ICalTriggerType_, JNIEnv *env, jobject thisICalTriggerType)
{
	jobject	lcl_jobj0;
	lcl_jobj0 = env->GetObjectField(thisICalTriggerType, ICalTriggerType_Duration_FID);
	jni_SetIs_neg_in_ICalDurationType(&(__ICalTriggerType_->duration), env, lcl_jobj0);
	jni_SetDays_in_ICalDurationType(&(__ICalTriggerType_->duration), env, lcl_jobj0);
	jni_SetWeeks_in_ICalDurationType(&(__ICalTriggerType_->duration), env, lcl_jobj0);
	jni_SetHours_in_ICalDurationType(&(__ICalTriggerType_->duration), env, lcl_jobj0);
	jni_SetMinutes_in_ICalDurationType(&(__ICalTriggerType_->duration), env, lcl_jobj0);
	jni_SetSeconds_in_ICalDurationType(&(__ICalTriggerType_->duration), env, lcl_jobj0);
}

void jni_GetDuration_from_ICalTriggerType(struct ICalTriggerType* __ICalTriggerType_, JNIEnv *env, jobject thisICalTriggerType)
{
	jobject	lcl_jobj0;
	lcl_jobj0 = env->GetObjectField(thisICalTriggerType, ICalTriggerType_Duration_FID);
	jni_GetIs_neg_from_ICalDurationType(&(__ICalTriggerType_->duration), env, lcl_jobj0);
	jni_GetDays_from_ICalDurationType(&(__ICalTriggerType_->duration), env, lcl_jobj0);
	jni_GetWeeks_from_ICalDurationType(&(__ICalTriggerType_->duration), env, lcl_jobj0);
	jni_GetHours_from_ICalDurationType(&(__ICalTriggerType_->duration), env, lcl_jobj0);
	jni_GetMinutes_from_ICalDurationType(&(__ICalTriggerType_->duration), env, lcl_jobj0);
	jni_GetSeconds_from_ICalDurationType(&(__ICalTriggerType_->duration), env, lcl_jobj0);
}

// copy all fields from the c struct (__ICalTriggerType_) to the java object (thisICalTriggerType).
void jni_SetAll_in_ICalTriggerType(struct ICalTriggerType* __ICalTriggerType_, JNIEnv* env, jobject thisICalTriggerType)
{
	jni_SetTime_in_ICalTriggerType(__ICalTriggerType_, env, thisICalTriggerType);
	jni_SetDuration_in_ICalTriggerType(__ICalTriggerType_, env, thisICalTriggerType);
}

// copy all fields from the java object (thisICalTriggerType) to the c struct (__ICalTriggerType_).
void jni_GetAll_from_ICalTriggerType(struct ICalTriggerType* __ICalTriggerType_, JNIEnv* env, jobject thisICalTriggerType)
{
	jni_GetTime_from_ICalTriggerType(__ICalTriggerType_, env, thisICalTriggerType);
	jni_GetDuration_from_ICalTriggerType(__ICalTriggerType_, env, thisICalTriggerType);
}

/*
 * Class:     net_cp_jlibical_ICalTriggerType
 * Method:    init
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalTriggerType_init__J
  (JNIEnv* env, jobject thisICalTriggerType, jlong data)
{
	// copy all fields from the c struct (data) to the java object (thisICalTimeType). 
	jni_SetAll_in_ICalTriggerType((ICalTriggerType*)data,env,thisICalTriggerType);
}

/*
 * Class:     net_cp_jlibical_ICalTriggerType
 * Method:    initFIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalTriggerType_initFIDs(JNIEnv *env, jclass clazz) {
	initICalTriggerTypeFieldIDs(env, clazz);
}
