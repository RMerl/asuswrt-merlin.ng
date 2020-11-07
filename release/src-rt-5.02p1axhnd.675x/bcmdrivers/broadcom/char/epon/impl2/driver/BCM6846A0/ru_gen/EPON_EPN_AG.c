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
 * Field: EPN_CONTROL_0_CFGEN1588TS
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_CFGEN1588TS_FIELD =
{
    "CFGEN1588TS",
#if RU_INCLUDE_DESC
    "",
    "Enable IEEE 1588 packet timestamping, applicable only in"
    "point-to-point mode.",
#endif
    EPN_CONTROL_0_CFGEN1588TS_FIELD_MASK,
    0,
    EPN_CONTROL_0_CFGEN1588TS_FIELD_WIDTH,
    EPN_CONTROL_0_CFGEN1588TS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_CFGREPLACEUPFCS
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_CFGREPLACEUPFCS_FIELD =
{
    "CFGREPLACEUPFCS",
#if RU_INCLUDE_DESC
    "",
    "Replaces FCS of upstream packet, resulting in no change in packet's"
    "length. Set cfgAppendUpFcs/cfgReplaceUpFcs to 0 for pass-through.",
#endif
    EPN_CONTROL_0_CFGREPLACEUPFCS_FIELD_MASK,
    0,
    EPN_CONTROL_0_CFGREPLACEUPFCS_FIELD_WIDTH,
    EPN_CONTROL_0_CFGREPLACEUPFCS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_CFGAPPENDUPFCS
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_CFGAPPENDUPFCS_FIELD =
{
    "CFGAPPENDUPFCS",
#if RU_INCLUDE_DESC
    "",
    "Appends FCS to upstream packet, resulting in increase of packet's"
    "length +4 bytes.  Set cfgAppendUpFcs/cfgReplaceUpFcs to 0 for"
    "pass-through.",
#endif
    EPN_CONTROL_0_CFGAPPENDUPFCS_FIELD_MASK,
    0,
    EPN_CONTROL_0_CFGAPPENDUPFCS_FIELD_WIDTH,
    EPN_CONTROL_0_CFGAPPENDUPFCS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_CFGDROPSCB
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_CFGDROPSCB_FIELD =
{
    "CFGDROPSCB",
#if RU_INCLUDE_DESC
    "",
    "Drops Single Copy Broadcast packets that are unmapped by the LIF."
    "0: Ignore LLID bit 15."
    "1: Drop packets that have LLID bit 15 set and have LLID index bit 5"
    "set.",
#endif
    EPN_CONTROL_0_CFGDROPSCB_FIELD_MASK,
    0,
    EPN_CONTROL_0_CFGDROPSCB_FIELD_WIDTH,
    EPN_CONTROL_0_CFGDROPSCB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_MODUNCAPPEDREPORTLIMIT
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_MODUNCAPPEDREPORTLIMIT_FIELD =
{
    "MODUNCAPPEDREPORTLIMIT",
#if RU_INCLUDE_DESC
    "",
    "Ignores the first queue set limits. This is only applied when"
    "Threshold First Queue Service Discipline is enabled."
    "0: Transmit packets in the order they were last reported in the"
    "first queue set."
    "1: Transmit packets in the order they were last reported in the"
    "un-capped queue set.",
#endif
    EPN_CONTROL_0_MODUNCAPPEDREPORTLIMIT_FIELD_MASK,
    0,
    EPN_CONTROL_0_MODUNCAPPEDREPORTLIMIT_FIELD_WIDTH,
    EPN_CONTROL_0_MODUNCAPPEDREPORTLIMIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_MODMPQUESETFIRST
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_MODMPQUESETFIRST_FIELD =
{
    "MODMPQUESETFIRST",
#if RU_INCLUDE_DESC
    "",
    "Enables Threshold First Queue Service Discipline.  This is only"
    "applied to multi-priority reporting mode."
    "0: Strict priority. The highest priority packet is transmitted."
    "This mode 'pulls' late-arriving/un-reported high priority packets"
    "ahead of previously reported lower priority packets."
    "1: Transmit packets in the order they were last reported.",
#endif
    EPN_CONTROL_0_MODMPQUESETFIRST_FIELD_MASK,
    0,
    EPN_CONTROL_0_MODMPQUESETFIRST_FIELD_WIDTH,
    EPN_CONTROL_0_MODMPQUESETFIRST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_CONTROL_0_RESERVED0_FIELD_MASK,
    0,
    EPN_CONTROL_0_RESERVED0_FIELD_WIDTH,
    EPN_CONTROL_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_PRVLOCALMPCPPROPAGATION
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_PRVLOCALMPCPPROPAGATION_FIELD =
{
    "PRVLOCALMPCPPROPAGATION",
#if RU_INCLUDE_DESC
    "",
    "Propagates the ONU's local MPCP time by replacing the last four"
    "bytes of a GATE frame with the MPCP time the GATE arrived at the"
    "ONU.  This only applies to downstream gate messages passed from the"
    "Epn to the BBH."
    "0: Do not propagate the local MPCP time"
    "1: Propagate the local MPCP time",
#endif
    EPN_CONTROL_0_PRVLOCALMPCPPROPAGATION_FIELD_MASK,
    0,
    EPN_CONTROL_0_PRVLOCALMPCPPROPAGATION_FIELD_WIDTH,
    EPN_CONTROL_0_PRVLOCALMPCPPROPAGATION_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_PRVTEKMODEPREFETCH
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_PRVTEKMODEPREFETCH_FIELD =
{
    "PRVTEKMODEPREFETCH",
#if RU_INCLUDE_DESC
    "",
    "Allows accumulator 3 to be prefetched before accumulator 0 is"
    "emptied and the accumulator shift occurs. This eliminates the race"
    "starting when the accumulators shift and their values are reported"
    "(i.e. accumulator 3 will fully represent the current queue state)."
    "0: Do not prefetch accumulator 3"
    "1: Prefetch accumulator 3",
#endif
    EPN_CONTROL_0_PRVTEKMODEPREFETCH_FIELD_MASK,
    0,
    EPN_CONTROL_0_PRVTEKMODEPREFETCH_FIELD_WIDTH,
    EPN_CONTROL_0_PRVTEKMODEPREFETCH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_RESERVED1
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_CONTROL_0_RESERVED1_FIELD_MASK,
    0,
    EPN_CONTROL_0_RESERVED1_FIELD_WIDTH,
    EPN_CONTROL_0_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_PRVNOUNMAPPPEDFCS
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_PRVNOUNMAPPPEDFCS_FIELD =
{
    "PRVNOUNMAPPPEDFCS",
#if RU_INCLUDE_DESC
    "",
    "Disables FCS checking of un-mapped frames. This is intended to be"
    "used when passing unmapped frames to a UNI port."
    "0: All FCS errored un-mapped frames are discarded"
    "1: All un-mapped frames are passed to a UNI port",
#endif
    EPN_CONTROL_0_PRVNOUNMAPPPEDFCS_FIELD_MASK,
    0,
    EPN_CONTROL_0_PRVNOUNMAPPPEDFCS_FIELD_WIDTH,
    EPN_CONTROL_0_PRVNOUNMAPPPEDFCS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_PRVSUPRESSDISCEN
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_PRVSUPRESSDISCEN_FIELD =
{
    "PRVSUPRESSDISCEN",
#if RU_INCLUDE_DESC
    "",
    "Causes discovery gates for empty queues to be discarded."
    "0: All discovery gates are processed"
    "1: Discovery gates for empty queues are discarded",
#endif
    EPN_CONTROL_0_PRVSUPRESSDISCEN_FIELD_MASK,
    0,
    EPN_CONTROL_0_PRVSUPRESSDISCEN_FIELD_WIDTH,
    EPN_CONTROL_0_PRVSUPRESSDISCEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_CFGVLANMAX
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_CFGVLANMAX_FIELD =
{
    "CFGVLANMAX",
#if RU_INCLUDE_DESC
    "",
    "Overrides the value in EPON Downstream Max Size Frame register."
    "0: Use Downstream Max Size Frame register value"
    "1: The maximum frame size for non-VLAN frames is 1518 and the"
    "maximum size for VLAN-tagged frames is 1522",
#endif
    EPN_CONTROL_0_CFGVLANMAX_FIELD_MASK,
    0,
    EPN_CONTROL_0_CFGVLANMAX_FIELD_WIDTH,
    EPN_CONTROL_0_CFGVLANMAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_FCSERRONLYDATAFR
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_FCSERRONLYDATAFR_FIELD =
{
    "FCSERRONLYDATAFR",
#if RU_INCLUDE_DESC
    "",
    "Determines which types of upstream frames are affected when forcing"
    "upstream FCS errors (as configured in EPON Force FCS Error register)"
    "0: Force FCS errors on all upstream frames."
    "1: Force FCS errors on user data frames only (not REPORT or"
    "processor frames).",
#endif
    EPN_CONTROL_0_FCSERRONLYDATAFR_FIELD_MASK,
    0,
    EPN_CONTROL_0_FCSERRONLYDATAFR_FIELD_WIDTH,
    EPN_CONTROL_0_FCSERRONLYDATAFR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_RESERVED2
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_CONTROL_0_RESERVED2_FIELD_MASK,
    0,
    EPN_CONTROL_0_RESERVED2_FIELD_WIDTH,
    EPN_CONTROL_0_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_PRVDROPUNMAPPPEDLLID
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_PRVDROPUNMAPPPEDLLID_FIELD =
{
    "PRVDROPUNMAPPPEDLLID",
#if RU_INCLUDE_DESC
    "",
    "Determines handling of traffic not mapped to a provisioned LLID"
    "0: Forward unmapped packets"
    "1: Drop unmapped packets",
#endif
    EPN_CONTROL_0_PRVDROPUNMAPPPEDLLID_FIELD_MASK,
    0,
    EPN_CONTROL_0_PRVDROPUNMAPPPEDLLID_FIELD_WIDTH,
    EPN_CONTROL_0_PRVDROPUNMAPPPEDLLID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_PRVSUPPRESSLLIDMODEBIT
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_PRVSUPPRESSLLIDMODEBIT_FIELD =
{
    "PRVSUPPRESSLLIDMODEBIT",
#if RU_INCLUDE_DESC
    "",
    "Controls LLID mode bit suppression"
    "0: LLID mode enabled"
    "1: Suppress LLID mode by masking bit-15 of the LLID",
#endif
    EPN_CONTROL_0_PRVSUPPRESSLLIDMODEBIT_FIELD_MASK,
    0,
    EPN_CONTROL_0_PRVSUPPRESSLLIDMODEBIT_FIELD_WIDTH,
    EPN_CONTROL_0_PRVSUPPRESSLLIDMODEBIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_MODDISCOVERYDAFILTEREN
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_MODDISCOVERYDAFILTEREN_FIELD =
{
    "MODDISCOVERYDAFILTEREN",
#if RU_INCLUDE_DESC
    "",
    "Discovery gate destination address filter enable."
    "0: Always ignore discovery gate DA value"
    "1: Process discovery gate only if the following criteria are met."
    "Please keep in mind the functionality provided by the legacy \"Drop"
    "Discovery Gate\" controls. No discovery gate will be processed if the"
    "discovery gate's LLID index has been provisioned to \"Drop Discovery"
    "Gates\". Also, only the first 8 LLID index values are eligible for"
    "discovery gate processing. Discovery gates with any other LLID index"
    "values received from the LIF will not be processed."
    "When this feature is disabled the discovery DA is ignored. When this"
    "feature is enabled there are four possible scenarios:"
    "(1) Received broadcast LLID (0x7FFF) and a unicast (not broadcast"
    "and not multicast) DA."
    "Discovery gate is processed if one of the provisioned ONT addresses"
    "must match the discovery gate's DA."
    "(2) Received broadcast LLID (0x7FFF) and a broadcast DA."
    "Discovery gate is processed. i.e., the discovery gate's DA is"
    "ignored."
    "(3) Received a non-broadcast LLID and a unicast DA."
    "Discovery gate is processed if the discovery gate's LLID index"
    "provisioned ONT address matches its DA."
    "(4) Received a non-broadcast LLID and a broadcast DA."
    "Discovery gate is processed. i.e., the discovery gate's DA is"
    "ignored."
    "Please remember the \"Drop Discovery Gate\" control takes precedence"
    "over all other configuration options",
#endif
    EPN_CONTROL_0_MODDISCOVERYDAFILTEREN_FIELD_MASK,
    0,
    EPN_CONTROL_0_MODDISCOVERYDAFILTEREN_FIELD_WIDTH,
    EPN_CONTROL_0_MODDISCOVERYDAFILTEREN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_RPTSELECT
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_RPTSELECT_FIELD =
{
    "RPTSELECT",
#if RU_INCLUDE_DESC
    "",
    "Selects the number of Queue Sets generated for all LLID Indexes"
    "0: Dual queue set  (default)"
    "1: Multi queue set ('Teknovus-style')"
    "Others: Reserved",
#endif
    EPN_CONTROL_0_RPTSELECT_FIELD_MASK,
    0,
    EPN_CONTROL_0_RPTSELECT_FIELD_WIDTH,
    EPN_CONTROL_0_RPTSELECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_PRVDISABLESVAQUESTATUSBP
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_PRVDISABLESVAQUESTATUSBP_FIELD =
{
    "PRVDISABLESVAQUESTATUSBP",
#if RU_INCLUDE_DESC
    "",
    "Disables Shaped Virtual Accumulator backpressure of BBH queue status"
    "interface."
    "0: SVA normal operation.  Allows shapers to backpressure the BBH"
    "queue status interface."
    "1: Disables Shaped Virtual Accumulator backpressure of BBH queue"
    "status interface..",
#endif
    EPN_CONTROL_0_PRVDISABLESVAQUESTATUSBP_FIELD_MASK,
    0,
    EPN_CONTROL_0_PRVDISABLESVAQUESTATUSBP_FIELD_WIDTH,
    EPN_CONTROL_0_PRVDISABLESVAQUESTATUSBP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_UTXLOOPBACK
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_UTXLOOPBACK_FIELD =
{
    "UTXLOOPBACK",
#if RU_INCLUDE_DESC
    "",
    "Places the upstream transmitter (UTX) in loopback mode."
    "0: UTX normal operation"
    "1: UTX is in loopback mode. This setting is also used for Point to"
    "Point mode (in conjunction with settings in the LIF Control"
    "register).",
#endif
    EPN_CONTROL_0_UTXLOOPBACK_FIELD_MASK,
    0,
    EPN_CONTROL_0_UTXLOOPBACK_FIELD_WIDTH,
    EPN_CONTROL_0_UTXLOOPBACK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_UTXEN
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_UTXEN_FIELD =
{
    "UTXEN",
#if RU_INCLUDE_DESC
    "",
    "UTX Enable bit."
    "0: Disable the UTX block"
    "1: Enable UTX operation",
#endif
    EPN_CONTROL_0_UTXEN_FIELD_MASK,
    0,
    EPN_CONTROL_0_UTXEN_FIELD_WIDTH,
    EPN_CONTROL_0_UTXEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_UTXRST_PRE_N
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_UTXRST_PRE_N_FIELD =
{
    "UTXRST_PRE_N",
#if RU_INCLUDE_DESC
    "",
    "Reset the EPN upstream transmitter (UTX) logic. Asserting (active"
    "low) this bit resets all UTX state machines and pointers. Note that"
    "this does not reset the UTX configuration registers."
    "0: Hold the UTX in reset"
    "1: Normal UTX operation",
#endif
    EPN_CONTROL_0_UTXRST_PRE_N_FIELD_MASK,
    0,
    EPN_CONTROL_0_UTXRST_PRE_N_FIELD_WIDTH,
    EPN_CONTROL_0_UTXRST_PRE_N_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_CFGDISABLEDNS
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_CFGDISABLEDNS_FIELD =
{
    "CFGDISABLEDNS",
#if RU_INCLUDE_DESC
    "",
    "Prevents any downstream traffic from being sent to the BBH. This"
    "control is only applied between downstream packets. So, it can be"
    "toggled any time the drxEn bit is set."
    "0: Normal operation."
    "1: No data is sent to BBH.",
#endif
    EPN_CONTROL_0_CFGDISABLEDNS_FIELD_MASK,
    0,
    EPN_CONTROL_0_CFGDISABLEDNS_FIELD_WIDTH,
    EPN_CONTROL_0_CFGDISABLEDNS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_DRXLOOPBACK
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_DRXLOOPBACK_FIELD =
{
    "DRXLOOPBACK",
#if RU_INCLUDE_DESC
    "",
    "Places the downstream receiver (DRX) in loopback mode. The loopback"
    "mode disables the EPON processing in EPN's downstream data path. It"
    "does not 'loopback' any data."
    "0: DRX normal operation"
    "1: DRX is in loopback mode",
#endif
    EPN_CONTROL_0_DRXLOOPBACK_FIELD_MASK,
    0,
    EPN_CONTROL_0_DRXLOOPBACK_FIELD_WIDTH,
    EPN_CONTROL_0_DRXLOOPBACK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_DRXEN
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_DRXEN_FIELD =
{
    "DRXEN",
#if RU_INCLUDE_DESC
    "",
    "DRX Enable bit."
    "0: Disable the DRX block"
    "1: Enable DRX operation",
#endif
    EPN_CONTROL_0_DRXEN_FIELD_MASK,
    0,
    EPN_CONTROL_0_DRXEN_FIELD_WIDTH,
    EPN_CONTROL_0_DRXEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_0_DRXRST_PRE_N
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_0_DRXRST_PRE_N_FIELD =
{
    "DRXRST_PRE_N",
#if RU_INCLUDE_DESC
    "",
    "Reset the EPN downstream receiver (DRX) logic. Asserting (active"
    "low) this bit resets all DRX state machines and pointers. Note that"
    "this does not reset the DRX configuration registers."
    "0: Hold the DRX in reset"
    "1: Normal DRX operation",
#endif
    EPN_CONTROL_0_DRXRST_PRE_N_FIELD_MASK,
    0,
    EPN_CONTROL_0_DRXRST_PRE_N_FIELD_WIDTH,
    EPN_CONTROL_0_DRXRST_PRE_N_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_1_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_CONTROL_1_RESERVED0_FIELD_MASK,
    0,
    EPN_CONTROL_1_RESERVED0_FIELD_WIDTH,
    EPN_CONTROL_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_1_CFGDISABLEMPCPCORRECTIONDITHERING
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_1_CFGDISABLEMPCPCORRECTIONDITHERING_FIELD =
{
    "CFGDISABLEMPCPCORRECTIONDITHERING",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set, the EPON MAC will not dither the MPCP"
    "correction values sent to the LIF. Default value is 0",
#endif
    EPN_CONTROL_1_CFGDISABLEMPCPCORRECTIONDITHERING_FIELD_MASK,
    0,
    EPN_CONTROL_1_CFGDISABLEMPCPCORRECTIONDITHERING_FIELD_WIDTH,
    EPN_CONTROL_1_CFGDISABLEMPCPCORRECTIONDITHERING_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_1_PRVOVERLAPPEDGNTENABLE
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_1_PRVOVERLAPPEDGNTENABLE_FIELD =
{
    "PRVOVERLAPPEDGNTENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enables adjustment of \"destructively overlapped\" grants."
    "Destructively overlapped grants are grants that overlap by more than"
    "the provisioned grant overhead (Lon + Loff + Sync time)."
    "0: Disable adjustment of destructively overlapped grants."
    "Destuctively overlapped grants are dropped and the GrantMisalign"
    "interrupt sets."
    "1: Destructively overlapped grants are adjusted to limit overlap to"
    "the grant overhead value (which maximizes the useful length of the"
    "earlier grant).",
#endif
    EPN_CONTROL_1_PRVOVERLAPPEDGNTENABLE_FIELD_MASK,
    0,
    EPN_CONTROL_1_PRVOVERLAPPEDGNTENABLE_FIELD_WIDTH,
    EPN_CONTROL_1_PRVOVERLAPPEDGNTENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_1_RSTMISALIGNTHR
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_1_RSTMISALIGNTHR_FIELD =
{
    "RSTMISALIGNTHR",
#if RU_INCLUDE_DESC
    "",
    "Reset the grant misalignment hardware",
#endif
    EPN_CONTROL_1_RSTMISALIGNTHR_FIELD_MASK,
    0,
    EPN_CONTROL_1_RSTMISALIGNTHR_FIELD_WIDTH,
    EPN_CONTROL_1_RSTMISALIGNTHR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_1_RESERVED1
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_1_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_CONTROL_1_RESERVED1_FIELD_MASK,
    0,
    EPN_CONTROL_1_RESERVED1_FIELD_WIDTH,
    EPN_CONTROL_1_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_1_CFGSTALEGNTCHK
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_1_CFGSTALEGNTCHK_FIELD =
{
    "CFGSTALEGNTCHK",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set, the EPON MAC will check the grant FIFOs for"
    "stale grants, and delete them. A stale grant has a grant start time"
    "that is smaller (earlier) than the local ONU time. Default value is"
    "1",
#endif
    EPN_CONTROL_1_CFGSTALEGNTCHK_FIELD_MASK,
    0,
    EPN_CONTROL_1_CFGSTALEGNTCHK_FIELD_WIDTH,
    EPN_CONTROL_1_CFGSTALEGNTCHK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_1_FECRPTEN
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_1_FECRPTEN_FIELD =
{
    "FECRPTEN",
#if RU_INCLUDE_DESC
    "",
    "Global upstream FEC enable. When this bit is set, the EPON MAC will"
    "take into account FEC overhead when generating report frames and"
    "filling grants. Please note that the per-LLID index bit must also be"
    "set.",
#endif
    EPN_CONTROL_1_FECRPTEN_FIELD_MASK,
    0,
    EPN_CONTROL_1_FECRPTEN_FIELD_WIDTH,
    EPN_CONTROL_1_FECRPTEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_1_RESERVED2
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_1_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_CONTROL_1_RESERVED2_FIELD_MASK,
    0,
    EPN_CONTROL_1_RESERVED2_FIELD_WIDTH,
    EPN_CONTROL_1_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_1_CFGL1L2TRUESTRICT
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_1_CFGL1L2TRUESTRICT_FIELD =
{
    "CFGL1L2TRUESTRICT",
#if RU_INCLUDE_DESC
    "",
    "Enables an alternate scheme in the L1-to-L2 strict-priority"
    "scheduler. This alternate scheme is useful only when both"
    "cfgSharedBurstCap and cfgSharedL2 are set (\"TK3715 CTC"
    "compatibility\" mode)."
    "Note: Support for this bit begins in Revision B0."
    "0: Default scheme."
    "1: Alternate mode. Use only in multi-priority mode when"
    "cfgSharedBurstCap is set.",
#endif
    EPN_CONTROL_1_CFGL1L2TRUESTRICT_FIELD_MASK,
    0,
    EPN_CONTROL_1_CFGL1L2TRUESTRICT_FIELD_WIDTH,
    EPN_CONTROL_1_CFGL1L2TRUESTRICT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_1_CFGCTCRPT
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_1_CFGCTCRPT_FIELD =
{
    "CFGCTCRPT",
#if RU_INCLUDE_DESC
    "",
    "Sets the number of priorities for multi-priority mode. The 8"
    "available L2s queues are sequentially mapped to the priorities"
    "within each LLID index. For example in mode \"01\": L2s queue 0 is"
    "mapped to LLID index 0 priority 0; L2s queue 1 is mapped to LLID"
    "index 0 priority 1; L2s queue 2 is mapped to LLID index 0 priority"
    "2;  L2s queue 3 is mapped to LLID index 1 priority 0; and so on."
    ""
    "00: Multi-priority mode disabled."
    "01: Two LLID indexes with 3 priorities each."
    "10: Two LLID indexes with 4 priorities each."
    "11: One LLID indexes with 8-priorities each.",
#endif
    EPN_CONTROL_1_CFGCTCRPT_FIELD_MASK,
    0,
    EPN_CONTROL_1_CFGCTCRPT_FIELD_WIDTH,
    EPN_CONTROL_1_CFGCTCRPT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_1_CFGTSCORRDIS
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_1_CFGTSCORRDIS_FIELD =
{
    "CFGTSCORRDIS",
#if RU_INCLUDE_DESC
    "",
    "Disables incremental (+1, -1) correction of local downstream MPCP"
    "time. When set, MPCP time is updated only when the difference"
    "between the local MPCP time and the timestamp received in an MPCPDU"
    "is different by greater than the EPN Time Stamp Differential"
    "register value.",
#endif
    EPN_CONTROL_1_CFGTSCORRDIS_FIELD_MASK,
    0,
    EPN_CONTROL_1_CFGTSCORRDIS_FIELD_WIDTH,
    EPN_CONTROL_1_CFGTSCORRDIS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_1_CFGNODISCRPT
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_1_CFGNODISCRPT_FIELD =
{
    "CFGNODISCRPT",
#if RU_INCLUDE_DESC
    "",
    "When set the ONU will ignore the force report bit on discovery"
    "frames.",
#endif
    EPN_CONTROL_1_CFGNODISCRPT_FIELD_MASK,
    0,
    EPN_CONTROL_1_CFGNODISCRPT_FIELD_WIDTH,
    EPN_CONTROL_1_CFGNODISCRPT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_1_DISABLEDISCSCALE
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_1_DISABLEDISCSCALE_FIELD =
{
    "DISABLEDISCSCALE",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set, the start time offset for a discovery response"
    "will be equal to the discovery seed. Units of the offset are 16-bit"
    "times.",
#endif
    EPN_CONTROL_1_DISABLEDISCSCALE_FIELD_MASK,
    0,
    EPN_CONTROL_1_DISABLEDISCSCALE_FIELD_WIDTH,
    EPN_CONTROL_1_DISABLEDISCSCALE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CONTROL_1_CLRONRD
 ******************************************************************************/
const ru_field_rec EPN_CONTROL_1_CLRONRD_FIELD =
{
    "CLRONRD",
#if RU_INCLUDE_DESC
    "",
    "All statistics RAM reads will clear the read location.",
#endif
    EPN_CONTROL_1_CLRONRD_FIELD_MASK,
    0,
    EPN_CONTROL_1_CLRONRD_FIELD_WIDTH,
    EPN_CONTROL_1_CLRONRD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_ENABLE_GRANTS_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_ENABLE_GRANTS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_ENABLE_GRANTS_RESERVED0_FIELD_MASK,
    0,
    EPN_ENABLE_GRANTS_RESERVED0_FIELD_WIDTH,
    EPN_ENABLE_GRANTS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_ENABLE_GRANTS_ENGNT7
 ******************************************************************************/
const ru_field_rec EPN_ENABLE_GRANTS_ENGNT7_FIELD =
{
    "ENGNT7",
#if RU_INCLUDE_DESC
    "",
    "Enable Grants on LLID Index 7."
    "Reset default is 1.",
#endif
    EPN_ENABLE_GRANTS_ENGNT7_FIELD_MASK,
    0,
    EPN_ENABLE_GRANTS_ENGNT7_FIELD_WIDTH,
    EPN_ENABLE_GRANTS_ENGNT7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_ENABLE_GRANTS_ENGNT6
 ******************************************************************************/
const ru_field_rec EPN_ENABLE_GRANTS_ENGNT6_FIELD =
{
    "ENGNT6",
#if RU_INCLUDE_DESC
    "",
    "Enable Grants on LLID Index 6."
    "Reset default is 1.",
#endif
    EPN_ENABLE_GRANTS_ENGNT6_FIELD_MASK,
    0,
    EPN_ENABLE_GRANTS_ENGNT6_FIELD_WIDTH,
    EPN_ENABLE_GRANTS_ENGNT6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_ENABLE_GRANTS_ENGNT5
 ******************************************************************************/
const ru_field_rec EPN_ENABLE_GRANTS_ENGNT5_FIELD =
{
    "ENGNT5",
#if RU_INCLUDE_DESC
    "",
    "Enable Grants on LLID Index 5."
    "Reset default is 1.",
#endif
    EPN_ENABLE_GRANTS_ENGNT5_FIELD_MASK,
    0,
    EPN_ENABLE_GRANTS_ENGNT5_FIELD_WIDTH,
    EPN_ENABLE_GRANTS_ENGNT5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_ENABLE_GRANTS_ENGNT4
 ******************************************************************************/
const ru_field_rec EPN_ENABLE_GRANTS_ENGNT4_FIELD =
{
    "ENGNT4",
#if RU_INCLUDE_DESC
    "",
    "Enable Grants on LLID Index 4."
    "Reset default is 1.",
#endif
    EPN_ENABLE_GRANTS_ENGNT4_FIELD_MASK,
    0,
    EPN_ENABLE_GRANTS_ENGNT4_FIELD_WIDTH,
    EPN_ENABLE_GRANTS_ENGNT4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_ENABLE_GRANTS_ENGNT3
 ******************************************************************************/
const ru_field_rec EPN_ENABLE_GRANTS_ENGNT3_FIELD =
{
    "ENGNT3",
#if RU_INCLUDE_DESC
    "",
    "Enable Grants on LLID Index 3."
    "Reset default is 1.",
#endif
    EPN_ENABLE_GRANTS_ENGNT3_FIELD_MASK,
    0,
    EPN_ENABLE_GRANTS_ENGNT3_FIELD_WIDTH,
    EPN_ENABLE_GRANTS_ENGNT3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_ENABLE_GRANTS_ENGNT2
 ******************************************************************************/
const ru_field_rec EPN_ENABLE_GRANTS_ENGNT2_FIELD =
{
    "ENGNT2",
#if RU_INCLUDE_DESC
    "",
    "Enable Grants on LLID Index 2."
    "Reset default is 1.",
#endif
    EPN_ENABLE_GRANTS_ENGNT2_FIELD_MASK,
    0,
    EPN_ENABLE_GRANTS_ENGNT2_FIELD_WIDTH,
    EPN_ENABLE_GRANTS_ENGNT2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_ENABLE_GRANTS_ENGNT1
 ******************************************************************************/
const ru_field_rec EPN_ENABLE_GRANTS_ENGNT1_FIELD =
{
    "ENGNT1",
#if RU_INCLUDE_DESC
    "",
    "Enable Grants on LLID Index 1."
    "Reset default is 1.",
#endif
    EPN_ENABLE_GRANTS_ENGNT1_FIELD_MASK,
    0,
    EPN_ENABLE_GRANTS_ENGNT1_FIELD_WIDTH,
    EPN_ENABLE_GRANTS_ENGNT1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_ENABLE_GRANTS_ENGNT0
 ******************************************************************************/
const ru_field_rec EPN_ENABLE_GRANTS_ENGNT0_FIELD =
{
    "ENGNT0",
#if RU_INCLUDE_DESC
    "",
    "Enable Grants on LLID Index 0."
    "Reset default is 1.",
#endif
    EPN_ENABLE_GRANTS_ENGNT0_FIELD_MASK,
    0,
    EPN_ENABLE_GRANTS_ENGNT0_FIELD_WIDTH,
    EPN_ENABLE_GRANTS_ENGNT0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DROP_DISC_GATES_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_DROP_DISC_GATES_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DROP_DISC_GATES_RESERVED0_FIELD_MASK,
    0,
    EPN_DROP_DISC_GATES_RESERVED0_FIELD_WIDTH,
    EPN_DROP_DISC_GATES_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DROP_DISC_GATES_SINKDISCGATES7
 ******************************************************************************/
const ru_field_rec EPN_DROP_DISC_GATES_SINKDISCGATES7_FIELD =
{
    "SINKDISCGATES7",
#if RU_INCLUDE_DESC
    "",
    "Discard Discovery Gates on LLID Index 7.",
#endif
    EPN_DROP_DISC_GATES_SINKDISCGATES7_FIELD_MASK,
    0,
    EPN_DROP_DISC_GATES_SINKDISCGATES7_FIELD_WIDTH,
    EPN_DROP_DISC_GATES_SINKDISCGATES7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DROP_DISC_GATES_SINKDISCGATES6
 ******************************************************************************/
const ru_field_rec EPN_DROP_DISC_GATES_SINKDISCGATES6_FIELD =
{
    "SINKDISCGATES6",
#if RU_INCLUDE_DESC
    "",
    "Discard Discovery Gates on LLID Index 6.",
#endif
    EPN_DROP_DISC_GATES_SINKDISCGATES6_FIELD_MASK,
    0,
    EPN_DROP_DISC_GATES_SINKDISCGATES6_FIELD_WIDTH,
    EPN_DROP_DISC_GATES_SINKDISCGATES6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DROP_DISC_GATES_SINKDISCGATES5
 ******************************************************************************/
const ru_field_rec EPN_DROP_DISC_GATES_SINKDISCGATES5_FIELD =
{
    "SINKDISCGATES5",
#if RU_INCLUDE_DESC
    "",
    "Discard Discovery Gates on LLID Index 5.",
#endif
    EPN_DROP_DISC_GATES_SINKDISCGATES5_FIELD_MASK,
    0,
    EPN_DROP_DISC_GATES_SINKDISCGATES5_FIELD_WIDTH,
    EPN_DROP_DISC_GATES_SINKDISCGATES5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DROP_DISC_GATES_SINKDISCGATES4
 ******************************************************************************/
const ru_field_rec EPN_DROP_DISC_GATES_SINKDISCGATES4_FIELD =
{
    "SINKDISCGATES4",
#if RU_INCLUDE_DESC
    "",
    "Discard Discovery Gates on LLID Index 4.",
#endif
    EPN_DROP_DISC_GATES_SINKDISCGATES4_FIELD_MASK,
    0,
    EPN_DROP_DISC_GATES_SINKDISCGATES4_FIELD_WIDTH,
    EPN_DROP_DISC_GATES_SINKDISCGATES4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DROP_DISC_GATES_SINKDISCGATES3
 ******************************************************************************/
const ru_field_rec EPN_DROP_DISC_GATES_SINKDISCGATES3_FIELD =
{
    "SINKDISCGATES3",
#if RU_INCLUDE_DESC
    "",
    "Discard Discovery Gates on LLID Index 3.",
#endif
    EPN_DROP_DISC_GATES_SINKDISCGATES3_FIELD_MASK,
    0,
    EPN_DROP_DISC_GATES_SINKDISCGATES3_FIELD_WIDTH,
    EPN_DROP_DISC_GATES_SINKDISCGATES3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DROP_DISC_GATES_SINKDISCGATES2
 ******************************************************************************/
const ru_field_rec EPN_DROP_DISC_GATES_SINKDISCGATES2_FIELD =
{
    "SINKDISCGATES2",
#if RU_INCLUDE_DESC
    "",
    "Discard Discovery Gates on LLID Index 2.",
#endif
    EPN_DROP_DISC_GATES_SINKDISCGATES2_FIELD_MASK,
    0,
    EPN_DROP_DISC_GATES_SINKDISCGATES2_FIELD_WIDTH,
    EPN_DROP_DISC_GATES_SINKDISCGATES2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DROP_DISC_GATES_SINKDISCGATES1
 ******************************************************************************/
const ru_field_rec EPN_DROP_DISC_GATES_SINKDISCGATES1_FIELD =
{
    "SINKDISCGATES1",
#if RU_INCLUDE_DESC
    "",
    "Discard Discovery Gates on LLID Index 1.",
#endif
    EPN_DROP_DISC_GATES_SINKDISCGATES1_FIELD_MASK,
    0,
    EPN_DROP_DISC_GATES_SINKDISCGATES1_FIELD_WIDTH,
    EPN_DROP_DISC_GATES_SINKDISCGATES1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DROP_DISC_GATES_SINKDISCGATES0
 ******************************************************************************/
const ru_field_rec EPN_DROP_DISC_GATES_SINKDISCGATES0_FIELD =
{
    "SINKDISCGATES0",
#if RU_INCLUDE_DESC
    "",
    "Discard Discovery Gates on LLID Index 0.",
#endif
    EPN_DROP_DISC_GATES_SINKDISCGATES0_FIELD_MASK,
    0,
    EPN_DROP_DISC_GATES_SINKDISCGATES0_FIELD_WIDTH,
    EPN_DROP_DISC_GATES_SINKDISCGATES0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DIS_FCS_CHK_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_DIS_FCS_CHK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DIS_FCS_CHK_RESERVED0_FIELD_MASK,
    0,
    EPN_DIS_FCS_CHK_RESERVED0_FIELD_WIDTH,
    EPN_DIS_FCS_CHK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DIS_FCS_CHK_DISABLEFCSCHKDN23
 ******************************************************************************/
const ru_field_rec EPN_DIS_FCS_CHK_DISABLEFCSCHKDN23_FIELD =
{
    "DISABLEFCSCHKDN23",
#if RU_INCLUDE_DESC
    "",
    "Do not check FCS on downstream frames received on downstream-only"
    "LLID index 23",
#endif
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN23_FIELD_MASK,
    0,
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN23_FIELD_WIDTH,
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN23_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DIS_FCS_CHK_DISABLEFCSCHKDN22
 ******************************************************************************/
const ru_field_rec EPN_DIS_FCS_CHK_DISABLEFCSCHKDN22_FIELD =
{
    "DISABLEFCSCHKDN22",
#if RU_INCLUDE_DESC
    "",
    "Do not check FCS on downstream frames received on downstream-only"
    "LLID index 22",
#endif
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN22_FIELD_MASK,
    0,
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN22_FIELD_WIDTH,
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN22_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DIS_FCS_CHK_DISABLEFCSCHKDN21
 ******************************************************************************/
const ru_field_rec EPN_DIS_FCS_CHK_DISABLEFCSCHKDN21_FIELD =
{
    "DISABLEFCSCHKDN21",
#if RU_INCLUDE_DESC
    "",
    "Do not check FCS on downstream frames received on downstream-only"
    "LLID index 21",
#endif
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN21_FIELD_MASK,
    0,
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN21_FIELD_WIDTH,
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN21_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DIS_FCS_CHK_DISABLEFCSCHKDN20
 ******************************************************************************/
const ru_field_rec EPN_DIS_FCS_CHK_DISABLEFCSCHKDN20_FIELD =
{
    "DISABLEFCSCHKDN20",
#if RU_INCLUDE_DESC
    "",
    "Do not check FCS on downstream frames received on downstream-only"
    "LLID index 20",
#endif
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN20_FIELD_MASK,
    0,
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN20_FIELD_WIDTH,
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN20_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DIS_FCS_CHK_DISABLEFCSCHKDN19
 ******************************************************************************/
const ru_field_rec EPN_DIS_FCS_CHK_DISABLEFCSCHKDN19_FIELD =
{
    "DISABLEFCSCHKDN19",
#if RU_INCLUDE_DESC
    "",
    "Do not check FCS on downstream frames received on downstream-only"
    "LLID index 19",
#endif
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN19_FIELD_MASK,
    0,
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN19_FIELD_WIDTH,
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN19_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DIS_FCS_CHK_DISABLEFCSCHKDN18
 ******************************************************************************/
const ru_field_rec EPN_DIS_FCS_CHK_DISABLEFCSCHKDN18_FIELD =
{
    "DISABLEFCSCHKDN18",
#if RU_INCLUDE_DESC
    "",
    "Do not check FCS on downstream frames received on downstream-only"
    "LLID index 18",
#endif
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN18_FIELD_MASK,
    0,
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN18_FIELD_WIDTH,
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN18_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DIS_FCS_CHK_DISABLEFCSCHKDN17
 ******************************************************************************/
const ru_field_rec EPN_DIS_FCS_CHK_DISABLEFCSCHKDN17_FIELD =
{
    "DISABLEFCSCHKDN17",
#if RU_INCLUDE_DESC
    "",
    "Do not check FCS on downstream frames received on downstream-only"
    "LLID index 17",
#endif
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN17_FIELD_MASK,
    0,
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN17_FIELD_WIDTH,
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN17_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DIS_FCS_CHK_DISABLEFCSCHKDN16
 ******************************************************************************/
const ru_field_rec EPN_DIS_FCS_CHK_DISABLEFCSCHKDN16_FIELD =
{
    "DISABLEFCSCHKDN16",
#if RU_INCLUDE_DESC
    "",
    "Do not check FCS on downstream frames received on downstream-only"
    "LLID index 16",
#endif
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN16_FIELD_MASK,
    0,
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN16_FIELD_WIDTH,
    EPN_DIS_FCS_CHK_DISABLEFCSCHKDN16_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DIS_FCS_CHK_RESERVED1
 ******************************************************************************/
const ru_field_rec EPN_DIS_FCS_CHK_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DIS_FCS_CHK_RESERVED1_FIELD_MASK,
    0,
    EPN_DIS_FCS_CHK_RESERVED1_FIELD_WIDTH,
    EPN_DIS_FCS_CHK_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DIS_FCS_CHK_DISABLEFCSCHK7
 ******************************************************************************/
const ru_field_rec EPN_DIS_FCS_CHK_DISABLEFCSCHK7_FIELD =
{
    "DISABLEFCSCHK7",
#if RU_INCLUDE_DESC
    "",
    "Do not check FCS on downstream frames received on LLID index 7",
#endif
    EPN_DIS_FCS_CHK_DISABLEFCSCHK7_FIELD_MASK,
    0,
    EPN_DIS_FCS_CHK_DISABLEFCSCHK7_FIELD_WIDTH,
    EPN_DIS_FCS_CHK_DISABLEFCSCHK7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DIS_FCS_CHK_DISABLEFCSCHK6
 ******************************************************************************/
const ru_field_rec EPN_DIS_FCS_CHK_DISABLEFCSCHK6_FIELD =
{
    "DISABLEFCSCHK6",
#if RU_INCLUDE_DESC
    "",
    "Do not check FCS on downstream frames received on LLID index 6",
#endif
    EPN_DIS_FCS_CHK_DISABLEFCSCHK6_FIELD_MASK,
    0,
    EPN_DIS_FCS_CHK_DISABLEFCSCHK6_FIELD_WIDTH,
    EPN_DIS_FCS_CHK_DISABLEFCSCHK6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DIS_FCS_CHK_DISABLEFCSCHK5
 ******************************************************************************/
const ru_field_rec EPN_DIS_FCS_CHK_DISABLEFCSCHK5_FIELD =
{
    "DISABLEFCSCHK5",
#if RU_INCLUDE_DESC
    "",
    "Do not check FCS on downstream frames received on LLID index 5",
#endif
    EPN_DIS_FCS_CHK_DISABLEFCSCHK5_FIELD_MASK,
    0,
    EPN_DIS_FCS_CHK_DISABLEFCSCHK5_FIELD_WIDTH,
    EPN_DIS_FCS_CHK_DISABLEFCSCHK5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DIS_FCS_CHK_DISABLEFCSCHK4
 ******************************************************************************/
const ru_field_rec EPN_DIS_FCS_CHK_DISABLEFCSCHK4_FIELD =
{
    "DISABLEFCSCHK4",
#if RU_INCLUDE_DESC
    "",
    "Do not check FCS on downstream frames received on LLID index 4",
#endif
    EPN_DIS_FCS_CHK_DISABLEFCSCHK4_FIELD_MASK,
    0,
    EPN_DIS_FCS_CHK_DISABLEFCSCHK4_FIELD_WIDTH,
    EPN_DIS_FCS_CHK_DISABLEFCSCHK4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DIS_FCS_CHK_DISABLEFCSCHK3
 ******************************************************************************/
const ru_field_rec EPN_DIS_FCS_CHK_DISABLEFCSCHK3_FIELD =
{
    "DISABLEFCSCHK3",
#if RU_INCLUDE_DESC
    "",
    "Do not check FCS on downstream frames received on LLID index 3",
#endif
    EPN_DIS_FCS_CHK_DISABLEFCSCHK3_FIELD_MASK,
    0,
    EPN_DIS_FCS_CHK_DISABLEFCSCHK3_FIELD_WIDTH,
    EPN_DIS_FCS_CHK_DISABLEFCSCHK3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DIS_FCS_CHK_DISABLEFCSCHK2
 ******************************************************************************/
const ru_field_rec EPN_DIS_FCS_CHK_DISABLEFCSCHK2_FIELD =
{
    "DISABLEFCSCHK2",
#if RU_INCLUDE_DESC
    "",
    "Do not check FCS on downstream frames received on LLID index 2",
#endif
    EPN_DIS_FCS_CHK_DISABLEFCSCHK2_FIELD_MASK,
    0,
    EPN_DIS_FCS_CHK_DISABLEFCSCHK2_FIELD_WIDTH,
    EPN_DIS_FCS_CHK_DISABLEFCSCHK2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DIS_FCS_CHK_DISABLEFCSCHK1
 ******************************************************************************/
const ru_field_rec EPN_DIS_FCS_CHK_DISABLEFCSCHK1_FIELD =
{
    "DISABLEFCSCHK1",
#if RU_INCLUDE_DESC
    "",
    "Do not check FCS on downstream frames received on LLID index 1",
#endif
    EPN_DIS_FCS_CHK_DISABLEFCSCHK1_FIELD_MASK,
    0,
    EPN_DIS_FCS_CHK_DISABLEFCSCHK1_FIELD_WIDTH,
    EPN_DIS_FCS_CHK_DISABLEFCSCHK1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DIS_FCS_CHK_DISABLEFCSCHK0
 ******************************************************************************/
const ru_field_rec EPN_DIS_FCS_CHK_DISABLEFCSCHK0_FIELD =
{
    "DISABLEFCSCHK0",
#if RU_INCLUDE_DESC
    "",
    "Do not check FCS on downstream frames received on LLID index 0",
#endif
    EPN_DIS_FCS_CHK_DISABLEFCSCHK0_FIELD_MASK,
    0,
    EPN_DIS_FCS_CHK_DISABLEFCSCHK0_FIELD_WIDTH,
    EPN_DIS_FCS_CHK_DISABLEFCSCHK0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_PASS_GATES_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_PASS_GATES_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_PASS_GATES_RESERVED0_FIELD_MASK,
    0,
    EPN_PASS_GATES_RESERVED0_FIELD_WIDTH,
    EPN_PASS_GATES_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_PASS_GATES_PASSGATELLID7
 ******************************************************************************/
const ru_field_rec EPN_PASS_GATES_PASSGATELLID7_FIELD =
{
    "PASSGATELLID7",
#if RU_INCLUDE_DESC
    "",
    "If set, downstream gate frames will be passed to the BBH for LLID 7",
#endif
    EPN_PASS_GATES_PASSGATELLID7_FIELD_MASK,
    0,
    EPN_PASS_GATES_PASSGATELLID7_FIELD_WIDTH,
    EPN_PASS_GATES_PASSGATELLID7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_PASS_GATES_PASSGATELLID6
 ******************************************************************************/
const ru_field_rec EPN_PASS_GATES_PASSGATELLID6_FIELD =
{
    "PASSGATELLID6",
#if RU_INCLUDE_DESC
    "",
    "If set, downstream gate frames will be passed to the BBH for LLID 6",
#endif
    EPN_PASS_GATES_PASSGATELLID6_FIELD_MASK,
    0,
    EPN_PASS_GATES_PASSGATELLID6_FIELD_WIDTH,
    EPN_PASS_GATES_PASSGATELLID6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_PASS_GATES_PASSGATELLID5
 ******************************************************************************/
const ru_field_rec EPN_PASS_GATES_PASSGATELLID5_FIELD =
{
    "PASSGATELLID5",
#if RU_INCLUDE_DESC
    "",
    "If set, downstream gate frames will be passed to the BBH for LLID 5",
#endif
    EPN_PASS_GATES_PASSGATELLID5_FIELD_MASK,
    0,
    EPN_PASS_GATES_PASSGATELLID5_FIELD_WIDTH,
    EPN_PASS_GATES_PASSGATELLID5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_PASS_GATES_PASSGATELLID4
 ******************************************************************************/
const ru_field_rec EPN_PASS_GATES_PASSGATELLID4_FIELD =
{
    "PASSGATELLID4",
#if RU_INCLUDE_DESC
    "",
    "If set, downstream gate frames will be passed to the BBH for LLID 4",
#endif
    EPN_PASS_GATES_PASSGATELLID4_FIELD_MASK,
    0,
    EPN_PASS_GATES_PASSGATELLID4_FIELD_WIDTH,
    EPN_PASS_GATES_PASSGATELLID4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_PASS_GATES_PASSGATELLID3
 ******************************************************************************/
const ru_field_rec EPN_PASS_GATES_PASSGATELLID3_FIELD =
{
    "PASSGATELLID3",
#if RU_INCLUDE_DESC
    "",
    "If set, downstream gate frames will be passed to the BBH for LLID 3",
#endif
    EPN_PASS_GATES_PASSGATELLID3_FIELD_MASK,
    0,
    EPN_PASS_GATES_PASSGATELLID3_FIELD_WIDTH,
    EPN_PASS_GATES_PASSGATELLID3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_PASS_GATES_PASSGATELLID2
 ******************************************************************************/
const ru_field_rec EPN_PASS_GATES_PASSGATELLID2_FIELD =
{
    "PASSGATELLID2",
#if RU_INCLUDE_DESC
    "",
    "If set, downstream gate frames will be passed to the BBH for LLID 2",
#endif
    EPN_PASS_GATES_PASSGATELLID2_FIELD_MASK,
    0,
    EPN_PASS_GATES_PASSGATELLID2_FIELD_WIDTH,
    EPN_PASS_GATES_PASSGATELLID2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_PASS_GATES_PASSGATELLID1
 ******************************************************************************/
const ru_field_rec EPN_PASS_GATES_PASSGATELLID1_FIELD =
{
    "PASSGATELLID1",
#if RU_INCLUDE_DESC
    "",
    "If set, downstream gate frames will be passed to the BBH for LLID 1",
#endif
    EPN_PASS_GATES_PASSGATELLID1_FIELD_MASK,
    0,
    EPN_PASS_GATES_PASSGATELLID1_FIELD_WIDTH,
    EPN_PASS_GATES_PASSGATELLID1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_PASS_GATES_PASSGATELLID0
 ******************************************************************************/
const ru_field_rec EPN_PASS_GATES_PASSGATELLID0_FIELD =
{
    "PASSGATELLID0",
#if RU_INCLUDE_DESC
    "",
    "If set, downstream gate frames will be passed to the BBH for LLID 0",
#endif
    EPN_PASS_GATES_PASSGATELLID0_FIELD_MASK,
    0,
    EPN_PASS_GATES_PASSGATELLID0_FIELD_WIDTH,
    EPN_PASS_GATES_PASSGATELLID0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CFG_MISALGN_FB_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_CFG_MISALGN_FB_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_CFG_MISALGN_FB_RESERVED0_FIELD_MASK,
    0,
    EPN_CFG_MISALGN_FB_RESERVED0_FIELD_WIDTH,
    EPN_CFG_MISALGN_FB_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK7
 ******************************************************************************/
const ru_field_rec EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK7_FIELD =
{
    "CFGMISALIGNFEEDBACK7",
#if RU_INCLUDE_DESC
    "",
    "0: Ignore misalignment condition"
    "1: Enable grant misalignment detection on LLID Index 7. When"
    "detected, EPON MAC will temporarily report empty queue status"
    "(REPORT frame) on the LLID Index.",
#endif
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK7_FIELD_MASK,
    0,
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK7_FIELD_WIDTH,
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK6
 ******************************************************************************/
const ru_field_rec EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK6_FIELD =
{
    "CFGMISALIGNFEEDBACK6",
#if RU_INCLUDE_DESC
    "",
    "0: Ignore misalignment condition"
    "1: Enable grant misalignment detection on LLID Index 6. When"
    "detected, EPON MAC will temporarily report empty queue status"
    "(REPORT frame) on the LLID Index.",
#endif
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK6_FIELD_MASK,
    0,
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK6_FIELD_WIDTH,
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK5
 ******************************************************************************/
const ru_field_rec EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK5_FIELD =
{
    "CFGMISALIGNFEEDBACK5",
#if RU_INCLUDE_DESC
    "",
    "0: Ignore misalignment condition"
    "1: Enable grant misalignment detection on LLID Index 5. When"
    "detected, EPON MAC will temporarily report empty queue status"
    "(REPORT frame) on the LLID Index.",
#endif
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK5_FIELD_MASK,
    0,
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK5_FIELD_WIDTH,
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK4
 ******************************************************************************/
const ru_field_rec EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK4_FIELD =
{
    "CFGMISALIGNFEEDBACK4",
#if RU_INCLUDE_DESC
    "",
    "0: Ignore misalignment condition"
    "1: Enable grant misalignment detection on LLID Index 4. When"
    "detected, EPON MAC will temporarily report empty queue status"
    "(REPORT frame) on the LLID Index.",
#endif
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK4_FIELD_MASK,
    0,
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK4_FIELD_WIDTH,
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK3
 ******************************************************************************/
const ru_field_rec EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK3_FIELD =
{
    "CFGMISALIGNFEEDBACK3",
#if RU_INCLUDE_DESC
    "",
    "0: Ignore misalignment condition"
    "1: Enable grant misalignment detection on LLID Index 3. When"
    "detected, EPON MAC will temporarily report empty queue status"
    "(REPORT frame) on the LLID Index.",
#endif
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK3_FIELD_MASK,
    0,
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK3_FIELD_WIDTH,
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK2
 ******************************************************************************/
const ru_field_rec EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK2_FIELD =
{
    "CFGMISALIGNFEEDBACK2",
#if RU_INCLUDE_DESC
    "",
    "0: Ignore misalignment condition"
    "1: Enable grant misalignment detection on LLID Index 2. When"
    "detected, EPON MAC will temporarily report empty queue status"
    "(REPORT frame) on the LLID Index.",
#endif
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK2_FIELD_MASK,
    0,
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK2_FIELD_WIDTH,
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK1
 ******************************************************************************/
const ru_field_rec EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK1_FIELD =
{
    "CFGMISALIGNFEEDBACK1",
#if RU_INCLUDE_DESC
    "",
    "0: Ignore misalignment condition"
    "1: Enable grant misalignment detection on LLID Index 1. When"
    "detected, EPON MAC will temporarily report empty queue status"
    "(REPORT frame) on the LLID Index.",
#endif
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK1_FIELD_MASK,
    0,
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK1_FIELD_WIDTH,
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK0
 ******************************************************************************/
const ru_field_rec EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK0_FIELD =
{
    "CFGMISALIGNFEEDBACK0",
#if RU_INCLUDE_DESC
    "",
    "0: Ignore misalignment condition"
    "1: Enable grant misalignment detection on LLID Index 0. When"
    "detected, EPON MAC will temporarily report empty queue status"
    "(REPORT frame) on the LLID Index.",
#endif
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK0_FIELD_MASK,
    0,
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK0_FIELD_WIDTH,
    EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DISCOVERY_FILTER_PRVDISCINFOMASK
 ******************************************************************************/
const ru_field_rec EPN_DISCOVERY_FILTER_PRVDISCINFOMASK_FIELD =
{
    "PRVDISCINFOMASK",
#if RU_INCLUDE_DESC
    "",
    "Any mask bit that is set will exclude its corresponding bit from the"
    "above comparison (i.e. set mask bits are considered \"don't care"
    "bits).",
#endif
    EPN_DISCOVERY_FILTER_PRVDISCINFOMASK_FIELD_MASK,
    0,
    EPN_DISCOVERY_FILTER_PRVDISCINFOMASK_FIELD_WIDTH,
    EPN_DISCOVERY_FILTER_PRVDISCINFOMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DISCOVERY_FILTER_PRVDISCINFOVALUE
 ******************************************************************************/
const ru_field_rec EPN_DISCOVERY_FILTER_PRVDISCINFOVALUE_FIELD =
{
    "PRVDISCINFOVALUE",
#if RU_INCLUDE_DESC
    "",
    "The value to match",
#endif
    EPN_DISCOVERY_FILTER_PRVDISCINFOVALUE_FIELD_MASK,
    0,
    EPN_DISCOVERY_FILTER_PRVDISCINFOVALUE_FIELD_WIDTH,
    EPN_DISCOVERY_FILTER_PRVDISCINFOVALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MINIMUM_GRANT_SETUP_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_MINIMUM_GRANT_SETUP_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MINIMUM_GRANT_SETUP_RESERVED0_FIELD_MASK,
    0,
    EPN_MINIMUM_GRANT_SETUP_RESERVED0_FIELD_WIDTH,
    EPN_MINIMUM_GRANT_SETUP_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MINIMUM_GRANT_SETUP_CFGMINGRANTSETUP
 ******************************************************************************/
const ru_field_rec EPN_MINIMUM_GRANT_SETUP_CFGMINGRANTSETUP_FIELD =
{
    "CFGMINGRANTSETUP",
#if RU_INCLUDE_DESC
    "",
    "Minimum amount of grant processing time required to guarantee the"
    "upstream data will be transmitted.  The units are EPON TimeQuanta"
    "(16 nS).",
#endif
    EPN_MINIMUM_GRANT_SETUP_CFGMINGRANTSETUP_FIELD_MASK,
    0,
    EPN_MINIMUM_GRANT_SETUP_CFGMINGRANTSETUP_FIELD_WIDTH,
    EPN_MINIMUM_GRANT_SETUP_CFGMINGRANTSETUP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_GNT_FIFO_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_RESET_GNT_FIFO_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_RESET_GNT_FIFO_RESERVED0_FIELD_MASK,
    0,
    EPN_RESET_GNT_FIFO_RESERVED0_FIELD_WIDTH,
    EPN_RESET_GNT_FIFO_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_GNT_FIFO_RSTGNTFIFO7
 ******************************************************************************/
const ru_field_rec EPN_RESET_GNT_FIFO_RSTGNTFIFO7_FIELD =
{
    "RSTGNTFIFO7",
#if RU_INCLUDE_DESC
    "",
    "Resets the read and write pointers for grant FIFO 7.",
#endif
    EPN_RESET_GNT_FIFO_RSTGNTFIFO7_FIELD_MASK,
    0,
    EPN_RESET_GNT_FIFO_RSTGNTFIFO7_FIELD_WIDTH,
    EPN_RESET_GNT_FIFO_RSTGNTFIFO7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_GNT_FIFO_RSTGNTFIFO6
 ******************************************************************************/
const ru_field_rec EPN_RESET_GNT_FIFO_RSTGNTFIFO6_FIELD =
{
    "RSTGNTFIFO6",
#if RU_INCLUDE_DESC
    "",
    "Resets the read and write pointers for grant FIFO 6.",
#endif
    EPN_RESET_GNT_FIFO_RSTGNTFIFO6_FIELD_MASK,
    0,
    EPN_RESET_GNT_FIFO_RSTGNTFIFO6_FIELD_WIDTH,
    EPN_RESET_GNT_FIFO_RSTGNTFIFO6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_GNT_FIFO_RSTGNTFIFO5
 ******************************************************************************/
const ru_field_rec EPN_RESET_GNT_FIFO_RSTGNTFIFO5_FIELD =
{
    "RSTGNTFIFO5",
#if RU_INCLUDE_DESC
    "",
    "Resets the read and write pointers for grant FIFO 5.",
#endif
    EPN_RESET_GNT_FIFO_RSTGNTFIFO5_FIELD_MASK,
    0,
    EPN_RESET_GNT_FIFO_RSTGNTFIFO5_FIELD_WIDTH,
    EPN_RESET_GNT_FIFO_RSTGNTFIFO5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_GNT_FIFO_RSTGNTFIFO4
 ******************************************************************************/
const ru_field_rec EPN_RESET_GNT_FIFO_RSTGNTFIFO4_FIELD =
{
    "RSTGNTFIFO4",
#if RU_INCLUDE_DESC
    "",
    "Resets the read and write pointers for grant FIFO 4.",
#endif
    EPN_RESET_GNT_FIFO_RSTGNTFIFO4_FIELD_MASK,
    0,
    EPN_RESET_GNT_FIFO_RSTGNTFIFO4_FIELD_WIDTH,
    EPN_RESET_GNT_FIFO_RSTGNTFIFO4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_GNT_FIFO_RSTGNTFIFO3
 ******************************************************************************/
const ru_field_rec EPN_RESET_GNT_FIFO_RSTGNTFIFO3_FIELD =
{
    "RSTGNTFIFO3",
#if RU_INCLUDE_DESC
    "",
    "Resets the read and write pointers for grant FIFO 3.",
#endif
    EPN_RESET_GNT_FIFO_RSTGNTFIFO3_FIELD_MASK,
    0,
    EPN_RESET_GNT_FIFO_RSTGNTFIFO3_FIELD_WIDTH,
    EPN_RESET_GNT_FIFO_RSTGNTFIFO3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_GNT_FIFO_RSTGNTFIFO2
 ******************************************************************************/
const ru_field_rec EPN_RESET_GNT_FIFO_RSTGNTFIFO2_FIELD =
{
    "RSTGNTFIFO2",
#if RU_INCLUDE_DESC
    "",
    "Resets the read and write pointers for grant FIFO 2.",
#endif
    EPN_RESET_GNT_FIFO_RSTGNTFIFO2_FIELD_MASK,
    0,
    EPN_RESET_GNT_FIFO_RSTGNTFIFO2_FIELD_WIDTH,
    EPN_RESET_GNT_FIFO_RSTGNTFIFO2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_GNT_FIFO_RSTGNTFIFO1
 ******************************************************************************/
const ru_field_rec EPN_RESET_GNT_FIFO_RSTGNTFIFO1_FIELD =
{
    "RSTGNTFIFO1",
#if RU_INCLUDE_DESC
    "",
    "Resets the read and write pointers for grant FIFO 1.",
#endif
    EPN_RESET_GNT_FIFO_RSTGNTFIFO1_FIELD_MASK,
    0,
    EPN_RESET_GNT_FIFO_RSTGNTFIFO1_FIELD_WIDTH,
    EPN_RESET_GNT_FIFO_RSTGNTFIFO1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_GNT_FIFO_RSTGNTFIFO0
 ******************************************************************************/
const ru_field_rec EPN_RESET_GNT_FIFO_RSTGNTFIFO0_FIELD =
{
    "RSTGNTFIFO0",
#if RU_INCLUDE_DESC
    "",
    "Resets the read and write pointers for grant FIFO 0.",
#endif
    EPN_RESET_GNT_FIFO_RSTGNTFIFO0_FIELD_MASK,
    0,
    EPN_RESET_GNT_FIFO_RSTGNTFIFO0_FIELD_WIDTH,
    EPN_RESET_GNT_FIFO_RSTGNTFIFO0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_L1_ACCUMULATOR_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_RESET_L1_ACCUMULATOR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_RESET_L1_ACCUMULATOR_RESERVED0_FIELD_MASK,
    0,
    EPN_RESET_L1_ACCUMULATOR_RESERVED0_FIELD_WIDTH,
    EPN_RESET_L1_ACCUMULATOR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_L1_ACCUMULATOR_CFGL1SCLRACUM
 ******************************************************************************/
const ru_field_rec EPN_RESET_L1_ACCUMULATOR_CFGL1SCLRACUM_FIELD =
{
    "CFGL1SCLRACUM",
#if RU_INCLUDE_DESC
    "",
    "Set the respective bit(s) to reset L1 accumulator(s).",
#endif
    EPN_RESET_L1_ACCUMULATOR_CFGL1SCLRACUM_FIELD_MASK,
    0,
    EPN_RESET_L1_ACCUMULATOR_CFGL1SCLRACUM_FIELD_WIDTH,
    EPN_RESET_L1_ACCUMULATOR_CFGL1SCLRACUM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_L1_ACCUMULATOR_SEL_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_L1_ACCUMULATOR_SEL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_L1_ACCUMULATOR_SEL_RESERVED0_FIELD_MASK,
    0,
    EPN_L1_ACCUMULATOR_SEL_RESERVED0_FIELD_WIDTH,
    EPN_L1_ACCUMULATOR_SEL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_L1_ACCUMULATOR_SEL_CFGL1SUVASIZESEL
 ******************************************************************************/
const ru_field_rec EPN_L1_ACCUMULATOR_SEL_CFGL1SUVASIZESEL_FIELD =
{
    "CFGL1SUVASIZESEL",
#if RU_INCLUDE_DESC
    "",
    "Selects which L1S Un-shaped Virtual Accumulator size will be"
    "reported.",
#endif
    EPN_L1_ACCUMULATOR_SEL_CFGL1SUVASIZESEL_FIELD_MASK,
    0,
    EPN_L1_ACCUMULATOR_SEL_CFGL1SUVASIZESEL_FIELD_WIDTH,
    EPN_L1_ACCUMULATOR_SEL_CFGL1SUVASIZESEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_L1_ACCUMULATOR_SEL_CFGL1SSVASIZESEL
 ******************************************************************************/
const ru_field_rec EPN_L1_ACCUMULATOR_SEL_CFGL1SSVASIZESEL_FIELD =
{
    "CFGL1SSVASIZESEL",
#if RU_INCLUDE_DESC
    "",
    "Selects which L1S Shaped Virtual Accumulator size will be reported.",
#endif
    EPN_L1_ACCUMULATOR_SEL_CFGL1SSVASIZESEL_FIELD_MASK,
    0,
    EPN_L1_ACCUMULATOR_SEL_CFGL1SSVASIZESEL_FIELD_WIDTH,
    EPN_L1_ACCUMULATOR_SEL_CFGL1SSVASIZESEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_L1_SVA_BYTES_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_L1_SVA_BYTES_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_L1_SVA_BYTES_RESERVED0_FIELD_MASK,
    0,
    EPN_L1_SVA_BYTES_RESERVED0_FIELD_WIDTH,
    EPN_L1_SVA_BYTES_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_L1_SVA_BYTES_L1SSVASIZE
 ******************************************************************************/
const ru_field_rec EPN_L1_SVA_BYTES_L1SSVASIZE_FIELD =
{
    "L1SSVASIZE",
#if RU_INCLUDE_DESC
    "",
    "Signed number of bytes in the selected L1S Shaped Virtual"
    "Accumulator."
    "Bit-29 is the sign bit.  A negative number indicates the Runner/BBH"
    "created a rounding error"
    "Bit-28 can be considered an overflow indication."
    "Bits 27-0 are the actual number of bytes.",
#endif
    EPN_L1_SVA_BYTES_L1SSVASIZE_FIELD_MASK,
    0,
    EPN_L1_SVA_BYTES_L1SSVASIZE_FIELD_WIDTH,
    EPN_L1_SVA_BYTES_L1SSVASIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_L1_UVA_BYTES_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_L1_UVA_BYTES_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_L1_UVA_BYTES_RESERVED0_FIELD_MASK,
    0,
    EPN_L1_UVA_BYTES_RESERVED0_FIELD_WIDTH,
    EPN_L1_UVA_BYTES_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_L1_UVA_BYTES_L1SUVASIZE
 ******************************************************************************/
const ru_field_rec EPN_L1_UVA_BYTES_L1SUVASIZE_FIELD =
{
    "L1SUVASIZE",
#if RU_INCLUDE_DESC
    "",
    "Signed number of bytes in the selected L1S Un-shaped Virtual"
    "Accumulator."
    "Bit-29 is the sign bit.  A negative number indicates the Runner/BBH"
    "created a rounding error"
    "Bit-28 can be considered an overflow indication."
    "Bits 27-0 are the actual number of bytes.",
#endif
    EPN_L1_UVA_BYTES_L1SUVASIZE_FIELD_MASK,
    0,
    EPN_L1_UVA_BYTES_L1SUVASIZE_FIELD_WIDTH,
    EPN_L1_UVA_BYTES_L1SUVASIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_L1_SVA_OVERFLOW_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_L1_SVA_OVERFLOW_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_L1_SVA_OVERFLOW_RESERVED0_FIELD_MASK,
    0,
    EPN_L1_SVA_OVERFLOW_RESERVED0_FIELD_WIDTH,
    EPN_L1_SVA_OVERFLOW_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_L1_SVA_OVERFLOW_L1SSVAOVERFLOW
 ******************************************************************************/
const ru_field_rec EPN_L1_SVA_OVERFLOW_L1SSVAOVERFLOW_FIELD =
{
    "L1SSVAOVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "Indicates which SVAs have overflowed.  The overflow can only be"
    "corrected by reset.",
#endif
    EPN_L1_SVA_OVERFLOW_L1SSVAOVERFLOW_FIELD_MASK,
    0,
    EPN_L1_SVA_OVERFLOW_L1SSVAOVERFLOW_FIELD_WIDTH,
    EPN_L1_SVA_OVERFLOW_L1SSVAOVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_L1_UVA_OVERFLOW_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_L1_UVA_OVERFLOW_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_L1_UVA_OVERFLOW_RESERVED0_FIELD_MASK,
    0,
    EPN_L1_UVA_OVERFLOW_RESERVED0_FIELD_WIDTH,
    EPN_L1_UVA_OVERFLOW_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_L1_UVA_OVERFLOW_L1SUVAOVERFLOW
 ******************************************************************************/
const ru_field_rec EPN_L1_UVA_OVERFLOW_L1SUVAOVERFLOW_FIELD =
{
    "L1SUVAOVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "Indicates which UVAs have overflowed.  The overflow can only be"
    "corrected by reset.",
#endif
    EPN_L1_UVA_OVERFLOW_L1SUVAOVERFLOW_FIELD_MASK,
    0,
    EPN_L1_UVA_OVERFLOW_L1SUVAOVERFLOW_FIELD_WIDTH,
    EPN_L1_UVA_OVERFLOW_L1SUVAOVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_RESET_RPT_PRI_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_RESET_RPT_PRI_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_RESET_RPT_PRI_RESERVED0_FIELD_MASK,
    0,
    EPN_RESET_RPT_PRI_RESERVED0_FIELD_WIDTH,
    EPN_RESET_RPT_PRI_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_RPT_PRI_NULLRPTPRI15
 ******************************************************************************/
const ru_field_rec EPN_RESET_RPT_PRI_NULLRPTPRI15_FIELD =
{
    "NULLRPTPRI15",
#if RU_INCLUDE_DESC
    "",
    "Force priority 15 report values to zero.",
#endif
    EPN_RESET_RPT_PRI_NULLRPTPRI15_FIELD_MASK,
    0,
    EPN_RESET_RPT_PRI_NULLRPTPRI15_FIELD_WIDTH,
    EPN_RESET_RPT_PRI_NULLRPTPRI15_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_RPT_PRI_NULLRPTPRI14
 ******************************************************************************/
const ru_field_rec EPN_RESET_RPT_PRI_NULLRPTPRI14_FIELD =
{
    "NULLRPTPRI14",
#if RU_INCLUDE_DESC
    "",
    "Force priority 14 report values to zero.",
#endif
    EPN_RESET_RPT_PRI_NULLRPTPRI14_FIELD_MASK,
    0,
    EPN_RESET_RPT_PRI_NULLRPTPRI14_FIELD_WIDTH,
    EPN_RESET_RPT_PRI_NULLRPTPRI14_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_RPT_PRI_NULLRPTPRI13
 ******************************************************************************/
const ru_field_rec EPN_RESET_RPT_PRI_NULLRPTPRI13_FIELD =
{
    "NULLRPTPRI13",
#if RU_INCLUDE_DESC
    "",
    "Force priority 13 report values to zero.",
#endif
    EPN_RESET_RPT_PRI_NULLRPTPRI13_FIELD_MASK,
    0,
    EPN_RESET_RPT_PRI_NULLRPTPRI13_FIELD_WIDTH,
    EPN_RESET_RPT_PRI_NULLRPTPRI13_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_RPT_PRI_NULLRPTPRI12
 ******************************************************************************/
const ru_field_rec EPN_RESET_RPT_PRI_NULLRPTPRI12_FIELD =
{
    "NULLRPTPRI12",
#if RU_INCLUDE_DESC
    "",
    "Force priority 12 report values to zero.",
#endif
    EPN_RESET_RPT_PRI_NULLRPTPRI12_FIELD_MASK,
    0,
    EPN_RESET_RPT_PRI_NULLRPTPRI12_FIELD_WIDTH,
    EPN_RESET_RPT_PRI_NULLRPTPRI12_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_RPT_PRI_NULLRPTPRI11
 ******************************************************************************/
const ru_field_rec EPN_RESET_RPT_PRI_NULLRPTPRI11_FIELD =
{
    "NULLRPTPRI11",
#if RU_INCLUDE_DESC
    "",
    "Force priority 11 report values to zero.",
#endif
    EPN_RESET_RPT_PRI_NULLRPTPRI11_FIELD_MASK,
    0,
    EPN_RESET_RPT_PRI_NULLRPTPRI11_FIELD_WIDTH,
    EPN_RESET_RPT_PRI_NULLRPTPRI11_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_RPT_PRI_NULLRPTPRI10
 ******************************************************************************/
const ru_field_rec EPN_RESET_RPT_PRI_NULLRPTPRI10_FIELD =
{
    "NULLRPTPRI10",
#if RU_INCLUDE_DESC
    "",
    "Force priority 10 report values to zero.",
#endif
    EPN_RESET_RPT_PRI_NULLRPTPRI10_FIELD_MASK,
    0,
    EPN_RESET_RPT_PRI_NULLRPTPRI10_FIELD_WIDTH,
    EPN_RESET_RPT_PRI_NULLRPTPRI10_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_RPT_PRI_NULLRPTPRI9
 ******************************************************************************/
const ru_field_rec EPN_RESET_RPT_PRI_NULLRPTPRI9_FIELD =
{
    "NULLRPTPRI9",
#if RU_INCLUDE_DESC
    "",
    "Force priority 9 report values to zero.",
#endif
    EPN_RESET_RPT_PRI_NULLRPTPRI9_FIELD_MASK,
    0,
    EPN_RESET_RPT_PRI_NULLRPTPRI9_FIELD_WIDTH,
    EPN_RESET_RPT_PRI_NULLRPTPRI9_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_RPT_PRI_NULLRPTPRI8
 ******************************************************************************/
const ru_field_rec EPN_RESET_RPT_PRI_NULLRPTPRI8_FIELD =
{
    "NULLRPTPRI8",
#if RU_INCLUDE_DESC
    "",
    "Force priority 8 report values to zero.",
#endif
    EPN_RESET_RPT_PRI_NULLRPTPRI8_FIELD_MASK,
    0,
    EPN_RESET_RPT_PRI_NULLRPTPRI8_FIELD_WIDTH,
    EPN_RESET_RPT_PRI_NULLRPTPRI8_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_RPT_PRI_NULLRPTPRI7
 ******************************************************************************/
const ru_field_rec EPN_RESET_RPT_PRI_NULLRPTPRI7_FIELD =
{
    "NULLRPTPRI7",
#if RU_INCLUDE_DESC
    "",
    "Force priority 7 report values to zero.",
#endif
    EPN_RESET_RPT_PRI_NULLRPTPRI7_FIELD_MASK,
    0,
    EPN_RESET_RPT_PRI_NULLRPTPRI7_FIELD_WIDTH,
    EPN_RESET_RPT_PRI_NULLRPTPRI7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_RPT_PRI_NULLRPTPRI6
 ******************************************************************************/
const ru_field_rec EPN_RESET_RPT_PRI_NULLRPTPRI6_FIELD =
{
    "NULLRPTPRI6",
#if RU_INCLUDE_DESC
    "",
    "Force priority 6 report values to zero.",
#endif
    EPN_RESET_RPT_PRI_NULLRPTPRI6_FIELD_MASK,
    0,
    EPN_RESET_RPT_PRI_NULLRPTPRI6_FIELD_WIDTH,
    EPN_RESET_RPT_PRI_NULLRPTPRI6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_RPT_PRI_NULLRPTPRI5
 ******************************************************************************/
const ru_field_rec EPN_RESET_RPT_PRI_NULLRPTPRI5_FIELD =
{
    "NULLRPTPRI5",
#if RU_INCLUDE_DESC
    "",
    "Force priority 5 report values to zero.",
#endif
    EPN_RESET_RPT_PRI_NULLRPTPRI5_FIELD_MASK,
    0,
    EPN_RESET_RPT_PRI_NULLRPTPRI5_FIELD_WIDTH,
    EPN_RESET_RPT_PRI_NULLRPTPRI5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_RPT_PRI_NULLRPTPRI4
 ******************************************************************************/
const ru_field_rec EPN_RESET_RPT_PRI_NULLRPTPRI4_FIELD =
{
    "NULLRPTPRI4",
#if RU_INCLUDE_DESC
    "",
    "Force priority 4 report values to zero.",
#endif
    EPN_RESET_RPT_PRI_NULLRPTPRI4_FIELD_MASK,
    0,
    EPN_RESET_RPT_PRI_NULLRPTPRI4_FIELD_WIDTH,
    EPN_RESET_RPT_PRI_NULLRPTPRI4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_RPT_PRI_NULLRPTPRI3
 ******************************************************************************/
const ru_field_rec EPN_RESET_RPT_PRI_NULLRPTPRI3_FIELD =
{
    "NULLRPTPRI3",
#if RU_INCLUDE_DESC
    "",
    "Force priority 3 report values to zero.",
#endif
    EPN_RESET_RPT_PRI_NULLRPTPRI3_FIELD_MASK,
    0,
    EPN_RESET_RPT_PRI_NULLRPTPRI3_FIELD_WIDTH,
    EPN_RESET_RPT_PRI_NULLRPTPRI3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_RPT_PRI_NULLRPTPRI2
 ******************************************************************************/
const ru_field_rec EPN_RESET_RPT_PRI_NULLRPTPRI2_FIELD =
{
    "NULLRPTPRI2",
#if RU_INCLUDE_DESC
    "",
    "Force priority 2 report values to zero.",
#endif
    EPN_RESET_RPT_PRI_NULLRPTPRI2_FIELD_MASK,
    0,
    EPN_RESET_RPT_PRI_NULLRPTPRI2_FIELD_WIDTH,
    EPN_RESET_RPT_PRI_NULLRPTPRI2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_RPT_PRI_NULLRPTPRI1
 ******************************************************************************/
const ru_field_rec EPN_RESET_RPT_PRI_NULLRPTPRI1_FIELD =
{
    "NULLRPTPRI1",
#if RU_INCLUDE_DESC
    "",
    "Force priority 1 report values to zero.",
#endif
    EPN_RESET_RPT_PRI_NULLRPTPRI1_FIELD_MASK,
    0,
    EPN_RESET_RPT_PRI_NULLRPTPRI1_FIELD_WIDTH,
    EPN_RESET_RPT_PRI_NULLRPTPRI1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_RPT_PRI_NULLRPTPRI0
 ******************************************************************************/
const ru_field_rec EPN_RESET_RPT_PRI_NULLRPTPRI0_FIELD =
{
    "NULLRPTPRI0",
#if RU_INCLUDE_DESC
    "",
    "Force priority 0 report values to zero.",
#endif
    EPN_RESET_RPT_PRI_NULLRPTPRI0_FIELD_MASK,
    0,
    EPN_RESET_RPT_PRI_NULLRPTPRI0_FIELD_WIDTH,
    EPN_RESET_RPT_PRI_NULLRPTPRI0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_L2_RPT_FIFO_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_RESET_L2_RPT_FIFO_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_RESET_L2_RPT_FIFO_RESERVED0_FIELD_MASK,
    0,
    EPN_RESET_L2_RPT_FIFO_RESERVED0_FIELD_WIDTH,
    EPN_RESET_L2_RPT_FIFO_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RESET_L2_RPT_FIFO_CFGL2SCLRQUE
 ******************************************************************************/
const ru_field_rec EPN_RESET_L2_RPT_FIFO_CFGL2SCLRQUE_FIELD =
{
    "CFGL2SCLRQUE",
#if RU_INCLUDE_DESC
    "",
    "Set the respective bit(s) to reset L2 FIFO(s).",
#endif
    EPN_RESET_L2_RPT_FIFO_CFGL2SCLRQUE_FIELD_MASK,
    0,
    EPN_RESET_L2_RPT_FIFO_CFGL2SCLRQUE_FIELD_WIDTH,
    EPN_RESET_L2_RPT_FIFO_CFGL2SCLRQUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_ENABLE_UPSTREAM_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_ENABLE_UPSTREAM_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_ENABLE_UPSTREAM_RESERVED0_FIELD_MASK,
    0,
    EPN_ENABLE_UPSTREAM_RESERVED0_FIELD_WIDTH,
    EPN_ENABLE_UPSTREAM_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_ENABLE_UPSTREAM_CFGENABLEUPSTREAMREG
 ******************************************************************************/
const ru_field_rec EPN_ENABLE_UPSTREAM_CFGENABLEUPSTREAMREG_FIELD =
{
    "CFGENABLEUPSTREAMREG",
#if RU_INCLUDE_DESC
    "",
    "Set the respective bit(s) to enable the upstream LLID(s).",
#endif
    EPN_ENABLE_UPSTREAM_CFGENABLEUPSTREAMREG_FIELD_MASK,
    0,
    EPN_ENABLE_UPSTREAM_CFGENABLEUPSTREAMREG_FIELD_WIDTH,
    EPN_ENABLE_UPSTREAM_CFGENABLEUPSTREAMREG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_ENABLE_UPSTREAM_FB_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_ENABLE_UPSTREAM_FB_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_ENABLE_UPSTREAM_FB_RESERVED0_FIELD_MASK,
    0,
    EPN_ENABLE_UPSTREAM_FB_RESERVED0_FIELD_WIDTH,
    EPN_ENABLE_UPSTREAM_FB_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_ENABLE_UPSTREAM_FB_CFGENABLEUPSTREAMFEEDBACK
 ******************************************************************************/
const ru_field_rec EPN_ENABLE_UPSTREAM_FB_CFGENABLEUPSTREAMFEEDBACK_FIELD =
{
    "CFGENABLEUPSTREAMFEEDBACK",
#if RU_INCLUDE_DESC
    "",
    "Indicates the operational state of the upstream LLIDs.  See"
    "EPN_ENABLE_UPSTREAM register description for details.",
#endif
    EPN_ENABLE_UPSTREAM_FB_CFGENABLEUPSTREAMFEEDBACK_FIELD_MASK,
    0,
    EPN_ENABLE_UPSTREAM_FB_CFGENABLEUPSTREAMFEEDBACK_FIELD_WIDTH,
    EPN_ENABLE_UPSTREAM_FB_CFGENABLEUPSTREAMFEEDBACK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_ENABLE_UPSTREAM_FEC_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_ENABLE_UPSTREAM_FEC_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_ENABLE_UPSTREAM_FEC_RESERVED0_FIELD_MASK,
    0,
    EPN_ENABLE_UPSTREAM_FEC_RESERVED0_FIELD_WIDTH,
    EPN_ENABLE_UPSTREAM_FEC_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_ENABLE_UPSTREAM_FEC_CFGENABLEUPSTREAMFEC
 ******************************************************************************/
const ru_field_rec EPN_ENABLE_UPSTREAM_FEC_CFGENABLEUPSTREAMFEC_FIELD =
{
    "CFGENABLEUPSTREAMFEC",
#if RU_INCLUDE_DESC
    "",
    "Set the respective bit(s) to enable upstream FEC for LLID(s).",
#endif
    EPN_ENABLE_UPSTREAM_FEC_CFGENABLEUPSTREAMFEC_FIELD_MASK,
    0,
    EPN_ENABLE_UPSTREAM_FEC_CFGENABLEUPSTREAMFEC_FIELD_WIDTH,
    EPN_ENABLE_UPSTREAM_FEC_CFGENABLEUPSTREAMFEC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_REPORT_BYTE_LENGTH_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_REPORT_BYTE_LENGTH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_REPORT_BYTE_LENGTH_RESERVED0_FIELD_MASK,
    0,
    EPN_REPORT_BYTE_LENGTH_RESERVED0_FIELD_WIDTH,
    EPN_REPORT_BYTE_LENGTH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_REPORT_BYTE_LENGTH_PRVRPTBYTELEN
 ******************************************************************************/
const ru_field_rec EPN_REPORT_BYTE_LENGTH_PRVRPTBYTELEN_FIELD =
{
    "PRVRPTBYTELEN",
#if RU_INCLUDE_DESC
    "",
    "Number of bytes reserved for upstream report.",
#endif
    EPN_REPORT_BYTE_LENGTH_PRVRPTBYTELEN_FIELD_MASK,
    0,
    EPN_REPORT_BYTE_LENGTH_PRVRPTBYTELEN_FIELD_WIDTH,
    EPN_REPORT_BYTE_LENGTH_PRVRPTBYTELEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTBBHUPFRABORT
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTBBHUPFRABORT_FIELD =
{
    "INTBBHUPFRABORT",
#if RU_INCLUDE_DESC
    "",
    "Indicates the Runner/BBH aborted an upstream frame transfer.  Please"
    "reference the Runner/BBH documentation for a list of events that"
    "will cause Runner/BBH to abort  packets.",
#endif
    EPN_MAIN_INT_STATUS_INTBBHUPFRABORT_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTBBHUPFRABORT_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTBBHUPFRABORT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTCOL2SBURSTCAPOVERFLOWPRES
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTCOL2SBURSTCAPOVERFLOWPRES_FIELD =
{
    "INTCOL2SBURSTCAPOVERFLOWPRES",
#if RU_INCLUDE_DESC
    "",
    "Coalesced per-L2 burst cap overflow event indicator.  This is"
    "triggered when the burst cap is dynamically resized below respective"
    "L2 accumulator's value.",
#endif
    EPN_MAIN_INT_STATUS_INTCOL2SBURSTCAPOVERFLOWPRES_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTCOL2SBURSTCAPOVERFLOWPRES_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTCOL2SBURSTCAPOVERFLOWPRES_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTCOEMPTYRPT
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTCOEMPTYRPT_FIELD =
{
    "INTCOEMPTYRPT",
#if RU_INCLUDE_DESC
    "",
    "Coalesced Empty Report interrupt. One or more LLID indexes has a"
    "transmitted a report in which all time quanta values were zero. See"
    "EPON Empty Report Interrupt Status for per-LLID Index interrupt"
    "bits.",
#endif
    EPN_MAIN_INT_STATUS_INTCOEMPTYRPT_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTCOEMPTYRPT_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTCOEMPTYRPT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTCODRXERRABORTPRES
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTCODRXERRABORTPRES_FIELD =
{
    "INTCODRXERRABORTPRES",
#if RU_INCLUDE_DESC
    "",
    "The Drx detected an error that required the frame to be aborted."
    "Culpable errors are FCS, oversize-frame, or undersize-frame."
    "Note: The intDrxErrorAbortMask will prevent this 'coalesced bit from"
    "being set. This is in contrast to the 'individual' bits (0x41b)"
    "still being set even if the interrupt is masked.",
#endif
    EPN_MAIN_INT_STATUS_INTCODRXERRABORTPRES_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTCODRXERRABORTPRES_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTCODRXERRABORTPRES_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTL2SFIFOOVERRUN
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTL2SFIFOOVERRUN_FIELD =
{
    "INTL2SFIFOOVERRUN",
#if RU_INCLUDE_DESC
    "",
    "The Level 2 structure FIFO has overflowed. A frame length has been"
    "lost.  The Runner/BBH and EPN must be reset to recover from this.",
#endif
    EPN_MAIN_INT_STATUS_INTL2SFIFOOVERRUN_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTL2SFIFOOVERRUN_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTL2SFIFOOVERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTCO1588TSINT
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTCO1588TSINT_FIELD =
{
    "INTCO1588TSINT",
#if RU_INCLUDE_DESC
    "",
    "Coalesced 1588 timestamp interrupt. See"
    "EPN_1588_TIMESTAMP_INT_STATUS for the interupts.",
#endif
    EPN_MAIN_INT_STATUS_INTCO1588TSINT_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTCO1588TSINT_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTCO1588TSINT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTCORPTPRES
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTCORPTPRES_FIELD =
{
    "INTCORPTPRES",
#if RU_INCLUDE_DESC
    "",
    "Coalesced Report FIFO non-empty interrupt. One or more LLID indices"
    "has a frame length present in its report FIFO. See EPON Report"
    "Present Interrupt Status for per-LLID Index interrupt bits.",
#endif
    EPN_MAIN_INT_STATUS_INTCORPTPRES_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTCORPTPRES_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTCORPTPRES_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTCOGNTPRES
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTCOGNTPRES_FIELD =
{
    "INTCOGNTPRES",
#if RU_INCLUDE_DESC
    "",
    "Coalesced Grant Ready interrupt. One or more LLID indexes has a"
    "grant present in its Grant RAM. See EPON Grant Present Interrupt"
    "Status for per-LLID Index interrupt bits.",
#endif
    EPN_MAIN_INT_STATUS_INTCOGNTPRES_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTCOGNTPRES_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTCOGNTPRES_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTCODELSTALEGNT
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTCODELSTALEGNT_FIELD =
{
    "INTCODELSTALEGNT",
#if RU_INCLUDE_DESC
    "",
    "Coalesced stale grant delete interrupt. One or more LLID indexes"
    "deleted a grant deleted from its grant RAM. See EPON Deleted Stale"
    "Grant Interrupt Status for per-LLID Index interrupt bits.",
#endif
    EPN_MAIN_INT_STATUS_INTCODELSTALEGNT_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTCODELSTALEGNT_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTCODELSTALEGNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTCOGNTNONPOLL
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTCOGNTNONPOLL_FIELD =
{
    "INTCOGNTNONPOLL",
#if RU_INCLUDE_DESC
    "",
    "Coalesced grant non-poll interrupt. One or more LLID indexes"
    "exceeded the Non-poll grant interval. See EPON Non-Poll Grant"
    "Interrupt Status for per-LLID Index interrupt bits.",
#endif
    EPN_MAIN_INT_STATUS_INTCOGNTNONPOLL_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTCOGNTNONPOLL_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTCOGNTNONPOLL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTCOGNTMISALIGN
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTCOGNTMISALIGN_FIELD =
{
    "INTCOGNTMISALIGN",
#if RU_INCLUDE_DESC
    "",
    "Coalesced grant misalign interrupt. One or more LLID indexes"
    "received a grant that was not aligned on frame boundaries. See EPON"
    "Grant Misalign Interrupt Status for per-LLID Index interrupt bits.",
#endif
    EPN_MAIN_INT_STATUS_INTCOGNTMISALIGN_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTCOGNTMISALIGN_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTCOGNTMISALIGN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTCOGNTTOOFAR
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTCOGNTTOOFAR_FIELD =
{
    "INTCOGNTTOOFAR",
#if RU_INCLUDE_DESC
    "",
    "Coalesced grant too far abort interrupt. One or more LLID indexes"
    "received a grant for greater than 34 seconds into the future. See"
    "EPON Grant Too Far Interrupt Status for per-LLID Index interrupt"
    "bits."
    "Note: This interrupt can set during registration of the first LLID"
    "Index (before the local MPCP clock is synchronized to the OLT)."
    "Firmware should check and clear this interrupt while registering the"
    "first link.",
#endif
    EPN_MAIN_INT_STATUS_INTCOGNTTOOFAR_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTCOGNTTOOFAR_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTCOGNTTOOFAR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTCOGNTINTERVAL
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTCOGNTINTERVAL_FIELD =
{
    "INTCOGNTINTERVAL",
#if RU_INCLUDE_DESC
    "",
    "Coalesced grant interval interrupt. One or more LLID indexes is not"
    "receiving gates fast enough. See EPON Grant Interval Interrupt"
    "Status for per-LLID Index interrupt bits.",
#endif
    EPN_MAIN_INT_STATUS_INTCOGNTINTERVAL_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTCOGNTINTERVAL_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTCOGNTINTERVAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTCOGNTDISCOVERY
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTCOGNTDISCOVERY_FIELD =
{
    "INTCOGNTDISCOVERY",
#if RU_INCLUDE_DESC
    "",
    "Coalesced Discovery Gate received interrupt. One or more LLID"
    "indexes received a Discovery Gate. See EPON Discovery Gate Interrupt"
    "Status for per-LLID Index interrupt bits.",
#endif
    EPN_MAIN_INT_STATUS_INTCOGNTDISCOVERY_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTCOGNTDISCOVERY_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTCOGNTDISCOVERY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTCOGNTMISSABORT
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTCOGNTMISSABORT_FIELD =
{
    "INTCOGNTMISSABORT",
#if RU_INCLUDE_DESC
    "",
    "Coalesced grant miss abort interrupt. One or more LLID indexes"
    "aborted a grant because it missed its slot time to transmit. See"
    "EPON Grant Miss Interrupt Status for per-LLID Index interrupt bits.",
#endif
    EPN_MAIN_INT_STATUS_INTCOGNTMISSABORT_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTCOGNTMISSABORT_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTCOGNTMISSABORT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTCOGNTFULLABORT
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTCOGNTFULLABORT_FIELD =
{
    "INTCOGNTFULLABORT",
#if RU_INCLUDE_DESC
    "",
    "Coalesced grant full abort interrupt. One or more LLID indexes"
    "aborted a grant due to its grant FIFO being full. See EPON Grant"
    "Full Interrupt Status for per-LLID Index interrupt bits.",
#endif
    EPN_MAIN_INT_STATUS_INTCOGNTFULLABORT_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTCOGNTFULLABORT_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTCOGNTFULLABORT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTBADUPFRLEN
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTBADUPFRLEN_FIELD =
{
    "INTBADUPFRLEN",
#if RU_INCLUDE_DESC
    "",
    "[FATAL] The EPN received an upstream frame whose length did not"
    "match the expected frame length.  This is a fatal event.  The entire"
    "data path must be reset to recover from this event.",
#endif
    EPN_MAIN_INT_STATUS_INTBADUPFRLEN_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTBADUPFRLEN_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTBADUPFRLEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTUPTARDYPACKET
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTUPTARDYPACKET_FIELD =
{
    "INTUPTARDYPACKET",
#if RU_INCLUDE_DESC
    "",
    "The Runner/BBH upstream data path failed to deliver upstream data in"
    "time to meet the upPacketTxMargin requirement.",
#endif
    EPN_MAIN_INT_STATUS_INTUPTARDYPACKET_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTUPTARDYPACKET_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTUPTARDYPACKET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTUPRPTFRXMT
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTUPRPTFRXMT_FIELD =
{
    "INTUPRPTFRXMT",
#if RU_INCLUDE_DESC
    "",
    "Report frame has been transmitted by EPON MAC.",
#endif
    EPN_MAIN_INT_STATUS_INTUPRPTFRXMT_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTUPRPTFRXMT_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTUPRPTFRXMT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTBIFIFOOVERRUN
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTBIFIFOOVERRUN_FIELD =
{
    "INTBIFIFOOVERRUN",
#if RU_INCLUDE_DESC
    "",
    "[FATAL] The burst information FIFO over ran. This is a fatal event"
    "and requires the entire device to be reset and re-initialized.",
#endif
    EPN_MAIN_INT_STATUS_INTBIFIFOOVERRUN_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTBIFIFOOVERRUN_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTBIFIFOOVERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTBURSTGNTTOOBIG
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTBURSTGNTTOOBIG_FIELD =
{
    "INTBURSTGNTTOOBIG",
#if RU_INCLUDE_DESC
    "",
    "A grant passed to the Upstream transmitter has size greater than"
    "that defined EPON Max Grant Size register.",
#endif
    EPN_MAIN_INT_STATUS_INTBURSTGNTTOOBIG_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTBURSTGNTTOOBIG_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTBURSTGNTTOOBIG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTWRGNTTOOBIG
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTWRGNTTOOBIG_FIELD =
{
    "INTWRGNTTOOBIG",
#if RU_INCLUDE_DESC
    "",
    "A grant written into EPON grant RAM has size greater than that"
    "defined EPON Max Grant Size register.",
#endif
    EPN_MAIN_INT_STATUS_INTWRGNTTOOBIG_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTWRGNTTOOBIG_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTWRGNTTOOBIG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTRCVGNTTOOBIG
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTRCVGNTTOOBIG_FIELD =
{
    "INTRCVGNTTOOBIG",
#if RU_INCLUDE_DESC
    "",
    "A grant received by EPON MAC has size greater than that defined in"
    "the EPON Max Grant Size register.",
#endif
    EPN_MAIN_INT_STATUS_INTRCVGNTTOOBIG_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTRCVGNTTOOBIG_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTRCVGNTTOOBIG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTDNSTATSOVERRUN
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTDNSTATSOVERRUN_FIELD =
{
    "INTDNSTATSOVERRUN",
#if RU_INCLUDE_DESC
    "",
    "EPON block cannot accumulate statistics quickly enough to count runt"
    "frames. Bursts of frames less than 20 bytes in size will cause this"
    "interrupt.",
#endif
    EPN_MAIN_INT_STATUS_INTDNSTATSOVERRUN_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTDNSTATSOVERRUN_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTDNSTATSOVERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTUPSTATSOVERRUN
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTUPSTATSOVERRUN_FIELD =
{
    "INTUPSTATSOVERRUN",
#if RU_INCLUDE_DESC
    "",
    "EPON block was not able to process an upstream transmission event.",
#endif
    EPN_MAIN_INT_STATUS_INTUPSTATSOVERRUN_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTUPSTATSOVERRUN_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTUPSTATSOVERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTDNOUTOFORDER
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTDNOUTOFORDER_FIELD =
{
    "INTDNOUTOFORDER",
#if RU_INCLUDE_DESC
    "",
    "An out of order grant was received",
#endif
    EPN_MAIN_INT_STATUS_INTDNOUTOFORDER_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTDNOUTOFORDER_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTDNOUTOFORDER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTTRUANTBBHHALT
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTTRUANTBBHHALT_FIELD =
{
    "INTTRUANTBBHHALT",
#if RU_INCLUDE_DESC
    "",
    "[FATAL] Fatal Event. The Runner/BBH upstream data path stopped"
    "delivering packets. All upstream traffic for all LLID indexes has"
    "been halted.  Any upstream grants received are terminated with empty"
    "reports (if requested).  The only way to recover from this fatal"
    "event is to reset the entire upstream data path."
    "Check the EPN Fatal Upstream Fault Interrupt Status register to see"
    "which LLID(s) experienced the fault.",
#endif
    EPN_MAIN_INT_STATUS_INTTRUANTBBHHALT_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTTRUANTBBHHALT_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTTRUANTBBHHALT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTUPINVLDGNTLEN
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTUPINVLDGNTLEN_FIELD =
{
    "INTUPINVLDGNTLEN",
#if RU_INCLUDE_DESC
    "",
    "Grant length is less than overhead. Possible configuration error.",
#endif
    EPN_MAIN_INT_STATUS_INTUPINVLDGNTLEN_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTUPINVLDGNTLEN_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTUPINVLDGNTLEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTCOBBHUPSFAULT
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTCOBBHUPSFAULT_FIELD =
{
    "INTCOBBHUPSFAULT",
#if RU_INCLUDE_DESC
    "",
    "[FATAL] Coalesced per-LLID index Runner/BBH fatal upstream delivery"
    "fault indicator.  Runner/BBH has lost coherency with the EPN. The"
    "four trigger events are:"
    "1. Runner/BBH aborted a packet"
    "2. Runner/BBH transferred a packet shorter than was requested"
    "3. Runner/BBH transferred a packet longer than was requested"
    "4. Runner/BBH stopped transferring packets (as indicated by"
    "intTruantBbhHalt, below)"
    "Check the EPN Fatal Upstream Fault Interrupt Status register to see"
    "which LLID(s) experienced the fault.",
#endif
    EPN_MAIN_INT_STATUS_INTCOBBHUPSFAULT_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTCOBBHUPSFAULT_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTCOBBHUPSFAULT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTDNTIMEINSYNC
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTDNTIMEINSYNC_FIELD =
{
    "INTDNTIMEINSYNC",
#if RU_INCLUDE_DESC
    "",
    "ONU timer is in sync",
#endif
    EPN_MAIN_INT_STATUS_INTDNTIMEINSYNC_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTDNTIMEINSYNC_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTDNTIMEINSYNC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTDNTIMENOTINSYNC
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTDNTIMENOTINSYNC_FIELD =
{
    "INTDNTIMENOTINSYNC",
#if RU_INCLUDE_DESC
    "",
    "ONU timer is out of sync",
#endif
    EPN_MAIN_INT_STATUS_INTDNTIMENOTINSYNC_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTDNTIMENOTINSYNC_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTDNTIMENOTINSYNC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_STATUS_INTDPORTRDY
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_STATUS_INTDPORTRDY_FIELD =
{
    "INTDPORTRDY",
#if RU_INCLUDE_DESC
    "",
    "EPON Data Port is ready"
    "0: Data Port is busy"
    "1: Data Port is ready",
#endif
    EPN_MAIN_INT_STATUS_INTDPORTRDY_FIELD_MASK,
    0,
    EPN_MAIN_INT_STATUS_INTDPORTRDY_FIELD_WIDTH,
    EPN_MAIN_INT_STATUS_INTDPORTRDY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FULL_INT_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_GNT_FULL_INT_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_GNT_FULL_INT_STATUS_RESERVED0_FIELD_MASK,
    0,
    EPN_GNT_FULL_INT_STATUS_RESERVED0_FIELD_WIDTH,
    EPN_GNT_FULL_INT_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT7
 ******************************************************************************/
const ru_field_rec EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT7_FIELD =
{
    "INTDNGNTFULLABORT7",
#if RU_INCLUDE_DESC
    "",
    "LLID index 7 aborted a grant due to its grant FIFO being full.",
#endif
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT7_FIELD_MASK,
    0,
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT7_FIELD_WIDTH,
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT6
 ******************************************************************************/
const ru_field_rec EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT6_FIELD =
{
    "INTDNGNTFULLABORT6",
#if RU_INCLUDE_DESC
    "",
    "LLID index 6 aborted a grant due to its grant FIFO being full.",
#endif
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT6_FIELD_MASK,
    0,
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT6_FIELD_WIDTH,
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT5
 ******************************************************************************/
const ru_field_rec EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT5_FIELD =
{
    "INTDNGNTFULLABORT5",
#if RU_INCLUDE_DESC
    "",
    "LLID index 5 aborted a grant due to its grant FIFO being full.",
#endif
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT5_FIELD_MASK,
    0,
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT5_FIELD_WIDTH,
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT4
 ******************************************************************************/
const ru_field_rec EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT4_FIELD =
{
    "INTDNGNTFULLABORT4",
#if RU_INCLUDE_DESC
    "",
    "LLID index 4 aborted a grant due to its grant FIFO being full.",
#endif
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT4_FIELD_MASK,
    0,
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT4_FIELD_WIDTH,
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT3
 ******************************************************************************/
const ru_field_rec EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT3_FIELD =
{
    "INTDNGNTFULLABORT3",
#if RU_INCLUDE_DESC
    "",
    "LLID index 3 aborted a grant due to its grant FIFO being full.",
#endif
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT3_FIELD_MASK,
    0,
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT3_FIELD_WIDTH,
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT2
 ******************************************************************************/
const ru_field_rec EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT2_FIELD =
{
    "INTDNGNTFULLABORT2",
#if RU_INCLUDE_DESC
    "",
    "LLID index 2 aborted a grant due to its grant FIFO being full.",
#endif
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT2_FIELD_MASK,
    0,
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT2_FIELD_WIDTH,
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT1
 ******************************************************************************/
const ru_field_rec EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT1_FIELD =
{
    "INTDNGNTFULLABORT1",
#if RU_INCLUDE_DESC
    "",
    "LLID index 1 aborted a grant due to its grant FIFO being full.",
#endif
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT1_FIELD_MASK,
    0,
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT1_FIELD_WIDTH,
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT0
 ******************************************************************************/
const ru_field_rec EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT0_FIELD =
{
    "INTDNGNTFULLABORT0",
#if RU_INCLUDE_DESC
    "",
    "LLID index 0 aborted a grant due to its grant FIFO being full.",
#endif
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT0_FIELD_MASK,
    0,
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT0_FIELD_WIDTH,
    EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FULL_INT_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_GNT_FULL_INT_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_GNT_FULL_INT_MASK_RESERVED0_FIELD_MASK,
    0,
    EPN_GNT_FULL_INT_MASK_RESERVED0_FIELD_WIDTH,
    EPN_GNT_FULL_INT_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT7
 ******************************************************************************/
const ru_field_rec EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT7_FIELD =
{
    "MASKINTDNGNTFULLABORT7",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 7 aborted a grant due to its grant FIFO being full"
    "interrupt.",
#endif
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT7_FIELD_MASK,
    0,
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT7_FIELD_WIDTH,
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT6
 ******************************************************************************/
const ru_field_rec EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT6_FIELD =
{
    "MASKINTDNGNTFULLABORT6",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 6 aborted a grant due to its grant FIFO being full"
    "interrupt.",
#endif
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT6_FIELD_MASK,
    0,
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT6_FIELD_WIDTH,
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT5
 ******************************************************************************/
const ru_field_rec EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT5_FIELD =
{
    "MASKINTDNGNTFULLABORT5",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 5 aborted a grant due to its grant FIFO being full"
    "interrupt.",
#endif
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT5_FIELD_MASK,
    0,
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT5_FIELD_WIDTH,
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT4
 ******************************************************************************/
const ru_field_rec EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT4_FIELD =
{
    "MASKINTDNGNTFULLABORT4",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 4 aborted a grant due to its grant FIFO being full"
    "interrupt.",
#endif
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT4_FIELD_MASK,
    0,
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT4_FIELD_WIDTH,
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT3
 ******************************************************************************/
const ru_field_rec EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT3_FIELD =
{
    "MASKINTDNGNTFULLABORT3",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 3 aborted a grant due to its grant FIFO being full"
    "interrupt.",
#endif
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT3_FIELD_MASK,
    0,
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT3_FIELD_WIDTH,
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT2
 ******************************************************************************/
const ru_field_rec EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT2_FIELD =
{
    "MASKINTDNGNTFULLABORT2",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 2 aborted a grant due to its grant FIFO being full"
    "interrupt.",
#endif
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT2_FIELD_MASK,
    0,
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT2_FIELD_WIDTH,
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT1
 ******************************************************************************/
const ru_field_rec EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT1_FIELD =
{
    "MASKINTDNGNTFULLABORT1",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 1 aborted a grant due to its grant FIFO being full"
    "interrupt.",
#endif
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT1_FIELD_MASK,
    0,
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT1_FIELD_WIDTH,
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT0
 ******************************************************************************/
const ru_field_rec EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT0_FIELD =
{
    "MASKINTDNGNTFULLABORT0",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 0 aborted a grant due to its grant FIFO being full"
    "interrupt.",
#endif
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT0_FIELD_MASK,
    0,
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT0_FIELD_WIDTH,
    EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISS_INT_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISS_INT_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_GNT_MISS_INT_STATUS_RESERVED0_FIELD_MASK,
    0,
    EPN_GNT_MISS_INT_STATUS_RESERVED0_FIELD_WIDTH,
    EPN_GNT_MISS_INT_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT7
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT7_FIELD =
{
    "INTDNGNTMISSABORT7",
#if RU_INCLUDE_DESC
    "",
    "LLID index 7 aborted a grant because it missed its slot time to"
    "transmit.",
#endif
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT7_FIELD_MASK,
    0,
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT7_FIELD_WIDTH,
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT6
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT6_FIELD =
{
    "INTDNGNTMISSABORT6",
#if RU_INCLUDE_DESC
    "",
    "LLID index 6 aborted a grant because it missed its slot time to"
    "transmit.",
#endif
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT6_FIELD_MASK,
    0,
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT6_FIELD_WIDTH,
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT5
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT5_FIELD =
{
    "INTDNGNTMISSABORT5",
#if RU_INCLUDE_DESC
    "",
    "LLID index 5 aborted a grant because it missed its slot time to"
    "transmit.",
#endif
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT5_FIELD_MASK,
    0,
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT5_FIELD_WIDTH,
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT4
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT4_FIELD =
{
    "INTDNGNTMISSABORT4",
#if RU_INCLUDE_DESC
    "",
    "LLID index 4 aborted a grant because it missed its slot time to"
    "transmit.",
#endif
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT4_FIELD_MASK,
    0,
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT4_FIELD_WIDTH,
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT3
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT3_FIELD =
{
    "INTDNGNTMISSABORT3",
#if RU_INCLUDE_DESC
    "",
    "LLID index 3 aborted a grant because it missed its slot time to"
    "transmit.",
#endif
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT3_FIELD_MASK,
    0,
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT3_FIELD_WIDTH,
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT2
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT2_FIELD =
{
    "INTDNGNTMISSABORT2",
#if RU_INCLUDE_DESC
    "",
    "LLID index 2 aborted a grant because it missed its slot time to"
    "transmit.",
#endif
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT2_FIELD_MASK,
    0,
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT2_FIELD_WIDTH,
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT1
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT1_FIELD =
{
    "INTDNGNTMISSABORT1",
#if RU_INCLUDE_DESC
    "",
    "LLID index 1 aborted a grant because it missed its slot time to"
    "transmit.",
#endif
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT1_FIELD_MASK,
    0,
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT1_FIELD_WIDTH,
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT0
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT0_FIELD =
{
    "INTDNGNTMISSABORT0",
#if RU_INCLUDE_DESC
    "",
    "LLID index 0 aborted a grant because it missed its slot time to"
    "transmit.",
#endif
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT0_FIELD_MASK,
    0,
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT0_FIELD_WIDTH,
    EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISS_INT_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISS_INT_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_GNT_MISS_INT_MASK_RESERVED0_FIELD_MASK,
    0,
    EPN_GNT_MISS_INT_MASK_RESERVED0_FIELD_WIDTH,
    EPN_GNT_MISS_INT_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT7
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT7_FIELD =
{
    "MASKINTDNGNTMISSABORT7",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 7 aborted a grant because it missed its slot time to"
    "transmit interrupt.",
#endif
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT7_FIELD_MASK,
    0,
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT7_FIELD_WIDTH,
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT6
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT6_FIELD =
{
    "MASKINTDNGNTMISSABORT6",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 6 aborted a grant because it missed its slot time to"
    "transmit interrupt.",
#endif
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT6_FIELD_MASK,
    0,
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT6_FIELD_WIDTH,
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT5
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT5_FIELD =
{
    "MASKINTDNGNTMISSABORT5",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 5 aborted a grant because it missed its slot time to"
    "transmit interrupt.",
#endif
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT5_FIELD_MASK,
    0,
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT5_FIELD_WIDTH,
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT4
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT4_FIELD =
{
    "MASKINTDNGNTMISSABORT4",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 4 aborted a grant because it missed its slot time to"
    "transmit interrupt.",
#endif
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT4_FIELD_MASK,
    0,
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT4_FIELD_WIDTH,
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT3
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT3_FIELD =
{
    "MASKINTDNGNTMISSABORT3",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 3 aborted a grant because it missed its slot time to"
    "transmit interrupt.",
#endif
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT3_FIELD_MASK,
    0,
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT3_FIELD_WIDTH,
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT2
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT2_FIELD =
{
    "MASKINTDNGNTMISSABORT2",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 2 aborted a grant because it missed its slot time to"
    "transmit interrupt.",
#endif
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT2_FIELD_MASK,
    0,
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT2_FIELD_WIDTH,
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT1
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT1_FIELD =
{
    "MASKINTDNGNTMISSABORT1",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 1 aborted a grant because it missed its slot time to"
    "transmit interrupt.",
#endif
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT1_FIELD_MASK,
    0,
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT1_FIELD_WIDTH,
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT0
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT0_FIELD =
{
    "MASKINTDNGNTMISSABORT0",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 0 aborted a grant because it missed its slot time to"
    "transmit interrupt.",
#endif
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT0_FIELD_MASK,
    0,
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT0_FIELD_WIDTH,
    EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DISC_RX_INT_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_DISC_RX_INT_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DISC_RX_INT_STATUS_RESERVED0_FIELD_MASK,
    0,
    EPN_DISC_RX_INT_STATUS_RESERVED0_FIELD_WIDTH,
    EPN_DISC_RX_INT_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY7
 ******************************************************************************/
const ru_field_rec EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY7_FIELD =
{
    "INTDNGNTDISCOVERY7",
#if RU_INCLUDE_DESC
    "",
    "LLID index 7 received a discovery gate",
#endif
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY7_FIELD_MASK,
    0,
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY7_FIELD_WIDTH,
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY6
 ******************************************************************************/
const ru_field_rec EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY6_FIELD =
{
    "INTDNGNTDISCOVERY6",
#if RU_INCLUDE_DESC
    "",
    "LLID index 6 received a discovery gate",
#endif
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY6_FIELD_MASK,
    0,
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY6_FIELD_WIDTH,
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY5
 ******************************************************************************/
const ru_field_rec EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY5_FIELD =
{
    "INTDNGNTDISCOVERY5",
#if RU_INCLUDE_DESC
    "",
    "LLID index 5 received a discovery gate",
#endif
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY5_FIELD_MASK,
    0,
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY5_FIELD_WIDTH,
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY4
 ******************************************************************************/
const ru_field_rec EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY4_FIELD =
{
    "INTDNGNTDISCOVERY4",
#if RU_INCLUDE_DESC
    "",
    "LLID index 4 received a discovery gate",
#endif
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY4_FIELD_MASK,
    0,
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY4_FIELD_WIDTH,
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY3
 ******************************************************************************/
const ru_field_rec EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY3_FIELD =
{
    "INTDNGNTDISCOVERY3",
#if RU_INCLUDE_DESC
    "",
    "LLID index 3 received a discovery gate",
#endif
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY3_FIELD_MASK,
    0,
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY3_FIELD_WIDTH,
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY2
 ******************************************************************************/
const ru_field_rec EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY2_FIELD =
{
    "INTDNGNTDISCOVERY2",
#if RU_INCLUDE_DESC
    "",
    "LLID index 2 received a discovery gate",
#endif
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY2_FIELD_MASK,
    0,
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY2_FIELD_WIDTH,
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY1
 ******************************************************************************/
const ru_field_rec EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY1_FIELD =
{
    "INTDNGNTDISCOVERY1",
#if RU_INCLUDE_DESC
    "",
    "LLID index 1 received a discovery gate",
#endif
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY1_FIELD_MASK,
    0,
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY1_FIELD_WIDTH,
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY0
 ******************************************************************************/
const ru_field_rec EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY0_FIELD =
{
    "INTDNGNTDISCOVERY0",
#if RU_INCLUDE_DESC
    "",
    "LLID index 0 received a discovery gate",
#endif
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY0_FIELD_MASK,
    0,
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY0_FIELD_WIDTH,
    EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DISC_RX_INT_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_DISC_RX_INT_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DISC_RX_INT_MASK_RESERVED0_FIELD_MASK,
    0,
    EPN_DISC_RX_INT_MASK_RESERVED0_FIELD_WIDTH,
    EPN_DISC_RX_INT_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY7
 ******************************************************************************/
const ru_field_rec EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY7_FIELD =
{
    "MASKINTDNGNTDISCOVERY7",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 7 received a discovery gate interrupt.",
#endif
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY7_FIELD_MASK,
    0,
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY7_FIELD_WIDTH,
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY6
 ******************************************************************************/
const ru_field_rec EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY6_FIELD =
{
    "MASKINTDNGNTDISCOVERY6",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 6 received a discovery gate interrupt.",
#endif
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY6_FIELD_MASK,
    0,
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY6_FIELD_WIDTH,
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY5
 ******************************************************************************/
const ru_field_rec EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY5_FIELD =
{
    "MASKINTDNGNTDISCOVERY5",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 5 received a discovery gate interrupt.",
#endif
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY5_FIELD_MASK,
    0,
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY5_FIELD_WIDTH,
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY4
 ******************************************************************************/
const ru_field_rec EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY4_FIELD =
{
    "MASKINTDNGNTDISCOVERY4",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 4 received a discovery gate interrupt.",
#endif
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY4_FIELD_MASK,
    0,
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY4_FIELD_WIDTH,
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY3
 ******************************************************************************/
const ru_field_rec EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY3_FIELD =
{
    "MASKINTDNGNTDISCOVERY3",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 3 received a discovery gate interrupt.",
#endif
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY3_FIELD_MASK,
    0,
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY3_FIELD_WIDTH,
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY2
 ******************************************************************************/
const ru_field_rec EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY2_FIELD =
{
    "MASKINTDNGNTDISCOVERY2",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 2 received a discovery gate interrupt.",
#endif
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY2_FIELD_MASK,
    0,
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY2_FIELD_WIDTH,
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY1
 ******************************************************************************/
const ru_field_rec EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY1_FIELD =
{
    "MASKINTDNGNTDISCOVERY1",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 1 received a discovery gate interrupt.",
#endif
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY1_FIELD_MASK,
    0,
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY1_FIELD_WIDTH,
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY0
 ******************************************************************************/
const ru_field_rec EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY0_FIELD =
{
    "MASKINTDNGNTDISCOVERY0",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 0 received a discovery gate interrupt.",
#endif
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY0_FIELD_MASK,
    0,
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY0_FIELD_WIDTH,
    EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_INTV_INT_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_GNT_INTV_INT_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_GNT_INTV_INT_STATUS_RESERVED0_FIELD_MASK,
    0,
    EPN_GNT_INTV_INT_STATUS_RESERVED0_FIELD_WIDTH,
    EPN_GNT_INTV_INT_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL7
 ******************************************************************************/
const ru_field_rec EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL7_FIELD =
{
    "INTDNGNTINTERVAL7",
#if RU_INCLUDE_DESC
    "",
    "LLID index 7 failed to receive a GATE within a time period defined"
    "by the EPN Grant Interval register.",
#endif
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL7_FIELD_MASK,
    0,
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL7_FIELD_WIDTH,
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL6
 ******************************************************************************/
const ru_field_rec EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL6_FIELD =
{
    "INTDNGNTINTERVAL6",
#if RU_INCLUDE_DESC
    "",
    "LLID index 6 failed to receive a GATE within a time period defined"
    "by the EPN Grant Interval register.",
#endif
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL6_FIELD_MASK,
    0,
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL6_FIELD_WIDTH,
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL5
 ******************************************************************************/
const ru_field_rec EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL5_FIELD =
{
    "INTDNGNTINTERVAL5",
#if RU_INCLUDE_DESC
    "",
    "LLID index 5 failed to receive a GATE within a time period defined"
    "by the EPN Grant Interval register.",
#endif
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL5_FIELD_MASK,
    0,
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL5_FIELD_WIDTH,
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL4
 ******************************************************************************/
const ru_field_rec EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL4_FIELD =
{
    "INTDNGNTINTERVAL4",
#if RU_INCLUDE_DESC
    "",
    "LLID index 4 failed to receive a GATE within a time period defined"
    "by the EPN Grant Interval register.",
#endif
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL4_FIELD_MASK,
    0,
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL4_FIELD_WIDTH,
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL3
 ******************************************************************************/
const ru_field_rec EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL3_FIELD =
{
    "INTDNGNTINTERVAL3",
#if RU_INCLUDE_DESC
    "",
    "LLID index 3 failed to receive a GATE within a time period defined"
    "by the EPN Grant Interval register.",
#endif
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL3_FIELD_MASK,
    0,
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL3_FIELD_WIDTH,
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL2
 ******************************************************************************/
const ru_field_rec EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL2_FIELD =
{
    "INTDNGNTINTERVAL2",
#if RU_INCLUDE_DESC
    "",
    "LLID index 2 failed to receive a GATE within a time period defined"
    "by the EPN Grant Interval register.",
#endif
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL2_FIELD_MASK,
    0,
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL2_FIELD_WIDTH,
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL1
 ******************************************************************************/
const ru_field_rec EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL1_FIELD =
{
    "INTDNGNTINTERVAL1",
#if RU_INCLUDE_DESC
    "",
    "LLID index 1 failed to receive a GATE within a time period defined"
    "by the EPN Grant Interval register.",
#endif
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL1_FIELD_MASK,
    0,
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL1_FIELD_WIDTH,
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL0
 ******************************************************************************/
const ru_field_rec EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL0_FIELD =
{
    "INTDNGNTINTERVAL0",
#if RU_INCLUDE_DESC
    "",
    "LLID index 0 failed to receive a GATE within a time period defined"
    "by the EPN Grant Interval register.",
#endif
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL0_FIELD_MASK,
    0,
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL0_FIELD_WIDTH,
    EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_INTV_INT_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_GNT_INTV_INT_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_GNT_INTV_INT_MASK_RESERVED0_FIELD_MASK,
    0,
    EPN_GNT_INTV_INT_MASK_RESERVED0_FIELD_WIDTH,
    EPN_GNT_INTV_INT_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL7
 ******************************************************************************/
const ru_field_rec EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL7_FIELD =
{
    "MASKINTDNGNTINTERVAL7",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 7 failed to receive a GATE within a time period"
    "defined by the EPN Grant Interval register interrupt.",
#endif
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL7_FIELD_MASK,
    0,
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL7_FIELD_WIDTH,
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL6
 ******************************************************************************/
const ru_field_rec EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL6_FIELD =
{
    "MASKINTDNGNTINTERVAL6",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 6 failed to receive a GATE within a time period"
    "defined by the EPN Grant Interval register interrupt.",
#endif
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL6_FIELD_MASK,
    0,
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL6_FIELD_WIDTH,
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL5
 ******************************************************************************/
const ru_field_rec EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL5_FIELD =
{
    "MASKINTDNGNTINTERVAL5",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 5 failed to receive a GATE within a time period"
    "defined by the EPN Grant Interval register interrupt.",
#endif
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL5_FIELD_MASK,
    0,
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL5_FIELD_WIDTH,
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL4
 ******************************************************************************/
const ru_field_rec EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL4_FIELD =
{
    "MASKINTDNGNTINTERVAL4",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 4 failed to receive a GATE within a time period"
    "defined by the EPN Grant Interval register interrupt.",
#endif
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL4_FIELD_MASK,
    0,
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL4_FIELD_WIDTH,
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL3
 ******************************************************************************/
const ru_field_rec EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL3_FIELD =
{
    "MASKINTDNGNTINTERVAL3",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 3 failed to receive a GATE within a time period"
    "defined by the EPN Grant Interval register interrupt.",
#endif
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL3_FIELD_MASK,
    0,
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL3_FIELD_WIDTH,
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL2
 ******************************************************************************/
const ru_field_rec EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL2_FIELD =
{
    "MASKINTDNGNTINTERVAL2",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 2 failed to receive a GATE within a time period"
    "defined by the EPN Grant Interval register interrupt.",
#endif
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL2_FIELD_MASK,
    0,
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL2_FIELD_WIDTH,
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL1
 ******************************************************************************/
const ru_field_rec EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL1_FIELD =
{
    "MASKINTDNGNTINTERVAL1",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 1 failed to receive a GATE within a time period"
    "defined by the EPN Grant Interval register interrupt.",
#endif
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL1_FIELD_MASK,
    0,
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL1_FIELD_WIDTH,
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL0
 ******************************************************************************/
const ru_field_rec EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL0_FIELD =
{
    "MASKINTDNGNTINTERVAL0",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 0 failed to receive a GATE within a time period"
    "defined by the EPN Grant Interval register interrupt.",
#endif
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL0_FIELD_MASK,
    0,
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL0_FIELD_WIDTH,
    EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FAR_INT_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_GNT_FAR_INT_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_GNT_FAR_INT_STATUS_RESERVED0_FIELD_MASK,
    0,
    EPN_GNT_FAR_INT_STATUS_RESERVED0_FIELD_WIDTH,
    EPN_GNT_FAR_INT_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR7
 ******************************************************************************/
const ru_field_rec EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR7_FIELD =
{
    "INTDNGNTTOOFAR7",
#if RU_INCLUDE_DESC
    "",
    "LLID index 7 received (and aborted) a grant with a start time"
    "greater than 34 sec in the future",
#endif
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR7_FIELD_MASK,
    0,
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR7_FIELD_WIDTH,
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR6
 ******************************************************************************/
const ru_field_rec EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR6_FIELD =
{
    "INTDNGNTTOOFAR6",
#if RU_INCLUDE_DESC
    "",
    "LLID index 6 received (and aborted) a grant with a start time"
    "greater than 34 sec in the future",
#endif
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR6_FIELD_MASK,
    0,
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR6_FIELD_WIDTH,
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR5
 ******************************************************************************/
const ru_field_rec EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR5_FIELD =
{
    "INTDNGNTTOOFAR5",
#if RU_INCLUDE_DESC
    "",
    "LLID index 5 received (and aborted) a grant with a start time"
    "greater than 34 sec in the future",
#endif
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR5_FIELD_MASK,
    0,
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR5_FIELD_WIDTH,
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR4
 ******************************************************************************/
const ru_field_rec EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR4_FIELD =
{
    "INTDNGNTTOOFAR4",
#if RU_INCLUDE_DESC
    "",
    "LLID index 4 received (and aborted) a grant with a start time"
    "greater than 34 sec in the future",
#endif
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR4_FIELD_MASK,
    0,
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR4_FIELD_WIDTH,
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR3
 ******************************************************************************/
const ru_field_rec EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR3_FIELD =
{
    "INTDNGNTTOOFAR3",
#if RU_INCLUDE_DESC
    "",
    "LLID index 3 received (and aborted) a grant with a start time"
    "greater than 34 sec in the future",
#endif
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR3_FIELD_MASK,
    0,
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR3_FIELD_WIDTH,
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR2
 ******************************************************************************/
const ru_field_rec EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR2_FIELD =
{
    "INTDNGNTTOOFAR2",
#if RU_INCLUDE_DESC
    "",
    "LLID index 2 received (and aborted) a grant with a start time"
    "greater than 34 sec in the future",
#endif
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR2_FIELD_MASK,
    0,
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR2_FIELD_WIDTH,
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR1
 ******************************************************************************/
const ru_field_rec EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR1_FIELD =
{
    "INTDNGNTTOOFAR1",
#if RU_INCLUDE_DESC
    "",
    "LLID index 1 received (and aborted) a grant with a start time"
    "greater than 34 sec in the future",
#endif
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR1_FIELD_MASK,
    0,
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR1_FIELD_WIDTH,
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR0
 ******************************************************************************/
const ru_field_rec EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR0_FIELD =
{
    "INTDNGNTTOOFAR0",
#if RU_INCLUDE_DESC
    "",
    "LLID index 0 received (and aborted) a grant with a start time"
    "greater than 34 sec in the future",
#endif
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR0_FIELD_MASK,
    0,
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR0_FIELD_WIDTH,
    EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FAR_INT_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_GNT_FAR_INT_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_GNT_FAR_INT_MASK_RESERVED0_FIELD_MASK,
    0,
    EPN_GNT_FAR_INT_MASK_RESERVED0_FIELD_WIDTH,
    EPN_GNT_FAR_INT_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR7
 ******************************************************************************/
const ru_field_rec EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR7_FIELD =
{
    "MASKDNGNTTOOFAR7",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 7 received (and aborted) a grant with a start time"
    "greater than 34 sec in the future interrupt",
#endif
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR7_FIELD_MASK,
    0,
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR7_FIELD_WIDTH,
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR6
 ******************************************************************************/
const ru_field_rec EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR6_FIELD =
{
    "MASKDNGNTTOOFAR6",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 6 received (and aborted) a grant with a start time"
    "greater than 34 sec in the future interrupt",
#endif
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR6_FIELD_MASK,
    0,
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR6_FIELD_WIDTH,
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR5
 ******************************************************************************/
const ru_field_rec EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR5_FIELD =
{
    "MASKDNGNTTOOFAR5",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 5 received (and aborted) a grant with a start time"
    "greater than 34 sec in the future interrupt",
#endif
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR5_FIELD_MASK,
    0,
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR5_FIELD_WIDTH,
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR4
 ******************************************************************************/
const ru_field_rec EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR4_FIELD =
{
    "MASKDNGNTTOOFAR4",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 4 received (and aborted) a grant with a start time"
    "greater than 34 sec in the future interrupt",
#endif
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR4_FIELD_MASK,
    0,
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR4_FIELD_WIDTH,
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR3
 ******************************************************************************/
const ru_field_rec EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR3_FIELD =
{
    "MASKDNGNTTOOFAR3",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 3 received (and aborted) a grant with a start time"
    "greater than 34 sec in the future interrupt",
#endif
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR3_FIELD_MASK,
    0,
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR3_FIELD_WIDTH,
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR2
 ******************************************************************************/
const ru_field_rec EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR2_FIELD =
{
    "MASKDNGNTTOOFAR2",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 2 received (and aborted) a grant with a start time"
    "greater than 34 sec in the future interrupt",
#endif
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR2_FIELD_MASK,
    0,
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR2_FIELD_WIDTH,
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR1
 ******************************************************************************/
const ru_field_rec EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR1_FIELD =
{
    "MASKDNGNTTOOFAR1",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 1 received (and aborted) a grant with a start time"
    "greater than 34 sec in the future interrupt",
#endif
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR1_FIELD_MASK,
    0,
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR1_FIELD_WIDTH,
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR0
 ******************************************************************************/
const ru_field_rec EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR0_FIELD =
{
    "MASKDNGNTTOOFAR0",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 0 received (and aborted) a grant with a start time"
    "greater than 34 sec in the future interrupt",
#endif
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR0_FIELD_MASK,
    0,
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR0_FIELD_WIDTH,
    EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISALGN_INT_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISALGN_INT_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_GNT_MISALGN_INT_STATUS_RESERVED0_FIELD_MASK,
    0,
    EPN_GNT_MISALGN_INT_STATUS_RESERVED0_FIELD_WIDTH,
    EPN_GNT_MISALGN_INT_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN7
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN7_FIELD =
{
    "INTDNGNTMISALIGN7",
#if RU_INCLUDE_DESC
    "",
    "LLID index 7 received a misaligned grant",
#endif
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN7_FIELD_MASK,
    0,
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN7_FIELD_WIDTH,
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN6
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN6_FIELD =
{
    "INTDNGNTMISALIGN6",
#if RU_INCLUDE_DESC
    "",
    "LLID index 6 received a misaligned grant",
#endif
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN6_FIELD_MASK,
    0,
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN6_FIELD_WIDTH,
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN5
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN5_FIELD =
{
    "INTDNGNTMISALIGN5",
#if RU_INCLUDE_DESC
    "",
    "LLID index 5 received a misaligned grant",
#endif
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN5_FIELD_MASK,
    0,
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN5_FIELD_WIDTH,
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN4
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN4_FIELD =
{
    "INTDNGNTMISALIGN4",
#if RU_INCLUDE_DESC
    "",
    "LLID index 4 received a misaligned grant",
#endif
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN4_FIELD_MASK,
    0,
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN4_FIELD_WIDTH,
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN3
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN3_FIELD =
{
    "INTDNGNTMISALIGN3",
#if RU_INCLUDE_DESC
    "",
    "LLID index 3 received a misaligned grant",
#endif
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN3_FIELD_MASK,
    0,
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN3_FIELD_WIDTH,
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN2
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN2_FIELD =
{
    "INTDNGNTMISALIGN2",
#if RU_INCLUDE_DESC
    "",
    "LLID index 2 received a misaligned grant",
#endif
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN2_FIELD_MASK,
    0,
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN2_FIELD_WIDTH,
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN1
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN1_FIELD =
{
    "INTDNGNTMISALIGN1",
#if RU_INCLUDE_DESC
    "",
    "LLID index 1 received a misaligned grant",
#endif
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN1_FIELD_MASK,
    0,
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN1_FIELD_WIDTH,
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN0
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN0_FIELD =
{
    "INTDNGNTMISALIGN0",
#if RU_INCLUDE_DESC
    "",
    "LLID index 0 received a misaligned grant",
#endif
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN0_FIELD_MASK,
    0,
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN0_FIELD_WIDTH,
    EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISALGN_INT_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISALGN_INT_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_GNT_MISALGN_INT_MASK_RESERVED0_FIELD_MASK,
    0,
    EPN_GNT_MISALGN_INT_MASK_RESERVED0_FIELD_WIDTH,
    EPN_GNT_MISALGN_INT_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN7
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN7_FIELD =
{
    "MASKINTDNGNTMISALIGN7",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 7 received a misaligned grant interrupt",
#endif
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN7_FIELD_MASK,
    0,
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN7_FIELD_WIDTH,
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN6
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN6_FIELD =
{
    "MASKINTDNGNTMISALIGN6",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 6 received a misaligned grant interrupt",
#endif
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN6_FIELD_MASK,
    0,
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN6_FIELD_WIDTH,
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN5
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN5_FIELD =
{
    "MASKINTDNGNTMISALIGN5",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 5 received a misaligned grant interrupt",
#endif
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN5_FIELD_MASK,
    0,
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN5_FIELD_WIDTH,
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN4
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN4_FIELD =
{
    "MASKINTDNGNTMISALIGN4",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 4 received a misaligned grant interrupt",
#endif
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN4_FIELD_MASK,
    0,
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN4_FIELD_WIDTH,
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN3
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN3_FIELD =
{
    "MASKINTDNGNTMISALIGN3",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 3 received a misaligned grant interrupt",
#endif
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN3_FIELD_MASK,
    0,
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN3_FIELD_WIDTH,
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN2
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN2_FIELD =
{
    "MASKINTDNGNTMISALIGN2",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 2 received a misaligned grant interrupt",
#endif
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN2_FIELD_MASK,
    0,
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN2_FIELD_WIDTH,
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN1
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN1_FIELD =
{
    "MASKINTDNGNTMISALIGN1",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 1 received a misaligned grant interrupt",
#endif
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN1_FIELD_MASK,
    0,
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN1_FIELD_WIDTH,
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN0
 ******************************************************************************/
const ru_field_rec EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN0_FIELD =
{
    "MASKINTDNGNTMISALIGN0",
#if RU_INCLUDE_DESC
    "",
    "Mask LLID index 0 received a misaligned grant interrupt",
#endif
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN0_FIELD_MASK,
    0,
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN0_FIELD_WIDTH,
    EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_NP_GNT_INT_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_NP_GNT_INT_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_NP_GNT_INT_STATUS_RESERVED0_FIELD_MASK,
    0,
    EPN_NP_GNT_INT_STATUS_RESERVED0_FIELD_WIDTH,
    EPN_NP_GNT_INT_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL7
 ******************************************************************************/
const ru_field_rec EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL7_FIELD =
{
    "INTDNGNTNONPOLL7",
#if RU_INCLUDE_DESC
    "",
    "Non poll grant interval exceeded on LLID Index 7",
#endif
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL7_FIELD_MASK,
    0,
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL7_FIELD_WIDTH,
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL6
 ******************************************************************************/
const ru_field_rec EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL6_FIELD =
{
    "INTDNGNTNONPOLL6",
#if RU_INCLUDE_DESC
    "",
    "Non poll grant interval exceeded on LLID Index 6",
#endif
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL6_FIELD_MASK,
    0,
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL6_FIELD_WIDTH,
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL5
 ******************************************************************************/
const ru_field_rec EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL5_FIELD =
{
    "INTDNGNTNONPOLL5",
#if RU_INCLUDE_DESC
    "",
    "Non poll grant interval exceeded on LLID Index 5",
#endif
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL5_FIELD_MASK,
    0,
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL5_FIELD_WIDTH,
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL4
 ******************************************************************************/
const ru_field_rec EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL4_FIELD =
{
    "INTDNGNTNONPOLL4",
#if RU_INCLUDE_DESC
    "",
    "Non poll grant interval exceeded on LLID Index 4",
#endif
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL4_FIELD_MASK,
    0,
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL4_FIELD_WIDTH,
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL3
 ******************************************************************************/
const ru_field_rec EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL3_FIELD =
{
    "INTDNGNTNONPOLL3",
#if RU_INCLUDE_DESC
    "",
    "Non poll grant interval exceeded on LLID Index 3",
#endif
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL3_FIELD_MASK,
    0,
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL3_FIELD_WIDTH,
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL2
 ******************************************************************************/
const ru_field_rec EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL2_FIELD =
{
    "INTDNGNTNONPOLL2",
#if RU_INCLUDE_DESC
    "",
    "Non poll grant interval exceeded on LLID Index 2",
#endif
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL2_FIELD_MASK,
    0,
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL2_FIELD_WIDTH,
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL1
 ******************************************************************************/
const ru_field_rec EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL1_FIELD =
{
    "INTDNGNTNONPOLL1",
#if RU_INCLUDE_DESC
    "",
    "Non poll grant interval exceeded on LLID Index 1",
#endif
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL1_FIELD_MASK,
    0,
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL1_FIELD_WIDTH,
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL0
 ******************************************************************************/
const ru_field_rec EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL0_FIELD =
{
    "INTDNGNTNONPOLL0",
#if RU_INCLUDE_DESC
    "",
    "Non poll grant interval exceeded on LLID Index 0",
#endif
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL0_FIELD_MASK,
    0,
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL0_FIELD_WIDTH,
    EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_NP_GNT_INT_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_NP_GNT_INT_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_NP_GNT_INT_MASK_RESERVED0_FIELD_MASK,
    0,
    EPN_NP_GNT_INT_MASK_RESERVED0_FIELD_WIDTH,
    EPN_NP_GNT_INT_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL7
 ******************************************************************************/
const ru_field_rec EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL7_FIELD =
{
    "MASKDNGNTNONPOLL7",
#if RU_INCLUDE_DESC
    "",
    "Non poll grant interval exceeded on LLID Index 7 interrupt mask",
#endif
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL7_FIELD_MASK,
    0,
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL7_FIELD_WIDTH,
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL6
 ******************************************************************************/
const ru_field_rec EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL6_FIELD =
{
    "MASKDNGNTNONPOLL6",
#if RU_INCLUDE_DESC
    "",
    "Non poll grant interval exceeded on LLID Index 6 interrupt mask",
#endif
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL6_FIELD_MASK,
    0,
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL6_FIELD_WIDTH,
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL5
 ******************************************************************************/
const ru_field_rec EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL5_FIELD =
{
    "MASKDNGNTNONPOLL5",
#if RU_INCLUDE_DESC
    "",
    "Non poll grant interval exceeded on LLID Index 5 interrupt mask",
#endif
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL5_FIELD_MASK,
    0,
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL5_FIELD_WIDTH,
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL4
 ******************************************************************************/
const ru_field_rec EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL4_FIELD =
{
    "MASKDNGNTNONPOLL4",
#if RU_INCLUDE_DESC
    "",
    "Non poll grant interval exceeded on LLID Index 4 interrupt mask",
#endif
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL4_FIELD_MASK,
    0,
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL4_FIELD_WIDTH,
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL3
 ******************************************************************************/
const ru_field_rec EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL3_FIELD =
{
    "MASKDNGNTNONPOLL3",
#if RU_INCLUDE_DESC
    "",
    "Non poll grant interval exceeded on LLID Index 3 interrupt mask",
#endif
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL3_FIELD_MASK,
    0,
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL3_FIELD_WIDTH,
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL2
 ******************************************************************************/
const ru_field_rec EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL2_FIELD =
{
    "MASKDNGNTNONPOLL2",
#if RU_INCLUDE_DESC
    "",
    "Non poll grant interval exceeded on LLID Index 2 interrupt mask",
#endif
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL2_FIELD_MASK,
    0,
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL2_FIELD_WIDTH,
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL1
 ******************************************************************************/
const ru_field_rec EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL1_FIELD =
{
    "MASKDNGNTNONPOLL1",
#if RU_INCLUDE_DESC
    "",
    "Non poll grant interval exceeded on LLID Index 1 interrupt mask",
#endif
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL1_FIELD_MASK,
    0,
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL1_FIELD_WIDTH,
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL0
 ******************************************************************************/
const ru_field_rec EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL0_FIELD =
{
    "MASKDNGNTNONPOLL0",
#if RU_INCLUDE_DESC
    "",
    "Non poll grant interval exceeded on LLID Index 0 interrupt mask",
#endif
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL0_FIELD_MASK,
    0,
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL0_FIELD_WIDTH,
    EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DEL_STALE_INT_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_DEL_STALE_INT_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DEL_STALE_INT_STATUS_RESERVED0_FIELD_MASK,
    0,
    EPN_DEL_STALE_INT_STATUS_RESERVED0_FIELD_WIDTH,
    EPN_DEL_STALE_INT_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT7
 ******************************************************************************/
const ru_field_rec EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT7_FIELD =
{
    "INTDELSTALEGNT7",
#if RU_INCLUDE_DESC
    "",
    "Stale grant deleted from LLID Index 7 grant RAM.",
#endif
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT7_FIELD_MASK,
    0,
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT7_FIELD_WIDTH,
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT6
 ******************************************************************************/
const ru_field_rec EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT6_FIELD =
{
    "INTDELSTALEGNT6",
#if RU_INCLUDE_DESC
    "",
    "Stale grant deleted from LLID Index 6 grant RAM.",
#endif
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT6_FIELD_MASK,
    0,
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT6_FIELD_WIDTH,
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT5
 ******************************************************************************/
const ru_field_rec EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT5_FIELD =
{
    "INTDELSTALEGNT5",
#if RU_INCLUDE_DESC
    "",
    "Stale grant deleted from LLID Index 5 grant RAM.",
#endif
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT5_FIELD_MASK,
    0,
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT5_FIELD_WIDTH,
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT4
 ******************************************************************************/
const ru_field_rec EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT4_FIELD =
{
    "INTDELSTALEGNT4",
#if RU_INCLUDE_DESC
    "",
    "Stale grant deleted from LLID Index 4 grant RAM.",
#endif
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT4_FIELD_MASK,
    0,
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT4_FIELD_WIDTH,
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT3
 ******************************************************************************/
const ru_field_rec EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT3_FIELD =
{
    "INTDELSTALEGNT3",
#if RU_INCLUDE_DESC
    "",
    "Stale grant deleted from LLID Index 3 grant RAM.",
#endif
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT3_FIELD_MASK,
    0,
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT3_FIELD_WIDTH,
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT2
 ******************************************************************************/
const ru_field_rec EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT2_FIELD =
{
    "INTDELSTALEGNT2",
#if RU_INCLUDE_DESC
    "",
    "Stale grant deleted from LLID Index 2 grant RAM.",
#endif
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT2_FIELD_MASK,
    0,
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT2_FIELD_WIDTH,
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT1
 ******************************************************************************/
const ru_field_rec EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT1_FIELD =
{
    "INTDELSTALEGNT1",
#if RU_INCLUDE_DESC
    "",
    "Stale grant deleted from LLID Index 1 grant RAM.",
#endif
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT1_FIELD_MASK,
    0,
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT1_FIELD_WIDTH,
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT0
 ******************************************************************************/
const ru_field_rec EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT0_FIELD =
{
    "INTDELSTALEGNT0",
#if RU_INCLUDE_DESC
    "",
    "Stale grant deleted from LLID Index 0 grant RAM.",
#endif
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT0_FIELD_MASK,
    0,
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT0_FIELD_WIDTH,
    EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DEL_STALE_INT_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_DEL_STALE_INT_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DEL_STALE_INT_MASK_RESERVED0_FIELD_MASK,
    0,
    EPN_DEL_STALE_INT_MASK_RESERVED0_FIELD_WIDTH,
    EPN_DEL_STALE_INT_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT7
 ******************************************************************************/
const ru_field_rec EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT7_FIELD =
{
    "MASKINTDELSTALEGNT7",
#if RU_INCLUDE_DESC
    "",
    "Stale grant deleted from LLID Index 7 grant RAM interrupt mask.",
#endif
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT7_FIELD_MASK,
    0,
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT7_FIELD_WIDTH,
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT6
 ******************************************************************************/
const ru_field_rec EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT6_FIELD =
{
    "MASKINTDELSTALEGNT6",
#if RU_INCLUDE_DESC
    "",
    "Stale grant deleted from LLID Index 6 grant RAM interrupt mask.",
#endif
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT6_FIELD_MASK,
    0,
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT6_FIELD_WIDTH,
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT5
 ******************************************************************************/
const ru_field_rec EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT5_FIELD =
{
    "MASKINTDELSTALEGNT5",
#if RU_INCLUDE_DESC
    "",
    "Stale grant deleted from LLID Index 5 grant RAM interrupt mask.",
#endif
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT5_FIELD_MASK,
    0,
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT5_FIELD_WIDTH,
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT4
 ******************************************************************************/
const ru_field_rec EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT4_FIELD =
{
    "MASKINTDELSTALEGNT4",
#if RU_INCLUDE_DESC
    "",
    "Stale grant deleted from LLID Index 4 grant RAM interrupt mask.",
#endif
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT4_FIELD_MASK,
    0,
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT4_FIELD_WIDTH,
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT3
 ******************************************************************************/
const ru_field_rec EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT3_FIELD =
{
    "MASKINTDELSTALEGNT3",
#if RU_INCLUDE_DESC
    "",
    "Stale grant deleted from LLID Index 3 grant RAM interrupt mask.",
#endif
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT3_FIELD_MASK,
    0,
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT3_FIELD_WIDTH,
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT2
 ******************************************************************************/
const ru_field_rec EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT2_FIELD =
{
    "MASKINTDELSTALEGNT2",
#if RU_INCLUDE_DESC
    "",
    "Stale grant deleted from LLID Index 2 grant RAM interrupt mask.",
#endif
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT2_FIELD_MASK,
    0,
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT2_FIELD_WIDTH,
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT1
 ******************************************************************************/
const ru_field_rec EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT1_FIELD =
{
    "MASKINTDELSTALEGNT1",
#if RU_INCLUDE_DESC
    "",
    "Stale grant deleted from LLID Index 1 grant RAM interrupt mask.",
#endif
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT1_FIELD_MASK,
    0,
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT1_FIELD_WIDTH,
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT0
 ******************************************************************************/
const ru_field_rec EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT0_FIELD =
{
    "MASKINTDELSTALEGNT0",
#if RU_INCLUDE_DESC
    "",
    "Stale grant deleted from LLID Index 0 grant RAM interrupt mask.",
#endif
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT0_FIELD_MASK,
    0,
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT0_FIELD_WIDTH,
    EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_PRES_INT_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_GNT_PRES_INT_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_GNT_PRES_INT_STATUS_RESERVED0_FIELD_MASK,
    0,
    EPN_GNT_PRES_INT_STATUS_RESERVED0_FIELD_WIDTH,
    EPN_GNT_PRES_INT_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY7
 ******************************************************************************/
const ru_field_rec EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY7_FIELD =
{
    "INTDNGNTRDY7",
#if RU_INCLUDE_DESC
    "",
    "Grant present in LLID Index 7 grant RAM",
#endif
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY7_FIELD_MASK,
    0,
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY7_FIELD_WIDTH,
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY6
 ******************************************************************************/
const ru_field_rec EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY6_FIELD =
{
    "INTDNGNTRDY6",
#if RU_INCLUDE_DESC
    "",
    "Grant present in LLID Index 6 grant RAM",
#endif
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY6_FIELD_MASK,
    0,
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY6_FIELD_WIDTH,
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY5
 ******************************************************************************/
const ru_field_rec EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY5_FIELD =
{
    "INTDNGNTRDY5",
#if RU_INCLUDE_DESC
    "",
    "Grant present in LLID Index 5 grant RAM",
#endif
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY5_FIELD_MASK,
    0,
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY5_FIELD_WIDTH,
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY4
 ******************************************************************************/
const ru_field_rec EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY4_FIELD =
{
    "INTDNGNTRDY4",
#if RU_INCLUDE_DESC
    "",
    "Grant present in LLID Index 4 grant RAM",
#endif
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY4_FIELD_MASK,
    0,
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY4_FIELD_WIDTH,
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY3
 ******************************************************************************/
const ru_field_rec EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY3_FIELD =
{
    "INTDNGNTRDY3",
#if RU_INCLUDE_DESC
    "",
    "Grant present in LLID Index 3 grant RAM",
#endif
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY3_FIELD_MASK,
    0,
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY3_FIELD_WIDTH,
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY2
 ******************************************************************************/
const ru_field_rec EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY2_FIELD =
{
    "INTDNGNTRDY2",
#if RU_INCLUDE_DESC
    "",
    "Grant present in LLID Index 2 grant RAM",
#endif
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY2_FIELD_MASK,
    0,
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY2_FIELD_WIDTH,
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY1
 ******************************************************************************/
const ru_field_rec EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY1_FIELD =
{
    "INTDNGNTRDY1",
#if RU_INCLUDE_DESC
    "",
    "Grant present in LLID Index 1 grant RAM",
#endif
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY1_FIELD_MASK,
    0,
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY1_FIELD_WIDTH,
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY0
 ******************************************************************************/
const ru_field_rec EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY0_FIELD =
{
    "INTDNGNTRDY0",
#if RU_INCLUDE_DESC
    "",
    "Grant present in LLID Index 0 grant RAM",
#endif
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY0_FIELD_MASK,
    0,
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY0_FIELD_WIDTH,
    EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_PRES_INT_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_GNT_PRES_INT_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_GNT_PRES_INT_MASK_RESERVED0_FIELD_MASK,
    0,
    EPN_GNT_PRES_INT_MASK_RESERVED0_FIELD_WIDTH,
    EPN_GNT_PRES_INT_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY7
 ******************************************************************************/
const ru_field_rec EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY7_FIELD =
{
    "MASKDNGNTRDY7",
#if RU_INCLUDE_DESC
    "",
    "Grant present in LLID Index 7 grant RAM interrupt mask",
#endif
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY7_FIELD_MASK,
    0,
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY7_FIELD_WIDTH,
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY6
 ******************************************************************************/
const ru_field_rec EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY6_FIELD =
{
    "MASKDNGNTRDY6",
#if RU_INCLUDE_DESC
    "",
    "Grant present in LLID Index 6 grant RAM interrupt mask",
#endif
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY6_FIELD_MASK,
    0,
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY6_FIELD_WIDTH,
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY5
 ******************************************************************************/
const ru_field_rec EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY5_FIELD =
{
    "MASKDNGNTRDY5",
#if RU_INCLUDE_DESC
    "",
    "Grant present in LLID Index 5 grant RAM interrupt mask",
#endif
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY5_FIELD_MASK,
    0,
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY5_FIELD_WIDTH,
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY4
 ******************************************************************************/
const ru_field_rec EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY4_FIELD =
{
    "MASKDNGNTRDY4",
#if RU_INCLUDE_DESC
    "",
    "Grant present in LLID Index 4 grant RAM interrupt mask",
#endif
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY4_FIELD_MASK,
    0,
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY4_FIELD_WIDTH,
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY3
 ******************************************************************************/
const ru_field_rec EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY3_FIELD =
{
    "MASKDNGNTRDY3",
#if RU_INCLUDE_DESC
    "",
    "Grant present in LLID Index 3 grant RAM interrupt mask",
#endif
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY3_FIELD_MASK,
    0,
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY3_FIELD_WIDTH,
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY2
 ******************************************************************************/
const ru_field_rec EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY2_FIELD =
{
    "MASKDNGNTRDY2",
#if RU_INCLUDE_DESC
    "",
    "Grant present in LLID Index 2 grant RAM interrupt mask",
#endif
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY2_FIELD_MASK,
    0,
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY2_FIELD_WIDTH,
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY1
 ******************************************************************************/
const ru_field_rec EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY1_FIELD =
{
    "MASKDNGNTRDY1",
#if RU_INCLUDE_DESC
    "",
    "Grant present in LLID Index 1 grant RAM interrupt mask",
#endif
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY1_FIELD_MASK,
    0,
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY1_FIELD_WIDTH,
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY0
 ******************************************************************************/
const ru_field_rec EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY0_FIELD =
{
    "MASKDNGNTRDY0",
#if RU_INCLUDE_DESC
    "",
    "Grant present in LLID Index 0 grant RAM interrupt mask",
#endif
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY0_FIELD_MASK,
    0,
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY0_FIELD_WIDTH,
    EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RPT_PRES_INT_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_RPT_PRES_INT_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_RPT_PRES_INT_STATUS_RESERVED0_FIELD_MASK,
    0,
    EPN_RPT_PRES_INT_STATUS_RESERVED0_FIELD_WIDTH,
    EPN_RPT_PRES_INT_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO7
 ******************************************************************************/
const ru_field_rec EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO7_FIELD =
{
    "INTUPRPTFIFO7",
#if RU_INCLUDE_DESC
    "",
    "Frame length present in LLID Index 7 report FIFO",
#endif
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO7_FIELD_MASK,
    0,
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO7_FIELD_WIDTH,
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO6
 ******************************************************************************/
const ru_field_rec EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO6_FIELD =
{
    "INTUPRPTFIFO6",
#if RU_INCLUDE_DESC
    "",
    "Frame length present in LLID Index 6 report FIFO",
#endif
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO6_FIELD_MASK,
    0,
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO6_FIELD_WIDTH,
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO5
 ******************************************************************************/
const ru_field_rec EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO5_FIELD =
{
    "INTUPRPTFIFO5",
#if RU_INCLUDE_DESC
    "",
    "Frame length present in LLID Index 5 report FIFO",
#endif
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO5_FIELD_MASK,
    0,
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO5_FIELD_WIDTH,
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO4
 ******************************************************************************/
const ru_field_rec EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO4_FIELD =
{
    "INTUPRPTFIFO4",
#if RU_INCLUDE_DESC
    "",
    "Frame length present in LLID Index 4 report FIFO",
#endif
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO4_FIELD_MASK,
    0,
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO4_FIELD_WIDTH,
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO3
 ******************************************************************************/
const ru_field_rec EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO3_FIELD =
{
    "INTUPRPTFIFO3",
#if RU_INCLUDE_DESC
    "",
    "Frame length present in LLID Index 3 report FIFO",
#endif
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO3_FIELD_MASK,
    0,
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO3_FIELD_WIDTH,
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO2
 ******************************************************************************/
const ru_field_rec EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO2_FIELD =
{
    "INTUPRPTFIFO2",
#if RU_INCLUDE_DESC
    "",
    "Frame length present in LLID Index 2 report FIFO",
#endif
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO2_FIELD_MASK,
    0,
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO2_FIELD_WIDTH,
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO1
 ******************************************************************************/
const ru_field_rec EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO1_FIELD =
{
    "INTUPRPTFIFO1",
#if RU_INCLUDE_DESC
    "",
    "Frame length present in LLID Index 1 report FIFO",
#endif
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO1_FIELD_MASK,
    0,
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO1_FIELD_WIDTH,
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO0
 ******************************************************************************/
const ru_field_rec EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO0_FIELD =
{
    "INTUPRPTFIFO0",
#if RU_INCLUDE_DESC
    "",
    "Frame length present in LLID Index 0 report FIFO",
#endif
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO0_FIELD_MASK,
    0,
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO0_FIELD_WIDTH,
    EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RPT_PRES_INT_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_RPT_PRES_INT_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_RPT_PRES_INT_MASK_RESERVED0_FIELD_MASK,
    0,
    EPN_RPT_PRES_INT_MASK_RESERVED0_FIELD_WIDTH,
    EPN_RPT_PRES_INT_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO7
 ******************************************************************************/
const ru_field_rec EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO7_FIELD =
{
    "MASKINTUPRPTFIFO7",
#if RU_INCLUDE_DESC
    "",
    "Frame length present in LLID Index 7 report FIFO interrupt mask",
#endif
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO7_FIELD_MASK,
    0,
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO7_FIELD_WIDTH,
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO6
 ******************************************************************************/
const ru_field_rec EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO6_FIELD =
{
    "MASKINTUPRPTFIFO6",
#if RU_INCLUDE_DESC
    "",
    "Frame length present in LLID Index 6 report FIFO interrupt mask",
#endif
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO6_FIELD_MASK,
    0,
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO6_FIELD_WIDTH,
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO5
 ******************************************************************************/
const ru_field_rec EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO5_FIELD =
{
    "MASKINTUPRPTFIFO5",
#if RU_INCLUDE_DESC
    "",
    "Frame length present in LLID Index 5 report FIFO interrupt mask",
#endif
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO5_FIELD_MASK,
    0,
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO5_FIELD_WIDTH,
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO4
 ******************************************************************************/
const ru_field_rec EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO4_FIELD =
{
    "MASKINTUPRPTFIFO4",
#if RU_INCLUDE_DESC
    "",
    "Frame length present in LLID Index 4 report FIFO interrupt mask",
#endif
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO4_FIELD_MASK,
    0,
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO4_FIELD_WIDTH,
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO3
 ******************************************************************************/
const ru_field_rec EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO3_FIELD =
{
    "MASKINTUPRPTFIFO3",
#if RU_INCLUDE_DESC
    "",
    "Frame length present in LLID Index 3 report FIFO interrupt mask",
#endif
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO3_FIELD_MASK,
    0,
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO3_FIELD_WIDTH,
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO2
 ******************************************************************************/
const ru_field_rec EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO2_FIELD =
{
    "MASKINTUPRPTFIFO2",
#if RU_INCLUDE_DESC
    "",
    "Frame length present in LLID Index 2 report FIFO interrupt mask",
#endif
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO2_FIELD_MASK,
    0,
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO2_FIELD_WIDTH,
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO1
 ******************************************************************************/
const ru_field_rec EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO1_FIELD =
{
    "MASKINTUPRPTFIFO1",
#if RU_INCLUDE_DESC
    "",
    "Frame length present in LLID Index 1 report FIFO interrupt mask",
#endif
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO1_FIELD_MASK,
    0,
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO1_FIELD_WIDTH,
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO0
 ******************************************************************************/
const ru_field_rec EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO0_FIELD =
{
    "MASKINTUPRPTFIFO0",
#if RU_INCLUDE_DESC
    "",
    "Frame length present in LLID Index 0 report FIFO interrupt mask",
#endif
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO0_FIELD_MASK,
    0,
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO0_FIELD_WIDTH,
    EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DRX_ABORT_INT_STATUS_INTDRXERRABORT
 ******************************************************************************/
const ru_field_rec EPN_DRX_ABORT_INT_STATUS_INTDRXERRABORT_FIELD =
{
    "INTDRXERRABORT",
#if RU_INCLUDE_DESC
    "",
    "The Drx detected an error that required an LLID Index 0-31 (bitwise)"
    "frame be aborted in the RDP",
#endif
    EPN_DRX_ABORT_INT_STATUS_INTDRXERRABORT_FIELD_MASK,
    0,
    EPN_DRX_ABORT_INT_STATUS_INTDRXERRABORT_FIELD_WIDTH,
    EPN_DRX_ABORT_INT_STATUS_INTDRXERRABORT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DRX_ABORT_INT_MASK_MASKINTDRXERRABORT
 ******************************************************************************/
const ru_field_rec EPN_DRX_ABORT_INT_MASK_MASKINTDRXERRABORT_FIELD =
{
    "MASKINTDRXERRABORT",
#if RU_INCLUDE_DESC
    "",
    "Mask the Drx detected an error that required an LLID Index 0-31"
    "(bitwise) frame be aborted in the RDP interrupt.",
#endif
    EPN_DRX_ABORT_INT_MASK_MASKINTDRXERRABORT_FIELD_MASK,
    0,
    EPN_DRX_ABORT_INT_MASK_MASKINTDRXERRABORT_FIELD_WIDTH,
    EPN_DRX_ABORT_INT_MASK_MASKINTDRXERRABORT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_EMPTY_RPT_INT_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_EMPTY_RPT_INT_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_EMPTY_RPT_INT_STATUS_RESERVED0_FIELD_MASK,
    0,
    EPN_EMPTY_RPT_INT_STATUS_RESERVED0_FIELD_WIDTH,
    EPN_EMPTY_RPT_INT_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT7
 ******************************************************************************/
const ru_field_rec EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT7_FIELD =
{
    "INTEMPTYRPT7",
#if RU_INCLUDE_DESC
    "",
    "Time quanta values present in LLID Index 7 report were all zero.",
#endif
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT7_FIELD_MASK,
    0,
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT7_FIELD_WIDTH,
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT6
 ******************************************************************************/
const ru_field_rec EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT6_FIELD =
{
    "INTEMPTYRPT6",
#if RU_INCLUDE_DESC
    "",
    "Time quanta values present in LLID Index 6 report were all zero.",
#endif
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT6_FIELD_MASK,
    0,
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT6_FIELD_WIDTH,
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT5
 ******************************************************************************/
const ru_field_rec EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT5_FIELD =
{
    "INTEMPTYRPT5",
#if RU_INCLUDE_DESC
    "",
    "Time quanta values present in LLID Index 5 report were all zero.",
#endif
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT5_FIELD_MASK,
    0,
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT5_FIELD_WIDTH,
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT4
 ******************************************************************************/
const ru_field_rec EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT4_FIELD =
{
    "INTEMPTYRPT4",
#if RU_INCLUDE_DESC
    "",
    "Time quanta values present in LLID Index 4 report were all zero.",
#endif
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT4_FIELD_MASK,
    0,
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT4_FIELD_WIDTH,
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT3
 ******************************************************************************/
const ru_field_rec EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT3_FIELD =
{
    "INTEMPTYRPT3",
#if RU_INCLUDE_DESC
    "",
    "Time quanta values present in LLID Index 3 report were all zero.",
#endif
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT3_FIELD_MASK,
    0,
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT3_FIELD_WIDTH,
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT2
 ******************************************************************************/
const ru_field_rec EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT2_FIELD =
{
    "INTEMPTYRPT2",
#if RU_INCLUDE_DESC
    "",
    "Time quanta values present in LLID Index 2 report were all zero.",
#endif
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT2_FIELD_MASK,
    0,
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT2_FIELD_WIDTH,
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT1
 ******************************************************************************/
const ru_field_rec EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT1_FIELD =
{
    "INTEMPTYRPT1",
#if RU_INCLUDE_DESC
    "",
    "Time quanta values present in LLID Index 1 report were all zero.",
#endif
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT1_FIELD_MASK,
    0,
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT1_FIELD_WIDTH,
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT0
 ******************************************************************************/
const ru_field_rec EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT0_FIELD =
{
    "INTEMPTYRPT0",
#if RU_INCLUDE_DESC
    "",
    "Time quanta values present in LLID Index 0 report were all zero.",
#endif
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT0_FIELD_MASK,
    0,
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT0_FIELD_WIDTH,
    EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_EMPTY_RPT_INT_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_EMPTY_RPT_INT_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_EMPTY_RPT_INT_MASK_RESERVED0_FIELD_MASK,
    0,
    EPN_EMPTY_RPT_INT_MASK_RESERVED0_FIELD_WIDTH,
    EPN_EMPTY_RPT_INT_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT7
 ******************************************************************************/
const ru_field_rec EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT7_FIELD =
{
    "MASKINTEMPTYRPT7",
#if RU_INCLUDE_DESC
    "",
    "Mask time quanta values present in LLID Index 7 report were all zero"
    "interrupt.",
#endif
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT7_FIELD_MASK,
    0,
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT7_FIELD_WIDTH,
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT6
 ******************************************************************************/
const ru_field_rec EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT6_FIELD =
{
    "MASKINTEMPTYRPT6",
#if RU_INCLUDE_DESC
    "",
    "Mask time quanta values present in LLID Index 6 report were all zero"
    "interrupt.",
#endif
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT6_FIELD_MASK,
    0,
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT6_FIELD_WIDTH,
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT5
 ******************************************************************************/
const ru_field_rec EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT5_FIELD =
{
    "MASKINTEMPTYRPT5",
#if RU_INCLUDE_DESC
    "",
    "Mask time quanta values present in LLID Index 5 report were all zero"
    "interrupt.",
#endif
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT5_FIELD_MASK,
    0,
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT5_FIELD_WIDTH,
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT4
 ******************************************************************************/
const ru_field_rec EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT4_FIELD =
{
    "MASKINTEMPTYRPT4",
#if RU_INCLUDE_DESC
    "",
    "Mask time quanta values present in LLID Index 4 report were all zero"
    "interrupt.",
#endif
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT4_FIELD_MASK,
    0,
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT4_FIELD_WIDTH,
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT3
 ******************************************************************************/
const ru_field_rec EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT3_FIELD =
{
    "MASKINTEMPTYRPT3",
#if RU_INCLUDE_DESC
    "",
    "Mask time quanta values present in LLID Index 3 report were all zero"
    "interrupt.",
#endif
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT3_FIELD_MASK,
    0,
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT3_FIELD_WIDTH,
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT2
 ******************************************************************************/
const ru_field_rec EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT2_FIELD =
{
    "MASKINTEMPTYRPT2",
#if RU_INCLUDE_DESC
    "",
    "Mask time quanta values present in LLID Index 2 report were all zero"
    "interrupt.",
#endif
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT2_FIELD_MASK,
    0,
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT2_FIELD_WIDTH,
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT1
 ******************************************************************************/
const ru_field_rec EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT1_FIELD =
{
    "MASKINTEMPTYRPT1",
#if RU_INCLUDE_DESC
    "",
    "Mask time quanta values present in LLID Index 1 report were all zero"
    "interrupt.",
#endif
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT1_FIELD_MASK,
    0,
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT1_FIELD_WIDTH,
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT0
 ******************************************************************************/
const ru_field_rec EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT0_FIELD =
{
    "MASKINTEMPTYRPT0",
#if RU_INCLUDE_DESC
    "",
    "Mask time quanta values present in LLID Index 0 report were all zero"
    "interrupt.",
#endif
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT0_FIELD_MASK,
    0,
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT0_FIELD_WIDTH,
    EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BCAP_OVERFLOW_INT_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_BCAP_OVERFLOW_INT_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_BCAP_OVERFLOW_INT_STATUS_RESERVED0_FIELD_MASK,
    0,
    EPN_BCAP_OVERFLOW_INT_STATUS_RESERVED0_FIELD_WIDTH,
    EPN_BCAP_OVERFLOW_INT_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BCAP_OVERFLOW_INT_STATUS_INTL2SBURSTCAPOVERFLOW
 ******************************************************************************/
const ru_field_rec EPN_BCAP_OVERFLOW_INT_STATUS_INTL2SBURSTCAPOVERFLOW_FIELD =
{
    "INTL2SBURSTCAPOVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "Indicates that the L2 accumulator 0-7 (bitwise) has exceeded its"
    "burst cap value.",
#endif
    EPN_BCAP_OVERFLOW_INT_STATUS_INTL2SBURSTCAPOVERFLOW_FIELD_MASK,
    0,
    EPN_BCAP_OVERFLOW_INT_STATUS_INTL2SBURSTCAPOVERFLOW_FIELD_WIDTH,
    EPN_BCAP_OVERFLOW_INT_STATUS_INTL2SBURSTCAPOVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BCAP_OVERFLOW_INT_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_BCAP_OVERFLOW_INT_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_BCAP_OVERFLOW_INT_MASK_RESERVED0_FIELD_MASK,
    0,
    EPN_BCAP_OVERFLOW_INT_MASK_RESERVED0_FIELD_WIDTH,
    EPN_BCAP_OVERFLOW_INT_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BCAP_OVERFLOW_INT_MASK_MASKINTL2SBURSTCAPOVERFLOW
 ******************************************************************************/
const ru_field_rec EPN_BCAP_OVERFLOW_INT_MASK_MASKINTL2SBURSTCAPOVERFLOW_FIELD =
{
    "MASKINTL2SBURSTCAPOVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "Mask interrupt indicating that the L2 accumulator 0-7 (bitwise) has"
    "exceeded its burst cap value.",
#endif
    EPN_BCAP_OVERFLOW_INT_MASK_MASKINTL2SBURSTCAPOVERFLOW_FIELD_MASK,
    0,
    EPN_BCAP_OVERFLOW_INT_MASK_MASKINTL2SBURSTCAPOVERFLOW_FIELD_WIDTH,
    EPN_BCAP_OVERFLOW_INT_MASK_MASKINTL2SBURSTCAPOVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_DNS_FAULT_INT_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_BBH_DNS_FAULT_INT_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_BBH_DNS_FAULT_INT_STATUS_RESERVED0_FIELD_MASK,
    0,
    EPN_BBH_DNS_FAULT_INT_STATUS_RESERVED0_FIELD_WIDTH,
    EPN_BBH_DNS_FAULT_INT_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_DNS_FAULT_INT_STATUS_INTBBHDNSOVERFLOW
 ******************************************************************************/
const ru_field_rec EPN_BBH_DNS_FAULT_INT_STATUS_INTBBHDNSOVERFLOW_FIELD =
{
    "INTBBHDNSOVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "Indicates the downstream BBH interface failed to transfer the"
    "downstream fast enough.  This occurs when the Epn dropped a"
    "downstream packet (sent abort).",
#endif
    EPN_BBH_DNS_FAULT_INT_STATUS_INTBBHDNSOVERFLOW_FIELD_MASK,
    0,
    EPN_BBH_DNS_FAULT_INT_STATUS_INTBBHDNSOVERFLOW_FIELD_WIDTH,
    EPN_BBH_DNS_FAULT_INT_STATUS_INTBBHDNSOVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_DNS_FAULT_INT_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_BBH_DNS_FAULT_INT_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_BBH_DNS_FAULT_INT_MASK_RESERVED0_FIELD_MASK,
    0,
    EPN_BBH_DNS_FAULT_INT_MASK_RESERVED0_FIELD_WIDTH,
    EPN_BBH_DNS_FAULT_INT_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_DNS_FAULT_INT_MASK_MASKINTBBHDNSOVERFLOW
 ******************************************************************************/
const ru_field_rec EPN_BBH_DNS_FAULT_INT_MASK_MASKINTBBHDNSOVERFLOW_FIELD =
{
    "MASKINTBBHDNSOVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "Mask downstream BBH data path overflow interrupt.",
#endif
    EPN_BBH_DNS_FAULT_INT_MASK_MASKINTBBHDNSOVERFLOW_FIELD_MASK,
    0,
    EPN_BBH_DNS_FAULT_INT_MASK_MASKINTBBHDNSOVERFLOW_FIELD_WIDTH,
    EPN_BBH_DNS_FAULT_INT_MASK_MASKINTBBHDNSOVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UPS_FAULT_INT_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_BBH_UPS_FAULT_INT_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_BBH_UPS_FAULT_INT_STATUS_RESERVED0_FIELD_MASK,
    0,
    EPN_BBH_UPS_FAULT_INT_STATUS_RESERVED0_FIELD_WIDTH,
    EPN_BBH_UPS_FAULT_INT_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT7
 ******************************************************************************/
const ru_field_rec EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT7_FIELD =
{
    "INTBBHUPSFAULT7",
#if RU_INCLUDE_DESC
    "",
    "Indicates upstream LLID index 7 has lost coherency with the"
    "Runner/BBH. This condition can be recovered by resetting the data"
    "path associated with the LLID index."
    "Note: Do not clear these interrupts until the LLID index data path"
    "has been reset or the LLID index's upstream traffic has been"
    "disabled using 'EPN Enable Upstream' register.",
#endif
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT7_FIELD_MASK,
    0,
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT7_FIELD_WIDTH,
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT6
 ******************************************************************************/
const ru_field_rec EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT6_FIELD =
{
    "INTBBHUPSFAULT6",
#if RU_INCLUDE_DESC
    "",
    "Indicates upstream LLID index 6 has lost coherency with the"
    "Runner/BBH. This condition can be recovered by resetting the data"
    "path associated with the LLID index."
    "Note: Do not clear these interrupts until the LLID index data path"
    "has been reset or the LLID index's upstream traffic has been"
    "disabled using 'EPN Enable Upstream' register.",
#endif
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT6_FIELD_MASK,
    0,
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT6_FIELD_WIDTH,
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT5
 ******************************************************************************/
const ru_field_rec EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT5_FIELD =
{
    "INTBBHUPSFAULT5",
#if RU_INCLUDE_DESC
    "",
    "Indicates upstream LLID index 5 has lost coherency with the"
    "Runner/BBH. This condition can be recovered by resetting the data"
    "path associated with the LLID index."
    "Note: Do not clear these interrupts until the LLID index data path"
    "has been reset or the LLID index's upstream traffic has been"
    "disabled using 'EPN Enable Upstream' register.",
#endif
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT5_FIELD_MASK,
    0,
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT5_FIELD_WIDTH,
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT4
 ******************************************************************************/
const ru_field_rec EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT4_FIELD =
{
    "INTBBHUPSFAULT4",
#if RU_INCLUDE_DESC
    "",
    "Indicates upstream LLID index 4 has lost coherency with the"
    "Runner/BBH. This condition can be recovered by resetting the data"
    "path associated with the LLID index."
    "Note: Do not clear these interrupts until the LLID index data path"
    "has been reset or the LLID index's upstream traffic has been"
    "disabled using 'EPN Enable Upstream' register.",
#endif
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT4_FIELD_MASK,
    0,
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT4_FIELD_WIDTH,
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT3
 ******************************************************************************/
const ru_field_rec EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT3_FIELD =
{
    "INTBBHUPSFAULT3",
#if RU_INCLUDE_DESC
    "",
    "Indicates upstream LLID index 3 has lost coherency with the"
    "Runner/BBH. This condition can be recovered by resetting the data"
    "path associated with the LLID index."
    "Note: Do not clear these interrupts until the LLID index data path"
    "has been reset or the LLID index's upstream traffic has been"
    "disabled using 'EPN Enable Upstream' register.",
#endif
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT3_FIELD_MASK,
    0,
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT3_FIELD_WIDTH,
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT2
 ******************************************************************************/
const ru_field_rec EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT2_FIELD =
{
    "INTBBHUPSFAULT2",
#if RU_INCLUDE_DESC
    "",
    "Indicates upstream LLID index 2 has lost coherency with the"
    "Runner/BBH. This condition can be recovered by resetting the data"
    "path associated with the LLID index."
    "Note: Do not clear these interrupts until the LLID index data path"
    "has been reset or the LLID index's upstream traffic has been"
    "disabled using 'EPN Enable Upstream' register.",
#endif
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT2_FIELD_MASK,
    0,
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT2_FIELD_WIDTH,
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT1
 ******************************************************************************/
const ru_field_rec EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT1_FIELD =
{
    "INTBBHUPSFAULT1",
#if RU_INCLUDE_DESC
    "",
    "Indicates upstream LLID index 1 has lost coherency with the"
    "Runner/BBH. This condition can be recovered by resetting the data"
    "path associated with the LLID index."
    "Note: Do not clear these interrupts until the LLID index data path"
    "has been reset or the LLID index's upstream traffic has been"
    "disabled using 'EPN Enable Upstream' register.",
#endif
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT1_FIELD_MASK,
    0,
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT1_FIELD_WIDTH,
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT0
 ******************************************************************************/
const ru_field_rec EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT0_FIELD =
{
    "INTBBHUPSFAULT0",
#if RU_INCLUDE_DESC
    "",
    "Indicates upstream LLID index 0 has lost coherency with the"
    "Runner/BBH. This condition can be recovered by resetting the data"
    "path associated with the LLID index."
    "Note: Do not clear these interrupts until the LLID index data path"
    "has been reset or the LLID index's upstream traffic has been"
    "disabled using 'EPN Enable Upstream' register.",
#endif
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT0_FIELD_MASK,
    0,
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT0_FIELD_WIDTH,
    EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UPS_FAULT_INT_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_BBH_UPS_FAULT_INT_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_BBH_UPS_FAULT_INT_MASK_RESERVED0_FIELD_MASK,
    0,
    EPN_BBH_UPS_FAULT_INT_MASK_RESERVED0_FIELD_WIDTH,
    EPN_BBH_UPS_FAULT_INT_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT7
 ******************************************************************************/
const ru_field_rec EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT7_FIELD =
{
    "MASKINTBBHUPSFAULT7",
#if RU_INCLUDE_DESC
    "",
    "Mask upstream LLID index 7 has lost coherency with the Runner/BBH"
    "interrupt.",
#endif
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT7_FIELD_MASK,
    0,
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT7_FIELD_WIDTH,
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT6
 ******************************************************************************/
const ru_field_rec EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT6_FIELD =
{
    "MASKINTBBHUPSFAULT6",
#if RU_INCLUDE_DESC
    "",
    "Mask upstream LLID index 6 has lost coherency with the Runner/BBH"
    "interrupt.",
#endif
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT6_FIELD_MASK,
    0,
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT6_FIELD_WIDTH,
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT5
 ******************************************************************************/
const ru_field_rec EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT5_FIELD =
{
    "MASKINTBBHUPSFAULT5",
#if RU_INCLUDE_DESC
    "",
    "Mask upstream LLID index 5 has lost coherency with the Runner/BBH"
    "interrupt.",
#endif
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT5_FIELD_MASK,
    0,
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT5_FIELD_WIDTH,
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT4
 ******************************************************************************/
const ru_field_rec EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT4_FIELD =
{
    "MASKINTBBHUPSFAULT4",
#if RU_INCLUDE_DESC
    "",
    "Mask upstream LLID index 4 has lost coherency with the Runner/BBH"
    "interrupt.",
#endif
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT4_FIELD_MASK,
    0,
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT4_FIELD_WIDTH,
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT3
 ******************************************************************************/
const ru_field_rec EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT3_FIELD =
{
    "MASKINTBBHUPSFAULT3",
#if RU_INCLUDE_DESC
    "",
    "Mask upstream LLID index 3 has lost coherency with the Runner/BBH"
    "interrupt.",
#endif
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT3_FIELD_MASK,
    0,
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT3_FIELD_WIDTH,
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT2
 ******************************************************************************/
const ru_field_rec EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT2_FIELD =
{
    "MASKINTBBHUPSFAULT2",
#if RU_INCLUDE_DESC
    "",
    "Mask upstream LLID index 2 has lost coherency with the Runner/BBH"
    "interrupt.",
#endif
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT2_FIELD_MASK,
    0,
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT2_FIELD_WIDTH,
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT1
 ******************************************************************************/
const ru_field_rec EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT1_FIELD =
{
    "MASKINTBBHUPSFAULT1",
#if RU_INCLUDE_DESC
    "",
    "Mask upstream LLID index 1 has lost coherency with the Runner/BBH"
    "interrupt.",
#endif
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT1_FIELD_MASK,
    0,
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT1_FIELD_WIDTH,
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT0
 ******************************************************************************/
const ru_field_rec EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT0_FIELD =
{
    "MASKINTBBHUPSFAULT0",
#if RU_INCLUDE_DESC
    "",
    "Mask upstream LLID index 0 has lost coherency with the Runner/BBH"
    "interrupt.",
#endif
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT0_FIELD_MASK,
    0,
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT0_FIELD_WIDTH,
    EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UPS_ABORT_INT_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_BBH_UPS_ABORT_INT_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_BBH_UPS_ABORT_INT_STATUS_RESERVED0_FIELD_MASK,
    0,
    EPN_BBH_UPS_ABORT_INT_STATUS_RESERVED0_FIELD_WIDTH,
    EPN_BBH_UPS_ABORT_INT_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UPS_ABORT_INT_STATUS_TARDYBBHABORT
 ******************************************************************************/
const ru_field_rec EPN_BBH_UPS_ABORT_INT_STATUS_TARDYBBHABORT_FIELD =
{
    "TARDYBBHABORT",
#if RU_INCLUDE_DESC
    "",
    "[FATAL] This bit indicates that BBH aborted an upstream packet at a"
    "time it was considered tardy by EPN. It is valid only if bit 31 of"
    "register 0x4b0 (fatalTardyBbhAbortEn) is set."
    "0: BBH has not aborted a tardy upstream packet."
    "1: BBH has aborted a tardy upstream packet. All upstream data"
    "traffic has been disabled. EPN must be reset to recover from this"
    "condition.",
#endif
    EPN_BBH_UPS_ABORT_INT_STATUS_TARDYBBHABORT_FIELD_MASK,
    0,
    EPN_BBH_UPS_ABORT_INT_STATUS_TARDYBBHABORT_FIELD_WIDTH,
    EPN_BBH_UPS_ABORT_INT_STATUS_TARDYBBHABORT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UPS_ABORT_INT_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_BBH_UPS_ABORT_INT_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_BBH_UPS_ABORT_INT_MASK_RESERVED0_FIELD_MASK,
    0,
    EPN_BBH_UPS_ABORT_INT_MASK_RESERVED0_FIELD_WIDTH,
    EPN_BBH_UPS_ABORT_INT_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UPS_ABORT_INT_MASK_MASKTARDYBBHABORT
 ******************************************************************************/
const ru_field_rec EPN_BBH_UPS_ABORT_INT_MASK_MASKTARDYBBHABORT_FIELD =
{
    "MASKTARDYBBHABORT",
#if RU_INCLUDE_DESC
    "",
    "Mask BBH aborted an upstream packet at a time it was considered"
    "tardy by EPN interrupt.",
#endif
    EPN_BBH_UPS_ABORT_INT_MASK_MASKTARDYBBHABORT_FIELD_MASK,
    0,
    EPN_BBH_UPS_ABORT_INT_MASK_MASKTARDYBBHABORT_FIELD_WIDTH,
    EPN_BBH_UPS_ABORT_INT_MASK_MASKTARDYBBHABORT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_BBHUPFRABORTMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_BBHUPFRABORTMASK_FIELD =
{
    "BBHUPFRABORTMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_BBHUPFRABORTMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_BBHUPFRABORTMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_BBHUPFRABORTMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_INTL2SBURSTCAPOVERFLOWMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_INTL2SBURSTCAPOVERFLOWMASK_FIELD =
{
    "INTL2SBURSTCAPOVERFLOWMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_INTL2SBURSTCAPOVERFLOWMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_INTL2SBURSTCAPOVERFLOWMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_INTL2SBURSTCAPOVERFLOWMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_INTCOEMPTYRPTMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_INTCOEMPTYRPTMASK_FIELD =
{
    "INTCOEMPTYRPTMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_INTCOEMPTYRPTMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_INTCOEMPTYRPTMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_INTCOEMPTYRPTMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_INTDRXERRABORTMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_INTDRXERRABORTMASK_FIELD =
{
    "INTDRXERRABORTMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_INTDRXERRABORTMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_INTDRXERRABORTMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_INTDRXERRABORTMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_INTL2SFIFOOVERRUNMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_INTL2SFIFOOVERRUNMASK_FIELD =
{
    "INTL2SFIFOOVERRUNMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_INTL2SFIFOOVERRUNMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_INTL2SFIFOOVERRUNMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_INTL2SFIFOOVERRUNMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_INTCO1588TSMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_INTCO1588TSMASK_FIELD =
{
    "INTCO1588TSMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_INTCO1588TSMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_INTCO1588TSMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_INTCO1588TSMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_INTCORPTPRESMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_INTCORPTPRESMASK_FIELD =
{
    "INTCORPTPRESMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_INTCORPTPRESMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_INTCORPTPRESMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_INTCORPTPRESMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_INTCOGNTPRESMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_INTCOGNTPRESMASK_FIELD =
{
    "INTCOGNTPRESMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_INTCOGNTPRESMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_INTCOGNTPRESMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_INTCOGNTPRESMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_INTCODELSTALEGNTMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_INTCODELSTALEGNTMASK_FIELD =
{
    "INTCODELSTALEGNTMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_INTCODELSTALEGNTMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_INTCODELSTALEGNTMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_INTCODELSTALEGNTMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_INTCOGNTNONPOLLMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_INTCOGNTNONPOLLMASK_FIELD =
{
    "INTCOGNTNONPOLLMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_INTCOGNTNONPOLLMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_INTCOGNTNONPOLLMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_INTCOGNTNONPOLLMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_INTCOGNTMISALIGNMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_INTCOGNTMISALIGNMASK_FIELD =
{
    "INTCOGNTMISALIGNMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_INTCOGNTMISALIGNMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_INTCOGNTMISALIGNMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_INTCOGNTMISALIGNMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_INTCOGNTTOOFARMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_INTCOGNTTOOFARMASK_FIELD =
{
    "INTCOGNTTOOFARMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_INTCOGNTTOOFARMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_INTCOGNTTOOFARMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_INTCOGNTTOOFARMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_INTCOGNTINTERVALMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_INTCOGNTINTERVALMASK_FIELD =
{
    "INTCOGNTINTERVALMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_INTCOGNTINTERVALMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_INTCOGNTINTERVALMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_INTCOGNTINTERVALMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_INTCOGNTDISCOVERYMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_INTCOGNTDISCOVERYMASK_FIELD =
{
    "INTCOGNTDISCOVERYMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_INTCOGNTDISCOVERYMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_INTCOGNTDISCOVERYMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_INTCOGNTDISCOVERYMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_INTCOGNTMISSABORTMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_INTCOGNTMISSABORTMASK_FIELD =
{
    "INTCOGNTMISSABORTMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_INTCOGNTMISSABORTMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_INTCOGNTMISSABORTMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_INTCOGNTMISSABORTMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_INTCOGNTFULLABORTMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_INTCOGNTFULLABORTMASK_FIELD =
{
    "INTCOGNTFULLABORTMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_INTCOGNTFULLABORTMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_INTCOGNTFULLABORTMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_INTCOGNTFULLABORTMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_BADUPFRLENMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_BADUPFRLENMASK_FIELD =
{
    "BADUPFRLENMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_BADUPFRLENMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_BADUPFRLENMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_BADUPFRLENMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_UPTARDYPACKETMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_UPTARDYPACKETMASK_FIELD =
{
    "UPTARDYPACKETMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_UPTARDYPACKETMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_UPTARDYPACKETMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_UPTARDYPACKETMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_UPRPTFRXMTMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_UPRPTFRXMTMASK_FIELD =
{
    "UPRPTFRXMTMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_UPRPTFRXMTMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_UPRPTFRXMTMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_UPRPTFRXMTMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_INTBIFIFOOVERRUNMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_INTBIFIFOOVERRUNMASK_FIELD =
{
    "INTBIFIFOOVERRUNMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_INTBIFIFOOVERRUNMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_INTBIFIFOOVERRUNMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_INTBIFIFOOVERRUNMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_BURSTGNTTOOBIGMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_BURSTGNTTOOBIGMASK_FIELD =
{
    "BURSTGNTTOOBIGMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_BURSTGNTTOOBIGMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_BURSTGNTTOOBIGMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_BURSTGNTTOOBIGMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_WRGNTTOOBIGMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_WRGNTTOOBIGMASK_FIELD =
{
    "WRGNTTOOBIGMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_WRGNTTOOBIGMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_WRGNTTOOBIGMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_WRGNTTOOBIGMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_RCVGNTTOOBIGMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_RCVGNTTOOBIGMASK_FIELD =
{
    "RCVGNTTOOBIGMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_RCVGNTTOOBIGMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_RCVGNTTOOBIGMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_RCVGNTTOOBIGMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_DNSTATSOVERRUNMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_DNSTATSOVERRUNMASK_FIELD =
{
    "DNSTATSOVERRUNMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_DNSTATSOVERRUNMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_DNSTATSOVERRUNMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_DNSTATSOVERRUNMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_INTUPSTATSOVERRUNMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_INTUPSTATSOVERRUNMASK_FIELD =
{
    "INTUPSTATSOVERRUNMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_INTUPSTATSOVERRUNMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_INTUPSTATSOVERRUNMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_INTUPSTATSOVERRUNMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_DNOUTOFORDERMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_DNOUTOFORDERMASK_FIELD =
{
    "DNOUTOFORDERMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_DNOUTOFORDERMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_DNOUTOFORDERMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_DNOUTOFORDERMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_TRUANTBBHHALTMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_TRUANTBBHHALTMASK_FIELD =
{
    "TRUANTBBHHALTMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_TRUANTBBHHALTMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_TRUANTBBHHALTMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_TRUANTBBHHALTMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_UPINVLDGNTLENMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_UPINVLDGNTLENMASK_FIELD =
{
    "UPINVLDGNTLENMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_UPINVLDGNTLENMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_UPINVLDGNTLENMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_UPINVLDGNTLENMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_INTCOBBHUPSFAULTMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_INTCOBBHUPSFAULTMASK_FIELD =
{
    "INTCOBBHUPSFAULTMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_INTCOBBHUPSFAULTMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_INTCOBBHUPSFAULTMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_INTCOBBHUPSFAULTMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_DNTIMEINSYNCMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_DNTIMEINSYNCMASK_FIELD =
{
    "DNTIMEINSYNCMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_DNTIMEINSYNCMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_DNTIMEINSYNCMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_DNTIMEINSYNCMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_DNTIMENOTINSYNCMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_DNTIMENOTINSYNCMASK_FIELD =
{
    "DNTIMENOTINSYNCMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_DNTIMENOTINSYNCMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_DNTIMENOTINSYNCMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_DNTIMENOTINSYNCMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAIN_INT_MASK_DPORTRDYMASK
 ******************************************************************************/
const ru_field_rec EPN_MAIN_INT_MASK_DPORTRDYMASK_FIELD =
{
    "DPORTRDYMASK",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAIN_INT_MASK_DPORTRDYMASK_FIELD_MASK,
    0,
    EPN_MAIN_INT_MASK_DPORTRDYMASK_FIELD_WIDTH,
    EPN_MAIN_INT_MASK_DPORTRDYMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAX_GNT_SIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_MAX_GNT_SIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAX_GNT_SIZE_RESERVED0_FIELD_MASK,
    0,
    EPN_MAX_GNT_SIZE_RESERVED0_FIELD_WIDTH,
    EPN_MAX_GNT_SIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAX_GNT_SIZE_MAXGNTSIZE
 ******************************************************************************/
const ru_field_rec EPN_MAX_GNT_SIZE_MAXGNTSIZE_FIELD =
{
    "MAXGNTSIZE",
#if RU_INCLUDE_DESC
    "",
    "Sets the Grant Size threshold for the three Grant Too Big"
    "interrupts. Units are TQ.",
#endif
    EPN_MAX_GNT_SIZE_MAXGNTSIZE_FIELD_MASK,
    0,
    EPN_MAX_GNT_SIZE_MAXGNTSIZE_FIELD_WIDTH,
    EPN_MAX_GNT_SIZE_MAXGNTSIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAX_FRAME_SIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_MAX_FRAME_SIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MAX_FRAME_SIZE_RESERVED0_FIELD_MASK,
    0,
    EPN_MAX_FRAME_SIZE_RESERVED0_FIELD_WIDTH,
    EPN_MAX_FRAME_SIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MAX_FRAME_SIZE_CFGMAXFRAMESIZE
 ******************************************************************************/
const ru_field_rec EPN_MAX_FRAME_SIZE_CFGMAXFRAMESIZE_FIELD =
{
    "CFGMAXFRAMESIZE",
#if RU_INCLUDE_DESC
    "",
    "Maximum allowable downstream frame size. Frames larger than this"
    "value are discarded.",
#endif
    EPN_MAX_FRAME_SIZE_CFGMAXFRAMESIZE_FIELD_MASK,
    0,
    EPN_MAX_FRAME_SIZE_CFGMAXFRAMESIZE_FIELD_WIDTH,
    EPN_MAX_FRAME_SIZE_CFGMAXFRAMESIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GRANT_OVR_HD_GNTOVRHDFEC
 ******************************************************************************/
const ru_field_rec EPN_GRANT_OVR_HD_GNTOVRHDFEC_FIELD =
{
    "GNTOVRHDFEC",
#if RU_INCLUDE_DESC
    "",
    "1G upstream mode -> Grant length consumed by overhead when FEC is"
    "enabled."
    "10G upstream mode -> Not used.",
#endif
    EPN_GRANT_OVR_HD_GNTOVRHDFEC_FIELD_MASK,
    0,
    EPN_GRANT_OVR_HD_GNTOVRHDFEC_FIELD_WIDTH,
    EPN_GRANT_OVR_HD_GNTOVRHDFEC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GRANT_OVR_HD_GNTOVRHD
 ******************************************************************************/
const ru_field_rec EPN_GRANT_OVR_HD_GNTOVRHD_FIELD =
{
    "GNTOVRHD",
#if RU_INCLUDE_DESC
    "",
    "1G upstream mode -> Grant length consumed by overhead when FEC is"
    "disabled."
    "10G upstream mode -> Grant length consumed by overhead. Used for"
    "both FEC and FEC-less modes.",
#endif
    EPN_GRANT_OVR_HD_GNTOVRHD_FIELD_MASK,
    0,
    EPN_GRANT_OVR_HD_GNTOVRHD_FIELD_WIDTH,
    EPN_GRANT_OVR_HD_GNTOVRHD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_POLL_SIZE_POLLSIZEFEC
 ******************************************************************************/
const ru_field_rec EPN_POLL_SIZE_POLLSIZEFEC_FIELD =
{
    "POLLSIZEFEC",
#if RU_INCLUDE_DESC
    "",
    "Size of polling grants when FEC is enabled. Units are TQ. Defaults"
    "to 64.",
#endif
    EPN_POLL_SIZE_POLLSIZEFEC_FIELD_MASK,
    0,
    EPN_POLL_SIZE_POLLSIZEFEC_FIELD_WIDTH,
    EPN_POLL_SIZE_POLLSIZEFEC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_POLL_SIZE_POLLSIZE
 ******************************************************************************/
const ru_field_rec EPN_POLL_SIZE_POLLSIZE_FIELD =
{
    "POLLSIZE",
#if RU_INCLUDE_DESC
    "",
    "Size of polling grants when FEC is disabled. Units are TQ. Defaults"
    "to 64.",
#endif
    EPN_POLL_SIZE_POLLSIZE_FIELD_MASK,
    0,
    EPN_POLL_SIZE_POLLSIZE_FIELD_WIDTH,
    EPN_POLL_SIZE_POLLSIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DN_RD_GNT_MARGIN_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_DN_RD_GNT_MARGIN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DN_RD_GNT_MARGIN_RESERVED0_FIELD_MASK,
    0,
    EPN_DN_RD_GNT_MARGIN_RESERVED0_FIELD_WIDTH,
    EPN_DN_RD_GNT_MARGIN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DN_RD_GNT_MARGIN_RDGNTSTARTMARGIN
 ******************************************************************************/
const ru_field_rec EPN_DN_RD_GNT_MARGIN_RDGNTSTARTMARGIN_FIELD =
{
    "RDGNTSTARTMARGIN",
#if RU_INCLUDE_DESC
    "",
    "How far in advance of Grant Start Time to consider a grant for"
    "removal from the grant FIFO.",
#endif
    EPN_DN_RD_GNT_MARGIN_RDGNTSTARTMARGIN_FIELD_MASK,
    0,
    EPN_DN_RD_GNT_MARGIN_RDGNTSTARTMARGIN_FIELD_WIDTH,
    EPN_DN_RD_GNT_MARGIN_RDGNTSTARTMARGIN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_TIME_START_DELTA_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_GNT_TIME_START_DELTA_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_GNT_TIME_START_DELTA_RESERVED0_FIELD_MASK,
    0,
    EPN_GNT_TIME_START_DELTA_RESERVED0_FIELD_WIDTH,
    EPN_GNT_TIME_START_DELTA_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_TIME_START_DELTA_GNTSTARTTIMEDELTA
 ******************************************************************************/
const ru_field_rec EPN_GNT_TIME_START_DELTA_GNTSTARTTIMEDELTA_FIELD =
{
    "GNTSTARTTIMEDELTA",
#if RU_INCLUDE_DESC
    "",
    "This value determines how far in advance of the Grant Start Time"
    "that the next selected grant (already extracted from the Grant FIFO)"
    "will be handed to the EPN UTX (upstream transmit) logic and start to"
    "pre-fetch frames.."
    "The units of this register are TQ. The reset default value is 640"
    "decimal.",
#endif
    EPN_GNT_TIME_START_DELTA_GNTSTARTTIMEDELTA_FIELD_MASK,
    0,
    EPN_GNT_TIME_START_DELTA_GNTSTARTTIMEDELTA_FIELD_WIDTH,
    EPN_GNT_TIME_START_DELTA_GNTSTARTTIMEDELTA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_TIME_STAMP_DIFF_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_TIME_STAMP_DIFF_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_TIME_STAMP_DIFF_RESERVED0_FIELD_MASK,
    0,
    EPN_TIME_STAMP_DIFF_RESERVED0_FIELD_WIDTH,
    EPN_TIME_STAMP_DIFF_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_TIME_STAMP_DIFF_TIMESTAMPDIFFDELTA
 ******************************************************************************/
const ru_field_rec EPN_TIME_STAMP_DIFF_TIMESTAMPDIFFDELTA_FIELD =
{
    "TIMESTAMPDIFFDELTA",
#if RU_INCLUDE_DESC
    "",
    "Threshold for local time reference updates and related interrupts."
    "Reset default is10 decimal.",
#endif
    EPN_TIME_STAMP_DIFF_TIMESTAMPDIFFDELTA_FIELD_MASK,
    0,
    EPN_TIME_STAMP_DIFF_TIMESTAMPDIFFDELTA_FIELD_WIDTH,
    EPN_TIME_STAMP_DIFF_TIMESTAMPDIFFDELTA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_UP_TIME_STAMP_OFF_TIMESTAMPOFFSETFEC
 ******************************************************************************/
const ru_field_rec EPN_UP_TIME_STAMP_OFF_TIMESTAMPOFFSETFEC_FIELD =
{
    "TIMESTAMPOFFSETFEC",
#if RU_INCLUDE_DESC
    "",
    "Offset from Grant Start Time to use as the Timestamp field in"
    "REPORTs and processor-sent packets when FEC is enabled."
    "Only used for 1G modes."
    "Units are TQ.",
#endif
    EPN_UP_TIME_STAMP_OFF_TIMESTAMPOFFSETFEC_FIELD_MASK,
    0,
    EPN_UP_TIME_STAMP_OFF_TIMESTAMPOFFSETFEC_FIELD_WIDTH,
    EPN_UP_TIME_STAMP_OFF_TIMESTAMPOFFSETFEC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_UP_TIME_STAMP_OFF_TIMESTAMPOFFSET
 ******************************************************************************/
const ru_field_rec EPN_UP_TIME_STAMP_OFF_TIMESTAMPOFFSET_FIELD =
{
    "TIMESTAMPOFFSET",
#if RU_INCLUDE_DESC
    "",
    "Offset from Grant Start Time to use as the Timestamp field in"
    "REPORTs and processor-sent packets when FEC is disabled."
    "Used for both 1G no FEC and all 10G modes."
    "Units are TQ.",
#endif
    EPN_UP_TIME_STAMP_OFF_TIMESTAMPOFFSET_FIELD_MASK,
    0,
    EPN_UP_TIME_STAMP_OFF_TIMESTAMPOFFSET_FIELD_WIDTH,
    EPN_UP_TIME_STAMP_OFF_TIMESTAMPOFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_INTERVAL_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_GNT_INTERVAL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_GNT_INTERVAL_RESERVED0_FIELD_MASK,
    0,
    EPN_GNT_INTERVAL_RESERVED0_FIELD_WIDTH,
    EPN_GNT_INTERVAL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GNT_INTERVAL_GNTINTERVAL
 ******************************************************************************/
const ru_field_rec EPN_GNT_INTERVAL_GNTINTERVAL_FIELD =
{
    "GNTINTERVAL",
#if RU_INCLUDE_DESC
    "",
    "Grant interval",
#endif
    EPN_GNT_INTERVAL_GNTINTERVAL_FIELD_MASK,
    0,
    EPN_GNT_INTERVAL_GNTINTERVAL_FIELD_WIDTH,
    EPN_GNT_INTERVAL_GNTINTERVAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DN_GNT_MISALIGN_THR_PRVUNUSEDGNTTHRESHOLD
 ******************************************************************************/
const ru_field_rec EPN_DN_GNT_MISALIGN_THR_PRVUNUSEDGNTTHRESHOLD_FIELD =
{
    "PRVUNUSEDGNTTHRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "Sets the minimum number of unused time quanta in a grant required in"
    "order for it to be considered misaligned.",
#endif
    EPN_DN_GNT_MISALIGN_THR_PRVUNUSEDGNTTHRESHOLD_FIELD_MASK,
    0,
    EPN_DN_GNT_MISALIGN_THR_PRVUNUSEDGNTTHRESHOLD_FIELD_WIDTH,
    EPN_DN_GNT_MISALIGN_THR_PRVUNUSEDGNTTHRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DN_GNT_MISALIGN_THR_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_DN_GNT_MISALIGN_THR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DN_GNT_MISALIGN_THR_RESERVED0_FIELD_MASK,
    0,
    EPN_DN_GNT_MISALIGN_THR_RESERVED0_FIELD_WIDTH,
    EPN_DN_GNT_MISALIGN_THR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DN_GNT_MISALIGN_THR_GNTMISALIGNTHRESH
 ******************************************************************************/
const ru_field_rec EPN_DN_GNT_MISALIGN_THR_GNTMISALIGNTHRESH_FIELD =
{
    "GNTMISALIGNTHRESH",
#if RU_INCLUDE_DESC
    "",
    "Sets the number of misaligned grants needed to trigger misalignment"
    "handling. The value set here is one fewer than the desired number of"
    "consecutive misaligned grants, i.e. setting a value of 2 means that"
    "three misaligned grants in a row will trigger misalignment handling.",
#endif
    EPN_DN_GNT_MISALIGN_THR_GNTMISALIGNTHRESH_FIELD_MASK,
    0,
    EPN_DN_GNT_MISALIGN_THR_GNTMISALIGNTHRESH_FIELD_WIDTH,
    EPN_DN_GNT_MISALIGN_THR_GNTMISALIGNTHRESH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DN_GNT_MISALIGN_PAUSE_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_DN_GNT_MISALIGN_PAUSE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DN_GNT_MISALIGN_PAUSE_RESERVED0_FIELD_MASK,
    0,
    EPN_DN_GNT_MISALIGN_PAUSE_RESERVED0_FIELD_WIDTH,
    EPN_DN_GNT_MISALIGN_PAUSE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DN_GNT_MISALIGN_PAUSE_GNTMISALIGNPAUSE
 ******************************************************************************/
const ru_field_rec EPN_DN_GNT_MISALIGN_PAUSE_GNTMISALIGNPAUSE_FIELD =
{
    "GNTMISALIGNPAUSE",
#if RU_INCLUDE_DESC
    "",
    "How long to stall reporting of queue status.",
#endif
    EPN_DN_GNT_MISALIGN_PAUSE_GNTMISALIGNPAUSE_FIELD_MASK,
    0,
    EPN_DN_GNT_MISALIGN_PAUSE_GNTMISALIGNPAUSE_FIELD_WIDTH,
    EPN_DN_GNT_MISALIGN_PAUSE_GNTMISALIGNPAUSE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_NON_POLL_INTV_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_NON_POLL_INTV_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_NON_POLL_INTV_RESERVED0_FIELD_MASK,
    0,
    EPN_NON_POLL_INTV_RESERVED0_FIELD_WIDTH,
    EPN_NON_POLL_INTV_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_NON_POLL_INTV_NONPOLLGNTINTV
 ******************************************************************************/
const ru_field_rec EPN_NON_POLL_INTV_NONPOLLGNTINTV_FIELD =
{
    "NONPOLLGNTINTV",
#if RU_INCLUDE_DESC
    "",
    "If amount of time since last non poll grant exceed this value, the"
    "respective LLID's interrupt will assert. Units of 65 us.",
#endif
    EPN_NON_POLL_INTV_NONPOLLGNTINTV_FIELD_MASK,
    0,
    EPN_NON_POLL_INTV_NONPOLLGNTINTV_FIELD_WIDTH,
    EPN_NON_POLL_INTV_NONPOLLGNTINTV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_FORCE_FCS_ERR_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_FORCE_FCS_ERR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_FORCE_FCS_ERR_RESERVED0_FIELD_MASK,
    0,
    EPN_FORCE_FCS_ERR_RESERVED0_FIELD_WIDTH,
    EPN_FORCE_FCS_ERR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_FORCE_FCS_ERR_FORCEFCSERR7
 ******************************************************************************/
const ru_field_rec EPN_FORCE_FCS_ERR_FORCEFCSERR7_FIELD =
{
    "FORCEFCSERR7",
#if RU_INCLUDE_DESC
    "",
    "Force bad FCS for frames transmitting out of LLID 7",
#endif
    EPN_FORCE_FCS_ERR_FORCEFCSERR7_FIELD_MASK,
    0,
    EPN_FORCE_FCS_ERR_FORCEFCSERR7_FIELD_WIDTH,
    EPN_FORCE_FCS_ERR_FORCEFCSERR7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_FORCE_FCS_ERR_FORCEFCSERR6
 ******************************************************************************/
const ru_field_rec EPN_FORCE_FCS_ERR_FORCEFCSERR6_FIELD =
{
    "FORCEFCSERR6",
#if RU_INCLUDE_DESC
    "",
    "Force bad FCS for frames transmitting out of LLID 6",
#endif
    EPN_FORCE_FCS_ERR_FORCEFCSERR6_FIELD_MASK,
    0,
    EPN_FORCE_FCS_ERR_FORCEFCSERR6_FIELD_WIDTH,
    EPN_FORCE_FCS_ERR_FORCEFCSERR6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_FORCE_FCS_ERR_FORCEFCSERR5
 ******************************************************************************/
const ru_field_rec EPN_FORCE_FCS_ERR_FORCEFCSERR5_FIELD =
{
    "FORCEFCSERR5",
#if RU_INCLUDE_DESC
    "",
    "Force bad FCS for frames transmitting out of LLID 5",
#endif
    EPN_FORCE_FCS_ERR_FORCEFCSERR5_FIELD_MASK,
    0,
    EPN_FORCE_FCS_ERR_FORCEFCSERR5_FIELD_WIDTH,
    EPN_FORCE_FCS_ERR_FORCEFCSERR5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_FORCE_FCS_ERR_FORCEFCSERR4
 ******************************************************************************/
const ru_field_rec EPN_FORCE_FCS_ERR_FORCEFCSERR4_FIELD =
{
    "FORCEFCSERR4",
#if RU_INCLUDE_DESC
    "",
    "Force bad FCS for frames transmitting out of LLID 4",
#endif
    EPN_FORCE_FCS_ERR_FORCEFCSERR4_FIELD_MASK,
    0,
    EPN_FORCE_FCS_ERR_FORCEFCSERR4_FIELD_WIDTH,
    EPN_FORCE_FCS_ERR_FORCEFCSERR4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_FORCE_FCS_ERR_FORCEFCSERR3
 ******************************************************************************/
const ru_field_rec EPN_FORCE_FCS_ERR_FORCEFCSERR3_FIELD =
{
    "FORCEFCSERR3",
#if RU_INCLUDE_DESC
    "",
    "Force bad FCS for frames transmitting out of LLID 3",
#endif
    EPN_FORCE_FCS_ERR_FORCEFCSERR3_FIELD_MASK,
    0,
    EPN_FORCE_FCS_ERR_FORCEFCSERR3_FIELD_WIDTH,
    EPN_FORCE_FCS_ERR_FORCEFCSERR3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_FORCE_FCS_ERR_FORCEFCSERR2
 ******************************************************************************/
const ru_field_rec EPN_FORCE_FCS_ERR_FORCEFCSERR2_FIELD =
{
    "FORCEFCSERR2",
#if RU_INCLUDE_DESC
    "",
    "Force bad FCS for frames transmitting out of LLID 2",
#endif
    EPN_FORCE_FCS_ERR_FORCEFCSERR2_FIELD_MASK,
    0,
    EPN_FORCE_FCS_ERR_FORCEFCSERR2_FIELD_WIDTH,
    EPN_FORCE_FCS_ERR_FORCEFCSERR2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_FORCE_FCS_ERR_FORCEFCSERR1
 ******************************************************************************/
const ru_field_rec EPN_FORCE_FCS_ERR_FORCEFCSERR1_FIELD =
{
    "FORCEFCSERR1",
#if RU_INCLUDE_DESC
    "",
    "Force bad FCS for frames transmitting out of LLID 1",
#endif
    EPN_FORCE_FCS_ERR_FORCEFCSERR1_FIELD_MASK,
    0,
    EPN_FORCE_FCS_ERR_FORCEFCSERR1_FIELD_WIDTH,
    EPN_FORCE_FCS_ERR_FORCEFCSERR1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_FORCE_FCS_ERR_FORCEFCSERR0
 ******************************************************************************/
const ru_field_rec EPN_FORCE_FCS_ERR_FORCEFCSERR0_FIELD =
{
    "FORCEFCSERR0",
#if RU_INCLUDE_DESC
    "",
    "Force bad FCS for frames transmitting out of LLID 0",
#endif
    EPN_FORCE_FCS_ERR_FORCEFCSERR0_FIELD_MASK,
    0,
    EPN_FORCE_FCS_ERR_FORCEFCSERR0_FIELD_WIDTH,
    EPN_FORCE_FCS_ERR_FORCEFCSERR0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GRANT_OVERLAP_LIMIT_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_GRANT_OVERLAP_LIMIT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_GRANT_OVERLAP_LIMIT_RESERVED0_FIELD_MASK,
    0,
    EPN_GRANT_OVERLAP_LIMIT_RESERVED0_FIELD_WIDTH,
    EPN_GRANT_OVERLAP_LIMIT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_GRANT_OVERLAP_LIMIT_PRVGRANTOVERLAPLIMIT
 ******************************************************************************/
const ru_field_rec EPN_GRANT_OVERLAP_LIMIT_PRVGRANTOVERLAPLIMIT_FIELD =
{
    "PRVGRANTOVERLAPLIMIT",
#if RU_INCLUDE_DESC
    "",
    "Number of time quanta by which two consecutive grants are allowed to"
    "overlap",
#endif
    EPN_GRANT_OVERLAP_LIMIT_PRVGRANTOVERLAPLIMIT_FIELD_MASK,
    0,
    EPN_GRANT_OVERLAP_LIMIT_PRVGRANTOVERLAPLIMIT_FIELD_WIDTH,
    EPN_GRANT_OVERLAP_LIMIT_PRVGRANTOVERLAPLIMIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_AES_CONFIGURATION_0_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_AES_CONFIGURATION_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_AES_CONFIGURATION_0_RESERVED0_FIELD_MASK,
    0,
    EPN_AES_CONFIGURATION_0_RESERVED0_FIELD_WIDTH,
    EPN_AES_CONFIGURATION_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_AES_CONFIGURATION_0_PRVUPSTREAMAESMODE_0
 ******************************************************************************/
const ru_field_rec EPN_AES_CONFIGURATION_0_PRVUPSTREAMAESMODE_0_FIELD =
{
    "PRVUPSTREAMAESMODE_0",
#if RU_INCLUDE_DESC
    "",
    "LLID index 0 AES overhead mode."
    "0: Implicit SCI AES overhead mode."
    "1: Explicit SCI AES overhead mode."
    "."
    "."
    "LLID index 7 AES overhead mode."
    "14: Implicit SCI AES overhead mode."
    "15: Explicit SCI AES overhead mode.",
#endif
    EPN_AES_CONFIGURATION_0_PRVUPSTREAMAESMODE_0_FIELD_MASK,
    0,
    EPN_AES_CONFIGURATION_0_PRVUPSTREAMAESMODE_0_FIELD_WIDTH,
    EPN_AES_CONFIGURATION_0_PRVUPSTREAMAESMODE_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DISC_GRANT_OVR_HD_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_DISC_GRANT_OVR_HD_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DISC_GRANT_OVR_HD_RESERVED0_FIELD_MASK,
    0,
    EPN_DISC_GRANT_OVR_HD_RESERVED0_FIELD_WIDTH,
    EPN_DISC_GRANT_OVR_HD_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DISC_GRANT_OVR_HD_DISCGNTOVRHD
 ******************************************************************************/
const ru_field_rec EPN_DISC_GRANT_OVR_HD_DISCGNTOVRHD_FIELD =
{
    "DISCGNTOVRHD",
#if RU_INCLUDE_DESC
    "",
    "This defines the amount of overhead used for Discovery gates. Units"
    "are 16-bit words.",
#endif
    EPN_DISC_GRANT_OVR_HD_DISCGNTOVRHD_FIELD_MASK,
    0,
    EPN_DISC_GRANT_OVR_HD_DISCGNTOVRHD_FIELD_WIDTH,
    EPN_DISC_GRANT_OVR_HD_DISCGNTOVRHD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DN_DISCOVERY_SEED_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_DN_DISCOVERY_SEED_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DN_DISCOVERY_SEED_RESERVED0_FIELD_MASK,
    0,
    EPN_DN_DISCOVERY_SEED_RESERVED0_FIELD_WIDTH,
    EPN_DN_DISCOVERY_SEED_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DN_DISCOVERY_SEED_CFGDISCSEED
 ******************************************************************************/
const ru_field_rec EPN_DN_DISCOVERY_SEED_CFGDISCSEED_FIELD =
{
    "CFGDISCSEED",
#if RU_INCLUDE_DESC
    "",
    "Specifies basis for generating the discovery offset. Units are TQ.",
#endif
    EPN_DN_DISCOVERY_SEED_CFGDISCSEED_FIELD_MASK,
    0,
    EPN_DN_DISCOVERY_SEED_CFGDISCSEED_FIELD_WIDTH,
    EPN_DN_DISCOVERY_SEED_CFGDISCSEED_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DN_DISCOVERY_INC_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_DN_DISCOVERY_INC_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DN_DISCOVERY_INC_RESERVED0_FIELD_MASK,
    0,
    EPN_DN_DISCOVERY_INC_RESERVED0_FIELD_WIDTH,
    EPN_DN_DISCOVERY_INC_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DN_DISCOVERY_INC_CFGDISCINC
 ******************************************************************************/
const ru_field_rec EPN_DN_DISCOVERY_INC_CFGDISCINC_FIELD =
{
    "CFGDISCINC",
#if RU_INCLUDE_DESC
    "",
    "Units are TQ",
#endif
    EPN_DN_DISCOVERY_INC_CFGDISCINC_FIELD_MASK,
    0,
    EPN_DN_DISCOVERY_INC_CFGDISCINC_FIELD_WIDTH,
    EPN_DN_DISCOVERY_INC_CFGDISCINC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DN_DISCOVERY_SIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_DN_DISCOVERY_SIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DN_DISCOVERY_SIZE_RESERVED0_FIELD_MASK,
    0,
    EPN_DN_DISCOVERY_SIZE_RESERVED0_FIELD_WIDTH,
    EPN_DN_DISCOVERY_SIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DN_DISCOVERY_SIZE_CFGDISCSIZE
 ******************************************************************************/
const ru_field_rec EPN_DN_DISCOVERY_SIZE_CFGDISCSIZE_FIELD =
{
    "CFGDISCSIZE",
#if RU_INCLUDE_DESC
    "",
    "Size of response to discovery gate. Units are TQ.",
#endif
    EPN_DN_DISCOVERY_SIZE_CFGDISCSIZE_FIELD_MASK,
    0,
    EPN_DN_DISCOVERY_SIZE_CFGDISCSIZE_FIELD_WIDTH,
    EPN_DN_DISCOVERY_SIZE_CFGDISCSIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_FEC_IPG_LENGTH_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_FEC_IPG_LENGTH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_FEC_IPG_LENGTH_RESERVED0_FIELD_MASK,
    0,
    EPN_FEC_IPG_LENGTH_RESERVED0_FIELD_WIDTH,
    EPN_FEC_IPG_LENGTH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_FEC_IPG_LENGTH_MODIPGPREAMBLEBYTES
 ******************************************************************************/
const ru_field_rec EPN_FEC_IPG_LENGTH_MODIPGPREAMBLEBYTES_FIELD =
{
    "MODIPGPREAMBLEBYTES",
#if RU_INCLUDE_DESC
    "",
    "10G and 1G upstream: The number of the bytes in the sum of IPG +"
    "PREAMBLE.  This value must be programmed before the upstream is"
    "enabled.  It must not be modified while the upstream is active."
    "Default is 20. Units are bytes.",
#endif
    EPN_FEC_IPG_LENGTH_MODIPGPREAMBLEBYTES_FIELD_MASK,
    0,
    EPN_FEC_IPG_LENGTH_MODIPGPREAMBLEBYTES_FIELD_WIDTH,
    EPN_FEC_IPG_LENGTH_MODIPGPREAMBLEBYTES_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_FEC_IPG_LENGTH_CFGRPTLEN
 ******************************************************************************/
const ru_field_rec EPN_FEC_IPG_LENGTH_CFGRPTLEN_FIELD =
{
    "CFGRPTLEN",
#if RU_INCLUDE_DESC
    "",
    "1G upstream non-FEC: The length of the REPORT Frame + IPG +"
    "PREAMBLE. Used for non-FEC LLIDs. Use default value of 42."
    "10G upstream FEC : The length of the Report Frame + IPG + PREAMBLE +"
    "FEC. Set to 13."
    "10G upstream non-FEC : The length of the Report Frame + IPG +"
    "PREAMBLE. Set to 5."
    "Defaults to 42 for 1G non-FEC. Units are TQ.",
#endif
    EPN_FEC_IPG_LENGTH_CFGRPTLEN_FIELD_MASK,
    0,
    EPN_FEC_IPG_LENGTH_CFGRPTLEN_FIELD_WIDTH,
    EPN_FEC_IPG_LENGTH_CFGRPTLEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_FEC_IPG_LENGTH_CFGFECRPTLENGTH
 ******************************************************************************/
const ru_field_rec EPN_FEC_IPG_LENGTH_CFGFECRPTLENGTH_FIELD =
{
    "CFGFECRPTLENGTH",
#if RU_INCLUDE_DESC
    "",
    "The length of the REPORT Frame + IPG + PREAMBLE + FEC. Used only for"
    "1G FEC upstream operation."
    "Default is 58 for 1G upstream FEC. Units are TQ.",
#endif
    EPN_FEC_IPG_LENGTH_CFGFECRPTLENGTH_FIELD_MASK,
    0,
    EPN_FEC_IPG_LENGTH_CFGFECRPTLENGTH_FIELD_WIDTH,
    EPN_FEC_IPG_LENGTH_CFGFECRPTLENGTH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_FEC_IPG_LENGTH_CFGFECIPGLENGTH
 ******************************************************************************/
const ru_field_rec EPN_FEC_IPG_LENGTH_CFGFECIPGLENGTH_FIELD =
{
    "CFGFECIPGLENGTH",
#if RU_INCLUDE_DESC
    "",
    "Length of IPG to be used in 1G upstream FEC computations. There are"
    "8-bytes of overhead built into the Epn's 1G upstream FEC"
    "calculations."
    "This means that if this register is set to zero then the Epn will"
    "add 8-bytes of overhead to each packet transmitted upstream."
    "The size of the 1G upstream FEC IPG+preamble used by the Epn is"
    "(cfgFecIpgLength + 8)."
    "The smallest supported FEC IPG+preamble value is 8. To use the"
    "LIF's short preamble (7-byte minimum) capability; set the LIF's IPG"
    "value to 1 and set the Epn's cfgFecIpgLength to 0."
    "Default 10. Units are bytes.",
#endif
    EPN_FEC_IPG_LENGTH_CFGFECIPGLENGTH_FIELD_MASK,
    0,
    EPN_FEC_IPG_LENGTH_CFGFECIPGLENGTH_FIELD_WIDTH,
    EPN_FEC_IPG_LENGTH_CFGFECIPGLENGTH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_FAKE_REPORT_VALUE_EN_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_FAKE_REPORT_VALUE_EN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_FAKE_REPORT_VALUE_EN_RESERVED0_FIELD_MASK,
    0,
    EPN_FAKE_REPORT_VALUE_EN_RESERVED0_FIELD_WIDTH,
    EPN_FAKE_REPORT_VALUE_EN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_FAKE_REPORT_VALUE_EN_FAKEREPORTVALUEEN
 ******************************************************************************/
const ru_field_rec EPN_FAKE_REPORT_VALUE_EN_FAKEREPORTVALUEEN_FIELD =
{
    "FAKEREPORTVALUEEN",
#if RU_INCLUDE_DESC
    "",
    "Per-LLID Index bits for enabling fake reporting. Setting a bit will"
    "cause that Index to send fake reports (when requested by a Force"
    "Report grant) with the value specified in EPN Fake Report Value.",
#endif
    EPN_FAKE_REPORT_VALUE_EN_FAKEREPORTVALUEEN_FIELD_MASK,
    0,
    EPN_FAKE_REPORT_VALUE_EN_FAKEREPORTVALUEEN_FIELD_WIDTH,
    EPN_FAKE_REPORT_VALUE_EN_FAKEREPORTVALUEEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_FAKE_REPORT_VALUE_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_FAKE_REPORT_VALUE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_FAKE_REPORT_VALUE_RESERVED0_FIELD_MASK,
    0,
    EPN_FAKE_REPORT_VALUE_RESERVED0_FIELD_WIDTH,
    EPN_FAKE_REPORT_VALUE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_FAKE_REPORT_VALUE_FAKEREPORTVALUE
 ******************************************************************************/
const ru_field_rec EPN_FAKE_REPORT_VALUE_FAKEREPORTVALUE_FIELD =
{
    "FAKEREPORTVALUE",
#if RU_INCLUDE_DESC
    "",
    "The value to be used when sending fake reports. This value is in"
    "units of bytes. (This value is converted to TQ as necessary for the"
    "configured upstream data rate.)",
#endif
    EPN_FAKE_REPORT_VALUE_FAKEREPORTVALUE_FIELD_MASK,
    0,
    EPN_FAKE_REPORT_VALUE_FAKEREPORTVALUE_FIELD_WIDTH,
    EPN_FAKE_REPORT_VALUE_FAKEREPORTVALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BURST_CAP__RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_BURST_CAP__RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_BURST_CAP__RESERVED0_FIELD_MASK,
    0,
    EPN_BURST_CAP__RESERVED0_FIELD_WIDTH,
    EPN_BURST_CAP__RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BURST_CAP__BURSTCAP
 ******************************************************************************/
const ru_field_rec EPN_BURST_CAP__BURSTCAP_FIELD =
{
    "BURSTCAP",
#if RU_INCLUDE_DESC
    "",
    "Defines maximum size of a report on LLID index 0 in Tek mode and on"
    "L2 i in multi-priority mode.",
#endif
    EPN_BURST_CAP__BURSTCAP_FIELD_MASK,
    0,
    EPN_BURST_CAP__BURSTCAP_FIELD_WIDTH,
    EPN_BURST_CAP__BURSTCAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_QUEUE_LLID_MAP__RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_QUEUE_LLID_MAP__RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_QUEUE_LLID_MAP__RESERVED0_FIELD_MASK,
    0,
    EPN_QUEUE_LLID_MAP__RESERVED0_FIELD_WIDTH,
    EPN_QUEUE_LLID_MAP__RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_QUEUE_LLID_MAP__QUELLIDMAP
 ******************************************************************************/
const ru_field_rec EPN_QUEUE_LLID_MAP__QUELLIDMAP_FIELD =
{
    "QUELLIDMAP",
#if RU_INCLUDE_DESC
    "",
    "Selects the L2 FIFO to which Queue i is mapped.",
#endif
    EPN_QUEUE_LLID_MAP__QUELLIDMAP_FIELD_MASK,
    0,
    EPN_QUEUE_LLID_MAP__QUELLIDMAP_FIELD_WIDTH,
    EPN_QUEUE_LLID_MAP__QUELLIDMAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_VALID_OPCODE_MAP_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_VALID_OPCODE_MAP_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_VALID_OPCODE_MAP_RESERVED0_FIELD_MASK,
    0,
    EPN_VALID_OPCODE_MAP_RESERVED0_FIELD_WIDTH,
    EPN_VALID_OPCODE_MAP_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_VALID_OPCODE_MAP_PRVVALIDMPCPOPCODES
 ******************************************************************************/
const ru_field_rec EPN_VALID_OPCODE_MAP_PRVVALIDMPCPOPCODES_FIELD =
{
    "PRVVALIDMPCPOPCODES",
#if RU_INCLUDE_DESC
    "",
    "The 14 MPCP opcode values in the range of 2-15 can be enabled by"
    "setting the corresponding bit."
    "Note: Opcode values 0 and 1 must always be disabled!",
#endif
    EPN_VALID_OPCODE_MAP_PRVVALIDMPCPOPCODES_FIELD_MASK,
    0,
    EPN_VALID_OPCODE_MAP_PRVVALIDMPCPOPCODES_FIELD_WIDTH,
    EPN_VALID_OPCODE_MAP_PRVVALIDMPCPOPCODES_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_UP_PACKET_TX_MARGIN_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_UP_PACKET_TX_MARGIN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_UP_PACKET_TX_MARGIN_RESERVED0_FIELD_MASK,
    0,
    EPN_UP_PACKET_TX_MARGIN_RESERVED0_FIELD_WIDTH,
    EPN_UP_PACKET_TX_MARGIN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_UP_PACKET_TX_MARGIN_UPPACKETTXMARGIN
 ******************************************************************************/
const ru_field_rec EPN_UP_PACKET_TX_MARGIN_UPPACKETTXMARGIN_FIELD =
{
    "UPPACKETTXMARGIN",
#if RU_INCLUDE_DESC
    "",
    "Minimum upstream data setup time for LIF/XIF.  Units are TQ.",
#endif
    EPN_UP_PACKET_TX_MARGIN_UPPACKETTXMARGIN_FIELD_MASK,
    0,
    EPN_UP_PACKET_TX_MARGIN_UPPACKETTXMARGIN_FIELD_WIDTH,
    EPN_UP_PACKET_TX_MARGIN_UPPACKETTXMARGIN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MULTI_PRI_CFG_0_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_MULTI_PRI_CFG_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MULTI_PRI_CFG_0_RESERVED0_FIELD_MASK,
    0,
    EPN_MULTI_PRI_CFG_0_RESERVED0_FIELD_WIDTH,
    EPN_MULTI_PRI_CFG_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MULTI_PRI_CFG_0_CFGCTCSCHDEFICITEN
 ******************************************************************************/
const ru_field_rec EPN_MULTI_PRI_CFG_0_CFGCTCSCHDEFICITEN_FIELD =
{
    "CFGCTCSCHDEFICITEN",
#if RU_INCLUDE_DESC
    "",
    "Enables deficit accounting in the CTC scheduler. Applies only when"
    "the CTC scheduler is configured for weighted round-robin operation.",
#endif
    EPN_MULTI_PRI_CFG_0_CFGCTCSCHDEFICITEN_FIELD_MASK,
    0,
    EPN_MULTI_PRI_CFG_0_CFGCTCSCHDEFICITEN_FIELD_WIDTH,
    EPN_MULTI_PRI_CFG_0_CFGCTCSCHDEFICITEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MULTI_PRI_CFG_0_RESERVED1
 ******************************************************************************/
const ru_field_rec EPN_MULTI_PRI_CFG_0_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MULTI_PRI_CFG_0_RESERVED1_FIELD_MASK,
    0,
    EPN_MULTI_PRI_CFG_0_RESERVED1_FIELD_WIDTH,
    EPN_MULTI_PRI_CFG_0_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MULTI_PRI_CFG_0_PRVZEROBURSTCAPOVERRIDEMODE
 ******************************************************************************/
const ru_field_rec EPN_MULTI_PRI_CFG_0_PRVZEROBURSTCAPOVERRIDEMODE_FIELD =
{
    "PRVZEROBURSTCAPOVERRIDEMODE",
#if RU_INCLUDE_DESC
    "",
    "Determines an internal burst cap value for any priority/L2 which has"
    "its burst cap set to zero. The internal burst cap value is used by"
    "the L1-to-L2 packet transfer logic."
    "0: Use an internal burst cap value of zero. (Do not use.)"
    "1: Use an internal burst cap value of 2 KB. Reset default."
    "2: Use a \"max value\" internal burst cap: 128KB for 1G upstream."
    "3. Reserved (do not use)",
#endif
    EPN_MULTI_PRI_CFG_0_PRVZEROBURSTCAPOVERRIDEMODE_FIELD_MASK,
    0,
    EPN_MULTI_PRI_CFG_0_PRVZEROBURSTCAPOVERRIDEMODE_FIELD_WIDTH,
    EPN_MULTI_PRI_CFG_0_PRVZEROBURSTCAPOVERRIDEMODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MULTI_PRI_CFG_0_RESERVED2
 ******************************************************************************/
const ru_field_rec EPN_MULTI_PRI_CFG_0_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MULTI_PRI_CFG_0_RESERVED2_FIELD_MASK,
    0,
    EPN_MULTI_PRI_CFG_0_RESERVED2_FIELD_WIDTH,
    EPN_MULTI_PRI_CFG_0_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MULTI_PRI_CFG_0_CFGSHAREDL2
 ******************************************************************************/
const ru_field_rec EPN_MULTI_PRI_CFG_0_CFGSHAREDL2_FIELD =
{
    "CFGSHAREDL2",
#if RU_INCLUDE_DESC
    "",
    "For the burst-cap limited queue set, configures whether each queue"
    "report/priority uses a separate L2 queue, or a shared one. Setting"
    "this bit, along with cfgSharedBurstCap, configures EPN into a"
    "TK3715-compatible reporting mode."
    "Note: Using a shared L2 queue effectively disables the CTC output"
    "scheduler, since packets from all priorities are merged into a"
    "single pipe and scheduled (in strict priority) upon entry to the L2"
    "queue."
    "0: Each priority level uses its own dedicated L2 queue."
    "1: All priority levels use a single, shared L2 queue",
#endif
    EPN_MULTI_PRI_CFG_0_CFGSHAREDL2_FIELD_MASK,
    0,
    EPN_MULTI_PRI_CFG_0_CFGSHAREDL2_FIELD_WIDTH,
    EPN_MULTI_PRI_CFG_0_CFGSHAREDL2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MULTI_PRI_CFG_0_CFGSHAREDBURSTCAP
 ******************************************************************************/
const ru_field_rec EPN_MULTI_PRI_CFG_0_CFGSHAREDBURSTCAP_FIELD =
{
    "CFGSHAREDBURSTCAP",
#if RU_INCLUDE_DESC
    "",
    "For the burst-cap limited queue set, configures whether each queue"
    "report/priority is individually limited to the Burst Cap, or whether"
    "the sum of all queue report values is limited to the Burst Cap."
    "0: Limit each queue report value to a Burst Cap. Each queue"
    "report/priority uses the Burst Cap corresponding to its L2 FIFO."
    "This bit has effect only when cfgRptMultiPri is 1."
    "Note: Setting this bit, along with cfgSharedL2, configures EPN into"
    "a TK3715-compatible reporting mode."
    "1: Limit the sum of the queue reports/priorities to the Burst Cap"
    "value. (Use the Burst Cap value corresponding to the lowest L2 FIFO"
    "assigned to the LLID index)."
    "Note: The following offsets must be included in the burst cap value"
    "calculation when \"shared burst cap mode\" is enabled.  Decrease the"
    "desired burst cap value by 21 bytes in 1G non-FEC mode, 193 bytes in"
    "1G FEC mode.",
#endif
    EPN_MULTI_PRI_CFG_0_CFGSHAREDBURSTCAP_FIELD_MASK,
    0,
    EPN_MULTI_PRI_CFG_0_CFGSHAREDBURSTCAP_FIELD_WIDTH,
    EPN_MULTI_PRI_CFG_0_CFGSHAREDBURSTCAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MULTI_PRI_CFG_0_CFGRPTGNTSOUTST0
 ******************************************************************************/
const ru_field_rec EPN_MULTI_PRI_CFG_0_CFGRPTGNTSOUTST0_FIELD =
{
    "CFGRPTGNTSOUTST0",
#if RU_INCLUDE_DESC
    "",
    "Configures whether Multi-Priority REPORT frames include"
    "already-granted frames."
    "0: Report frames do NOT include frames that have already been"
    "granted. This is the setting for NTT operation."
    "1: Report frames DO include frames that have already been granted"
    "(but not yet sent upstream)."
    "This bit has effect only when cfgRptMultiPri is 1.",
#endif
    EPN_MULTI_PRI_CFG_0_CFGRPTGNTSOUTST0_FIELD_MASK,
    0,
    EPN_MULTI_PRI_CFG_0_CFGRPTGNTSOUTST0_FIELD_WIDTH,
    EPN_MULTI_PRI_CFG_0_CFGRPTGNTSOUTST0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MULTI_PRI_CFG_0_CFGRPTHIPRIFIRST0
 ******************************************************************************/
const ru_field_rec EPN_MULTI_PRI_CFG_0_CFGRPTHIPRIFIRST0_FIELD =
{
    "CFGRPTHIPRIFIRST0",
#if RU_INCLUDE_DESC
    "",
    "Determines the order within a queue set in which priorities are"
    "reported on."
    "0: Priorities are reported low to high."
    "1: Priorities are reported high to low."
    "This bit has effect only when cfgRptMultiPri0 is 1.",
#endif
    EPN_MULTI_PRI_CFG_0_CFGRPTHIPRIFIRST0_FIELD_MASK,
    0,
    EPN_MULTI_PRI_CFG_0_CFGRPTHIPRIFIRST0_FIELD_WIDTH,
    EPN_MULTI_PRI_CFG_0_CFGRPTHIPRIFIRST0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MULTI_PRI_CFG_0_CFGRPTSWAPQS0
 ******************************************************************************/
const ru_field_rec EPN_MULTI_PRI_CFG_0_CFGRPTSWAPQS0_FIELD =
{
    "CFGRPTSWAPQS0",
#if RU_INCLUDE_DESC
    "",
    "Configure order of queue sets."
    "0: The first queue set reports the full packet buffer queue depths;"
    "the second queue set reports up to the T/Burst Cap 0 threshold."
    "1: Swap queue sets in Multi-Priority REPORT frames. The first Queue"
    "set reports up to the T/Burst Cap 0 threshold; the second queue set"
    "reports the the full packet buffer queue depths."
    "This bit has effect only when cfgRptMultiPri0 is 1.",
#endif
    EPN_MULTI_PRI_CFG_0_CFGRPTSWAPQS0_FIELD_MASK,
    0,
    EPN_MULTI_PRI_CFG_0_CFGRPTSWAPQS0_FIELD_WIDTH,
    EPN_MULTI_PRI_CFG_0_CFGRPTSWAPQS0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MULTI_PRI_CFG_0_RESERVED3
 ******************************************************************************/
const ru_field_rec EPN_MULTI_PRI_CFG_0_RESERVED3_FIELD =
{
    "RESERVED3",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MULTI_PRI_CFG_0_RESERVED3_FIELD_MASK,
    0,
    EPN_MULTI_PRI_CFG_0_RESERVED3_FIELD_WIDTH,
    EPN_MULTI_PRI_CFG_0_RESERVED3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MULTI_PRI_CFG_0_CFGRPTMULTIPRI0
 ******************************************************************************/
const ru_field_rec EPN_MULTI_PRI_CFG_0_CFGRPTMULTIPRI0_FIELD =
{
    "CFGRPTMULTIPRI0",
#if RU_INCLUDE_DESC
    "",
    "Configure Multi-Priority mode."
    "0: Disable Multi-Priority mode. The reporting style is selected by"
    "Report Select bit (bit 8 in the EPON Control 0 register)."
    "1: Configured for Multi-Priority reporting mode.",
#endif
    EPN_MULTI_PRI_CFG_0_CFGRPTMULTIPRI0_FIELD_MASK,
    0,
    EPN_MULTI_PRI_CFG_0_CFGRPTMULTIPRI0_FIELD_WIDTH,
    EPN_MULTI_PRI_CFG_0_CFGRPTMULTIPRI0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_SHARED_BCAP_OVRFLOW_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_SHARED_BCAP_OVRFLOW_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_SHARED_BCAP_OVRFLOW_RESERVED0_FIELD_MASK,
    0,
    EPN_SHARED_BCAP_OVRFLOW_RESERVED0_FIELD_WIDTH,
    EPN_SHARED_BCAP_OVRFLOW_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_SHARED_BCAP_OVRFLOW_SHAREDBURSTCAPOVERFLOW
 ******************************************************************************/
const ru_field_rec EPN_SHARED_BCAP_OVRFLOW_SHAREDBURSTCAPOVERFLOW_FIELD =
{
    "SHAREDBURSTCAPOVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_SHARED_BCAP_OVRFLOW_SHAREDBURSTCAPOVERFLOW_FIELD_MASK,
    0,
    EPN_SHARED_BCAP_OVRFLOW_SHAREDBURSTCAPOVERFLOW_FIELD_WIDTH,
    EPN_SHARED_BCAP_OVRFLOW_SHAREDBURSTCAPOVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_FORCED_REPORT_EN_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_FORCED_REPORT_EN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_FORCED_REPORT_EN_RESERVED0_FIELD_MASK,
    0,
    EPN_FORCED_REPORT_EN_RESERVED0_FIELD_WIDTH,
    EPN_FORCED_REPORT_EN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_FORCED_REPORT_EN_CFGFORCEREPORTEN
 ******************************************************************************/
const ru_field_rec EPN_FORCED_REPORT_EN_CFGFORCEREPORTEN_FIELD =
{
    "CFGFORCEREPORTEN",
#if RU_INCLUDE_DESC
    "",
    "Per-LLID Index bits for enabling forced reporting. Setting a bit"
    "will cause that Index to send a report as part of the first upstream"
    "burst that occurs more than the number of mS provisioned in"
    "cfgMaxReportInterval\" after the last report was sent.",
#endif
    EPN_FORCED_REPORT_EN_CFGFORCEREPORTEN_FIELD_MASK,
    0,
    EPN_FORCED_REPORT_EN_CFGFORCEREPORTEN_FIELD_WIDTH,
    EPN_FORCED_REPORT_EN_CFGFORCEREPORTEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_FORCED_REPORT_MAX_INTERVAL_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_FORCED_REPORT_MAX_INTERVAL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_FORCED_REPORT_MAX_INTERVAL_RESERVED0_FIELD_MASK,
    0,
    EPN_FORCED_REPORT_MAX_INTERVAL_RESERVED0_FIELD_WIDTH,
    EPN_FORCED_REPORT_MAX_INTERVAL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_FORCED_REPORT_MAX_INTERVAL_CFGMAXREPORTINTERVAL
 ******************************************************************************/
const ru_field_rec EPN_FORCED_REPORT_MAX_INTERVAL_CFGMAXREPORTINTERVAL_FIELD =
{
    "CFGMAXREPORTINTERVAL",
#if RU_INCLUDE_DESC
    "",
    "The number of mS after the last report was sent to force the report.",
#endif
    EPN_FORCED_REPORT_MAX_INTERVAL_CFGMAXREPORTINTERVAL_FIELD_MASK,
    0,
    EPN_FORCED_REPORT_MAX_INTERVAL_CFGMAXREPORTINTERVAL_FIELD_WIDTH,
    EPN_FORCED_REPORT_MAX_INTERVAL_CFGMAXREPORTINTERVAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_L2S_FLUSH_CONFIG_CFGFLUSHL2SEN
 ******************************************************************************/
const ru_field_rec EPN_L2S_FLUSH_CONFIG_CFGFLUSHL2SEN_FIELD =
{
    "CFGFLUSHL2SEN",
#if RU_INCLUDE_DESC
    "",
    "Enables the selected L2 queue to be flushed.  This configuration bit"
    "must be forced low and \"flushL2sDone\" must be read back as low;"
    "before, \"cfgFlushL2sSel\" is written."
    "0: L2 queue flush is disabled"
    "1: L2 queue flush is enabled",
#endif
    EPN_L2S_FLUSH_CONFIG_CFGFLUSHL2SEN_FIELD_MASK,
    0,
    EPN_L2S_FLUSH_CONFIG_CFGFLUSHL2SEN_FIELD_WIDTH,
    EPN_L2S_FLUSH_CONFIG_CFGFLUSHL2SEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_L2S_FLUSH_CONFIG_FLUSHL2SDONE
 ******************************************************************************/
const ru_field_rec EPN_L2S_FLUSH_CONFIG_FLUSHL2SDONE_FIELD =
{
    "FLUSHL2SDONE",
#if RU_INCLUDE_DESC
    "",
    "This bit is set when the selected L2 queue has no more packets to"
    "flush. This bit will always be zero when the enable bit is zero.",
#endif
    EPN_L2S_FLUSH_CONFIG_FLUSHL2SDONE_FIELD_MASK,
    0,
    EPN_L2S_FLUSH_CONFIG_FLUSHL2SDONE_FIELD_WIDTH,
    EPN_L2S_FLUSH_CONFIG_FLUSHL2SDONE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_L2S_FLUSH_CONFIG_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_L2S_FLUSH_CONFIG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_L2S_FLUSH_CONFIG_RESERVED0_FIELD_MASK,
    0,
    EPN_L2S_FLUSH_CONFIG_RESERVED0_FIELD_WIDTH,
    EPN_L2S_FLUSH_CONFIG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_L2S_FLUSH_CONFIG_CFGFLUSHL2SSEL
 ******************************************************************************/
const ru_field_rec EPN_L2S_FLUSH_CONFIG_CFGFLUSHL2SSEL_FIELD =
{
    "CFGFLUSHL2SSEL",
#if RU_INCLUDE_DESC
    "",
    "Selects the L2 queue that will be flushed when the enable is set."
    "Do not write this register unless both \"flushL2sDone\" and"
    "cfgFlushL2sEn\" are low.",
#endif
    EPN_L2S_FLUSH_CONFIG_CFGFLUSHL2SSEL_FIELD_MASK,
    0,
    EPN_L2S_FLUSH_CONFIG_CFGFLUSHL2SSEL_FIELD_WIDTH,
    EPN_L2S_FLUSH_CONFIG_CFGFLUSHL2SSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DATA_PORT_COMMAND_DPORTBUSY
 ******************************************************************************/
const ru_field_rec EPN_DATA_PORT_COMMAND_DPORTBUSY_FIELD =
{
    "DPORTBUSY",
#if RU_INCLUDE_DESC
    "",
    "Indicates access to RAM is in progress."
    "0: Data port is ready to accept a command"
    "1: Data port is busy",
#endif
    EPN_DATA_PORT_COMMAND_DPORTBUSY_FIELD_MASK,
    0,
    EPN_DATA_PORT_COMMAND_DPORTBUSY_FIELD_WIDTH,
    EPN_DATA_PORT_COMMAND_DPORTBUSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DATA_PORT_COMMAND_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_DATA_PORT_COMMAND_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DATA_PORT_COMMAND_RESERVED0_FIELD_MASK,
    0,
    EPN_DATA_PORT_COMMAND_RESERVED0_FIELD_WIDTH,
    EPN_DATA_PORT_COMMAND_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DATA_PORT_COMMAND_DPORTSELECT
 ******************************************************************************/
const ru_field_rec EPN_DATA_PORT_COMMAND_DPORTSELECT_FIELD =
{
    "DPORTSELECT",
#if RU_INCLUDE_DESC
    "",
    "Selects RAM to be access"
    "0:   Downstream statistics RAM"
    "1:   Upstream statistics RAM"
    "2:   Reserved"
    "3:   L2 Queue RAM"
    "4-31: Reserved",
#endif
    EPN_DATA_PORT_COMMAND_DPORTSELECT_FIELD_MASK,
    0,
    EPN_DATA_PORT_COMMAND_DPORTSELECT_FIELD_WIDTH,
    EPN_DATA_PORT_COMMAND_DPORTSELECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DATA_PORT_COMMAND_RESERVED1
 ******************************************************************************/
const ru_field_rec EPN_DATA_PORT_COMMAND_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DATA_PORT_COMMAND_RESERVED1_FIELD_MASK,
    0,
    EPN_DATA_PORT_COMMAND_RESERVED1_FIELD_WIDTH,
    EPN_DATA_PORT_COMMAND_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DATA_PORT_COMMAND_DPORTCONTROL
 ******************************************************************************/
const ru_field_rec EPN_DATA_PORT_COMMAND_DPORTCONTROL_FIELD =
{
    "DPORTCONTROL",
#if RU_INCLUDE_DESC
    "",
    "Indicates data port operation"
    "0: Do a RAM read operation"
    "1: Do a RAM write operation",
#endif
    EPN_DATA_PORT_COMMAND_DPORTCONTROL_FIELD_MASK,
    0,
    EPN_DATA_PORT_COMMAND_DPORTCONTROL_FIELD_WIDTH,
    EPN_DATA_PORT_COMMAND_DPORTCONTROL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DATA_PORT_ADDRESS_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_DATA_PORT_ADDRESS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DATA_PORT_ADDRESS_RESERVED0_FIELD_MASK,
    0,
    EPN_DATA_PORT_ADDRESS_RESERVED0_FIELD_WIDTH,
    EPN_DATA_PORT_ADDRESS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DATA_PORT_ADDRESS_DPORTADDR
 ******************************************************************************/
const ru_field_rec EPN_DATA_PORT_ADDRESS_DPORTADDR_FIELD =
{
    "DPORTADDR",
#if RU_INCLUDE_DESC
    "",
    "Address for RAM accesses.",
#endif
    EPN_DATA_PORT_ADDRESS_DPORTADDR_FIELD_MASK,
    0,
    EPN_DATA_PORT_ADDRESS_DPORTADDR_FIELD_WIDTH,
    EPN_DATA_PORT_ADDRESS_DPORTADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DATA_PORT_DATA_0_DPORTDATA0
 ******************************************************************************/
const ru_field_rec EPN_DATA_PORT_DATA_0_DPORTDATA0_FIELD =
{
    "DPORTDATA0",
#if RU_INCLUDE_DESC
    "",
    "Low-order data dword for data port accesses",
#endif
    EPN_DATA_PORT_DATA_0_DPORTDATA0_FIELD_MASK,
    0,
    EPN_DATA_PORT_DATA_0_DPORTDATA0_FIELD_WIDTH,
    EPN_DATA_PORT_DATA_0_DPORTDATA0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_UNMAP_BIG_CNT_UNMAPBIGERRCNT
 ******************************************************************************/
const ru_field_rec EPN_UNMAP_BIG_CNT_UNMAPBIGERRCNT_FIELD =
{
    "UNMAPBIGERRCNT",
#if RU_INCLUDE_DESC
    "",
    "Counts illegally large frames.",
#endif
    EPN_UNMAP_BIG_CNT_UNMAPBIGERRCNT_FIELD_MASK,
    0,
    EPN_UNMAP_BIG_CNT_UNMAPBIGERRCNT_FIELD_WIDTH,
    EPN_UNMAP_BIG_CNT_UNMAPBIGERRCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_UNMAP_FRAME_CNT_UNMAPFRCNT
 ******************************************************************************/
const ru_field_rec EPN_UNMAP_FRAME_CNT_UNMAPFRCNT_FIELD =
{
    "UNMAPFRCNT",
#if RU_INCLUDE_DESC
    "",
    "Counts valid frames that are not Gates or OAM frames.",
#endif
    EPN_UNMAP_FRAME_CNT_UNMAPFRCNT_FIELD_MASK,
    0,
    EPN_UNMAP_FRAME_CNT_UNMAPFRCNT_FIELD_WIDTH,
    EPN_UNMAP_FRAME_CNT_UNMAPFRCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_UNMAP_FCS_CNT_UNMAPFCSERRCNT
 ******************************************************************************/
const ru_field_rec EPN_UNMAP_FCS_CNT_UNMAPFCSERRCNT_FIELD =
{
    "UNMAPFCSERRCNT",
#if RU_INCLUDE_DESC
    "",
    "Counts frame with FCS errors.",
#endif
    EPN_UNMAP_FCS_CNT_UNMAPFCSERRCNT_FIELD_MASK,
    0,
    EPN_UNMAP_FCS_CNT_UNMAPFCSERRCNT_FIELD_WIDTH,
    EPN_UNMAP_FCS_CNT_UNMAPFCSERRCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_UNMAP_GATE_CNT_UNMAPGATECNT
 ******************************************************************************/
const ru_field_rec EPN_UNMAP_GATE_CNT_UNMAPGATECNT_FIELD =
{
    "UNMAPGATECNT",
#if RU_INCLUDE_DESC
    "",
    "Counts un-mapped gate frames.",
#endif
    EPN_UNMAP_GATE_CNT_UNMAPGATECNT_FIELD_MASK,
    0,
    EPN_UNMAP_GATE_CNT_UNMAPGATECNT_FIELD_WIDTH,
    EPN_UNMAP_GATE_CNT_UNMAPGATECNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_UNMAP_OAM_CNT_UNMAPOAMCNT
 ******************************************************************************/
const ru_field_rec EPN_UNMAP_OAM_CNT_UNMAPOAMCNT_FIELD =
{
    "UNMAPOAMCNT",
#if RU_INCLUDE_DESC
    "",
    "Counts un-mapped OAM frames.",
#endif
    EPN_UNMAP_OAM_CNT_UNMAPOAMCNT_FIELD_MASK,
    0,
    EPN_UNMAP_OAM_CNT_UNMAPOAMCNT_FIELD_WIDTH,
    EPN_UNMAP_OAM_CNT_UNMAPOAMCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_UNMAP_SMALL_CNT_UNMAPSMALLERRCNT
 ******************************************************************************/
const ru_field_rec EPN_UNMAP_SMALL_CNT_UNMAPSMALLERRCNT_FIELD =
{
    "UNMAPSMALLERRCNT",
#if RU_INCLUDE_DESC
    "",
    "Counts illegally small frames.",
#endif
    EPN_UNMAP_SMALL_CNT_UNMAPSMALLERRCNT_FIELD_MASK,
    0,
    EPN_UNMAP_SMALL_CNT_UNMAPSMALLERRCNT_FIELD_WIDTH,
    EPN_UNMAP_SMALL_CNT_UNMAPSMALLERRCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_FIF_DEQUEUE_EVENT_CNT_FIFDEQUEUEEVENTCNT
 ******************************************************************************/
const ru_field_rec EPN_FIF_DEQUEUE_EVENT_CNT_FIFDEQUEUEEVENTCNT_FIELD =
{
    "FIFDEQUEUEEVENTCNT",
#if RU_INCLUDE_DESC
    "",
    "Debug only!",
#endif
    EPN_FIF_DEQUEUE_EVENT_CNT_FIFDEQUEUEEVENTCNT_FIELD_MASK,
    0,
    EPN_FIF_DEQUEUE_EVENT_CNT_FIFDEQUEUEEVENTCNT_FIELD_WIDTH,
    EPN_FIF_DEQUEUE_EVENT_CNT_FIFDEQUEUEEVENTCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_UNUSED_TQ_CNT_UNUSEDTQCNT
 ******************************************************************************/
const ru_field_rec EPN_UNUSED_TQ_CNT_UNUSEDTQCNT_FIELD =
{
    "UNUSEDTQCNT",
#if RU_INCLUDE_DESC
    "",
    "The number of unused time quanta for LLID i.",
#endif
    EPN_UNUSED_TQ_CNT_UNUSEDTQCNT_FIELD_MASK,
    0,
    EPN_UNUSED_TQ_CNT_UNUSEDTQCNT_FIELD_WIDTH,
    EPN_UNUSED_TQ_CNT_UNUSEDTQCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UP_FAULT_HALT_EN_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_BBH_UP_FAULT_HALT_EN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_BBH_UP_FAULT_HALT_EN_RESERVED0_FIELD_MASK,
    0,
    EPN_BBH_UP_FAULT_HALT_EN_RESERVED0_FIELD_WIDTH,
    EPN_BBH_UP_FAULT_HALT_EN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UP_FAULT_HALT_EN_BBHUPSFAULTHALTEN
 ******************************************************************************/
const ru_field_rec EPN_BBH_UP_FAULT_HALT_EN_BBHUPSFAULTHALTEN_FIELD =
{
    "BBHUPSFAULTHALTEN",
#if RU_INCLUDE_DESC
    "",
    "Per-LLID Index Runner/BBH upstream fault halt enable."
    "0: Do not disable upstream data traffic."
    "1: Disable upstream data traffic for LLID indexes the Runner/BBH"
    "faulted on.",
#endif
    EPN_BBH_UP_FAULT_HALT_EN_BBHUPSFAULTHALTEN_FIELD_MASK,
    0,
    EPN_BBH_UP_FAULT_HALT_EN_BBHUPSFAULTHALTEN_FIELD_WIDTH,
    EPN_BBH_UP_FAULT_HALT_EN_BBHUPSFAULTHALTEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UP_TARDY_HALT_EN_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_BBH_UP_TARDY_HALT_EN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_BBH_UP_TARDY_HALT_EN_RESERVED0_FIELD_MASK,
    0,
    EPN_BBH_UP_TARDY_HALT_EN_RESERVED0_FIELD_WIDTH,
    EPN_BBH_UP_TARDY_HALT_EN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_UP_TARDY_HALT_EN_FATALTARDYBBHABORTEN
 ******************************************************************************/
const ru_field_rec EPN_BBH_UP_TARDY_HALT_EN_FATALTARDYBBHABORTEN_FIELD =
{
    "FATALTARDYBBHABORTEN",
#if RU_INCLUDE_DESC
    "",
    "Disable all upstream data traffic when the BHH aborts a tardy"
    "packet."
    "0: Do not disable upstream data traffic on tardy/abort"
    "1: Disable all upstream data traffic when the BBH aborts a tardy"
    "packet.",
#endif
    EPN_BBH_UP_TARDY_HALT_EN_FATALTARDYBBHABORTEN_FIELD_MASK,
    0,
    EPN_BBH_UP_TARDY_HALT_EN_FATALTARDYBBHABORTEN_FIELD_WIDTH,
    EPN_BBH_UP_TARDY_HALT_EN_FATALTARDYBBHABORTEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DEBUG_STATUS_0_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_DEBUG_STATUS_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DEBUG_STATUS_0_RESERVED0_FIELD_MASK,
    0,
    EPN_DEBUG_STATUS_0_RESERVED0_FIELD_WIDTH,
    EPN_DEBUG_STATUS_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_DEBUG_STATUS_0_L2SQUEFULLDEBUG
 ******************************************************************************/
const ru_field_rec EPN_DEBUG_STATUS_0_L2SQUEFULLDEBUG_FIELD =
{
    "L2SQUEFULLDEBUG",
#if RU_INCLUDE_DESC
    "",
    "Indicates which LLID's report FIFO is full",
#endif
    EPN_DEBUG_STATUS_0_L2SQUEFULLDEBUG_FIELD_MASK,
    0,
    EPN_DEBUG_STATUS_0_L2SQUEFULLDEBUG_FIELD_WIDTH,
    EPN_DEBUG_STATUS_0_L2SQUEFULLDEBUG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_DEBUG_STATUS_0_RESERVED1
 ******************************************************************************/
const ru_field_rec EPN_DEBUG_STATUS_0_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DEBUG_STATUS_0_RESERVED1_FIELD_MASK,
    0,
    EPN_DEBUG_STATUS_0_RESERVED1_FIELD_WIDTH,
    EPN_DEBUG_STATUS_0_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_DEBUG_STATUS_0_DNDLUFULL
 ******************************************************************************/
const ru_field_rec EPN_DEBUG_STATUS_0_DNDLUFULL_FIELD =
{
    "DNDLUFULL",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DEBUG_STATUS_0_DNDLUFULL_FIELD_MASK,
    0,
    EPN_DEBUG_STATUS_0_DNDLUFULL_FIELD_WIDTH,
    EPN_DEBUG_STATUS_0_DNDLUFULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_DEBUG_STATUS_0_DNSECFULL
 ******************************************************************************/
const ru_field_rec EPN_DEBUG_STATUS_0_DNSECFULL_FIELD =
{
    "DNSECFULL",
#if RU_INCLUDE_DESC
    "",
    "SEC and EPN downstream",
#endif
    EPN_DEBUG_STATUS_0_DNSECFULL_FIELD_MASK,
    0,
    EPN_DEBUG_STATUS_0_DNSECFULL_FIELD_WIDTH,
    EPN_DEBUG_STATUS_0_DNSECFULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_DEBUG_STATUS_0_EPNLIFFIFOFULL
 ******************************************************************************/
const ru_field_rec EPN_DEBUG_STATUS_0_EPNLIFFIFOFULL_FIELD =
{
    "EPNLIFFIFOFULL",
#if RU_INCLUDE_DESC
    "",
    "SEC and EPN upstream  interface FIFO is full",
#endif
    EPN_DEBUG_STATUS_0_EPNLIFFIFOFULL_FIELD_MASK,
    0,
    EPN_DEBUG_STATUS_0_EPNLIFFIFOFULL_FIELD_WIDTH,
    EPN_DEBUG_STATUS_0_EPNLIFFIFOFULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_DEBUG_STATUS_1_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_DEBUG_STATUS_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DEBUG_STATUS_1_RESERVED0_FIELD_MASK,
    0,
    EPN_DEBUG_STATUS_1_RESERVED0_FIELD_WIDTH,
    EPN_DEBUG_STATUS_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_DEBUG_STATUS_1_GNTRDY7
 ******************************************************************************/
const ru_field_rec EPN_DEBUG_STATUS_1_GNTRDY7_FIELD =
{
    "GNTRDY7",
#if RU_INCLUDE_DESC
    "",
    "LLID Index 7 has grant(s) pending",
#endif
    EPN_DEBUG_STATUS_1_GNTRDY7_FIELD_MASK,
    0,
    EPN_DEBUG_STATUS_1_GNTRDY7_FIELD_WIDTH,
    EPN_DEBUG_STATUS_1_GNTRDY7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_DEBUG_STATUS_1_GNTRDY6
 ******************************************************************************/
const ru_field_rec EPN_DEBUG_STATUS_1_GNTRDY6_FIELD =
{
    "GNTRDY6",
#if RU_INCLUDE_DESC
    "",
    "LLID Index 6 has grant(s) pending",
#endif
    EPN_DEBUG_STATUS_1_GNTRDY6_FIELD_MASK,
    0,
    EPN_DEBUG_STATUS_1_GNTRDY6_FIELD_WIDTH,
    EPN_DEBUG_STATUS_1_GNTRDY6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_DEBUG_STATUS_1_GNTRDY5
 ******************************************************************************/
const ru_field_rec EPN_DEBUG_STATUS_1_GNTRDY5_FIELD =
{
    "GNTRDY5",
#if RU_INCLUDE_DESC
    "",
    "LLID Index 5 has grant(s) pending",
#endif
    EPN_DEBUG_STATUS_1_GNTRDY5_FIELD_MASK,
    0,
    EPN_DEBUG_STATUS_1_GNTRDY5_FIELD_WIDTH,
    EPN_DEBUG_STATUS_1_GNTRDY5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_DEBUG_STATUS_1_GNTRDY4
 ******************************************************************************/
const ru_field_rec EPN_DEBUG_STATUS_1_GNTRDY4_FIELD =
{
    "GNTRDY4",
#if RU_INCLUDE_DESC
    "",
    "LLID Index 4 has grant(s) pending",
#endif
    EPN_DEBUG_STATUS_1_GNTRDY4_FIELD_MASK,
    0,
    EPN_DEBUG_STATUS_1_GNTRDY4_FIELD_WIDTH,
    EPN_DEBUG_STATUS_1_GNTRDY4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_DEBUG_STATUS_1_GNTRDY3
 ******************************************************************************/
const ru_field_rec EPN_DEBUG_STATUS_1_GNTRDY3_FIELD =
{
    "GNTRDY3",
#if RU_INCLUDE_DESC
    "",
    "LLID Index 3 has grant(s) pending",
#endif
    EPN_DEBUG_STATUS_1_GNTRDY3_FIELD_MASK,
    0,
    EPN_DEBUG_STATUS_1_GNTRDY3_FIELD_WIDTH,
    EPN_DEBUG_STATUS_1_GNTRDY3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_DEBUG_STATUS_1_GNTRDY2
 ******************************************************************************/
const ru_field_rec EPN_DEBUG_STATUS_1_GNTRDY2_FIELD =
{
    "GNTRDY2",
#if RU_INCLUDE_DESC
    "",
    "LLID Index 2 has grant(s) pending",
#endif
    EPN_DEBUG_STATUS_1_GNTRDY2_FIELD_MASK,
    0,
    EPN_DEBUG_STATUS_1_GNTRDY2_FIELD_WIDTH,
    EPN_DEBUG_STATUS_1_GNTRDY2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_DEBUG_STATUS_1_GNTRDY1
 ******************************************************************************/
const ru_field_rec EPN_DEBUG_STATUS_1_GNTRDY1_FIELD =
{
    "GNTRDY1",
#if RU_INCLUDE_DESC
    "",
    "LLID Index 1 has grant(s) pending",
#endif
    EPN_DEBUG_STATUS_1_GNTRDY1_FIELD_MASK,
    0,
    EPN_DEBUG_STATUS_1_GNTRDY1_FIELD_WIDTH,
    EPN_DEBUG_STATUS_1_GNTRDY1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_DEBUG_STATUS_1_GNTRDY0
 ******************************************************************************/
const ru_field_rec EPN_DEBUG_STATUS_1_GNTRDY0_FIELD =
{
    "GNTRDY0",
#if RU_INCLUDE_DESC
    "",
    "LLID Index 0 has grant(s) pending",
#endif
    EPN_DEBUG_STATUS_1_GNTRDY0_FIELD_MASK,
    0,
    EPN_DEBUG_STATUS_1_GNTRDY0_FIELD_WIDTH,
    EPN_DEBUG_STATUS_1_GNTRDY0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_DEBUG_L2S_PTR_SEL_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_DEBUG_L2S_PTR_SEL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DEBUG_L2S_PTR_SEL_RESERVED0_FIELD_MASK,
    0,
    EPN_DEBUG_L2S_PTR_SEL_RESERVED0_FIELD_WIDTH,
    EPN_DEBUG_L2S_PTR_SEL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DEBUG_L2S_PTR_SEL_CFGL2SDEBUGPTRSEL
 ******************************************************************************/
const ru_field_rec EPN_DEBUG_L2S_PTR_SEL_CFGL2SDEBUGPTRSEL_FIELD =
{
    "CFGL2SDEBUGPTRSEL",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DEBUG_L2S_PTR_SEL_CFGL2SDEBUGPTRSEL_FIELD_MASK,
    0,
    EPN_DEBUG_L2S_PTR_SEL_CFGL2SDEBUGPTRSEL_FIELD_WIDTH,
    EPN_DEBUG_L2S_PTR_SEL_CFGL2SDEBUGPTRSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DEBUG_L2S_PTR_SEL_RESERVED1
 ******************************************************************************/
const ru_field_rec EPN_DEBUG_L2S_PTR_SEL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DEBUG_L2S_PTR_SEL_RESERVED1_FIELD_MASK,
    0,
    EPN_DEBUG_L2S_PTR_SEL_RESERVED1_FIELD_WIDTH,
    EPN_DEBUG_L2S_PTR_SEL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DEBUG_L2S_PTR_SEL_L2SDEBUGPTRSTATE
 ******************************************************************************/
const ru_field_rec EPN_DEBUG_L2S_PTR_SEL_L2SDEBUGPTRSTATE_FIELD =
{
    "L2SDEBUGPTRSTATE",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_DEBUG_L2S_PTR_SEL_L2SDEBUGPTRSTATE_FIELD_MASK,
    0,
    EPN_DEBUG_L2S_PTR_SEL_L2SDEBUGPTRSTATE_FIELD_WIDTH,
    EPN_DEBUG_L2S_PTR_SEL_L2SDEBUGPTRSTATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_OLT_MAC_ADDR_LO_OLTADDRLO
 ******************************************************************************/
const ru_field_rec EPN_OLT_MAC_ADDR_LO_OLTADDRLO_FIELD =
{
    "OLTADDRLO",
#if RU_INCLUDE_DESC
    "",
    "OLT MAC Address",
#endif
    EPN_OLT_MAC_ADDR_LO_OLTADDRLO_FIELD_MASK,
    0,
    EPN_OLT_MAC_ADDR_LO_OLTADDRLO_FIELD_WIDTH,
    EPN_OLT_MAC_ADDR_LO_OLTADDRLO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_OLT_MAC_ADDR_HI_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_OLT_MAC_ADDR_HI_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_OLT_MAC_ADDR_HI_RESERVED0_FIELD_MASK,
    0,
    EPN_OLT_MAC_ADDR_HI_RESERVED0_FIELD_WIDTH,
    EPN_OLT_MAC_ADDR_HI_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_OLT_MAC_ADDR_HI_OLTADDRHI
 ******************************************************************************/
const ru_field_rec EPN_OLT_MAC_ADDR_HI_OLTADDRHI_FIELD =
{
    "OLTADDRHI",
#if RU_INCLUDE_DESC
    "",
    "OLT MAC Address",
#endif
    EPN_OLT_MAC_ADDR_HI_OLTADDRHI_FIELD_MASK,
    0,
    EPN_OLT_MAC_ADDR_HI_OLTADDRHI_FIELD_WIDTH,
    EPN_OLT_MAC_ADDR_HI_OLTADDRHI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_TX_L1S_SHP_DQU_EMPTY_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_TX_L1S_SHP_DQU_EMPTY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_TX_L1S_SHP_DQU_EMPTY_RESERVED0_FIELD_MASK,
    0,
    EPN_TX_L1S_SHP_DQU_EMPTY_RESERVED0_FIELD_WIDTH,
    EPN_TX_L1S_SHP_DQU_EMPTY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_TX_L1S_SHP_DQU_EMPTY_L1SDQUQUEEMPTY
 ******************************************************************************/
const ru_field_rec EPN_TX_L1S_SHP_DQU_EMPTY_L1SDQUQUEEMPTY_FIELD =
{
    "L1SDQUQUEEMPTY",
#if RU_INCLUDE_DESC
    "",
    "Each bit in this register contains the status of the respective"
    "queue"
    "0: L1 accumulator is not empty"
    "1: L1 accumulator is empty",
#endif
    EPN_TX_L1S_SHP_DQU_EMPTY_L1SDQUQUEEMPTY_FIELD_MASK,
    0,
    EPN_TX_L1S_SHP_DQU_EMPTY_L1SDQUQUEEMPTY_FIELD_WIDTH,
    EPN_TX_L1S_SHP_DQU_EMPTY_L1SDQUQUEEMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_TX_L1S_UNSHAPED_EMPTY_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_TX_L1S_UNSHAPED_EMPTY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_TX_L1S_UNSHAPED_EMPTY_RESERVED0_FIELD_MASK,
    0,
    EPN_TX_L1S_UNSHAPED_EMPTY_RESERVED0_FIELD_WIDTH,
    EPN_TX_L1S_UNSHAPED_EMPTY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_TX_L1S_UNSHAPED_EMPTY_L1SUNSHAPEDQUEEMPTY
 ******************************************************************************/
const ru_field_rec EPN_TX_L1S_UNSHAPED_EMPTY_L1SUNSHAPEDQUEEMPTY_FIELD =
{
    "L1SUNSHAPEDQUEEMPTY",
#if RU_INCLUDE_DESC
    "",
    "Each bit in this register contains the status of the respective"
    "accumulators"
    "0: L1 unshaped accumulator is not empty"
    "1: L1 unshaped accumulator is empty",
#endif
    EPN_TX_L1S_UNSHAPED_EMPTY_L1SUNSHAPEDQUEEMPTY_FIELD_MASK,
    0,
    EPN_TX_L1S_UNSHAPED_EMPTY_L1SUNSHAPEDQUEEMPTY_FIELD_WIDTH,
    EPN_TX_L1S_UNSHAPED_EMPTY_L1SUNSHAPEDQUEEMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_TX_L1S_SHP_QUE_MASK__RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_TX_L1S_SHP_QUE_MASK__RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_TX_L1S_SHP_QUE_MASK__RESERVED0_FIELD_MASK,
    0,
    EPN_TX_L1S_SHP_QUE_MASK__RESERVED0_FIELD_WIDTH,
    EPN_TX_L1S_SHP_QUE_MASK__RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_TX_L1S_SHP_QUE_MASK__CFGSHPMASK
 ******************************************************************************/
const ru_field_rec EPN_TX_L1S_SHP_QUE_MASK__CFGSHPMASK_FIELD =
{
    "CFGSHPMASK",
#if RU_INCLUDE_DESC
    "",
    "Each mask bit in this register controls the effect of shaper i on"
    "the respective queue"
    "0: Shaper i can affect this queue"
    "1: Shaper i does not affect this queue",
#endif
    EPN_TX_L1S_SHP_QUE_MASK__CFGSHPMASK_FIELD_MASK,
    0,
    EPN_TX_L1S_SHP_QUE_MASK__CFGSHPMASK_FIELD_WIDTH,
    EPN_TX_L1S_SHP_QUE_MASK__CFGSHPMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_TX_L2S_QUE_CONFIG__RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_TX_L2S_QUE_CONFIG__RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_TX_L2S_QUE_CONFIG__RESERVED0_FIELD_MASK,
    0,
    EPN_TX_L2S_QUE_CONFIG__RESERVED0_FIELD_WIDTH,
    EPN_TX_L2S_QUE_CONFIG__RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_TX_L2S_QUE_CONFIG__CFGL2SQUEEND
 ******************************************************************************/
const ru_field_rec EPN_TX_L2S_QUE_CONFIG__CFGL2SQUEEND_FIELD =
{
    "CFGL2SQUEEND",
#if RU_INCLUDE_DESC
    "",
    "Queue i End address",
#endif
    EPN_TX_L2S_QUE_CONFIG__CFGL2SQUEEND_FIELD_MASK,
    0,
    EPN_TX_L2S_QUE_CONFIG__CFGL2SQUEEND_FIELD_WIDTH,
    EPN_TX_L2S_QUE_CONFIG__CFGL2SQUEEND_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_TX_L2S_QUE_CONFIG__RESERVED1
 ******************************************************************************/
const ru_field_rec EPN_TX_L2S_QUE_CONFIG__RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_TX_L2S_QUE_CONFIG__RESERVED1_FIELD_MASK,
    0,
    EPN_TX_L2S_QUE_CONFIG__RESERVED1_FIELD_WIDTH,
    EPN_TX_L2S_QUE_CONFIG__RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_TX_L2S_QUE_CONFIG__CFGL2SQUESTART
 ******************************************************************************/
const ru_field_rec EPN_TX_L2S_QUE_CONFIG__CFGL2SQUESTART_FIELD =
{
    "CFGL2SQUESTART",
#if RU_INCLUDE_DESC
    "",
    "Queue i Start address",
#endif
    EPN_TX_L2S_QUE_CONFIG__CFGL2SQUESTART_FIELD_MASK,
    0,
    EPN_TX_L2S_QUE_CONFIG__CFGL2SQUESTART_FIELD_WIDTH,
    EPN_TX_L2S_QUE_CONFIG__CFGL2SQUESTART_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_TX_L2S_QUE_EMPTY_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_TX_L2S_QUE_EMPTY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_TX_L2S_QUE_EMPTY_RESERVED0_FIELD_MASK,
    0,
    EPN_TX_L2S_QUE_EMPTY_RESERVED0_FIELD_WIDTH,
    EPN_TX_L2S_QUE_EMPTY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_TX_L2S_QUE_EMPTY_L2SQUEEMPTY
 ******************************************************************************/
const ru_field_rec EPN_TX_L2S_QUE_EMPTY_L2SQUEEMPTY_FIELD =
{
    "L2SQUEEMPTY",
#if RU_INCLUDE_DESC
    "",
    "Each bit in this register contains the status of the respective"
    "queue"
    "0: L2 queue is not empty"
    "1: L2 queue is empty",
#endif
    EPN_TX_L2S_QUE_EMPTY_L2SQUEEMPTY_FIELD_MASK,
    0,
    EPN_TX_L2S_QUE_EMPTY_L2SQUEEMPTY_FIELD_WIDTH,
    EPN_TX_L2S_QUE_EMPTY_L2SQUEEMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_TX_L2S_QUE_FULL_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_TX_L2S_QUE_FULL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_TX_L2S_QUE_FULL_RESERVED0_FIELD_MASK,
    0,
    EPN_TX_L2S_QUE_FULL_RESERVED0_FIELD_WIDTH,
    EPN_TX_L2S_QUE_FULL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_TX_L2S_QUE_FULL_L2SQUEFULL
 ******************************************************************************/
const ru_field_rec EPN_TX_L2S_QUE_FULL_L2SQUEFULL_FIELD =
{
    "L2SQUEFULL",
#if RU_INCLUDE_DESC
    "",
    "Each bit in this register contains the status of the respective"
    "queue"
    "0: L2 queue is not full"
    "1: L2 queue is full",
#endif
    EPN_TX_L2S_QUE_FULL_L2SQUEFULL_FIELD_MASK,
    0,
    EPN_TX_L2S_QUE_FULL_L2SQUEFULL_FIELD_WIDTH,
    EPN_TX_L2S_QUE_FULL_L2SQUEFULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_TX_L2S_QUE_STOPPED_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_TX_L2S_QUE_STOPPED_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_TX_L2S_QUE_STOPPED_RESERVED0_FIELD_MASK,
    0,
    EPN_TX_L2S_QUE_STOPPED_RESERVED0_FIELD_WIDTH,
    EPN_TX_L2S_QUE_STOPPED_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_TX_L2S_QUE_STOPPED_L2SSTOPPEDQUEUES
 ******************************************************************************/
const ru_field_rec EPN_TX_L2S_QUE_STOPPED_L2SSTOPPEDQUEUES_FIELD =
{
    "L2SSTOPPEDQUEUES",
#if RU_INCLUDE_DESC
    "",
    "Each bit in this register contains the status of the respective"
    "queue"
    "0: L2 queue is not stopped"
    "1: L2 queue is stopped",
#endif
    EPN_TX_L2S_QUE_STOPPED_L2SSTOPPEDQUEUES_FIELD_MASK,
    0,
    EPN_TX_L2S_QUE_STOPPED_L2SSTOPPEDQUEUES_FIELD_WIDTH,
    EPN_TX_L2S_QUE_STOPPED_L2SSTOPPEDQUEUES_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_TX_CTC_BURST_LIMIT__RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_TX_CTC_BURST_LIMIT__RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_TX_CTC_BURST_LIMIT__RESERVED0_FIELD_MASK,
    0,
    EPN_TX_CTC_BURST_LIMIT__RESERVED0_FIELD_WIDTH,
    EPN_TX_CTC_BURST_LIMIT__RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_TX_CTC_BURST_LIMIT__PRVBURSTLIMIT
 ******************************************************************************/
const ru_field_rec EPN_TX_CTC_BURST_LIMIT__PRVBURSTLIMIT_FIELD =
{
    "PRVBURSTLIMIT",
#if RU_INCLUDE_DESC
    "",
    "L2 queue i CTC mode burst limit",
#endif
    EPN_TX_CTC_BURST_LIMIT__PRVBURSTLIMIT_FIELD_MASK,
    0,
    EPN_TX_CTC_BURST_LIMIT__PRVBURSTLIMIT_FIELD_WIDTH,
    EPN_TX_CTC_BURST_LIMIT__PRVBURSTLIMIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_MAX_OUTSTANDING_TARDY_PACKETS_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_BBH_MAX_OUTSTANDING_TARDY_PACKETS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_BBH_MAX_OUTSTANDING_TARDY_PACKETS_RESERVED0_FIELD_MASK,
    0,
    EPN_BBH_MAX_OUTSTANDING_TARDY_PACKETS_RESERVED0_FIELD_WIDTH,
    EPN_BBH_MAX_OUTSTANDING_TARDY_PACKETS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_MAX_OUTSTANDING_TARDY_PACKETS_CFGMAXOUTSTANDINGTARDYPACKETS
 ******************************************************************************/
const ru_field_rec EPN_BBH_MAX_OUTSTANDING_TARDY_PACKETS_CFGMAXOUTSTANDINGTARDYPACKETS_FIELD =
{
    "CFGMAXOUTSTANDINGTARDYPACKETS",
#if RU_INCLUDE_DESC
    "",
    "Maximum number of packets outstanding tardy packet the BBH can"
    "accumulate at one time.",
#endif
    EPN_BBH_MAX_OUTSTANDING_TARDY_PACKETS_CFGMAXOUTSTANDINGTARDYPACKETS_FIELD_MASK,
    0,
    EPN_BBH_MAX_OUTSTANDING_TARDY_PACKETS_CFGMAXOUTSTANDINGTARDYPACKETS_FIELD_WIDTH,
    EPN_BBH_MAX_OUTSTANDING_TARDY_PACKETS_CFGMAXOUTSTANDINGTARDYPACKETS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MIN_REPORT_VALUE_DIFFERENCE_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_MIN_REPORT_VALUE_DIFFERENCE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_MIN_REPORT_VALUE_DIFFERENCE_RESERVED0_FIELD_MASK,
    0,
    EPN_MIN_REPORT_VALUE_DIFFERENCE_RESERVED0_FIELD_WIDTH,
    EPN_MIN_REPORT_VALUE_DIFFERENCE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_MIN_REPORT_VALUE_DIFFERENCE_PRVMINREPORTDIFF
 ******************************************************************************/
const ru_field_rec EPN_MIN_REPORT_VALUE_DIFFERENCE_PRVMINREPORTDIFF_FIELD =
{
    "PRVMINREPORTDIFF",
#if RU_INCLUDE_DESC
    "",
    "The smallest allowable difference between ajacent non-zero queue"
    "sets.",
#endif
    EPN_MIN_REPORT_VALUE_DIFFERENCE_PRVMINREPORTDIFF_FIELD_MASK,
    0,
    EPN_MIN_REPORT_VALUE_DIFFERENCE_PRVMINREPORTDIFF_FIELD_WIDTH,
    EPN_MIN_REPORT_VALUE_DIFFERENCE_PRVMINREPORTDIFF_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_BBH_STATUS_FIFO_OVERFLOW_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_BBH_STATUS_FIFO_OVERFLOW_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_BBH_STATUS_FIFO_OVERFLOW_RESERVED0_FIELD_MASK,
    0,
    EPN_BBH_STATUS_FIFO_OVERFLOW_RESERVED0_FIELD_WIDTH,
    EPN_BBH_STATUS_FIFO_OVERFLOW_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_BBH_STATUS_FIFO_OVERFLOW_UTXBBHSTATUSFIFOOVERFLOW
 ******************************************************************************/
const ru_field_rec EPN_BBH_STATUS_FIFO_OVERFLOW_UTXBBHSTATUSFIFOOVERFLOW_FIELD =
{
    "UTXBBHSTATUSFIFOOVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "Indicates which BBH queue status interface event FIFOs have"
    "overflowed. The overflow can only be corrected by reset.",
#endif
    EPN_BBH_STATUS_FIFO_OVERFLOW_UTXBBHSTATUSFIFOOVERFLOW_FIELD_MASK,
    0,
    EPN_BBH_STATUS_FIFO_OVERFLOW_UTXBBHSTATUSFIFOOVERFLOW_FIELD_WIDTH,
    EPN_BBH_STATUS_FIFO_OVERFLOW_UTXBBHSTATUSFIFOOVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_SPARE_CTL_CFGEPNSPARE
 ******************************************************************************/
const ru_field_rec EPN_SPARE_CTL_CFGEPNSPARE_FIELD =
{
    "CFGEPNSPARE",
#if RU_INCLUDE_DESC
    "",
    "Spare RW bits",
#endif
    EPN_SPARE_CTL_CFGEPNSPARE_FIELD_MASK,
    0,
    EPN_SPARE_CTL_CFGEPNSPARE_FIELD_WIDTH,
    EPN_SPARE_CTL_CFGEPNSPARE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_SPARE_CTL_ECOUTXSNFENABLE
 ******************************************************************************/
const ru_field_rec EPN_SPARE_CTL_ECOUTXSNFENABLE_FIELD =
{
    "ECOUTXSNFENABLE",
#if RU_INCLUDE_DESC
    "",
    "Set this bit to enable store and forward upstream AE operation."
    "Only set this bit when operating in Point-to-Point or Active"
    "Ethernet modes.",
#endif
    EPN_SPARE_CTL_ECOUTXSNFENABLE_FIELD_MASK,
    0,
    EPN_SPARE_CTL_ECOUTXSNFENABLE_FIELD_WIDTH,
    EPN_SPARE_CTL_ECOUTXSNFENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_SPARE_CTL_ECOJIRA758ENABLE
 ******************************************************************************/
const ru_field_rec EPN_SPARE_CTL_ECOJIRA758ENABLE_FIELD =
{
    "ECOJIRA758ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Set this bit to prevent transmitting when the LLID's shapers are"
    "empty.  This will prevent"
    "the ONU from using excess bandwidth granted to it by the OLT."
    "Do not set this bit when operating in Point-to-Point or Active"
    "Ethernet modes.",
#endif
    EPN_SPARE_CTL_ECOJIRA758ENABLE_FIELD_MASK,
    0,
    EPN_SPARE_CTL_ECOJIRA758ENABLE_FIELD_WIDTH,
    EPN_SPARE_CTL_ECOJIRA758ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_TS_SYNC_OFFSET_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_TS_SYNC_OFFSET_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_TS_SYNC_OFFSET_RESERVED0_FIELD_MASK,
    0,
    EPN_TS_SYNC_OFFSET_RESERVED0_FIELD_WIDTH,
    EPN_TS_SYNC_OFFSET_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_TS_SYNC_OFFSET_CFGTSSYNCOFFSET
 ******************************************************************************/
const ru_field_rec EPN_TS_SYNC_OFFSET_CFGTSSYNCOFFSET_FIELD =
{
    "CFGTSSYNCOFFSET",
#if RU_INCLUDE_DESC
    "",
    "Provides lowest 6 bits of timestamp synchronizer, from 250 MHz"
    "domain to 125 MHz.",
#endif
    EPN_TS_SYNC_OFFSET_CFGTSSYNCOFFSET_FIELD_MASK,
    0,
    EPN_TS_SYNC_OFFSET_CFGTSSYNCOFFSET_FIELD_WIDTH,
    EPN_TS_SYNC_OFFSET_CFGTSSYNCOFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DN_TS_OFFSET_CFGDNTSOFFSET
 ******************************************************************************/
const ru_field_rec EPN_DN_TS_OFFSET_CFGDNTSOFFSET_FIELD =
{
    "CFGDNTSOFFSET",
#if RU_INCLUDE_DESC
    "",
    "Provides signed offset for downstream packet timestamping.",
#endif
    EPN_DN_TS_OFFSET_CFGDNTSOFFSET_FIELD_MASK,
    0,
    EPN_DN_TS_OFFSET_CFGDNTSOFFSET_FIELD_WIDTH,
    EPN_DN_TS_OFFSET_CFGDNTSOFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_UP_TS_OFFSET_LO_CFGUPTSOFFSET_LO
 ******************************************************************************/
const ru_field_rec EPN_UP_TS_OFFSET_LO_CFGUPTSOFFSET_LO_FIELD =
{
    "CFGUPTSOFFSET_LO",
#if RU_INCLUDE_DESC
    "",
    "Provides signed offset for upstream packet timestamping.",
#endif
    EPN_UP_TS_OFFSET_LO_CFGUPTSOFFSET_LO_FIELD_MASK,
    0,
    EPN_UP_TS_OFFSET_LO_CFGUPTSOFFSET_LO_FIELD_WIDTH,
    EPN_UP_TS_OFFSET_LO_CFGUPTSOFFSET_LO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_UP_TS_OFFSET_HI_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_UP_TS_OFFSET_HI_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_UP_TS_OFFSET_HI_RESERVED0_FIELD_MASK,
    0,
    EPN_UP_TS_OFFSET_HI_RESERVED0_FIELD_WIDTH,
    EPN_UP_TS_OFFSET_HI_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_UP_TS_OFFSET_HI_CFGUPTSOFFSET_HI
 ******************************************************************************/
const ru_field_rec EPN_UP_TS_OFFSET_HI_CFGUPTSOFFSET_HI_FIELD =
{
    "CFGUPTSOFFSET_HI",
#if RU_INCLUDE_DESC
    "",
    "Provides signed offset for upstream packet timestamping.",
#endif
    EPN_UP_TS_OFFSET_HI_CFGUPTSOFFSET_HI_FIELD_MASK,
    0,
    EPN_UP_TS_OFFSET_HI_CFGUPTSOFFSET_HI_FIELD_WIDTH,
    EPN_UP_TS_OFFSET_HI_CFGUPTSOFFSET_HI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_TWO_STEP_TS_CTL_TWOSTEPFFRD
 ******************************************************************************/
const ru_field_rec EPN_TWO_STEP_TS_CTL_TWOSTEPFFRD_FIELD =
{
    "TWOSTEPFFRD",
#if RU_INCLUDE_DESC
    "",
    "Provides the reading of the two step timestamp FIFO.  A write value"
    "of 1 will advance the FIFO to the next entry.  The 48-bits value is"
    "provided by registers"
    "EPN_TWO_STEP_TS_VALUE_HI/EPN_TWO_STEP_TS_VALUE_LO.",
#endif
    EPN_TWO_STEP_TS_CTL_TWOSTEPFFRD_FIELD_MASK,
    0,
    EPN_TWO_STEP_TS_CTL_TWOSTEPFFRD_FIELD_WIDTH,
    EPN_TWO_STEP_TS_CTL_TWOSTEPFFRD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_TWO_STEP_TS_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_TWO_STEP_TS_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_TWO_STEP_TS_CTL_RESERVED0_FIELD_MASK,
    0,
    EPN_TWO_STEP_TS_CTL_RESERVED0_FIELD_WIDTH,
    EPN_TWO_STEP_TS_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_TWO_STEP_TS_CTL_TWOSTEPFFENTRIES
 ******************************************************************************/
const ru_field_rec EPN_TWO_STEP_TS_CTL_TWOSTEPFFENTRIES_FIELD =
{
    "TWOSTEPFFENTRIES",
#if RU_INCLUDE_DESC
    "",
    "Indicates the number of entries in the two step timestamp FIFO.",
#endif
    EPN_TWO_STEP_TS_CTL_TWOSTEPFFENTRIES_FIELD_MASK,
    0,
    EPN_TWO_STEP_TS_CTL_TWOSTEPFFENTRIES_FIELD_WIDTH,
    EPN_TWO_STEP_TS_CTL_TWOSTEPFFENTRIES_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_TWO_STEP_TS_VALUE_LO_TWOSTEPTIMESTAMP_LO
 ******************************************************************************/
const ru_field_rec EPN_TWO_STEP_TS_VALUE_LO_TWOSTEPTIMESTAMP_LO_FIELD =
{
    "TWOSTEPTIMESTAMP_LO",
#if RU_INCLUDE_DESC
    "",
    "Lower 32-bits of two-step timestamp value for IEEE 1588"
    "timestamping.",
#endif
    EPN_TWO_STEP_TS_VALUE_LO_TWOSTEPTIMESTAMP_LO_FIELD_MASK,
    0,
    EPN_TWO_STEP_TS_VALUE_LO_TWOSTEPTIMESTAMP_LO_FIELD_WIDTH,
    EPN_TWO_STEP_TS_VALUE_LO_TWOSTEPTIMESTAMP_LO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_TWO_STEP_TS_VALUE_HI_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_TWO_STEP_TS_VALUE_HI_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_TWO_STEP_TS_VALUE_HI_RESERVED0_FIELD_MASK,
    0,
    EPN_TWO_STEP_TS_VALUE_HI_RESERVED0_FIELD_WIDTH,
    EPN_TWO_STEP_TS_VALUE_HI_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_TWO_STEP_TS_VALUE_HI_TWOSTEPTIMESTAMP_HI
 ******************************************************************************/
const ru_field_rec EPN_TWO_STEP_TS_VALUE_HI_TWOSTEPTIMESTAMP_HI_FIELD =
{
    "TWOSTEPTIMESTAMP_HI",
#if RU_INCLUDE_DESC
    "",
    "Upper 16-bits of two-step timestamp value for IEEE 1588"
    "timestamping.",
#endif
    EPN_TWO_STEP_TS_VALUE_HI_TWOSTEPTIMESTAMP_HI_FIELD_MASK,
    0,
    EPN_TWO_STEP_TS_VALUE_HI_TWOSTEPTIMESTAMP_HI_FIELD_WIDTH,
    EPN_TWO_STEP_TS_VALUE_HI_TWOSTEPTIMESTAMP_HI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_1588_TIMESTAMP_INT_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_1588_TIMESTAMP_INT_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_1588_TIMESTAMP_INT_STATUS_RESERVED0_FIELD_MASK,
    0,
    EPN_1588_TIMESTAMP_INT_STATUS_RESERVED0_FIELD_WIDTH,
    EPN_1588_TIMESTAMP_INT_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_1588_TIMESTAMP_INT_STATUS_INT1588PKTABORT
 ******************************************************************************/
const ru_field_rec EPN_1588_TIMESTAMP_INT_STATUS_INT1588PKTABORT_FIELD =
{
    "INT1588PKTABORT",
#if RU_INCLUDE_DESC
    "",
    "Indicated 1588 timestamp packet was aborted due to illegal checksum"
    "or timestamp offsets.",
#endif
    EPN_1588_TIMESTAMP_INT_STATUS_INT1588PKTABORT_FIELD_MASK,
    0,
    EPN_1588_TIMESTAMP_INT_STATUS_INT1588PKTABORT_FIELD_WIDTH,
    EPN_1588_TIMESTAMP_INT_STATUS_INT1588PKTABORT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_1588_TIMESTAMP_INT_STATUS_INT1588TWOSTEPFFINT
 ******************************************************************************/
const ru_field_rec EPN_1588_TIMESTAMP_INT_STATUS_INT1588TWOSTEPFFINT_FIELD =
{
    "INT1588TWOSTEPFFINT",
#if RU_INCLUDE_DESC
    "",
    "Indicates timestamp in two step FIFO is available for reading. The"
    "48-bits value is provided by registers"
    "EPN_TWO_STEP_TS_VALUE_HI/EPN_TWO_STEP_TS_VALUE_LO.",
#endif
    EPN_1588_TIMESTAMP_INT_STATUS_INT1588TWOSTEPFFINT_FIELD_MASK,
    0,
    EPN_1588_TIMESTAMP_INT_STATUS_INT1588TWOSTEPFFINT_FIELD_WIDTH,
    EPN_1588_TIMESTAMP_INT_STATUS_INT1588TWOSTEPFFINT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_1588_TIMESTAMP_INT_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_1588_TIMESTAMP_INT_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_1588_TIMESTAMP_INT_MASK_RESERVED0_FIELD_MASK,
    0,
    EPN_1588_TIMESTAMP_INT_MASK_RESERVED0_FIELD_WIDTH,
    EPN_1588_TIMESTAMP_INT_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_1588_TIMESTAMP_INT_MASK_TS1588PKTABORT_MASK
 ******************************************************************************/
const ru_field_rec EPN_1588_TIMESTAMP_INT_MASK_TS1588PKTABORT_MASK_FIELD =
{
    "TS1588PKTABORT_MASK",
#if RU_INCLUDE_DESC
    "",
    "Mask 1588 timestamp packet abort.",
#endif
    EPN_1588_TIMESTAMP_INT_MASK_TS1588PKTABORT_MASK_FIELD_MASK,
    0,
    EPN_1588_TIMESTAMP_INT_MASK_TS1588PKTABORT_MASK_FIELD_WIDTH,
    EPN_1588_TIMESTAMP_INT_MASK_TS1588PKTABORT_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_1588_TIMESTAMP_INT_MASK_TS1588TWOSTEPFF_MASK
 ******************************************************************************/
const ru_field_rec EPN_1588_TIMESTAMP_INT_MASK_TS1588TWOSTEPFF_MASK_FIELD =
{
    "TS1588TWOSTEPFF_MASK",
#if RU_INCLUDE_DESC
    "",
    "Mask two step FIFO interrupt.",
#endif
    EPN_1588_TIMESTAMP_INT_MASK_TS1588TWOSTEPFF_MASK_FIELD_MASK,
    0,
    EPN_1588_TIMESTAMP_INT_MASK_TS1588TWOSTEPFF_MASK_FIELD_WIDTH,
    EPN_1588_TIMESTAMP_INT_MASK_TS1588TWOSTEPFF_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_UP_PACKET_FETCH_MARGIN_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_UP_PACKET_FETCH_MARGIN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_UP_PACKET_FETCH_MARGIN_RESERVED0_FIELD_MASK,
    0,
    EPN_UP_PACKET_FETCH_MARGIN_RESERVED0_FIELD_WIDTH,
    EPN_UP_PACKET_FETCH_MARGIN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_UP_PACKET_FETCH_MARGIN_UPPACKETFETCHMARGIN
 ******************************************************************************/
const ru_field_rec EPN_UP_PACKET_FETCH_MARGIN_UPPACKETFETCHMARGIN_FIELD =
{
    "UPPACKETFETCHMARGIN",
#if RU_INCLUDE_DESC
    "",
    "Minimum BBH upstream packet request latency. Units are TQ.",
#endif
    EPN_UP_PACKET_FETCH_MARGIN_UPPACKETFETCHMARGIN_FIELD_MASK,
    0,
    EPN_UP_PACKET_FETCH_MARGIN_UPPACKETFETCHMARGIN_FIELD_WIDTH,
    EPN_UP_PACKET_FETCH_MARGIN_UPPACKETFETCHMARGIN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_DN_1588_TIMESTAMP_DN_1588_TS
 ******************************************************************************/
const ru_field_rec EPN_DN_1588_TIMESTAMP_DN_1588_TS_FIELD =
{
    "DN_1588_TS",
#if RU_INCLUDE_DESC
    "",
    "32-bits timestamp value of downstream packet.",
#endif
    EPN_DN_1588_TIMESTAMP_DN_1588_TS_FIELD_MASK,
    0,
    EPN_DN_1588_TIMESTAMP_DN_1588_TS_FIELD_WIDTH,
    EPN_DN_1588_TIMESTAMP_DN_1588_TS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPN_PERSISTENT_REPORT_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_PERSISTENT_REPORT_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_PERSISTENT_REPORT_CFG_RESERVED0_FIELD_MASK,
    0,
    EPN_PERSISTENT_REPORT_CFG_RESERVED0_FIELD_WIDTH,
    EPN_PERSISTENT_REPORT_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_PERSISTENT_REPORT_CFG_CFGPERSRPTDURATION
 ******************************************************************************/
const ru_field_rec EPN_PERSISTENT_REPORT_CFG_CFGPERSRPTDURATION_FIELD =
{
    "CFGPERSRPTDURATION",
#if RU_INCLUDE_DESC
    "",
    "How long report persistance lasts."
    "(How many report persistance timer ticks after the last non-zero"
    "report should empty reports"
    "be replaced with a persistant report)"
    "Units are report perisitance timer ticks.",
#endif
    EPN_PERSISTENT_REPORT_CFG_CFGPERSRPTDURATION_FIELD_MASK,
    0,
    EPN_PERSISTENT_REPORT_CFG_CFGPERSRPTDURATION_FIELD_WIDTH,
    EPN_PERSISTENT_REPORT_CFG_CFGPERSRPTDURATION_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_PERSISTENT_REPORT_CFG_CFGPERSRPTTICKSIZE
 ******************************************************************************/
const ru_field_rec EPN_PERSISTENT_REPORT_CFG_CFGPERSRPTTICKSIZE_FIELD =
{
    "CFGPERSRPTTICKSIZE",
#if RU_INCLUDE_DESC
    "",
    "How many clock cycles are in each report persistance timer tick."
    "The 125 MHz core clock rate requires 125 clocks per 1 uS tick."
    "Units are core clock cycles.",
#endif
    EPN_PERSISTENT_REPORT_CFG_CFGPERSRPTTICKSIZE_FIELD_MASK,
    0,
    EPN_PERSISTENT_REPORT_CFG_CFGPERSRPTTICKSIZE_FIELD_WIDTH,
    EPN_PERSISTENT_REPORT_CFG_CFGPERSRPTTICKSIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_PERSISTENT_REPORT_ENABLES_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_PERSISTENT_REPORT_ENABLES_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_PERSISTENT_REPORT_ENABLES_RESERVED0_FIELD_MASK,
    0,
    EPN_PERSISTENT_REPORT_ENABLES_RESERVED0_FIELD_WIDTH,
    EPN_PERSISTENT_REPORT_ENABLES_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_PERSISTENT_REPORT_ENABLES_CFGPERSRPTENABLE
 ******************************************************************************/
const ru_field_rec EPN_PERSISTENT_REPORT_ENABLES_CFGPERSRPTENABLE_FIELD =
{
    "CFGPERSRPTENABLE",
#if RU_INCLUDE_DESC
    "",
    "Per LLID enable for persistent reporting.  Set the bit to enable"
    "persistent reporting."
    "0: Disable persistent reporting."
    "1: Enable persistent reporting.",
#endif
    EPN_PERSISTENT_REPORT_ENABLES_CFGPERSRPTENABLE_FIELD_MASK,
    0,
    EPN_PERSISTENT_REPORT_ENABLES_CFGPERSRPTENABLE_FIELD_WIDTH,
    EPN_PERSISTENT_REPORT_ENABLES_CFGPERSRPTENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_PERSISTENT_REPORT_REQUEST_SIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_PERSISTENT_REPORT_REQUEST_SIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_PERSISTENT_REPORT_REQUEST_SIZE_RESERVED0_FIELD_MASK,
    0,
    EPN_PERSISTENT_REPORT_REQUEST_SIZE_RESERVED0_FIELD_WIDTH,
    EPN_PERSISTENT_REPORT_REQUEST_SIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_PERSISTENT_REPORT_REQUEST_SIZE_CFGPERSRPTREQTQ
 ******************************************************************************/
const ru_field_rec EPN_PERSISTENT_REPORT_REQUEST_SIZE_CFGPERSRPTREQTQ_FIELD =
{
    "CFGPERSRPTREQTQ",
#if RU_INCLUDE_DESC
    "",
    "How many Time Quanta the persistent report should request.  Smaller"
    "values waste less upstream bandwidth."
    "Units are Time Quanta.",
#endif
    EPN_PERSISTENT_REPORT_REQUEST_SIZE_CFGPERSRPTREQTQ_FIELD_MASK,
    0,
    EPN_PERSISTENT_REPORT_REQUEST_SIZE_CFGPERSRPTREQTQ_FIELD_WIDTH,
    EPN_PERSISTENT_REPORT_REQUEST_SIZE_CFGPERSRPTREQTQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: EPN_CONTROL_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_CONTROL_0_FIELDS[] =
{
    &EPN_CONTROL_0_CFGEN1588TS_FIELD,
    &EPN_CONTROL_0_CFGREPLACEUPFCS_FIELD,
    &EPN_CONTROL_0_CFGAPPENDUPFCS_FIELD,
    &EPN_CONTROL_0_CFGDROPSCB_FIELD,
    &EPN_CONTROL_0_MODUNCAPPEDREPORTLIMIT_FIELD,
    &EPN_CONTROL_0_MODMPQUESETFIRST_FIELD,
    &EPN_CONTROL_0_RESERVED0_FIELD,
    &EPN_CONTROL_0_PRVLOCALMPCPPROPAGATION_FIELD,
    &EPN_CONTROL_0_PRVTEKMODEPREFETCH_FIELD,
    &EPN_CONTROL_0_RESERVED1_FIELD,
    &EPN_CONTROL_0_PRVNOUNMAPPPEDFCS_FIELD,
    &EPN_CONTROL_0_PRVSUPRESSDISCEN_FIELD,
    &EPN_CONTROL_0_CFGVLANMAX_FIELD,
    &EPN_CONTROL_0_FCSERRONLYDATAFR_FIELD,
    &EPN_CONTROL_0_RESERVED2_FIELD,
    &EPN_CONTROL_0_PRVDROPUNMAPPPEDLLID_FIELD,
    &EPN_CONTROL_0_PRVSUPPRESSLLIDMODEBIT_FIELD,
    &EPN_CONTROL_0_MODDISCOVERYDAFILTEREN_FIELD,
    &EPN_CONTROL_0_RPTSELECT_FIELD,
    &EPN_CONTROL_0_PRVDISABLESVAQUESTATUSBP_FIELD,
    &EPN_CONTROL_0_UTXLOOPBACK_FIELD,
    &EPN_CONTROL_0_UTXEN_FIELD,
    &EPN_CONTROL_0_UTXRST_PRE_N_FIELD,
    &EPN_CONTROL_0_CFGDISABLEDNS_FIELD,
    &EPN_CONTROL_0_DRXLOOPBACK_FIELD,
    &EPN_CONTROL_0_DRXEN_FIELD,
    &EPN_CONTROL_0_DRXRST_PRE_N_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_CONTROL_0_REG = 
{
    "CONTROL_0",
#if RU_INCLUDE_DESC
    "EPN_CONTROL_0 Register",
    "This register controls and configures the modules in the EPON block.",
#endif
    EPN_CONTROL_0_REG_OFFSET,
    0,
    0,
    18,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    27,
    EPN_CONTROL_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_CONTROL_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_CONTROL_1_FIELDS[] =
{
    &EPN_CONTROL_1_RESERVED0_FIELD,
    &EPN_CONTROL_1_CFGDISABLEMPCPCORRECTIONDITHERING_FIELD,
    &EPN_CONTROL_1_PRVOVERLAPPEDGNTENABLE_FIELD,
    &EPN_CONTROL_1_RSTMISALIGNTHR_FIELD,
    &EPN_CONTROL_1_RESERVED1_FIELD,
    &EPN_CONTROL_1_CFGSTALEGNTCHK_FIELD,
    &EPN_CONTROL_1_FECRPTEN_FIELD,
    &EPN_CONTROL_1_RESERVED2_FIELD,
    &EPN_CONTROL_1_CFGL1L2TRUESTRICT_FIELD,
    &EPN_CONTROL_1_CFGCTCRPT_FIELD,
    &EPN_CONTROL_1_CFGTSCORRDIS_FIELD,
    &EPN_CONTROL_1_CFGNODISCRPT_FIELD,
    &EPN_CONTROL_1_DISABLEDISCSCALE_FIELD,
    &EPN_CONTROL_1_CLRONRD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_CONTROL_1_REG = 
{
    "CONTROL_1",
#if RU_INCLUDE_DESC
    "EPN_CONTROL_1 Register",
    "This register controls and configures the modules in the EPN block.",
#endif
    EPN_CONTROL_1_REG_OFFSET,
    0,
    0,
    19,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    14,
    EPN_CONTROL_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_ENABLE_GRANTS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_ENABLE_GRANTS_FIELDS[] =
{
    &EPN_ENABLE_GRANTS_RESERVED0_FIELD,
    &EPN_ENABLE_GRANTS_ENGNT7_FIELD,
    &EPN_ENABLE_GRANTS_ENGNT6_FIELD,
    &EPN_ENABLE_GRANTS_ENGNT5_FIELD,
    &EPN_ENABLE_GRANTS_ENGNT4_FIELD,
    &EPN_ENABLE_GRANTS_ENGNT3_FIELD,
    &EPN_ENABLE_GRANTS_ENGNT2_FIELD,
    &EPN_ENABLE_GRANTS_ENGNT1_FIELD,
    &EPN_ENABLE_GRANTS_ENGNT0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_ENABLE_GRANTS_REG = 
{
    "ENABLE_GRANTS",
#if RU_INCLUDE_DESC
    "EPN_ENABLE_GRANTS Register",
    "This register allows per-LLID control over whether the EPON MAC accepts"
    "grants from the OLT.",
#endif
    EPN_ENABLE_GRANTS_REG_OFFSET,
    0,
    0,
    20,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_ENABLE_GRANTS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DROP_DISC_GATES
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DROP_DISC_GATES_FIELDS[] =
{
    &EPN_DROP_DISC_GATES_RESERVED0_FIELD,
    &EPN_DROP_DISC_GATES_SINKDISCGATES7_FIELD,
    &EPN_DROP_DISC_GATES_SINKDISCGATES6_FIELD,
    &EPN_DROP_DISC_GATES_SINKDISCGATES5_FIELD,
    &EPN_DROP_DISC_GATES_SINKDISCGATES4_FIELD,
    &EPN_DROP_DISC_GATES_SINKDISCGATES3_FIELD,
    &EPN_DROP_DISC_GATES_SINKDISCGATES2_FIELD,
    &EPN_DROP_DISC_GATES_SINKDISCGATES1_FIELD,
    &EPN_DROP_DISC_GATES_SINKDISCGATES0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DROP_DISC_GATES_REG = 
{
    "DROP_DISC_GATES",
#if RU_INCLUDE_DESC
    "EPN_DROP_DISC_GATES Register",
    "This register allows per-LLID control over whether the EPON MAC"
    "processes Discovery Gates.",
#endif
    EPN_DROP_DISC_GATES_REG_OFFSET,
    0,
    0,
    21,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_DROP_DISC_GATES_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DIS_FCS_CHK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DIS_FCS_CHK_FIELDS[] =
{
    &EPN_DIS_FCS_CHK_RESERVED0_FIELD,
    &EPN_DIS_FCS_CHK_DISABLEFCSCHKDN23_FIELD,
    &EPN_DIS_FCS_CHK_DISABLEFCSCHKDN22_FIELD,
    &EPN_DIS_FCS_CHK_DISABLEFCSCHKDN21_FIELD,
    &EPN_DIS_FCS_CHK_DISABLEFCSCHKDN20_FIELD,
    &EPN_DIS_FCS_CHK_DISABLEFCSCHKDN19_FIELD,
    &EPN_DIS_FCS_CHK_DISABLEFCSCHKDN18_FIELD,
    &EPN_DIS_FCS_CHK_DISABLEFCSCHKDN17_FIELD,
    &EPN_DIS_FCS_CHK_DISABLEFCSCHKDN16_FIELD,
    &EPN_DIS_FCS_CHK_RESERVED1_FIELD,
    &EPN_DIS_FCS_CHK_DISABLEFCSCHK7_FIELD,
    &EPN_DIS_FCS_CHK_DISABLEFCSCHK6_FIELD,
    &EPN_DIS_FCS_CHK_DISABLEFCSCHK5_FIELD,
    &EPN_DIS_FCS_CHK_DISABLEFCSCHK4_FIELD,
    &EPN_DIS_FCS_CHK_DISABLEFCSCHK3_FIELD,
    &EPN_DIS_FCS_CHK_DISABLEFCSCHK2_FIELD,
    &EPN_DIS_FCS_CHK_DISABLEFCSCHK1_FIELD,
    &EPN_DIS_FCS_CHK_DISABLEFCSCHK0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DIS_FCS_CHK_REG = 
{
    "DIS_FCS_CHK",
#if RU_INCLUDE_DESC
    "EPN_DIS_FCS_CHK Register",
    "This register allows per-LLID control over Ethernet frame CRC checking"
    "in the EPON downstream block.",
#endif
    EPN_DIS_FCS_CHK_REG_OFFSET,
    0,
    0,
    22,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    18,
    EPN_DIS_FCS_CHK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_PASS_GATES
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_PASS_GATES_FIELDS[] =
{
    &EPN_PASS_GATES_RESERVED0_FIELD,
    &EPN_PASS_GATES_PASSGATELLID7_FIELD,
    &EPN_PASS_GATES_PASSGATELLID6_FIELD,
    &EPN_PASS_GATES_PASSGATELLID5_FIELD,
    &EPN_PASS_GATES_PASSGATELLID4_FIELD,
    &EPN_PASS_GATES_PASSGATELLID3_FIELD,
    &EPN_PASS_GATES_PASSGATELLID2_FIELD,
    &EPN_PASS_GATES_PASSGATELLID1_FIELD,
    &EPN_PASS_GATES_PASSGATELLID0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_PASS_GATES_REG = 
{
    "PASS_GATES",
#if RU_INCLUDE_DESC
    "EPN_PASS_GATES Register",
    "This register allows per-LLID control over whether the EPON MAC passes"
    "downstream gate frames to the BBH.",
#endif
    EPN_PASS_GATES_REG_OFFSET,
    0,
    0,
    23,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_PASS_GATES_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_CFG_MISALGN_FB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_CFG_MISALGN_FB_FIELDS[] =
{
    &EPN_CFG_MISALGN_FB_RESERVED0_FIELD,
    &EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK7_FIELD,
    &EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK6_FIELD,
    &EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK5_FIELD,
    &EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK4_FIELD,
    &EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK3_FIELD,
    &EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK2_FIELD,
    &EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK1_FIELD,
    &EPN_CFG_MISALGN_FB_CFGMISALIGNFEEDBACK0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_CFG_MISALGN_FB_REG = 
{
    "CFG_MISALGN_FB",
#if RU_INCLUDE_DESC
    "EPN_CFG_MISALGN_FB Register",
    "This register allows per-LLID control over grant misalignment checking"
    "and feedback.",
#endif
    EPN_CFG_MISALGN_FB_REG_OFFSET,
    0,
    0,
    24,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_CFG_MISALGN_FB_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DISCOVERY_FILTER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DISCOVERY_FILTER_FIELDS[] =
{
    &EPN_DISCOVERY_FILTER_PRVDISCINFOMASK_FIELD,
    &EPN_DISCOVERY_FILTER_PRVDISCINFOVALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DISCOVERY_FILTER_REG = 
{
    "DISCOVERY_FILTER",
#if RU_INCLUDE_DESC
    "EPN_DISCOVERY_FILTER Register",
    "The 10G Discovery Information field is 16 bits right after the Sync"
    "time in the Disc Gate. The 10G discovery gate filter consistes of two"
    "bit-fields: a 16-bit Disc Info Value and 16-bit Disc Info Mask. For all"
    "bits in Disc Info Value whose corresponding Mask bits are clear (not"
    "masked), the bits in Disc Info Value must exactly match the"
    "corresponding bits in the GATE Discovery Information field. If they"
    "don't match, drop the GATE (nothing is added into the Grant FIFO). If"
    "they match, the Start Time and Length are added to the Grant FIFO.",
#endif
    EPN_DISCOVERY_FILTER_REG_OFFSET,
    0,
    0,
    25,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_DISCOVERY_FILTER_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_MINIMUM_GRANT_SETUP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_MINIMUM_GRANT_SETUP_FIELDS[] =
{
    &EPN_MINIMUM_GRANT_SETUP_RESERVED0_FIELD,
    &EPN_MINIMUM_GRANT_SETUP_CFGMINGRANTSETUP_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_MINIMUM_GRANT_SETUP_REG = 
{
    "MINIMUM_GRANT_SETUP",
#if RU_INCLUDE_DESC
    "EPN_MINIMUM_GRANT_SETUP Register",
    "The EPN requires a minimum amount of time to process each grant.  This"
    "processing time includes the time for the BBH to fetch a packet and"
    "setup time for the"
    "upstream data to be processed by the LIF/XIF.  It is possible for"
    "backpressure generated by the BBH and LIF/XIF to stall the grant"
    "processing beyond the time required to process the grant."
    "Any grants that are not processed this many TimeQuanta before it Grant"
    "Start Time will be aborted and a grant miss-abort interrupt will be"
    "generated.",
#endif
    EPN_MINIMUM_GRANT_SETUP_REG_OFFSET,
    0,
    0,
    26,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_MINIMUM_GRANT_SETUP_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_RESET_GNT_FIFO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_RESET_GNT_FIFO_FIELDS[] =
{
    &EPN_RESET_GNT_FIFO_RESERVED0_FIELD,
    &EPN_RESET_GNT_FIFO_RSTGNTFIFO7_FIELD,
    &EPN_RESET_GNT_FIFO_RSTGNTFIFO6_FIELD,
    &EPN_RESET_GNT_FIFO_RSTGNTFIFO5_FIELD,
    &EPN_RESET_GNT_FIFO_RSTGNTFIFO4_FIELD,
    &EPN_RESET_GNT_FIFO_RSTGNTFIFO3_FIELD,
    &EPN_RESET_GNT_FIFO_RSTGNTFIFO2_FIELD,
    &EPN_RESET_GNT_FIFO_RSTGNTFIFO1_FIELD,
    &EPN_RESET_GNT_FIFO_RSTGNTFIFO0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_RESET_GNT_FIFO_REG = 
{
    "RESET_GNT_FIFO",
#if RU_INCLUDE_DESC
    "EPN_RESET_GNT_FIFO Register",
    "This register allows resetting of the Grant FIFOs on a per-LLID basis.",
#endif
    EPN_RESET_GNT_FIFO_REG_OFFSET,
    0,
    0,
    27,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_RESET_GNT_FIFO_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_RESET_L1_ACCUMULATOR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_RESET_L1_ACCUMULATOR_FIELDS[] =
{
    &EPN_RESET_L1_ACCUMULATOR_RESERVED0_FIELD,
    &EPN_RESET_L1_ACCUMULATOR_CFGL1SCLRACUM_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_RESET_L1_ACCUMULATOR_REG = 
{
    "RESET_L1_ACCUMULATOR",
#if RU_INCLUDE_DESC
    "EPN_RESET_L1_ACCUMULATOR Register",
    "This register allows resetting of the L1 accumulators.",
#endif
    EPN_RESET_L1_ACCUMULATOR_REG_OFFSET,
    0,
    0,
    28,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_RESET_L1_ACCUMULATOR_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_L1_ACCUMULATOR_SEL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_L1_ACCUMULATOR_SEL_FIELDS[] =
{
    &EPN_L1_ACCUMULATOR_SEL_RESERVED0_FIELD,
    &EPN_L1_ACCUMULATOR_SEL_CFGL1SUVASIZESEL_FIELD,
    &EPN_L1_ACCUMULATOR_SEL_CFGL1SSVASIZESEL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_L1_ACCUMULATOR_SEL_REG = 
{
    "L1_ACCUMULATOR_SEL",
#if RU_INCLUDE_DESC
    "EPN_L1_ACCUMULATOR_SEL Register",
    "This register selects which virtual accumulator sizes are reported.",
#endif
    EPN_L1_ACCUMULATOR_SEL_REG_OFFSET,
    0,
    0,
    29,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    EPN_L1_ACCUMULATOR_SEL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_L1_SVA_BYTES
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_L1_SVA_BYTES_FIELDS[] =
{
    &EPN_L1_SVA_BYTES_RESERVED0_FIELD,
    &EPN_L1_SVA_BYTES_L1SSVASIZE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_L1_SVA_BYTES_REG = 
{
    "L1_SVA_BYTES",
#if RU_INCLUDE_DESC
    "EPN_L1_SVA_BYTES Register",
    "Signed number of bytes in the selected L1S Shaped Virtual Accumulator",
#endif
    EPN_L1_SVA_BYTES_REG_OFFSET,
    0,
    0,
    30,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_L1_SVA_BYTES_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_L1_UVA_BYTES
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_L1_UVA_BYTES_FIELDS[] =
{
    &EPN_L1_UVA_BYTES_RESERVED0_FIELD,
    &EPN_L1_UVA_BYTES_L1SUVASIZE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_L1_UVA_BYTES_REG = 
{
    "L1_UVA_BYTES",
#if RU_INCLUDE_DESC
    "EPN_L1_UVA_BYTES Register",
    "Signed number of bytes in the selected L1S Un-shaped Virtual"
    "Accumulator",
#endif
    EPN_L1_UVA_BYTES_REG_OFFSET,
    0,
    0,
    31,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_L1_UVA_BYTES_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_L1_SVA_OVERFLOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_L1_SVA_OVERFLOW_FIELDS[] =
{
    &EPN_L1_SVA_OVERFLOW_RESERVED0_FIELD,
    &EPN_L1_SVA_OVERFLOW_L1SSVAOVERFLOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_L1_SVA_OVERFLOW_REG = 
{
    "L1_SVA_OVERFLOW",
#if RU_INCLUDE_DESC
    "EPN_L1_SVA_OVERFLOW Register",
    "Indicates which SVAs have overflowed",
#endif
    EPN_L1_SVA_OVERFLOW_REG_OFFSET,
    0,
    0,
    32,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_L1_SVA_OVERFLOW_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_L1_UVA_OVERFLOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_L1_UVA_OVERFLOW_FIELDS[] =
{
    &EPN_L1_UVA_OVERFLOW_RESERVED0_FIELD,
    &EPN_L1_UVA_OVERFLOW_L1SUVAOVERFLOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_L1_UVA_OVERFLOW_REG = 
{
    "L1_UVA_OVERFLOW",
#if RU_INCLUDE_DESC
    "EPN_L1_UVA_OVERFLOW Register",
    "Indicates which UVAs have overflowed",
#endif
    EPN_L1_UVA_OVERFLOW_REG_OFFSET,
    0,
    0,
    33,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_L1_UVA_OVERFLOW_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_RESET_RPT_PRI
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_RESET_RPT_PRI_FIELDS[] =
{
    &EPN_RESET_RPT_PRI_RESERVED0_FIELD,
    &EPN_RESET_RPT_PRI_NULLRPTPRI15_FIELD,
    &EPN_RESET_RPT_PRI_NULLRPTPRI14_FIELD,
    &EPN_RESET_RPT_PRI_NULLRPTPRI13_FIELD,
    &EPN_RESET_RPT_PRI_NULLRPTPRI12_FIELD,
    &EPN_RESET_RPT_PRI_NULLRPTPRI11_FIELD,
    &EPN_RESET_RPT_PRI_NULLRPTPRI10_FIELD,
    &EPN_RESET_RPT_PRI_NULLRPTPRI9_FIELD,
    &EPN_RESET_RPT_PRI_NULLRPTPRI8_FIELD,
    &EPN_RESET_RPT_PRI_NULLRPTPRI7_FIELD,
    &EPN_RESET_RPT_PRI_NULLRPTPRI6_FIELD,
    &EPN_RESET_RPT_PRI_NULLRPTPRI5_FIELD,
    &EPN_RESET_RPT_PRI_NULLRPTPRI4_FIELD,
    &EPN_RESET_RPT_PRI_NULLRPTPRI3_FIELD,
    &EPN_RESET_RPT_PRI_NULLRPTPRI2_FIELD,
    &EPN_RESET_RPT_PRI_NULLRPTPRI1_FIELD,
    &EPN_RESET_RPT_PRI_NULLRPTPRI0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_RESET_RPT_PRI_REG = 
{
    "RESET_RPT_PRI",
#if RU_INCLUDE_DESC
    "EPN_RESET_RPT_PRI Register",
    "This register allows real-time forcing the per-priority report values"
    "to zero. This applies only to multi-priority reporting modes (CTC,"
    "NTT)."
    "Note: These bits are used for debug only.",
#endif
    EPN_RESET_RPT_PRI_REG_OFFSET,
    0,
    0,
    34,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    17,
    EPN_RESET_RPT_PRI_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_RESET_L2_RPT_FIFO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_RESET_L2_RPT_FIFO_FIELDS[] =
{
    &EPN_RESET_L2_RPT_FIFO_RESERVED0_FIELD,
    &EPN_RESET_L2_RPT_FIFO_CFGL2SCLRQUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_RESET_L2_RPT_FIFO_REG = 
{
    "RESET_L2_RPT_FIFO",
#if RU_INCLUDE_DESC
    "EPN_RESET_L2_RPT_FIFO Register",
    "This register allows resetting of the L2 Report FIFO pointers. The"
    "corresponding L2 accumulators are also cleared.",
#endif
    EPN_RESET_L2_RPT_FIFO_REG_OFFSET,
    0,
    0,
    35,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_RESET_L2_RPT_FIFO_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_ENABLE_UPSTREAM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_ENABLE_UPSTREAM_FIELDS[] =
{
    &EPN_ENABLE_UPSTREAM_RESERVED0_FIELD,
    &EPN_ENABLE_UPSTREAM_CFGENABLEUPSTREAMREG_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_ENABLE_UPSTREAM_REG = 
{
    "ENABLE_UPSTREAM",
#if RU_INCLUDE_DESC
    "EPN_ENABLE_UPSTREAM Register",
    "This register allows per-LLID enabling of upstream traffic. Disabling"
    "the upstream on a particular LLID Index means that:"
    "1. REPORT frames sent upstream on the LLID Index will report NO"
    "data"
    "2. Grants on the LLID Index will be acted upon (the laser will"
    "turn on and any requested REPORT frame will be sent), but no user"
    "frames will be pulled from FIF and sent upstream.",
#endif
    EPN_ENABLE_UPSTREAM_REG_OFFSET,
    0,
    0,
    36,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_ENABLE_UPSTREAM_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_ENABLE_UPSTREAM_FB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_ENABLE_UPSTREAM_FB_FIELDS[] =
{
    &EPN_ENABLE_UPSTREAM_FB_RESERVED0_FIELD,
    &EPN_ENABLE_UPSTREAM_FB_CFGENABLEUPSTREAMFEEDBACK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_ENABLE_UPSTREAM_FB_REG = 
{
    "ENABLE_UPSTREAM_FB",
#if RU_INCLUDE_DESC
    "EPN_ENABLE_UPSTREAM_FB Register",
    "Feedback register to indicate pending/complete changes in the EPN"
    "Enable Upstream register. A non-zero result from a bitwise XOR between"
    "this register and EPN Enable Upstream indicates that a new value"
    "written to EPN Enable Upstream has not yet taken effect.",
#endif
    EPN_ENABLE_UPSTREAM_FB_REG_OFFSET,
    0,
    0,
    37,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_ENABLE_UPSTREAM_FB_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_ENABLE_UPSTREAM_FEC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_ENABLE_UPSTREAM_FEC_FIELDS[] =
{
    &EPN_ENABLE_UPSTREAM_FEC_RESERVED0_FIELD,
    &EPN_ENABLE_UPSTREAM_FEC_CFGENABLEUPSTREAMFEC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_ENABLE_UPSTREAM_FEC_REG = 
{
    "ENABLE_UPSTREAM_FEC",
#if RU_INCLUDE_DESC
    "EPN_ENABLE_UPSTREAM_FEC Register",
    "Per-LLID index based upstream FEC enable. Set the bit corresponding to"
    "the LLID index to enable FEC overhead to be added to the packet length"
    "adjustment. Please note that the global FEC enable in Control register"
    "1 must also be set. For 10G upstream operation, per-LLID FEC enable is"
    "not supported; set all of these bits for FEC operation, and clear all"
    "of them for non-FEC.",
#endif
    EPN_ENABLE_UPSTREAM_FEC_REG_OFFSET,
    0,
    0,
    38,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_ENABLE_UPSTREAM_FEC_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_REPORT_BYTE_LENGTH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_REPORT_BYTE_LENGTH_FIELDS[] =
{
    &EPN_REPORT_BYTE_LENGTH_RESERVED0_FIELD,
    &EPN_REPORT_BYTE_LENGTH_PRVRPTBYTELEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_REPORT_BYTE_LENGTH_REG = 
{
    "REPORT_BYTE_LENGTH",
#if RU_INCLUDE_DESC
    "EPN_REPORT_BYTE_LENGTH Register",
    "The number of bytes of 10G upstream payload that should be reserved for"
    "a piggy-back report."
    "Note that if the \"force report\" is not set then this register is not"
    "used. Also, this value must be increased by 16-bytes for FEC-less"
    "upstream 10G mode. The extra bytes are required to compensate for the"
    "10G upstream \"scrambler sync pattern\".",
#endif
    EPN_REPORT_BYTE_LENGTH_REG_OFFSET,
    0,
    0,
    39,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_REPORT_BYTE_LENGTH_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_MAIN_INT_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_MAIN_INT_STATUS_FIELDS[] =
{
    &EPN_MAIN_INT_STATUS_INTBBHUPFRABORT_FIELD,
    &EPN_MAIN_INT_STATUS_INTCOL2SBURSTCAPOVERFLOWPRES_FIELD,
    &EPN_MAIN_INT_STATUS_INTCOEMPTYRPT_FIELD,
    &EPN_MAIN_INT_STATUS_INTCODRXERRABORTPRES_FIELD,
    &EPN_MAIN_INT_STATUS_INTL2SFIFOOVERRUN_FIELD,
    &EPN_MAIN_INT_STATUS_INTCO1588TSINT_FIELD,
    &EPN_MAIN_INT_STATUS_INTCORPTPRES_FIELD,
    &EPN_MAIN_INT_STATUS_INTCOGNTPRES_FIELD,
    &EPN_MAIN_INT_STATUS_INTCODELSTALEGNT_FIELD,
    &EPN_MAIN_INT_STATUS_INTCOGNTNONPOLL_FIELD,
    &EPN_MAIN_INT_STATUS_INTCOGNTMISALIGN_FIELD,
    &EPN_MAIN_INT_STATUS_INTCOGNTTOOFAR_FIELD,
    &EPN_MAIN_INT_STATUS_INTCOGNTINTERVAL_FIELD,
    &EPN_MAIN_INT_STATUS_INTCOGNTDISCOVERY_FIELD,
    &EPN_MAIN_INT_STATUS_INTCOGNTMISSABORT_FIELD,
    &EPN_MAIN_INT_STATUS_INTCOGNTFULLABORT_FIELD,
    &EPN_MAIN_INT_STATUS_INTBADUPFRLEN_FIELD,
    &EPN_MAIN_INT_STATUS_INTUPTARDYPACKET_FIELD,
    &EPN_MAIN_INT_STATUS_INTUPRPTFRXMT_FIELD,
    &EPN_MAIN_INT_STATUS_INTBIFIFOOVERRUN_FIELD,
    &EPN_MAIN_INT_STATUS_INTBURSTGNTTOOBIG_FIELD,
    &EPN_MAIN_INT_STATUS_INTWRGNTTOOBIG_FIELD,
    &EPN_MAIN_INT_STATUS_INTRCVGNTTOOBIG_FIELD,
    &EPN_MAIN_INT_STATUS_INTDNSTATSOVERRUN_FIELD,
    &EPN_MAIN_INT_STATUS_INTUPSTATSOVERRUN_FIELD,
    &EPN_MAIN_INT_STATUS_INTDNOUTOFORDER_FIELD,
    &EPN_MAIN_INT_STATUS_INTTRUANTBBHHALT_FIELD,
    &EPN_MAIN_INT_STATUS_INTUPINVLDGNTLEN_FIELD,
    &EPN_MAIN_INT_STATUS_INTCOBBHUPSFAULT_FIELD,
    &EPN_MAIN_INT_STATUS_INTDNTIMEINSYNC_FIELD,
    &EPN_MAIN_INT_STATUS_INTDNTIMENOTINSYNC_FIELD,
    &EPN_MAIN_INT_STATUS_INTDPORTRDY_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_MAIN_INT_STATUS_REG = 
{
    "MAIN_INT_STATUS",
#if RU_INCLUDE_DESC
    "EPN_MAIN_INT_STATUS Register",
    "This register contains interrupt status for the EPON module. These bits"
    "are sticky; to clear a bit, write 1 to it."
    ""
    "The Data Port Busy Interrupt indicates whether or not the data port is"
    "free to do another RAM access."
    ""
    "The Grant Full interrupt indicates that a grant was aborted due to its"
    "grant FIFO being full."
    ""
    "The Missed Grant interrupt indicates that a grant missed its time to"
    "transmit and was aborted. This occurs from the MPCP time having"
    "exceeded the start time when the grant is to be scheduled. (Note that"
    "grant start time is adjusted by the Grant Start Time Delta register for"
    "this calculation)"
    ""
    "The Grant Interval interrupt indicates that an LLID is not receiving"
    "gates quickly enough. If the amount of time elapsed since receiving a"
    "Gate frame exceeds a programmed value the interrupt will be asserted."
    "See the EPON LLID Grant Interval register."
    ""
    "The Discovery Gate interrupt indicates that a discovery gate was"
    "received."
    ""
    "The Local Time Not Synced interrupt is used to indicate that the ONU's"
    "local time is out of sync with EPON time. The time a MPCPDU frames"
    "arrives is compared with the value of its time stamp. If this"
    "difference is greater than value specified by the Time Stamp"
    "Differential register the interrupt will assert."
    ""
    "The Local Time Synced interrupt is used to indicate that the ONU's"
    "local time is in sync with the OLT EPON time. The time a MPCPDU frames"
    "arrives is compared with the value of its time stamp. If this"
    "difference is less than or equal to the value specified by the Time"
    "Stamp Differential register the interrupt will assert.",
#endif
    EPN_MAIN_INT_STATUS_REG_OFFSET,
    0,
    0,
    40,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    32,
    EPN_MAIN_INT_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_GNT_FULL_INT_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_GNT_FULL_INT_STATUS_FIELDS[] =
{
    &EPN_GNT_FULL_INT_STATUS_RESERVED0_FIELD,
    &EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT7_FIELD,
    &EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT6_FIELD,
    &EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT5_FIELD,
    &EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT4_FIELD,
    &EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT3_FIELD,
    &EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT2_FIELD,
    &EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT1_FIELD,
    &EPN_GNT_FULL_INT_STATUS_INTDNGNTFULLABORT0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_GNT_FULL_INT_STATUS_REG = 
{
    "GNT_FULL_INT_STATUS",
#if RU_INCLUDE_DESC
    "EPN_GNT_FULL_INT_STATUS Register",
    "This register contains interrupt status for the EPON module. These bits"
    "are sticky; to clear a bit, write 1 to it.",
#endif
    EPN_GNT_FULL_INT_STATUS_REG_OFFSET,
    0,
    0,
    41,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_GNT_FULL_INT_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_GNT_FULL_INT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_GNT_FULL_INT_MASK_FIELDS[] =
{
    &EPN_GNT_FULL_INT_MASK_RESERVED0_FIELD,
    &EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT7_FIELD,
    &EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT6_FIELD,
    &EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT5_FIELD,
    &EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT4_FIELD,
    &EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT3_FIELD,
    &EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT2_FIELD,
    &EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT1_FIELD,
    &EPN_GNT_FULL_INT_MASK_MASKINTDNGNTFULLABORT0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_GNT_FULL_INT_MASK_REG = 
{
    "GNT_FULL_INT_MASK",
#if RU_INCLUDE_DESC
    "EPN_GNT_FULL_INT_MASK Register",
    "This register contains interrupt mask for the EPON module.",
#endif
    EPN_GNT_FULL_INT_MASK_REG_OFFSET,
    0,
    0,
    42,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_GNT_FULL_INT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_GNT_MISS_INT_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_GNT_MISS_INT_STATUS_FIELDS[] =
{
    &EPN_GNT_MISS_INT_STATUS_RESERVED0_FIELD,
    &EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT7_FIELD,
    &EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT6_FIELD,
    &EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT5_FIELD,
    &EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT4_FIELD,
    &EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT3_FIELD,
    &EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT2_FIELD,
    &EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT1_FIELD,
    &EPN_GNT_MISS_INT_STATUS_INTDNGNTMISSABORT0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_GNT_MISS_INT_STATUS_REG = 
{
    "GNT_MISS_INT_STATUS",
#if RU_INCLUDE_DESC
    "EPN_GNT_MISS_INT_STATUS Register",
    "This register contains interrupt status for the EPON module. These bits"
    "are sticky; to clear a bit, write 1 to it.",
#endif
    EPN_GNT_MISS_INT_STATUS_REG_OFFSET,
    0,
    0,
    43,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_GNT_MISS_INT_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_GNT_MISS_INT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_GNT_MISS_INT_MASK_FIELDS[] =
{
    &EPN_GNT_MISS_INT_MASK_RESERVED0_FIELD,
    &EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT7_FIELD,
    &EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT6_FIELD,
    &EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT5_FIELD,
    &EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT4_FIELD,
    &EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT3_FIELD,
    &EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT2_FIELD,
    &EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT1_FIELD,
    &EPN_GNT_MISS_INT_MASK_MASKINTDNGNTMISSABORT0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_GNT_MISS_INT_MASK_REG = 
{
    "GNT_MISS_INT_MASK",
#if RU_INCLUDE_DESC
    "EPN_GNT_MISS_INT_MASK Register",
    "This register contains interrupt mask for the EPON module.",
#endif
    EPN_GNT_MISS_INT_MASK_REG_OFFSET,
    0,
    0,
    44,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_GNT_MISS_INT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DISC_RX_INT_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DISC_RX_INT_STATUS_FIELDS[] =
{
    &EPN_DISC_RX_INT_STATUS_RESERVED0_FIELD,
    &EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY7_FIELD,
    &EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY6_FIELD,
    &EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY5_FIELD,
    &EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY4_FIELD,
    &EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY3_FIELD,
    &EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY2_FIELD,
    &EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY1_FIELD,
    &EPN_DISC_RX_INT_STATUS_INTDNGNTDISCOVERY0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DISC_RX_INT_STATUS_REG = 
{
    "DISC_RX_INT_STATUS",
#if RU_INCLUDE_DESC
    "EPN_DISC_RX_INT_STATUS Register",
    "This register contains interrupt status for the EPON module. These bits"
    "are sticky; to clear a bit, write 1 to it.",
#endif
    EPN_DISC_RX_INT_STATUS_REG_OFFSET,
    0,
    0,
    45,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_DISC_RX_INT_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DISC_RX_INT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DISC_RX_INT_MASK_FIELDS[] =
{
    &EPN_DISC_RX_INT_MASK_RESERVED0_FIELD,
    &EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY7_FIELD,
    &EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY6_FIELD,
    &EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY5_FIELD,
    &EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY4_FIELD,
    &EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY3_FIELD,
    &EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY2_FIELD,
    &EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY1_FIELD,
    &EPN_DISC_RX_INT_MASK_MASKINTDNGNTDISCOVERY0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DISC_RX_INT_MASK_REG = 
{
    "DISC_RX_INT_MASK",
#if RU_INCLUDE_DESC
    "EPN_DISC_RX_INT_MASK Register",
    "This register contains interrupt mask for the EPON module.",
#endif
    EPN_DISC_RX_INT_MASK_REG_OFFSET,
    0,
    0,
    46,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_DISC_RX_INT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_GNT_INTV_INT_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_GNT_INTV_INT_STATUS_FIELDS[] =
{
    &EPN_GNT_INTV_INT_STATUS_RESERVED0_FIELD,
    &EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL7_FIELD,
    &EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL6_FIELD,
    &EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL5_FIELD,
    &EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL4_FIELD,
    &EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL3_FIELD,
    &EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL2_FIELD,
    &EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL1_FIELD,
    &EPN_GNT_INTV_INT_STATUS_INTDNGNTINTERVAL0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_GNT_INTV_INT_STATUS_REG = 
{
    "GNT_INTV_INT_STATUS",
#if RU_INCLUDE_DESC
    "EPN_GNT_INTV_INT_STATUS Register",
    "This register contains interrupt status for the EPON module. These bits"
    "are sticky; to clear a bit, write 1 to it.",
#endif
    EPN_GNT_INTV_INT_STATUS_REG_OFFSET,
    0,
    0,
    47,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_GNT_INTV_INT_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_GNT_INTV_INT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_GNT_INTV_INT_MASK_FIELDS[] =
{
    &EPN_GNT_INTV_INT_MASK_RESERVED0_FIELD,
    &EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL7_FIELD,
    &EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL6_FIELD,
    &EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL5_FIELD,
    &EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL4_FIELD,
    &EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL3_FIELD,
    &EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL2_FIELD,
    &EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL1_FIELD,
    &EPN_GNT_INTV_INT_MASK_MASKINTDNGNTINTERVAL0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_GNT_INTV_INT_MASK_REG = 
{
    "GNT_INTV_INT_MASK",
#if RU_INCLUDE_DESC
    "EPN_GNT_INTV_INT_MASK Register",
    "This register contains interrupt mask for the EPON module.",
#endif
    EPN_GNT_INTV_INT_MASK_REG_OFFSET,
    0,
    0,
    48,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_GNT_INTV_INT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_GNT_FAR_INT_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_GNT_FAR_INT_STATUS_FIELDS[] =
{
    &EPN_GNT_FAR_INT_STATUS_RESERVED0_FIELD,
    &EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR7_FIELD,
    &EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR6_FIELD,
    &EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR5_FIELD,
    &EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR4_FIELD,
    &EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR3_FIELD,
    &EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR2_FIELD,
    &EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR1_FIELD,
    &EPN_GNT_FAR_INT_STATUS_INTDNGNTTOOFAR0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_GNT_FAR_INT_STATUS_REG = 
{
    "GNT_FAR_INT_STATUS",
#if RU_INCLUDE_DESC
    "EPN_GNT_FAR_INT_STATUS Register",
    "This register contains interrupt status for the EPON module. These bits"
    "are sticky; to clear a bit, write 1 to it.",
#endif
    EPN_GNT_FAR_INT_STATUS_REG_OFFSET,
    0,
    0,
    49,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_GNT_FAR_INT_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_GNT_FAR_INT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_GNT_FAR_INT_MASK_FIELDS[] =
{
    &EPN_GNT_FAR_INT_MASK_RESERVED0_FIELD,
    &EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR7_FIELD,
    &EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR6_FIELD,
    &EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR5_FIELD,
    &EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR4_FIELD,
    &EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR3_FIELD,
    &EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR2_FIELD,
    &EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR1_FIELD,
    &EPN_GNT_FAR_INT_MASK_MASKDNGNTTOOFAR0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_GNT_FAR_INT_MASK_REG = 
{
    "GNT_FAR_INT_MASK",
#if RU_INCLUDE_DESC
    "EPN_GNT_FAR_INT_MASK Register",
    "This register contains interrupt mask for the EPON module.",
#endif
    EPN_GNT_FAR_INT_MASK_REG_OFFSET,
    0,
    0,
    50,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_GNT_FAR_INT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_GNT_MISALGN_INT_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_GNT_MISALGN_INT_STATUS_FIELDS[] =
{
    &EPN_GNT_MISALGN_INT_STATUS_RESERVED0_FIELD,
    &EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN7_FIELD,
    &EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN6_FIELD,
    &EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN5_FIELD,
    &EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN4_FIELD,
    &EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN3_FIELD,
    &EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN2_FIELD,
    &EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN1_FIELD,
    &EPN_GNT_MISALGN_INT_STATUS_INTDNGNTMISALIGN0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_GNT_MISALGN_INT_STATUS_REG = 
{
    "GNT_MISALGN_INT_STATUS",
#if RU_INCLUDE_DESC
    "EPN_GNT_MISALGN_INT_STATUS Register",
    "This register contains interrupt status for the EPON module.These bits"
    "are sticky; to clear a bit, write 1 to it.",
#endif
    EPN_GNT_MISALGN_INT_STATUS_REG_OFFSET,
    0,
    0,
    51,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_GNT_MISALGN_INT_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_GNT_MISALGN_INT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_GNT_MISALGN_INT_MASK_FIELDS[] =
{
    &EPN_GNT_MISALGN_INT_MASK_RESERVED0_FIELD,
    &EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN7_FIELD,
    &EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN6_FIELD,
    &EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN5_FIELD,
    &EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN4_FIELD,
    &EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN3_FIELD,
    &EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN2_FIELD,
    &EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN1_FIELD,
    &EPN_GNT_MISALGN_INT_MASK_MASKINTDNGNTMISALIGN0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_GNT_MISALGN_INT_MASK_REG = 
{
    "GNT_MISALGN_INT_MASK",
#if RU_INCLUDE_DESC
    "EPN_GNT_MISALGN_INT_MASK Register",
    "This register contains interrupt mask for the EPON module.",
#endif
    EPN_GNT_MISALGN_INT_MASK_REG_OFFSET,
    0,
    0,
    52,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_GNT_MISALGN_INT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_NP_GNT_INT_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_NP_GNT_INT_STATUS_FIELDS[] =
{
    &EPN_NP_GNT_INT_STATUS_RESERVED0_FIELD,
    &EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL7_FIELD,
    &EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL6_FIELD,
    &EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL5_FIELD,
    &EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL4_FIELD,
    &EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL3_FIELD,
    &EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL2_FIELD,
    &EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL1_FIELD,
    &EPN_NP_GNT_INT_STATUS_INTDNGNTNONPOLL0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_NP_GNT_INT_STATUS_REG = 
{
    "NP_GNT_INT_STATUS",
#if RU_INCLUDE_DESC
    "EPN_NP_GNT_INT_STATUS Register",
    "This register contains interrupt status for the EPON module. These bits"
    "are sticky; to clear a bit, write 1 to it.",
#endif
    EPN_NP_GNT_INT_STATUS_REG_OFFSET,
    0,
    0,
    53,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_NP_GNT_INT_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_NP_GNT_INT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_NP_GNT_INT_MASK_FIELDS[] =
{
    &EPN_NP_GNT_INT_MASK_RESERVED0_FIELD,
    &EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL7_FIELD,
    &EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL6_FIELD,
    &EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL5_FIELD,
    &EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL4_FIELD,
    &EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL3_FIELD,
    &EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL2_FIELD,
    &EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL1_FIELD,
    &EPN_NP_GNT_INT_MASK_MASKDNGNTNONPOLL0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_NP_GNT_INT_MASK_REG = 
{
    "NP_GNT_INT_MASK",
#if RU_INCLUDE_DESC
    "EPN_NP_GNT_INT_MASK Register",
    "This register contains interrupt mask for the EPON module.",
#endif
    EPN_NP_GNT_INT_MASK_REG_OFFSET,
    0,
    0,
    54,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_NP_GNT_INT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DEL_STALE_INT_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DEL_STALE_INT_STATUS_FIELDS[] =
{
    &EPN_DEL_STALE_INT_STATUS_RESERVED0_FIELD,
    &EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT7_FIELD,
    &EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT6_FIELD,
    &EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT5_FIELD,
    &EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT4_FIELD,
    &EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT3_FIELD,
    &EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT2_FIELD,
    &EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT1_FIELD,
    &EPN_DEL_STALE_INT_STATUS_INTDELSTALEGNT0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DEL_STALE_INT_STATUS_REG = 
{
    "DEL_STALE_INT_STATUS",
#if RU_INCLUDE_DESC
    "EPN_DEL_STALE_INT_STATUS Register",
    "This register contains interrupt status for the EPON module. These bits"
    "are sticky; to clear a bit, write 1 to it.",
#endif
    EPN_DEL_STALE_INT_STATUS_REG_OFFSET,
    0,
    0,
    55,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_DEL_STALE_INT_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DEL_STALE_INT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DEL_STALE_INT_MASK_FIELDS[] =
{
    &EPN_DEL_STALE_INT_MASK_RESERVED0_FIELD,
    &EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT7_FIELD,
    &EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT6_FIELD,
    &EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT5_FIELD,
    &EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT4_FIELD,
    &EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT3_FIELD,
    &EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT2_FIELD,
    &EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT1_FIELD,
    &EPN_DEL_STALE_INT_MASK_MASKINTDELSTALEGNT0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DEL_STALE_INT_MASK_REG = 
{
    "DEL_STALE_INT_MASK",
#if RU_INCLUDE_DESC
    "EPN_DEL_STALE_INT_MASK Register",
    "Interrupt mask for EPN_DEL_STALE_INT_STATUS",
#endif
    EPN_DEL_STALE_INT_MASK_REG_OFFSET,
    0,
    0,
    56,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_DEL_STALE_INT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_GNT_PRES_INT_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_GNT_PRES_INT_STATUS_FIELDS[] =
{
    &EPN_GNT_PRES_INT_STATUS_RESERVED0_FIELD,
    &EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY7_FIELD,
    &EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY6_FIELD,
    &EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY5_FIELD,
    &EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY4_FIELD,
    &EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY3_FIELD,
    &EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY2_FIELD,
    &EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY1_FIELD,
    &EPN_GNT_PRES_INT_STATUS_INTDNGNTRDY0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_GNT_PRES_INT_STATUS_REG = 
{
    "GNT_PRES_INT_STATUS",
#if RU_INCLUDE_DESC
    "EPN_GNT_PRES_INT_STATUS Register",
    "This register contains interrupt status for the EPON module. These bits"
    "are sticky; to clear a bit, write a 1 to it.",
#endif
    EPN_GNT_PRES_INT_STATUS_REG_OFFSET,
    0,
    0,
    57,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_GNT_PRES_INT_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_GNT_PRES_INT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_GNT_PRES_INT_MASK_FIELDS[] =
{
    &EPN_GNT_PRES_INT_MASK_RESERVED0_FIELD,
    &EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY7_FIELD,
    &EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY6_FIELD,
    &EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY5_FIELD,
    &EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY4_FIELD,
    &EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY3_FIELD,
    &EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY2_FIELD,
    &EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY1_FIELD,
    &EPN_GNT_PRES_INT_MASK_MASKDNGNTRDY0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_GNT_PRES_INT_MASK_REG = 
{
    "GNT_PRES_INT_MASK",
#if RU_INCLUDE_DESC
    "EPN_GNT_PRES_INT_MASK Register",
    "This register contains interrupt mask for the EPON module.",
#endif
    EPN_GNT_PRES_INT_MASK_REG_OFFSET,
    0,
    0,
    58,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_GNT_PRES_INT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_RPT_PRES_INT_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_RPT_PRES_INT_STATUS_FIELDS[] =
{
    &EPN_RPT_PRES_INT_STATUS_RESERVED0_FIELD,
    &EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO7_FIELD,
    &EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO6_FIELD,
    &EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO5_FIELD,
    &EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO4_FIELD,
    &EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO3_FIELD,
    &EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO2_FIELD,
    &EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO1_FIELD,
    &EPN_RPT_PRES_INT_STATUS_INTUPRPTFIFO0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_RPT_PRES_INT_STATUS_REG = 
{
    "RPT_PRES_INT_STATUS",
#if RU_INCLUDE_DESC
    "EPN_RPT_PRES_INT_STATUS Register",
    "This register contains interrupt status for the EPON module. These bits"
    "are sticky; to clear a bit, write 1 to it.",
#endif
    EPN_RPT_PRES_INT_STATUS_REG_OFFSET,
    0,
    0,
    59,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_RPT_PRES_INT_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_RPT_PRES_INT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_RPT_PRES_INT_MASK_FIELDS[] =
{
    &EPN_RPT_PRES_INT_MASK_RESERVED0_FIELD,
    &EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO7_FIELD,
    &EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO6_FIELD,
    &EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO5_FIELD,
    &EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO4_FIELD,
    &EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO3_FIELD,
    &EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO2_FIELD,
    &EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO1_FIELD,
    &EPN_RPT_PRES_INT_MASK_MASKINTUPRPTFIFO0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_RPT_PRES_INT_MASK_REG = 
{
    "RPT_PRES_INT_MASK",
#if RU_INCLUDE_DESC
    "EPN_RPT_PRES_INT_MASK Register",
    "This register contains interrupt mask for the EPON module.",
#endif
    EPN_RPT_PRES_INT_MASK_REG_OFFSET,
    0,
    0,
    60,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_RPT_PRES_INT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DRX_ABORT_INT_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DRX_ABORT_INT_STATUS_FIELDS[] =
{
    &EPN_DRX_ABORT_INT_STATUS_INTDRXERRABORT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DRX_ABORT_INT_STATUS_REG = 
{
    "DRX_ABORT_INT_STATUS",
#if RU_INCLUDE_DESC
    "EPN_DRX_ABORT_INT_STATUS Register",
    "This register contains interrupt status for the Drx error abort events."
    "These bits are sticky; to clear a bit, write 1 to it.",
#endif
    EPN_DRX_ABORT_INT_STATUS_REG_OFFSET,
    0,
    0,
    61,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPN_DRX_ABORT_INT_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DRX_ABORT_INT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DRX_ABORT_INT_MASK_FIELDS[] =
{
    &EPN_DRX_ABORT_INT_MASK_MASKINTDRXERRABORT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DRX_ABORT_INT_MASK_REG = 
{
    "DRX_ABORT_INT_MASK",
#if RU_INCLUDE_DESC
    "EPN_DRX_ABORT_INT_MASK Register",
    "This register contains interrupt mask for the Drx error abort events.",
#endif
    EPN_DRX_ABORT_INT_MASK_REG_OFFSET,
    0,
    0,
    62,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPN_DRX_ABORT_INT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_EMPTY_RPT_INT_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_EMPTY_RPT_INT_STATUS_FIELDS[] =
{
    &EPN_EMPTY_RPT_INT_STATUS_RESERVED0_FIELD,
    &EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT7_FIELD,
    &EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT6_FIELD,
    &EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT5_FIELD,
    &EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT4_FIELD,
    &EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT3_FIELD,
    &EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT2_FIELD,
    &EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT1_FIELD,
    &EPN_EMPTY_RPT_INT_STATUS_INTEMPTYRPT0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_EMPTY_RPT_INT_STATUS_REG = 
{
    "EMPTY_RPT_INT_STATUS",
#if RU_INCLUDE_DESC
    "EPN_EMPTY_RPT_INT_STATUS Register",
    "This register contains interrupt status for the EPON module's empty"
    "report transmission. Any time the EPON module sends a report upstream"
    "and all the report values are zero, the bit corresponding to the LLID"
    "index will be set. These bits are sticky; to clear a bit, write 1 to"
    "it.",
#endif
    EPN_EMPTY_RPT_INT_STATUS_REG_OFFSET,
    0,
    0,
    63,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_EMPTY_RPT_INT_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_EMPTY_RPT_INT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_EMPTY_RPT_INT_MASK_FIELDS[] =
{
    &EPN_EMPTY_RPT_INT_MASK_RESERVED0_FIELD,
    &EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT7_FIELD,
    &EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT6_FIELD,
    &EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT5_FIELD,
    &EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT4_FIELD,
    &EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT3_FIELD,
    &EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT2_FIELD,
    &EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT1_FIELD,
    &EPN_EMPTY_RPT_INT_MASK_MASKINTEMPTYRPT0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_EMPTY_RPT_INT_MASK_REG = 
{
    "EMPTY_RPT_INT_MASK",
#if RU_INCLUDE_DESC
    "EPN_EMPTY_RPT_INT_MASK Register",
    "This register contains interrupt mask for the EPON module's empty"
    "report transmission. Any time the EPON module sends a report upstream"
    "and all the report values are zero, the bit corresponding to the LLID"
    "index will be set.",
#endif
    EPN_EMPTY_RPT_INT_MASK_REG_OFFSET,
    0,
    0,
    64,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_EMPTY_RPT_INT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_BCAP_OVERFLOW_INT_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_BCAP_OVERFLOW_INT_STATUS_FIELDS[] =
{
    &EPN_BCAP_OVERFLOW_INT_STATUS_RESERVED0_FIELD,
    &EPN_BCAP_OVERFLOW_INT_STATUS_INTL2SBURSTCAPOVERFLOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_BCAP_OVERFLOW_INT_STATUS_REG = 
{
    "BCAP_OVERFLOW_INT_STATUS",
#if RU_INCLUDE_DESC
    "EPN_BCAP_OVERFLOW_INT_STATUS Register",
    "This register contains interrupt status indicating when the L2"
    "accumulators exceed their burst-cap values. These bits are sticky; to"
    "clear a bit, write 1 to it.",
#endif
    EPN_BCAP_OVERFLOW_INT_STATUS_REG_OFFSET,
    0,
    0,
    65,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_BCAP_OVERFLOW_INT_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_BCAP_OVERFLOW_INT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_BCAP_OVERFLOW_INT_MASK_FIELDS[] =
{
    &EPN_BCAP_OVERFLOW_INT_MASK_RESERVED0_FIELD,
    &EPN_BCAP_OVERFLOW_INT_MASK_MASKINTL2SBURSTCAPOVERFLOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_BCAP_OVERFLOW_INT_MASK_REG = 
{
    "BCAP_OVERFLOW_INT_MASK",
#if RU_INCLUDE_DESC
    "EPN_BCAP_OVERFLOW_INT_MASK Register",
    "This register contains interrupt mask indicating when the L2"
    "accumulators exceed their burst-cap values.",
#endif
    EPN_BCAP_OVERFLOW_INT_MASK_REG_OFFSET,
    0,
    0,
    66,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_BCAP_OVERFLOW_INT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_BBH_DNS_FAULT_INT_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_BBH_DNS_FAULT_INT_STATUS_FIELDS[] =
{
    &EPN_BBH_DNS_FAULT_INT_STATUS_RESERVED0_FIELD,
    &EPN_BBH_DNS_FAULT_INT_STATUS_INTBBHDNSOVERFLOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_BBH_DNS_FAULT_INT_STATUS_REG = 
{
    "BBH_DNS_FAULT_INT_STATUS",
#if RU_INCLUDE_DESC
    "EPN_BBH_DNS_FAULT_INT_STATUS Register",
    "",
#endif
    EPN_BBH_DNS_FAULT_INT_STATUS_REG_OFFSET,
    0,
    0,
    67,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_BBH_DNS_FAULT_INT_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_BBH_DNS_FAULT_INT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_BBH_DNS_FAULT_INT_MASK_FIELDS[] =
{
    &EPN_BBH_DNS_FAULT_INT_MASK_RESERVED0_FIELD,
    &EPN_BBH_DNS_FAULT_INT_MASK_MASKINTBBHDNSOVERFLOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_BBH_DNS_FAULT_INT_MASK_REG = 
{
    "BBH_DNS_FAULT_INT_MASK",
#if RU_INCLUDE_DESC
    "EPN_BBH_DNS_FAULT_INT_MASK Register",
    "",
#endif
    EPN_BBH_DNS_FAULT_INT_MASK_REG_OFFSET,
    0,
    0,
    68,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_BBH_DNS_FAULT_INT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_BBH_UPS_FAULT_INT_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_BBH_UPS_FAULT_INT_STATUS_FIELDS[] =
{
    &EPN_BBH_UPS_FAULT_INT_STATUS_RESERVED0_FIELD,
    &EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT7_FIELD,
    &EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT6_FIELD,
    &EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT5_FIELD,
    &EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT4_FIELD,
    &EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT3_FIELD,
    &EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT2_FIELD,
    &EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT1_FIELD,
    &EPN_BBH_UPS_FAULT_INT_STATUS_INTBBHUPSFAULT0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_BBH_UPS_FAULT_INT_STATUS_REG = 
{
    "BBH_UPS_FAULT_INT_STATUS",
#if RU_INCLUDE_DESC
    "EPN_BBH_UPS_FAULT_INT_STATUS Register",
    "",
#endif
    EPN_BBH_UPS_FAULT_INT_STATUS_REG_OFFSET,
    0,
    0,
    69,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_BBH_UPS_FAULT_INT_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_BBH_UPS_FAULT_INT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_BBH_UPS_FAULT_INT_MASK_FIELDS[] =
{
    &EPN_BBH_UPS_FAULT_INT_MASK_RESERVED0_FIELD,
    &EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT7_FIELD,
    &EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT6_FIELD,
    &EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT5_FIELD,
    &EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT4_FIELD,
    &EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT3_FIELD,
    &EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT2_FIELD,
    &EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT1_FIELD,
    &EPN_BBH_UPS_FAULT_INT_MASK_MASKINTBBHUPSFAULT0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_BBH_UPS_FAULT_INT_MASK_REG = 
{
    "BBH_UPS_FAULT_INT_MASK",
#if RU_INCLUDE_DESC
    "EPN_BBH_UPS_FAULT_INT_MASK Register",
    "",
#endif
    EPN_BBH_UPS_FAULT_INT_MASK_REG_OFFSET,
    0,
    0,
    70,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_BBH_UPS_FAULT_INT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_BBH_UPS_ABORT_INT_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_BBH_UPS_ABORT_INT_STATUS_FIELDS[] =
{
    &EPN_BBH_UPS_ABORT_INT_STATUS_RESERVED0_FIELD,
    &EPN_BBH_UPS_ABORT_INT_STATUS_TARDYBBHABORT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_BBH_UPS_ABORT_INT_STATUS_REG = 
{
    "BBH_UPS_ABORT_INT_STATUS",
#if RU_INCLUDE_DESC
    "EPN_BBH_UPS_ABORT_INT_STATUS Register",
    "",
#endif
    EPN_BBH_UPS_ABORT_INT_STATUS_REG_OFFSET,
    0,
    0,
    71,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_BBH_UPS_ABORT_INT_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_BBH_UPS_ABORT_INT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_BBH_UPS_ABORT_INT_MASK_FIELDS[] =
{
    &EPN_BBH_UPS_ABORT_INT_MASK_RESERVED0_FIELD,
    &EPN_BBH_UPS_ABORT_INT_MASK_MASKTARDYBBHABORT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_BBH_UPS_ABORT_INT_MASK_REG = 
{
    "BBH_UPS_ABORT_INT_MASK",
#if RU_INCLUDE_DESC
    "EPN_BBH_UPS_ABORT_INT_MASK Register",
    "",
#endif
    EPN_BBH_UPS_ABORT_INT_MASK_REG_OFFSET,
    0,
    0,
    72,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_BBH_UPS_ABORT_INT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_MAIN_INT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_MAIN_INT_MASK_FIELDS[] =
{
    &EPN_MAIN_INT_MASK_BBHUPFRABORTMASK_FIELD,
    &EPN_MAIN_INT_MASK_INTL2SBURSTCAPOVERFLOWMASK_FIELD,
    &EPN_MAIN_INT_MASK_INTCOEMPTYRPTMASK_FIELD,
    &EPN_MAIN_INT_MASK_INTDRXERRABORTMASK_FIELD,
    &EPN_MAIN_INT_MASK_INTL2SFIFOOVERRUNMASK_FIELD,
    &EPN_MAIN_INT_MASK_INTCO1588TSMASK_FIELD,
    &EPN_MAIN_INT_MASK_INTCORPTPRESMASK_FIELD,
    &EPN_MAIN_INT_MASK_INTCOGNTPRESMASK_FIELD,
    &EPN_MAIN_INT_MASK_INTCODELSTALEGNTMASK_FIELD,
    &EPN_MAIN_INT_MASK_INTCOGNTNONPOLLMASK_FIELD,
    &EPN_MAIN_INT_MASK_INTCOGNTMISALIGNMASK_FIELD,
    &EPN_MAIN_INT_MASK_INTCOGNTTOOFARMASK_FIELD,
    &EPN_MAIN_INT_MASK_INTCOGNTINTERVALMASK_FIELD,
    &EPN_MAIN_INT_MASK_INTCOGNTDISCOVERYMASK_FIELD,
    &EPN_MAIN_INT_MASK_INTCOGNTMISSABORTMASK_FIELD,
    &EPN_MAIN_INT_MASK_INTCOGNTFULLABORTMASK_FIELD,
    &EPN_MAIN_INT_MASK_BADUPFRLENMASK_FIELD,
    &EPN_MAIN_INT_MASK_UPTARDYPACKETMASK_FIELD,
    &EPN_MAIN_INT_MASK_UPRPTFRXMTMASK_FIELD,
    &EPN_MAIN_INT_MASK_INTBIFIFOOVERRUNMASK_FIELD,
    &EPN_MAIN_INT_MASK_BURSTGNTTOOBIGMASK_FIELD,
    &EPN_MAIN_INT_MASK_WRGNTTOOBIGMASK_FIELD,
    &EPN_MAIN_INT_MASK_RCVGNTTOOBIGMASK_FIELD,
    &EPN_MAIN_INT_MASK_DNSTATSOVERRUNMASK_FIELD,
    &EPN_MAIN_INT_MASK_INTUPSTATSOVERRUNMASK_FIELD,
    &EPN_MAIN_INT_MASK_DNOUTOFORDERMASK_FIELD,
    &EPN_MAIN_INT_MASK_TRUANTBBHHALTMASK_FIELD,
    &EPN_MAIN_INT_MASK_UPINVLDGNTLENMASK_FIELD,
    &EPN_MAIN_INT_MASK_INTCOBBHUPSFAULTMASK_FIELD,
    &EPN_MAIN_INT_MASK_DNTIMEINSYNCMASK_FIELD,
    &EPN_MAIN_INT_MASK_DNTIMENOTINSYNCMASK_FIELD,
    &EPN_MAIN_INT_MASK_DPORTRDYMASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_MAIN_INT_MASK_REG = 
{
    "MAIN_INT_MASK",
#if RU_INCLUDE_DESC
    "EPN_MAIN_INT_MASK Register",
    "",
#endif
    EPN_MAIN_INT_MASK_REG_OFFSET,
    0,
    0,
    73,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    32,
    EPN_MAIN_INT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_MAX_GNT_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_MAX_GNT_SIZE_FIELDS[] =
{
    &EPN_MAX_GNT_SIZE_RESERVED0_FIELD,
    &EPN_MAX_GNT_SIZE_MAXGNTSIZE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_MAX_GNT_SIZE_REG = 
{
    "MAX_GNT_SIZE",
#if RU_INCLUDE_DESC
    "EPN_MAX_GNT_SIZE Register",
    "The Maximum Grant Size register sets the threshold for the three Grant"
    "Too Big interrupts (in the EPN Main Interrupt Status register).",
#endif
    EPN_MAX_GNT_SIZE_REG_OFFSET,
    0,
    0,
    74,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_MAX_GNT_SIZE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_MAX_FRAME_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_MAX_FRAME_SIZE_FIELDS[] =
{
    &EPN_MAX_FRAME_SIZE_RESERVED0_FIELD,
    &EPN_MAX_FRAME_SIZE_CFGMAXFRAMESIZE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_MAX_FRAME_SIZE_REG = 
{
    "MAX_FRAME_SIZE",
#if RU_INCLUDE_DESC
    "EPN_MAX_FRAME_SIZE Register",
    "Provisions the maximum allowable downstream frame size. The reset"
    "default is 1536. This register is overridden by the cfgVlanMaxSize bit."
    "The maximum allowable value for this register is 2000 in 1G/2G mode.",
#endif
    EPN_MAX_FRAME_SIZE_REG_OFFSET,
    0,
    0,
    75,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_MAX_FRAME_SIZE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_GRANT_OVR_HD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_GRANT_OVR_HD_FIELDS[] =
{
    &EPN_GRANT_OVR_HD_GNTOVRHDFEC_FIELD,
    &EPN_GRANT_OVR_HD_GNTOVRHD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_GRANT_OVR_HD_REG = 
{
    "GRANT_OVR_HD",
#if RU_INCLUDE_DESC
    "EPN_GRANT_OVR_HD Register",
    "Defines how much of the grant length is consumed by laser on time,"
    "laser off time, and idle (sync) time. This value is subtracted from the"
    "grant length and the remainder is used to fill frames from FIF queues"
    "into the upstream burst. This register is used in both 1G and 10G"
    "upstream modes. The units are in TQ. Reset default is 0."
    "NOTE: In 10G mode the Xif requires 2 extra TimeQuanta for \"Eob\". Also,"
    "in FECless 10G mode the Epn's \"Report Byte Length\" must have an extra"
    "16-bytes added to its value to account for 10G \"scrambler sync time\".",
#endif
    EPN_GRANT_OVR_HD_REG_OFFSET,
    0,
    0,
    76,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_GRANT_OVR_HD_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_POLL_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_POLL_SIZE_FIELDS[] =
{
    &EPN_POLL_SIZE_POLLSIZEFEC_FIELD,
    &EPN_POLL_SIZE_POLLSIZE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_POLL_SIZE_REG = 
{
    "POLL_SIZE",
#if RU_INCLUDE_DESC
    "EPN_POLL_SIZE Register",
    "Sets the size of polling grants for the purpose of generating the"
    "dnGntNonPoll interrupts. If a received grant's length, less EPN Grant"
    "Length Overhead, is less than or equal to the poll size, the grant is"
    "considered a poll and resets the poll grant interval timer for the LLID"
    "Index. Reset default is 64 decimal.",
#endif
    EPN_POLL_SIZE_REG_OFFSET,
    0,
    0,
    77,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_POLL_SIZE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DN_RD_GNT_MARGIN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DN_RD_GNT_MARGIN_FIELDS[] =
{
    &EPN_DN_RD_GNT_MARGIN_RESERVED0_FIELD,
    &EPN_DN_RD_GNT_MARGIN_RDGNTSTARTMARGIN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DN_RD_GNT_MARGIN_REG = 
{
    "DN_RD_GNT_MARGIN",
#if RU_INCLUDE_DESC
    "EPN_DN_RD_GNT_MARGIN Register",
    "This register determines how far in advance of the Grant Start Time"
    "that grants are considered for removal from the DRX Grant FIFO. Once a"
    "grant is chosen (the various LLID Indexes compete for the next burst"
    "slot - the Index with a grant that is within Read Grant Margin and"
    "closest to its Grant Start Time wins), it is popped from its grant FIFO"
    "and held until it meets the Grant Start Time Delta criteria (see"
    "below)."
    "The units of this register are TQ. The reset default value is 256"
    "decimal.",
#endif
    EPN_DN_RD_GNT_MARGIN_REG_OFFSET,
    0,
    0,
    78,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_DN_RD_GNT_MARGIN_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_GNT_TIME_START_DELTA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_GNT_TIME_START_DELTA_FIELDS[] =
{
    &EPN_GNT_TIME_START_DELTA_RESERVED0_FIELD,
    &EPN_GNT_TIME_START_DELTA_GNTSTARTTIMEDELTA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_GNT_TIME_START_DELTA_REG = 
{
    "GNT_TIME_START_DELTA",
#if RU_INCLUDE_DESC
    "EPN_GNT_TIME_START_DELTA Register",
    "This value determines how far in advance of the Grant Start Time that"
    "the next selected grant (already extracted from the Grant FIFO) will be"
    "handed to the EPN UTX (upstream transmit) logic and start to pre-fetch"
    "frames.."
    "The units of this register are TQ. The reset default value is 640"
    "decimal.",
#endif
    EPN_GNT_TIME_START_DELTA_REG_OFFSET,
    0,
    0,
    79,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_GNT_TIME_START_DELTA_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_TIME_STAMP_DIFF
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_TIME_STAMP_DIFF_FIELDS[] =
{
    &EPN_TIME_STAMP_DIFF_RESERVED0_FIELD,
    &EPN_TIME_STAMP_DIFF_TIMESTAMPDIFFDELTA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_TIME_STAMP_DIFF_REG = 
{
    "TIME_STAMP_DIFF",
#if RU_INCLUDE_DESC
    "EPN_TIME_STAMP_DIFF Register",
    "This register sets a threshold for LocalTimeInSync/LocalTimeNotSync"
    "interrupts, and for local time reference updates. When the difference"
    "between the EPON MAC's local time and an MPCPDU's timestamp exceeds"
    "this value the LocalTimeNotSync interrupt is asserted. The units of"
    "this register are TQ. Reset default is 10 decimal.",
#endif
    EPN_TIME_STAMP_DIFF_REG_OFFSET,
    0,
    0,
    80,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_TIME_STAMP_DIFF_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_UP_TIME_STAMP_OFF
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_UP_TIME_STAMP_OFF_FIELDS[] =
{
    &EPN_UP_TIME_STAMP_OFF_TIMESTAMPOFFSETFEC_FIELD,
    &EPN_UP_TIME_STAMP_OFF_TIMESTAMPOFFSET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_UP_TIME_STAMP_OFF_REG = 
{
    "UP_TIME_STAMP_OFF",
#if RU_INCLUDE_DESC
    "EPN_UP_TIME_STAMP_OFF Register",
    "This register helps determine the value for the Timestamp field"
    "inserted into REPORT and Processor frames. This value specifies an"
    "offset from the Grant Start Time of the upstream burst. The value"
    "programmed here will be roughly equivalent to the sum of Laser-On time"
    "plus IDLE time plus Preamble time. The goal is for the Timestamp"
    "inserted into a frame to match the MPCP time at which the first byte of"
    "the frame's Destination Address is transmitted.",
#endif
    EPN_UP_TIME_STAMP_OFF_REG_OFFSET,
    0,
    0,
    81,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_UP_TIME_STAMP_OFF_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_GNT_INTERVAL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_GNT_INTERVAL_FIELDS[] =
{
    &EPN_GNT_INTERVAL_RESERVED0_FIELD,
    &EPN_GNT_INTERVAL_GNTINTERVAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_GNT_INTERVAL_REG = 
{
    "GNT_INTERVAL",
#if RU_INCLUDE_DESC
    "EPN_GNT_INTERVAL Register",
    "This register specifies the maximum allowed time between GATE messages"
    "received on an LLID. If the time elapsed is greater than the specified"
    "value, the Gate Interval interrupt asserts for that LLID Index. The"
    "units of this register are 262 us. The maximum interval is ~17 seconds.",
#endif
    EPN_GNT_INTERVAL_REG_OFFSET,
    0,
    0,
    82,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_GNT_INTERVAL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DN_GNT_MISALIGN_THR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DN_GNT_MISALIGN_THR_FIELDS[] =
{
    &EPN_DN_GNT_MISALIGN_THR_PRVUNUSEDGNTTHRESHOLD_FIELD,
    &EPN_DN_GNT_MISALIGN_THR_RESERVED0_FIELD,
    &EPN_DN_GNT_MISALIGN_THR_GNTMISALIGNTHRESH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DN_GNT_MISALIGN_THR_REG = 
{
    "DN_GNT_MISALIGN_THR",
#if RU_INCLUDE_DESC
    "EPN_DN_GNT_MISALIGN_THR Register",
    "Sets the threshold for misalignment detection and handling."
    "A grant misalignment condition is detected by the ONU whenever a grant"
    "to the ONU cannot be used efficiently (due to the grant size not"
    "aligning to even frame boundaries)."
    "When cfgGntMisalignX bits are set, the ONU uses this register to"
    "determine a misalignment condition and to take corrective action."
    "prvUnusedGntThresh determines how many unused TQ there must be in a"
    "given grant in order for it to be considered misaligned."
    "gntMisalignThresh indicates how many consecutive misaligned grants must"
    "be received in order to trigger handling of this condition."
    "The ONU handles the misaligned condition by temporarily reporting 0"
    "(\"no data\") in that LLID's REPORT frames.",
#endif
    EPN_DN_GNT_MISALIGN_THR_REG_OFFSET,
    0,
    0,
    83,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    EPN_DN_GNT_MISALIGN_THR_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DN_GNT_MISALIGN_PAUSE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DN_GNT_MISALIGN_PAUSE_FIELDS[] =
{
    &EPN_DN_GNT_MISALIGN_PAUSE_RESERVED0_FIELD,
    &EPN_DN_GNT_MISALIGN_PAUSE_GNTMISALIGNPAUSE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DN_GNT_MISALIGN_PAUSE_REG = 
{
    "DN_GNT_MISALIGN_PAUSE",
#if RU_INCLUDE_DESC
    "EPN_DN_GNT_MISALIGN_PAUSE Register",
    "Indicates for how long after the misalignment condition is detected"
    "that the LLID Index's reporting will be \"paused\". This is achieved"
    "through reporting queue report values of zero."
    "Units are 1 us.",
#endif
    EPN_DN_GNT_MISALIGN_PAUSE_REG_OFFSET,
    0,
    0,
    84,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_DN_GNT_MISALIGN_PAUSE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_NON_POLL_INTV
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_NON_POLL_INTV_FIELDS[] =
{
    &EPN_NON_POLL_INTV_RESERVED0_FIELD,
    &EPN_NON_POLL_INTV_NONPOLLGNTINTV_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_NON_POLL_INTV_REG = 
{
    "NON_POLL_INTV",
#if RU_INCLUDE_DESC
    "EPN_NON_POLL_INTV Register",
    "Defines the amount of time required for triggering the non poll grant"
    "interrupts.",
#endif
    EPN_NON_POLL_INTV_REG_OFFSET,
    0,
    0,
    85,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_NON_POLL_INTV_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_FORCE_FCS_ERR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_FORCE_FCS_ERR_FIELDS[] =
{
    &EPN_FORCE_FCS_ERR_RESERVED0_FIELD,
    &EPN_FORCE_FCS_ERR_FORCEFCSERR7_FIELD,
    &EPN_FORCE_FCS_ERR_FORCEFCSERR6_FIELD,
    &EPN_FORCE_FCS_ERR_FORCEFCSERR5_FIELD,
    &EPN_FORCE_FCS_ERR_FORCEFCSERR4_FIELD,
    &EPN_FORCE_FCS_ERR_FORCEFCSERR3_FIELD,
    &EPN_FORCE_FCS_ERR_FORCEFCSERR2_FIELD,
    &EPN_FORCE_FCS_ERR_FORCEFCSERR1_FIELD,
    &EPN_FORCE_FCS_ERR_FORCEFCSERR0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_FORCE_FCS_ERR_REG = 
{
    "FORCE_FCS_ERR",
#if RU_INCLUDE_DESC
    "EPN_FORCE_FCS_ERR Register",
    "Forces upstream FCS errors on the selected LLID(s).",
#endif
    EPN_FORCE_FCS_ERR_REG_OFFSET,
    0,
    0,
    86,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_FORCE_FCS_ERR_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_GRANT_OVERLAP_LIMIT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_GRANT_OVERLAP_LIMIT_FIELDS[] =
{
    &EPN_GRANT_OVERLAP_LIMIT_RESERVED0_FIELD,
    &EPN_GRANT_OVERLAP_LIMIT_PRVGRANTOVERLAPLIMIT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_GRANT_OVERLAP_LIMIT_REG = 
{
    "GRANT_OVERLAP_LIMIT",
#if RU_INCLUDE_DESC
    "EPN_GRANT_OVERLAP_LIMIT Register",
    "Defines how much overlap is allowed between consecutive grants to the"
    "same ONU. Effectively this register defines how much of the grant"
    "overhead the Epn should attempt to recover when processing overlapped"
    "grants. Normally this register is provisioned with the same value as in"
    "the EPN Grant Overhead Length register. However, if the Xif or Lif"
    "require extra overhead not associated with Lon/Sync/Loff, then this"
    "register value must be smaller than the EPN Grant Overhead Length"
    "register value."
    "This register is used in both 1G and 10G upstream modes. The units are"
    "in TQ. Reset default is 0."
    "NOTE: In 10G upstream mode this register must be provisioned to match"
    "the value written to the XIF Overlapping Grant Overhead register"
    "(0x0364). The proper value is calculated as:"
    "EPN Grant Overlap Limit = Lon + Sync Time + Loff - 1 (start of burst) -"
    "1 (Idle Sync) - 3 (XIF Overlapping Grant Overhead).",
#endif
    EPN_GRANT_OVERLAP_LIMIT_REG_OFFSET,
    0,
    0,
    87,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_GRANT_OVERLAP_LIMIT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_AES_CONFIGURATION_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_AES_CONFIGURATION_0_FIELDS[] =
{
    &EPN_AES_CONFIGURATION_0_RESERVED0_FIELD,
    &EPN_AES_CONFIGURATION_0_PRVUPSTREAMAESMODE_0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_AES_CONFIGURATION_0_REG = 
{
    "AES_CONFIGURATION_0",
#if RU_INCLUDE_DESC
    "EPN_AES_CONFIGURATION_0 Register",
    "Allows control over reporting the extra per-packet overhead associated"
    "with 802.1AE encryption. The AES overhead compensation logic supports"
    "two overhead modes: implicit SCI and explicit SCI. The implicit SCI"
    "mode increases the per-packet overhead by 24 bytes. Explicit SCI mode"
    "increases the per-packet overhead by 32 bytes.",
#endif
    EPN_AES_CONFIGURATION_0_REG_OFFSET,
    0,
    0,
    88,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_AES_CONFIGURATION_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DISC_GRANT_OVR_HD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DISC_GRANT_OVR_HD_FIELDS[] =
{
    &EPN_DISC_GRANT_OVR_HD_RESERVED0_FIELD,
    &EPN_DISC_GRANT_OVR_HD_DISCGNTOVRHD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DISC_GRANT_OVR_HD_REG = 
{
    "DISC_GRANT_OVR_HD",
#if RU_INCLUDE_DESC
    "EPN_DISC_GRANT_OVR_HD Register",
    "The amount of overhead used in discovery gates.",
#endif
    EPN_DISC_GRANT_OVR_HD_REG_OFFSET,
    0,
    0,
    89,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_DISC_GRANT_OVR_HD_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DN_DISCOVERY_SEED
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DN_DISCOVERY_SEED_FIELDS[] =
{
    &EPN_DN_DISCOVERY_SEED_RESERVED0_FIELD,
    &EPN_DN_DISCOVERY_SEED_CFGDISCSEED_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DN_DISCOVERY_SEED_REG = 
{
    "DN_DISCOVERY_SEED",
#if RU_INCLUDE_DESC
    "EPN_DN_DISCOVERY_SEED Register",
    "Serves as the seed for generating the random offset during discovery"
    "gates. When this register is written the discovery random offset become"
    "this value",
#endif
    EPN_DN_DISCOVERY_SEED_REG_OFFSET,
    0,
    0,
    90,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_DN_DISCOVERY_SEED_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DN_DISCOVERY_INC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DN_DISCOVERY_INC_FIELDS[] =
{
    &EPN_DN_DISCOVERY_INC_RESERVED0_FIELD,
    &EPN_DN_DISCOVERY_INC_CFGDISCINC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DN_DISCOVERY_INC_REG = 
{
    "DN_DISCOVERY_INC",
#if RU_INCLUDE_DESC
    "EPN_DN_DISCOVERY_INC Register",
    "Sets the amount by which the discovery random offset is incremented.",
#endif
    EPN_DN_DISCOVERY_INC_REG_OFFSET,
    0,
    0,
    91,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_DN_DISCOVERY_INC_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DN_DISCOVERY_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DN_DISCOVERY_SIZE_FIELDS[] =
{
    &EPN_DN_DISCOVERY_SIZE_RESERVED0_FIELD,
    &EPN_DN_DISCOVERY_SIZE_CFGDISCSIZE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DN_DISCOVERY_SIZE_REG = 
{
    "DN_DISCOVERY_SIZE",
#if RU_INCLUDE_DESC
    "EPN_DN_DISCOVERY_SIZE Register",
    "Size of the grant for responses to Discovery Gates. When a Discovery"
    "Gate is received from the PON, EPN substitutes this value into the"
    "grant length as the grant goes into the Grant FIFO. Normally, the value"
    "set in this register will be 42 greater than what is set in EPN"
    "Discovery Gate Overhead.",
#endif
    EPN_DN_DISCOVERY_SIZE_REG_OFFSET,
    0,
    0,
    92,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_DN_DISCOVERY_SIZE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_FEC_IPG_LENGTH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_FEC_IPG_LENGTH_FIELDS[] =
{
    &EPN_FEC_IPG_LENGTH_RESERVED0_FIELD,
    &EPN_FEC_IPG_LENGTH_MODIPGPREAMBLEBYTES_FIELD,
    &EPN_FEC_IPG_LENGTH_CFGRPTLEN_FIELD,
    &EPN_FEC_IPG_LENGTH_CFGFECRPTLENGTH_FIELD,
    &EPN_FEC_IPG_LENGTH_CFGFECIPGLENGTH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_FEC_IPG_LENGTH_REG = 
{
    "FEC_IPG_LENGTH",
#if RU_INCLUDE_DESC
    "EPN_FEC_IPG_LENGTH Register",
    "Specifies the IPG and REPORT frame sizes used in computating reported"
    "values. cfgRptLen and cfgFecRptLen are also used to qualify grant"
    "lengths and generate the intInvGntLength interrupt."
    "Note the operating modes in which each of these fields is used:"
    "cfgFecIpgLength - Used only in 1G upstream FEC. Note 8-bytes of"
    "overhead are built in to the Epn's 1G upstream FEC calculations."
    "cfgFecRptLen - Used only in 1G upstream, for LLIDs which are"
    "FEC-enabled."
    "cfgRptLen - Used in 1G upstream for non-FEC LLIDs. Also used in 10G"
    "upstream, whether or not FEC is enabled (FEC is global at 10G"
    "upstream).",
#endif
    EPN_FEC_IPG_LENGTH_REG_OFFSET,
    0,
    0,
    93,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    EPN_FEC_IPG_LENGTH_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_FAKE_REPORT_VALUE_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_FAKE_REPORT_VALUE_EN_FIELDS[] =
{
    &EPN_FAKE_REPORT_VALUE_EN_RESERVED0_FIELD,
    &EPN_FAKE_REPORT_VALUE_EN_FAKEREPORTVALUEEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_FAKE_REPORT_VALUE_EN_REG = 
{
    "FAKE_REPORT_VALUE_EN",
#if RU_INCLUDE_DESC
    "EPN_FAKE_REPORT_VALUE_EN Register",
    "Enables a mode in which an LLID Index falsely reports that it has data,"
    "even when it does not. This mode is enabled per-LLID Index. Enabling"
    "this mode for an LLID Index causes it to report the value set in EPN"
    "Fake Report Value, regardless of any actual data that is in its"
    "associated queue(s).",
#endif
    EPN_FAKE_REPORT_VALUE_EN_REG_OFFSET,
    0,
    0,
    94,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_FAKE_REPORT_VALUE_EN_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_FAKE_REPORT_VALUE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_FAKE_REPORT_VALUE_FIELDS[] =
{
    &EPN_FAKE_REPORT_VALUE_RESERVED0_FIELD,
    &EPN_FAKE_REPORT_VALUE_FAKEREPORTVALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_FAKE_REPORT_VALUE_REG = 
{
    "FAKE_REPORT_VALUE",
#if RU_INCLUDE_DESC
    "EPN_FAKE_REPORT_VALUE Register",
    "Specifies the value sent in fake reports.",
#endif
    EPN_FAKE_REPORT_VALUE_REG_OFFSET,
    0,
    0,
    95,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_FAKE_REPORT_VALUE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_BURST_CAP_
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_BURST_CAP__FIELDS[] =
{
    &EPN_BURST_CAP__RESERVED0_FIELD,
    &EPN_BURST_CAP__BURSTCAP_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_BURST_CAP__REG = 
{
    "BURST_CAP_",
#if RU_INCLUDE_DESC
    "EPN_BURST_CAP %i Register",
    "These registers limit how much data can be reported in REPORT frames."
    "The EPON MAC reports data (that is sitting in the FIFO Queues) in"
    "between 1 and 4 \"chunks\". The size of these chunks is determined by the"
    "Burst Cap settings here. The units for these registers are 1 TQ for 1G"
    "upstream mode. The units are one-tenth of a TQ for 10G upstream mode."
    "To provision 4 TQ in 1G upstream mode, write a value of 4. To provision"
    "4 TQ in 10G upstream mode write a value of 40. Note that when register"
    "0x400 bit 21 is set that the provisioned value must be one time quantum"
    "less than the maximum value that should be reported to the OLT."
    "When configured in shared burst cap mode, the burst cap value must be"
    "reduced by the following amounts (relative to the threshold/token size"
    "value sent from the OLT)"
    "Mode         Reduction Amount (bytes)"
    "----------------       -----------------------------------"
    "1G non-FEC  21 + number of CTC priority levels"
    "1G FEC         193 + number of CTC priority levels"
    "10G   20 + number of CTC priority levels"
    "10G implicit SCI 44 + number of CTC priority levels"
    "10G explicit SCI 52 + number of CTC priority levels",
#endif
    EPN_BURST_CAP__REG_OFFSET,
    EPN_BURST_CAP__REG_RAM_CNT,
    4,
    96,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_BURST_CAP__FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_QUEUE_LLID_MAP_
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_QUEUE_LLID_MAP__FIELDS[] =
{
    &EPN_QUEUE_LLID_MAP__RESERVED0_FIELD,
    &EPN_QUEUE_LLID_MAP__QUELLIDMAP_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_QUEUE_LLID_MAP__REG = 
{
    "QUEUE_LLID_MAP_",
#if RU_INCLUDE_DESC
    "EPN_QUEUE_LLID_MAP %i Register",
    "",
#endif
    EPN_QUEUE_LLID_MAP__REG_OFFSET,
    EPN_QUEUE_LLID_MAP__REG_RAM_CNT,
    4,
    97,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_QUEUE_LLID_MAP__FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_VALID_OPCODE_MAP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_VALID_OPCODE_MAP_FIELDS[] =
{
    &EPN_VALID_OPCODE_MAP_RESERVED0_FIELD,
    &EPN_VALID_OPCODE_MAP_PRVVALIDMPCPOPCODES_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_VALID_OPCODE_MAP_REG = 
{
    "VALID_OPCODE_MAP",
#if RU_INCLUDE_DESC
    "EPN_VALID_OPCODE_MAP Register",
    "Specifies which of the first 16 MPCP opcode values should have their"
    "MPCP time overwritten. Opcode values 0 and 1 are always disabled.  The"
    "14 MPCP opcode values in the range of 2-15 can be enabled by setting"
    "the corresponding bit. The reset default value is 0x0058.",
#endif
    EPN_VALID_OPCODE_MAP_REG_OFFSET,
    0,
    0,
    98,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_VALID_OPCODE_MAP_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_UP_PACKET_TX_MARGIN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_UP_PACKET_TX_MARGIN_FIELDS[] =
{
    &EPN_UP_PACKET_TX_MARGIN_RESERVED0_FIELD,
    &EPN_UP_PACKET_TX_MARGIN_UPPACKETTXMARGIN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_UP_PACKET_TX_MARGIN_REG = 
{
    "UP_PACKET_TX_MARGIN",
#if RU_INCLUDE_DESC
    "EPN_UP_PACKET_TX_MARGIN Register",
    "Specifies the setup time margin for upstream data transfers to LIF."
    "This margin is used to police the arrival time of data from the"
    "Runner/BBH upstream data path. If BBH fails to deliver (any part of) a"
    "packet in time, EPON inserts a \"fake packet\" into the upstream burst to"
    "substitute for the late-delivered packet. When the actual packet is"
    "eventually delivered from BBH, EPON discards it. Fake packets are"
    "zero-padded and contain a guaranteed-bad FCS."
    "The upPacketTxMargin value needs to be larger than the upstream data"
    "path latency. The upstream data path latency is composed of LIF data"
    "path delay + MPCP time jitter + EPN data path delay (upTimeStampOff +"
    "42 + 1 + 10). Please note that EPN's 10 TQ upstream data path latency"
    "includes 3 TQ for the LIF to respond to valid data on the upstream"
    "EPN-to-LIF interface, and LIF's upstream data path latency includes the"
    "provisioned upTimeStampOff value.  This is because the LIF does not"
    "process a grant until it receives data from the EPN.  So, the initial"
    "packet data must arrive 42 time quanta before the Grant Start Time.",
#endif
    EPN_UP_PACKET_TX_MARGIN_REG_OFFSET,
    0,
    0,
    99,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_UP_PACKET_TX_MARGIN_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_MULTI_PRI_CFG_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_MULTI_PRI_CFG_0_FIELDS[] =
{
    &EPN_MULTI_PRI_CFG_0_RESERVED0_FIELD,
    &EPN_MULTI_PRI_CFG_0_CFGCTCSCHDEFICITEN_FIELD,
    &EPN_MULTI_PRI_CFG_0_RESERVED1_FIELD,
    &EPN_MULTI_PRI_CFG_0_PRVZEROBURSTCAPOVERRIDEMODE_FIELD,
    &EPN_MULTI_PRI_CFG_0_RESERVED2_FIELD,
    &EPN_MULTI_PRI_CFG_0_CFGSHAREDL2_FIELD,
    &EPN_MULTI_PRI_CFG_0_CFGSHAREDBURSTCAP_FIELD,
    &EPN_MULTI_PRI_CFG_0_CFGRPTGNTSOUTST0_FIELD,
    &EPN_MULTI_PRI_CFG_0_CFGRPTHIPRIFIRST0_FIELD,
    &EPN_MULTI_PRI_CFG_0_CFGRPTSWAPQS0_FIELD,
    &EPN_MULTI_PRI_CFG_0_RESERVED3_FIELD,
    &EPN_MULTI_PRI_CFG_0_CFGRPTMULTIPRI0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_MULTI_PRI_CFG_0_REG = 
{
    "MULTI_PRI_CFG_0",
#if RU_INCLUDE_DESC
    "EPN_MULTI_PRI_CFG_0 Register",
    "This register configures Multi-Priority reporting for all LLID indices",
#endif
    EPN_MULTI_PRI_CFG_0_REG_OFFSET,
    0,
    0,
    100,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    12,
    EPN_MULTI_PRI_CFG_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_SHARED_BCAP_OVRFLOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_SHARED_BCAP_OVRFLOW_FIELDS[] =
{
    &EPN_SHARED_BCAP_OVRFLOW_RESERVED0_FIELD,
    &EPN_SHARED_BCAP_OVRFLOW_SHAREDBURSTCAPOVERFLOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_SHARED_BCAP_OVRFLOW_REG = 
{
    "SHARED_BCAP_OVRFLOW",
#if RU_INCLUDE_DESC
    "EPN_SHARED_BCAP_OVRFLOW Register",
    "",
#endif
    EPN_SHARED_BCAP_OVRFLOW_REG_OFFSET,
    0,
    0,
    101,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_SHARED_BCAP_OVRFLOW_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_FORCED_REPORT_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_FORCED_REPORT_EN_FIELDS[] =
{
    &EPN_FORCED_REPORT_EN_RESERVED0_FIELD,
    &EPN_FORCED_REPORT_EN_CFGFORCEREPORTEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_FORCED_REPORT_EN_REG = 
{
    "FORCED_REPORT_EN",
#if RU_INCLUDE_DESC
    "EPN_FORCED_REPORT_EN Register",
    "Option to force an upstream report for an LLID Index that has not"
    "received an upstream grant with the \"force report\" bit set in more than"
    "50 mS. This mode is enabled per-LLID Index.   Discovery gates will not"
    "have their \"force report\" bits set.  This bit should not be enabled"
    "unless the LLID index is registered with the OLT.",
#endif
    EPN_FORCED_REPORT_EN_REG_OFFSET,
    0,
    0,
    102,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_FORCED_REPORT_EN_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_FORCED_REPORT_MAX_INTERVAL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_FORCED_REPORT_MAX_INTERVAL_FIELDS[] =
{
    &EPN_FORCED_REPORT_MAX_INTERVAL_RESERVED0_FIELD,
    &EPN_FORCED_REPORT_MAX_INTERVAL_CFGMAXREPORTINTERVAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_FORCED_REPORT_MAX_INTERVAL_REG = 
{
    "FORCED_REPORT_MAX_INTERVAL",
#if RU_INCLUDE_DESC
    "EPN_FORCED_REPORT_MAX_INTERVAL Register",
    "Option to force an upstream report for an LLID Index that has not"
    "received an upstream grant with the \"force report\" bit set in more than"
    "50 mS. This mode is enabled per-LLID Index.   Discovery gates will not"
    "have their \"force report\" bits set.  This bit should not be enabled"
    "unless the LLID index is registered with the OLT.",
#endif
    EPN_FORCED_REPORT_MAX_INTERVAL_REG_OFFSET,
    0,
    0,
    103,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_FORCED_REPORT_MAX_INTERVAL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_L2S_FLUSH_CONFIG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_L2S_FLUSH_CONFIG_FIELDS[] =
{
    &EPN_L2S_FLUSH_CONFIG_CFGFLUSHL2SEN_FIELD,
    &EPN_L2S_FLUSH_CONFIG_FLUSHL2SDONE_FIELD,
    &EPN_L2S_FLUSH_CONFIG_RESERVED0_FIELD,
    &EPN_L2S_FLUSH_CONFIG_CFGFLUSHL2SSEL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_L2S_FLUSH_CONFIG_REG = 
{
    "L2S_FLUSH_CONFIG",
#if RU_INCLUDE_DESC
    "EPN_L2S_FLUSH_CONFIG Register",
    "Provides a mechanism to flush an L2 queue. The associated LLID index"
    "must be upstream disabled (including waiting for the upstream LLID"
    "index enable feedback confirmation) before starting the flush"
    "mechanism. Once an LLID index has been disabled, the L2 queues that"
    "comprise it may be flushed one at a time via this register. A flush is"
    "started by writing the cfgFlushL2sSel field and setting the"
    "cfgFlushL2sEn bit. The flushL2sDone bit will be set by the hardware"
    "when the selected L2 queue has been flushed.  Note: Do not change the"
    "cfgFlushL2sSel value unless both \"flushL2sDone\" and \"cfgFlushL2sEn\" are"
    "low.",
#endif
    EPN_L2S_FLUSH_CONFIG_REG_OFFSET,
    0,
    0,
    104,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    EPN_L2S_FLUSH_CONFIG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DATA_PORT_COMMAND
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DATA_PORT_COMMAND_FIELDS[] =
{
    &EPN_DATA_PORT_COMMAND_DPORTBUSY_FIELD,
    &EPN_DATA_PORT_COMMAND_RESERVED0_FIELD,
    &EPN_DATA_PORT_COMMAND_DPORTSELECT_FIELD,
    &EPN_DATA_PORT_COMMAND_RESERVED1_FIELD,
    &EPN_DATA_PORT_COMMAND_DPORTCONTROL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DATA_PORT_COMMAND_REG = 
{
    "DATA_PORT_COMMAND",
#if RU_INCLUDE_DESC
    "EPN_DATA_PORT_COMMAND Register",
    "This set of registers allows processor access to RAMs controlled by the"
    "modules in the EPON block. Data Port Control indicates whether a read"
    "or write access is occurring. Data Port Select indicates which RAM is"
    "being accessed. Writing to the Data Port Command register (offset 0)"
    "initiates the memory access."
    "The processor may only access the Downstream Statistics RAM and"
    "Upstream Statistics RAM during run time. Accessing the L2 queue RAM may"
    "cause the EPON to fail in an unknown and random way."
    "The flow for a write operation is as follows."
    "1. Check if the Data Port Interrupt is ready."
    "2. Update the Data Port Address register."
    "3. Update the Data Port Data register"
    "4. Update the Data Port Command register. Write a \"1\" to the Data"
    "Port Control and the RAM's index into the Data Port Select."
    "5. The operation is completed when the Data Port Interrupt is"
    "ready again."
    "The flow for a read operation is as follows."
    "1. Check if the Data Port Interrupt is ready."
    "2. Update the Data Port Address register."
    "3. Update the Data Port Command register. Write a \"0\" to the Data"
    "Port Control and the RAM's index into the Data Port Select."
    "4. Check to see if the Data Port Interrupt is read."
    "5. Read the Data Port Data register to get the operation's"
    "results."
    ""
    "RAM Name  RAM Size AutoInit?"
    "Downstream Statistics   256 x 32 Yes"
    "Upstream Statistics   256 x 32 Yes"
    "L2 Queue RAM  16384 x 15 No"
    ""
    "The Downstream Statistics RAM is divided into 32 segments.  The"
    "bidirectional LLIDs accumulate 18 different statistics.  The"
    "downstream-only LLIDs accumulate 14 different statistics."
    "Downstream-only LLIDs do not have upstream counterparts and"
    "consequently may not send upstream traffic.  LLIDs 0-7 are"
    "bidirectional. LLIDs 8-15 are downstream-only."
    "Downstream Statistics RAM definition are as follows :"
    ""
    "LLID Index RAM Locations"
    "0 - Bidir    0 - 17"
    "8 - Down only   18 - 31"
    "1 - Bidir   32 - 49"
    "9 - Down only   50 - 63"
    "2 - Bidir   64 - 81"
    "10 - Down only   82 - 95"
    "3 - Bidir   96 - 113"
    "11 - Down only  114 - 127"
    "4 - Bidir  128 - 145"
    "12 - Down only  146 - 159"
    "5 - Bidir  160 - 177"
    "13 - Down only  178 - 191"
    "6 - Bidir  192 - 209"
    "14 - Down only  210 - 223"
    "7 - Bidir  224 - 241"
    "15 - Down only  242 - 255"
    ""
    "RAM Offset  Description"
    "0     Bidir downstream Total bytes received"
    "1     Bidir downstream FCS Errors"
    "2     Bidir downstream OAM frames received"
    "3     Bidir downstream GATE frames received"
    "4     Bidir downstream 64 byte frames received"
    "5     Bidir downstream 65 - 127 byte frames received"
    "6     Bidir downstream 128 - 255 byte frames received"
    "7     Bidir downstream 256 - 511 byte frames received"
    "8     Bidir downstream 512 - 1023 byte frames received"
    "9     Bidir downstream 1024 - 1518 byte frames received"
    "10     Bidir downstream Greater than 1518 byte frames received"
    "11     Bidir downstream Oversized frames received"
    "12     Bidir downstream Broadcast frames received"
    "13     Bidir downstream Multicast frames received"
    "14     Bidir downstream Unicast frames received"
    "15     Bidir downstream Undersized frames received"
    "16     Bidir downstream OAM bytes received"
    "17     Bidir downstream Register frames received"
    "18     Downstream-only Total bytes received"
    "19     Downstream-only FCS Errors"
    "20     Downstream-only OAM frames received"
    "21     Downstream-only GATE frames received"
    "22     Downstream-only Broadcast frames received"
    "23     Downstream-only Multicast frames received"
    "24     Downstream-only Unicast frames received"
    "25     Downstream-only 64 - 511 byte frames received"
    "26     Downstream-only 512 - 1023 byte frames received"
    "27     Downstream-only 1024 - 1518 byte frames received"
    "28     Downstream-only Greater than 1518 bytes frames received"
    "29     Downstream-only Oversized frames received"
    "30     Downstream-only Undersized frames received"
    "31     Downstream-only OAM bytes received"
    ""
    "The Upstream Statistics RAMs accumulate statistics for 8 LLIDs. Each"
    "LLID occupies 16 RAM offsets.  It is logically divided into 16"
    "segments.  The lower 8 segments contain the normal upstream statistics"
    "for each of the 8 LLID Indexes.  The upper 8 segments are used to"
    "report the \"fake packet\" statistics.  Fake packets are inserted into"
    "upstream bursts to substitute for packets that are delivered late from"
    "the Runner/BBH subsystem. Fake packets are zero-padded and contain a"
    "guaranteed-bad FCS. Note that a faked packet can cause broadcast and"
    "multicast packets to be reported as unicast packets, depending on"
    "whether the Runner/BBH upstream data delivery failure occurred before"
    "the DA data was delivered."
    ""
    "LLID Index RAM Locations"
    "0    0 - 15"
    "1   16 - 31"
    "2   32 - 47"
    "3   48 - 63"
    "4   64 - 79"
    "5   80 - 95"
    "6   96 - 111"
    "7  112 - 127"
    ""
    "Fake packet 0 128 - 143"
    "Fake packet 1 144 - 159"
    "Fake packet 2 160 - 175"
    "Fake packet 3 176 - 191"
    "Fake packet 4 192 - 207"
    "Fake packet 5 208 - 223"
    "Fake packet 6 224 - 239"
    "Fake packet 7 240 - 255"
    ""
    ""
    "RAM Offset  Description"
    "0     Total bytes sent"
    "1     Reserved"
    "2     OAM frames sent"
    "3     REPORT frames sent"
    "4     64 byte frames sent"
    "5     65 - 127 byte frames sent"
    "6     128 - 255 byte frames sent"
    "7     256 - 511 byte frames sent"
    "8     512 - 1023 byte frames sent"
    "9     1024 - 1518 byte frames sent"
    "10     Greater than 1518 bytes frames sent"
    "11     OAM bytes sent"
    "12     Broadcast frames sent"
    "13     Multicast frames sent"
    "14     Unicast frames sent"
    "15     Reserved"
    ""
    "Notes:"
    "Total bytes sent do not include OAM or Report frames."
    "The various frame \"bucket\" statistics do not include OAM or Report"
    "frames."
    "Oversized frames are frames that are greater than the value specified"
    "by the EPON Max Frame Size register. The exception is when cfgVlanMax"
    "mode is used. In this case an oversized condition occurs when a frame"
    "is greater than 1522 byte when a VLAN tag is present, otherwise greater"
    "than 1518 bytes"
    "An undersized frame condition occurs when the received frame is less"
    "than 64 bytes in length and has a valid FCS."
    "The Unused Words counts the number of words that in a grant that the"
    "LLID was not able to send upstream traffic on. If the LLID received a"
    "grant for 2K words, but it has only 1K words of data, the unused word"
    "count will increase by 1 K. In most cases this condition should not"
    "happen. It indicates that the OLT is granting inefficiently. There"
    "could be a problem with the ONU's EPON overhead setting being"
    "incorrect.",
#endif
    EPN_DATA_PORT_COMMAND_REG_OFFSET,
    0,
    0,
    105,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    EPN_DATA_PORT_COMMAND_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DATA_PORT_ADDRESS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DATA_PORT_ADDRESS_FIELDS[] =
{
    &EPN_DATA_PORT_ADDRESS_RESERVED0_FIELD,
    &EPN_DATA_PORT_ADDRESS_DPORTADDR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DATA_PORT_ADDRESS_REG = 
{
    "DATA_PORT_ADDRESS",
#if RU_INCLUDE_DESC
    "EPN_DATA_PORT_ADDRESS Register",
    "",
#endif
    EPN_DATA_PORT_ADDRESS_REG_OFFSET,
    0,
    0,
    106,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_DATA_PORT_ADDRESS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DATA_PORT_DATA_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DATA_PORT_DATA_0_FIELDS[] =
{
    &EPN_DATA_PORT_DATA_0_DPORTDATA0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DATA_PORT_DATA_0_REG = 
{
    "DATA_PORT_DATA_0",
#if RU_INCLUDE_DESC
    "EPN_DATA_PORT_DATA_0 Register",
    "",
#endif
    EPN_DATA_PORT_DATA_0_REG_OFFSET,
    0,
    0,
    107,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPN_DATA_PORT_DATA_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_UNMAP_BIG_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_UNMAP_BIG_CNT_FIELDS[] =
{
    &EPN_UNMAP_BIG_CNT_UNMAPBIGERRCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_UNMAP_BIG_CNT_REG = 
{
    "UNMAP_BIG_CNT",
#if RU_INCLUDE_DESC
    "EPN_UNMAP_BIG_CNT Register",
    "Statistics register for traffic sent on unmapped LLIDs."
    "This register saturates at maximum value and self-clears when read.",
#endif
    EPN_UNMAP_BIG_CNT_REG_OFFSET,
    0,
    0,
    108,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPN_UNMAP_BIG_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_UNMAP_FRAME_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_UNMAP_FRAME_CNT_FIELDS[] =
{
    &EPN_UNMAP_FRAME_CNT_UNMAPFRCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_UNMAP_FRAME_CNT_REG = 
{
    "UNMAP_FRAME_CNT",
#if RU_INCLUDE_DESC
    "EPN_UNMAP_FRAME_CNT Register",
    "Statistics register for traffic sent on unmapped LLIDs."
    "This register saturates at maximum value and self-clears when read.",
#endif
    EPN_UNMAP_FRAME_CNT_REG_OFFSET,
    0,
    0,
    109,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPN_UNMAP_FRAME_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_UNMAP_FCS_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_UNMAP_FCS_CNT_FIELDS[] =
{
    &EPN_UNMAP_FCS_CNT_UNMAPFCSERRCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_UNMAP_FCS_CNT_REG = 
{
    "UNMAP_FCS_CNT",
#if RU_INCLUDE_DESC
    "EPN_UNMAP_FCS_CNT Register",
    "Statistics register for traffic sent on unmapped LLIDs."
    "This register saturates at maximum value and self-clears when read.",
#endif
    EPN_UNMAP_FCS_CNT_REG_OFFSET,
    0,
    0,
    110,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPN_UNMAP_FCS_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_UNMAP_GATE_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_UNMAP_GATE_CNT_FIELDS[] =
{
    &EPN_UNMAP_GATE_CNT_UNMAPGATECNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_UNMAP_GATE_CNT_REG = 
{
    "UNMAP_GATE_CNT",
#if RU_INCLUDE_DESC
    "EPN_UNMAP_GATE_CNT Register",
    "Statistics register for traffic sent on unmapped LLIDs."
    "This register saturates at maximum value and self-clears when read.",
#endif
    EPN_UNMAP_GATE_CNT_REG_OFFSET,
    0,
    0,
    111,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPN_UNMAP_GATE_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_UNMAP_OAM_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_UNMAP_OAM_CNT_FIELDS[] =
{
    &EPN_UNMAP_OAM_CNT_UNMAPOAMCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_UNMAP_OAM_CNT_REG = 
{
    "UNMAP_OAM_CNT",
#if RU_INCLUDE_DESC
    "EPN_UNMAP_OAM_CNT Register",
    "Statistics register for traffic sent on unmapped LLIDs."
    "This register saturates at maximum value and self-clears when read.",
#endif
    EPN_UNMAP_OAM_CNT_REG_OFFSET,
    0,
    0,
    112,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPN_UNMAP_OAM_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_UNMAP_SMALL_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_UNMAP_SMALL_CNT_FIELDS[] =
{
    &EPN_UNMAP_SMALL_CNT_UNMAPSMALLERRCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_UNMAP_SMALL_CNT_REG = 
{
    "UNMAP_SMALL_CNT",
#if RU_INCLUDE_DESC
    "EPN_UNMAP_SMALL_CNT Register",
    "Statistics register for traffic sent on unmapped LLIDs."
    "This register saturates at maximum value and self-clears when read.",
#endif
    EPN_UNMAP_SMALL_CNT_REG_OFFSET,
    0,
    0,
    113,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPN_UNMAP_SMALL_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_FIF_DEQUEUE_EVENT_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_FIF_DEQUEUE_EVENT_CNT_FIELDS[] =
{
    &EPN_FIF_DEQUEUE_EVENT_CNT_FIFDEQUEUEEVENTCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_FIF_DEQUEUE_EVENT_CNT_REG = 
{
    "FIF_DEQUEUE_EVENT_CNT",
#if RU_INCLUDE_DESC
    "EPN_FIF_DEQUEUE_EVENT_CNT Register",
    "",
#endif
    EPN_FIF_DEQUEUE_EVENT_CNT_REG_OFFSET,
    0,
    0,
    114,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPN_FIF_DEQUEUE_EVENT_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_UNUSED_TQ_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_UNUSED_TQ_CNT_FIELDS[] =
{
    &EPN_UNUSED_TQ_CNT_UNUSEDTQCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_UNUSED_TQ_CNT_REG = 
{
    "UNUSED_TQ_CNT",
#if RU_INCLUDE_DESC
    "EPN_UNUSED_TQ_CNT %i Register",
    "Statistics register that accumulates the number of upstream unused time"
    "quanta for an upstream LLID."
    "The register saturates at maximum value. The register will clear upon"
    "read.",
#endif
    EPN_UNUSED_TQ_CNT_REG_OFFSET,
    EPN_UNUSED_TQ_CNT_REG_RAM_CNT,
    4,
    115,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPN_UNUSED_TQ_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_BBH_UP_FAULT_HALT_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_BBH_UP_FAULT_HALT_EN_FIELDS[] =
{
    &EPN_BBH_UP_FAULT_HALT_EN_RESERVED0_FIELD,
    &EPN_BBH_UP_FAULT_HALT_EN_BBHUPSFAULTHALTEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_BBH_UP_FAULT_HALT_EN_REG = 
{
    "BBH_UP_FAULT_HALT_EN",
#if RU_INCLUDE_DESC
    "EPN_BBH_UP_FAULT_HALT_EN Register",
    "Per-LLID index Runner/BBH upstream fault halt enable.  This allows the"
    "default fatal upstream fault halt behavior to be disabled.  The reset"
    "default value is enabled (all ones).  This register should only be used"
    "for debug.  All the bits in this register must be set during normal"
    "operation.",
#endif
    EPN_BBH_UP_FAULT_HALT_EN_REG_OFFSET,
    0,
    0,
    116,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_BBH_UP_FAULT_HALT_EN_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_BBH_UP_TARDY_HALT_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_BBH_UP_TARDY_HALT_EN_FIELDS[] =
{
    &EPN_BBH_UP_TARDY_HALT_EN_RESERVED0_FIELD,
    &EPN_BBH_UP_TARDY_HALT_EN_FATALTARDYBBHABORTEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_BBH_UP_TARDY_HALT_EN_REG = 
{
    "BBH_UP_TARDY_HALT_EN",
#if RU_INCLUDE_DESC
    "EPN_BBH_UP_TARDY_HALT_EN Register",
    "Per-LLID index Runner/BBH upstream fault halt enable.  This allows the"
    "default fatal upstream fault halt behavior to be disabled.  The reset"
    "default value is enabled (all ones).  This register should only be used"
    "for debug.  All the bits in this register must be set during normal"
    "operation.",
#endif
    EPN_BBH_UP_TARDY_HALT_EN_REG_OFFSET,
    0,
    0,
    117,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_BBH_UP_TARDY_HALT_EN_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DEBUG_STATUS_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DEBUG_STATUS_0_FIELDS[] =
{
    &EPN_DEBUG_STATUS_0_RESERVED0_FIELD,
    &EPN_DEBUG_STATUS_0_L2SQUEFULLDEBUG_FIELD,
    &EPN_DEBUG_STATUS_0_RESERVED1_FIELD,
    &EPN_DEBUG_STATUS_0_DNDLUFULL_FIELD,
    &EPN_DEBUG_STATUS_0_DNSECFULL_FIELD,
    &EPN_DEBUG_STATUS_0_EPNLIFFIFOFULL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DEBUG_STATUS_0_REG = 
{
    "DEBUG_STATUS_0",
#if RU_INCLUDE_DESC
    "EPN_DEBUG_STATUS_0 Register",
    "This register contains the real time status bits to aid debugging the"
    "EPN module.",
#endif
    EPN_DEBUG_STATUS_0_REG_OFFSET,
    0,
    0,
    118,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    EPN_DEBUG_STATUS_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DEBUG_STATUS_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DEBUG_STATUS_1_FIELDS[] =
{
    &EPN_DEBUG_STATUS_1_RESERVED0_FIELD,
    &EPN_DEBUG_STATUS_1_GNTRDY7_FIELD,
    &EPN_DEBUG_STATUS_1_GNTRDY6_FIELD,
    &EPN_DEBUG_STATUS_1_GNTRDY5_FIELD,
    &EPN_DEBUG_STATUS_1_GNTRDY4_FIELD,
    &EPN_DEBUG_STATUS_1_GNTRDY3_FIELD,
    &EPN_DEBUG_STATUS_1_GNTRDY2_FIELD,
    &EPN_DEBUG_STATUS_1_GNTRDY1_FIELD,
    &EPN_DEBUG_STATUS_1_GNTRDY0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DEBUG_STATUS_1_REG = 
{
    "DEBUG_STATUS_1",
#if RU_INCLUDE_DESC
    "EPN_DEBUG_STATUS_1 Register",
    "This register contains the real time status bits to aid debugging the"
    "EPN module.",
#endif
    EPN_DEBUG_STATUS_1_REG_OFFSET,
    0,
    0,
    119,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPN_DEBUG_STATUS_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DEBUG_L2S_PTR_SEL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DEBUG_L2S_PTR_SEL_FIELDS[] =
{
    &EPN_DEBUG_L2S_PTR_SEL_RESERVED0_FIELD,
    &EPN_DEBUG_L2S_PTR_SEL_CFGL2SDEBUGPTRSEL_FIELD,
    &EPN_DEBUG_L2S_PTR_SEL_RESERVED1_FIELD,
    &EPN_DEBUG_L2S_PTR_SEL_L2SDEBUGPTRSTATE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DEBUG_L2S_PTR_SEL_REG = 
{
    "DEBUG_L2S_PTR_SEL",
#if RU_INCLUDE_DESC
    "EPN_DEBUG_L2S_PTR_SEL Register",
    "",
#endif
    EPN_DEBUG_L2S_PTR_SEL_REG_OFFSET,
    0,
    0,
    120,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    EPN_DEBUG_L2S_PTR_SEL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_OLT_MAC_ADDR_LO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_OLT_MAC_ADDR_LO_FIELDS[] =
{
    &EPN_OLT_MAC_ADDR_LO_OLTADDRLO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_OLT_MAC_ADDR_LO_REG = 
{
    "OLT_MAC_ADDR_LO",
#if RU_INCLUDE_DESC
    "EPN_OLT_MAC_ADDR_LO Register",
    "This register stores a MAC address for the OLT. This address is"
    "inserted as the DA in REPORT frames sent upstream.",
#endif
    EPN_OLT_MAC_ADDR_LO_REG_OFFSET,
    0,
    0,
    121,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPN_OLT_MAC_ADDR_LO_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_OLT_MAC_ADDR_HI
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_OLT_MAC_ADDR_HI_FIELDS[] =
{
    &EPN_OLT_MAC_ADDR_HI_RESERVED0_FIELD,
    &EPN_OLT_MAC_ADDR_HI_OLTADDRHI_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_OLT_MAC_ADDR_HI_REG = 
{
    "OLT_MAC_ADDR_HI",
#if RU_INCLUDE_DESC
    "EPN_OLT_MAC_ADDR_HI Register",
    "This register stores a MAC address for the OLT. This address is"
    "inserted as the DA in REPORT frames sent upstream.",
#endif
    EPN_OLT_MAC_ADDR_HI_REG_OFFSET,
    0,
    0,
    122,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_OLT_MAC_ADDR_HI_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_TX_L1S_SHP_DQU_EMPTY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_TX_L1S_SHP_DQU_EMPTY_FIELDS[] =
{
    &EPN_TX_L1S_SHP_DQU_EMPTY_RESERVED0_FIELD,
    &EPN_TX_L1S_SHP_DQU_EMPTY_L1SDQUQUEEMPTY_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_TX_L1S_SHP_DQU_EMPTY_REG = 
{
    "TX_L1S_SHP_DQU_EMPTY",
#if RU_INCLUDE_DESC
    "EPN_TX_L1S_SHP_DQU_EMPTY Register",
    "Indicates empty status of L1 shaped queues",
#endif
    EPN_TX_L1S_SHP_DQU_EMPTY_REG_OFFSET,
    0,
    0,
    123,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_TX_L1S_SHP_DQU_EMPTY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_TX_L1S_UNSHAPED_EMPTY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_TX_L1S_UNSHAPED_EMPTY_FIELDS[] =
{
    &EPN_TX_L1S_UNSHAPED_EMPTY_RESERVED0_FIELD,
    &EPN_TX_L1S_UNSHAPED_EMPTY_L1SUNSHAPEDQUEEMPTY_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_TX_L1S_UNSHAPED_EMPTY_REG = 
{
    "TX_L1S_UNSHAPED_EMPTY",
#if RU_INCLUDE_DESC
    "EPN_TX_L1S_UNSHAPED_EMPTY Register",
    "Indicates status of L1 unshaped-empty accumulators",
#endif
    EPN_TX_L1S_UNSHAPED_EMPTY_REG_OFFSET,
    0,
    0,
    124,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_TX_L1S_UNSHAPED_EMPTY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_TX_L1S_SHP_QUE_MASK_
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_TX_L1S_SHP_QUE_MASK__FIELDS[] =
{
    &EPN_TX_L1S_SHP_QUE_MASK__RESERVED0_FIELD,
    &EPN_TX_L1S_SHP_QUE_MASK__CFGSHPMASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_TX_L1S_SHP_QUE_MASK__REG = 
{
    "TX_L1S_SHP_QUE_MASK_",
#if RU_INCLUDE_DESC
    "EPN_TX_L1S_SHP_QUE_MASK %i Register",
    "This register allows the effects of the upstream shaper 0 to be masked."
    "When multiple priority queues point to a common shaping bucket, the"
    "low priority traffic can use all the credits prior to a high priority"
    "frame arriving. The high priority would then incur additional latency"
    "waiting for credits to accrue. This adds undesirable delay to the data"
    "path. This feature separates the queues that apply credit to a shaper"
    "and those that are masked by the shaper. The Shaping function has an"
    "input that credits the shaping bucket when a frame is sent. It also has"
    "an output that masks a queue from being eligible to transmit when the"
    "shaping rate is violated. In this case the low priority queue will both"
    "credit and be masked by the shaper as typically done. The high priority"
    "on the other hand will credit the shaper, but not be limited by it. The"
    "shaper will then be allowed to go into a deficit. In most cases the"
    "high priority is a small portion of the total bandwidth. In some cases"
    "however it may be desired to limit the high priority traffic with a"
    "different shaper.",
#endif
    EPN_TX_L1S_SHP_QUE_MASK__REG_OFFSET,
    EPN_TX_L1S_SHP_QUE_MASK__REG_RAM_CNT,
    4,
    125,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_TX_L1S_SHP_QUE_MASK__FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_TX_L2S_QUE_CONFIG_
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_TX_L2S_QUE_CONFIG__FIELDS[] =
{
    &EPN_TX_L2S_QUE_CONFIG__RESERVED0_FIELD,
    &EPN_TX_L2S_QUE_CONFIG__CFGL2SQUEEND_FIELD,
    &EPN_TX_L2S_QUE_CONFIG__RESERVED1_FIELD,
    &EPN_TX_L2S_QUE_CONFIG__CFGL2SQUESTART_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_TX_L2S_QUE_CONFIG__REG = 
{
    "TX_L2S_QUE_CONFIG_",
#if RU_INCLUDE_DESC
    "EPN_TX_L2S_QUE_CONFIG %i Register",
    "This registers configures the base addresses of the L2 structure 0."
    "Internally the least significant two bits are treated as constants: The"
    "two least significant start address bits are 0 and the two least"
    "significant end address bits are 1. Therefore the queue base addresses"
    "cannot be read directly from the register contents without performing"
    "some mental gymnastics. Try to picture a 2-bit left shift and stuff"
    "operation. The base address granularity is 4 entries. It is critical"
    "that the number of entries allocated for each FIFO be sufficient to"
    "accommodate the worst case number of frames that can be contained in"
    "the associated Fif queue. A general rule-of-thumb would be to divide"
    "the number of bytes in the respective Fif queue by 80. The quotient+1"
    "will be the minimum number of entries that should be allocated. Do not"
    "forget to round up to the nearest 4-entry quanta.  If the L2 is being"
    "sized to match the burst cap (instead of the entire Fif queue size) do"
    "not forget to multiply the quotient+1 by four (or by 5 if"
    "prvTekModePrefetch is set) when operating in Teknovus mode. There are"
    "four burst cap values reported in Teknovus mode"
    "Note: This register cannot be programmed \"on-the-fly\". The queue"
    "start/end address values should be changed only when its associated"
    "clear L2 report FIFO\" bit is set.",
#endif
    EPN_TX_L2S_QUE_CONFIG__REG_OFFSET,
    EPN_TX_L2S_QUE_CONFIG__REG_RAM_CNT,
    4,
    126,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    EPN_TX_L2S_QUE_CONFIG__FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_TX_L2S_QUE_EMPTY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_TX_L2S_QUE_EMPTY_FIELDS[] =
{
    &EPN_TX_L2S_QUE_EMPTY_RESERVED0_FIELD,
    &EPN_TX_L2S_QUE_EMPTY_L2SQUEEMPTY_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_TX_L2S_QUE_EMPTY_REG = 
{
    "TX_L2S_QUE_EMPTY",
#if RU_INCLUDE_DESC
    "EPN_TX_L2S_QUE_EMPTY Register",
    "L2 queue empty status",
#endif
    EPN_TX_L2S_QUE_EMPTY_REG_OFFSET,
    0,
    0,
    127,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_TX_L2S_QUE_EMPTY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_TX_L2S_QUE_FULL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_TX_L2S_QUE_FULL_FIELDS[] =
{
    &EPN_TX_L2S_QUE_FULL_RESERVED0_FIELD,
    &EPN_TX_L2S_QUE_FULL_L2SQUEFULL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_TX_L2S_QUE_FULL_REG = 
{
    "TX_L2S_QUE_FULL",
#if RU_INCLUDE_DESC
    "EPN_TX_L2S_QUE_FULL Register",
    "L2 queue full status",
#endif
    EPN_TX_L2S_QUE_FULL_REG_OFFSET,
    0,
    0,
    128,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_TX_L2S_QUE_FULL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_TX_L2S_QUE_STOPPED
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_TX_L2S_QUE_STOPPED_FIELDS[] =
{
    &EPN_TX_L2S_QUE_STOPPED_RESERVED0_FIELD,
    &EPN_TX_L2S_QUE_STOPPED_L2SSTOPPEDQUEUES_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_TX_L2S_QUE_STOPPED_REG = 
{
    "TX_L2S_QUE_STOPPED",
#if RU_INCLUDE_DESC
    "EPN_TX_L2S_QUE_STOPPED Register",
    "L2 queue stopped status",
#endif
    EPN_TX_L2S_QUE_STOPPED_REG_OFFSET,
    0,
    0,
    129,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_TX_L2S_QUE_STOPPED_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_TX_CTC_BURST_LIMIT_
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_TX_CTC_BURST_LIMIT__FIELDS[] =
{
    &EPN_TX_CTC_BURST_LIMIT__RESERVED0_FIELD,
    &EPN_TX_CTC_BURST_LIMIT__PRVBURSTLIMIT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_TX_CTC_BURST_LIMIT__REG = 
{
    "TX_CTC_BURST_LIMIT_",
#if RU_INCLUDE_DESC
    "EPN_TX_CTC_BURST_LIMIT %i Register",
    "This register configures the maximum number of bytes that L2 queue 0"
    "can transmit during any given round. A round ends when all L2 queues"
    "have reached their respective burst limit or there is no more data to"
    "transmit. Note that setting a burst limit to zero enables the"
    "respective L2 queue to transmit in strict priority. Also, any burst"
    "limits that are set to 1 will cause those L2 queues to transmit in"
    "round-robin\" fashion. It is possible to allocate bandwidth as a"
    "percentage. Simply multiply the desired percentage by 2000 bytes (the"
    "maximum frame length) and write that value in the associated burst"
    "limit register."
    "Example bandwidth sharing configuration:"
    ""
    "Priority   Burst Limit Description"
    "--------   ----------- ------------"
    "0    0  High priority unlimited bandwidth"
    "1    0  Low priority unlimited bandwidth"
    "2    1  Equal priority UNI 1 unlimited bandwidth"
    "3    1  Equal priority UNI 2 unlimited bandwidth"
    "4    1  Equal priority UNI 3 unlimited bandwidth"
    "5    1  Equal priority UNI 4 unlimited bandwidth"
    "6    0  High priority best effort bandwidth"
    "7    0  Low priority best effort bandwidth"
    ""
    "Priority 0 is for the highest priority traffic. i.e., Management"
    "traffic"
    "Priority 1 is for real-time traffic. i.e., VOIP"
    "Priorities 2, 3, 4 and 5 equally share bandwidth, i.e. Premium business"
    "traffic."
    "Priorities 6 and 7 provide two classes of best effort traffic, i.e. two"
    "classes of consumer traffic"
    ""
    "Please note that the EPN shaper is the mechanism used to limit the"
    "amount of traffic.",
#endif
    EPN_TX_CTC_BURST_LIMIT__REG_OFFSET,
    EPN_TX_CTC_BURST_LIMIT__REG_RAM_CNT,
    4,
    130,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_TX_CTC_BURST_LIMIT__FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_BBH_MAX_OUTSTANDING_TARDY_PACKETS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_BBH_MAX_OUTSTANDING_TARDY_PACKETS_FIELDS[] =
{
    &EPN_BBH_MAX_OUTSTANDING_TARDY_PACKETS_RESERVED0_FIELD,
    &EPN_BBH_MAX_OUTSTANDING_TARDY_PACKETS_CFGMAXOUTSTANDINGTARDYPACKETS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_BBH_MAX_OUTSTANDING_TARDY_PACKETS_REG = 
{
    "BBH_MAX_OUTSTANDING_TARDY_PACKETS",
#if RU_INCLUDE_DESC
    "EPN_BBH_MAX_OUTSTANDING_TARDY_PACKETS Register",
    "Everytime the BBH fails to deliver a packet in time to be transmitted"
    "upstream"
    "it is placed in failed state and the Epn increments a tardy packet"
    "counter.  Every time the BBH delivers a packet"
    "while it is in the failed state the tardy packet counter is"
    "decremented.  The BBH will be taken out of the"
    "failed state when the tardy packet counter reaches 0.  If the tardy"
    "packet counter"
    "value exceeds the value of this register; then the BBH becomes truant"
    "and the Epn stops upstream traffic until"
    "the Epn is reset.  It is expected the BBH, runner and Epn will be reset"
    "at the same time.",
#endif
    EPN_BBH_MAX_OUTSTANDING_TARDY_PACKETS_REG_OFFSET,
    0,
    0,
    131,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_BBH_MAX_OUTSTANDING_TARDY_PACKETS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_MIN_REPORT_VALUE_DIFFERENCE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_MIN_REPORT_VALUE_DIFFERENCE_FIELDS[] =
{
    &EPN_MIN_REPORT_VALUE_DIFFERENCE_RESERVED0_FIELD,
    &EPN_MIN_REPORT_VALUE_DIFFERENCE_PRVMINREPORTDIFF_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_MIN_REPORT_VALUE_DIFFERENCE_REG = 
{
    "MIN_REPORT_VALUE_DIFFERENCE",
#if RU_INCLUDE_DESC
    "EPN_MIN_REPORT_VALUE_DIFFERENCE Register",
    "The Virtual Thresholds are determined from the smaller of the queue set"
    "shaper and the accumulated queue length.  The Virtual Thresholds must"
    "also prevent pathological values from being reported.  The reported"
    "value should be at least one maximum frame length greater than the"
    "previous queue set threshold or else it will be reported as zero.  This"
    "prevents a head of line blocking issue if a large frame is at the head"
    "of the queue and also prevents from reporting illegally short grant"
    "lengths.Everytime the BBH fails to deliver a packet in time to be"
    "transmitted upstream",
#endif
    EPN_MIN_REPORT_VALUE_DIFFERENCE_REG_OFFSET,
    0,
    0,
    132,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_MIN_REPORT_VALUE_DIFFERENCE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_BBH_STATUS_FIFO_OVERFLOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_BBH_STATUS_FIFO_OVERFLOW_FIELDS[] =
{
    &EPN_BBH_STATUS_FIFO_OVERFLOW_RESERVED0_FIELD,
    &EPN_BBH_STATUS_FIFO_OVERFLOW_UTXBBHSTATUSFIFOOVERFLOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_BBH_STATUS_FIFO_OVERFLOW_REG = 
{
    "BBH_STATUS_FIFO_OVERFLOW",
#if RU_INCLUDE_DESC
    "EPN_BBH_STATUS_FIFO_OVERFLOW Register",
    "Indicates which BBH queue status interface event FIFOs have overflowed."
    "Note: These bits are used for debug only.",
#endif
    EPN_BBH_STATUS_FIFO_OVERFLOW_REG_OFFSET,
    0,
    0,
    133,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_BBH_STATUS_FIFO_OVERFLOW_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_SPARE_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_SPARE_CTL_FIELDS[] =
{
    &EPN_SPARE_CTL_CFGEPNSPARE_FIELD,
    &EPN_SPARE_CTL_ECOUTXSNFENABLE_FIELD,
    &EPN_SPARE_CTL_ECOJIRA758ENABLE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_SPARE_CTL_REG = 
{
    "SPARE_CTL",
#if RU_INCLUDE_DESC
    "EPN_SPARE_CTL Register",
    "Spare RW bits",
#endif
    EPN_SPARE_CTL_REG_OFFSET,
    0,
    0,
    134,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    EPN_SPARE_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_TS_SYNC_OFFSET
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_TS_SYNC_OFFSET_FIELDS[] =
{
    &EPN_TS_SYNC_OFFSET_RESERVED0_FIELD,
    &EPN_TS_SYNC_OFFSET_CFGTSSYNCOFFSET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_TS_SYNC_OFFSET_REG = 
{
    "TS_SYNC_OFFSET",
#if RU_INCLUDE_DESC
    "EPN_TS_SYNC_OFFSET Register",
    "Timestamp synchronizer offset.",
#endif
    EPN_TS_SYNC_OFFSET_REG_OFFSET,
    0,
    0,
    135,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_TS_SYNC_OFFSET_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DN_TS_OFFSET
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DN_TS_OFFSET_FIELDS[] =
{
    &EPN_DN_TS_OFFSET_CFGDNTSOFFSET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DN_TS_OFFSET_REG = 
{
    "DN_TS_OFFSET",
#if RU_INCLUDE_DESC
    "EPN_DN_TS_OFFSET Register",
    "Downstream timestamp offset.",
#endif
    EPN_DN_TS_OFFSET_REG_OFFSET,
    0,
    0,
    136,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPN_DN_TS_OFFSET_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_UP_TS_OFFSET_LO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_UP_TS_OFFSET_LO_FIELDS[] =
{
    &EPN_UP_TS_OFFSET_LO_CFGUPTSOFFSET_LO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_UP_TS_OFFSET_LO_REG = 
{
    "UP_TS_OFFSET_LO",
#if RU_INCLUDE_DESC
    "EPN_UP_TS_OFFSET_LO Register",
    "Upstream timestamp offset, lower 32 bits.",
#endif
    EPN_UP_TS_OFFSET_LO_REG_OFFSET,
    0,
    0,
    137,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPN_UP_TS_OFFSET_LO_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_UP_TS_OFFSET_HI
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_UP_TS_OFFSET_HI_FIELDS[] =
{
    &EPN_UP_TS_OFFSET_HI_RESERVED0_FIELD,
    &EPN_UP_TS_OFFSET_HI_CFGUPTSOFFSET_HI_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_UP_TS_OFFSET_HI_REG = 
{
    "UP_TS_OFFSET_HI",
#if RU_INCLUDE_DESC
    "EPN_UP_TS_OFFSET_HI Register",
    "Upstream timestamp offset, upper 16 bits.",
#endif
    EPN_UP_TS_OFFSET_HI_REG_OFFSET,
    0,
    0,
    138,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_UP_TS_OFFSET_HI_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_TWO_STEP_TS_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_TWO_STEP_TS_CTL_FIELDS[] =
{
    &EPN_TWO_STEP_TS_CTL_TWOSTEPFFRD_FIELD,
    &EPN_TWO_STEP_TS_CTL_RESERVED0_FIELD,
    &EPN_TWO_STEP_TS_CTL_TWOSTEPFFENTRIES_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_TWO_STEP_TS_CTL_REG = 
{
    "TWO_STEP_TS_CTL",
#if RU_INCLUDE_DESC
    "EPN_TWO_STEP_TS_CTL Register",
    "Provides control for the reading of two step timestamp FIFO, 4 entries"
    "deep.",
#endif
    EPN_TWO_STEP_TS_CTL_REG_OFFSET,
    0,
    0,
    139,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    EPN_TWO_STEP_TS_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_TWO_STEP_TS_VALUE_LO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_TWO_STEP_TS_VALUE_LO_FIELDS[] =
{
    &EPN_TWO_STEP_TS_VALUE_LO_TWOSTEPTIMESTAMP_LO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_TWO_STEP_TS_VALUE_LO_REG = 
{
    "TWO_STEP_TS_VALUE_LO",
#if RU_INCLUDE_DESC
    "EPN_TWO_STEP_TS_VALUE_LO Register",
    "Lower 32-bits of 48-bits timestamp value, applicable for two step"
    "timestamping.",
#endif
    EPN_TWO_STEP_TS_VALUE_LO_REG_OFFSET,
    0,
    0,
    140,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPN_TWO_STEP_TS_VALUE_LO_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_TWO_STEP_TS_VALUE_HI
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_TWO_STEP_TS_VALUE_HI_FIELDS[] =
{
    &EPN_TWO_STEP_TS_VALUE_HI_RESERVED0_FIELD,
    &EPN_TWO_STEP_TS_VALUE_HI_TWOSTEPTIMESTAMP_HI_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_TWO_STEP_TS_VALUE_HI_REG = 
{
    "TWO_STEP_TS_VALUE_HI",
#if RU_INCLUDE_DESC
    "EPN_TWO_STEP_TS_VALUE_HI Register",
    "Upper 16-bits of 48-bits timestamp value, applicable for two step"
    "timestamping.",
#endif
    EPN_TWO_STEP_TS_VALUE_HI_REG_OFFSET,
    0,
    0,
    141,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_TWO_STEP_TS_VALUE_HI_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_1588_TIMESTAMP_INT_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_1588_TIMESTAMP_INT_STATUS_FIELDS[] =
{
    &EPN_1588_TIMESTAMP_INT_STATUS_RESERVED0_FIELD,
    &EPN_1588_TIMESTAMP_INT_STATUS_INT1588PKTABORT_FIELD,
    &EPN_1588_TIMESTAMP_INT_STATUS_INT1588TWOSTEPFFINT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_1588_TIMESTAMP_INT_STATUS_REG = 
{
    "1588_TIMESTAMP_INT_STATUS",
#if RU_INCLUDE_DESC
    "EPN_1588_TIMESTAMP_INT_STATUS Register",
    "This register contains interrupt status for 1588 timestamp. These bits"
    "are sticky; to clear a bit, write 1 to it.",
#endif
    EPN_1588_TIMESTAMP_INT_STATUS_REG_OFFSET,
    0,
    0,
    142,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    EPN_1588_TIMESTAMP_INT_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_1588_TIMESTAMP_INT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_1588_TIMESTAMP_INT_MASK_FIELDS[] =
{
    &EPN_1588_TIMESTAMP_INT_MASK_RESERVED0_FIELD,
    &EPN_1588_TIMESTAMP_INT_MASK_TS1588PKTABORT_MASK_FIELD,
    &EPN_1588_TIMESTAMP_INT_MASK_TS1588TWOSTEPFF_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_1588_TIMESTAMP_INT_MASK_REG = 
{
    "1588_TIMESTAMP_INT_MASK",
#if RU_INCLUDE_DESC
    "EPN_1588_TIMESTAMP_INT_MASK Register",
    "This register contains interrupt mask for 1588 timestamp interrupts.",
#endif
    EPN_1588_TIMESTAMP_INT_MASK_REG_OFFSET,
    0,
    0,
    143,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    EPN_1588_TIMESTAMP_INT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_UP_PACKET_FETCH_MARGIN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_UP_PACKET_FETCH_MARGIN_FIELDS[] =
{
    &EPN_UP_PACKET_FETCH_MARGIN_RESERVED0_FIELD,
    &EPN_UP_PACKET_FETCH_MARGIN_UPPACKETFETCHMARGIN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_UP_PACKET_FETCH_MARGIN_REG = 
{
    "UP_PACKET_FETCH_MARGIN",
#if RU_INCLUDE_DESC
    "EPN_UP_PACKET_FETCH_MARGIN Register",
    "Specifies the setup time margin for the BBH to fetch upstream data to"
    "transfer to EPN."
    "This margin MUST be used when the Epn is provisioned to delay the burst"
    "termination while waiting for the Runner/BBH upstream queue status to"
    "be updated."
    "(see prvBbhQueStatDelay)"
    "The Power-On Reset default value of 0 will disable this delay."
    "The units are in TimeQuanta (TQ = 16nS).  The estimated DDR latency is"
    "2 [uS].  So, the minimum non-zero value for this register is 150 [TQ].",
#endif
    EPN_UP_PACKET_FETCH_MARGIN_REG_OFFSET,
    0,
    0,
    144,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_UP_PACKET_FETCH_MARGIN_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_DN_1588_TIMESTAMP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_DN_1588_TIMESTAMP_FIELDS[] =
{
    &EPN_DN_1588_TIMESTAMP_DN_1588_TS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_DN_1588_TIMESTAMP_REG = 
{
    "DN_1588_TIMESTAMP",
#if RU_INCLUDE_DESC
    "EPN_DN_1588_TIMESTAMP Register",
    "Provides real time indication of 1588 downstream timestamp.  A change"
    "in value indicates downstream traffic to BBH.",
#endif
    EPN_DN_1588_TIMESTAMP_REG_OFFSET,
    0,
    0,
    145,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPN_DN_1588_TIMESTAMP_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_PERSISTENT_REPORT_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_PERSISTENT_REPORT_CFG_FIELDS[] =
{
    &EPN_PERSISTENT_REPORT_CFG_RESERVED0_FIELD,
    &EPN_PERSISTENT_REPORT_CFG_CFGPERSRPTDURATION_FIELD,
    &EPN_PERSISTENT_REPORT_CFG_CFGPERSRPTTICKSIZE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_PERSISTENT_REPORT_CFG_REG = 
{
    "PERSISTENT_REPORT_CFG",
#if RU_INCLUDE_DESC
    "EPN_PERSISTENT_REPORT_CFG Register",
    "Specifies how long reports should persist.",
#endif
    EPN_PERSISTENT_REPORT_CFG_REG_OFFSET,
    0,
    0,
    146,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    EPN_PERSISTENT_REPORT_CFG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_PERSISTENT_REPORT_ENABLES
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_PERSISTENT_REPORT_ENABLES_FIELDS[] =
{
    &EPN_PERSISTENT_REPORT_ENABLES_RESERVED0_FIELD,
    &EPN_PERSISTENT_REPORT_ENABLES_CFGPERSRPTENABLE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_PERSISTENT_REPORT_ENABLES_REG = 
{
    "PERSISTENT_REPORT_ENABLES",
#if RU_INCLUDE_DESC
    "EPN_PERSISTENT_REPORT_ENABLES Register",
    "Per LLID enable for persistent reporting.",
#endif
    EPN_PERSISTENT_REPORT_ENABLES_REG_OFFSET,
    0,
    0,
    147,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_PERSISTENT_REPORT_ENABLES_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_PERSISTENT_REPORT_REQUEST_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_PERSISTENT_REPORT_REQUEST_SIZE_FIELDS[] =
{
    &EPN_PERSISTENT_REPORT_REQUEST_SIZE_RESERVED0_FIELD,
    &EPN_PERSISTENT_REPORT_REQUEST_SIZE_CFGPERSRPTREQTQ_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_PERSISTENT_REPORT_REQUEST_SIZE_REG = 
{
    "PERSISTENT_REPORT_REQUEST_SIZE",
#if RU_INCLUDE_DESC
    "EPN_PERSISTENT_REPORT_REQUEST_SIZE Register",
    "How many Time Quanta the persistent report should request.",
#endif
    EPN_PERSISTENT_REPORT_REQUEST_SIZE_REG_OFFSET,
    0,
    0,
    148,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_PERSISTENT_REPORT_REQUEST_SIZE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: EPN
 ******************************************************************************/
static const ru_reg_rec *EPN_REGS[] =
{
    &EPN_CONTROL_0_REG,
    &EPN_CONTROL_1_REG,
    &EPN_ENABLE_GRANTS_REG,
    &EPN_DROP_DISC_GATES_REG,
    &EPN_DIS_FCS_CHK_REG,
    &EPN_PASS_GATES_REG,
    &EPN_CFG_MISALGN_FB_REG,
    &EPN_DISCOVERY_FILTER_REG,
    &EPN_MINIMUM_GRANT_SETUP_REG,
    &EPN_RESET_GNT_FIFO_REG,
    &EPN_RESET_L1_ACCUMULATOR_REG,
    &EPN_L1_ACCUMULATOR_SEL_REG,
    &EPN_L1_SVA_BYTES_REG,
    &EPN_L1_UVA_BYTES_REG,
    &EPN_L1_SVA_OVERFLOW_REG,
    &EPN_L1_UVA_OVERFLOW_REG,
    &EPN_RESET_RPT_PRI_REG,
    &EPN_RESET_L2_RPT_FIFO_REG,
    &EPN_ENABLE_UPSTREAM_REG,
    &EPN_ENABLE_UPSTREAM_FB_REG,
    &EPN_ENABLE_UPSTREAM_FEC_REG,
    &EPN_REPORT_BYTE_LENGTH_REG,
    &EPN_MAIN_INT_STATUS_REG,
    &EPN_GNT_FULL_INT_STATUS_REG,
    &EPN_GNT_FULL_INT_MASK_REG,
    &EPN_GNT_MISS_INT_STATUS_REG,
    &EPN_GNT_MISS_INT_MASK_REG,
    &EPN_DISC_RX_INT_STATUS_REG,
    &EPN_DISC_RX_INT_MASK_REG,
    &EPN_GNT_INTV_INT_STATUS_REG,
    &EPN_GNT_INTV_INT_MASK_REG,
    &EPN_GNT_FAR_INT_STATUS_REG,
    &EPN_GNT_FAR_INT_MASK_REG,
    &EPN_GNT_MISALGN_INT_STATUS_REG,
    &EPN_GNT_MISALGN_INT_MASK_REG,
    &EPN_NP_GNT_INT_STATUS_REG,
    &EPN_NP_GNT_INT_MASK_REG,
    &EPN_DEL_STALE_INT_STATUS_REG,
    &EPN_DEL_STALE_INT_MASK_REG,
    &EPN_GNT_PRES_INT_STATUS_REG,
    &EPN_GNT_PRES_INT_MASK_REG,
    &EPN_RPT_PRES_INT_STATUS_REG,
    &EPN_RPT_PRES_INT_MASK_REG,
    &EPN_DRX_ABORT_INT_STATUS_REG,
    &EPN_DRX_ABORT_INT_MASK_REG,
    &EPN_EMPTY_RPT_INT_STATUS_REG,
    &EPN_EMPTY_RPT_INT_MASK_REG,
    &EPN_BCAP_OVERFLOW_INT_STATUS_REG,
    &EPN_BCAP_OVERFLOW_INT_MASK_REG,
    &EPN_BBH_DNS_FAULT_INT_STATUS_REG,
    &EPN_BBH_DNS_FAULT_INT_MASK_REG,
    &EPN_BBH_UPS_FAULT_INT_STATUS_REG,
    &EPN_BBH_UPS_FAULT_INT_MASK_REG,
    &EPN_BBH_UPS_ABORT_INT_STATUS_REG,
    &EPN_BBH_UPS_ABORT_INT_MASK_REG,
    &EPN_MAIN_INT_MASK_REG,
    &EPN_MAX_GNT_SIZE_REG,
    &EPN_MAX_FRAME_SIZE_REG,
    &EPN_GRANT_OVR_HD_REG,
    &EPN_POLL_SIZE_REG,
    &EPN_DN_RD_GNT_MARGIN_REG,
    &EPN_GNT_TIME_START_DELTA_REG,
    &EPN_TIME_STAMP_DIFF_REG,
    &EPN_UP_TIME_STAMP_OFF_REG,
    &EPN_GNT_INTERVAL_REG,
    &EPN_DN_GNT_MISALIGN_THR_REG,
    &EPN_DN_GNT_MISALIGN_PAUSE_REG,
    &EPN_NON_POLL_INTV_REG,
    &EPN_FORCE_FCS_ERR_REG,
    &EPN_GRANT_OVERLAP_LIMIT_REG,
    &EPN_AES_CONFIGURATION_0_REG,
    &EPN_DISC_GRANT_OVR_HD_REG,
    &EPN_DN_DISCOVERY_SEED_REG,
    &EPN_DN_DISCOVERY_INC_REG,
    &EPN_DN_DISCOVERY_SIZE_REG,
    &EPN_FEC_IPG_LENGTH_REG,
    &EPN_FAKE_REPORT_VALUE_EN_REG,
    &EPN_FAKE_REPORT_VALUE_REG,
    &EPN_BURST_CAP__REG,
    &EPN_QUEUE_LLID_MAP__REG,
    &EPN_VALID_OPCODE_MAP_REG,
    &EPN_UP_PACKET_TX_MARGIN_REG,
    &EPN_MULTI_PRI_CFG_0_REG,
    &EPN_SHARED_BCAP_OVRFLOW_REG,
    &EPN_FORCED_REPORT_EN_REG,
    &EPN_FORCED_REPORT_MAX_INTERVAL_REG,
    &EPN_L2S_FLUSH_CONFIG_REG,
    &EPN_DATA_PORT_COMMAND_REG,
    &EPN_DATA_PORT_ADDRESS_REG,
    &EPN_DATA_PORT_DATA_0_REG,
    &EPN_UNMAP_BIG_CNT_REG,
    &EPN_UNMAP_FRAME_CNT_REG,
    &EPN_UNMAP_FCS_CNT_REG,
    &EPN_UNMAP_GATE_CNT_REG,
    &EPN_UNMAP_OAM_CNT_REG,
    &EPN_UNMAP_SMALL_CNT_REG,
    &EPN_FIF_DEQUEUE_EVENT_CNT_REG,
    &EPN_UNUSED_TQ_CNT_REG,
    &EPN_BBH_UP_FAULT_HALT_EN_REG,
    &EPN_BBH_UP_TARDY_HALT_EN_REG,
    &EPN_DEBUG_STATUS_0_REG,
    &EPN_DEBUG_STATUS_1_REG,
    &EPN_DEBUG_L2S_PTR_SEL_REG,
    &EPN_OLT_MAC_ADDR_LO_REG,
    &EPN_OLT_MAC_ADDR_HI_REG,
    &EPN_TX_L1S_SHP_DQU_EMPTY_REG,
    &EPN_TX_L1S_UNSHAPED_EMPTY_REG,
    &EPN_TX_L1S_SHP_QUE_MASK__REG,
    &EPN_TX_L2S_QUE_CONFIG__REG,
    &EPN_TX_L2S_QUE_EMPTY_REG,
    &EPN_TX_L2S_QUE_FULL_REG,
    &EPN_TX_L2S_QUE_STOPPED_REG,
    &EPN_TX_CTC_BURST_LIMIT__REG,
    &EPN_BBH_MAX_OUTSTANDING_TARDY_PACKETS_REG,
    &EPN_MIN_REPORT_VALUE_DIFFERENCE_REG,
    &EPN_BBH_STATUS_FIFO_OVERFLOW_REG,
    &EPN_SPARE_CTL_REG,
    &EPN_TS_SYNC_OFFSET_REG,
    &EPN_DN_TS_OFFSET_REG,
    &EPN_UP_TS_OFFSET_LO_REG,
    &EPN_UP_TS_OFFSET_HI_REG,
    &EPN_TWO_STEP_TS_CTL_REG,
    &EPN_TWO_STEP_TS_VALUE_LO_REG,
    &EPN_TWO_STEP_TS_VALUE_HI_REG,
    &EPN_1588_TIMESTAMP_INT_STATUS_REG,
    &EPN_1588_TIMESTAMP_INT_MASK_REG,
    &EPN_UP_PACKET_FETCH_MARGIN_REG,
    &EPN_DN_1588_TIMESTAMP_REG,
    &EPN_PERSISTENT_REPORT_CFG_REG,
    &EPN_PERSISTENT_REPORT_ENABLES_REG,
    &EPN_PERSISTENT_REPORT_REQUEST_SIZE_REG,
};

static unsigned long EPN_ADDRS[] =
{
    0x82dad000,
};

const ru_block_rec EPN_BLOCK = 
{
    "EPN",
    EPN_ADDRS,
    1,
    131,
    EPN_REGS
};

/* End of file EPON_EPN.c */
