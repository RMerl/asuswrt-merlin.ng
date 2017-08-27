/*
 * Copyright (C) 2013 Andreas Steffen
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

/**
 *
 * @defgroup imv_workitem_t imv_workitem
 * @{ @ingroup libimcv_imv
 */

#ifndef IMV_WORKITEM_H_
#define IMV_WORKITEM_H_

#include <tncifimv.h>

#include <library.h>

typedef struct imv_workitem_t imv_workitem_t;
typedef enum imv_workitem_type_t imv_workitem_type_t;

enum imv_workitem_type_t {
	IMV_WORKITEM_PACKAGES =        1,
	IMV_WORKITEM_UNKNOWN_SOURCE =  2,
	IMV_WORKITEM_FORWARDING =      3,
	IMV_WORKITEM_DEFAULT_PWD =     4,
	IMV_WORKITEM_FILE_REF_MEAS =   5,
	IMV_WORKITEM_FILE_MEAS =       6,
	IMV_WORKITEM_FILE_META =       7,
	IMV_WORKITEM_DIR_REF_MEAS =    8,
	IMV_WORKITEM_DIR_MEAS =        9,
	IMV_WORKITEM_DIR_META =       10,
	IMV_WORKITEM_TCP_PORT_OPEN =  11,
	IMV_WORKITEM_TCP_PORT_BLOCK = 12,
	IMV_WORKITEM_UDP_PORT_OPEN =  13,
	IMV_WORKITEM_UDP_PORT_BLOCK = 14,
	IMV_WORKITEM_SWID_TAGS =      15,
	IMV_WORKITEM_TPM_ATTEST =     16
};

extern enum_name_t *imv_workitem_type_names;

/**
 * IMV database interface 
 */
struct imv_workitem_t {

	/**
	 * Get primary workitem key 
	 *
	 * @return				Primary workitem key
	 */
	 int (*get_id)(imv_workitem_t *this);

	/**
	 * Get workitem type
	 *
	 * @return				Workitem type
	 */
	 imv_workitem_type_t (*get_type)(imv_workitem_t *this);

	/**
	 * Set IMV ID
	 *
	 * @param id			IMV ID
	 */
	 void (*set_imv_id)(imv_workitem_t *this, TNC_IMVID imv_id);

	/**
	 * Get IMV ID
	 *
	 * @return				IMV ID
	 */
	 TNC_IMVID (*get_imv_id)(imv_workitem_t *this);

	/**
	 * Get string argument
	 *
	 * @return				Argument string
	 */
	 char* (*get_arg_str)(imv_workitem_t *this);

	/**
	 * Get integer argument
	 *
	 * @return				Argument integer
	 */
	 int (*get_arg_int)(imv_workitem_t *this);

	/**
	 * Set result string
	 *
	 * @param result		Result string
	 * @param eval			Evaluation Result
	 * @return				Action Recommendation
	 */
	 TNC_IMV_Action_Recommendation (*set_result)(imv_workitem_t *this, 
						char *result, TNC_IMV_Evaluation_Result eval);

	/**
	 * Set result string
	 *
	 * @param result		Result string
	 * @return				Action Recommendatino
	 */
	 TNC_IMV_Action_Recommendation (*get_result)(imv_workitem_t *this, 
												 char **result);

	/**
	 * Destroys an imv_workitem_t object
	 */
	void (*destroy)(imv_workitem_t *this);
};

/**
 * Create an imv_workitem_t instance
 *
 * @param id				Primary workitem key
 * @param type				Workitem type
 * @param arg_str			String argument
 * @param arg_int			Integer argument
 * @param rec_fail			Recommendation with minor/major non-compliance case
 * @param rec_noresult		Recommendation in don't know/error case
 */
imv_workitem_t *imv_workitem_create(int id, imv_workitem_type_t type,
									char *arg_str, int arg_int,
									TNC_IMV_Action_Recommendation rec_fail,
									TNC_IMV_Action_Recommendation rec_noresult);

#endif /** IMV_WORKITEM_H_ @}*/
