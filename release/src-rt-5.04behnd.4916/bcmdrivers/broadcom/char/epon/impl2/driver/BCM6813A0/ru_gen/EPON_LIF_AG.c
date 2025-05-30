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
 * Field: LIF_PON_CONTROL_CFG_LIF_SGMII_MODE
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_CFG_LIF_SGMII_MODE_FIELD =
{
    "CFG_LIF_SGMII_MODE",
#if RU_INCLUDE_DESC
    "",
    "Select SGMII mode of operation."
    "0 = bypass, PON and 1G and 2G AE"
    "1 = SGMII 10,  TX to replicate data 100X, RX to decimate data by 100"
    "2 = SGMII 100, TX to replicate data 10X,  RX to decimate data by 10",
#endif
    LIF_PON_CONTROL_CFG_LIF_SGMII_MODE_FIELD_MASK,
    0,
    LIF_PON_CONTROL_CFG_LIF_SGMII_MODE_FIELD_WIDTH,
    LIF_PON_CONTROL_CFG_LIF_SGMII_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_CFGDISRUNTFILTER
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_CFGDISRUNTFILTER_FIELD =
{
    "CFGDISRUNTFILTER",
#if RU_INCLUDE_DESC
    "",
    "Disable runt packet filtering.",
#endif
    LIF_PON_CONTROL_CFGDISRUNTFILTER_FIELD_MASK,
    0,
    LIF_PON_CONTROL_CFGDISRUNTFILTER_FIELD_WIDTH,
    LIF_PON_CONTROL_CFGDISRUNTFILTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_CFMAXCOMMAERRCNT
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_CFMAXCOMMAERRCNT_FIELD =
{
    "CFMAXCOMMAERRCNT",
#if RU_INCLUDE_DESC
    "",
    "Malifum number of allowable comma errors before LOS is asserted."
    "0: Functionality Disabled",
#endif
    LIF_PON_CONTROL_CFMAXCOMMAERRCNT_FIELD_MASK,
    0,
    LIF_PON_CONTROL_CFMAXCOMMAERRCNT_FIELD_WIDTH,
    LIF_PON_CONTROL_CFMAXCOMMAERRCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_CFSYNCSMSELECT
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_CFSYNCSMSELECT_FIELD =
{
    "CFSYNCSMSELECT",
#if RU_INCLUDE_DESC
    "",
    "Selects between three 802.3 synchronization state machines:"
    "0: 802.3ah-65 FEC (single code word traversal)"
    "1: 802.3-36 non FEC (single code word traversal)"
    "2: Legacy (based on FEC, but traverses state machine two code words"
    "at a time)."
    "3: Reserved",
#endif
    LIF_PON_CONTROL_CFSYNCSMSELECT_FIELD_MASK,
    0,
    LIF_PON_CONTROL_CFSYNCSMSELECT_FIELD_WIDTH,
    LIF_PON_CONTROL_CFSYNCSMSELECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_CFPONRXFORCENONFECABORT
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_CFPONRXFORCENONFECABORT_FIELD =
{
    "CFPONRXFORCENONFECABORT",
#if RU_INCLUDE_DESC
    "",
    "Force aborts any NonFec frames in the PON module. This abort occurs"
    "before the FecRx module; therefore any frames dropped at this point"
    "will not have statistics tabulated."
    "0: Disabled"
    "1: Enabled",
#endif
    LIF_PON_CONTROL_CFPONRXFORCENONFECABORT_FIELD_MASK,
    0,
    LIF_PON_CONTROL_CFPONRXFORCENONFECABORT_FIELD_WIDTH,
    LIF_PON_CONTROL_CFPONRXFORCENONFECABORT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_CFPONRXFORCEFECABORT
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_CFPONRXFORCEFECABORT_FIELD =
{
    "CFPONRXFORCEFECABORT",
#if RU_INCLUDE_DESC
    "",
    "Force aborts any FEC frames in the PON module. This abort occurs"
    "before the FecRx module; therefore any frames dropped at this point"
    "will not have statistics tabulated."
    "0: Disabled"
    "1: Enabled",
#endif
    LIF_PON_CONTROL_CFPONRXFORCEFECABORT_FIELD_MASK,
    0,
    LIF_PON_CONTROL_CFPONRXFORCEFECABORT_FIELD_WIDTH,
    LIF_PON_CONTROL_CFPONRXFORCEFECABORT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_CFGRXDATABITFLIP
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_CFGRXDATABITFLIP_FIELD =
{
    "CFGRXDATABITFLIP",
#if RU_INCLUDE_DESC
    "",
    "Controls the order of the 10B/20B sent from the SERDES to the LIF"
    "module."
    "0: Receive data is unflipped."
    "1: Receive data is flipped."
    "Default : 1",
#endif
    LIF_PON_CONTROL_CFGRXDATABITFLIP_FIELD_MASK,
    0,
    LIF_PON_CONTROL_CFGRXDATABITFLIP_FIELD_WIDTH,
    LIF_PON_CONTROL_CFGRXDATABITFLIP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_PON_CONTROL_RESERVED0_FIELD_MASK,
    0,
    LIF_PON_CONTROL_RESERVED0_FIELD_WIDTH,
    LIF_PON_CONTROL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_CFGENMPCPOAMPAD
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_CFGENMPCPOAMPAD_FIELD =
{
    "CFGENMPCPOAMPAD",
#if RU_INCLUDE_DESC
    "",
    "Enable upstream padding of MPCP/OAM in an 802.1ae encrypted link to"
    "include security overhead.",
#endif
    LIF_PON_CONTROL_CFGENMPCPOAMPAD_FIELD_MASK,
    0,
    LIF_PON_CONTROL_CFGENMPCPOAMPAD_FIELD_WIDTH,
    LIF_PON_CONTROL_CFGENMPCPOAMPAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_CFGENTXIDLEPKT
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_CFGENTXIDLEPKT_FIELD =
{
    "CFGENTXIDLEPKT",
#if RU_INCLUDE_DESC
    "",
    "Enable IDLE packet support to prevent upstream underrun.",
#endif
    LIF_PON_CONTROL_CFGENTXIDLEPKT_FIELD_MASK,
    0,
    LIF_PON_CONTROL_CFGENTXIDLEPKT_FIELD_WIDTH,
    LIF_PON_CONTROL_CFGENTXIDLEPKT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_CFENABLESOFTWARESYNCHOLD
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_CFENABLESOFTWARESYNCHOLD_FIELD =
{
    "CFENABLESOFTWARESYNCHOLD",
#if RU_INCLUDE_DESC
    "",
    "Lock synchronization state indefinitely. This bit must be used in"
    "conjunction with cfEnableExtendSync.",
#endif
    LIF_PON_CONTROL_CFENABLESOFTWARESYNCHOLD_FIELD_MASK,
    0,
    LIF_PON_CONTROL_CFENABLESOFTWARESYNCHOLD_FIELD_WIDTH,
    LIF_PON_CONTROL_CFENABLESOFTWARESYNCHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_CFENABLEEXTENDSYNC
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_CFENABLEEXTENDSYNC_FIELD =
{
    "CFENABLEEXTENDSYNC",
#if RU_INCLUDE_DESC
    "",
    "Extend synchronization state. This can be used to improve FEC gain.",
#endif
    LIF_PON_CONTROL_CFENABLEEXTENDSYNC_FIELD_MASK,
    0,
    LIF_PON_CONTROL_CFENABLEEXTENDSYNC_FIELD_WIDTH,
    LIF_PON_CONTROL_CFENABLEEXTENDSYNC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_CFENABLEQUICKSYNC
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_CFENABLEQUICKSYNC_FIELD =
{
    "CFENABLEQUICKSYNC",
#if RU_INCLUDE_DESC
    "",
    "Allow alignment state machine to achieve code word lock in three"
    "idles, as opposed to four.",
#endif
    LIF_PON_CONTROL_CFENABLEQUICKSYNC_FIELD_MASK,
    0,
    LIF_PON_CONTROL_CFENABLEQUICKSYNC_FIELD_WIDTH,
    LIF_PON_CONTROL_CFENABLEQUICKSYNC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_CFPPSEN
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_CFPPSEN_FIELD =
{
    "CFPPSEN",
#if RU_INCLUDE_DESC
    "",
    "No function in this release of EPON."
    "Default: 1",
#endif
    LIF_PON_CONTROL_CFPPSEN_FIELD_MASK,
    0,
    LIF_PON_CONTROL_CFPPSEN_FIELD_WIDTH,
    LIF_PON_CONTROL_CFPPSEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_CFPPSCLKRBC
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_CFPPSCLKRBC_FIELD =
{
    "CFPPSCLKRBC",
#if RU_INCLUDE_DESC
    "",
    "0: 1PPS is not aligned to the 10MHz clock."
    "1: 1PPS is aligned to the positive edge of the 10MHz clock."
    "Default: 1",
#endif
    LIF_PON_CONTROL_CFPPSCLKRBC_FIELD_MASK,
    0,
    LIF_PON_CONTROL_CFPPSCLKRBC_FIELD_WIDTH,
    LIF_PON_CONTROL_CFPPSCLKRBC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_CFRX2TXLPBACK
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_CFRX2TXLPBACK_FIELD =
{
    "CFRX2TXLPBACK",
#if RU_INCLUDE_DESC
    "",
    "0: Disable Loop Back from Downstream to Upstream in the LIF."
    "1: Enable Loop Back from Downstream to Upstream in the LIF.",
#endif
    LIF_PON_CONTROL_CFRX2TXLPBACK_FIELD_MASK,
    0,
    LIF_PON_CONTROL_CFRX2TXLPBACK_FIELD_WIDTH,
    LIF_PON_CONTROL_CFRX2TXLPBACK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_CFTX2RXLPBACK
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_CFTX2RXLPBACK_FIELD =
{
    "CFTX2RXLPBACK",
#if RU_INCLUDE_DESC
    "",
    "0: Disable Loop Back from Upstream to Downstream in the LIF."
    "1: Enable Loop Back from Upstream to Downstream in the LIF."
    "Note: Due to a bug in BCM6838A0, RX stats will not tabulate"
    "correctly while in this loopback mode. TX stats should be used"
    "instead.",
#endif
    LIF_PON_CONTROL_CFTX2RXLPBACK_FIELD_MASK,
    0,
    LIF_PON_CONTROL_CFTX2RXLPBACK_FIELD_WIDTH,
    LIF_PON_CONTROL_CFTX2RXLPBACK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_CFTXDATAENDURLON
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_CFTXDATAENDURLON_FIELD =
{
    "CFTXDATAENDURLON",
#if RU_INCLUDE_DESC
    "",
    "0:  IDLE words continue to transmit while laser is turned off."
    "1: Transmitted data bus is de-asserted to zero when laser is turned"
    "off.",
#endif
    LIF_PON_CONTROL_CFTXDATAENDURLON_FIELD_MASK,
    0,
    LIF_PON_CONTROL_CFTXDATAENDURLON_FIELD_WIDTH,
    LIF_PON_CONTROL_CFTXDATAENDURLON_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_CFP2PMODE
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_CFP2PMODE_FIELD =
{
    "CFP2PMODE",
#if RU_INCLUDE_DESC
    "",
    "0: LIF sends standard EPON Preamble."
    "1: LIF sends Ethernet Preamble for Point-to-Point operation.",
#endif
    LIF_PON_CONTROL_CFP2PMODE_FIELD_MASK,
    0,
    LIF_PON_CONTROL_CFP2PMODE_FIELD_WIDTH,
    LIF_PON_CONTROL_CFP2PMODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_CFP2PSHORTPRE
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_CFP2PSHORTPRE_FIELD =
{
    "CFP2PSHORTPRE",
#if RU_INCLUDE_DESC
    "",
    "0: Send Standard P2P Ethernet Preamble."
    "1: Send Short (7 byte) P2P Ethernet Preamble. This bit must be set"
    "with cfP2PMode if full line rate is desired with odd sized frames."
    "If this bit is not set in P2P mode, only even sized frames are"
    "capable of line rate. The link partner must also be short preamble"
    "receive capable.",
#endif
    LIF_PON_CONTROL_CFP2PSHORTPRE_FIELD_MASK,
    0,
    LIF_PON_CONTROL_CFP2PSHORTPRE_FIELD_WIDTH,
    LIF_PON_CONTROL_CFP2PSHORTPRE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_CFLASEREN
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_CFLASEREN_FIELD =
{
    "CFLASEREN",
#if RU_INCLUDE_DESC
    "",
    "The output enable control for the 1Gbps upstream laser control"
    "(TXEN) pin."
    "0: 1G laser control pin is tri-stated"
    "1: 1G laser control is driven by the LIF module.",
#endif
    LIF_PON_CONTROL_CFLASEREN_FIELD_MASK,
    0,
    LIF_PON_CONTROL_CFLASEREN_FIELD_WIDTH,
    LIF_PON_CONTROL_CFLASEREN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_CFTXLASERON
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_CFTXLASERON_FIELD =
{
    "CFTXLASERON",
#if RU_INCLUDE_DESC
    "",
    "0: Laser is turned on at grant start time."
    "1: Laser is turned on continuously.",
#endif
    LIF_PON_CONTROL_CFTXLASERON_FIELD_MASK,
    0,
    LIF_PON_CONTROL_CFTXLASERON_FIELD_WIDTH,
    LIF_PON_CONTROL_CFTXLASERON_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_CFTXLASERONACTHI
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_CFTXLASERONACTHI_FIELD =
{
    "CFTXLASERONACTHI",
#if RU_INCLUDE_DESC
    "",
    "0: Configures the Laser On signal as active low."
    "1: Configures the Laser On signal as active high.",
#endif
    LIF_PON_CONTROL_CFTXLASERONACTHI_FIELD_MASK,
    0,
    LIF_PON_CONTROL_CFTXLASERONACTHI_FIELD_WIDTH,
    LIF_PON_CONTROL_CFTXLASERONACTHI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_LIFTXRSTN_PRE
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_LIFTXRSTN_PRE_FIELD =
{
    "LIFTXRSTN_PRE",
#if RU_INCLUDE_DESC
    "",
    "Resets the transmit side of the LIF."
    "0: Reset LIF TX."
    "1: Normal operation.",
#endif
    LIF_PON_CONTROL_LIFTXRSTN_PRE_FIELD_MASK,
    0,
    LIF_PON_CONTROL_LIFTXRSTN_PRE_FIELD_WIDTH,
    LIF_PON_CONTROL_LIFTXRSTN_PRE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_LIFRXRSTN_PRE
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_LIFRXRSTN_PRE_FIELD =
{
    "LIFRXRSTN_PRE",
#if RU_INCLUDE_DESC
    "",
    "Resets the receive side of the LIF."
    "0: Reset LIF RX."
    "1: Normal operation.",
#endif
    LIF_PON_CONTROL_LIFRXRSTN_PRE_FIELD_MASK,
    0,
    LIF_PON_CONTROL_LIFRXRSTN_PRE_FIELD_WIDTH,
    LIF_PON_CONTROL_LIFRXRSTN_PRE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_LIFTXEN
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_LIFTXEN_FIELD =
{
    "LIFTXEN",
#if RU_INCLUDE_DESC
    "",
    "Enables LIF TX for operation."
    "0: Disable the external interface to and from the LIF TX."
    "1: Enable the external interface to and from the LIF TX.",
#endif
    LIF_PON_CONTROL_LIFTXEN_FIELD_MASK,
    0,
    LIF_PON_CONTROL_LIFTXEN_FIELD_WIDTH,
    LIF_PON_CONTROL_LIFTXEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_LIFRXEN
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_LIFRXEN_FIELD =
{
    "LIFRXEN",
#if RU_INCLUDE_DESC
    "",
    "Enables LIF RX for operation."
    "0: Disable the external interface to and from the LIF RX."
    "1: Enable the external interface to and from the LIF RX.",
#endif
    LIF_PON_CONTROL_LIFRXEN_FIELD_MASK,
    0,
    LIF_PON_CONTROL_LIFRXEN_FIELD_WIDTH,
    LIF_PON_CONTROL_LIFRXEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_INTER_OP_CONTROL_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_PON_INTER_OP_CONTROL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_PON_INTER_OP_CONTROL_RESERVED0_FIELD_MASK,
    0,
    LIF_PON_INTER_OP_CONTROL_RESERVED0_FIELD_WIDTH,
    LIF_PON_INTER_OP_CONTROL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_INTER_OP_CONTROL_CFIPGFILTER
 ******************************************************************************/
const ru_field_rec LIF_PON_INTER_OP_CONTROL_CFIPGFILTER_FIELD =
{
    "CFIPGFILTER",
#if RU_INCLUDE_DESC
    "",
    "Units are in code group pairs (two 10b code groups)."
    ""
    "0: Disable Ipg Filter (Legacy Behavior)"
    "1-3: DO NOT USE, Illegal  Values, allows faster than line rate"
    "operation."
    "4-15: Allow 4 to 15 code group pairs to elapse after a frame before"
    "becoming receptive to a SOP or SFEC again."
    ""
    "Default: 5 code group pairs or \"10 bytes of IPG\".  Per spec, \"12"
    "bytes of IPG\" is line rate, but the"
    "default setting allows for a rate that is slightly faster than line"
    "rate for tolerance purposes.",
#endif
    LIF_PON_INTER_OP_CONTROL_CFIPGFILTER_FIELD_MASK,
    0,
    LIF_PON_INTER_OP_CONTROL_CFIPGFILTER_FIELD_WIDTH,
    LIF_PON_INTER_OP_CONTROL_CFIPGFILTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_INTER_OP_CONTROL_CFDISABLELOSLASERBLOCK
 ******************************************************************************/
const ru_field_rec LIF_PON_INTER_OP_CONTROL_CFDISABLELOSLASERBLOCK_FIELD =
{
    "CFDISABLELOSLASERBLOCK",
#if RU_INCLUDE_DESC
    "",
    "Allows for control of logic which blocks laser enable based on"
    "condition of downstream sync."
    "0: Allow for los to block laser enable."
    "1: Laser will toggle regardless of state of downstream code group"
    "sync",
#endif
    LIF_PON_INTER_OP_CONTROL_CFDISABLELOSLASERBLOCK_FIELD_MASK,
    0,
    LIF_PON_INTER_OP_CONTROL_CFDISABLELOSLASERBLOCK_FIELD_WIDTH,
    LIF_PON_INTER_OP_CONTROL_CFDISABLELOSLASERBLOCK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_INTER_OP_CONTROL_CFGLLIDPROMISCUOUSMODE
 ******************************************************************************/
const ru_field_rec LIF_PON_INTER_OP_CONTROL_CFGLLIDPROMISCUOUSMODE_FIELD =
{
    "CFGLLIDPROMISCUOUSMODE",
#if RU_INCLUDE_DESC
    "",
    "All unmapped LLIDs will be redirected and mapped to Index 0."
    "0: Unmapped LLIDs will appear to be unmapped to EPN."
    "1: Unmapped LLIDs will appear on Index 0 to EPN.",
#endif
    LIF_PON_INTER_OP_CONTROL_CFGLLIDPROMISCUOUSMODE_FIELD_MASK,
    0,
    LIF_PON_INTER_OP_CONTROL_CFGLLIDPROMISCUOUSMODE_FIELD_WIDTH,
    LIF_PON_INTER_OP_CONTROL_CFGLLIDPROMISCUOUSMODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_INTER_OP_CONTROL_CFGLLIDMODMSK
 ******************************************************************************/
const ru_field_rec LIF_PON_INTER_OP_CONTROL_CFGLLIDMODMSK_FIELD =
{
    "CFGLLIDMODMSK",
#if RU_INCLUDE_DESC
    "",
    "Masks MSB of 16 bit raw LLID for Index translation."
    "0: Don't mask, look at full 16 bits."
    "1: Mask bit[15], map based on [14:0].",
#endif
    LIF_PON_INTER_OP_CONTROL_CFGLLIDMODMSK_FIELD_MASK,
    0,
    LIF_PON_INTER_OP_CONTROL_CFGLLIDMODMSK_FIELD_WIDTH,
    LIF_PON_INTER_OP_CONTROL_CFGLLIDMODMSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_INTER_OP_CONTROL_CFUSEFECIPG
 ******************************************************************************/
const ru_field_rec LIF_PON_INTER_OP_CONTROL_CFUSEFECIPG_FIELD =
{
    "CFUSEFECIPG",
#if RU_INCLUDE_DESC
    "",
    "Allows SFEC to consume 4 bytes of IPG per standard."
    "0: 4 Additional bytes of IPG will be added for SFEC."
    "1: SFEC will carve 4 bytes out of existing IPG.",
#endif
    LIF_PON_INTER_OP_CONTROL_CFUSEFECIPG_FIELD_MASK,
    0,
    LIF_PON_INTER_OP_CONTROL_CFUSEFECIPG_FIELD_WIDTH,
    LIF_PON_INTER_OP_CONTROL_CFUSEFECIPG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_INTER_OP_CONTROL_CFRXCRC8INVCHK
 ******************************************************************************/
const ru_field_rec LIF_PON_INTER_OP_CONTROL_CFRXCRC8INVCHK_FIELD =
{
    "CFRXCRC8INVCHK",
#if RU_INCLUDE_DESC
    "",
    "Enable inverted CRC-8 checking."
    "0: Disable CRC-8 checking. Packets with inverted CRC-8 are"
    "discarded."
    "1: Enable CRC-8 checking. Packets with inverted CRC-8 are considered"
    "valid.",
#endif
    LIF_PON_INTER_OP_CONTROL_CFRXCRC8INVCHK_FIELD_MASK,
    0,
    LIF_PON_INTER_OP_CONTROL_CFRXCRC8INVCHK_FIELD_WIDTH,
    LIF_PON_INTER_OP_CONTROL_CFRXCRC8INVCHK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_INTER_OP_CONTROL_CFRXCRC8BITSWAP
 ******************************************************************************/
const ru_field_rec LIF_PON_INTER_OP_CONTROL_CFRXCRC8BITSWAP_FIELD =
{
    "CFRXCRC8BITSWAP",
#if RU_INCLUDE_DESC
    "",
    "0: CRC-8 is checked from LSB to MSB in the downstream direction."
    "1: CRC-8 is checked from MSB to LSB in the downstream direction.",
#endif
    LIF_PON_INTER_OP_CONTROL_CFRXCRC8BITSWAP_FIELD_MASK,
    0,
    LIF_PON_INTER_OP_CONTROL_CFRXCRC8BITSWAP_FIELD_WIDTH,
    LIF_PON_INTER_OP_CONTROL_CFRXCRC8BITSWAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_INTER_OP_CONTROL_CFRXCRC8MSB2LSB
 ******************************************************************************/
const ru_field_rec LIF_PON_INTER_OP_CONTROL_CFRXCRC8MSB2LSB_FIELD =
{
    "CFRXCRC8MSB2LSB",
#if RU_INCLUDE_DESC
    "",
    "0: CRC-8 is checked by shifting data from MSB to LSB in the"
    "downstream direction."
    "1: CRC-8 is checked by shifting data from LSB to MSB in the"
    "downstream direction.",
#endif
    LIF_PON_INTER_OP_CONTROL_CFRXCRC8MSB2LSB_FIELD_MASK,
    0,
    LIF_PON_INTER_OP_CONTROL_CFRXCRC8MSB2LSB_FIELD_WIDTH,
    LIF_PON_INTER_OP_CONTROL_CFRXCRC8MSB2LSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_INTER_OP_CONTROL_CFRXCRC8DISABLE
 ******************************************************************************/
const ru_field_rec LIF_PON_INTER_OP_CONTROL_CFRXCRC8DISABLE_FIELD =
{
    "CFRXCRC8DISABLE",
#if RU_INCLUDE_DESC
    "",
    "0: Enable Crc-8 checking."
    "1: Disable Crc-8 checking.",
#endif
    LIF_PON_INTER_OP_CONTROL_CFRXCRC8DISABLE_FIELD_MASK,
    0,
    LIF_PON_INTER_OP_CONTROL_CFRXCRC8DISABLE_FIELD_WIDTH,
    LIF_PON_INTER_OP_CONTROL_CFRXCRC8DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_INTER_OP_CONTROL_RESERVED1
 ******************************************************************************/
const ru_field_rec LIF_PON_INTER_OP_CONTROL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_PON_INTER_OP_CONTROL_RESERVED1_FIELD_MASK,
    0,
    LIF_PON_INTER_OP_CONTROL_RESERVED1_FIELD_WIDTH,
    LIF_PON_INTER_OP_CONTROL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_INTER_OP_CONTROL_CFTXLLIDBIT15SET
 ******************************************************************************/
const ru_field_rec LIF_PON_INTER_OP_CONTROL_CFTXLLIDBIT15SET_FIELD =
{
    "CFTXLLIDBIT15SET",
#if RU_INCLUDE_DESC
    "",
    "0: Bit 15 of LLID in the upstream path is zero."
    "1: Bit 15 of LLID in the upstream path is set.",
#endif
    LIF_PON_INTER_OP_CONTROL_CFTXLLIDBIT15SET_FIELD_MASK,
    0,
    LIF_PON_INTER_OP_CONTROL_CFTXLLIDBIT15SET_FIELD_WIDTH,
    LIF_PON_INTER_OP_CONTROL_CFTXLLIDBIT15SET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_INTER_OP_CONTROL_CFTXCRC8INV
 ******************************************************************************/
const ru_field_rec LIF_PON_INTER_OP_CONTROL_CFTXCRC8INV_FIELD =
{
    "CFTXCRC8INV",
#if RU_INCLUDE_DESC
    "",
    "0: Transmit correct Crc-8"
    "1: Transmit inverted Crc-8 on a per packet basis.",
#endif
    LIF_PON_INTER_OP_CONTROL_CFTXCRC8INV_FIELD_MASK,
    0,
    LIF_PON_INTER_OP_CONTROL_CFTXCRC8INV_FIELD_WIDTH,
    LIF_PON_INTER_OP_CONTROL_CFTXCRC8INV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_INTER_OP_CONTROL_CFTXCRC8BAD
 ******************************************************************************/
const ru_field_rec LIF_PON_INTER_OP_CONTROL_CFTXCRC8BAD_FIELD =
{
    "CFTXCRC8BAD",
#if RU_INCLUDE_DESC
    "",
    "0: Transmit correct Crc-8"
    "1: Transmit bad Crc-8",
#endif
    LIF_PON_INTER_OP_CONTROL_CFTXCRC8BAD_FIELD_MASK,
    0,
    LIF_PON_INTER_OP_CONTROL_CFTXCRC8BAD_FIELD_WIDTH,
    LIF_PON_INTER_OP_CONTROL_CFTXCRC8BAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_INTER_OP_CONTROL_CFTXCRC8BITSWAP
 ******************************************************************************/
const ru_field_rec LIF_PON_INTER_OP_CONTROL_CFTXCRC8BITSWAP_FIELD =
{
    "CFTXCRC8BITSWAP",
#if RU_INCLUDE_DESC
    "",
    "0: Generated upstream Crc-8 byte is transmitted from Lsb to Msb."
    "1: Generated upstream Crc-8 byte is transmitted from Msb to Lsb."
    ""
    "Note: This feature is added to give the ONU more flexibility of"
    "transmitting the Crc-8 byte.",
#endif
    LIF_PON_INTER_OP_CONTROL_CFTXCRC8BITSWAP_FIELD_MASK,
    0,
    LIF_PON_INTER_OP_CONTROL_CFTXCRC8BITSWAP_FIELD_WIDTH,
    LIF_PON_INTER_OP_CONTROL_CFTXCRC8BITSWAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_INTER_OP_CONTROL_CFTXCRC8MSB2LSB
 ******************************************************************************/
const ru_field_rec LIF_PON_INTER_OP_CONTROL_CFTXCRC8MSB2LSB_FIELD =
{
    "CFTXCRC8MSB2LSB",
#if RU_INCLUDE_DESC
    "",
    "0: Generate upstream Crc-8 by shifting data from Lsb to Msb."
    "1: Generate upstream Crc-8 by shifting data from Msb to Lsb.",
#endif
    LIF_PON_INTER_OP_CONTROL_CFTXCRC8MSB2LSB_FIELD_MASK,
    0,
    LIF_PON_INTER_OP_CONTROL_CFTXCRC8MSB2LSB_FIELD_WIDTH,
    LIF_PON_INTER_OP_CONTROL_CFTXCRC8MSB2LSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_INTER_OP_CONTROL_CFTXSHORTPRE
 ******************************************************************************/
const ru_field_rec LIF_PON_INTER_OP_CONTROL_CFTXSHORTPRE_FIELD =
{
    "CFTXSHORTPRE",
#if RU_INCLUDE_DESC
    "",
    "0: Normal operation."
    "1: Enable the LIF module to transmit short pre-amble in the upstream"
    "path.",
#endif
    LIF_PON_INTER_OP_CONTROL_CFTXSHORTPRE_FIELD_MASK,
    0,
    LIF_PON_INTER_OP_CONTROL_CFTXSHORTPRE_FIELD_WIDTH,
    LIF_PON_INTER_OP_CONTROL_CFTXSHORTPRE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_INTER_OP_CONTROL_CFTXIPGCNT
 ******************************************************************************/
const ru_field_rec LIF_PON_INTER_OP_CONTROL_CFTXIPGCNT_FIELD =
{
    "CFTXIPGCNT",
#if RU_INCLUDE_DESC
    "",
    "LIF upstream IPG counter. Each unit represents one time quanta or"
    "16ns."
    "Default: 2",
#endif
    LIF_PON_INTER_OP_CONTROL_CFTXIPGCNT_FIELD_MASK,
    0,
    LIF_PON_INTER_OP_CONTROL_CFTXIPGCNT_FIELD_WIDTH,
    LIF_PON_INTER_OP_CONTROL_CFTXIPGCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_INTER_OP_CONTROL_CFTXAASYNCLEN
 ******************************************************************************/
const ru_field_rec LIF_PON_INTER_OP_CONTROL_CFTXAASYNCLEN_FIELD =
{
    "CFTXAASYNCLEN",
#if RU_INCLUDE_DESC
    "",
    "0: Normal Operation"
    "Greater than 0: This bit field is used for testing with Panasonic"
    "transceivers. The pattern of 10'b10_1010_1010 (2AA) will transmit at"
    "the beginning of every burst. The number transmitted word is"
    "programmable based on the configuration of cfInitIdle in register"
    "0x10d. The amount transmitted is always a portion of cfInitIdle up"
    "to a malifum of 16.",
#endif
    LIF_PON_INTER_OP_CONTROL_CFTXAASYNCLEN_FIELD_MASK,
    0,
    LIF_PON_INTER_OP_CONTROL_CFTXAASYNCLEN_FIELD_WIDTH,
    LIF_PON_INTER_OP_CONTROL_CFTXAASYNCLEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_INTER_OP_CONTROL_CFTXPIPEDELAY
 ******************************************************************************/
const ru_field_rec LIF_PON_INTER_OP_CONTROL_CFTXPIPEDELAY_FIELD =
{
    "CFTXPIPEDELAY",
#if RU_INCLUDE_DESC
    "",
    "Pipeline delay to be added on to laser on time and laser off time"
    "Default: 6",
#endif
    LIF_PON_INTER_OP_CONTROL_CFTXPIPEDELAY_FIELD_MASK,
    0,
    LIF_PON_INTER_OP_CONTROL_CFTXPIPEDELAY_FIELD_WIDTH,
    LIF_PON_INTER_OP_CONTROL_CFTXPIPEDELAY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_FEC_CONTROL_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_FEC_CONTROL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_FEC_CONTROL_RESERVED0_FIELD_MASK,
    0,
    LIF_FEC_CONTROL_RESERVED0_FIELD_WIDTH,
    LIF_FEC_CONTROL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_FEC_CONTROL_CFFECRXERRORPROP
 ******************************************************************************/
const ru_field_rec LIF_FEC_CONTROL_CFFECRXERRORPROP_FIELD =
{
    "CFFECRXERRORPROP",
#if RU_INCLUDE_DESC
    "",
    "0: Uncorrectable frames are aborted."
    "1: Uncorrectable frames are forwarded.",
#endif
    LIF_FEC_CONTROL_CFFECRXERRORPROP_FIELD_MASK,
    0,
    LIF_FEC_CONTROL_CFFECRXERRORPROP_FIELD_WIDTH,
    LIF_FEC_CONTROL_CFFECRXERRORPROP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_FEC_CONTROL_CFFECRXFORCENONFECABORT
 ******************************************************************************/
const ru_field_rec LIF_FEC_CONTROL_CFFECRXFORCENONFECABORT_FIELD =
{
    "CFFECRXFORCENONFECABORT",
#if RU_INCLUDE_DESC
    "",
    "Aborts non-FEC frames after the FecRx module. Statistics will still"
    "be tabulated for frames aborted through this manner."
    "0: Non-FEC frames are forwarded."
    "1: Non-FEC frames are aborted.",
#endif
    LIF_FEC_CONTROL_CFFECRXFORCENONFECABORT_FIELD_MASK,
    0,
    LIF_FEC_CONTROL_CFFECRXFORCENONFECABORT_FIELD_WIDTH,
    LIF_FEC_CONTROL_CFFECRXFORCENONFECABORT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_FEC_CONTROL_CFFECRXFORCEFECABORT
 ******************************************************************************/
const ru_field_rec LIF_FEC_CONTROL_CFFECRXFORCEFECABORT_FIELD =
{
    "CFFECRXFORCEFECABORT",
#if RU_INCLUDE_DESC
    "",
    "Aborts FEC frames after the FecRx module. Statistics will still be"
    "tabulated for frames aborted through this manner."
    "0: FEC frames are forwarded."
    "1: FEC frames are aborted.",
#endif
    LIF_FEC_CONTROL_CFFECRXFORCEFECABORT_FIELD_MASK,
    0,
    LIF_FEC_CONTROL_CFFECRXFORCEFECABORT_FIELD_WIDTH,
    LIF_FEC_CONTROL_CFFECRXFORCEFECABORT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_FEC_CONTROL_CFFECRXENABLE
 ******************************************************************************/
const ru_field_rec LIF_FEC_CONTROL_CFFECRXENABLE_FIELD =
{
    "CFFECRXENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enable FEC on Receiver"
    "0: FEC Disabled"
    "1: FEC Enabled",
#endif
    LIF_FEC_CONTROL_CFFECRXENABLE_FIELD_MASK,
    0,
    LIF_FEC_CONTROL_CFFECRXENABLE_FIELD_WIDTH,
    LIF_FEC_CONTROL_CFFECRXENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_FEC_CONTROL_CFFECTXFECPERLLID
 ******************************************************************************/
const ru_field_rec LIF_FEC_CONTROL_CFFECTXFECPERLLID_FIELD =
{
    "CFFECTXFECPERLLID",
#if RU_INCLUDE_DESC
    "",
    "Enables per LLID Fec Parity Generation."
    "This bit, cfFecTxEn, corresponding llid enable bit cfFecTxFecLlidEn#"
    "below must be set."
    "0: Disabled"
    "1: Enabled",
#endif
    LIF_FEC_CONTROL_CFFECTXFECPERLLID_FIELD_MASK,
    0,
    LIF_FEC_CONTROL_CFFECTXFECPERLLID_FIELD_WIDTH,
    LIF_FEC_CONTROL_CFFECTXFECPERLLID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_FEC_CONTROL_CFFECTXENABLE
 ******************************************************************************/
const ru_field_rec LIF_FEC_CONTROL_CFFECTXENABLE_FIELD =
{
    "CFFECTXENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enable FEC on Transmitter"
    "0: FEC Disabled"
    "1: FEC Enabled",
#endif
    LIF_FEC_CONTROL_CFFECTXENABLE_FIELD_MASK,
    0,
    LIF_FEC_CONTROL_CFFECTXENABLE_FIELD_WIDTH,
    LIF_FEC_CONTROL_CFFECTXENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_CFGDISMPCPENCRYPT
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_CFGDISMPCPENCRYPT_FIELD =
{
    "CFGDISMPCPENCRYPT",
#if RU_INCLUDE_DESC
    "",
    "Disable OAM encryption.",
#endif
    LIF_SEC_CONTROL_CFGDISMPCPENCRYPT_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_CFGDISMPCPENCRYPT_FIELD_WIDTH,
    LIF_SEC_CONTROL_CFGDISMPCPENCRYPT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_CFGDISOAMENCRYPT
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_CFGDISOAMENCRYPT_FIELD =
{
    "CFGDISOAMENCRYPT",
#if RU_INCLUDE_DESC
    "",
    "Disable OAM encryption.",
#endif
    LIF_SEC_CONTROL_CFGDISOAMENCRYPT_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_CFGDISOAMENCRYPT_FIELD_WIDTH,
    LIF_SEC_CONTROL_CFGDISOAMENCRYPT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_CFGSECENSHORTLEN
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_CFGSECENSHORTLEN_FIELD =
{
    "CFGSECENSHORTLEN",
#if RU_INCLUDE_DESC
    "",
    "Enables downstream security short length support.",
#endif
    LIF_SEC_CONTROL_CFGSECENSHORTLEN_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_CFGSECENSHORTLEN_FIELD_WIDTH,
    LIF_SEC_CONTROL_CFGSECENSHORTLEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_CFGSECDNENPKTNUMRLOVR
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_CFGSECDNENPKTNUMRLOVR_FIELD =
{
    "CFGSECDNENPKTNUMRLOVR",
#if RU_INCLUDE_DESC
    "",
    "Enables downstream security packet number rollover.",
#endif
    LIF_SEC_CONTROL_CFGSECDNENPKTNUMRLOVR_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_CFGSECDNENPKTNUMRLOVR_FIELD_WIDTH,
    LIF_SEC_CONTROL_CFGSECDNENPKTNUMRLOVR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_CFGSECUPENPKTNUMRLOVR
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_CFGSECUPENPKTNUMRLOVR_FIELD =
{
    "CFGSECUPENPKTNUMRLOVR",
#if RU_INCLUDE_DESC
    "",
    "Enables upstream security packet number rollover.",
#endif
    LIF_SEC_CONTROL_CFGSECUPENPKTNUMRLOVR_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_CFGSECUPENPKTNUMRLOVR_FIELD_WIDTH,
    LIF_SEC_CONTROL_CFGSECUPENPKTNUMRLOVR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_CFGENAEREPLAYPRCT
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_CFGENAEREPLAYPRCT_FIELD =
{
    "CFGENAEREPLAYPRCT",
#if RU_INCLUDE_DESC
    "",
    "Enables replay protection on RX security.",
#endif
    LIF_SEC_CONTROL_CFGENAEREPLAYPRCT_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_CFGENAEREPLAYPRCT_FIELD_WIDTH,
    LIF_SEC_CONTROL_CFGENAEREPLAYPRCT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_CFGENLEGACYRCC
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_CFGENLEGACYRCC_FIELD =
{
    "CFGENLEGACYRCC",
#if RU_INCLUDE_DESC
    "",
    "Enables legacy RCC priority encoding mode for EPON encryption.",
#endif
    LIF_SEC_CONTROL_CFGENLEGACYRCC_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_CFGENLEGACYRCC_FIELD_WIDTH,
    LIF_SEC_CONTROL_CFGENLEGACYRCC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_ENFAKEUPAES
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_ENFAKEUPAES_FIELD =
{
    "ENFAKEUPAES",
#if RU_INCLUDE_DESC
    "",
    "Enables fake AES mode in the upstream for FPGA testing."
    "0: Normal operation."
    "1: Enable fake AES.",
#endif
    LIF_SEC_CONTROL_ENFAKEUPAES_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_ENFAKEUPAES_FIELD_WIDTH,
    LIF_SEC_CONTROL_ENFAKEUPAES_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_ENFAKEDNAES
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_ENFAKEDNAES_FIELD =
{
    "ENFAKEDNAES",
#if RU_INCLUDE_DESC
    "",
    "Enables fake AES mode in the downstream for FPGA testing."
    "0: Normal operation."
    "1: Enable fake AES.",
#endif
    LIF_SEC_CONTROL_ENFAKEDNAES_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_ENFAKEDNAES_FIELD_WIDTH,
    LIF_SEC_CONTROL_ENFAKEDNAES_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_SEC_CONTROL_RESERVED0_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_RESERVED0_FIELD_WIDTH,
    LIF_SEC_CONTROL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_CFGFECIPGLEN
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_CFGFECIPGLEN_FIELD =
{
    "CFGFECIPGLEN",
#if RU_INCLUDE_DESC
    "",
    "FEC IPG Len used by SEC to support certain security modes."
    "Default: 0xa",
#endif
    LIF_SEC_CONTROL_CFGFECIPGLEN_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_CFGFECIPGLEN_FIELD_WIDTH,
    LIF_SEC_CONTROL_CFGFECIPGLEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_DISDNDASAENCRPT
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_DISDNDASAENCRPT_FIELD =
{
    "DISDNDASAENCRPT",
#if RU_INCLUDE_DESC
    "",
    "Disable downstream DA/SA encryption."
    "0: Enable DA/SA encryption"
    "1: Disable DA/SA encryption",
#endif
    LIF_SEC_CONTROL_DISDNDASAENCRPT_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_DISDNDASAENCRPT_FIELD_WIDTH,
    LIF_SEC_CONTROL_DISDNDASAENCRPT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_ENTRIPLECHURN
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_ENTRIPLECHURN_FIELD =
{
    "ENTRIPLECHURN",
#if RU_INCLUDE_DESC
    "",
    "0: Single Churning encryption (do not use)"
    "1: Triple Churning encryption"
    "This bit matters only when dnEncryptScheme is set to \"CEPON\"."
    "Default: 1",
#endif
    LIF_SEC_CONTROL_ENTRIPLECHURN_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_ENTRIPLECHURN_FIELD_WIDTH,
    LIF_SEC_CONTROL_ENTRIPLECHURN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_ENEPNMIXENCRYPT
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_ENEPNMIXENCRYPT_FIELD =
{
    "ENEPNMIXENCRYPT",
#if RU_INCLUDE_DESC
    "",
    "0: In EPON mode, all packets must be either encrypted or"
    "non-encrypted; a mixture of both is not allowed. The turning on/off"
    "of encryption can be initiated only by the OLT."
    "1: In EPON mode, mixing of encrypted and non-encrypted packets is"
    "allowed for a particular LLID."
    "Default: 1",
#endif
    LIF_SEC_CONTROL_ENEPNMIXENCRYPT_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_ENEPNMIXENCRYPT_FIELD_WIDTH,
    LIF_SEC_CONTROL_ENEPNMIXENCRYPT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_DISUPDASAENCRPT
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_DISUPDASAENCRPT_FIELD =
{
    "DISUPDASAENCRPT",
#if RU_INCLUDE_DESC
    "",
    "Disable upstream DA/SA encryption."
    "0: Enable DA/SA encryption"
    "1: Disable DA/SA encryption",
#endif
    LIF_SEC_CONTROL_DISUPDASAENCRPT_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_DISUPDASAENCRPT_FIELD_WIDTH,
    LIF_SEC_CONTROL_DISUPDASAENCRPT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_SECUPENCRYPTSCHEME
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_SECUPENCRYPTSCHEME_FIELD =
{
    "SECUPENCRYPTSCHEME",
#if RU_INCLUDE_DESC
    "",
    "Defines the upstream security decryption scheme."
    "0: Teknovus encryption"
    "1: Zero-overhead encryption"
    "2: EPON encryption"
    "3: AE encryption",
#endif
    LIF_SEC_CONTROL_SECUPENCRYPTSCHEME_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_SECUPENCRYPTSCHEME_FIELD_WIDTH,
    LIF_SEC_CONTROL_SECUPENCRYPTSCHEME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_SECDNENCRYPTSCHEME
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_SECDNENCRYPTSCHEME_FIELD =
{
    "SECDNENCRYPTSCHEME",
#if RU_INCLUDE_DESC
    "",
    "Defines the downstream security decryption scheme."
    "0: Teknovus encryption"
    "1: Reserved"
    "2: EPON encryption"
    "3: CEPON encryption"
    "4: Zero-overhead encryption"
    "5: AE encryption",
#endif
    LIF_SEC_CONTROL_SECDNENCRYPTSCHEME_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_SECDNENCRYPTSCHEME_FIELD_WIDTH,
    LIF_SEC_CONTROL_SECDNENCRYPTSCHEME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_SECUPRSTN_PRE
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_SECUPRSTN_PRE_FIELD =
{
    "SECUPRSTN_PRE",
#if RU_INCLUDE_DESC
    "",
    "Resets upstream SEC."
    "0: Reset UP SEC."
    "1: Normal operation",
#endif
    LIF_SEC_CONTROL_SECUPRSTN_PRE_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_SECUPRSTN_PRE_FIELD_WIDTH,
    LIF_SEC_CONTROL_SECUPRSTN_PRE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_SECDNRSTN_PRE
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_SECDNRSTN_PRE_FIELD =
{
    "SECDNRSTN_PRE",
#if RU_INCLUDE_DESC
    "",
    "Resets downstream SEC."
    "0: Reset DN SEC."
    "1: Normal operation.",
#endif
    LIF_SEC_CONTROL_SECDNRSTN_PRE_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_SECDNRSTN_PRE_FIELD_WIDTH,
    LIF_SEC_CONTROL_SECDNRSTN_PRE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_SECENUP
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_SECENUP_FIELD =
{
    "SECENUP",
#if RU_INCLUDE_DESC
    "",
    "Global enable for upstream encryption"
    "0: Disable upstream encryption."
    "1: Enable upstream encryption.",
#endif
    LIF_SEC_CONTROL_SECENUP_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_SECENUP_FIELD_WIDTH,
    LIF_SEC_CONTROL_SECENUP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_SECENDN
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_SECENDN_FIELD =
{
    "SECENDN",
#if RU_INCLUDE_DESC
    "",
    "Global enable for downstream decryption"
    "0: Disable downstream decryption."
    "1: Enable downstream decryption.",
#endif
    LIF_SEC_CONTROL_SECENDN_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_SECENDN_FIELD_WIDTH,
    LIF_SEC_CONTROL_SECENDN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_MACSEC_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_MACSEC_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_MACSEC_RESERVED0_FIELD_MASK,
    0,
    LIF_MACSEC_RESERVED0_FIELD_WIDTH,
    LIF_MACSEC_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_MACSEC_CFGMACSECETHERTYPE
 ******************************************************************************/
const ru_field_rec LIF_MACSEC_CFGMACSECETHERTYPE_FIELD =
{
    "CFGMACSECETHERTYPE",
#if RU_INCLUDE_DESC
    "",
    "Defines the MacSec Ethertype.",
#endif
    LIF_MACSEC_CFGMACSECETHERTYPE_FIELD_MASK,
    0,
    LIF_MACSEC_CFGMACSECETHERTYPE_FIELD_WIDTH,
    LIF_MACSEC_CFGMACSECETHERTYPE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_INT_STATUS_RESERVED0_FIELD_MASK,
    0,
    LIF_INT_STATUS_RESERVED0_FIELD_WIDTH,
    LIF_INT_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_SGMIIABORTFRINT
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_SGMIIABORTFRINT_FIELD =
{
    "SGMIIABORTFRINT",
#if RU_INCLUDE_DESC
    "",
    "FIFO empty during frame for SGMII 10/100.",
#endif
    LIF_INT_STATUS_SGMIIABORTFRINT_FIELD_MASK,
    0,
    LIF_INT_STATUS_SGMIIABORTFRINT_FIELD_WIDTH,
    LIF_INT_STATUS_SGMIIABORTFRINT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_INT_SOP_SFEC_IPG_VIOLATION
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_INT_SOP_SFEC_IPG_VIOLATION_FIELD =
{
    "INT_SOP_SFEC_IPG_VIOLATION",
#if RU_INCLUDE_DESC
    "",
    "[NON-FATAL]"
    ""
    "Indicates that an SOP or SFEC was detected in an IPG window in"
    "excess of what was provisioned in cfIpgFilter.  Please see"
    "cfIpgFilter for more details.",
#endif
    LIF_INT_STATUS_INT_SOP_SFEC_IPG_VIOLATION_FIELD_MASK,
    0,
    LIF_INT_STATUS_INT_SOP_SFEC_IPG_VIOLATION_FIELD_WIDTH,
    LIF_INT_STATUS_INT_SOP_SFEC_IPG_VIOLATION_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_LASERONMAX
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_LASERONMAX_FIELD =
{
    "LASERONMAX",
#if RU_INCLUDE_DESC
    "",
    "Indicates laser enable on time exceeed the malifum threshold, as"
    "defined by register LIF_LSR_MON_A_MAX_THR.",
#endif
    LIF_INT_STATUS_LASERONMAX_FIELD_MASK,
    0,
    LIF_INT_STATUS_LASERONMAX_FIELD_WIDTH,
    LIF_INT_STATUS_LASERONMAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_LASEROFF
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_LASEROFF_FIELD =
{
    "LASEROFF",
#if RU_INCLUDE_DESC
    "",
    "Indicates laser enable deassertion.",
#endif
    LIF_INT_STATUS_LASEROFF_FIELD_MASK,
    0,
    LIF_INT_STATUS_LASEROFF_FIELD_WIDTH,
    LIF_INT_STATUS_LASEROFF_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_SECDNREPLAYPROTCTABORT
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_SECDNREPLAYPROTCTABORT_FIELD =
{
    "SECDNREPLAYPROTCTABORT",
#if RU_INCLUDE_DESC
    "",
    "[NON-FATAL] Applicable only in 802.1ae security.  Indicates the"
    "received packet was aborted due to replay protection.",
#endif
    LIF_INT_STATUS_SECDNREPLAYPROTCTABORT_FIELD_MASK,
    0,
    LIF_INT_STATUS_SECDNREPLAYPROTCTABORT_FIELD_WIDTH,
    LIF_INT_STATUS_SECDNREPLAYPROTCTABORT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_SECUPPKTNUMOVERFLOW
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_SECUPPKTNUMOVERFLOW_FIELD =
{
    "SECUPPKTNUMOVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "[NON-FATAL] Applicable only in 802.1ae security.  Indicates the"
    "transmit packet number exceeded the malifum threshold and about to"
    "overflow.  Threshold is programmed in register LIF_AE_PKTNUM_THRESH.",
#endif
    LIF_INT_STATUS_SECUPPKTNUMOVERFLOW_FIELD_MASK,
    0,
    LIF_INT_STATUS_SECUPPKTNUMOVERFLOW_FIELD_WIDTH,
    LIF_INT_STATUS_SECUPPKTNUMOVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_INTLASEROFFDURBURST
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_INTLASEROFFDURBURST_FIELD =
{
    "INTLASEROFFDURBURST",
#if RU_INCLUDE_DESC
    "",
    "[NON-FATAL]"
    "Laser was turned off in the middle of a burst.  This usually"
    "indicates misconfiguration which results in EPN \"overstuffing\" a"
    "burst."
    "Note: This interrupt will fire while in P2P or in DN2UP Loopback"
    "mode.  S/W is to mask this bit during those modes.  A fix may be"
    "introduced into later revisions of chip to fix this cosmetic issue"
    "(FLEXIPON-138).",
#endif
    LIF_INT_STATUS_INTLASEROFFDURBURST_FIELD_MASK,
    0,
    LIF_INT_STATUS_INTLASEROFFDURBURST_FIELD_WIDTH,
    LIF_INT_STATUS_INTLASEROFFDURBURST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_INTRXBERTHRESHEXC
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_INTRXBERTHRESHEXC_FIELD =
{
    "INTRXBERTHRESHEXC",
#if RU_INCLUDE_DESC
    "",
    "[NON-FATAL]"
    "Line code error threshold was exceeded."
    "Program with LIF RX BER Threshold and Interval register (0x1b4)",
#endif
    LIF_INT_STATUS_INTRXBERTHRESHEXC_FIELD_MASK,
    0,
    LIF_INT_STATUS_INTRXBERTHRESHEXC_FIELD_WIDTH,
    LIF_INT_STATUS_INTRXBERTHRESHEXC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_INTFECRXFECRECVSTATUS
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_INTFECRXFECRECVSTATUS_FIELD =
{
    "INTFECRXFECRECVSTATUS",
#if RU_INCLUDE_DESC
    "",
    "The LIF detected a FEC receive frame.",
#endif
    LIF_INT_STATUS_INTFECRXFECRECVSTATUS_FIELD_MASK,
    0,
    LIF_INT_STATUS_INTFECRXFECRECVSTATUS_FIELD_WIDTH,
    LIF_INT_STATUS_INTFECRXFECRECVSTATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_INTFECRXCORERRFIFOFULLSTATUS
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_INTFECRXCORERRFIFOFULLSTATUS_FIELD =
{
    "INTFECRXCORERRFIFOFULLSTATUS",
#if RU_INCLUDE_DESC
    "",
    "[FATAL] Error location FIFO in Corrector logic has overflowed; some"
    "data blocks will go uncorrected."
    "This is considered a fatal interrupt because this introduces FEC"
    "block level inconsistencies, which may cause the correction of the"
    "wrong blocks.",
#endif
    LIF_INT_STATUS_INTFECRXCORERRFIFOFULLSTATUS_FIELD_MASK,
    0,
    LIF_INT_STATUS_INTFECRXCORERRFIFOFULLSTATUS_FIELD_WIDTH,
    LIF_INT_STATUS_INTFECRXCORERRFIFOFULLSTATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_INTFECRXCORERRFIFOUNEXPEMPTY
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_INTFECRXCORERRFIFOUNEXPEMPTY_FIELD =
{
    "INTFECRXCORERRFIFOUNEXPEMPTY",
#if RU_INCLUDE_DESC
    "",
    "Error location FIFO in Corrector logic has gone empty before"
    "finishing a FEC frame. This is considered a fatal interrupt because"
    "this introduces FEC block level inconsistencies, which may cause the"
    "correction of the wrong blocks.",
#endif
    LIF_INT_STATUS_INTFECRXCORERRFIFOUNEXPEMPTY_FIELD_MASK,
    0,
    LIF_INT_STATUS_INTFECRXCORERRFIFOUNEXPEMPTY_FIELD_WIDTH,
    LIF_INT_STATUS_INTFECRXCORERRFIFOUNEXPEMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_INTFECBUFPOPEMPTYPUSH
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_INTFECBUFPOPEMPTYPUSH_FIELD =
{
    "INTFECBUFPOPEMPTYPUSH",
#if RU_INCLUDE_DESC
    "",
    "[FATAL] Data was popped from an empty FEC Buffer pipeline, which"
    "simultaneously was having data pushed into it from the FEC Buffer"
    "SRAM.",
#endif
    LIF_INT_STATUS_INTFECBUFPOPEMPTYPUSH_FIELD_MASK,
    0,
    LIF_INT_STATUS_INTFECBUFPOPEMPTYPUSH_FIELD_WIDTH,
    LIF_INT_STATUS_INTFECBUFPOPEMPTYPUSH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_INTFECBUFPOPEMPTYNOPUSH
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_INTFECBUFPOPEMPTYNOPUSH_FIELD =
{
    "INTFECBUFPOPEMPTYNOPUSH",
#if RU_INCLUDE_DESC
    "",
    "[FATAL] Data was popped from an empty FEC Buffer pipeline, which"
    "simultaneously no data was being pushed into the pipeline from the"
    "FEC Buffer SRAM.",
#endif
    LIF_INT_STATUS_INTFECBUFPOPEMPTYNOPUSH_FIELD_MASK,
    0,
    LIF_INT_STATUS_INTFECBUFPOPEMPTYNOPUSH_FIELD_WIDTH,
    LIF_INT_STATUS_INTFECBUFPOPEMPTYNOPUSH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_INTFECBUFPUSHFULL
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_INTFECBUFPUSHFULL_FIELD =
{
    "INTFECBUFPUSHFULL",
#if RU_INCLUDE_DESC
    "",
    "[FATAL] Data was read from FEC Buffer SRAM and pushed into a full"
    "FEC Buffer pipeline."
    "This is fatal."
    "Write a one to clear this bit.",
#endif
    LIF_INT_STATUS_INTFECBUFPUSHFULL_FIELD_MASK,
    0,
    LIF_INT_STATUS_INTFECBUFPUSHFULL_FIELD_WIDTH,
    LIF_INT_STATUS_INTFECBUFPUSHFULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_INTUPTIMEFULLUPDSTAT
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_INTUPTIMEFULLUPDSTAT_FIELD =
{
    "INTUPTIMEFULLUPDSTAT",
#if RU_INCLUDE_DESC
    "",
    "[NON-FATAL] Gate frame's MPCP timestamp vastly different than"
    "current MPCP time.  Triggered a"
    "full timestamp update.",
#endif
    LIF_INT_STATUS_INTUPTIMEFULLUPDSTAT_FIELD_MASK,
    0,
    LIF_INT_STATUS_INTUPTIMEFULLUPDSTAT_FIELD_WIDTH,
    LIF_INT_STATUS_INTUPTIMEFULLUPDSTAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_INTFROUTOFALIGNSTAT
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_INTFROUTOFALIGNSTAT_FIELD =
{
    "INTFROUTOFALIGNSTAT",
#if RU_INCLUDE_DESC
    "",
    "[FATAL] LIF detects a frame from Epn Utx that should be first frame"
    "of the burst but it is not.",
#endif
    LIF_INT_STATUS_INTFROUTOFALIGNSTAT_FIELD_MASK,
    0,
    LIF_INT_STATUS_INTFROUTOFALIGNSTAT_FIELD_WIDTH,
    LIF_INT_STATUS_INTFROUTOFALIGNSTAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_INTGRNTSTARTTIMELAGSTAT
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_INTGRNTSTARTTIMELAGSTAT_FIELD =
{
    "INTGRNTSTARTTIMELAGSTAT",
#if RU_INCLUDE_DESC
    "",
    "[FATAL] LIF detects a grant start time that is less than its current"
    "timer.",
#endif
    LIF_INT_STATUS_INTGRNTSTARTTIMELAGSTAT_FIELD_MASK,
    0,
    LIF_INT_STATUS_INTGRNTSTARTTIMELAGSTAT_FIELD_WIDTH,
    LIF_INT_STATUS_INTGRNTSTARTTIMELAGSTAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_INTABORTRXFRMSTAT
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_INTABORTRXFRMSTAT_FIELD =
{
    "INTABORTRXFRMSTAT",
#if RU_INCLUDE_DESC
    "",
    "[NON-FATAL] LIF had to abort frames due to misalignment.",
#endif
    LIF_INT_STATUS_INTABORTRXFRMSTAT_FIELD_MASK,
    0,
    LIF_INT_STATUS_INTABORTRXFRMSTAT_FIELD_WIDTH,
    LIF_INT_STATUS_INTABORTRXFRMSTAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_INTNORXCLKSTAT
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_INTNORXCLKSTAT_FIELD =
{
    "INTNORXCLKSTAT",
#if RU_INCLUDE_DESC
    "",
    "[NON-FATAL] LIF detects an idle condition for received clock.",
#endif
    LIF_INT_STATUS_INTNORXCLKSTAT_FIELD_MASK,
    0,
    LIF_INT_STATUS_INTNORXCLKSTAT_FIELD_WIDTH,
    LIF_INT_STATUS_INTNORXCLKSTAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_INTRXMAXLENERRSTAT
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_INTRXMAXLENERRSTAT_FIELD =
{
    "INTRXMAXLENERRSTAT",
#if RU_INCLUDE_DESC
    "",
    "[NON-FATAL] Lif detects a runaway frame. LIF could not detect an end"
    "of frame character for 64K clocks.",
#endif
    LIF_INT_STATUS_INTRXMAXLENERRSTAT_FIELD_MASK,
    0,
    LIF_INT_STATUS_INTRXMAXLENERRSTAT_FIELD_WIDTH,
    LIF_INT_STATUS_INTRXMAXLENERRSTAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_INTRXERRAFTALIGNSTAT
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_INTRXERRAFTALIGNSTAT_FIELD =
{
    "INTRXERRAFTALIGNSTAT",
#if RU_INCLUDE_DESC
    "",
    "[NON-FATAL] LIF detects code error after it has successfully"
    "acquired sync.",
#endif
    LIF_INT_STATUS_INTRXERRAFTALIGNSTAT_FIELD_MASK,
    0,
    LIF_INT_STATUS_INTRXERRAFTALIGNSTAT_FIELD_WIDTH,
    LIF_INT_STATUS_INTRXERRAFTALIGNSTAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_INTRXSYNCHACQSTAT
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_INTRXSYNCHACQSTAT_FIELD =
{
    "INTRXSYNCHACQSTAT",
#if RU_INCLUDE_DESC
    "",
    "[NON-FATAL] LIF is successfully acquired sync.",
#endif
    LIF_INT_STATUS_INTRXSYNCHACQSTAT_FIELD_MASK,
    0,
    LIF_INT_STATUS_INTRXSYNCHACQSTAT_FIELD_WIDTH,
    LIF_INT_STATUS_INTRXSYNCHACQSTAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_STATUS_INTRXOUTOFSYNCHSTAT
 ******************************************************************************/
const ru_field_rec LIF_INT_STATUS_INTRXOUTOFSYNCHSTAT_FIELD =
{
    "INTRXOUTOFSYNCHSTAT",
#if RU_INCLUDE_DESC
    "",
    "[NON-FATAL] LIF is out of sync. This condition indicates that the"
    "LIF could not align to IDLE characters or it detects code errors.",
#endif
    LIF_INT_STATUS_INTRXOUTOFSYNCHSTAT_FIELD_MASK,
    0,
    LIF_INT_STATUS_INTRXOUTOFSYNCHSTAT_FIELD_WIDTH,
    LIF_INT_STATUS_INTRXOUTOFSYNCHSTAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_INT_MASK_RESERVED0_FIELD_MASK,
    0,
    LIF_INT_MASK_RESERVED0_FIELD_WIDTH,
    LIF_INT_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_SGMIIABORTFRINTMASK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_SGMIIABORTFRINTMASK_FIELD =
{
    "SGMIIABORTFRINTMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask.",
#endif
    LIF_INT_MASK_SGMIIABORTFRINTMASK_FIELD_MASK,
    0,
    LIF_INT_MASK_SGMIIABORTFRINTMASK_FIELD_WIDTH,
    LIF_INT_MASK_SGMIIABORTFRINTMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_INT_SOP_SFEC_IPG_VIOLATION_MASK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_INT_SOP_SFEC_IPG_VIOLATION_MASK_FIELD =
{
    "INT_SOP_SFEC_IPG_VIOLATION_MASK",
#if RU_INCLUDE_DESC
    "",
    "[NON-FATAL]"
    ""
    "Mask for int_sop_sfec_ipg_violation interrupt."
    "0: Disabled -- don't toggle interrupt pin."
    "1: Enabled -- toggle interrupt pin."
    "Default : 1",
#endif
    LIF_INT_MASK_INT_SOP_SFEC_IPG_VIOLATION_MASK_FIELD_MASK,
    0,
    LIF_INT_MASK_INT_SOP_SFEC_IPG_VIOLATION_MASK_FIELD_WIDTH,
    LIF_INT_MASK_INT_SOP_SFEC_IPG_VIOLATION_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_LASERONMAXMASK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_LASERONMAXMASK_FIELD =
{
    "LASERONMAXMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask.",
#endif
    LIF_INT_MASK_LASERONMAXMASK_FIELD_MASK,
    0,
    LIF_INT_MASK_LASERONMAXMASK_FIELD_WIDTH,
    LIF_INT_MASK_LASERONMAXMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_LASEROFFMASK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_LASEROFFMASK_FIELD =
{
    "LASEROFFMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask.",
#endif
    LIF_INT_MASK_LASEROFFMASK_FIELD_MASK,
    0,
    LIF_INT_MASK_LASEROFFMASK_FIELD_WIDTH,
    LIF_INT_MASK_LASEROFFMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_SECDNREPLAYPROTCTABORTMSK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_SECDNREPLAYPROTCTABORTMSK_FIELD =
{
    "SECDNREPLAYPROTCTABORTMSK",
#if RU_INCLUDE_DESC
    "",
    "Mask for replay protection abort interrupt",
#endif
    LIF_INT_MASK_SECDNREPLAYPROTCTABORTMSK_FIELD_MASK,
    0,
    LIF_INT_MASK_SECDNREPLAYPROTCTABORTMSK_FIELD_WIDTH,
    LIF_INT_MASK_SECDNREPLAYPROTCTABORTMSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_SECUPPKTNUMOVERFLOWMSK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_SECUPPKTNUMOVERFLOWMSK_FIELD =
{
    "SECUPPKTNUMOVERFLOWMSK",
#if RU_INCLUDE_DESC
    "",
    "Mask for packet number overflow interrupt",
#endif
    LIF_INT_MASK_SECUPPKTNUMOVERFLOWMSK_FIELD_MASK,
    0,
    LIF_INT_MASK_SECUPPKTNUMOVERFLOWMSK_FIELD_WIDTH,
    LIF_INT_MASK_SECUPPKTNUMOVERFLOWMSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_INTLASEROFFDURBURSTMASK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_INTLASEROFFDURBURSTMASK_FIELD =
{
    "INTLASEROFFDURBURSTMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for laserOffDurBurstMask interrupt."
    "0: Disabled -- don't toggle interrupt pin."
    "1: Enabled -- toggle interrupt pin."
    "Default : 1",
#endif
    LIF_INT_MASK_INTLASEROFFDURBURSTMASK_FIELD_MASK,
    0,
    LIF_INT_MASK_INTLASEROFFDURBURSTMASK_FIELD_WIDTH,
    LIF_INT_MASK_INTLASEROFFDURBURSTMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_INTRXBERTHRESHEXCMASK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_INTRXBERTHRESHEXCMASK_FIELD =
{
    "INTRXBERTHRESHEXCMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for rxBerThreshExc interrupt."
    "0: Disabled -- don't toggle interrupt pin."
    "1: Enabled -- toggle interrupt pin."
    "Default : 1",
#endif
    LIF_INT_MASK_INTRXBERTHRESHEXCMASK_FIELD_MASK,
    0,
    LIF_INT_MASK_INTRXBERTHRESHEXCMASK_FIELD_WIDTH,
    LIF_INT_MASK_INTRXBERTHRESHEXCMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_INTFECRXFECRECVMASK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_INTFECRXFECRECVMASK_FIELD =
{
    "INTFECRXFECRECVMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for fecRxFrmRecv interrupt."
    "0: Disabled -- don't toggle interrupt pin."
    "1: Enabled -- toggle interrupt pin."
    "Default : 1",
#endif
    LIF_INT_MASK_INTFECRXFECRECVMASK_FIELD_MASK,
    0,
    LIF_INT_MASK_INTFECRXFECRECVMASK_FIELD_WIDTH,
    LIF_INT_MASK_INTFECRXFECRECVMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_INTFECRXCORERRFIFOFULLMASK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_INTFECRXCORERRFIFOFULLMASK_FIELD =
{
    "INTFECRXCORERRFIFOFULLMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for fecCorrErrFifFullMask interrupt."
    "0: Disabled -- don't toggle interrupt pin."
    "1: Enabled -- toggle interrupt pin."
    "Default : 1",
#endif
    LIF_INT_MASK_INTFECRXCORERRFIFOFULLMASK_FIELD_MASK,
    0,
    LIF_INT_MASK_INTFECRXCORERRFIFOFULLMASK_FIELD_WIDTH,
    LIF_INT_MASK_INTFECRXCORERRFIFOFULLMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_INTFECRXCORERRFIFOUNEXPEMPTYMASK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_INTFECRXCORERRFIFOUNEXPEMPTYMASK_FIELD =
{
    "INTFECRXCORERRFIFOUNEXPEMPTYMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for FecRxCorErrFifoUnExpEmpty interrupt."
    "0: Disabled -- don't toggle interrupt pin."
    "1: Enabled -- toggle interrupt pin."
    "Default : 1",
#endif
    LIF_INT_MASK_INTFECRXCORERRFIFOUNEXPEMPTYMASK_FIELD_MASK,
    0,
    LIF_INT_MASK_INTFECRXCORERRFIFOUNEXPEMPTYMASK_FIELD_WIDTH,
    LIF_INT_MASK_INTFECRXCORERRFIFOUNEXPEMPTYMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_INTFECBUFPOPEMPTYPUSHMASK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_INTFECBUFPOPEMPTYPUSHMASK_FIELD =
{
    "INTFECBUFPOPEMPTYPUSHMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for fecBufPopEmptyPush interrupt."
    "0: Disabled -- don't toggle interrupt pin."
    "1: Enabled -- toggle interrupt pin."
    "Default : 1",
#endif
    LIF_INT_MASK_INTFECBUFPOPEMPTYPUSHMASK_FIELD_MASK,
    0,
    LIF_INT_MASK_INTFECBUFPOPEMPTYPUSHMASK_FIELD_WIDTH,
    LIF_INT_MASK_INTFECBUFPOPEMPTYPUSHMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_INTFECBUFPOPEMPTYNOPUSHMASK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_INTFECBUFPOPEMPTYNOPUSHMASK_FIELD =
{
    "INTFECBUFPOPEMPTYNOPUSHMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for fecBufPopEmptyNoPush interrupt."
    "0: Disabled -- don't toggle interrupt pin."
    "1: Enabled -- toggle interrupt pin."
    "Default : 1",
#endif
    LIF_INT_MASK_INTFECBUFPOPEMPTYNOPUSHMASK_FIELD_MASK,
    0,
    LIF_INT_MASK_INTFECBUFPOPEMPTYNOPUSHMASK_FIELD_WIDTH,
    LIF_INT_MASK_INTFECBUFPOPEMPTYNOPUSHMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_INTFECBUFPUSHFULLMASK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_INTFECBUFPUSHFULLMASK_FIELD =
{
    "INTFECBUFPUSHFULLMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for fecBufPushFull interrupt."
    "0: Disabled -- don't toggle interrupt pin."
    "1: Enabled -- toggle interrupt pin."
    "Default : 1",
#endif
    LIF_INT_MASK_INTFECBUFPUSHFULLMASK_FIELD_MASK,
    0,
    LIF_INT_MASK_INTFECBUFPUSHFULLMASK_FIELD_WIDTH,
    LIF_INT_MASK_INTFECBUFPUSHFULLMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_INTUPTIMEFULLUPDMASK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_INTUPTIMEFULLUPDMASK_FIELD =
{
    "INTUPTIMEFULLUPDMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for upTimeFullUpdStat interrupt."
    "0: Disabled -- don't toggle interrupt pin."
    "1: Enabled -- toggle interrupt pin."
    "Default : 1",
#endif
    LIF_INT_MASK_INTUPTIMEFULLUPDMASK_FIELD_MASK,
    0,
    LIF_INT_MASK_INTUPTIMEFULLUPDMASK_FIELD_WIDTH,
    LIF_INT_MASK_INTUPTIMEFULLUPDMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_INTFROUTOFALIGNMASK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_INTFROUTOFALIGNMASK_FIELD =
{
    "INTFROUTOFALIGNMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for frmOutOfAlignStat interrupt."
    "0: Disabled -- don't toggle interrupt pin."
    "1: Enabled -- toggle interrupt pin."
    "Default : 1",
#endif
    LIF_INT_MASK_INTFROUTOFALIGNMASK_FIELD_MASK,
    0,
    LIF_INT_MASK_INTFROUTOFALIGNMASK_FIELD_WIDTH,
    LIF_INT_MASK_INTFROUTOFALIGNMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_INTGRNTSTARTTIMELAGMASK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_INTGRNTSTARTTIMELAGMASK_FIELD =
{
    "INTGRNTSTARTTIMELAGMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for grntStartTimeLagStat interrupt."
    "0: Disabled -- don't toggle interrupt pin."
    "1: Enabled -- toggle interrupt pin."
    "Default : 1",
#endif
    LIF_INT_MASK_INTGRNTSTARTTIMELAGMASK_FIELD_MASK,
    0,
    LIF_INT_MASK_INTGRNTSTARTTIMELAGMASK_FIELD_WIDTH,
    LIF_INT_MASK_INTGRNTSTARTTIMELAGMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_INTABORTRXFRMMASK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_INTABORTRXFRMMASK_FIELD =
{
    "INTABORTRXFRMMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for rxFrmAbortStat interrupt."
    "0: Disabled -- don't toggle interrupt pin."
    "1: Enabled -- toggle interrupt pin."
    "Default : 1",
#endif
    LIF_INT_MASK_INTABORTRXFRMMASK_FIELD_MASK,
    0,
    LIF_INT_MASK_INTABORTRXFRMMASK_FIELD_WIDTH,
    LIF_INT_MASK_INTABORTRXFRMMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_INTNORXCLKMASK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_INTNORXCLKMASK_FIELD =
{
    "INTNORXCLKMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for noRxClkStat interrupt."
    "0: Disabled -- don't toggle interrupt pin."
    "1: Enabled -- toggle interrupt pin."
    "Default : 1",
#endif
    LIF_INT_MASK_INTNORXCLKMASK_FIELD_MASK,
    0,
    LIF_INT_MASK_INTNORXCLKMASK_FIELD_WIDTH,
    LIF_INT_MASK_INTNORXCLKMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_INTRXMAXLENERRMASK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_INTRXMAXLENERRMASK_FIELD =
{
    "INTRXMAXLENERRMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for noRxClkStat interrupt."
    "0: Disabled -- don't toggle interrupt pin."
    "1: Enabled -- toggle interrupt pin."
    "Default : 1",
#endif
    LIF_INT_MASK_INTRXMAXLENERRMASK_FIELD_MASK,
    0,
    LIF_INT_MASK_INTRXMAXLENERRMASK_FIELD_WIDTH,
    LIF_INT_MASK_INTRXMAXLENERRMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_INTRXERRAFTALIGNMASK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_INTRXERRAFTALIGNMASK_FIELD =
{
    "INTRXERRAFTALIGNMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for rxErrAftAlignStat interrupt."
    "0: Disabled -- don't toggle interrupt pin."
    "1: Enabled -- toggle interrupt pin."
    "Default : 1",
#endif
    LIF_INT_MASK_INTRXERRAFTALIGNMASK_FIELD_MASK,
    0,
    LIF_INT_MASK_INTRXERRAFTALIGNMASK_FIELD_WIDTH,
    LIF_INT_MASK_INTRXERRAFTALIGNMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_INTRXSYNCHACQMASK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_INTRXSYNCHACQMASK_FIELD =
{
    "INTRXSYNCHACQMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for rxSynchAcqStat interrupt."
    "0: Disabled -- don't toggle interrupt pin."
    "1: Enabled -- toggle interrupt pin."
    "Default : 1",
#endif
    LIF_INT_MASK_INTRXSYNCHACQMASK_FIELD_MASK,
    0,
    LIF_INT_MASK_INTRXSYNCHACQMASK_FIELD_WIDTH,
    LIF_INT_MASK_INTRXSYNCHACQMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_INT_MASK_INTRXOUTOFSYNCHMASK
 ******************************************************************************/
const ru_field_rec LIF_INT_MASK_INTRXOUTOFSYNCHMASK_FIELD =
{
    "INTRXOUTOFSYNCHMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for rxOutOfSyncStat interrupt."
    "0: Disabled -- don't toggle interrupt pin."
    "1: Enabled -- toggle interrupt pin."
    "Default : 1",
#endif
    LIF_INT_MASK_INTRXOUTOFSYNCHMASK_FIELD_MASK,
    0,
    LIF_INT_MASK_INTRXOUTOFSYNCHMASK_FIELD_WIDTH,
    LIF_INT_MASK_INTRXOUTOFSYNCHMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_DATA_PORT_COMMAND_DATA_PORT_BUSY
 ******************************************************************************/
const ru_field_rec LIF_DATA_PORT_COMMAND_DATA_PORT_BUSY_FIELD =
{
    "DATA_PORT_BUSY",
#if RU_INCLUDE_DESC
    "",
    "Indicates access to RAM is in progress."
    "0: Data port is ready to accept a command"
    "1: Data port is busy",
#endif
    LIF_DATA_PORT_COMMAND_DATA_PORT_BUSY_FIELD_MASK,
    0,
    LIF_DATA_PORT_COMMAND_DATA_PORT_BUSY_FIELD_WIDTH,
    LIF_DATA_PORT_COMMAND_DATA_PORT_BUSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_DATA_PORT_COMMAND_DATA_PORT_ERROR
 ******************************************************************************/
const ru_field_rec LIF_DATA_PORT_COMMAND_DATA_PORT_ERROR_FIELD =
{
    "DATA_PORT_ERROR",
#if RU_INCLUDE_DESC
    "",
    "0 = Previous Data Port Operation Successful"
    "1 = Previous Data Port Operation Failed",
#endif
    LIF_DATA_PORT_COMMAND_DATA_PORT_ERROR_FIELD_MASK,
    0,
    LIF_DATA_PORT_COMMAND_DATA_PORT_ERROR_FIELD_WIDTH,
    LIF_DATA_PORT_COMMAND_DATA_PORT_ERROR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_DATA_PORT_COMMAND_RAM_SELECT
 ******************************************************************************/
const ru_field_rec LIF_DATA_PORT_COMMAND_RAM_SELECT_FIELD =
{
    "RAM_SELECT",
#if RU_INCLUDE_DESC
    "",
    "Selects the internal RAM for access:"
    "0: Downstream Statistics (per-LLID)"
    "(256 x 32 bits)"
    "1: SEC Downstream AES128 Key (64 x 225 bits)"
    "2: FEC Downstream Data (2048 x 20 bits)*"
    "3: SEC Upstream AES128 Key (32 x 234 bits)"
    "5: FEC Downstream Partial Syndrome (16 x 136 bits)*"
    "6: FEC Downstream Full Syndrome (16 x 136 bits) *"
    "7: FEC Upstream Parity (16 x 128)*"
    "8: SEC Downstream AES256 Key (64 x 225 bits)"
    "9: SEC Upstream AES256 Key (32 x 234 bits)"
    "See LIF Data Port Data register for bit descriptions of these RAMs."
    "* Module level resets must be active to access these rams.",
#endif
    LIF_DATA_PORT_COMMAND_RAM_SELECT_FIELD_MASK,
    0,
    LIF_DATA_PORT_COMMAND_RAM_SELECT_FIELD_WIDTH,
    LIF_DATA_PORT_COMMAND_RAM_SELECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_DATA_PORT_COMMAND_DATA_PORT_OP_CODE
 ******************************************************************************/
const ru_field_rec LIF_DATA_PORT_COMMAND_DATA_PORT_OP_CODE_FIELD =
{
    "DATA_PORT_OP_CODE",
#if RU_INCLUDE_DESC
    "",
    "Specifies RAM read or write operation."
    "0: Read"
    "1: Write"
    "2-255: NO OP",
#endif
    LIF_DATA_PORT_COMMAND_DATA_PORT_OP_CODE_FIELD_MASK,
    0,
    LIF_DATA_PORT_COMMAND_DATA_PORT_OP_CODE_FIELD_WIDTH,
    LIF_DATA_PORT_COMMAND_DATA_PORT_OP_CODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_DATA_PORT_COMMAND_DATA_PORT_ADDR
 ******************************************************************************/
const ru_field_rec LIF_DATA_PORT_COMMAND_DATA_PORT_ADDR_FIELD =
{
    "DATA_PORT_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Specifies the RAM address for the port operation."
    "Note: This field is also used by the LIF memory initialization"
    "logic, so it has a non-zero value after reset."
    "Default: 0xffff",
#endif
    LIF_DATA_PORT_COMMAND_DATA_PORT_ADDR_FIELD_MASK,
    0,
    LIF_DATA_PORT_COMMAND_DATA_PORT_ADDR_FIELD_WIDTH,
    LIF_DATA_PORT_COMMAND_DATA_PORT_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_DATA_PORT_DATA_0_PBIPORTDATA0
 ******************************************************************************/
const ru_field_rec LIF_DATA_PORT_DATA_0_PBIPORTDATA0_FIELD =
{
    "PBIPORTDATA0",
#if RU_INCLUDE_DESC
    "",
    "For write operations, the data to be written to the RAM location."
    "For read operations, the data read back from the RAM location.",
#endif
    LIF_DATA_PORT_DATA_0_PBIPORTDATA0_FIELD_MASK,
    0,
    LIF_DATA_PORT_DATA_0_PBIPORTDATA0_FIELD_WIDTH,
    LIF_DATA_PORT_DATA_0_PBIPORTDATA0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_DATA_PORT_DATA_1_PBIPORTDATA1
 ******************************************************************************/
const ru_field_rec LIF_DATA_PORT_DATA_1_PBIPORTDATA1_FIELD =
{
    "PBIPORTDATA1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_DATA_PORT_DATA_1_PBIPORTDATA1_FIELD_MASK,
    0,
    LIF_DATA_PORT_DATA_1_PBIPORTDATA1_FIELD_WIDTH,
    LIF_DATA_PORT_DATA_1_PBIPORTDATA1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_DATA_PORT_DATA_2_PBIPORTDATA2
 ******************************************************************************/
const ru_field_rec LIF_DATA_PORT_DATA_2_PBIPORTDATA2_FIELD =
{
    "PBIPORTDATA2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_DATA_PORT_DATA_2_PBIPORTDATA2_FIELD_MASK,
    0,
    LIF_DATA_PORT_DATA_2_PBIPORTDATA2_FIELD_WIDTH,
    LIF_DATA_PORT_DATA_2_PBIPORTDATA2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_DATA_PORT_DATA_3_PBIPORTDATA3
 ******************************************************************************/
const ru_field_rec LIF_DATA_PORT_DATA_3_PBIPORTDATA3_FIELD =
{
    "PBIPORTDATA3",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_DATA_PORT_DATA_3_PBIPORTDATA3_FIELD_MASK,
    0,
    LIF_DATA_PORT_DATA_3_PBIPORTDATA3_FIELD_WIDTH,
    LIF_DATA_PORT_DATA_3_PBIPORTDATA3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_DATA_PORT_DATA_4_PBIPORTDATA4
 ******************************************************************************/
const ru_field_rec LIF_DATA_PORT_DATA_4_PBIPORTDATA4_FIELD =
{
    "PBIPORTDATA4",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_DATA_PORT_DATA_4_PBIPORTDATA4_FIELD_MASK,
    0,
    LIF_DATA_PORT_DATA_4_PBIPORTDATA4_FIELD_WIDTH,
    LIF_DATA_PORT_DATA_4_PBIPORTDATA4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_DATA_PORT_DATA_5_PBIPORTDATA5
 ******************************************************************************/
const ru_field_rec LIF_DATA_PORT_DATA_5_PBIPORTDATA5_FIELD =
{
    "PBIPORTDATA5",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_DATA_PORT_DATA_5_PBIPORTDATA5_FIELD_MASK,
    0,
    LIF_DATA_PORT_DATA_5_PBIPORTDATA5_FIELD_WIDTH,
    LIF_DATA_PORT_DATA_5_PBIPORTDATA5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_DATA_PORT_DATA_6_PBIPORTDATA6
 ******************************************************************************/
const ru_field_rec LIF_DATA_PORT_DATA_6_PBIPORTDATA6_FIELD =
{
    "PBIPORTDATA6",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_DATA_PORT_DATA_6_PBIPORTDATA6_FIELD_MASK,
    0,
    LIF_DATA_PORT_DATA_6_PBIPORTDATA6_FIELD_WIDTH,
    LIF_DATA_PORT_DATA_6_PBIPORTDATA6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_DATA_PORT_DATA_7_PBIPORTDATA7
 ******************************************************************************/
const ru_field_rec LIF_DATA_PORT_DATA_7_PBIPORTDATA7_FIELD =
{
    "PBIPORTDATA7",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_DATA_PORT_DATA_7_PBIPORTDATA7_FIELD_MASK,
    0,
    LIF_DATA_PORT_DATA_7_PBIPORTDATA7_FIELD_WIDTH,
    LIF_DATA_PORT_DATA_7_PBIPORTDATA7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_0_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_0_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_0_RESERVED0_FIELD_WIDTH,
    LIF_LLID_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_0_CFGLLID0
 ******************************************************************************/
const ru_field_rec LIF_LLID_0_CFGLLID0_FIELD =
{
    "CFGLLID0",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 0."
    "In downstream P2P, 802.1ae mode, registers"
    "LIF_P2P_AE_SCI_LO[0:15]/LIF_P2P_AE_SCI_HI[0:15]"
    "provide lookup with packet's explicit SCI to an index.",
#endif
    LIF_LLID_0_CFGLLID0_FIELD_MASK,
    0,
    LIF_LLID_0_CFGLLID0_FIELD_WIDTH,
    LIF_LLID_0_CFGLLID0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_1_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_1_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_1_RESERVED0_FIELD_WIDTH,
    LIF_LLID_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_1_CFGLLID1
 ******************************************************************************/
const ru_field_rec LIF_LLID_1_CFGLLID1_FIELD =
{
    "CFGLLID1",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 1."
    "In downstream P2P, 802.1ae mode, registers"
    "LIF_P2P_AE_SCI_LO[0:15]/LIF_P2P_AE_SCI_HI[0:15]"
    "provide lookup with packet's explicit SCI to an index.",
#endif
    LIF_LLID_1_CFGLLID1_FIELD_MASK,
    0,
    LIF_LLID_1_CFGLLID1_FIELD_WIDTH,
    LIF_LLID_1_CFGLLID1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_2_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_2_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_2_RESERVED0_FIELD_WIDTH,
    LIF_LLID_2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_2_CFGLLID2
 ******************************************************************************/
const ru_field_rec LIF_LLID_2_CFGLLID2_FIELD =
{
    "CFGLLID2",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 2."
    "In downstream P2P, 802.1ae mode, registers"
    "LIF_P2P_AE_SCI_LO[0:15]/LIF_P2P_AE_SCI_HI[0:15]"
    "provide lookup with packet's explicit SCI to an index.",
#endif
    LIF_LLID_2_CFGLLID2_FIELD_MASK,
    0,
    LIF_LLID_2_CFGLLID2_FIELD_WIDTH,
    LIF_LLID_2_CFGLLID2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_3_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_3_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_3_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_3_RESERVED0_FIELD_WIDTH,
    LIF_LLID_3_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_3_CFGLLID3
 ******************************************************************************/
const ru_field_rec LIF_LLID_3_CFGLLID3_FIELD =
{
    "CFGLLID3",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 3."
    "In downstream P2P, 802.1ae mode, registers"
    "LIF_P2P_AE_SCI_LO[0:15]/LIF_P2P_AE_SCI_HI[0:15]"
    "provide lookup with packet's explicit SCI to an index.",
#endif
    LIF_LLID_3_CFGLLID3_FIELD_MASK,
    0,
    LIF_LLID_3_CFGLLID3_FIELD_WIDTH,
    LIF_LLID_3_CFGLLID3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_4_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_4_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_4_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_4_RESERVED0_FIELD_WIDTH,
    LIF_LLID_4_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_4_CFGLLID4
 ******************************************************************************/
const ru_field_rec LIF_LLID_4_CFGLLID4_FIELD =
{
    "CFGLLID4",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 4."
    "In downstream P2P, 802.1ae mode, registers"
    "LIF_P2P_AE_SCI_LO[0:15]/LIF_P2P_AE_SCI_HI[0:15]"
    "provide lookup with packet's explicit SCI to an index.",
#endif
    LIF_LLID_4_CFGLLID4_FIELD_MASK,
    0,
    LIF_LLID_4_CFGLLID4_FIELD_WIDTH,
    LIF_LLID_4_CFGLLID4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_5_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_5_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_5_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_5_RESERVED0_FIELD_WIDTH,
    LIF_LLID_5_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_5_CFGLLID5
 ******************************************************************************/
const ru_field_rec LIF_LLID_5_CFGLLID5_FIELD =
{
    "CFGLLID5",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 5."
    "In downstream P2P, 802.1ae mode, registers"
    "LIF_P2P_AE_SCI_LO[0:15]/LIF_P2P_AE_SCI_HI[0:15]"
    "provide lookup with packet's explicit SCI to an index.",
#endif
    LIF_LLID_5_CFGLLID5_FIELD_MASK,
    0,
    LIF_LLID_5_CFGLLID5_FIELD_WIDTH,
    LIF_LLID_5_CFGLLID5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_6_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_6_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_6_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_6_RESERVED0_FIELD_WIDTH,
    LIF_LLID_6_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_6_CFGLLID6
 ******************************************************************************/
const ru_field_rec LIF_LLID_6_CFGLLID6_FIELD =
{
    "CFGLLID6",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 6."
    "In downstream P2P, 802.1ae mode, registers"
    "LIF_P2P_AE_SCI_LO[0:15]/LIF_P2P_AE_SCI_HI[0:15]"
    "provide lookup with packet's explicit SCI to an index.",
#endif
    LIF_LLID_6_CFGLLID6_FIELD_MASK,
    0,
    LIF_LLID_6_CFGLLID6_FIELD_WIDTH,
    LIF_LLID_6_CFGLLID6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_7_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_7_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_7_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_7_RESERVED0_FIELD_WIDTH,
    LIF_LLID_7_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_7_CFGLLID7
 ******************************************************************************/
const ru_field_rec LIF_LLID_7_CFGLLID7_FIELD =
{
    "CFGLLID7",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 7."
    "In downstream P2P, 802.1ae mode, registers"
    "LIF_P2P_AE_SCI_LO[0:15]/LIF_P2P_AE_SCI_HI[0:15]"
    "provide lookup with packet's explicit SCI to an index.",
#endif
    LIF_LLID_7_CFGLLID7_FIELD_MASK,
    0,
    LIF_LLID_7_CFGLLID7_FIELD_WIDTH,
    LIF_LLID_7_CFGLLID7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_16_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_16_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_16_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_16_RESERVED0_FIELD_WIDTH,
    LIF_LLID_16_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_16_CFGLLID16
 ******************************************************************************/
const ru_field_rec LIF_LLID_16_CFGLLID16_FIELD =
{
    "CFGLLID16",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 16.",
#endif
    LIF_LLID_16_CFGLLID16_FIELD_MASK,
    0,
    LIF_LLID_16_CFGLLID16_FIELD_WIDTH,
    LIF_LLID_16_CFGLLID16_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_17_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_17_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_17_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_17_RESERVED0_FIELD_WIDTH,
    LIF_LLID_17_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_17_CFGLLID17
 ******************************************************************************/
const ru_field_rec LIF_LLID_17_CFGLLID17_FIELD =
{
    "CFGLLID17",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 17.",
#endif
    LIF_LLID_17_CFGLLID17_FIELD_MASK,
    0,
    LIF_LLID_17_CFGLLID17_FIELD_WIDTH,
    LIF_LLID_17_CFGLLID17_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_18_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_18_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_18_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_18_RESERVED0_FIELD_WIDTH,
    LIF_LLID_18_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_18_CFGLLID18
 ******************************************************************************/
const ru_field_rec LIF_LLID_18_CFGLLID18_FIELD =
{
    "CFGLLID18",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 18.",
#endif
    LIF_LLID_18_CFGLLID18_FIELD_MASK,
    0,
    LIF_LLID_18_CFGLLID18_FIELD_WIDTH,
    LIF_LLID_18_CFGLLID18_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_19_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_19_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_19_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_19_RESERVED0_FIELD_WIDTH,
    LIF_LLID_19_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_19_CFGLLID19
 ******************************************************************************/
const ru_field_rec LIF_LLID_19_CFGLLID19_FIELD =
{
    "CFGLLID19",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 19.",
#endif
    LIF_LLID_19_CFGLLID19_FIELD_MASK,
    0,
    LIF_LLID_19_CFGLLID19_FIELD_WIDTH,
    LIF_LLID_19_CFGLLID19_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_20_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_20_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_20_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_20_RESERVED0_FIELD_WIDTH,
    LIF_LLID_20_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_20_CFGLLID20
 ******************************************************************************/
const ru_field_rec LIF_LLID_20_CFGLLID20_FIELD =
{
    "CFGLLID20",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 20.",
#endif
    LIF_LLID_20_CFGLLID20_FIELD_MASK,
    0,
    LIF_LLID_20_CFGLLID20_FIELD_WIDTH,
    LIF_LLID_20_CFGLLID20_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_21_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_21_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_21_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_21_RESERVED0_FIELD_WIDTH,
    LIF_LLID_21_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_21_CFGLLID21
 ******************************************************************************/
const ru_field_rec LIF_LLID_21_CFGLLID21_FIELD =
{
    "CFGLLID21",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 21.",
#endif
    LIF_LLID_21_CFGLLID21_FIELD_MASK,
    0,
    LIF_LLID_21_CFGLLID21_FIELD_WIDTH,
    LIF_LLID_21_CFGLLID21_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_22_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_22_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_22_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_22_RESERVED0_FIELD_WIDTH,
    LIF_LLID_22_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_22_CFGLLID22
 ******************************************************************************/
const ru_field_rec LIF_LLID_22_CFGLLID22_FIELD =
{
    "CFGLLID22",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 22.",
#endif
    LIF_LLID_22_CFGLLID22_FIELD_MASK,
    0,
    LIF_LLID_22_CFGLLID22_FIELD_WIDTH,
    LIF_LLID_22_CFGLLID22_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_23_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_23_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_23_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_23_RESERVED0_FIELD_WIDTH,
    LIF_LLID_23_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_23_CFGLLID23
 ******************************************************************************/
const ru_field_rec LIF_LLID_23_CFGLLID23_FIELD =
{
    "CFGLLID23",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 23.",
#endif
    LIF_LLID_23_CFGLLID23_FIELD_MASK,
    0,
    LIF_LLID_23_CFGLLID23_FIELD_WIDTH,
    LIF_LLID_23_CFGLLID23_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_TIME_REF_CNT_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_TIME_REF_CNT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_TIME_REF_CNT_RESERVED0_FIELD_MASK,
    0,
    LIF_TIME_REF_CNT_RESERVED0_FIELD_WIDTH,
    LIF_TIME_REF_CNT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_TIME_REF_CNT_CFFULLUPDATEVALUE
 ******************************************************************************/
const ru_field_rec LIF_TIME_REF_CNT_CFFULLUPDATEVALUE_FIELD =
{
    "CFFULLUPDATEVALUE",
#if RU_INCLUDE_DESC
    "",
    "If the (absolute) difference between the timestamp received in a"
    "GATE message and the MPCP timer is larger than this value, then a"
    "full update\" occurs: the downstream timestamp is transferred into"
    "the MPCP timer."
    "Default: 0x20",
#endif
    LIF_TIME_REF_CNT_CFFULLUPDATEVALUE_FIELD_MASK,
    0,
    LIF_TIME_REF_CNT_CFFULLUPDATEVALUE_FIELD_WIDTH,
    LIF_TIME_REF_CNT_CFFULLUPDATEVALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_TIME_REF_CNT_CFMAXNEGVALUE
 ******************************************************************************/
const ru_field_rec LIF_TIME_REF_CNT_CFMAXNEGVALUE_FIELD =
{
    "CFMAXNEGVALUE",
#if RU_INCLUDE_DESC
    "",
    "If the difference between the timestamp received in a GATE message"
    "and the MPCP timer is negative AND larger than this value (but not"
    "larger than cfFullUpdate) then the MPCP timer is held for one TQ"
    "(effectively decrementing it)."
    "Default: 0x2",
#endif
    LIF_TIME_REF_CNT_CFMAXNEGVALUE_FIELD_MASK,
    0,
    LIF_TIME_REF_CNT_CFMAXNEGVALUE_FIELD_WIDTH,
    LIF_TIME_REF_CNT_CFMAXNEGVALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_TIME_REF_CNT_CFMAXPOSVALUE
 ******************************************************************************/
const ru_field_rec LIF_TIME_REF_CNT_CFMAXPOSVALUE_FIELD =
{
    "CFMAXPOSVALUE",
#if RU_INCLUDE_DESC
    "",
    "If the difference between the timestamp received in a GATE message"
    "and the MPCP timer is positive AND larger than this value (but not"
    "larger than cfFullUpdate) then the MPCP timer is incremented by two"
    "TQ."
    "Default: 0x4",
#endif
    LIF_TIME_REF_CNT_CFMAXPOSVALUE_FIELD_MASK,
    0,
    LIF_TIME_REF_CNT_CFMAXPOSVALUE_FIELD_WIDTH,
    LIF_TIME_REF_CNT_CFMAXPOSVALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_TIMESTAMP_UPD_PER_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_TIMESTAMP_UPD_PER_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_TIMESTAMP_UPD_PER_RESERVED0_FIELD_MASK,
    0,
    LIF_TIMESTAMP_UPD_PER_RESERVED0_FIELD_WIDTH,
    LIF_TIMESTAMP_UPD_PER_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_TIMESTAMP_UPD_PER_CFTIMESTAMPUPDPER
 ******************************************************************************/
const ru_field_rec LIF_TIMESTAMP_UPD_PER_CFTIMESTAMPUPDPER_FIELD =
{
    "CFTIMESTAMPUPDPER",
#if RU_INCLUDE_DESC
    "",
    "Time period after an MPCP time correction during which LIF ignores"
    "further MPCP time corrections."
    "The units are TQ.",
#endif
    LIF_TIMESTAMP_UPD_PER_CFTIMESTAMPUPDPER_FIELD_MASK,
    0,
    LIF_TIMESTAMP_UPD_PER_CFTIMESTAMPUPDPER_FIELD_WIDTH,
    LIF_TIMESTAMP_UPD_PER_CFTIMESTAMPUPDPER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_TP_TIME_CFTRANSPORTTIME
 ******************************************************************************/
const ru_field_rec LIF_TP_TIME_CFTRANSPORTTIME_FIELD =
{
    "CFTRANSPORTTIME",
#if RU_INCLUDE_DESC
    "",
    "MPCP time at which the pulse per second output will be asserted.",
#endif
    LIF_TP_TIME_CFTRANSPORTTIME_FIELD_MASK,
    0,
    LIF_TP_TIME_CFTRANSPORTTIME_FIELD_WIDTH,
    LIF_TP_TIME_CFTRANSPORTTIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_MPCP_TIME_LTMPCPTIME
 ******************************************************************************/
const ru_field_rec LIF_MPCP_TIME_LTMPCPTIME_FIELD =
{
    "LTMPCPTIME",
#if RU_INCLUDE_DESC
    "",
    "Provides the least significant 32 bits of the receive time.",
#endif
    LIF_MPCP_TIME_LTMPCPTIME_FIELD_MASK,
    0,
    LIF_MPCP_TIME_LTMPCPTIME_FIELD_WIDTH,
    LIF_MPCP_TIME_LTMPCPTIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_MAXLEN_CTR_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_MAXLEN_CTR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_MAXLEN_CTR_RESERVED0_FIELD_MASK,
    0,
    LIF_MAXLEN_CTR_RESERVED0_FIELD_WIDTH,
    LIF_MAXLEN_CTR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_MAXLEN_CTR_CFRXMAXFRAMELENGTH
 ******************************************************************************/
const ru_field_rec LIF_MAXLEN_CTR_CFRXMAXFRAMELENGTH_FIELD =
{
    "CFRXMAXFRAMELENGTH",
#if RU_INCLUDE_DESC
    "",
    "Approlifately the malifum number of double words which the frame can"
    "contain before being truncated by the LIF Sanitizer circuit. The"
    "default of 1005 corresponds to a frame length of 2000 bytes."
    "Default: 1005",
#endif
    LIF_MAXLEN_CTR_CFRXMAXFRAMELENGTH_FIELD_MASK,
    0,
    LIF_MAXLEN_CTR_CFRXMAXFRAMELENGTH_FIELD_WIDTH,
    LIF_MAXLEN_CTR_CFRXMAXFRAMELENGTH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LASER_ON_DELTA_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LASER_ON_DELTA_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LASER_ON_DELTA_RESERVED0_FIELD_MASK,
    0,
    LIF_LASER_ON_DELTA_RESERVED0_FIELD_WIDTH,
    LIF_LASER_ON_DELTA_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LASER_ON_DELTA_CFTXLASERONDELTA
 ******************************************************************************/
const ru_field_rec LIF_LASER_ON_DELTA_CFTXLASERONDELTA_FIELD =
{
    "CFTXLASERONDELTA",
#if RU_INCLUDE_DESC
    "",
    "[11:0] Offset (+/-) from Grant Start time to turn laser on"
    "[12]    0: Positive value: turn on laser after the grant start time."
    "1: Negative value: turn on laser prior to the grant start"
    "time",
#endif
    LIF_LASER_ON_DELTA_CFTXLASERONDELTA_FIELD_MASK,
    0,
    LIF_LASER_ON_DELTA_CFTXLASERONDELTA_FIELD_WIDTH,
    LIF_LASER_ON_DELTA_CFTXLASERONDELTA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LASER_OFF_IDLE_CFTXINITIDLE
 ******************************************************************************/
const ru_field_rec LIF_LASER_OFF_IDLE_CFTXINITIDLE_FIELD =
{
    "CFTXINITIDLE",
#if RU_INCLUDE_DESC
    "",
    "A period during which the LIF transmits idle characters before the"
    "transmission burst of non-FEC frames.",
#endif
    LIF_LASER_OFF_IDLE_CFTXINITIDLE_FIELD_MASK,
    0,
    LIF_LASER_OFF_IDLE_CFTXINITIDLE_FIELD_WIDTH,
    LIF_LASER_OFF_IDLE_CFTXINITIDLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LASER_OFF_IDLE_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LASER_OFF_IDLE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LASER_OFF_IDLE_RESERVED0_FIELD_MASK,
    0,
    LIF_LASER_OFF_IDLE_RESERVED0_FIELD_WIDTH,
    LIF_LASER_OFF_IDLE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LASER_OFF_IDLE_CFTXLASEROFFDELTA
 ******************************************************************************/
const ru_field_rec LIF_LASER_OFF_IDLE_CFTXLASEROFFDELTA_FIELD =
{
    "CFTXLASEROFFDELTA",
#if RU_INCLUDE_DESC
    "",
    "[6:0] Offset (+/-) from Grant End time at which to turn laser off"
    "[7]: 0: Positive value: turn off laser after end of grant slot."
    "1: Negative value: turn off laser before end of grant slot.",
#endif
    LIF_LASER_OFF_IDLE_CFTXLASEROFFDELTA_FIELD_MASK,
    0,
    LIF_LASER_OFF_IDLE_CFTXLASEROFFDELTA_FIELD_WIDTH,
    LIF_LASER_OFF_IDLE_CFTXLASEROFFDELTA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_FEC_INIT_IDLE_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_FEC_INIT_IDLE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_FEC_INIT_IDLE_RESERVED0_FIELD_MASK,
    0,
    LIF_FEC_INIT_IDLE_RESERVED0_FIELD_WIDTH,
    LIF_FEC_INIT_IDLE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_FEC_INIT_IDLE_CFTXFECINITIDLE
 ******************************************************************************/
const ru_field_rec LIF_FEC_INIT_IDLE_CFTXFECINITIDLE_FIELD =
{
    "CFTXFECINITIDLE",
#if RU_INCLUDE_DESC
    "",
    "A period during which the LIF transmits idle characters before the"
    "transmission burst of FEC frames.",
#endif
    LIF_FEC_INIT_IDLE_CFTXFECINITIDLE_FIELD_MASK,
    0,
    LIF_FEC_INIT_IDLE_CFTXFECINITIDLE_FIELD_WIDTH,
    LIF_FEC_INIT_IDLE_CFTXFECINITIDLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_FEC_ERR_ALLOW_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_FEC_ERR_ALLOW_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_FEC_ERR_ALLOW_RESERVED0_FIELD_MASK,
    0,
    LIF_FEC_ERR_ALLOW_RESERVED0_FIELD_WIDTH,
    LIF_FEC_ERR_ALLOW_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_FEC_ERR_ALLOW_CFRXTFECBITERRALLOW
 ******************************************************************************/
const ru_field_rec LIF_FEC_ERR_ALLOW_CFRXTFECBITERRALLOW_FIELD =
{
    "CFRXTFECBITERRALLOW",
#if RU_INCLUDE_DESC
    "",
    "The number of bit error allow for TFEC detection."
    "Default : 0x5",
#endif
    LIF_FEC_ERR_ALLOW_CFRXTFECBITERRALLOW_FIELD_MASK,
    0,
    LIF_FEC_ERR_ALLOW_CFRXTFECBITERRALLOW_FIELD_WIDTH,
    LIF_FEC_ERR_ALLOW_CFRXTFECBITERRALLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_FEC_ERR_ALLOW_CFRXSFECBITERRALLOW
 ******************************************************************************/
const ru_field_rec LIF_FEC_ERR_ALLOW_CFRXSFECBITERRALLOW_FIELD =
{
    "CFRXSFECBITERRALLOW",
#if RU_INCLUDE_DESC
    "",
    "The number of bit error allow for SFEC detection."
    "Default : 0x5",
#endif
    LIF_FEC_ERR_ALLOW_CFRXSFECBITERRALLOW_FIELD_MASK,
    0,
    LIF_FEC_ERR_ALLOW_CFRXSFECBITERRALLOW_FIELD_WIDTH,
    LIF_FEC_ERR_ALLOW_CFRXSFECBITERRALLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_KEY_SEL_KEYSEL
 ******************************************************************************/
const ru_field_rec LIF_SEC_KEY_SEL_KEYSEL_FIELD =
{
    "KEYSEL",
#if RU_INCLUDE_DESC
    "",
    "[31:0] Key select status for bidirectional LLIDs; bitwise encoded"
    "per LLID index",
#endif
    LIF_SEC_KEY_SEL_KEYSEL_FIELD_MASK,
    0,
    LIF_SEC_KEY_SEL_KEYSEL_FIELD_WIDTH,
    LIF_SEC_KEY_SEL_KEYSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_DN_ENCRYPT_STAT_ENENCRYPT
 ******************************************************************************/
const ru_field_rec LIF_DN_ENCRYPT_STAT_ENENCRYPT_FIELD =
{
    "ENENCRYPT",
#if RU_INCLUDE_DESC
    "",
    "Provides the status of the current encryption mode for each LLID. In"
    "EPON mode (and with bit enEpnMixEncryption set in the LIF Control"
    "Register) encryption for an LLID can be disabled only by writing \"1"
    "to the appropriate bit in this register."
    "0: Encryption disabled"
    "1: Encryption enabled"
    "Bitwise encoded per LLID index.",
#endif
    LIF_DN_ENCRYPT_STAT_ENENCRYPT_FIELD_MASK,
    0,
    LIF_DN_ENCRYPT_STAT_ENENCRYPT_FIELD_WIDTH,
    LIF_DN_ENCRYPT_STAT_ENENCRYPT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_UP_KEY_STAT_KEYUPSEL
 ******************************************************************************/
const ru_field_rec LIF_SEC_UP_KEY_STAT_KEYUPSEL_FIELD =
{
    "KEYUPSEL",
#if RU_INCLUDE_DESC
    "",
    "Provides current active key for upstream LLIDs. Bitwise encoded per"
    "LLID index.",
#endif
    LIF_SEC_UP_KEY_STAT_KEYUPSEL_FIELD_MASK,
    0,
    LIF_SEC_UP_KEY_STAT_KEYUPSEL_FIELD_WIDTH,
    LIF_SEC_UP_KEY_STAT_KEYUPSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_SEC_UP_ENCRYPT_STAT_ENUPENCRYPT
 ******************************************************************************/
const ru_field_rec LIF_SEC_UP_ENCRYPT_STAT_ENUPENCRYPT_FIELD =
{
    "ENUPENCRYPT",
#if RU_INCLUDE_DESC
    "",
    "Provides per-LLID status of the current upstream encryption mode."
    "0: Encryption disabled"
    "1: Encryption enabled"
    "Bitwise encoded per LLID index.",
#endif
    LIF_SEC_UP_ENCRYPT_STAT_ENUPENCRYPT_FIELD_MASK,
    0,
    LIF_SEC_UP_ENCRYPT_STAT_ENUPENCRYPT_FIELD_WIDTH,
    LIF_SEC_UP_ENCRYPT_STAT_ENUPENCRYPT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_SEC_UP_MPCP_OFFSET_SECUPMPCPOFFSET
 ******************************************************************************/
const ru_field_rec LIF_SEC_UP_MPCP_OFFSET_SECUPMPCPOFFSET_FIELD =
{
    "SECUPMPCPOFFSET",
#if RU_INCLUDE_DESC
    "",
    "Provides MPCP offset correction.",
#endif
    LIF_SEC_UP_MPCP_OFFSET_SECUPMPCPOFFSET_FIELD_MASK,
    0,
    LIF_SEC_UP_MPCP_OFFSET_SECUPMPCPOFFSET_FIELD_WIDTH,
    LIF_SEC_UP_MPCP_OFFSET_SECUPMPCPOFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_FEC_PER_LLID_CFFECTXFECENLLID
 ******************************************************************************/
const ru_field_rec LIF_FEC_PER_LLID_CFFECTXFECENLLID_FIELD =
{
    "CFFECTXFECENLLID",
#if RU_INCLUDE_DESC
    "",
    "Per-LLID FEC Enable for LLID 0-31.",
#endif
    LIF_FEC_PER_LLID_CFFECTXFECENLLID_FIELD_MASK,
    0,
    LIF_FEC_PER_LLID_CFFECTXFECENLLID_FIELD_WIDTH,
    LIF_FEC_PER_LLID_CFFECTXFECENLLID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_RX_LINE_CODE_ERR_CNT_RXLINECODEERRCNT
 ******************************************************************************/
const ru_field_rec LIF_RX_LINE_CODE_ERR_CNT_RXLINECODEERRCNT_FIELD =
{
    "RXLINECODEERRCNT",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_RX_LINE_CODE_ERR_CNT_RXLINECODEERRCNT_FIELD_MASK,
    0,
    LIF_RX_LINE_CODE_ERR_CNT_RXLINECODEERRCNT_FIELD_WIDTH,
    LIF_RX_LINE_CODE_ERR_CNT_RXLINECODEERRCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_RX_AGG_MPCP_FRM_RXAGGMPCPCNT
 ******************************************************************************/
const ru_field_rec LIF_RX_AGG_MPCP_FRM_RXAGGMPCPCNT_FIELD =
{
    "RXAGGMPCPCNT",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_RX_AGG_MPCP_FRM_RXAGGMPCPCNT_FIELD_MASK,
    0,
    LIF_RX_AGG_MPCP_FRM_RXAGGMPCPCNT_FIELD_WIDTH,
    LIF_RX_AGG_MPCP_FRM_RXAGGMPCPCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_RX_AGG_GOOD_FRM_RXAGGGOODCNT
 ******************************************************************************/
const ru_field_rec LIF_RX_AGG_GOOD_FRM_RXAGGGOODCNT_FIELD =
{
    "RXAGGGOODCNT",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_RX_AGG_GOOD_FRM_RXAGGGOODCNT_FIELD_MASK,
    0,
    LIF_RX_AGG_GOOD_FRM_RXAGGGOODCNT_FIELD_WIDTH,
    LIF_RX_AGG_GOOD_FRM_RXAGGGOODCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_RX_AGG_GOOD_BYTE_RXAGGGOODBYTESCNT
 ******************************************************************************/
const ru_field_rec LIF_RX_AGG_GOOD_BYTE_RXAGGGOODBYTESCNT_FIELD =
{
    "RXAGGGOODBYTESCNT",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_RX_AGG_GOOD_BYTE_RXAGGGOODBYTESCNT_FIELD_MASK,
    0,
    LIF_RX_AGG_GOOD_BYTE_RXAGGGOODBYTESCNT_FIELD_WIDTH,
    LIF_RX_AGG_GOOD_BYTE_RXAGGGOODBYTESCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_RX_AGG_UNDERSZ_FRM_RXAGGUNDERSZCNT
 ******************************************************************************/
const ru_field_rec LIF_RX_AGG_UNDERSZ_FRM_RXAGGUNDERSZCNT_FIELD =
{
    "RXAGGUNDERSZCNT",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_RX_AGG_UNDERSZ_FRM_RXAGGUNDERSZCNT_FIELD_MASK,
    0,
    LIF_RX_AGG_UNDERSZ_FRM_RXAGGUNDERSZCNT_FIELD_WIDTH,
    LIF_RX_AGG_UNDERSZ_FRM_RXAGGUNDERSZCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_RX_AGG_OVERSZ_FRM_RXAGGOVERSZCNT
 ******************************************************************************/
const ru_field_rec LIF_RX_AGG_OVERSZ_FRM_RXAGGOVERSZCNT_FIELD =
{
    "RXAGGOVERSZCNT",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_RX_AGG_OVERSZ_FRM_RXAGGOVERSZCNT_FIELD_MASK,
    0,
    LIF_RX_AGG_OVERSZ_FRM_RXAGGOVERSZCNT_FIELD_WIDTH,
    LIF_RX_AGG_OVERSZ_FRM_RXAGGOVERSZCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_RX_AGG_CRC8_FRM_RXAGGCRC8ERRCNT
 ******************************************************************************/
const ru_field_rec LIF_RX_AGG_CRC8_FRM_RXAGGCRC8ERRCNT_FIELD =
{
    "RXAGGCRC8ERRCNT",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_RX_AGG_CRC8_FRM_RXAGGCRC8ERRCNT_FIELD_MASK,
    0,
    LIF_RX_AGG_CRC8_FRM_RXAGGCRC8ERRCNT_FIELD_WIDTH,
    LIF_RX_AGG_CRC8_FRM_RXAGGCRC8ERRCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_RX_AGG_FEC_FRM_RXAGGFEC
 ******************************************************************************/
const ru_field_rec LIF_RX_AGG_FEC_FRM_RXAGGFEC_FIELD =
{
    "RXAGGFEC",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_RX_AGG_FEC_FRM_RXAGGFEC_FIELD_MASK,
    0,
    LIF_RX_AGG_FEC_FRM_RXAGGFEC_FIELD_WIDTH,
    LIF_RX_AGG_FEC_FRM_RXAGGFEC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_RX_AGG_FEC_BYTE_RXAGGFECBYTES
 ******************************************************************************/
const ru_field_rec LIF_RX_AGG_FEC_BYTE_RXAGGFECBYTES_FIELD =
{
    "RXAGGFECBYTES",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_RX_AGG_FEC_BYTE_RXAGGFECBYTES_FIELD_MASK,
    0,
    LIF_RX_AGG_FEC_BYTE_RXAGGFECBYTES_FIELD_WIDTH,
    LIF_RX_AGG_FEC_BYTE_RXAGGFECBYTES_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_RX_AGG_FEC_EXC_ERR_FRM_RXAGGFECEXCEEDERRS
 ******************************************************************************/
const ru_field_rec LIF_RX_AGG_FEC_EXC_ERR_FRM_RXAGGFECEXCEEDERRS_FIELD =
{
    "RXAGGFECEXCEEDERRS",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_RX_AGG_FEC_EXC_ERR_FRM_RXAGGFECEXCEEDERRS_FIELD_MASK,
    0,
    LIF_RX_AGG_FEC_EXC_ERR_FRM_RXAGGFECEXCEEDERRS_FIELD_WIDTH,
    LIF_RX_AGG_FEC_EXC_ERR_FRM_RXAGGFECEXCEEDERRS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_RX_AGG_NONFEC_GOOD_FRM_RXAGGNONFECGOOD
 ******************************************************************************/
const ru_field_rec LIF_RX_AGG_NONFEC_GOOD_FRM_RXAGGNONFECGOOD_FIELD =
{
    "RXAGGNONFECGOOD",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_RX_AGG_NONFEC_GOOD_FRM_RXAGGNONFECGOOD_FIELD_MASK,
    0,
    LIF_RX_AGG_NONFEC_GOOD_FRM_RXAGGNONFECGOOD_FIELD_WIDTH,
    LIF_RX_AGG_NONFEC_GOOD_FRM_RXAGGNONFECGOOD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_RX_AGG_NONFEC_GOOD_BYTE_RXAGGNONFECGOODBYTES
 ******************************************************************************/
const ru_field_rec LIF_RX_AGG_NONFEC_GOOD_BYTE_RXAGGNONFECGOODBYTES_FIELD =
{
    "RXAGGNONFECGOODBYTES",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_RX_AGG_NONFEC_GOOD_BYTE_RXAGGNONFECGOODBYTES_FIELD_MASK,
    0,
    LIF_RX_AGG_NONFEC_GOOD_BYTE_RXAGGNONFECGOODBYTES_FIELD_WIDTH,
    LIF_RX_AGG_NONFEC_GOOD_BYTE_RXAGGNONFECGOODBYTES_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_RX_AGG_ERR_BYTES_RXAGGERRBYTES
 ******************************************************************************/
const ru_field_rec LIF_RX_AGG_ERR_BYTES_RXAGGERRBYTES_FIELD =
{
    "RXAGGERRBYTES",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_RX_AGG_ERR_BYTES_RXAGGERRBYTES_FIELD_MASK,
    0,
    LIF_RX_AGG_ERR_BYTES_RXAGGERRBYTES_FIELD_WIDTH,
    LIF_RX_AGG_ERR_BYTES_RXAGGERRBYTES_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_RX_AGG_ERR_ZEROES_RXAGGERRZEROES
 ******************************************************************************/
const ru_field_rec LIF_RX_AGG_ERR_ZEROES_RXAGGERRZEROES_FIELD =
{
    "RXAGGERRZEROES",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_RX_AGG_ERR_ZEROES_RXAGGERRZEROES_FIELD_MASK,
    0,
    LIF_RX_AGG_ERR_ZEROES_RXAGGERRZEROES_FIELD_WIDTH,
    LIF_RX_AGG_ERR_ZEROES_RXAGGERRZEROES_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_RX_AGG_NO_ERR_BLKS_RXAGGNOERRBLKS
 ******************************************************************************/
const ru_field_rec LIF_RX_AGG_NO_ERR_BLKS_RXAGGNOERRBLKS_FIELD =
{
    "RXAGGNOERRBLKS",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_RX_AGG_NO_ERR_BLKS_RXAGGNOERRBLKS_FIELD_MASK,
    0,
    LIF_RX_AGG_NO_ERR_BLKS_RXAGGNOERRBLKS_FIELD_WIDTH,
    LIF_RX_AGG_NO_ERR_BLKS_RXAGGNOERRBLKS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_RX_AGG_COR_BLKS_RXAGGCORRBLKS
 ******************************************************************************/
const ru_field_rec LIF_RX_AGG_COR_BLKS_RXAGGCORRBLKS_FIELD =
{
    "RXAGGCORRBLKS",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_RX_AGG_COR_BLKS_RXAGGCORRBLKS_FIELD_MASK,
    0,
    LIF_RX_AGG_COR_BLKS_RXAGGCORRBLKS_FIELD_WIDTH,
    LIF_RX_AGG_COR_BLKS_RXAGGCORRBLKS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_RX_AGG_UNCOR_BLKS_RXAGGUNCORRBLKS
 ******************************************************************************/
const ru_field_rec LIF_RX_AGG_UNCOR_BLKS_RXAGGUNCORRBLKS_FIELD =
{
    "RXAGGUNCORRBLKS",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_RX_AGG_UNCOR_BLKS_RXAGGUNCORRBLKS_FIELD_MASK,
    0,
    LIF_RX_AGG_UNCOR_BLKS_RXAGGUNCORRBLKS_FIELD_WIDTH,
    LIF_RX_AGG_UNCOR_BLKS_RXAGGUNCORRBLKS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_RX_AGG_ERR_ONES_RXAGGERRONES
 ******************************************************************************/
const ru_field_rec LIF_RX_AGG_ERR_ONES_RXAGGERRONES_FIELD =
{
    "RXAGGERRONES",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_RX_AGG_ERR_ONES_RXAGGERRONES_FIELD_MASK,
    0,
    LIF_RX_AGG_ERR_ONES_RXAGGERRONES_FIELD_WIDTH,
    LIF_RX_AGG_ERR_ONES_RXAGGERRONES_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_RX_AGG_ERR_FRM_RXAGGERROREDCNT
 ******************************************************************************/
const ru_field_rec LIF_RX_AGG_ERR_FRM_RXAGGERROREDCNT_FIELD =
{
    "RXAGGERROREDCNT",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_RX_AGG_ERR_FRM_RXAGGERROREDCNT_FIELD_MASK,
    0,
    LIF_RX_AGG_ERR_FRM_RXAGGERROREDCNT_FIELD_WIDTH,
    LIF_RX_AGG_ERR_FRM_RXAGGERROREDCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_TX_PKT_CNT_TXFRAMECNT
 ******************************************************************************/
const ru_field_rec LIF_TX_PKT_CNT_TXFRAMECNT_FIELD =
{
    "TXFRAMECNT",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_TX_PKT_CNT_TXFRAMECNT_FIELD_MASK,
    0,
    LIF_TX_PKT_CNT_TXFRAMECNT_FIELD_WIDTH,
    LIF_TX_PKT_CNT_TXFRAMECNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_TX_BYTE_CNT_TXBYTECNT
 ******************************************************************************/
const ru_field_rec LIF_TX_BYTE_CNT_TXBYTECNT_FIELD =
{
    "TXBYTECNT",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_TX_BYTE_CNT_TXBYTECNT_FIELD_MASK,
    0,
    LIF_TX_BYTE_CNT_TXBYTECNT_FIELD_WIDTH,
    LIF_TX_BYTE_CNT_TXBYTECNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_TX_NON_FEC_PKT_CNT_TXNONFECFRAMECNT
 ******************************************************************************/
const ru_field_rec LIF_TX_NON_FEC_PKT_CNT_TXNONFECFRAMECNT_FIELD =
{
    "TXNONFECFRAMECNT",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_TX_NON_FEC_PKT_CNT_TXNONFECFRAMECNT_FIELD_MASK,
    0,
    LIF_TX_NON_FEC_PKT_CNT_TXNONFECFRAMECNT_FIELD_WIDTH,
    LIF_TX_NON_FEC_PKT_CNT_TXNONFECFRAMECNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_TX_NON_FEC_BYTE_CNT_TXNONFECBYTECNT
 ******************************************************************************/
const ru_field_rec LIF_TX_NON_FEC_BYTE_CNT_TXNONFECBYTECNT_FIELD =
{
    "TXNONFECBYTECNT",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_TX_NON_FEC_BYTE_CNT_TXNONFECBYTECNT_FIELD_MASK,
    0,
    LIF_TX_NON_FEC_BYTE_CNT_TXNONFECBYTECNT_FIELD_WIDTH,
    LIF_TX_NON_FEC_BYTE_CNT_TXNONFECBYTECNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_TX_FEC_PKT_CNT_TXFECFRAMECNT
 ******************************************************************************/
const ru_field_rec LIF_TX_FEC_PKT_CNT_TXFECFRAMECNT_FIELD =
{
    "TXFECFRAMECNT",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_TX_FEC_PKT_CNT_TXFECFRAMECNT_FIELD_MASK,
    0,
    LIF_TX_FEC_PKT_CNT_TXFECFRAMECNT_FIELD_WIDTH,
    LIF_TX_FEC_PKT_CNT_TXFECFRAMECNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_TX_FEC_BYTE_CNT_TXFECBYTECNT
 ******************************************************************************/
const ru_field_rec LIF_TX_FEC_BYTE_CNT_TXFECBYTECNT_FIELD =
{
    "TXFECBYTECNT",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_TX_FEC_BYTE_CNT_TXFECBYTECNT_FIELD_MASK,
    0,
    LIF_TX_FEC_BYTE_CNT_TXFECBYTECNT_FIELD_WIDTH,
    LIF_TX_FEC_BYTE_CNT_TXFECBYTECNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_TX_FEC_BLK_CNT_TXFECBLKSCNT
 ******************************************************************************/
const ru_field_rec LIF_TX_FEC_BLK_CNT_TXFECBLKSCNT_FIELD =
{
    "TXFECBLKSCNT",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_TX_FEC_BLK_CNT_TXFECBLKSCNT_FIELD_MASK,
    0,
    LIF_TX_FEC_BLK_CNT_TXFECBLKSCNT_FIELD_WIDTH,
    LIF_TX_FEC_BLK_CNT_TXFECBLKSCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_TX_MPCP_PKT_CNT_TXMPCPFRAMECNT
 ******************************************************************************/
const ru_field_rec LIF_TX_MPCP_PKT_CNT_TXMPCPFRAMECNT_FIELD =
{
    "TXMPCPFRAMECNT",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_TX_MPCP_PKT_CNT_TXMPCPFRAMECNT_FIELD_MASK,
    0,
    LIF_TX_MPCP_PKT_CNT_TXMPCPFRAMECNT_FIELD_WIDTH,
    LIF_TX_MPCP_PKT_CNT_TXMPCPFRAMECNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_DEBUG_TX_DATA_PKT_CNT_TXDATAFRAMECNT
 ******************************************************************************/
const ru_field_rec LIF_DEBUG_TX_DATA_PKT_CNT_TXDATAFRAMECNT_FIELD =
{
    "TXDATAFRAMECNT",
#if RU_INCLUDE_DESC
    "",
    "Counter value.",
#endif
    LIF_DEBUG_TX_DATA_PKT_CNT_TXDATAFRAMECNT_FIELD_MASK,
    0,
    LIF_DEBUG_TX_DATA_PKT_CNT_TXDATAFRAMECNT_FIELD_WIDTH,
    LIF_DEBUG_TX_DATA_PKT_CNT_TXDATAFRAMECNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_FEC_LLID_STATUS_STKYFECREVCLLIDBMSK
 ******************************************************************************/
const ru_field_rec LIF_FEC_LLID_STATUS_STKYFECREVCLLIDBMSK_FIELD =
{
    "STKYFECREVCLLIDBMSK",
#if RU_INCLUDE_DESC
    "",
    "[31:0]  stkyFecFecvLlid Per-LLID FEC receive status for"
    "bidirectional LLIDs."
    "0: No FEC frames detected"
    "1: FEC frame reception detected",
#endif
    LIF_FEC_LLID_STATUS_STKYFECREVCLLIDBMSK_FIELD_MASK,
    0,
    LIF_FEC_LLID_STATUS_STKYFECREVCLLIDBMSK_FIELD_WIDTH,
    LIF_FEC_LLID_STATUS_STKYFECREVCLLIDBMSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_RX_TEK_IG_IV_LLID_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_SEC_RX_TEK_IG_IV_LLID_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_SEC_RX_TEK_IG_IV_LLID_RESERVED0_FIELD_MASK,
    0,
    LIF_SEC_RX_TEK_IG_IV_LLID_RESERVED0_FIELD_WIDTH,
    LIF_SEC_RX_TEK_IG_IV_LLID_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_RX_TEK_IG_IV_LLID_CFIGIVNULLLLID
 ******************************************************************************/
const ru_field_rec LIF_SEC_RX_TEK_IG_IV_LLID_CFIGIVNULLLLID_FIELD =
{
    "CFIGIVNULLLLID",
#if RU_INCLUDE_DESC
    "",
    "[15:0]  cfIgIvNullLlid    Program with the 16 bit LLID of the"
    "Raman generated random frames."
    "[16]    cfIgIvNullLlidEn    Enable Ignore LLID functionality.",
#endif
    LIF_SEC_RX_TEK_IG_IV_LLID_CFIGIVNULLLLID_FIELD_MASK,
    0,
    LIF_SEC_RX_TEK_IG_IV_LLID_CFIGIVNULLLLID_FIELD_WIDTH,
    LIF_SEC_RX_TEK_IG_IV_LLID_CFIGIVNULLLLID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_BER_INTERV_THRESH_CFRXLIFBERINTERVAL
 ******************************************************************************/
const ru_field_rec LIF_PON_BER_INTERV_THRESH_CFRXLIFBERINTERVAL_FIELD =
{
    "CFRXLIFBERINTERVAL",
#if RU_INCLUDE_DESC
    "",
    "Programmable interval of time. Units are in 16 ns increments, for a"
    "malifum of 1 ms.",
#endif
    LIF_PON_BER_INTERV_THRESH_CFRXLIFBERINTERVAL_FIELD_MASK,
    0,
    LIF_PON_BER_INTERV_THRESH_CFRXLIFBERINTERVAL_FIELD_WIDTH,
    LIF_PON_BER_INTERV_THRESH_CFRXLIFBERINTERVAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_BER_INTERV_THRESH_CFRXLIFBERTHRESHLD
 ******************************************************************************/
const ru_field_rec LIF_PON_BER_INTERV_THRESH_CFRXLIFBERTHRESHLD_FIELD =
{
    "CFRXLIFBERTHRESHLD",
#if RU_INCLUDE_DESC
    "",
    "Programmable error threshold. Malifum number of errors seen within a"
    "programmed interval.",
#endif
    LIF_PON_BER_INTERV_THRESH_CFRXLIFBERTHRESHLD_FIELD_MASK,
    0,
    LIF_PON_BER_INTERV_THRESH_CFRXLIFBERTHRESHLD_FIELD_WIDTH,
    LIF_PON_BER_INTERV_THRESH_CFRXLIFBERTHRESHLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_BER_INTERV_THRESH_CFRXLIFBERCNTRL
 ******************************************************************************/
const ru_field_rec LIF_PON_BER_INTERV_THRESH_CFRXLIFBERCNTRL_FIELD =
{
    "CFRXLIFBERCNTRL",
#if RU_INCLUDE_DESC
    "",
    "0: Disabled"
    "1: Count Line Code Errors"
    "2: Count Corrected Symbols"
    "3: Count Uncorrectable Blocks (9 symbol errors)",
#endif
    LIF_PON_BER_INTERV_THRESH_CFRXLIFBERCNTRL_FIELD_MASK,
    0,
    LIF_PON_BER_INTERV_THRESH_CFRXLIFBERCNTRL_FIELD_WIDTH,
    LIF_PON_BER_INTERV_THRESH_CFRXLIFBERCNTRL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LSR_MON_A_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LSR_MON_A_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LSR_MON_A_CTRL_RESERVED0_FIELD_MASK,
    0,
    LIF_LSR_MON_A_CTRL_RESERVED0_FIELD_WIDTH,
    LIF_LSR_MON_A_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LSR_MON_A_CTRL_IOPBILASERENS1A
 ******************************************************************************/
const ru_field_rec LIF_LSR_MON_A_CTRL_IOPBILASERENS1A_FIELD =
{
    "IOPBILASERENS1A",
#if RU_INCLUDE_DESC
    "",
    "Provides status of laser enable, directly from the I/O pin input"
    "stage.",
#endif
    LIF_LSR_MON_A_CTRL_IOPBILASERENS1A_FIELD_MASK,
    0,
    LIF_LSR_MON_A_CTRL_IOPBILASERENS1A_FIELD_WIDTH,
    LIF_LSR_MON_A_CTRL_IOPBILASERENS1A_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LSR_MON_A_CTRL_CFGLSRMONACTHI
 ******************************************************************************/
const ru_field_rec LIF_LSR_MON_A_CTRL_CFGLSRMONACTHI_FIELD =
{
    "CFGLSRMONACTHI",
#if RU_INCLUDE_DESC
    "",
    "Laser monitor polarity. 0 - active low; 1 - active high.",
#endif
    LIF_LSR_MON_A_CTRL_CFGLSRMONACTHI_FIELD_MASK,
    0,
    LIF_LSR_MON_A_CTRL_CFGLSRMONACTHI_FIELD_WIDTH,
    LIF_LSR_MON_A_CTRL_CFGLSRMONACTHI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LSR_MON_A_CTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec LIF_LSR_MON_A_CTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LSR_MON_A_CTRL_RESERVED1_FIELD_MASK,
    0,
    LIF_LSR_MON_A_CTRL_RESERVED1_FIELD_WIDTH,
    LIF_LSR_MON_A_CTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LSR_MON_A_CTRL_PBILASERMONRSTA_N_PRE
 ******************************************************************************/
const ru_field_rec LIF_LSR_MON_A_CTRL_PBILASERMONRSTA_N_PRE_FIELD =
{
    "PBILASERMONRSTA_N_PRE",
#if RU_INCLUDE_DESC
    "",
    "Main reset for laser monitor."
    "0: Reset"
    "1: Normal operation",
#endif
    LIF_LSR_MON_A_CTRL_PBILASERMONRSTA_N_PRE_FIELD_MASK,
    0,
    LIF_LSR_MON_A_CTRL_PBILASERMONRSTA_N_PRE_FIELD_WIDTH,
    LIF_LSR_MON_A_CTRL_PBILASERMONRSTA_N_PRE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LSR_MON_A_MAX_THR_CFGLASERMONMAXA
 ******************************************************************************/
const ru_field_rec LIF_LSR_MON_A_MAX_THR_CFGLASERMONMAXA_FIELD =
{
    "CFGLASERMONMAXA",
#if RU_INCLUDE_DESC
    "",
    "Specifies the threshold for laserOnMaxInt. Units are TQ."
    "Default: 0x80ffff",
#endif
    LIF_LSR_MON_A_MAX_THR_CFGLASERMONMAXA_FIELD_MASK,
    0,
    LIF_LSR_MON_A_MAX_THR_CFGLASERMONMAXA_FIELD_WIDTH,
    LIF_LSR_MON_A_MAX_THR_CFGLASERMONMAXA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LSR_MON_A_BST_LEN_LASERONTIMEA
 ******************************************************************************/
const ru_field_rec LIF_LSR_MON_A_BST_LEN_LASERONTIMEA_FIELD =
{
    "LASERONTIMEA",
#if RU_INCLUDE_DESC
    "",
    "Indicates length of most recent burst, in TQ.",
#endif
    LIF_LSR_MON_A_BST_LEN_LASERONTIMEA_FIELD_MASK,
    0,
    LIF_LSR_MON_A_BST_LEN_LASERONTIMEA_FIELD_WIDTH,
    LIF_LSR_MON_A_BST_LEN_LASERONTIMEA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_LSR_MON_A_BST_CNT_LASERMONBRSTCNTA
 ******************************************************************************/
const ru_field_rec LIF_LSR_MON_A_BST_CNT_LASERMONBRSTCNTA_FIELD =
{
    "LASERMONBRSTCNTA",
#if RU_INCLUDE_DESC
    "",
    "This value increments on the negating edge of laser enable."
    "Saturates at 0xffffffff; and clears on read.",
#endif
    LIF_LSR_MON_A_BST_CNT_LASERMONBRSTCNTA_FIELD_MASK,
    0,
    LIF_LSR_MON_A_BST_CNT_LASERMONBRSTCNTA_FIELD_WIDTH,
    LIF_LSR_MON_A_BST_CNT_LASERMONBRSTCNTA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_DEBUG_PON_SM_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_DEBUG_PON_SM_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_DEBUG_PON_SM_RESERVED0_FIELD_MASK,
    0,
    LIF_DEBUG_PON_SM_RESERVED0_FIELD_WIDTH,
    LIF_DEBUG_PON_SM_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_DEBUG_PON_SM_ALIGNCSQQ
 ******************************************************************************/
const ru_field_rec LIF_DEBUG_PON_SM_ALIGNCSQQ_FIELD =
{
    "ALIGNCSQQ",
#if RU_INCLUDE_DESC
    "",
    "10B Alignment State Machine States",
#endif
    LIF_DEBUG_PON_SM_ALIGNCSQQ_FIELD_MASK,
    0,
    LIF_DEBUG_PON_SM_ALIGNCSQQ_FIELD_WIDTH,
    LIF_DEBUG_PON_SM_ALIGNCSQQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_DEBUG_PON_SM_RESERVED1
 ******************************************************************************/
const ru_field_rec LIF_DEBUG_PON_SM_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_DEBUG_PON_SM_RESERVED1_FIELD_MASK,
    0,
    LIF_DEBUG_PON_SM_RESERVED1_FIELD_WIDTH,
    LIF_DEBUG_PON_SM_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_DEBUG_PON_SM_RXFECIFCSQQ
 ******************************************************************************/
const ru_field_rec LIF_DEBUG_PON_SM_RXFECIFCSQQ_FIELD =
{
    "RXFECIFCSQQ",
#if RU_INCLUDE_DESC
    "",
    "8B State Machine States",
#endif
    LIF_DEBUG_PON_SM_RXFECIFCSQQ_FIELD_MASK,
    0,
    LIF_DEBUG_PON_SM_RXFECIFCSQQ_FIELD_WIDTH,
    LIF_DEBUG_PON_SM_RXFECIFCSQQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_DEBUG_FEC_SM_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_DEBUG_FEC_SM_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_DEBUG_FEC_SM_RESERVED0_FIELD_MASK,
    0,
    LIF_DEBUG_FEC_SM_RESERVED0_FIELD_WIDTH,
    LIF_DEBUG_FEC_SM_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_DEBUG_FEC_SM_RXSYNCSQQ
 ******************************************************************************/
const ru_field_rec LIF_DEBUG_FEC_SM_RXSYNCSQQ_FIELD =
{
    "RXSYNCSQQ",
#if RU_INCLUDE_DESC
    "",
    "FEC Receive Syndrome States",
#endif
    LIF_DEBUG_FEC_SM_RXSYNCSQQ_FIELD_MASK,
    0,
    LIF_DEBUG_FEC_SM_RXSYNCSQQ_FIELD_WIDTH,
    LIF_DEBUG_FEC_SM_RXSYNCSQQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_DEBUG_FEC_SM_RESERVED1
 ******************************************************************************/
const ru_field_rec LIF_DEBUG_FEC_SM_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_DEBUG_FEC_SM_RESERVED1_FIELD_MASK,
    0,
    LIF_DEBUG_FEC_SM_RESERVED1_FIELD_WIDTH,
    LIF_DEBUG_FEC_SM_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_DEBUG_FEC_SM_RXCORCS
 ******************************************************************************/
const ru_field_rec LIF_DEBUG_FEC_SM_RXCORCS_FIELD =
{
    "RXCORCS",
#if RU_INCLUDE_DESC
    "",
    "FEC Receive Corrector States",
#endif
    LIF_DEBUG_FEC_SM_RXCORCS_FIELD_MASK,
    0,
    LIF_DEBUG_FEC_SM_RXCORCS_FIELD_WIDTH,
    LIF_DEBUG_FEC_SM_RXCORCS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_DEBUG_FEC_SM_RESERVED2
 ******************************************************************************/
const ru_field_rec LIF_DEBUG_FEC_SM_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_DEBUG_FEC_SM_RESERVED2_FIELD_MASK,
    0,
    LIF_DEBUG_FEC_SM_RESERVED2_FIELD_WIDTH,
    LIF_DEBUG_FEC_SM_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_DEBUG_FEC_SM_FECRXOUTCS
 ******************************************************************************/
const ru_field_rec LIF_DEBUG_FEC_SM_FECRXOUTCS_FIELD =
{
    "FECRXOUTCS",
#if RU_INCLUDE_DESC
    "",
    "FEC Receive Output States",
#endif
    LIF_DEBUG_FEC_SM_FECRXOUTCS_FIELD_MASK,
    0,
    LIF_DEBUG_FEC_SM_FECRXOUTCS_FIELD_WIDTH,
    LIF_DEBUG_FEC_SM_FECRXOUTCS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_AE_PKTNUM_WINDOW_CFGAEPKTNUMWND
 ******************************************************************************/
const ru_field_rec LIF_AE_PKTNUM_WINDOW_CFGAEPKTNUMWND_FIELD =
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
    LIF_AE_PKTNUM_WINDOW_CFGAEPKTNUMWND_FIELD_MASK,
    0,
    LIF_AE_PKTNUM_WINDOW_CFGAEPKTNUMWND_FIELD_WIDTH,
    LIF_AE_PKTNUM_WINDOW_CFGAEPKTNUMWND_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_AE_PKTNUM_THRESH_CFGPKTNUMMAXTHRESH
 ******************************************************************************/
const ru_field_rec LIF_AE_PKTNUM_THRESH_CFGPKTNUMMAXTHRESH_FIELD =
{
    "CFGPKTNUMMAXTHRESH",
#if RU_INCLUDE_DESC
    "",
    "Defines the malifum packet number rollover.",
#endif
    LIF_AE_PKTNUM_THRESH_CFGPKTNUMMAXTHRESH_FIELD_MASK,
    0,
    LIF_AE_PKTNUM_THRESH_CFGPKTNUMMAXTHRESH_FIELD_WIDTH,
    LIF_AE_PKTNUM_THRESH_CFGPKTNUMMAXTHRESH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_AE_PKTNUM_STAT_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_AE_PKTNUM_STAT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_AE_PKTNUM_STAT_RESERVED0_FIELD_MASK,
    0,
    LIF_AE_PKTNUM_STAT_RESERVED0_FIELD_WIDTH,
    LIF_AE_PKTNUM_STAT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_AE_PKTNUM_STAT_SECUPINDXWTPKTNUMMAX
 ******************************************************************************/
const ru_field_rec LIF_AE_PKTNUM_STAT_SECUPINDXWTPKTNUMMAX_FIELD =
{
    "SECUPINDXWTPKTNUMMAX",
#if RU_INCLUDE_DESC
    "",
    "Provides the LLID index whose packet number exceeded the malifum"
    "packet number threhsold.",
#endif
    LIF_AE_PKTNUM_STAT_SECUPINDXWTPKTNUMMAX_FIELD_MASK,
    0,
    LIF_AE_PKTNUM_STAT_SECUPINDXWTPKTNUMMAX_FIELD_WIDTH,
    LIF_AE_PKTNUM_STAT_SECUPINDXWTPKTNUMMAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_AE_PKTNUM_STAT_RESERVED1
 ******************************************************************************/
const ru_field_rec LIF_AE_PKTNUM_STAT_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_AE_PKTNUM_STAT_RESERVED1_FIELD_MASK,
    0,
    LIF_AE_PKTNUM_STAT_RESERVED1_FIELD_WIDTH,
    LIF_AE_PKTNUM_STAT_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_AE_PKTNUM_STAT_SECDNINDXWTPKTNUMABORT
 ******************************************************************************/
const ru_field_rec LIF_AE_PKTNUM_STAT_SECDNINDXWTPKTNUMABORT_FIELD =
{
    "SECDNINDXWTPKTNUMABORT",
#if RU_INCLUDE_DESC
    "",
    "Provides the LLID index that was aborted due to replay protection.",
#endif
    LIF_AE_PKTNUM_STAT_SECDNINDXWTPKTNUMABORT_FIELD_MASK,
    0,
    LIF_AE_PKTNUM_STAT_SECDNINDXWTPKTNUMABORT_FIELD_WIDTH,
    LIF_AE_PKTNUM_STAT_SECDNINDXWTPKTNUMABORT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_LLID_8_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_8_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_8_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_8_RESERVED0_FIELD_WIDTH,
    LIF_LLID_8_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_8_CFGLLID8
 ******************************************************************************/
const ru_field_rec LIF_LLID_8_CFGLLID8_FIELD =
{
    "CFGLLID8",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 8."
    "In downstream P2P, 802.1ae mode, registers"
    "LIF_P2P_AE_SCI_LO[0:15]/LIF_P2P_AE_SCI_HI[0:15]"
    "provide lookup with packet's explicit SCI to an index.",
#endif
    LIF_LLID_8_CFGLLID8_FIELD_MASK,
    0,
    LIF_LLID_8_CFGLLID8_FIELD_WIDTH,
    LIF_LLID_8_CFGLLID8_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_9_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_9_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_9_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_9_RESERVED0_FIELD_WIDTH,
    LIF_LLID_9_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_9_CFGLLID9
 ******************************************************************************/
const ru_field_rec LIF_LLID_9_CFGLLID9_FIELD =
{
    "CFGLLID9",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 9."
    "In downstream P2P, 802.1ae mode, registers"
    "LIF_P2P_AE_SCI_LO[0:15]/LIF_P2P_AE_SCI_HI[0:15]"
    "provide lookup with packet's explicit SCI to an index.",
#endif
    LIF_LLID_9_CFGLLID9_FIELD_MASK,
    0,
    LIF_LLID_9_CFGLLID9_FIELD_WIDTH,
    LIF_LLID_9_CFGLLID9_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_10_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_10_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_10_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_10_RESERVED0_FIELD_WIDTH,
    LIF_LLID_10_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_10_CFGLLID10
 ******************************************************************************/
const ru_field_rec LIF_LLID_10_CFGLLID10_FIELD =
{
    "CFGLLID10",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 10."
    "In downstream P2P, 802.1ae mode, registers"
    "LIF_P2P_AE_SCI_LO[0:15]/LIF_P2P_AE_SCI_HI[0:15]"
    "provide lookup with packet's explicit SCI to an index.",
#endif
    LIF_LLID_10_CFGLLID10_FIELD_MASK,
    0,
    LIF_LLID_10_CFGLLID10_FIELD_WIDTH,
    LIF_LLID_10_CFGLLID10_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_11_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_11_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_11_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_11_RESERVED0_FIELD_WIDTH,
    LIF_LLID_11_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_11_CFGLLID11
 ******************************************************************************/
const ru_field_rec LIF_LLID_11_CFGLLID11_FIELD =
{
    "CFGLLID11",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 11."
    "In downstream P2P, 802.1ae mode, registers"
    "LIF_P2P_AE_SCI_LO[0:15]/LIF_P2P_AE_SCI_HI[0:15]"
    "provide lookup with packet's explicit SCI to an index.",
#endif
    LIF_LLID_11_CFGLLID11_FIELD_MASK,
    0,
    LIF_LLID_11_CFGLLID11_FIELD_WIDTH,
    LIF_LLID_11_CFGLLID11_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_12_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_12_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_12_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_12_RESERVED0_FIELD_WIDTH,
    LIF_LLID_12_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_12_CFGLLID12
 ******************************************************************************/
const ru_field_rec LIF_LLID_12_CFGLLID12_FIELD =
{
    "CFGLLID12",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 12."
    "In downstream P2P, 802.1ae mode, registers"
    "LIF_P2P_AE_SCI_LO[0:15]/LIF_P2P_AE_SCI_HI[0:15]"
    "provide lookup with packet's explicit SCI to an index.",
#endif
    LIF_LLID_12_CFGLLID12_FIELD_MASK,
    0,
    LIF_LLID_12_CFGLLID12_FIELD_WIDTH,
    LIF_LLID_12_CFGLLID12_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_13_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_13_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_13_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_13_RESERVED0_FIELD_WIDTH,
    LIF_LLID_13_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_13_CFGLLID13
 ******************************************************************************/
const ru_field_rec LIF_LLID_13_CFGLLID13_FIELD =
{
    "CFGLLID13",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 13."
    "In downstream P2P, 802.1ae mode, registers"
    "LIF_P2P_AE_SCI_LO[0:15]/LIF_P2P_AE_SCI_HI[0:15]"
    "provide lookup with packet's explicit SCI to an index.",
#endif
    LIF_LLID_13_CFGLLID13_FIELD_MASK,
    0,
    LIF_LLID_13_CFGLLID13_FIELD_WIDTH,
    LIF_LLID_13_CFGLLID13_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_14_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_14_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_14_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_14_RESERVED0_FIELD_WIDTH,
    LIF_LLID_14_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_14_CFGLLID14
 ******************************************************************************/
const ru_field_rec LIF_LLID_14_CFGLLID14_FIELD =
{
    "CFGLLID14",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 14."
    "In downstream P2P, 802.1ae mode, registers"
    "LIF_P2P_AE_SCI_LO[0:15]/LIF_P2P_AE_SCI_HI[0:15]"
    "provide lookup with packet's explicit SCI to an index.",
#endif
    LIF_LLID_14_CFGLLID14_FIELD_MASK,
    0,
    LIF_LLID_14_CFGLLID14_FIELD_WIDTH,
    LIF_LLID_14_CFGLLID14_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_15_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_15_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_15_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_15_RESERVED0_FIELD_WIDTH,
    LIF_LLID_15_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_15_CFGLLID15
 ******************************************************************************/
const ru_field_rec LIF_LLID_15_CFGLLID15_FIELD =
{
    "CFGLLID15",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 15."
    "In downstream P2P, 802.1ae mode, registers"
    "LIF_P2P_AE_SCI_LO[0:15]/LIF_P2P_AE_SCI_HI[0:15]"
    "provide lookup with packet's explicit SCI to an index.",
#endif
    LIF_LLID_15_CFGLLID15_FIELD_MASK,
    0,
    LIF_LLID_15_CFGLLID15_FIELD_WIDTH,
    LIF_LLID_15_CFGLLID15_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_24_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_24_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_24_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_24_RESERVED0_FIELD_WIDTH,
    LIF_LLID_24_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_24_CFGLLID24
 ******************************************************************************/
const ru_field_rec LIF_LLID_24_CFGLLID24_FIELD =
{
    "CFGLLID24",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 24.",
#endif
    LIF_LLID_24_CFGLLID24_FIELD_MASK,
    0,
    LIF_LLID_24_CFGLLID24_FIELD_WIDTH,
    LIF_LLID_24_CFGLLID24_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_25_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_25_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_25_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_25_RESERVED0_FIELD_WIDTH,
    LIF_LLID_25_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_25_CFGLLID25
 ******************************************************************************/
const ru_field_rec LIF_LLID_25_CFGLLID25_FIELD =
{
    "CFGLLID25",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 25.",
#endif
    LIF_LLID_25_CFGLLID25_FIELD_MASK,
    0,
    LIF_LLID_25_CFGLLID25_FIELD_WIDTH,
    LIF_LLID_25_CFGLLID25_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_26_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_26_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_26_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_26_RESERVED0_FIELD_WIDTH,
    LIF_LLID_26_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_26_CFGLLID26
 ******************************************************************************/
const ru_field_rec LIF_LLID_26_CFGLLID26_FIELD =
{
    "CFGLLID26",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 26.",
#endif
    LIF_LLID_26_CFGLLID26_FIELD_MASK,
    0,
    LIF_LLID_26_CFGLLID26_FIELD_WIDTH,
    LIF_LLID_26_CFGLLID26_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_27_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_27_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_27_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_27_RESERVED0_FIELD_WIDTH,
    LIF_LLID_27_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_27_CFGLLID27
 ******************************************************************************/
const ru_field_rec LIF_LLID_27_CFGLLID27_FIELD =
{
    "CFGLLID27",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 27.",
#endif
    LIF_LLID_27_CFGLLID27_FIELD_MASK,
    0,
    LIF_LLID_27_CFGLLID27_FIELD_WIDTH,
    LIF_LLID_27_CFGLLID27_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_28_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_28_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_28_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_28_RESERVED0_FIELD_WIDTH,
    LIF_LLID_28_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_28_CFGLLID28
 ******************************************************************************/
const ru_field_rec LIF_LLID_28_CFGLLID28_FIELD =
{
    "CFGLLID28",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 28.",
#endif
    LIF_LLID_28_CFGLLID28_FIELD_MASK,
    0,
    LIF_LLID_28_CFGLLID28_FIELD_WIDTH,
    LIF_LLID_28_CFGLLID28_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_29_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_29_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_29_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_29_RESERVED0_FIELD_WIDTH,
    LIF_LLID_29_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_29_CFGLLID29
 ******************************************************************************/
const ru_field_rec LIF_LLID_29_CFGLLID29_FIELD =
{
    "CFGLLID29",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 29.",
#endif
    LIF_LLID_29_CFGLLID29_FIELD_MASK,
    0,
    LIF_LLID_29_CFGLLID29_FIELD_WIDTH,
    LIF_LLID_29_CFGLLID29_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_30_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_30_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_30_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_30_RESERVED0_FIELD_WIDTH,
    LIF_LLID_30_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_30_CFGLLID30
 ******************************************************************************/
const ru_field_rec LIF_LLID_30_CFGLLID30_FIELD =
{
    "CFGLLID30",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 30.",
#endif
    LIF_LLID_30_CFGLLID30_FIELD_MASK,
    0,
    LIF_LLID_30_CFGLLID30_FIELD_WIDTH,
    LIF_LLID_30_CFGLLID30_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_31_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_LLID_31_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_LLID_31_RESERVED0_FIELD_MASK,
    0,
    LIF_LLID_31_RESERVED0_FIELD_WIDTH,
    LIF_LLID_31_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_LLID_31_CFGLLID31
 ******************************************************************************/
const ru_field_rec LIF_LLID_31_CFGLLID31_FIELD =
{
    "CFGLLID31",
#if RU_INCLUDE_DESC
    "",
    "[15:0]: Specifies LLID Index lookup value"
    "[16]:   Enable LLID"
    "0: Disable LLID"
    "1: Enable LLID"
    ""
    "In upstream P2P, 802.1ae mode, bit[11:0] provides lookup with VLAN's"
    "VID to index 31.",
#endif
    LIF_LLID_31_CFGLLID31_FIELD_MASK,
    0,
    LIF_LLID_31_CFGLLID31_FIELD_WIDTH,
    LIF_LLID_31_CFGLLID31_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_VLAN_TYPE_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_VLAN_TYPE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_VLAN_TYPE_RESERVED0_FIELD_MASK,
    0,
    LIF_VLAN_TYPE_RESERVED0_FIELD_WIDTH,
    LIF_VLAN_TYPE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_VLAN_TYPE_CFGVLANTYPE
 ******************************************************************************/
const ru_field_rec LIF_VLAN_TYPE_CFGVLANTYPE_FIELD =
{
    "CFGVLANTYPE",
#if RU_INCLUDE_DESC
    "",
    "Defines a VLAN type, in addition to 0x8100.",
#endif
    LIF_VLAN_TYPE_CFGVLANTYPE_FIELD_MASK,
    0,
    LIF_VLAN_TYPE_CFGVLANTYPE_FIELD_WIDTH,
    LIF_VLAN_TYPE_CFGVLANTYPE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_EN_CFGP2PSCIEN
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_EN_CFGP2PSCIEN_FIELD =
{
    "CFGP2PSCIEN",
#if RU_INCLUDE_DESC
    "",
    "Enables SCI lookup, via"
    "LIF_P2P_AE_SCI_LO_[0:31]/LIF_P2P_AE_SCI_HI[0:31] registers. Each bit"
    "corresponds to index 0 - 31.",
#endif
    LIF_P2P_AE_SCI_EN_CFGP2PSCIEN_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_EN_CFGP2PSCIEN_FIELD_WIDTH,
    LIF_P2P_AE_SCI_EN_CFGP2PSCIEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_0_CFGP2PSCI_LO_0
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_0_CFGP2PSCI_LO_0_FIELD =
{
    "CFGP2PSCI_LO_0",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_0_CFGP2PSCI_LO_0_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_0_CFGP2PSCI_LO_0_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_0_CFGP2PSCI_LO_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_0_CFGP2PSCI_HI_0
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_0_CFGP2PSCI_HI_0_FIELD =
{
    "CFGP2PSCI_HI_0",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_0_CFGP2PSCI_HI_0_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_0_CFGP2PSCI_HI_0_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_0_CFGP2PSCI_HI_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_1_CFGP2PSCI_LO_1
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_1_CFGP2PSCI_LO_1_FIELD =
{
    "CFGP2PSCI_LO_1",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_1_CFGP2PSCI_LO_1_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_1_CFGP2PSCI_LO_1_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_1_CFGP2PSCI_LO_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_1_CFGP2PSCI_HI_1
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_1_CFGP2PSCI_HI_1_FIELD =
{
    "CFGP2PSCI_HI_1",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_1_CFGP2PSCI_HI_1_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_1_CFGP2PSCI_HI_1_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_1_CFGP2PSCI_HI_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_2_CFGP2PSCI_LO_2
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_2_CFGP2PSCI_LO_2_FIELD =
{
    "CFGP2PSCI_LO_2",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_2_CFGP2PSCI_LO_2_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_2_CFGP2PSCI_LO_2_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_2_CFGP2PSCI_LO_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_2_CFGP2PSCI_HI_2
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_2_CFGP2PSCI_HI_2_FIELD =
{
    "CFGP2PSCI_HI_2",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_2_CFGP2PSCI_HI_2_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_2_CFGP2PSCI_HI_2_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_2_CFGP2PSCI_HI_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_3_CFGP2PSCI_LO_3
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_3_CFGP2PSCI_LO_3_FIELD =
{
    "CFGP2PSCI_LO_3",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_3_CFGP2PSCI_LO_3_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_3_CFGP2PSCI_LO_3_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_3_CFGP2PSCI_LO_3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_3_CFGP2PSCI_HI_3
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_3_CFGP2PSCI_HI_3_FIELD =
{
    "CFGP2PSCI_HI_3",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_3_CFGP2PSCI_HI_3_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_3_CFGP2PSCI_HI_3_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_3_CFGP2PSCI_HI_3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_4_CFGP2PSCI_LO_4
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_4_CFGP2PSCI_LO_4_FIELD =
{
    "CFGP2PSCI_LO_4",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_4_CFGP2PSCI_LO_4_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_4_CFGP2PSCI_LO_4_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_4_CFGP2PSCI_LO_4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_4_CFGP2PSCI_HI_4
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_4_CFGP2PSCI_HI_4_FIELD =
{
    "CFGP2PSCI_HI_4",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_4_CFGP2PSCI_HI_4_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_4_CFGP2PSCI_HI_4_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_4_CFGP2PSCI_HI_4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_5_CFGP2PSCI_LO_5
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_5_CFGP2PSCI_LO_5_FIELD =
{
    "CFGP2PSCI_LO_5",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_5_CFGP2PSCI_LO_5_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_5_CFGP2PSCI_LO_5_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_5_CFGP2PSCI_LO_5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_5_CFGP2PSCI_HI_5
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_5_CFGP2PSCI_HI_5_FIELD =
{
    "CFGP2PSCI_HI_5",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_5_CFGP2PSCI_HI_5_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_5_CFGP2PSCI_HI_5_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_5_CFGP2PSCI_HI_5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_6_CFGP2PSCI_LO_6
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_6_CFGP2PSCI_LO_6_FIELD =
{
    "CFGP2PSCI_LO_6",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_6_CFGP2PSCI_LO_6_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_6_CFGP2PSCI_LO_6_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_6_CFGP2PSCI_LO_6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_6_CFGP2PSCI_HI_6
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_6_CFGP2PSCI_HI_6_FIELD =
{
    "CFGP2PSCI_HI_6",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_6_CFGP2PSCI_HI_6_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_6_CFGP2PSCI_HI_6_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_6_CFGP2PSCI_HI_6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_7_CFGP2PSCI_LO_7
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_7_CFGP2PSCI_LO_7_FIELD =
{
    "CFGP2PSCI_LO_7",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_7_CFGP2PSCI_LO_7_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_7_CFGP2PSCI_LO_7_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_7_CFGP2PSCI_LO_7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_7_CFGP2PSCI_HI_7
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_7_CFGP2PSCI_HI_7_FIELD =
{
    "CFGP2PSCI_HI_7",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_7_CFGP2PSCI_HI_7_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_7_CFGP2PSCI_HI_7_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_7_CFGP2PSCI_HI_7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_8_CFGP2PSCI_LO_8
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_8_CFGP2PSCI_LO_8_FIELD =
{
    "CFGP2PSCI_LO_8",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_8_CFGP2PSCI_LO_8_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_8_CFGP2PSCI_LO_8_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_8_CFGP2PSCI_LO_8_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_8_CFGP2PSCI_HI_8
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_8_CFGP2PSCI_HI_8_FIELD =
{
    "CFGP2PSCI_HI_8",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_8_CFGP2PSCI_HI_8_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_8_CFGP2PSCI_HI_8_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_8_CFGP2PSCI_HI_8_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_9_CFGP2PSCI_LO_9
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_9_CFGP2PSCI_LO_9_FIELD =
{
    "CFGP2PSCI_LO_9",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_9_CFGP2PSCI_LO_9_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_9_CFGP2PSCI_LO_9_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_9_CFGP2PSCI_LO_9_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_9_CFGP2PSCI_HI_9
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_9_CFGP2PSCI_HI_9_FIELD =
{
    "CFGP2PSCI_HI_9",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_9_CFGP2PSCI_HI_9_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_9_CFGP2PSCI_HI_9_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_9_CFGP2PSCI_HI_9_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_10_CFGP2PSCI_LO_10
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_10_CFGP2PSCI_LO_10_FIELD =
{
    "CFGP2PSCI_LO_10",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_10_CFGP2PSCI_LO_10_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_10_CFGP2PSCI_LO_10_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_10_CFGP2PSCI_LO_10_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_10_CFGP2PSCI_HI_10
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_10_CFGP2PSCI_HI_10_FIELD =
{
    "CFGP2PSCI_HI_10",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_10_CFGP2PSCI_HI_10_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_10_CFGP2PSCI_HI_10_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_10_CFGP2PSCI_HI_10_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_11_CFGP2PSCI_LO_11
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_11_CFGP2PSCI_LO_11_FIELD =
{
    "CFGP2PSCI_LO_11",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_11_CFGP2PSCI_LO_11_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_11_CFGP2PSCI_LO_11_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_11_CFGP2PSCI_LO_11_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_11_CFGP2PSCI_HI_11
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_11_CFGP2PSCI_HI_11_FIELD =
{
    "CFGP2PSCI_HI_11",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_11_CFGP2PSCI_HI_11_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_11_CFGP2PSCI_HI_11_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_11_CFGP2PSCI_HI_11_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_12_CFGP2PSCI_LO_12
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_12_CFGP2PSCI_LO_12_FIELD =
{
    "CFGP2PSCI_LO_12",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_12_CFGP2PSCI_LO_12_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_12_CFGP2PSCI_LO_12_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_12_CFGP2PSCI_LO_12_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_12_CFGP2PSCI_HI_12
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_12_CFGP2PSCI_HI_12_FIELD =
{
    "CFGP2PSCI_HI_12",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_12_CFGP2PSCI_HI_12_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_12_CFGP2PSCI_HI_12_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_12_CFGP2PSCI_HI_12_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_13_CFGP2PSCI_LO_13
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_13_CFGP2PSCI_LO_13_FIELD =
{
    "CFGP2PSCI_LO_13",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_13_CFGP2PSCI_LO_13_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_13_CFGP2PSCI_LO_13_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_13_CFGP2PSCI_LO_13_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_13_CFGP2PSCI_HI_13
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_13_CFGP2PSCI_HI_13_FIELD =
{
    "CFGP2PSCI_HI_13",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_13_CFGP2PSCI_HI_13_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_13_CFGP2PSCI_HI_13_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_13_CFGP2PSCI_HI_13_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_14_CFGP2PSCI_LO_14
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_14_CFGP2PSCI_LO_14_FIELD =
{
    "CFGP2PSCI_LO_14",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_14_CFGP2PSCI_LO_14_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_14_CFGP2PSCI_LO_14_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_14_CFGP2PSCI_LO_14_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_14_CFGP2PSCI_HI_14
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_14_CFGP2PSCI_HI_14_FIELD =
{
    "CFGP2PSCI_HI_14",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_14_CFGP2PSCI_HI_14_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_14_CFGP2PSCI_HI_14_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_14_CFGP2PSCI_HI_14_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_15_CFGP2PSCI_LO_15
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_15_CFGP2PSCI_LO_15_FIELD =
{
    "CFGP2PSCI_LO_15",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_15_CFGP2PSCI_LO_15_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_15_CFGP2PSCI_LO_15_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_15_CFGP2PSCI_LO_15_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_15_CFGP2PSCI_HI_15
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_15_CFGP2PSCI_HI_15_FIELD =
{
    "CFGP2PSCI_HI_15",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_15_CFGP2PSCI_HI_15_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_15_CFGP2PSCI_HI_15_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_15_CFGP2PSCI_HI_15_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_KEY_SEL_1_KEYSEL_HI
 ******************************************************************************/
const ru_field_rec LIF_SEC_KEY_SEL_1_KEYSEL_HI_FIELD =
{
    "KEYSEL_HI",
#if RU_INCLUDE_DESC
    "",
    "In 802.1ae mode, this register provides the upper bit of TCI's"
    "AN[1:0].  The mapping to even/odd key, based on AN[1:0], is provided"
    "by cfgKeyNum_4_AN_0 - cfgKeyNum_4_AN_3.",
#endif
    LIF_SEC_KEY_SEL_1_KEYSEL_HI_FIELD_MASK,
    0,
    LIF_SEC_KEY_SEL_1_KEYSEL_HI_FIELD_WIDTH,
    LIF_SEC_KEY_SEL_1_KEYSEL_HI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_RESERVED0_FIELD_MASK,
    0,
    LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_RESERVED0_FIELD_WIDTH,
    LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_CF_PLAINTXT_OH_B_IDLE_PAD
 ******************************************************************************/
const ru_field_rec LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_CF_PLAINTXT_OH_B_IDLE_PAD_FIELD =
{
    "CF_PLAINTXT_OH_B_IDLE_PAD",
#if RU_INCLUDE_DESC
    "",
    "Used to pad plain text frames in 802.1AE encryption mode with a"
    "fixed number of IPG equivalent to the"
    "AE overhead.  Overhead B is equivalent to 16TQ or 32 bytes of"
    "preceding IPG for explicit SCI.",
#endif
    LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_CF_PLAINTXT_OH_B_IDLE_PAD_FIELD_MASK,
    0,
    LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_CF_PLAINTXT_OH_B_IDLE_PAD_FIELD_WIDTH,
    LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_CF_PLAINTXT_OH_B_IDLE_PAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_CF_PLAINTXT_OH_A_IDLE_PAD
 ******************************************************************************/
const ru_field_rec LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_CF_PLAINTXT_OH_A_IDLE_PAD_FIELD =
{
    "CF_PLAINTXT_OH_A_IDLE_PAD",
#if RU_INCLUDE_DESC
    "",
    "Used to pad plain text frames in 802.1AE encryption mode with a"
    "fixed number of IPG equivalent to the"
    "AE overhead.  Overhead A is equivalent to 12TQ or 24 bytes of"
    "preceding IPG for implicit SCI.",
#endif
    LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_CF_PLAINTXT_OH_A_IDLE_PAD_FIELD_MASK,
    0,
    LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_CF_PLAINTXT_OH_A_IDLE_PAD_FIELD_WIDTH,
    LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_CF_PLAINTXT_OH_A_IDLE_PAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_LINKTIMER
 ******************************************************************************/
const ru_field_rec LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_LINKTIMER_FIELD =
{
    "CF_AUTONEG_LINKTIMER",
#if RU_INCLUDE_DESC
    "",
    "Link Timer to allow link partner time to process current state"
    "before advancing to next state"
    ""
    "HW Default is about 33.5 ms"
    ""
    "S/W should set to 0x0fff for 2.0ms timer for SGMII Style Formatting",
#endif
    LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_LINKTIMER_FIELD_MASK,
    0,
    LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_LINKTIMER_FIELD_WIDTH,
    LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_LINKTIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AUTONEG_CONTROL_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_P2P_AUTONEG_CONTROL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_P2P_AUTONEG_CONTROL_RESERVED0_FIELD_MASK,
    0,
    LIF_P2P_AUTONEG_CONTROL_RESERVED0_FIELD_WIDTH,
    LIF_P2P_AUTONEG_CONTROL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_MODE_SEL
 ******************************************************************************/
const ru_field_rec LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_MODE_SEL_FIELD =
{
    "CF_AUTONEG_MODE_SEL",
#if RU_INCLUDE_DESC
    "",
    "Mode Select for Auto Neg"
    "0 = CL37 Style (33.5ms link timer)"
    "1 = SGMII Style (2.0ms link timer)"
    ""
    "See LP ability and advertisement registers for format",
#endif
    LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_MODE_SEL_FIELD_MASK,
    0,
    LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_MODE_SEL_FIELD_WIDTH,
    LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_MODE_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_RESTART
 ******************************************************************************/
const ru_field_rec LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_RESTART_FIELD =
{
    "CF_AUTONEG_RESTART",
#if RU_INCLUDE_DESC
    "",
    "0 = Restart Disabled / Completed"
    "1 = Trigger Restart"
    ""
    "H/W will clear to 0 when restart occurs.",
#endif
    LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_RESTART_FIELD_MASK,
    0,
    LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_RESTART_FIELD_WIDTH,
    LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_RESTART_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_EN
 ******************************************************************************/
const ru_field_rec LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_EN_FIELD =
{
    "CF_AUTONEG_EN",
#if RU_INCLUDE_DESC
    "",
    "0 = Disable Autonegotiation"
    "1 = Enable Autonegotiation",
#endif
    LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_EN_FIELD_MASK,
    0,
    LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_EN_FIELD_WIDTH,
    LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AUTONEG_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_P2P_AUTONEG_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_P2P_AUTONEG_STATUS_RESERVED0_FIELD_MASK,
    0,
    LIF_P2P_AUTONEG_STATUS_RESERVED0_FIELD_WIDTH,
    LIF_P2P_AUTONEG_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AUTONEG_STATUS_AN_LP_REMOTE_FAULT
 ******************************************************************************/
const ru_field_rec LIF_P2P_AUTONEG_STATUS_AN_LP_REMOTE_FAULT_FIELD =
{
    "AN_LP_REMOTE_FAULT",
#if RU_INCLUDE_DESC
    "",
    "0 = Remote Fault Not Detected from Link Partner"
    "1 = Remote Fault Detected from Link Partner"
    ""
    "Will only update after AN process.  Will reset upon restart.",
#endif
    LIF_P2P_AUTONEG_STATUS_AN_LP_REMOTE_FAULT_FIELD_MASK,
    0,
    LIF_P2P_AUTONEG_STATUS_AN_LP_REMOTE_FAULT_FIELD_WIDTH,
    LIF_P2P_AUTONEG_STATUS_AN_LP_REMOTE_FAULT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AUTONEG_STATUS_AN_SYNC_STATUS
 ******************************************************************************/
const ru_field_rec LIF_P2P_AUTONEG_STATUS_AN_SYNC_STATUS_FIELD =
{
    "AN_SYNC_STATUS",
#if RU_INCLUDE_DESC
    "",
    "0 = No sync after AN complete"
    "1 = Sync after AN complete"
    ""
    "Will only update after AN process.  Will reset upon restart.",
#endif
    LIF_P2P_AUTONEG_STATUS_AN_SYNC_STATUS_FIELD_MASK,
    0,
    LIF_P2P_AUTONEG_STATUS_AN_SYNC_STATUS_FIELD_WIDTH,
    LIF_P2P_AUTONEG_STATUS_AN_SYNC_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AUTONEG_STATUS_AN_COMPLETE
 ******************************************************************************/
const ru_field_rec LIF_P2P_AUTONEG_STATUS_AN_COMPLETE_FIELD =
{
    "AN_COMPLETE",
#if RU_INCLUDE_DESC
    "",
    "0 = Autoneg Not Complete"
    "1 = Autoneg Completed"
    ""
    "Will only update after AN process.  Will reset upon restart.",
#endif
    LIF_P2P_AUTONEG_STATUS_AN_COMPLETE_FIELD_MASK,
    0,
    LIF_P2P_AUTONEG_STATUS_AN_COMPLETE_FIELD_WIDTH,
    LIF_P2P_AUTONEG_STATUS_AN_COMPLETE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AUTONEG_ABILITY_CONFIG_REG_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_P2P_AUTONEG_ABILITY_CONFIG_REG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_P2P_AUTONEG_ABILITY_CONFIG_REG_RESERVED0_FIELD_MASK,
    0,
    LIF_P2P_AUTONEG_ABILITY_CONFIG_REG_RESERVED0_FIELD_WIDTH,
    LIF_P2P_AUTONEG_ABILITY_CONFIG_REG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AUTONEG_ABILITY_CONFIG_REG_CF_LIF_P2P_AE_AUTONEG_CONFIG_ABILITY
 ******************************************************************************/
const ru_field_rec LIF_P2P_AUTONEG_ABILITY_CONFIG_REG_CF_LIF_P2P_AE_AUTONEG_CONFIG_ABILITY_FIELD =
{
    "CF_LIF_P2P_AE_AUTONEG_CONFIG_ABILITY",
#if RU_INCLUDE_DESC
    "",
    "Defines the Autonegotiation Ability / Config Register of this device"
    ""
    "cf_autoneg_mode_sel = 0 (CL37)"
    ""
    "Bits"
    "[15]: NP, Next Page"
    "[14]: ACK (H/W overwrite)"
    "[13]: RF2, Remote Fault"
    "[12]: RF1, Remote Fault"
    "[11:9]: Reserved"
    "[8]: PS2, Asymmetric Pause"
    "[7]: PS1, Symmetric Pause"
    "[6]: HD, Half Duplex"
    "[5]: FD, Full Duplex"
    "[4:0]: Reserved"
    ""
    "cf_autoneg_mode_sel = 1 (SGMII)"
    ""
    "Bits"
    "[15]: Link State (HW Controlled) 1 = Link Up, 0 = Link Down"
    "[14]: ACK (H/W overwrite)"
    "[13]: Reserved"
    "[12]: Duplex mode: 1 = Full Duplex, 0 = Half Duplex"
    "[11:10]: 11 = Reserved, 10 = 1000Mbps, 01 = 100Mbps, 00 = 10 Mbps"
    "[9:1]: Reserved"
    "[0]: Set 1 to per SGMII Spec (S/W must set)",
#endif
    LIF_P2P_AUTONEG_ABILITY_CONFIG_REG_CF_LIF_P2P_AE_AUTONEG_CONFIG_ABILITY_FIELD_MASK,
    0,
    LIF_P2P_AUTONEG_ABILITY_CONFIG_REG_CF_LIF_P2P_AE_AUTONEG_CONFIG_ABILITY_FIELD_WIDTH,
    LIF_P2P_AUTONEG_ABILITY_CONFIG_REG_CF_LIF_P2P_AE_AUTONEG_CONFIG_ABILITY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ_RESERVED0_FIELD_MASK,
    0,
    LIF_P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ_RESERVED0_FIELD_WIDTH,
    LIF_P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ_CF_LIF_P2P_AE_AUTONEG_LP_ABILITY_READ
 ******************************************************************************/
const ru_field_rec LIF_P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ_CF_LIF_P2P_AE_AUTONEG_LP_ABILITY_READ_FIELD =
{
    "CF_LIF_P2P_AE_AUTONEG_LP_ABILITY_READ",
#if RU_INCLUDE_DESC
    "",
    "Defines the Autonegotiation Ability / Config Register of LINK"
    "PARTNER"
    ""
    "Only updates when AN is complete"
    ""
    "cf_autoneg_mode_sel = 0 (CL37)"
    ""
    "Bits"
    "[15]: NP, Next Page"
    "[14]: ACK (H/W overwrite)"
    "[13]: RF2, Remote Fault"
    "[12]: RF1, Remote Fault"
    "[11:9]: Reserved"
    "[8]: PS2, Asymmetric Pause"
    "[7]: PS1, Symmetric Pause"
    "[6]: HD, Half Duplex"
    "[5]: FD, Full Duplex"
    "[4:0]: Reserved"
    ""
    ""
    "cf_autoneg_mode_sel = 1 (SGMII)"
    ""
    "Bits"
    "[15]: Link State (HW Controlled) 1 = Link Up, 0 = Link Down"
    "[14]: ACK (H/W overwrite)"
    "[13]: Reserved"
    "[12]: Duplex mode: 1 = Full Duplex, 0 = Half Duplex"
    "[11:10]: 11 = Reserved, 10 = 1000Mbps, 01 = 100Mbps, 00 = 10 Mbps"
    "[9:1]: Reserved"
    "[0]: Set 1 to per SGMII Spec (S/W must set)",
#endif
    LIF_P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ_CF_LIF_P2P_AE_AUTONEG_LP_ABILITY_READ_FIELD_MASK,
    0,
    LIF_P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ_CF_LIF_P2P_AE_AUTONEG_LP_ABILITY_READ_FIELD_WIDTH,
    LIF_P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ_CF_LIF_P2P_AE_AUTONEG_LP_ABILITY_READ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LIF_P2P_INTER_OP_CONTROL_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_P2P_INTER_OP_CONTROL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_P2P_INTER_OP_CONTROL_RESERVED0_FIELD_MASK,
    0,
    LIF_P2P_INTER_OP_CONTROL_RESERVED0_FIELD_WIDTH,
    LIF_P2P_INTER_OP_CONTROL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_INTER_OP_CONTROL_CFP2PMASKONEPRECIDLE
 ******************************************************************************/
const ru_field_rec LIF_P2P_INTER_OP_CONTROL_CFP2PMASKONEPRECIDLE_FIELD =
{
    "CFP2PMASKONEPRECIDLE",
#if RU_INCLUDE_DESC
    "",
    "Mask one preceding idle before SOP detection.  This allows for 4-5"
    "bytes of IPG operation for compability with 2.5X1000BASE-X AE 2.5G"
    "mode.",
#endif
    LIF_P2P_INTER_OP_CONTROL_CFP2PMASKONEPRECIDLE_FIELD_MASK,
    0,
    LIF_P2P_INTER_OP_CONTROL_CFP2PMASKONEPRECIDLE_FIELD_WIDTH,
    LIF_P2P_INTER_OP_CONTROL_CFP2PMASKONEPRECIDLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_INTER_OP_CONTROL_CFP2PMODETRANSMITREMFAULT
 ******************************************************************************/
const ru_field_rec LIF_P2P_INTER_OP_CONTROL_CFP2PMODETRANSMITREMFAULT_FIELD =
{
    "CFP2PMODETRANSMITREMFAULT",
#if RU_INCLUDE_DESC
    "",
    "Enable Transmit Remote Fault for AE"
    ""
    "Send /K28.5/W/ in response to loss of sync at the 1G AE receiver.",
#endif
    LIF_P2P_INTER_OP_CONTROL_CFP2PMODETRANSMITREMFAULT_FIELD_MASK,
    0,
    LIF_P2P_INTER_OP_CONTROL_CFP2PMODETRANSMITREMFAULT_FIELD_WIDTH,
    LIF_P2P_INTER_OP_CONTROL_CFP2PMODETRANSMITREMFAULT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_INTER_OP_CONTROL_CFP2PMODETRANSMITDEFICITIDLEENABLE
 ******************************************************************************/
const ru_field_rec LIF_P2P_INTER_OP_CONTROL_CFP2PMODETRANSMITDEFICITIDLEENABLE_FIELD =
{
    "CFP2PMODETRANSMITDEFICITIDLEENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enable Deficit Idle Count Mode"
    ""
    "Vary IPG between 9-15 to align /S/ to particular lane.",
#endif
    LIF_P2P_INTER_OP_CONTROL_CFP2PMODETRANSMITDEFICITIDLEENABLE_FIELD_MASK,
    0,
    LIF_P2P_INTER_OP_CONTROL_CFP2PMODETRANSMITDEFICITIDLEENABLE_FIELD_WIDTH,
    LIF_P2P_INTER_OP_CONTROL_CFP2PMODETRANSMITDEFICITIDLEENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_INTER_OP_CONTROL_RESERVED1
 ******************************************************************************/
const ru_field_rec LIF_P2P_INTER_OP_CONTROL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_P2P_INTER_OP_CONTROL_RESERVED1_FIELD_MASK,
    0,
    LIF_P2P_INTER_OP_CONTROL_RESERVED1_FIELD_WIDTH,
    LIF_P2P_INTER_OP_CONTROL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_INTER_OP_CONTROL_CFP2PMODEADAPTIVESHORTPRE
 ******************************************************************************/
const ru_field_rec LIF_P2P_INTER_OP_CONTROL_CFP2PMODEADAPTIVESHORTPRE_FIELD =
{
    "CFP2PMODEADAPTIVESHORTPRE",
#if RU_INCLUDE_DESC
    "",
    "Enables adaptive short preamble mode.  AE PCS Transmitter will send"
    "short preambles in an adaptive manner to maintain 1Gbps rate.",
#endif
    LIF_P2P_INTER_OP_CONTROL_CFP2PMODEADAPTIVESHORTPRE_FIELD_MASK,
    0,
    LIF_P2P_INTER_OP_CONTROL_CFP2PMODEADAPTIVESHORTPRE_FIELD_WIDTH,
    LIF_P2P_INTER_OP_CONTROL_CFP2PMODEADAPTIVESHORTPRE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_2_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_PON_CONTROL_2_RESERVED0_FIELD_MASK,
    0,
    LIF_PON_CONTROL_2_RESERVED0_FIELD_WIDTH,
    LIF_PON_CONTROL_2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_2_CFGDISP2PDNINDEXMAP
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_2_CFGDISP2PDNINDEXMAP_FIELD =
{
    "CFGDISP2PDNINDEXMAP",
#if RU_INCLUDE_DESC
    "",
    "Disable P2P downstream index mapping, default to index 0.",
#endif
    LIF_PON_CONTROL_2_CFGDISP2PDNINDEXMAP_FIELD_MASK,
    0,
    LIF_PON_CONTROL_2_CFGDISP2PDNINDEXMAP_FIELD_WIDTH,
    LIF_PON_CONTROL_2_CFGDISP2PDNINDEXMAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_PON_CONTROL_2_CFGDISP2PUPINDEXMAP
 ******************************************************************************/
const ru_field_rec LIF_PON_CONTROL_2_CFGDISP2PUPINDEXMAP_FIELD =
{
    "CFGDISP2PUPINDEXMAP",
#if RU_INCLUDE_DESC
    "",
    "Disable P2P upstream index mapping, default to index 0.",
#endif
    LIF_PON_CONTROL_2_CFGDISP2PUPINDEXMAP_FIELD_MASK,
    0,
    LIF_PON_CONTROL_2_CFGDISP2PUPINDEXMAP_FIELD_WIDTH,
    LIF_PON_CONTROL_2_CFGDISP2PUPINDEXMAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_VLAN_TYPE_1_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_VLAN_TYPE_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_VLAN_TYPE_1_RESERVED0_FIELD_MASK,
    0,
    LIF_VLAN_TYPE_1_RESERVED0_FIELD_WIDTH,
    LIF_VLAN_TYPE_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_VLAN_TYPE_1_CFGVLANTYPE_1
 ******************************************************************************/
const ru_field_rec LIF_VLAN_TYPE_1_CFGVLANTYPE_1_FIELD =
{
    "CFGVLANTYPE_1",
#if RU_INCLUDE_DESC
    "",
    "Defines a VLAN type.",
#endif
    LIF_VLAN_TYPE_1_CFGVLANTYPE_1_FIELD_MASK,
    0,
    LIF_VLAN_TYPE_1_CFGVLANTYPE_1_FIELD_WIDTH,
    LIF_VLAN_TYPE_1_CFGVLANTYPE_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_16_CFGP2PSCI_LO_16
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_16_CFGP2PSCI_LO_16_FIELD =
{
    "CFGP2PSCI_LO_16",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_16_CFGP2PSCI_LO_16_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_16_CFGP2PSCI_LO_16_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_16_CFGP2PSCI_LO_16_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_16_CFGP2PSCI_HI_16
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_16_CFGP2PSCI_HI_16_FIELD =
{
    "CFGP2PSCI_HI_16",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_16_CFGP2PSCI_HI_16_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_16_CFGP2PSCI_HI_16_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_16_CFGP2PSCI_HI_16_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_17_CFGP2PSCI_LO_17
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_17_CFGP2PSCI_LO_17_FIELD =
{
    "CFGP2PSCI_LO_17",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_17_CFGP2PSCI_LO_17_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_17_CFGP2PSCI_LO_17_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_17_CFGP2PSCI_LO_17_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_17_CFGP2PSCI_HI_17
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_17_CFGP2PSCI_HI_17_FIELD =
{
    "CFGP2PSCI_HI_17",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_17_CFGP2PSCI_HI_17_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_17_CFGP2PSCI_HI_17_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_17_CFGP2PSCI_HI_17_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_18_CFGP2PSCI_LO_18
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_18_CFGP2PSCI_LO_18_FIELD =
{
    "CFGP2PSCI_LO_18",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_18_CFGP2PSCI_LO_18_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_18_CFGP2PSCI_LO_18_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_18_CFGP2PSCI_LO_18_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_18_CFGP2PSCI_HI_18
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_18_CFGP2PSCI_HI_18_FIELD =
{
    "CFGP2PSCI_HI_18",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_18_CFGP2PSCI_HI_18_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_18_CFGP2PSCI_HI_18_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_18_CFGP2PSCI_HI_18_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_19_CFGP2PSCI_LO_19
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_19_CFGP2PSCI_LO_19_FIELD =
{
    "CFGP2PSCI_LO_19",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_19_CFGP2PSCI_LO_19_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_19_CFGP2PSCI_LO_19_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_19_CFGP2PSCI_LO_19_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_19_CFGP2PSCI_HI_19
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_19_CFGP2PSCI_HI_19_FIELD =
{
    "CFGP2PSCI_HI_19",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_19_CFGP2PSCI_HI_19_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_19_CFGP2PSCI_HI_19_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_19_CFGP2PSCI_HI_19_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_20_CFGP2PSCI_LO_20
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_20_CFGP2PSCI_LO_20_FIELD =
{
    "CFGP2PSCI_LO_20",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_20_CFGP2PSCI_LO_20_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_20_CFGP2PSCI_LO_20_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_20_CFGP2PSCI_LO_20_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_20_CFGP2PSCI_HI_20
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_20_CFGP2PSCI_HI_20_FIELD =
{
    "CFGP2PSCI_HI_20",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_20_CFGP2PSCI_HI_20_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_20_CFGP2PSCI_HI_20_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_20_CFGP2PSCI_HI_20_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_21_CFGP2PSCI_LO_21
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_21_CFGP2PSCI_LO_21_FIELD =
{
    "CFGP2PSCI_LO_21",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_21_CFGP2PSCI_LO_21_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_21_CFGP2PSCI_LO_21_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_21_CFGP2PSCI_LO_21_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_21_CFGP2PSCI_HI_21
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_21_CFGP2PSCI_HI_21_FIELD =
{
    "CFGP2PSCI_HI_21",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_21_CFGP2PSCI_HI_21_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_21_CFGP2PSCI_HI_21_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_21_CFGP2PSCI_HI_21_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_22_CFGP2PSCI_LO_22
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_22_CFGP2PSCI_LO_22_FIELD =
{
    "CFGP2PSCI_LO_22",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_22_CFGP2PSCI_LO_22_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_22_CFGP2PSCI_LO_22_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_22_CFGP2PSCI_LO_22_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_22_CFGP2PSCI_HI_22
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_22_CFGP2PSCI_HI_22_FIELD =
{
    "CFGP2PSCI_HI_22",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_22_CFGP2PSCI_HI_22_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_22_CFGP2PSCI_HI_22_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_22_CFGP2PSCI_HI_22_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_23_CFGP2PSCI_LO_23
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_23_CFGP2PSCI_LO_23_FIELD =
{
    "CFGP2PSCI_LO_23",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_23_CFGP2PSCI_LO_23_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_23_CFGP2PSCI_LO_23_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_23_CFGP2PSCI_LO_23_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_23_CFGP2PSCI_HI_23
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_23_CFGP2PSCI_HI_23_FIELD =
{
    "CFGP2PSCI_HI_23",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_23_CFGP2PSCI_HI_23_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_23_CFGP2PSCI_HI_23_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_23_CFGP2PSCI_HI_23_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_24_CFGP2PSCI_LO_24
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_24_CFGP2PSCI_LO_24_FIELD =
{
    "CFGP2PSCI_LO_24",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_24_CFGP2PSCI_LO_24_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_24_CFGP2PSCI_LO_24_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_24_CFGP2PSCI_LO_24_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_24_CFGP2PSCI_HI_24
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_24_CFGP2PSCI_HI_24_FIELD =
{
    "CFGP2PSCI_HI_24",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_24_CFGP2PSCI_HI_24_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_24_CFGP2PSCI_HI_24_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_24_CFGP2PSCI_HI_24_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_25_CFGP2PSCI_LO_25
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_25_CFGP2PSCI_LO_25_FIELD =
{
    "CFGP2PSCI_LO_25",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_25_CFGP2PSCI_LO_25_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_25_CFGP2PSCI_LO_25_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_25_CFGP2PSCI_LO_25_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_25_CFGP2PSCI_HI_25
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_25_CFGP2PSCI_HI_25_FIELD =
{
    "CFGP2PSCI_HI_25",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_25_CFGP2PSCI_HI_25_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_25_CFGP2PSCI_HI_25_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_25_CFGP2PSCI_HI_25_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_26_CFGP2PSCI_LO_26
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_26_CFGP2PSCI_LO_26_FIELD =
{
    "CFGP2PSCI_LO_26",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_26_CFGP2PSCI_LO_26_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_26_CFGP2PSCI_LO_26_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_26_CFGP2PSCI_LO_26_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_26_CFGP2PSCI_HI_26
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_26_CFGP2PSCI_HI_26_FIELD =
{
    "CFGP2PSCI_HI_26",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_26_CFGP2PSCI_HI_26_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_26_CFGP2PSCI_HI_26_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_26_CFGP2PSCI_HI_26_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_27_CFGP2PSCI_LO_27
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_27_CFGP2PSCI_LO_27_FIELD =
{
    "CFGP2PSCI_LO_27",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_27_CFGP2PSCI_LO_27_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_27_CFGP2PSCI_LO_27_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_27_CFGP2PSCI_LO_27_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_27_CFGP2PSCI_HI_27
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_27_CFGP2PSCI_HI_27_FIELD =
{
    "CFGP2PSCI_HI_27",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_27_CFGP2PSCI_HI_27_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_27_CFGP2PSCI_HI_27_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_27_CFGP2PSCI_HI_27_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_28_CFGP2PSCI_LO_28
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_28_CFGP2PSCI_LO_28_FIELD =
{
    "CFGP2PSCI_LO_28",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_28_CFGP2PSCI_LO_28_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_28_CFGP2PSCI_LO_28_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_28_CFGP2PSCI_LO_28_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_28_CFGP2PSCI_HI_28
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_28_CFGP2PSCI_HI_28_FIELD =
{
    "CFGP2PSCI_HI_28",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_28_CFGP2PSCI_HI_28_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_28_CFGP2PSCI_HI_28_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_28_CFGP2PSCI_HI_28_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_29_CFGP2PSCI_LO_29
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_29_CFGP2PSCI_LO_29_FIELD =
{
    "CFGP2PSCI_LO_29",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_29_CFGP2PSCI_LO_29_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_29_CFGP2PSCI_LO_29_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_29_CFGP2PSCI_LO_29_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_29_CFGP2PSCI_HI_29
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_29_CFGP2PSCI_HI_29_FIELD =
{
    "CFGP2PSCI_HI_29",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_29_CFGP2PSCI_HI_29_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_29_CFGP2PSCI_HI_29_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_29_CFGP2PSCI_HI_29_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_30_CFGP2PSCI_LO_30
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_30_CFGP2PSCI_LO_30_FIELD =
{
    "CFGP2PSCI_LO_30",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_30_CFGP2PSCI_LO_30_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_30_CFGP2PSCI_LO_30_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_30_CFGP2PSCI_LO_30_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_30_CFGP2PSCI_HI_30
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_30_CFGP2PSCI_HI_30_FIELD =
{
    "CFGP2PSCI_HI_30",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_30_CFGP2PSCI_HI_30_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_30_CFGP2PSCI_HI_30_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_30_CFGP2PSCI_HI_30_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_LO_31_CFGP2PSCI_LO_31
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_LO_31_CFGP2PSCI_LO_31_FIELD =
{
    "CFGP2PSCI_LO_31",
#if RU_INCLUDE_DESC
    "",
    "Defines the lower 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_LO_31_CFGP2PSCI_LO_31_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_LO_31_CFGP2PSCI_LO_31_FIELD_WIDTH,
    LIF_P2P_AE_SCI_LO_31_CFGP2PSCI_LO_31_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_P2P_AE_SCI_HI_31_CFGP2PSCI_HI_31
 ******************************************************************************/
const ru_field_rec LIF_P2P_AE_SCI_HI_31_CFGP2PSCI_HI_31_FIELD =
{
    "CFGP2PSCI_HI_31",
#if RU_INCLUDE_DESC
    "",
    "Defines the upper 32-bits lookup value of SCI to index 0. If"
    "implicit SCI mode, index defaults to what was mapped by"
    "LIF_LLID_[0:15] with value 0x5555.",
#endif
    LIF_P2P_AE_SCI_HI_31_CFGP2PSCI_HI_31_FIELD_MASK,
    0,
    LIF_P2P_AE_SCI_HI_31_CFGP2PSCI_HI_31_FIELD_WIDTH,
    LIF_P2P_AE_SCI_HI_31_CFGP2PSCI_HI_31_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_2_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_SEC_CONTROL_2_RESERVED0_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_2_RESERVED0_FIELD_WIDTH,
    LIF_SEC_CONTROL_2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_2_CFGENSECOUTFFPRIME
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_2_CFGENSECOUTFFPRIME_FIELD =
{
    "CFGENSECOUTFFPRIME",
#if RU_INCLUDE_DESC
    "",
    "Enables priming of upstream security output FIFO to LIF's PCS.",
#endif
    LIF_SEC_CONTROL_2_CFGENSECOUTFFPRIME_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_2_CFGENSECOUTFFPRIME_FIELD_WIDTH,
    LIF_SEC_CONTROL_2_CFGENSECOUTFFPRIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_2_CFGENAES_256_UP
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_2_CFGENAES_256_UP_FIELD =
{
    "CFGENAES_256_UP",
#if RU_INCLUDE_DESC
    "",
    "Enables upstream 256 AES encryption : 0 - 128; 1 - 256.",
#endif
    LIF_SEC_CONTROL_2_CFGENAES_256_UP_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_2_CFGENAES_256_UP_FIELD_WIDTH,
    LIF_SEC_CONTROL_2_CFGENAES_256_UP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_2_CFGENAES_256_DN
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_2_CFGENAES_256_DN_FIELD =
{
    "CFGENAES_256_DN",
#if RU_INCLUDE_DESC
    "",
    "Enables downstream 256 AES encryption : 0 - 128; 1 - 256.",
#endif
    LIF_SEC_CONTROL_2_CFGENAES_256_DN_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_2_CFGENAES_256_DN_FIELD_WIDTH,
    LIF_SEC_CONTROL_2_CFGENAES_256_DN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_2_CFG_MACSEC_XPN_TX
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_2_CFG_MACSEC_XPN_TX_FIELD =
{
    "CFG_MACSEC_XPN_TX",
#if RU_INCLUDE_DESC
    "",
    "Enable upstream 802.1ae extended packet numbering.",
#endif
    LIF_SEC_CONTROL_2_CFG_MACSEC_XPN_TX_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_2_CFG_MACSEC_XPN_TX_FIELD_WIDTH,
    LIF_SEC_CONTROL_2_CFG_MACSEC_XPN_TX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_2_CFG_MACSEC_XPN_RX
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_2_CFG_MACSEC_XPN_RX_FIELD =
{
    "CFG_MACSEC_XPN_RX",
#if RU_INCLUDE_DESC
    "",
    "Enable downstream 802.1ae extended packet numbering.",
#endif
    LIF_SEC_CONTROL_2_CFG_MACSEC_XPN_RX_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_2_CFG_MACSEC_XPN_RX_FIELD_WIDTH,
    LIF_SEC_CONTROL_2_CFG_MACSEC_XPN_RX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_3
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_3_FIELD =
{
    "CFGKEYNUM_4_AN_3",
#if RU_INCLUDE_DESC
    "",
    "Applicable only for 802.1ae RX.  Defines the key number mapping for"
    "TCI's association number 3, AN[1:0].",
#endif
    LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_3_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_3_FIELD_WIDTH,
    LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_2
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_2_FIELD =
{
    "CFGKEYNUM_4_AN_2",
#if RU_INCLUDE_DESC
    "",
    "Applicable only for 802.1ae RX.  Defines the key number mapping for"
    "TCI's association number 2, AN[1:0].",
#endif
    LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_2_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_2_FIELD_WIDTH,
    LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_1
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_1_FIELD =
{
    "CFGKEYNUM_4_AN_1",
#if RU_INCLUDE_DESC
    "",
    "Applicable only for 802.1ae RX.  Defines the key number mapping for"
    "TCI's association number 1, AN[1:0].",
#endif
    LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_1_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_1_FIELD_WIDTH,
    LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_0
 ******************************************************************************/
const ru_field_rec LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_0_FIELD =
{
    "CFGKEYNUM_4_AN_0",
#if RU_INCLUDE_DESC
    "",
    "Applicable only for 802.1ae RX.  Defines the key number mapping for"
    "TCI's association number 0, AN[1:0].",
#endif
    LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_0_FIELD_MASK,
    0,
    LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_0_FIELD_WIDTH,
    LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_MPCP_OFFSET_CFGSECDNMPCPOFFSET
 ******************************************************************************/
const ru_field_rec LIF_SEC_MPCP_OFFSET_CFGSECDNMPCPOFFSET_FIELD =
{
    "CFGSECDNMPCPOFFSET",
#if RU_INCLUDE_DESC
    "",
    "Provides MPCP adjustment for ZOH/EPON security.",
#endif
    LIF_SEC_MPCP_OFFSET_CFGSECDNMPCPOFFSET_FIELD_MASK,
    0,
    LIF_SEC_MPCP_OFFSET_CFGSECDNMPCPOFFSET_FIELD_WIDTH,
    LIF_SEC_MPCP_OFFSET_CFGSECDNMPCPOFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_1588_TS_SYNC_OFFSET_CFG_TS48_SYNC_NS_INCREMENT
 ******************************************************************************/
const ru_field_rec LIF_1588_TS_SYNC_OFFSET_CFG_TS48_SYNC_NS_INCREMENT_FIELD =
{
    "CFG_TS48_SYNC_NS_INCREMENT",
#if RU_INCLUDE_DESC
    "",
    "Selects the synchronized TS48 nanosecond increment : 0 - 8ns unit; 1"
    "- 3.2ns unit.  Should be set in 2.5G active Ethernet mode.",
#endif
    LIF_1588_TS_SYNC_OFFSET_CFG_TS48_SYNC_NS_INCREMENT_FIELD_MASK,
    0,
    LIF_1588_TS_SYNC_OFFSET_CFG_TS48_SYNC_NS_INCREMENT_FIELD_WIDTH,
    LIF_1588_TS_SYNC_OFFSET_CFG_TS48_SYNC_NS_INCREMENT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_1588_TS_SYNC_OFFSET_RESERVED0
 ******************************************************************************/
const ru_field_rec LIF_1588_TS_SYNC_OFFSET_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LIF_1588_TS_SYNC_OFFSET_RESERVED0_FIELD_MASK,
    0,
    LIF_1588_TS_SYNC_OFFSET_RESERVED0_FIELD_WIDTH,
    LIF_1588_TS_SYNC_OFFSET_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_1588_TS_SYNC_OFFSET_CFGTSSYNCOFFSET_312
 ******************************************************************************/
const ru_field_rec LIF_1588_TS_SYNC_OFFSET_CFGTSSYNCOFFSET_312_FIELD =
{
    "CFGTSSYNCOFFSET_312",
#if RU_INCLUDE_DESC
    "",
    "Provides lowest 6 bits of timestamp synchronizer, from 250 MHz"
    "domain to 312.5 MHz.  It is utilized when cfg_ts48_sync_ns_increment"
    "is set to 1.",
#endif
    LIF_1588_TS_SYNC_OFFSET_CFGTSSYNCOFFSET_312_FIELD_MASK,
    0,
    LIF_1588_TS_SYNC_OFFSET_CFGTSSYNCOFFSET_312_FIELD_WIDTH,
    LIF_1588_TS_SYNC_OFFSET_CFGTSSYNCOFFSET_312_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_1588_TS_SYNC_OFFSET_CFGTSSYNCOFFSET_125
 ******************************************************************************/
const ru_field_rec LIF_1588_TS_SYNC_OFFSET_CFGTSSYNCOFFSET_125_FIELD =
{
    "CFGTSSYNCOFFSET_125",
#if RU_INCLUDE_DESC
    "",
    "Provides lowest 6 bits of timestamp synchronizer, from 250 MHz"
    "domain to 125 MHz.  It is utilized when cfg_ts48_sync_ns_increment"
    "is set to 0.",
#endif
    LIF_1588_TS_SYNC_OFFSET_CFGTSSYNCOFFSET_125_FIELD_MASK,
    0,
    LIF_1588_TS_SYNC_OFFSET_CFGTSSYNCOFFSET_125_FIELD_WIDTH,
    LIF_1588_TS_SYNC_OFFSET_CFGTSSYNCOFFSET_125_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_DATA_PORT_DATA_8_PBIPORTDATA8
 ******************************************************************************/
const ru_field_rec LIF_DATA_PORT_DATA_8_PBIPORTDATA8_FIELD =
{
    "PBIPORTDATA8",
#if RU_INCLUDE_DESC
    "",
    "For write operations, the data to be written to the RAM location."
    "For read operations, the data read back from the RAM location.",
#endif
    LIF_DATA_PORT_DATA_8_PBIPORTDATA8_FIELD_MASK,
    0,
    LIF_DATA_PORT_DATA_8_PBIPORTDATA8_FIELD_WIDTH,
    LIF_DATA_PORT_DATA_8_PBIPORTDATA8_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_DATA_PORT_DATA_9_PBIPORTDATA9
 ******************************************************************************/
const ru_field_rec LIF_DATA_PORT_DATA_9_PBIPORTDATA9_FIELD =
{
    "PBIPORTDATA9",
#if RU_INCLUDE_DESC
    "",
    "For write operations, the data to be written to the RAM location."
    "For read operations, the data read back from the RAM location.",
#endif
    LIF_DATA_PORT_DATA_9_PBIPORTDATA9_FIELD_MASK,
    0,
    LIF_DATA_PORT_DATA_9_PBIPORTDATA9_FIELD_WIDTH,
    LIF_DATA_PORT_DATA_9_PBIPORTDATA9_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_DATA_PORT_DATA_10_PBIPORTDATA10
 ******************************************************************************/
const ru_field_rec LIF_DATA_PORT_DATA_10_PBIPORTDATA10_FIELD =
{
    "PBIPORTDATA10",
#if RU_INCLUDE_DESC
    "",
    "For write operations, the data to be written to the RAM location."
    "For read operations, the data read back from the RAM location.",
#endif
    LIF_DATA_PORT_DATA_10_PBIPORTDATA10_FIELD_MASK,
    0,
    LIF_DATA_PORT_DATA_10_PBIPORTDATA10_FIELD_WIDTH,
    LIF_DATA_PORT_DATA_10_PBIPORTDATA10_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_DATA_PORT_DATA_11_PBIPORTDATA11
 ******************************************************************************/
const ru_field_rec LIF_DATA_PORT_DATA_11_PBIPORTDATA11_FIELD =
{
    "PBIPORTDATA11",
#if RU_INCLUDE_DESC
    "",
    "For write operations, the data to be written to the RAM location."
    "For read operations, the data read back from the RAM location.",
#endif
    LIF_DATA_PORT_DATA_11_PBIPORTDATA11_FIELD_MASK,
    0,
    LIF_DATA_PORT_DATA_11_PBIPORTDATA11_FIELD_WIDTH,
    LIF_DATA_PORT_DATA_11_PBIPORTDATA11_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_TX_SSCI_CFG_MACSEC_XPN_TX_SSCI
 ******************************************************************************/
const ru_field_rec LIF_SEC_TX_SSCI_CFG_MACSEC_XPN_TX_SSCI_FIELD =
{
    "CFG_MACSEC_XPN_TX_SSCI",
#if RU_INCLUDE_DESC
    "",
    "SSCI[31:0].",
#endif
    LIF_SEC_TX_SSCI_CFG_MACSEC_XPN_TX_SSCI_FIELD_MASK,
    0,
    LIF_SEC_TX_SSCI_CFG_MACSEC_XPN_TX_SSCI_FIELD_WIDTH,
    LIF_SEC_TX_SSCI_CFG_MACSEC_XPN_TX_SSCI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_TX_SALT_0_CFG_MACSEC_XPN_TX_SALT_0
 ******************************************************************************/
const ru_field_rec LIF_SEC_TX_SALT_0_CFG_MACSEC_XPN_TX_SALT_0_FIELD =
{
    "CFG_MACSEC_XPN_TX_SALT_0",
#if RU_INCLUDE_DESC
    "",
    "SALT[31:0].",
#endif
    LIF_SEC_TX_SALT_0_CFG_MACSEC_XPN_TX_SALT_0_FIELD_MASK,
    0,
    LIF_SEC_TX_SALT_0_CFG_MACSEC_XPN_TX_SALT_0_FIELD_WIDTH,
    LIF_SEC_TX_SALT_0_CFG_MACSEC_XPN_TX_SALT_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_TX_SALT_1_CFG_MACSEC_XPN_TX_SALT_1
 ******************************************************************************/
const ru_field_rec LIF_SEC_TX_SALT_1_CFG_MACSEC_XPN_TX_SALT_1_FIELD =
{
    "CFG_MACSEC_XPN_TX_SALT_1",
#if RU_INCLUDE_DESC
    "",
    "SALT[63:32].",
#endif
    LIF_SEC_TX_SALT_1_CFG_MACSEC_XPN_TX_SALT_1_FIELD_MASK,
    0,
    LIF_SEC_TX_SALT_1_CFG_MACSEC_XPN_TX_SALT_1_FIELD_WIDTH,
    LIF_SEC_TX_SALT_1_CFG_MACSEC_XPN_TX_SALT_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_TX_SALT_2_CFG_MACSEC_XPN_TX_SALT_2
 ******************************************************************************/
const ru_field_rec LIF_SEC_TX_SALT_2_CFG_MACSEC_XPN_TX_SALT_2_FIELD =
{
    "CFG_MACSEC_XPN_TX_SALT_2",
#if RU_INCLUDE_DESC
    "",
    "SALT[95:64].",
#endif
    LIF_SEC_TX_SALT_2_CFG_MACSEC_XPN_TX_SALT_2_FIELD_MASK,
    0,
    LIF_SEC_TX_SALT_2_CFG_MACSEC_XPN_TX_SALT_2_FIELD_WIDTH,
    LIF_SEC_TX_SALT_2_CFG_MACSEC_XPN_TX_SALT_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_RX_SSCI_CFG_MACSEC_XPN_RX_SSCI
 ******************************************************************************/
const ru_field_rec LIF_SEC_RX_SSCI_CFG_MACSEC_XPN_RX_SSCI_FIELD =
{
    "CFG_MACSEC_XPN_RX_SSCI",
#if RU_INCLUDE_DESC
    "",
    "SSCI[31:0].",
#endif
    LIF_SEC_RX_SSCI_CFG_MACSEC_XPN_RX_SSCI_FIELD_MASK,
    0,
    LIF_SEC_RX_SSCI_CFG_MACSEC_XPN_RX_SSCI_FIELD_WIDTH,
    LIF_SEC_RX_SSCI_CFG_MACSEC_XPN_RX_SSCI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_RX_SALT_0_CFG_MACSEC_XPN_RX_SALT_0
 ******************************************************************************/
const ru_field_rec LIF_SEC_RX_SALT_0_CFG_MACSEC_XPN_RX_SALT_0_FIELD =
{
    "CFG_MACSEC_XPN_RX_SALT_0",
#if RU_INCLUDE_DESC
    "",
    "SALT[31:0].",
#endif
    LIF_SEC_RX_SALT_0_CFG_MACSEC_XPN_RX_SALT_0_FIELD_MASK,
    0,
    LIF_SEC_RX_SALT_0_CFG_MACSEC_XPN_RX_SALT_0_FIELD_WIDTH,
    LIF_SEC_RX_SALT_0_CFG_MACSEC_XPN_RX_SALT_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_RX_SALT_1_CFG_MACSEC_XPN_RX_SALT_1
 ******************************************************************************/
const ru_field_rec LIF_SEC_RX_SALT_1_CFG_MACSEC_XPN_RX_SALT_1_FIELD =
{
    "CFG_MACSEC_XPN_RX_SALT_1",
#if RU_INCLUDE_DESC
    "",
    "SALT[63:32].",
#endif
    LIF_SEC_RX_SALT_1_CFG_MACSEC_XPN_RX_SALT_1_FIELD_MASK,
    0,
    LIF_SEC_RX_SALT_1_CFG_MACSEC_XPN_RX_SALT_1_FIELD_WIDTH,
    LIF_SEC_RX_SALT_1_CFG_MACSEC_XPN_RX_SALT_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LIF_SEC_RX_SALT_2_CFG_MACSEC_XPN_RX_SALT_2
 ******************************************************************************/
const ru_field_rec LIF_SEC_RX_SALT_2_CFG_MACSEC_XPN_RX_SALT_2_FIELD =
{
    "CFG_MACSEC_XPN_RX_SALT_2",
#if RU_INCLUDE_DESC
    "",
    "SALT[95:64].",
#endif
    LIF_SEC_RX_SALT_2_CFG_MACSEC_XPN_RX_SALT_2_FIELD_MASK,
    0,
    LIF_SEC_RX_SALT_2_CFG_MACSEC_XPN_RX_SALT_2_FIELD_WIDTH,
    LIF_SEC_RX_SALT_2_CFG_MACSEC_XPN_RX_SALT_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: LIF_PON_CONTROL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_PON_CONTROL_FIELDS[] =
{
    &LIF_PON_CONTROL_CFG_LIF_SGMII_MODE_FIELD,
    &LIF_PON_CONTROL_CFGDISRUNTFILTER_FIELD,
    &LIF_PON_CONTROL_CFMAXCOMMAERRCNT_FIELD,
    &LIF_PON_CONTROL_CFSYNCSMSELECT_FIELD,
    &LIF_PON_CONTROL_CFPONRXFORCENONFECABORT_FIELD,
    &LIF_PON_CONTROL_CFPONRXFORCEFECABORT_FIELD,
    &LIF_PON_CONTROL_CFGRXDATABITFLIP_FIELD,
    &LIF_PON_CONTROL_RESERVED0_FIELD,
    &LIF_PON_CONTROL_CFGENMPCPOAMPAD_FIELD,
    &LIF_PON_CONTROL_CFGENTXIDLEPKT_FIELD,
    &LIF_PON_CONTROL_CFENABLESOFTWARESYNCHOLD_FIELD,
    &LIF_PON_CONTROL_CFENABLEEXTENDSYNC_FIELD,
    &LIF_PON_CONTROL_CFENABLEQUICKSYNC_FIELD,
    &LIF_PON_CONTROL_CFPPSEN_FIELD,
    &LIF_PON_CONTROL_CFPPSCLKRBC_FIELD,
    &LIF_PON_CONTROL_CFRX2TXLPBACK_FIELD,
    &LIF_PON_CONTROL_CFTX2RXLPBACK_FIELD,
    &LIF_PON_CONTROL_CFTXDATAENDURLON_FIELD,
    &LIF_PON_CONTROL_CFP2PMODE_FIELD,
    &LIF_PON_CONTROL_CFP2PSHORTPRE_FIELD,
    &LIF_PON_CONTROL_CFLASEREN_FIELD,
    &LIF_PON_CONTROL_CFTXLASERON_FIELD,
    &LIF_PON_CONTROL_CFTXLASERONACTHI_FIELD,
    &LIF_PON_CONTROL_LIFTXRSTN_PRE_FIELD,
    &LIF_PON_CONTROL_LIFRXRSTN_PRE_FIELD,
    &LIF_PON_CONTROL_LIFTXEN_FIELD,
    &LIF_PON_CONTROL_LIFRXEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_PON_CONTROL_REG = 
{
    "PON_CONTROL",
#if RU_INCLUDE_DESC
    "LIF_PON_CONTROL Register",
    "This register controls and configures the LIF PON module. Configuration"
    "bits dealing directly with the operational mode, laser control, and"
    "loopback are here.",
#endif
    LIF_PON_CONTROL_REG_OFFSET,
    0,
    0,
    273,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    27,
    LIF_PON_CONTROL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_PON_INTER_OP_CONTROL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_PON_INTER_OP_CONTROL_FIELDS[] =
{
    &LIF_PON_INTER_OP_CONTROL_RESERVED0_FIELD,
    &LIF_PON_INTER_OP_CONTROL_CFIPGFILTER_FIELD,
    &LIF_PON_INTER_OP_CONTROL_CFDISABLELOSLASERBLOCK_FIELD,
    &LIF_PON_INTER_OP_CONTROL_CFGLLIDPROMISCUOUSMODE_FIELD,
    &LIF_PON_INTER_OP_CONTROL_CFGLLIDMODMSK_FIELD,
    &LIF_PON_INTER_OP_CONTROL_CFUSEFECIPG_FIELD,
    &LIF_PON_INTER_OP_CONTROL_CFRXCRC8INVCHK_FIELD,
    &LIF_PON_INTER_OP_CONTROL_CFRXCRC8BITSWAP_FIELD,
    &LIF_PON_INTER_OP_CONTROL_CFRXCRC8MSB2LSB_FIELD,
    &LIF_PON_INTER_OP_CONTROL_CFRXCRC8DISABLE_FIELD,
    &LIF_PON_INTER_OP_CONTROL_RESERVED1_FIELD,
    &LIF_PON_INTER_OP_CONTROL_CFTXLLIDBIT15SET_FIELD,
    &LIF_PON_INTER_OP_CONTROL_CFTXCRC8INV_FIELD,
    &LIF_PON_INTER_OP_CONTROL_CFTXCRC8BAD_FIELD,
    &LIF_PON_INTER_OP_CONTROL_CFTXCRC8BITSWAP_FIELD,
    &LIF_PON_INTER_OP_CONTROL_CFTXCRC8MSB2LSB_FIELD,
    &LIF_PON_INTER_OP_CONTROL_CFTXSHORTPRE_FIELD,
    &LIF_PON_INTER_OP_CONTROL_CFTXIPGCNT_FIELD,
    &LIF_PON_INTER_OP_CONTROL_CFTXAASYNCLEN_FIELD,
    &LIF_PON_INTER_OP_CONTROL_CFTXPIPEDELAY_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_PON_INTER_OP_CONTROL_REG = 
{
    "PON_INTER_OP_CONTROL",
#if RU_INCLUDE_DESC
    "LIF_PON_INTER_OP_CONTROL Register",
    "This register controls and configures the LIF PON module specifically"
    "dealing with interoperability.",
#endif
    LIF_PON_INTER_OP_CONTROL_REG_OFFSET,
    0,
    0,
    274,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    20,
    LIF_PON_INTER_OP_CONTROL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_FEC_CONTROL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_FEC_CONTROL_FIELDS[] =
{
    &LIF_FEC_CONTROL_RESERVED0_FIELD,
    &LIF_FEC_CONTROL_CFFECRXERRORPROP_FIELD,
    &LIF_FEC_CONTROL_CFFECRXFORCENONFECABORT_FIELD,
    &LIF_FEC_CONTROL_CFFECRXFORCEFECABORT_FIELD,
    &LIF_FEC_CONTROL_CFFECRXENABLE_FIELD,
    &LIF_FEC_CONTROL_CFFECTXFECPERLLID_FIELD,
    &LIF_FEC_CONTROL_CFFECTXENABLE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_FEC_CONTROL_REG = 
{
    "FEC_CONTROL",
#if RU_INCLUDE_DESC
    "LIF_FEC_CONTROL Register",
    "This register controls and configures the LIF FEC sub-module block."
    "Configuration bits dealing directly with forward error correction are"
    "here.",
#endif
    LIF_FEC_CONTROL_REG_OFFSET,
    0,
    0,
    275,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    LIF_FEC_CONTROL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_SEC_CONTROL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_SEC_CONTROL_FIELDS[] =
{
    &LIF_SEC_CONTROL_CFGDISMPCPENCRYPT_FIELD,
    &LIF_SEC_CONTROL_CFGDISOAMENCRYPT_FIELD,
    &LIF_SEC_CONTROL_CFGSECENSHORTLEN_FIELD,
    &LIF_SEC_CONTROL_CFGSECDNENPKTNUMRLOVR_FIELD,
    &LIF_SEC_CONTROL_CFGSECUPENPKTNUMRLOVR_FIELD,
    &LIF_SEC_CONTROL_CFGENAEREPLAYPRCT_FIELD,
    &LIF_SEC_CONTROL_CFGENLEGACYRCC_FIELD,
    &LIF_SEC_CONTROL_ENFAKEUPAES_FIELD,
    &LIF_SEC_CONTROL_ENFAKEDNAES_FIELD,
    &LIF_SEC_CONTROL_RESERVED0_FIELD,
    &LIF_SEC_CONTROL_CFGFECIPGLEN_FIELD,
    &LIF_SEC_CONTROL_DISDNDASAENCRPT_FIELD,
    &LIF_SEC_CONTROL_ENTRIPLECHURN_FIELD,
    &LIF_SEC_CONTROL_ENEPNMIXENCRYPT_FIELD,
    &LIF_SEC_CONTROL_DISUPDASAENCRPT_FIELD,
    &LIF_SEC_CONTROL_SECUPENCRYPTSCHEME_FIELD,
    &LIF_SEC_CONTROL_SECDNENCRYPTSCHEME_FIELD,
    &LIF_SEC_CONTROL_SECUPRSTN_PRE_FIELD,
    &LIF_SEC_CONTROL_SECDNRSTN_PRE_FIELD,
    &LIF_SEC_CONTROL_SECENUP_FIELD,
    &LIF_SEC_CONTROL_SECENDN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_SEC_CONTROL_REG = 
{
    "SEC_CONTROL",
#if RU_INCLUDE_DESC
    "LIF_SEC_CONTROL Register",
    "This register controls and configures the LIF SEC sub-module block."
    "Configuration bits dealing directly with security encryption are here.",
#endif
    LIF_SEC_CONTROL_REG_OFFSET,
    0,
    0,
    276,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    21,
    LIF_SEC_CONTROL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_MACSEC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_MACSEC_FIELDS[] =
{
    &LIF_MACSEC_RESERVED0_FIELD,
    &LIF_MACSEC_CFGMACSECETHERTYPE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_MACSEC_REG = 
{
    "MACSEC",
#if RU_INCLUDE_DESC
    "LIF_MACSEC Register",
    "This register specifies the 802.1ae MacSec Ethertype to be inserted"
    "into the packet.",
#endif
    LIF_MACSEC_REG_OFFSET,
    0,
    0,
    277,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_MACSEC_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_INT_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_INT_STATUS_FIELDS[] =
{
    &LIF_INT_STATUS_RESERVED0_FIELD,
    &LIF_INT_STATUS_SGMIIABORTFRINT_FIELD,
    &LIF_INT_STATUS_INT_SOP_SFEC_IPG_VIOLATION_FIELD,
    &LIF_INT_STATUS_LASERONMAX_FIELD,
    &LIF_INT_STATUS_LASEROFF_FIELD,
    &LIF_INT_STATUS_SECDNREPLAYPROTCTABORT_FIELD,
    &LIF_INT_STATUS_SECUPPKTNUMOVERFLOW_FIELD,
    &LIF_INT_STATUS_INTLASEROFFDURBURST_FIELD,
    &LIF_INT_STATUS_INTRXBERTHRESHEXC_FIELD,
    &LIF_INT_STATUS_INTFECRXFECRECVSTATUS_FIELD,
    &LIF_INT_STATUS_INTFECRXCORERRFIFOFULLSTATUS_FIELD,
    &LIF_INT_STATUS_INTFECRXCORERRFIFOUNEXPEMPTY_FIELD,
    &LIF_INT_STATUS_INTFECBUFPOPEMPTYPUSH_FIELD,
    &LIF_INT_STATUS_INTFECBUFPOPEMPTYNOPUSH_FIELD,
    &LIF_INT_STATUS_INTFECBUFPUSHFULL_FIELD,
    &LIF_INT_STATUS_INTUPTIMEFULLUPDSTAT_FIELD,
    &LIF_INT_STATUS_INTFROUTOFALIGNSTAT_FIELD,
    &LIF_INT_STATUS_INTGRNTSTARTTIMELAGSTAT_FIELD,
    &LIF_INT_STATUS_INTABORTRXFRMSTAT_FIELD,
    &LIF_INT_STATUS_INTNORXCLKSTAT_FIELD,
    &LIF_INT_STATUS_INTRXMAXLENERRSTAT_FIELD,
    &LIF_INT_STATUS_INTRXERRAFTALIGNSTAT_FIELD,
    &LIF_INT_STATUS_INTRXSYNCHACQSTAT_FIELD,
    &LIF_INT_STATUS_INTRXOUTOFSYNCHSTAT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_INT_STATUS_REG = 
{
    "INT_STATUS",
#if RU_INCLUDE_DESC
    "LIF_INT_STATUS Register",
    "This register contains interrupt status for LIF modules."
    "These bits are sticky; to clear a bit, write 1 to it.",
#endif
    LIF_INT_STATUS_REG_OFFSET,
    0,
    0,
    278,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    24,
    LIF_INT_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_INT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_INT_MASK_FIELDS[] =
{
    &LIF_INT_MASK_RESERVED0_FIELD,
    &LIF_INT_MASK_SGMIIABORTFRINTMASK_FIELD,
    &LIF_INT_MASK_INT_SOP_SFEC_IPG_VIOLATION_MASK_FIELD,
    &LIF_INT_MASK_LASERONMAXMASK_FIELD,
    &LIF_INT_MASK_LASEROFFMASK_FIELD,
    &LIF_INT_MASK_SECDNREPLAYPROTCTABORTMSK_FIELD,
    &LIF_INT_MASK_SECUPPKTNUMOVERFLOWMSK_FIELD,
    &LIF_INT_MASK_INTLASEROFFDURBURSTMASK_FIELD,
    &LIF_INT_MASK_INTRXBERTHRESHEXCMASK_FIELD,
    &LIF_INT_MASK_INTFECRXFECRECVMASK_FIELD,
    &LIF_INT_MASK_INTFECRXCORERRFIFOFULLMASK_FIELD,
    &LIF_INT_MASK_INTFECRXCORERRFIFOUNEXPEMPTYMASK_FIELD,
    &LIF_INT_MASK_INTFECBUFPOPEMPTYPUSHMASK_FIELD,
    &LIF_INT_MASK_INTFECBUFPOPEMPTYNOPUSHMASK_FIELD,
    &LIF_INT_MASK_INTFECBUFPUSHFULLMASK_FIELD,
    &LIF_INT_MASK_INTUPTIMEFULLUPDMASK_FIELD,
    &LIF_INT_MASK_INTFROUTOFALIGNMASK_FIELD,
    &LIF_INT_MASK_INTGRNTSTARTTIMELAGMASK_FIELD,
    &LIF_INT_MASK_INTABORTRXFRMMASK_FIELD,
    &LIF_INT_MASK_INTNORXCLKMASK_FIELD,
    &LIF_INT_MASK_INTRXMAXLENERRMASK_FIELD,
    &LIF_INT_MASK_INTRXERRAFTALIGNMASK_FIELD,
    &LIF_INT_MASK_INTRXSYNCHACQMASK_FIELD,
    &LIF_INT_MASK_INTRXOUTOFSYNCHMASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_INT_MASK_REG = 
{
    "INT_MASK",
#if RU_INCLUDE_DESC
    "LIF_INT_MASK Register",
    "This register contains interrupt masks for LIF modules.",
#endif
    LIF_INT_MASK_REG_OFFSET,
    0,
    0,
    279,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    24,
    LIF_INT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_DATA_PORT_COMMAND
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_DATA_PORT_COMMAND_FIELDS[] =
{
    &LIF_DATA_PORT_COMMAND_DATA_PORT_BUSY_FIELD,
    &LIF_DATA_PORT_COMMAND_DATA_PORT_ERROR_FIELD,
    &LIF_DATA_PORT_COMMAND_RAM_SELECT_FIELD,
    &LIF_DATA_PORT_COMMAND_DATA_PORT_OP_CODE_FIELD,
    &LIF_DATA_PORT_COMMAND_DATA_PORT_ADDR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_DATA_PORT_COMMAND_REG = 
{
    "DATA_PORT_COMMAND",
#if RU_INCLUDE_DESC
    "LIF_DATA_PORT_COMMAND Register",
    "This set of registers allows reading the LIF per-LLID statistics. It"
    "also allows general access to the LIF internal RAMs. All RAMs are"
    "initialized to zero upon hardware reset. The following describes data"
    "port read/write sequences:"
    ""
    "Write cycle:"
    "1) Write the address of the RAM entry to Offset 1."
    "2) Write data to be written to Offsets 2 through 5 (as required"
    "for the given RAM's width)."
    "3) Write to Offset 0 to select the RAM and to indicate Write"
    "cycle."
    "4) Poll on Offset 0 until the dataPortBusy bit is cleared."
    ""
    "Read cycle:"
    "1) Write the address of the RAM entry to Offset 1."
    "2) Write to Offset 0 to select the RAM and to indicate Read cycle."
    "3) Poll on Offset 0 until the dataPortBusy bit is cleared."
    "4) Read Offsets 2 through 5 (as needed according to the width of"
    "the accessed RAM) to retrieve the RAM read data."
    ""
    "The following paragraphs describe the port data format for each RAM:"
    ""
    "Statistics Downstream Per LLID - Stores the statistics information for"
    "each downstream LLID (32 bidirectional)."
    "The memory is auto-initialized within the hardware, and can be accessed"
    "as soon as the module is brought out of reset. The statistics are"
    "cleared on read."
    "portData0[31:0] - Desired statistic"
    ""
    "LLID Index     RAM Locations"
    "Bidirectional 0     0  - 15"
    "Bidirectional 1     16 - 31"
    "..."
    "Bidirectional 30    480-495"
    "Bidirectional 31    496-511"
    ""
    "Table:"
    "RAM Offset Description Remarks"
    "0 Bidir Downstream Good Frames Received Not Aborted Prior to"
    "FEC"
    "1 Bidir Downstream Good Bytes Received Not Aborted Prior to"
    "FEC"
    "2 Bidir Downstream Oversized Frames As defined by LIF"
    "Sanitizer Register (0x134)"
    "3 Bidir Downstream NonFEC Good Frames Received NonFEC Frames"
    "regardless of FEC Enable"
    "4 Bidir Downstream NonFEC Good Bytes Received NonFEC Bytes"
    "regardless of FEC Enable"
    "5 Bidir Downstream FEC Good Frames Received Will increment"
    "even if FEC is disabled."
    "6 Bidir Downstream FEC Good Bytes Received Will increment"
    "even if FEC is disabled."
    "7 Bidir Downstream FEC Frames Exceeded Error Threshold"
    "8 Bidir Downstream FEC Data Blocks No Errors"
    "9 Bidir Downstream FEC Data Blocks Corrected"
    "10 Bidir Downstream FEC Data Blocks Uncorrected"
    "11 Bidir Downstream FEC Data Corrected Bytes"
    "12 Bidir Downstream FEC Data Corrected Zeroes"
    "13 Bidir Downstream FEC Data Corrected Ones"
    "14 Bidir Downstream Undersized Frames"
    "15 Bidir Downstream Errored Frames"
    ""
    "SEC Downstream Key RAM - specifies the security key. When writing to"
    "the key RAM, the mode and encryption key MUST be specified. The"
    "even/odd keys must be written for key switching. For LLID X, the"
    "corresponding RAM offset for the even key is (X*2), and the odd key is"
    "(X*2)+1. For AES256, the number of supported LLID is reduced by half."
    "The even key is at (X*4); and odd key, (X*4)+2."
    ""
    "CEPON encryption mode:"
    "portData0 - Specifies input to 1st churning key generation."
    "Bits [15:0] specify P16 - P1; and bits [23:16] specify X8 - X1."
    "portData1 - Specifies input to 2nd churning key generation."
    "The input is byte shift of 1st churning key. Bits [7:0]"
    "specify P16 - P8; bits [15:8] specify X8 - X1; and"
    "bits [23:16] specify P7 - P1."
    "portData2 - Specifies input to 3rd churning key generation."
    "The input is byte shift of 2nd churning key. Bits [7:0] specify X8 -"
    "X1; and bits [23:8] specify P16 - P1."
    "portData3 - Specifies P-input to churning function. Bits [15:0]"
    "specify P16 - P1. Same data as portData0."
    ""
    "NTT encryption modes"
    "portData0 - Specifies bits [31:0] of the 128-bits Encryption key."
    "portData1 - Specifies bits [63:32] of the 128-bits Encryption key."
    "portData2 - Specifies bits [95:64] of the 128-bits Encryption key."
    "portData3 - Specifies bits [127:96] of the 128-bits Encryption key."
    ""
    "802.1ae encryption modes"
    "portData0    - Specifies bits [31:0] of the 128/256 bits Encryption"
    "key."
    "portData1    - Specifies bits [63:32] of the 128/256 bits Encryption"
    "key."
    "portData2    - Specifies bits [95:64] of the 128/256 bits Encryption"
    "key."
    "portData3    - Specifies bits [127:96] of the 128/256 bits Encryption"
    "key."
    "portData4    - Specifies bits [31:0] of the 64-bits implicit SCI"
    "portData5    - Specifies bits [63:32] of the 64-bits implicit SCI"
    "portData6    - Specifies bits [31:0] of the initial packet number for"
    "replay protection."
    "portData7[0] - encryption enable.  When cleared, 802.1ae packets will"
    "pass through undecrypted."
    "portData8    - Specifies bits [159:128] of the 128/256 bits Encryption"
    "key."
    "portData9    - Specifies bits [191:160] of the 128/256 bits Encryption"
    "key."
    "portData10   - Specifies bits [223:192] of the 128/256 bits Encryption"
    "key."
    "portData11   - Specifies bits [255:224] of the 128/256 bits Encryption"
    "key."
    ""
    "Zero-ovehead encryption modes"
    "portData0    - Specifies bits [31:0] of the 128/256 bits Encryption"
    "key."
    "portData1    - Specifies bits [63:32] of the 128/256 bits Encryption"
    "key."
    "portData2    - Specifies bits [95:64] of the 128/256 bits Encryption"
    "key."
    "portData3    - Specifies bits [127:96] of the 128/256 bits Encryption"
    "key."
    "portData4    - Specifies bits [31:0] of the 64-bits implicit SCI"
    "portData5    - Specifies bits [63:32] of the 64-bits implicit SCI"
    "portData6    - Specifies bits [31:0] of the initial packet number for"
    "replay protection."
    "portData8    - Specifies bits [159:128] of the 128/256 bits Encryption"
    "key."
    "portData9    - Specifies bits [191:160] of the 128/256 bits Encryption"
    "key."
    "portData10   - Specifies bits [223:192] of the 128/256 bits Encryption"
    "key."
    "portData11   - Specifies bits [255:224] of the 128/256 bits Encryption"
    "key."
    ""
    ""
    "SEC Upstream Key RAM - specifies the security key. When writing to the"
    "key RAM, the mode and encryption key MUST be specified. For AES256, the"
    "number of supported LLID is reduced by half.  For LLID X, the"
    "corresponding RAM offset is (X*2)."
    ""
    "NTT encryption modes"
    "portData0    - Specifies bits [31:0] of the 128-bits Encryption key."
    "portData1    - Specifies bits [63:32] of the 128-bits Encryption key."
    "portData2    - Specifies bits [95:64] of the 128- bits Encryption key."
    "portData3    - Specifies bits [127:96] of the 128-bits Encryption key."
    "portData7[8] - key number."
    "portData7[9] - encryption enable."
    ""
    "802.1ae encryption modes"
    "portData0      - Specifies bits [31:0] of the 128/256 bits Encryption"
    "key."
    "portData1      - Specifies bits [63:32] of the 128/256 bits Encryption"
    "key."
    "portData2      - Specifies bits [95:64] of the 128/256 bits Encryption"
    "key."
    "portData3      - Specifies bits [127:96] of the 128/256 bits Encryption"
    "key."
    "portData4      - Specifies bits [31:0] of the 64-bits explicit SCI."
    "portData5      - Specifies bits [63:32] of the 64-bits explicit SCI."
    "portData6      - Specifies bits [31:0] of the initial packet number."
    "portData7[7:0] - TCI[7:0]"
    "TCI[1:0]     - key number.  In PON mode, only TCI[0] is utilized for"
    "even/odd key."
    "Must match key number specified in portData7[8]. In"
    "P2P mode, TCI[1:0]"
    "specifies 1 of 4 keys."
    "TCI[3:2]     - encryption mode bits : TCI[3] - E encryption bit;"
    "TCI[2] - C change bit."
    "- E=0; C=0 : Authentication only.  Data is not"
    "encryption."
    "Only ICV is inserted at the end of packet."
    "- E=0; C=1 : Reserved."
    "- E=1; C=0 : Reserved."
    "- E=1; C=1 : Encryption/authentication. Data is"
    "encrypted and ICV inserted."
    "TCI[4]       - single copy broadcast.  Set to 0."
    "TCI[5]       - SC specifies whether SecTag's SCI is"
    "implicit(0)/explicit(1)."
    "TCI[6]       - ES end station byte.  Set to 0."
    "TCI[7]       - V version number.  Set to 0."
    "portData7[8]   - key number."
    "portData7[9]   - encryption enable."
    "portData8      - Specifies bits [159:128] of the 128/256 bits"
    "Encryption key."
    "portData9      - Specifies bits [191:160] of the 128/256 bits"
    "Encryption key."
    "portData10     - Specifies bits [223:192] of the 128/256 bits"
    "Encryption key."
    "portData11     - Specifies bits [255:224] of the 128/256 bits"
    "Encryption key."
    ""
    "Zero-ovehead encryption modes"
    "portData0    - Specifies bits [31:0] of the 128/256 bits Encryption"
    "key."
    "portData1    - Specifies bits [63:32] of the 128/256 bits Encryption"
    "key."
    "portData2    - Specifies bits [95:64] of the 128/256 bits Encryption"
    "key."
    "portData3    - Specifies bits [127:96] of the 128/256 bits Encryption"
    "key."
    "portData4    - Specifies bits [31:0] of the 64-bits implicit SCI."
    "portData5    - Specifies bits [63:32] of the 64-bits implicit SCI."
    "portData6    - Specifies bits [31:0] of the initial packet number."
    "portData7[8] - key number."
    "portData7[9] - encryption enable."
    "portData8    - Specifies bits [159:128] of the 128/256 bits Encryption"
    "key."
    "portData9    - Specifies bits [191:160] of the 128/256 bits Encryption"
    "key."
    "portData10   - Specifies bits [223:192] of the 128/256 bits Encryption"
    "key."
    "portData11   - Specifies bits [255:224] of the 128/256 bits Encryption"
    "key.",
#endif
    LIF_DATA_PORT_COMMAND_REG_OFFSET,
    0,
    0,
    280,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    LIF_DATA_PORT_COMMAND_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_DATA_PORT_DATA_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_DATA_PORT_DATA_0_FIELDS[] =
{
    &LIF_DATA_PORT_DATA_0_PBIPORTDATA0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_DATA_PORT_DATA_0_REG = 
{
    "DATA_PORT_DATA_0",
#if RU_INCLUDE_DESC
    "LIF_DATA_PORT_DATA_0 Register",
    "",
#endif
    LIF_DATA_PORT_DATA_0_REG_OFFSET,
    0,
    0,
    281,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_DATA_PORT_DATA_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_DATA_PORT_DATA_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_DATA_PORT_DATA_1_FIELDS[] =
{
    &LIF_DATA_PORT_DATA_1_PBIPORTDATA1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_DATA_PORT_DATA_1_REG = 
{
    "DATA_PORT_DATA_1",
#if RU_INCLUDE_DESC
    "LIF_DATA_PORT_DATA_1 Register",
    "",
#endif
    LIF_DATA_PORT_DATA_1_REG_OFFSET,
    0,
    0,
    282,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_DATA_PORT_DATA_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_DATA_PORT_DATA_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_DATA_PORT_DATA_2_FIELDS[] =
{
    &LIF_DATA_PORT_DATA_2_PBIPORTDATA2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_DATA_PORT_DATA_2_REG = 
{
    "DATA_PORT_DATA_2",
#if RU_INCLUDE_DESC
    "LIF_DATA_PORT_DATA_2 Register",
    "",
#endif
    LIF_DATA_PORT_DATA_2_REG_OFFSET,
    0,
    0,
    283,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_DATA_PORT_DATA_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_DATA_PORT_DATA_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_DATA_PORT_DATA_3_FIELDS[] =
{
    &LIF_DATA_PORT_DATA_3_PBIPORTDATA3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_DATA_PORT_DATA_3_REG = 
{
    "DATA_PORT_DATA_3",
#if RU_INCLUDE_DESC
    "LIF_DATA_PORT_DATA_3 Register",
    "",
#endif
    LIF_DATA_PORT_DATA_3_REG_OFFSET,
    0,
    0,
    284,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_DATA_PORT_DATA_3_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_DATA_PORT_DATA_4
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_DATA_PORT_DATA_4_FIELDS[] =
{
    &LIF_DATA_PORT_DATA_4_PBIPORTDATA4_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_DATA_PORT_DATA_4_REG = 
{
    "DATA_PORT_DATA_4",
#if RU_INCLUDE_DESC
    "LIF_DATA_PORT_DATA_4 Register",
    "",
#endif
    LIF_DATA_PORT_DATA_4_REG_OFFSET,
    0,
    0,
    285,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_DATA_PORT_DATA_4_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_DATA_PORT_DATA_5
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_DATA_PORT_DATA_5_FIELDS[] =
{
    &LIF_DATA_PORT_DATA_5_PBIPORTDATA5_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_DATA_PORT_DATA_5_REG = 
{
    "DATA_PORT_DATA_5",
#if RU_INCLUDE_DESC
    "LIF_DATA_PORT_DATA_5 Register",
    "",
#endif
    LIF_DATA_PORT_DATA_5_REG_OFFSET,
    0,
    0,
    286,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_DATA_PORT_DATA_5_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_DATA_PORT_DATA_6
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_DATA_PORT_DATA_6_FIELDS[] =
{
    &LIF_DATA_PORT_DATA_6_PBIPORTDATA6_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_DATA_PORT_DATA_6_REG = 
{
    "DATA_PORT_DATA_6",
#if RU_INCLUDE_DESC
    "LIF_DATA_PORT_DATA_6 Register",
    "",
#endif
    LIF_DATA_PORT_DATA_6_REG_OFFSET,
    0,
    0,
    287,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_DATA_PORT_DATA_6_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_DATA_PORT_DATA_7
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_DATA_PORT_DATA_7_FIELDS[] =
{
    &LIF_DATA_PORT_DATA_7_PBIPORTDATA7_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_DATA_PORT_DATA_7_REG = 
{
    "DATA_PORT_DATA_7",
#if RU_INCLUDE_DESC
    "LIF_DATA_PORT_DATA_7 Register",
    "",
#endif
    LIF_DATA_PORT_DATA_7_REG_OFFSET,
    0,
    0,
    288,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_DATA_PORT_DATA_7_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_0_FIELDS[] =
{
    &LIF_LLID_0_RESERVED0_FIELD,
    &LIF_LLID_0_CFGLLID0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_0_REG = 
{
    "LLID_0",
#if RU_INCLUDE_DESC
    "LIF_LLID_0 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_0_REG_OFFSET,
    0,
    0,
    289,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_1_FIELDS[] =
{
    &LIF_LLID_1_RESERVED0_FIELD,
    &LIF_LLID_1_CFGLLID1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_1_REG = 
{
    "LLID_1",
#if RU_INCLUDE_DESC
    "LIF_LLID_1 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_1_REG_OFFSET,
    0,
    0,
    290,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_2_FIELDS[] =
{
    &LIF_LLID_2_RESERVED0_FIELD,
    &LIF_LLID_2_CFGLLID2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_2_REG = 
{
    "LLID_2",
#if RU_INCLUDE_DESC
    "LIF_LLID_2 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_2_REG_OFFSET,
    0,
    0,
    291,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_3_FIELDS[] =
{
    &LIF_LLID_3_RESERVED0_FIELD,
    &LIF_LLID_3_CFGLLID3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_3_REG = 
{
    "LLID_3",
#if RU_INCLUDE_DESC
    "LIF_LLID_3 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_3_REG_OFFSET,
    0,
    0,
    292,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_3_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_4
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_4_FIELDS[] =
{
    &LIF_LLID_4_RESERVED0_FIELD,
    &LIF_LLID_4_CFGLLID4_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_4_REG = 
{
    "LLID_4",
#if RU_INCLUDE_DESC
    "LIF_LLID_4 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_4_REG_OFFSET,
    0,
    0,
    293,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_4_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_5
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_5_FIELDS[] =
{
    &LIF_LLID_5_RESERVED0_FIELD,
    &LIF_LLID_5_CFGLLID5_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_5_REG = 
{
    "LLID_5",
#if RU_INCLUDE_DESC
    "LIF_LLID_5 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_5_REG_OFFSET,
    0,
    0,
    294,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_5_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_6
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_6_FIELDS[] =
{
    &LIF_LLID_6_RESERVED0_FIELD,
    &LIF_LLID_6_CFGLLID6_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_6_REG = 
{
    "LLID_6",
#if RU_INCLUDE_DESC
    "LIF_LLID_6 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_6_REG_OFFSET,
    0,
    0,
    295,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_6_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_7
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_7_FIELDS[] =
{
    &LIF_LLID_7_RESERVED0_FIELD,
    &LIF_LLID_7_CFGLLID7_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_7_REG = 
{
    "LLID_7",
#if RU_INCLUDE_DESC
    "LIF_LLID_7 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_7_REG_OFFSET,
    0,
    0,
    296,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_7_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_16
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_16_FIELDS[] =
{
    &LIF_LLID_16_RESERVED0_FIELD,
    &LIF_LLID_16_CFGLLID16_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_16_REG = 
{
    "LLID_16",
#if RU_INCLUDE_DESC
    "LIF_LLID_16 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_16_REG_OFFSET,
    0,
    0,
    297,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_16_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_17
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_17_FIELDS[] =
{
    &LIF_LLID_17_RESERVED0_FIELD,
    &LIF_LLID_17_CFGLLID17_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_17_REG = 
{
    "LLID_17",
#if RU_INCLUDE_DESC
    "LIF_LLID_17 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_17_REG_OFFSET,
    0,
    0,
    298,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_17_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_18
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_18_FIELDS[] =
{
    &LIF_LLID_18_RESERVED0_FIELD,
    &LIF_LLID_18_CFGLLID18_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_18_REG = 
{
    "LLID_18",
#if RU_INCLUDE_DESC
    "LIF_LLID_18 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_18_REG_OFFSET,
    0,
    0,
    299,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_18_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_19
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_19_FIELDS[] =
{
    &LIF_LLID_19_RESERVED0_FIELD,
    &LIF_LLID_19_CFGLLID19_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_19_REG = 
{
    "LLID_19",
#if RU_INCLUDE_DESC
    "LIF_LLID_19 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_19_REG_OFFSET,
    0,
    0,
    300,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_19_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_20
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_20_FIELDS[] =
{
    &LIF_LLID_20_RESERVED0_FIELD,
    &LIF_LLID_20_CFGLLID20_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_20_REG = 
{
    "LLID_20",
#if RU_INCLUDE_DESC
    "LIF_LLID_20 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_20_REG_OFFSET,
    0,
    0,
    301,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_20_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_21
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_21_FIELDS[] =
{
    &LIF_LLID_21_RESERVED0_FIELD,
    &LIF_LLID_21_CFGLLID21_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_21_REG = 
{
    "LLID_21",
#if RU_INCLUDE_DESC
    "LIF_LLID_21 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_21_REG_OFFSET,
    0,
    0,
    302,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_21_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_22
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_22_FIELDS[] =
{
    &LIF_LLID_22_RESERVED0_FIELD,
    &LIF_LLID_22_CFGLLID22_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_22_REG = 
{
    "LLID_22",
#if RU_INCLUDE_DESC
    "LIF_LLID_22 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_22_REG_OFFSET,
    0,
    0,
    303,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_22_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_23
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_23_FIELDS[] =
{
    &LIF_LLID_23_RESERVED0_FIELD,
    &LIF_LLID_23_CFGLLID23_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_23_REG = 
{
    "LLID_23",
#if RU_INCLUDE_DESC
    "LIF_LLID_23 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_23_REG_OFFSET,
    0,
    0,
    304,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_23_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_TIME_REF_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_TIME_REF_CNT_FIELDS[] =
{
    &LIF_TIME_REF_CNT_RESERVED0_FIELD,
    &LIF_TIME_REF_CNT_CFFULLUPDATEVALUE_FIELD,
    &LIF_TIME_REF_CNT_CFMAXNEGVALUE_FIELD,
    &LIF_TIME_REF_CNT_CFMAXPOSVALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_TIME_REF_CNT_REG = 
{
    "TIME_REF_CNT",
#if RU_INCLUDE_DESC
    "LIF_TIME_REF_CNT Register",
    "This register provides programmable parameters for dynamic updates to"
    "the MPCP timer.",
#endif
    LIF_TIME_REF_CNT_REG_OFFSET,
    0,
    0,
    305,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    LIF_TIME_REF_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_TIMESTAMP_UPD_PER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_TIMESTAMP_UPD_PER_FIELDS[] =
{
    &LIF_TIMESTAMP_UPD_PER_RESERVED0_FIELD,
    &LIF_TIMESTAMP_UPD_PER_CFTIMESTAMPUPDPER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_TIMESTAMP_UPD_PER_REG = 
{
    "TIMESTAMP_UPD_PER",
#if RU_INCLUDE_DESC
    "LIF_TIMESTAMP_UPD_PER Register",
    "This register provides the LIF the ability to filter MPCP time"
    "corrections when the EPON MAC requests them too frequently. This"
    "register specifies a time period after an update during which the LIF"
    "will ignore MPCP time corrections from EPN.",
#endif
    LIF_TIMESTAMP_UPD_PER_REG_OFFSET,
    0,
    0,
    306,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_TIMESTAMP_UPD_PER_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_TP_TIME
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_TP_TIME_FIELDS[] =
{
    &LIF_TP_TIME_CFTRANSPORTTIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_TP_TIME_REG = 
{
    "TP_TIME",
#if RU_INCLUDE_DESC
    "LIF_TP_TIME Register",
    "The one pulse per second signal is asserted when the local MPCP time"
    "increments past the value programmed in cfTransportTime. Software must"
    "update this register once per second.",
#endif
    LIF_TP_TIME_REG_OFFSET,
    0,
    0,
    307,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_TP_TIME_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_MPCP_TIME
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_MPCP_TIME_FIELDS[] =
{
    &LIF_MPCP_TIME_LTMPCPTIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_MPCP_TIME_REG = 
{
    "MPCP_TIME",
#if RU_INCLUDE_DESC
    "LIF_MPCP_TIME Register",
    "Provides the receive MPCP time of the most recently received downstream"
    "packet. It is updated only when a downstream packet is received.",
#endif
    LIF_MPCP_TIME_REG_OFFSET,
    0,
    0,
    308,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_MPCP_TIME_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_MAXLEN_CTR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_MAXLEN_CTR_FIELDS[] =
{
    &LIF_MAXLEN_CTR_RESERVED0_FIELD,
    &LIF_MAXLEN_CTR_CFRXMAXFRAMELENGTH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_MAXLEN_CTR_REG = 
{
    "MAXLEN_CTR",
#if RU_INCLUDE_DESC
    "LIF_MAXLEN_CTR Register",
    "Max Length setting for the receive sanitizer. The sanitizer ensures"
    "that frames entering latter LIF receive logic are truncated to a"
    "useable size.",
#endif
    LIF_MAXLEN_CTR_REG_OFFSET,
    0,
    0,
    309,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_MAXLEN_CTR_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LASER_ON_DELTA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LASER_ON_DELTA_FIELDS[] =
{
    &LIF_LASER_ON_DELTA_RESERVED0_FIELD,
    &LIF_LASER_ON_DELTA_CFTXLASERONDELTA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LASER_ON_DELTA_REG = 
{
    "LASER_ON_DELTA",
#if RU_INCLUDE_DESC
    "LIF_LASER_ON_DELTA Register",
    "Specifies an offset, before or after the grant start time, to turn on"
    "the laser. Units are double-octet words.",
#endif
    LIF_LASER_ON_DELTA_REG_OFFSET,
    0,
    0,
    310,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LASER_ON_DELTA_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LASER_OFF_IDLE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LASER_OFF_IDLE_FIELDS[] =
{
    &LIF_LASER_OFF_IDLE_CFTXINITIDLE_FIELD,
    &LIF_LASER_OFF_IDLE_RESERVED0_FIELD,
    &LIF_LASER_OFF_IDLE_CFTXLASEROFFDELTA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LASER_OFF_IDLE_REG = 
{
    "LASER_OFF_IDLE",
#if RU_INCLUDE_DESC
    "LIF_LASER_OFF_IDLE Register",
    "Defines when to turn the laser off and the number of IDLE characters to"
    "transmit at the beginning of a grant of nonFEC frames. Units are"
    "double-octet words (TQ).",
#endif
    LIF_LASER_OFF_IDLE_REG_OFFSET,
    0,
    0,
    311,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LIF_LASER_OFF_IDLE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_FEC_INIT_IDLE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_FEC_INIT_IDLE_FIELDS[] =
{
    &LIF_FEC_INIT_IDLE_RESERVED0_FIELD,
    &LIF_FEC_INIT_IDLE_CFTXFECINITIDLE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_FEC_INIT_IDLE_REG = 
{
    "FEC_INIT_IDLE",
#if RU_INCLUDE_DESC
    "LIF_FEC_INIT_IDLE Register",
    "Defines the number of IDLE characters to transmit at the beginning of a"
    "grant of FEC frames. Units are double-octet words (TQ).",
#endif
    LIF_FEC_INIT_IDLE_REG_OFFSET,
    0,
    0,
    312,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_FEC_INIT_IDLE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_FEC_ERR_ALLOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_FEC_ERR_ALLOW_FIELDS[] =
{
    &LIF_FEC_ERR_ALLOW_RESERVED0_FIELD,
    &LIF_FEC_ERR_ALLOW_CFRXTFECBITERRALLOW_FIELD,
    &LIF_FEC_ERR_ALLOW_CFRXSFECBITERRALLOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_FEC_ERR_ALLOW_REG = 
{
    "FEC_ERR_ALLOW",
#if RU_INCLUDE_DESC
    "LIF_FEC_ERR_ALLOW Register",
    "Allowed hamming distance between received SFEC/TFEC and reference"
    "value.",
#endif
    LIF_FEC_ERR_ALLOW_REG_OFFSET,
    0,
    0,
    313,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LIF_FEC_ERR_ALLOW_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_SEC_KEY_SEL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_SEC_KEY_SEL_FIELDS[] =
{
    &LIF_SEC_KEY_SEL_KEYSEL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_SEC_KEY_SEL_REG = 
{
    "SEC_KEY_SEL",
#if RU_INCLUDE_DESC
    "LIF_SEC_KEY_SEL Register",
    "This register is a read-only status of the last downstream key selected"
    "on the 32 downstream LLIDs. This register allows for software to detect"
    "a key switchover or to determine the current downstream key."
    ""
    "In 802.1ae, it provides the lower bif of TCI's AN[1:0].  The mapping to"
    "even/odd key, based on AN[1:0], is provided by cfgKeyNum_4_AN_0 -"
    "cfgKeyNum_4_AN_3.",
#endif
    LIF_SEC_KEY_SEL_REG_OFFSET,
    0,
    0,
    314,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_SEC_KEY_SEL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_DN_ENCRYPT_STAT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_DN_ENCRYPT_STAT_FIELDS[] =
{
    &LIF_DN_ENCRYPT_STAT_ENENCRYPT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_DN_ENCRYPT_STAT_REG = 
{
    "DN_ENCRYPT_STAT",
#if RU_INCLUDE_DESC
    "LIF_DN_ENCRYPT_STAT Register",
    "Provides per-LLID status of downstream encryption."
    "Clear a bit (disable encryption on an LLID) by writing 1 to it.",
#endif
    LIF_DN_ENCRYPT_STAT_REG_OFFSET,
    0,
    0,
    315,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_DN_ENCRYPT_STAT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_SEC_UP_KEY_STAT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_SEC_UP_KEY_STAT_FIELDS[] =
{
    &LIF_SEC_UP_KEY_STAT_KEYUPSEL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_SEC_UP_KEY_STAT_REG = 
{
    "SEC_UP_KEY_STAT",
#if RU_INCLUDE_DESC
    "LIF_SEC_UP_KEY_STAT Register",
    "Provides per-LLID status of upstream security key.",
#endif
    LIF_SEC_UP_KEY_STAT_REG_OFFSET,
    0,
    0,
    316,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_SEC_UP_KEY_STAT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_SEC_UP_ENCRYPT_STAT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_SEC_UP_ENCRYPT_STAT_FIELDS[] =
{
    &LIF_SEC_UP_ENCRYPT_STAT_ENUPENCRYPT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_SEC_UP_ENCRYPT_STAT_REG = 
{
    "SEC_UP_ENCRYPT_STAT",
#if RU_INCLUDE_DESC
    "LIF_SEC_UP_ENCRYPT_STAT Register",
    "Provides per-LLID status of upstream security.",
#endif
    LIF_SEC_UP_ENCRYPT_STAT_REG_OFFSET,
    0,
    0,
    317,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_SEC_UP_ENCRYPT_STAT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_SEC_UP_MPCP_OFFSET
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_SEC_UP_MPCP_OFFSET_FIELDS[] =
{
    &LIF_SEC_UP_MPCP_OFFSET_SECUPMPCPOFFSET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_SEC_UP_MPCP_OFFSET_REG = 
{
    "SEC_UP_MPCP_OFFSET",
#if RU_INCLUDE_DESC
    "LIF_SEC_UP_MPCP_OFFSET Register",
    "Provides MPCP correction for EPON encryption.",
#endif
    LIF_SEC_UP_MPCP_OFFSET_REG_OFFSET,
    0,
    0,
    318,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_SEC_UP_MPCP_OFFSET_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_FEC_PER_LLID
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_FEC_PER_LLID_FIELDS[] =
{
    &LIF_FEC_PER_LLID_CFFECTXFECENLLID_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_FEC_PER_LLID_REG = 
{
    "FEC_PER_LLID",
#if RU_INCLUDE_DESC
    "LIF_FEC_PER_LLID Register",
    "Provides upstream per LLID FEC enabling.",
#endif
    LIF_FEC_PER_LLID_REG_OFFSET,
    0,
    0,
    319,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_FEC_PER_LLID_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_RX_LINE_CODE_ERR_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_RX_LINE_CODE_ERR_CNT_FIELDS[] =
{
    &LIF_RX_LINE_CODE_ERR_CNT_RXLINECODEERRCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_RX_LINE_CODE_ERR_CNT_REG = 
{
    "RX_LINE_CODE_ERR_CNT",
#if RU_INCLUDE_DESC
    "LIF_RX_LINE_CODE_ERR_CNT Register",
    "Aggregate statistics for the LIF receive channel. These registers"
    "saturate at their malifum and clear when read."
    "Note: These registers are also writable for test/diagnostics purposes.",
#endif
    LIF_RX_LINE_CODE_ERR_CNT_REG_OFFSET,
    0,
    0,
    320,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_RX_LINE_CODE_ERR_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_RX_AGG_MPCP_FRM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_RX_AGG_MPCP_FRM_FIELDS[] =
{
    &LIF_RX_AGG_MPCP_FRM_RXAGGMPCPCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_RX_AGG_MPCP_FRM_REG = 
{
    "RX_AGG_MPCP_FRM",
#if RU_INCLUDE_DESC
    "LIF_RX_AGG_MPCP_FRM Register",
    "",
#endif
    LIF_RX_AGG_MPCP_FRM_REG_OFFSET,
    0,
    0,
    321,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_RX_AGG_MPCP_FRM_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_RX_AGG_GOOD_FRM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_RX_AGG_GOOD_FRM_FIELDS[] =
{
    &LIF_RX_AGG_GOOD_FRM_RXAGGGOODCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_RX_AGG_GOOD_FRM_REG = 
{
    "RX_AGG_GOOD_FRM",
#if RU_INCLUDE_DESC
    "LIF_RX_AGG_GOOD_FRM Register",
    "",
#endif
    LIF_RX_AGG_GOOD_FRM_REG_OFFSET,
    0,
    0,
    322,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_RX_AGG_GOOD_FRM_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_RX_AGG_GOOD_BYTE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_RX_AGG_GOOD_BYTE_FIELDS[] =
{
    &LIF_RX_AGG_GOOD_BYTE_RXAGGGOODBYTESCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_RX_AGG_GOOD_BYTE_REG = 
{
    "RX_AGG_GOOD_BYTE",
#if RU_INCLUDE_DESC
    "LIF_RX_AGG_GOOD_BYTE Register",
    "",
#endif
    LIF_RX_AGG_GOOD_BYTE_REG_OFFSET,
    0,
    0,
    323,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_RX_AGG_GOOD_BYTE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_RX_AGG_UNDERSZ_FRM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_RX_AGG_UNDERSZ_FRM_FIELDS[] =
{
    &LIF_RX_AGG_UNDERSZ_FRM_RXAGGUNDERSZCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_RX_AGG_UNDERSZ_FRM_REG = 
{
    "RX_AGG_UNDERSZ_FRM",
#if RU_INCLUDE_DESC
    "LIF_RX_AGG_UNDERSZ_FRM Register",
    "",
#endif
    LIF_RX_AGG_UNDERSZ_FRM_REG_OFFSET,
    0,
    0,
    324,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_RX_AGG_UNDERSZ_FRM_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_RX_AGG_OVERSZ_FRM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_RX_AGG_OVERSZ_FRM_FIELDS[] =
{
    &LIF_RX_AGG_OVERSZ_FRM_RXAGGOVERSZCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_RX_AGG_OVERSZ_FRM_REG = 
{
    "RX_AGG_OVERSZ_FRM",
#if RU_INCLUDE_DESC
    "LIF_RX_AGG_OVERSZ_FRM Register",
    "",
#endif
    LIF_RX_AGG_OVERSZ_FRM_REG_OFFSET,
    0,
    0,
    325,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_RX_AGG_OVERSZ_FRM_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_RX_AGG_CRC8_FRM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_RX_AGG_CRC8_FRM_FIELDS[] =
{
    &LIF_RX_AGG_CRC8_FRM_RXAGGCRC8ERRCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_RX_AGG_CRC8_FRM_REG = 
{
    "RX_AGG_CRC8_FRM",
#if RU_INCLUDE_DESC
    "LIF_RX_AGG_CRC8_FRM Register",
    "",
#endif
    LIF_RX_AGG_CRC8_FRM_REG_OFFSET,
    0,
    0,
    326,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_RX_AGG_CRC8_FRM_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_RX_AGG_FEC_FRM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_RX_AGG_FEC_FRM_FIELDS[] =
{
    &LIF_RX_AGG_FEC_FRM_RXAGGFEC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_RX_AGG_FEC_FRM_REG = 
{
    "RX_AGG_FEC_FRM",
#if RU_INCLUDE_DESC
    "LIF_RX_AGG_FEC_FRM Register",
    "",
#endif
    LIF_RX_AGG_FEC_FRM_REG_OFFSET,
    0,
    0,
    327,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_RX_AGG_FEC_FRM_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_RX_AGG_FEC_BYTE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_RX_AGG_FEC_BYTE_FIELDS[] =
{
    &LIF_RX_AGG_FEC_BYTE_RXAGGFECBYTES_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_RX_AGG_FEC_BYTE_REG = 
{
    "RX_AGG_FEC_BYTE",
#if RU_INCLUDE_DESC
    "LIF_RX_AGG_FEC_BYTE Register",
    "",
#endif
    LIF_RX_AGG_FEC_BYTE_REG_OFFSET,
    0,
    0,
    328,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_RX_AGG_FEC_BYTE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_RX_AGG_FEC_EXC_ERR_FRM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_RX_AGG_FEC_EXC_ERR_FRM_FIELDS[] =
{
    &LIF_RX_AGG_FEC_EXC_ERR_FRM_RXAGGFECEXCEEDERRS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_RX_AGG_FEC_EXC_ERR_FRM_REG = 
{
    "RX_AGG_FEC_EXC_ERR_FRM",
#if RU_INCLUDE_DESC
    "LIF_RX_AGG_FEC_EXC_ERR_FRM Register",
    "",
#endif
    LIF_RX_AGG_FEC_EXC_ERR_FRM_REG_OFFSET,
    0,
    0,
    329,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_RX_AGG_FEC_EXC_ERR_FRM_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_RX_AGG_NONFEC_GOOD_FRM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_RX_AGG_NONFEC_GOOD_FRM_FIELDS[] =
{
    &LIF_RX_AGG_NONFEC_GOOD_FRM_RXAGGNONFECGOOD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_RX_AGG_NONFEC_GOOD_FRM_REG = 
{
    "RX_AGG_NONFEC_GOOD_FRM",
#if RU_INCLUDE_DESC
    "LIF_RX_AGG_NONFEC_GOOD_FRM Register",
    "",
#endif
    LIF_RX_AGG_NONFEC_GOOD_FRM_REG_OFFSET,
    0,
    0,
    330,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_RX_AGG_NONFEC_GOOD_FRM_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_RX_AGG_NONFEC_GOOD_BYTE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_RX_AGG_NONFEC_GOOD_BYTE_FIELDS[] =
{
    &LIF_RX_AGG_NONFEC_GOOD_BYTE_RXAGGNONFECGOODBYTES_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_RX_AGG_NONFEC_GOOD_BYTE_REG = 
{
    "RX_AGG_NONFEC_GOOD_BYTE",
#if RU_INCLUDE_DESC
    "LIF_RX_AGG_NONFEC_GOOD_BYTE Register",
    "",
#endif
    LIF_RX_AGG_NONFEC_GOOD_BYTE_REG_OFFSET,
    0,
    0,
    331,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_RX_AGG_NONFEC_GOOD_BYTE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_RX_AGG_ERR_BYTES
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_RX_AGG_ERR_BYTES_FIELDS[] =
{
    &LIF_RX_AGG_ERR_BYTES_RXAGGERRBYTES_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_RX_AGG_ERR_BYTES_REG = 
{
    "RX_AGG_ERR_BYTES",
#if RU_INCLUDE_DESC
    "LIF_RX_AGG_ERR_BYTES Register",
    "",
#endif
    LIF_RX_AGG_ERR_BYTES_REG_OFFSET,
    0,
    0,
    332,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_RX_AGG_ERR_BYTES_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_RX_AGG_ERR_ZEROES
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_RX_AGG_ERR_ZEROES_FIELDS[] =
{
    &LIF_RX_AGG_ERR_ZEROES_RXAGGERRZEROES_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_RX_AGG_ERR_ZEROES_REG = 
{
    "RX_AGG_ERR_ZEROES",
#if RU_INCLUDE_DESC
    "LIF_RX_AGG_ERR_ZEROES Register",
    "",
#endif
    LIF_RX_AGG_ERR_ZEROES_REG_OFFSET,
    0,
    0,
    333,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_RX_AGG_ERR_ZEROES_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_RX_AGG_NO_ERR_BLKS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_RX_AGG_NO_ERR_BLKS_FIELDS[] =
{
    &LIF_RX_AGG_NO_ERR_BLKS_RXAGGNOERRBLKS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_RX_AGG_NO_ERR_BLKS_REG = 
{
    "RX_AGG_NO_ERR_BLKS",
#if RU_INCLUDE_DESC
    "LIF_RX_AGG_NO_ERR_BLKS Register",
    "",
#endif
    LIF_RX_AGG_NO_ERR_BLKS_REG_OFFSET,
    0,
    0,
    334,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_RX_AGG_NO_ERR_BLKS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_RX_AGG_COR_BLKS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_RX_AGG_COR_BLKS_FIELDS[] =
{
    &LIF_RX_AGG_COR_BLKS_RXAGGCORRBLKS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_RX_AGG_COR_BLKS_REG = 
{
    "RX_AGG_COR_BLKS",
#if RU_INCLUDE_DESC
    "LIF_RX_AGG_COR_BLKS Register",
    "",
#endif
    LIF_RX_AGG_COR_BLKS_REG_OFFSET,
    0,
    0,
    335,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_RX_AGG_COR_BLKS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_RX_AGG_UNCOR_BLKS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_RX_AGG_UNCOR_BLKS_FIELDS[] =
{
    &LIF_RX_AGG_UNCOR_BLKS_RXAGGUNCORRBLKS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_RX_AGG_UNCOR_BLKS_REG = 
{
    "RX_AGG_UNCOR_BLKS",
#if RU_INCLUDE_DESC
    "LIF_RX_AGG_UNCOR_BLKS Register",
    "",
#endif
    LIF_RX_AGG_UNCOR_BLKS_REG_OFFSET,
    0,
    0,
    336,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_RX_AGG_UNCOR_BLKS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_RX_AGG_ERR_ONES
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_RX_AGG_ERR_ONES_FIELDS[] =
{
    &LIF_RX_AGG_ERR_ONES_RXAGGERRONES_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_RX_AGG_ERR_ONES_REG = 
{
    "RX_AGG_ERR_ONES",
#if RU_INCLUDE_DESC
    "LIF_RX_AGG_ERR_ONES Register",
    "",
#endif
    LIF_RX_AGG_ERR_ONES_REG_OFFSET,
    0,
    0,
    337,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_RX_AGG_ERR_ONES_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_RX_AGG_ERR_FRM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_RX_AGG_ERR_FRM_FIELDS[] =
{
    &LIF_RX_AGG_ERR_FRM_RXAGGERROREDCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_RX_AGG_ERR_FRM_REG = 
{
    "RX_AGG_ERR_FRM",
#if RU_INCLUDE_DESC
    "LIF_RX_AGG_ERR_FRM Register",
    "",
#endif
    LIF_RX_AGG_ERR_FRM_REG_OFFSET,
    0,
    0,
    338,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_RX_AGG_ERR_FRM_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_TX_PKT_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_TX_PKT_CNT_FIELDS[] =
{
    &LIF_TX_PKT_CNT_TXFRAMECNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_TX_PKT_CNT_REG = 
{
    "TX_PKT_CNT",
#if RU_INCLUDE_DESC
    "LIF_TX_PKT_CNT Register",
    "Aggregate statistics for the LIF transmit channel. These registers"
    "saturate at their malifum value and clear when read."
    "Note: These registers are also writable for test/diagnostics purposes.",
#endif
    LIF_TX_PKT_CNT_REG_OFFSET,
    0,
    0,
    339,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_TX_PKT_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_TX_BYTE_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_TX_BYTE_CNT_FIELDS[] =
{
    &LIF_TX_BYTE_CNT_TXBYTECNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_TX_BYTE_CNT_REG = 
{
    "TX_BYTE_CNT",
#if RU_INCLUDE_DESC
    "LIF_TX_BYTE_CNT Register",
    "",
#endif
    LIF_TX_BYTE_CNT_REG_OFFSET,
    0,
    0,
    340,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_TX_BYTE_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_TX_NON_FEC_PKT_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_TX_NON_FEC_PKT_CNT_FIELDS[] =
{
    &LIF_TX_NON_FEC_PKT_CNT_TXNONFECFRAMECNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_TX_NON_FEC_PKT_CNT_REG = 
{
    "TX_NON_FEC_PKT_CNT",
#if RU_INCLUDE_DESC
    "LIF_TX_NON_FEC_PKT_CNT Register",
    "",
#endif
    LIF_TX_NON_FEC_PKT_CNT_REG_OFFSET,
    0,
    0,
    341,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_TX_NON_FEC_PKT_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_TX_NON_FEC_BYTE_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_TX_NON_FEC_BYTE_CNT_FIELDS[] =
{
    &LIF_TX_NON_FEC_BYTE_CNT_TXNONFECBYTECNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_TX_NON_FEC_BYTE_CNT_REG = 
{
    "TX_NON_FEC_BYTE_CNT",
#if RU_INCLUDE_DESC
    "LIF_TX_NON_FEC_BYTE_CNT Register",
    "",
#endif
    LIF_TX_NON_FEC_BYTE_CNT_REG_OFFSET,
    0,
    0,
    342,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_TX_NON_FEC_BYTE_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_TX_FEC_PKT_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_TX_FEC_PKT_CNT_FIELDS[] =
{
    &LIF_TX_FEC_PKT_CNT_TXFECFRAMECNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_TX_FEC_PKT_CNT_REG = 
{
    "TX_FEC_PKT_CNT",
#if RU_INCLUDE_DESC
    "LIF_TX_FEC_PKT_CNT Register",
    "",
#endif
    LIF_TX_FEC_PKT_CNT_REG_OFFSET,
    0,
    0,
    343,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_TX_FEC_PKT_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_TX_FEC_BYTE_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_TX_FEC_BYTE_CNT_FIELDS[] =
{
    &LIF_TX_FEC_BYTE_CNT_TXFECBYTECNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_TX_FEC_BYTE_CNT_REG = 
{
    "TX_FEC_BYTE_CNT",
#if RU_INCLUDE_DESC
    "LIF_TX_FEC_BYTE_CNT Register",
    "Count of FEC bytes transmitted by Line-Coder.",
#endif
    LIF_TX_FEC_BYTE_CNT_REG_OFFSET,
    0,
    0,
    344,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_TX_FEC_BYTE_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_TX_FEC_BLK_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_TX_FEC_BLK_CNT_FIELDS[] =
{
    &LIF_TX_FEC_BLK_CNT_TXFECBLKSCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_TX_FEC_BLK_CNT_REG = 
{
    "TX_FEC_BLK_CNT",
#if RU_INCLUDE_DESC
    "LIF_TX_FEC_BLK_CNT Register",
    "",
#endif
    LIF_TX_FEC_BLK_CNT_REG_OFFSET,
    0,
    0,
    345,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_TX_FEC_BLK_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_TX_MPCP_PKT_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_TX_MPCP_PKT_CNT_FIELDS[] =
{
    &LIF_TX_MPCP_PKT_CNT_TXMPCPFRAMECNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_TX_MPCP_PKT_CNT_REG = 
{
    "TX_MPCP_PKT_CNT",
#if RU_INCLUDE_DESC
    "LIF_TX_MPCP_PKT_CNT Register",
    "",
#endif
    LIF_TX_MPCP_PKT_CNT_REG_OFFSET,
    0,
    0,
    346,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_TX_MPCP_PKT_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_DEBUG_TX_DATA_PKT_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_DEBUG_TX_DATA_PKT_CNT_FIELDS[] =
{
    &LIF_DEBUG_TX_DATA_PKT_CNT_TXDATAFRAMECNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_DEBUG_TX_DATA_PKT_CNT_REG = 
{
    "DEBUG_TX_DATA_PKT_CNT",
#if RU_INCLUDE_DESC
    "LIF_DEBUG_TX_DATA_PKT_CNT Register",
    "Count of transmitted frames. For debug.",
#endif
    LIF_DEBUG_TX_DATA_PKT_CNT_REG_OFFSET,
    0,
    0,
    347,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_DEBUG_TX_DATA_PKT_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_FEC_LLID_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_FEC_LLID_STATUS_FIELDS[] =
{
    &LIF_FEC_LLID_STATUS_STKYFECREVCLLIDBMSK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_FEC_LLID_STATUS_REG = 
{
    "FEC_LLID_STATUS",
#if RU_INCLUDE_DESC
    "LIF_FEC_LLID_STATUS Register",
    "Provides sticky status of which LLIDs have received FEC-encoded frames."
    "Status is provided bitwise per-LLID; each status bit can be cleared by"
    "writing 1 to it.",
#endif
    LIF_FEC_LLID_STATUS_REG_OFFSET,
    0,
    0,
    348,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_FEC_LLID_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_SEC_RX_TEK_IG_IV_LLID
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_SEC_RX_TEK_IG_IV_LLID_FIELDS[] =
{
    &LIF_SEC_RX_TEK_IG_IV_LLID_RESERVED0_FIELD,
    &LIF_SEC_RX_TEK_IG_IV_LLID_CFIGIVNULLLLID_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_SEC_RX_TEK_IG_IV_LLID_REG = 
{
    "SEC_RX_TEK_IG_IV_LLID",
#if RU_INCLUDE_DESC
    "LIF_SEC_RX_TEK_IG_IV_LLID Register",
    "Provides a programmable LLID field that will allow for the"
    "initialization vector (in TEK mode only) belonging to non-Raman frames"
    "to be preserved across Raman frames.",
#endif
    LIF_SEC_RX_TEK_IG_IV_LLID_REG_OFFSET,
    0,
    0,
    349,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_SEC_RX_TEK_IG_IV_LLID_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_PON_BER_INTERV_THRESH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_PON_BER_INTERV_THRESH_FIELDS[] =
{
    &LIF_PON_BER_INTERV_THRESH_CFRXLIFBERINTERVAL_FIELD,
    &LIF_PON_BER_INTERV_THRESH_CFRXLIFBERTHRESHLD_FIELD,
    &LIF_PON_BER_INTERV_THRESH_CFRXLIFBERCNTRL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_PON_BER_INTERV_THRESH_REG = 
{
    "PON_BER_INTERV_THRESH",
#if RU_INCLUDE_DESC
    "LIF_PON_BER_INTERV_THRESH Register",
    "Provides control for determining when to assert an interrupt when a"
    "defined number of programmable errors are observed within a defined"
    "window of time. These parameters directly control the behavior of"
    "intRxBerThreshExc.",
#endif
    LIF_PON_BER_INTERV_THRESH_REG_OFFSET,
    0,
    0,
    350,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LIF_PON_BER_INTERV_THRESH_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LSR_MON_A_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LSR_MON_A_CTRL_FIELDS[] =
{
    &LIF_LSR_MON_A_CTRL_RESERVED0_FIELD,
    &LIF_LSR_MON_A_CTRL_IOPBILASERENS1A_FIELD,
    &LIF_LSR_MON_A_CTRL_CFGLSRMONACTHI_FIELD,
    &LIF_LSR_MON_A_CTRL_RESERVED1_FIELD,
    &LIF_LSR_MON_A_CTRL_PBILASERMONRSTA_N_PRE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LSR_MON_A_CTRL_REG = 
{
    "LSR_MON_A_CTRL",
#if RU_INCLUDE_DESC
    "LIF_LSR_MON_A_CTRL Register",
    "Provides control over the laser monitor.",
#endif
    LIF_LSR_MON_A_CTRL_REG_OFFSET,
    0,
    0,
    351,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    LIF_LSR_MON_A_CTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LSR_MON_A_MAX_THR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LSR_MON_A_MAX_THR_FIELDS[] =
{
    &LIF_LSR_MON_A_MAX_THR_CFGLASERMONMAXA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LSR_MON_A_MAX_THR_REG = 
{
    "LSR_MON_A_MAX_THR",
#if RU_INCLUDE_DESC
    "LIF_LSR_MON_A_MAX_THR Register",
    "Defines a threshold for the laserOnMaxInt interrupt. laserOnMaxInt"
    "asserts when the laser enable signal stays active for a time greater"
    "than or equal to the laserOnMaxThresh setting. laserOnMaxThresh is"
    "expressed in units of TQ. However, be aware that the laser monitor"
    "operates from the core epnClk125 clock, so there may be some inaccuracy"
    "if the transmitter is running loop-timed (i.e. from the recovered"
    "receive clock). In addition, the laser-on time can jitter by 8 ns (1/2"
    "TQ) even for non-loop timed applications as the laser monitor on-time"
    "counter may not be TQ-aligned so an off-by-one error will occur half"
    "the time.",
#endif
    LIF_LSR_MON_A_MAX_THR_REG_OFFSET,
    0,
    0,
    352,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_LSR_MON_A_MAX_THR_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LSR_MON_A_BST_LEN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LSR_MON_A_BST_LEN_FIELDS[] =
{
    &LIF_LSR_MON_A_BST_LEN_LASERONTIMEA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LSR_MON_A_BST_LEN_REG = 
{
    "LSR_MON_A_BST_LEN",
#if RU_INCLUDE_DESC
    "LIF_LSR_MON_A_BST_LEN Register",
    "Indicates the laser-on time of the burst that set laserOffInt (i.e. the"
    "first burst which ended while laserOffInt was clear). Value is latched"
    "upon assertion of laserOffInt.",
#endif
    LIF_LSR_MON_A_BST_LEN_REG_OFFSET,
    0,
    0,
    353,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_LSR_MON_A_BST_LEN_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LSR_MON_A_BST_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LSR_MON_A_BST_CNT_FIELDS[] =
{
    &LIF_LSR_MON_A_BST_CNT_LASERMONBRSTCNTA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LSR_MON_A_BST_CNT_REG = 
{
    "LSR_MON_A_BST_CNT",
#if RU_INCLUDE_DESC
    "LIF_LSR_MON_A_BST_CNT Register",
    "Counts the number of bursts (laser-off events) since the last Laser"
    "Monitor reset or since the last read of this register. This register"
    "clears when read.",
#endif
    LIF_LSR_MON_A_BST_CNT_REG_OFFSET,
    0,
    0,
    354,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_LSR_MON_A_BST_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_DEBUG_PON_SM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_DEBUG_PON_SM_FIELDS[] =
{
    &LIF_DEBUG_PON_SM_RESERVED0_FIELD,
    &LIF_DEBUG_PON_SM_ALIGNCSQQ_FIELD,
    &LIF_DEBUG_PON_SM_RESERVED1_FIELD,
    &LIF_DEBUG_PON_SM_RXFECIFCSQQ_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_DEBUG_PON_SM_REG = 
{
    "DEBUG_PON_SM",
#if RU_INCLUDE_DESC
    "LIF_DEBUG_PON_SM Register",
    "Provides status of state machines in the PON receive side of the LIF.",
#endif
    LIF_DEBUG_PON_SM_REG_OFFSET,
    0,
    0,
    355,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    LIF_DEBUG_PON_SM_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_DEBUG_FEC_SM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_DEBUG_FEC_SM_FIELDS[] =
{
    &LIF_DEBUG_FEC_SM_RESERVED0_FIELD,
    &LIF_DEBUG_FEC_SM_RXSYNCSQQ_FIELD,
    &LIF_DEBUG_FEC_SM_RESERVED1_FIELD,
    &LIF_DEBUG_FEC_SM_RXCORCS_FIELD,
    &LIF_DEBUG_FEC_SM_RESERVED2_FIELD,
    &LIF_DEBUG_FEC_SM_FECRXOUTCS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_DEBUG_FEC_SM_REG = 
{
    "DEBUG_FEC_SM",
#if RU_INCLUDE_DESC
    "LIF_DEBUG_FEC_SM Register",
    "Provides status of state machines in the FEC receive side of the LIF.",
#endif
    LIF_DEBUG_FEC_SM_REG_OFFSET,
    0,
    0,
    356,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    LIF_DEBUG_FEC_SM_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_AE_PKTNUM_WINDOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_AE_PKTNUM_WINDOW_FIELDS[] =
{
    &LIF_AE_PKTNUM_WINDOW_CFGAEPKTNUMWND_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_AE_PKTNUM_WINDOW_REG = 
{
    "AE_PKTNUM_WINDOW",
#if RU_INCLUDE_DESC
    "LIF_AE_PKTNUM_WINDOW Register",
    "Provides the tolerance for packet number reception in replay protection"
    "mode.  Only applicable in 802.1ae security mode.",
#endif
    LIF_AE_PKTNUM_WINDOW_REG_OFFSET,
    0,
    0,
    357,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_AE_PKTNUM_WINDOW_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_AE_PKTNUM_THRESH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_AE_PKTNUM_THRESH_FIELDS[] =
{
    &LIF_AE_PKTNUM_THRESH_CFGPKTNUMMAXTHRESH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_AE_PKTNUM_THRESH_REG = 
{
    "AE_PKTNUM_THRESH",
#if RU_INCLUDE_DESC
    "LIF_AE_PKTNUM_THRESH Register",
    "Provides the threshold to warn of impending packet number rollover on"
    "transmit.",
#endif
    LIF_AE_PKTNUM_THRESH_REG_OFFSET,
    0,
    0,
    358,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_AE_PKTNUM_THRESH_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_AE_PKTNUM_STAT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_AE_PKTNUM_STAT_FIELDS[] =
{
    &LIF_AE_PKTNUM_STAT_RESERVED0_FIELD,
    &LIF_AE_PKTNUM_STAT_SECUPINDXWTPKTNUMMAX_FIELD,
    &LIF_AE_PKTNUM_STAT_RESERVED1_FIELD,
    &LIF_AE_PKTNUM_STAT_SECDNINDXWTPKTNUMABORT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_AE_PKTNUM_STAT_REG = 
{
    "AE_PKTNUM_STAT",
#if RU_INCLUDE_DESC
    "LIF_AE_PKTNUM_STAT Register",
    "Provides the status of packet number.",
#endif
    LIF_AE_PKTNUM_STAT_REG_OFFSET,
    0,
    0,
    359,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    LIF_AE_PKTNUM_STAT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_8
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_8_FIELDS[] =
{
    &LIF_LLID_8_RESERVED0_FIELD,
    &LIF_LLID_8_CFGLLID8_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_8_REG = 
{
    "LLID_8",
#if RU_INCLUDE_DESC
    "LIF_LLID_8 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_8_REG_OFFSET,
    0,
    0,
    360,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_8_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_9
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_9_FIELDS[] =
{
    &LIF_LLID_9_RESERVED0_FIELD,
    &LIF_LLID_9_CFGLLID9_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_9_REG = 
{
    "LLID_9",
#if RU_INCLUDE_DESC
    "LIF_LLID_9 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_9_REG_OFFSET,
    0,
    0,
    361,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_9_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_10
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_10_FIELDS[] =
{
    &LIF_LLID_10_RESERVED0_FIELD,
    &LIF_LLID_10_CFGLLID10_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_10_REG = 
{
    "LLID_10",
#if RU_INCLUDE_DESC
    "LIF_LLID_10 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_10_REG_OFFSET,
    0,
    0,
    362,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_10_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_11
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_11_FIELDS[] =
{
    &LIF_LLID_11_RESERVED0_FIELD,
    &LIF_LLID_11_CFGLLID11_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_11_REG = 
{
    "LLID_11",
#if RU_INCLUDE_DESC
    "LIF_LLID_11 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_11_REG_OFFSET,
    0,
    0,
    363,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_11_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_12
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_12_FIELDS[] =
{
    &LIF_LLID_12_RESERVED0_FIELD,
    &LIF_LLID_12_CFGLLID12_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_12_REG = 
{
    "LLID_12",
#if RU_INCLUDE_DESC
    "LIF_LLID_12 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_12_REG_OFFSET,
    0,
    0,
    364,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_12_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_13
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_13_FIELDS[] =
{
    &LIF_LLID_13_RESERVED0_FIELD,
    &LIF_LLID_13_CFGLLID13_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_13_REG = 
{
    "LLID_13",
#if RU_INCLUDE_DESC
    "LIF_LLID_13 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_13_REG_OFFSET,
    0,
    0,
    365,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_13_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_14
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_14_FIELDS[] =
{
    &LIF_LLID_14_RESERVED0_FIELD,
    &LIF_LLID_14_CFGLLID14_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_14_REG = 
{
    "LLID_14",
#if RU_INCLUDE_DESC
    "LIF_LLID_14 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_14_REG_OFFSET,
    0,
    0,
    366,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_14_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_15
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_15_FIELDS[] =
{
    &LIF_LLID_15_RESERVED0_FIELD,
    &LIF_LLID_15_CFGLLID15_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_15_REG = 
{
    "LLID_15",
#if RU_INCLUDE_DESC
    "LIF_LLID_15 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_15_REG_OFFSET,
    0,
    0,
    367,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_15_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_24
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_24_FIELDS[] =
{
    &LIF_LLID_24_RESERVED0_FIELD,
    &LIF_LLID_24_CFGLLID24_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_24_REG = 
{
    "LLID_24",
#if RU_INCLUDE_DESC
    "LIF_LLID_24 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_24_REG_OFFSET,
    0,
    0,
    368,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_24_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_25
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_25_FIELDS[] =
{
    &LIF_LLID_25_RESERVED0_FIELD,
    &LIF_LLID_25_CFGLLID25_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_25_REG = 
{
    "LLID_25",
#if RU_INCLUDE_DESC
    "LIF_LLID_25 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_25_REG_OFFSET,
    0,
    0,
    369,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_25_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_26
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_26_FIELDS[] =
{
    &LIF_LLID_26_RESERVED0_FIELD,
    &LIF_LLID_26_CFGLLID26_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_26_REG = 
{
    "LLID_26",
#if RU_INCLUDE_DESC
    "LIF_LLID_26 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_26_REG_OFFSET,
    0,
    0,
    370,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_26_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_27
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_27_FIELDS[] =
{
    &LIF_LLID_27_RESERVED0_FIELD,
    &LIF_LLID_27_CFGLLID27_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_27_REG = 
{
    "LLID_27",
#if RU_INCLUDE_DESC
    "LIF_LLID_27 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_27_REG_OFFSET,
    0,
    0,
    371,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_27_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_28
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_28_FIELDS[] =
{
    &LIF_LLID_28_RESERVED0_FIELD,
    &LIF_LLID_28_CFGLLID28_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_28_REG = 
{
    "LLID_28",
#if RU_INCLUDE_DESC
    "LIF_LLID_28 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_28_REG_OFFSET,
    0,
    0,
    372,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_28_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_29
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_29_FIELDS[] =
{
    &LIF_LLID_29_RESERVED0_FIELD,
    &LIF_LLID_29_CFGLLID29_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_29_REG = 
{
    "LLID_29",
#if RU_INCLUDE_DESC
    "LIF_LLID_29 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_29_REG_OFFSET,
    0,
    0,
    373,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_29_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_30
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_30_FIELDS[] =
{
    &LIF_LLID_30_RESERVED0_FIELD,
    &LIF_LLID_30_CFGLLID30_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_30_REG = 
{
    "LLID_30",
#if RU_INCLUDE_DESC
    "LIF_LLID_30 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_30_REG_OFFSET,
    0,
    0,
    374,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_30_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_LLID_31
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_LLID_31_FIELDS[] =
{
    &LIF_LLID_31_RESERVED0_FIELD,
    &LIF_LLID_31_CFGLLID31_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_LLID_31_REG = 
{
    "LLID_31",
#if RU_INCLUDE_DESC
    "LIF_LLID_31 Register",
    "Provides configuration for LLID mapping. LIF supports 32 bidirectional"
    "LLIDs.",
#endif
    LIF_LLID_31_REG_OFFSET,
    0,
    0,
    375,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_LLID_31_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_VLAN_TYPE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_VLAN_TYPE_FIELDS[] =
{
    &LIF_VLAN_TYPE_RESERVED0_FIELD,
    &LIF_VLAN_TYPE_CFGVLANTYPE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_VLAN_TYPE_REG = 
{
    "VLAN_TYPE",
#if RU_INCLUDE_DESC
    "LIF_VLAN_TYPE Register",
    "Provides a programmable VLAN type identifier for upstream P2P traffic.",
#endif
    LIF_VLAN_TYPE_REG_OFFSET,
    0,
    0,
    376,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_VLAN_TYPE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_EN_FIELDS[] =
{
    &LIF_P2P_AE_SCI_EN_CFGP2PSCIEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_EN_REG = 
{
    "P2P_AE_SCI_EN",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_EN Register",
    "Enables SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_EN_REG_OFFSET,
    0,
    0,
    377,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_EN_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_0_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_0_CFGP2PSCI_LO_0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_0_REG = 
{
    "P2P_AE_SCI_LO_0",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_0 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_0_REG_OFFSET,
    0,
    0,
    378,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_0_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_0_CFGP2PSCI_HI_0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_0_REG = 
{
    "P2P_AE_SCI_HI_0",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_0 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_0_REG_OFFSET,
    0,
    0,
    379,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_1_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_1_CFGP2PSCI_LO_1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_1_REG = 
{
    "P2P_AE_SCI_LO_1",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_1 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_1_REG_OFFSET,
    0,
    0,
    380,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_1_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_1_CFGP2PSCI_HI_1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_1_REG = 
{
    "P2P_AE_SCI_HI_1",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_1 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_1_REG_OFFSET,
    0,
    0,
    381,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_2_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_2_CFGP2PSCI_LO_2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_2_REG = 
{
    "P2P_AE_SCI_LO_2",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_2 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_2_REG_OFFSET,
    0,
    0,
    382,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_2_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_2_CFGP2PSCI_HI_2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_2_REG = 
{
    "P2P_AE_SCI_HI_2",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_2 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_2_REG_OFFSET,
    0,
    0,
    383,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_3_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_3_CFGP2PSCI_LO_3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_3_REG = 
{
    "P2P_AE_SCI_LO_3",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_3 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_3_REG_OFFSET,
    0,
    0,
    384,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_3_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_3_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_3_CFGP2PSCI_HI_3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_3_REG = 
{
    "P2P_AE_SCI_HI_3",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_3 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_3_REG_OFFSET,
    0,
    0,
    385,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_3_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_4
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_4_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_4_CFGP2PSCI_LO_4_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_4_REG = 
{
    "P2P_AE_SCI_LO_4",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_4 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_4_REG_OFFSET,
    0,
    0,
    386,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_4_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_4
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_4_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_4_CFGP2PSCI_HI_4_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_4_REG = 
{
    "P2P_AE_SCI_HI_4",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_4 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_4_REG_OFFSET,
    0,
    0,
    387,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_4_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_5
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_5_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_5_CFGP2PSCI_LO_5_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_5_REG = 
{
    "P2P_AE_SCI_LO_5",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_5 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_5_REG_OFFSET,
    0,
    0,
    388,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_5_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_5
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_5_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_5_CFGP2PSCI_HI_5_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_5_REG = 
{
    "P2P_AE_SCI_HI_5",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_5 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_5_REG_OFFSET,
    0,
    0,
    389,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_5_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_6
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_6_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_6_CFGP2PSCI_LO_6_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_6_REG = 
{
    "P2P_AE_SCI_LO_6",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_6 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_6_REG_OFFSET,
    0,
    0,
    390,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_6_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_6
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_6_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_6_CFGP2PSCI_HI_6_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_6_REG = 
{
    "P2P_AE_SCI_HI_6",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_6 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_6_REG_OFFSET,
    0,
    0,
    391,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_6_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_7
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_7_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_7_CFGP2PSCI_LO_7_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_7_REG = 
{
    "P2P_AE_SCI_LO_7",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_7 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_7_REG_OFFSET,
    0,
    0,
    392,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_7_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_7
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_7_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_7_CFGP2PSCI_HI_7_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_7_REG = 
{
    "P2P_AE_SCI_HI_7",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_7 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_7_REG_OFFSET,
    0,
    0,
    393,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_7_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_8
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_8_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_8_CFGP2PSCI_LO_8_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_8_REG = 
{
    "P2P_AE_SCI_LO_8",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_8 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_8_REG_OFFSET,
    0,
    0,
    394,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_8_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_8
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_8_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_8_CFGP2PSCI_HI_8_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_8_REG = 
{
    "P2P_AE_SCI_HI_8",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_8 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_8_REG_OFFSET,
    0,
    0,
    395,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_8_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_9
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_9_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_9_CFGP2PSCI_LO_9_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_9_REG = 
{
    "P2P_AE_SCI_LO_9",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_9 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_9_REG_OFFSET,
    0,
    0,
    396,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_9_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_9
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_9_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_9_CFGP2PSCI_HI_9_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_9_REG = 
{
    "P2P_AE_SCI_HI_9",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_9 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_9_REG_OFFSET,
    0,
    0,
    397,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_9_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_10
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_10_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_10_CFGP2PSCI_LO_10_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_10_REG = 
{
    "P2P_AE_SCI_LO_10",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_10 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_10_REG_OFFSET,
    0,
    0,
    398,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_10_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_10
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_10_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_10_CFGP2PSCI_HI_10_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_10_REG = 
{
    "P2P_AE_SCI_HI_10",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_10 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_10_REG_OFFSET,
    0,
    0,
    399,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_10_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_11
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_11_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_11_CFGP2PSCI_LO_11_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_11_REG = 
{
    "P2P_AE_SCI_LO_11",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_11 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_11_REG_OFFSET,
    0,
    0,
    400,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_11_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_11
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_11_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_11_CFGP2PSCI_HI_11_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_11_REG = 
{
    "P2P_AE_SCI_HI_11",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_11 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_11_REG_OFFSET,
    0,
    0,
    401,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_11_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_12
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_12_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_12_CFGP2PSCI_LO_12_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_12_REG = 
{
    "P2P_AE_SCI_LO_12",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_12 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_12_REG_OFFSET,
    0,
    0,
    402,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_12_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_12
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_12_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_12_CFGP2PSCI_HI_12_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_12_REG = 
{
    "P2P_AE_SCI_HI_12",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_12 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_12_REG_OFFSET,
    0,
    0,
    403,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_12_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_13
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_13_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_13_CFGP2PSCI_LO_13_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_13_REG = 
{
    "P2P_AE_SCI_LO_13",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_13 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_13_REG_OFFSET,
    0,
    0,
    404,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_13_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_13
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_13_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_13_CFGP2PSCI_HI_13_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_13_REG = 
{
    "P2P_AE_SCI_HI_13",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_13 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_13_REG_OFFSET,
    0,
    0,
    405,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_13_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_14
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_14_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_14_CFGP2PSCI_LO_14_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_14_REG = 
{
    "P2P_AE_SCI_LO_14",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_14 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_14_REG_OFFSET,
    0,
    0,
    406,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_14_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_14
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_14_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_14_CFGP2PSCI_HI_14_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_14_REG = 
{
    "P2P_AE_SCI_HI_14",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_14 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_14_REG_OFFSET,
    0,
    0,
    407,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_14_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_15
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_15_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_15_CFGP2PSCI_LO_15_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_15_REG = 
{
    "P2P_AE_SCI_LO_15",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_15 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_15_REG_OFFSET,
    0,
    0,
    408,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_15_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_15
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_15_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_15_CFGP2PSCI_HI_15_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_15_REG = 
{
    "P2P_AE_SCI_HI_15",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_15 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_15_REG_OFFSET,
    0,
    0,
    409,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_15_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_SEC_KEY_SEL_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_SEC_KEY_SEL_1_FIELDS[] =
{
    &LIF_SEC_KEY_SEL_1_KEYSEL_HI_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_SEC_KEY_SEL_1_REG = 
{
    "SEC_KEY_SEL_1",
#if RU_INCLUDE_DESC
    "LIF_SEC_KEY_SEL_1 Register",
    "Provides addition per-LLID status of downstream security key for"
    "802.1ae P2P.",
#endif
    LIF_SEC_KEY_SEL_1_REG_OFFSET,
    0,
    0,
    410,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_SEC_KEY_SEL_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_FIELDS[] =
{
    &LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_RESERVED0_FIELD,
    &LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_CF_PLAINTXT_OH_B_IDLE_PAD_FIELD,
    &LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_CF_PLAINTXT_OH_A_IDLE_PAD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_REG = 
{
    "PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL",
#if RU_INCLUDE_DESC
    "LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL Register",
    "",
#endif
    LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_REG_OFFSET,
    0,
    0,
    411,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AUTONEG_CONTROL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AUTONEG_CONTROL_FIELDS[] =
{
    &LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_LINKTIMER_FIELD,
    &LIF_P2P_AUTONEG_CONTROL_RESERVED0_FIELD,
    &LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_MODE_SEL_FIELD,
    &LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_RESTART_FIELD,
    &LIF_P2P_AUTONEG_CONTROL_CF_AUTONEG_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AUTONEG_CONTROL_REG = 
{
    "P2P_AUTONEG_CONTROL",
#if RU_INCLUDE_DESC
    "LIF_P2P_AUTONEG_CONTROL Register",
    "Autonegotiation Configuration",
#endif
    LIF_P2P_AUTONEG_CONTROL_REG_OFFSET,
    0,
    0,
    412,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    LIF_P2P_AUTONEG_CONTROL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AUTONEG_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AUTONEG_STATUS_FIELDS[] =
{
    &LIF_P2P_AUTONEG_STATUS_RESERVED0_FIELD,
    &LIF_P2P_AUTONEG_STATUS_AN_LP_REMOTE_FAULT_FIELD,
    &LIF_P2P_AUTONEG_STATUS_AN_SYNC_STATUS_FIELD,
    &LIF_P2P_AUTONEG_STATUS_AN_COMPLETE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AUTONEG_STATUS_REG = 
{
    "P2P_AUTONEG_STATUS",
#if RU_INCLUDE_DESC
    "LIF_P2P_AUTONEG_STATUS Register",
    "Autonegotiation Status",
#endif
    LIF_P2P_AUTONEG_STATUS_REG_OFFSET,
    0,
    0,
    413,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    LIF_P2P_AUTONEG_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AUTONEG_ABILITY_CONFIG_REG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AUTONEG_ABILITY_CONFIG_REG_FIELDS[] =
{
    &LIF_P2P_AUTONEG_ABILITY_CONFIG_REG_RESERVED0_FIELD,
    &LIF_P2P_AUTONEG_ABILITY_CONFIG_REG_CF_LIF_P2P_AE_AUTONEG_CONFIG_ABILITY_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AUTONEG_ABILITY_CONFIG_REG_REG = 
{
    "P2P_AUTONEG_ABILITY_CONFIG_REG",
#if RU_INCLUDE_DESC
    "LIF_P2P_AUTONEG_ABILITY_CONFIG_REG Register",
    "Autonegotiation Ability / Config Register of this device",
#endif
    LIF_P2P_AUTONEG_ABILITY_CONFIG_REG_REG_OFFSET,
    0,
    0,
    414,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_P2P_AUTONEG_ABILITY_CONFIG_REG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ_FIELDS[] =
{
    &LIF_P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ_RESERVED0_FIELD,
    &LIF_P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ_CF_LIF_P2P_AE_AUTONEG_LP_ABILITY_READ_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ_REG = 
{
    "P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ",
#if RU_INCLUDE_DESC
    "LIF_P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ Register",
    "Autonegotiation Ability / Config Register of Link Partner",
#endif
    LIF_P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ_REG_OFFSET,
    0,
    0,
    415,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_INTER_OP_CONTROL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_INTER_OP_CONTROL_FIELDS[] =
{
    &LIF_P2P_INTER_OP_CONTROL_RESERVED0_FIELD,
    &LIF_P2P_INTER_OP_CONTROL_CFP2PMASKONEPRECIDLE_FIELD,
    &LIF_P2P_INTER_OP_CONTROL_CFP2PMODETRANSMITREMFAULT_FIELD,
    &LIF_P2P_INTER_OP_CONTROL_CFP2PMODETRANSMITDEFICITIDLEENABLE_FIELD,
    &LIF_P2P_INTER_OP_CONTROL_RESERVED1_FIELD,
    &LIF_P2P_INTER_OP_CONTROL_CFP2PMODEADAPTIVESHORTPRE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_INTER_OP_CONTROL_REG = 
{
    "P2P_INTER_OP_CONTROL",
#if RU_INCLUDE_DESC
    "LIF_P2P_INTER_OP_CONTROL Register",
    "Additional interop controls for LIF P2P AE mode",
#endif
    LIF_P2P_INTER_OP_CONTROL_REG_OFFSET,
    0,
    0,
    416,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    LIF_P2P_INTER_OP_CONTROL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_PON_CONTROL_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_PON_CONTROL_2_FIELDS[] =
{
    &LIF_PON_CONTROL_2_RESERVED0_FIELD,
    &LIF_PON_CONTROL_2_CFGDISP2PDNINDEXMAP_FIELD,
    &LIF_PON_CONTROL_2_CFGDISP2PUPINDEXMAP_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_PON_CONTROL_2_REG = 
{
    "PON_CONTROL_2",
#if RU_INCLUDE_DESC
    "LIF_PON_CONTROL_2 Register",
    "LIF control register.",
#endif
    LIF_PON_CONTROL_2_REG_OFFSET,
    0,
    0,
    417,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LIF_PON_CONTROL_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_VLAN_TYPE_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_VLAN_TYPE_1_FIELDS[] =
{
    &LIF_VLAN_TYPE_1_RESERVED0_FIELD,
    &LIF_VLAN_TYPE_1_CFGVLANTYPE_1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_VLAN_TYPE_1_REG = 
{
    "VLAN_TYPE_1",
#if RU_INCLUDE_DESC
    "LIF_VLAN_TYPE_1 Register",
    "Provides a programmable VLAN type identifier for upstream P2P traffic.",
#endif
    LIF_VLAN_TYPE_1_REG_OFFSET,
    0,
    0,
    418,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LIF_VLAN_TYPE_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_16
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_16_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_16_CFGP2PSCI_LO_16_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_16_REG = 
{
    "P2P_AE_SCI_LO_16",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_16 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_16_REG_OFFSET,
    0,
    0,
    419,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_16_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_16
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_16_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_16_CFGP2PSCI_HI_16_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_16_REG = 
{
    "P2P_AE_SCI_HI_16",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_16 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_16_REG_OFFSET,
    0,
    0,
    420,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_16_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_17
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_17_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_17_CFGP2PSCI_LO_17_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_17_REG = 
{
    "P2P_AE_SCI_LO_17",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_17 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_17_REG_OFFSET,
    0,
    0,
    421,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_17_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_17
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_17_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_17_CFGP2PSCI_HI_17_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_17_REG = 
{
    "P2P_AE_SCI_HI_17",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_17 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_17_REG_OFFSET,
    0,
    0,
    422,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_17_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_18
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_18_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_18_CFGP2PSCI_LO_18_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_18_REG = 
{
    "P2P_AE_SCI_LO_18",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_18 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_18_REG_OFFSET,
    0,
    0,
    423,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_18_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_18
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_18_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_18_CFGP2PSCI_HI_18_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_18_REG = 
{
    "P2P_AE_SCI_HI_18",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_18 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_18_REG_OFFSET,
    0,
    0,
    424,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_18_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_19
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_19_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_19_CFGP2PSCI_LO_19_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_19_REG = 
{
    "P2P_AE_SCI_LO_19",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_19 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_19_REG_OFFSET,
    0,
    0,
    425,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_19_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_19
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_19_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_19_CFGP2PSCI_HI_19_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_19_REG = 
{
    "P2P_AE_SCI_HI_19",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_19 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_19_REG_OFFSET,
    0,
    0,
    426,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_19_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_20
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_20_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_20_CFGP2PSCI_LO_20_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_20_REG = 
{
    "P2P_AE_SCI_LO_20",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_20 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_20_REG_OFFSET,
    0,
    0,
    427,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_20_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_20
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_20_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_20_CFGP2PSCI_HI_20_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_20_REG = 
{
    "P2P_AE_SCI_HI_20",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_20 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_20_REG_OFFSET,
    0,
    0,
    428,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_20_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_21
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_21_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_21_CFGP2PSCI_LO_21_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_21_REG = 
{
    "P2P_AE_SCI_LO_21",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_21 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_21_REG_OFFSET,
    0,
    0,
    429,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_21_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_21
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_21_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_21_CFGP2PSCI_HI_21_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_21_REG = 
{
    "P2P_AE_SCI_HI_21",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_21 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_21_REG_OFFSET,
    0,
    0,
    430,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_21_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_22
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_22_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_22_CFGP2PSCI_LO_22_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_22_REG = 
{
    "P2P_AE_SCI_LO_22",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_22 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_22_REG_OFFSET,
    0,
    0,
    431,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_22_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_22
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_22_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_22_CFGP2PSCI_HI_22_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_22_REG = 
{
    "P2P_AE_SCI_HI_22",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_22 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_22_REG_OFFSET,
    0,
    0,
    432,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_22_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_23
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_23_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_23_CFGP2PSCI_LO_23_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_23_REG = 
{
    "P2P_AE_SCI_LO_23",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_23 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_23_REG_OFFSET,
    0,
    0,
    433,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_23_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_23
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_23_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_23_CFGP2PSCI_HI_23_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_23_REG = 
{
    "P2P_AE_SCI_HI_23",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_23 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_23_REG_OFFSET,
    0,
    0,
    434,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_23_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_24
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_24_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_24_CFGP2PSCI_LO_24_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_24_REG = 
{
    "P2P_AE_SCI_LO_24",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_24 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_24_REG_OFFSET,
    0,
    0,
    435,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_24_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_24
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_24_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_24_CFGP2PSCI_HI_24_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_24_REG = 
{
    "P2P_AE_SCI_HI_24",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_24 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_24_REG_OFFSET,
    0,
    0,
    436,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_24_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_25
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_25_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_25_CFGP2PSCI_LO_25_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_25_REG = 
{
    "P2P_AE_SCI_LO_25",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_25 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_25_REG_OFFSET,
    0,
    0,
    437,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_25_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_25
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_25_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_25_CFGP2PSCI_HI_25_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_25_REG = 
{
    "P2P_AE_SCI_HI_25",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_25 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_25_REG_OFFSET,
    0,
    0,
    438,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_25_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_26
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_26_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_26_CFGP2PSCI_LO_26_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_26_REG = 
{
    "P2P_AE_SCI_LO_26",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_26 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_26_REG_OFFSET,
    0,
    0,
    439,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_26_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_26
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_26_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_26_CFGP2PSCI_HI_26_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_26_REG = 
{
    "P2P_AE_SCI_HI_26",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_26 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_26_REG_OFFSET,
    0,
    0,
    440,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_26_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_27
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_27_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_27_CFGP2PSCI_LO_27_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_27_REG = 
{
    "P2P_AE_SCI_LO_27",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_27 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_27_REG_OFFSET,
    0,
    0,
    441,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_27_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_27
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_27_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_27_CFGP2PSCI_HI_27_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_27_REG = 
{
    "P2P_AE_SCI_HI_27",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_27 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_27_REG_OFFSET,
    0,
    0,
    442,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_27_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_28
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_28_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_28_CFGP2PSCI_LO_28_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_28_REG = 
{
    "P2P_AE_SCI_LO_28",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_28 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_28_REG_OFFSET,
    0,
    0,
    443,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_28_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_28
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_28_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_28_CFGP2PSCI_HI_28_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_28_REG = 
{
    "P2P_AE_SCI_HI_28",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_28 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_28_REG_OFFSET,
    0,
    0,
    444,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_28_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_29
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_29_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_29_CFGP2PSCI_LO_29_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_29_REG = 
{
    "P2P_AE_SCI_LO_29",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_29 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_29_REG_OFFSET,
    0,
    0,
    445,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_29_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_29
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_29_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_29_CFGP2PSCI_HI_29_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_29_REG = 
{
    "P2P_AE_SCI_HI_29",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_29 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_29_REG_OFFSET,
    0,
    0,
    446,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_29_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_30
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_30_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_30_CFGP2PSCI_LO_30_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_30_REG = 
{
    "P2P_AE_SCI_LO_30",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_30 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_30_REG_OFFSET,
    0,
    0,
    447,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_30_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_30
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_30_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_30_CFGP2PSCI_HI_30_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_30_REG = 
{
    "P2P_AE_SCI_HI_30",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_30 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_30_REG_OFFSET,
    0,
    0,
    448,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_30_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_LO_31
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_LO_31_FIELDS[] =
{
    &LIF_P2P_AE_SCI_LO_31_CFGP2PSCI_LO_31_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_LO_31_REG = 
{
    "P2P_AE_SCI_LO_31",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_LO_31 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_LO_31_REG_OFFSET,
    0,
    0,
    449,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_LO_31_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_P2P_AE_SCI_HI_31
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_P2P_AE_SCI_HI_31_FIELDS[] =
{
    &LIF_P2P_AE_SCI_HI_31_CFGP2PSCI_HI_31_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_P2P_AE_SCI_HI_31_REG = 
{
    "P2P_AE_SCI_HI_31",
#if RU_INCLUDE_DESC
    "LIF_P2P_AE_SCI_HI_31 Register",
    "Provides SCI lookup for 802.1ae, P2P downstream traffic.",
#endif
    LIF_P2P_AE_SCI_HI_31_REG_OFFSET,
    0,
    0,
    450,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_P2P_AE_SCI_HI_31_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_SEC_CONTROL_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_SEC_CONTROL_2_FIELDS[] =
{
    &LIF_SEC_CONTROL_2_RESERVED0_FIELD,
    &LIF_SEC_CONTROL_2_CFGENSECOUTFFPRIME_FIELD,
    &LIF_SEC_CONTROL_2_CFGENAES_256_UP_FIELD,
    &LIF_SEC_CONTROL_2_CFGENAES_256_DN_FIELD,
    &LIF_SEC_CONTROL_2_CFG_MACSEC_XPN_TX_FIELD,
    &LIF_SEC_CONTROL_2_CFG_MACSEC_XPN_RX_FIELD,
    &LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_3_FIELD,
    &LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_2_FIELD,
    &LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_1_FIELD,
    &LIF_SEC_CONTROL_2_CFGKEYNUM_4_AN_0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_SEC_CONTROL_2_REG = 
{
    "SEC_CONTROL_2",
#if RU_INCLUDE_DESC
    "LIF_SEC_CONTROL_2 Register",
    "Security control register.",
#endif
    LIF_SEC_CONTROL_2_REG_OFFSET,
    0,
    0,
    451,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    10,
    LIF_SEC_CONTROL_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_SEC_MPCP_OFFSET
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_SEC_MPCP_OFFSET_FIELDS[] =
{
    &LIF_SEC_MPCP_OFFSET_CFGSECDNMPCPOFFSET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_SEC_MPCP_OFFSET_REG = 
{
    "SEC_MPCP_OFFSET",
#if RU_INCLUDE_DESC
    "LIF_SEC_MPCP_OFFSET Register",
    "Offset for downstream MPCP adjustment.",
#endif
    LIF_SEC_MPCP_OFFSET_REG_OFFSET,
    0,
    0,
    452,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_SEC_MPCP_OFFSET_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_1588_TS_SYNC_OFFSET
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_1588_TS_SYNC_OFFSET_FIELDS[] =
{
    &LIF_1588_TS_SYNC_OFFSET_CFG_TS48_SYNC_NS_INCREMENT_FIELD,
    &LIF_1588_TS_SYNC_OFFSET_RESERVED0_FIELD,
    &LIF_1588_TS_SYNC_OFFSET_CFGTSSYNCOFFSET_312_FIELD,
    &LIF_1588_TS_SYNC_OFFSET_CFGTSSYNCOFFSET_125_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_1588_TS_SYNC_OFFSET_REG = 
{
    "1588_TS_SYNC_OFFSET",
#if RU_INCLUDE_DESC
    "LIF_1588_TS_SYNC_OFFSET Register",
    "Timestamp synchronizer offset.",
#endif
    LIF_1588_TS_SYNC_OFFSET_REG_OFFSET,
    0,
    0,
    453,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    LIF_1588_TS_SYNC_OFFSET_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_DATA_PORT_DATA_8
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_DATA_PORT_DATA_8_FIELDS[] =
{
    &LIF_DATA_PORT_DATA_8_PBIPORTDATA8_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_DATA_PORT_DATA_8_REG = 
{
    "DATA_PORT_DATA_8",
#if RU_INCLUDE_DESC
    "LIF_DATA_PORT_DATA_8 Register",
    "",
#endif
    LIF_DATA_PORT_DATA_8_REG_OFFSET,
    0,
    0,
    454,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_DATA_PORT_DATA_8_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_DATA_PORT_DATA_9
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_DATA_PORT_DATA_9_FIELDS[] =
{
    &LIF_DATA_PORT_DATA_9_PBIPORTDATA9_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_DATA_PORT_DATA_9_REG = 
{
    "DATA_PORT_DATA_9",
#if RU_INCLUDE_DESC
    "LIF_DATA_PORT_DATA_9 Register",
    "",
#endif
    LIF_DATA_PORT_DATA_9_REG_OFFSET,
    0,
    0,
    455,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_DATA_PORT_DATA_9_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_DATA_PORT_DATA_10
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_DATA_PORT_DATA_10_FIELDS[] =
{
    &LIF_DATA_PORT_DATA_10_PBIPORTDATA10_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_DATA_PORT_DATA_10_REG = 
{
    "DATA_PORT_DATA_10",
#if RU_INCLUDE_DESC
    "LIF_DATA_PORT_DATA_10 Register",
    "",
#endif
    LIF_DATA_PORT_DATA_10_REG_OFFSET,
    0,
    0,
    456,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_DATA_PORT_DATA_10_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_DATA_PORT_DATA_11
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_DATA_PORT_DATA_11_FIELDS[] =
{
    &LIF_DATA_PORT_DATA_11_PBIPORTDATA11_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_DATA_PORT_DATA_11_REG = 
{
    "DATA_PORT_DATA_11",
#if RU_INCLUDE_DESC
    "LIF_DATA_PORT_DATA_11 Register",
    "",
#endif
    LIF_DATA_PORT_DATA_11_REG_OFFSET,
    0,
    0,
    457,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_DATA_PORT_DATA_11_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_SEC_TX_SSCI
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_SEC_TX_SSCI_FIELDS[] =
{
    &LIF_SEC_TX_SSCI_CFG_MACSEC_XPN_TX_SSCI_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_SEC_TX_SSCI_REG = 
{
    "SEC_TX_SSCI",
#if RU_INCLUDE_DESC
    "LIF_SEC_TX_SSCI Register",
    "Upstream SSCI for 802.1ae extended packet numbering.",
#endif
    LIF_SEC_TX_SSCI_REG_OFFSET,
    0,
    0,
    458,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_SEC_TX_SSCI_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_SEC_TX_SALT_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_SEC_TX_SALT_0_FIELDS[] =
{
    &LIF_SEC_TX_SALT_0_CFG_MACSEC_XPN_TX_SALT_0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_SEC_TX_SALT_0_REG = 
{
    "SEC_TX_SALT_0",
#if RU_INCLUDE_DESC
    "LIF_SEC_TX_SALT_0 Register",
    "Upstream 96-bits SALT for 802.1ae extended packet numbering.",
#endif
    LIF_SEC_TX_SALT_0_REG_OFFSET,
    0,
    0,
    459,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_SEC_TX_SALT_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_SEC_TX_SALT_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_SEC_TX_SALT_1_FIELDS[] =
{
    &LIF_SEC_TX_SALT_1_CFG_MACSEC_XPN_TX_SALT_1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_SEC_TX_SALT_1_REG = 
{
    "SEC_TX_SALT_1",
#if RU_INCLUDE_DESC
    "LIF_SEC_TX_SALT_1 Register",
    "Upstream 96-bits SALT for 802.1ae extended packet numbering.",
#endif
    LIF_SEC_TX_SALT_1_REG_OFFSET,
    0,
    0,
    460,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_SEC_TX_SALT_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_SEC_TX_SALT_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_SEC_TX_SALT_2_FIELDS[] =
{
    &LIF_SEC_TX_SALT_2_CFG_MACSEC_XPN_TX_SALT_2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_SEC_TX_SALT_2_REG = 
{
    "SEC_TX_SALT_2",
#if RU_INCLUDE_DESC
    "LIF_SEC_TX_SALT_2 Register",
    "Upstream 96-bits SALT for 802.1ae extended packet numbering.",
#endif
    LIF_SEC_TX_SALT_2_REG_OFFSET,
    0,
    0,
    461,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_SEC_TX_SALT_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_SEC_RX_SSCI
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_SEC_RX_SSCI_FIELDS[] =
{
    &LIF_SEC_RX_SSCI_CFG_MACSEC_XPN_RX_SSCI_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_SEC_RX_SSCI_REG = 
{
    "SEC_RX_SSCI",
#if RU_INCLUDE_DESC
    "LIF_SEC_RX_SSCI Register",
    "Downstream SSCI for 802.1ae extended packet numbering.",
#endif
    LIF_SEC_RX_SSCI_REG_OFFSET,
    0,
    0,
    462,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_SEC_RX_SSCI_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_SEC_RX_SALT_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_SEC_RX_SALT_0_FIELDS[] =
{
    &LIF_SEC_RX_SALT_0_CFG_MACSEC_XPN_RX_SALT_0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_SEC_RX_SALT_0_REG = 
{
    "SEC_RX_SALT_0",
#if RU_INCLUDE_DESC
    "LIF_SEC_RX_SALT_0 Register",
    "Downstream 96-bits SALT for 802.1ae extended packet numbering.",
#endif
    LIF_SEC_RX_SALT_0_REG_OFFSET,
    0,
    0,
    463,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_SEC_RX_SALT_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_SEC_RX_SALT_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_SEC_RX_SALT_1_FIELDS[] =
{
    &LIF_SEC_RX_SALT_1_CFG_MACSEC_XPN_RX_SALT_1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_SEC_RX_SALT_1_REG = 
{
    "SEC_RX_SALT_1",
#if RU_INCLUDE_DESC
    "LIF_SEC_RX_SALT_1 Register",
    "Downstream 96-bits SALT for 802.1ae extended packet numbering.",
#endif
    LIF_SEC_RX_SALT_1_REG_OFFSET,
    0,
    0,
    464,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_SEC_RX_SALT_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LIF_SEC_RX_SALT_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LIF_SEC_RX_SALT_2_FIELDS[] =
{
    &LIF_SEC_RX_SALT_2_CFG_MACSEC_XPN_RX_SALT_2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LIF_SEC_RX_SALT_2_REG = 
{
    "SEC_RX_SALT_2",
#if RU_INCLUDE_DESC
    "LIF_SEC_RX_SALT_2 Register",
    "Downstream 96-bits SALT for 802.1ae extended packet numbering.",
#endif
    LIF_SEC_RX_SALT_2_REG_OFFSET,
    0,
    0,
    465,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LIF_SEC_RX_SALT_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: LIF
 ******************************************************************************/
static const ru_reg_rec *LIF_REGS[] =
{
    &LIF_PON_CONTROL_REG,
    &LIF_PON_INTER_OP_CONTROL_REG,
    &LIF_FEC_CONTROL_REG,
    &LIF_SEC_CONTROL_REG,
    &LIF_MACSEC_REG,
    &LIF_INT_STATUS_REG,
    &LIF_INT_MASK_REG,
    &LIF_DATA_PORT_COMMAND_REG,
    &LIF_DATA_PORT_DATA_0_REG,
    &LIF_DATA_PORT_DATA_1_REG,
    &LIF_DATA_PORT_DATA_2_REG,
    &LIF_DATA_PORT_DATA_3_REG,
    &LIF_DATA_PORT_DATA_4_REG,
    &LIF_DATA_PORT_DATA_5_REG,
    &LIF_DATA_PORT_DATA_6_REG,
    &LIF_DATA_PORT_DATA_7_REG,
    &LIF_LLID_0_REG,
    &LIF_LLID_1_REG,
    &LIF_LLID_2_REG,
    &LIF_LLID_3_REG,
    &LIF_LLID_4_REG,
    &LIF_LLID_5_REG,
    &LIF_LLID_6_REG,
    &LIF_LLID_7_REG,
    &LIF_LLID_16_REG,
    &LIF_LLID_17_REG,
    &LIF_LLID_18_REG,
    &LIF_LLID_19_REG,
    &LIF_LLID_20_REG,
    &LIF_LLID_21_REG,
    &LIF_LLID_22_REG,
    &LIF_LLID_23_REG,
    &LIF_TIME_REF_CNT_REG,
    &LIF_TIMESTAMP_UPD_PER_REG,
    &LIF_TP_TIME_REG,
    &LIF_MPCP_TIME_REG,
    &LIF_MAXLEN_CTR_REG,
    &LIF_LASER_ON_DELTA_REG,
    &LIF_LASER_OFF_IDLE_REG,
    &LIF_FEC_INIT_IDLE_REG,
    &LIF_FEC_ERR_ALLOW_REG,
    &LIF_SEC_KEY_SEL_REG,
    &LIF_DN_ENCRYPT_STAT_REG,
    &LIF_SEC_UP_KEY_STAT_REG,
    &LIF_SEC_UP_ENCRYPT_STAT_REG,
    &LIF_SEC_UP_MPCP_OFFSET_REG,
    &LIF_FEC_PER_LLID_REG,
    &LIF_RX_LINE_CODE_ERR_CNT_REG,
    &LIF_RX_AGG_MPCP_FRM_REG,
    &LIF_RX_AGG_GOOD_FRM_REG,
    &LIF_RX_AGG_GOOD_BYTE_REG,
    &LIF_RX_AGG_UNDERSZ_FRM_REG,
    &LIF_RX_AGG_OVERSZ_FRM_REG,
    &LIF_RX_AGG_CRC8_FRM_REG,
    &LIF_RX_AGG_FEC_FRM_REG,
    &LIF_RX_AGG_FEC_BYTE_REG,
    &LIF_RX_AGG_FEC_EXC_ERR_FRM_REG,
    &LIF_RX_AGG_NONFEC_GOOD_FRM_REG,
    &LIF_RX_AGG_NONFEC_GOOD_BYTE_REG,
    &LIF_RX_AGG_ERR_BYTES_REG,
    &LIF_RX_AGG_ERR_ZEROES_REG,
    &LIF_RX_AGG_NO_ERR_BLKS_REG,
    &LIF_RX_AGG_COR_BLKS_REG,
    &LIF_RX_AGG_UNCOR_BLKS_REG,
    &LIF_RX_AGG_ERR_ONES_REG,
    &LIF_RX_AGG_ERR_FRM_REG,
    &LIF_TX_PKT_CNT_REG,
    &LIF_TX_BYTE_CNT_REG,
    &LIF_TX_NON_FEC_PKT_CNT_REG,
    &LIF_TX_NON_FEC_BYTE_CNT_REG,
    &LIF_TX_FEC_PKT_CNT_REG,
    &LIF_TX_FEC_BYTE_CNT_REG,
    &LIF_TX_FEC_BLK_CNT_REG,
    &LIF_TX_MPCP_PKT_CNT_REG,
    &LIF_DEBUG_TX_DATA_PKT_CNT_REG,
    &LIF_FEC_LLID_STATUS_REG,
    &LIF_SEC_RX_TEK_IG_IV_LLID_REG,
    &LIF_PON_BER_INTERV_THRESH_REG,
    &LIF_LSR_MON_A_CTRL_REG,
    &LIF_LSR_MON_A_MAX_THR_REG,
    &LIF_LSR_MON_A_BST_LEN_REG,
    &LIF_LSR_MON_A_BST_CNT_REG,
    &LIF_DEBUG_PON_SM_REG,
    &LIF_DEBUG_FEC_SM_REG,
    &LIF_AE_PKTNUM_WINDOW_REG,
    &LIF_AE_PKTNUM_THRESH_REG,
    &LIF_AE_PKTNUM_STAT_REG,
    &LIF_LLID_8_REG,
    &LIF_LLID_9_REG,
    &LIF_LLID_10_REG,
    &LIF_LLID_11_REG,
    &LIF_LLID_12_REG,
    &LIF_LLID_13_REG,
    &LIF_LLID_14_REG,
    &LIF_LLID_15_REG,
    &LIF_LLID_24_REG,
    &LIF_LLID_25_REG,
    &LIF_LLID_26_REG,
    &LIF_LLID_27_REG,
    &LIF_LLID_28_REG,
    &LIF_LLID_29_REG,
    &LIF_LLID_30_REG,
    &LIF_LLID_31_REG,
    &LIF_VLAN_TYPE_REG,
    &LIF_P2P_AE_SCI_EN_REG,
    &LIF_P2P_AE_SCI_LO_0_REG,
    &LIF_P2P_AE_SCI_HI_0_REG,
    &LIF_P2P_AE_SCI_LO_1_REG,
    &LIF_P2P_AE_SCI_HI_1_REG,
    &LIF_P2P_AE_SCI_LO_2_REG,
    &LIF_P2P_AE_SCI_HI_2_REG,
    &LIF_P2P_AE_SCI_LO_3_REG,
    &LIF_P2P_AE_SCI_HI_3_REG,
    &LIF_P2P_AE_SCI_LO_4_REG,
    &LIF_P2P_AE_SCI_HI_4_REG,
    &LIF_P2P_AE_SCI_LO_5_REG,
    &LIF_P2P_AE_SCI_HI_5_REG,
    &LIF_P2P_AE_SCI_LO_6_REG,
    &LIF_P2P_AE_SCI_HI_6_REG,
    &LIF_P2P_AE_SCI_LO_7_REG,
    &LIF_P2P_AE_SCI_HI_7_REG,
    &LIF_P2P_AE_SCI_LO_8_REG,
    &LIF_P2P_AE_SCI_HI_8_REG,
    &LIF_P2P_AE_SCI_LO_9_REG,
    &LIF_P2P_AE_SCI_HI_9_REG,
    &LIF_P2P_AE_SCI_LO_10_REG,
    &LIF_P2P_AE_SCI_HI_10_REG,
    &LIF_P2P_AE_SCI_LO_11_REG,
    &LIF_P2P_AE_SCI_HI_11_REG,
    &LIF_P2P_AE_SCI_LO_12_REG,
    &LIF_P2P_AE_SCI_HI_12_REG,
    &LIF_P2P_AE_SCI_LO_13_REG,
    &LIF_P2P_AE_SCI_HI_13_REG,
    &LIF_P2P_AE_SCI_LO_14_REG,
    &LIF_P2P_AE_SCI_HI_14_REG,
    &LIF_P2P_AE_SCI_LO_15_REG,
    &LIF_P2P_AE_SCI_HI_15_REG,
    &LIF_SEC_KEY_SEL_1_REG,
    &LIF_PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL_REG,
    &LIF_P2P_AUTONEG_CONTROL_REG,
    &LIF_P2P_AUTONEG_STATUS_REG,
    &LIF_P2P_AUTONEG_ABILITY_CONFIG_REG_REG,
    &LIF_P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ_REG,
    &LIF_P2P_INTER_OP_CONTROL_REG,
    &LIF_PON_CONTROL_2_REG,
    &LIF_VLAN_TYPE_1_REG,
    &LIF_P2P_AE_SCI_LO_16_REG,
    &LIF_P2P_AE_SCI_HI_16_REG,
    &LIF_P2P_AE_SCI_LO_17_REG,
    &LIF_P2P_AE_SCI_HI_17_REG,
    &LIF_P2P_AE_SCI_LO_18_REG,
    &LIF_P2P_AE_SCI_HI_18_REG,
    &LIF_P2P_AE_SCI_LO_19_REG,
    &LIF_P2P_AE_SCI_HI_19_REG,
    &LIF_P2P_AE_SCI_LO_20_REG,
    &LIF_P2P_AE_SCI_HI_20_REG,
    &LIF_P2P_AE_SCI_LO_21_REG,
    &LIF_P2P_AE_SCI_HI_21_REG,
    &LIF_P2P_AE_SCI_LO_22_REG,
    &LIF_P2P_AE_SCI_HI_22_REG,
    &LIF_P2P_AE_SCI_LO_23_REG,
    &LIF_P2P_AE_SCI_HI_23_REG,
    &LIF_P2P_AE_SCI_LO_24_REG,
    &LIF_P2P_AE_SCI_HI_24_REG,
    &LIF_P2P_AE_SCI_LO_25_REG,
    &LIF_P2P_AE_SCI_HI_25_REG,
    &LIF_P2P_AE_SCI_LO_26_REG,
    &LIF_P2P_AE_SCI_HI_26_REG,
    &LIF_P2P_AE_SCI_LO_27_REG,
    &LIF_P2P_AE_SCI_HI_27_REG,
    &LIF_P2P_AE_SCI_LO_28_REG,
    &LIF_P2P_AE_SCI_HI_28_REG,
    &LIF_P2P_AE_SCI_LO_29_REG,
    &LIF_P2P_AE_SCI_HI_29_REG,
    &LIF_P2P_AE_SCI_LO_30_REG,
    &LIF_P2P_AE_SCI_HI_30_REG,
    &LIF_P2P_AE_SCI_LO_31_REG,
    &LIF_P2P_AE_SCI_HI_31_REG,
    &LIF_SEC_CONTROL_2_REG,
    &LIF_SEC_MPCP_OFFSET_REG,
    &LIF_1588_TS_SYNC_OFFSET_REG,
    &LIF_DATA_PORT_DATA_8_REG,
    &LIF_DATA_PORT_DATA_9_REG,
    &LIF_DATA_PORT_DATA_10_REG,
    &LIF_DATA_PORT_DATA_11_REG,
    &LIF_SEC_TX_SSCI_REG,
    &LIF_SEC_TX_SALT_0_REG,
    &LIF_SEC_TX_SALT_1_REG,
    &LIF_SEC_TX_SALT_2_REG,
    &LIF_SEC_RX_SSCI_REG,
    &LIF_SEC_RX_SALT_0_REG,
    &LIF_SEC_RX_SALT_1_REG,
    &LIF_SEC_RX_SALT_2_REG,
};

static unsigned long LIF_ADDRS[] =
{
    0x828c1800,
};

const ru_block_rec LIF_BLOCK = 
{
    "LIF",
    LIF_ADDRS,
    1,
    193,
    LIF_REGS
};

/* End of file EPON_LIF.c */
