/*
 * Copyright (C) 2010 Andreas Steffen
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
 * @defgroup tnc_imv_t tnc_imv
 * @{ @ingroup tnc_imv
 */

#ifndef TNC_IMV_H_
#define TNC_IMV_H_

#include <tnc/imv/imv.h>

/**
 * Create an Integrity Measurement Verifier loaded from a library.
 *
 * @param name			name of the IMV
 * @param filename		path to the dynamic IMV library
 * @return				instance of the imv_t interface
 */
imv_t* tnc_imv_create(char *name, char *filename);

/**
 * Create an Integrity Measurement Verifier from a set of IMV functions.
 *
 * @param name						name of the IMV
 * @param initialize				TNC_IMV_InitializePointer
 * @param notify_connection_change	TNC_IMV_NotifyConnectionChangePointer
 * @param receive_message			TNC_IMV_ReceiveMessagePointer
 * @param receive_message_long		TNC_IMV_ReceiveMessageLongPointer
 * @param solicit_recommendation	TNC_IMV_SolicitRecommendationPointer
 * @param batch_ending				TNC_IMV_BatchEndingPointer
 * @param terminate					TNC_IMV_TerminatePointer
 * @param provide_bind_function		TNC_IMV_ProvideBindFunctionPointer
 * @return							instance of the imv_t interface
 */
imv_t* tnc_imv_create_from_functions(char *name,
				TNC_IMV_InitializePointer initialize,
				TNC_IMV_NotifyConnectionChangePointer notify_connection_change,
				TNC_IMV_ReceiveMessagePointer receive_message,
				TNC_IMV_ReceiveMessageLongPointer receive_message_long,
				TNC_IMV_SolicitRecommendationPointer solicit_recommendation,
				TNC_IMV_BatchEndingPointer batch_ending,
				TNC_IMV_TerminatePointer terminate,
				TNC_IMV_ProvideBindFunctionPointer provide_bind_function);

#endif /** TNC_IMV_H_ @}*/
