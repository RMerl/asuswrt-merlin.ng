/******************************************************************************

    Copyright 2022 Maxlinear

    SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0-only)

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.
******************************************************************************/

/**
   \file lif_api.c
   This file implements the Ethernet MDIO Link Interface (LIF) API.

   This API abstract the underlying Target Ethernet MDIO connection by selecting with the
   Link Interface Id (lid) the corresponding MDIO library.

   Note: The LIF abstraction is not implemented. The lif api is tailored for the C mdio library
   located in PyRPIO package. The lif will be reworked once the missing block (conf,..) will be available
   in the future.
*/

/* ========================================================================== */
/*                                 Includes                                   */
/* ========================================================================== */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bcm2835.h"
#include "mdio.h"
#include "lif_api.h"

/* ========================================================================== */
/*                             Macro definitions                              */
/* ========================================================================== */

/* ========================================================================== */
/*                            Types definitions                               */
/* ========================================================================== */

/** define the state of the mdio pins in the global pin table */
typedef struct
{
    /** mdio clock pin with the following values
        - (0) pin not valid
        - (1) valid pin close
        - (pin) valid open pin as clock pin */
    uint8_t cpin;

    /** mdio data pin with the following values
        - (0) pin not valid
        - (1) valid pin closed
        - (pin) valid open pin as data pin */
    uint8_t dpin;

} Y_S_lif_pins;

/*  Global Variable handling Scan function
        - scanned_devices: Scanned Devices
        - PHY_MAX_VAL: Maximum PHY Value
        - COMB_MAX_VAL: Maximum possible combination of pins
        - MAX_LINKS: Maximum number of devices connected to RPI4

*/

typedef struct
{
    /** mdio clock pin with the following values
        - (0) pin not valid
        - (1) valid pin close
        - (pin) valid open pin as clock pin */
    char *lib;

    /** mdio clock pin with the following values
        - (0) pin not valid
        - (1) valid pin close
        - (pin) valid open pin as clock pin */
    uint8_t cpin;

    /** mdio data pin with the following values
        - (0) pin not valid
        - (1) valid pin closed
        - (pin) valid open pin as data pin */
    uint8_t dpin;

    /** Number of Phys Found */
    uint8_t nr_phys;

    /** Phy information in structure [Address, PHYID] */
    uint32_t phy_info[PHY_MAX_VAL][2];

} device_struc;

/* ========================================================================== */
/*                           Local Function prototypes                        */
/* ========================================================================== */
static void tableinit_lif_pins(void);
static int32_t update_init_lif_lib(char *lib);
static int32_t update_deinit_lif_lib(char *lib);
static int32_t check_valid_lif_lib(char *lib);
static int32_t check_valid_lif_pins(uint8_t clk_pin, uint8_t data_pin);
static int32_t update_open_lif_pins(uint8_t clk_pin, uint8_t data_pin);
static int32_t update_close_lif_pins(uint8_t clk_pin, uint8_t data_pin);
static int32_t check_rw_lif_pins(uint8_t clk_pin, uint8_t data_pin);

/* ========================================================================== */
/*                             Global variables                                */
/* ========================================================================== */

/* ========================================================================== */
/*                             Local variables                                */
/* ========================================================================== */

/*  Global Variable for handling the initialisation and shutdown of the
    underlying mdio library. gLifMdioInit
        - MAGIC_KEY_LIF_NOT_INITIALZED: not initialized
        - MAGIC_KEY_LIF_INITIALZED: initialized
*/

#define MAGIC_KEY_LIF_NOT_INITIALZED 0xffffffff
#define MAGIC_KEY_LIF_INITIALZED 0x12345678
static int32_t gLifMdioInit;
static int32_t scanned_nr_lif;
static device_struc scanned_devices[MAX_LINKS];

/*  Global Table for handling or checking the validaty of the clock and data
    pins used for the mdio connection between the  RPI4 and the EVK. The valid
    pins of the 40-pin RPI4 are the GPIO pins that will be used as unique
    (clock, data) pins couple for the physical connection and driven thru the
    lif API. The valid GPIO pins are 4, 5, 6, 12, 13, 22, 23, 24, 25, 26 and 27
    meaning up to 5 mdio physical connections can be made. Each connection is
    made of a unique clock, data pins.

    Only the entries marked as "LIF_VALID_PIN" are accepted (valid) pins corresponding
    to the pins 4, 5, 6, 12, 13, 22, 23, 24, 25, 26 and 27.
    For a valid entry, the cpin and dpin fields of the Y_S_lif_pins structure provide
    the status of both pins from the initialisation (LIF_VALID_PIN)  to open (pin number)
    and close (LIF_VALID_PIN).

    For a Valid entry, the cpin field matches the entry number when the cpin is open,
    dpin will contain the data pin numver associated to the clock pin.

    Example with clock pin = 5, data pin = 6:
    initialization:
    gLifTablePins[5].cpin = LIF_VALID_PIN, gLifTablePins[5].dpin = LIF_VALID_PIN
    gLifTablePins[6].cpin = LIF_VALID_PIN, gLifTablePins[6].dpin = LIF_VALID_PIN

    open:
    clock and data pins can only be open when both pin are free (LIF_VALID_PIN),
    this is checkedwith the value in gLifTablePins[5].cpin for the clock pin
    and gLifTablePins[6].dpin for the data pin.
    When open, the table is updated as follow (see update_open_lif_pins function)
    providing the unique pins couple:
    gLifTablePins[5].cpin = 5, gLifTablePins[5].dpin = 6
    gLifTablePins[6].cpin = 5, gLifTablePins[6].dpin = 6

    close:
    Only open clock and data pins couple can be closed for that
    gLifTablePins[5].cpin = 5 for clock pin and gLifTablePins[6].dpin = 6 for data pin
    are checked.
    When closed, the table is updated as follow (see update_close_lif_pins function)
    to the init values and the pins are free for a new assignment:
    gLifTablePins[5].cpin = LIF_VALID_PIN, gLifTablePins[5].dpin = LIF_VALID_PIN
    gLifTablePins[6].cpin = LIF_VALID_PIN, gLifTablePins[6].dpin = LIF_VALID_PIN
*/
#define LIF_INVALID_PIN 0
#define LIF_VALID_PIN 1

#define LIF_TABLE_PINS_ENTRIES_MAX 32
Y_S_lif_pins gLifTablePins[LIF_TABLE_PINS_ENTRIES_MAX] =
    {
/* for documenting the initial pins mapping as the table
   is initialized in "tableinit_lif_pins ()" function */
#if 0
    {LIF_INVALID_PIN,   LIF_INVALID_PIN},   /* 0  : not valid */
    {LIF_INVALID_PIN,   LIF_INVALID_PIN},   /* 1  : not valid */
    {LIF_INVALID_PIN,   LIF_INVALID_PIN},   /* 2  : not valid */
    {LIF_INVALID_PIN,   LIF_INVALID_PIN},   /* 3  : not valid */
    {LIF_VALID_PIN,     LIF_VALID_PIN},     /* 4  : valid */
    {LIF_VALID_PIN,     LIF_VALID_PIN},     /* 5  : valid */
    {LIF_VALID_PIN,     LIF_VALID_PIN},     /* 6  : valid */
    {LIF_INVALID_PIN,   LIF_INVALID_PIN},   /* 7  : not valid */
    {LIF_INVALID_PIN,   LIF_INVALID_PIN},   /* 8  : not valid */
    {LIF_INVALID_PIN,   LIF_INVALID_PIN},   /* 9  : not valid */
    {LIF_INVALID_PIN,   LIF_INVALID_PIN},   /* 10 : not valid */
    {LIF_INVALID_PIN,   LIF_INVALID_PIN},   /* 11 : not valid */
    {LIF_VALID_PIN,     LIF_VALID_PIN},     /* 12 : valid */
    {LIF_VALID_PIN,     LIF_VALID_PIN},     /* 13 : valid */
    {LIF_INVALID_PIN,   LIF_INVALID_PIN},   /* 14 : not valid */
    {LIF_INVALID_PIN,   LIF_INVALID_PIN},   /* 15 : not valid */
    {LIF_INVALID_PIN,   LIF_INVALID_PIN},   /* 16 : not valid */
    {LIF_INVALID_PIN,   LIF_INVALID_PIN},   /* 17 : not valid */
    {LIF_INVALID_PIN,   LIF_INVALID_PIN},   /* 18 : not valid */
    {LIF_INVALID_PIN,   LIF_INVALID_PIN},   /* 19 : not valid */
    {LIF_INVALID_PIN,   LIF_INVALID_PIN},   /* 20 : not valid */
    {LIF_INVALID_PIN,   LIF_INVALID_PIN},   /* 21 : not valid */
    {LIF_VALID_PIN,     LIF_VALID_PIN},     /* 22 : valid */
    {LIF_VALID_PIN,     LIF_VALID_PIN},     /* 23 : valid */
    {LIF_VALID_PIN,     LIF_VALID_PIN},     /* 24 : valid */
    {LIF_VALID_PIN,     LIF_VALID_PIN},     /* 25 : valid */
    {LIF_VALID_PIN,     LIF_VALID_PIN},     /* 26 : valid */
    {LIF_VALID_PIN,     LIF_VALID_PIN},     /* 27 : valid */
    {LIF_INVALID_PIN,   LIF_INVALID_PIN},   /* 28 : not valid */
    {LIF_INVALID_PIN,   LIF_INVALID_PIN},   /* 29 : not valid */
    {LIF_INVALID_PIN,   LIF_INVALID_PIN},   /* 30 : not valid */
    {LIF_INVALID_PIN,   LIF_INVALID_PIN}    /* 31 : not valid */
#endif
};

#ifdef LIF_NO_SCAN
#define LIF_CLK_PIN_ENTRY 0
#define LIF_DATA_PIN_ENTRY 1

static int32_t GPIO_PINS[RASP_AVAILABLE_PINS] = {LIF_CLK_PIN, LIF_DATA_PIN};
#else
/* from  lif_api.h
   TO-DO: rework of the lif scan handling */
static int32_t GPIO_PINS[RASP_AVAILABLE_PINS] = {4, 5, 6, 12, 13, 22, 23, 24, 25, 26, 27};
#endif

/* ========================================================================== */
/*                   Local Function implementation                            */
/* ========================================================================== */

/**
    This function initializes the global pins table.

   \param
    none
   \return
    none
*/
static void tableinit_lif_pins(void)
{
    uint8_t i;

    /* all cleared to invalid pin */
    for (i = 0; i < LIF_TABLE_PINS_ENTRIES_MAX; i++)
    {
        gLifTablePins[i].cpin = gLifTablePins[i].dpin = LIF_INVALID_PIN;
    }

    /* set the valid pin to valid for the allowed ranges */

    /* pins 4 to 6 */
    for (i = 4; i < 7; i++)
    {
        gLifTablePins[i].cpin = LIF_VALID_PIN;
        gLifTablePins[i].dpin = gLifTablePins[i].cpin;
    }

    /* pins 12 and 13 */
    for (i = 12; i < 14; i++)
    {
        gLifTablePins[i].cpin = LIF_VALID_PIN;
        gLifTablePins[i].dpin = gLifTablePins[i].cpin;
    }

    /* pins 22 to 27 */
    for (i = 22; i < 28; i++)
    {
        gLifTablePins[i].cpin = LIF_VALID_PIN;
        gLifTablePins[i].dpin = gLifTablePins[i].cpin;
    }
}

/**
    This function checks the validity of the library and updates
    the global pins table accordingly at the library initialisation.

   \param
    none
   \return
    LIF_API_RET_SUCCESS      No error
    LIF_API_RET_LIB_ERROR    Library error

*/
static int32_t update_init_lif_lib(char *lib)
{

    /* Ensure we select the correct mdio library for RPI4 */
    if ((strcmp(lib, "bcm2835") != 0) || gLifMdioInit == MAGIC_KEY_LIF_INITIALZED)
    {
        return LIF_API_RET_LIB_ERROR;
    }

    /* library initialized, update the magic key */
    (void)tableinit_lif_pins();
    gLifMdioInit = MAGIC_KEY_LIF_INITIALZED;

    return LIF_API_RET_SUCCESS;
}

/**
    This function checks the validity of the library and updates
    the global pins table accordingly at the library deinitialisation.

   \param
    none
   \return
    LIF_API_RET_SUCCESS      No error
    LIF_API_RET_LIB_ERROR    Library error
*/
static int32_t update_deinit_lif_lib(char *lib)
{

    /* Ensure we select the correct mdio library for RPI4 */
    if ((strcmp(lib, "bcm2835") != 0) || gLifMdioInit != MAGIC_KEY_LIF_INITIALZED)
    {
        return LIF_API_RET_LIB_ERROR;
    }

    /* library deinitialzed, update the magic key */
    gLifMdioInit = MAGIC_KEY_LIF_NOT_INITIALZED;

    return LIF_API_RET_SUCCESS;
}

/**
    This function checks the validity of the library for open, close, read
    and write operations.

   \param
    none
   \return
    LIF_API_RET_SUCCESS      No error
    LIF_API_RET_LIB_ERROR    Library error
*/
static int32_t check_valid_lif_lib(char *lib)
{

    /* Ensure we select the correct mdio library for RPI4 */
    if ((strcmp(lib, "bcm2835") != 0) || gLifMdioInit != MAGIC_KEY_LIF_INITIALZED)
    {
        return LIF_API_RET_LIB_ERROR;
    }

    return LIF_API_RET_SUCCESS;
}

/**
    This function checks the validity of the used GPIO pins on the 40-pin Header
    on the RPI4 board for the clock and data signals of the mdio.

    The valid pin numbers are: 4,5,6,12,13,22,23,24,25,26 and 27.


   \param
    uint8_t clk_pin,   GPIO mdio clock pin number
    uint8_t data_pin,  GPIO mdio data pin number

   \return
    LIF_API_RET_SUCCESS      No error
    LIF_API_RET_PINS_ERROR   Pins validity error
*/
static int32_t check_valid_lif_pins(uint8_t clk_pin, uint8_t data_pin)
{
    /* check the validity of the pins */
    if ((clk_pin == data_pin) ||
        (clk_pin > (LIF_TABLE_PINS_ENTRIES_MAX - 1)) ||
        (data_pin > (LIF_TABLE_PINS_ENTRIES_MAX - 1)))
    {
        return LIF_API_RET_PINS_ERROR;
    }

    if ((gLifTablePins[clk_pin].cpin == LIF_INVALID_PIN) ||
        (gLifTablePins[data_pin].cpin == LIF_INVALID_PIN))
    {
        return LIF_API_RET_PINS_ERROR;
    }

    return LIF_API_RET_SUCCESS;
}

/**
    This function checks the validity of the pins and updates
    the global pins table accordingly for the open opearation.

   \param
    uint16_t clk_pin,   GPIO mdio clock pin number
    uint16_t data_pin,  GPIO mdio data pin number
   \return
    LIF_API_RET_SUCCESS      No error
    LIF_API_RET_PINS_ERROR   Pins validity error
    LIF_API_RET_OPEN_ERROR   Pins open error
*/
static int32_t update_open_lif_pins(uint8_t clk_pin, uint8_t data_pin)
{
    /* check pins validity */
    if (check_valid_lif_pins(clk_pin, data_pin) != LIF_API_RET_SUCCESS)
    {
        return LIF_API_RET_PINS_ERROR;
    }

    /* check if the clock and data pin are free:
       clock and data pins can only be open when both pin are free (LIF_VALID_PIN),
       this is checked with the value in gLifTablePins[clk_pin].cpin for the clock pin
       and gLifTablePins[dat_pin].dpin for the data pin.
       (refer to the gLifTablePins description) */
    if ((gLifTablePins[clk_pin].cpin > LIF_VALID_PIN) ||
        (gLifTablePins[data_pin].dpin > LIF_VALID_PIN))
    {

        return LIF_API_RET_OPEN_ERROR;
    }

    /* update pin table
       (refer to the gLifTablePins description)*/
    gLifTablePins[clk_pin].cpin = gLifTablePins[data_pin].cpin = clk_pin;
    gLifTablePins[clk_pin].dpin = gLifTablePins[data_pin].dpin = data_pin;

    return LIF_API_RET_SUCCESS;
}

/**
    This function checks the validity of the pins and updates
    the global pins table accordingly for the close opearation.

   \param
    uint16_t clk_pin,   GPIO mdio clock pin number
    uint16_t data_pin,  GPIO mdio data pin number
   \return
    LIF_API_RET_SUCCESS      No error
    LIF_API_RET_PINS_ERROR   Pins validity error
    LIF_API_RET_CLOSE_ERROR  Pins close error
*/
static int32_t update_close_lif_pins(uint8_t clk_pin, uint8_t data_pin)
{

    /* check pins validity */
    if (check_valid_lif_pins(clk_pin, data_pin) != LIF_API_RET_SUCCESS)
    {
        return LIF_API_RET_PINS_ERROR;
    }

    /* check that both pins are open:
        clk_pin matches cpin in clk_pin entry in global table pin.
        data_pin matches dpin in data_pin entry in global table pin.
        (refer to the gLifTablePins description). */
    if ((gLifTablePins[clk_pin].cpin != clk_pin) ||
        (gLifTablePins[data_pin].dpin != data_pin))
    {
        return LIF_API_RET_CLOSE_ERROR;
    }

    /* update pin table with both pins set to close (value LIF_VALID_PIN) */
    gLifTablePins[clk_pin].cpin = LIF_VALID_PIN;
    gLifTablePins[clk_pin].dpin = gLifTablePins[clk_pin].cpin;

    gLifTablePins[data_pin].cpin = LIF_VALID_PIN;
    gLifTablePins[data_pin].dpin = gLifTablePins[data_pin].cpin;

    return LIF_API_RET_SUCCESS;
}

/**
    This function checks the validity of the pins according
    to the global pin table for read and write operations.

   \param
    uint16_t clk_pin,   GPIO mdio clock pin number
    uint16_t data_pin,  GPIO mdio data pin number
   \return
    LIF_API_RET_SUCCESS      No error
    LIF_API_RET_PINS_ERROR   Pins validity error
    LIF_API_RET_ACCESS_ERROR  Pins RW Access error
*/
static int32_t check_rw_lif_pins(uint8_t clk_pin, uint8_t data_pin)
{

    /* check pins validity */
    if (check_valid_lif_pins(clk_pin, data_pin) != LIF_API_RET_SUCCESS)
    {
        return LIF_API_RET_PINS_ERROR;
    }

    /* check that both pins are open:
        clk_pin matches cpin in clk_pin entry in global table pin.
        data_pin matches dpin in data_pin entry in global table pin.
     */
    if ((gLifTablePins[clk_pin].cpin != clk_pin) ||
        (gLifTablePins[data_pin].dpin != data_pin))
    {
        return LIF_API_RET_ACCESS_ERROR;
    }
    return LIF_API_RET_SUCCESS;
}

/* ========================================================================== */
/*                   Function implementation                                  */
/* ========================================================================== */
/**
    Implements the MDIO library (PyRPIO only) Initialization.


   \param
    char*    lib,       GPIO Interface library name

   \return
    LIF_API_RET_SUCCESS      No error
    LIF_API_RET_LIB_ERROR    Library error
*/
int32_t lif_mdio_init(char *lib)
{
    int32_t ret;

    /* Ensure we select the correct mdio library for RPI4
       and update global vaiable */
    ret = update_init_lif_lib(lib);
    if (ret != LIF_API_RET_SUCCESS)
    {
        return ret;
    }

    /* initialize the mdio library:
       return code: 1=success, 0=error
    */
    if (bcm2835_init(1) == 1)
    {
        return LIF_API_RET_SUCCESS;
    }
    else
    {
        /* we do not evaluate the return code */
        (void)lif_mdio_deinit(lib);
        return LIF_API_RET_LIB_ERROR;
    }
}

/**
    Implements the MDIO library (PyRPIO only) Deinitialization.


   \param
    char*    lib,       GPIO Interface library name

   \return
    LIF_API_RET_SUCCESS      No error
    LIF_API_RET_LIB_ERROR    Library error
*/
int32_t lif_mdio_deinit(char *lib)
{
    int32_t ret;

    /* Ensure we select the correct mdio library for RPI4
       and update global vaiable */
    ret = update_deinit_lif_lib(lib);
    if (ret != LIF_API_RET_SUCCESS)
    {
        return ret;
    }

    /* shutdown the mdio library:
       return code: always 1
       we do not evaluate the return code
    */
    (void)bcm2835_close();

    return LIF_API_RET_SUCCESS;
}

/**
    Implements the MDIO library (PyRPIO only) Open.
    Initialize the MDIO clock and data pins for the MDIO connection.

   \param
    char*    lib,       GPIO Interface library name
    uint16_t clk_pin,   GPIO mdio clock pin number
    uint16_t data_pin,  GPIO mdio data pin number

   \return
    LIF_API_RET_SUCCESS      No error
    LIF_API_RET_LIB_ERROR    Library error
    LIF_API_RET_PINS_ERROR   Pins validity error
    LIF_API_RET_OPEN_ERROR   Pins open error
*/
int32_t lif_mdio_open(char *lib, uint8_t clk_pin, uint8_t data_pin)
{
    int32_t ret;

    /* check library validity */
    ret = check_valid_lif_lib(lib);
    if (ret != LIF_API_RET_SUCCESS)
    {
        return ret;
    }

    /* check and update global pins table */
    ret = update_open_lif_pins(clk_pin, data_pin);
    if (ret != LIF_API_RET_SUCCESS)
    {
        return ret;
    }

    /* Open the mdio connection
       return code: always 0.
       we do not evaluate the return code
    */
    (void)mdio_open(clk_pin, data_pin);

    return LIF_API_RET_SUCCESS;
}

/**
    Implements the MDIO library (PyRPIO only) Close.
    Deinitialize the MDIO clock and data pins of the MDIO connection.

   \param
    char*    lib,       GPIO Interface library name
    uint16_t clk_pin,   GPIO mdio clock pin number
    uint16_t data_pin,  GPIO mdio data pin number

   \return
    LIF_API_RET_SUCCESS      No error
    LIF_API_RET_LIB_ERROR    Library error
    LIF_API_RET_PINS_ERROR   Pins validity error
    LIF_API_RET_CLOSE_ERROR  Pins close error
*/

int32_t lif_mdio_close(char *lib, uint8_t clk_pin, uint8_t data_pin)
{
    int32_t ret;

    /* check library validity */
    ret = check_valid_lif_lib(lib);
    if (ret != LIF_API_RET_SUCCESS)
    {
        return ret;
    }

    /* check and update global pins table */
    ret = update_close_lif_pins(clk_pin, data_pin);
    if (ret != LIF_API_RET_SUCCESS)
    {
        return ret;
    }

    /* close the mdio connection
       return code: always 0.
       we do not evaluate the return code
    */
    (void)mdio_close(clk_pin, data_pin);

    return LIF_API_RET_SUCCESS;
}

/**
    Implements the MDIO library (PyRPIO only) Read Register Clause 22.

   \param
    uint8_t lif_id,     Link Index number
    uint16_t pad,       PHY address
    uint16_t dad,       Register Address

   \return
    >= 0                      No error: Register value
    LIF_API_RET_PINS_ERROR    Pins validity error
    LIF_API_RET_ACCESS_ERROR  Pins RW Access error
*/
int32_t lif_mdio_c22_read(uint8_t lif_id, uint8_t pad, uint8_t dad)
{
    uint16_t rret; /* return type from the underlying mdio library */
    int32_t ret;

    /* check library validity */
    ret = check_valid_lif_lib(scanned_devices[lif_id].lib);
    if (ret != LIF_API_RET_SUCCESS)
    {
        return ret;
    }

    /* check pins validity */
    ret = check_rw_lif_pins(scanned_devices[lif_id].cpin, scanned_devices[lif_id].dpin);
    if (ret != LIF_API_RET_SUCCESS)
    {
        return ret;
    }

    /* call the underlying mdio library returning
       the uint16_t register value */

    rret = mdio_c22_read(scanned_devices[lif_id].cpin, scanned_devices[lif_id].dpin, pad, dad);

    /* convert from 16-bit to 32-bit unsigned */
    return ((int32_t)rret & 0x0000FFFF);
}

/**
    Implements the MDIO library (PyRPIO only) Write Register Clause 22.

   \param
    uint8_t lif_id,     Link Index number
    uint16_t pad,       PHY address
    uint16_t dad,       Register Address
    uint16_t val,       Register value to write
   \return
    LIF_API_RET_SUCCESS      No error
    LIF_API_RET_PINS_ERROR   Pins validity error
    LIF_API_RET_ACCESS_ERROR Pins RW Access error
*/
int32_t lif_mdio_c22_write(uint8_t lif_id, uint8_t pad, uint8_t dad, uint16_t val)
{
    int32_t ret;

    /* check library validity */
    ret = check_valid_lif_lib(scanned_devices[lif_id].lib);
    if (ret != LIF_API_RET_SUCCESS)
    {
        return ret;
    }

    /* check pins validity */
    ret = check_rw_lif_pins(scanned_devices[lif_id].cpin, scanned_devices[lif_id].dpin);
    if (ret != LIF_API_RET_SUCCESS)
    {
        return ret;
    }

    /* call the underlying mdio library returning
       always 0. We do not evaluate the return code */
    (void)mdio_c22_write(scanned_devices[lif_id].cpin, scanned_devices[lif_id].dpin, pad, dad, val);

    return LIF_API_RET_SUCCESS;
}

/**
    Implements the MDIO library (PyRPIO only) Read Register Clause 45.

   \param
    uint8_t lif_id,     Link Index number
    uint16_t pad,       PHY address
    uint16_t dad,       DEVICE address
    uint16_t reg,       Register Address

   \return
    >= 0                      No error: Register value
    LIF_API_RET_PINS_ERROR    Pins validity error
    LIF_API_RET_ACCESS_ERROR  Pins RW Access error
*/
int32_t lif_mdio_c45_read(uint8_t lif_id, uint8_t pad, uint8_t dad, uint16_t reg)
{
    uint16_t rret; /* return type from the underlying mdio library */
    int32_t ret;

    /* check library validity */
    ret = check_valid_lif_lib(scanned_devices[lif_id].lib);
    if (ret != LIF_API_RET_SUCCESS)
    {
        return ret;
    }

    /* check pins validity */
    ret = check_rw_lif_pins(scanned_devices[lif_id].cpin, scanned_devices[lif_id].dpin);
    if (ret != LIF_API_RET_SUCCESS)
    {
        return ret;
    }

    /* call the underlying mdio library returning
       the uint16_t register value */
    rret = mdio_c45_read(scanned_devices[lif_id].cpin, scanned_devices[lif_id].dpin, pad, dad, reg);

    /*  convert from 16-bit to 32-bit unsigned */
    return ((int32_t)rret & 0x0000FFFF);
}

/**
    Implements the MDIO library (PyRPIO only) Write Register Clause 45.

   \param
    uint8_t lif_id,     Link Index number
    uint16_t pad,       PHY address
    uint16_t dad,       DEVICE Address
    uint16_t val,       Register value to write
   \return
    LIF_API_RET_SUCCESS      No error
    LIF_API_RET_PINS_ERROR   Pins validity error
    LIF_API_RET_ACCESS_ERROR Pins RW Access error
*/
int32_t lif_mdio_c45_write(uint8_t lif_id, uint8_t pad, uint8_t dad, uint16_t reg, uint16_t val)
{

    int32_t ret;

    /* check library validity */
    ret = check_valid_lif_lib(scanned_devices[lif_id].lib);
    if (ret != LIF_API_RET_SUCCESS)
    {
        return ret;
    }

    /* check pins validity */
    ret = check_rw_lif_pins(scanned_devices[lif_id].cpin, scanned_devices[lif_id].dpin);
    if (ret != LIF_API_RET_SUCCESS)
    {
        return ret;
    }

    /* call the underlying mdio library returning
       always 0. We do not evaluate the return code */
    (void)mdio_c45_write(scanned_devices[lif_id].cpin, scanned_devices[lif_id].dpin, pad, dad, reg, val);

    return LIF_API_RET_SUCCESS;
}

/******************************************
    Implements MDIO links scans functions.
*******************************************/

#ifdef LIF_NO_SCAN

int32_t lif_scan(char *lib)
{
    int32_t ret;
    int32_t nr_dev;
    uint8_t phy;
    uint8_t phy_index;

    nr_dev = 0;
    scanned_devices[nr_dev].lib = lib;
    scanned_devices[nr_dev].cpin = GPIO_PINS[LIF_CLK_PIN_ENTRY];
    scanned_devices[nr_dev].dpin = GPIO_PINS[LIF_DATA_PIN_ENTRY];
    scanned_devices[nr_dev].nr_phys = 0;

    ret = lif_mdio_open(lib, scanned_devices[nr_dev].cpin, scanned_devices[nr_dev].dpin);
    if (ret != LIF_API_RET_SUCCESS)
    {
        scanned_nr_lif = 0;
        return scanned_nr_lif;
    }

    phy_index = 0;
    for (phy = 0; phy < PHY_MAX_VAL; phy++)
    {
        /* we read MDIO PHY Identifier 2 (Register 0.3) for checking
            the presence  of a PHY port  (value > 0) */

        ret = lif_mdio_c22_read(nr_dev, phy, 3);
        if (ret != 0xffff)
        {
            /* SMDIO (SSB) there is no valid ID and is returning 0 value */
            if (phy == 0 && ret == 0)
                break;

            scanned_devices[nr_dev].nr_phys++;
            scanned_devices[nr_dev].phy_info[phy_index][0] = phy;
            scanned_devices[nr_dev].phy_info[phy_index][1] = ret;
            phy_index++;
        }
    }

    if (phy_index == 0)
    {
        lif_mdio_close(lib, scanned_devices[nr_dev].cpin, scanned_devices[nr_dev].dpin);
    }

    nr_dev++;
    scanned_nr_lif = nr_dev;

    return scanned_nr_lif;
}

#else  /* LIF_NO_SCAN */
int32_t lif_scan(char *lib)
{
    int i, j;
    int32_t nr_dev = 0;
    uint8_t pins[COMB_MAX_VAL][2];
    int count = 0;

    for (i = 0; i < RASP_AVAILABLE_PINS; i++)
    {
        for (j = 0; j < RASP_AVAILABLE_PINS; j++)
        {
            if (GPIO_PINS[i] != GPIO_PINS[j])
            {
                pins[count][0] = GPIO_PINS[i];
                pins[count][1] = GPIO_PINS[j];
                count++;
            }
        }
    }

    for (i = 0; i < COMB_MAX_VAL; i++)
    {
        int32_t ret;

        ret = lif_mdio_open(lib, pins[i][0], pins[i][1]);
        if (ret == LIF_API_RET_SUCCESS)
        {
            uint8_t phy;
            uint8_t phy_index = 0;
            int flag_first = 1;

            if (nr_dev >= MAX_LINKS)
            {
                scanned_nr_lif = 0;
                return scanned_nr_lif;
            }
            scanned_devices[nr_dev].lib = lib;
            scanned_devices[nr_dev].cpin = pins[i][0];
            scanned_devices[nr_dev].dpin = pins[i][1];

            for (phy = 0; phy < PHY_MAX_VAL; phy++)
            {
                /* we read MDIO PHY Identifier 2 (Register 0.3) for checking
                 the presence  of a PHY port  (value > 0) */

                ret = lif_mdio_c22_read(nr_dev, phy, 3);
                if (ret != 0xffff)
                {
                    /* SMDIO (SSB) there is no valid ID and is returning 0 value */
                    if (phy == 0 && ret == 0)
                        break;

                    if (nr_dev >= MAX_LINKS)
                    {
                        scanned_nr_lif = 0;
                        return scanned_nr_lif;
                    }

                    scanned_devices[nr_dev].nr_phys++;
                    scanned_devices[nr_dev].phy_info[phy_index][0] = phy;
                    scanned_devices[nr_dev].phy_info[phy_index][1] = ret;
                    phy_index++;
                }
            }

            if (scanned_devices[nr_dev].nr_phys > 0)
            {
                nr_dev++;
            }

            if (phy_index == 0)
            {
                lif_mdio_close(lib, pins[i][0], pins[i][1]);
            }
        }
    }

    if (nr_dev >= MAX_LINKS)
        nr_dev = 0;
    scanned_nr_lif = nr_dev;

    return scanned_nr_lif;
}
#endif /* LIF_NO_SCAN */

int32_t lif_get_cpin(uint8_t lif_id)
{
    return scanned_devices[lif_id].cpin;
}

int32_t lif_get_dpin(uint8_t lif_id)
{
    return scanned_devices[lif_id].dpin;
}

int32_t lif_get_nr_phys(uint8_t lif_id)
{
    return scanned_devices[lif_id].nr_phys;
}

int32_t lif_get_phy_addr(uint8_t lif_id, uint8_t phy)
{
    return scanned_devices[lif_id].phy_info[phy][0];
}

int32_t lif_get_phy_id(uint8_t lif_id, uint8_t phy)
{
    return scanned_devices[lif_id].phy_info[phy][1];
}

int32_t lif_get_nr_lif(void)
{
    return scanned_nr_lif;
}
