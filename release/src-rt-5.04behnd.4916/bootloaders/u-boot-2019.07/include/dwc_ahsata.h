/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __DWC_AHSATA_H__
#define __DWC_AHSATA_H__

int dwc_ahsata_bus_reset(struct udevice *dev);
int dwc_ahsata_probe(struct udevice *dev);
int dwc_ahsata_scan(struct udevice *dev);
int dwc_ahsata_port_status(struct udevice *dev, int port);

#endif
