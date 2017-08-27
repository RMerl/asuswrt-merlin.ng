/* -*- Mode: C -*- */
/*======================================================================
 FILE: net_cp_jlibical_ICalProperty_cxx.cpp
 CREATOR: gnorman 1/10/02
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

#ifndef NET_CP_JLIBICAL_ICALPROPERTY_CXX_H
#include "net_cp_jlibical_ICalProperty_cxx.h"
#endif

#ifndef JLIBICAL_CONSTS_CXX_H
#include "jlibical_consts_cxx.h"
#endif

#ifndef JLIBICAL_UTILS_CXX_H
#include "jlibical_utils_cxx.h"
#endif

#ifndef ICALPROPERTY_CXX_H
#include "icalproperty_cxx.h"
#endif

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    as_ical_string
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_as_1ical_1string
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		char* icalStr = cObj->as_ical_string();

		if (icalStr == NULL)
		{
			icalStr = "";
		}

		result = env->NewStringUTF(icalStr);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    isa
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalProperty_isa
  (JNIEnv *env, jobject jobj)
{
	jint result = 0;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		result = cObj->isa();
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    isa_property
 * Signature: (Ljava/lang/Object;)I
 */
JNIEXPORT jboolean JNICALL Java_net_cp_jlibical_ICalProperty_isa_1property
  (JNIEnv *env, jobject jobj, jobject arg)
{
	jboolean result = 0;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		void* argObjPtr = 0;

		if (arg != NULL)
		{
			argObjPtr = getCObjectPtr(env,arg);
		}

		// get the result from the c++ object (argObjPtr can be 0, it's cObj's responsibility to handle this if an error).
		result = cObj->isa_property(argObjPtr) != 0;
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    add_parameter
 * Signature: (Lnet/cp/jlibical/ICalParameter;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_add_1parameter
  (JNIEnv *env, jobject jobj, jobject arg)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		ICalParameter* icalparameter = getSubjectAsICalParameter(env,arg,JLIBICAL_ERR_ILLEGAL_ARGUMENT);

		if (icalparameter != NULL)
		{
			cObj->add_parameter(*icalparameter);
		}
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_parameter
 * Signature: (Lnet/cp/jlibical/ICalParameter;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1parameter
  (JNIEnv *env, jobject jobj, jobject arg)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		ICalParameter* icalparameter = getSubjectAsICalParameter(env,arg,JLIBICAL_ERR_ILLEGAL_ARGUMENT);

		if (icalparameter != NULL)
		{
			cObj->set_parameter(*icalparameter);
		}
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_parameter_from_string
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1parameter_1from_1string
  (JNIEnv *env, jobject jobj, jstring name, jstring value)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		const char* szName = env->GetStringUTFChars(name,NULL);
		const char* szValue = env->GetStringUTFChars(value,NULL);

		if (szName != NULL && szValue != NULL)
		{
			cObj->set_parameter_from_string((string)szName, (string)szValue);
		}
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_parameter_as_string
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1parameter_1as_1string
  (JNIEnv *env, jobject jobj, jstring name)
{
	jstring result = NULL;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		const char* szName = env->GetStringUTFChars(name,NULL);

		if (szName != NULL)
		{
			char* szValue = cObj->get_parameter_as_string((string)szName);

			if (szValue == NULL)
			{
				szValue = "";
			}

			result = env->NewStringUTF(szValue);
		}
	}

	return (result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    remove_parameter
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_remove_1parameter
  (JNIEnv *env, jobject jobj, jint kind)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		cObj->remove_parameter((icalparameter_kind)kind);
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    count_parameters
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalProperty_count_1parameters
  (JNIEnv *env, jobject jobj)
{
	jint result = 0;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		result = cObj->count_parameters();
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_first_parameter
 * Signature: (I)Lnet/cp/jlibical/ICalParameter;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1first_1parameter
  (JNIEnv *env, jobject jobj, jint kind)
{
	jobject result = 0;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the first parameter from CObj
		ICalParameter* aParameter = cObj->get_first_parameter((icalparameter_kind)kind);

		// create a new surrogate, using aParameter as the subject (returns NULL if subject is NULL).
		result = createNewICalParameterSurrogate(env,aParameter);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_next_parameter
 * Signature: (I)Lnet/cp/jlibical/ICalParameter;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1next_1parameter
  (JNIEnv *env, jobject jobj, jint kind)
{
	jobject result = 0;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the first parameter from CObj
		ICalParameter* aParameter = cObj->get_next_parameter((icalparameter_kind)kind);

		// create a new surrogate, using aParameter as the subject (returns NULL if subject is NULL).
		result = createNewICalParameterSurrogate(env,aParameter);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_value
 * Signature: (Lnet/cp/jlibical/ICalValue;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1value
  (JNIEnv *env, jobject jobj, jobject arg)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		ICalValue* aValue = getSubjectAsICalValue(env,arg,JLIBICAL_ERR_ILLEGAL_ARGUMENT);
		cObj->set_value(*aValue);
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_value_from_string
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1value_1from_1string
  (JNIEnv *env, jobject jobj, jstring name, jstring value)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		const char* szName = env->GetStringUTFChars(name,NULL);
		const char* szValue = env->GetStringUTFChars(value,NULL);

		if (szName != NULL && szValue != NULL)
		{
			cObj->set_value_from_string((string)szName, (string)szValue);
		}
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_value
 * Signature: ()Lnet/cp/jlibical/ICalValue;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1value
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the first value from CObj
		ICalValue* aValue = cObj->get_value();

		// create a new surrogate, using aValue as the subject (returns NULL if subject is NULL).
		result = createNewICalValueSurrogate(env,aValue);
	}

	return (result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_value_as_string
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1value_1as_1string
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		char* icalStr = cObj->get_value_as_string();

		if (icalStr == NULL)
		{
			icalStr = "";
		}

		result = env->NewStringUTF(icalStr);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_name
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1name
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		char* icalStr = cObj->get_name();

		if (icalStr == NULL)
		{
			icalStr = "";
		}

		result = env->NewStringUTF(icalStr);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_action
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1action
  (JNIEnv *env, jobject jobj, jint value)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		cObj->set_action((icalproperty_action)value);
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_action
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalProperty_get_1action
  (JNIEnv *env, jobject jobj)
{
	jint result = 0;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		result = cObj->get_action();
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_attendee
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1attendee
  (JNIEnv *env, jobject jobj, jstring str)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		cObj->set_attendee((char*)szTemp);
		env->ReleaseStringUTFChars(str,szTemp);
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_attendee
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1attendee
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		char* icalStr = cObj->get_attendee();

		if (icalStr == NULL)
		{
			icalStr = "";
		}

		result = env->NewStringUTF(icalStr);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_comment
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1comment
  (JNIEnv *env, jobject jobj, jstring str)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		cObj->set_comment((char*)szTemp);
		env->ReleaseStringUTFChars(str,szTemp);
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_comment
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1comment
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		char* commentStr = cObj->get_comment();

		if (commentStr == NULL)
		{
			commentStr = "";
		}

		result = env->NewStringUTF(commentStr);
	}
	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_description
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1description
  (JNIEnv *env, jobject jobj, jstring str)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		cObj->set_description((char*)szTemp);
		env->ReleaseStringUTFChars(str,szTemp);
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_description
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1description
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		char* icalStr = cObj->get_description();

		if (icalStr == NULL)
		{
			icalStr = "";
		}

		result = env->NewStringUTF(icalStr);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_dtend
 * Signature: (Lnet/cp/jlibical/ICalTimeType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1dtend
  (JNIEnv *env, jobject jobj, jobject arg)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		icaltimetype aTime;

		if (copyObjToicaltimetype(env,arg,&aTime))
		{
			cObj->set_dtend(aTime);
		}
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_dtend
 * Signature: ()Lnet/cp/jlibical/ICalTimeType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1dtend
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the dtend time from CObj
		icaltimetype aTime = cObj->get_dtend();

		// create a new surrogate, using aTime as the subject.
		result = createNewICalTimeType(env,&aTime);
	}

	return (result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_dtstamp
 * Signature: (Lnet/cp/jlibical/ICalTimeType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1dtstamp
  (JNIEnv *env, jobject jobj, jobject dtstamp)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		icaltimetype aDTStamp;

		if (copyObjToicaltimetype(env,dtstamp,&aDTStamp))
		{
			cObj->set_dtstamp(aDTStamp);
		}
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_dtstamp
 * Signature: ()Lnet/cp/jlibical/ICalTimeType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1dtstamp
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the dtstamp time from CObj
		icaltimetype aDTStamp = cObj->get_dtstamp();

		// create a new surrogate, using aDTStamp as the subject.
		result = createNewICalTimeType(env,&aDTStamp);
	}

	return (result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_dtstart
 * Signature: (Lnet/cp/jlibical/ICalTimeType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1dtstart
  (JNIEnv *env, jobject jobj, jobject arg)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		icaltimetype aTime;

		if (copyObjToicaltimetype(env,arg,&aTime))
		{
			cObj->set_dtstart(aTime);
		}
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_dtstart
 * Signature: ()Lnet/cp/jlibical/ICalTimeType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1dtstart
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the dtend time from CObj
		icaltimetype aTime = cObj->get_dtstart();

		// create a new surrogate, using aTime as the subject.
		result = createNewICalTimeType(env,&aTime);
	}

	return (result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_due
 * Signature: (Lnet/cp/jlibical/ICalTimeType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1due
  (JNIEnv *env, jobject jobj, jobject arg)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		icaltimetype aTime;

		if (copyObjToicaltimetype(env,arg,&aTime))
		{
			cObj->set_due(aTime);
		}
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_due
 * Signature: ()Lnet/cp/jlibical/ICalTimeType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1due
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the dtend time from CObj
		icaltimetype aTime = cObj->get_due();

		// create a new surrogate, using aTime as the subject.
		result = createNewICalTimeType(env,&aTime);
	}

	return (result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_duration
 * Signature: (Lnet/cp/jlibical/ICalDurationType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1duration
  (JNIEnv *env, jobject jobj, jobject arg)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		icaldurationtype aDuration;

		if (copyObjToicaldurationtype(env,arg,&aDuration))
		{
			cObj->set_duration(aDuration);
		}
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_duration
 * Signature: ()Lnet/cp/jlibical/ICalDurationType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1duration
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the dtend time from CObj
		icaldurationtype aDuration = cObj->get_duration();

		// create a new surrogate, using aTime as the subject.
		result = createNewICalDurationType(env,&aDuration);
	}

	return (result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_location
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1location
  (JNIEnv *env, jobject jobj, jstring str)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		cObj->set_location((char*)szTemp);
		env->ReleaseStringUTFChars(str,szTemp);
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_location
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1location
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		char* icalStr = cObj->get_location();

		if (icalStr == NULL)
		{
			icalStr = "";
		}

		result = env->NewStringUTF(icalStr);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_method
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1method
  (JNIEnv *env, jobject jobj, jint value)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		cObj->set_method((icalproperty_method)value);
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_method
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalProperty_get_1method
  (JNIEnv *env, jobject jobj)
{
	jint result = 0;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		result = cObj->get_method();
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_organizer
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1organizer
  (JNIEnv *env, jobject jobj, jstring str)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		cObj->set_organizer((char*)szTemp);
		env->ReleaseStringUTFChars(str,szTemp);
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_organizer
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1organizer
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		char* organizerStr = cObj->get_organizer();

		if (organizerStr == NULL)
		{
			organizerStr = "";
		}

		result = env->NewStringUTF(organizerStr);
	}
	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_owner
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1owner
  (JNIEnv *env, jobject jobj, jstring str)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		cObj->set_owner((char*)szTemp);
		env->ReleaseStringUTFChars(str,szTemp);
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_owner
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1owner
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		char* icalStr = cObj->get_owner();

		if (icalStr == NULL)
		{
			icalStr = "";
		}

		result = env->NewStringUTF(icalStr);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_prodid
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1prodid
  (JNIEnv *env, jobject jobj, jstring str)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		cObj->set_prodid((char*)szTemp);
		env->ReleaseStringUTFChars(str,szTemp);
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_prodid
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1prodid
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		char* icalStr = cObj->get_prodid();

		if (icalStr == NULL)
		{
			icalStr = "";
		}

		result = env->NewStringUTF(icalStr);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_query
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1query
  (JNIEnv *env, jobject jobj, jstring str)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		cObj->set_query((char*)szTemp);
		env->ReleaseStringUTFChars(str,szTemp);
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_query
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1query
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		char* icalStr = cObj->get_query();

		if (icalStr == NULL)
		{
			icalStr = "";
		}

		result = env->NewStringUTF(icalStr);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_queryname
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1queryname
  (JNIEnv *env, jobject jobj, jstring str)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		cObj->set_queryname((char*)szTemp);
		env->ReleaseStringUTFChars(str,szTemp);
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_queryname
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1queryname
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		char* icalStr = cObj->get_queryname();

		if (icalStr == NULL)
		{
			icalStr = "";
		}

		result = env->NewStringUTF(icalStr);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_repeat
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1repeat
  (JNIEnv *env, jobject jobj, jint value)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		cObj->set_repeat(value);
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_repeat
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalProperty_get_1repeat
  (JNIEnv *env, jobject jobj)
{
	jint result = 0;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		result = cObj->get_repeat();
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_summary
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1summary
  (JNIEnv *env, jobject jobj, jstring str)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		cObj->set_summary((char*)szTemp);
		env->ReleaseStringUTFChars(str,szTemp);
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_summary
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1summary
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		char* icalStr = cObj->get_summary();

		if (icalStr == NULL)
		{
			icalStr = "";
		}

		result = env->NewStringUTF(icalStr);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_target
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1target
  (JNIEnv *env, jobject jobj, jstring str)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		cObj->set_target((char*)szTemp);
		env->ReleaseStringUTFChars(str,szTemp);
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_target
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1target
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		char* icalStr = cObj->get_target();

		if (icalStr == NULL)
		{
			icalStr = "";
		}

		result = env->NewStringUTF(icalStr);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_trigger
 * Signature: (Lnet/cp/jlibical/ICalTriggerType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1trigger
  (JNIEnv *env, jobject jobj, jobject arg)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		icaltriggertype aTrigger;

		if (copyObjToicaltriggertype(env,arg,&aTrigger))
		{
			cObj->set_trigger(aTrigger);
		}
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_trigger
 * Signature: ()Lnet/cp/jlibical/ICalTriggerType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1trigger
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the dtend time from CObj
		icaltriggertype aTrigger = cObj->get_trigger();

		// create a new surrogate, using aTime as the subject.
		result = createNewICalTriggerType(env,&aTrigger);
	}

	return (result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_tzid
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1tzid
  (JNIEnv *env, jobject jobj, jstring str)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		cObj->set_tzid((char*)szTemp);
		env->ReleaseStringUTFChars(str,szTemp);
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_tzid
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1tzid
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		char* icalStr = cObj->get_tzid();

		if (icalStr == NULL)
		{
			icalStr = "";
		}

		result = env->NewStringUTF(icalStr);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_uid
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1uid
  (JNIEnv *env, jobject jobj, jstring str)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		cObj->set_uid((char*)szTemp);
		env->ReleaseStringUTFChars(str,szTemp);
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_uid
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1uid
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		char* icalStr = cObj->get_uid();

		if (icalStr == NULL)
		{
			icalStr = "";
		}

		result = env->NewStringUTF(icalStr);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_init__
  (JNIEnv *env, jobject jobj)
{
	setCObjectPtr(env,jobj,new ICalProperty());
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    init
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_init__Ljava_lang_String_2
  (JNIEnv *env, jobject jobj, jstring str)
{
	if (str != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		setCObjectPtr(env,jobj,new ICalProperty((char*)szTemp));
		env->ReleaseStringUTFChars(str,szTemp);
	}
	else
	{
		throwException( env, JLIBICAL_ERR_ILLEGAL_ARGUMENT );
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    init
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_init__I
  (JNIEnv *env, jobject jobj, jint kind)
{
	setCObjectPtr(env,jobj,new ICalProperty((icalproperty_kind)kind));
}


/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_status
 * Signature: (I)V
 */

JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1status
  (JNIEnv *env, jobject jobj, jint value)
{
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		cObj->set_status((icalproperty_status)value);
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_status
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalProperty_get_1status
  (JNIEnv *env, jobject jobj)
{
	jint result = 0;
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		result = cObj->get_status();
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_relcalid
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1relcalid
  (JNIEnv *env, jobject jobj, jstring str)
{
        ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

        if (cObj != NULL)
        {
                const char* szTemp = env->GetStringUTFChars(str,NULL);

                cObj->set_relcalid((char *)szTemp);

                env->ReleaseStringUTFChars(str,szTemp);
        }
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_relcalid
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalProperty_get_1relcalid
  (JNIEnv *env, jobject jobj)
{
        jstring result = NULL;
        ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

        if (cObj != NULL)
        {
                char* icalStr = cObj->get_relcalid();

                if (icalStr == NULL)
                {
                       icalStr = "";
                }

                result = env->NewStringUTF(icalStr);
        }

        return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_exdate
 * Signature: (Lnet/cp/jlibical/ICalTimeType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1exdate
  (JNIEnv *env, jobject jobj, jobject exdate)
{
	// get the ICalProperty c++ object from jobj
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		icaltimetype aExDate;

		if (copyObjToicaltimetype(env,exdate,&aExDate))
		{
			cObj->set_exdate(aExDate);
		}
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_exdate
 * Signature: ()Lnet/cp/jlibical/ICalTimeType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1exdate
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;
	// get the ICalProperty c++ object from jobj
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the exdate from CObj
		icaltimetype aExDate = cObj->get_exdate();

		// create a new surrogate, using aRecurrenceId as the subject.
		result = createNewICalTimeType(env,&aExDate);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_exrule
 * Signature: (Lnet/cp/jlibical/ICalRecurrenceType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1exrule
  (JNIEnv *env, jobject jobj, jobject exrule)
{
	// get the ICalProperty c++ object from jobj
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);
	if (cObj != NULL)
	{
		icalrecurrencetype aExRule;
		if (copyObjToicalrecurrencetype(env,exrule,&aExRule))
		{
			cObj->set_exrule(aExRule);
		}
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_exrule
 * Signature: ()Lnet/cp/jlibical/ICalRecurrenceType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1exrule
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;
	// get the ICalProperty c++ object from jobj
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the exrule from CObj
		icalrecurrencetype aExRule = cObj->get_exrule();

		// create a new surrogate, using aExRule as the subject.
		result = createNewICalRecurrenceType(env,&aExRule);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_freebusy
 * Signature: (Lnet/cp/jlibical/ICalPeriodType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1freebusy
  (JNIEnv *env, jobject jobj, jobject period)
{
	// get the ICalProperty c++ object from jobj
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		icalperiodtype aPeriod;

		if (copyObjToicalperiodtype(env,period,&aPeriod))
		{
			cObj->set_freebusy(aPeriod);
		}
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_freebusy
 * Signature: ()Lnet/cp/jlibical/ICalPeriodType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1freebusy
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;
	// get the ICalProperty c++ object from jobj
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the period from CObj
		icalperiodtype aPeriod = cObj->get_freebusy();

		// create a new surrogate, using aPeriod as the subject.
		result = createNewICalPeriodType(env,&aPeriod);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_recurrenceid
 * Signature: (Lnet/cp/jlibical/ICalTimeType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1recurrenceid
  (JNIEnv *env, jobject jobj, jobject recurrenceid)
{
	// get the ICalProperty c++ object from jobj
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		icaltimetype aRecurrenceId;

		if (copyObjToicaltimetype(env,recurrenceid,&aRecurrenceId))
		{
			cObj->set_recurrenceid(aRecurrenceId);
		}
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_recurrenceid
 * Signature: ()Lnet/cp/jlibical/ICalTimeType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1recurrenceid
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;
	// get the ICalProperty c++ object from jobj
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the exdate from CObj
		icaltimetype aRecurrenceId = cObj->get_recurrenceid();

		// create a new surrogate, using aRecurrenceId as the subject.
		result = createNewICalTimeType(env,&aRecurrenceId);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    set_rrule
 * Signature: (Lnet/cp/jlibical/ICalRecurrenceType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalProperty_set_1rrule
  (JNIEnv *env, jobject jobj, jobject rrule)
{
	// get the ICalProperty c++ object from jobj
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		icalrecurrencetype aRRule;

		if (copyObjToicalrecurrencetype(env,rrule,&aRRule))
		{
			cObj->set_rrule(aRRule);
		}
	}
}

/*
 * Class:     net_cp_jlibical_ICalProperty
 * Method:    get_rrule
 * Signature: ()Lnet/cp/jlibical/ICalRecurrenceType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalProperty_get_1rrule
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;
	// get the ICalProperty c++ object from jobj
	ICalProperty* cObj = getSubjectAsICalProperty(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the rrule from CObj
		icalrecurrencetype aRRule = cObj->get_rrule();

		// create a new surrogate, using aExRule as the subject.
		result = createNewICalRecurrenceType(env,&aRRule);
	}

	return(result);
}
