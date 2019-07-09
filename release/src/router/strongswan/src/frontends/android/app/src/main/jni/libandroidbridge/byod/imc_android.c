/*
 * Copyright (C) 2012-2013 Tobias Brunner
 * Copyright (C) 2012 Christoph Buehler
 * Copyright (C) 2012 Patrick Loetscher
 * Copyright (C) 2011-2012 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "imc_android_state.h"
#include "../android_jni.h"
#include "../charonservice.h"

#include <tnc/tnc.h>
#include <imcv.h>
#include <imc/imc_agent.h>
#include <imc/imc_msg.h>
#include <pa_tnc/pa_tnc_msg.h>
#include <ietf/ietf_attr.h>
#include <ietf/ietf_attr_attr_request.h>
#include <ietf/ietf_attr_installed_packages.h>
#include <ietf/ietf_attr_pa_tnc_error.h>
#include <ietf/ietf_attr_product_info.h>
#include <ietf/ietf_attr_remediation_instr.h>
#include <ietf/ietf_attr_string_version.h>
#include <ita/ita_attr.h>
#include <ita/ita_attr_get_settings.h>
#include <tcg/pts/tcg_pts_attr_file_meas.h>
#include <tcg/pts/tcg_pts_attr_meas_algo.h>
#include <tcg/pts/tcg_pts_attr_proto_caps.h>
#include <tcg/pts/tcg_pts_attr_req_file_meas.h>
#include <os_info/os_info.h>

#include <tncif_pa_subtypes.h>

#include <pen/pen.h>
#include <utils/debug.h>

#include <stdio.h>

/* IMC definitions */

static const char imc_name[] = "Android";

static pen_type_t msg_types[] = {
	{ PEN_IETF, PA_SUBTYPE_IETF_OPERATING_SYSTEM },
	{ PEN_IETF, PA_SUBTYPE_IETF_VPN },
	{ PEN_TCG, PA_SUBTYPE_TCG_PTS },
};

static imc_agent_t *imc_android;

/**
 * AndroidImc object accessed via JNI
 */
static jobject android_imc;

/**
 * AndroidImc class object
 */
static jclass android_imc_cls;

/**
 * see section 3.8.1 of TCG TNC IF-IMC Specification 1.3
 */
static TNC_Result tnc_imc_initialize(TNC_IMCID imc_id,
									 TNC_Version min_version,
									 TNC_Version max_version,
									 TNC_Version *actual_version)
{
	if (imc_android)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has already been initialized", imc_name);
		return TNC_RESULT_ALREADY_INITIALIZED;
	}
	imc_android = imc_agent_create(imc_name, msg_types, countof(msg_types),
								   imc_id, actual_version);
	if (!imc_android)
	{
		return TNC_RESULT_FATAL;
	}

	if (min_version > TNC_IFIMC_VERSION_1 || max_version < TNC_IFIMC_VERSION_1)
	{
		DBG1(DBG_IMC, "no common IF-IMC version");
		return TNC_RESULT_NO_COMMON_VERSION;
	}
	return TNC_RESULT_SUCCESS;
}

/**
 * Update the state in the GUI.
 */
static void update_imc_state(TNC_ConnectionState state)
{
	android_imc_state_t imc_state = ANDROID_IMC_STATE_UNKNOWN;

	switch (state)
	{	/* map connection states to the values used by the GUI */
		case TNC_CONNECTION_STATE_ACCESS_ALLOWED:
			imc_state = ANDROID_IMC_STATE_ALLOW;
			break;
		case TNC_CONNECTION_STATE_ACCESS_ISOLATED:
			imc_state = ANDROID_IMC_STATE_ISOLATE;
			break;
		case TNC_CONNECTION_STATE_ACCESS_NONE:
			imc_state = ANDROID_IMC_STATE_BLOCK;
			break;
	}

	charonservice->update_imc_state(charonservice, imc_state);
}

/**
 * see section 3.8.2 of TCG TNC IF-IMC Specification 1.3
 */
static TNC_Result tnc_imc_notifyconnectionchange(TNC_IMCID imc_id,
												 TNC_ConnectionID connection_id,
												 TNC_ConnectionState new_state)
{
	imc_state_t *state;

	if (!imc_android)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	switch (new_state)
	{
		case TNC_CONNECTION_STATE_CREATE:
			state = imc_android_state_create(connection_id);
			return imc_android->create_state(imc_android, state);
		case TNC_CONNECTION_STATE_HANDSHAKE:
			if (imc_android->change_state(imc_android, connection_id, new_state,
										  &state) != TNC_RESULT_SUCCESS)
			{
				return TNC_RESULT_FATAL;
			}
			state->set_result(state, imc_id,
							  TNC_IMV_EVALUATION_RESULT_DONT_KNOW);
			return TNC_RESULT_SUCCESS;
		case TNC_CONNECTION_STATE_DELETE:
			return imc_android->delete_state(imc_android, connection_id);
		case TNC_CONNECTION_STATE_ACCESS_ALLOWED:
		case TNC_CONNECTION_STATE_ACCESS_ISOLATED:
		case TNC_CONNECTION_STATE_ACCESS_NONE:
			update_imc_state(new_state);
			/* fall-through */
		default:
			return imc_android->change_state(imc_android, connection_id,
											 new_state, NULL);
	}
}

/**
 * Convert the native C strings in the enumerator to a Java String array.
 * The given enumerator gets destroyed.
 */
static jobjectArray string_array_create(JNIEnv *env, enumerator_t *enumerator)
{
	linked_list_t *list;
	jobjectArray jarray;
	jstring jstring;
	char *native;
	jclass cls;
	int i = 0;

	cls = (*env)->FindClass(env, "java/lang/String");
	list = linked_list_create_from_enumerator(enumerator);
	jarray = (*env)->NewObjectArray(env, list->get_count(list), cls, NULL);
	if (!jarray)
	{
		goto failed;
	}
	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, (void**)&native))
	{
		jstring = (*env)->NewStringUTF(env, native);
		if (!jstring)
		{
			enumerator->destroy(enumerator);
			goto failed;
		}
		(*env)->SetObjectArrayElement(env, jarray, i++, jstring);
	}
	enumerator->destroy(enumerator);
	list->destroy(list);
	return jarray;

failed:
	androidjni_exception_occurred(env);
	list->destroy(list);
	return NULL;
}

/**
 * Get a measurement for the given attribute type from the Android IMC.
 * NULL is returned if no measurement is available or an error occurred.
 *
 * The optional args is an enumerator over char* (gets destroyed).
 */
static pa_tnc_attr_t *get_measurement(pen_type_t attr_type, enumerator_t *args)
{
	JNIEnv *env;
	pa_tnc_attr_t *attr;
	jmethodID method_id;
	jbyteArray jmeasurement;
	jobjectArray jargs = NULL;
	chunk_t data;

	androidjni_attach_thread(&env);
	if (args)
	{
		jargs = string_array_create(env, args);
		if (!jargs)
		{
			goto failed;
		}
		method_id = (*env)->GetMethodID(env, android_imc_cls, "getMeasurement",
										"(II[Ljava/lang/String;)[B");
	}
	else
	{
		method_id = (*env)->GetMethodID(env, android_imc_cls, "getMeasurement",
										"(II)[B");
	}
	if (!method_id)
	{
		goto failed;
	}
	jmeasurement = (*env)->CallObjectMethod(env, android_imc, method_id,
											attr_type.vendor_id, attr_type.type,
											jargs);
	if (!jmeasurement || androidjni_exception_occurred(env))
	{
		goto failed;
	}
	data = chunk_create((*env)->GetByteArrayElements(env, jmeasurement, NULL),
						(*env)->GetArrayLength(env, jmeasurement));
	if (!data.ptr)
	{
		goto failed;
	}
	attr = imcv_pa_tnc_attributes->construct(imcv_pa_tnc_attributes,
									attr_type.vendor_id, attr_type.type, data);
	(*env)->ReleaseByteArrayElements(env, jmeasurement, data.ptr, JNI_ABORT);
	androidjni_detach_thread();
	return attr;

failed:
	androidjni_exception_occurred(env);
	androidjni_detach_thread();
	return NULL;
}

/**
 * Add the measurement for the requested attribute type with optional
 * arguments (enumerator over char*, gets destroyed).
 */
static void add_measurement(pen_type_t attr_type, imc_msg_t *msg,
							enumerator_t *args)
{
	pa_tnc_attr_t *attr;
	enum_name_t *pa_attr_names;

	attr = get_measurement(attr_type, args);
	if (attr)
	{
		msg->add_attribute(msg, attr);
		return;
	}
	pa_attr_names = imcv_pa_tnc_attributes->get_names(imcv_pa_tnc_attributes,
													  attr_type.vendor_id);
	if (pa_attr_names)
	{
		DBG1(DBG_IMC, "no measurement available for PA-TNC attribute type "
			 "'%N/%N' 0x%06x/0x%08x", pen_names, attr_type.vendor_id,
			 pa_attr_names, attr_type.type, attr_type.vendor_id, attr_type.type);
	}
	else
	{
		DBG1(DBG_IMC, "no measurement available for PA-TNC attribute type '%N' "
			 "0x%06x/0x%08x", pen_names, attr_type.vendor_id,
			 attr_type.vendor_id, attr_type.type);
	}
}

/**
 * Handle an IETF attribute
 */
static void handle_ietf_attribute(pen_type_t attr_type, pa_tnc_attr_t *attr,
								  imc_msg_t *out_msg)
{
	if (attr_type.type == IETF_ATTR_ATTRIBUTE_REQUEST)
	{
		ietf_attr_attr_request_t *attr_cast;
		pen_type_t *entry;
		enumerator_t *enumerator;

		attr_cast = (ietf_attr_attr_request_t*)attr;
		enumerator = attr_cast->create_enumerator(attr_cast);
		while (enumerator->enumerate(enumerator, &entry))
		{
			add_measurement(*entry, out_msg, NULL);
		}
		enumerator->destroy(enumerator);
	}
	else if (attr_type.type == IETF_ATTR_REMEDIATION_INSTRUCTIONS)
	{
		ietf_attr_remediation_instr_t *attr_cast;
		pen_type_t param;
		chunk_t str;
		char *instr;

		attr_cast = (ietf_attr_remediation_instr_t*)attr;
		param = attr_cast->get_parameters_type(attr_cast);

		if (pen_type_is(param, PEN_IETF, IETF_REMEDIATION_PARAMETERS_STRING))
		{
			str = attr_cast->get_string(attr_cast, NULL);
			instr = strndup(str.ptr, str.len);
			charonservice->add_remediation_instr(charonservice, instr);
			free (instr);
		}
	}
}

/**
 * Handle an ITA attribute
 */
static void handle_ita_attribute(pen_type_t attr_type, pa_tnc_attr_t *attr,
								 imc_msg_t *out_msg)
{
	if (attr_type.type == ITA_ATTR_GET_SETTINGS)
	{
		ita_attr_get_settings_t *attr_cast;

		attr_cast = (ita_attr_get_settings_t*)attr;
		add_measurement((pen_type_t){ PEN_ITA, ITA_ATTR_SETTINGS },
						out_msg, attr_cast->create_enumerator(attr_cast));
	}
}

/**
 * Handle a TCG attribute
 */
static void handle_tcg_attribute(imc_android_state_t *state,
								 pen_type_t attr_type, pa_tnc_attr_t *attr,
								 imc_msg_t *out_msg)
{
	pts_t *pts;

	pts = state->get_pts(state);
	switch (attr_type.type)
	{
		case TCG_PTS_REQ_PROTO_CAPS:
		{
			tcg_pts_attr_proto_caps_t *attr_cast;
			pts_proto_caps_flag_t caps;

			attr_cast = (tcg_pts_attr_proto_caps_t*)attr;
			caps = attr_cast->get_flags(attr_cast) & pts->get_proto_caps(pts);
			pts->set_proto_caps(pts, caps);
			attr = tcg_pts_attr_proto_caps_create(caps, FALSE);
			out_msg->add_attribute(out_msg, attr);
			break;
		}
		case TCG_PTS_MEAS_ALGO:
		{
			tcg_pts_attr_meas_algo_t *attr_cast;
			pts_meas_algorithms_t supported, algo;

			if (!pts_meas_algo_probe(&supported))
			{
				attr = pts_hash_alg_error_create(PTS_MEAS_ALGO_NONE);
				out_msg->add_attribute(out_msg, attr);
				break;
			}
			attr_cast = (tcg_pts_attr_meas_algo_t*)attr;
			algo = pts_meas_algo_select(supported,
										attr_cast->get_algorithms(attr_cast));
			if (algo == PTS_MEAS_ALGO_NONE)
			{
				attr = pts_hash_alg_error_create(supported);
				out_msg->add_attribute(out_msg, attr);
				break;
			}
			pts->set_meas_algorithm(pts, algo);
			attr = tcg_pts_attr_meas_algo_create(algo, TRUE);
			out_msg->add_attribute(out_msg, attr);
			break;
		}
		case TCG_PTS_REQ_FILE_MEAS:
		{
			tcg_pts_attr_req_file_meas_t *attr_cast;
			pts_file_meas_t *measurements;
			pts_error_code_t pts_error;
			uint32_t delim;
			uint16_t req_id;
			bool is_dir;
			char *path;

			attr_cast = (tcg_pts_attr_req_file_meas_t*)attr;
			path = attr_cast->get_pathname(attr_cast);
			if (!pts->is_path_valid(pts, path, &pts_error))
			{	/* silently ignore internal errors */
				break;
			}
			else if (pts_error)
			{
				attr = ietf_attr_pa_tnc_error_create(pen_type_create(PEN_TCG,
											pts_error), attr->get_value(attr));
				out_msg->add_attribute(out_msg, attr);
				break;
			}
			delim = attr_cast->get_delimiter(attr_cast);
			if (delim != SOLIDUS_UTF && delim != REVERSE_SOLIDUS_UTF)
			{
				attr = ietf_attr_pa_tnc_error_create(pen_type_create(PEN_TCG,
							TCG_PTS_INVALID_DELIMITER), attr->get_value(attr));
				out_msg->add_attribute(out_msg, attr);
				break;
			}
			req_id = attr_cast->get_request_id(attr_cast);
			is_dir = attr_cast->get_directory_flag(attr_cast);

			DBG1(DBG_IMC, "measurement request %d for %s '%s'", req_id,
				 is_dir ? "directory" : "file", path);
			measurements = pts_file_meas_create_from_path(req_id, path, is_dir,
											TRUE, pts->get_meas_algorithm(pts));
			if (!measurements)
			{
				attr = ietf_attr_pa_tnc_error_create(pen_type_create(PEN_TCG,
								TCG_PTS_FILE_NOT_FOUND), attr->get_value(attr));
				out_msg->add_attribute(out_msg, attr);
				break;
			}
			attr = tcg_pts_attr_file_meas_create(measurements);
			attr->set_noskip_flag(attr, TRUE);
			out_msg->add_attribute(out_msg, attr);
			break;
		}
		default:
			DBG1(DBG_IMC, "received unsupported TCG attribute '%N'",
				 tcg_attr_names, attr_type.type);
			break;
	}
}

/**
 * see section 3.8.3 of TCG TNC IF-IMC Specification 1.3
 */
static TNC_Result tnc_imc_beginhandshake(TNC_IMCID imc_id,
										 TNC_ConnectionID connection_id)
{
	imc_state_t *state;
	imc_msg_t *out_msg;
	TNC_Result result = TNC_RESULT_SUCCESS;

	if (!imc_android)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	if (!imc_android->get_state(imc_android, connection_id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	if (lib->settings->get_bool(lib->settings,
								"android.imc.send_os_info", TRUE))
	{
		out_msg = imc_msg_create(imc_android, state, connection_id, imc_id,
								 TNC_IMVID_ANY, msg_types[0]);
		add_measurement((pen_type_t){ PEN_IETF, IETF_ATTR_PRODUCT_INFORMATION },
						out_msg, NULL);
		add_measurement((pen_type_t){ PEN_IETF, IETF_ATTR_STRING_VERSION },
						out_msg, NULL);
		add_measurement((pen_type_t){ PEN_ITA, ITA_ATTR_DEVICE_ID },
						out_msg, NULL);
		/* send PA-TNC message with the excl flag not set */
		result = out_msg->send(out_msg, FALSE);
		out_msg->destroy(out_msg);
	}

	return result;
}

static TNC_Result receive_message(imc_android_state_t *state, imc_msg_t *in_msg)
{
	imc_msg_t *out_msg;
	enumerator_t *enumerator;
	pa_tnc_attr_t *attr;
	pen_type_t attr_type;
	TNC_Result result;
	bool fatal_error = FALSE;

	out_msg = imc_msg_create_as_reply(in_msg);

	/* parse received PA-TNC message and handle local and remote errors */
	result = in_msg->receive(in_msg, out_msg, &fatal_error);
	if (result != TNC_RESULT_SUCCESS)
	{
		out_msg->destroy(out_msg);
		return result;
	}

	/* analyze PA-TNC attributes */
	enumerator = in_msg->create_attribute_enumerator(in_msg);
	while (enumerator->enumerate(enumerator, &attr))
	{
		attr_type = attr->get_type(attr);

		switch (attr_type.vendor_id)
		{
			case PEN_IETF:
				handle_ietf_attribute(attr_type, attr, out_msg);
				continue;
			case PEN_ITA:
				handle_ita_attribute(attr_type, attr, out_msg);
				continue;
			case PEN_TCG:
				handle_tcg_attribute(state, attr_type, attr, out_msg);
				continue;
			default:
				continue;
		}
	}
	enumerator->destroy(enumerator);

	if (fatal_error)
	{
		result = TNC_RESULT_FATAL;
	}
	else
	{
		result = out_msg->send(out_msg, TRUE);
	}
	out_msg->destroy(out_msg);

	return result;
}

/**
 * see section 3.8.4 of TCG TNC IF-IMC Specification 1.3

 */
static TNC_Result tnc_imc_receivemessage(TNC_IMCID imc_id,
										 TNC_ConnectionID connection_id,
										 TNC_BufferReference msg,
										 TNC_UInt32 msg_len,
										 TNC_MessageType msg_type)
{
	imc_state_t *state;
	imc_msg_t *in_msg;
	TNC_Result result;

	if (!imc_android)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	if (!imc_android->get_state(imc_android, connection_id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	in_msg = imc_msg_create_from_data(imc_android, state, connection_id,
									  msg_type, chunk_create(msg, msg_len));
	result = receive_message((imc_android_state_t*)state, in_msg);
	in_msg->destroy(in_msg);

	return result;
}

/**
 * see section 3.8.6 of TCG TNC IF-IMV Specification 1.3
 */
static TNC_Result tnc_imc_receivemessagelong(TNC_IMCID imc_id,
											 TNC_ConnectionID connection_id,
											 TNC_UInt32 msg_flags,
											 TNC_BufferReference msg,
											 TNC_UInt32 msg_len,
											 TNC_VendorID msg_vid,
											 TNC_MessageSubtype msg_subtype,
											 TNC_UInt32 src_imv_id,
											 TNC_UInt32 dst_imc_id)
{
	imc_state_t *state;
	imc_msg_t *in_msg;
	TNC_Result result;

	if (!imc_android)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	if (!imc_android->get_state(imc_android, connection_id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	in_msg = imc_msg_create_from_long_data(imc_android, state, connection_id,
								src_imv_id, dst_imc_id,msg_vid, msg_subtype,
								chunk_create(msg, msg_len));
	result = receive_message((imc_android_state_t*)state, in_msg);
	in_msg->destroy(in_msg);

	return result;
}

/**
 * see section 3.8.7 of TCG TNC IF-IMC Specification 1.3
 */
static TNC_Result tnc_imc_batchending(TNC_IMCID imc_id,
									  TNC_ConnectionID connection_id)
{
	if (!imc_android)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	return TNC_RESULT_SUCCESS;
}

/**
 * see section 3.8.8 of TCG TNC IF-IMC Specification 1.3
 */
static TNC_Result tnc_imc_terminate(TNC_IMCID imc_id)
{
	if (!imc_android)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	imc_android->destroy(imc_android);
	imc_android = NULL;
	return TNC_RESULT_SUCCESS;
}

/**
 * see section 4.2.8.1 of TCG TNC IF-IMC Specification 1.3
 */
static TNC_Result tnc_imc_providebindfunction(TNC_IMCID imc_id,
											  TNC_TNCC_BindFunctionPointer bind_function)
{
	if (!imc_android)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	return imc_android->bind_functions(imc_android, bind_function);
}

/*
 * Described in header
 */
bool imc_android_register(plugin_t *plugin, plugin_feature_t *feature,
						  bool reg, void *data)
{
	JNIEnv *env;
	jmethodID method_id;
	jobject obj, context = (jobject)data;
	jclass cls;
	bool success = TRUE;

	androidjni_attach_thread(&env);
	if (reg)
	{
		cls = (*env)->FindClass(env, JNI_PACKAGE_STRING "/imc/AndroidImc");
		if (!cls)
		{
			goto failed;
		}
		android_imc_cls = (*env)->NewGlobalRef(env, cls);
		method_id = (*env)->GetMethodID(env, cls, "<init>",
										"(Landroid/content/Context;)V");
		if (!method_id)
		{
			goto failed;
		}
		obj = (*env)->NewObject(env, cls, method_id, context);
		if (!obj)
		{
			goto failed;
		}
		android_imc = (*env)->NewGlobalRef(env, obj);
		androidjni_detach_thread();

		if (tnc->imcs->load_from_functions(tnc->imcs, "Android",
							tnc_imc_initialize, tnc_imc_notifyconnectionchange,
							tnc_imc_beginhandshake, tnc_imc_receivemessage,
							tnc_imc_receivemessagelong, tnc_imc_batchending,
							tnc_imc_terminate, tnc_imc_providebindfunction))
		{
			return TRUE;
		}
failed:
		DBG1(DBG_IMC, "initialization of Android IMC failed");
		androidjni_exception_occurred(env);
		success = FALSE;
	}

	if (android_imc)
	{
		(*env)->DeleteGlobalRef(env, android_imc);
		android_imc = NULL;
	}
	if (android_imc_cls)
	{
		(*env)->DeleteGlobalRef(env, android_imc_cls);
		android_imc_cls = NULL;
	}
	androidjni_detach_thread();
	return success;
}
