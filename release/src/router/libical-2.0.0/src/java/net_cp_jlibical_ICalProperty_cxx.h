
/*======================================================================
 FILE: net_cp_jlibical_ICalProperty_cxx.h
 CREATOR: javah 1/11/02
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

#ifndef _Included_net_cp_jlibical_ICalProperty
#define _Included_net_cp_jlibical_ICalProperty

#ifndef JNI_H
#include <jni.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    as_ical_string
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_as_1ical_1string
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    isa
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalProperty_isa
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    isa_property
 * Signature: (Ljava/lang/Object;)I
 */
JNIEXPORT jboolean JNICALL Java_net_cp_jlibical_ICalProperty_isa_1property
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    add_parameter
 * Signature: (Lnet/cp/jlibical/ICalParameter;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_add_1parameter
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_parameter
 * Signature: (Lnet/cp/jlibical/ICalParameter;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1parameter
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_parameter_from_string
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1parameter_1from_1string
  (JNIEnv *, jobject, jstring, jstring);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_parameter_as_string
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1parameter_1as_1string
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    remove_parameter
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_remove_1parameter
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    count_parameters
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalProperty_count_1parameters
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_first_parameter
 * Signature: (I)Lnet/cp/jlibical/ICalParameter;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1first_1parameter
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_next_parameter
 * Signature: (I)Lnet/cp/jlibical/ICalParameter;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1next_1parameter
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_value
 * Signature: (Lnet/cp/jlibical/ICalValue;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1value
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_value_from_string
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1value_1from_1string
  (JNIEnv *, jobject, jstring, jstring);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_value
 * Signature: ()Lnet/cp/jlibical/ICalValue;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1value
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_value_as_string
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1value_1as_1string
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_name
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1name
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    value_to_value_kind
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalProperty_value_1to_1value_1kind
  (JNIEnv *, jclass, jint);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_action
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1action
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_action
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalProperty_get_1action
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_attendee
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1attendee
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_attendee
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1attendee
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_comment
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1comment
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_comment
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1comment
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_description
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1description
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_description
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1description
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_dtend
 * Signature: (Lnet/cp/jlibical/ICalTimeType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1dtend
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_dtend
 * Signature: ()Lnet/cp/jlibical/ICalTimeType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1dtend
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_dtstamp
 * Signature: (Lnet/cp/jlibical/ICalTimeType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1dtstamp
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_dtstamp
 * Signature: ()Lnet/cp/jlibical/ICalTimeType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1dtstamp
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_dtstart
 * Signature: (Lnet/cp/jlibical/ICalTimeType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1dtstart
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_dtstart
 * Signature: ()Lnet/cp/jlibical/ICalTimeType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1dtstart
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_due
 * Signature: (Lnet/cp/jlibical/ICalTimeType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1due
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_due
 * Signature: ()Lnet/cp/jlibical/ICalTimeType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1due
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_duration
 * Signature: (Lnet/cp/jlibical/ICalDurationType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1duration
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_duration
 * Signature: ()Lnet/cp/jlibical/ICalDurationType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1duration
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_location
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1location
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_location
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1location
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_method
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1method
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_method
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalProperty_get_1method
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_organizer
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1organizer
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_organizer
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1organizer
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_owner
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1owner
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_owner
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1owner
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_prodid
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1prodid
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_prodid
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1prodid
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_query
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1query
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_query
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1query
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_queryname
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1queryname
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_queryname
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1queryname
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_repeat
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1repeat
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_repeat
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalProperty_get_1repeat
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_summary
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1summary
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_summary
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1summary
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_target
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1target
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_target
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1target
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_trigger
 * Signature: (Lnet/cp/jlibical/ICalTriggerType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1trigger
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_trigger
 * Signature: ()Lnet/cp/jlibical/ICalTriggerType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1trigger
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_tzid
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1tzid
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_tzid
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1tzid
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_uid
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1uid
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_uid
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1uid
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_init__
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    init
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_init__Ljava_lang_String_2
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    init
 * Signature: (ILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_init__ILjava_lang_String_2
  (JNIEnv *, jobject, jint, jstring);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    init
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_init__I
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_status
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1status
  (JNIEnv *, jobject, jint);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_status
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalProperty_get_1status
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_relcalid
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1relcalid
  (JNIEnv *, jobject, jstring);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_relcalid
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1relcalid
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_exdate
 * Signature: (Lnet/cp/jlibical/ICalTimeType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1exdate
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_exdate
 * Signature: ()Lnet/cp/jlibical/ICalTimeType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1exdate
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_exrule
 * Signature: (Lnet/cp/jlibical/ICalRecurrenceType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1exrule
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_exrule
 * Signature: ()Lnet/cp/jlibical/ICalRecurrenceType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1exrule
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_freebusy
 * Signature: (Lnet/cp/jlibical/ICalPeriodType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1freebusy
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_freebusy
 * Signature: ()Lnet/cp/jlibical/ICalPeriodType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1freebusy
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_recurrenceid
 * Signature: (Lnet/cp/jlibical/ICalTimeType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1recurrenceid
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_recurrenceid
 * Signature: ()Lnet/cp/jlibical/ICalTimeType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1recurrenceid
  (JNIEnv *, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_rrule
 * Signature: (Lnet/cp/jlibical/ICalRecurrenceType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1rrule
  (JNIEnv *, jobject, jobject);

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_rrule
 * Signature: ()Lnet/cp/jlibical/ICalRecurrenceType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1rrule
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
