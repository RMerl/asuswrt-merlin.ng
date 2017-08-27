/* -*- Mode: C -*- */
/*======================================================================
 FILE: jniICalPeriodType_cxx.cpp
 CREATOR: structConverter
======================================================================*/

#include <jni.h>

#include "jniICalPeriodType_cxx.h"
#include "jniICalTimeType_cxx.h"
#include "jniICalDurationType_cxx.h"

static jfieldID ICalPeriodType_Start_FID;
static jfieldID ICalPeriodType_End_FID;
static jfieldID ICalPeriodType_Duration_FID;


void initICalPeriodTypeFieldIDs(JNIEnv* env, jclass clazz)
{
	ICalPeriodType_Start_FID = env->GetFieldID(clazz, "start", "Lnet/cp/jlibical/ICalTimeType;");
	ICalPeriodType_End_FID = env->GetFieldID(clazz, "end", "Lnet/cp/jlibical/ICalTimeType;");
	ICalPeriodType_Duration_FID = env->GetFieldID(clazz, "duration", "Lnet/cp/jlibical/ICalDurationType;");
}

void  jni_SetStart_in_ICalPeriodType(struct ICalPeriodType* __ICalPeriodType_, JNIEnv *env, jobject thisICalPeriodType)
{
	jobject	lcl_jobj0;
	lcl_jobj0 = env->GetObjectField(thisICalPeriodType, ICalPeriodType_Start_FID);
	jni_SetYear_in_ICalTimeType(&(__ICalPeriodType_->start), env, lcl_jobj0);
	jni_SetMonth_in_ICalTimeType(&(__ICalPeriodType_->start), env, lcl_jobj0);
	jni_SetDay_in_ICalTimeType(&(__ICalPeriodType_->start), env, lcl_jobj0);
	jni_SetHour_in_ICalTimeType(&(__ICalPeriodType_->start), env, lcl_jobj0);
	jni_SetMinute_in_ICalTimeType(&(__ICalPeriodType_->start), env, lcl_jobj0);
	jni_SetSecond_in_ICalTimeType(&(__ICalPeriodType_->start), env, lcl_jobj0);
	jni_SetIs_utc_in_ICalTimeType(&(__ICalPeriodType_->start), env, lcl_jobj0);
	jni_SetIs_date_in_ICalTimeType(&(__ICalPeriodType_->start), env, lcl_jobj0);
	jni_SetZone_in_ICalTimeType(&(__ICalPeriodType_->start), env, lcl_jobj0);
}

void jni_GetStart_from_ICalPeriodType(struct ICalPeriodType* __ICalPeriodType_, JNIEnv *env, jobject thisICalPeriodType)
{
	jobject	lcl_jobj0;
	lcl_jobj0 = env->GetObjectField(thisICalPeriodType, ICalPeriodType_Start_FID);
	jni_GetYear_from_ICalTimeType(&(__ICalPeriodType_->start), env, lcl_jobj0);
	jni_GetMonth_from_ICalTimeType(&(__ICalPeriodType_->start), env, lcl_jobj0);
	jni_GetDay_from_ICalTimeType(&(__ICalPeriodType_->start), env, lcl_jobj0);
	jni_GetHour_from_ICalTimeType(&(__ICalPeriodType_->start), env, lcl_jobj0);
	jni_GetMinute_from_ICalTimeType(&(__ICalPeriodType_->start), env, lcl_jobj0);
	jni_GetSecond_from_ICalTimeType(&(__ICalPeriodType_->start), env, lcl_jobj0);
	jni_GetIs_utc_from_ICalTimeType(&(__ICalPeriodType_->start), env, lcl_jobj0);
	jni_GetIs_date_from_ICalTimeType(&(__ICalPeriodType_->start), env, lcl_jobj0);
	jni_GetZone_from_ICalTimeType(&(__ICalPeriodType_->start), env, lcl_jobj0);
}

void  jni_SetEnd_in_ICalPeriodType(struct ICalPeriodType* __ICalPeriodType_, JNIEnv *env, jobject thisICalPeriodType)
{
	jobject	lcl_jobj0;
	lcl_jobj0 = env->GetObjectField(thisICalPeriodType, ICalPeriodType_End_FID);
	jni_SetYear_in_ICalTimeType(&(__ICalPeriodType_->end), env, lcl_jobj0);
	jni_SetMonth_in_ICalTimeType(&(__ICalPeriodType_->end), env, lcl_jobj0);
	jni_SetDay_in_ICalTimeType(&(__ICalPeriodType_->end), env, lcl_jobj0);
	jni_SetHour_in_ICalTimeType(&(__ICalPeriodType_->end), env, lcl_jobj0);
	jni_SetMinute_in_ICalTimeType(&(__ICalPeriodType_->end), env, lcl_jobj0);
	jni_SetSecond_in_ICalTimeType(&(__ICalPeriodType_->end), env, lcl_jobj0);
	jni_SetIs_utc_in_ICalTimeType(&(__ICalPeriodType_->end), env, lcl_jobj0);
	jni_SetIs_date_in_ICalTimeType(&(__ICalPeriodType_->end), env, lcl_jobj0);
	jni_SetZone_in_ICalTimeType(&(__ICalPeriodType_->end), env, lcl_jobj0);
}

void jni_GetEnd_from_ICalPeriodType(struct ICalPeriodType* __ICalPeriodType_, JNIEnv *env, jobject thisICalPeriodType)
{
	jobject	lcl_jobj0;
	lcl_jobj0 = env->GetObjectField(thisICalPeriodType, ICalPeriodType_End_FID);
	jni_GetYear_from_ICalTimeType(&(__ICalPeriodType_->end), env, lcl_jobj0);
	jni_GetMonth_from_ICalTimeType(&(__ICalPeriodType_->end), env, lcl_jobj0);
	jni_GetDay_from_ICalTimeType(&(__ICalPeriodType_->end), env, lcl_jobj0);
	jni_GetHour_from_ICalTimeType(&(__ICalPeriodType_->end), env, lcl_jobj0);
	jni_GetMinute_from_ICalTimeType(&(__ICalPeriodType_->end), env, lcl_jobj0);
	jni_GetSecond_from_ICalTimeType(&(__ICalPeriodType_->end), env, lcl_jobj0);
	jni_GetIs_utc_from_ICalTimeType(&(__ICalPeriodType_->end), env, lcl_jobj0);
	jni_GetIs_date_from_ICalTimeType(&(__ICalPeriodType_->end), env, lcl_jobj0);
	jni_GetZone_from_ICalTimeType(&(__ICalPeriodType_->end), env, lcl_jobj0);
}

void  jni_SetDuration_in_ICalPeriodType(struct ICalPeriodType* __ICalPeriodType_, JNIEnv *env, jobject thisICalPeriodType)
{
	jobject	lcl_jobj0;
	lcl_jobj0 = env->GetObjectField(thisICalPeriodType, ICalPeriodType_Duration_FID);
	jni_SetIs_neg_in_ICalDurationType(&(__ICalPeriodType_->duration), env, lcl_jobj0);
	jni_SetDays_in_ICalDurationType(&(__ICalPeriodType_->duration), env, lcl_jobj0);
	jni_SetWeeks_in_ICalDurationType(&(__ICalPeriodType_->duration), env, lcl_jobj0);
	jni_SetHours_in_ICalDurationType(&(__ICalPeriodType_->duration), env, lcl_jobj0);
	jni_SetMinutes_in_ICalDurationType(&(__ICalPeriodType_->duration), env, lcl_jobj0);
	jni_SetSeconds_in_ICalDurationType(&(__ICalPeriodType_->duration), env, lcl_jobj0);
}

void jni_GetDuration_from_ICalPeriodType(struct ICalPeriodType* __ICalPeriodType_, JNIEnv *env, jobject thisICalPeriodType)
{
	jobject	lcl_jobj0;
	lcl_jobj0 = env->GetObjectField(thisICalPeriodType, ICalPeriodType_Duration_FID);
	jni_GetIs_neg_from_ICalDurationType(&(__ICalPeriodType_->duration), env, lcl_jobj0);
	jni_GetDays_from_ICalDurationType(&(__ICalPeriodType_->duration), env, lcl_jobj0);
	jni_GetWeeks_from_ICalDurationType(&(__ICalPeriodType_->duration), env, lcl_jobj0);
	jni_GetHours_from_ICalDurationType(&(__ICalPeriodType_->duration), env, lcl_jobj0);
	jni_GetMinutes_from_ICalDurationType(&(__ICalPeriodType_->duration), env, lcl_jobj0);
	jni_GetSeconds_from_ICalDurationType(&(__ICalPeriodType_->duration), env, lcl_jobj0);
}

// copy all fields from the c struct (__ICalPeriodType_) to the java object (thisICalPeriodType).
void jni_SetAll_in_ICalPeriodType(struct ICalPeriodType* __ICalPeriodType_, JNIEnv* env, jobject thisICalPeriodType)
{
	jni_SetStart_in_ICalPeriodType(__ICalPeriodType_, env, thisICalPeriodType);
	jni_SetEnd_in_ICalPeriodType(__ICalPeriodType_, env, thisICalPeriodType);
	jni_SetDuration_in_ICalPeriodType(__ICalPeriodType_, env, thisICalPeriodType);
}

// copy all fields from the java object (thisICalPeriodType) to the c struct (__ICalPeriodType_).
void jni_GetAll_from_ICalPeriodType(struct ICalPeriodType* __ICalPeriodType_, JNIEnv* env, jobject thisICalPeriodType)
{
	jni_GetStart_from_ICalPeriodType(__ICalPeriodType_, env, thisICalPeriodType);
	jni_GetEnd_from_ICalPeriodType(__ICalPeriodType_, env, thisICalPeriodType);
	jni_GetDuration_from_ICalPeriodType(__ICalPeriodType_, env, thisICalPeriodType);
}

/*
 * Class:     net_cp_jlibical_ICalPeriodType
 * Method:    init
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalPeriodType_init__J
  (JNIEnv* env, jobject thisICalPeriodType, jlong data)
{
	// copy all fields from the c struct (data) to the java object (thisICalTimeType).
	jni_SetAll_in_ICalPeriodType((ICalPeriodType*)data,env,thisICalPeriodType);
}

/*
 * Class:     net_cp_jlibical_ICalPeriodType
 * Method:    initFIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalPeriodType_initFIDs(JNIEnv *env, jclass clazz) {
	initICalPeriodTypeFieldIDs(env, clazz);
}
