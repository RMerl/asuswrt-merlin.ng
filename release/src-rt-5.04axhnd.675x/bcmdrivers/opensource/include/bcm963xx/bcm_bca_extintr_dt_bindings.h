/*
* <:copyright-BRCM:2019:DUAL/GPL:standard
* 
*    Copyright (c) 2019 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :> 
*/

#ifndef BCM_BCA_EXTINTR_DT_BINDINGS_H
#define BCM_BCA_EXTINTR_DT_BINDINGS_H

#define BCA_EXTINTR_TYPE_LEVEL_SHIFT        1 
#define BCA_EXTINTR_TYPE_LEVEL_MASK         (0x1 << BCA_EXTINTR_TYPE_LEVEL_SHIFT)
#define BCA_EXTINTR_TYPE_LOW_LEVEL          (0x0 << BCA_EXTINTR_TYPE_LEVEL_SHIFT)
#define BCA_EXTINTR_TYPE_HIGH_LEVEL         (0x1 << BCA_EXTINTR_TYPE_LEVEL_SHIFT)
#define BCA_EXTINTR_TYPE_SENSE_SHIFT        2
#define BCA_EXTINTR_TYPE_SENSE_MASK         (0x1 << BCA_EXTINTR_TYPE_SENSE_SHIFT)
#define BCA_EXTINTR_TYPE_SENSE_LEVEL        (0x0 << BCA_EXTINTR_TYPE_SENSE_SHIFT)
#define BCA_EXTINTR_TYPE_SENSE_EDGE         (0x1 << BCA_EXTINTR_TYPE_SENSE_SHIFT)
#define BCA_EXTINTR_TYPE_BOTH_EDGE_SHIFT    3
#define BCA_EXTINTR_TYPE_BOTH_EDGE_MASK     (0x1 << BCA_EXTINTR_TYPE_BOTH_EDGE_SHIFT)
#define BCA_EXTINTR_TYPE_BOTH_EDGE          (0x1 << BCA_EXTINTR_TYPE_BOTH_EDGE_SHIFT)
#define BCA_EXTINTR_SHARE_SHIFT             4
#define BCA_EXTINTR_SHARE_MASK              (0x1 << BCA_EXTINTR_SHARE_SHIFT)
#define BCA_EXTINTR_NOT_SHARED              (0x0 << BCA_EXTINTR_SHARE_SHIFT)
#define BCA_EXTINTR_SHARED                  (0x1 << BCA_EXTINTR_SHARE_SHIFT)

#define BCA_GPIO_ACTIVE_HIGH                (0x0 << 1)
#define BCA_GPIO_ACTIVE_LOW                 (0x1 << 1)

#endif
