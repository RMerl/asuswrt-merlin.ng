/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#ifndef __KEYPROGRAM_H
#define __KEYPROGRAM_H

int load_and_run_keyprog(struct udevice *tpm);
int flush_keys(struct udevice *tpm);

#endif /* __KEYPROGRAM_H */
