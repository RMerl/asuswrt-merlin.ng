/* -*- Mode: C -*- */
/*======================================================================
 FILE: net_cp_jlibical_ICalValue_cxx.cpp
 CREATOR: gnorman 1/10/02
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

#ifndef NET_CP_JLIBICAL_ICALVALUE_CXX_H
#include "net_cp_jlibical_ICalValue_cxx.h"
#endif

#ifndef JLIBICAL_CONSTS_CXX_H
#include "jlibical_consts_cxx.h"
#endif

#ifndef JLIBICAL_UTILS_CXX_H
#include "jlibical_utils_cxx.h"
#endif

#ifndef ICALVALUE_CXX_H
#include "icalvalue_cxx.h"
#endif

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    as_ical_string
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalValue_as_1ical_1string
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	ICalValue* cObj = getSubjectAsICalValue(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

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
 * Class:     net_cp_jlibical_ICalValue
 * Method:    isa
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalValue_isa
  (JNIEnv *env, jobject jobj)
{
	jint result = 0;
	ICalValue* cObj = getSubjectAsICalValue(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		result = cObj->isa();
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    isa_value
 * Signature: (Ljava/lang/Object;)Z
 */
JNIEXPORT jboolean JNICALL Java_net_cp_jlibical_ICalValue_isa_1value
  (JNIEnv *env, jobject jobj, jobject arg)
{
	jboolean result = 0;

	// get the c++ object from the jobj
	ICalValue* cObj = getSubjectAsICalValue(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);
	if (cObj != NULL)
	{
		// get the c++ object from arg
		void* argObjPtr = 0;

		if (arg != NULL)
		{
			argObjPtr = getCObjectPtr(env,jobj);
		}

		// get the result from the c++ object
		result = cObj->isa_value(argObjPtr) != 0;
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    set_trigger
 * Signature: (Lnet/cp/jlibical/ICalTriggerType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalValue_set_1trigger
  (JNIEnv *env, jobject jobj, jobject arg)
{
	// get the c++ object from the jobj
	ICalValue* cObj = getSubjectAsICalValue(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);
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
 * Class:     net_cp_jlibical_ICalValue
 * Method:    get_trigger
 * Signature: ()Lnet/cp/jlibical/ICalTriggerType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalValue_get_1trigger
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;

	// get the c++ object from the jobj
	ICalValue* cObj = getSubjectAsICalValue(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the trigger from CObj
		icaltriggertype aTrigger = cObj->get_trigger();

		// create a new surrogate, using aTrigger as the subject.
		result = createNewICalTriggerType(env,&aTrigger);
	}

	return (result);
}

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    get_method
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalValue_get_1method
  (JNIEnv *env, jobject jobj)
{
	jint result = 0;
	ICalValue* cObj = getSubjectAsICalValue(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		result = cObj->get_method();
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    set_method
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalValue_set_1method
  (JNIEnv *env, jobject jobj, jint value)
{
	ICalValue* cObj = getSubjectAsICalValue(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		cObj->set_method((icalproperty_method)value);
	}
}

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    get_text
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalValue_get_1text
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	ICalValue* cObj = getSubjectAsICalValue(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		char* icalStr = cObj->get_text();

		if (icalStr == NULL)
		{
			icalStr = "";
		}

		result = env->NewStringUTF(icalStr);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    set_text
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalValue_set_1text
  (JNIEnv *env, jobject jobj, jstring str)
{
	ICalValue* cObj = getSubjectAsICalValue(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		cObj->set_text((char*)szTemp);                                                                                                                                                                                                                    
		env->ReleaseStringUTFChars(str,szTemp);
	}
}

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    get_duration
 * Signature: ()Lnet/cp/jlibical/ICalDurationType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalValue_get_1duration
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;

	// get the c++ object from the jobj
	ICalValue* cObj = getSubjectAsICalValue(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the duration from CObj
		icaldurationtype aDuration = cObj->get_duration();

		// create a new surrogate, using aDuration as the subject.
		result = createNewICalDurationType(env,&aDuration);
	}

	return (result);
}

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    set_duration
 * Signature: (Lnet/cp/jlibical/ICalDurationType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalValue_set_1duration
  (JNIEnv *env, jobject jobj, jobject arg)
{
	// get the c++ object from the jobj
	ICalValue* cObj = getSubjectAsICalValue(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

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
 * Class:     net_cp_jlibical_ICalValue
 * Method:    get_query
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalValue_get_1query
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	ICalValue* cObj = getSubjectAsICalValue(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

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
 * Class:     net_cp_jlibical_ICalValue
 * Method:    set_query
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalValue_set_1query
  (JNIEnv *env, jobject jobj, jstring str)
{
	ICalValue* cObj = getSubjectAsICalValue(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		cObj->set_query((char*)szTemp);
		env->ReleaseStringUTFChars(str,szTemp);
	}
}

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    get_datetime
 * Signature: ()Lnet/cp/jlibical/ICalTimeType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_ICalValue_get_1datetime
  (JNIEnv *env, jobject jobj)
{
	jobject result = NULL;

	// get the c++ object from the jobj
	ICalValue* cObj = getSubjectAsICalValue(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		icaltimetype aTime = cObj->get_datetime();
		result = createNewICalTimeType(env,&aTime);
	}

	return (result);
}

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    set_datetime
 * Signature: (Lnet/cp/jlibical/ICalTimeType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalValue_set_1datetime
  (JNIEnv *env, jobject jobj, jobject arg)
{
	// get the c++ object from the jobj
	ICalValue* cObj = getSubjectAsICalValue(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		icaltimetype aTime;

		if (copyObjToicaltimetype(env,arg,&aTime))
		{
			cObj->set_datetime(aTime);
		}
	}
}

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    get_action
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalValue_get_1action
  (JNIEnv *env, jobject jobj)
{
	jint result = 0;
	ICalValue* cObj = getSubjectAsICalValue(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		result = cObj->get_action();
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    set_action
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalValue_set_1action
  (JNIEnv *env, jobject jobj, jint value)
{
	ICalValue* cObj = getSubjectAsICalValue(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		cObj->set_action((icalproperty_action)value);
	}
}

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalValue_init__
  (JNIEnv *env, jobject jobj)
{
	setCObjectPtr(env,jobj,new ICalValue());
}

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    init
 * Signature: (ILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalValue_init__ILjava_lang_String_2
  (JNIEnv *env, jobject jobj, jint kind, jstring str)
{
	if (str != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		setCObjectPtr(env,jobj,new ICalValue((icalvalue_kind)kind,(char*)szTemp));
		env->ReleaseStringUTFChars(str,szTemp);
	}
	else
	{
		throwException( env, JLIBICAL_ERR_ILLEGAL_ARGUMENT );
	}
}

/*
 * Class:     net_cp_jlibical_ICalValue
 * Method:    init
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalValue_init__I
  (JNIEnv *env, jobject jobj, jint kind)
{
	setCObjectPtr(env,jobj,new ICalValue((icalvalue_kind)kind));
}
