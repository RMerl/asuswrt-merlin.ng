/*
 * <:copyright-BRCM:2022:DUAL/GPL:standard
 *
 *    Copyright (c) 2022 Broadcom
 *    All Rights Reserved
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 *
 * :>
 */

#include "user_api.h" 
#include <net/if.h>
#include "rdpactl_api.h"

#define BDMF_NULL (bdmf_object_handle)0
#define perror(fmt, arg...) {printf("ERROR[%s.%u]: " fmt, __FUNCTION__, __LINE__, ##arg);}


/*copy from cms.h to avoid dependancy on
* on cms will be removed together with fix_up_if_name
*/
#define VEIP_IFC_STR          "veip"
#define EPON_IFC_STR          "epon"

/* This is a temoroary solution correct solution will be
*  finding root device for veip in upper level and not relay on
*  hardcoded values
*/

static const char *fix_up_if_name(const char *ifname)
{
   if (strstr(ifname, VEIP_IFC_STR))
   {
       if (rdpactl_has_rdpa_port_type_gpon(NULL))
               return GPON_ROOT;
       if (rdpactl_has_rdpa_port_type_epon(NULL))
               return EPON_ROOT;
       /* should never happend on PON plaforms */
       return "bad_type";
   }

   if (strstr(ifname, EPON_IFC_STR))
      return EPON_IFC_STR;

   return ifname;
}

int rdpactl_get_port_by_name(const char *ifname, bdmf_object_handle *port)
{
    return rdpa_port_get(fix_up_if_name(ifname), port);
}

static int _has_rdpa_port_type(int is_gpon, int is_epon, char *pon_name)
{
    bdmf_object_handle port = BDMF_NULL;
    rdpa_port_type type = rdpa_port_type_none;

    if (pon_name)
        pon_name[0]='\0';

    while ((port = bdmf_get_next(rdpa_port_drv(), port, NULL)))
    {
        rdpa_port_type_get(port, &type);

        if ((is_gpon && (type == rdpa_port_gpon || type == rdpa_port_xgpon)) ||
            (is_epon && (type == rdpa_port_epon || type == rdpa_port_xepon)))
        {
            if (pon_name)
                rdpa_port_name_get(port, pon_name, IFNAMSIZ-1);
            bdmf_put(port);
            return 1;
        }
    }

    return 0;
}

rdpa_port_type rdpactl_get_port_type(const char *name)
{
    bdmf_object_handle port = BDMF_NULL;
    rdpa_port_type type = rdpa_port_type_none;
    int rc = 0;

    rc = rdpactl_get_port_by_name(name, &port);
    if (rc)
        goto Exit;
    
    rc = rdpa_port_type_get(port, &type);
    
    bdmf_put(port);

Exit:
    if (rc)
        perror("call failed for dev %s\n", name);
    return type;
}

int rdpactl_has_rdpa_port_type_gpon(char* gpon_name)
{
    return _has_rdpa_port_type(1, 0, gpon_name);
}
int rdpactl_has_rdpa_port_type_epon(char* epon_name)
{
    return _has_rdpa_port_type(0, 1, epon_name);
}

int rdpactl_set_sys_car_mode(int enable)
{
    bdmf_object_handle system = BDMF_NULL;
    rdpa_system_cfg_t cfg;
    int rc;

    rc = rdpa_system_get(&system);
    if (rc)
        goto Exit;

    rc = rdpa_system_cfg_get(system, &cfg);
    if (rc)
        goto Exit;

    cfg.car_mode = enable;

    rc = rdpa_system_cfg_set(system, &cfg);
    if (rc)
        goto Exit;

Exit:
    if (system)
        bdmf_put(system);
    if (rc)
        perror("call failed\n");

    return rc;
}

int rdpactl_set_learning_ind(char *name, unsigned char learningInd)
{
    bdmf_object_handle port = BDMF_NULL;
    rdpa_port_dp_cfg_t cfg;
    int rc;

    rc = rdpa_port_get(name, &port);
    if (rc)
        goto Exit;

    rc = rdpa_port_cfg_get(port, &cfg);
    if (rc)
        goto Exit;

    cfg.sal_enable = cfg.dal_enable = learningInd;
    cfg.sal_miss_action = cfg.dal_miss_action = rdpa_forward_action_host;

    rc = rdpa_port_cfg_set(port, &cfg);
    if (rc)
        goto Exit;

Exit:
    if (port)
        bdmf_put(port);
    if (rc)
        perror("call failed\n");

    return rc;
}
