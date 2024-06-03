/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

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
/*
 * pmc_wan.c
 *
 *  Created on: Nov 30 2015
 *      Author: yonatani
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/of.h>
#include "pmc_drv.h"
#include "BPCM.h"
#include "pmc_wan.h"
#include "bcm_ubus4.h"


static int wan_xpon_zones[] = XPON_POWER_ZONES;
static int wan_xdsl_zones[] = XDSL_POWER_ZONES;
static int wan_aeth_zones[] = AETH_POWER_ZONES;
static int wan_eth_zones[]  = EWAN_POWER_ZONES;

/* Manual/Force power control of the zones

   devAddr : PMB device address
   zone    : zone number
   on      : 1 = Power ON. 0 = Power OFF
*/
int manual_zone_power_control(int devAddr, int zone, int on)
{
    BPCM_PWR_ZONE_N_CONTROL ctrl;
    BPCM_PWR_ZONE_N_STATUS sts;
    int status = 0;

    status = ReadBPCMRegister(devAddr, BPCMZoneCtrlRegOffset(zone), &ctrl.Reg32);
    status += ReadBPCMRegister(devAddr, BPCMZoneStsRegOffset(zone), &sts.Reg32);
    if (status == kPMC_NO_ERROR) {
       if (on && sts.Bits.pwr_on_state == 0) {
          /* Requesting to power ON */
          ctrl.Bits.pwr_dn_req = 0;
          ctrl.Bits.pwr_up_req = 1;
          ctrl.Bits.dpg_ctl_en = 1;
          ctrl.Bits.mem_pwr_ctl_en = 1;
          ctrl.Bits.manual_ctl = 0;
          ctrl.Bits.manual_clk_en = 0;
          ctrl.Bits.manual_reset_ctl = 0;
          ctrl.Bits.manual_mem_pwr = 0;
          ctrl.Bits.manual_iso_ctl = 0;
          status = WriteBPCMRegister(devAddr, BPCMZoneCtrlRegOffset(zone), ctrl.Reg32);
       }
       else if(!on && sts.Bits.pwr_off_state == 0) {
          /* Requesting to power OFF */
          ctrl.Bits.pwr_dn_req = 1;
          ctrl.Bits.pwr_up_req = 0;
          ctrl.Bits.manual_ctl = 1;
          ctrl.Bits.manual_clk_en = 0;
          ctrl.Bits.manual_reset_ctl = 1;
          ctrl.Bits.manual_mem_pwr = 0;
          ctrl.Bits.manual_iso_ctl = 1;
          status = WriteBPCMRegister(devAddr, BPCMZoneCtrlRegOffset(zone), ctrl.Reg32);
       }
    }

    return status;
}

/*
  Power ON/OFF zones, specific to WAN interface.

  interface: WAN interface
  ctrl     : 1 = Power ON. 0 = Power OFF request
*/
int pmc_wan_interface_power_control(WAN_INTF interface, int ctrl)
{
   int *zones = NULL;
   int i, status = 0;

   /* Find the power zones for the given interface */
   switch (interface){
   case WAN_INTF_XPON:
      zones = wan_xpon_zones;
      break;
   case WAN_INTF_XDSL:
      zones = wan_xdsl_zones;
      break;
   case WAN_INTF_AETH:
      zones = wan_aeth_zones;
      break;
   case WAN_INTF_ETH:
      zones = wan_eth_zones;
      break;
   default:
      status = -1;
   }

   for (i = 0; zones && zones[i] != -1; i++){
      status += manual_zone_power_control(PMB_ADDR_WAN, zones[i], ctrl);
   }

   return status;
}
EXPORT_SYMBOL(pmc_wan_interface_power_control);

#if defined (CONFIG_BCM963158)
static int is_xpon_intf_exist(void)
{
    int ret = 0;
    struct device_node *np = of_find_compatible_node(NULL, NULL, "brcm,pon-drv");

    if (np && of_device_is_available(np)) {
        of_node_put(np);
        ret = 1;
    }

    return ret;
}

static int is_ae_intf_exist(void)
{
    int ret = 0;
    struct device_node *np = of_find_compatible_node(NULL, NULL, "brcm,enet");
    struct device_node *child, *port;

    if (!np) return ret;

    for_each_available_child_of_node(np, child)
    {
        for_each_available_child_of_node(child, port)
        {
            const unsigned int *port_reg;
            unsigned int port_index = -1;
            port_reg = of_get_property(port, "reg", NULL);
            
            if (port_reg)
            {
                port_index = be32_to_cpup(port_reg);
                if (port_index == 4)
                    ret=1;
            }
            
        };
    };
    of_node_put(np);

    return ret;
}

static int is_dsl_intf_exist(void)
{
    int ret = 0;
    struct device_node *np = of_find_compatible_node(NULL, NULL, "brcm,dsl");

    if (np) {
        if (of_device_is_available(np)) {
            ret = 1;
        } else {  //check for the next dsl line
            np = of_find_compatible_node(np, NULL, "brcm,dsl");
            if(np && of_device_is_available(np)) {
                ret =1;
            }
        }
    }

    if (np)
        of_node_put(np);

    return ret;
}

static int is_xport_intf_exist(void)
{
    int ret = 0;
    struct device_node *np = of_find_compatible_node(NULL, NULL, "brcm,enet");
    struct device_node *child, *port;

    if (!np) return ret;

    for_each_available_child_of_node(np, child)
    {
        for_each_available_child_of_node(child, port)
        {
            const unsigned int *port_reg;
            unsigned int port_index = -1;
            port_reg = of_get_property(port, "reg", NULL);
            
            if (port_reg)
            {
                port_index = be32_to_cpup(port_reg);
                if ((port_index == 4) || (port_index == 5))
                    ret=1;
            }
            
        };
    };
    of_node_put(np);

    return ret;
}

#endif

static int pmc_wan_initialized = 0;

int pmc_wan_init(void)
{
    int status = 0;

#if defined (CONFIG_BCM96858) || defined(_BCM96858_)
    BPCM_SR_CONTROL sreset;
#endif

    if (pmc_wan_initialized)
        return 0;

    // In 63158, WAN block is alreday powered ON in CFE to configure
    // VREG. Powering it again would reset the VREG settings.
#if !(defined(CONFIG_BCM963158) || defined(_BCM963158_))
    // To avoid glitch due to warm reboot, powerdown the block first.
    status  = PowerOffDevice(PMB_ADDR_WAN, 0);
    status += PowerOnDevice(PMB_ADDR_WAN);
#endif

#if defined (CONFIG_BCM96858) || defined(_BCM96858_)
    ubus_register_port(UCB_NODE_ID_SLV_WAN);

    // take pins out of reset
    // can be modified later for smaller granularity
    sreset.Bits_Wantop.wan_main_rst_n = 0;
    sreset.Bits_Wantop.wan_top_bb_rst_n = 0;
    sreset.Bits_Wantop.epon_core_rst_n = 0;
    sreset.Bits_Wantop.epon_rx_rclk16_sw_reset_n = 0;
    sreset.Bits_Wantop.epon_rx_rbc125_sw_reset_n = 0;
    sreset.Bits_Wantop.epon_tx_tclk16_sw_reset_n = 0;
    sreset.Bits_Wantop.epon_tx_clk125_sw_reset_n = 0;
    sreset.Bits_Wantop.gpon_main_rst_n = 0;
    sreset.Bits_Wantop.gpon_rx_rst_n = 0;
    sreset.Bits_Wantop.gpon_tx_rst_n = 0;
    sreset.Bits_Wantop.gpon_8khz_rst_n = 0;
    sreset.Bits_Wantop.ngpon_main_rst_n = 0;
    sreset.Bits_Wantop.ngpon_rx_rst_n = 0;
    sreset.Bits_Wantop.ngpon_tx_rst_n = 0;
    sreset.Bits_Wantop.ngpon_8khz_rst_n = 0;
    sreset.Bits_Wantop.gpon_nco_rst_n = 0;
    sreset.Bits_Wantop.apm_rst_n = 0;

    status += WriteBPCMRegister(PMB_ADDR_WAN, BPCMRegOffset(sr_control), sreset.Reg32);
#endif

#if defined (CONFIG_BCM963158)
    /* Power ON interfaces that this platform is configured with.
       Otherwise turn OFF the interfaces to save power */
    if (is_xpon_intf_exist())
        status += pmc_wan_interface_power_control(WAN_INTF_XPON, 1);
    else
        status += pmc_wan_interface_power_control(WAN_INTF_XPON, 0);

    if (is_ae_intf_exist())
        status += pmc_wan_interface_power_control(WAN_INTF_AETH, 1);
    else
        status += pmc_wan_interface_power_control(WAN_INTF_AETH, 0);

    if (is_dsl_intf_exist())
        status += pmc_wan_interface_power_control(WAN_INTF_XDSL, 1);
    else
        status += pmc_wan_interface_power_control(WAN_INTF_XDSL, 0);

    if (is_xport_intf_exist())
        status += pmc_wan_interface_power_control(WAN_INTF_ETH, 1);
    else
        status += pmc_wan_interface_power_control(WAN_INTF_ETH, 0);
#endif

    pmc_wan_initialized = 1;

    return status;
}
EXPORT_SYMBOL(pmc_wan_init);

int pmc_wan_uninit(void)
{
    pmc_wan_initialized = 0;

    return PowerOffDevice(PMB_ADDR_WAN, 0);
}
EXPORT_SYMBOL(pmc_wan_uninit);

static int pmc_wan_initcall(void)
{
#if defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837)
    return 0;
#else
    return pmc_wan_init();
#endif
}

#if defined (CONFIG_BCM963158)
int pmc_wan_ae_reset(void)
{
    BPCM_SR_CONTROL sreset;
    int status;

    status = ReadBPCMRegister(PMB_ADDR_WAN, BPCMRegOffset(sr_control), &sreset.Reg32);
    if( status != kPMC_NO_ERROR )
        return status;

    sreset.Bits_Wantop.ae_rx_rclk16_sw_reset_n = 1;
    sreset.Bits_Wantop.ae_rx_rbc125_sw_reset_n = 1;
    sreset.Bits_Wantop.ae_tx_tclk16_sw_reset_n = 1;
    sreset.Bits_Wantop.ae_tx_clk125_sw_reset_n = 1;

    status = WriteBPCMRegister(PMB_ADDR_WAN, BPCMRegOffset(sr_control), sreset.Reg32);
    if( status != kPMC_NO_ERROR )
        return status;

    udelay(5);

    sreset.Bits_Wantop.ae_rx_rclk16_sw_reset_n = 0;
    sreset.Bits_Wantop.ae_rx_rbc125_sw_reset_n = 0;
    sreset.Bits_Wantop.ae_tx_tclk16_sw_reset_n = 0;
    sreset.Bits_Wantop.ae_tx_clk125_sw_reset_n = 0;
    
    status = WriteBPCMRegister(PMB_ADDR_WAN, BPCMRegOffset(sr_control), sreset.Reg32);

    return status;
}
EXPORT_SYMBOL(pmc_wan_ae_reset);
#endif

arch_initcall(pmc_wan_initcall);
