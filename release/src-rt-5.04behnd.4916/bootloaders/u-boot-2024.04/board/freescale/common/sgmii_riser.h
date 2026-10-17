/*
 * Freescale SGMII Riser Card
 *
 * This driver supports the SGMII Riser card found on the
 * "DS" style of development board from Freescale.
 *
 * This software may be used and distributed according to the
 * terms of the GNU Public License, Version 2, incorporated
 * herein by reference.
 *
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 */

void fsl_sgmii_riser_init(struct tsec_info_struct *tsec_info, int num);
void fsl_sgmii_riser_fdt_fixup(void *fdt);
