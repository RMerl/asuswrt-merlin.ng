/******************************************************************************
 *
 * Module Name: utglobal - Global variables for the ACPI subsystem
 *
 *****************************************************************************/

/*
 * Copyright (C) 2000 - 2015, Intel Corp.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    substantially similar to the "NO WARRANTY" disclaimer below
 *    ("Disclaimer") and any redistribution must be conditioned upon
 *    including a substantially similar Disclaimer requirement for further
 *    binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 */

#define EXPORT_ACPI_INTERFACES
#define DEFINE_ACPI_GLOBALS

#include <acpi/acpi.h>
#include "accommon.h"

#define _COMPONENT          ACPI_UTILITIES
ACPI_MODULE_NAME("utglobal")

/*******************************************************************************
 *
 * Static global variable initialization.
 *
 ******************************************************************************/
/* Various state name strings */
const char *acpi_gbl_sleep_state_names[ACPI_S_STATE_COUNT] = {
	"\\_S0_",
	"\\_S1_",
	"\\_S2_",
	"\\_S3_",
	"\\_S4_",
	"\\_S5_"
};

const char *acpi_gbl_lowest_dstate_names[ACPI_NUM_sx_w_METHODS] = {
	"_S0W",
	"_S1W",
	"_S2W",
	"_S3W",
	"_S4W"
};

const char *acpi_gbl_highest_dstate_names[ACPI_NUM_sx_d_METHODS] = {
	"_S1D",
	"_S2D",
	"_S3D",
	"_S4D"
};

/*******************************************************************************
 *
 * Namespace globals
 *
 ******************************************************************************/
/*
 * Predefined ACPI Names (Built-in to the Interpreter)
 *
 * NOTES:
 * 1) _SB_ is defined to be a device to allow \_SB_._INI to be run
 *    during the initialization sequence.
 * 2) _TZ_ is defined to be a thermal zone in order to allow ASL code to
 *    perform a Notify() operation on it. 09/2010: Changed to type Device.
 *    This still allows notifies, but does not confuse host code that
 *    searches for valid thermal_zone objects.
 */
const struct acpi_predefined_names acpi_gbl_pre_defined_names[] = {
	{"_GPE", ACPI_TYPE_LOCAL_SCOPE, NULL},
	{"_PR_", ACPI_TYPE_LOCAL_SCOPE, NULL},
	{"_SB_", ACPI_TYPE_DEVICE, NULL},
	{"_SI_", ACPI_TYPE_LOCAL_SCOPE, NULL},
	{"_TZ_", ACPI_TYPE_DEVICE, NULL},
	{"_REV", ACPI_TYPE_INTEGER, (char *)ACPI_CA_SUPPORT_LEVEL},
	{"_OS_", ACPI_TYPE_STRING, ACPI_OS_NAME},
	{"_GL_", ACPI_TYPE_MUTEX, (char *)1},

#if !defined (ACPI_NO_METHOD_EXECUTION) || defined (ACPI_CONSTANT_EVAL_ONLY)
	{"_OSI", ACPI_TYPE_METHOD, (char *)1},
#endif

	/* Table terminator */

	{NULL, ACPI_TYPE_ANY, NULL}
};

#if (!ACPI_REDUCED_HARDWARE)
/******************************************************************************
 *
 * Event and Hardware globals
 *
 ******************************************************************************/

struct acpi_bit_register_info acpi_gbl_bit_register_info[ACPI_NUM_BITREG] = {
	/* Name                                     Parent Register             Register Bit Position                   Register Bit Mask       */

	/* ACPI_BITREG_TIMER_STATUS         */ {ACPI_REGISTER_PM1_STATUS,
						ACPI_BITPOSITION_TIMER_STATUS,
						ACPI_BITMASK_TIMER_STATUS},
	/* ACPI_BITREG_BUS_MASTER_STATUS    */ {ACPI_REGISTER_PM1_STATUS,
						ACPI_BITPOSITION_BUS_MASTER_STATUS,
						ACPI_BITMASK_BUS_MASTER_STATUS},
	/* ACPI_BITREG_GLOBAL_LOCK_STATUS   */ {ACPI_REGISTER_PM1_STATUS,
						ACPI_BITPOSITION_GLOBAL_LOCK_STATUS,
						ACPI_BITMASK_GLOBAL_LOCK_STATUS},
	/* ACPI_BITREG_POWER_BUTTON_STATUS  */ {ACPI_REGISTER_PM1_STATUS,
						ACPI_BITPOSITION_POWER_BUTTON_STATUS,
						ACPI_BITMASK_POWER_BUTTON_STATUS},
	/* ACPI_BITREG_SLEEP_BUTTON_STATUS  */ {ACPI_REGISTER_PM1_STATUS,
						ACPI_BITPOSITION_SLEEP_BUTTON_STATUS,
						ACPI_BITMASK_SLEEP_BUTTON_STATUS},
	/* ACPI_BITREG_RT_CLOCK_STATUS      */ {ACPI_REGISTER_PM1_STATUS,
						ACPI_BITPOSITION_RT_CLOCK_STATUS,
						ACPI_BITMASK_RT_CLOCK_STATUS},
	/* ACPI_BITREG_WAKE_STATUS          */ {ACPI_REGISTER_PM1_STATUS,
						ACPI_BITPOSITION_WAKE_STATUS,
						ACPI_BITMASK_WAKE_STATUS},
	/* ACPI_BITREG_PCIEXP_WAKE_STATUS   */ {ACPI_REGISTER_PM1_STATUS,
						ACPI_BITPOSITION_PCIEXP_WAKE_STATUS,
						ACPI_BITMASK_PCIEXP_WAKE_STATUS},

	/* ACPI_BITREG_TIMER_ENABLE         */ {ACPI_REGISTER_PM1_ENABLE,
						ACPI_BITPOSITION_TIMER_ENABLE,
						ACPI_BITMASK_TIMER_ENABLE},
	/* ACPI_BITREG_GLOBAL_LOCK_ENABLE   */ {ACPI_REGISTER_PM1_ENABLE,
						ACPI_BITPOSITION_GLOBAL_LOCK_ENABLE,
						ACPI_BITMASK_GLOBAL_LOCK_ENABLE},
	/* ACPI_BITREG_POWER_BUTTON_ENABLE  */ {ACPI_REGISTER_PM1_ENABLE,
						ACPI_BITPOSITION_POWER_BUTTON_ENABLE,
						ACPI_BITMASK_POWER_BUTTON_ENABLE},
	/* ACPI_BITREG_SLEEP_BUTTON_ENABLE  */ {ACPI_REGISTER_PM1_ENABLE,
						ACPI_BITPOSITION_SLEEP_BUTTON_ENABLE,
						ACPI_BITMASK_SLEEP_BUTTON_ENABLE},
	/* ACPI_BITREG_RT_CLOCK_ENABLE      */ {ACPI_REGISTER_PM1_ENABLE,
						ACPI_BITPOSITION_RT_CLOCK_ENABLE,
						ACPI_BITMASK_RT_CLOCK_ENABLE},
	/* ACPI_BITREG_PCIEXP_WAKE_DISABLE  */ {ACPI_REGISTER_PM1_ENABLE,
						ACPI_BITPOSITION_PCIEXP_WAKE_DISABLE,
						ACPI_BITMASK_PCIEXP_WAKE_DISABLE},

	/* ACPI_BITREG_SCI_ENABLE           */ {ACPI_REGISTER_PM1_CONTROL,
						ACPI_BITPOSITION_SCI_ENABLE,
						ACPI_BITMASK_SCI_ENABLE},
	/* ACPI_BITREG_BUS_MASTER_RLD       */ {ACPI_REGISTER_PM1_CONTROL,
						ACPI_BITPOSITION_BUS_MASTER_RLD,
						ACPI_BITMASK_BUS_MASTER_RLD},
	/* ACPI_BITREG_GLOBAL_LOCK_RELEASE  */ {ACPI_REGISTER_PM1_CONTROL,
						ACPI_BITPOSITION_GLOBAL_LOCK_RELEASE,
						ACPI_BITMASK_GLOBAL_LOCK_RELEASE},
	/* ACPI_BITREG_SLEEP_TYPE           */ {ACPI_REGISTER_PM1_CONTROL,
						ACPI_BITPOSITION_SLEEP_TYPE,
						ACPI_BITMASK_SLEEP_TYPE},
	/* ACPI_BITREG_SLEEP_ENABLE         */ {ACPI_REGISTER_PM1_CONTROL,
						ACPI_BITPOSITION_SLEEP_ENABLE,
						ACPI_BITMASK_SLEEP_ENABLE},

	/* ACPI_BITREG_ARB_DIS              */ {ACPI_REGISTER_PM2_CONTROL,
						ACPI_BITPOSITION_ARB_DISABLE,
						ACPI_BITMASK_ARB_DISABLE}
};

struct acpi_fixed_event_info acpi_gbl_fixed_event_info[ACPI_NUM_FIXED_EVENTS] = {
	/* ACPI_EVENT_PMTIMER       */ {ACPI_BITREG_TIMER_STATUS,
					ACPI_BITREG_TIMER_ENABLE,
					ACPI_BITMASK_TIMER_STATUS,
					ACPI_BITMASK_TIMER_ENABLE},
	/* ACPI_EVENT_GLOBAL        */ {ACPI_BITREG_GLOBAL_LOCK_STATUS,
					ACPI_BITREG_GLOBAL_LOCK_ENABLE,
					ACPI_BITMASK_GLOBAL_LOCK_STATUS,
					ACPI_BITMASK_GLOBAL_LOCK_ENABLE},
	/* ACPI_EVENT_POWER_BUTTON  */ {ACPI_BITREG_POWER_BUTTON_STATUS,
					ACPI_BITREG_POWER_BUTTON_ENABLE,
					ACPI_BITMASK_POWER_BUTTON_STATUS,
					ACPI_BITMASK_POWER_BUTTON_ENABLE},
	/* ACPI_EVENT_SLEEP_BUTTON  */ {ACPI_BITREG_SLEEP_BUTTON_STATUS,
					ACPI_BITREG_SLEEP_BUTTON_ENABLE,
					ACPI_BITMASK_SLEEP_BUTTON_STATUS,
					ACPI_BITMASK_SLEEP_BUTTON_ENABLE},
	/* ACPI_EVENT_RTC           */ {ACPI_BITREG_RT_CLOCK_STATUS,
					ACPI_BITREG_RT_CLOCK_ENABLE,
					ACPI_BITMASK_RT_CLOCK_STATUS,
					ACPI_BITMASK_RT_CLOCK_ENABLE},
};
#endif				/* !ACPI_REDUCED_HARDWARE */

/* Public globals */

ACPI_EXPORT_SYMBOL(acpi_gbl_FADT)
ACPI_EXPORT_SYMBOL(acpi_dbg_level)
ACPI_EXPORT_SYMBOL(acpi_dbg_layer)
ACPI_EXPORT_SYMBOL(acpi_gpe_count)
ACPI_EXPORT_SYMBOL(acpi_current_gpe_count)
