/*
   Copyright (c) 2015 Broadcom
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

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_BBCFG
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_BBCFG_REG = 
{
    "GENERAL_CONFIGURATION_BBCFG",
#if RU_INCLUDE_DESC
    "BROADBUS_CONFIGURATION Register",
    "Each BBH unit has its own position on the BB tree. The BB defines the Route address for the specific unit.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_REG_OFFSET,
    0,
    0,
    555,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_DISPVIQ
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_REG = 
{
    "GENERAL_CONFIGURATION_DISPVIQ",
#if RU_INCLUDE_DESC
    "DISPATCHER_FLOW Register",
    "For every reassembled packet in the PSRAM the BBH writes a packet descriptor (PD) into the Dispatcher. The PDs are arranged using a link list in the Dispatcher. The Dispatcher has 32 virtual queues (ingress queues) and the BBH may be assigned to each of the 32 virtual queues of the Dispatcher"
    "This register defines virtual queue for normal and exclusive packets.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_REG_OFFSET,
    0,
    0,
    556,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_REG = 
{
    "GENERAL_CONFIGURATION_PATTERNDATALSB",
#if RU_INCLUDE_DESC
    "PATTERN_RECOGNITION_DATA_LSB Register",
    "The BBH may direct a packet into the Dispatchers exclusive VIQ (Virtual Ingress Queue) according to a match in the pattern recognition.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_REG_OFFSET,
    0,
    0,
    557,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_REG = 
{
    "GENERAL_CONFIGURATION_PATTERNDATAMSB",
#if RU_INCLUDE_DESC
    "PATTERN_RECOGNITION_DATA_MSB Register",
    "The BBH may direct a packet into the Dispatchers exclusive VIQ (Virtual Ingress Queue) according to a match in the pattern recognition.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_REG_OFFSET,
    0,
    0,
    558,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_REG = 
{
    "GENERAL_CONFIGURATION_PATTERNMASKLSB",
#if RU_INCLUDE_DESC
    "PATTERN_RECOGNITION_MASK_LSB Register",
    "The BBH may direct a packet into the Dispatchers exclusive VIQ (Virtual Ingress Queue) according to a match in the pattern recognition.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_REG_OFFSET,
    0,
    0,
    559,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_REG = 
{
    "GENERAL_CONFIGURATION_PATTERNMASKMSB",
#if RU_INCLUDE_DESC
    "PATTERN_RECOGNITION_MASK_MSB Register",
    "The BBH may direct a packet into the Dispatchers exclusive VIQ (Virtual Ingress Queue) according to a match in the pattern recognition.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_REG_OFFSET,
    0,
    0,
    560,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_REG = 
{
    "GENERAL_CONFIGURATION_EXCLQCFG",
#if RU_INCLUDE_DESC
    "EXCLUSIVE_QUEUE_CFG Register",
    "The BBH may direct a packet into the Dispatchers exclusive VIQ (Virtual Ingress Queue) according to special packet types (e.g. pause)."
    "This register enables this function",
#endif
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_REG_OFFSET,
    0,
    0,
    561,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_SDMAADDR
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_REG = 
{
    "GENERAL_CONFIGURATION_SDMAADDR",
#if RU_INCLUDE_DESC
    "SDMA_ADDRESS_CONFIGURATION Register",
    "The BBH reassembles the incoming data in the SRAM. The Data is written into the SRAM using the SDMA. The data is organized in a configurable number of chunks of 128 bytes."
    "The BBH arranges the written data in the SDMA in these chunks. It arranges the data in a predefined address space in the SDMA memory and manages the chunks in a cyclic FIFO style."
    "For every write chunk the BBH writes a write descriptor. The write descriptors are arranged in a predefined space in the SDMA memory and managed in a cyclic FIFO style as well."
    "This register defines the Data and descriptors base addresses.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_REG_OFFSET,
    0,
    0,
    562,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_SDMACFG
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_SDMACFG_REG = 
{
    "GENERAL_CONFIGURATION_SDMACFG",
#if RU_INCLUDE_DESC
    "SDMA_CONFIGURATION Register",
    "The BBH reassembles the incoming data in the SRAM. The Data is written into the SRAM using the SDMA. The data is organized in a configurable number of chunks of 128 bytes."
    "The BBH arranges the written data in the SDMA in these chunks. It arranges the data in a predefined address space in the SDMA memory and manages the chunks in a cyclic FIFO style."
    "For every write chunk the BBH writes a write descriptor. The write descriptors are arranged in a predefined space in the SDMA memory and managed in a cyclic FIFO style as well."
    ""
    "The BBH handles the congestion over the SDMA write chunks according to 2 priorities (low + high, exclusive). This field defines the number of occupied write chunks for dropping normal or high priority packets. If the number of occupied chunk is lower than this threshold, then all packets are passed. If the number of occupied chunk is equal or higher than this threshold, then only exclusive priority packets are passed."
    ""
    "This register defines the Data and descriptors FIFO sizes and the exclusive threshold.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_REG_OFFSET,
    0,
    0,
    563,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_MINPKT0
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MINPKT0_REG = 
{
    "GENERAL_CONFIGURATION_MINPKT0",
#if RU_INCLUDE_DESC
    "MINIMUM_PACKET_SIZE Register",
    "There are 4 global configuration for Minimum packet size. Each flow can get one out of these 4 global configurations."
    "Packets shorter than this threshold will be discarded.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MINPKT0_REG_OFFSET,
    0,
    0,
    564,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_MAXPKT0
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_REG = 
{
    "GENERAL_CONFIGURATION_MAXPKT0",
#if RU_INCLUDE_DESC
    "MAXIMUM_PACKET_SIZE_0 Register",
    "There are 4 global configuration for Maximum packet size. Each flow can get one out of these 4 global configurations."
    "Packets longer than this threshold will be discarded.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_REG_OFFSET,
    0,
    0,
    565,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_MAXPKT1
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_REG = 
{
    "GENERAL_CONFIGURATION_MAXPKT1",
#if RU_INCLUDE_DESC
    "MAXIMUM_PACKET_SIZE_1 Register",
    "There are 4 global configuration for Maximum packet size. Each flow can get one out of these 4 global configurations."
    "Packets longer than this threshold will be discarded.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_REG_OFFSET,
    0,
    0,
    566,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_REG = 
{
    "GENERAL_CONFIGURATION_SOPOFFSET",
#if RU_INCLUDE_DESC
    "SOP_OFFSET Register",
    "The BBH writes the packets into the PSRAM. The start of data offset is configurable. This register defines the SOP (start of packet) offset.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_REG_OFFSET,
    0,
    0,
    567,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_REG = 
{
    "GENERAL_CONFIGURATION_FLOWCTRL",
#if RU_INCLUDE_DESC
    "FLOW_CONTROL_CONFIGURATION Register",
    "The BBH manages a flow control indication towards the Ethernet MAC according to BB messages from the FW."
    "Each FW command will assert the flow control indication towards the Ethernet MAC and will trigger a timer. When the timer expires, the BBH will de-assert the flow control indication."
    "This register also disable BBH packet drop due to no space in the SDMA, SBPM or Dispatcher.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_REG_OFFSET,
    0,
    0,
    568,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_REG = 
{
    "GENERAL_CONFIGURATION_CRCOMITDIS",
#if RU_INCLUDE_DESC
    "CRC_OMIT_DISABLE Register",
    "The BBH omits the 4 CRC bytes of the packet for all packets except PLOAMs and OMCI (marked as exclusive priority)."
    "The configuration will disable this functionality.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_REG_OFFSET,
    0,
    0,
    569,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_ENABLE
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_ENABLE_REG = 
{
    "GENERAL_CONFIGURATION_ENABLE",
#if RU_INCLUDE_DESC
    "BBH_ENABLE Register",
    "Controls the BBH enable configuration",
#endif
    BBH_RX_GENERAL_CONFIGURATION_ENABLE_REG_OFFSET,
    0,
    0,
    570,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_G9991EN
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_G9991EN_REG = 
{
    "GENERAL_CONFIGURATION_G9991EN",
#if RU_INCLUDE_DESC
    "G999_1_ENABLE Register",
    "When asserted, G999.1 fragments are received by the BBH."
    "The BBH will pass the G999.1 header in the PD instead of the 1588 time-stamp.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_G9991EN_REG_OFFSET,
    0,
    0,
    571,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_REG = 
{
    "GENERAL_CONFIGURATION_PERFLOWTH",
#if RU_INCLUDE_DESC
    "PER_FLOW_THRESHOLD Register",
    "The DS has 256 flows. Minimum packet size (2 bits) and Maximum packet size (2 bits) are configured per flow."
    "Flows 0-31 will have full configurations. Flows 32-X and flows (X+1)-255 will have global set of configurations."
    "X is configurable."
    "This register defines X.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_REG_OFFSET,
    0,
    0,
    572,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_REG = 
{
    "GENERAL_CONFIGURATION_PERFLOWSETS",
#if RU_INCLUDE_DESC
    "PER_FLOW_SETS Register",
    "The DS has 256 flows. Minimum packet size (2 bits) and Maximum packet size (2 bits) are configured per flow."
    "Flows 0-31 will have full configurations. Flows 32-X and flows (X+1)-255 will have global set of configurations."
    "X is configurable."
    "This register defines the configurations sets.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_REG_OFFSET,
    0,
    0,
    573,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_REG = 
{
    "GENERAL_CONFIGURATION_MINPKTSEL0",
#if RU_INCLUDE_DESC
    "MINIMUM_PACKET_SELECT_0 Register",
    "The DS has 256 flows. Minimum packet size (2 bits) and Maximum packet size (2 bits) are configured per flow."
    "Flows 0-31 will have full configurations. Flows 32-X and flows (X+1)-255 will have global set of configurations."
    "X is configurable."
    "This register defines the minimum packet size for flows 0-15.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_REG_OFFSET,
    0,
    0,
    574,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_REG = 
{
    "GENERAL_CONFIGURATION_MINPKTSEL1",
#if RU_INCLUDE_DESC
    "MINIMUM_PACKET_SELECT_1 Register",
    "The DS has 256 flows. Minimum packet size (2 bits) and Maximum packet size (2 bits) are configured per flow."
    "Flows 0-31 will have full configurations. Flows 32-X and flows (X+1)-255 will have global set of configurations."
    "X is configurable."
    "This register defines the minimum packet size for flows 16-31.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_REG_OFFSET,
    0,
    0,
    575,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_REG = 
{
    "GENERAL_CONFIGURATION_MAXPKTSEL0",
#if RU_INCLUDE_DESC
    "MAXIMUM_PACKET_SELECT_0 Register",
    "The DS has 256 flows. Minimum packet size (2 bits) and Maximum packet size (2 bits) are configured per flow."
    "Flows 0-31 will have full configurations. Flows 32-X and flows (X+1)-255 will have global set of configurations."
    "X is configurable."
    "This register defines the maximum packet size for flows 0-15.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_REG_OFFSET,
    0,
    0,
    576,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_REG = 
{
    "GENERAL_CONFIGURATION_MAXPKTSEL1",
#if RU_INCLUDE_DESC
    "MAXIMUM_PACKET_SELECT_1 Register",
    "The DS has 256 flows. Minimum packet size (2 bits) and Maximum packet size (2 bits) are configured per flow."
    "Flows 0-31 will have full configurations. Flows 32-X and flows (X+1)-255 will have global set of configurations."
    "X is configurable."
    "This register defines the maximum packet size for flows 16-31.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_REG_OFFSET,
    0,
    0,
    577,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_MACMODE
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MACMODE_REG = 
{
    "GENERAL_CONFIGURATION_MACMODE",
#if RU_INCLUDE_DESC
    "MAC_MODE Register",
    "When the BBH functions as a PON BBH, this bit selects between N/X/GPON/2 and 10G/EPON functionality",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MACMODE_REG_OFFSET,
    0,
    0,
    578,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_SBPMCFG
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_REG = 
{
    "GENERAL_CONFIGURATION_SBPMCFG",
#if RU_INCLUDE_DESC
    "SBPM_CFG Register",
    "Configure max on the fly requests to SBPM",
#endif
    BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_REG_OFFSET,
    0,
    0,
    579,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_RXRSTRST
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_REG = 
{
    "GENERAL_CONFIGURATION_RXRSTRST",
#if RU_INCLUDE_DESC
    "RX_RESET_COMMAND Register",
    "This register enable reset of internal units (for WA perposes).",
#endif
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_REG_OFFSET,
    0,
    0,
    580,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_REG = 
{
    "GENERAL_CONFIGURATION_RXDBGSEL",
#if RU_INCLUDE_DESC
    "RX_DEBUG_SELECT Register",
    "Selects one out of 10 possible debug vectors",
#endif
    BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_REG_OFFSET,
    0,
    0,
    581,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_REG = 
{
    "GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER",
#if RU_INCLUDE_DESC
    "BBH_RX_RADDR_DECODER Register",
    "This register enables changing the route address for a specified BB ID",
#endif
    BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_REG_OFFSET,
    0,
    0,
    582,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_NONETH
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_NONETH_REG = 
{
    "GENERAL_CONFIGURATION_NONETH",
#if RU_INCLUDE_DESC
    "NON_ETHERNET_FLOW Register",
    "There an option to disable CRC error counting for this flow.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_NONETH_REG_OFFSET,
    0,
    0,
    583,
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL
 ******************************************************************************/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_REG = 
{
    "GENERAL_CONFIGURATION_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control",
#endif
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_REG_OFFSET,
    0,
    0,
    584,
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_INPKT
 ******************************************************************************/
const ru_reg_rec BBH_RX_PM_COUNTERS_INPKT_REG = 
{
    "PM_COUNTERS_INPKT",
#if RU_INCLUDE_DESC
    "INCOMING_PACKETS Register",
    "This counter counts the number of incoming good packets."
    "It counts the packets from all flows together."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_INPKT_REG_OFFSET,
    0,
    0,
    585,
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_THIRDFLOW
 ******************************************************************************/
const ru_reg_rec BBH_RX_PM_COUNTERS_THIRDFLOW_REG = 
{
    "PM_COUNTERS_THIRDFLOW",
#if RU_INCLUDE_DESC
    "THIRD_FLOW_ERROR Register",
    "This counter counts the packets drop due to Third flow error."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_THIRDFLOW_REG_OFFSET,
    0,
    0,
    586,
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_SOPASOP
 ******************************************************************************/
const ru_reg_rec BBH_RX_PM_COUNTERS_SOPASOP_REG = 
{
    "PM_COUNTERS_SOPASOP",
#if RU_INCLUDE_DESC
    "SOP_AFTER_SOP_ERROR Register",
    "This counter counts the packets drop due to SOP after SOP error."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_SOPASOP_REG_OFFSET,
    0,
    0,
    587,
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_TOOSHORT
 ******************************************************************************/
const ru_reg_rec BBH_RX_PM_COUNTERS_TOOSHORT_REG = 
{
    "PM_COUNTERS_TOOSHORT",
#if RU_INCLUDE_DESC
    "TOO_SHORT_ERROR Register",
    "This counter counts the packets drop due to Too short error."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_TOOSHORT_REG_OFFSET,
    0,
    0,
    588,
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_TOOLONG
 ******************************************************************************/
const ru_reg_rec BBH_RX_PM_COUNTERS_TOOLONG_REG = 
{
    "PM_COUNTERS_TOOLONG",
#if RU_INCLUDE_DESC
    "TOO_LONG_ERROR Register",
    "This counter counts the packets drop due to Too long error."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_TOOLONG_REG_OFFSET,
    0,
    0,
    589,
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_CRCERROR
 ******************************************************************************/
const ru_reg_rec BBH_RX_PM_COUNTERS_CRCERROR_REG = 
{
    "PM_COUNTERS_CRCERROR",
#if RU_INCLUDE_DESC
    "CRC_ERROR Register",
    "This counter counts the packets drop due to CRC error."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_CRCERROR_REG_OFFSET,
    0,
    0,
    590,
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_ENCRYPTERROR
 ******************************************************************************/
const ru_reg_rec BBH_RX_PM_COUNTERS_ENCRYPTERROR_REG = 
{
    "PM_COUNTERS_ENCRYPTERROR",
#if RU_INCLUDE_DESC
    "ENCRYPTION_ERROR Register",
    "This counter counts the packets drop due to XGPON encryption error."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_ENCRYPTERROR_REG_OFFSET,
    0,
    0,
    591,
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_DISPCONG
 ******************************************************************************/
const ru_reg_rec BBH_RX_PM_COUNTERS_DISPCONG_REG = 
{
    "PM_COUNTERS_DISPCONG",
#if RU_INCLUDE_DESC
    "DISPATCHER_CONGESTION_ERROR Register",
    "This counter counts the packets drop due to Dispatcher congestion error."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_DISPCONG_REG_OFFSET,
    0,
    0,
    592,
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_NOSBPMSBN
 ******************************************************************************/
const ru_reg_rec BBH_RX_PM_COUNTERS_NOSBPMSBN_REG = 
{
    "PM_COUNTERS_NOSBPMSBN",
#if RU_INCLUDE_DESC
    "NO_SBPM_SBN_ERROR Register",
    "This counter counts the packets drop due to NO SBPM SBN error."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_NOSBPMSBN_REG_OFFSET,
    0,
    0,
    593,
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_NOSDMACD
 ******************************************************************************/
const ru_reg_rec BBH_RX_PM_COUNTERS_NOSDMACD_REG = 
{
    "PM_COUNTERS_NOSDMACD",
#if RU_INCLUDE_DESC
    "NO_SDMA_CD_ERROR Register",
    "This counter counts the packets drop due to No SDMA CD error."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_NOSDMACD_REG_OFFSET,
    0,
    0,
    594,
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_INPLOAM
 ******************************************************************************/
const ru_reg_rec BBH_RX_PM_COUNTERS_INPLOAM_REG = 
{
    "PM_COUNTERS_INPLOAM",
#if RU_INCLUDE_DESC
    "INCOMING_PLOAM Register",
    "This counter counts the number of incoming good PLOAMs."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_INPLOAM_REG_OFFSET,
    0,
    0,
    595,
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_CRCERRORPLOAM
 ******************************************************************************/
const ru_reg_rec BBH_RX_PM_COUNTERS_CRCERRORPLOAM_REG = 
{
    "PM_COUNTERS_CRCERRORPLOAM",
#if RU_INCLUDE_DESC
    "CRC_PLOAM_ERROR Register",
    "This counter counts the PLOAMs drop due to CRC error."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_CRCERRORPLOAM_REG_OFFSET,
    0,
    0,
    596,
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_DISPCONGPLOAM
 ******************************************************************************/
const ru_reg_rec BBH_RX_PM_COUNTERS_DISPCONGPLOAM_REG = 
{
    "PM_COUNTERS_DISPCONGPLOAM",
#if RU_INCLUDE_DESC
    "DISPATCHER_CONGESTION_PLOAM_ERROR Register",
    "This counter counts the packets drop due to Dispatcher congestion error for PLOAM."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_DISPCONGPLOAM_REG_OFFSET,
    0,
    0,
    597,
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM
 ******************************************************************************/
const ru_reg_rec BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_REG = 
{
    "PM_COUNTERS_NOSBPMSBNPLOAM",
#if RU_INCLUDE_DESC
    "NO_SBPM_SBN_PLOAM_ERROR Register",
    "This counter counts the PLOAMs drop due to No SBPM SBN error."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_REG_OFFSET,
    0,
    0,
    598,
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_NOSDMACDPLOAM
 ******************************************************************************/
const ru_reg_rec BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_REG = 
{
    "PM_COUNTERS_NOSDMACDPLOAM",
#if RU_INCLUDE_DESC
    "NO_SDMA_CD_PLOAM_ERROR Register",
    "This counter counts the packets drop due to No SDMA CD error for PLOAMs."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_REG_OFFSET,
    0,
    0,
    599,
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_EPONTYPERROR
 ******************************************************************************/
const ru_reg_rec BBH_RX_PM_COUNTERS_EPONTYPERROR_REG = 
{
    "PM_COUNTERS_EPONTYPERROR",
#if RU_INCLUDE_DESC
    "EPON_TYPE_ERROR Register",
    "This counter counts the events of EPON type sequence which is wrong, meaning no sop after header, or sop/header in the middle of packet (before eop)."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_EPONTYPERROR_REG_OFFSET,
    0,
    0,
    600,
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_RUNTERROR
 ******************************************************************************/
const ru_reg_rec BBH_RX_PM_COUNTERS_RUNTERROR_REG = 
{
    "PM_COUNTERS_RUNTERROR",
#if RU_INCLUDE_DESC
    "RUNT_ERROR Register",
    "This counter counts the number of RUNT packets received from the XLMAC."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_RUNTERROR_REG_OFFSET,
    0,
    0,
    601,
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_CNTXTX0LSB
 ******************************************************************************/
const ru_reg_rec BBH_RX_DEBUG_CNTXTX0LSB_REG = 
{
    "DEBUG_CNTXTX0LSB",
#if RU_INCLUDE_DESC
    "CONTEXT_0_LSB Register",
    "In the case of GPON peripheral, DS flows may arrive interleaved. The BBH supports parallel reassembly of up to two interleaved flows (out of 256). For the reassembly process the BBH stores a double flow context.",
#endif
    BBH_RX_DEBUG_CNTXTX0LSB_REG_OFFSET,
    0,
    0,
    602,
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_CNTXTX0MSB
 ******************************************************************************/
const ru_reg_rec BBH_RX_DEBUG_CNTXTX0MSB_REG = 
{
    "DEBUG_CNTXTX0MSB",
#if RU_INCLUDE_DESC
    "CONTEXT_0_MSB Register",
    "In the case of GPON peripheral, DS flows may arrive interleaved. The BBH supports parallel reassembly of up to two interleaved flows (out of 256). For the reassembly process the BBH stores a double flow context.",
#endif
    BBH_RX_DEBUG_CNTXTX0MSB_REG_OFFSET,
    0,
    0,
    603,
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_CNTXTX1LSB
 ******************************************************************************/
const ru_reg_rec BBH_RX_DEBUG_CNTXTX1LSB_REG = 
{
    "DEBUG_CNTXTX1LSB",
#if RU_INCLUDE_DESC
    "CONTEXT_1_LSB Register",
    "In the case of GPON peripheral, DS flows may arrive interleaved. The BBH supports parallel reassembly of up to two interleaved flows (out of 256). For the reassembly process the BBH stores a double flow context.",
#endif
    BBH_RX_DEBUG_CNTXTX1LSB_REG_OFFSET,
    0,
    0,
    604,
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_CNTXTX1MSB
 ******************************************************************************/
const ru_reg_rec BBH_RX_DEBUG_CNTXTX1MSB_REG = 
{
    "DEBUG_CNTXTX1MSB",
#if RU_INCLUDE_DESC
    "CONTEXT_1_MSB Register",
    "In the case of GPON peripheral, DS flows may arrive interleaved. The BBH supports parallel reassembly of up to two interleaved flows (out of 256). For the reassembly process the BBH stores a double flow context.",
#endif
    BBH_RX_DEBUG_CNTXTX1MSB_REG_OFFSET,
    0,
    0,
    605,
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_CNTXTX0INGRESS
 ******************************************************************************/
const ru_reg_rec BBH_RX_DEBUG_CNTXTX0INGRESS_REG = 
{
    "DEBUG_CNTXTX0INGRESS",
#if RU_INCLUDE_DESC
    "INGRESS_CONTEXT_0 Register",
    "In the case of GPON peripheral, DS flows may arrive interleaved. The BBH supports parallel reassembly of up to two interleaved flows (out of 256). For the reassembly process the BBH stores a double flow context.",
#endif
    BBH_RX_DEBUG_CNTXTX0INGRESS_REG_OFFSET,
    0,
    0,
    606,
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_CNTXTX1INGRESS
 ******************************************************************************/
const ru_reg_rec BBH_RX_DEBUG_CNTXTX1INGRESS_REG = 
{
    "DEBUG_CNTXTX1INGRESS",
#if RU_INCLUDE_DESC
    "INGRESS_CONTEXT_1 Register",
    "In the case of GPON peripheral, DS flows may arrive interleaved. The BBH supports parallel reassembly of up to two interleaved flows (out of 256). For the reassembly process the BBH stores a double flow context.",
#endif
    BBH_RX_DEBUG_CNTXTX1INGRESS_REG_OFFSET,
    0,
    0,
    607,
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_IBUW
 ******************************************************************************/
const ru_reg_rec BBH_RX_DEBUG_IBUW_REG = 
{
    "DEBUG_IBUW",
#if RU_INCLUDE_DESC
    "INPUT_BUF_USED_WORDS Register",
    "Input buf used words",
#endif
    BBH_RX_DEBUG_IBUW_REG_OFFSET,
    0,
    0,
    608,
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_BBUW
 ******************************************************************************/
const ru_reg_rec BBH_RX_DEBUG_BBUW_REG = 
{
    "DEBUG_BBUW",
#if RU_INCLUDE_DESC
    "BURST_BUF_USED_WORDS Register",
    "Burst buf used words",
#endif
    BBH_RX_DEBUG_BBUW_REG_OFFSET,
    0,
    0,
    609,
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_CFUW
 ******************************************************************************/
const ru_reg_rec BBH_RX_DEBUG_CFUW_REG = 
{
    "DEBUG_CFUW",
#if RU_INCLUDE_DESC
    "COHERENCY_FIFO_USED_WORDS Register",
    "Coherency FIFO used words",
#endif
    BBH_RX_DEBUG_CFUW_REG_OFFSET,
    0,
    0,
    610,
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_ACKCNT
 ******************************************************************************/
const ru_reg_rec BBH_RX_DEBUG_ACKCNT_REG = 
{
    "DEBUG_ACKCNT",
#if RU_INCLUDE_DESC
    "ACK_COUNTERS Register",
    "The register reflects 2 ACK counters:"
    "SDMA"
    "CONNECT",
#endif
    BBH_RX_DEBUG_ACKCNT_REG_OFFSET,
    0,
    0,
    611,
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_COHERENCYCNT
 ******************************************************************************/
const ru_reg_rec BBH_RX_DEBUG_COHERENCYCNT_REG = 
{
    "DEBUG_COHERENCYCNT",
#if RU_INCLUDE_DESC
    "COHERENCY_COUNTERS Register",
    "The register 2 pending coherency counters:"
    "Normal"
    "Exclusive",
#endif
    BBH_RX_DEBUG_COHERENCYCNT_REG_OFFSET,
    0,
    0,
    612,
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_DBGVEC
 ******************************************************************************/
const ru_reg_rec BBH_RX_DEBUG_DBGVEC_REG = 
{
    "DEBUG_DBGVEC",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR Register",
    "selected debug vector",
#endif
    BBH_RX_DEBUG_DBGVEC_REG_OFFSET,
    0,
    0,
    613,
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_UFUW
 ******************************************************************************/
const ru_reg_rec BBH_RX_DEBUG_UFUW_REG = 
{
    "DEBUG_UFUW",
#if RU_INCLUDE_DESC
    "UPLOAD_FIFO_USED_WORDS Register",
    "Upload FIFO used words",
#endif
    BBH_RX_DEBUG_UFUW_REG_OFFSET,
    0,
    0,
    614,
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_CREDITCNT
 ******************************************************************************/
const ru_reg_rec BBH_RX_DEBUG_CREDITCNT_REG = 
{
    "DEBUG_CREDITCNT",
#if RU_INCLUDE_DESC
    "CREDIT_COUNTERS Register",
    "This register holds 2 credit counters:"
    "Normal"
    "Exclusive",
#endif
    BBH_RX_DEBUG_CREDITCNT_REG_OFFSET,
    0,
    0,
    615,
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_SDMACNT
 ******************************************************************************/
const ru_reg_rec BBH_RX_DEBUG_SDMACNT_REG = 
{
    "DEBUG_SDMACNT",
#if RU_INCLUDE_DESC
    "USED_SDMA_CD_CNT Register",
    "Number of used SDMA CDs",
#endif
    BBH_RX_DEBUG_SDMACNT_REG_OFFSET,
    0,
    0,
    616,
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_CMFUW
 ******************************************************************************/
const ru_reg_rec BBH_RX_DEBUG_CMFUW_REG = 
{
    "DEBUG_CMFUW",
#if RU_INCLUDE_DESC
    "CMD_FIFO_USED_WORDS Register",
    "CMD FIFO used words",
#endif
    BBH_RX_DEBUG_CMFUW_REG_OFFSET,
    0,
    0,
    617,
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_SBNFIFO
 ******************************************************************************/
const ru_reg_rec BBH_RX_DEBUG_SBNFIFO_REG = 
{
    "DEBUG_SBNFIFO",
#if RU_INCLUDE_DESC
    "SRAM_BN_FIFO %i Register",
    "The BBH RX hold a FIFO with 16 BN.",
#endif
    BBH_RX_DEBUG_SBNFIFO_REG_OFFSET,
    BBH_RX_DEBUG_SBNFIFO_REG_RAM_CNT,
    4,
    618,
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_CMDFIFO
 ******************************************************************************/
const ru_reg_rec BBH_RX_DEBUG_CMDFIFO_REG = 
{
    "DEBUG_CMDFIFO",
#if RU_INCLUDE_DESC
    "CMD_FIFO %i Register",
    "The BBH RX hold a FIFO with 8 command.",
#endif
    BBH_RX_DEBUG_CMDFIFO_REG_OFFSET,
    BBH_RX_DEBUG_CMDFIFO_REG_RAM_CNT,
    4,
    619,
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_SBNRECYCLEFIFO
 ******************************************************************************/
const ru_reg_rec BBH_RX_DEBUG_SBNRECYCLEFIFO_REG = 
{
    "DEBUG_SBNRECYCLEFIFO",
#if RU_INCLUDE_DESC
    "SRAM_BN_RECYCLE_FIFO %i Register",
    "The BBH RX hold a recycle FIFO with up to 2 BN.",
#endif
    BBH_RX_DEBUG_SBNRECYCLEFIFO_REG_OFFSET,
    BBH_RX_DEBUG_SBNRECYCLEFIFO_REG_RAM_CNT,
    4,
    620,
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_COHERENCYCNT2
 ******************************************************************************/
const ru_reg_rec BBH_RX_DEBUG_COHERENCYCNT2_REG = 
{
    "DEBUG_COHERENCYCNT2",
#if RU_INCLUDE_DESC
    "COHERENCY_COUNTERS_METHOD2 Register",
    "Read of 4 coherency counters:"
    "CD CMD sent (1 per flow)"
    "EOP ACK received (1 per flow)",
#endif
    BBH_RX_DEBUG_COHERENCYCNT2_REG_OFFSET,
    0,
    0,
    621,
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_DROPSTATUS
 ******************************************************************************/
const ru_reg_rec BBH_RX_DEBUG_DROPSTATUS_REG = 
{
    "DEBUG_DROPSTATUS",
#if RU_INCLUDE_DESC
    "SPECIAL_DROP_STATUS Register",
    "Information of the following:"
    "- Dispatcher drop due to coherency FIFO full"
    "- SDMA drop due to coherency method 2 counters over 63 (dec)"
    "",
#endif
    BBH_RX_DEBUG_DROPSTATUS_REG_OFFSET,
    0,
    0,
    622,
};

/******************************************************************************
 * Block: BBH_RX
 ******************************************************************************/
static const ru_reg_rec *BBH_RX_REGS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_BBCFG_REG,
    &BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_REG,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_REG,
    &BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_REG,
    &BBH_RX_GENERAL_CONFIGURATION_SDMACFG_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MINPKT0_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_REG,
    &BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_REG,
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_REG,
    &BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_REG,
    &BBH_RX_GENERAL_CONFIGURATION_ENABLE_REG,
    &BBH_RX_GENERAL_CONFIGURATION_G9991EN_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MACMODE_REG,
    &BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_REG,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_REG,
    &BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_REG,
    &BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_REG,
    &BBH_RX_GENERAL_CONFIGURATION_NONETH_REG,
    &BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_REG,
    &BBH_RX_PM_COUNTERS_INPKT_REG,
    &BBH_RX_PM_COUNTERS_THIRDFLOW_REG,
    &BBH_RX_PM_COUNTERS_SOPASOP_REG,
    &BBH_RX_PM_COUNTERS_TOOSHORT_REG,
    &BBH_RX_PM_COUNTERS_TOOLONG_REG,
    &BBH_RX_PM_COUNTERS_CRCERROR_REG,
    &BBH_RX_PM_COUNTERS_ENCRYPTERROR_REG,
    &BBH_RX_PM_COUNTERS_DISPCONG_REG,
    &BBH_RX_PM_COUNTERS_NOSBPMSBN_REG,
    &BBH_RX_PM_COUNTERS_NOSDMACD_REG,
    &BBH_RX_PM_COUNTERS_INPLOAM_REG,
    &BBH_RX_PM_COUNTERS_CRCERRORPLOAM_REG,
    &BBH_RX_PM_COUNTERS_DISPCONGPLOAM_REG,
    &BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_REG,
    &BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_REG,
    &BBH_RX_PM_COUNTERS_EPONTYPERROR_REG,
    &BBH_RX_PM_COUNTERS_RUNTERROR_REG,
    &BBH_RX_DEBUG_CNTXTX0LSB_REG,
    &BBH_RX_DEBUG_CNTXTX0MSB_REG,
    &BBH_RX_DEBUG_CNTXTX1LSB_REG,
    &BBH_RX_DEBUG_CNTXTX1MSB_REG,
    &BBH_RX_DEBUG_CNTXTX0INGRESS_REG,
    &BBH_RX_DEBUG_CNTXTX1INGRESS_REG,
    &BBH_RX_DEBUG_IBUW_REG,
    &BBH_RX_DEBUG_BBUW_REG,
    &BBH_RX_DEBUG_CFUW_REG,
    &BBH_RX_DEBUG_ACKCNT_REG,
    &BBH_RX_DEBUG_COHERENCYCNT_REG,
    &BBH_RX_DEBUG_DBGVEC_REG,
    &BBH_RX_DEBUG_UFUW_REG,
    &BBH_RX_DEBUG_CREDITCNT_REG,
    &BBH_RX_DEBUG_SDMACNT_REG,
    &BBH_RX_DEBUG_CMFUW_REG,
    &BBH_RX_DEBUG_SBNFIFO_REG,
    &BBH_RX_DEBUG_CMDFIFO_REG,
    &BBH_RX_DEBUG_SBNRECYCLEFIFO_REG,
    &BBH_RX_DEBUG_COHERENCYCNT2_REG,
    &BBH_RX_DEBUG_DROPSTATUS_REG,
};

unsigned long BBH_RX_ADDRS[] =
{
    0x82d94000,
    0x82d94400,
    0x82d94800,
    0x82d94c00,
    0x82d95000,
    0x82d95400,
};

const ru_block_rec BBH_RX_BLOCK = 
{
    "BBH_RX",
    BBH_RX_ADDRS,
    6,
    68,
    BBH_RX_REGS
};

/* End of file XRDP_BBH_RX.c */
