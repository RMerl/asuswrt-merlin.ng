/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Freescale Semiconductor
 *
 * Extracted from gic_64.S
 */

#include <config.h>
#include <linux/linkage.h>
#include <asm/macro.h>

/*************************************************************************
 *
 * void ccn504_add_masters_to_dvm(CCI_MN_BASE, CCI_MN_RNF_NODEID_LIST,
 *				  CCI_MN_DVM_DOMAIN_CTL_SET);
 *
 * Add fully-coherent masters to DVM domain
 *
 *************************************************************************/
ENTRY(ccn504_add_masters_to_dvm)
	/*
	 * x0: CCI_MN_BASE
	 * x1: CCI_MN_RNF_NODEID_LIST
	 * x2: CCI_MN_DVM_DOMAIN_CTL_SET
	 */

	/* Add fully-coherent masters to DVM domain */
	ldr	x9, [x0, x1]
	str	x9, [x0, x2]
1:	ldr	x10, [x0, x2]
	mvn	x11, x10
	tst	x11, x10 /* Wait for domain addition to complete */
	b.ne	1b

	ret
ENDPROC(ccn504_add_masters_to_dvm)

/*************************************************************************
 *
 * void ccn504_set_qos(CCI_Sx_QOS_CONTROL_BASE, QoS Value);
 *
 * Initialize QoS settings for AR/AW override.
 * Right now, this function sets the same QoS value for all RN-I ports
 *
 *************************************************************************/
ENTRY(ccn504_set_qos)
	/*
	 * x0: CCI_Sx_QOS_CONTROL_BASE
	 * x1: QoS Value
	 */

	/* Set all RN-I ports to QoS value denoted by x1 */
	ldr	x9, [x0]
	mov	x10, x1
	orr	x9, x9, x10
	str	x9, [x0]

	ret
ENDPROC(ccn504_set_qos)

/*************************************************************************
 *
 * void ccn504_set_aux(CCI_AUX_CONTROL_BASE, Value);
 *
 * Initialize AUX control settings
 *
 *************************************************************************/
ENTRY(ccn504_set_aux)
	/*
	 * x0: CCI_AUX_CONTROL_BASE
	 * x1: Value
	 */

	ldr	x9, [x0]
	mov	x10, x1
	orr	x9, x9, x10
	str	x9, [x0]

	ret
ENDPROC(ccn504_set_aux)
