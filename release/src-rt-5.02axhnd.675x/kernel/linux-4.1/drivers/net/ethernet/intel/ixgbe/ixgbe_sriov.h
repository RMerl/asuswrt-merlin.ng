/*******************************************************************************

  Intel 10 Gigabit PCI Express Linux driver
  Copyright(c) 1999 - 2013 Intel Corporation.

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information:
  Linux NICS <linux.nics@intel.com>
  e1000-devel Mailing List <e1000-devel@lists.sourceforge.net>
  Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497

*******************************************************************************/

#ifndef _IXGBE_SRIOV_H_
#define _IXGBE_SRIOV_H_

/*  ixgbe driver limit the max number of VFs could be enabled to
 *  63 (IXGBE_MAX_VF_FUNCTIONS - 1)
 */
#define IXGBE_MAX_VFS_DRV_LIMIT  (IXGBE_MAX_VF_FUNCTIONS - 1)

#ifdef CONFIG_PCI_IOV
void ixgbe_restore_vf_multicasts(struct ixgbe_adapter *adapter);
#endif
void ixgbe_msg_task(struct ixgbe_adapter *adapter);
int ixgbe_vf_configuration(struct pci_dev *pdev, unsigned int event_mask);
void ixgbe_disable_tx_rx(struct ixgbe_adapter *adapter);
void ixgbe_ping_all_vfs(struct ixgbe_adapter *adapter);
int ixgbe_ndo_set_vf_mac(struct net_device *netdev, int queue, u8 *mac);
int ixgbe_ndo_set_vf_vlan(struct net_device *netdev, int queue, u16 vlan,
			   u8 qos);
int ixgbe_ndo_set_vf_bw(struct net_device *netdev, int vf, int min_tx_rate,
			int max_tx_rate);
int ixgbe_ndo_set_vf_spoofchk(struct net_device *netdev, int vf, bool setting);
int ixgbe_ndo_set_vf_rss_query_en(struct net_device *netdev, int vf,
				  bool setting);
int ixgbe_ndo_get_vf_config(struct net_device *netdev,
			    int vf, struct ifla_vf_info *ivi);
void ixgbe_check_vf_rate_limit(struct ixgbe_adapter *adapter);
int ixgbe_disable_sriov(struct ixgbe_adapter *adapter);
#ifdef CONFIG_PCI_IOV
void ixgbe_enable_sriov(struct ixgbe_adapter *adapter);
#endif
int ixgbe_pci_sriov_configure(struct pci_dev *dev, int num_vfs);

static inline void ixgbe_set_vmvir(struct ixgbe_adapter *adapter,
				   u16 vid, u16 qos, u32 vf)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 vmvir = vid | (qos << VLAN_PRIO_SHIFT) | IXGBE_VMVIR_VLANA_DEFAULT;

	IXGBE_WRITE_REG(hw, IXGBE_VMVIR(vf), vmvir);
}

#endif /* _IXGBE_SRIOV_H_ */

