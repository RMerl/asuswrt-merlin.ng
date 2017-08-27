/* -*- Mode: C -*- */
/*======================================================================
 FILE: jniICalTimeType_cxx.cpp
 CREATOR: structConverter
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

#include <jni.h>

#ifndef _jni_ICalTimeType_H
#include "jniICalTimeType_cxx.h"
#endif

static jfieldID ICalTimeType_Year_FID;
static jfieldID ICalTimeType_Month_FID;
static jfieldID ICalTimeType_Day_FID;
static jfieldID ICalTimeType_Hour_FID;
static jfieldID ICalTimeType_Minute_FID;
static jfieldID ICalTimeType_Second_FID;
static jfieldID ICalTimeType_Is_utc_FID;
static jfieldID ICalTimeType_Is_date_FID;
static jfieldID ICalTimeType_Zone_FID;


void initICalTimeTypeFieldIDs(JNIEnv* env, jclass clazz)
{
	ICalTimeType_Year_FID = env->GetFieldID(clazz, "year", "I");
	ICalTimeType_Month_FID = env->GetFieldID(clazz, "month", "I");
	ICalTimeType_Day_FID = env->GetFieldID(clazz, "day", "I");
	ICalTimeType_Hour_FID = env->GetFieldID(clazz, "hour", "I");
	ICalTimeType_Minute_FID = env->GetFieldID(clazz, "minute", "I");
	ICalTimeType_Second_FID = env->GetFieldID(clazz, "second", "I");
	ICalTimeType_Is_utc_FID = env->GetFieldID(clazz, "is_utc", "I");
	ICalTimeType_Is_date_FID = env->GetFieldID(clazz, "is_date", "I");
	ICalTimeType_Zone_FID = env->GetFieldID(clazz, "zone", "Ljava/lang/String;");
}

void  jni_SetYear_in_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType)
{
	env->SetIntField(thisICalTimeType, ICalTimeType_Year_FID, (jint) __ICalTimeType_->year);
}

void jni_GetYear_from_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType)
{
	__ICalTimeType_->year = env->GetIntField(thisICalTimeType, ICalTimeType_Year_FID);
}

void  jni_SetMonth_in_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType)
{
	env->SetIntField(thisICalTimeType, ICalTimeType_Month_FID, (jint) __ICalTimeType_->month);
}

void jni_GetMonth_from_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType)
{
	__ICalTimeType_->month = env->GetIntField(thisICalTimeType, ICalTimeType_Month_FID);
}

void  jni_SetDay_in_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType)
{
	env->SetIntField(thisICalTimeType, ICalTimeType_Day_FID, (jint) __ICalTimeType_->day);
}

void jni_GetDay_from_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType)
{
	__ICalTimeType_->day = env->GetIntField(thisICalTimeType, ICalTimeType_Day_FID);
}

void  jni_SetHour_in_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType)
{
	env->SetIntField(thisICalTimeType, ICalTimeType_Hour_FID, (jint) __ICalTimeType_->hour);
}

void jni_GetHour_from_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType)
{
	__ICalTimeType_->hour = env->GetIntField(thisICalTimeType, ICalTimeType_Hour_FID);
}

void  jni_SetMinute_in_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType)
{
	env->SetIntField(thisICalTimeType, ICalTimeType_Minute_FID, (jint) __ICalTimeType_->minute);
}

void jni_GetMinute_from_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType)
{
	__ICalTimeType_->minute = env->GetIntField(thisICalTimeType, ICalTimeType_Minute_FID);
}

void  jni_SetSecond_in_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType)
{
	env->SetIntField(thisICalTimeType, ICalTimeType_Second_FID, (jint) __ICalTimeType_->second);
}

void jni_GetSecond_from_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType)
{
	__ICalTimeType_->second = env->GetIntField(thisICalTimeType, ICalTimeType_Second_FID);
}

void  jni_SetIs_utc_in_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType)
{
	env->SetIntField(thisICalTimeType, ICalTimeType_Is_utc_FID, (jint) __ICalTimeType_->is_utc);
}

void jni_GetIs_utc_from_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType)
{
	__ICalTimeType_->is_utc = env->GetIntField(thisICalTimeType, ICalTimeType_Is_utc_FID);
}

void  jni_SetIs_date_in_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType)
{
	env->SetIntField(thisICalTimeType, ICalTimeType_Is_date_FID, (jint) __ICalTimeType_->is_date);
}

void jni_GetIs_date_from_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType)
{
	__ICalTimeType_->is_date = env->GetIntField(thisICalTimeType, ICalTimeType_Is_date_FID);
}

void  jni_SetZone_in_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType)
{
	env->SetObjectField(thisICalTimeType, ICalTimeType_Zone_FID, env->NewStringUTF(icaltime_get_tzid(*__ICalTimeType_)));
}

void jni_GetZone_from_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv *env, jobject thisICalTimeType)
{
	__ICalTimeType_->zone = icaltimezone_get_builtin_timezone_from_tzid((char*) env->GetStringUTFChars((jstring) env->GetObjectField(thisICalTimeType, ICalTimeType_Zone_FID), NULL));
}

// copy all fields from the c struct (__ICalTimeType_) to the java object (thisICalTimeType).
void jni_SetAll_in_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv* env, jobject thisICalTimeType)
{
	jni_SetYear_in_ICalTimeType(__ICalTimeType_, env, thisICalTimeType);
	jni_SetMonth_in_ICalTimeType(__ICalTimeType_, env, thisICalTimeType);
	jni_SetDay_in_ICalTimeType(__ICalTimeType_, env, thisICalTimeType);
	jni_SetHour_in_ICalTimeType(__ICalTimeType_, env, thisICalTimeType);
	jni_SetMinute_in_ICalTimeType(__ICalTimeType_, env, thisICalTimeType);
	jni_SetSecond_in_ICalTimeType(__ICalTimeType_, env, thisICalTimeType);
	jni_SetIs_utc_in_ICalTimeType(__ICalTimeType_, env, thisICalTimeType);
	jni_SetIs_date_in_ICalTimeType(__ICalTimeType_, env, thisICalTimeType);
	jni_SetZone_in_ICalTimeType(__ICalTimeType_, env, thisICalTimeType);
}

// copy all fields from the java object (thisICalTimeType) to the c struct (__ICalTimeType_).
void jni_GetAll_from_ICalTimeType(struct ICalTimeType* __ICalTimeType_, JNIEnv* env, jobject thisICalTimeType)
{
	jni_GetYear_from_ICalTimeType(__ICalTimeType_, env, thisICalTimeType);
	jni_GetMonth_from_ICalTimeType(__ICalTimeType_, env, thisICalTimeType);
	jni_GetDay_from_ICalTimeType(__ICalTimeType_, env, thisICalTimeType);
	jni_GetHour_from_ICalTimeType(__ICalTimeType_, env, thisICalTimeType);
	jni_GetMinute_from_ICalTimeType(__ICalTimeType_, env, thisICalTimeType);
	jni_GetSecond_from_ICalTimeType(__ICalTimeType_, env, thisICalTimeType);
	jni_GetIs_utc_from_ICalTimeType(__ICalTimeType_, env, thisICalTimeType);
	jni_GetIs_date_from_ICalTimeType(__ICalTimeType_, env, thisICalTimeType);
	jni_GetZone_from_ICalTimeType(__ICalTimeType_, env, thisICalTimeType);
}

/*
 * Class:     net_cp_jlibical_ICalTimeType
 * Method:    init
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalTimeType_init__J
  (JNIEnv* env, jobject thisICalTimeType, jlong data)
{
	// copy all fields from the c struct (data) to the java object (thisICalTimeType). 
	jni_SetAll_in_ICalTimeType((ICalTimeType*)data,env,thisICalTimeType);
}

/*
 * Class:     net_cp_jlibical_ICalTimeType
 * Method:    initFIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalTimeType_initFIDs(JNIEnv *env, jclass clazz) 
{
	initICalTimeTypeFieldIDs(env, clazz);
}
