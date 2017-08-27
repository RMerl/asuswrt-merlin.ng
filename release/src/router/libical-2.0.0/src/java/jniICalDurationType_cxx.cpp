/* -*- Mode: C -*- */
/*======================================================================
 FILE: jniICalDurationType_cxx.cpp
 CREATOR: structConverter
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

#include <jni.h>

#include "jniICalDurationType_cxx.h"

static jfieldID ICalDurationType_Is_neg_FID;
static jfieldID ICalDurationType_Days_FID;
static jfieldID ICalDurationType_Weeks_FID;
static jfieldID ICalDurationType_Hours_FID;
static jfieldID ICalDurationType_Minutes_FID;
static jfieldID ICalDurationType_Seconds_FID;


void initICalDurationTypeFieldIDs(JNIEnv* env, jclass clazz)
{
	ICalDurationType_Is_neg_FID = env->GetFieldID(clazz, "is_neg", "I");
	ICalDurationType_Days_FID = env->GetFieldID(clazz, "days", "J");
	ICalDurationType_Weeks_FID = env->GetFieldID(clazz, "weeks", "J");
	ICalDurationType_Hours_FID = env->GetFieldID(clazz, "hours", "J");
	ICalDurationType_Minutes_FID = env->GetFieldID(clazz, "minutes", "J");
	ICalDurationType_Seconds_FID = env->GetFieldID(clazz, "seconds", "J");
}

void  jni_SetIs_neg_in_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType)
{
	env->SetIntField(thisICalDurationType, ICalDurationType_Is_neg_FID, (jint) __ICalDurationType_->is_neg);
}

void jni_GetIs_neg_from_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType)
{
	__ICalDurationType_->is_neg = env->GetIntField(thisICalDurationType, ICalDurationType_Is_neg_FID);
}

void  jni_SetDays_in_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType)
{
	env->SetLongField(thisICalDurationType, ICalDurationType_Days_FID, (jlong) __ICalDurationType_->days);
}

void jni_GetDays_from_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType)
{
	__ICalDurationType_->days = env->GetLongField(thisICalDurationType, ICalDurationType_Days_FID);
}

void  jni_SetWeeks_in_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType)
{
	env->SetLongField(thisICalDurationType, ICalDurationType_Weeks_FID, (jlong) __ICalDurationType_->weeks);
}

void jni_GetWeeks_from_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType)
{
	__ICalDurationType_->weeks = env->GetLongField(thisICalDurationType, ICalDurationType_Weeks_FID);
}

void  jni_SetHours_in_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType)
{
	env->SetLongField(thisICalDurationType, ICalDurationType_Hours_FID, (jlong) __ICalDurationType_->hours);
}

void jni_GetHours_from_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType)
{
	__ICalDurationType_->hours = env->GetLongField(thisICalDurationType, ICalDurationType_Hours_FID);
}

void  jni_SetMinutes_in_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType)
{
	env->SetLongField(thisICalDurationType, ICalDurationType_Minutes_FID, (jlong) __ICalDurationType_->minutes);
}

void jni_GetMinutes_from_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType)
{
	__ICalDurationType_->minutes = env->GetLongField(thisICalDurationType, ICalDurationType_Minutes_FID);
}

void  jni_SetSeconds_in_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType)
{
	env->SetLongField(thisICalDurationType, ICalDurationType_Seconds_FID, (jlong) __ICalDurationType_->seconds);
}

void jni_GetSeconds_from_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv *env, jobject thisICalDurationType)
{
	__ICalDurationType_->seconds = env->GetLongField(thisICalDurationType, ICalDurationType_Seconds_FID);
}

// copy all fields from the c struct (ICalDurationType) to the java object (thisICalDurationType).
void jni_SetAll_in_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv* env, jobject thisICalDurationType)
{
	jni_SetIs_neg_in_ICalDurationType(__ICalDurationType_, env, thisICalDurationType);
	jni_SetDays_in_ICalDurationType(__ICalDurationType_, env, thisICalDurationType);
	jni_SetWeeks_in_ICalDurationType(__ICalDurationType_, env, thisICalDurationType);
	jni_SetHours_in_ICalDurationType(__ICalDurationType_, env, thisICalDurationType);
	jni_SetMinutes_in_ICalDurationType(__ICalDurationType_, env, thisICalDurationType);
	jni_SetSeconds_in_ICalDurationType(__ICalDurationType_, env, thisICalDurationType);
}

// copy all fields from the java object (thisICalDurationType) to the c struct (__ICalDurationType_).
void jni_GetAll_from_ICalDurationType(struct ICalDurationType* __ICalDurationType_, JNIEnv* env, jobject thisICalDurationType)
{
	jni_GetIs_neg_from_ICalDurationType(__ICalDurationType_, env, thisICalDurationType);
	jni_GetDays_from_ICalDurationType(__ICalDurationType_, env, thisICalDurationType);
	jni_GetWeeks_from_ICalDurationType(__ICalDurationType_, env, thisICalDurationType);
	jni_GetHours_from_ICalDurationType(__ICalDurationType_, env, thisICalDurationType);
	jni_GetMinutes_from_ICalDurationType(__ICalDurationType_, env, thisICalDurationType);
	jni_GetSeconds_from_ICalDurationType(__ICalDurationType_, env, thisICalDurationType);
}
/*
 * Class:     net_cp_jlibical_ICalDurationType
 * Method:    init
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalDurationType_init__J
  (JNIEnv *env, jobject thisICalDurationType, jlong data)
{
	// copy all fields from the c struct (data) to the java object (thisICalDurationType). 
	jni_SetAll_in_ICalDurationType((ICalDurationType*)data,env,thisICalDurationType);
}

/*
 * Class:     net_cp_jlibical_ICalDurationType
 * Method:    initFIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalDurationType_initFIDs(JNIEnv *env, jclass clazz) {
	initICalDurationTypeFieldIDs(env, clazz);
}
