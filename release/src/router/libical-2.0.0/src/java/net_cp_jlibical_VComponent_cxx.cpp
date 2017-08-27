/*
/*======================================================================
 FILE: net_cp_jlibical_VComponent_cxx.cpp
 CREATOR: gnorman 1/10/02
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

#ifndef NET_CP_JLIBICAL_VCOMPONENT_CXX_H
#include "net_cp_jlibical_VComponent_cxx.h"
#endif

#ifndef JLIBICAL_CONSTS_CXX_H
#include "jlibical_consts_cxx.h"
#endif

#ifndef JLIBICAL_UTILS_CXX_H
#include "jlibical_utils_cxx.h"
#endif

#ifndef VCOMPONENT_CXX_H
#include "vcomponent_cxx.h"
#endif

#ifndef ICALPROPERTY_CXX_H
#include "icalproperty_cxx.h"
#endif

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    as_ical_string
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_VComponent_as_1ical_1string
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

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
 * Class:     net_cp_jlibical_VComponent
 * Method:    isa
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_VComponent_isa
  (JNIEnv *env, jobject jobj)
{
	jint result = 0;
	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		result = cObj->isa();
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    isa_component
 * Signature: (Ljava/lang/Object;)Z
 */
JNIEXPORT jboolean JNICALL Java_net_cp_jlibical_VComponent_isa_1component
  (JNIEnv *env, jobject jobj, jobject candidateObj)
{
	jboolean result = 0;

	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);
	if (cObj != NULL)
	{
		void* candidateValue = 0;

		if (candidateObj != NULL)
		{
			// get the c++ object from candidateObj (as long)
			candidateValue = getCObjectPtr(env,candidateObj);
		}

		// get the result from the c++ object (candidateValue can be 0, it's cObj's responsibility to handle this if an error).
		result = cObj->isa_component(candidateValue) != 0;
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    add_property
 * Signature: (Lnet/cp/jlibical/ICalProperty;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_add_1property
  (JNIEnv *env, jobject jobj, jobject jprop)
{
	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);
	if (cObj != NULL)
	{
		// get the ICalProperty c++ object from jprop
		ICalProperty* icalProperty = getSubjectAsICalProperty(env,jprop,JLIBICAL_ERR_ILLEGAL_ARGUMENT);

		if (icalProperty != NULL)
		{
			cObj->add_property(icalProperty);
		}
	}
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    remove_property
 * Signature: (Lnet/cp/jlibical/ICalProperty;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_remove_1property
  (JNIEnv *env, jobject jobj, jobject jprop)
{
	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);
	if (cObj != NULL)
	{
		// get the ICalProperty c++ object from jprop
		ICalProperty* icalProperty = getSubjectAsICalProperty(env,jprop,JLIBICAL_ERR_ILLEGAL_ARGUMENT);

		if (icalProperty != NULL)
		{
			cObj->remove_property(icalProperty);
		}
	}
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    count_properties
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_VComponent_count_1properties
  (JNIEnv *env, jobject jobj, jint kind)
{
	jint result = 0;

	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);
	if (cObj != NULL)
	{
		result = cObj->count_properties((icalproperty_kind)kind);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_current_property
 * Signature: ()Lnet/cp/jlibical/ICalProperty;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1current_1property
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;
	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the current property from CObj
		ICalProperty* aProperty = cObj->get_current_property();

		// create a new surrogate, using aProperty as the subject (returns NULL if subject is NULL).
		result = createNewICalPropertySurrogate(env,aProperty);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_first_property
 * Signature: (I)Lnet/cp/jlibical/ICalProperty;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1first_1property
  (JNIEnv *env, jobject jobj, jint kind)
{
	jobject result = 0;
	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the first property from CObj
		ICalProperty* aProperty = cObj->get_first_property((icalproperty_kind)kind);

		// create a new surrogate, using aProperty as the subject (returns NULL if subject is NULL).
		result = createNewICalPropertySurrogate(env,aProperty);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_next_property
 * Signature: (I)Lnet/cp/jlibical/ICalProperty;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1next_1property
  (JNIEnv *env, jobject jobj, jint kind)
{
	jobject result = 0;
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the next property from CObj
		ICalProperty* aProperty = cObj->get_next_property((icalproperty_kind)kind);

		// create a new surrogate, using aProperty as the subject (returns NULL if subject is NULL).
		result = createNewICalPropertySurrogate(env,aProperty);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_inner
 * Signature: ()Lnet/cp/jlibical/VComponent;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1inner
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the next property from CObj
		VComponent* inner = cObj->get_inner();

		// create a new surrogate, using inner as the subject (returns NULL if subject is NULL).
		result = createNewVComponentSurrogate(env,inner);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    add_component
 * Signature: (Lnet/cp/jlibical/VComponent;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_add_1component
  (JNIEnv *env, jobject jobj, jobject jcomp)
{
	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);
	if (cObj != NULL)
	{
		// get the VComponent c++ object from jcomp
		VComponent* aComponent = getSubjectAsVComponent(env,jcomp,JLIBICAL_ERR_ILLEGAL_ARGUMENT);

		if (aComponent != NULL)
		{
			cObj->add_component(aComponent);
		}
	}
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    remove_component
 * Signature: (Lnet/cp/jlibical/VComponent;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_remove_1component
  (JNIEnv *env, jobject jobj, jobject jcomp)
{
	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);
	if (cObj != NULL)
	{
		// get the VComponent c++ object from jcomp
		VComponent* aComponent = getSubjectAsVComponent(env,jcomp,JLIBICAL_ERR_ILLEGAL_ARGUMENT);

		if (aComponent != NULL)
		{
			cObj->remove_component(aComponent);
		}
	}
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    count_components
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_VComponent_count_1components
  (JNIEnv *env, jobject jobj, jint kind)
{
	jint result = 0;

	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);
	if (cObj != NULL)
	{
		result = cObj->count_components((icalcomponent_kind)kind);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_current_component
 * Signature: ()Lnet/cp/jlibical/VComponent;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1current_1component
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the next property from CObj
		VComponent* aComponent = cObj->get_current_component();

		// create a new surrogate, using aComponent as the subject (returns NULL if subject is NULL).
		result = createNewVComponentSurrogate(env,aComponent);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_first_component
 * Signature: (I)Lnet/cp/jlibical/VComponent;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1first_1component
  (JNIEnv *env, jobject jobj, jint kind)
{
	jobject result = 0;
	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the first component from CObj
		VComponent* aComponent = cObj->get_first_component((icalcomponent_kind)kind);

		// create a new surrogate, using aComponent as the subject (returns NULL if subject is NULL).
		result = createNewVComponentSurrogate(env,aComponent);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_next_component
 * Signature: (I)Lnet/cp/jlibical/VComponent;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1next_1component
  (JNIEnv *env, jobject jobj, jint kind)
{
	jobject result = 0;
	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the first component from CObj
		VComponent* aComponent = cObj->get_next_component((icalcomponent_kind)kind);

		// create a new surrogate, using aComponent as the subject (returns NULL if subject is NULL).
		result = createNewVComponentSurrogate(env,aComponent);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_dtstart
 * Signature: ()Lnet/cp/jlibical/ICalTimeType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1dtstart
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;
	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the dtstart time from CObj
		icaltimetype aTime = cObj->get_dtstart();

                // create a new surrogate, using aTime as the subject.
		result = createNewICalTimeType(env,&aTime);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    set_dtstart
 * Signature: (Lnet/cp/jlibical/ICalTimeType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_set_1dtstart
  (JNIEnv *env, jobject jobj, jobject dtstart)
{
	jobject result = 0;
	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		icaltimetype aStartTime;

		if (copyObjToicaltimetype(env,dtstart,&aStartTime))
		{
			cObj->set_dtstart(aStartTime);
		}
	}
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_dtend
 * Signature: ()Lnet/cp/jlibical/ICalTimeType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1dtend
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;
	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the dtend time from CObj
		icaltimetype aTime = cObj->get_dtend();

		// create a new surrogate, using aTime as the subject.
		result = createNewICalTimeType(env,&aTime);

	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    set_dtend
 * Signature: (Lnet/cp/jlibical/ICalTimeType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_set_1dtend
  (JNIEnv *env, jobject jobj, jobject dtend)
{
	jobject result = 0;
	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		icaltimetype anEndTime;

		if (copyObjToicaltimetype(env,dtend,&anEndTime))
		{
			cObj->set_dtend(anEndTime);
		}
	}
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_duration
 * Signature: ()Lnet/cp/jlibical/ICalDurationType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1duration
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;
	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the duration time from CObj
		icaldurationtype aDuration = cObj->get_duration();

		// create a new surrogate, using aDuration as the subject.
		result = createNewICalDurationType(env,&aDuration);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    set_duration
 * Signature: (Lnet/cp/jlibical/ICalDurationType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_set_1duration
  (JNIEnv *env, jobject jobj, jobject duration)
{
	jobject result = 0;
	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		icaldurationtype aDuration;

		if (copyObjToicaldurationtype(env,duration,&aDuration))
		{
			cObj->set_duration(aDuration);
		}
	}
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_method
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_cp_jlibical_VComponent_get_1method
  (JNIEnv *env, jobject jobj)
{
	jint result = 0;
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		result = cObj->get_method();
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    set_method
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_set_1method
  (JNIEnv *env, jobject jobj, jint value)
{
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		cObj->set_method((icalproperty_method)value);
	}
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_summary
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_VComponent_get_1summary
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

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
 * Class:     net_cp_jlibical_VComponent
 * Method:    set_summary
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_set_1summary
  (JNIEnv *env, jobject jobj, jstring str)
{
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		cObj->set_summary((char*)szTemp);
		env->ReleaseStringUTFChars(str,szTemp);
	}
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_dtstamp
 * Signature: ()Lnet/cp/jlibical/ICalTimeType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1dtstamp
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;
	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the recurrenceid from CObj
		icaltimetype aDTStamp = cObj->get_dtstamp();

        // create a new surrogate, using aRecurrenceId as the subject.
		result = createNewICalTimeType(env,&aDTStamp);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    set_dtstamp
 * Signature: (Lnet/cp/jlibical/ICalTimeType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_set_1dtstamp
  (JNIEnv *env, jobject jobj, jobject dtstamp)
{
	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

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
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_location
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_VComponent_get_1location
  (JNIEnv *env, jobject jobj)
{
        jstring result = NULL;
        VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

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
 * Class:     net_cp_jlibical_VComponent
 * Method:    set_location
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_set_1location
  (JNIEnv *env, jobject jobj, jstring str)
{
        VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

        if (cObj != NULL)
        {
                const char* szTemp = env->GetStringUTFChars(str,NULL);

                cObj->set_summary((char*)szTemp);
                env->ReleaseStringUTFChars(str,szTemp);
        }
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_description
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_VComponent_get_1description
  (JNIEnv *env, jobject jobj)
{
        jstring result = NULL;
        VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

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
 * Class:     net_cp_jlibical_VComponent
 * Method:    set_description
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_set_1description
  (JNIEnv *env, jobject jobj, jstring str)
{
        VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

        if (cObj != NULL)
        {
                const char* szTemp = env->GetStringUTFChars(str,NULL);

                cObj->set_summary((char*)szTemp);
                env->ReleaseStringUTFChars(str,szTemp);
        }
}
/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_uid
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_VComponent_get_1uid
  (JNIEnv *env, jobject jobj)
{
	jstring result = NULL;
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

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
 * Class:     net_cp_jlibical_VComponent
 * Method:    set_uid
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_set_1uid
  (JNIEnv *env, jobject jobj, jstring str)
{
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		cObj->set_uid((char*)szTemp);
		env->ReleaseStringUTFChars(str,szTemp);
	}
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_first_real_component
 * Signature: ()Lnet/cp/jlibical/VComponent;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1first_1real_1component
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;
	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the first component from CObj
		VComponent* aComponent = cObj->get_first_real_component();

		// create a new surrogate, using aComponent as the subject (returns NULL if subject is NULL).
		result = createNewVComponentSurrogate(env,aComponent);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_init__
  (JNIEnv *env, jobject jobj)
{
	setCObjectPtr(env,jobj,new VComponent());
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    init
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_init__Ljava_lang_String_2
  (JNIEnv *env, jobject jobj, jstring str)
{
	if (str != NULL)
	{
		const char* szTemp = env->GetStringUTFChars(str,NULL);

		setCObjectPtr(env,jobj,new VComponent((char*)szTemp));
		env->ReleaseStringUTFChars(str,szTemp);
	}
	else
	{
		throwException( env, JLIBICAL_ERR_ILLEGAL_ARGUMENT );
	}
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    init
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_init__I
  (JNIEnv *env, jobject jobj, jint kind)
{
	setCObjectPtr(env,jobj,new VComponent((icalcomponent_kind)kind));
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    set_relcalid
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_set_1relcalid
  (JNIEnv *env, jobject jobj, jstring str)
{
        VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

        if (cObj != NULL)
        {
                const char* szTemp = env->GetStringUTFChars(str,NULL);

                cObj->set_relcalid((char*)szTemp);
                env->ReleaseStringUTFChars(str,szTemp);
        }
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_relcalid
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_cp_jlibical_VComponent_get_1relcalid
  (JNIEnv *env, jobject jobj)
{
        jstring result = NULL;
        VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

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
 * Class:     net_cp_jlibical_VComponent
 * Method:    get_recurrenceid
 * Signature: ()Lnet/cp/jlibical/ICalTimeType;
 */
JNIEXPORT jobject JNICALL Java_net_cp_jlibical_VComponent_get_1recurrenceid
  (JNIEnv *env, jobject jobj)
{
	jobject result = 0;
	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		// get the recurrenceid from CObj
		icaltimetype aRecurrenceId = cObj->get_recurrenceid();

        // create a new surrogate, using aRecurrenceId as the subject.
		result = createNewICalTimeType(env,&aRecurrenceId);
	}

	return(result);
}

/*
 * Class:     net_cp_jlibical_VComponent
 * Method:    set_recurrenceid
 * Signature: (Lnet/cp/jlibical/ICalTimeType;)V
 */
JNIEXPORT void JNICALL Java_net_cp_jlibical_VComponent_set_1recurrenceid
  (JNIEnv *env, jobject jobj, jobject recurrenceid)
{
	jobject result = 0;
	// get the VComponent c++ object from jobj
	VComponent* cObj = getSubjectAsVComponent(env,jobj,JLIBICAL_ERR_CLIENT_INTERNAL);

	if (cObj != NULL)
	{
		icaltimetype aRecurrenceId;

		if (copyObjToicaltimetype(env,recurrenceid,&aRecurrenceId))
		{
			cObj->set_recurrenceid(aRecurrenceId);
		}
	}
}
