/*
/*======================================================================
 FILE: jlibical_utils_cxx.cpp
 CREATOR: Srinivasa Boppana/George Norman
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

#ifndef JLIBICAL_UTILS_CXX_H
#include "jlibical_utils_cxx.h"
#endif

#ifndef JLIBICAL_CONSTS_CXX_H
#include "jlibical_consts_cxx.h"
#endif

#ifndef ICALPARAMETER_CXX_H
#include "icalparameter_cxx.h"
#endif

#ifndef VCOMPONENT_CXX_H
#include "vcomponent_cxx.h"
#endif

#ifndef ICALPROPERTY_CXX_H
#include "icalproperty_cxx.h"
#endif

#ifndef ICALVALUE_CXX_H
#include "icalvalue_cxx.h"
#endif

#ifndef _jni_ICalTimeType_H
#include "jniICalTimeType_cxx.h"
#endif

#ifndef _jni_ICalTriggerType_H
#include "jniICalTriggerType_cxx.h"
#endif

#ifndef _jni_ICalDurationType_H
#include "jniICalDurationType_cxx.h"
#endif

#ifndef _jni_ICalRecurrenceType_H
#include "jniICalRecurrenceType_cxx.h"
#endif

#ifndef _jni_ICalPeriodType_H
#include "jniICalPeriodType_cxx.h"
#endif

//-------------------------------------------------------
// Returns a pointer to the subject (a c++ object) for the given surrogate (a java object)
//-------------------------------------------------------
void* getCObjectPtr(JNIEnv *env, jobject surrogate)
{
	void* result = 0;
    jclass jcls = env->GetObjectClass(surrogate);
    jfieldID fid = env->GetFieldID(jcls,"m_Obj","J");

    if (fid == NULL)
    {
		// this should never happen.
        throwException( env, JLIBICAL_ERR_CLIENT_INTERNAL );
        return(NULL);
    }

	result = (void*)env->GetLongField(surrogate,fid);
    if (result == NULL)
	{
		// the proxy object (java) has no subject (c++ object)
        throwException( env, JLIBICAL_ERR_CLIENT_INTERNAL );
        return(NULL);
	}

	return(result);
}

//-------------------------------------------------------
// Set the subject (a c++ object) for the given surrogate (a java object).
// Throws exception if the m_Obj field can not be found.
//-------------------------------------------------------
void setCObjectPtr(JNIEnv *env, jobject surrogate, void* subject)
{
    jclass jcls = env->GetObjectClass(surrogate);
    jfieldID fid = env->GetFieldID(jcls,"m_Obj","J");

    if (fid == NULL)
    {
        throwException( env, JLIBICAL_ERR_CLIENT_INTERNAL );
        return;
    }

	env->SetLongField(surrogate,fid,(long)subject);
}

//-------------------------------------------------------
// Return the pointer to the subject (as an VComponent*) from the given surrogate.
// If the subject is not an VComponent type, or if the subject is NULL, then return NULL.
//-------------------------------------------------------
VComponent* getSubjectAsVComponent(JNIEnv *env, jobject surrogateComponent, int exceptionType)
{
	VComponent* result = (VComponent*)(getCObjectPtr(env,surrogateComponent));

	if (result == NULL)
	{
        throwException(env, exceptionType );
	}

	return(result);
}

//-------------------------------------------------------
// Return the pointer to the subject (as an ICalProperty*) from the given surrogate.
// If the subject is not an ICalProperty type, or if the subject is NULL, then return NULL.
//-------------------------------------------------------
ICalProperty* getSubjectAsICalProperty(JNIEnv *env, jobject surrogateProperty, int exceptionType)
{
	ICalProperty* result = (ICalProperty*)(getCObjectPtr(env,surrogateProperty));

	if (result == NULL)
	{
        throwException(env, exceptionType );
	}

	return(result);
}

//-------------------------------------------------------
// Return the pointer to the subject (as an ICalValue*) from the given surrogate.
// If the subject is not an ICalValue type, or if the subject is NULL, then return NULL.
//-------------------------------------------------------
ICalValue* getSubjectAsICalValue(JNIEnv *env, jobject surrogateValue, int exceptionType)
{
	ICalValue* result = (ICalValue*)(getCObjectPtr(env,surrogateValue));

	if (result == NULL)
	{
        throwException( env, exceptionType );
	}

	return(result);
}

//-------------------------------------------------------
// Return the pointer to the subject (as an ICalParameter*) from the given surrogate.
// If the subject is not an ICalParameter type, or if the subject is NULL, then return NULL.
//-------------------------------------------------------
ICalParameter* getSubjectAsICalParameter(JNIEnv *env, jobject surrogateParameter, int exceptionType)
{
	ICalParameter* result = (ICalParameter*)(getCObjectPtr(env,surrogateParameter));

	if (result == NULL)
	{
        throwException( env, exceptionType );
	}

	return(result);
}

//-------------------------------------------------------
// Copy the data from the src (a java ICalTimeType object)
// to the dest (a c struct icaltimetype*).
// Returns true if success.  False if exception is thrown:
//	- the src java object is not an ICalTimeType type
//	- the dest c struct is null.
//-------------------------------------------------------
bool copyObjToicaltimetype(JNIEnv *env, jobject src, icaltimetype* dest)
{
	bool result = false;

	if (dest != NULL && env->IsInstanceOf(src,env->FindClass(JLIBICAL_CLASS_ICALTIMETYPE)))
	{
		jni_GetAll_from_ICalTimeType(dest, env, src);
		result = true;
	}
	else
	{
        throwException( env, JLIBICAL_ERR_ILLEGAL_ARGUMENT );
	}

	return(result);
}

//-------------------------------------------------------
// Copy the data from the src (a java ICalTriggerType object)
// to the dest (a c struct icaltriggertype*).
// Returns true if success.  False if exception is thrown:
//	- the src java object is not an ICalTriggerType type
//	- the dest c struct is null.
//-------------------------------------------------------
bool copyObjToicaltriggertype(JNIEnv *env, jobject src, icaltriggertype* dest)
{
	bool result = false;

	if (dest != NULL && env->IsInstanceOf(src,env->FindClass(JLIBICAL_CLASS_ICALTRIGGERTYPE)))
	{
		jni_GetAll_from_ICalTriggerType(dest, env, src);
		result = true;
	}
	else
	{
        throwException( env, JLIBICAL_ERR_ILLEGAL_ARGUMENT );
	}

	return(result);
}

//-------------------------------------------------------
// Copy the data from the src (a java ICalDurationType object)
// to the dest (a c struct icaldurationtype*).
// Returns true if success.  False if exception is thrown:
//	- the src java object is not an ICalDurationType type
//	- the dest c struct is null.
//-------------------------------------------------------
bool copyObjToicaldurationtype(JNIEnv *env, jobject src, icaldurationtype* dest)
{
	bool result = false;

	if (dest != NULL && env->IsInstanceOf(src,env->FindClass(JLIBICAL_CLASS_ICALDURATIONTYPE)))
	{
		jni_GetAll_from_ICalDurationType(dest, env, src);
		result = true;
	}
	else
	{
        throwException( env, JLIBICAL_ERR_ILLEGAL_ARGUMENT );
	}

	return(result);
}

//-------------------------------------------------------
// Copy the data from the src (a java ICalRecurrenceType object)
// to the dest (a c struct icalrecurrencetype*).
// Returns true if success.  False if exception is thrown:
//	- the src java object is not an ICalRecurrenceType type
//	- the dest c struct is null.
//-------------------------------------------------------
bool copyObjToicalrecurrencetype(JNIEnv *env, jobject src, icalrecurrencetype* dest)
{
	bool result = false;

	if (dest != NULL && env->IsInstanceOf(src,env->FindClass(JLIBICAL_CLASS_ICALRECURRENCETYPE)))
	{
		jni_GetAll_from_ICalRecurrenceType(dest, env, src);
		result = true;
	}
	else
	{
        throwException( env, JLIBICAL_ERR_ILLEGAL_ARGUMENT );
	}

	return(result);
}

//-------------------------------------------------------
// Copy the data from the src (a java ICalPeriodType object)
// to the dest (a c struct icalperiodtype*).
// Returns true if success.  False if exception is thrown:
//	- the src java object is not an ICalPeriodType type
//	- the dest c struct is null.
//-------------------------------------------------------
bool copyObjToicalperiodtype(JNIEnv *env, jobject src, icalperiodtype* dest)
{
	bool result = false;

	if (dest != NULL && env->IsInstanceOf(src,env->FindClass(JLIBICAL_CLASS_ICALPERIODTYPE)))
	{
		jni_GetAll_from_ICalPeriodType(dest, env, src);
		result = true;
	}
	else
	{
        throwException( env, JLIBICAL_ERR_ILLEGAL_ARGUMENT );
	}

	return(result);
}

//-------------------------------------------------------
// Create a new VComponent surrogate for given subject.
// If subject is NULL, then returns NULL (will not create a
// surrogate to a NULL subject);
//-------------------------------------------------------
jobject createNewVComponentSurrogate(JNIEnv *env, VComponent* subject)
{
	char* classname = JLIBICAL_CLASS_VCOMPONENT;
	if (dynamic_cast<VAlarm*>(subject))
		classname = JLIBICAL_CLASS_VALARM;
	else if (dynamic_cast<VCalendar*>(subject))
		classname = JLIBICAL_CLASS_VCALENDAR;
	else if (dynamic_cast<VEvent*>(subject))
		classname = JLIBICAL_CLASS_VEVENT;
	else if (dynamic_cast<VQuery*>(subject))
		classname = JLIBICAL_CLASS_VQUERY;
	else if (dynamic_cast<VToDo*>(subject))
		classname = JLIBICAL_CLASS_VTODO;
        else if (dynamic_cast<VAgenda*>(subject))
                classname = JLIBICAL_CLASS_VAGENDA;

	return(doCreateNewSurrogate(env,env->FindClass(classname),(jlong)subject));
}

//-------------------------------------------------------
// Create a new ICalProperty surrogate for given subject.
// If subject is NULL, then returns NULL (will not create a
// surrogate to a NULL subject);
//-------------------------------------------------------
jobject createNewICalPropertySurrogate(JNIEnv *env, ICalProperty* subject)
{
	return(doCreateNewSurrogate(env,env->FindClass(JLIBICAL_CLASS_ICALPROPERTY),(jlong)subject));
}

//-------------------------------------------------------
// Create a new ICalValue surrogate for given subject.
// If subject is NULL, then returns NULL (will not create a
// surrogate to a NULL subject);
//-------------------------------------------------------
jobject createNewICalValueSurrogate(JNIEnv *env, ICalValue* subject)
{
	return(doCreateNewSurrogate(env,env->FindClass(JLIBICAL_CLASS_ICALVALUE),(jlong)subject));
}

//-------------------------------------------------------
// Create a new ICalParameter surrogate for given subject.
// If subject is NULL, then returns NULL (will not create a
// surrogate to a NULL subject);
//-------------------------------------------------------
jobject createNewICalParameterSurrogate(JNIEnv *env, ICalParameter* subject)
{
	return(doCreateNewSurrogate(env,env->FindClass(JLIBICAL_CLASS_ICALPARAMETER),(jlong)subject));
}

//-------------------------------------------------------
// Create a new ICalTimeType object from the given source struct.
// A copy is made,  .
// If source is NULL, then returns NULL (will not create an
// object to a NULL source);
//-------------------------------------------------------
jobject createNewICalTimeType(JNIEnv *env, icaltimetype* source)
{
	return(doCreateNewSurrogate(env,env->FindClass(JLIBICAL_CLASS_ICALTIMETYPE),(jlong)source));
}

//-------------------------------------------------------
// Create a new ICalTriggerType object from the given source struct.
// A copy is made,  .
// If source is NULL, then returns NULL (will not create an
// object to a NULL source);
//-------------------------------------------------------
jobject createNewICalTriggerType(JNIEnv *env, icaltriggertype* source)
{
	return(doCreateNewSurrogate(env,env->FindClass(JLIBICAL_CLASS_ICALTRIGGERTYPE),(jlong)source));
}

//-------------------------------------------------------
// Create a new ICalDurationType object from the given source struct.
// A copy is made,  .
// If source is NULL, then returns NULL (will not create an
// object to a NULL source);
//-------------------------------------------------------
jobject createNewICalDurationType(JNIEnv *env, icaldurationtype* source)
{
	return(doCreateNewSurrogate(env,env->FindClass(JLIBICAL_CLASS_ICALDURATIONTYPE),(jlong)source));
}

//-------------------------------------------------------
// Create a new ICalRecurrenceType object from the given source struct.
// A copy is made,  .
// If source is NULL, then returns NULL (will not create an
// object to a NULL source);
//-------------------------------------------------------
jobject createNewICalRecurrenceType(JNIEnv *env, icalrecurrencetype* source)
{
	return(doCreateNewSurrogate(env,env->FindClass(JLIBICAL_CLASS_ICALRECURRENCETYPE),(jlong)source));
}

//-------------------------------------------------------
// Create a new ICalPeriodType object from the given source struct.
// A copy is made,  .
// If source is NULL, then returns NULL (will not create an
// object to a NULL source);
//-------------------------------------------------------
jobject createNewICalPeriodType(JNIEnv *env, icalperiodtype* source)
{
	return(doCreateNewSurrogate(env,env->FindClass(JLIBICAL_CLASS_ICALPERIODTYPE),(jlong)source));
}

//-------------------------------------------------------
// Creat a new surrogate of the given type for the given subject.
//-------------------------------------------------------
jobject doCreateNewSurrogate(JNIEnv *env, jclass surrogateClass, jlong subject)
{
	jobject result = NULL;

	if (subject != NULL)
	{
		jmethodID jconstructorID = env->GetMethodID(surrogateClass, "<init>", "(J)V");

		result = env->NewObject(surrogateClass, jconstructorID, subject);
	}

	return(result);
}


//-------------------------------------------------------
// For the given cpErr, create a new Exception and send it to env.
// Note: Throw does not throw anything.  It only sets the state.
//		The JVM will check this and throw an exception later.
//-------------------------------------------------------
void throwException(JNIEnv *env, int cpErr)
{
	jclass jexceptionClass;
	jthrowable jexceptionObj;
	jmethodID jconstructorID;
	const char* exClassName;

	if (env->ExceptionOccurred())
	{
		return;
	}

    switch ( cpErr )
    {
    case JLIBICAL_ERR_NETWORK:
            exClassName = "net.cp.jlibical/JLCNetworkException";
	    break;

    case JLIBICAL_ERR_SERVER_INTERNAL:
            exClassName = "net.cp.jlibical/JLCServerInternalException";
	    break;

    case JLIBICAL_ERR_CLIENT_INTERNAL:
            exClassName = "net.cp.jlibical/JLCClientInternalException";
	    break;

    case JLIBICAL_ERR_ILLEGAL_ARGUMENT:
            exClassName = "net.cp.jlibical/JLCIllegalArgumentException";
	    break;

    case JLIBICAL_ERR_API_NOT_INITED:
            exClassName = "net.cp.jlibical/JLCNotInitedException";
	    break;

    case JLIBICAL_ERR_HOST_INVALID:
            exClassName = "net.cp.jlibical/JLCHostInvalidException";
	    break;

    default:
            exClassName = "net.cp.jlibical/JLCClientInternalException";
	    printf("*** JLIBICAL JNI throwException: unknown error code: %d\n", cpErr );
	    break;
    }

    env->ThrowNew(env->FindClass(exClassName),"");
}
