/* -*- Mode: C -*- */
/*======================================================================
 FILE: net_cp_jlibical_ICalParameter_cxx.cpp
 CREATOR: gnorman 1/10/02
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

#ifndef NET_CP_JLIBICAL_ICALPARAMETER_CXX_H
#include "net_cp_jlibical_ICalParameter_cxx.h"
#endif

#ifndef JLIBICAL_CONSTS_CXX_H
#include "jlibical_consts_cxx.h"
#endif

#ifndef JLIBICAL_UTILS_CXX_H
#include "jlibical_utils_cxx.h"
#endif

#ifndef ICALPARAMETER_CXX_H
#include "icalparameter_cxx.h"
#endif

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    as_ical_string
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalParameter_as_1ical_1string
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	ICalParameter* cObj = getSubjectAsICalParameter(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

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
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    isa
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalParameter_isa
  (JNIEnv *env, jobject jobj)
{
	jint result = 0;
	ICalParameter* cObj = getSubjectAsICalParameter(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		result = cObj->isa();
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    isa_parameter
 * Signature: (Ljava/lang/Object;)Z
 */
JNIEXPORT jboolean JNICALL Java_net_cp_jlibical_ICalParameter_isa_1parameter
  (JNIEnv *env, jobject jobj, jobject arg)
{
	jboolean result = 0;

	// get the c++ object from the jobj
	ICalParameter* cObj = getSubjectAsICalParameter(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);
	if (cObj != NULL)
	{
		// get the c++ object from the arg
		void* argObjPtr = 0;

		if (arg != NULL)
		{
			argObjPtr = getCObjectPtr(env,arg);
		}

		// get the result from the c++ object (candidateValue can be 0, it's cObj's responsibility to handle this if an error).
		result = cObj->isa_parameter(argObjPtr) != 0;
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    get_language
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_ICalParameter_get_1language
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	ICalParameter* cObj = getSubjectAsICalParameter(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		char* icalStr = cObj->get_language();

		if (icalStr == NULL)
		{
			icalStr = "";
		}

		result = env->NewStringUTF(icalStr);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    set_language
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalParameter_set_1language
  (JNIEnv *env, jobject jobj, jstring str)
{
	ICalParameter* cObj = getSubjectAsICalParameter(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		cObj->set_language((char*)szTemp);
		env->ReleaseStringUTFChars(str,szTemp);
	}
}

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    get_encoding
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalParameter_get_1encoding
  (JNIEnv *env, jobject jobj)
{
	jint result = 0;
	ICalParameter* cObj = getSubjectAsICalParameter(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		result = cObj->get_encoding();
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    set_encoding
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalParameter_set_1encoding
  (JNIEnv *env, jobject jobj, jint value)
{
	ICalParameter* cObj = getSubjectAsICalParameter(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		cObj->set_encoding((icalparameter_encoding)value);
	}
}

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    get_role
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalParameter_get_1role
  (JNIEnv *env, jobject jobj)
{
	jint result = 0;
	ICalParameter* cObj = getSubjectAsICalParameter(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		result = cObj->get_role();
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    set_role
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalParameter_set_1role
  (JNIEnv *env, jobject jobj, jint value)
{
	ICalParameter* cObj = getSubjectAsICalParameter(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		cObj->set_role((icalparameter_role)value);
	}
}

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    get_partstat
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_ICalParameter_get_1partstat
  (JNIEnv *env, jobject jobj)
{
	jint result = 0;
	ICalParameter* cObj = getSubjectAsICalParameter(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		result = cObj->get_partstat();
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    set_partstat
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalParameter_set_1partstat
  (JNIEnv *env, jobject jobj, jint value)
{
	ICalParameter* cObj = getSubjectAsICalParameter(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		cObj->set_partstat((icalparameter_partstat)value);
	}
}

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalParameter_init__
  (JNIEnv *env, jobject jobj)
{
	setCObjectPtr(env,jobj,new ICalParameter());
}

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    init
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalParameter_init__Ljava_lang_String_2
  (JNIEnv *env, jobject jobj, jstring str)
{
	if (str != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		setCObjectPtr(env,jobj,new ICalParameter((char*)szTemp));
		env->ReleaseStringUTFChars(str,szTemp);
	}
	else
	{
		throwException( env, JLIBICAL_ERR_ILLEGAL_ARGUMENT );
	}
}

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    init
 * Signature: (ILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalParameter_init__ILjava_lang_String_2
  (JNIEnv *env, jobject jobj, jint kind, jstring str)
{
	if (str != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		setCObjectPtr(env,jobj,new ICalParameter((icalparameter_kind)kind,(char*)szTemp));
		env->ReleaseStringUTFChars(str,szTemp);
	}
	else
	{
		throwException( env, JLIBICAL_ERR_ILLEGAL_ARGUMENT );
	}
}

/*
 * Class:     net_cp_jlibical_ICalParameter
 * Method:    init
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_ICalParameter_init__I
  (JNIEnv *env, jobject jobj, jint kind)
{
	setCObjectPtr(env,jobj,new ICalParameter((icalparameter_kind)kind));
}
