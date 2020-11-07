/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

:>
*/

#include "ru.h"

#if RU_INCLUDE_FIELD_DB
/******************************************************************************
 * Field: XIF_CTL_RXENCRYPTEN
 ******************************************************************************/
const ru_field_rec XIF_CTL_RXENCRYPTEN_FIELD =
{
    "RXENCRYPTEN",
#if RU_INCLUDE_DESC
    "",
    "Global downstream receive encryption enable : 0 - Normal operation;"
    "1 - Enable encryption.",
#endif
    XIF_CTL_RXENCRYPTEN_FIELD_MASK,
    0,
    XIF_CTL_RXENCRYPTEN_FIELD_WIDTH,
    XIF_CTL_RXENCRYPTEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_CFGDISRXDASAENCRPT
 ******************************************************************************/
const ru_field_rec XIF_CTL_CFGDISRXDASAENCRPT_FIELD =
{
    "CFGDISRXDASAENCRPT",
#if RU_INCLUDE_DESC
    "",
    "Disable DA/SA downstream decryption : 0 - Normal operation; 1 -"
    "Disable DA/SA decryption.",
#endif
    XIF_CTL_CFGDISRXDASAENCRPT_FIELD_MASK,
    0,
    XIF_CTL_CFGDISRXDASAENCRPT_FIELD_WIDTH,
    XIF_CTL_CFGDISRXDASAENCRPT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_RXENCRYPTMODE
 ******************************************************************************/
const ru_field_rec XIF_CTL_RXENCRYPTMODE_FIELD =
{
    "RXENCRYPTMODE",
#if RU_INCLUDE_DESC
    "",
    "Downstream receive security mode : 0 - Zero overhead; 2 - 802.1ae; 3"
    "- 3Churn.",
#endif
    XIF_CTL_RXENCRYPTMODE_FIELD_MASK,
    0,
    XIF_CTL_RXENCRYPTMODE_FIELD_WIDTH,
    XIF_CTL_RXENCRYPTMODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_TXENCRYPTEN
 ******************************************************************************/
const ru_field_rec XIF_CTL_TXENCRYPTEN_FIELD =
{
    "TXENCRYPTEN",
#if RU_INCLUDE_DESC
    "",
    "Global upstream transmit encryption enable : 0 - Normal operation;"
    "1 - Enable encryption.",
#endif
    XIF_CTL_TXENCRYPTEN_FIELD_MASK,
    0,
    XIF_CTL_TXENCRYPTEN_FIELD_WIDTH,
    XIF_CTL_TXENCRYPTEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_CFGDISTXDASAENCRPT
 ******************************************************************************/
const ru_field_rec XIF_CTL_CFGDISTXDASAENCRPT_FIELD =
{
    "CFGDISTXDASAENCRPT",
#if RU_INCLUDE_DESC
    "",
    "Disable DA/SA upstream encryption : 0 - Normal operation; 1 -"
    "Disable DA/SA encryption.",
#endif
    XIF_CTL_CFGDISTXDASAENCRPT_FIELD_MASK,
    0,
    XIF_CTL_CFGDISTXDASAENCRPT_FIELD_WIDTH,
    XIF_CTL_CFGDISTXDASAENCRPT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_TXENCRYPTMODE
 ******************************************************************************/
const ru_field_rec XIF_CTL_TXENCRYPTMODE_FIELD =
{
    "TXENCRYPTMODE",
#if RU_INCLUDE_DESC
    "",
    "Upstream transmit security mode : 0 - Zero overhead; 2 - 802.1ae.",
#endif
    XIF_CTL_TXENCRYPTMODE_FIELD_MASK,
    0,
    XIF_CTL_TXENCRYPTMODE_FIELD_WIDTH,
    XIF_CTL_TXENCRYPTMODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec XIF_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_CTL_RESERVED0_FIELD_MASK,
    0,
    XIF_CTL_RESERVED0_FIELD_WIDTH,
    XIF_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_CFGLLIDMODEMSK
 ******************************************************************************/
const ru_field_rec XIF_CTL_CFGLLIDMODEMSK_FIELD =
{
    "CFGLLIDMODEMSK",
#if RU_INCLUDE_DESC
    "",
    "Masks MSB of 16 bit raw LLID for Index translation."
    "0: Don't mask, look at full 16 bits."
    "1: Mask bit[15], map based on [14:0].",
#endif
    XIF_CTL_CFGLLIDMODEMSK_FIELD_MASK,
    0,
    XIF_CTL_CFGLLIDMODEMSK_FIELD_WIDTH,
    XIF_CTL_CFGLLIDMODEMSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_CFGXPNBADCRC32
 ******************************************************************************/
const ru_field_rec XIF_CTL_CFGXPNBADCRC32_FIELD =
{
    "CFGXPNBADCRC32",
#if RU_INCLUDE_DESC
    "",
    "Enable bad upstream FCS generation :  0 - Normal operation; 1 -"
    "Enable bad FCS generation of 0's.",
#endif
    XIF_CTL_CFGXPNBADCRC32_FIELD_MASK,
    0,
    XIF_CTL_CFGXPNBADCRC32_FIELD_WIDTH,
    XIF_CTL_CFGXPNBADCRC32_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_CFGDISDISCINFO
 ******************************************************************************/
const ru_field_rec XIF_CTL_CFGDISDISCINFO_FIELD =
{
    "CFGDISDISCINFO",
#if RU_INCLUDE_DESC
    "",
    "Disable Discovery Info field : 0 - Normal operation; 1 - Disable"
    "DISCOVERY info.",
#endif
    XIF_CTL_CFGDISDISCINFO_FIELD_MASK,
    0,
    XIF_CTL_CFGDISDISCINFO_FIELD_WIDTH,
    XIF_CTL_CFGDISDISCINFO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_CFGPMCTX2RXLPBK
 ******************************************************************************/
const ru_field_rec XIF_CTL_CFGPMCTX2RXLPBK_FIELD =
{
    "CFGPMCTX2RXLPBK",
#if RU_INCLUDE_DESC
    "",
    "Enable PMC loopback : 0 - Normal operation; 1 - Loopback.  NOT"
    "APPLICABLE in ONU since Tx/Rx clocks are not the same clock.",
#endif
    XIF_CTL_CFGPMCTX2RXLPBK_FIELD_MASK,
    0,
    XIF_CTL_CFGPMCTX2RXLPBK_FIELD_WIDTH,
    XIF_CTL_CFGPMCTX2RXLPBK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_CFGPMCTXENCRC8BAD
 ******************************************************************************/
const ru_field_rec XIF_CTL_CFGPMCTXENCRC8BAD_FIELD =
{
    "CFGPMCTXENCRC8BAD",
#if RU_INCLUDE_DESC
    "",
    "Enable upstream bad CRC8 transmission : 0 - Normal operation; 1 -"
    "Enable bad CRC8 generation.",
#endif
    XIF_CTL_CFGPMCTXENCRC8BAD_FIELD_MASK,
    0,
    XIF_CTL_CFGPMCTXENCRC8BAD_FIELD_WIDTH,
    XIF_CTL_CFGPMCTXENCRC8BAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_CFGENP2P
 ******************************************************************************/
const ru_field_rec XIF_CTL_CFGENP2P_FIELD =
{
    "CFGENP2P",
#if RU_INCLUDE_DESC
    "",
    "Enable point-2-point mode for downstream and upstream :"
    "0 - PON mode. Upstream's preamble will be of type 0x55_55_d5_55."
    "Downstream expects the same preamble type else packet will be"
    "aborted. CRC8 checking is configurable by bit \"cfgPmcRxEnCrc8Chk\"."
    "1 - P2P mode.  Upstream's preamble will be of type 0x55_55_55_55."
    "Downstream expects the same preamble type else packet will be"
    "aborted.  CRC8 checking will be disabled.",
#endif
    XIF_CTL_CFGENP2P_FIELD_MASK,
    0,
    XIF_CTL_CFGENP2P_FIELD_WIDTH,
    XIF_CTL_CFGENP2P_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_RESERVED1
 ******************************************************************************/
const ru_field_rec XIF_CTL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_CTL_RESERVED1_FIELD_MASK,
    0,
    XIF_CTL_RESERVED1_FIELD_WIDTH,
    XIF_CTL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_CFGPMCRXENCRC8CHK
 ******************************************************************************/
const ru_field_rec XIF_CTL_CFGPMCRXENCRC8CHK_FIELD =
{
    "CFGPMCRXENCRC8CHK",
#if RU_INCLUDE_DESC
    "",
    "Enable PMC-RX checking of CRC8 : 0 - Disable; 1 - Enable.",
#endif
    XIF_CTL_CFGPMCRXENCRC8CHK_FIELD_MASK,
    0,
    XIF_CTL_CFGPMCRXENCRC8CHK_FIELD_WIDTH,
    XIF_CTL_CFGPMCRXENCRC8CHK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_RESERVED2
 ******************************************************************************/
const ru_field_rec XIF_CTL_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_CTL_RESERVED2_FIELD_MASK,
    0,
    XIF_CTL_RESERVED2_FIELD_WIDTH,
    XIF_CTL_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_CFGFECEN
 ******************************************************************************/
const ru_field_rec XIF_CTL_CFGFECEN_FIELD =
{
    "CFGFECEN",
#if RU_INCLUDE_DESC
    "",
    "Enable upstream FEC : 0 - no FEC.  1 - FEC.",
#endif
    XIF_CTL_CFGFECEN_FIELD_MASK,
    0,
    XIF_CTL_CFGFECEN_FIELD_WIDTH,
    XIF_CTL_CFGFECEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_CFGLEGACYRCVTSUPD
 ******************************************************************************/
const ru_field_rec XIF_CTL_CFGLEGACYRCVTSUPD_FIELD =
{
    "CFGLEGACYRCVTSUPD",
#if RU_INCLUDE_DESC
    "",
    "Enable legacy receive timestamp update.",
#endif
    XIF_CTL_CFGLEGACYRCVTSUPD_FIELD_MASK,
    0,
    XIF_CTL_CFGLEGACYRCVTSUPD_FIELD_WIDTH,
    XIF_CTL_CFGLEGACYRCVTSUPD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_CFGXPNENCRCPASSTHRU
 ******************************************************************************/
const ru_field_rec XIF_CTL_CFGXPNENCRCPASSTHRU_FIELD =
{
    "CFGXPNENCRCPASSTHRU",
#if RU_INCLUDE_DESC
    "",
    "Enable FCS pass through : 0 - Modify packet's FCS; 1 - Pass through"
    "with no FCS modification. Feature is only supported in A0.",
#endif
    XIF_CTL_CFGXPNENCRCPASSTHRU_FIELD_MASK,
    0,
    XIF_CTL_CFGXPNENCRCPASSTHRU_FIELD_WIDTH,
    XIF_CTL_CFGXPNENCRCPASSTHRU_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_CFGXPNDISTIMESTAMPMOD
 ******************************************************************************/
const ru_field_rec XIF_CTL_CFGXPNDISTIMESTAMPMOD_FIELD =
{
    "CFGXPNDISTIMESTAMPMOD",
#if RU_INCLUDE_DESC
    "",
    "Debug function to disable timestamp modification of MPCP packet. 0 -"
    "Normal Operation; 1 - Disable timestamp modification.",
#endif
    XIF_CTL_CFGXPNDISTIMESTAMPMOD_FIELD_MASK,
    0,
    XIF_CTL_CFGXPNDISTIMESTAMPMOD_FIELD_WIDTH,
    XIF_CTL_CFGXPNDISTIMESTAMPMOD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_XIFNOTRDY
 ******************************************************************************/
const ru_field_rec XIF_CTL_XIFNOTRDY_FIELD =
{
    "XIFNOTRDY",
#if RU_INCLUDE_DESC
    "",
    "XIF not ready indication due to RAM init :  1 - Not ready. 0 - Ready"
    "for operation.  All RAMs are initialized to 0's.",
#endif
    XIF_CTL_XIFNOTRDY_FIELD_MASK,
    0,
    XIF_CTL_XIFNOTRDY_FIELD_WIDTH,
    XIF_CTL_XIFNOTRDY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_XIFDTPORTRSTN
 ******************************************************************************/
const ru_field_rec XIF_CTL_XIFDTPORTRSTN_FIELD =
{
    "XIFDTPORTRSTN",
#if RU_INCLUDE_DESC
    "",
    "Active low reset for RAM data port. RAM init starts upon"
    "deassertion.  Bit xifNotRdy is to be polled for completion",
#endif
    XIF_CTL_XIFDTPORTRSTN_FIELD_MASK,
    0,
    XIF_CTL_XIFDTPORTRSTN_FIELD_WIDTH,
    XIF_CTL_XIFDTPORTRSTN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_XPNTXRSTN
 ******************************************************************************/
const ru_field_rec XIF_CTL_XPNTXRSTN_FIELD =
{
    "XPNTXRSTN",
#if RU_INCLUDE_DESC
    "",
    "Reset control for transmit XPN module.0 - Reset. 1 - Normal"
    "Operation.",
#endif
    XIF_CTL_XPNTXRSTN_FIELD_MASK,
    0,
    XIF_CTL_XPNTXRSTN_FIELD_WIDTH,
    XIF_CTL_XPNTXRSTN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_PMCTXRSTN
 ******************************************************************************/
const ru_field_rec XIF_CTL_PMCTXRSTN_FIELD =
{
    "PMCTXRSTN",
#if RU_INCLUDE_DESC
    "",
    "Reset control for transmit PMC module.0 - Reset. 1 - Normal"
    "Operation.",
#endif
    XIF_CTL_PMCTXRSTN_FIELD_MASK,
    0,
    XIF_CTL_PMCTXRSTN_FIELD_WIDTH,
    XIF_CTL_PMCTXRSTN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_SECTXRSTN
 ******************************************************************************/
const ru_field_rec XIF_CTL_SECTXRSTN_FIELD =
{
    "SECTXRSTN",
#if RU_INCLUDE_DESC
    "",
    "Reset control for transmit security module. 0 - Reset. 1 - Normal"
    "Operation.",
#endif
    XIF_CTL_SECTXRSTN_FIELD_MASK,
    0,
    XIF_CTL_SECTXRSTN_FIELD_WIDTH,
    XIF_CTL_SECTXRSTN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_CFGDISTXOAMENCRPT
 ******************************************************************************/
const ru_field_rec XIF_CTL_CFGDISTXOAMENCRPT_FIELD =
{
    "CFGDISTXOAMENCRPT",
#if RU_INCLUDE_DESC
    "",
    "Disable OAM encryption. 0 - Normal Operation.  1 - Disable MPCP"
    "encryption.",
#endif
    XIF_CTL_CFGDISTXOAMENCRPT_FIELD_MASK,
    0,
    XIF_CTL_CFGDISTXOAMENCRPT_FIELD_WIDTH,
    XIF_CTL_CFGDISTXOAMENCRPT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_CFGDISTXMPCPENCRPT
 ******************************************************************************/
const ru_field_rec XIF_CTL_CFGDISTXMPCPENCRPT_FIELD =
{
    "CFGDISTXMPCPENCRPT",
#if RU_INCLUDE_DESC
    "",
    "Disable MPCP encryption. 0 - Normal Operation.  1 - Disable MPCP"
    "encryption.",
#endif
    XIF_CTL_CFGDISTXMPCPENCRPT_FIELD_MASK,
    0,
    XIF_CTL_CFGDISTXMPCPENCRPT_FIELD_WIDTH,
    XIF_CTL_CFGDISTXMPCPENCRPT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_PMCRXRSTN
 ******************************************************************************/
const ru_field_rec XIF_CTL_PMCRXRSTN_FIELD =
{
    "PMCRXRSTN",
#if RU_INCLUDE_DESC
    "",
    "Reset control for receive PMC module.0 - Reset. 1 - Normal"
    "Operation.",
#endif
    XIF_CTL_PMCRXRSTN_FIELD_MASK,
    0,
    XIF_CTL_PMCRXRSTN_FIELD_WIDTH,
    XIF_CTL_PMCRXRSTN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_CTL_SECRXRSTN
 ******************************************************************************/
const ru_field_rec XIF_CTL_SECRXRSTN_FIELD =
{
    "SECRXRSTN",
#if RU_INCLUDE_DESC
    "",
    "Reset control for receive security module. 0 - Reset. 1 - Normal"
    "Operation.",
#endif
    XIF_CTL_SECRXRSTN_FIELD_MASK,
    0,
    XIF_CTL_SECRXRSTN_FIELD_WIDTH,
    XIF_CTL_SECRXRSTN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_INT_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec XIF_INT_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_INT_STATUS_RESERVED0_FIELD_MASK,
    0,
    XIF_INT_STATUS_RESERVED0_FIELD_WIDTH,
    XIF_INT_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_INT_STATUS_SECRXRPLYPRTCTABRTINT
 ******************************************************************************/
const ru_field_rec XIF_INT_STATUS_SECRXRPLYPRTCTABRTINT_FIELD =
{
    "SECRXRPLYPRTCTABRTINT",
#if RU_INCLUDE_DESC
    "",
    "[NON-FATAL] Applicable only in 802.1ae security.  Indicates the"
    "received packet was aborted due to replay protection.",
#endif
    XIF_INT_STATUS_SECRXRPLYPRTCTABRTINT_FIELD_MASK,
    0,
    XIF_INT_STATUS_SECRXRPLYPRTCTABRTINT_FIELD_WIDTH,
    XIF_INT_STATUS_SECRXRPLYPRTCTABRTINT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_INT_STATUS_SECTXPKTNUMMAXINT
 ******************************************************************************/
const ru_field_rec XIF_INT_STATUS_SECTXPKTNUMMAXINT_FIELD =
{
    "SECTXPKTNUMMAXINT",
#if RU_INCLUDE_DESC
    "",
    "[NON-FATAL] Applicable only in 802.1ae security.  Indicates the"
    "transmit packet number exceeded the maximum threshold and about to"
    "overflow.  Threshold is programmed in register XIF_AE_PKTNUM_THRESH.",
#endif
    XIF_INT_STATUS_SECTXPKTNUMMAXINT_FIELD_MASK,
    0,
    XIF_INT_STATUS_SECTXPKTNUMMAXINT_FIELD_WIDTH,
    XIF_INT_STATUS_SECTXPKTNUMMAXINT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_INT_STATUS_RESERVED1
 ******************************************************************************/
const ru_field_rec XIF_INT_STATUS_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_INT_STATUS_RESERVED1_FIELD_MASK,
    0,
    XIF_INT_STATUS_RESERVED1_FIELD_WIDTH,
    XIF_INT_STATUS_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_INT_STATUS_TSFULLUPDINT
 ******************************************************************************/
const ru_field_rec XIF_INT_STATUS_TSFULLUPDINT_FIELD =
{
    "TSFULLUPDINT",
#if RU_INCLUDE_DESC
    "",
    "Indicates full MPCP timestamp update due to value greater than"
    "threshold programmed into cfgTsFullUpdThr in register XIF_TS_UPDATE.",
#endif
    XIF_INT_STATUS_TSFULLUPDINT_FIELD_MASK,
    0,
    XIF_INT_STATUS_TSFULLUPDINT_FIELD_WIDTH,
    XIF_INT_STATUS_TSFULLUPDINT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_INT_STATUS_TXHANGINT
 ******************************************************************************/
const ru_field_rec XIF_INT_STATUS_TXHANGINT_FIELD =
{
    "TXHANGINT",
#if RU_INCLUDE_DESC
    "",
    "[FATAL] Indicates request to transmit never got serviced.",
#endif
    XIF_INT_STATUS_TXHANGINT_FIELD_MASK,
    0,
    XIF_INT_STATUS_TXHANGINT_FIELD_WIDTH,
    XIF_INT_STATUS_TXHANGINT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_INT_STATUS_NEGTIMEINT
 ******************************************************************************/
const ru_field_rec XIF_INT_STATUS_NEGTIMEINT_FIELD =
{
    "NEGTIMEINT",
#if RU_INCLUDE_DESC
    "",
    "[FATAL] Indicates scheduled transmit time is negative, relative to"
    "the current MPCP time.",
#endif
    XIF_INT_STATUS_NEGTIMEINT_FIELD_MASK,
    0,
    XIF_INT_STATUS_NEGTIMEINT_FIELD_WIDTH,
    XIF_INT_STATUS_NEGTIMEINT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_INT_STATUS_PMCTSJTTRINT
 ******************************************************************************/
const ru_field_rec XIF_INT_STATUS_PMCTSJTTRINT_FIELD =
{
    "PMCTSJTTRINT",
#if RU_INCLUDE_DESC
    "",
    "[NON-FATAL] Indicates the magnitude of the MPCP timestamp updated"
    "exceeded the value programmed into XIF_TS_JITTER_THRESH register.",
#endif
    XIF_INT_STATUS_PMCTSJTTRINT_FIELD_MASK,
    0,
    XIF_INT_STATUS_PMCTSJTTRINT_FIELD_WIDTH,
    XIF_INT_STATUS_PMCTSJTTRINT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_INT_STATUS_SECRXOUTFFOVRFLWINT
 ******************************************************************************/
const ru_field_rec XIF_INT_STATUS_SECRXOUTFFOVRFLWINT_FIELD =
{
    "SECRXOUTFFOVRFLWINT",
#if RU_INCLUDE_DESC
    "",
    "[FATAL] Indicates SEC-RX output FIFO overflowed.",
#endif
    XIF_INT_STATUS_SECRXOUTFFOVRFLWINT_FIELD_MASK,
    0,
    XIF_INT_STATUS_SECRXOUTFFOVRFLWINT_FIELD_WIDTH,
    XIF_INT_STATUS_SECRXOUTFFOVRFLWINT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_INT_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec XIF_INT_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_INT_MASK_RESERVED0_FIELD_MASK,
    0,
    XIF_INT_MASK_RESERVED0_FIELD_WIDTH,
    XIF_INT_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_INT_MASK_MSKSECRXREPLAYPROTCTABORT
 ******************************************************************************/
const ru_field_rec XIF_INT_MASK_MSKSECRXREPLAYPROTCTABORT_FIELD =
{
    "MSKSECRXREPLAYPROTCTABORT",
#if RU_INCLUDE_DESC
    "",
    "Interrupt mask, active low.",
#endif
    XIF_INT_MASK_MSKSECRXREPLAYPROTCTABORT_FIELD_MASK,
    0,
    XIF_INT_MASK_MSKSECRXREPLAYPROTCTABORT_FIELD_WIDTH,
    XIF_INT_MASK_MSKSECRXREPLAYPROTCTABORT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_INT_MASK_MSKPKTNUMTHRESHINT
 ******************************************************************************/
const ru_field_rec XIF_INT_MASK_MSKPKTNUMTHRESHINT_FIELD =
{
    "MSKPKTNUMTHRESHINT",
#if RU_INCLUDE_DESC
    "",
    "Interrupt mask, active low.",
#endif
    XIF_INT_MASK_MSKPKTNUMTHRESHINT_FIELD_MASK,
    0,
    XIF_INT_MASK_MSKPKTNUMTHRESHINT_FIELD_WIDTH,
    XIF_INT_MASK_MSKPKTNUMTHRESHINT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_INT_MASK_RESERVED1
 ******************************************************************************/
const ru_field_rec XIF_INT_MASK_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_INT_MASK_RESERVED1_FIELD_MASK,
    0,
    XIF_INT_MASK_RESERVED1_FIELD_WIDTH,
    XIF_INT_MASK_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_INT_MASK_MSKTSFULLUPDINT
 ******************************************************************************/
const ru_field_rec XIF_INT_MASK_MSKTSFULLUPDINT_FIELD =
{
    "MSKTSFULLUPDINT",
#if RU_INCLUDE_DESC
    "",
    "Interrupt mask, active low.",
#endif
    XIF_INT_MASK_MSKTSFULLUPDINT_FIELD_MASK,
    0,
    XIF_INT_MASK_MSKTSFULLUPDINT_FIELD_WIDTH,
    XIF_INT_MASK_MSKTSFULLUPDINT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_INT_MASK_MSKTXHANGINT
 ******************************************************************************/
const ru_field_rec XIF_INT_MASK_MSKTXHANGINT_FIELD =
{
    "MSKTXHANGINT",
#if RU_INCLUDE_DESC
    "",
    "Interrupt mask, active low.",
#endif
    XIF_INT_MASK_MSKTXHANGINT_FIELD_MASK,
    0,
    XIF_INT_MASK_MSKTXHANGINT_FIELD_WIDTH,
    XIF_INT_MASK_MSKTXHANGINT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_INT_MASK_MSKNEGTIMEINT
 ******************************************************************************/
const ru_field_rec XIF_INT_MASK_MSKNEGTIMEINT_FIELD =
{
    "MSKNEGTIMEINT",
#if RU_INCLUDE_DESC
    "",
    "Interrupt mask, active low.",
#endif
    XIF_INT_MASK_MSKNEGTIMEINT_FIELD_MASK,
    0,
    XIF_INT_MASK_MSKNEGTIMEINT_FIELD_WIDTH,
    XIF_INT_MASK_MSKNEGTIMEINT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_INT_MASK_MSKPMCTSJTTRINT
 ******************************************************************************/
const ru_field_rec XIF_INT_MASK_MSKPMCTSJTTRINT_FIELD =
{
    "MSKPMCTSJTTRINT",
#if RU_INCLUDE_DESC
    "",
    "Interrupt mask, active low.",
#endif
    XIF_INT_MASK_MSKPMCTSJTTRINT_FIELD_MASK,
    0,
    XIF_INT_MASK_MSKPMCTSJTTRINT_FIELD_WIDTH,
    XIF_INT_MASK_MSKPMCTSJTTRINT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_INT_MASK_MSKSECRXOUTFFINT
 ******************************************************************************/
const ru_field_rec XIF_INT_MASK_MSKSECRXOUTFFINT_FIELD =
{
    "MSKSECRXOUTFFINT",
#if RU_INCLUDE_DESC
    "",
    "Interrupt mask, active low.",
#endif
    XIF_INT_MASK_MSKSECRXOUTFFINT_FIELD_MASK,
    0,
    XIF_INT_MASK_MSKSECRXOUTFFINT_FIELD_WIDTH,
    XIF_INT_MASK_MSKSECRXOUTFFINT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_PORT_COMMAND_DATAPORTBUSY
 ******************************************************************************/
const ru_field_rec XIF_PORT_COMMAND_DATAPORTBUSY_FIELD =
{
    "DATAPORTBUSY",
#if RU_INCLUDE_DESC
    "",
    "Indicates dataPort access is in progress.  Bit must be cleared"
    "before the next dataPort access can be issued.",
#endif
    XIF_PORT_COMMAND_DATAPORTBUSY_FIELD_MASK,
    0,
    XIF_PORT_COMMAND_DATAPORTBUSY_FIELD_WIDTH,
    XIF_PORT_COMMAND_DATAPORTBUSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_PORT_COMMAND_RESERVED0
 ******************************************************************************/
const ru_field_rec XIF_PORT_COMMAND_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_PORT_COMMAND_RESERVED0_FIELD_MASK,
    0,
    XIF_PORT_COMMAND_RESERVED0_FIELD_WIDTH,
    XIF_PORT_COMMAND_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_PORT_COMMAND_PORTSELECT
 ******************************************************************************/
const ru_field_rec XIF_PORT_COMMAND_PORTSELECT_FIELD =
{
    "PORTSELECT",
#if RU_INCLUDE_DESC
    "",
    "Selects the RAM for access : 0 - RX key; 2 - TX key; 4 - RX IV; 5 -"
    "TX IV.",
#endif
    XIF_PORT_COMMAND_PORTSELECT_FIELD_MASK,
    0,
    XIF_PORT_COMMAND_PORTSELECT_FIELD_WIDTH,
    XIF_PORT_COMMAND_PORTSELECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_PORT_COMMAND_PORTOPCODE
 ******************************************************************************/
const ru_field_rec XIF_PORT_COMMAND_PORTOPCODE_FIELD =
{
    "PORTOPCODE",
#if RU_INCLUDE_DESC
    "",
    "Indicates write access : 0 - read; 1 - write.",
#endif
    XIF_PORT_COMMAND_PORTOPCODE_FIELD_MASK,
    0,
    XIF_PORT_COMMAND_PORTOPCODE_FIELD_WIDTH,
    XIF_PORT_COMMAND_PORTOPCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_PORT_COMMAND_PORTADDRESS
 ******************************************************************************/
const ru_field_rec XIF_PORT_COMMAND_PORTADDRESS_FIELD =
{
    "PORTADDRESS",
#if RU_INCLUDE_DESC
    "",
    "Specifies the RAM address for access.",
#endif
    XIF_PORT_COMMAND_PORTADDRESS_FIELD_MASK,
    0,
    XIF_PORT_COMMAND_PORTADDRESS_FIELD_WIDTH,
    XIF_PORT_COMMAND_PORTADDRESS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_PORT_DATA__PORTDATA
 ******************************************************************************/
const ru_field_rec XIF_PORT_DATA__PORTDATA_FIELD =
{
    "PORTDATA",
#if RU_INCLUDE_DESC
    "",
    "TX/RX SEC key RAM, key[31:0].",
#endif
    XIF_PORT_DATA__PORTDATA_FIELD_MASK,
    0,
    XIF_PORT_DATA__PORTDATA_FIELD_WIDTH,
    XIF_PORT_DATA__PORTDATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_MACSEC_RESERVED0
 ******************************************************************************/
const ru_field_rec XIF_MACSEC_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_MACSEC_RESERVED0_FIELD_MASK,
    0,
    XIF_MACSEC_RESERVED0_FIELD_WIDTH,
    XIF_MACSEC_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_MACSEC_CFGMACSECETHERTYPE
 ******************************************************************************/
const ru_field_rec XIF_MACSEC_CFGMACSECETHERTYPE_FIELD =
{
    "CFGMACSECETHERTYPE",
#if RU_INCLUDE_DESC
    "",
    "Defines the MacSec Ethertype.",
#endif
    XIF_MACSEC_CFGMACSECETHERTYPE_FIELD_MASK,
    0,
    XIF_MACSEC_CFGMACSECETHERTYPE_FIELD_WIDTH,
    XIF_MACSEC_CFGMACSECETHERTYPE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_XMT_OFFSET_RESERVED0
 ******************************************************************************/
const ru_field_rec XIF_XPN_XMT_OFFSET_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_XPN_XMT_OFFSET_RESERVED0_FIELD_MASK,
    0,
    XIF_XPN_XMT_OFFSET_RESERVED0_FIELD_WIDTH,
    XIF_XPN_XMT_OFFSET_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_XMT_OFFSET_CFGXPNXMTOFFSET
 ******************************************************************************/
const ru_field_rec XIF_XPN_XMT_OFFSET_CFGXPNXMTOFFSET_FIELD =
{
    "CFGXPNXMTOFFSET",
#if RU_INCLUDE_DESC
    "",
    "Specifies the transmit offset, to account for the delay through"
    "SEC-TX and PMC-TX.",
#endif
    XIF_XPN_XMT_OFFSET_CFGXPNXMTOFFSET_FIELD_MASK,
    0,
    XIF_XPN_XMT_OFFSET_CFGXPNXMTOFFSET_FIELD_WIDTH,
    XIF_XPN_XMT_OFFSET_CFGXPNXMTOFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_TIMESTAMP_OFFSET_CFGXPNMPCPTSOFFSET
 ******************************************************************************/
const ru_field_rec XIF_XPN_TIMESTAMP_OFFSET_CFGXPNMPCPTSOFFSET_FIELD =
{
    "CFGXPNMPCPTSOFFSET",
#if RU_INCLUDE_DESC
    "",
    "Debug funtion to add the offset to the regenerated MPCP's timestamp.",
#endif
    XIF_XPN_TIMESTAMP_OFFSET_CFGXPNMPCPTSOFFSET_FIELD_MASK,
    0,
    XIF_XPN_TIMESTAMP_OFFSET_CFGXPNMPCPTSOFFSET_FIELD_WIDTH,
    XIF_XPN_TIMESTAMP_OFFSET_CFGXPNMPCPTSOFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_PKTGEN_CTL_CFGONUBURSTSIZE
 ******************************************************************************/
const ru_field_rec XIF_XPN_PKTGEN_CTL_CFGONUBURSTSIZE_FIELD =
{
    "CFGONUBURSTSIZE",
#if RU_INCLUDE_DESC
    "",
    "Burst since, in TQ unit.",
#endif
    XIF_XPN_PKTGEN_CTL_CFGONUBURSTSIZE_FIELD_MASK,
    0,
    XIF_XPN_PKTGEN_CTL_CFGONUBURSTSIZE_FIELD_WIDTH,
    XIF_XPN_PKTGEN_CTL_CFGONUBURSTSIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_PKTGEN_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec XIF_XPN_PKTGEN_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_XPN_PKTGEN_CTL_RESERVED0_FIELD_MASK,
    0,
    XIF_XPN_PKTGEN_CTL_RESERVED0_FIELD_WIDTH,
    XIF_XPN_PKTGEN_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_PKTGEN_CTL_CFGENBCK2BCKPKTGEN
 ******************************************************************************/
const ru_field_rec XIF_XPN_PKTGEN_CTL_CFGENBCK2BCKPKTGEN_FIELD =
{
    "CFGENBCK2BCKPKTGEN",
#if RU_INCLUDE_DESC
    "",
    "Enable back-2-back grants for overlap testing.",
#endif
    XIF_XPN_PKTGEN_CTL_CFGENBCK2BCKPKTGEN_FIELD_MASK,
    0,
    XIF_XPN_PKTGEN_CTL_CFGENBCK2BCKPKTGEN_FIELD_WIDTH,
    XIF_XPN_PKTGEN_CTL_CFGENBCK2BCKPKTGEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_PKTGEN_CTL_CFGENALLMPCPPKTGEN
 ******************************************************************************/
const ru_field_rec XIF_XPN_PKTGEN_CTL_CFGENALLMPCPPKTGEN_FIELD =
{
    "CFGENALLMPCPPKTGEN",
#if RU_INCLUDE_DESC
    "",
    "Enable all MPCP packet generation.",
#endif
    XIF_XPN_PKTGEN_CTL_CFGENALLMPCPPKTGEN_FIELD_MASK,
    0,
    XIF_XPN_PKTGEN_CTL_CFGENALLMPCPPKTGEN_FIELD_WIDTH,
    XIF_XPN_PKTGEN_CTL_CFGENALLMPCPPKTGEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_PKTGEN_CTL_RESERVED1
 ******************************************************************************/
const ru_field_rec XIF_XPN_PKTGEN_CTL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_XPN_PKTGEN_CTL_RESERVED1_FIELD_MASK,
    0,
    XIF_XPN_PKTGEN_CTL_RESERVED1_FIELD_WIDTH,
    XIF_XPN_PKTGEN_CTL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_PKTGEN_CTL_CFGXPNSTARTPKTGEN
 ******************************************************************************/
const ru_field_rec XIF_XPN_PKTGEN_CTL_CFGXPNSTARTPKTGEN_FIELD =
{
    "CFGXPNSTARTPKTGEN",
#if RU_INCLUDE_DESC
    "",
    "Starts packet generator.",
#endif
    XIF_XPN_PKTGEN_CTL_CFGXPNSTARTPKTGEN_FIELD_MASK,
    0,
    XIF_XPN_PKTGEN_CTL_CFGXPNSTARTPKTGEN_FIELD_WIDTH,
    XIF_XPN_PKTGEN_CTL_CFGXPNSTARTPKTGEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_PKTGEN_CTL_CFGXPNENPKTGEN
 ******************************************************************************/
const ru_field_rec XIF_XPN_PKTGEN_CTL_CFGXPNENPKTGEN_FIELD =
{
    "CFGXPNENPKTGEN",
#if RU_INCLUDE_DESC
    "",
    "Enables packet generator.",
#endif
    XIF_XPN_PKTGEN_CTL_CFGXPNENPKTGEN_FIELD_MASK,
    0,
    XIF_XPN_PKTGEN_CTL_CFGXPNENPKTGEN_FIELD_WIDTH,
    XIF_XPN_PKTGEN_CTL_CFGXPNENPKTGEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_PKTGEN_LLID_CFGXPNPKTGENLLID1
 ******************************************************************************/
const ru_field_rec XIF_XPN_PKTGEN_LLID_CFGXPNPKTGENLLID1_FIELD =
{
    "CFGXPNPKTGENLLID1",
#if RU_INCLUDE_DESC
    "",
    "LLID for index 1.",
#endif
    XIF_XPN_PKTGEN_LLID_CFGXPNPKTGENLLID1_FIELD_MASK,
    0,
    XIF_XPN_PKTGEN_LLID_CFGXPNPKTGENLLID1_FIELD_WIDTH,
    XIF_XPN_PKTGEN_LLID_CFGXPNPKTGENLLID1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_PKTGEN_LLID_CFGXPNPKTGENLLID0
 ******************************************************************************/
const ru_field_rec XIF_XPN_PKTGEN_LLID_CFGXPNPKTGENLLID0_FIELD =
{
    "CFGXPNPKTGENLLID0",
#if RU_INCLUDE_DESC
    "",
    "LLID for index 0.",
#endif
    XIF_XPN_PKTGEN_LLID_CFGXPNPKTGENLLID0_FIELD_MASK,
    0,
    XIF_XPN_PKTGEN_LLID_CFGXPNPKTGENLLID0_FIELD_WIDTH,
    XIF_XPN_PKTGEN_LLID_CFGXPNPKTGENLLID0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_PKTGEN_PKT_CNT_CFGXPNPKTGENBURSTMODE
 ******************************************************************************/
const ru_field_rec XIF_XPN_PKTGEN_PKT_CNT_CFGXPNPKTGENBURSTMODE_FIELD =
{
    "CFGXPNPKTGENBURSTMODE",
#if RU_INCLUDE_DESC
    "",
    "Burst mode generation : 0 - continuous; 1 - burst mode as defined by"
    "cfgXpnPktGenBurstSize.",
#endif
    XIF_XPN_PKTGEN_PKT_CNT_CFGXPNPKTGENBURSTMODE_FIELD_MASK,
    0,
    XIF_XPN_PKTGEN_PKT_CNT_CFGXPNPKTGENBURSTMODE_FIELD_WIDTH,
    XIF_XPN_PKTGEN_PKT_CNT_CFGXPNPKTGENBURSTMODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_PKTGEN_PKT_CNT_RESERVED0
 ******************************************************************************/
const ru_field_rec XIF_XPN_PKTGEN_PKT_CNT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_XPN_PKTGEN_PKT_CNT_RESERVED0_FIELD_MASK,
    0,
    XIF_XPN_PKTGEN_PKT_CNT_RESERVED0_FIELD_WIDTH,
    XIF_XPN_PKTGEN_PKT_CNT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_PKTGEN_PKT_CNT_CFGXPNPKTGENBURSTSIZE
 ******************************************************************************/
const ru_field_rec XIF_XPN_PKTGEN_PKT_CNT_CFGXPNPKTGENBURSTSIZE_FIELD =
{
    "CFGXPNPKTGENBURSTSIZE",
#if RU_INCLUDE_DESC
    "",
    "Number of packets to transmit.",
#endif
    XIF_XPN_PKTGEN_PKT_CNT_CFGXPNPKTGENBURSTSIZE_FIELD_MASK,
    0,
    XIF_XPN_PKTGEN_PKT_CNT_CFGXPNPKTGENBURSTSIZE_FIELD_WIDTH,
    XIF_XPN_PKTGEN_PKT_CNT_CFGXPNPKTGENBURSTSIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_PKTGEN_PKT_SIZE_CFGXPNPKTGENSIZEINCR
 ******************************************************************************/
const ru_field_rec XIF_XPN_PKTGEN_PKT_SIZE_CFGXPNPKTGENSIZEINCR_FIELD =
{
    "CFGXPNPKTGENSIZEINCR",
#if RU_INCLUDE_DESC
    "",
    "Size mode : 0 - fixed packet size, defined by cfgXpnPktGenSizeStart;"
    "1 - increment packet size, from cfgXpnPktGenSizeStart to"
    "cfgXpnPktGenSizeEnd.",
#endif
    XIF_XPN_PKTGEN_PKT_SIZE_CFGXPNPKTGENSIZEINCR_FIELD_MASK,
    0,
    XIF_XPN_PKTGEN_PKT_SIZE_CFGXPNPKTGENSIZEINCR_FIELD_WIDTH,
    XIF_XPN_PKTGEN_PKT_SIZE_CFGXPNPKTGENSIZEINCR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_PKTGEN_PKT_SIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec XIF_XPN_PKTGEN_PKT_SIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_XPN_PKTGEN_PKT_SIZE_RESERVED0_FIELD_MASK,
    0,
    XIF_XPN_PKTGEN_PKT_SIZE_RESERVED0_FIELD_WIDTH,
    XIF_XPN_PKTGEN_PKT_SIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_PKTGEN_PKT_SIZE_CFGXPNPKTGENSIZEEND
 ******************************************************************************/
const ru_field_rec XIF_XPN_PKTGEN_PKT_SIZE_CFGXPNPKTGENSIZEEND_FIELD =
{
    "CFGXPNPKTGENSIZEEND",
#if RU_INCLUDE_DESC
    "",
    "Indicates the ending size.",
#endif
    XIF_XPN_PKTGEN_PKT_SIZE_CFGXPNPKTGENSIZEEND_FIELD_MASK,
    0,
    XIF_XPN_PKTGEN_PKT_SIZE_CFGXPNPKTGENSIZEEND_FIELD_WIDTH,
    XIF_XPN_PKTGEN_PKT_SIZE_CFGXPNPKTGENSIZEEND_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_PKTGEN_PKT_SIZE_RESERVED1
 ******************************************************************************/
const ru_field_rec XIF_XPN_PKTGEN_PKT_SIZE_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_XPN_PKTGEN_PKT_SIZE_RESERVED1_FIELD_MASK,
    0,
    XIF_XPN_PKTGEN_PKT_SIZE_RESERVED1_FIELD_WIDTH,
    XIF_XPN_PKTGEN_PKT_SIZE_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_PKTGEN_PKT_SIZE_CFGXPNPKTGENSIZESTART
 ******************************************************************************/
const ru_field_rec XIF_XPN_PKTGEN_PKT_SIZE_CFGXPNPKTGENSIZESTART_FIELD =
{
    "CFGXPNPKTGENSIZESTART",
#if RU_INCLUDE_DESC
    "",
    "Indicates the starting size.",
#endif
    XIF_XPN_PKTGEN_PKT_SIZE_CFGXPNPKTGENSIZESTART_FIELD_MASK,
    0,
    XIF_XPN_PKTGEN_PKT_SIZE_CFGXPNPKTGENSIZESTART_FIELD_WIDTH,
    XIF_XPN_PKTGEN_PKT_SIZE_CFGXPNPKTGENSIZESTART_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_PKTGEN_IPG_CFGXPNPKTGENBCK2BCKIPG
 ******************************************************************************/
const ru_field_rec XIF_XPN_PKTGEN_IPG_CFGXPNPKTGENBCK2BCKIPG_FIELD =
{
    "CFGXPNPKTGENBCK2BCKIPG",
#if RU_INCLUDE_DESC
    "",
    "IPG insertion for back-2-back grants.",
#endif
    XIF_XPN_PKTGEN_IPG_CFGXPNPKTGENBCK2BCKIPG_FIELD_MASK,
    0,
    XIF_XPN_PKTGEN_IPG_CFGXPNPKTGENBCK2BCKIPG_FIELD_WIDTH,
    XIF_XPN_PKTGEN_IPG_CFGXPNPKTGENBCK2BCKIPG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_PKTGEN_IPG_CFGXPNPKTGENIPG
 ******************************************************************************/
const ru_field_rec XIF_XPN_PKTGEN_IPG_CFGXPNPKTGENIPG_FIELD =
{
    "CFGXPNPKTGENIPG",
#if RU_INCLUDE_DESC
    "",
    "IPG insertion in between packets.",
#endif
    XIF_XPN_PKTGEN_IPG_CFGXPNPKTGENIPG_FIELD_MASK,
    0,
    XIF_XPN_PKTGEN_IPG_CFGXPNPKTGENIPG_FIELD_WIDTH,
    XIF_XPN_PKTGEN_IPG_CFGXPNPKTGENIPG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_TS_JITTER_THRESH_RESERVED0
 ******************************************************************************/
const ru_field_rec XIF_TS_JITTER_THRESH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_TS_JITTER_THRESH_RESERVED0_FIELD_MASK,
    0,
    XIF_TS_JITTER_THRESH_RESERVED0_FIELD_WIDTH,
    XIF_TS_JITTER_THRESH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_TS_JITTER_THRESH_CFGTSJTTRTHRESH
 ******************************************************************************/
const ru_field_rec XIF_TS_JITTER_THRESH_CFGTSJTTRTHRESH_FIELD =
{
    "CFGTSJTTRTHRESH",
#if RU_INCLUDE_DESC
    "",
    "Defines the value to generate jitter interrupt when timestamp update"
    "exceeds this threshold.",
#endif
    XIF_TS_JITTER_THRESH_CFGTSJTTRTHRESH_FIELD_MASK,
    0,
    XIF_TS_JITTER_THRESH_CFGTSJTTRTHRESH_FIELD_WIDTH,
    XIF_TS_JITTER_THRESH_CFGTSJTTRTHRESH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_TS_UPDATE_CFGTSFULLUPDTHR
 ******************************************************************************/
const ru_field_rec XIF_TS_UPDATE_CFGTSFULLUPDTHR_FIELD =
{
    "CFGTSFULLUPDTHR",
#if RU_INCLUDE_DESC
    "",
    "Defines the full update threshold.  Timestamp update is done in 1 TQ"
    "increment.  If update is equal to or greater than threshold, full"
    "update will result.",
#endif
    XIF_TS_UPDATE_CFGTSFULLUPDTHR_FIELD_MASK,
    0,
    XIF_TS_UPDATE_CFGTSFULLUPDTHR_FIELD_WIDTH,
    XIF_TS_UPDATE_CFGTSFULLUPDTHR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_TS_UPDATE_CFGENAUTOTSUPD
 ******************************************************************************/
const ru_field_rec XIF_TS_UPDATE_CFGENAUTOTSUPD_FIELD =
{
    "CFGENAUTOTSUPD",
#if RU_INCLUDE_DESC
    "",
    "Provides auto timestamp update for debugging.  This is to test for"
    "timestamp jitter.",
#endif
    XIF_TS_UPDATE_CFGENAUTOTSUPD_FIELD_MASK,
    0,
    XIF_TS_UPDATE_CFGENAUTOTSUPD_FIELD_WIDTH,
    XIF_TS_UPDATE_CFGENAUTOTSUPD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_TS_UPDATE_RESERVED0
 ******************************************************************************/
const ru_field_rec XIF_TS_UPDATE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_TS_UPDATE_RESERVED0_FIELD_MASK,
    0,
    XIF_TS_UPDATE_RESERVED0_FIELD_WIDTH,
    XIF_TS_UPDATE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_TS_UPDATE_CFGTSUPDPER
 ******************************************************************************/
const ru_field_rec XIF_TS_UPDATE_CFGTSUPDPER_FIELD =
{
    "CFGTSUPDPER",
#if RU_INCLUDE_DESC
    "",
    "Defines the period between MPCP timestamp update.",
#endif
    XIF_TS_UPDATE_CFGTSUPDPER_FIELD_MASK,
    0,
    XIF_TS_UPDATE_CFGTSUPDPER_FIELD_WIDTH,
    XIF_TS_UPDATE_CFGTSUPDPER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_GNT_OVERHEAD_RESERVED0
 ******************************************************************************/
const ru_field_rec XIF_GNT_OVERHEAD_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_GNT_OVERHEAD_RESERVED0_FIELD_MASK,
    0,
    XIF_GNT_OVERHEAD_RESERVED0_FIELD_WIDTH,
    XIF_GNT_OVERHEAD_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_GNT_OVERHEAD_CFGGNTOH
 ******************************************************************************/
const ru_field_rec XIF_GNT_OVERHEAD_CFGGNTOH_FIELD =
{
    "CFGGNTOH",
#if RU_INCLUDE_DESC
    "",
    "Burst overhead of laser_on + sync_time.",
#endif
    XIF_GNT_OVERHEAD_CFGGNTOH_FIELD_MASK,
    0,
    XIF_GNT_OVERHEAD_CFGGNTOH_FIELD_WIDTH,
    XIF_GNT_OVERHEAD_CFGGNTOH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_DISCOVER_OVERHEAD_RESERVED0
 ******************************************************************************/
const ru_field_rec XIF_DISCOVER_OVERHEAD_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_DISCOVER_OVERHEAD_RESERVED0_FIELD_MASK,
    0,
    XIF_DISCOVER_OVERHEAD_RESERVED0_FIELD_WIDTH,
    XIF_DISCOVER_OVERHEAD_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_DISCOVER_OVERHEAD_CFGDISCOH
 ******************************************************************************/
const ru_field_rec XIF_DISCOVER_OVERHEAD_CFGDISCOH_FIELD =
{
    "CFGDISCOH",
#if RU_INCLUDE_DESC
    "",
    "Burst overhead of laser_on + sync_time.",
#endif
    XIF_DISCOVER_OVERHEAD_CFGDISCOH_FIELD_MASK,
    0,
    XIF_DISCOVER_OVERHEAD_CFGDISCOH_FIELD_WIDTH,
    XIF_DISCOVER_OVERHEAD_CFGDISCOH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_DISCOVER_INFO_RESERVED0
 ******************************************************************************/
const ru_field_rec XIF_DISCOVER_INFO_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_DISCOVER_INFO_RESERVED0_FIELD_MASK,
    0,
    XIF_DISCOVER_INFO_RESERVED0_FIELD_WIDTH,
    XIF_DISCOVER_INFO_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_DISCOVER_INFO_CFGDISCINFOFLD
 ******************************************************************************/
const ru_field_rec XIF_DISCOVER_INFO_CFGDISCINFOFLD_FIELD =
{
    "CFGDISCINFOFLD",
#if RU_INCLUDE_DESC
    "",
    "Defines the discovery info field : 0 - upstream 1G; 1 - upstream"
    "10G; 4 - open 1G window; 5 - open 10G window.",
#endif
    XIF_DISCOVER_INFO_CFGDISCINFOFLD_FIELD_MASK,
    0,
    XIF_DISCOVER_INFO_CFGDISCINFOFLD_FIELD_WIDTH,
    XIF_DISCOVER_INFO_CFGDISCINFOFLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_OVERSIZE_THRESH_RESERVED0
 ******************************************************************************/
const ru_field_rec XIF_XPN_OVERSIZE_THRESH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_XPN_OVERSIZE_THRESH_RESERVED0_FIELD_MASK,
    0,
    XIF_XPN_OVERSIZE_THRESH_RESERVED0_FIELD_WIDTH,
    XIF_XPN_OVERSIZE_THRESH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_XPN_OVERSIZE_THRESH_CFGXPNOVRSZTHRESH
 ******************************************************************************/
const ru_field_rec XIF_XPN_OVERSIZE_THRESH_CFGXPNOVRSZTHRESH_FIELD =
{
    "CFGXPNOVRSZTHRESH",
#if RU_INCLUDE_DESC
    "",
    "Increments oversize stat when packet's size is greater than or equal"
    "to threshold.",
#endif
    XIF_XPN_OVERSIZE_THRESH_CFGXPNOVRSZTHRESH_FIELD_MASK,
    0,
    XIF_XPN_OVERSIZE_THRESH_CFGXPNOVRSZTHRESH_FIELD_WIDTH,
    XIF_XPN_OVERSIZE_THRESH_CFGXPNOVRSZTHRESH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_SECRX_KEYNUM_KEYSTATRX
 ******************************************************************************/
const ru_field_rec XIF_SECRX_KEYNUM_KEYSTATRX_FIELD =
{
    "KEYSTATRX",
#if RU_INCLUDE_DESC
    "",
    "Key number stat.",
#endif
    XIF_SECRX_KEYNUM_KEYSTATRX_FIELD_MASK,
    0,
    XIF_SECRX_KEYNUM_KEYSTATRX_FIELD_WIDTH,
    XIF_SECRX_KEYNUM_KEYSTATRX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XIF_SECRX_ENCRYPT_ENCRSTATRX
 ******************************************************************************/
const ru_field_rec XIF_SECRX_ENCRYPT_ENCRSTATRX_FIELD =
{
    "ENCRSTATRX",
#if RU_INCLUDE_DESC
    "",
    "Encryption stat.",
#endif
    XIF_SECRX_ENCRYPT_ENCRSTATRX_FIELD_MASK,
    0,
    XIF_SECRX_ENCRYPT_ENCRSTATRX_FIELD_WIDTH,
    XIF_SECRX_ENCRYPT_ENCRSTATRX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XIF_PMC_FRAME_RX_CNT_PMCRXFRAMECNT
 ******************************************************************************/
const ru_field_rec XIF_PMC_FRAME_RX_CNT_PMCRXFRAMECNT_FIELD =
{
    "PMCRXFRAMECNT",
#if RU_INCLUDE_DESC
    "",
    "Frame count stat.  Peg at max value.",
#endif
    XIF_PMC_FRAME_RX_CNT_PMCRXFRAMECNT_FIELD_MASK,
    0,
    XIF_PMC_FRAME_RX_CNT_PMCRXFRAMECNT_FIELD_WIDTH,
    XIF_PMC_FRAME_RX_CNT_PMCRXFRAMECNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XIF_PMC_BYTE_RX_CNT_PMCRXBYTECNT
 ******************************************************************************/
const ru_field_rec XIF_PMC_BYTE_RX_CNT_PMCRXBYTECNT_FIELD =
{
    "PMCRXBYTECNT",
#if RU_INCLUDE_DESC
    "",
    "Byte count stat.  Peg at max value.",
#endif
    XIF_PMC_BYTE_RX_CNT_PMCRXBYTECNT_FIELD_MASK,
    0,
    XIF_PMC_BYTE_RX_CNT_PMCRXBYTECNT_FIELD_WIDTH,
    XIF_PMC_BYTE_RX_CNT_PMCRXBYTECNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XIF_PMC_RUNT_RX_CNT_PMCRXRUNTCNT
 ******************************************************************************/
const ru_field_rec XIF_PMC_RUNT_RX_CNT_PMCRXRUNTCNT_FIELD =
{
    "PMCRXRUNTCNT",
#if RU_INCLUDE_DESC
    "",
    "Runt count stat.  Peg at max value.",
#endif
    XIF_PMC_RUNT_RX_CNT_PMCRXRUNTCNT_FIELD_MASK,
    0,
    XIF_PMC_RUNT_RX_CNT_PMCRXRUNTCNT_FIELD_WIDTH,
    XIF_PMC_RUNT_RX_CNT_PMCRXRUNTCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XIF_PMC_CW_ERR_RX_CNT_PMCRXCWERRCNT
 ******************************************************************************/
const ru_field_rec XIF_PMC_CW_ERR_RX_CNT_PMCRXCWERRCNT_FIELD =
{
    "PMCRXCWERRCNT",
#if RU_INCLUDE_DESC
    "",
    "Codeword error stat.  Peg at max value.",
#endif
    XIF_PMC_CW_ERR_RX_CNT_PMCRXCWERRCNT_FIELD_MASK,
    0,
    XIF_PMC_CW_ERR_RX_CNT_PMCRXCWERRCNT_FIELD_WIDTH,
    XIF_PMC_CW_ERR_RX_CNT_PMCRXCWERRCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XIF_PMC_CRC8_ERR_RX_CNT_PMCRXCRC8ERRCNT
 ******************************************************************************/
const ru_field_rec XIF_PMC_CRC8_ERR_RX_CNT_PMCRXCRC8ERRCNT_FIELD =
{
    "PMCRXCRC8ERRCNT",
#if RU_INCLUDE_DESC
    "",
    "CRC-8 error stat.  Peg at max value.",
#endif
    XIF_PMC_CRC8_ERR_RX_CNT_PMCRXCRC8ERRCNT_FIELD_MASK,
    0,
    XIF_PMC_CRC8_ERR_RX_CNT_PMCRXCRC8ERRCNT_FIELD_WIDTH,
    XIF_PMC_CRC8_ERR_RX_CNT_PMCRXCRC8ERRCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XIF_XPN_DATA_FRM_CNT_XPNDTFRAMECNT
 ******************************************************************************/
const ru_field_rec XIF_XPN_DATA_FRM_CNT_XPNDTFRAMECNT_FIELD =
{
    "XPNDTFRAMECNT",
#if RU_INCLUDE_DESC
    "",
    "Data frame count stat, excluding MPCP/OAM.  Peg at max.",
#endif
    XIF_XPN_DATA_FRM_CNT_XPNDTFRAMECNT_FIELD_MASK,
    0,
    XIF_XPN_DATA_FRM_CNT_XPNDTFRAMECNT_FIELD_WIDTH,
    XIF_XPN_DATA_FRM_CNT_XPNDTFRAMECNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XIF_XPN_DATA_BYTE_CNT_XPNDTBYTECNT
 ******************************************************************************/
const ru_field_rec XIF_XPN_DATA_BYTE_CNT_XPNDTBYTECNT_FIELD =
{
    "XPNDTBYTECNT",
#if RU_INCLUDE_DESC
    "",
    "Data byte count stat, excluding MPCP/OAM.  Peg at max.",
#endif
    XIF_XPN_DATA_BYTE_CNT_XPNDTBYTECNT_FIELD_MASK,
    0,
    XIF_XPN_DATA_BYTE_CNT_XPNDTBYTECNT_FIELD_WIDTH,
    XIF_XPN_DATA_BYTE_CNT_XPNDTBYTECNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XIF_XPN_MPCP_FRM_CNT_XPNMPCPFRAMECNT
 ******************************************************************************/
const ru_field_rec XIF_XPN_MPCP_FRM_CNT_XPNMPCPFRAMECNT_FIELD =
{
    "XPNMPCPFRAMECNT",
#if RU_INCLUDE_DESC
    "",
    "MPCP frame count stat.  Peg at max.",
#endif
    XIF_XPN_MPCP_FRM_CNT_XPNMPCPFRAMECNT_FIELD_MASK,
    0,
    XIF_XPN_MPCP_FRM_CNT_XPNMPCPFRAMECNT_FIELD_WIDTH,
    XIF_XPN_MPCP_FRM_CNT_XPNMPCPFRAMECNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XIF_XPN_OAM_FRM_CNT_XPNOAMFRAMECNT
 ******************************************************************************/
const ru_field_rec XIF_XPN_OAM_FRM_CNT_XPNOAMFRAMECNT_FIELD =
{
    "XPNOAMFRAMECNT",
#if RU_INCLUDE_DESC
    "",
    "MPCP frame count stat.  Peg at max.",
#endif
    XIF_XPN_OAM_FRM_CNT_XPNOAMFRAMECNT_FIELD_MASK,
    0,
    XIF_XPN_OAM_FRM_CNT_XPNOAMFRAMECNT_FIELD_WIDTH,
    XIF_XPN_OAM_FRM_CNT_XPNOAMFRAMECNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XIF_XPN_OAM_BYTE_CNT_XPNOAMBYTECNT
 ******************************************************************************/
const ru_field_rec XIF_XPN_OAM_BYTE_CNT_XPNOAMBYTECNT_FIELD =
{
    "XPNOAMBYTECNT",
#if RU_INCLUDE_DESC
    "",
    "OAM byte count stat.  Peg at max.",
#endif
    XIF_XPN_OAM_BYTE_CNT_XPNOAMBYTECNT_FIELD_MASK,
    0,
    XIF_XPN_OAM_BYTE_CNT_XPNOAMBYTECNT_FIELD_WIDTH,
    XIF_XPN_OAM_BYTE_CNT_XPNOAMBYTECNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XIF_XPN_OVERSIZE_FRM_CNT_XPNDTOVERSIZECNT
 ******************************************************************************/
const ru_field_rec XIF_XPN_OVERSIZE_FRM_CNT_XPNDTOVERSIZECNT_FIELD =
{
    "XPNDTOVERSIZECNT",
#if RU_INCLUDE_DESC
    "",
    "Oversize frame, as defined by XIF_XPN_OVERSIZE_THRESH register.",
#endif
    XIF_XPN_OVERSIZE_FRM_CNT_XPNDTOVERSIZECNT_FIELD_MASK,
    0,
    XIF_XPN_OVERSIZE_FRM_CNT_XPNDTOVERSIZECNT_FIELD_WIDTH,
    XIF_XPN_OVERSIZE_FRM_CNT_XPNDTOVERSIZECNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XIF_SEC_ABORT_FRM_CNT_SECRXABORTFRMCNT
 ******************************************************************************/
const ru_field_rec XIF_SEC_ABORT_FRM_CNT_SECRXABORTFRMCNT_FIELD =
{
    "SECRXABORTFRMCNT",
#if RU_INCLUDE_DESC
    "",
    "Abort frame stat.  Peg at max.",
#endif
    XIF_SEC_ABORT_FRM_CNT_SECRXABORTFRMCNT_FIELD_MASK,
    0,
    XIF_SEC_ABORT_FRM_CNT_SECRXABORTFRMCNT_FIELD_WIDTH,
    XIF_SEC_ABORT_FRM_CNT_SECRXABORTFRMCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XIF_LLID__RESERVED0
 ******************************************************************************/
const ru_field_rec XIF_LLID__RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_LLID__RESERVED0_FIELD_MASK,
    0,
    XIF_LLID__RESERVED0_FIELD_WIDTH,
    XIF_LLID__RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_LLID__CFGONULLID
 ******************************************************************************/
const ru_field_rec XIF_LLID__CFGONULLID_FIELD =
{
    "CFGONULLID",
#if RU_INCLUDE_DESC
    "",
    "Defines the 16-bits LLID for index i : [15:0] - LLID i; [16] -"
    "enable LLID index i.",
#endif
    XIF_LLID__CFGONULLID_FIELD_MASK,
    0,
    XIF_LLID__CFGONULLID_FIELD_WIDTH,
    XIF_LLID__CFGONULLID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_MAX_MPCP_UPDATE_CFGMAXPOSMPCPUPD
 ******************************************************************************/
const ru_field_rec XIF_MAX_MPCP_UPDATE_CFGMAXPOSMPCPUPD_FIELD =
{
    "CFGMAXPOSMPCPUPD",
#if RU_INCLUDE_DESC
    "",
    "Maximum MPCP update value.",
#endif
    XIF_MAX_MPCP_UPDATE_CFGMAXPOSMPCPUPD_FIELD_MASK,
    0,
    XIF_MAX_MPCP_UPDATE_CFGMAXPOSMPCPUPD_FIELD_WIDTH,
    XIF_MAX_MPCP_UPDATE_CFGMAXPOSMPCPUPD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_IPG_INSERTION_RESERVED0
 ******************************************************************************/
const ru_field_rec XIF_IPG_INSERTION_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_IPG_INSERTION_RESERVED0_FIELD_MASK,
    0,
    XIF_IPG_INSERTION_RESERVED0_FIELD_WIDTH,
    XIF_IPG_INSERTION_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_IPG_INSERTION_CFGSHORTIPG
 ******************************************************************************/
const ru_field_rec XIF_IPG_INSERTION_CFGSHORTIPG_FIELD =
{
    "CFGSHORTIPG",
#if RU_INCLUDE_DESC
    "",
    "Enable short IPG insertion, average of 8 bytes.  Should only be"
    "enabled only in FEC mode.  Otherwise, average of 12 bytes is"
    "inserted.",
#endif
    XIF_IPG_INSERTION_CFGSHORTIPG_FIELD_MASK,
    0,
    XIF_IPG_INSERTION_CFGSHORTIPG_FIELD_WIDTH,
    XIF_IPG_INSERTION_CFGSHORTIPG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_IPG_INSERTION_CFGINSERTIPG
 ******************************************************************************/
const ru_field_rec XIF_IPG_INSERTION_CFGINSERTIPG_FIELD =
{
    "CFGINSERTIPG",
#if RU_INCLUDE_DESC
    "",
    "Debug function to enable IPG insertion.",
#endif
    XIF_IPG_INSERTION_CFGINSERTIPG_FIELD_MASK,
    0,
    XIF_IPG_INSERTION_CFGINSERTIPG_FIELD_WIDTH,
    XIF_IPG_INSERTION_CFGINSERTIPG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_IPG_INSERTION_RESERVED1
 ******************************************************************************/
const ru_field_rec XIF_IPG_INSERTION_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_IPG_INSERTION_RESERVED1_FIELD_MASK,
    0,
    XIF_IPG_INSERTION_RESERVED1_FIELD_WIDTH,
    XIF_IPG_INSERTION_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_IPG_INSERTION_CFGIPGWORD
 ******************************************************************************/
const ru_field_rec XIF_IPG_INSERTION_CFGIPGWORD_FIELD =
{
    "CFGIPGWORD",
#if RU_INCLUDE_DESC
    "",
    "Configure the number of IPG word (2 bytes) to insert.  Only valid"
    "when cfgInsertIpg is asserted.",
#endif
    XIF_IPG_INSERTION_CFGIPGWORD_FIELD_MASK,
    0,
    XIF_IPG_INSERTION_CFGIPGWORD_FIELD_WIDTH,
    XIF_IPG_INSERTION_CFGIPGWORD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_TRANSPORT_TIME_CFTRANSPORTTIME
 ******************************************************************************/
const ru_field_rec XIF_TRANSPORT_TIME_CFTRANSPORTTIME_FIELD =
{
    "CFTRANSPORTTIME",
#if RU_INCLUDE_DESC
    "",
    "PPS is generated when the current MPCP is equal to the programmed"
    "value.",
#endif
    XIF_TRANSPORT_TIME_CFTRANSPORTTIME_FIELD_MASK,
    0,
    XIF_TRANSPORT_TIME_CFTRANSPORTTIME_FIELD_WIDTH,
    XIF_TRANSPORT_TIME_CFTRANSPORTTIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_MPCP_TIME_CURMPCPTS
 ******************************************************************************/
const ru_field_rec XIF_MPCP_TIME_CURMPCPTS_FIELD =
{
    "CURMPCPTS",
#if RU_INCLUDE_DESC
    "",
    "Current MPCP time.",
#endif
    XIF_MPCP_TIME_CURMPCPTS_FIELD_MASK,
    0,
    XIF_MPCP_TIME_CURMPCPTS_FIELD_WIDTH,
    XIF_MPCP_TIME_CURMPCPTS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XIF_OVERLAP_GNT_OH_CFGOVRLPOH
 ******************************************************************************/
const ru_field_rec XIF_OVERLAP_GNT_OH_CFGOVRLPOH_FIELD =
{
    "CFGOVRLPOH",
#if RU_INCLUDE_DESC
    "",
    "Provides the amount the laser_on time and laser_off time may"
    "overlap.",
#endif
    XIF_OVERLAP_GNT_OH_CFGOVRLPOH_FIELD_MASK,
    0,
    XIF_OVERLAP_GNT_OH_CFGOVRLPOH_FIELD_WIDTH,
    XIF_OVERLAP_GNT_OH_CFGOVRLPOH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_MAC_MODE_RESERVED0
 ******************************************************************************/
const ru_field_rec XIF_MAC_MODE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_MAC_MODE_RESERVED0_FIELD_MASK,
    0,
    XIF_MAC_MODE_RESERVED0_FIELD_WIDTH,
    XIF_MAC_MODE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_MAC_MODE_CFGENNOGNTXMT
 ******************************************************************************/
const ru_field_rec XIF_MAC_MODE_CFGENNOGNTXMT_FIELD =
{
    "CFGENNOGNTXMT",
#if RU_INCLUDE_DESC
    "",
    "Enable point-2-point transmission without grant. Must also set bit"
    "cfgPmcTxPreNotPon.",
#endif
    XIF_MAC_MODE_CFGENNOGNTXMT_FIELD_MASK,
    0,
    XIF_MAC_MODE_CFGENNOGNTXMT_FIELD_WIDTH,
    XIF_MAC_MODE_CFGENNOGNTXMT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_PMCTX_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec XIF_PMCTX_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_PMCTX_CTL_RESERVED0_FIELD_MASK,
    0,
    XIF_PMCTX_CTL_RESERVED0_FIELD_WIDTH,
    XIF_PMCTX_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_PMCTX_CTL_CFGMPCPUPDPERIOD
 ******************************************************************************/
const ru_field_rec XIF_PMCTX_CTL_CFGMPCPUPDPERIOD_FIELD =
{
    "CFGMPCPUPDPERIOD",
#if RU_INCLUDE_DESC
    "",
    "Define the MPCP update period. A value of 0xff disables update.",
#endif
    XIF_PMCTX_CTL_CFGMPCPUPDPERIOD_FIELD_MASK,
    0,
    XIF_PMCTX_CTL_CFGMPCPUPDPERIOD_FIELD_WIDTH,
    XIF_PMCTX_CTL_CFGMPCPUPDPERIOD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_PMCTX_CTL_RESERVED1
 ******************************************************************************/
const ru_field_rec XIF_PMCTX_CTL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_PMCTX_CTL_RESERVED1_FIELD_MASK,
    0,
    XIF_PMCTX_CTL_RESERVED1_FIELD_WIDTH,
    XIF_PMCTX_CTL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_PMCTX_CTL_CFGDIS4IDLEB4STARTCHAR
 ******************************************************************************/
const ru_field_rec XIF_PMCTX_CTL_CFGDIS4IDLEB4STARTCHAR_FIELD =
{
    "CFGDIS4IDLEB4STARTCHAR",
#if RU_INCLUDE_DESC
    "",
    "Disable the requirement of 4 IDLEs preceeding start character to"
    "consider packet valid.",
#endif
    XIF_PMCTX_CTL_CFGDIS4IDLEB4STARTCHAR_FIELD_MASK,
    0,
    XIF_PMCTX_CTL_CFGDIS4IDLEB4STARTCHAR_FIELD_WIDTH,
    XIF_PMCTX_CTL_CFGDIS4IDLEB4STARTCHAR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_PMCTX_CTL_CFGENIDLEDSCRD
 ******************************************************************************/
const ru_field_rec XIF_PMCTX_CTL_CFGENIDLEDSCRD_FIELD =
{
    "CFGENIDLEDSCRD",
#if RU_INCLUDE_DESC
    "",
    "Enable upstream IDLE discard",
#endif
    XIF_PMCTX_CTL_CFGENIDLEDSCRD_FIELD_MASK,
    0,
    XIF_PMCTX_CTL_CFGENIDLEDSCRD_FIELD_WIDTH,
    XIF_PMCTX_CTL_CFGENIDLEDSCRD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_PMCTX_CTL_CFGSELTXPONTIME
 ******************************************************************************/
const ru_field_rec XIF_PMCTX_CTL_CFGSELTXPONTIME_FIELD =
{
    "CFGSELTXPONTIME",
#if RU_INCLUDE_DESC
    "",
    "Selects the source of transmit MPCP time : 0 - RX; 1 - TX.",
#endif
    XIF_PMCTX_CTL_CFGSELTXPONTIME_FIELD_MASK,
    0,
    XIF_PMCTX_CTL_CFGSELTXPONTIME_FIELD_WIDTH,
    XIF_PMCTX_CTL_CFGSELTXPONTIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_PMCTX_CTL_CFGMPCPCONTUPD
 ******************************************************************************/
const ru_field_rec XIF_PMCTX_CTL_CFGMPCPCONTUPD_FIELD =
{
    "CFGMPCPCONTUPD",
#if RU_INCLUDE_DESC
    "",
    "Enable continous MPCP update.",
#endif
    XIF_PMCTX_CTL_CFGMPCPCONTUPD_FIELD_MASK,
    0,
    XIF_PMCTX_CTL_CFGMPCPCONTUPD_FIELD_WIDTH,
    XIF_PMCTX_CTL_CFGMPCPCONTUPD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_PMCTX_CTL_CFGENMAXMPCPUPD
 ******************************************************************************/
const ru_field_rec XIF_PMCTX_CTL_CFGENMAXMPCPUPD_FIELD =
{
    "CFGENMAXMPCPUPD",
#if RU_INCLUDE_DESC
    "",
    "Enable the restriction of positive MPCP update, limitted by"
    "cfgMaxPosMpcpUpd value set in register XIF_MAX_MPCP_UPDATE.",
#endif
    XIF_PMCTX_CTL_CFGENMAXMPCPUPD_FIELD_MASK,
    0,
    XIF_PMCTX_CTL_CFGENMAXMPCPUPD_FIELD_WIDTH,
    XIF_PMCTX_CTL_CFGENMAXMPCPUPD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_PMCTX_CTL_CFGENNEGTIMEABORT
 ******************************************************************************/
const ru_field_rec XIF_PMCTX_CTL_CFGENNEGTIMEABORT_FIELD =
{
    "CFGENNEGTIMEABORT",
#if RU_INCLUDE_DESC
    "",
    "Enable the discard of packet with negative scheduled transmit time,"
    "relative to the current MPCP.  This is fatal since it indicates"
    "error with scheduled transmit time.",
#endif
    XIF_PMCTX_CTL_CFGENNEGTIMEABORT_FIELD_MASK,
    0,
    XIF_PMCTX_CTL_CFGENNEGTIMEABORT_FIELD_WIDTH,
    XIF_PMCTX_CTL_CFGENNEGTIMEABORT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_SEC_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec XIF_SEC_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_SEC_CTL_RESERVED0_FIELD_MASK,
    0,
    XIF_SEC_CTL_RESERVED0_FIELD_WIDTH,
    XIF_SEC_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_SEC_CTL_CFGSECRXENSHORTLEN
 ******************************************************************************/
const ru_field_rec XIF_SEC_CTL_CFGSECRXENSHORTLEN_FIELD =
{
    "CFGSECRXENSHORTLEN",
#if RU_INCLUDE_DESC
    "",
    "[A0 BUG] - HWBCM6858-457"
    ""
    "Enables downstream short length support. This feature cannot be"
    "supported in"
    "A0. The workaround would be to disable this feature by clearing the"
    "bit.",
#endif
    XIF_SEC_CTL_CFGSECRXENSHORTLEN_FIELD_MASK,
    0,
    XIF_SEC_CTL_CFGSECRXENSHORTLEN_FIELD_WIDTH,
    XIF_SEC_CTL_CFGSECRXENSHORTLEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_SEC_CTL_CFGENSECTXFAKEAES
 ******************************************************************************/
const ru_field_rec XIF_SEC_CTL_CFGENSECTXFAKEAES_FIELD =
{
    "CFGENSECTXFAKEAES",
#if RU_INCLUDE_DESC
    "",
    "Enable fake AES on TX security.",
#endif
    XIF_SEC_CTL_CFGENSECTXFAKEAES_FIELD_MASK,
    0,
    XIF_SEC_CTL_CFGENSECTXFAKEAES_FIELD_WIDTH,
    XIF_SEC_CTL_CFGENSECTXFAKEAES_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_SEC_CTL_CFGENSECRXFAKEAES
 ******************************************************************************/
const ru_field_rec XIF_SEC_CTL_CFGENSECRXFAKEAES_FIELD =
{
    "CFGENSECRXFAKEAES",
#if RU_INCLUDE_DESC
    "",
    "Enable fake AES on RX security.",
#endif
    XIF_SEC_CTL_CFGENSECRXFAKEAES_FIELD_MASK,
    0,
    XIF_SEC_CTL_CFGENSECRXFAKEAES_FIELD_WIDTH,
    XIF_SEC_CTL_CFGENSECRXFAKEAES_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_SEC_CTL_CFGSECRXENPKTNUMRLOVR
 ******************************************************************************/
const ru_field_rec XIF_SEC_CTL_CFGSECRXENPKTNUMRLOVR_FIELD =
{
    "CFGSECRXENPKTNUMRLOVR",
#if RU_INCLUDE_DESC
    "",
    "Enables packet number rollover on receive.",
#endif
    XIF_SEC_CTL_CFGSECRXENPKTNUMRLOVR_FIELD_MASK,
    0,
    XIF_SEC_CTL_CFGSECRXENPKTNUMRLOVR_FIELD_WIDTH,
    XIF_SEC_CTL_CFGSECRXENPKTNUMRLOVR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_SEC_CTL_RESERVED1
 ******************************************************************************/
const ru_field_rec XIF_SEC_CTL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_SEC_CTL_RESERVED1_FIELD_MASK,
    0,
    XIF_SEC_CTL_RESERVED1_FIELD_WIDTH,
    XIF_SEC_CTL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_SEC_CTL_CFGSECTXENPKTNUMRLOVR
 ******************************************************************************/
const ru_field_rec XIF_SEC_CTL_CFGSECTXENPKTNUMRLOVR_FIELD =
{
    "CFGSECTXENPKTNUMRLOVR",
#if RU_INCLUDE_DESC
    "",
    "Enables packet number rollover on transmit.",
#endif
    XIF_SEC_CTL_CFGSECTXENPKTNUMRLOVR_FIELD_MASK,
    0,
    XIF_SEC_CTL_CFGSECTXENPKTNUMRLOVR_FIELD_WIDTH,
    XIF_SEC_CTL_CFGSECTXENPKTNUMRLOVR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_SEC_CTL_CFGENAEREPLAYPRCT
 ******************************************************************************/
const ru_field_rec XIF_SEC_CTL_CFGENAEREPLAYPRCT_FIELD =
{
    "CFGENAEREPLAYPRCT",
#if RU_INCLUDE_DESC
    "",
    "Enables replay protection on RX security.",
#endif
    XIF_SEC_CTL_CFGENAEREPLAYPRCT_FIELD_MASK,
    0,
    XIF_SEC_CTL_CFGENAEREPLAYPRCT_FIELD_WIDTH,
    XIF_SEC_CTL_CFGENAEREPLAYPRCT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_AE_PKTNUM_WINDOW_CFGAEPKTNUMWND
 ******************************************************************************/
const ru_field_rec XIF_AE_PKTNUM_WINDOW_CFGAEPKTNUMWND_FIELD =
{
    "CFGAEPKTNUMWND",
#if RU_INCLUDE_DESC
    "",
    "In replay protection, the packet number is checked against the"
    "expected packet number.  If it is greater than or equal to, packet"
    "will be accepted.  Otherwise, it will be discarded. This register"
    "provides the tolerance by subtracting the current expected packet"
    "number by this amount.",
#endif
    XIF_AE_PKTNUM_WINDOW_CFGAEPKTNUMWND_FIELD_MASK,
    0,
    XIF_AE_PKTNUM_WINDOW_CFGAEPKTNUMWND_FIELD_WIDTH,
    XIF_AE_PKTNUM_WINDOW_CFGAEPKTNUMWND_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_AE_PKTNUM_THRESH_CFGPKTNUMMAXTHRESH
 ******************************************************************************/
const ru_field_rec XIF_AE_PKTNUM_THRESH_CFGPKTNUMMAXTHRESH_FIELD =
{
    "CFGPKTNUMMAXTHRESH",
#if RU_INCLUDE_DESC
    "",
    "Defines the threshold of impending packet number rollover.",
#endif
    XIF_AE_PKTNUM_THRESH_CFGPKTNUMMAXTHRESH_FIELD_MASK,
    0,
    XIF_AE_PKTNUM_THRESH_CFGPKTNUMMAXTHRESH_FIELD_WIDTH,
    XIF_AE_PKTNUM_THRESH_CFGPKTNUMMAXTHRESH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XIF_SECTX_KEYNUM_KEYSTATTX
 ******************************************************************************/
const ru_field_rec XIF_SECTX_KEYNUM_KEYSTATTX_FIELD =
{
    "KEYSTATTX",
#if RU_INCLUDE_DESC
    "",
    "KeyNumber stat",
#endif
    XIF_SECTX_KEYNUM_KEYSTATTX_FIELD_MASK,
    0,
    XIF_SECTX_KEYNUM_KEYSTATTX_FIELD_WIDTH,
    XIF_SECTX_KEYNUM_KEYSTATTX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XIF_SECTX_ENCRYPT_ENCRSTATTX
 ******************************************************************************/
const ru_field_rec XIF_SECTX_ENCRYPT_ENCRSTATTX_FIELD =
{
    "ENCRSTATTX",
#if RU_INCLUDE_DESC
    "",
    "Encryption stat.",
#endif
    XIF_SECTX_ENCRYPT_ENCRSTATTX_FIELD_MASK,
    0,
    XIF_SECTX_ENCRYPT_ENCRSTATTX_FIELD_WIDTH,
    XIF_SECTX_ENCRYPT_ENCRSTATTX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XIF_AE_PKTNUM_STAT_RESERVED0
 ******************************************************************************/
const ru_field_rec XIF_AE_PKTNUM_STAT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_AE_PKTNUM_STAT_RESERVED0_FIELD_MASK,
    0,
    XIF_AE_PKTNUM_STAT_RESERVED0_FIELD_WIDTH,
    XIF_AE_PKTNUM_STAT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XIF_AE_PKTNUM_STAT_SECTXINDXWTPKTNUMMAX
 ******************************************************************************/
const ru_field_rec XIF_AE_PKTNUM_STAT_SECTXINDXWTPKTNUMMAX_FIELD =
{
    "SECTXINDXWTPKTNUMMAX",
#if RU_INCLUDE_DESC
    "",
    "Provides the LLID index whose packet number exceeded the maximum"
    "packet number threhsold.",
#endif
    XIF_AE_PKTNUM_STAT_SECTXINDXWTPKTNUMMAX_FIELD_MASK,
    0,
    XIF_AE_PKTNUM_STAT_SECTXINDXWTPKTNUMMAX_FIELD_WIDTH,
    XIF_AE_PKTNUM_STAT_SECTXINDXWTPKTNUMMAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XIF_AE_PKTNUM_STAT_RESERVED1
 ******************************************************************************/
const ru_field_rec XIF_AE_PKTNUM_STAT_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XIF_AE_PKTNUM_STAT_RESERVED1_FIELD_MASK,
    0,
    XIF_AE_PKTNUM_STAT_RESERVED1_FIELD_WIDTH,
    XIF_AE_PKTNUM_STAT_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XIF_AE_PKTNUM_STAT_SECRXINDXWTPKTNUMABORT
 ******************************************************************************/
const ru_field_rec XIF_AE_PKTNUM_STAT_SECRXINDXWTPKTNUMABORT_FIELD =
{
    "SECRXINDXWTPKTNUMABORT",
#if RU_INCLUDE_DESC
    "",
    "Provides the LLID index that was aborted due to replay protection.",
#endif
    XIF_AE_PKTNUM_STAT_SECRXINDXWTPKTNUMABORT_FIELD_MASK,
    0,
    XIF_AE_PKTNUM_STAT_SECRXINDXWTPKTNUMABORT_FIELD_WIDTH,
    XIF_AE_PKTNUM_STAT_SECRXINDXWTPKTNUMABORT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XIF_MPCP_UPDATE_MPCPUPDPERIOD
 ******************************************************************************/
const ru_field_rec XIF_MPCP_UPDATE_MPCPUPDPERIOD_FIELD =
{
    "MPCPUPDPERIOD",
#if RU_INCLUDE_DESC
    "",
    "Time between MPCP updates.",
#endif
    XIF_MPCP_UPDATE_MPCPUPDPERIOD_FIELD_MASK,
    0,
    XIF_MPCP_UPDATE_MPCPUPDPERIOD_FIELD_WIDTH,
    XIF_MPCP_UPDATE_MPCPUPDPERIOD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XIF_BURST_PRELAUNCH_OFFSET_CFGBURSTPRELAUNCHOFFSET
 ******************************************************************************/
const ru_field_rec XIF_BURST_PRELAUNCH_OFFSET_CFGBURSTPRELAUNCHOFFSET_FIELD =
{
    "CFGBURSTPRELAUNCHOFFSET",
#if RU_INCLUDE_DESC
    "",
    "Defines the prelaunch time of burst data, in unit of TQ.",
#endif
    XIF_BURST_PRELAUNCH_OFFSET_CFGBURSTPRELAUNCHOFFSET_FIELD_MASK,
    0,
    XIF_BURST_PRELAUNCH_OFFSET_CFGBURSTPRELAUNCHOFFSET_FIELD_WIDTH,
    XIF_BURST_PRELAUNCH_OFFSET_CFGBURSTPRELAUNCHOFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: XIF_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_CTL_FIELDS[] =
{
    &XIF_CTL_RXENCRYPTEN_FIELD,
    &XIF_CTL_CFGDISRXDASAENCRPT_FIELD,
    &XIF_CTL_RXENCRYPTMODE_FIELD,
    &XIF_CTL_TXENCRYPTEN_FIELD,
    &XIF_CTL_CFGDISTXDASAENCRPT_FIELD,
    &XIF_CTL_TXENCRYPTMODE_FIELD,
    &XIF_CTL_RESERVED0_FIELD,
    &XIF_CTL_CFGLLIDMODEMSK_FIELD,
    &XIF_CTL_CFGXPNBADCRC32_FIELD,
    &XIF_CTL_CFGDISDISCINFO_FIELD,
    &XIF_CTL_CFGPMCTX2RXLPBK_FIELD,
    &XIF_CTL_CFGPMCTXENCRC8BAD_FIELD,
    &XIF_CTL_CFGENP2P_FIELD,
    &XIF_CTL_RESERVED1_FIELD,
    &XIF_CTL_CFGPMCRXENCRC8CHK_FIELD,
    &XIF_CTL_RESERVED2_FIELD,
    &XIF_CTL_CFGFECEN_FIELD,
    &XIF_CTL_CFGLEGACYRCVTSUPD_FIELD,
    &XIF_CTL_CFGXPNENCRCPASSTHRU_FIELD,
    &XIF_CTL_CFGXPNDISTIMESTAMPMOD_FIELD,
    &XIF_CTL_XIFNOTRDY_FIELD,
    &XIF_CTL_XIFDTPORTRSTN_FIELD,
    &XIF_CTL_XPNTXRSTN_FIELD,
    &XIF_CTL_PMCTXRSTN_FIELD,
    &XIF_CTL_SECTXRSTN_FIELD,
    &XIF_CTL_CFGDISTXOAMENCRPT_FIELD,
    &XIF_CTL_CFGDISTXMPCPENCRPT_FIELD,
    &XIF_CTL_PMCRXRSTN_FIELD,
    &XIF_CTL_SECRXRSTN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_CTL_REG = 
{
    "CTL",
#if RU_INCLUDE_DESC
    "XIF_CTL Register",
    "XIF control register.",
#endif
    XIF_CTL_REG_OFFSET,
    0,
    0,
    226,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    29,
    XIF_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_INT_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_INT_STATUS_FIELDS[] =
{
    &XIF_INT_STATUS_RESERVED0_FIELD,
    &XIF_INT_STATUS_SECRXRPLYPRTCTABRTINT_FIELD,
    &XIF_INT_STATUS_SECTXPKTNUMMAXINT_FIELD,
    &XIF_INT_STATUS_RESERVED1_FIELD,
    &XIF_INT_STATUS_TSFULLUPDINT_FIELD,
    &XIF_INT_STATUS_TXHANGINT_FIELD,
    &XIF_INT_STATUS_NEGTIMEINT_FIELD,
    &XIF_INT_STATUS_PMCTSJTTRINT_FIELD,
    &XIF_INT_STATUS_SECRXOUTFFOVRFLWINT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_INT_STATUS_REG = 
{
    "INT_STATUS",
#if RU_INCLUDE_DESC
    "XIF_INT_STATUS Register",
    "Interrupts.",
#endif
    XIF_INT_STATUS_REG_OFFSET,
    0,
    0,
    227,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    XIF_INT_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_INT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_INT_MASK_FIELDS[] =
{
    &XIF_INT_MASK_RESERVED0_FIELD,
    &XIF_INT_MASK_MSKSECRXREPLAYPROTCTABORT_FIELD,
    &XIF_INT_MASK_MSKPKTNUMTHRESHINT_FIELD,
    &XIF_INT_MASK_RESERVED1_FIELD,
    &XIF_INT_MASK_MSKTSFULLUPDINT_FIELD,
    &XIF_INT_MASK_MSKTXHANGINT_FIELD,
    &XIF_INT_MASK_MSKNEGTIMEINT_FIELD,
    &XIF_INT_MASK_MSKPMCTSJTTRINT_FIELD,
    &XIF_INT_MASK_MSKSECRXOUTFFINT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_INT_MASK_REG = 
{
    "INT_MASK",
#if RU_INCLUDE_DESC
    "XIF_INT_MASK Register",
    "Interrupt masks, active low : 0 - mask interrupt; 1 - enable interrupt.",
#endif
    XIF_INT_MASK_REG_OFFSET,
    0,
    0,
    228,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    XIF_INT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_PORT_COMMAND
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_PORT_COMMAND_FIELDS[] =
{
    &XIF_PORT_COMMAND_DATAPORTBUSY_FIELD,
    &XIF_PORT_COMMAND_RESERVED0_FIELD,
    &XIF_PORT_COMMAND_PORTSELECT_FIELD,
    &XIF_PORT_COMMAND_PORTOPCODE_FIELD,
    &XIF_PORT_COMMAND_PORTADDRESS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_PORT_COMMAND_REG = 
{
    "PORT_COMMAND",
#if RU_INCLUDE_DESC
    "XIF_PORT_COMMAND Register",
    "Provides dataPort read/write access to various RAMs.",
#endif
    XIF_PORT_COMMAND_REG_OFFSET,
    0,
    0,
    229,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XIF_PORT_COMMAND_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_PORT_DATA_
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_PORT_DATA__FIELDS[] =
{
    &XIF_PORT_DATA__PORTDATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_PORT_DATA__REG = 
{
    "PORT_DATA_",
#if RU_INCLUDE_DESC
    "XIF_PORT_DATA %i Register",
    "Stores the pre-write data for writing; and the post-read data for"
    "reading.",
#endif
    XIF_PORT_DATA__REG_OFFSET,
    XIF_PORT_DATA__REG_RAM_CNT,
    4,
    230,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_PORT_DATA__FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_MACSEC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_MACSEC_FIELDS[] =
{
    &XIF_MACSEC_RESERVED0_FIELD,
    &XIF_MACSEC_CFGMACSECETHERTYPE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_MACSEC_REG = 
{
    "MACSEC",
#if RU_INCLUDE_DESC
    "XIF_MACSEC Register",
    "This register specifies the 802.1ae MacSec Ethertype to be inserted"
    "into the packet.",
#endif
    XIF_MACSEC_REG_OFFSET,
    0,
    0,
    231,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XIF_MACSEC_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_XPN_XMT_OFFSET
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_XPN_XMT_OFFSET_FIELDS[] =
{
    &XIF_XPN_XMT_OFFSET_RESERVED0_FIELD,
    &XIF_XPN_XMT_OFFSET_CFGXPNXMTOFFSET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_XPN_XMT_OFFSET_REG = 
{
    "XPN_XMT_OFFSET",
#if RU_INCLUDE_DESC
    "XIF_XPN_XMT_OFFSET Register",
    "Specifies the transmit offset, relative to the current MPCP.",
#endif
    XIF_XPN_XMT_OFFSET_REG_OFFSET,
    0,
    0,
    232,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XIF_XPN_XMT_OFFSET_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_XPN_TIMESTAMP_OFFSET
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_XPN_TIMESTAMP_OFFSET_FIELDS[] =
{
    &XIF_XPN_TIMESTAMP_OFFSET_CFGXPNMPCPTSOFFSET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_XPN_TIMESTAMP_OFFSET_REG = 
{
    "XPN_TIMESTAMP_OFFSET",
#if RU_INCLUDE_DESC
    "XIF_XPN_TIMESTAMP_OFFSET Register",
    "Specifies the offset to add to MPCP's timestamp.",
#endif
    XIF_XPN_TIMESTAMP_OFFSET_REG_OFFSET,
    0,
    0,
    233,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_XPN_TIMESTAMP_OFFSET_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_XPN_PKTGEN_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_XPN_PKTGEN_CTL_FIELDS[] =
{
    &XIF_XPN_PKTGEN_CTL_CFGONUBURSTSIZE_FIELD,
    &XIF_XPN_PKTGEN_CTL_RESERVED0_FIELD,
    &XIF_XPN_PKTGEN_CTL_CFGENBCK2BCKPKTGEN_FIELD,
    &XIF_XPN_PKTGEN_CTL_CFGENALLMPCPPKTGEN_FIELD,
    &XIF_XPN_PKTGEN_CTL_RESERVED1_FIELD,
    &XIF_XPN_PKTGEN_CTL_CFGXPNSTARTPKTGEN_FIELD,
    &XIF_XPN_PKTGEN_CTL_CFGXPNENPKTGEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_XPN_PKTGEN_CTL_REG = 
{
    "XPN_PKTGEN_CTL",
#if RU_INCLUDE_DESC
    "XIF_XPN_PKTGEN_CTL Register",
    "This register controls Xif's packet generator. When enabled the packet"
    "generator's frames will be inserted in place of the normal downstream"
    "data.",
#endif
    XIF_XPN_PKTGEN_CTL_REG_OFFSET,
    0,
    0,
    234,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    XIF_XPN_PKTGEN_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_XPN_PKTGEN_LLID
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_XPN_PKTGEN_LLID_FIELDS[] =
{
    &XIF_XPN_PKTGEN_LLID_CFGXPNPKTGENLLID1_FIELD,
    &XIF_XPN_PKTGEN_LLID_CFGXPNPKTGENLLID0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_XPN_PKTGEN_LLID_REG = 
{
    "XPN_PKTGEN_LLID",
#if RU_INCLUDE_DESC
    "XIF_XPN_PKTGEN_LLID Register",
    "Specifies the packet generation LLID for index 0 and 1.",
#endif
    XIF_XPN_PKTGEN_LLID_REG_OFFSET,
    0,
    0,
    235,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XIF_XPN_PKTGEN_LLID_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_XPN_PKTGEN_PKT_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_XPN_PKTGEN_PKT_CNT_FIELDS[] =
{
    &XIF_XPN_PKTGEN_PKT_CNT_CFGXPNPKTGENBURSTMODE_FIELD,
    &XIF_XPN_PKTGEN_PKT_CNT_RESERVED0_FIELD,
    &XIF_XPN_PKTGEN_PKT_CNT_CFGXPNPKTGENBURSTSIZE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_XPN_PKTGEN_PKT_CNT_REG = 
{
    "XPN_PKTGEN_PKT_CNT",
#if RU_INCLUDE_DESC
    "XIF_XPN_PKTGEN_PKT_CNT Register",
    "Specifies the number of packet to transmit.",
#endif
    XIF_XPN_PKTGEN_PKT_CNT_REG_OFFSET,
    0,
    0,
    236,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    XIF_XPN_PKTGEN_PKT_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_XPN_PKTGEN_PKT_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_XPN_PKTGEN_PKT_SIZE_FIELDS[] =
{
    &XIF_XPN_PKTGEN_PKT_SIZE_CFGXPNPKTGENSIZEINCR_FIELD,
    &XIF_XPN_PKTGEN_PKT_SIZE_RESERVED0_FIELD,
    &XIF_XPN_PKTGEN_PKT_SIZE_CFGXPNPKTGENSIZEEND_FIELD,
    &XIF_XPN_PKTGEN_PKT_SIZE_RESERVED1_FIELD,
    &XIF_XPN_PKTGEN_PKT_SIZE_CFGXPNPKTGENSIZESTART_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_XPN_PKTGEN_PKT_SIZE_REG = 
{
    "XPN_PKTGEN_PKT_SIZE",
#if RU_INCLUDE_DESC
    "XIF_XPN_PKTGEN_PKT_SIZE Register",
    "Specifies the size of each packet.",
#endif
    XIF_XPN_PKTGEN_PKT_SIZE_REG_OFFSET,
    0,
    0,
    237,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XIF_XPN_PKTGEN_PKT_SIZE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_XPN_PKTGEN_IPG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_XPN_PKTGEN_IPG_FIELDS[] =
{
    &XIF_XPN_PKTGEN_IPG_CFGXPNPKTGENBCK2BCKIPG_FIELD,
    &XIF_XPN_PKTGEN_IPG_CFGXPNPKTGENIPG_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_XPN_PKTGEN_IPG_REG = 
{
    "XPN_PKTGEN_IPG",
#if RU_INCLUDE_DESC
    "XIF_XPN_PKTGEN_IPG Register",
    "IPG insertion for packet generator.",
#endif
    XIF_XPN_PKTGEN_IPG_REG_OFFSET,
    0,
    0,
    238,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XIF_XPN_PKTGEN_IPG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_TS_JITTER_THRESH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_TS_JITTER_THRESH_FIELDS[] =
{
    &XIF_TS_JITTER_THRESH_RESERVED0_FIELD,
    &XIF_TS_JITTER_THRESH_CFGTSJTTRTHRESH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_TS_JITTER_THRESH_REG = 
{
    "TS_JITTER_THRESH",
#if RU_INCLUDE_DESC
    "XIF_TS_JITTER_THRESH Register",
    "Specifies the threshold to generate pmcRxTsJitter interrupt.",
#endif
    XIF_TS_JITTER_THRESH_REG_OFFSET,
    0,
    0,
    239,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XIF_TS_JITTER_THRESH_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_TS_UPDATE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_TS_UPDATE_FIELDS[] =
{
    &XIF_TS_UPDATE_CFGTSFULLUPDTHR_FIELD,
    &XIF_TS_UPDATE_CFGENAUTOTSUPD_FIELD,
    &XIF_TS_UPDATE_RESERVED0_FIELD,
    &XIF_TS_UPDATE_CFGTSUPDPER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_TS_UPDATE_REG = 
{
    "TS_UPDATE",
#if RU_INCLUDE_DESC
    "XIF_TS_UPDATE Register",
    "Provides timestamp update control.",
#endif
    XIF_TS_UPDATE_REG_OFFSET,
    0,
    0,
    240,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    XIF_TS_UPDATE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_GNT_OVERHEAD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_GNT_OVERHEAD_FIELDS[] =
{
    &XIF_GNT_OVERHEAD_RESERVED0_FIELD,
    &XIF_GNT_OVERHEAD_CFGGNTOH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_GNT_OVERHEAD_REG = 
{
    "GNT_OVERHEAD",
#if RU_INCLUDE_DESC
    "XIF_GNT_OVERHEAD Register",
    "Specifies the burst overhead for normal grant.",
#endif
    XIF_GNT_OVERHEAD_REG_OFFSET,
    0,
    0,
    241,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XIF_GNT_OVERHEAD_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_DISCOVER_OVERHEAD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_DISCOVER_OVERHEAD_FIELDS[] =
{
    &XIF_DISCOVER_OVERHEAD_RESERVED0_FIELD,
    &XIF_DISCOVER_OVERHEAD_CFGDISCOH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_DISCOVER_OVERHEAD_REG = 
{
    "DISCOVER_OVERHEAD",
#if RU_INCLUDE_DESC
    "XIF_DISCOVER_OVERHEAD Register",
    "Specifies the burst overhead for discovery grant.",
#endif
    XIF_DISCOVER_OVERHEAD_REG_OFFSET,
    0,
    0,
    242,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XIF_DISCOVER_OVERHEAD_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_DISCOVER_INFO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_DISCOVER_INFO_FIELDS[] =
{
    &XIF_DISCOVER_INFO_RESERVED0_FIELD,
    &XIF_DISCOVER_INFO_CFGDISCINFOFLD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_DISCOVER_INFO_REG = 
{
    "DISCOVER_INFO",
#if RU_INCLUDE_DESC
    "XIF_DISCOVER_INFO Register",
    "Specifies the discovery information field.",
#endif
    XIF_DISCOVER_INFO_REG_OFFSET,
    0,
    0,
    243,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XIF_DISCOVER_INFO_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_XPN_OVERSIZE_THRESH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_XPN_OVERSIZE_THRESH_FIELDS[] =
{
    &XIF_XPN_OVERSIZE_THRESH_RESERVED0_FIELD,
    &XIF_XPN_OVERSIZE_THRESH_CFGXPNOVRSZTHRESH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_XPN_OVERSIZE_THRESH_REG = 
{
    "XPN_OVERSIZE_THRESH",
#if RU_INCLUDE_DESC
    "XIF_XPN_OVERSIZE_THRESH Register",
    "Specifies the oversize threshold to increment oversize stat.",
#endif
    XIF_XPN_OVERSIZE_THRESH_REG_OFFSET,
    0,
    0,
    244,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XIF_XPN_OVERSIZE_THRESH_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_SECRX_KEYNUM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_SECRX_KEYNUM_FIELDS[] =
{
    &XIF_SECRX_KEYNUM_KEYSTATRX_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_SECRX_KEYNUM_REG = 
{
    "SECRX_KEYNUM",
#if RU_INCLUDE_DESC
    "XIF_SECRX_KEYNUM Register",
    "Provides downstream encryption key number stat, per LLID.",
#endif
    XIF_SECRX_KEYNUM_REG_OFFSET,
    0,
    0,
    245,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_SECRX_KEYNUM_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_SECRX_ENCRYPT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_SECRX_ENCRYPT_FIELDS[] =
{
    &XIF_SECRX_ENCRYPT_ENCRSTATRX_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_SECRX_ENCRYPT_REG = 
{
    "SECRX_ENCRYPT",
#if RU_INCLUDE_DESC
    "XIF_SECRX_ENCRYPT Register",
    "Provides downstream encryption stat, per LLID.",
#endif
    XIF_SECRX_ENCRYPT_REG_OFFSET,
    0,
    0,
    246,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_SECRX_ENCRYPT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_PMC_FRAME_RX_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_PMC_FRAME_RX_CNT_FIELDS[] =
{
    &XIF_PMC_FRAME_RX_CNT_PMCRXFRAMECNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_PMC_FRAME_RX_CNT_REG = 
{
    "PMC_FRAME_RX_CNT",
#if RU_INCLUDE_DESC
    "XIF_PMC_FRAME_RX_CNT Register",
    "PMC-RX receive frame count stat.",
#endif
    XIF_PMC_FRAME_RX_CNT_REG_OFFSET,
    0,
    0,
    247,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_PMC_FRAME_RX_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_PMC_BYTE_RX_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_PMC_BYTE_RX_CNT_FIELDS[] =
{
    &XIF_PMC_BYTE_RX_CNT_PMCRXBYTECNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_PMC_BYTE_RX_CNT_REG = 
{
    "PMC_BYTE_RX_CNT",
#if RU_INCLUDE_DESC
    "XIF_PMC_BYTE_RX_CNT Register",
    "PMC-RX byte count stat.",
#endif
    XIF_PMC_BYTE_RX_CNT_REG_OFFSET,
    0,
    0,
    248,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_PMC_BYTE_RX_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_PMC_RUNT_RX_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_PMC_RUNT_RX_CNT_FIELDS[] =
{
    &XIF_PMC_RUNT_RX_CNT_PMCRXRUNTCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_PMC_RUNT_RX_CNT_REG = 
{
    "PMC_RUNT_RX_CNT",
#if RU_INCLUDE_DESC
    "XIF_PMC_RUNT_RX_CNT Register",
    "PMC-RX runt count stat.",
#endif
    XIF_PMC_RUNT_RX_CNT_REG_OFFSET,
    0,
    0,
    249,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_PMC_RUNT_RX_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_PMC_CW_ERR_RX_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_PMC_CW_ERR_RX_CNT_FIELDS[] =
{
    &XIF_PMC_CW_ERR_RX_CNT_PMCRXCWERRCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_PMC_CW_ERR_RX_CNT_REG = 
{
    "PMC_CW_ERR_RX_CNT",
#if RU_INCLUDE_DESC
    "XIF_PMC_CW_ERR_RX_CNT Register",
    "PMC-RX code word error stat.",
#endif
    XIF_PMC_CW_ERR_RX_CNT_REG_OFFSET,
    0,
    0,
    250,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_PMC_CW_ERR_RX_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_PMC_CRC8_ERR_RX_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_PMC_CRC8_ERR_RX_CNT_FIELDS[] =
{
    &XIF_PMC_CRC8_ERR_RX_CNT_PMCRXCRC8ERRCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_PMC_CRC8_ERR_RX_CNT_REG = 
{
    "PMC_CRC8_ERR_RX_CNT",
#if RU_INCLUDE_DESC
    "XIF_PMC_CRC8_ERR_RX_CNT Register",
    "PMC-RX crc8 error stat.",
#endif
    XIF_PMC_CRC8_ERR_RX_CNT_REG_OFFSET,
    0,
    0,
    251,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_PMC_CRC8_ERR_RX_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_XPN_DATA_FRM_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_XPN_DATA_FRM_CNT_FIELDS[] =
{
    &XIF_XPN_DATA_FRM_CNT_XPNDTFRAMECNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_XPN_DATA_FRM_CNT_REG = 
{
    "XPN_DATA_FRM_CNT",
#if RU_INCLUDE_DESC
    "XIF_XPN_DATA_FRM_CNT Register",
    "XPN transmit data frame count.",
#endif
    XIF_XPN_DATA_FRM_CNT_REG_OFFSET,
    0,
    0,
    252,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_XPN_DATA_FRM_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_XPN_DATA_BYTE_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_XPN_DATA_BYTE_CNT_FIELDS[] =
{
    &XIF_XPN_DATA_BYTE_CNT_XPNDTBYTECNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_XPN_DATA_BYTE_CNT_REG = 
{
    "XPN_DATA_BYTE_CNT",
#if RU_INCLUDE_DESC
    "XIF_XPN_DATA_BYTE_CNT Register",
    "XPN transmit data byte count.",
#endif
    XIF_XPN_DATA_BYTE_CNT_REG_OFFSET,
    0,
    0,
    253,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_XPN_DATA_BYTE_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_XPN_MPCP_FRM_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_XPN_MPCP_FRM_CNT_FIELDS[] =
{
    &XIF_XPN_MPCP_FRM_CNT_XPNMPCPFRAMECNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_XPN_MPCP_FRM_CNT_REG = 
{
    "XPN_MPCP_FRM_CNT",
#if RU_INCLUDE_DESC
    "XIF_XPN_MPCP_FRM_CNT Register",
    "XPN transmit MPCP frame count.",
#endif
    XIF_XPN_MPCP_FRM_CNT_REG_OFFSET,
    0,
    0,
    254,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_XPN_MPCP_FRM_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_XPN_OAM_FRM_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_XPN_OAM_FRM_CNT_FIELDS[] =
{
    &XIF_XPN_OAM_FRM_CNT_XPNOAMFRAMECNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_XPN_OAM_FRM_CNT_REG = 
{
    "XPN_OAM_FRM_CNT",
#if RU_INCLUDE_DESC
    "XIF_XPN_OAM_FRM_CNT Register",
    "XPN transmit OAM frame count.",
#endif
    XIF_XPN_OAM_FRM_CNT_REG_OFFSET,
    0,
    0,
    255,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_XPN_OAM_FRM_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_XPN_OAM_BYTE_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_XPN_OAM_BYTE_CNT_FIELDS[] =
{
    &XIF_XPN_OAM_BYTE_CNT_XPNOAMBYTECNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_XPN_OAM_BYTE_CNT_REG = 
{
    "XPN_OAM_BYTE_CNT",
#if RU_INCLUDE_DESC
    "XIF_XPN_OAM_BYTE_CNT Register",
    "XPN transmit OAM byte count.",
#endif
    XIF_XPN_OAM_BYTE_CNT_REG_OFFSET,
    0,
    0,
    256,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_XPN_OAM_BYTE_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_XPN_OVERSIZE_FRM_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_XPN_OVERSIZE_FRM_CNT_FIELDS[] =
{
    &XIF_XPN_OVERSIZE_FRM_CNT_XPNDTOVERSIZECNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_XPN_OVERSIZE_FRM_CNT_REG = 
{
    "XPN_OVERSIZE_FRM_CNT",
#if RU_INCLUDE_DESC
    "XIF_XPN_OVERSIZE_FRM_CNT Register",
    "XPN transmit oversize frame stat.",
#endif
    XIF_XPN_OVERSIZE_FRM_CNT_REG_OFFSET,
    0,
    0,
    257,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_XPN_OVERSIZE_FRM_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_SEC_ABORT_FRM_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_SEC_ABORT_FRM_CNT_FIELDS[] =
{
    &XIF_SEC_ABORT_FRM_CNT_SECRXABORTFRMCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_SEC_ABORT_FRM_CNT_REG = 
{
    "SEC_ABORT_FRM_CNT",
#if RU_INCLUDE_DESC
    "XIF_SEC_ABORT_FRM_CNT Register",
    "SEC-RX abort frame stat.",
#endif
    XIF_SEC_ABORT_FRM_CNT_REG_OFFSET,
    0,
    0,
    258,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_SEC_ABORT_FRM_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_LLID_
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_LLID__FIELDS[] =
{
    &XIF_LLID__RESERVED0_FIELD,
    &XIF_LLID__CFGONULLID_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_LLID__REG = 
{
    "LLID_",
#if RU_INCLUDE_DESC
    "XIF_LLID %i Register",
    "Configures LLID index 0 translation.",
#endif
    XIF_LLID__REG_OFFSET,
    XIF_LLID__REG_RAM_CNT,
    4,
    259,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XIF_LLID__FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_MAX_MPCP_UPDATE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_MAX_MPCP_UPDATE_FIELDS[] =
{
    &XIF_MAX_MPCP_UPDATE_CFGMAXPOSMPCPUPD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_MAX_MPCP_UPDATE_REG = 
{
    "MAX_MPCP_UPDATE",
#if RU_INCLUDE_DESC
    "XIF_MAX_MPCP_UPDATE Register",
    "Specifies the maximum MPCP update.",
#endif
    XIF_MAX_MPCP_UPDATE_REG_OFFSET,
    0,
    0,
    260,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_MAX_MPCP_UPDATE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_IPG_INSERTION
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_IPG_INSERTION_FIELDS[] =
{
    &XIF_IPG_INSERTION_RESERVED0_FIELD,
    &XIF_IPG_INSERTION_CFGSHORTIPG_FIELD,
    &XIF_IPG_INSERTION_CFGINSERTIPG_FIELD,
    &XIF_IPG_INSERTION_RESERVED1_FIELD,
    &XIF_IPG_INSERTION_CFGIPGWORD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_IPG_INSERTION_REG = 
{
    "IPG_INSERTION",
#if RU_INCLUDE_DESC
    "XIF_IPG_INSERTION Register",
    "Specifies the IPG insertion between packets.",
#endif
    XIF_IPG_INSERTION_REG_OFFSET,
    0,
    0,
    261,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XIF_IPG_INSERTION_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_TRANSPORT_TIME
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_TRANSPORT_TIME_FIELDS[] =
{
    &XIF_TRANSPORT_TIME_CFTRANSPORTTIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_TRANSPORT_TIME_REG = 
{
    "TRANSPORT_TIME",
#if RU_INCLUDE_DESC
    "XIF_TRANSPORT_TIME Register",
    "Specifies the MPCP time to generate a one pulse per second (PPS)"
    "signal.",
#endif
    XIF_TRANSPORT_TIME_REG_OFFSET,
    0,
    0,
    262,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_TRANSPORT_TIME_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_MPCP_TIME
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_MPCP_TIME_FIELDS[] =
{
    &XIF_MPCP_TIME_CURMPCPTS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_MPCP_TIME_REG = 
{
    "MPCP_TIME",
#if RU_INCLUDE_DESC
    "XIF_MPCP_TIME Register",
    "Provides the current MPCP time.",
#endif
    XIF_MPCP_TIME_REG_OFFSET,
    0,
    0,
    263,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_MPCP_TIME_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_OVERLAP_GNT_OH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_OVERLAP_GNT_OH_FIELDS[] =
{
    &XIF_OVERLAP_GNT_OH_CFGOVRLPOH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_OVERLAP_GNT_OH_REG = 
{
    "OVERLAP_GNT_OH",
#if RU_INCLUDE_DESC
    "XIF_OVERLAP_GNT_OH Register",
    "Provides the overhead for overlapping grant.",
#endif
    XIF_OVERLAP_GNT_OH_REG_OFFSET,
    0,
    0,
    264,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_OVERLAP_GNT_OH_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_MAC_MODE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_MAC_MODE_FIELDS[] =
{
    &XIF_MAC_MODE_RESERVED0_FIELD,
    &XIF_MAC_MODE_CFGENNOGNTXMT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_MAC_MODE_REG = 
{
    "MAC_MODE",
#if RU_INCLUDE_DESC
    "XIF_MAC_MODE Register",
    "Specifies the MAC mode of operation.",
#endif
    XIF_MAC_MODE_REG_OFFSET,
    0,
    0,
    265,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XIF_MAC_MODE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_PMCTX_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_PMCTX_CTL_FIELDS[] =
{
    &XIF_PMCTX_CTL_RESERVED0_FIELD,
    &XIF_PMCTX_CTL_CFGMPCPUPDPERIOD_FIELD,
    &XIF_PMCTX_CTL_RESERVED1_FIELD,
    &XIF_PMCTX_CTL_CFGDIS4IDLEB4STARTCHAR_FIELD,
    &XIF_PMCTX_CTL_CFGENIDLEDSCRD_FIELD,
    &XIF_PMCTX_CTL_CFGSELTXPONTIME_FIELD,
    &XIF_PMCTX_CTL_CFGMPCPCONTUPD_FIELD,
    &XIF_PMCTX_CTL_CFGENMAXMPCPUPD_FIELD,
    &XIF_PMCTX_CTL_CFGENNEGTIMEABORT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_PMCTX_CTL_REG = 
{
    "PMCTX_CTL",
#if RU_INCLUDE_DESC
    "XIF_PMCTX_CTL Register",
    "Provides the control for PMC.",
#endif
    XIF_PMCTX_CTL_REG_OFFSET,
    0,
    0,
    266,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    XIF_PMCTX_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_SEC_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_SEC_CTL_FIELDS[] =
{
    &XIF_SEC_CTL_RESERVED0_FIELD,
    &XIF_SEC_CTL_CFGSECRXENSHORTLEN_FIELD,
    &XIF_SEC_CTL_CFGENSECTXFAKEAES_FIELD,
    &XIF_SEC_CTL_CFGENSECRXFAKEAES_FIELD,
    &XIF_SEC_CTL_CFGSECRXENPKTNUMRLOVR_FIELD,
    &XIF_SEC_CTL_RESERVED1_FIELD,
    &XIF_SEC_CTL_CFGSECTXENPKTNUMRLOVR_FIELD,
    &XIF_SEC_CTL_CFGENAEREPLAYPRCT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_SEC_CTL_REG = 
{
    "SEC_CTL",
#if RU_INCLUDE_DESC
    "XIF_SEC_CTL Register",
    "Provides control for security.",
#endif
    XIF_SEC_CTL_REG_OFFSET,
    0,
    0,
    267,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    XIF_SEC_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_AE_PKTNUM_WINDOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_AE_PKTNUM_WINDOW_FIELDS[] =
{
    &XIF_AE_PKTNUM_WINDOW_CFGAEPKTNUMWND_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_AE_PKTNUM_WINDOW_REG = 
{
    "AE_PKTNUM_WINDOW",
#if RU_INCLUDE_DESC
    "XIF_AE_PKTNUM_WINDOW Register",
    "Provides the tolerance for packet number reception in replay protection"
    "mode.  Only applicable in 802.1ae security mode.",
#endif
    XIF_AE_PKTNUM_WINDOW_REG_OFFSET,
    0,
    0,
    268,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_AE_PKTNUM_WINDOW_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_AE_PKTNUM_THRESH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_AE_PKTNUM_THRESH_FIELDS[] =
{
    &XIF_AE_PKTNUM_THRESH_CFGPKTNUMMAXTHRESH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_AE_PKTNUM_THRESH_REG = 
{
    "AE_PKTNUM_THRESH",
#if RU_INCLUDE_DESC
    "XIF_AE_PKTNUM_THRESH Register",
    "Provides the threshold to warn of impending packet number rollover on"
    "transmit.",
#endif
    XIF_AE_PKTNUM_THRESH_REG_OFFSET,
    0,
    0,
    269,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_AE_PKTNUM_THRESH_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_SECTX_KEYNUM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_SECTX_KEYNUM_FIELDS[] =
{
    &XIF_SECTX_KEYNUM_KEYSTATTX_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_SECTX_KEYNUM_REG = 
{
    "SECTX_KEYNUM",
#if RU_INCLUDE_DESC
    "XIF_SECTX_KEYNUM Register",
    "Provides upstream encryption key number stat, per LLID.",
#endif
    XIF_SECTX_KEYNUM_REG_OFFSET,
    0,
    0,
    270,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_SECTX_KEYNUM_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_SECTX_ENCRYPT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_SECTX_ENCRYPT_FIELDS[] =
{
    &XIF_SECTX_ENCRYPT_ENCRSTATTX_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_SECTX_ENCRYPT_REG = 
{
    "SECTX_ENCRYPT",
#if RU_INCLUDE_DESC
    "XIF_SECTX_ENCRYPT Register",
    "Provides upstream encryption  stat, per LLID.",
#endif
    XIF_SECTX_ENCRYPT_REG_OFFSET,
    0,
    0,
    271,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_SECTX_ENCRYPT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_AE_PKTNUM_STAT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_AE_PKTNUM_STAT_FIELDS[] =
{
    &XIF_AE_PKTNUM_STAT_RESERVED0_FIELD,
    &XIF_AE_PKTNUM_STAT_SECTXINDXWTPKTNUMMAX_FIELD,
    &XIF_AE_PKTNUM_STAT_RESERVED1_FIELD,
    &XIF_AE_PKTNUM_STAT_SECRXINDXWTPKTNUMABORT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_AE_PKTNUM_STAT_REG = 
{
    "AE_PKTNUM_STAT",
#if RU_INCLUDE_DESC
    "XIF_AE_PKTNUM_STAT Register",
    "Provides packet number status.",
#endif
    XIF_AE_PKTNUM_STAT_REG_OFFSET,
    0,
    0,
    272,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    XIF_AE_PKTNUM_STAT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_MPCP_UPDATE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_MPCP_UPDATE_FIELDS[] =
{
    &XIF_MPCP_UPDATE_MPCPUPDPERIOD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_MPCP_UPDATE_REG = 
{
    "MPCP_UPDATE",
#if RU_INCLUDE_DESC
    "XIF_MPCP_UPDATE Register",
    "Debug register showing time between MPCP updates.",
#endif
    XIF_MPCP_UPDATE_REG_OFFSET,
    0,
    0,
    273,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_MPCP_UPDATE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XIF_BURST_PRELAUNCH_OFFSET
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XIF_BURST_PRELAUNCH_OFFSET_FIELDS[] =
{
    &XIF_BURST_PRELAUNCH_OFFSET_CFGBURSTPRELAUNCHOFFSET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XIF_BURST_PRELAUNCH_OFFSET_REG = 
{
    "BURST_PRELAUNCH_OFFSET",
#if RU_INCLUDE_DESC
    "XIF_BURST_PRELAUNCH_OFFSET Register",
    "Provides prelaunch time of burst data from ONU, relative to the"
    "grant-start-time.",
#endif
    XIF_BURST_PRELAUNCH_OFFSET_REG_OFFSET,
    0,
    0,
    274,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XIF_BURST_PRELAUNCH_OFFSET_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: XIF
 ******************************************************************************/
static const ru_reg_rec *XIF_REGS[] =
{
    &XIF_CTL_REG,
    &XIF_INT_STATUS_REG,
    &XIF_INT_MASK_REG,
    &XIF_PORT_COMMAND_REG,
    &XIF_PORT_DATA__REG,
    &XIF_MACSEC_REG,
    &XIF_XPN_XMT_OFFSET_REG,
    &XIF_XPN_TIMESTAMP_OFFSET_REG,
    &XIF_XPN_PKTGEN_CTL_REG,
    &XIF_XPN_PKTGEN_LLID_REG,
    &XIF_XPN_PKTGEN_PKT_CNT_REG,
    &XIF_XPN_PKTGEN_PKT_SIZE_REG,
    &XIF_XPN_PKTGEN_IPG_REG,
    &XIF_TS_JITTER_THRESH_REG,
    &XIF_TS_UPDATE_REG,
    &XIF_GNT_OVERHEAD_REG,
    &XIF_DISCOVER_OVERHEAD_REG,
    &XIF_DISCOVER_INFO_REG,
    &XIF_XPN_OVERSIZE_THRESH_REG,
    &XIF_SECRX_KEYNUM_REG,
    &XIF_SECRX_ENCRYPT_REG,
    &XIF_PMC_FRAME_RX_CNT_REG,
    &XIF_PMC_BYTE_RX_CNT_REG,
    &XIF_PMC_RUNT_RX_CNT_REG,
    &XIF_PMC_CW_ERR_RX_CNT_REG,
    &XIF_PMC_CRC8_ERR_RX_CNT_REG,
    &XIF_XPN_DATA_FRM_CNT_REG,
    &XIF_XPN_DATA_BYTE_CNT_REG,
    &XIF_XPN_MPCP_FRM_CNT_REG,
    &XIF_XPN_OAM_FRM_CNT_REG,
    &XIF_XPN_OAM_BYTE_CNT_REG,
    &XIF_XPN_OVERSIZE_FRM_CNT_REG,
    &XIF_SEC_ABORT_FRM_CNT_REG,
    &XIF_LLID__REG,
    &XIF_MAX_MPCP_UPDATE_REG,
    &XIF_IPG_INSERTION_REG,
    &XIF_TRANSPORT_TIME_REG,
    &XIF_MPCP_TIME_REG,
    &XIF_OVERLAP_GNT_OH_REG,
    &XIF_MAC_MODE_REG,
    &XIF_PMCTX_CTL_REG,
    &XIF_SEC_CTL_REG,
    &XIF_AE_PKTNUM_WINDOW_REG,
    &XIF_AE_PKTNUM_THRESH_REG,
    &XIF_SECTX_KEYNUM_REG,
    &XIF_SECTX_ENCRYPT_REG,
    &XIF_AE_PKTNUM_STAT_REG,
    &XIF_MPCP_UPDATE_REG,
    &XIF_BURST_PRELAUNCH_OFFSET_REG,
};

static unsigned long XIF_ADDRS[] =
{
    0x80142800,
};

const ru_block_rec XIF_BLOCK = 
{
    "XIF",
    XIF_ADDRS,
    1,
    49,
    XIF_REGS
};

/* End of file EPON_XIF.c */
