#include <rtk_switch.h>
#include <rtk_error.h>
#include <stat.h>
#include <string.h>
#include "rtk_types.h" 
#include "port.h" 
#include "rtl8367c_asicdrv_phy.h"

/* Function Name:
 *      rtk_port_phyTestMode_set
 * Description:
 *      Set PHY in test mode.
 * Input:
 *      port    - port id.
 *      mode    - PHY test mode 0:normal 1:test mode 1 2:test mode 2 3: test mode 3 4:test mode 4 5~7:reserved
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 *      RT_ERR_NOT_ALLOWED      - The Setting is not allowed, caused by set more than 1 port in Test mode.
 * Note:
 *      Set PHY in test mode and only one PHY can be in test mode at the same time.
 *      It means API will return FAILED if other PHY is in test mode.
 *      This API only provide test mode 1 & 4 setup, and if users want other test modes,
 *      please contact realtek FAE.
 */
rtk_api_ret_t rtk_port_phyTestMode_set(rtk_port_t port, rtk_port_phy_test_mode_t mode)
{
    rtk_uint32          data, regData, i;
    rtk_api_ret_t       retVal;

    RTK_CHK_PORT_IS_UTP(port);

    if(mode >= PHY_TEST_MODE_END)
        return RT_ERR_INPUT;

    if( (mode == PHY_TEST_MODE_2) || (mode == PHY_TEST_MODE_3) )
        return RT_ERR_INPUT;

    if (PHY_TEST_MODE_NORMAL != mode)
    {
        /* Other port should be Normal mode */
        RTK_SCAN_ALL_LOG_PORT(i)
        {
            if(rtk_switch_isUtpPort(i) == RT_ERR_OK)
            {
                if(i != port)
                {
                    if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(i), 9, &data)) != RT_ERR_OK)
                        return retVal;

                    if((data & 0xE000) != 0)
                        return RT_ERR_NOT_ALLOWED;
                }
            }
        }
    }

    if (PHY_TEST_MODE_4 == mode)
    {
        if((retVal = rtl8367c_setAsicReg(0x13C2, 0x0249)) != RT_ERR_OK)
            return retVal;

        if((retVal = rtl8367c_getAsicReg(0x1300, &regData)) != RT_ERR_OK)
            return retVal;

        if(regData == 0x6511)
        {
            if ((retVal = rtl8367c_setAsicPHYOCPReg(rtk_switch_port_L2P_get(port), 0xa436, 0x8081)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl8367c_setAsicPHYOCPReg(rtk_switch_port_L2P_get(port), 0xa438, 0x7e00)) != RT_ERR_OK)
                return retVal;
        }
    }

    if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(port), 9, &data)) != RT_ERR_OK)
        return retVal;

    data &= ~0xE000;
    data |= (mode << 13);
    if ((retVal = rtl8367c_setAsicPHYReg(rtk_switch_port_L2P_get(port), 9, data)) != RT_ERR_OK)
        return retVal;

    if (PHY_TEST_MODE_4 == mode)
    {
        if((retVal = rtl8367c_setAsicReg(0x13C2, 0x0249)) != RT_ERR_OK)
            return retVal;

        if((retVal = rtl8367c_getAsicReg(0x1300, &regData)) != RT_ERR_OK)
            return retVal;

        if( (regData == 0x0276) || (regData == 0x0597) )
        {
            if ((retVal = rtl8367c_setAsicPHYOCPReg(rtk_switch_port_L2P_get(port), 0xbcc2, 0xF4F4)) != RT_ERR_OK)
                return retVal;
        }

        if( (regData == 0x6367) )
        {
            if ((retVal = rtl8367c_setAsicPHYOCPReg(rtk_switch_port_L2P_get(port), 0xa436, 0x80c1)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl8367c_setAsicPHYOCPReg(rtk_switch_port_L2P_get(port), 0xa438, 0xfe00)) != RT_ERR_OK)
                return retVal;
        }
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_phyTestMode_get
 * Description:
 *      Get PHY in which test mode.
 * Input:
 *      port - Port id.
 * Output:
 *      mode - PHY test mode 0:normal 1:test mode 1 2:test mode 2 3: test mode 3 4:test mode 4 5~7:reserved
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      Get test mode of PHY from register setting 9.15 to 9.13.
 */
rtk_api_ret_t rtk_port_phyTestMode_get(rtk_port_t port, rtk_port_phy_test_mode_t *pMode)
{
    rtk_uint32      data;
    rtk_api_ret_t   retVal;

    RTK_CHK_PORT_IS_UTP(port);

    if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(port), 9, &data)) != RT_ERR_OK)
        return retVal;

    *pMode = (data & 0xE000) >> 13;

    return RT_ERR_OK;
}

