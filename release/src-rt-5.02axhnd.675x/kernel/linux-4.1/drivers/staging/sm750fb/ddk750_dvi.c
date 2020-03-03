#define USE_DVICHIP 
#ifdef USE_DVICHIP
#include "ddk750_help.h"
#include "ddk750_reg.h"
#include "ddk750_dvi.h"
#include "ddk750_sii164.h"


/* This global variable contains all the supported driver and its corresponding
   function API. Please set the function pointer to NULL whenever the function
   is not supported. */
static dvi_ctrl_device_t g_dcftSupportedDviController[] =
{
#ifdef DVI_CTRL_SII164
    {
        .pfnInit = sii164InitChip,
        .pfnGetVendorId = sii164GetVendorID,
        .pfnGetDeviceId = sii164GetDeviceID,
#ifdef SII164_FULL_FUNCTIONS
        .pfnResetChip = sii164ResetChip,
        .pfnGetChipString = sii164GetChipString,
        .pfnSetPower = sii164SetPower,
        .pfnEnableHotPlugDetection = sii164EnableHotPlugDetection,
        .pfnIsConnected = sii164IsConnected,
        .pfnCheckInterrupt = sii164CheckInterrupt,
        .pfnClearInterrupt = sii164ClearInterrupt,
#endif
    },
#endif
};


int dviInit(
    unsigned char edgeSelect,
    unsigned char busSelect,
    unsigned char dualEdgeClkSelect,
    unsigned char hsyncEnable,
    unsigned char vsyncEnable,
    unsigned char deskewEnable,
    unsigned char deskewSetting,
    unsigned char continuousSyncEnable,
    unsigned char pllFilterEnable,
    unsigned char pllFilterValue
			)
{
	dvi_ctrl_device_t *pCurrentDviCtrl;
	pCurrentDviCtrl = g_dcftSupportedDviController;
	if(pCurrentDviCtrl->pfnInit != NULL)
	{
		return pCurrentDviCtrl->pfnInit(edgeSelect, busSelect, dualEdgeClkSelect, hsyncEnable,
                              vsyncEnable, deskewEnable, deskewSetting, continuousSyncEnable,
                              pllFilterEnable, pllFilterValue);
	}
	return -1;//error
}


/*
 *  dviGetVendorID
 *      This function gets the vendor ID of the DVI controller chip.
 *
 *  Output:
 *      Vendor ID
 */
unsigned short dviGetVendorID(void)
{
    dvi_ctrl_device_t *pCurrentDviCtrl;

    //pCurrentDviCtrl = getDviCtrl();
    pCurrentDviCtrl = g_dcftSupportedDviController;
    if (pCurrentDviCtrl != (dvi_ctrl_device_t *)0)
        return pCurrentDviCtrl->pfnGetVendorId();

    return 0x0000;
}


/*
 *  dviGetDeviceID
 *      This function gets the device ID of the DVI controller chip.
 *
 *  Output:
 *      Device ID
 */
unsigned short dviGetDeviceID(void)
{
    dvi_ctrl_device_t *pCurrentDviCtrl;

//    pCurrentDviCtrl = getDviCtrl();
	pCurrentDviCtrl = g_dcftSupportedDviController;
    if (pCurrentDviCtrl != (dvi_ctrl_device_t *)0)
        return pCurrentDviCtrl->pfnGetDeviceId();

    return 0x0000;
}

#endif


