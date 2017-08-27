/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * libqmi-glib -- GLib/GIO based library to control QMI devices
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 * Copyright (C) 2012 Google Inc.
 */

#ifndef _LIBQMI_GLIB_QMI_ENUMS_UIM_H_
#define _LIBQMI_GLIB_QMI_ENUMS_UIM_H_

/**
 * SECTION: qmi-enums-uim
 * @title: UIM enumerations and flags
 *
 * This section defines enumerations and flags used in the UIM service
 * interface.
 */

/*****************************************************************************/
/* Helper enums for the 'QMI UIM Read Record' request/response */

/**
 * QmiUimSessionType:
 * @QMI_UIM_SESSION_TYPE_PRIMARY_GW_PROVISIONING: Primary GSM/WCDMA provisioning.
 * @QMI_UIM_SESSION_TYPE_PRIMARY_1X_PROVISIONING: Primary CDMA1x provisioning.
 * @QMI_UIM_SESSION_TYPE_SECONDARY_GW_PROVISIONING: Secondary GSM/WCDMA provisioning.
 * @QMI_UIM_SESSION_TYPE_SECONDARY_1X_PROVISIONING: Secondary CDMA1x provisioning.
 * @QMI_UIM_SESSION_TYPE_NONPROVISIONING_SLOT_1: Nonprovisioning on slot 1.
 * @QMI_UIM_SESSION_TYPE_NONPROVISIONING_SLOT_2: Nonprovisioning on slot 2.
 * @QMI_UIM_SESSION_TYPE_CARD_SLOT_1: Card on slot 1.
 * @QMI_UIM_SESSION_TYPE_CARD_SLOT_2: Card on slot 2.
 * @QMI_UIM_SESSION_TYPE_LOGICAL_CHANNEL_SLOT_1: Logical channel on slot 1.
 * @QMI_UIM_SESSION_TYPE_LOGICAL_CHANNEL_SLOT_2: Logical channel on slot 2.
 *
 * Type of UIM session.
 */
typedef enum {
    QMI_UIM_SESSION_TYPE_PRIMARY_GW_PROVISIONING   = 0,
    QMI_UIM_SESSION_TYPE_PRIMARY_1X_PROVISIONING   = 1,
    QMI_UIM_SESSION_TYPE_SECONDARY_GW_PROVISIONING = 2,
    QMI_UIM_SESSION_TYPE_SECONDARY_1X_PROVISIONING = 3,
    QMI_UIM_SESSION_TYPE_NONPROVISIONING_SLOT_1    = 4,
    QMI_UIM_SESSION_TYPE_NONPROVISIONING_SLOT_2    = 5,
    QMI_UIM_SESSION_TYPE_CARD_SLOT_1               = 6,
    QMI_UIM_SESSION_TYPE_CARD_SLOT_2               = 7,
    QMI_UIM_SESSION_TYPE_LOGICAL_CHANNEL_SLOT_1    = 8,
    QMI_UIM_SESSION_TYPE_LOGICAL_CHANNEL_SLOT_2    = 9
} QmiUimSessionType;

/*****************************************************************************/
/* Helper enums for the 'QMI UIM Get File Attributes' request/response */

/**
 * QmiUimFileType:
 * @QMI_UIM_FILE_TYPE_TRANSPARENT: Transparent.
 * @QMI_UIM_FILE_TYPE_CYCLIC: Cyclic.
 * @QMI_UIM_FILE_TYPE_LINEAR_FIXED: Linear fixed.
 * @QMI_UIM_FILE_TYPE_DEDICATED_FILE: Dedicated file.
 * @QMI_UIM_FILE_TYPE_MASTER_FILE: Master file.
 *
 * Type of UIM file.
 */
typedef enum {
    QMI_UIM_FILE_TYPE_TRANSPARENT    = 0,
    QMI_UIM_FILE_TYPE_CYCLIC         = 1,
    QMI_UIM_FILE_TYPE_LINEAR_FIXED   = 2,
    QMI_UIM_FILE_TYPE_DEDICATED_FILE = 3,
    QMI_UIM_FILE_TYPE_MASTER_FILE    = 4
} QmiUimFileType;

/**
 * QmiUimSecurityAttributeLogic:
 * @QMI_UIM_SECURITY_ATTRIBUTE_LOGIC_ALWAYS: Always.
 * @QMI_UIM_SECURITY_ATTRIBUTE_LOGIC_NEVER: Never.
 * @QMI_UIM_SECURITY_ATTRIBUTE_LOGIC_AND: And.
 * @QMI_UIM_SECURITY_ATTRIBUTE_LOGIC_OR: Or.
 * @QMI_UIM_SECURITY_ATTRIBUTE_LOGIC_SINGLE: Single.
 *
 * Logic applicable to security attributes.
 */
typedef enum {
    QMI_UIM_SECURITY_ATTRIBUTE_LOGIC_ALWAYS = 0,
    QMI_UIM_SECURITY_ATTRIBUTE_LOGIC_NEVER  = 1,
    QMI_UIM_SECURITY_ATTRIBUTE_LOGIC_AND    = 2,
    QMI_UIM_SECURITY_ATTRIBUTE_LOGIC_OR     = 3,
    QMI_UIM_SECURITY_ATTRIBUTE_LOGIC_SINGLE = 4
} QmiUimSecurityAttributeLogic;

/**
 * QmiUimSecurityAttribute:
 * @QMI_UIM_SECURITY_ATTRIBUTE_PIN1: PIN1.
 * @QMI_UIM_SECURITY_ATTRIBUTE_PIN2: PIN2.
 * @QMI_UIM_SECURITY_ATTRIBUTE_UPIN: UPIN.
 * @QMI_UIM_SECURITY_ATTRIBUTE_ADM: ADM.
 *
 * Security Attributes.
 */
typedef enum {
    QMI_UIM_SECURITY_ATTRIBUTE_PIN1 = 1 << 0,
    QMI_UIM_SECURITY_ATTRIBUTE_PIN2 = 1 << 1,
    QMI_UIM_SECURITY_ATTRIBUTE_UPIN = 1 << 2,
    QMI_UIM_SECURITY_ATTRIBUTE_ADM  = 1 << 3
} QmiUimSecurityAttribute;

/*****************************************************************************/
/* Helper enums for the 'QMI UIM Set PIN Protection' */

/**
 * QmiUimPinId:
 * @QMI_UIM_PIN_ID_UNKNOWN: Unknown.
 * @QMI_UIM_PIN_ID_PIN1: PIN1.
 * @QMI_UIM_PIN_ID_PIN2: PIN2.
 * @QMI_UIM_PIN_ID_UPIN: UPIN.
 * @QMI_UIM_PIN_ID_HIDDEN_KEY: Hidden key.
 *
 * PIN ID.
 */
typedef enum {
    QMI_UIM_PIN_ID_UNKNOWN    = 0,
    QMI_UIM_PIN_ID_PIN1       = 1,
    QMI_UIM_PIN_ID_PIN2       = 2,
    QMI_UIM_PIN_ID_UPIN       = 3,
    QMI_UIM_PIN_ID_HIDDEN_KEY = 4
} QmiUimPinId;

/*****************************************************************************/
/* Helper enums for the 'QMI UIM Get Card Status' request/response */

/**
 * QmiUimCardState:
 * @QMI_UIM_CARD_STATE_ABSENT: Absent.
 * @QMI_UIM_CARD_STATE_PRESENT: Present.
 * @QMI_UIM_CARD_STATE_ERROR: Error.
 *
 * State of the card.
 */
typedef enum {
    QMI_UIM_CARD_STATE_ABSENT  = 0,
    QMI_UIM_CARD_STATE_PRESENT = 1,
    QMI_UIM_CARD_STATE_ERROR   = 2
} QmiUimCardState;

/**
 * QmiUimPinState:
 * @QMI_UIM_PIN_STATE_NOT_INITIALIZED: Not initialized.
 * @QMI_UIM_PIN_STATE_ENABLED_NOT_VERIFIED: Enabled, not verified.
 * @QMI_UIM_PIN_STATE_ENABLED_VERIFIED: Enabled, verified.
 * @QMI_UIM_PIN_STATE_DISABLED: Disabled.
 * @QMI_UIM_PIN_STATE_BLOCKED: Blocked.
 * @QMI_UIM_PIN_STATE_PERMANENTLY_BLOCKED: Permanently Blocked.
 *
 * The PIN state.
 */
typedef enum {
    QMI_UIM_PIN_STATE_NOT_INITIALIZED      = 0,
    QMI_UIM_PIN_STATE_ENABLED_NOT_VERIFIED = 1,
    QMI_UIM_PIN_STATE_ENABLED_VERIFIED     = 2,
    QMI_UIM_PIN_STATE_DISABLED             = 3,
    QMI_UIM_PIN_STATE_BLOCKED              = 4,
    QMI_UIM_PIN_STATE_PERMANENTLY_BLOCKED  = 5,
} QmiUimPinState;

/**
 * QmiUimCardError:
 * @QMI_UIM_CARD_ERROR_UNKNOWN: Unknown error.
 * @QMI_UIM_CARD_ERROR_POWER_DOWN: Power down.
 * @QMI_UIM_CARD_ERROR_POLL: Poll error.
 * @QMI_UIM_CARD_ERROR_NO_ATR_RECEIVED: No ATR received.
 * @QMI_UIM_CARD_ERROR_VOLTAGE_MISMATCH: Voltage mismatch.
 * @QMI_UIM_CARD_ERROR_PARITY: Parity error.
 * @QMI_UIM_CARD_ERROR_POSSIBLY_REMOVED: Unknown error, possibly removed.
 * @QMI_UIM_CARD_ERROR_TECHNICAL: Technical problem.
 *
 * Card error.
 */
typedef enum {
    QMI_UIM_CARD_ERROR_UNKNOWN          = 0,
    QMI_UIM_CARD_ERROR_POWER_DOWN       = 1,
    QMI_UIM_CARD_ERROR_POLL             = 2,
    QMI_UIM_CARD_ERROR_NO_ATR_RECEIVED  = 3,
    QMI_UIM_CARD_ERROR_VOLTAGE_MISMATCH = 4,
    QMI_UIM_CARD_ERROR_PARITY           = 5,
    QMI_UIM_CARD_ERROR_POSSIBLY_REMOVED = 6,
    QMI_UIM_CARD_ERROR_TECHNICAL        = 7
} QmiUimCardError;

/**
 * QmiUimCardApplicationType:
 * @QMI_UIM_CARD_APPLICATION_TYPE_UNKNOWN: Unknown.
 * @QMI_UIM_CARD_APPLICATION_TYPE_SIM: SIM.
 * @QMI_UIM_CARD_APPLICATION_TYPE_USIM: USIM.
 * @QMI_UIM_CARD_APPLICATION_TYPE_RUIM: RUIM.
 * @QMI_UIM_CARD_APPLICATION_TYPE_CSIM: CSIM.
 * @QMI_UIM_CARD_APPLICATION_TYPE_ISIM: ISIM.
 *
 * Card application type.
 */
typedef enum {
    QMI_UIM_CARD_APPLICATION_TYPE_UNKNOWN = 0,
    QMI_UIM_CARD_APPLICATION_TYPE_SIM     = 1,
    QMI_UIM_CARD_APPLICATION_TYPE_USIM    = 2,
    QMI_UIM_CARD_APPLICATION_TYPE_RUIM    = 3,
    QMI_UIM_CARD_APPLICATION_TYPE_CSIM    = 4,
    QMI_UIM_CARD_APPLICATION_TYPE_ISIM    = 5,
} QmiUimCardApplicationType;

/**
 * QmiUimCardApplicationState:
 * @QMI_UIM_CARD_APPLICATION_STATE_UNKNOWN: Unknown.
 * @QMI_UIM_CARD_APPLICATION_STATE_DETECTED: Detected.
 * @QMI_UIM_CARD_APPLICATION_STATE_PIN1_OR_UPIN_PIN_REQUIRED: PIN1 or UPIN PIN required.
 * @QMI_UIM_CARD_APPLICATION_STATE_PUK1_OR_UPIN_PUK_REQUIRED: PUK1 or UPIN PUK required.
 * @QMI_UIM_CARD_APPLICATION_STATE_CHECK_PERSONALIZATION_STATE: Personalization state must be checked.
 * @QMI_UIM_CARD_APPLICATION_STATE_PIN1_BLOCKED: PIN1 blocked.
 * @QMI_UIM_CARD_APPLICATION_STATE_ILLEGAL: Illegal.
 * @QMI_UIM_CARD_APPLICATION_STATE_READY: Ready
 *
 * Card application state.
 */
typedef enum {
    QMI_UIM_CARD_APPLICATION_STATE_UNKNOWN                     = 0,
    QMI_UIM_CARD_APPLICATION_STATE_DETECTED                    = 1,
    QMI_UIM_CARD_APPLICATION_STATE_PIN1_OR_UPIN_PIN_REQUIRED   = 2,
    QMI_UIM_CARD_APPLICATION_STATE_PUK1_OR_UPIN_PUK_REQUIRED   = 3,
    QMI_UIM_CARD_APPLICATION_STATE_CHECK_PERSONALIZATION_STATE = 4,
    QMI_UIM_CARD_APPLICATION_STATE_PIN1_BLOCKED                = 5,
    QMI_UIM_CARD_APPLICATION_STATE_ILLEGAL                     = 6,
    QMI_UIM_CARD_APPLICATION_STATE_READY                       = 7,
} QmiUimCardApplicationState;

/**
 * QmiUimCardApplicationPersonalizationState:
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_UNKNOWN: Unknown.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_IN_PROGRESS: Operation in progress.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_READY: Ready.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_CODE_REQUIRED: Code required.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_PUK_CODE_REQUIRED: PUK code required.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_PERMANENTLY_BLOCKED: Permanently blocked-
 *
 * Card application personalization state.
 */
typedef enum {
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_UNKNOWN             = 0,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_IN_PROGRESS         = 1,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_READY               = 2,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_CODE_REQUIRED       = 3,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_PUK_CODE_REQUIRED   = 4,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_PERMANENTLY_BLOCKED = 5,
} QmiUimCardApplicationPersonalizationState;

/**
 * QmiUimCardApplicationPersonalizationFeature:
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_GW_NETWORK: GW network.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_GW_NETWORK_SUBSET: GW network subset.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_GW_SERVICE_PROVIDER: GW service provider.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_GW_CORPORATE: GW corporate.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_GW_UIM: UIM.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_NETWORK_TYPE_1: 1X network type 1.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_NETWORK_TYPE_2: 1X network type 2.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_HRPD: 1X HRPD.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_SERVICE_PROVIDER: 1X service provider.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_CORPORATE: 1X corporate.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_RUIM: 1X R-UIM.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_UNKNOWN: Unknown.
 *
 * Card application personalization feature, when a code is required.
 */
typedef enum {
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_GW_NETWORK          = 0,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_GW_NETWORK_SUBSET   = 1,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_GW_SERVICE_PROVIDER = 2,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_GW_CORPORATE        = 3,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_GW_UIM              = 4,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_NETWORK_TYPE_1   = 5,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_NETWORK_TYPE_2   = 6,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_HRPD             = 7,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_SERVICE_PROVIDER = 8,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_CORPORATE        = 9,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_RUIM             = 10,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_UNKNOWN             = 11
} QmiUimCardApplicationPersonalizationFeature;

#endif /* _LIBQMI_GLIB_QMI_ENUMS_UIM_H_ */
