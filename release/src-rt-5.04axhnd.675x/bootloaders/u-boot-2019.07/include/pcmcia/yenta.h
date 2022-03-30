/*
 * yenta.h 1.20 2001/08/24 12:15:34
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License
 * at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and
 * limitations under the License.
 *
 * The initial developer of the original code is David A. Hinds
 * <dahinds@users.sourceforge.net>.  Portions created by David A. Hinds
 * are Copyright (C) 1999 David A. Hinds.  All Rights Reserved.
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU General Public License version 2 (the "GPL"), in
 * which case the provisions of the GPL are applicable instead of the
 * above.  If you wish to allow the use of your version of this file
 * only under the terms of the GPL and not to allow others to use
 * your version of this file under the MPL, indicate your decision by
 * deleting the provisions above and replace them with the notice and
 * other provisions required by the GPL.  If you do not delete the
 * provisions above, a recipient may use your version of this file
 * under either the MPL or the GPL.
 */

#ifndef _LINUX_YENTA_H
#define _LINUX_YENTA_H

/* PCI Configuration Registers */

#define PCI_STATUS_CAPLIST		0x10
#define PCI_CB_CAPABILITY_POINTER	0x14	/* 8 bit */
#define PCI_CAPABILITY_ID		0x00	/* 8 bit */
#define  PCI_CAPABILITY_PM		0x01
#define PCI_NEXT_CAPABILITY		0x01	/* 8 bit */
#define PCI_PM_CAPABILITIES		0x02	/* 16 bit */
#define  PCI_PMCAP_PME_D3COLD		0x8000
#define  PCI_PMCAP_PME_D3HOT		0x4000
#define  PCI_PMCAP_PME_D2		0x2000
#define  PCI_PMCAP_PME_D1		0x1000
#define  PCI_PMCAP_PME_D0		0x0800
#define  PCI_PMCAP_D2_CAP		0x0400
#define  PCI_PMCAP_D1_CAP		0x0200
#define  PCI_PMCAP_DYN_DATA		0x0100
#define  PCI_PMCAP_DSI			0x0020
#define  PCI_PMCAP_AUX_PWR		0x0010
#define  PCI_PMCAP_PMECLK		0x0008
#define  PCI_PMCAP_VERSION_MASK		0x0007
#define PCI_PM_CONTROL_STATUS		0x04	/* 16 bit */
#define  PCI_PMCS_PME_STATUS		0x8000
#define  PCI_PMCS_DATASCALE_MASK	0x6000
#define  PCI_PMCS_DATASCALE_SHIFT	13
#define  PCI_PMCS_DATASEL_MASK		0x1e00
#define  PCI_PMCS_DATASEL_SHIFT		9
#define  PCI_PMCS_PME_ENABLE		0x0100
#define  PCI_PMCS_PWR_STATE_MASK	0x0003
#define  PCI_PMCS_PWR_STATE_D0		0x0000
#define  PCI_PMCS_PWR_STATE_D1		0x0001
#define  PCI_PMCS_PWR_STATE_D2		0x0002
#define  PCI_PMCS_PWR_STATE_D3		0x0003
#define PCI_PM_BRIDGE_EXT		0x06	/* 8 bit */
#define PCI_PM_DATA			0x07	/* 8 bit */

#define CB_PRIMARY_BUS			0x18	/* 8 bit */
#define CB_CARDBUS_BUS			0x19	/* 8 bit */
#define CB_SUBORD_BUS			0x1a	/* 8 bit */
#define CB_LATENCY_TIMER		0x1b	/* 8 bit */

#define CB_MEM_BASE(m)			(0x1c + 8*(m))
#define CB_MEM_LIMIT(m)			(0x20 + 8*(m))
#define CB_IO_BASE(m)			(0x2c + 8*(m))
#define CB_IO_LIMIT(m)			(0x30 + 8*(m))

#define CB_BRIDGE_CONTROL		0x3e	/* 16 bit */
#define  CB_BCR_PARITY_ENA		0x0001
#define  CB_BCR_SERR_ENA		0x0002
#define  CB_BCR_ISA_ENA			0x0004
#define  CB_BCR_VGA_ENA			0x0008
#define  CB_BCR_MABORT			0x0020
#define  CB_BCR_CB_RESET		0x0040
#define  CB_BCR_ISA_IRQ			0x0080
#define  CB_BCR_PREFETCH(m)		(0x0100 << (m))
#define  CB_BCR_WRITE_POST		0x0400

#define CB_LEGACY_MODE_BASE		0x44

/* Memory mapped registers */

#define CB_SOCKET_EVENT			0x0000
#define  CB_SE_CSTSCHG			0x00000001
#define  CB_SE_CCD			0x00000006
#define  CB_SE_CCD1			0x00000002
#define  CB_SE_CCD2			0x00000004
#define  CB_SE_PWRCYCLE			0x00000008

#define CB_SOCKET_MASK			0x0004
#define  CB_SM_CSTSCHG			0x00000001
#define  CB_SM_CCD			0x00000006
#define  CB_SM_PWRCYCLE			0x00000008

#define CB_SOCKET_STATE			0x0008
#define  CB_SS_CSTSCHG			0x00000001
#define  CB_SS_CCD			0x00000006
#define  CB_SS_CCD1			0x00000002
#define  CB_SS_CCD2			0x00000004
#define  CB_SS_PWRCYCLE			0x00000008
#define  CB_SS_16BIT			0x00000010
#define  CB_SS_32BIT			0x00000020
#define  CB_SS_CINT			0x00000040
#define  CB_SS_BADCARD			0x00000080
#define  CB_SS_DATALOST			0x00000100
#define  CB_SS_BADVCC			0x00000200
#define  CB_SS_5VCARD			0x00000400
#define  CB_SS_3VCARD			0x00000800
#define  CB_SS_XVCARD			0x00001000
#define  CB_SS_YVCARD			0x00002000
#define  CB_SS_VSENSE			0x00003c86
#define  CB_SS_5VSOCKET			0x10000000
#define  CB_SS_3VSOCKET			0x20000000
#define  CB_SS_XVSOCKET			0x40000000
#define  CB_SS_YVSOCKET			0x80000000

#define CB_SOCKET_FORCE			0x000c
#define  CB_SF_CVSTEST			0x00004000

#define CB_SOCKET_CONTROL		0x0010
#define  CB_SC_VPP_MASK			0x00000007
#define   CB_SC_VPP_OFF			0x00000000
#define   CB_SC_VPP_12V			0x00000001
#define   CB_SC_VPP_5V			0x00000002
#define   CB_SC_VPP_3V			0x00000003
#define   CB_SC_VPP_XV			0x00000004
#define   CB_SC_VPP_YV			0x00000005
#define  CB_SC_VCC_MASK			0x00000070
#define   CB_SC_VCC_OFF			0x00000000
#define   CB_SC_VCC_5V			0x00000020
#define   CB_SC_VCC_3V			0x00000030
#define   CB_SC_VCC_XV			0x00000040
#define   CB_SC_VCC_YV			0x00000050
#define  CB_SC_CCLK_STOP		0x00000080

#define CB_SOCKET_POWER			0x0020
#define  CB_SP_CLK_CTRL			0x00000001
#define  CB_SP_CLK_CTRL_ENA		0x00010000
#define  CB_SP_CLK_MODE			0x01000000
#define  CB_SP_ACCESS			0x02000000

/* Address bits 31..24 for memory windows for 16-bit cards,
   accessable only by memory mapping the 16-bit register set */
#define CB_MEM_PAGE(map)		(0x40 + (map))

#endif /* _LINUX_YENTA_H */
