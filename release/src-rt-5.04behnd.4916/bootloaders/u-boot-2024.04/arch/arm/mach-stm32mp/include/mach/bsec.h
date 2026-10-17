/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright (C) 2020, STMicroelectronics - All Rights Reserved
 */

/* check self hosted debug status = BSEC_DENABLE.DBGSWENABLE */
bool bsec_dbgswenable(void);

/* Bitfield definition for LOCK status */
#define BSEC_LOCK_PERM			BIT(30)
#define BSEC_LOCK_SHADOW_R		BIT(29)
#define BSEC_LOCK_SHADOW_W		BIT(28)
#define BSEC_LOCK_SHADOW_P		BIT(27)
#define BSEC_LOCK_ERROR			BIT(26)
