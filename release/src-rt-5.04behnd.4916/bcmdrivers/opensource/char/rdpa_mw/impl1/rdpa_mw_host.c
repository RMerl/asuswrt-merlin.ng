#include "rdpa_types.h"
#include "rdpa_api.h"
#include "rdpa_mw_arch.h"
#include "rdpa_mw_blog_parse.h"
#include "rdpa_mw_vlan.h"
#include "rdpa_mw_qos.h"
#include "bcmenet_common.h"
#include "bcm_util_func.h"

static bdmf_object_handle rdpa_mw_dev2rdpa_port_obj(struct net_device *root_dev)
{
    bcm_netdev_priv_info_get_cb_fn_t priv_info_get = bcm_netdev_ext_field_get(root_dev, bcm_netdev_cb_fn);
    bcm_netdev_priv_info_out_t info_out = {};

    if (!priv_info_get || priv_info_get(root_dev, BCM_NETDEV_TO_RDPA_PORT_OBJ, &info_out))
        return NULL;

    return info_out.bcm_netdev_to_rdpa_port_obj.rdpa_port_obj;
}
#if !defined(CONFIG_BRCM_QEMU) && defined(CONFIG_BCM_GDX_HW)
extern bcm_netdev_priv_info_get_cb_fn_t blog_gdx_dev_cb_fn;
#endif

/*
 *------------------------------------------------------------------------------
 * Function:
 *   bdmf_object_handle rdpa_mw_get_port_object_by_dev(struct net_device *dev)
 * Description:
 *   Find the first net device from the leaf device node to the
 *   root device node that has valid port object and return it.
 * Parameters:
 *   dev:  The leaf device node of device tree.
 * Returns:
 *   pointer the port object.
 *------------------------------------------------------------------------------
 */
bdmf_object_handle rdpa_mw_get_port_object_by_dev_dir(struct net_device *dev,
                                                      uint32_t dir)
{
#if !defined(CONFIG_BRCM_QEMU) && defined(CONFIG_BCM_GDX_HW)
    bcm_netdev_priv_info_out_t info_out = {};
#endif
    bdmf_object_handle port_obj = NULL;

    if (!dev)
        return NULL;

#if !defined(CONFIG_BRCM_QEMU) && defined(CONFIG_BCM_GDX_HW)
    if ((is_netdev_accel_gdx_rx(dev) && (dir == RDPA_DIR_RX)) ||
        (is_netdev_accel_gdx_tx(dev) && (dir == RDPA_DIR_TX)))
    {
        if (!blog_gdx_dev_cb_fn)
        {
           BCM_LOG_ERROR(BCM_LOG_ID_RDPA, "gdx enabled but gdx_dev_cb_fn NULL");
           return NULL;
        }
        blog_gdx_dev_cb_fn(dev, BCM_NETDEV_TO_RDPA_PORT_OBJ, &info_out);
        return info_out.bcm_netdev_to_rdpa_port_obj.rdpa_port_obj;
    }
#endif

    while (!(port_obj = rdpa_mw_dev2rdpa_port_obj(dev)) && !netdev_path_is_root(dev))
        dev = netdev_path_next_dev(dev);

    return port_obj;
}
EXPORT_SYMBOL(rdpa_mw_get_port_object_by_dev_dir);

uint8_t rdpa_mw_root_dev2rdpa_ssid(struct net_device *root_dev)
{
    uint32_t hw_port, hw_port_type;

    hw_port = netdev_path_get_hw_port(root_dev);
    hw_port_type = netdev_path_get_hw_port_type(root_dev);

    switch (hw_port_type)
    {
    case BLOG_WLANPHY:
        return WLAN_SSID_GET(hw_port);
    default:
        break;
    }

    return (uint8_t)-1;
}
EXPORT_SYMBOL(rdpa_mw_root_dev2rdpa_ssid);
