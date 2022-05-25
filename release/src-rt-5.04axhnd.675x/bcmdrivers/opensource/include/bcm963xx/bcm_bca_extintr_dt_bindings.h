/*
* <:copyright-BRCM:2019:DUAL/GPL:standard
* 
*    Copyright (c) 2019 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
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
