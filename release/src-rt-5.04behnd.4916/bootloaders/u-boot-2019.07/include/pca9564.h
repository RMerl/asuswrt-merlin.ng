/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * File:         include/pca9564.h
 * Author:
 *
 * Created:      2009-06-23
 * Description:  PCA9564 i2c bridge driver
 *
 * Modified:
 *               Copyright 2009 CJSC "NII STT", http://www.niistt.ru/
 *
 * Bugs:
 */

#ifndef _PCA9564_H
#define _PCA9564_H

/* Clock speeds for the bus */
#define PCA_CON_330kHz      0x00
#define PCA_CON_288kHz      0x01
#define PCA_CON_217kHz      0x02
#define PCA_CON_146kHz      0x03
#define PCA_CON_88kHz       0x04
#define PCA_CON_59kHz       0x05
#define PCA_CON_44kHz       0x06
#define PCA_CON_36kHz       0x07

#define PCA_CON_AA          0x80 /* Assert Acknowledge */
#define PCA_CON_ENSIO       0x40 /* Enable */
#define PCA_CON_STA         0x20 /* Start */
#define PCA_CON_STO         0x10 /* Stop */
#define PCA_CON_SI          0x08 /* Serial Interrupt */
#define PCA_CON_CR          0x07 /* Clock Rate (MASK) */

#endif
