#define USE_DVICHIP
#ifdef USE_DVICHIP

#include "ddk750_sii164.h"
#include "ddk750_hwi2c.h"

/* I2C Address of each SII164 chip */
#define SII164_I2C_ADDRESS                  0x70

/* Define this definition to use hardware i2c. */
#define USE_HW_I2C

#ifdef USE_HW_I2C
    #define i2cWriteReg hwI2CWriteReg
    #define i2cReadReg  hwI2CReadReg
#else
    #define i2cWriteReg swI2CWriteReg
    #define i2cReadReg  swI2CReadReg
#endif

/* SII164 Vendor and Device ID */
#define SII164_VENDOR_ID                    0x0001
#define SII164_DEVICE_ID                    0x0006

#ifdef SII164_FULL_FUNCTIONS
/* Name of the DVI Controller chip */
static char *gDviCtrlChipName = "Silicon Image SiI 164";
#endif

/*
 *  sii164GetVendorID
 *      This function gets the vendor ID of the DVI controller chip.
 *
 *  Output:
 *      Vendor ID
 */
unsigned short sii164GetVendorID(void)
{
    unsigned short vendorID;

    vendorID = ((unsigned short) i2cReadReg(SII164_I2C_ADDRESS, SII164_VENDOR_ID_HIGH) << 8) |
                (unsigned short) i2cReadReg(SII164_I2C_ADDRESS, SII164_VENDOR_ID_LOW);

    return vendorID;
}

/*
 *  sii164GetDeviceID
 *      This function gets the device ID of the DVI controller chip.
 *
 *  Output:
 *      Device ID
 */
unsigned short sii164GetDeviceID(void)
{
    unsigned short deviceID;

    deviceID = ((unsigned short) i2cReadReg(SII164_I2C_ADDRESS, SII164_DEVICE_ID_HIGH) << 8) |
                (unsigned short) i2cReadReg(SII164_I2C_ADDRESS, SII164_DEVICE_ID_LOW);

    return deviceID;
}



/* DVI.C will handle all SiI164 chip stuffs and try it best to make code minimal and useful */

/*
 *  sii164InitChip
 *      This function initialize and detect the DVI controller chip.
 *
 *  Input:
 *      edgeSelect          - Edge Select:
 *                              0 = Input data is falling edge latched (falling edge
 *                                  latched first in dual edge mode)
 *                              1 = Input data is rising edge latched (rising edge
 *                                  latched first in dual edge mode)
 *      busSelect           - Input Bus Select:
 *                              0 = Input data bus is 12-bits wide
 *                              1 = Input data bus is 24-bits wide
 *      dualEdgeClkSelect   - Dual Edge Clock Select
 *                              0 = Input data is single edge latched
 *                              1 = Input data is dual edge latched
 *      hsyncEnable         - Horizontal Sync Enable:
 *                              0 = HSYNC input is transmitted as fixed LOW
 *                              1 = HSYNC input is transmitted as is
 *      vsyncEnable         - Vertical Sync Enable:
 *                              0 = VSYNC input is transmitted as fixed LOW
 *                              1 = VSYNC input is transmitted as is
 *      deskewEnable        - De-skewing Enable:
 *                              0 = De-skew disabled
 *                              1 = De-skew enabled
 *      deskewSetting       - De-skewing Setting (increment of 260psec)
 *                              0 = 1 step --> minimum setup / maximum hold
 *                              1 = 2 step
 *                              2 = 3 step
 *                              3 = 4 step
 *                              4 = 5 step
 *                              5 = 6 step
 *                              6 = 7 step
 *                              7 = 8 step --> maximum setup / minimum hold
 *      continuousSyncEnable- SYNC Continuous:
 *                              0 = Disable
 *                              1 = Enable
 *      pllFilterEnable     - PLL Filter Enable
 *                              0 = Disable PLL Filter
 *                              1 = Enable PLL Filter
 *      pllFilterValue      - PLL Filter characteristics:
 *                              0~7 (recommended value is 4)
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail.
 */
long sii164InitChip(
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
    //unsigned char ucRegIndex, ucRegValue;
    //unsigned char ucDeviceAddress,
	unsigned char config;
    //unsigned long delayCount;

    /* Initialize the i2c bus */
#ifdef USE_HW_I2C
    /* Use fast mode. */
    hwI2CInit(1);
#else
    swI2CInit(DEFAULT_I2C_SCL, DEFAULT_I2C_SDA);
#endif

    /* Check if SII164 Chip exists */
    if ((sii164GetVendorID() == SII164_VENDOR_ID) && (sii164GetDeviceID() == SII164_DEVICE_ID))
    {

#ifdef DDKDEBUG
        //sii164PrintRegisterValues();
#endif
        /*
         *  Initialize SII164 controller chip.
         */

        /* Select the edge */
        if (edgeSelect == 0)
            config = SII164_CONFIGURATION_LATCH_FALLING;
        else
            config = SII164_CONFIGURATION_LATCH_RISING;

        /* Select bus wide */
        if (busSelect == 0)
            config |= SII164_CONFIGURATION_BUS_12BITS;
        else
            config |= SII164_CONFIGURATION_BUS_24BITS;

        /* Select Dual/Single Edge Clock */
        if (dualEdgeClkSelect == 0)
            config |= SII164_CONFIGURATION_CLOCK_SINGLE;
        else
            config |= SII164_CONFIGURATION_CLOCK_DUAL;

        /* Select HSync Enable */
        if (hsyncEnable == 0)
            config |= SII164_CONFIGURATION_HSYNC_FORCE_LOW;
        else
            config |= SII164_CONFIGURATION_HSYNC_AS_IS;

        /* Select VSync Enable */
        if (vsyncEnable == 0)
            config |= SII164_CONFIGURATION_VSYNC_FORCE_LOW;
        else
            config |= SII164_CONFIGURATION_VSYNC_AS_IS;

        i2cWriteReg(SII164_I2C_ADDRESS, SII164_CONFIGURATION, config);

        /* De-skew enabled with default 111b value.
           This will fix some artifacts problem in some mode on board 2.2.
           Somehow this fix does not affect board 2.1.
         */
        if (deskewEnable == 0)
            config = SII164_DESKEW_DISABLE;
        else
            config = SII164_DESKEW_ENABLE;

        switch (deskewSetting)
        {
            case 0:
                config |= SII164_DESKEW_1_STEP;
                break;
            case 1:
                config |= SII164_DESKEW_2_STEP;
                break;
            case 2:
                config |= SII164_DESKEW_3_STEP;
                break;
            case 3:
                config |= SII164_DESKEW_4_STEP;
                break;
            case 4:
                config |= SII164_DESKEW_5_STEP;
                break;
            case 5:
                config |= SII164_DESKEW_6_STEP;
                break;
            case 6:
                config |= SII164_DESKEW_7_STEP;
                break;
            case 7:
                config |= SII164_DESKEW_8_STEP;
                break;
        }
        i2cWriteReg(SII164_I2C_ADDRESS, SII164_DESKEW, config);

        /* Enable/Disable Continuous Sync. */
        if (continuousSyncEnable == 0)
            config = SII164_PLL_FILTER_SYNC_CONTINUOUS_DISABLE;
        else
            config = SII164_PLL_FILTER_SYNC_CONTINUOUS_ENABLE;

        /* Enable/Disable PLL Filter */
        if (pllFilterEnable == 0)
            config |= SII164_PLL_FILTER_DISABLE;
        else
            config |= SII164_PLL_FILTER_ENABLE;

        /* Set the PLL Filter value */
        config |= ((pllFilterValue & 0x07) << 1);

        i2cWriteReg(SII164_I2C_ADDRESS, SII164_PLL, config);

        /* Recover from Power Down and enable output. */
        config = i2cReadReg(SII164_I2C_ADDRESS, SII164_CONFIGURATION);
        config |= SII164_CONFIGURATION_POWER_NORMAL;
        i2cWriteReg(SII164_I2C_ADDRESS, SII164_CONFIGURATION, config);

#ifdef DDKDEBUG
        //sii164PrintRegisterValues();
#endif

        return 0;
    }

    /* Return -1 if initialization fails. */
    return (-1);
}





/* below sii164 function is not neccessary */

#ifdef SII164_FULL_FUNCTIONS

/*
 *  sii164ResetChip
 *      This function resets the DVI Controller Chip.
 */
void sii164ResetChip(void)
{
    /* Power down */
    sii164SetPower(0);
    sii164SetPower(1);
}


/*
 * sii164GetChipString
 *      This function returns a char string name of the current DVI Controller chip.
 *      It's convenient for application need to display the chip name.
 */
char *sii164GetChipString(void)
{
    return gDviCtrlChipName;
}


/*
 *  sii164SetPower
 *      This function sets the power configuration of the DVI Controller Chip.
 *
 *  Input:
 *      powerUp - Flag to set the power down or up
 */
void sii164SetPower(
    unsigned char powerUp
)
{
    unsigned char config;

    config = i2cReadReg(SII164_I2C_ADDRESS, SII164_CONFIGURATION);
    if (powerUp == 1)
    {
        /* Power up the chip */
        config &= ~SII164_CONFIGURATION_POWER_MASK;
        config |= SII164_CONFIGURATION_POWER_NORMAL;
        i2cWriteReg(SII164_I2C_ADDRESS, SII164_CONFIGURATION, config);
    }
    else
    {
        /* Power down the chip */
        config &= ~SII164_CONFIGURATION_POWER_MASK;
        config |= SII164_CONFIGURATION_POWER_DOWN;
        i2cWriteReg(SII164_I2C_ADDRESS, SII164_CONFIGURATION, config);
    }
}


/*
 *  sii164SelectHotPlugDetectionMode
 *      This function selects the mode of the hot plug detection.
 */
static void sii164SelectHotPlugDetectionMode(
    sii164_hot_plug_mode_t hotPlugMode
)
{
    unsigned char detectReg;

    detectReg = i2cReadReg(SII164_I2C_ADDRESS, SII164_DETECT) & ~SII164_DETECT_MONITOR_SENSE_OUTPUT_FLAG;
    switch (hotPlugMode)
    {
        case SII164_HOTPLUG_DISABLE:
            detectReg |= SII164_DETECT_MONITOR_SENSE_OUTPUT_HIGH;
            break;
        case SII164_HOTPLUG_USE_MDI:
            detectReg &= ~SII164_DETECT_INTERRUPT_MASK;
            detectReg |= SII164_DETECT_INTERRUPT_BY_HTPLG_PIN;
            detectReg |= SII164_DETECT_MONITOR_SENSE_OUTPUT_MDI;
            break;
        case SII164_HOTPLUG_USE_RSEN:
            detectReg |= SII164_DETECT_MONITOR_SENSE_OUTPUT_RSEN;
            break;
        case SII164_HOTPLUG_USE_HTPLG:
            detectReg |= SII164_DETECT_MONITOR_SENSE_OUTPUT_HTPLG;
            break;
    }

    i2cWriteReg(SII164_I2C_ADDRESS, SII164_DETECT, detectReg);
}

/*
 *  sii164EnableHotPlugDetection
 *      This function enables the Hot Plug detection.
 *
 *  enableHotPlug   - Enable (=1) / disable (=0) Hot Plug detection
 */
void sii164EnableHotPlugDetection(
    unsigned char enableHotPlug
)
{
    unsigned char detectReg;
    detectReg = i2cReadReg(SII164_I2C_ADDRESS, SII164_DETECT);

    /* Depending on each DVI controller, need to enable the hot plug based on each
       individual chip design. */
    if (enableHotPlug != 0)
        sii164SelectHotPlugDetectionMode(SII164_HOTPLUG_USE_MDI);
    else
        sii164SelectHotPlugDetectionMode(SII164_HOTPLUG_DISABLE);
}

/*
 *  sii164IsConnected
 *      Check if the DVI Monitor is connected.
 *
 *  Output:
 *      0   - Not Connected
 *      1   - Connected
 */
unsigned char sii164IsConnected(void)
{
    unsigned char hotPlugValue;

    hotPlugValue = i2cReadReg(SII164_I2C_ADDRESS, SII164_DETECT) & SII164_DETECT_HOT_PLUG_STATUS_MASK;
    if (hotPlugValue == SII164_DETECT_HOT_PLUG_STATUS_ON)
        return 1;
    else
        return 0;
}

/*
 *  sii164CheckInterrupt
 *      Checks if interrupt has occured.
 *
 *  Output:
 *      0   - No interrupt
 *      1   - Interrupt occurs
 */
unsigned char sii164CheckInterrupt(void)
{
    unsigned char detectReg;

    detectReg = i2cReadReg(SII164_I2C_ADDRESS, SII164_DETECT) & SII164_DETECT_MONITOR_STATE_MASK;
    if (detectReg == SII164_DETECT_MONITOR_STATE_CHANGE)
        return 1;
    else
        return 0;
}

/*
 *  sii164ClearInterrupt
 *      Clear the hot plug interrupt.
 */
void sii164ClearInterrupt(void)
{
    unsigned char detectReg;

    /* Clear the MDI interrupt */
    detectReg = i2cReadReg(SII164_I2C_ADDRESS, SII164_DETECT);
    i2cWriteReg(SII164_I2C_ADDRESS, SII164_DETECT, detectReg | SII164_DETECT_MONITOR_STATE_CLEAR);
}

#endif

#endif


