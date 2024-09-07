/******************************************************************************

   Copyright 2023-2024 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#ifndef MXL_GSW_API_H_
#define MXL_GSW_API_H_

#include "gsw.h"

/* General */

/** \addtogroup GSW_DEBUG
 *  @{
 */

/**
   \brief Read an internal register. The register offset defines which register to access.
   This routine only accesses the ETHSW_PDI of the switch.
   Note that the switch API implementation checks whether the given address is
   inside the valid address range. It returns with an error in case an invalid
   address is given.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_register_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_RegisterGet(const GSW_Device_t *dev, GSW_register_t *parm);

/**
   \brief Write to an internal register. The register offset defines which register to access.
   This routine only accesses the ETHSW_PDI of the switch.
   Note that the switch API implementation checks whether the given address is
   inside the valid address range. It returns with an error in case an invalid
   address is given.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_register_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_RegisterSet(const GSW_Device_t *dev, GSW_register_t *parm);

/**
   \brief Modify an internal register. The register offset defines which register to access.
   This routine only accesses the ETHSW_PDI of the switch.
   Note that the switch API implementation checks whether the given address is
   inside the valid address range. It returns with an error in case an invalid
   address is given.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_register_mod_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_RegisterMod(const GSW_Device_t *dev, GSW_register_mod_t *parm);

/** @}*/ /* GSW_DEBUG */


/** \addtogroup GSW_OAM
 *  @{
 */

/**
   \brief Defines one port that is directly connected to the software running on a CPU.
   This allows for the redirecting of protocol-specific packets to the CPU port.
   If the CPU port cannot be set, the function returns an error code.

   \param dev Pointer to switch device.
   \param parm Pointer to an \ref GSW_CPU_Port_t structure.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_CPU_PortSet(const GSW_Device_t *dev, GSW_CPU_Port_t *parm);

/**
   \brief Get the port that is directly connected to the software running on a CPU and defined as
   CPU port. This port assignment can be set using \ref GSW_CPU_PortSet
   if it is not fixed and defined by the switch device architecture.

   \param dev Pointer to switch device.
   \param parm Pointer to an \ref GSW_CPU_Port_t structure.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_CPU_PortGet(const GSW_Device_t *dev, GSW_CPU_Port_t *parm);

/**
   \brief Defines one port that is directly connected to the software running on a CPU.
   This allows for the redirecting of protocol-specific packets to the CPU port and
   special packet treatment when sent by the CPU.
   If the CPU port cannot be set, the function returns an error code.
   This is deprecated and replaced by \ref GSW_CPU_PortSet if only change
   the CPU port without port settings.

   \param dev Pointer to switch device.
   \param parm Pointer to an \ref GSW_CPU_PortCfg_t structure.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_CPU_PortCfgSet(const GSW_Device_t *dev, GSW_CPU_PortCfg_t *parm);

/**
   \brief Get the port that is directly connected to the software running on a CPU and defined as
   CPU port. This port assignment can be set using \ref GSW_CPU_PortCfgSet
   if it is not fixed and defined by the switch device architecture.
   This is deprecated and replaced by \ref GSW_CPU_PortGet if not to load the
   port settings.

   \param dev Pointer to switch device.
   \param parm Pointer to an \ref GSW_CPU_PortCfg_t structure.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_CPU_PortCfgGet(const GSW_Device_t *dev, GSW_CPU_PortCfg_t *parm);

/**
   \brief Set the Ethernet port link, speed status and flow control status.
   The configuration applies to a single port 'nPortId'.

   \param dev Pointer to switch device.
   \param parm Pointer to an \ref GSW_portLinkCfg_t structure to set the port configuration.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PortLinkCfgSet(const GSW_Device_t *dev, GSW_portLinkCfg_t *parm);

/**
   \brief Read out the Ethernet port's speed, link status, and flow control status.
   The information for one single port 'nPortId' is returned.
   An error code is returned if the selected port does not exist.

   \param dev Pointer to switch device.
   \param parm Pointer to an \ref GSW_portLinkCfg_t structure to read out the port status.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PortLinkCfgGet(const GSW_Device_t *dev, GSW_portLinkCfg_t *parm);

/**
   \brief Set the Ethernet port configuration.

   \param dev Pointer to switch device.
   \param parm Pointer to an \ref GSW_portCfg_t structure
   to configure the switch port hardware.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PortCfgSet(const GSW_Device_t *dev, GSW_portCfg_t *parm);

/**
   \brief Read out the current Ethernet port configuration.

   \param dev Pointer to switch device.
   \param parm Pointer to a port configuration \ref GSW_portCfg_t structure to fill out by the driver.
   The parameter 'nPortId' tells the driver which port parameter is requested.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PortCfgGet(const GSW_Device_t *dev, GSW_portCfg_t *parm);

/** \brief This returns the capability referenced by the provided index
    (zero-based counting index value). The Switch API uses the index to return
    the capability parameter from an internal list. For instance,
    the capability list contains information about the amount of supported
    features like number of supported VLAN groups or MAC table entries.
    The command returns zero-length strings ('') in case the
    requested index number is out of range.

   \param dev Pointer to switch device.
   \param parm Pointer to pre-allocated capability list structure \ref GSW_cap_t.
      The switch API implementation fills out the structure with the supported
      features, based on the provided 'nCapType' parameter.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_CapGet(const GSW_Device_t *dev, GSW_cap_t *parm);

/**
   \brief Modify the switch configuration.
   The configuration can be read using \ref GSW_CfgGet.

   \param dev Pointer to switch device.
   \param parm Pointer to an \ref GSW_cfg_t structure.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_CfgSet(const GSW_Device_t *dev, GSW_cfg_t *parm);

/**
   \brief Read the global switch configuration.
   This configuration can be set using \ref GSW_CfgSet.

   \param dev Pointer to switch device.
   \param parm Pointer to an \ref GSW_cfg_t structure.
      The structure is filled out by the switch implementation.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_CfgGet(const GSW_Device_t *dev, GSW_cfg_t *parm);

/** @}*/ /* GSW_OAM */


/** \addtogroup GSW_OAM
 *  @{
 */

/**
   \brief Reads out the current monitor options for a
   dedicated Ethernet port. This configuration can be set
   using \ref GSW_MonitorPortCfgSet.

   \param dev Pointer to switch device.
   \param parm Pointer
          to \ref GSW_monitorPortCfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MonitorPortCfgGet(const GSW_Device_t *dev, GSW_monitorPortCfg_t *parm);

/**
   \brief Configures the monitor options for a
   dedicated Ethernet port. This current configuration can be read back
   using \ref GSW_MonitorPortCfgGet.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_monitorPortCfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MonitorPortCfgSet(const GSW_Device_t *dev, GSW_monitorPortCfg_t *parm);

/**
   \brief Freeze switch hardware with traffic stall.
   It is used to configure switch with traffic blocked.
   This is required for configuration sensitive to ongoing traffic,
   such as clear counters, change queue configuration, etc.

   \param dev Pointer to switch device.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_Freeze(const GSW_Device_t *dev);

/**
   \brief Unfreeze switch hardward.
   It is used to re-enable switch after configuration is done.

   \param dev Pointer to switch device.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_UnFreeze(const GSW_Device_t *dev);

/**
 * \brief Get Hit Status from MAC or Multicast table.
 * It is used to get hit status of MAC table entry or Multicast table entry
 * and clear it.
 *
 * \param dev Pointer to switch device.
 * \param parm Pointer to \ref GSW_HitStatusRead_t.
 *
 * \remarks The function returns an error code in case an error occurs.
 *          The error code is described in \ref GSW_return_t.
 *
 * \return Return value as follows:
 * - GSW_statusOk: if successful
 * - An error code in case an error occurs
 */
GSW_return_t GSW_GetHitSts(const GSW_Device_t *dev, GSW_HitStatusRead_t *parm);

/**
 * \brief Get miscellaneous configurations such as 4-TPID support
 *  on logical port (0~15).
 *
 * \param dev Pointer to switch device.
 * \param parm Pointer to \ref GSW_MiscPortCfg_t.
 *
 * \remarks The function returns an error code in case an error occurs.
 *          The error code is described in \ref GSW_return_t.
 *
 * \return Return value as follows:
 * - GSW_statusOk: if successful
 * - An error code in case an error occurs
 */
GSW_return_t GSW_MiscPortCfgGet(const GSW_Device_t *dev, GSW_MiscPortCfg_t *parm);

/**
 * \brief Configure miscellaneous configurations such as 4-TPID support
 *  on logical port (0~15).
 *
 * \param dev Pointer to switch device.
 * \param parm Pointer to \ref GSW_MiscPortCfg_t.
 *
 * \remarks The function returns an error code in case an error occurs.
 *          The error code is described in \ref GSW_return_t.
 *
 * \return Return value as follows:
 * - GSW_statusOk: if successful
 * - An error code in case an error occurs
 */
GSW_return_t GSW_MiscPortCfgSet(const GSW_Device_t *dev, GSW_MiscPortCfg_t *parm);

/** @}*/ /* GSW_OAM */


/* TFLOW (PCE Rule) */

/** \addtogroup GSW_PCE
 *  @{
 */

/**
   \brief Allocate PCE Rule block.
   It allocates consecutive PCE Rule entries and return the block ID
   for further operations: \ref GSW_PceRuleFree.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_PCE_rule_alloc_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
 */
GSW_return_t GSW_PceRuleAlloc(const GSW_Device_t *dev, GSW_PCE_rule_alloc_t *parm);

/**
   \brief Release PCE Rule block.
   It is used to release PCE Rule block allocated by \ref GSW_PceRuleAlloc.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_PCE_rule_alloc_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
 */
GSW_return_t GSW_PceRuleFree(const GSW_Device_t *dev, GSW_PCE_rule_alloc_t *parm);

/**
   \brief Get size of PCE Rule block.
   It is used to get number of entries in the PCE Rule block allocated
   by \ref GSW_PceRuleAlloc.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_PCE_rule_alloc_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
 */
GSW_return_t GSW_PceRuleBlockSize(const GSW_Device_t *dev, GSW_PCE_rule_alloc_t *parm);

/**
   \brief Enable PCE Rule.
   It is used to Enable PCE Rule written by
   \ref GSW_PceRuleWrite.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_PCE_rule_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
 */
GSW_return_t GSW_PceRuleEnable(const GSW_Device_t *dev, GSW_PCE_ruleEntry_t *parm);

/**
   \brief Disable PCE Rule.
   It is used to Disable PCE Rule written by
   \ref GSW_PceRuleWrite.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_PCE_rule_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
 */
GSW_return_t GSW_PceRuleDisable(const GSW_Device_t *dev, GSW_PCE_ruleEntry_t *parm);

/**
   \brief This command allows the reading out of a rule pattern and action of the
   packet classification engine.
   A rule can be written using the command \ref GSW_PceRuleWrite.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_PCE_rule_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PceRuleRead(const GSW_Device_t *dev, GSW_PCE_rule_t *parm);

/**
   \brief This command writes a rule pattern and action to the table of the packet
   classification engine. The pattern part describes the parameter to identify an
   incoming packet to which the dedicated actions should be applied.
   A rule can be read using the command \ref GSW_PceRuleWrite.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_PCE_rule_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PceRuleWrite(const GSW_Device_t *dev, GSW_PCE_rule_t *parm);

/**
   \brief This command deletes a complete rule from the packet classification engine.
   A delete operation is done on the rule of a dedicated index 'nIndex'.
   A rule can be written over using the command \ref GSW_PceRuleWrite.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_PCE_ruleEntry_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PceRuleDelete(const GSW_Device_t *dev, GSW_PCE_ruleEntry_t *parm);

GSW_return_t GSW_PceRuleMove(const GSW_Device_t *dev, GSW_PCE_rule_move_t *parm);

/** @cond INTERNAL */
GSW_return_t GSW_DumpTable(const GSW_Device_t *dev, GSW_table_t *parm);
/** @endcond */

/** @}*/ /* GSW_PCE */

/* Bridge */

/** \addtogroup GSW_ETHERNET_BRIDGING
 *  @{
 */

/**
   \brief Allocate Bridge. Valid for GSWIP-3.1.
   It is used to allocate a bridge. Bridge 0 is always available as default
   bridge.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_BRIDGE_alloc_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_BridgeAlloc(const GSW_Device_t *dev, GSW_BRIDGE_alloc_t *parm);

/**
   \brief Delete Bridge. Valid for GSWIP-3.1.
   It is used to release bridge allocated with \ref GSW_BridgeAlloc.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_BRIDGE_alloc_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_BridgeFree(const GSW_Device_t *dev, GSW_BRIDGE_alloc_t *parm);

/**
   \brief Config Bridge. Valid for GSWIP-3.1.
   It is used to configure bridge. If \ref GSW_BRIDGE_config_t::eMask has
   \ref GSW_BridgeConfigMask_t::GSW_BRIDGE_CONFIG_MASK_FORCE,
   \ref GSW_BRIDGE_config_t::nBridgeId is absolute index of Bridge (FID) in
   hardware for debug purpose, bypassing any check.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_BRIDGE_config_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_BridgeConfigSet(const GSW_Device_t *dev, GSW_BRIDGE_config_t *parm);

/**
   \brief get configuration of Bridge. Valid for GSWIP-3.1.
   It is used to retrieve bridge configuration.
   If \ref GSW_BRIDGE_config_t::eMask has
   \ref GSW_BridgeConfigMask_t::GSW_BRIDGE_CONFIG_MASK_FORCE,
   \ref GSW_BRIDGE_config_t::nBridgeId is absolute index of Bridge (FID) in
   hardware for debug purpose, bypassing any check.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_BRIDGE_config_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_BridgeConfigGet(const GSW_Device_t *dev, GSW_BRIDGE_config_t *parm);


/* Bridge Port*/
/**
   \brief Allocate Bridge Port. Valid for GSWIP-3.1.
   It is used to allocate a bridge port.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_BRIDGE_portAlloc_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_BridgePortAlloc(const GSW_Device_t *dev, GSW_BRIDGE_portAlloc_t *parm);

/**
   \brief Config Bridge Port. Valid for GSWIP-3.1.
   It is used to configure bridge port. If \ref GSW_BRIDGE_portConfig_t::eMask
   has \ref GSW_BridgePortConfigMask_t::GSW_BRIDGE_PORT_CONFIG_MASK_FORCE,
   \ref GSW_BRIDGE_portConfig_t::nBridgePortId is absolute index of Bridge Port
   in hardware for debug purpose, bypassing any check.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_BRIDGE_portConfig_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_BridgePortConfigSet(const GSW_Device_t *dev, GSW_BRIDGE_portConfig_t *parm);

/**
   \brief get configuration of Bridge Port. Valid for GSWIP-3.1.
   It is used to retrieve bridge port configuration.
   If \ref GSW_BRIDGE_portConfig_t::eMask has
   \ref GSW_BridgePortConfigMask_t::GSW_BRIDGE_PORT_CONFIG_MASK_FORCE,
   \ref GSW_BRIDGE_portConfig_t::nBridgePortId is absolute index of Bridge Port
   in hardware for debug purpose, bypassing any check

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_BRIDGE_portConfig_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_BridgePortConfigGet(const GSW_Device_t *dev, GSW_BRIDGE_portConfig_t *parm);

/**
   \brief Delete Bridge Port. Valid for GSWIP-3.1.
   It is used to release bridge port allocated with
   \ref GSW_BridgePortConfigSet.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_BRIDGE_portAlloc_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_BridgePortFree(const GSW_Device_t *dev, GSW_BRIDGE_portAlloc_t *parm);

/**
   \brief Read and clear loop violation counter.
   It is used for loop detection implementation.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_BRIDGE_portLoopRead_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_BridgePortLoopRead(const GSW_Device_t *dev, GSW_BRIDGE_portLoopRead_t *parm);

/** @}*/ /* GSW_ETHERNET_BRIDGING */


/* CTP Port */

/** \addtogroup GSW_ETHERNET_BRIDGING
 *  @{
 */

/**
   \brief Assign CTP to Logical Port. Valid for GSWIP-3.1.
   It is used to allocate a range of CTP and associate them to Logical Port.
   Apart from setting proper mode, it will do basic mapping between CTP and
   Bridge Port, then enable SDMA to allow ingress traffic from this port.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_CTP_portAssignment_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_CTP_PortAssignmentAlloc(const GSW_Device_t *dev, GSW_CTP_portAssignment_t *parm);

/**
   \brief Free CTP from Logical Port. Valid for GSWIP-3.1.
   It is used to stop association between CTP and Logical port. And it will stop
   SDMA so that the ingress traffic from this port is stopped.
*/
GSW_return_t GSW_CTP_PortAssignmentFree(const GSW_Device_t *dev, GSW_CTP_portAssignment_t *parm);

/**
   \brief Assign CTP Ports to logical port. Valid for GSWIP-3.1.
   It is used to associate a range of CTP ports to logical port and set proper
   mode.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_CTP_portAssignment_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_CTP_PortAssignmentSet(const GSW_Device_t *dev, GSW_CTP_portAssignment_t *parm);

/**
   \brief Get CTP Ports assignment from logical port. Valid for GSWIP-3.1.
   It is used to retrieve CTP ports range of logical port and the mode of port.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_CTP_portAssignment_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_CTP_PortAssignmentGet(const GSW_Device_t *dev, GSW_CTP_portAssignment_t *parm);

/**
   \brief Config CTP Port. Valid for GSWIP-3.1.
   It is used to setup CTP port.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_CTP_portConfig_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_CtpPortConfigSet(const GSW_Device_t *dev, GSW_CTP_portConfig_t *parm);

/**
   \brief get configuration of CTP Port. Valid for GSWIP-3.1.
   It is used to retrieve CTP port configuration.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_CTP_portConfig_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_CtpPortConfigGet(const GSW_Device_t *dev, GSW_CTP_portConfig_t *parm);

/**
   \brief Reset configuration of CTP (Connectivity Termination Point). Valid for GSWIP-3.1.
   It is used to reset CTP configuration and, if necessary, release resources
   such as Extended VLAN, Meter, etc. Bridge Port ID will not be changed.
   If \ref GSW_CTP_portConfig_t::eMask has
   \ref GSW_CtpPortConfigMask_t::GSW_CTP_PORT_CONFIG_MASK_FORCE,
   \ref GSW_CTP_portConfig_t::nLogicalPortId is ignored and
   \ref GSW_CTP_portConfig_t::nSubIfIdGroup is absolute index of CTP in hardware
   for debug purpose, bypassing any check.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_CTP_portConfig_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_CtpPortConfigReset(const GSW_Device_t *dev, GSW_CTP_portConfig_t *parm);

/** @}*/ /* GSW_ETHERNET_BRIDGING  */


/* QoS */

/** \addtogroup GSW_QoS_SVC
 *  @{
 */

/** \brief This command configures the parameters of a rate meter instance.
    This instance can be assigned to an ingress/egress port by
    using \ref GSW_BridgePortConfigSet and \ref GSW_CtpPortConfigSet.
    It can also be used by the flow classification engine.
    The total number of available rate meters can be retrieved by the
    capability list using \ref GSW_CapGet.
    The current configuration of a meter instance can be retrieved
    using \ref GSW_QoS_MeterCfgGet.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_meterCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_MeterCfgSet(const GSW_Device_t *dev, GSW_QoS_meterCfg_t *parm);

/** \brief Configure the parameters of a rate meter instance.
    This instance can be assigned to an ingress/egress port
    using \ref GSW_BridgePortConfigSet and \ref GSW_CtpPortConfigSet.
    It can also be used by the flow classification engine.
    The total number of available rate meters can be retrieved by the
    capability list using \ref GSW_CapGet.
    The current configuration of a meter instance can be retrieved
    using \ref GSW_QoS_MeterCfgGet.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_meterCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_MeterCfgGet(const GSW_Device_t *dev, GSW_QoS_meterCfg_t *parm);

/**
   \brief Read out the QoS 64 DSCP mapping to the switch priority queues.
   The table configuration can be set using \ref GSW_QoS_DSCP_ClassSet.

   \param dev Pointer to switch device.
   \param parm Pointer to the QoS filter parameters
   \ref GSW_QoS_DSCP_ClassCfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_DSCP_ClassGet(const GSW_Device_t *dev, GSW_QoS_DSCP_ClassCfg_t *parm);

/**
   \brief Initialize the QoS 64 DSCP mapping to the switch priority queues.
   This configuration applies for the whole switch device. The table
   configuration can be read using \ref GSW_QoS_DSCP_ClassGet.

   \param dev Pointer to switch device.
   \param parm Pointer to the QoS filter parameters
   \ref GSW_QoS_DSCP_ClassCfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_DSCP_ClassSet(const GSW_Device_t *dev, GSW_QoS_DSCP_ClassCfg_t *parm);

/**
   \brief Configures the DSCP to Drop Precedence assignment mapping table.
   This mapping table is used to identify the switch internally used drop
   precedence based on the DSCP value of the incoming packet.
   The current mapping table configuration can be read
   using \ref GSW_QoS_DSCP_DropPrecedenceCfgGet.

   \param dev Pointer to switch device.
   \param parm Pointer to the QoS
   DSCP drop precedence parameters
   \ref GSW_QoS_DSCP_DropPrecedenceCfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_DSCP_DropPrecedenceCfgSet(const GSW_Device_t *dev, GSW_QoS_DSCP_DropPrecedenceCfg_t *parm);

/**
   \brief Read out the current DSCP to Drop Precedence assignment mapping table.
   The table can be configured
   using \ref GSW_QoS_DSCP_DropPrecedenceCfgSet.

   \param dev Pointer to switch device.
   \param parm Pointer to the QoS
   DSCP drop precedence parameters
   \ref GSW_QoS_DSCP_DropPrecedenceCfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_DSCP_DropPrecedenceCfgGet(const GSW_Device_t *dev, GSW_QoS_DSCP_DropPrecedenceCfg_t *parm);

/**
   \brief Port Remarking Configuration. Ingress and Egress remarking options for
   DSCP and PCP. Remarking is done either on the used traffic class or
   the drop precedence.
   The current configuration can be read
   using \ref GSW_QoS_PortRemarkingCfgGet.

   \param dev Pointer to switch device.
   \param parm Pointer to the remarking configuration
   \ref GSW_QoS_portRemarkingCfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_PortRemarkingCfgSet(const GSW_Device_t *dev, GSW_QoS_portRemarkingCfg_t *parm);

/**
   \brief Read out the Port Remarking Configuration. Ingress and Egress remarking options for
   DSCP and PCP. Remarking is done either on the used traffic class or
   the drop precedence.
   The current configuration can be set
   using \ref GSW_QoS_PortRemarkingCfgSet.

   \param dev Pointer to switch device.
   \param parm Pointer to the QoS filter parameters
   \ref GSW_QoS_portRemarkingCfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_PortRemarkingCfgGet(const GSW_Device_t *dev, GSW_QoS_portRemarkingCfg_t *parm);

/**
   \brief Read out the incoming PCP to traffic class mapping table.
   The table configuration can be set using \ref GSW_QoS_PCP_ClassSet.

   \param dev Pointer to switch device.
   \param parm Pointer to the QoS filter parameters
   \ref GSW_QoS_PCP_ClassCfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_PCP_ClassGet(const GSW_Device_t *dev, GSW_QoS_PCP_ClassCfg_t *parm);

/**
   \brief Configure the incoming PCP to traffic class mapping table.
   This configuration applies to the entire switch device.
   The table configuration can be read using \ref GSW_QoS_PCP_ClassGet.

   \param dev Pointer to switch device.
   \param parm Pointer to the QoS filter parameters
   \ref GSW_QoS_PCP_ClassCfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_PCP_ClassSet(const GSW_Device_t *dev, GSW_QoS_PCP_ClassCfg_t *parm);

/**
   \brief Read out the current Ethernet port traffic class of ingress packets.
   It is used to identify the packet priority and the related egress
   priority queue. The port configuration can be set
   using \ref GSW_QoS_PortCfgSet.

   \param dev Pointer to switch device.
   \param parm Pointer to a
      QOS port priority control configuration \ref GSW_QoS_portCfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_PortCfgGet(const GSW_Device_t *dev, GSW_QoS_portCfg_t *parm);

/**
   \brief Configures the Ethernet port based traffic class assignment of ingress packets.
   It is used to identify the packet priority and the related egress
   priority queue. For DSCP, the priority to queue assignment is done
   using \ref GSW_QoS_DSCP_ClassSet.
   For VLAN, the priority to queue assignment is done
   using \ref GSW_QoS_PCP_ClassSet. The current port configuration can be
   read using \ref GSW_QoS_PortCfgGet.

   \param dev Pointer to switch device.
   \param parm Pointer to a
      QOS port priority control configuration \ref GSW_QoS_portCfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_PortCfgSet(const GSW_Device_t *dev, GSW_QoS_portCfg_t *parm);

/** \brief This configuration decides how the egress queues, attached to a single port,
    are scheduled to transmit the queued Ethernet packets.
    The configuration differentiates between 'Strict Priority' and
    'weighted fair queuing'. This applies when multiple egress queues are
    assigned to an Ethernet port.
    Using the WFQ feature on a port requires the configuration of weights on all
    given queues that are assigned to that port.
    Strict Priority means that no dedicated weight is configured and the
    queue can transmit following its priority status.
    The given configuration can be read out
    using \ref GSW_QoS_SchedulerCfgGet.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_schedulerCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_SchedulerCfgSet(const GSW_Device_t *dev, GSW_QoS_schedulerCfg_t *parm);

/** \brief Read out the current scheduler configuration of a given egress port. This
    configuration can be modified
    using \ref GSW_QoS_SchedulerCfgSet.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_schedulerCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_SchedulerCfgGet(const GSW_Device_t *dev, GSW_QoS_schedulerCfg_t *parm);

/** \brief This command configures a rate shaper instance with the rate and the
    burst size. This instance can be assigned to QoS queues by
    using \ref GSW_QoS_ShaperQueueAssign.
    The total number of available rate shapers can be retrieved by the
    capability list using \ref GSW_CapGet.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_ShaperCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_ShaperCfgSet(const GSW_Device_t *dev, GSW_QoS_ShaperCfg_t *parm);

/** \brief This command retrieves the rate and the burst size configuration of a
    rate shaper instance. A configuration can be modified
    using \ref GSW_QoS_ShaperCfgSet.
    The total number of available rate shapers can be retrieved by the
    capability list using \ref GSW_CapGet.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_ShaperCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_ShaperCfgGet(const GSW_Device_t *dev, GSW_QoS_ShaperCfg_t *parm);

/** \brief Assign one rate shaper instance to a QoS queue. The function returns with an
    error in case there already are too many shaper instances assigned to a queue.
    The queue instance can be enabled and configured
    using \ref GSW_QoS_ShaperCfgSet.
    To remove a rate shaper instance from a QoS queue,
    please use \ref GSW_QoS_ShaperQueueDeassign.
    The total number of available rate shaper instances can be retrieved by the
    capability list using \ref GSW_CapGet.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_ShaperQueue_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_ShaperQueueAssign(const GSW_Device_t *dev, GSW_QoS_ShaperQueue_t *parm);

/** \brief De-assign or Unassign one rate shaper instance from a QoS queue. The function returns
    with an error in case the requested instance is not currently assigned
    to the queue.
    The queue instance can be enabled and configured by
    using \ref GSW_QoS_ShaperCfgSet.
    To assign a rate shaper instance to a QoS queue,
    please use \ref GSW_QoS_ShaperQueueAssign.
    The total number of available rate shapers can be retrieved by the
    capability list using \ref GSW_CapGet.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_ShaperQueue_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_ShaperQueueDeassign(const GSW_Device_t *dev, GSW_QoS_ShaperQueue_t *parm);

/** \brief Check whether a rate shaper instance is assigned to the egress queue.
    The egress queue index is the function input parameter.
    The switch API sets the boolean parameter 'bAssigned == 1' in case a
    rate shaper is assigned and then sets 'nRateShaperId' to describe the rater
    shaper instance.
    The parameter 'bAssigned == 0' in case no rate shaper instance
    is currently assigned to the queue instance.
    The commands \ref GSW_QoS_ShaperQueueAssign allow a
    rate shaper instance to be assigned, and \ref GSW_QoS_ShaperCfgSet allows
    for configuration of a shaper instance.
    The total number of available rate shapers can be retrieved by the
    capability list using \ref GSW_CapGet.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_ShaperQueueGet_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_ShaperQueueGet(const GSW_Device_t *dev, GSW_QoS_ShaperQueueGet_t *parm);

/** \brief This command configures one meter instances for storm control.
    These instances can be used for ingress broadcast-, multicast- and
    unknown unicast- packets. Some platforms support addition of additional meter
    instances for this type of packet.
    Repeated calls of \ref GSW_QoS_StormCfgSet allow addition of
    additional meter instances.
    An assignment can be retrieved using \ref GSW_QoS_StormCfgGet.
    Setting the broadcast, multicast and unknown unicast packets boolean switch to zero
    deletes all metering instance assignments.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_stormCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_StormCfgSet(const GSW_Device_t *dev, GSW_QoS_stormCfg_t *parm);

/** \brief Reads out the current meter instance assignment for storm control. This
    configuration can be modified using \ref GSW_QoS_StormCfgSet.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_stormCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_StormCfgGet(const GSW_Device_t *dev, GSW_QoS_stormCfg_t *parm);

/** \brief Configures the global WRED drop probability profile and thresholds of the device.
    Given parameters are rounded to the segment size of the HW platform.
    The supported segment size is given by the capability list by
    using \ref GSW_CapGet.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_WRED_Cfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_WredCfgSet(const GSW_Device_t *dev, GSW_QoS_WRED_Cfg_t *parm);

/** \brief Read out the global WRED drop probability profile and thresholds of the device.
    Given parameters are rounded to the segment size of the HW platform.
    The supported segment size is given by the capability list by
    using \ref GSW_CapGet.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_WRED_Cfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_WredCfgGet(const GSW_Device_t *dev, GSW_QoS_WRED_Cfg_t *parm);

/** \brief Configures the WRED drop thresholds for a dedicated egress queue.
    Given parameters are rounded to the segment size of the HW platform.
    The supported segment size is given by the capability list by
    using \ref GSW_CapGet.
    The command \ref GSW_QoS_WredQueueCfgGet retrieves the current
    configuration.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_WRED_QueueCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_WredQueueCfgSet(const GSW_Device_t *dev, GSW_QoS_WRED_QueueCfg_t *parm);

/** \brief Read out the WRED drop thresholds for a dedicated egress queue.
    Given parameters are rounded to the segment size of the HW platform.
    The supported segment size is given by the capability list by
    using \ref GSW_CapGet.
    The configuration can be changed by
    using \ref GSW_QoS_WredQueueCfgSet.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_WRED_QueueCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_WredQueueCfgGet(const GSW_Device_t *dev, GSW_QoS_WRED_QueueCfg_t *parm);

/** \brief Configures the WRED drop thresholds for a dedicated egress port.
    Given parameters are rounded to the segment size of the HW platform.
    The supported segment size is given by the capability list by
    using \ref GSW_CapGet.
    The command \ref GSW_QoS_WredPortCfgGet retrieves the current
    configuration.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_WRED_PortCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_WredPortCfgSet(const GSW_Device_t *dev, GSW_QoS_WRED_PortCfg_t *parm);

/** \brief Read out the WRED drop thresholds for a dedicated egress port.
    Given parameters are rounded to the segment size of the HW platform.
    The supported segment size is given by the capability list by
    using \ref GSW_CapGet.
    The configuration can be changed by
    using \ref GSW_QoS_WredPortCfgSet.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_WRED_PortCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_WredPortCfgGet(const GSW_Device_t *dev, GSW_QoS_WRED_PortCfg_t *parm);

/** \brief Configures the global flow control thresholds for conforming and non-conforming packets.
    The configured thresholds apply to the global switch segment buffer.
    The current configuration can be retrieved by \ref GSW_QoS_FlowctrlCfgGet.
    Given parameters are rounded to the segment size of the HW platform.
    The supported segment size is given by the capability list by
    using \ref GSW_CapGet.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_FlowCtrlCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_FlowctrlCfgSet(const GSW_Device_t *dev, GSW_QoS_FlowCtrlCfg_t *parm);

/** \brief Read out the global flow control thresholds for conforming and non-conforming packets.
    The configured thresholds apply to the global switch segment buffer.
    The configuration can be changed by \ref GSW_QoS_FlowctrlCfgSet.
    Given parameters are rounded to the segment size of the HW platform.
    The supported segment size is given by the capability list by
    using \ref GSW_CapGet.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_FlowCtrlCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_FlowctrlCfgGet(const GSW_Device_t *dev, GSW_QoS_FlowCtrlCfg_t *parm);

/** \brief Configures the ingress port flow control thresholds for occupied buffer segments.
    The current configuration can be retrieved by \ref GSW_QoS_FlowctrlPortCfgGet.
    Given parameters are rounded to the segment size of the HW platform.
    The supported segment size is given by the capability list by
    using \ref GSW_CapGet.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_FlowCtrlPortCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_FlowctrlPortCfgSet(const GSW_Device_t *dev, GSW_QoS_FlowCtrlPortCfg_t *parm);

/** \brief Read out the ingress port flow control thresholds for occupied buffer segments.
    The configuration can be changed by \ref GSW_QoS_FlowctrlPortCfgSet.
    Given parameters are rounded to the segment size of the HW platform.
    The supported segment size is given by the capability list by
    using \ref GSW_CapGet.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_FlowCtrlPortCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_FlowctrlPortCfgGet(const GSW_Device_t *dev, GSW_QoS_FlowCtrlPortCfg_t *parm);

/** \brief Configure the egress queue buffer reservation.
    WRED GREEN packets are never dropped by any WRED algorithm (queue,
    port or global buffer level) in case they are below this reservation threshold.
    The amount of reserved segments cannot be occupied by other queues of the switch.
    The egress queue related configuration can be retrieved by
    calling \ref GSW_QoS_QueueBufferReserveCfgGet.

    \remarks
    The command \ref GSW_QoS_QueuePortSet allows to assign egress queue to ports with related traffic classes.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_QueueBufferReserveCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_QueueBufferReserveCfgSet(const GSW_Device_t *dev, GSW_QoS_QueueBufferReserveCfg_t *parm);

/** \brief Read out the egress queue specific buffer reservation.
    Configuration can be read by \ref GSW_QoS_QueueBufferReserveCfgSet.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_QueueBufferReserveCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_QueueBufferReserveCfgGet(const GSW_Device_t *dev, GSW_QoS_QueueBufferReserveCfg_t *parm);

/**
   \brief Update Color Marking Table.
   Should be used to setup color marking table at early stage.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_QoS_colorMarkingEntry_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QOS_ColorMarkingTableSet(const GSW_Device_t *dev, GSW_QoS_colorMarkingEntry_t *parm);

/**
   \brief Get Color Marking Table.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_QoS_colorMarkingEntry_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QOS_ColorMarkingTableGet(const GSW_Device_t *dev, GSW_QoS_colorMarkingEntry_t *parm);

/**
   \brief Update Color Remarking Table.
   Should be used to setup color remarking table at early stage.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_QoS_colorRemarkingEntry_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QOS_ColorReMarkingTableSet(const GSW_Device_t *dev, GSW_QoS_colorRemarkingEntry_t *parm);

/**
   \brief Get Color Remarking Table.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_QoS_colorRemarkingEntry_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QOS_ColorReMarkingTableGet(const GSW_Device_t *dev, GSW_QoS_colorRemarkingEntry_t *parm);

/**
   \brief Allocate Meter.
   This is a part of APIs to manage meters. This API works in 2 modes. If
   \ref GSW_QoS_meterCfg_t::nMeterId is \ref INVALID_HANDLE, this API will
   allocate a free meter, config it with all parameters in
   \ref GSW_QoS_meterCfg_t, and return the meter ID in
   \ref GSW_QoS_meterCfg_t::nMeterId. Otherwise,
   \ref GSW_QoS_meterCfg_t::nMeterId should be a valid meter ID, and this API
   increase the reference counter of this meter. Other fields are ignored.
   This API is wrapper of \ref GSW_QoS_MeterCfgSet with meter resource
   management.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_QoS_meterCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QOS_MeterAlloc(const GSW_Device_t *dev, GSW_QoS_meterCfg_t *parm);

/**
   \brief Free Meter.
   Decrease reference counter of the meter. If reference counter is 0, disable
   the meter then free it.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_QoS_meterCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QOS_MeterFree(const GSW_Device_t *dev, GSW_QoS_meterCfg_t *parm);

/**
   \brief Set DSCP to PCP Mapping Table.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_DSCP2PCP_map_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QOS_Dscp2PcpTableSet(const GSW_Device_t *dev, GSW_DSCP2PCP_map_t *parm);

/**
   \brief Get DSCP to PCP Mapping Table.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_DSCP2PCP_map_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QOS_Dscp2PcpTableGet(const GSW_Device_t *dev, GSW_DSCP2PCP_map_t *parm);

/**
   \brief Set P-mapper Configuration.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_PMAPPER_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QOS_PmapperTableSet(const GSW_Device_t *dev, GSW_PMAPPER_t *parm);

/**
   \brief Get P-mapper Configuration.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_PMAPPER_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QOS_PmapperTableGet(const GSW_Device_t *dev, GSW_PMAPPER_t *parm);

/**
   \brief Read out the PCP to traffic class mapping table.
   The table configuration can be set using \ref GSW_QoS_SVLAN_PCP_ClassSet.

   \param dev Pointer to switch device.
   \param parm Pointer to the QoS filter parameters
   \ref GSW_QoS_SVLAN_PCP_ClassCfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_SVLAN_PCP_ClassGet(const GSW_Device_t *dev, GSW_QoS_SVLAN_PCP_ClassCfg_t *parm);

/**
   \brief Configure the PCP to traffic class mapping table.
   This configuration applies to the entire switch device.
   The table configuration can be read using \ref GSW_QoS_SVLAN_PCP_ClassGet.

   \param dev Pointer to switch device.
   \param parm Pointer to the QoS filter parameters
   \ref GSW_QoS_SVLAN_PCP_ClassCfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_SVLAN_PCP_ClassSet(const GSW_Device_t *dev, GSW_QoS_SVLAN_PCP_ClassCfg_t *parm);

/** \brief Sets queue specific configuration.
    Recommend to modify (read, change write) instead of overwrite.
    Call \ref GSW_QoS_QueueCfgGet to retrieve current config, modify it, then
    use this API to set.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters structure \ref GSW_QoS_queueCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_QueueCfgSet(const GSW_Device_t *dev, GSW_QoS_queueCfg_t *parm);

/** \brief Read out queue specific configuration.
    Please note that the device comes along with a default configuration.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters structure \ref GSW_QoS_queueCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_QueueCfgGet(const GSW_Device_t *dev, GSW_QoS_queueCfg_t *parm);

/** \brief Sets the Queue ID for one traffic class of one port.
    The total amount of supported ports, queues and traffic classes can be
    retrieved from the capability list using \ref GSW_CapGet.
    Please note that the device comes along with a
    default configuration and assignment.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_queuePort_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_QueuePortSet(const GSW_Device_t *dev, GSW_QoS_queuePort_t *parm);

/** \brief Read out the traffic class and port assignment done
    using \ref GSW_QoS_QueuePortSet.
    Please note that the device comes along with a
    default configuration and assignment.

   \param dev Pointer to switch device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_queuePort_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_QueuePortGet(const GSW_Device_t *dev, GSW_QoS_queuePort_t *parm);

/** @}*/ /* GSW_QoS_SVC */


/* RMON */

/** \addtogroup GSW_RMON
 *  @{
 */

/**
   \brief Read out the Ethernet port statistic counter (RMON counter).
   The zero-based 'nPortId' structure element describes the physical switch
   port for the requested statistic information.

   \param dev Pointer to switch device.
   \param parm  Pointer to pre-allocated
   \ref GSW_RMON_Port_cnt_t structure. The structure element 'nPortId' is
   an input parameter that describes from which port to read the RMON counter.
   All remaining structure elements are filled with the counter values.

   \remarks The function returns an error in case the given 'nPortId' is
   out of range.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_RMON_Port_Get(const GSW_Device_t *dev, GSW_RMON_Port_cnt_t *parm);

/**
   \brief Configures a Traffic Statistic Counter (RMON counter).

   \param dev Pointer to switch device.
   \param parm  Pointer to a pre-allocated
   \ref GSW_RMON_mode_t structure. The structure elements 'eRmonType' and 'eRmonMode' are input parameters to set RMON counting mode to bytes or Packet based.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_RMON_Mode_Set(const GSW_Device_t *dev, GSW_RMON_mode_t *parm);

/**
   \brief Read out the Meter Instance statistic counter (RMON counter).
   The zero-based 'nMeterId' structure element describes the Meter Identifier
   instance for the requested statistic information.

   \param dev Pointer to switch device.
   \param parm  Pointer to pre-allocated stats placeholder
   \ref GSW_RMON_Meter_cnt_t structure. The structure element 'nMeterId' is
   an input parameter that describes from which Meter to read the RMON counter.
   All remaining structure elements are filled with the counter values.

   \remarks The function returns an error in case the given 'nMeterId' is
   out of range.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_RMON_Meter_Get(const GSW_Device_t *dev, GSW_RMON_Meter_cnt_t *parm);

/**
   \brief Clears all or specific identifier (e.g. Port Id or Meter Id) Statistic counter (RMON counter).

   \param dev Pointer to switch device.
   \param parm  Pointer to a pre-allocated
   \ref GSW_RMON_clear_t structure. The structure element 'nRmonId' is
   an input parameter stating on which identifier to clear RMON counters.

   \remarks The function returns an error in case the given 'nRmonId' is
   out of range for given 'nRmonType'

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_RMON_Clear(const GSW_Device_t *dev, GSW_RMON_clear_t *parm);

/**
   \brief Clears all or specific identifier (e.g. Port Id or Meter Id) Statistic counter (RMON counter).

   \param dev Pointer to switch device.
   \param parm  Pointer to a pre-allocated
   \ref GSW_RMON_clear_t structure. The structure element 'nRmonId' is
   an input parameter stating on which identifier to clear RMON counters.

   \remarks The function returns an error in case the given 'nRmonId' is
   out of range for given 'nRmonType'

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_RmonTflowClear(const GSW_Device_t *dev, GSW_RMON_flowGet_t *parm);

/**
   \brief Read out additional traffic flow (RMON) counters. GSWIP-3.1 only.

   \param dev Pointer to switch device.
   \param parm  Pointer to a pre-allocated
   \ref GSW_RMON_flowGet_t structure.

   \remarks The function returns an error in case the given 'nIndex' is
   out of range.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_RMON_FlowGet(const GSW_Device_t *dev, GSW_RMON_flowGet_t *parm);

/**
   \brief Clears all or specific identifier (e.g. Port Id or Meter Id) Statistic counter (RMON counter).

   \param dev Pointer to switch device.
   \param parm  Pointer to a pre-allocated
   \ref GSW_RMON_clear_t structure. The structure element 'nRmonId' is
   an input parameter stating on which identifier to clear RMON counters.

   \remarks The function returns an error in case the given 'nRmonId' is
   out of range for given 'nRmonType'

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_RmonTflowClear(const GSW_Device_t *dev, GSW_RMON_flowGet_t *parm);

/** @}*/ /* GSW_RMON */


/** \addtogroup GSW_RMON
 *  @{
 */

/**
   \brief Sets TFLOW counter mode type.

   \param dev Pointer to switch device.
   \param parm  Pointer to a pre-allocated
   \ref GSW_RMON_clear_t structure.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_TflowCountModeSet(const GSW_Device_t *dev, GSW_TflowCmodeConf_t *parm);

/**
   \brief Sets TFLOW counter mode type.

   \param dev Pointer to switch device.
   \param parm  Pointer to a pre-allocated
   \ref GSW_RMON_clear_t structure.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_TflowCountModeGet(const GSW_Device_t *dev, GSW_TflowCmodeConf_t *parm);

/** @}*/ /* GSW_RMON */


/* Debug */

/**
   \brief Get per port RMON counters

   \param dev Pointer to switch device.
   \param parm  Pointer to a counter structure \ref GSW_Debug_RMON_Port_cnt_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_Debug_RMON_Port_Get(const GSW_Device_t *dev, GSW_Debug_RMON_Port_cnt_t *parm);

/** @cond INTERNAL */
GSW_return_t GSW_Debug_MeterTableStatus(const GSW_Device_t *dev, GSW_debug_t *parm);
/** @endcond */

/* PMAC */

/** \addtogroup GSW_PMAC
 *  @{
 */

/**
   \brief Reads the Counters for given DMA Channel Identifier associated to PMAC.
   It is used to read ingress statistics counters providing discarded packets and bytes on given PMAC port.

   \param dev Pointer to switch device.
   \param parm Pointer to a
      Statistics Counters of DMA channel associated to PMAC \ref GSW_PMAC_Cnt_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PMAC_CountGet(const GSW_Device_t *dev, GSW_PMAC_Cnt_t *parm);

/**
   \brief Writes the global PMAC settings applicable to GSWIP-3.0 PMAC ports.
   It is used to configure the global settings such as Padding, Checksum, Length and Egress PMAC Selector fields.

   \param dev Pointer to switch device.
   \param parm Pointer to a
      global config of PMAC \ref GSW_PMAC_Glbl_Cfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PMAC_GLBL_CfgSet(const GSW_Device_t *dev, GSW_PMAC_Glbl_Cfg_t *parm);

/**
   \brief Reads the global PMAC settings currently configured to GSWIP-3.0 PMAC ports.
   It is used to configure the global stetinsg such as PAdding, Checksum, Length and Egress PMAC Selector fields.

   \param dev Pointer to switch device.
   \param parm Pointer to a
      current global config of PMAC \ref GSW_PMAC_Glbl_Cfg_t (returned).

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PMAC_GLBL_CfgGet(const GSW_Device_t *dev, GSW_PMAC_Glbl_Cfg_t *parm);

/**
   \brief Configures the Backpressure Mapping Table for PMAC.
   It is used to configure backpressure mapping table between Tx Queues for Egress and Rx Ports for Ingress congestion on given DMA channel.
   The Backpressure mapping can also be read using \ref GSW_PMAC_BM_CfgGet.

   \param dev Pointer to switch device.
   \param parm Pointer to a
      Backpressure mapping configuration \ref GSW_PMAC_BM_Cfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PMAC_BM_CfgSet(const GSW_Device_t *dev, GSW_PMAC_BM_Cfg_t *parm);

/**
   \brief Queries the Backpressure Mapping Table for PMAC.
   It is used to read backpressure mapping table between Tx Queues for Egress and Rx Ports for Ingress congestion on given DMA channel.
   The mapping config can be written using \ref GSW_PMAC_BM_CfgSet.

   \param dev Pointer to switch device.
   \param parm Pointer to a
      Backpressure mapping configuration \ref GSW_PMAC_BM_Cfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PMAC_BM_CfgGet(const GSW_Device_t *dev, GSW_PMAC_BM_Cfg_t *parm);

/**
   \brief Configures the Ingress PMAC Attributes Config for Receive DMA Channel.
   It is used to configure ingress attributes on given Receive DMA channel.
   The PMAC Ingress config can also be read using \ref GSW_PMAC_IG_CfgGet.

   \param dev Pointer to switch device.
   \param parm Pointer to a
      Ingress PMAC Attributes configuration \ref GSW_PMAC_Ig_Cfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PMAC_IG_CfgSet(const GSW_Device_t *dev, GSW_PMAC_Ig_Cfg_t *parm);

/**
   \brief Queries the Ingress PMAC Attributes Config for given receive DMA channel.
   It is used to read ingress PMAC attributes config on given DMA Rx channel.
   The PMAC Ingress config can also be set using \ref GSW_PMAC_IG_CfgSet.

   \param dev Pointer to switch device.
   \param parm Pointer to a
      Ingress PMAC Attributes configuration \ref GSW_PMAC_Ig_Cfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PMAC_IG_CfgGet(const GSW_Device_t *dev, GSW_PMAC_Ig_Cfg_t *parm);

/**
   \brief Configures the Egress Attributes Config for a given PMAC port.
   It is used to configure egress attributes on given PMAC port.
   The PMAC Egress config can also be read using \ref GSW_PMAC_EG_CfgGet.

   \param dev Pointer to switch device.
   \param parm Pointer to a
      Egress PMAC Attributes configuration \ref GSW_PMAC_Eg_Cfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PMAC_EG_CfgSet(const GSW_Device_t *dev, GSW_PMAC_Eg_Cfg_t *parm);

/**
   \brief Queries the Egress Attributes Config for given PMAC port.
   It is used to read egress attributes config on given PMAC port.
   The PMAC Egress config can also be set using \ref GSW_PMAC_EG_CfgSet.

   \param dev Pointer to switch device.
   \param parm Pointer to a
      Egress PMAC Attributes configuration \ref GSW_PMAC_Eg_Cfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PMAC_EG_CfgGet(const GSW_Device_t *dev, GSW_PMAC_Eg_Cfg_t *parm);

/**@}*/ /* GSW_PMAC */


/* MAC Table */
#ifdef CONFIG_GSWIP_MAC

/** \addtogroup GSW_ETHERNET_BRIDGING
 *  @{
 */

/**
   \brief Remove all MAC entries from the MAC table.

   \param dev Pointer to switch device.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MAC_TableClear(const GSW_Device_t *dev);

/**
   \brief Remove MAC entries meeting criteria from the MAC table.

   \param dev Pointer to switch device.
   \param parm Pointer to condition structure
   \ref GSW_MAC_tableClearCond_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MAC_TableClearCond(const GSW_Device_t *dev, GSW_MAC_tableClearCond_t *parm);

/**
   \brief Add a MAC table entry. If an entry already exists for the given MAC Address
   in Filtering Database, this entry is overwritten. If not,
   a new entry is added.

   \param dev Pointer to switch device.
   \param parm Pointer to a MAC table entry
   \ref GSW_MAC_tableAdd_t structure.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MAC_TableEntryAdd(const GSW_Device_t *dev, GSW_MAC_tableAdd_t *parm);

/**
   \brief Search the MAC Address table for a specific address entry.
   A MAC address is provided by the application and Switch API
   performs a search operation on the hardware's MAC address table.
   Many hardware platforms provide an optimized and fast address search algorithm.

   \param dev Pointer to switch device.
   \param parm Pointer to a MAC table entry
   \ref GSW_MAC_tableQuery_t structure that is filled out by the switch
   implementation.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MAC_TableEntryQuery(const GSW_Device_t *dev, GSW_MAC_tableQuery_t *parm);

/**
   \brief Read an entry of the MAC table.
   If the parameter 'bInitial=TRUE', the GET operation starts at the beginning
   of the table. Otherwise it continues the GET operation at the entry that
   follows the previous access.
   The function sets all fields to zero in case the end of the table is reached.
   In order to read out the complete table, this function can be called in a loop.
   The Switch API sets 'bLast=1' when the last entry is read out.
   This 'bLast' parameter could be the loop exit criteria.

   \param dev Pointer to switch device.
   \param parm Pointer to a MAC table entry
   \ref GSW_MAC_tableRead_t structure that is filled out by the switch driver.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MAC_TableEntryRead(const GSW_Device_t *dev, GSW_MAC_tableRead_t *parm);

/**
   \brief Remove a single MAC entry from the MAC table.

   \param dev Pointer to switch device.
   \param parm Pointer to a MAC table entry
   \ref GSW_MAC_tableRemove_t structure.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MAC_TableEntryRemove(const GSW_Device_t *dev, GSW_MAC_tableRemove_t *parm);

/**
   \brief Set default MAC address filter

   \param dev Pointer to switch device.
   \param parm Pointer to a MAC address and filter type
   \ref GSW_MACFILTER_default_t structure.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_DefaultMacFilterSet(const GSW_Device_t *dev, GSW_MACFILTER_default_t *parm);

/**
   \brief Get default MAC address filter

   \param dev Pointer to switch device.
   \param parm Pointer to a MAC address and filter type
   \ref GSW_MACFILTER_default_t structure.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_DefaultMacFilterGet(const GSW_Device_t *dev, GSW_MACFILTER_default_t *parm);

/**
   \brief Detect MAC violation for input map of bridge ports
   Detect MAC violation for bridge ports in input bitmap and output a bitmap
   from which ports the MAC violation is found. This API is time consuming
   at millisecond level.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_MAC_tableLoopDetect_t structure.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MAC_TableLoopDetect(const GSW_Device_t *dev, GSW_MAC_tableLoopDetect_t *parm);
/**@}*/ /* GSW_ETHERNET_BRIDGING */

#endif

/* VLAN */

/** \addtogroup GSW_VLAN
 *  @{
 */

#ifdef CONFIG_GSWIP_EVLAN
/**
   \brief Allocate Extended VLAN Configuration block. Valid for GSWIP-3.1.
   It allocates consecutive VLAN configuration entries and return the block ID
   for further operations: \ref GSW_ExtendedVlanFree, \ref GSW_ExtendedVlanSet
   and \ref GSW_ExtendedVlanGet.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_EXTENDEDVLAN_alloc_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_ExtendedVlanAlloc(const GSW_Device_t *dev, GSW_EXTENDEDVLAN_alloc_t *parm);

/**
   \brief Set Extended VLAN Configuration entry. Valid for GSWIP-3.1.
   It is used to set Extended VLAN Configuration entry with index
   \ref GSW_EXTENDEDVLAN_config_t::nEntryIndex, ranging between 0 and
   \ref GSW_EXTENDEDVLAN_alloc_t::nNumberOfEntries - 1, with valid
   \ref GSW_EXTENDEDVLAN_config_t::nExtendedVlanBlockId returned by
   \ref GSW_ExtendedVlanAlloc.
   If \ref GSW_EXTENDEDVLAN_config_t::nExtendedVlanBlockId is
   \ref INVALID_HANDLE, this is absolute index of Extended VLAN Configuration
   entry in hardware, used for debugging purpose.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_EXTENDEDVLAN_config_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_ExtendedVlanSet(const GSW_Device_t *dev, GSW_EXTENDEDVLAN_config_t *parm);

/**
   \brief Get Extended VLAN Configuration entry. Valid for GSWIP-3.1.
   It is used to get Extended VLAN Configuration entry with index
   \ref GSW_EXTENDEDVLAN_config_t::nEntryIndex, ranging between 0 and
   \ref GSW_EXTENDEDVLAN_alloc_t::nNumberOfEntries - 1, with valid
   \ref GSW_EXTENDEDVLAN_config_t::nExtendedVlanBlockId returned by
   \ref GSW_ExtendedVlanAlloc.
   If \ref GSW_EXTENDEDVLAN_config_t::nExtendedVlanBlockId is
   \ref INVALID_HANDLE, this is absolute index of Extended VLAN Configuration
   entry in hardware, used for debugging purpose.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_EXTENDEDVLAN_config_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_ExtendedVlanGet(const GSW_Device_t *dev, GSW_EXTENDEDVLAN_config_t *parm);

/**
   \brief Release Extended VLAN Configuration block. Valid for GSWIP-3.1.
   It is used to release Extended VLAN Configuration block allocated by
   \ref GSW_ExtendedVlanAlloc.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_EXTENDEDVLAN_alloc_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_ExtendedVlanFree(const GSW_Device_t *dev, GSW_EXTENDEDVLAN_alloc_t *parm);

/** @cond INTERNAL */
GSW_return_t GSW_Debug_ExvlanTableStatus(const GSW_Device_t *dev, GSW_debug_t *parm);

GSW_return_t GSW_Debug_VlanFilterTableStatus(const GSW_Device_t *dev, GSW_debug_t *parm);
/** @endcond */

/**
   \brief Allocate VLAN Filter block. Valid for GSWIP-3.1.
   It allocates consecutive VLAN Filter entries and return the block ID
   for further operations: \ref GSW_VlanFilterFree, \ref GSW_VlanFilterSet
   and \ref GSW_VlanFilterGet.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_VLANFILTER_alloc_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_VlanFilterAlloc(const GSW_Device_t *dev, GSW_VLANFILTER_alloc_t *parm);

/**
   \brief Set VLAN Filter entry. Valid for GSWIP-3.1.
   It is used to set VLAN Filter entry with index
   \ref GSW_VLANFILTER_config_t::nEntryIndex, ranging between 0 and
   \ref GSW_VLANFILTER_alloc_t::nNumberOfEntries - 1, with valid
   \ref GSW_VLANFILTER_config_t::nVlanFilterBlockId returned by
   \ref GSW_VlanFilterAlloc.
   If \ref GSW_VLANFILTER_config_t::nVlanFilterBlockId is \ref INVALID_HANDLE,
   this is absolute index of VLAN Filter entry in hardware, used for debugging
   purpose.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_VLANFILTER_config_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_VlanFilterSet(const GSW_Device_t *dev, GSW_VLANFILTER_config_t *parm);

/**
   \brief Get VLAN Filter Entry. Valid for GSWIP-3.1.
   It is used to get VLAN filter entry with index
   \ref GSW_VLANFILTER_config_t::nEntryIndex, ranging between 0 and
   \ref GSW_VLANFILTER_alloc_t::nNumberOfEntries - 1, with valid
   \ref GSW_VLANFILTER_config_t::nVlanFilterBlockId returned by
   \ref GSW_VlanFilterAlloc.
   If \ref GSW_VLANFILTER_config_t::nVlanFilterBlockId is \ref INVALID_HANDLE,
   this is absolute index of VLAN Filter entry in hardware, used for debugging
   purpose.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_VLANFILTER_config_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_VlanFilterGet(const GSW_Device_t *dev, GSW_VLANFILTER_config_t *parm);

/**
   \brief Delete VLAN Filter Block. Valid for GSWIP-3.1.
   It is used to release VLAN Filter block allocated by
   \ref GSW_VlanFilterAlloc.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_VLANFILTER_alloc_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_VlanFilterFree(const GSW_Device_t *dev, GSW_VLANFILTER_alloc_t *parm);

/**
   \brief Configure VLAN RMON Counters.
   It is used to enable/disable VLAN RMON counters, configure whether to count
   broadcast packets, max number of VLAN entries to count.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_VLAN_RMON_control_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_Vlan_RMONControl_Set(const GSW_Device_t *dev, GSW_VLAN_RMON_control_t *parm);

/**
   \brief Read VLAN RMON Counters Configuration.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_VLAN_RMON_control_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_Vlan_RMONControl_Get(const GSW_Device_t *dev, GSW_VLAN_RMON_control_t *parm);

/**
   \brief Read VLAN RMON Counters of Specific Entry.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_VLAN_RMON_cnt_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_Vlan_RMON_Get(const GSW_Device_t *dev, GSW_VLAN_RMON_cnt_t *parm);

/**
   \brief Clear VLAN RMON Counters of Specific Entry or All Entries.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_VLAN_RMON_cnt_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_Vlan_RMON_Clear(const GSW_Device_t *dev, GSW_VLAN_RMON_cnt_t *parm);

/**
   \brief Map VLAN Counter to CTP.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_VlanCounterMapping_config_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_VlanCounterMapSet(const GSW_Device_t *dev, GSW_VlanCounterMapping_config_t *parm);

/**
   \brief Get Mapping between VLAN Counter and CTP.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_VlanCounterMapping_config_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_VlanCounterMapGet(const GSW_Device_t *dev, GSW_VlanCounterMapping_config_t *parm);

#endif
/**@}*/ /* GSW_VLAN */


/* Multicast */

/** \addtogroup GSW_MULTICAST
 *  @{
 */

#ifdef CONFIG_GSWIP_MCAST
/**
   \brief Add static router port to the switch hardware multicast table.
   These added router ports will not be removed by the router port learning aging process.
   The router port learning is enabled over the parameter 'bLearningRouter'
   over the \ref GSW_MulticastSnoopCfgGet command.
   Router port learning and static added entries can both be used together.
   In case of a software IGMP stack/daemon environment, the router port learning does
   not have to be configured on the switch hardware. Instead the router port
   management is handled by the IGMP stack/daemon.
   A port can be removed using \ref GSW_MulticastRouterPortRemove.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_multicastRouter_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MulticastRouterPortAdd(const GSW_Device_t *dev, GSW_multicastRouter_t *parm);

/**
   \brief Check if a port has been selected as a router port, either by automatic learning or by manual setting.
   A port can be added using \ref GSW_MulticastRouterPortAdd.
   A port can be removed again using \ref GSW_MulticastRouterPortRemove.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_multicastRouterRead_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs (e.g. Ethernet port parameter out of range)
*/
GSW_return_t GSW_MulticastRouterPortRead(const GSW_Device_t *dev, GSW_multicastRouterRead_t *parm);

/**
   \brief Remove an Ethernet router port from the switch hardware multicast table.
   A port can be added using \ref GSW_MulticastRouterPortAdd.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_multicastRouter_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs (e.g. Ethernet port parameter out of range)
*/
GSW_return_t GSW_MulticastRouterPortRemove(const GSW_Device_t *dev, GSW_multicastRouter_t *parm);

/**
   \brief Read out the current switch multicast configuration.
   The configuration can be set using \ref GSW_MulticastSnoopCfgSet.

   \param dev Pointer to switch device.
   \param parm Pointer to the
   multicast configuration \ref GSW_multicastSnoopCfg_t.

   \remarks IGMP/MLD snooping is disabled when
   'eIGMP_Mode = GSW_MULTICAST_SNOOP_MODE_SNOOPFORWARD'.
   Then all other structure parameters are unused.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MulticastSnoopCfgGet(const GSW_Device_t *dev, GSW_multicastSnoopCfg_t *parm);

/**
   \brief Configure the switch multicast configuration. The currently used
   configuration can be read using \ref GSW_MulticastSnoopCfgGet.

   \param dev Pointer to switch device.
   \param parm Pointer to the
   multicast configuration \ref GSW_multicastSnoopCfg_t.

   \remarks IGMP/MLD snooping is disabled when
   'eIGMP_Mode = GSW_MULTICAST_SNOOP_MODE_SNOOPFORWARD'.
   Then all other structure parameters are unused.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MulticastSnoopCfgSet(const GSW_Device_t *dev, GSW_multicastSnoopCfg_t *parm);

/**
   \brief Adds a multicast group configuration to the multicast table.
   No new entry is added in case this multicast group already
   exists in the table. This commands adds a host member to
   the multicast group.
   A member can be removed again using \ref GSW_MulticastTableEntryRemove.

   \param dev Pointer to switch device.
   \param parm Pointer
      to \ref GSW_multicastTable_t.

   \remarks The Source IP parameter is ignored in case IGMPv3 support is
      not enabled in the hardware.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MulticastTableEntryAdd(const GSW_Device_t *dev, GSW_multicastTable_t *parm);

/**
   \brief Read out the multicast membership table that is located inside the switch
   hardware. The 'bInitial' parameter restarts the read operation at the beginning of
   the table. Every following \ref GSW_MulticastTableEntryRead call reads out
   the next found entry. The 'bLast' parameter is set by the switch API in case
   the last entry of the table is reached.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_multicastTableRead_t.

   \remarks The 'bInitial' parameter is reset during the read operation.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MulticastTableEntryRead(const GSW_Device_t *dev, GSW_multicastTableRead_t *parm);

/**
   \brief Read specific multicast entry inside the switch hardware.
   \ref GSW_MulticastTableEntryRead iterates all valid entries in switch hardware,
   and this API reads single entry with specified index.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_multicastTableRead_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MulticastTableEntryIdxRead(const GSW_Device_t *dev, GSW_multicastTableRead_t *parm);

/**
   \brief Remove an host member from a multicast group. The multicast group entry
   is completely removed from the multicast table in case it has no
   host member port left.
   Group members can be added using \ref GSW_MulticastTableEntryAdd.

   \param dev Pointer to switch device.
   \param parm Pointer
      to \ref GSW_multicastTable_t.

   \remarks The Source IP parameter is ignored in case IGMPv3 support is
      not enabled in the hardware.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MulticastTableEntryRemove(const GSW_Device_t *dev, GSW_multicastTable_t *parm);

/**
   \brief Search the Multicast HW Table and return whether Entry is Found or Not
      if Found, Hardware Entry Index also returned.

   \param dev Pointer to switch device.
   \param parm Pointer
      to \ref GSW_multicastTable_t.

   \remarks The Source IP parameter is ignored in case IGMPv3 support is
      not enabled in the hardware.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if Entry is Found
   - An error code GSW_statusEntryNotFound in case entry not found
*/
GSW_return_t GSW_MulticastTableEntrySearch(const GSW_Device_t *dev, GSW_multicastTable_t *parm);

#endif

/** @}*/ /* GSW_MULTICAST */


/* STP */
#ifdef CONFIG_GSWIP_STP

/** \addtogroup GSW_ETHERNET_BRIDGING
 *  @{
 */

/**
   \brief Read out the current Spanning Tree Protocol state of an Ethernet port.
   This configuration can be set using \ref GSW_STP_PortCfgSet.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_STP_portCfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_STP_PortCfgGet(const GSW_Device_t *dev, GSW_STP_portCfg_t *parm);

/**
   \brief Configure the Spanning Tree Protocol state of an Ethernet port.
   The switch supports four Spanning Tree Port states (Disable/Discarding,
   Blocking/Listening, Learning and Forwarding state) for every port, to enable
   the Spanning Tree Protocol function when co-operating with software on
   the CPU port.
   Identified Spanning Tree Protocol packets can be redirected to the CPU port.
   Depending on the hardware implementation, the CPU port assignment is fixed
   or can be configured using \ref GSW_CPU_PortCfgSet.
   The current port state can be read back
   using \ref GSW_STP_PortCfgGet.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_STP_portCfg_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_STP_PortCfgSet(const GSW_Device_t *dev, GSW_STP_portCfg_t *parm);

/**
   \brief Read the Spanning Tree configuration.
   The configuration can be modified using \ref GSW_STP_BPDU_RuleSet.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_STP_BPDU_Rule_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_STP_BPDU_RuleGet(const GSW_Device_t *dev, GSW_STP_BPDU_Rule_t *parm);

/**
   \brief Set the Spanning Tree configuration. This configuration includes the
   filtering of detected spanning tree packets. These packets could be
   redirected to one dedicated port (e.g. CPU port) or they could be discarded.
   The current configuration can be read using \ref GSW_STP_BPDU_RuleGet.

   \param dev Pointer to switch device.
   \param parm Pointer to \ref GSW_STP_BPDU_Rule_t.

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_STP_BPDU_RuleSet(const GSW_Device_t *dev, GSW_STP_BPDU_Rule_t *parm);

/** @}*/ /* GSW_ETHERNET_BRIDGING */

#endif

/* Port Trunk (LAG) */
/** \addtogroup GSW_ETHERNET_BRIDGING
 *  @{
 */

/**
   \brief Configure the current port trunking algorithm that is used to retrieved if
   a packet is sent on the lower or higher trunking port index number. The algorithm
   performs an hash calculation over the MAC- and IP- addresses using the
   source- and destination- fields. This command retrieve which of the
   mentioned fields is used by the hash algorithm.
   The usage of any field could be configured over
   the \ref GSW_TrunkingCfgSet command.

   \param dev Pointer to switch device.
   \param parm Pointer to a
      configuration \ref GSW_trunkingCfg_t

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_TrunkingCfgSet(const GSW_Device_t *dev, GSW_trunkingCfg_t *parm);

/**
   \brief Read out the current port trunking algorithm that is used to retrieved if
   a packet is sent on the lower or higher trunking port index number. The algorithm
   performs an hash calculation over the MAC- and IP- addresses using the
   source- and destination- fields. This command retrieve which of the
   mentioned fields is used by the hash algorithm.
   The usage of any field could be configured over
   the \ref GSW_TrunkingCfgSet command.

   \param dev Pointer to switch device.
   \param parm Pointer to a
      configuration \ref GSW_trunkingCfg_t

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_TrunkingCfgGet(const GSW_Device_t *dev, GSW_trunkingCfg_t *parm);

/**@}*/ /* GSW_ETHERNET_BRIDGING */


/* PBB */

/** @cond DOC_ENABLE_PBB */
/** \addtogroup GSW_PBB
 *  @{
 */

/**
   \brief Allocate PBB Tunnel Template.

   \param dev Pointer to switch device.
   \param parm Pointer to a
      configuration \ref GSW_PBB_Tunnel_Template_Config_t

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PBB_TunnelTempate_Alloc(const GSW_Device_t *dev, GSW_PBB_Tunnel_Template_Config_t *parm);

/**
   \brief Free PBB Tunnel Template allocated by \ref GSW_PBB_TunnelTempate_Alloc.

   \param dev Pointer to switch device.
   \param parm Pointer to a
      configuration \ref GSW_PBB_Tunnel_Template_Config_t

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PBB_TunnelTempate_Free(const GSW_Device_t *dev, GSW_PBB_Tunnel_Template_Config_t *parm);

/**
   \brief Configure PBB Tunnel Template.
   A valid template should have been allocated by
   \ref GSW_PBB_TunnelTempate_Alloc.

   \param dev Pointer to switch device.
   \param parm Pointer to a
      configuration \ref GSW_PBB_Tunnel_Template_Config_t

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PBB_TunnelTempate_Config_Set(const GSW_Device_t *dev, GSW_PBB_Tunnel_Template_Config_t *parm);

/**
   \brief Get configuration of PBB Tunnel.

   \param dev Pointer to switch device.
   \param parm Pointer to a
      configuration \ref GSW_PBB_Tunnel_Template_Config_t

   \remarks The function returns an error code in case an error occurs.
            The error code is described in \ref GSW_return_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PBB_TunnelTempate_Config_Get(const GSW_Device_t *dev, GSW_PBB_Tunnel_Template_Config_t *parm);

/**@}*/ /* GSW_PBB */
/** @endcond DOC_ENABLE_PBB */


#endif /* MXL_GSW_API_H_ */
