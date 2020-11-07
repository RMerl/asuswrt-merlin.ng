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
 * Field: XPCSTX_TX_CONTROL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_CONTROL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSTX_TX_CONTROL_RESERVED0_FIELD_MASK,
    0,
    XPCSTX_TX_CONTROL_RESERVED0_FIELD_WIDTH,
    XPCSTX_TX_CONTROL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_CONTROL_CFGENRMTFAULTDET125
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_CONTROL_CFGENRMTFAULTDET125_FIELD =
{
    "CFGENRMTFAULTDET125",
#if RU_INCLUDE_DESC
    "",
    "Enable remote fault detection.",
#endif
    XPCSTX_TX_CONTROL_CFGENRMTFAULTDET125_FIELD_MASK,
    0,
    XPCSTX_TX_CONTROL_CFGENRMTFAULTDET125_FIELD_WIDTH,
    XPCSTX_TX_CONTROL_CFGENRMTFAULTDET125_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_CONTROL_CFGLSRTRISTATEEN125
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_CONTROL_CFGLSRTRISTATEEN125_FIELD =
{
    "CFGLSRTRISTATEEN125",
#if RU_INCLUDE_DESC
    "",
    "Enable the laser enable tri-state output.",
#endif
    XPCSTX_TX_CONTROL_CFGLSRTRISTATEEN125_FIELD_MASK,
    0,
    XPCSTX_TX_CONTROL_CFGLSRTRISTATEEN125_FIELD_WIDTH,
    XPCSTX_TX_CONTROL_CFGLSRTRISTATEEN125_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_CONTROL_CFGENSEQNUM125
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_CONTROL_CFGENSEQNUM125_FIELD =
{
    "CFGENSEQNUM125",
#if RU_INCLUDE_DESC
    "",
    "Debug function has been deprecated. It is now utilized to enable"
    "IDLE packet support :  0 - enable; 1 - disable.",
#endif
    XPCSTX_TX_CONTROL_CFGENSEQNUM125_FIELD_MASK,
    0,
    XPCSTX_TX_CONTROL_CFGENSEQNUM125_FIELD_WIDTH,
    XPCSTX_TX_CONTROL_CFGENSEQNUM125_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_CONTROL_RESERVED1
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_CONTROL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSTX_TX_CONTROL_RESERVED1_FIELD_MASK,
    0,
    XPCSTX_TX_CONTROL_RESERVED1_FIELD_WIDTH,
    XPCSTX_TX_CONTROL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_CONTROL_CFGENSCRMBCONT125
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_CONTROL_CFGENSCRMBCONT125_FIELD =
{
    "CFGENSCRMBCONT125",
#if RU_INCLUDE_DESC
    "",
    "Enable the scrambler to run continously. 0 - Only during burst; 1 -"
    "Continuosly.",
#endif
    XPCSTX_TX_CONTROL_CFGENSCRMBCONT125_FIELD_MASK,
    0,
    XPCSTX_TX_CONTROL_CFGENSCRMBCONT125_FIELD_WIDTH,
    XPCSTX_TX_CONTROL_CFGENSCRMBCONT125_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_CONTROL_CFGLSRENACTHI125
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_CONTROL_CFGLSRENACTHI125_FIELD =
{
    "CFGLSRENACTHI125",
#if RU_INCLUDE_DESC
    "",
    "Laser on polarity : 0 - laser on active low; 1 - laser on active"
    "high.",
#endif
    XPCSTX_TX_CONTROL_CFGLSRENACTHI125_FIELD_MASK,
    0,
    XPCSTX_TX_CONTROL_CFGLSRENACTHI125_FIELD_WIDTH,
    XPCSTX_TX_CONTROL_CFGLSRENACTHI125_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_CONTROL_CFGENLSRALWAYS125
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_CONTROL_CFGENLSRALWAYS125_FIELD =
{
    "CFGENLSRALWAYS125",
#if RU_INCLUDE_DESC
    "",
    "Enable laser on always.",
#endif
    XPCSTX_TX_CONTROL_CFGENLSRALWAYS125_FIELD_MASK,
    0,
    XPCSTX_TX_CONTROL_CFGENLSRALWAYS125_FIELD_WIDTH,
    XPCSTX_TX_CONTROL_CFGENLSRALWAYS125_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_CONTROL_CFGENLSRTILENDSLOT125
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_CONTROL_CFGENLSRTILENDSLOT125_FIELD =
{
    "CFGENLSRTILENDSLOT125",
#if RU_INCLUDE_DESC
    "",
    "Enable laser until the end-of-grant, non-strict IEEE mode.",
#endif
    XPCSTX_TX_CONTROL_CFGENLSRTILENDSLOT125_FIELD_MASK,
    0,
    XPCSTX_TX_CONTROL_CFGENLSRTILENDSLOT125_FIELD_WIDTH,
    XPCSTX_TX_CONTROL_CFGENLSRTILENDSLOT125_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_CONTROL_CFGTXOUTBYTEFLIP125
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_CONTROL_CFGTXOUTBYTEFLIP125_FIELD =
{
    "CFGTXOUTBYTEFLIP125",
#if RU_INCLUDE_DESC
    "",
    "Flip Gearbox's byte output to SERDES.",
#endif
    XPCSTX_TX_CONTROL_CFGTXOUTBYTEFLIP125_FIELD_MASK,
    0,
    XPCSTX_TX_CONTROL_CFGTXOUTBYTEFLIP125_FIELD_WIDTH,
    XPCSTX_TX_CONTROL_CFGTXOUTBYTEFLIP125_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_CONTROL_CFGENTXOUT125
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_CONTROL_CFGENTXOUT125_FIELD =
{
    "CFGENTXOUT125",
#if RU_INCLUDE_DESC
    "",
    "Enable transmit Gearbox's output. 0 - disable transmit; 1 - enable"
    "transmit.",
#endif
    XPCSTX_TX_CONTROL_CFGENTXOUT125_FIELD_MASK,
    0,
    XPCSTX_TX_CONTROL_CFGENTXOUT125_FIELD_WIDTH,
    XPCSTX_TX_CONTROL_CFGENTXOUT125_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_CONTROL_CFGENTXSCRB125
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_CONTROL_CFGENTXSCRB125_FIELD =
{
    "CFGENTXSCRB125",
#if RU_INCLUDE_DESC
    "",
    "Enable transmit scrambler. 0 - disable scrambler; 1 - enable"
    "scrambler.",
#endif
    XPCSTX_TX_CONTROL_CFGENTXSCRB125_FIELD_MASK,
    0,
    XPCSTX_TX_CONTROL_CFGENTXSCRB125_FIELD_WIDTH,
    XPCSTX_TX_CONTROL_CFGENTXSCRB125_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_CONTROL_CFGENTXFEC125
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_CONTROL_CFGENTXFEC125_FIELD =
{
    "CFGENTXFEC125",
#if RU_INCLUDE_DESC
    "",
    "Enables upstream FEC : 0 - nonFEC; 1 - FEC.",
#endif
    XPCSTX_TX_CONTROL_CFGENTXFEC125_FIELD_MASK,
    0,
    XPCSTX_TX_CONTROL_CFGENTXFEC125_FIELD_WIDTH,
    XPCSTX_TX_CONTROL_CFGENTXFEC125_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_CONTROL_PCSTXNOTRDY
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_CONTROL_PCSTXNOTRDY_FIELD =
{
    "PCSTXNOTRDY",
#if RU_INCLUDE_DESC
    "",
    "Indicates XPCS-TX not ready for operation.",
#endif
    XPCSTX_TX_CONTROL_PCSTXNOTRDY_FIELD_MASK,
    0,
    XPCSTX_TX_CONTROL_PCSTXNOTRDY_FIELD_WIDTH,
    XPCSTX_TX_CONTROL_PCSTXNOTRDY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_CONTROL_RESERVED2
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_CONTROL_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSTX_TX_CONTROL_RESERVED2_FIELD_MASK,
    0,
    XPCSTX_TX_CONTROL_RESERVED2_FIELD_WIDTH,
    XPCSTX_TX_CONTROL_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_CONTROL_PCSTXDTPORTRSTN
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_CONTROL_PCSTXDTPORTRSTN_FIELD =
{
    "PCSTXDTPORTRSTN",
#if RU_INCLUDE_DESC
    "",
    "Active low reset for RAM data port. RAM init starts upon"
    "deassertion.  Bit pcstxNotRdy is to be polled for completion.",
#endif
    XPCSTX_TX_CONTROL_PCSTXDTPORTRSTN_FIELD_MASK,
    0,
    XPCSTX_TX_CONTROL_PCSTXDTPORTRSTN_FIELD_WIDTH,
    XPCSTX_TX_CONTROL_PCSTXDTPORTRSTN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_CONTROL_PCSTXRSTN
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_CONTROL_PCSTXRSTN_FIELD =
{
    "PCSTXRSTN",
#if RU_INCLUDE_DESC
    "",
    "Reset control for XPCS-TX module. 0 - Reset. 1 - Normal Operation.",
#endif
    XPCSTX_TX_CONTROL_PCSTXRSTN_FIELD_MASK,
    0,
    XPCSTX_TX_CONTROL_PCSTXRSTN_FIELD_WIDTH,
    XPCSTX_TX_CONTROL_PCSTXRSTN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_INT_STAT_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_INT_STAT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSTX_TX_INT_STAT_RESERVED0_FIELD_MASK,
    0,
    XPCSTX_TX_INT_STAT_RESERVED0_FIELD_WIDTH,
    XPCSTX_TX_INT_STAT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_INT_STAT_LASERONMAX
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_INT_STAT_LASERONMAX_FIELD =
{
    "LASERONMAX",
#if RU_INCLUDE_DESC
    "",
    "Indicates laser enable on time exceeed the maximum threshold, as"
    "defined by register XPCS_TX_LASER_MONITOR_MAX_THRESH.",
#endif
    XPCSTX_TX_INT_STAT_LASERONMAX_FIELD_MASK,
    0,
    XPCSTX_TX_INT_STAT_LASERONMAX_FIELD_WIDTH,
    XPCSTX_TX_INT_STAT_LASERONMAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_INT_STAT_LASEROFF
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_INT_STAT_LASEROFF_FIELD =
{
    "LASEROFF",
#if RU_INCLUDE_DESC
    "",
    "Indicates laser enable deassertion.",
#endif
    XPCSTX_TX_INT_STAT_LASEROFF_FIELD_MASK,
    0,
    XPCSTX_TX_INT_STAT_LASEROFF_FIELD_WIDTH,
    XPCSTX_TX_INT_STAT_LASEROFF_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_INT_STAT_GRANTLAGERR
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_INT_STAT_GRANTLAGERR_FIELD =
{
    "GRANTLAGERR",
#if RU_INCLUDE_DESC
    "",
    "[FATAL] Indicates scheduled time lagged current MPCP time.",
#endif
    XPCSTX_TX_INT_STAT_GRANTLAGERR_FIELD_MASK,
    0,
    XPCSTX_TX_INT_STAT_GRANTLAGERR_FIELD_WIDTH,
    XPCSTX_TX_INT_STAT_GRANTLAGERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_INT_STAT_RESERVED1
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_INT_STAT_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSTX_TX_INT_STAT_RESERVED1_FIELD_MASK,
    0,
    XPCSTX_TX_INT_STAT_RESERVED1_FIELD_WIDTH,
    XPCSTX_TX_INT_STAT_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_INT_STAT_BACK2BACKGNT
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_INT_STAT_BACK2BACKGNT_FIELD =
{
    "BACK2BACKGNT",
#if RU_INCLUDE_DESC
    "",
    "[NON-FATAL] Indicates back-2-back grants.",
#endif
    XPCSTX_TX_INT_STAT_BACK2BACKGNT_FIELD_MASK,
    0,
    XPCSTX_TX_INT_STAT_BACK2BACKGNT_FIELD_WIDTH,
    XPCSTX_TX_INT_STAT_BACK2BACKGNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_INT_STAT_FECUNDERRUN
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_INT_STAT_FECUNDERRUN_FIELD =
{
    "FECUNDERRUN",
#if RU_INCLUDE_DESC
    "",
    "[FATAL] FEC transmit FIFO underrun.",
#endif
    XPCSTX_TX_INT_STAT_FECUNDERRUN_FIELD_MASK,
    0,
    XPCSTX_TX_INT_STAT_FECUNDERRUN_FIELD_WIDTH,
    XPCSTX_TX_INT_STAT_FECUNDERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_INT_STAT_GEARBOXUNDERRUN
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_INT_STAT_GEARBOXUNDERRUN_FIELD =
{
    "GEARBOXUNDERRUN",
#if RU_INCLUDE_DESC
    "",
    "[FATAL] Gearbox underrun.",
#endif
    XPCSTX_TX_INT_STAT_GEARBOXUNDERRUN_FIELD_MASK,
    0,
    XPCSTX_TX_INT_STAT_GEARBOXUNDERRUN_FIELD_WIDTH,
    XPCSTX_TX_INT_STAT_GEARBOXUNDERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_INT_STAT_GNTTOOSHORT
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_INT_STAT_GNTTOOSHORT_FIELD =
{
    "GNTTOOSHORT",
#if RU_INCLUDE_DESC
    "",
    "[NON-FATAL] Indicates grant slot is too short for transfer.",
#endif
    XPCSTX_TX_INT_STAT_GNTTOOSHORT_FIELD_MASK,
    0,
    XPCSTX_TX_INT_STAT_GNTTOOSHORT_FIELD_WIDTH,
    XPCSTX_TX_INT_STAT_GNTTOOSHORT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_INT_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_INT_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSTX_TX_INT_MASK_RESERVED0_FIELD_MASK,
    0,
    XPCSTX_TX_INT_MASK_RESERVED0_FIELD_WIDTH,
    XPCSTX_TX_INT_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_INT_MASK_LASERONMAXMASK
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_INT_MASK_LASERONMAXMASK_FIELD =
{
    "LASERONMAXMASK",
#if RU_INCLUDE_DESC
    "",
    "Interrupt mask, active low.",
#endif
    XPCSTX_TX_INT_MASK_LASERONMAXMASK_FIELD_MASK,
    0,
    XPCSTX_TX_INT_MASK_LASERONMAXMASK_FIELD_WIDTH,
    XPCSTX_TX_INT_MASK_LASERONMAXMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_INT_MASK_LASEROFFMASK
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_INT_MASK_LASEROFFMASK_FIELD =
{
    "LASEROFFMASK",
#if RU_INCLUDE_DESC
    "",
    "Interrupt mask, active low.",
#endif
    XPCSTX_TX_INT_MASK_LASEROFFMASK_FIELD_MASK,
    0,
    XPCSTX_TX_INT_MASK_LASEROFFMASK_FIELD_WIDTH,
    XPCSTX_TX_INT_MASK_LASEROFFMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_INT_MASK_GRANTLAGERRMSK
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_INT_MASK_GRANTLAGERRMSK_FIELD =
{
    "GRANTLAGERRMSK",
#if RU_INCLUDE_DESC
    "",
    "Interrupt mask, active low.",
#endif
    XPCSTX_TX_INT_MASK_GRANTLAGERRMSK_FIELD_MASK,
    0,
    XPCSTX_TX_INT_MASK_GRANTLAGERRMSK_FIELD_WIDTH,
    XPCSTX_TX_INT_MASK_GRANTLAGERRMSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_INT_MASK_RESERVED1
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_INT_MASK_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSTX_TX_INT_MASK_RESERVED1_FIELD_MASK,
    0,
    XPCSTX_TX_INT_MASK_RESERVED1_FIELD_WIDTH,
    XPCSTX_TX_INT_MASK_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_INT_MASK_BACK2BCKGNTMSK
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_INT_MASK_BACK2BCKGNTMSK_FIELD =
{
    "BACK2BCKGNTMSK",
#if RU_INCLUDE_DESC
    "",
    "Interrupt mask, active low.",
#endif
    XPCSTX_TX_INT_MASK_BACK2BCKGNTMSK_FIELD_MASK,
    0,
    XPCSTX_TX_INT_MASK_BACK2BCKGNTMSK_FIELD_WIDTH,
    XPCSTX_TX_INT_MASK_BACK2BCKGNTMSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_INT_MASK_FECUNDERRUNMSK
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_INT_MASK_FECUNDERRUNMSK_FIELD =
{
    "FECUNDERRUNMSK",
#if RU_INCLUDE_DESC
    "",
    "Interrupt mask, active low.",
#endif
    XPCSTX_TX_INT_MASK_FECUNDERRUNMSK_FIELD_MASK,
    0,
    XPCSTX_TX_INT_MASK_FECUNDERRUNMSK_FIELD_WIDTH,
    XPCSTX_TX_INT_MASK_FECUNDERRUNMSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_INT_MASK_GEARBOXUNDERRUNMSK
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_INT_MASK_GEARBOXUNDERRUNMSK_FIELD =
{
    "GEARBOXUNDERRUNMSK",
#if RU_INCLUDE_DESC
    "",
    "Interrupt mask, active low.",
#endif
    XPCSTX_TX_INT_MASK_GEARBOXUNDERRUNMSK_FIELD_MASK,
    0,
    XPCSTX_TX_INT_MASK_GEARBOXUNDERRUNMSK_FIELD_WIDTH,
    XPCSTX_TX_INT_MASK_GEARBOXUNDERRUNMSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_INT_MASK_GNTTOOSHORTMSK
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_INT_MASK_GNTTOOSHORTMSK_FIELD =
{
    "GNTTOOSHORTMSK",
#if RU_INCLUDE_DESC
    "",
    "Interrupt mask, active low.",
#endif
    XPCSTX_TX_INT_MASK_GNTTOOSHORTMSK_FIELD_MASK,
    0,
    XPCSTX_TX_INT_MASK_GNTTOOSHORTMSK_FIELD_WIDTH,
    XPCSTX_TX_INT_MASK_GNTTOOSHORTMSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_PORT_COMMAND_DATAPORTBUSY
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_PORT_COMMAND_DATAPORTBUSY_FIELD =
{
    "DATAPORTBUSY",
#if RU_INCLUDE_DESC
    "",
    "Indicates dataPort is in progress.  Bit must be cleared before the"
    "next dataPort access can be issued.",
#endif
    XPCSTX_TX_PORT_COMMAND_DATAPORTBUSY_FIELD_MASK,
    0,
    XPCSTX_TX_PORT_COMMAND_DATAPORTBUSY_FIELD_WIDTH,
    XPCSTX_TX_PORT_COMMAND_DATAPORTBUSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_PORT_COMMAND_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_PORT_COMMAND_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSTX_TX_PORT_COMMAND_RESERVED0_FIELD_MASK,
    0,
    XPCSTX_TX_PORT_COMMAND_RESERVED0_FIELD_WIDTH,
    XPCSTX_TX_PORT_COMMAND_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_PORT_COMMAND_PORTSELECT
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_PORT_COMMAND_PORTSELECT_FIELD =
{
    "PORTSELECT",
#if RU_INCLUDE_DESC
    "",
    "Selects the RAM for access : 0 - FEC vector RAM.",
#endif
    XPCSTX_TX_PORT_COMMAND_PORTSELECT_FIELD_MASK,
    0,
    XPCSTX_TX_PORT_COMMAND_PORTSELECT_FIELD_WIDTH,
    XPCSTX_TX_PORT_COMMAND_PORTSELECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_PORT_COMMAND_PORTOPCODE
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_PORT_COMMAND_PORTOPCODE_FIELD =
{
    "PORTOPCODE",
#if RU_INCLUDE_DESC
    "",
    "Indicates write access : 0 - read; 1 - write.",
#endif
    XPCSTX_TX_PORT_COMMAND_PORTOPCODE_FIELD_MASK,
    0,
    XPCSTX_TX_PORT_COMMAND_PORTOPCODE_FIELD_WIDTH,
    XPCSTX_TX_PORT_COMMAND_PORTOPCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_PORT_COMMAND_PORTADDRESS
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_PORT_COMMAND_PORTADDRESS_FIELD =
{
    "PORTADDRESS",
#if RU_INCLUDE_DESC
    "",
    "Specifies the RAM address for access.",
#endif
    XPCSTX_TX_PORT_COMMAND_PORTADDRESS_FIELD_MASK,
    0,
    XPCSTX_TX_PORT_COMMAND_PORTADDRESS_FIELD_WIDTH,
    XPCSTX_TX_PORT_COMMAND_PORTADDRESS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_DATA_PORT_0_PORTDATA0
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_DATA_PORT_0_PORTDATA0_FIELD =
{
    "PORTDATA0",
#if RU_INCLUDE_DESC
    "",
    "XPCSTX_VEC_RAM[31:0].",
#endif
    XPCSTX_TX_DATA_PORT_0_PORTDATA0_FIELD_MASK,
    0,
    XPCSTX_TX_DATA_PORT_0_PORTDATA0_FIELD_WIDTH,
    XPCSTX_TX_DATA_PORT_0_PORTDATA0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_DATA_PORT_1_PORTDATA1
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_DATA_PORT_1_PORTDATA1_FIELD =
{
    "PORTDATA1",
#if RU_INCLUDE_DESC
    "",
    "XPCSTX_VEC_RAM[63:32].",
#endif
    XPCSTX_TX_DATA_PORT_1_PORTDATA1_FIELD_MASK,
    0,
    XPCSTX_TX_DATA_PORT_1_PORTDATA1_FIELD_WIDTH,
    XPCSTX_TX_DATA_PORT_1_PORTDATA1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_DATA_PORT_2_PORTDATA2
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_DATA_PORT_2_PORTDATA2_FIELD =
{
    "PORTDATA2",
#if RU_INCLUDE_DESC
    "",
    "[1:0] - XPCSTX_VEC_RAM[66:65].",
#endif
    XPCSTX_TX_DATA_PORT_2_PORTDATA2_FIELD_MASK,
    0,
    XPCSTX_TX_DATA_PORT_2_PORTDATA2_FIELD_WIDTH,
    XPCSTX_TX_DATA_PORT_2_PORTDATA2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_SYNC_PATT_CWORD_LO_CFGSYNCPATCWL
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_SYNC_PATT_CWORD_LO_CFGSYNCPATCWL_FIELD =
{
    "CFGSYNCPATCWL",
#if RU_INCLUDE_DESC
    "",
    "Defines the low order sync pattern codeword.",
#endif
    XPCSTX_TX_SYNC_PATT_CWORD_LO_CFGSYNCPATCWL_FIELD_MASK,
    0,
    XPCSTX_TX_SYNC_PATT_CWORD_LO_CFGSYNCPATCWL_FIELD_WIDTH,
    XPCSTX_TX_SYNC_PATT_CWORD_LO_CFGSYNCPATCWL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_SYNC_PATT_CWORD_HI_CFGSYNCPATCWH
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_SYNC_PATT_CWORD_HI_CFGSYNCPATCWH_FIELD =
{
    "CFGSYNCPATCWH",
#if RU_INCLUDE_DESC
    "",
    "Defines the high order sync pattern codeword.",
#endif
    XPCSTX_TX_SYNC_PATT_CWORD_HI_CFGSYNCPATCWH_FIELD_MASK,
    0,
    XPCSTX_TX_SYNC_PATT_CWORD_HI_CFGSYNCPATCWH_FIELD_WIDTH,
    XPCSTX_TX_SYNC_PATT_CWORD_HI_CFGSYNCPATCWH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_START_BURST_DEL_CWORD_LO_CFGSTRTBRSTDLMTRCWL
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_START_BURST_DEL_CWORD_LO_CFGSTRTBRSTDLMTRCWL_FIELD =
{
    "CFGSTRTBRSTDLMTRCWL",
#if RU_INCLUDE_DESC
    "",
    "Defines the low order start-of-burst delimiter codeword.",
#endif
    XPCSTX_TX_START_BURST_DEL_CWORD_LO_CFGSTRTBRSTDLMTRCWL_FIELD_MASK,
    0,
    XPCSTX_TX_START_BURST_DEL_CWORD_LO_CFGSTRTBRSTDLMTRCWL_FIELD_WIDTH,
    XPCSTX_TX_START_BURST_DEL_CWORD_LO_CFGSTRTBRSTDLMTRCWL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_START_BURST_DEL_CWORD_HI_CFGSTRTBRSTDLMTRCWH
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_START_BURST_DEL_CWORD_HI_CFGSTRTBRSTDLMTRCWH_FIELD =
{
    "CFGSTRTBRSTDLMTRCWH",
#if RU_INCLUDE_DESC
    "",
    "Defines the high order start-of-burst delimiter codeword.",
#endif
    XPCSTX_TX_START_BURST_DEL_CWORD_HI_CFGSTRTBRSTDLMTRCWH_FIELD_MASK,
    0,
    XPCSTX_TX_START_BURST_DEL_CWORD_HI_CFGSTRTBRSTDLMTRCWH_FIELD_WIDTH,
    XPCSTX_TX_START_BURST_DEL_CWORD_HI_CFGSTRTBRSTDLMTRCWH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_END_BURST_DEL_CWORD_LO_CFGENDBRSTDLMTRCWL
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_END_BURST_DEL_CWORD_LO_CFGENDBRSTDLMTRCWL_FIELD =
{
    "CFGENDBRSTDLMTRCWL",
#if RU_INCLUDE_DESC
    "",
    "Defines the low order terminating codeword.",
#endif
    XPCSTX_TX_END_BURST_DEL_CWORD_LO_CFGENDBRSTDLMTRCWL_FIELD_MASK,
    0,
    XPCSTX_TX_END_BURST_DEL_CWORD_LO_CFGENDBRSTDLMTRCWL_FIELD_WIDTH,
    XPCSTX_TX_END_BURST_DEL_CWORD_LO_CFGENDBRSTDLMTRCWL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_END_BURST_DEL_CWORD_HI_CFGENDBRSTDLMTRCWH
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_END_BURST_DEL_CWORD_HI_CFGENDBRSTDLMTRCWH_FIELD =
{
    "CFGENDBRSTDLMTRCWH",
#if RU_INCLUDE_DESC
    "",
    "Defines the high order terminating codeword.",
#endif
    XPCSTX_TX_END_BURST_DEL_CWORD_HI_CFGENDBRSTDLMTRCWH_FIELD_MASK,
    0,
    XPCSTX_TX_END_BURST_DEL_CWORD_HI_CFGENDBRSTDLMTRCWH_FIELD_WIDTH,
    XPCSTX_TX_END_BURST_DEL_CWORD_HI_CFGENDBRSTDLMTRCWH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_IDLE_CWORD_LO_CFGIDLECWL
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_IDLE_CWORD_LO_CFGIDLECWL_FIELD =
{
    "CFGIDLECWL",
#if RU_INCLUDE_DESC
    "",
    "Defines the low order codeword.",
#endif
    XPCSTX_TX_IDLE_CWORD_LO_CFGIDLECWL_FIELD_MASK,
    0,
    XPCSTX_TX_IDLE_CWORD_LO_CFGIDLECWL_FIELD_WIDTH,
    XPCSTX_TX_IDLE_CWORD_LO_CFGIDLECWL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_IDLE_CWORD_HI_CFGIDLECWH
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_IDLE_CWORD_HI_CFGIDLECWH_FIELD =
{
    "CFGIDLECWH",
#if RU_INCLUDE_DESC
    "",
    "Defines the high order codeword.",
#endif
    XPCSTX_TX_IDLE_CWORD_HI_CFGIDLECWH_FIELD_MASK,
    0,
    XPCSTX_TX_IDLE_CWORD_HI_CFGIDLECWH_FIELD_WIDTH,
    XPCSTX_TX_IDLE_CWORD_HI_CFGIDLECWH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_BURST_PATT_CWORD_LO_CFGBURSTPATCWL
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_BURST_PATT_CWORD_LO_CFGBURSTPATCWL_FIELD =
{
    "CFGBURSTPATCWL",
#if RU_INCLUDE_DESC
    "",
    "Defines the low order burst delimiter codeword.",
#endif
    XPCSTX_TX_BURST_PATT_CWORD_LO_CFGBURSTPATCWL_FIELD_MASK,
    0,
    XPCSTX_TX_BURST_PATT_CWORD_LO_CFGBURSTPATCWL_FIELD_WIDTH,
    XPCSTX_TX_BURST_PATT_CWORD_LO_CFGBURSTPATCWL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_BURST_PATT_CWORD_HI_CFGBURSTPATCWH
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_BURST_PATT_CWORD_HI_CFGBURSTPATCWH_FIELD =
{
    "CFGBURSTPATCWH",
#if RU_INCLUDE_DESC
    "",
    "Defines the high order burst delimiter codeword.",
#endif
    XPCSTX_TX_BURST_PATT_CWORD_HI_CFGBURSTPATCWH_FIELD_MASK,
    0,
    XPCSTX_TX_BURST_PATT_CWORD_HI_CFGBURSTPATCWH_FIELD_WIDTH,
    XPCSTX_TX_BURST_PATT_CWORD_HI_CFGBURSTPATCWH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_LASER_TIME_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_LASER_TIME_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSTX_TX_LASER_TIME_RESERVED0_FIELD_MASK,
    0,
    XPCSTX_TX_LASER_TIME_RESERVED0_FIELD_WIDTH,
    XPCSTX_TX_LASER_TIME_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_LASER_TIME_CFGLASERPIPE125
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_LASER_TIME_CFGLASERPIPE125_FIELD =
{
    "CFGLASERPIPE125",
#if RU_INCLUDE_DESC
    "",
    "Specifies the laser pipeline delay, in unit of 6.2ns (161 MHz"
    "clock).",
#endif
    XPCSTX_TX_LASER_TIME_CFGLASERPIPE125_FIELD_MASK,
    0,
    XPCSTX_TX_LASER_TIME_CFGLASERPIPE125_FIELD_WIDTH,
    XPCSTX_TX_LASER_TIME_CFGLASERPIPE125_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_LASER_TIME_RESERVED1
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_LASER_TIME_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSTX_TX_LASER_TIME_RESERVED1_FIELD_MASK,
    0,
    XPCSTX_TX_LASER_TIME_RESERVED1_FIELD_WIDTH,
    XPCSTX_TX_LASER_TIME_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_LASER_TIME_CFGLASEROFFDLYTQ125
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_LASER_TIME_CFGLASEROFFDLYTQ125_FIELD =
{
    "CFGLASEROFFDLYTQ125",
#if RU_INCLUDE_DESC
    "",
    "Specifies the laser off delay to the actual laser off time, value X."
    "Resulting delay = INT(X*2.5)*6.2 ns, based on 161 MHz clock.",
#endif
    XPCSTX_TX_LASER_TIME_CFGLASEROFFDLYTQ125_FIELD_MASK,
    0,
    XPCSTX_TX_LASER_TIME_CFGLASEROFFDLYTQ125_FIELD_WIDTH,
    XPCSTX_TX_LASER_TIME_CFGLASEROFFDLYTQ125_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_LASER_TIME_RESERVED2
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_LASER_TIME_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSTX_TX_LASER_TIME_RESERVED2_FIELD_MASK,
    0,
    XPCSTX_TX_LASER_TIME_RESERVED2_FIELD_WIDTH,
    XPCSTX_TX_LASER_TIME_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_LASER_TIME_CFGLASERONDLYTQ125
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_LASER_TIME_CFGLASERONDLYTQ125_FIELD =
{
    "CFGLASERONDLYTQ125",
#if RU_INCLUDE_DESC
    "",
    "Specifies the laser enable delay to the actual laser on time, value"
    "X.  Resulting delay = INT(X*2.5)*6.2 ns, based on 161 MHz clock .",
#endif
    XPCSTX_TX_LASER_TIME_CFGLASERONDLYTQ125_FIELD_MASK,
    0,
    XPCSTX_TX_LASER_TIME_CFGLASERONDLYTQ125_FIELD_WIDTH,
    XPCSTX_TX_LASER_TIME_CFGLASERONDLYTQ125_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_MAC_MODE_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_MAC_MODE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSTX_TX_MAC_MODE_RESERVED0_FIELD_MASK,
    0,
    XPCSTX_TX_MAC_MODE_RESERVED0_FIELD_WIDTH,
    XPCSTX_TX_MAC_MODE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_MAC_MODE_CFGENNOGNTXMT125
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_MAC_MODE_CFGENNOGNTXMT125_FIELD =
{
    "CFGENNOGNTXMT125",
#if RU_INCLUDE_DESC
    "",
    "Enable point-to-point transmission without grant.",
#endif
    XPCSTX_TX_MAC_MODE_CFGENNOGNTXMT125_FIELD_MASK,
    0,
    XPCSTX_TX_MAC_MODE_CFGENNOGNTXMT125_FIELD_WIDTH,
    XPCSTX_TX_MAC_MODE_CFGENNOGNTXMT125_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_LASER_MONITOR_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_LASER_MONITOR_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSTX_TX_LASER_MONITOR_CTL_RESERVED0_FIELD_MASK,
    0,
    XPCSTX_TX_LASER_MONITOR_CTL_RESERVED0_FIELD_WIDTH,
    XPCSTX_TX_LASER_MONITOR_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_LASER_MONITOR_CTL_LASERENSTATUS
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_LASER_MONITOR_CTL_LASERENSTATUS_FIELD =
{
    "LASERENSTATUS",
#if RU_INCLUDE_DESC
    "",
    "Status of laser enable, directly from I/O pin.",
#endif
    XPCSTX_TX_LASER_MONITOR_CTL_LASERENSTATUS_FIELD_MASK,
    0,
    XPCSTX_TX_LASER_MONITOR_CTL_LASERENSTATUS_FIELD_WIDTH,
    XPCSTX_TX_LASER_MONITOR_CTL_LASERENSTATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_LASER_MONITOR_CTL_CFGLSRMONACTHI
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_LASER_MONITOR_CTL_CFGLSRMONACTHI_FIELD =
{
    "CFGLSRMONACTHI",
#if RU_INCLUDE_DESC
    "",
    "Laser monitor polarity. 0 - active low; 1 - active high.",
#endif
    XPCSTX_TX_LASER_MONITOR_CTL_CFGLSRMONACTHI_FIELD_MASK,
    0,
    XPCSTX_TX_LASER_MONITOR_CTL_CFGLSRMONACTHI_FIELD_WIDTH,
    XPCSTX_TX_LASER_MONITOR_CTL_CFGLSRMONACTHI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_LASER_MONITOR_CTL_RESERVED1
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_LASER_MONITOR_CTL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSTX_TX_LASER_MONITOR_CTL_RESERVED1_FIELD_MASK,
    0,
    XPCSTX_TX_LASER_MONITOR_CTL_RESERVED1_FIELD_WIDTH,
    XPCSTX_TX_LASER_MONITOR_CTL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_LASER_MONITOR_CTL_LASERMONRSTN
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_LASER_MONITOR_CTL_LASERMONRSTN_FIELD =
{
    "LASERMONRSTN",
#if RU_INCLUDE_DESC
    "",
    "Laser monitor reset. 0 - Reset; 1 - Normal operation.",
#endif
    XPCSTX_TX_LASER_MONITOR_CTL_LASERMONRSTN_FIELD_MASK,
    0,
    XPCSTX_TX_LASER_MONITOR_CTL_LASERMONRSTN_FIELD_WIDTH,
    XPCSTX_TX_LASER_MONITOR_CTL_LASERMONRSTN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_LASER_MONITOR_MAX_THRESH_CFGLSRMONMAXTQ
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_LASER_MONITOR_MAX_THRESH_CFGLSRMONMAXTQ_FIELD =
{
    "CFGLSRMONMAXTQ",
#if RU_INCLUDE_DESC
    "",
    "Maximum assertion threshold, in unit of TQ.",
#endif
    XPCSTX_TX_LASER_MONITOR_MAX_THRESH_CFGLSRMONMAXTQ_FIELD_MASK,
    0,
    XPCSTX_TX_LASER_MONITOR_MAX_THRESH_CFGLSRMONMAXTQ_FIELD_WIDTH,
    XPCSTX_TX_LASER_MONITOR_MAX_THRESH_CFGLSRMONMAXTQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_LASER_MONITOR_BURST_LEN_LASERONLENGTH
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_LASER_MONITOR_BURST_LEN_LASERONLENGTH_FIELD =
{
    "LASERONLENGTH",
#if RU_INCLUDE_DESC
    "",
    "Indicates the laser-on time of the burst that set laserOff"
    "interrupt.",
#endif
    XPCSTX_TX_LASER_MONITOR_BURST_LEN_LASERONLENGTH_FIELD_MASK,
    0,
    XPCSTX_TX_LASER_MONITOR_BURST_LEN_LASERONLENGTH_FIELD_WIDTH,
    XPCSTX_TX_LASER_MONITOR_BURST_LEN_LASERONLENGTH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSTX_TX_LASER_MONITOR_BURST_COUNT_BURSTCNT
 ******************************************************************************/
const ru_field_rec XPCSTX_TX_LASER_MONITOR_BURST_COUNT_BURSTCNT_FIELD =
{
    "BURSTCNT",
#if RU_INCLUDE_DESC
    "",
    "This values increments on deassertion edge of laser enable. Peg at"
    "max value of 0xFFFFFFFF",
#endif
    XPCSTX_TX_LASER_MONITOR_BURST_COUNT_BURSTCNT_FIELD_MASK,
    0,
    XPCSTX_TX_LASER_MONITOR_BURST_COUNT_BURSTCNT_FIELD_WIDTH,
    XPCSTX_TX_LASER_MONITOR_BURST_COUNT_BURSTCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: XPCSTX_TX_CONTROL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_CONTROL_FIELDS[] =
{
    &XPCSTX_TX_CONTROL_RESERVED0_FIELD,
    &XPCSTX_TX_CONTROL_CFGENRMTFAULTDET125_FIELD,
    &XPCSTX_TX_CONTROL_CFGLSRTRISTATEEN125_FIELD,
    &XPCSTX_TX_CONTROL_CFGENSEQNUM125_FIELD,
    &XPCSTX_TX_CONTROL_RESERVED1_FIELD,
    &XPCSTX_TX_CONTROL_CFGENSCRMBCONT125_FIELD,
    &XPCSTX_TX_CONTROL_CFGLSRENACTHI125_FIELD,
    &XPCSTX_TX_CONTROL_CFGENLSRALWAYS125_FIELD,
    &XPCSTX_TX_CONTROL_CFGENLSRTILENDSLOT125_FIELD,
    &XPCSTX_TX_CONTROL_CFGTXOUTBYTEFLIP125_FIELD,
    &XPCSTX_TX_CONTROL_CFGENTXOUT125_FIELD,
    &XPCSTX_TX_CONTROL_CFGENTXSCRB125_FIELD,
    &XPCSTX_TX_CONTROL_CFGENTXFEC125_FIELD,
    &XPCSTX_TX_CONTROL_PCSTXNOTRDY_FIELD,
    &XPCSTX_TX_CONTROL_RESERVED2_FIELD,
    &XPCSTX_TX_CONTROL_PCSTXDTPORTRSTN_FIELD,
    &XPCSTX_TX_CONTROL_PCSTXRSTN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_CONTROL_REG = 
{
    "TX_CONTROL",
#if RU_INCLUDE_DESC
    "XPCS_TX_CONTROL Register",
    "XPCS-TX control register.",
#endif
    XPCSTX_TX_CONTROL_REG_OFFSET,
    0,
    0,
    729,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    17,
    XPCSTX_TX_CONTROL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSTX_TX_INT_STAT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_INT_STAT_FIELDS[] =
{
    &XPCSTX_TX_INT_STAT_RESERVED0_FIELD,
    &XPCSTX_TX_INT_STAT_LASERONMAX_FIELD,
    &XPCSTX_TX_INT_STAT_LASEROFF_FIELD,
    &XPCSTX_TX_INT_STAT_GRANTLAGERR_FIELD,
    &XPCSTX_TX_INT_STAT_RESERVED1_FIELD,
    &XPCSTX_TX_INT_STAT_BACK2BACKGNT_FIELD,
    &XPCSTX_TX_INT_STAT_FECUNDERRUN_FIELD,
    &XPCSTX_TX_INT_STAT_GEARBOXUNDERRUN_FIELD,
    &XPCSTX_TX_INT_STAT_GNTTOOSHORT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_INT_STAT_REG = 
{
    "TX_INT_STAT",
#if RU_INCLUDE_DESC
    "XPCS_TX_INT_STAT Register",
    "",
#endif
    XPCSTX_TX_INT_STAT_REG_OFFSET,
    0,
    0,
    730,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    XPCSTX_TX_INT_STAT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSTX_TX_INT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_INT_MASK_FIELDS[] =
{
    &XPCSTX_TX_INT_MASK_RESERVED0_FIELD,
    &XPCSTX_TX_INT_MASK_LASERONMAXMASK_FIELD,
    &XPCSTX_TX_INT_MASK_LASEROFFMASK_FIELD,
    &XPCSTX_TX_INT_MASK_GRANTLAGERRMSK_FIELD,
    &XPCSTX_TX_INT_MASK_RESERVED1_FIELD,
    &XPCSTX_TX_INT_MASK_BACK2BCKGNTMSK_FIELD,
    &XPCSTX_TX_INT_MASK_FECUNDERRUNMSK_FIELD,
    &XPCSTX_TX_INT_MASK_GEARBOXUNDERRUNMSK_FIELD,
    &XPCSTX_TX_INT_MASK_GNTTOOSHORTMSK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_INT_MASK_REG = 
{
    "TX_INT_MASK",
#if RU_INCLUDE_DESC
    "XPCS_TX_INT_MASK Register",
    "Interrupt masks, active low : 0 - mask interrupt; 1 - enable interrupt.",
#endif
    XPCSTX_TX_INT_MASK_REG_OFFSET,
    0,
    0,
    731,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    XPCSTX_TX_INT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSTX_TX_PORT_COMMAND
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_PORT_COMMAND_FIELDS[] =
{
    &XPCSTX_TX_PORT_COMMAND_DATAPORTBUSY_FIELD,
    &XPCSTX_TX_PORT_COMMAND_RESERVED0_FIELD,
    &XPCSTX_TX_PORT_COMMAND_PORTSELECT_FIELD,
    &XPCSTX_TX_PORT_COMMAND_PORTOPCODE_FIELD,
    &XPCSTX_TX_PORT_COMMAND_PORTADDRESS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_PORT_COMMAND_REG = 
{
    "TX_PORT_COMMAND",
#if RU_INCLUDE_DESC
    "XPCS_TX_PORT_COMMAND Register",
    "Provides dataPort read/write access to various RAMs.",
#endif
    XPCSTX_TX_PORT_COMMAND_REG_OFFSET,
    0,
    0,
    732,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XPCSTX_TX_PORT_COMMAND_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSTX_TX_DATA_PORT_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_DATA_PORT_0_FIELDS[] =
{
    &XPCSTX_TX_DATA_PORT_0_PORTDATA0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_DATA_PORT_0_REG = 
{
    "TX_DATA_PORT_0",
#if RU_INCLUDE_DESC
    "XPCS_TX_DATA_PORT_0 Register",
    "Stores the pre-write data for writing; and the post-read data for"
    "reading.",
#endif
    XPCSTX_TX_DATA_PORT_0_REG_OFFSET,
    0,
    0,
    733,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSTX_TX_DATA_PORT_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSTX_TX_DATA_PORT_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_DATA_PORT_1_FIELDS[] =
{
    &XPCSTX_TX_DATA_PORT_1_PORTDATA1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_DATA_PORT_1_REG = 
{
    "TX_DATA_PORT_1",
#if RU_INCLUDE_DESC
    "XPCS_TX_DATA_PORT_1 Register",
    "Stores the pre-write data for writing; and the post-read data for"
    "reading.",
#endif
    XPCSTX_TX_DATA_PORT_1_REG_OFFSET,
    0,
    0,
    734,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSTX_TX_DATA_PORT_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSTX_TX_DATA_PORT_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_DATA_PORT_2_FIELDS[] =
{
    &XPCSTX_TX_DATA_PORT_2_PORTDATA2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_DATA_PORT_2_REG = 
{
    "TX_DATA_PORT_2",
#if RU_INCLUDE_DESC
    "XPCS_TX_DATA_PORT_2 Register",
    "Stores the pre-write data for writing; and the post-read data for"
    "reading.",
#endif
    XPCSTX_TX_DATA_PORT_2_REG_OFFSET,
    0,
    0,
    735,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSTX_TX_DATA_PORT_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSTX_TX_SYNC_PATT_CWORD_LO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_SYNC_PATT_CWORD_LO_FIELDS[] =
{
    &XPCSTX_TX_SYNC_PATT_CWORD_LO_CFGSYNCPATCWL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_SYNC_PATT_CWORD_LO_REG = 
{
    "TX_SYNC_PATT_CWORD_LO",
#if RU_INCLUDE_DESC
    "XPCS_TX_SYNC_PATT_CWORD_LO Register",
    "Specifies the sync pattern codeword.",
#endif
    XPCSTX_TX_SYNC_PATT_CWORD_LO_REG_OFFSET,
    0,
    0,
    736,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSTX_TX_SYNC_PATT_CWORD_LO_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSTX_TX_SYNC_PATT_CWORD_HI
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_SYNC_PATT_CWORD_HI_FIELDS[] =
{
    &XPCSTX_TX_SYNC_PATT_CWORD_HI_CFGSYNCPATCWH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_SYNC_PATT_CWORD_HI_REG = 
{
    "TX_SYNC_PATT_CWORD_HI",
#if RU_INCLUDE_DESC
    "XPCS_TX_SYNC_PATT_CWORD_HI Register",
    "Specifies the sync pattern codeword.",
#endif
    XPCSTX_TX_SYNC_PATT_CWORD_HI_REG_OFFSET,
    0,
    0,
    737,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSTX_TX_SYNC_PATT_CWORD_HI_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSTX_TX_START_BURST_DEL_CWORD_LO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_START_BURST_DEL_CWORD_LO_FIELDS[] =
{
    &XPCSTX_TX_START_BURST_DEL_CWORD_LO_CFGSTRTBRSTDLMTRCWL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_START_BURST_DEL_CWORD_LO_REG = 
{
    "TX_START_BURST_DEL_CWORD_LO",
#if RU_INCLUDE_DESC
    "XPCS_TX_START_BURST_DEL_CWORD_LO Register",
    "Specifies the start-of-burst delimiter codeword.",
#endif
    XPCSTX_TX_START_BURST_DEL_CWORD_LO_REG_OFFSET,
    0,
    0,
    738,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSTX_TX_START_BURST_DEL_CWORD_LO_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSTX_TX_START_BURST_DEL_CWORD_HI
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_START_BURST_DEL_CWORD_HI_FIELDS[] =
{
    &XPCSTX_TX_START_BURST_DEL_CWORD_HI_CFGSTRTBRSTDLMTRCWH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_START_BURST_DEL_CWORD_HI_REG = 
{
    "TX_START_BURST_DEL_CWORD_HI",
#if RU_INCLUDE_DESC
    "XPCS_TX_START_BURST_DEL_CWORD_HI Register",
    "Specifies the start-of-burst delimiter codeword.",
#endif
    XPCSTX_TX_START_BURST_DEL_CWORD_HI_REG_OFFSET,
    0,
    0,
    739,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSTX_TX_START_BURST_DEL_CWORD_HI_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSTX_TX_END_BURST_DEL_CWORD_LO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_END_BURST_DEL_CWORD_LO_FIELDS[] =
{
    &XPCSTX_TX_END_BURST_DEL_CWORD_LO_CFGENDBRSTDLMTRCWL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_END_BURST_DEL_CWORD_LO_REG = 
{
    "TX_END_BURST_DEL_CWORD_LO",
#if RU_INCLUDE_DESC
    "XPCS_TX_END_BURST_DEL_CWORD_LO Register",
    "Specifies the end-of-burst terminating codeword.",
#endif
    XPCSTX_TX_END_BURST_DEL_CWORD_LO_REG_OFFSET,
    0,
    0,
    740,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSTX_TX_END_BURST_DEL_CWORD_LO_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSTX_TX_END_BURST_DEL_CWORD_HI
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_END_BURST_DEL_CWORD_HI_FIELDS[] =
{
    &XPCSTX_TX_END_BURST_DEL_CWORD_HI_CFGENDBRSTDLMTRCWH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_END_BURST_DEL_CWORD_HI_REG = 
{
    "TX_END_BURST_DEL_CWORD_HI",
#if RU_INCLUDE_DESC
    "XPCS_TX_END_BURST_DEL_CWORD_HI Register",
    "Specifies the end-of-burst terminating codeword.",
#endif
    XPCSTX_TX_END_BURST_DEL_CWORD_HI_REG_OFFSET,
    0,
    0,
    741,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSTX_TX_END_BURST_DEL_CWORD_HI_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSTX_TX_IDLE_CWORD_LO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_IDLE_CWORD_LO_FIELDS[] =
{
    &XPCSTX_TX_IDLE_CWORD_LO_CFGIDLECWL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_IDLE_CWORD_LO_REG = 
{
    "TX_IDLE_CWORD_LO",
#if RU_INCLUDE_DESC
    "XPCS_TX_IDLE_CWORD_LO Register",
    "Specifies the IDLE codeword.",
#endif
    XPCSTX_TX_IDLE_CWORD_LO_REG_OFFSET,
    0,
    0,
    742,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSTX_TX_IDLE_CWORD_LO_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSTX_TX_IDLE_CWORD_HI
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_IDLE_CWORD_HI_FIELDS[] =
{
    &XPCSTX_TX_IDLE_CWORD_HI_CFGIDLECWH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_IDLE_CWORD_HI_REG = 
{
    "TX_IDLE_CWORD_HI",
#if RU_INCLUDE_DESC
    "XPCS_TX_IDLE_CWORD_HI Register",
    "Specifies the IDLE codeword.",
#endif
    XPCSTX_TX_IDLE_CWORD_HI_REG_OFFSET,
    0,
    0,
    743,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSTX_TX_IDLE_CWORD_HI_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSTX_TX_BURST_PATT_CWORD_LO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_BURST_PATT_CWORD_LO_FIELDS[] =
{
    &XPCSTX_TX_BURST_PATT_CWORD_LO_CFGBURSTPATCWL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_BURST_PATT_CWORD_LO_REG = 
{
    "TX_BURST_PATT_CWORD_LO",
#if RU_INCLUDE_DESC
    "XPCS_TX_BURST_PATT_CWORD_LO Register",
    "Specifies the codeword in between burst.",
#endif
    XPCSTX_TX_BURST_PATT_CWORD_LO_REG_OFFSET,
    0,
    0,
    744,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSTX_TX_BURST_PATT_CWORD_LO_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSTX_TX_BURST_PATT_CWORD_HI
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_BURST_PATT_CWORD_HI_FIELDS[] =
{
    &XPCSTX_TX_BURST_PATT_CWORD_HI_CFGBURSTPATCWH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_BURST_PATT_CWORD_HI_REG = 
{
    "TX_BURST_PATT_CWORD_HI",
#if RU_INCLUDE_DESC
    "XPCS_TX_BURST_PATT_CWORD_HI Register",
    "Specifies the codeword in between burst.",
#endif
    XPCSTX_TX_BURST_PATT_CWORD_HI_REG_OFFSET,
    0,
    0,
    745,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSTX_TX_BURST_PATT_CWORD_HI_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSTX_TX_LASER_TIME
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_LASER_TIME_FIELDS[] =
{
    &XPCSTX_TX_LASER_TIME_RESERVED0_FIELD,
    &XPCSTX_TX_LASER_TIME_CFGLASERPIPE125_FIELD,
    &XPCSTX_TX_LASER_TIME_RESERVED1_FIELD,
    &XPCSTX_TX_LASER_TIME_CFGLASEROFFDLYTQ125_FIELD,
    &XPCSTX_TX_LASER_TIME_RESERVED2_FIELD,
    &XPCSTX_TX_LASER_TIME_CFGLASERONDLYTQ125_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_LASER_TIME_REG = 
{
    "TX_LASER_TIME",
#if RU_INCLUDE_DESC
    "XPCS_TX_LASER_TIME Register",
    "Provides control for the laser enable.",
#endif
    XPCSTX_TX_LASER_TIME_REG_OFFSET,
    0,
    0,
    746,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    XPCSTX_TX_LASER_TIME_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSTX_TX_MAC_MODE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_MAC_MODE_FIELDS[] =
{
    &XPCSTX_TX_MAC_MODE_RESERVED0_FIELD,
    &XPCSTX_TX_MAC_MODE_CFGENNOGNTXMT125_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_MAC_MODE_REG = 
{
    "TX_MAC_MODE",
#if RU_INCLUDE_DESC
    "XPCS_TX_MAC_MODE Register",
    "Specifies the MAC mode of operation.",
#endif
    XPCSTX_TX_MAC_MODE_REG_OFFSET,
    0,
    0,
    747,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSTX_TX_MAC_MODE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSTX_TX_LASER_MONITOR_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_LASER_MONITOR_CTL_FIELDS[] =
{
    &XPCSTX_TX_LASER_MONITOR_CTL_RESERVED0_FIELD,
    &XPCSTX_TX_LASER_MONITOR_CTL_LASERENSTATUS_FIELD,
    &XPCSTX_TX_LASER_MONITOR_CTL_CFGLSRMONACTHI_FIELD,
    &XPCSTX_TX_LASER_MONITOR_CTL_RESERVED1_FIELD,
    &XPCSTX_TX_LASER_MONITOR_CTL_LASERMONRSTN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_LASER_MONITOR_CTL_REG = 
{
    "TX_LASER_MONITOR_CTL",
#if RU_INCLUDE_DESC
    "XPCS_TX_LASER_MONITOR_CTL Register",
    "Provides control for laser monitor.",
#endif
    XPCSTX_TX_LASER_MONITOR_CTL_REG_OFFSET,
    0,
    0,
    748,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XPCSTX_TX_LASER_MONITOR_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSTX_TX_LASER_MONITOR_MAX_THRESH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_LASER_MONITOR_MAX_THRESH_FIELDS[] =
{
    &XPCSTX_TX_LASER_MONITOR_MAX_THRESH_CFGLSRMONMAXTQ_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_LASER_MONITOR_MAX_THRESH_REG = 
{
    "TX_LASER_MONITOR_MAX_THRESH",
#if RU_INCLUDE_DESC
    "XPCS_TX_LASER_MONITOR_MAX_THRESH Register",
    "Specifies maximum threshold of laser_on assertion before interrupt is"
    "generated.",
#endif
    XPCSTX_TX_LASER_MONITOR_MAX_THRESH_REG_OFFSET,
    0,
    0,
    749,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSTX_TX_LASER_MONITOR_MAX_THRESH_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSTX_TX_LASER_MONITOR_BURST_LEN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_LASER_MONITOR_BURST_LEN_FIELDS[] =
{
    &XPCSTX_TX_LASER_MONITOR_BURST_LEN_LASERONLENGTH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_LASER_MONITOR_BURST_LEN_REG = 
{
    "TX_LASER_MONITOR_BURST_LEN",
#if RU_INCLUDE_DESC
    "XPCS_TX_LASER_MONITOR_BURST_LEN Register",
    "Indicates the burst length of current grant, in unit of TQ (16 ns).",
#endif
    XPCSTX_TX_LASER_MONITOR_BURST_LEN_REG_OFFSET,
    0,
    0,
    750,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSTX_TX_LASER_MONITOR_BURST_LEN_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSTX_TX_LASER_MONITOR_BURST_COUNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSTX_TX_LASER_MONITOR_BURST_COUNT_FIELDS[] =
{
    &XPCSTX_TX_LASER_MONITOR_BURST_COUNT_BURSTCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSTX_TX_LASER_MONITOR_BURST_COUNT_REG = 
{
    "TX_LASER_MONITOR_BURST_COUNT",
#if RU_INCLUDE_DESC
    "XPCS_TX_LASER_MONITOR_BURST_COUNT Register",
    "Counts the number of burst.  Clear on read.",
#endif
    XPCSTX_TX_LASER_MONITOR_BURST_COUNT_REG_OFFSET,
    0,
    0,
    751,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSTX_TX_LASER_MONITOR_BURST_COUNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: XPCSTX
 ******************************************************************************/
static const ru_reg_rec *XPCSTX_REGS[] =
{
    &XPCSTX_TX_CONTROL_REG,
    &XPCSTX_TX_INT_STAT_REG,
    &XPCSTX_TX_INT_MASK_REG,
    &XPCSTX_TX_PORT_COMMAND_REG,
    &XPCSTX_TX_DATA_PORT_0_REG,
    &XPCSTX_TX_DATA_PORT_1_REG,
    &XPCSTX_TX_DATA_PORT_2_REG,
    &XPCSTX_TX_SYNC_PATT_CWORD_LO_REG,
    &XPCSTX_TX_SYNC_PATT_CWORD_HI_REG,
    &XPCSTX_TX_START_BURST_DEL_CWORD_LO_REG,
    &XPCSTX_TX_START_BURST_DEL_CWORD_HI_REG,
    &XPCSTX_TX_END_BURST_DEL_CWORD_LO_REG,
    &XPCSTX_TX_END_BURST_DEL_CWORD_HI_REG,
    &XPCSTX_TX_IDLE_CWORD_LO_REG,
    &XPCSTX_TX_IDLE_CWORD_HI_REG,
    &XPCSTX_TX_BURST_PATT_CWORD_LO_REG,
    &XPCSTX_TX_BURST_PATT_CWORD_HI_REG,
    &XPCSTX_TX_LASER_TIME_REG,
    &XPCSTX_TX_MAC_MODE_REG,
    &XPCSTX_TX_LASER_MONITOR_CTL_REG,
    &XPCSTX_TX_LASER_MONITOR_MAX_THRESH_REG,
    &XPCSTX_TX_LASER_MONITOR_BURST_LEN_REG,
    &XPCSTX_TX_LASER_MONITOR_BURST_COUNT_REG,
};

static unsigned long XPCSTX_ADDRS[] =
{
    0x80143800,
};

const ru_block_rec XPCSTX_BLOCK = 
{
    "XPCSTX",
    XPCSTX_ADDRS,
    1,
    23,
    XPCSTX_REGS
};

/* End of file EPON_XPCSTX.c */
