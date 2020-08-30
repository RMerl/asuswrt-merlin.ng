#ifndef _RTL8367C_ASICDRV_I2C_H_
#define _RTL8367C_ASICDRV_I2C_H_
#include <rtk_types.h>
#include <rtl8367c_asicdrv.h>


#define TIMEROUT_FOR_MICROSEMI (0x400)

#define GPIO_INPUT 1
#define GPIO_OUTPUT 2

extern ret_t rtl8367c_setAsicI2C_checkBusIdle(void);
extern ret_t rtl8367c_setAsicI2CStartCmd(void);
extern ret_t rtl8367c_setAsicI2CStopCmd(void);
extern ret_t rtl8367c_setAsicI2CTxOneCharCmd(rtk_uint8 oneChar);
extern ret_t rtl8367c_setAsicI2CcheckRxAck(void);
extern ret_t rtl8367c_setAsicI2CRxOneCharCmd(rtk_uint8 *pValue);
extern ret_t rtl8367c_setAsicI2CTxAckCmd(void);
extern ret_t rtl8367c_setAsicI2CTxNoAckCmd(void);
extern ret_t rtl8367c_setAsicI2CSoftRSTseqCmd(void);
extern ret_t rtl8367c_setAsicI2CGpioPinGroup(rtk_uint32 pinGroup_ID);
extern ret_t rtl8367c_getAsicI2CGpioPinGroup(rtk_uint32 * pPinGroup_ID);





#endif /*#ifndef _RTL8367C_ASICDRV_I2C_H_*/

