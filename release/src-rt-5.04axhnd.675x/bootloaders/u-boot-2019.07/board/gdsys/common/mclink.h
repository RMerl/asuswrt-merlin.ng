/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 */

#ifndef _MCLINK_H_
#define _MCLINK_H_

int mclink_probe(void);
int mclink_send(u8 slave, u16 addr, u16 data);
int mclink_receive(u8 slave, u16 addr, u16 *data);

#endif
