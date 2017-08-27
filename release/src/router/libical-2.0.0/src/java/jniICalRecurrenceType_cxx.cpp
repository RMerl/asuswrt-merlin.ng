/* -*- Mode: C -*- */
/*======================================================================
 FILE: jniICalRecurrenceType_cxx.cpp
 CREATOR: structConverter
======================================================================*/

#include <jni.h>

#include "jniICalRecurrenceType_cxx.h"
#include "jniICalTimeType_cxx.h"

static jfieldID ICalRecurrenceType_Until_FID;
static jfieldID ICalRecurrenceType_Freq_FID;
static jfieldID ICalRecurrenceType_Week_start_FID;

static jfieldID ICalRecurrenceType_Count_FID;
static jfieldID ICalRecurrenceType_Interval_FID;
static jfieldID ICalRecurrenceType_By_second_FID;
static jfieldID ICalRecurrenceType_By_minute_FID;
static jfieldID ICalRecurrenceType_By_hour_FID;
static jfieldID ICalRecurrenceType_By_day_FID;
static jfieldID ICalRecurrenceType_By_month_day_FID;
static jfieldID ICalRecurrenceType_By_year_day_FID;
static jfieldID ICalRecurrenceType_By_week_no_FID;
static jfieldID ICalRecurrenceType_By_month_FID;
static jfieldID ICalRecurrenceType_By_set_pos_FID;


void initICalRecurrenceTypeFieldIDs(JNIEnv* env, jclass clazz)
{
	ICalRecurrenceType_Until_FID = env->GetFieldID(clazz, "until", "Lnet/cp/jlibical/ICalTimeType;");
	ICalRecurrenceType_Freq_FID = env->GetFieldID(clazz, "freq", "I");
	ICalRecurrenceType_Week_start_FID = env->GetFieldID(clazz, "week_start", "I");
	ICalRecurrenceType_Count_FID = env->GetFieldID(clazz, "count", "I");
	ICalRecurrenceType_Interval_FID = env->GetFieldID(clazz, "interval", "S");
	ICalRecurrenceType_By_second_FID = env->GetFieldID(clazz, "by_second", "[S");
	ICalRecurrenceType_By_minute_FID = env->GetFieldID(clazz, "by_minute", "[S");
	ICalRecurrenceType_By_hour_FID = env->GetFieldID(clazz, "by_hour", "[S");
	ICalRecurrenceType_By_day_FID = env->GetFieldID(clazz, "by_day", "[S");
	ICalRecurrenceType_By_month_day_FID = env->GetFieldID(clazz, "by_month_day", "[S");
	ICalRecurrenceType_By_year_day_FID = env->GetFieldID(clazz, "by_year_day", "[S");
	ICalRecurrenceType_By_week_no_FID = env->GetFieldID(clazz, "by_week_no", "[S");
	ICalRecurrenceType_By_month_FID = env->GetFieldID(clazz, "by_month", "[S");
	ICalRecurrenceType_By_set_pos_FID = env->GetFieldID(clazz, "by_set_pos", "[S");
}

void  jni_SetUntil_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	jobject	lcl_jobj0;
	lcl_jobj0 = env->GetObjectField(thisICalRecurrenceType, ICalRecurrenceType_Until_FID);
	jni_SetYear_in_ICalTimeType(&(__ICalRecurrenceType_->until), env, lcl_jobj0);
	jni_SetMonth_in_ICalTimeType(&(__ICalRecurrenceType_->until), env, lcl_jobj0);
	jni_SetDay_in_ICalTimeType(&(__ICalRecurrenceType_->until), env, lcl_jobj0);
	jni_SetHour_in_ICalTimeType(&(__ICalRecurrenceType_->until), env, lcl_jobj0);
	jni_SetMinute_in_ICalTimeType(&(__ICalRecurrenceType_->until), env, lcl_jobj0);
	jni_SetSecond_in_ICalTimeType(&(__ICalRecurrenceType_->until), env, lcl_jobj0);
	jni_SetIs_utc_in_ICalTimeType(&(__ICalRecurrenceType_->until), env, lcl_jobj0);
	jni_SetIs_date_in_ICalTimeType(&(__ICalRecurrenceType_->until), env, lcl_jobj0);
	jni_SetZone_in_ICalTimeType(&(__ICalRecurrenceType_->until), env, lcl_jobj0);
}

void jni_GetUntil_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	jobject	lcl_jobj0;
	lcl_jobj0 = env->GetObjectField(thisICalRecurrenceType, ICalRecurrenceType_Until_FID);
	jni_GetYear_from_ICalTimeType(&(__ICalRecurrenceType_->until), env, lcl_jobj0);
	jni_GetMonth_from_ICalTimeType(&(__ICalRecurrenceType_->until), env, lcl_jobj0);
	jni_GetDay_from_ICalTimeType(&(__ICalRecurrenceType_->until), env, lcl_jobj0);
	jni_GetHour_from_ICalTimeType(&(__ICalRecurrenceType_->until), env, lcl_jobj0);
	jni_GetMinute_from_ICalTimeType(&(__ICalRecurrenceType_->until), env, lcl_jobj0);
	jni_GetSecond_from_ICalTimeType(&(__ICalRecurrenceType_->until), env, lcl_jobj0);
	jni_GetIs_utc_from_ICalTimeType(&(__ICalRecurrenceType_->until), env, lcl_jobj0);
	jni_GetIs_date_from_ICalTimeType(&(__ICalRecurrenceType_->until), env, lcl_jobj0);
	jni_GetZone_from_ICalTimeType(&(__ICalRecurrenceType_->until), env, lcl_jobj0);
}

void  jni_SetFreq_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	env->SetIntField(thisICalRecurrenceType, ICalRecurrenceType_Freq_FID, (jint) __ICalRecurrenceType_->freq);
}

void jni_GetFreq_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	__ICalRecurrenceType_->freq = (icalrecurrencetype_frequency) env->GetIntField(thisICalRecurrenceType, ICalRecurrenceType_Freq_FID);
}

void  jni_SetWeek_start_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	env->SetIntField(thisICalRecurrenceType, ICalRecurrenceType_Week_start_FID, (jint) __ICalRecurrenceType_->week_start);
}

void jni_GetWeek_start_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	__ICalRecurrenceType_->week_start = (icalrecurrencetype_weekday) env->GetIntField(thisICalRecurrenceType, ICalRecurrenceType_Week_start_FID);
}

void  jni_SetCount_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	env->SetIntField(thisICalRecurrenceType, ICalRecurrenceType_Count_FID, (jint) __ICalRecurrenceType_->count);
}

void jni_GetCount_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	__ICalRecurrenceType_->count = env->GetIntField(thisICalRecurrenceType, ICalRecurrenceType_Count_FID);
}

void  jni_SetInterval_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	env->SetShortField(thisICalRecurrenceType, ICalRecurrenceType_Interval_FID, (jshort) __ICalRecurrenceType_->interval);
}

void jni_GetInterval_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	__ICalRecurrenceType_->interval = env->GetShortField(thisICalRecurrenceType, ICalRecurrenceType_Interval_FID);
}

void  jni_SetBy_second_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	jclass	elem_clazz;
	int ix0;
	jobject	lcl_jobj0;
	jarray lcl_jobjA0;
	lcl_jobjA0 = (jarray) env->GetObjectField(thisICalRecurrenceType, ICalRecurrenceType_By_second_FID);
	env->SetShortArrayRegion((jshortArray)lcl_jobjA0, 0,61, (jshort*)&(__ICalRecurrenceType_->by_second[0]));
}

void jni_GetBy_second_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	jclass	elem_clazz;
	jboolean isCopy;
	int ix0;
	jobject	lcl_jobj0;
	jarray lcl_jobjA0;
	lcl_jobjA0 = (jarray) env->GetObjectField(thisICalRecurrenceType, ICalRecurrenceType_By_second_FID);
	env->GetShortArrayRegion((jshortArray)lcl_jobjA0, 0,61, (jshort*)&(__ICalRecurrenceType_->by_second[0]));
}

void  jni_SetBy_minute_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	jclass	elem_clazz;
	int ix0;
	jobject	lcl_jobj0;
	jarray lcl_jobjA0;
	lcl_jobjA0 = (jarray) env->GetObjectField(thisICalRecurrenceType, ICalRecurrenceType_By_minute_FID);
	env->SetShortArrayRegion((jshortArray)lcl_jobjA0, 0,61, (jshort*)&(__ICalRecurrenceType_->by_minute[0]));
}

void jni_GetBy_minute_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	jclass	elem_clazz;
	jboolean isCopy;
	int ix0;
	jobject	lcl_jobj0;
	jarray lcl_jobjA0;
	lcl_jobjA0 = (jarray) env->GetObjectField(thisICalRecurrenceType, ICalRecurrenceType_By_minute_FID);
	env->GetShortArrayRegion((jshortArray)lcl_jobjA0, 0,61, (jshort*)&(__ICalRecurrenceType_->by_minute[0]));
}

void  jni_SetBy_hour_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	jclass	elem_clazz;
	int ix0;
	jobject	lcl_jobj0;
	jarray lcl_jobjA0;
	lcl_jobjA0 = (jarray) env->GetObjectField(thisICalRecurrenceType, ICalRecurrenceType_By_hour_FID);
	env->SetShortArrayRegion((jshortArray)lcl_jobjA0, 0,25, (jshort*)&(__ICalRecurrenceType_->by_hour[0]));
}

void jni_GetBy_hour_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	jclass	elem_clazz;
	jboolean isCopy;
	int ix0;
	jobject	lcl_jobj0;
	jarray lcl_jobjA0;
	lcl_jobjA0 = (jarray) env->GetObjectField(thisICalRecurrenceType, ICalRecurrenceType_By_hour_FID);
	env->GetShortArrayRegion((jshortArray)lcl_jobjA0, 0,25, (jshort*)&(__ICalRecurrenceType_->by_hour[0]));
}

void  jni_SetBy_day_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	jclass	elem_clazz;
	int ix0;
	jobject	lcl_jobj0;
	jarray lcl_jobjA0;
	lcl_jobjA0 = (jarray) env->GetObjectField(thisICalRecurrenceType, ICalRecurrenceType_By_day_FID);
	env->SetShortArrayRegion((jshortArray)lcl_jobjA0, 0,364, (jshort*)&(__ICalRecurrenceType_->by_day[0]));
}

void jni_GetBy_day_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	jclass	elem_clazz;
	jboolean isCopy;
	int ix0;
	jobject	lcl_jobj0;
	jarray lcl_jobjA0;
	lcl_jobjA0 = (jarray) env->GetObjectField(thisICalRecurrenceType, ICalRecurrenceType_By_day_FID);
	env->GetShortArrayRegion((jshortArray)lcl_jobjA0, 0,364, (jshort*)&(__ICalRecurrenceType_->by_day[0]));
}

void  jni_SetBy_month_day_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	jclass	elem_clazz;
	int ix0;
	jobject	lcl_jobj0;
	jarray lcl_jobjA0;
	lcl_jobjA0 = (jarray) env->GetObjectField(thisICalRecurrenceType, ICalRecurrenceType_By_month_day_FID);
	env->SetShortArrayRegion((jshortArray)lcl_jobjA0, 0,32, (jshort*)&(__ICalRecurrenceType_->by_month_day[0]));
}

void jni_GetBy_month_day_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	jclass	elem_clazz;
	jboolean isCopy;
	int ix0;
	jobject	lcl_jobj0;
	jarray lcl_jobjA0;
	lcl_jobjA0 = (jarray) env->GetObjectField(thisICalRecurrenceType, ICalRecurrenceType_By_month_day_FID);
	env->GetShortArrayRegion((jshortArray)lcl_jobjA0, 0,32, (jshort*)&(__ICalRecurrenceType_->by_month_day[0]));
}

void  jni_SetBy_year_day_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	jclass	elem_clazz;
	int ix0;
	jobject	lcl_jobj0;
	jarray lcl_jobjA0;
	lcl_jobjA0 = (jarray) env->GetObjectField(thisICalRecurrenceType, ICalRecurrenceType_By_year_day_FID);
	env->SetShortArrayRegion((jshortArray)lcl_jobjA0, 0,367, (jshort*)&(__ICalRecurrenceType_->by_year_day[0]));
}

void jni_GetBy_year_day_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	jclass	elem_clazz;
	jboolean isCopy;
	int ix0;
	jobject	lcl_jobj0;
	jarray lcl_jobjA0;
	lcl_jobjA0 = (jarray) env->GetObjectField(thisICalRecurrenceType, ICalRecurrenceType_By_year_day_FID);
	env->GetShortArrayRegion((jshortArray)lcl_jobjA0, 0,367, (jshort*)&(__ICalRecurrenceType_->by_year_day[0]));
}

void  jni_SetBy_week_no_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	jclass	elem_clazz;
	int ix0;
	jobject	lcl_jobj0;
	jarray lcl_jobjA0;
	lcl_jobjA0 = (jarray) env->GetObjectField(thisICalRecurrenceType, ICalRecurrenceType_By_week_no_FID);
	env->SetShortArrayRegion((jshortArray)lcl_jobjA0, 0,54, (jshort*)&(__ICalRecurrenceType_->by_week_no[0]));
}

void jni_GetBy_week_no_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	jclass	elem_clazz;
	jboolean isCopy;
	int ix0;
	jobject	lcl_jobj0;
	jarray lcl_jobjA0;
	lcl_jobjA0 = (jarray) env->GetObjectField(thisICalRecurrenceType, ICalRecurrenceType_By_week_no_FID);
	env->GetShortArrayRegion((jshortArray)lcl_jobjA0, 0,54, (jshort*)&(__ICalRecurrenceType_->by_week_no[0]));
}

void  jni_SetBy_month_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	jclass	elem_clazz;
	int ix0;
	jobject	lcl_jobj0;
	jarray lcl_jobjA0;
	lcl_jobjA0 = (jarray) env->GetObjectField(thisICalRecurrenceType, ICalRecurrenceType_By_month_FID);
	env->SetShortArrayRegion((jshortArray)lcl_jobjA0, 0,13, (jshort*)&(__ICalRecurrenceType_->by_month[0]));
}

void jni_GetBy_month_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	jclass	elem_clazz;
	jboolean isCopy;
	int ix0;
	jobject	lcl_jobj0;
	jarray lcl_jobjA0;
	lcl_jobjA0 = (jarray) env->GetObjectField(thisICalRecurrenceType, ICalRecurrenceType_By_month_FID);
	env->GetShortArrayRegion((jshortArray)lcl_jobjA0, 0,13, (jshort*)&(__ICalRecurrenceType_->by_month[0]));
}

void  jni_SetBy_set_pos_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	jclass	elem_clazz;
	int ix0;
	jobject	lcl_jobj0;
	jarray lcl_jobjA0;
	lcl_jobjA0 = (jarray) env->GetObjectField(thisICalRecurrenceType, ICalRecurrenceType_By_set_pos_FID);
	env->SetShortArrayRegion((jshortArray)lcl_jobjA0, 0,367, (jshort*)&(__ICalRecurrenceType_->by_set_pos[0]));
}

void jni_GetBy_set_pos_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv *env, jobject thisICalRecurrenceType)
{
	jclass	elem_clazz;
	jboolean isCopy;
	int ix0;
	jobject	lcl_jobj0;
	jarray lcl_jobjA0;
	lcl_jobjA0 = (jarray) env->GetObjectField(thisICalRecurrenceType, ICalRecurrenceType_By_set_pos_FID);
	env->GetShortArrayRegion((jshortArray)lcl_jobjA0, 0,367, (jshort*)&(__ICalRecurrenceType_->by_set_pos[0]));
}
void jni_SetAll_in_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv* env, jobject thisICalRecurrenceType)
{
	jni_SetUntil_in_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_SetFreq_in_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_SetWeek_start_in_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_SetCount_in_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_SetInterval_in_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_SetBy_second_in_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_SetBy_minute_in_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_SetBy_hour_in_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_SetBy_day_in_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_SetBy_month_day_in_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_SetBy_year_day_in_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_SetBy_week_no_in_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_SetBy_month_in_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_SetBy_set_pos_in_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
}
void jni_GetAll_from_ICalRecurrenceType(struct ICalRecurrenceType* __ICalRecurrenceType_, JNIEnv* env, jobject thisICalRecurrenceType)
{
	jni_GetUntil_from_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_GetFreq_from_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_GetWeek_start_from_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_GetCount_from_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_GetInterval_from_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_GetBy_second_from_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_GetBy_minute_from_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_GetBy_hour_from_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_GetBy_day_from_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_GetBy_month_day_from_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_GetBy_year_day_from_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_GetBy_week_no_from_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_GetBy_month_from_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	jni_GetBy_set_pos_from_ICalRecurrenceType(__ICalRecurrenceType_, env, thisICalRecurrenceType);
	
}

JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalRecurrenceType_init__J(JNIEnv* env, jobject thisICalRecurrenceType, jlong data) {
	jni_SetAll_in_ICalRecurrenceType((ICalRecurrenceType*)data,env,thisICalRecurrenceType);
}

JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalRecurrenceType_initFIDs(JNIEnv *env, jclass clazz) {
	initICalRecurrenceTypeFieldIDs(env, clazz);
}
