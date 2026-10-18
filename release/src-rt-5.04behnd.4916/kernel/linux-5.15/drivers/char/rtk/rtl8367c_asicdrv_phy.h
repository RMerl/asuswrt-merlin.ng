#ifndef _RTL8367C_ASICDRV_PHY_H_
#define _RTL8367C_ASICDRV_PHY_H_

#include <rtl8367c_asicdrv.h>

#define RTL8367C_PHY_REGNOMAX           0x1F
#define RTL8367C_PHY_EXTERNALMAX        0x7

#define RTL8367C_PHY_BASE               0x2000
#define RTL8367C_PHY_EXT_BASE           0xA000

#define RTL8367C_PHY_OFFSET             5
#define RTL8367C_PHY_EXT_OFFSET         9

#define RTL8367C_PHY_PAGE_ADDRESS       31


extern ret_t rtl8367c_setAsicPHYReg(rtk_uint32 phyNo, rtk_uint32 phyAddr, rtk_uint32 regData );
extern ret_t rtl8367c_getAsicPHYReg(rtk_uint32 phyNo, rtk_uint32 phyAddr, rtk_uint32* pRegData );
extern ret_t rtl8367c_setAsicPHYOCPReg(rtk_uint32 phyNo, rtk_uint32 ocpAddr, rtk_uint32 ocpData );
extern ret_t rtl8367c_getAsicPHYOCPReg(rtk_uint32 phyNo, rtk_uint32 ocpAddr, rtk_uint32 *pRegData );
extern ret_t rtl8367c_setAsicSdsReg(rtk_uint32 sdsId, rtk_uint32 sdsReg, rtk_uint32 sdsPage,  rtk_uint32 value);
extern ret_t rtl8367c_getAsicSdsReg(rtk_uint32 sdsId, rtk_uint32 sdsReg, rtk_uint32 sdsPage, rtk_uint32 *value);
extern ret_t rtl8367c_setAsicPHYSram(rtk_uint32 phyNo, rtk_uint32 sramAddr, rtk_uint32 sramData );


#endif /*#ifndef _RTL8367C_ASICDRV_PHY_H_*/

