/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 * Reinhard Pfau, Guntermann & Drunck GmbH, reinhard.pfau@gdsys.cc
 */

#ifndef __CONTROLCENTER_ID_H
#define __CONTROLCENTER_ID_H

int ccdm_compute_self_hash(void);
int startup_ccdm_id_module(void);

int show_self_hash(void);

#endif /* __CONTROLCENTER_ID_H */
