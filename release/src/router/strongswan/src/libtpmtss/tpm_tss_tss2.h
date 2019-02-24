/*
 * Copyright (C) 2016-2018 Andreas Steffen
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
 * @defgroup tpm_tss_tss2 tpm_tss_tss2
 * @{ @ingroup libtpmtss
 */

#ifndef TPM_TSS_TSS2_H_
#define TPM_TSS_TSS2_H_

#include "tpm_tss.h"

/**
 * Create a tpm_tss_tss2 instance.
 */
tpm_tss_t *tpm_tss_tss2_create(void);

/**
 * Initialize the tpm_tss_tss2 library.
 *
 * @return		TRUE if initialization was successful
 */
bool tpm_tss_tss2_init(void);

/**
 * /De-initialize the tpm_tss_tss2 library.
 */
void tpm_tss_tss2_deinit(void);

#endif /** TPM_TSS_TSS2_H_ @}*/
