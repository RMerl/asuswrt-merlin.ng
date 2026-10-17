/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 Dario Binacchi <dariobin@libero.it>
 */

#ifndef _TILCDC_PANEL_H
#define _TILCDC_PANEL_H

#include "tilcdc.h"

int tilcdc_panel_get_display_info(struct udevice *dev,
				  struct tilcdc_panel_info *info);

#endif /* _TILCDC_PANEL_H */
