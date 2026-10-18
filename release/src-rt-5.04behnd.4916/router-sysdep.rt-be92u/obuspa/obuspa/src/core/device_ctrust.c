/*
 *
 * Copyright (C) 2022-2024, Broadband Forum
 * Copyright (C) 2017-2024  CommScope, Inc
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \file device_ctrust.c
 *
 * Implements the data model objects associated with controller trust
 *
 */

#include "vendor_defs.h"  // for REMOVE_DEVICE_SECURITY

#include <time.h>
#include <string.h>

#ifndef REMOVE_DEVICE_SECURITY
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/safestack.h>
#endif

#include "common_defs.h"
#include "msg_handler.h"
#include "data_model.h"
#include "usp_api.h"
#include "device.h"
#include "dm_access.h"
#include "vendor_api.h"
#include "iso8601.h"
#include "text_utils.h"
#include "dm_inst_vector.h"
#include "database.h"

//------------------------------------------------------------------------------
// Location of the controller trust tables within the data model
#define DEVICE_CTRUST_ROOT "Device.LocalAgent.ControllerTrust"
#define DEVICE_ROLE_ROOT "Device.LocalAgent.ControllerTrust.Role.{i}"
#define DEVICE_PERMISSION_ROOT "Device.LocalAgent.ControllerTrust.Role.{i}.Permission.{i}"
#define DEVICE_CREDENTIAL_ROOT "Device.LocalAgent.ControllerTrust.Credential.{i}"
#define DEVICE_CHALLENGE_ROOT "Device.LocalAgent.ControllerTrust.Challenge.{i}"

static char *device_role_root = "Device.LocalAgent.ControllerTrust.Role";

//------------------------------------------------------------------------------
// Structure of an entry in the permissions table in the linked list of a role
typedef struct
{
    double_link_t link;   // Doubly linked list pointers. These must always be first in this structure
    int instance;         // Instance number of the permission Device.LocalAgent.ControllerTrust.Role.{i}.Permission.{i}
    bool enable;          // Device.LocalAgent.ControllerTrust.Role.{i}.Permission.{i}.Enable
    unsigned order;       // Device.LocalAgent.ControllerTrust.Role.{i}.Permission.{i}.Order. Higher order permissions override lower order permissions
    str_vector_t targets; // vector of data model paths to apply the permissions to
    unsigned short permission_bitmask;  // Bitmask of permissions eg PERMIT_GET
} permission_t;

//------------------------------------------------------------------------------
// Array of roles
// NOTE: The index number in this array is the same as the index number in node->permissions[]
typedef struct
{
    int instance;                       // Instance number of the role in Device.LocalAgent.ControllerTrust.Role.{i} or INVALID if this array entry is not used
    bool enable;                        // Device.LocalAgent.ControllerTrust.Role.{i}.Enable. Set to false if the role does not exist in the USP DB or is disabled
    double_linked_list_t permissions;   // Linked list of permissions associated with the role (arranged from low order to high order)
    char *name;                         // Name of the role (this must be unique)
} role_t;

role_t roles[MAX_CTRUST_ROLES];

//------------------------------------------------------------------------------
// Vector containing the instance numbers in Device.LocalAgent.ControllerTrust.Role.{i} to update with the new configuration
// (from the role[] data structure) when the sync timer fires
int_vector_t roles_to_update = { 0 };

//------------------------------------------------------------------------------
// Structure for Credential table
typedef struct
{
    int instance;           // instance number in the credentials table Device.LocalAgent.ControlleTrust.Credential.{i}
                            // NOTE: This instnace number is the same as cert_instance below.
                            //       This ensures that the credential table doesn't change every reboot if the order of populating certificates in the certificate table changes

    int role_instance;      // instance number of the role in Device.LocalAgent.ControllerTrust.Role.{i}
    int cert_instance;      // instance number of the certificate in Device.LocalAgent.Certificate.{i} table
} credential_t;

// Vector containing credential table entries
static int num_credentials = 0;
static credential_t *credentials = NULL;

#ifndef REMOVE_DEVICE_SECURITY
//------------------------------------------------------------------------------
// Variable containing the count of request challenge messages
unsigned request_challenge_count = 0;

//------------------------------------------------------------------------------------
// RequestChallenge() command parameters
static char *request_challenge_input_args[] =
{
    "ChallengeRef",
    "RequestExpiration",
};

static char *request_challenge_output_args[] =
{
    "Instruction",
    "InstructionType",
    "ValueType",
    "ChallengeID",
};

//------------------------------------------------------------------------------------
// ChallengeResponse() command parameters
static char *challenge_response_input_args[] =
{
    "ChallengeID",
    "Value",
};

//------------------------------------------------------------------------------------
// Controller Challenge structure
// When a controller issues a RequestChallenge() command, this structure stores the state of the challenge in the controller_challenges[] array
typedef struct
{
    char *controller_endpoint_id; // endpoint id of the controller that initiated this RequestChallenge()
                                  // or NULL if the entry in controller_challenges[] is not active
    char *challenge_id;           // Generated ChallengeID identifying this active RequestChallenge()

    int expiration;               // Number of seconds before this RequestChallenge() expires
    time_t expire_time;           // absolute time that this RequestChallenge() expires

    char *challenge_ref;          // Data model path identifying the instance in the Device.LocalAgent.ControllerTrust.Challenge.{i} table
                                  // This instance contains the password that the controller needs to provide in the ChallengeResponse() command
} controller_challenge_t;

typedef struct
{
    unsigned retries;             // number of times that the password provided by the controller was wrong
    time_t locked_time;           // Absolute time at which the lockout period expires, or 0 if not currently in the lockout period
} challenge_table_t;

static controller_challenge_t controller_challenges[MAX_CONTROLLERS];

static challenge_table_t *challenge_table = NULL;

//------------------------------------------------------------------------------
// Challenge mechanism constants that are currently supported
#define CHALLENGE_TYPE "Passphrase"
#define CHALLENGE_VALUE_TYPE "text/plain"
#define CHALLENGE_INSTRUCTION_TYPE "text/plain"

#define DEFAULT_REQUEST_EXPIRATION 900 // in seconds

#endif // REMOVE_DEVICE_SECURITY
//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void AddInternalRolePermission(role_t *role, unsigned order, char *path, unsigned permission_bitmask);
role_t *Process_CTrustRoleAdded(int role_instance);
int Process_CTrustPermAdded(role_t *role, int perm_instance);
int ExtractPermissions(role_t *role, permission_t *perm, char *param_name, unsigned short read_perm, unsigned short write_perm, unsigned short exec_perm, unsigned short notify_perm);
void AddCTrustPermission(double_linked_list_t *list, permission_t *new_entry);
void ApplyAllPermissionsForRole(role_t *role);
int Notify_CTrustRoleAdded(dm_req_t *req);
int Notify_CTrustRoleDeleted(dm_req_t *req);
int Notify_CTrustPermAdded(dm_req_t *req);
int Notify_CTrustPermDeleted(dm_req_t *req);
int ValidateAdd_CTrustRole(dm_req_t *req);
int Validate_CTrustRoleName(dm_req_t *req, char *value);
int Validate_CTrustPermOrder(dm_req_t *req, char *value);
int Validate_CTrustPermTargets(dm_req_t *req, char *value);
int Validate_CTrustPermString(dm_req_t *req, char *value);
int Notify_CTrustRoleEnable(dm_req_t *req, char *value);
int Notify_CTrustRoleName(dm_req_t *req, char *value);
int Notify_CTrustPermEnable(dm_req_t *req, char *value);
int Notify_CTrustPermOrder(dm_req_t *req, char *value);
int Notify_CTrustPermTargets(dm_req_t *req, char *value);
int Notify_CTrustPermParam(dm_req_t *req, char *value);
int Notify_CTrustPermObj(dm_req_t *req, char *value);
int Notify_CTrustPermInstObj(dm_req_t *req, char *value);
int Notify_CTrustPermCmdEvent(dm_req_t *req, char *value);
void FreeRole(role_t *role);
void FreePermission(role_t *role, permission_t *perm);
int ModifyCTrustPermissionNibbleFromReq(dm_req_t *req, char *value, unsigned short read_perm, unsigned short write_perm, unsigned short exec_perm, unsigned short notify_perm);
void ModifyCTrustPermissionNibble(permission_t *perm, char *value, unsigned short read_perm, unsigned short write_perm, unsigned short exec_perm, unsigned short notify_perm);
void ApplyModifiedPermissions(int id);
void ScheduleRolePermissionsUpdate(int instance);
permission_t *FindPermissionByInstance(role_t *role, int instance);
int ValidatePermTargetsVector(str_vector_t *targets);
int ValidatePermOrderUnique(role_t *role, int order, int instance);
int ValidateRoleNameUnique(char *name, int instance);
role_t *FindUnusedRole(void);
role_t *FindRoleByInstance(int instance);
credential_t *FindCredentialByInstance(int instance);
credential_t *FindCredentialByCertInstance(int cert_instance);
int Get_CredentialRole(dm_req_t *req, char *buf, int len);
int Get_CredentialCertificate(dm_req_t *req, char *buf, int len);
int Get_CredentialNumEntries(dm_req_t *req, char *buf, int len);

#ifndef REMOVE_DEVICE_SECURITY
int InitChallengeTable();
void DestroyControllerChallenge(controller_challenge_t *controller_challenge);
int FindAvailableControllerChallenge(char *controller_endpoint_id, char *challenge_ref, controller_challenge_t **cci);
controller_challenge_t *FindControllerChallengeByEndpointId(char *controller_endpoint_id);
int ControllerTrustRequestChallenge(dm_req_t *req, char *command_key, kv_vector_t *input_args, kv_vector_t *output_args);
char *GenerateChallengeId(char *challenge_id, int len);
int Validate_ChallengeRole(dm_req_t *req, char *value);
int Validate_ChallengeType(dm_req_t *req, char *value);
int Validate_ChallengeValueType(dm_req_t *req, char *value);
int Validate_ChallengeInstructionType(dm_req_t *req, char *value);
int ControllerTrustChallengeResponse(dm_req_t *req, char *command_key, kv_vector_t *input_args, kv_vector_t *output_args);
#endif

/*********************************************************************//**
**
** DEVICE_CTRUST_Init
**
** Initialises this component, and registers all parameters which it implements
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_CTRUST_Init(void)
{
    int err = USP_ERR_OK;
    int i;

    // Mark all roles as unused
    memset(roles, 0, sizeof(roles));
    for (i=0; i<NUM_ELEM(roles); i++)
    {
        roles[i].instance = INVALID;
    }

#ifndef REMOVE_DEVICE_SECURITY
    memset(controller_challenges, 0, sizeof(controller_challenges));
#endif

    // Create a timer which will be used to apply all modified permissions to the data model, after processing a USP Message
    SYNC_TIMER_Add(ApplyModifiedPermissions, 0, END_OF_TIME);

    // Register parameters implemented by this component
    // Device.LocalAgent.ControllerTrust.Role.{i}
    err |= USP_REGISTER_Object(DEVICE_ROLE_ROOT, ValidateAdd_CTrustRole, NULL, Notify_CTrustRoleAdded,
                                                 NULL, NULL, Notify_CTrustRoleDeleted);
    err |= USP_REGISTER_Param_NumEntries(DEVICE_CTRUST_ROOT ".RoleNumberOfEntries", DEVICE_ROLE_ROOT);

    err |= USP_REGISTER_DBParam_Alias(DEVICE_ROLE_ROOT ".Alias", NULL);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_ROLE_ROOT ".Enable", "false", DM_ACCESS_ValidateBool, Notify_CTrustRoleEnable, DM_BOOL);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_ROLE_ROOT ".Name", "", Validate_CTrustRoleName, Notify_CTrustRoleName, DM_STRING);

    char *role_unique_keys[]  = { "Name" };
    err |= USP_REGISTER_Object_UniqueKey(DEVICE_ROLE_ROOT, role_unique_keys, NUM_ELEM(role_unique_keys));

    // Device.LocalAgent.ControllerTrust.Role.{i}.Permission.{i}
    err |= USP_REGISTER_Object(DEVICE_PERMISSION_ROOT, NULL, NULL, Notify_CTrustPermAdded,
                                                       NULL, NULL, Notify_CTrustPermDeleted);
    err |= USP_REGISTER_Param_NumEntries(DEVICE_ROLE_ROOT ".PermissionNumberOfEntries", DEVICE_PERMISSION_ROOT);

    err |= USP_REGISTER_DBParam_Alias(DEVICE_PERMISSION_ROOT ".Alias", NULL);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_PERMISSION_ROOT ".Enable", "false", DM_ACCESS_ValidateBool, Notify_CTrustPermEnable, DM_BOOL);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_PERMISSION_ROOT ".Order", "0", Validate_CTrustPermOrder, Notify_CTrustPermOrder, DM_UINT);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_PERMISSION_ROOT ".Targets", "", Validate_CTrustPermTargets, Notify_CTrustPermTargets, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_PERMISSION_ROOT ".Param", "----", Validate_CTrustPermString, Notify_CTrustPermParam, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_PERMISSION_ROOT ".Obj", "----", Validate_CTrustPermString, Notify_CTrustPermObj, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_PERMISSION_ROOT ".InstantiatedObj", "----", Validate_CTrustPermString, Notify_CTrustPermInstObj, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_PERMISSION_ROOT ".CommandEvent", "----", Validate_CTrustPermString, Notify_CTrustPermCmdEvent, DM_STRING);

    char *alias_unique_key[] = { "Alias" };
    char *perm_unique_keys[]  = { "Order" };
    err |= USP_REGISTER_Object_UniqueKey(DEVICE_PERMISSION_ROOT, alias_unique_key, NUM_ELEM(alias_unique_key));
    err |= USP_REGISTER_Object_UniqueKey(DEVICE_PERMISSION_ROOT, perm_unique_keys, NUM_ELEM(perm_unique_keys));

    // Device.LocalAgent.ControllerTrust.Credential.{i}
    err |= USP_REGISTER_Object(DEVICE_CREDENTIAL_ROOT, USP_HOOK_DenyAddInstance, NULL, NULL,   // This table is read only
                                                       USP_HOOK_DenyDeleteInstance, NULL, NULL);

    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_CREDENTIAL_ROOT ".Alias", DM_ACCESS_PopulateAliasParam, DM_STRING);
    err |= USP_REGISTER_Param_Constant(DEVICE_CREDENTIAL_ROOT ".Enable", "true", DM_BOOL);
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_CREDENTIAL_ROOT ".Role", Get_CredentialRole, DM_STRING);
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_CREDENTIAL_ROOT ".Credential", Get_CredentialCertificate, DM_STRING);
    err |= USP_REGISTER_Param_Constant(DEVICE_CREDENTIAL_ROOT ".AllowedUses", "MTP-and-broker", DM_STRING);

    char *cred_unique_keys[]  = { "Credential" };
    err |= USP_REGISTER_Object_UniqueKey(DEVICE_CREDENTIAL_ROOT, alias_unique_key, NUM_ELEM(alias_unique_key));
    err |= USP_REGISTER_Object_UniqueKey(DEVICE_CREDENTIAL_ROOT, cred_unique_keys, NUM_ELEM(cred_unique_keys));
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_CTRUST_ROOT ".CredentialNumberOfEntries", Get_CredentialNumEntries, DM_UINT);

#ifndef REMOVE_DEVICE_SECURITY
    // Device.LocalAgent.ControllerTrust.Challenge.{i}
    err |= USP_REGISTER_Object(DEVICE_CHALLENGE_ROOT, NULL, NULL, NULL, NULL, NULL, NULL);
    err |= USP_REGISTER_DBParam_Alias(DEVICE_CHALLENGE_ROOT ".Alias", NULL);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CHALLENGE_ROOT ".Description", "", NULL, NULL, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CHALLENGE_ROOT ".Role", "", Validate_ChallengeRole, NULL, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CHALLENGE_ROOT ".Enable", "false", NULL, NULL, DM_BOOL);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CHALLENGE_ROOT ".Type", CHALLENGE_TYPE, Validate_ChallengeType, NULL, DM_STRING);
    err |= USP_REGISTER_DBParam_SecureWithType(DEVICE_CHALLENGE_ROOT ".Value", "", DM_ACCESS_ValidateBase64, NULL, DM_BASE64);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CHALLENGE_ROOT ".ValueType", CHALLENGE_VALUE_TYPE, Validate_ChallengeValueType, NULL, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CHALLENGE_ROOT ".Instruction", "", DM_ACCESS_ValidateBase64, NULL, DM_BASE64);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CHALLENGE_ROOT ".InstructionType", CHALLENGE_INSTRUCTION_TYPE, Validate_ChallengeInstructionType, NULL, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CHALLENGE_ROOT ".Retries", "", NULL, NULL, DM_UINT);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CHALLENGE_ROOT ".LockoutPeriod", "30", NULL, NULL, DM_INT);
    err |= USP_REGISTER_Param_NumEntries(DEVICE_CTRUST_ROOT ".ChallengeNumberOfEntries", DEVICE_CHALLENGE_ROOT);

    // Device.LocalAgent.ControllerTrust.RequestChallenge() command
    err |= USP_REGISTER_SyncOperation(DEVICE_CTRUST_ROOT ".RequestChallenge()", ControllerTrustRequestChallenge);
    err |= USP_REGISTER_OperationArguments(DEVICE_CTRUST_ROOT ".RequestChallenge()",
                        request_challenge_input_args, NUM_ELEM(request_challenge_input_args),
                        request_challenge_output_args, NUM_ELEM(request_challenge_output_args));

    // Device.LocalAgent.ControllerTrust.ChallengeResponse() command
    err |= USP_REGISTER_SyncOperation(DEVICE_CTRUST_ROOT ".ChallengeResponse()", ControllerTrustChallengeResponse);
    err |= USP_REGISTER_OperationArguments(DEVICE_CTRUST_ROOT ".ChallengeResponse()",
                        challenge_response_input_args, NUM_ELEM(challenge_response_input_args),
                        NULL, 0);
#endif
    // Exit if any errors occurred
    if (err != USP_ERR_OK)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // If the code gets here, then registration was successful
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DEVICE_CTRUST_Start
**
** Starts this component, adding all instances to the data model
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_CTRUST_Start(void)
{
    int i;
    int err;
    int_vector_t iv;
    int instance;
    char path[MAX_DM_PATH];
    role_t *role;

    // Exit if unable to get the object instance numbers present in the role table
    INT_VECTOR_Init(&iv);
    err = DATA_MODEL_GetInstances(device_role_root, &iv);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Add all role instances (and associated permissions)
    for (i=0; i < iv.num_entries; i++)
    {
        // Exit if unable to delete a Role connection with bad parameters from the DB
        instance = iv.vector[i];
        role = Process_CTrustRoleAdded(instance);
        if (role == NULL)
        {
            USP_SNPRINTF(path, sizeof(path), "%s.%d", device_role_root, instance);
            USP_LOG_Warning("%s: Deleting %s as it contained invalid parameters.", __FUNCTION__, path);
            DATA_MODEL_DeleteInstance(path, 0);
            err = USP_ERR_INTERNAL_ERROR;
            goto exit;
        }

        // Apply the permissions for this role to the data model nodes
        ApplyAllPermissionsForRole(role);
    }

#ifndef REMOVE_DEVICE_SECURITY
    // Init array associated with RequestChallenge/ChallengeResponse
    err = InitChallengeTable();
    if (err != USP_ERR_OK)
    {
        goto exit;
    }
#endif

exit:
    INT_VECTOR_Destroy(&iv);
    return err;
}

/*********************************************************************//**
**
** DEVICE_CTRUST_Stop
**
** Frees all memory used by this component
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void DEVICE_CTRUST_Stop(void)
{
    int i;
    role_t *role;

    // Free all roles and their associated permissions
    for (i=0; i<NUM_ELEM(roles); i++)
    {
        role = &roles[i];
        FreeRole(role);
    }

    // Free all credentials
    USP_SAFE_FREE(credentials);

#ifndef REMOVE_DEVICE_SECURITY
{
    controller_challenge_t *cc;

    // Free all controller challenges
    for (i=0; i<NUM_ELEM(controller_challenges); i++)
    {
        cc = &controller_challenges[i];
        DestroyControllerChallenge(cc);
    }

    // Free challenge_table
    USP_SAFE_FREE(challenge_table);
}
#endif

}

/*********************************************************************//**
**
** DEVICE_CTRUST_AddCertRole
**
** Adds a reference to a certificate and its associated role
** This function is called at startup when the Trust Store certificates are registered
**
** \param   cert_instance - instance number of the certificate in Device.LocalAgent.Certificate.{i} table
** \param   role_instance - instance number in Device.LocalAgent.ControllerTrust.Role.{i}
** \param   signal_event - Set to true, if the Agent should signal the object creation (set to false when called at startup)
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_CTRUST_AddCertRole(int cert_instance, int role_instance, bool signal_event)
{
    int err;
    int new_num_entries;
    credential_t *cp;
    char path[MAX_DM_PATH];

    // Exit if the credential already exists
    // NOTE: This should never happen, as the cert and credentials tables stay in step with one another
    cp = FindCredentialByCertInstance(cert_instance);
    if (cp != NULL)
    {
        USP_LOG_Error("%s: Cannot add credential referencing LocalAgent.Certificate.%d (already present at ControllerTrust.Credential.%d)", __FUNCTION__, cert_instance, cp->cert_instance);
        return USP_ERR_INTERNAL_ERROR;
    }

    // First increase the size of the vector
    new_num_entries = num_credentials + 1;
    credentials = USP_REALLOC(credentials, new_num_entries*sizeof(credential_t));

    // Fill in the new entry
    // NOTE: The instance number in the credentials table is the same as the instance number in the certificate table
    //       This ensures that the credential table doesn't change every reboot if the order of populating the certificate table changes
    cp = &credentials[ num_credentials ];
    cp->instance = cert_instance;
    cp->role_instance = role_instance;
    cp->cert_instance = cert_instance;
    num_credentials = new_num_entries;

    // Exit if unable to add credential instance into the data model
    USP_SNPRINTF(path, sizeof(path), "Device.LocalAgent.ControllerTrust.Credential.%d", cert_instance);
    if (signal_event)
    {
        err = USP_SIGNAL_ObjectAdded(path);
    }
    else
    {
        err = DATA_MODEL_InformInstance(path);
    }

    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}


/*********************************************************************//**
**
** DEVICE_CTRUST_GetCertInheritedRole
**
** Gets the instance number of the inherited role associated with the specified certificate
**
** \param   cert_instance - Instance number of certificate in Device.LocalAgent.Certificate.{i}
**                          that we want to find the registered role for
**
** \return  Instance number in Device.LocalAgent.ControllerTrust.Role.{i} associated with the certificate, or INVALID, if no matching role found
**
**************************************************************************/
int DEVICE_CTRUST_GetCertInheritedRole(int cert_instance)
{
    int i;
    credential_t *cp;

    // Iterate over all entries in the Credentials table
    for (i=0; i<num_credentials; i++)
    {
        // Exit if we've found a matching certificate
        cp = &credentials[i];
        if (cp->cert_instance == cert_instance)
        {
            return cp->role_instance;
        }
    }

    // If the code gets here, then no match was found
    // NOTE: This should never happen, as we ensure that all certificates in the trust store have an associated role
    return INVALID;
}

/*********************************************************************//**
**
** DEVICE_CTRUST_RoleInstanceToIndex
**
** Gets the index (in roles[]) of the specified instance number in Device.LocalAgent.ControllerTrust.Role.{i}
**
** \param   role_instance - instance number in Device.LocalAgent.ControllerTrust.Role.{i} to get the index of
**
** \return  index of the specified instance in roles[] or INVALID if the instance number was invalid
**
**************************************************************************/
int DEVICE_CTRUST_RoleInstanceToIndex(int role_instance)
{
    int i;
    role_t *role;

    // Iterate over the roles[] array, finding the matching entry
    for (i=0; i<NUM_ELEM(roles); i++)
    {
        role = &roles[i];
        if ((role->instance == role_instance) && (role->instance != INVALID))
        {
            return i;
        }
    }

    // If the code gets here, no matching entry was found
    return INVALID;
}

/*********************************************************************//**
**
** DEVICE_CTRUST_RoleIndexToInstance
**
** Gets the instance number of role in Device.LocalAgent.ControllerTust.Role.{i} based on index (in roles[])
**
** \param   role_index - index of role in roles[] to get the instance number of
**
** \return  instance number of the specified role, or INVALID if the entry in roles[] is not used
**
**************************************************************************/
int DEVICE_CTRUST_RoleIndexToInstance(int role_index)
{
    role_t *role;

    // Exit if role_index is out of range
    if ((role_index < 0) || (role_index >= MAX_CTRUST_ROLES))
    {
        return INVALID;
    }

    // Exit if entry in roles[] is unused
    role = &roles[role_index];
    if (role->instance == INVALID)
    {
        return INVALID;
    }

    return role->instance;
}

/*********************************************************************//**
**
** DEVICE_CTRUST_SetRoleParameter
**
** Ensures that the database contains the specified child parameter of the Role table with the specified value
**
** \param   instance - instance number in Device.LocalAgent.ControllerTrust.Role.{i}
** \param   param_name - name of parameter in the Role table to set
** \param   new_value - value to set the specified parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_CTRUST_SetRoleParameter(int instance, char *param_name, char *new_value)
{
    int err;
    char path[MAX_DM_PATH];
    dm_node_t *node;
    char instance_str[10];
    char cur_value[MAX_DM_SHORT_VALUE_LEN];

    // Form all arguments to pass to the database functions
    USP_SNPRINTF(instance_str, sizeof(instance_str), "%d", instance);
    USP_SNPRINTF(path, sizeof(path), "%s.%d.%s", device_role_root, instance, param_name);
    node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, 0);
    USP_ASSERT(node != NULL);

    // Write the new value, if the parameter didn't exist in the DB or was a different value
    err = DATABASE_GetParameterValue(path, node->hash, instance_str, cur_value, sizeof(cur_value), 0);
    if ((err != USP_ERR_OK) || (strcmp(cur_value, new_value) != 0))
    {
        err = DATABASE_SetParameterValue(path, node->hash, instance_str, new_value, 0);
    }

    return err;
}

/*********************************************************************//**
**
** DEVICE_CTRUST_SetPermissionParameter
**
** Ensures that the database contains the specified child parameter of the Permission table with the specified value
**
** \param   instance1 - instance number in Device.LocalAgent.ControllerTrust.Role.{i}
** \param   instance2 - instance number in Device.LocalAgent.ControllerTrust.Role.{i}.Permission.{i}
** \param   param_name - name of parameter in the Permission table to set
** \param   new_value - value to set the specified parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_CTRUST_SetPermissionParameter(int instance1, int instance2, char *param_name, char *new_value)
{
    int err;
    char path[MAX_DM_PATH];
    dm_node_t *node;
    char instance_str[10];
    char cur_value[MAX_DM_SHORT_VALUE_LEN];

    // Form all arguments to pass to the database functions
    USP_SNPRINTF(instance_str, sizeof(instance_str), "%d.%d", instance1, instance2);
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Permission.%d.%s", device_role_root, instance1, instance2, param_name);
    node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, 0);
    USP_ASSERT(node != NULL);

    // Write the new value, if the parameter didn't exist in the DB or was a different value
    err = DATABASE_GetParameterValue(path, node->hash, instance_str, cur_value, sizeof(cur_value), 0);
    if ((err != USP_ERR_OK) || (strcmp(cur_value, new_value) != 0))
    {
        err = DATABASE_SetParameterValue(path, node->hash, instance_str, new_value, 0);
    }

    return err;
}

/*********************************************************************//**
**
** DEVICE_CTRUST_ApplyPermissionsToSubTree
**
** Called to apply permissions to a sub tree of the data model
** This function is typically called to apply permissions to parts of the data model owned by a USP Service
**
** \param   path - data model path of the sub-tree of the data model to apply all permissions (for all roles to)
**                 NOTE: This path must exist in the data model
**
** \return  None
**
**************************************************************************/
void DEVICE_CTRUST_ApplyPermissionsToSubTree(char *path)
{
    int i, j;
    role_t *role;
    permission_t *perm;
    dm_node_t *node;
    dm_node_t *perm_node;
    char *perm_path;

    node =  DM_PRIV_GetNodeFromPath(path, NULL, NULL, DONT_LOG_ERRORS);
    USP_ASSERT(node != NULL);

    // Iterate over all roles
    for (i=0; i<NUM_ELEM(roles); i++)
    {
        role = &roles[i];
        if ((role->instance != INVALID) && (role->enable))
        {
            // Iterate over all permissions for this role
            perm = (permission_t *) role->permissions.head;
            while (perm != NULL)
            {
                if (perm->enable)
                {
                    // Iterate over all targets for this permission
                    for (j=0; j < perm->targets.num_entries; j++)
                    {
                        perm_path = perm->targets.vector[j];
                        perm_node =  DM_PRIV_GetNodeFromPath(perm_path, NULL, NULL, DONT_LOG_ERRORS);
                        if (perm_node != NULL)  // Node maybe NULL if it relates to a USP Service that hasn't registered yet
                        {
                            if ((perm_node == node) || (DM_PRIV_IsChildNodeOf(node, perm_node)))
                            {
                                // Case of permission applies to whole of specified subtree
                                DM_PRIV_ApplyPermissions(node, i, perm->permission_bitmask);
                            }
                            else if (DM_PRIV_IsChildNodeOf(perm_node, node))
                            {
                                // Case of permission applies within the subtree
                                DM_PRIV_ApplyPermissions(perm_node, i, perm->permission_bitmask);
                            }
                            // NOTE: if neither of these cases apply, then the permission applies outside of the specified subtree, so there's nothing more to do
                        }
                    }
                }

                perm = (permission_t *) perm->link.next;
            }
        }
    }
}

/*********************************************************************//**
**
** ValidateAdd_CTrustRole
**
** Function called to determin e whether it is possible to add another instance to the role table
**
** \param   req - pointer to structure identifying the request
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ValidateAdd_CTrustRole(dm_req_t *req)
{
    role_t *role;

    // Exit if unable to add any more roles
    role = FindUnusedRole();
    if (role == NULL)
    {
        USP_ERR_SetMessage("%s: Unable to add any more roles. Increase MAX_CTRUST_ROLES from %d", __FUNCTION__, MAX_CTRUST_ROLES);
        return USP_ERR_RESOURCES_EXCEEDED;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_CTrustRoleAdded
**
** Function called when an instance has been added to Device.LocalAgent.ControllerTrust.Role.{i}
**
** \param   req - pointer to structure identifying the request
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_CTrustRoleAdded(dm_req_t *req)
{
    Process_CTrustRoleAdded(inst1);
    ScheduleRolePermissionsUpdate(inst1);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_CTrustRoleDeleted
**
** Function called when an instance has been deleted from Device.LocalAgent.ControllerTrust.Role.{i}
**
** \param   req - pointer to structure identifying the request
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_CTrustRoleDeleted(dm_req_t *req)
{
    role_t *role;

    role = FindRoleByInstance(inst1);
    USP_ASSERT(role != NULL);

    FreeRole(role);

    // NOTE: There is no need to schedule the permissions being updated, since as this role is deleted, no permissions will be granted

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_CTrustPermAdded
**
** Function called when an instance has been added to Device.LocalAgent.ControllerTrust.Role.{i}.Permission.{i}
**
** \param   req - pointer to structure identifying the request
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_CTrustPermAdded(dm_req_t *req)
{
    role_t *role;

    role = FindRoleByInstance(inst1);
    USP_ASSERT(role != NULL);

    Process_CTrustPermAdded(role, inst2);

    ScheduleRolePermissionsUpdate(inst1);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_CTrustPermDeleted
**
** Function called when an instance has been deleted from Device.LocalAgent.ControllerTrust.Role.{i}.Permission.{i}
**
** \param   req - pointer to structure identifying the request
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_CTrustPermDeleted(dm_req_t *req)
{
    role_t *role;
    permission_t *perm;

    // Exit if the role for this permission has already been deleted
    role = FindRoleByInstance(inst1);
    if (role == NULL)
    {
        return USP_ERR_OK;
    }

    perm = FindPermissionByInstance(role, inst2);
    USP_ASSERT(perm != NULL);

    FreePermission(role, perm);

    ScheduleRolePermissionsUpdate(inst1);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_CTrustRoleName
**
** Validates Device.LocalAgent.ControllerTrust.Role.{i}.Name
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_CTrustRoleName(dm_req_t *req, char *value)
{
    return ValidateRoleNameUnique(value, inst1);
}

/*********************************************************************//**
**
** Validate_CTrustPermOrder
**
** Validates Device.LocalAgent.ControllerTrust.Role.{i}.Permission.{i}.Order
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_CTrustPermOrder(dm_req_t *req, char *value)
{
    role_t *role;

    role = FindRoleByInstance(inst1);
    USP_ASSERT(role != NULL);

    return ValidatePermOrderUnique(role, val_uint, inst2);
}

/*********************************************************************//**
**
** Validate_CTrustPermTargets
**
** Validates Device.LocalAgent.ControllerTrust.Role.{i}.Permission.{i}.Targets
**
** \param   req - pointer to structure identifying the parameter (unused)
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_CTrustPermTargets(dm_req_t *req, char *value)
{
    int err;

    // Exit if the permission targets are not a comma separated list with each list item starting 'Device.'
    str_vector_t targets;

    // Split the comma separated list of targets into a vector of targets
    STR_VECTOR_Init(&targets);
    TEXT_UTILS_SplitString(value, &targets, ",");

    err = ValidatePermTargetsVector(&targets);

    STR_VECTOR_Destroy(&targets);

    return err;
}

/*********************************************************************//**
**
** Validate_CTrustPermString
**
** Validates that a permissions string is of the form 'rwxn'
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_CTrustPermString(dm_req_t *req, char *value)
{
    // Exit if the permission string is not formed of 4 characters, or any of the characters is invalid
    if ((strlen(value) != 4) ||
        ((value[0] != 'r') && (value[0] != '-')) ||
        ((value[1] != 'w') && (value[1] != '-')) ||
        ((value[2] != 'x') && (value[2] != '-')) ||
        ((value[3] != 'n') && (value[3] != '-')))
    {
        USP_ERR_SetMessage("%s: Badly formed permission string '%s'", __FUNCTION__, value);
        return USP_ERR_INVALID_ARGUMENTS;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_CTrustRoleEnable
**
** Function called when Device.LocalAgent.ControllerTrust.Role.{i}.Enable has been modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_CTrustRoleEnable(dm_req_t *req, char *value)
{
    role_t *role;

    role = FindRoleByInstance(inst1);
    USP_ASSERT(role != NULL);

    // Exit if no change
    if (role->enable == val_bool)
    {
        goto exit;
    }

    role->enable = val_bool;
    ScheduleRolePermissionsUpdate(inst1);

exit:
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_CTrustRoleName
**
** Function called when Device.LocalAgent.ControllerTrust.Role.{i}.Name has been modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_CTrustRoleName(dm_req_t *req, char *value)
{
    role_t *role;

    role = FindRoleByInstance(inst1);
    USP_ASSERT(role != NULL);

    USP_SAFE_FREE(role->name);
    role->name = USP_STRDUP(value);

    // NOTE: We only maintain role name in the roles[] data structure in order that we can check that it is unique easily
    // Changing the role's name doesn't affect the role's permissions so no need to call ScheduleRolePermissionsUpdate()

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_CTrustPermEnable
**
** Function called when Device.LocalAgent.ControllerTrust.Role.{i}.Permission.{i}.Enable has been modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_CTrustPermEnable(dm_req_t *req, char *value)
{
    role_t *role;
    permission_t *perm;

    role = FindRoleByInstance(inst1);
    USP_ASSERT(role != NULL);

    perm = FindPermissionByInstance(role, inst2);
    USP_ASSERT(perm != NULL);

    // Exit if no change
    if (perm->enable == val_bool)
    {
        goto exit;
    }

    perm->enable = val_bool;
    ScheduleRolePermissionsUpdate(inst1);

exit:
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_CTrustPermOrder
**
** Function called when Device.LocalAgent.ControllerTrust.Role.{i}.Permission.{i}.Order has been modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_CTrustPermOrder(dm_req_t *req, char *value)
{
    role_t *role;
    permission_t *perm;

    role = FindRoleByInstance(inst1);
    USP_ASSERT(role != NULL);

    perm = FindPermissionByInstance(role, inst2);
    USP_ASSERT(perm != NULL);

    // Exit if no change
    if (perm->order == val_uint)
    {
        goto exit;
    }

    // Remove the permission from the list, then add it back in again with it's new order (ie at the right place)
    DLLIST_Unlink(&role->permissions, perm);
    perm->order = val_uint;
    AddCTrustPermission(&role->permissions, perm);

    ScheduleRolePermissionsUpdate(inst1);

exit:
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_CTrustPermTargets
**
** Function called when Device.LocalAgent.ControllerTrust.Role.{i}.Permission.{i}.Targets has been modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_CTrustPermTargets(dm_req_t *req, char *value)
{
    role_t *role;
    permission_t *perm;

    role = FindRoleByInstance(inst1);
    USP_ASSERT(role != NULL);

    perm = FindPermissionByInstance(role, inst2);
    USP_ASSERT(perm != NULL);

    STR_VECTOR_Destroy(&perm->targets);
    TEXT_UTILS_SplitString(value, &perm->targets, ",");

    ScheduleRolePermissionsUpdate(inst1);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_CTrustPermParam
**
** Function called when Device.LocalAgent.ControllerTrust.Role.{i}.Permission.{i}.Param has been modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_CTrustPermParam(dm_req_t *req, char *value)
{
    return ModifyCTrustPermissionNibbleFromReq(req, value, PERMIT_GET, PERMIT_SET, PERMIT_NONE, PERMIT_SUBS_VAL_CHANGE);
}

/*********************************************************************//**
**
** Notify_CTrustPermObj
**
** Function called when Device.LocalAgent.ControllerTrust.Role.{i}.Permission.{i}.Obj has been modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_CTrustPermObj(dm_req_t *req, char *value)
{
    return ModifyCTrustPermissionNibbleFromReq(req, value, PERMIT_OBJ_INFO, PERMIT_ADD, PERMIT_NONE, PERMIT_SUBS_OBJ_ADD);
}

/*********************************************************************//**
**
** Notify_CTrustPermInstObj
**
** Function called when Device.LocalAgent.ControllerTrust.Role.{i}.Permission.{i}.InstantiatedObj has been modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_CTrustPermInstObj(dm_req_t *req, char *value)
{
    return ModifyCTrustPermissionNibbleFromReq(req, value, PERMIT_GET_INST, PERMIT_DEL, PERMIT_NONE, PERMIT_SUBS_OBJ_DEL);
}

/*********************************************************************//**
**
** Notify_CTrustPermCmdEvent
**
** Function called when Device.LocalAgent.ControllerTrust.Role.{i}.Permission.{i}.CommandEvent has been modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_CTrustPermCmdEvent(dm_req_t *req, char *value)
{
    return ModifyCTrustPermissionNibbleFromReq(req, value, PERMIT_CMD_INFO, PERMIT_NONE, PERMIT_OPER, PERMIT_SUBS_EVT_OPER_COMP);
}

/*********************************************************************//**
**
** AddInternalRolePermission
**
** Adds a permission to an internal role
**
** \param   role - role to add the permission to
** \param   order - order of the role to add. Note this must be more than all previous permissions added for this role
** \param   path - data model path that the permission applies to
** \param   permission_bitmask - bitmask of permissions to add
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void AddInternalRolePermission(role_t *role, unsigned order, char *path, unsigned permission_bitmask)
{
    permission_t *perm;

    perm = USP_MALLOC(sizeof(permission_t));
    memset(perm, 0, sizeof(permission_t));
    perm->instance = INVALID;
    perm->enable = true;
    perm->order = order;
    STR_VECTOR_Init(&perm->targets);
    STR_VECTOR_Add(&perm->targets, path);
    perm->permission_bitmask = permission_bitmask;
    DLLIST_LinkToTail(&role->permissions, perm);
}

/*********************************************************************//**
**
** Process_CTrustRoleAdded
**
** Reads a Role instance from Device.LocalAgent.ControllerTrust.Role.{i} into the internal data structure
**
** \param   role_instance - Instance number of the role in Device.LocalAgent.ControllerTrust.Role.{i}
**
** \return  pointer to entry in roles[] just added, or NULL if an error occurred
**
**************************************************************************/
role_t *Process_CTrustRoleAdded(int role_instance)
{
    int i;
    int err;
    role_t *role = NULL;
    char path[MAX_DM_PATH];
    int_vector_t iv;
    int perm_instance;

    // Exit if unable to add any more roles
    role = FindUnusedRole();
    if (role == NULL)
    {
        USP_ERR_SetMessage("%s: Unable to add any more roles. Increase MAX_CTRUST_ROLES from %d", __FUNCTION__, MAX_CTRUST_ROLES);
        return NULL;
    }

    // Initialise role
    INT_VECTOR_Init(&iv);
    memset(role, 0, sizeof(role_t));
    role->instance = role_instance;
    DLLIST_Init(&role->permissions);

    // Exit if unable to get the Name for this Role
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Name", device_role_root, role_instance);
    err = DM_ACCESS_GetString(path, &role->name);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if role name is not unique
    err = ValidateRoleNameUnique(role->name, role_instance);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the Enable for this Role
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Enable", device_role_root, role_instance);
    err = DM_ACCESS_GetBool(path, &role->enable);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the instance numbers of the permissions for this role
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Permission.", device_role_root, role_instance);
    err = DATA_MODEL_GetInstances(path, &iv);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Add all permission instances
    for (i=0; i < iv.num_entries; i++)
    {
        perm_instance = iv.vector[i];
        err = Process_CTrustPermAdded(role, perm_instance);
        if (err != USP_ERR_OK)
        {
            // Delete a Permission instance with bad parameters from the DB
            USP_SNPRINTF(path, sizeof(path), "%s.%d.Permission.%d", device_role_root, role_instance, perm_instance);
            USP_LOG_Warning("%s: Deleting %s as it contained invalid parameters.", __FUNCTION__, path);
            DATA_MODEL_DeleteInstance(path, 0);
        }
    }

    err = USP_ERR_OK;

exit:
    INT_VECTOR_Destroy(&iv);
    if (err != USP_ERR_OK)
    {
        FreeRole(role);
        return NULL;
    }

    return role;
}

/*********************************************************************//**
**
** Process_CTrustPermAdded
**
** Reads a permission instance from Device.LocalAgent.ControllerTrust.Role.{i}.Permission.{i} into the internal data structure
**
** \param   role - pointer to role to add the permission to
** \param   perm_instance - Instance number of the permission in Device.LocalAgent.ControllerTrust.Role.{i}.Permission.{i}
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Process_CTrustPermAdded(role_t *role, int perm_instance)
{
    int err;
    permission_t *perm;
    char path[MAX_DM_PATH];

    // Initialise a new permission
    perm = USP_MALLOC(sizeof(permission_t));
    memset(perm, 0, sizeof(permission_t));
    perm->instance = perm_instance;
    STR_VECTOR_Init(&perm->targets);

    // Exit if unable to get the Enable for this Permission
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Permission.%d.Enable", device_role_root, role->instance, perm_instance);
    err = DM_ACCESS_GetBool(path, &perm->enable);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the Order for this Permission
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Permission.%d.Order", device_role_root, role->instance, perm_instance);
    err = DM_ACCESS_GetUnsigned(path, &perm->order);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if Order is not unique
    err = ValidatePermOrderUnique(role, perm->order, perm_instance);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the Targets for this Permission
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Permission.%d.Targets", device_role_root, role->instance, perm_instance);
    err = DM_ACCESS_GetStringVector(path, &perm->targets);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if the targets are not valid
    err = ValidatePermTargetsVector(&perm->targets);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to extract the permissions associated with this instance
    err  = ExtractPermissions(role, perm, "Param", PERMIT_GET, PERMIT_SET, PERMIT_NONE, PERMIT_SUBS_VAL_CHANGE);
    err |= ExtractPermissions(role, perm, "Obj",   PERMIT_OBJ_INFO, PERMIT_ADD, PERMIT_NONE, PERMIT_SUBS_OBJ_ADD);
    err |= ExtractPermissions(role, perm, "InstantiatedObj", PERMIT_GET_INST, PERMIT_DEL, PERMIT_NONE, PERMIT_SUBS_OBJ_DEL);
    err |= ExtractPermissions(role, perm, "CommandEvent", PERMIT_CMD_INFO, PERMIT_NONE, PERMIT_OPER, PERMIT_SUBS_EVT_OPER_COMP);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

exit:
    if (err == USP_ERR_OK)
    {
        // If successful, add the permission to the role[] entry
        AddCTrustPermission(&role->permissions, perm);
    }
    else
    {
        // Otherwise free the permission structure
        STR_VECTOR_Destroy(&perm->targets);
        USP_FREE(perm);
    }

    return err;
}

/*********************************************************************//**
**
** ValidateRoleNameUnique
**
** Validates that the role's name is unique
**
** \param   name - name to check for uniqueness
** \param   instance - instance number of the role which is being modified with this new name (not included in uniqueness check)
**
** \return  USP_ERR_OK if the targets are valid
**
**************************************************************************/
int ValidateRoleNameUnique(char *name, int instance)
{
    int i;
    role_t *role;

    // Exit if new role name is not unique
    for (i=0; i<NUM_ELEM(roles); i++)
    {
        role = &roles[i];
        if ((role->instance != INVALID) && (role->instance != instance) && (role->name != NULL) && (strcmp(role->name, name)==0))  // NOTE: The (i != inst1) test ensures that you can set the same name for an existing instance without error
        {
            USP_ERR_SetMessage("%s: Name not unique (already present in %s.%d)", __FUNCTION__, device_role_root, role->instance);
            return USP_ERR_INVALID_ARGUMENTS;
        }
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ValidatePermTargetsVector
**
** Validates that the targets in the specified string vector look valid and can be supported by this code
**
** \param   targets - vector of data model paths. Either partial paths or wildcarded paths
**
** \return  USP_ERR_OK if the targets are valid
**
**************************************************************************/
int ValidatePermTargetsVector(str_vector_t *targets)
{
    int i, j;
    str_vector_t dm_elements;
    char *path;
    char *element;
    char first_char;
    char c;
    int err;
    bool is_valid;

    STR_VECTOR_Init(&dm_elements);

    // Exit if there are no targets, this is valid
    if (targets->num_entries == 0)
    {
        err = USP_ERR_OK;
        goto exit;
    }

    // Check all of the targets in the vector
    for (i=0; i < targets->num_entries; i++)
    {
        // Split the target into its data model element constituent parts (separated by '.')
        path = targets->vector[i];
        TEXT_UTILS_SplitString(path, &dm_elements, ".");

        // Only allow partial paths or wildcarded paths, starting with Device.
        for (j=0; j < dm_elements.num_entries; j++)
        {
            // Exit if the path does not start with 'Device.'
            // NOTE: We cannot just test that the path is present in the data model, because it might be a path for part
            // of the data model owned by a USP Service which has not registered yet
            element = dm_elements.vector[j];
            if ((j==0) && (strcmp(element, "Device") != 0))
            {
                USP_ERR_SetMessage("%s: Target '%s' does not start 'Device.'", __FUNCTION__, path);
                err = USP_ERR_INVALID_ARGUMENTS;
                goto exit;
            }

            // Skip if this part of the path is a wildcard
            if (strcmp(element, "*")==0)
            {
                continue;
            }

            // Exit if the path contains a search expression
            if ((strchr(element, '[') != NULL) || (strchr(element, ']') != NULL))
            {
                USP_ERR_SetMessage("%s: Search expressions not supported in Target '%s'", __FUNCTION__, path);
                err = USP_ERR_INVALID_ARGUMENTS;
                goto exit;
            }

            // Exit if the path contains reference following
            if (strchr(element, '+') != NULL)
            {
                USP_ERR_SetMessage("%s: Reference following not supported in Target '%s'", __FUNCTION__, path);
                err = USP_ERR_INVALID_ARGUMENTS;
                goto exit;
            }

            // Exit if the path contains an instance number
            // NOTE: We only check the first character of the path element, since objects and parameters are allowed to contain numbers in them (just not at the start)
            first_char = *element;
            if ((first_char >= '0') && (first_char <= '9'))
            {
                USP_ERR_SetMessage("%s: Instance numbers not supported in Target '%s'", __FUNCTION__, path);
                err = USP_ERR_INVALID_ARGUMENTS;
                goto exit;
            }

            // Exit if the data model element contains any other characters which we weren't expecting in it
            c = *element++;
            while (c != '\0')
            {
                is_valid = IS_ALPHA_NUMERIC(c) || (c == '_') || (c == '-') || (c == '!') || (c == '(') || (c == ')');
                if (is_valid == false)
                {
                    USP_ERR_SetMessage("%s: Target '%s' contains invalid character '%c'", __FUNCTION__, path, c);
                    err = USP_ERR_INVALID_ARGUMENTS;
                    goto exit;
                }
                c = *element++;
            }
        }
        STR_VECTOR_Destroy(&dm_elements);
    }

    err = USP_ERR_OK;

exit:
    STR_VECTOR_Destroy(&dm_elements);
    return err;
}

/*********************************************************************//**
**
** ValidatePermOrderUnique
**
** Validates that the specified permission order is unique for the specified role
**
** \param   role - eq - pointer to structure identifying the parameter
** \param   order - order of the permission to check
** \param   instance - instance number of the permission that is being modified with this new order (not included in uniqueness check)
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ValidatePermOrderUnique(role_t *role, int order, int instance)
{
    permission_t *perm;

    // Exit if order is not unique
    perm = (permission_t *) role->permissions.head;
    while (perm != NULL)
    {
        if ((perm->instance != instance) && (perm->order == order))
        {
            USP_ERR_SetMessage("%s: Order(%d) not unique (already used by %s.%d.Permission.%d)", __FUNCTION__, order, device_role_root, role->instance, perm->instance);
            return USP_ERR_INVALID_ARGUMENTS;
        }

        perm = (permission_t *) perm->link.next;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ExtractPermissions
**
** Reads the value of the specified parameter, adding to the permissions bitmask of the specified permission
**
** \param   role - role that the permission is for
** \param   perm - permission structure to set the permission in
** \param   param_name - name of the parameter containing the permissions to add in it's value
** \param   read_perm - bitmask of permission to add if the parameter's value contains 'r'
** \param   write_perm - bitmask of permission to add if the parameter's value contains 'w'
** \param   exec_perm - bitmask of permission to add if the parameter's value contains 'x'
** \param   notify_perm - bitmask of permission to add if the parameter's value contains 'n'
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ExtractPermissions(role_t *role, permission_t *perm, char *param_name, unsigned short read_perm, unsigned short write_perm, unsigned short exec_perm, unsigned short notify_perm)
{
    int err;
    char path[MAX_DM_PATH];
    char value[MAX_DM_SHORT_VALUE_LEN];

    // Exit if unable to read the value of the parameter
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Permission.%d.%s", device_role_root, role->instance, perm->instance, param_name);
    err = DATA_MODEL_GetParameterValue(path, value, sizeof(value), 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if value is incorrect length, or any of the characters are invalid
    if ((strlen(value) != 4) ||
        ((value[0] != 'r') && (value[0] != '-')) ||
        ((value[1] != 'w') && (value[1] != '-')) ||
        ((value[2] != 'x') && (value[2] != '-')) ||
        ((value[3] != 'n') && (value[3] != '-')))
    {
        USP_ERR_SetMessage("%s: %s contains invalid value (%s)", __FUNCTION__, path, value);
        return USP_ERR_INVALID_ARGUMENTS;
    }

    ModifyCTrustPermissionNibble(perm, value, read_perm, write_perm, exec_perm, notify_perm);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ModifyCTrustPermissionNibble
**
** Modifies the permission bitmask of the specified permission to the specified value
**
** \param   perm - permission structure to modify the permissions bitmask in
** \param   value - 4 character string in the form 'rwxn' specifying the permissions to apply
** \param   read_perm - bitmask of permission to add if the parameter's value contains 'r'
** \param   write_perm - bitmask of permission to add if the parameter's value contains 'w'
** \param   exec_perm - bitmask of permission to add if the parameter's value contains 'x'
** \param   notify_perm - bitmask of permission to add if the parameter's value contains 'n'
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void ModifyCTrustPermissionNibble(permission_t *perm, char *value, unsigned short read_perm, unsigned short write_perm, unsigned short exec_perm, unsigned short notify_perm)
{
    // First clear out the existing set of permissions
    perm->permission_bitmask &= ~(read_perm | write_perm | exec_perm | notify_perm);

    // Convert letters in permissions value string to permissions bitmask
    if (value[0] == 'r')
    {
        perm->permission_bitmask |= read_perm;
    }

    if (value[1] == 'w')
    {
        perm->permission_bitmask |= write_perm;
    }

    if (value[2] == 'x')
    {
        perm->permission_bitmask |= exec_perm;
    }

    if (value[3] == 'n')
    {
        perm->permission_bitmask |= notify_perm;
    }
}

/*********************************************************************//**
**
** AddCTrustPermission
**
** Adds a permission to the specified linked list, ensuring that the entries in the list are in increasing order
**
** \param   list - pointer to doubly linked list to add the pemission to
** \param   new_entry - permission structure to add to the list
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void AddCTrustPermission(double_linked_list_t *list, permission_t *new_entry)
{
    permission_t *perm;

    // Exit if list is empty, we can immediately add in this case
    if (list->head == NULL)
    {
        DLLIST_LinkToHead(list, new_entry);
        return;
    }

    // Determine where to add the permission in the list
    // Insert the entry before the entry that has a higher order
    perm = (permission_t *) list->head;
    while (perm != NULL)
    {
        if (perm->order > new_entry->order)
        {
            DLLIST_InsertLinkBefore(perm, list, new_entry);
            return;
        }

        perm = (permission_t *) perm->link.next;
    }

    // If the code gets here, then the new entry has a higher order than all the existing entries in the list
    // So add it to the end of the list
    DLLIST_LinkToTail(list, new_entry);
}

/*********************************************************************//**
**
** ApplyAllPermissionsForRole
**
** Applies all permissions for the specified role to the data model nodes
**
** \param   role - role to apply permissions for
**
** \return  None
**
**************************************************************************/
void ApplyAllPermissionsForRole(role_t *role)
{
    int i;
    char *path;
    dm_node_t *node;
    permission_t *perm;
    int role_index;

    // Calculate the index number of the specified role in the roles[]
    role_index = role - &roles[0];

    // Ensure that we start from no permissions applying for this role
    DM_PRIV_ApplyPermissions(NULL, role_index, PERMIT_NONE);

    // Exit if role is not enabled - nothing more to do
    if ((role->instance == INVALID) || (role->enable == false))
    {
        return;
    }

    // Iterate over all permissions, applying them to the data model
    // NOTE: As the permissions are ordered from low priority to high priority, later permissions can override earlier ones
    perm = (permission_t *) role->permissions.head;
    while (perm != NULL)
    {
        if (perm->enable)
        {
            // Apply permissions to all targets listed in this permission instance
            for (i=0; i < perm->targets.num_entries; i++)
            {
                path = perm->targets.vector[i];
                node =  DM_PRIV_GetNodeFromPath(path, NULL, NULL, DONT_LOG_ERRORS);
                if (node != NULL)
                {
                    DM_PRIV_ApplyPermissions(node, role_index, perm->permission_bitmask);
                }
            }
        }

        perm = (permission_t *) perm->link.next;
    }
}

/*********************************************************************//**
**
** FindUnusedRole
**
** Returns the first unused entry in role[]
**
** \param   None
**
** \return  Pointer to first unused entry in role[], or NULL if all entries are used
**
**************************************************************************/
role_t *FindUnusedRole(void)
{
    int i;
    role_t *role;

    for (i=0; i<NUM_ELEM(roles); i++)
    {
        role = &roles[i];
        if (role->instance == INVALID)
        {
            return role;
        }
    }

    return NULL;
}

/*********************************************************************//**
**
** FindRoleByInstance
**
** Finds the entry in role[] matching the specified instance number
**
** \param   instance - instance number of role in Device.LocalAgent.ControllerTrust.Role.{i} to match
**
** \return  Pointer to entry in role[] with matching instance number, or NULL no match was found
**
**************************************************************************/
role_t *FindRoleByInstance(int instance)
{
    int i;
    role_t *role;

    for (i=0; i<NUM_ELEM(roles); i++)
    {
        role = &roles[i];
        if (role->instance == instance)
        {
            return role;
        }
    }

    return NULL;
}

/*********************************************************************//**
**
** FreeRole
**
** Frees all memory associated with the specified role, and marks it as not in use
**
** \param   role - pointer to role to free
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void FreeRole(role_t *role)
{
    permission_t *perm;
    permission_t *next_perm;

    // Free all memory associated with the role's permissions
    perm = (permission_t *) role->permissions.head;
    while (perm != NULL)
    {
        next_perm = (permission_t *) perm->link.next;
        FreePermission(role, perm);
        perm = next_perm;
    }

    // Mark the role as not in use
    role->instance = INVALID;
    role->enable = false;
    USP_SAFE_FREE(role->name);
}

/*********************************************************************//**
**
** FreePermission
**
** Frees all memory associated with the specified permission
**
** \param   role - pointer to role containing the permission to free
** \param   perm - pointer to permission to free
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void FreePermission(role_t *role, permission_t *perm)
{
    STR_VECTOR_Destroy(&perm->targets);
    DLLIST_Unlink(&role->permissions, perm);
    USP_FREE(perm);
}

/*********************************************************************//**
**
** ModifyCTrustPermissionNibbleFromReq
**
** Modifies the permissions bitmask of the specified permission by applying the speciified value
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
** \param   read_perm - bitmask of permission to add if the parameter's value contains 'r'
** \param   write_perm - bitmask of permission to add if the parameter's value contains 'w'
** \param   exec_perm - bitmask of permission to add if the parameter's value contains 'x'
** \param   notify_perm - bitmask of permission to add if the parameter's value contains 'n'
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ModifyCTrustPermissionNibbleFromReq(dm_req_t *req, char *value, unsigned short read_perm, unsigned short write_perm, unsigned short exec_perm, unsigned short notify_perm)
{
    role_t *role;
    permission_t *perm;
    unsigned short old_perm_bitmask;

    role = FindRoleByInstance(inst1);
    USP_ASSERT(role != NULL);

    perm = FindPermissionByInstance(role, inst2);
    USP_ASSERT(perm != NULL);

    // Change the permissions bitmask
    old_perm_bitmask = perm->permission_bitmask;
    ModifyCTrustPermissionNibble(perm, value, read_perm, write_perm, exec_perm, notify_perm);

    // Exit if no change
    if (perm->permission_bitmask == old_perm_bitmask)
    {
        goto exit;
    }

    ScheduleRolePermissionsUpdate(inst1);

exit:
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ScheduleRolePermissionsUpdate
**
** Schedule the permissions for the specified role to be applied to the data model nodes
** when the current USP message has finished processing
** We delay applying the permissions until all parts of the roles[] data structure have been modified (via the Notify_CTrustXXX functions)
** as re-applying permissions is computationally expensive, so we only do it once per USP Message
**
** \param   instance - instance number of the role in Device.LocalAgent.ControllerTrust.Role.{i}
**
** \return  None
**
**************************************************************************/
void ScheduleRolePermissionsUpdate(int instance)
{
    int index;
    time_t cur_time;

    // Exit if this role is already scheduled to be updated, in which case, nothing to do
    index = INT_VECTOR_Find(&roles_to_update, instance);
    if (index != INVALID)
    {
        return;
    }

    // Add the instance number of the role to update
    INT_VECTOR_Add(&roles_to_update, instance);

    // Schedule the sync timer to fire immediately after the current USP message has been processed
    cur_time = time(NULL);
    SYNC_TIMER_Reload(ApplyModifiedPermissions, 0, cur_time);
}

/*********************************************************************//**
**
** ApplyModifiedPermissions
**
** Applies all modified permissions for the roles indicated by roles_to_update
** This function is a sync timer callback, which is called after processing a USP message,
** and after all permissions modifications have been made to the roles[] data structure by the Notrify_CTrustXXX() functions
**
** \param   id - (unused) identifier of the sync timer which caused this callback
**
** \return  None
**
**************************************************************************/
void ApplyModifiedPermissions(int id)
{
    int i;
    int instance;
    role_t *role;

    for (i=0; i < roles_to_update.num_entries; i++)
    {
        instance = roles_to_update.vector[i];
        role = FindRoleByInstance(instance);
        if (role != NULL)
        {
            ApplyAllPermissionsForRole(role);
        }
    }

    // Since we've reapplied permissions for all the roles that were modified, we can destroy the queue of roles to update
    INT_VECTOR_Destroy(&roles_to_update);
}

/*********************************************************************//**
**
** FindPermissionByInstance
**
** Finds the permission with the specified instance number for the specified role
**
** \param   role - role to find permission in
** \param   instance - instance number of permission
**
** \return  None
**
**************************************************************************/
permission_t *FindPermissionByInstance(role_t *role, int instance)
{
    permission_t *perm;

    perm = (permission_t *) role->permissions.head;
    while (perm != NULL)
    {
        if (perm->instance == instance)
        {
            return perm;
        }

        perm = (permission_t *) perm->link.next;
    }

    return NULL;
}

/*********************************************************************//**
**
** Get_CredentialNumEntries
**
** Gets the value of Device.LocalAgent.ControllerTrust.CredentialNumberOfEntries
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Get_CredentialNumEntries(dm_req_t *req, char *buf, int len)
{
    val_uint = num_credentials;
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Get_CredentialRole
**
** Gets the value of Device.LocalAgent.ControllerTrust.Credential.{i}.Role
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Get_CredentialRole(dm_req_t *req, char *buf, int len)
{
    credential_t *cp;

    cp = FindCredentialByInstance(inst1);
    USP_SNPRINTF(buf, len, "Device.LocalAgent.ControllerTrust.Role.%d", cp->role_instance);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Get_CredentialCertificate
**
** Gets the value of Device.LocalAgent.ControllerTrust.Credential.{i}.Credential
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Get_CredentialCertificate(dm_req_t *req, char *buf, int len)
{
    credential_t *cp;

    cp = FindCredentialByInstance(inst1);
    USP_SNPRINTF(buf, len, "Device.LocalAgent.Certificate.%d", cp->cert_instance);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** FindCredentialByInstance
**
** Finds the entry in credentials[] that has the specified instance number
**
** \param   instance - Instance number in Device.LocalAgent.ControllerTrust.Credential.{i}
**
** \return  pointer to credential structure or NULL if no match was found
**
**************************************************************************/
credential_t *FindCredentialByInstance(int instance)
{
    int i;
    credential_t *cp;

    // Find the credential whose instance number matches the specified certificate instance number
    for (i=0; i<num_credentials; i++)
    {
        cp = &credentials[i];
        if (cp->instance == instance)
        {
            return cp;
        }
    }

    return NULL;
}

/*********************************************************************//**
**
** FindCredentialByCertInstance
**
** Finds the entry in credentials[] that is for the specified certificate
**
** \param   cert_instance - Instance number of certificate in Device.LocalAgent.Certificate.{i} to match
**
** \return  pointer to credential structure or NULL if no match was found
**
**************************************************************************/
credential_t *FindCredentialByCertInstance(int cert_instance)
{
    int i;
    credential_t *cp;

    // Find the credential whose instance number matches the specified certificate instance number
    for (i=0; i<num_credentials; i++)
    {
        cp = &credentials[i];
        if (cp->cert_instance == cert_instance)
        {
            return cp;
        }
    }

    return NULL;
}

#ifndef REMOVE_DEVICE_SECURITY
/*********************************************************************//**
**
** InitChallengeTable
**
** Initialize the challenge_table[]
**
** \param   None
**
** \return  None
**
**************************************************************************/
int InitChallengeTable()
{
    int err;
    char path[MAX_DM_PATH];
    char num_entry[MAX_DM_VALUE_LEN] = {0};
    unsigned num;

    challenge_table = NULL;

    USP_SNPRINTF(path, sizeof(path), "%s.ChallengeNumberOfEntries", DEVICE_CTRUST_ROOT);
    err = DATA_MODEL_GetParameterValue(path, num_entry, sizeof(num_entry), 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    err = TEXT_UTILS_StringToUnsigned(num_entry, &num);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Ensure there are some entries in the challenge table
    num = MAX(num, MAX_CONTROLLERS);

    // allocate the challenge table and initialize it
    challenge_table = (challenge_table_t *) USP_MALLOC(sizeof(challenge_table_t) * num);
    USP_ASSERT(challenge_table != NULL);

    memset(challenge_table, 0, sizeof(challenge_table_t) * num);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DestroyControllerChallenge
**
** Clean up controller challenge structure
**
** \param   controller_challenge - pointer to controller challenge structure
**
** \return  None
**
**************************************************************************/
void DestroyControllerChallenge(controller_challenge_t *controller_challenge)
{
    USP_SAFE_FREE(controller_challenge->challenge_id);
    USP_SAFE_FREE(controller_challenge->challenge_ref);
    USP_SAFE_FREE(controller_challenge->controller_endpoint_id);
}

/*********************************************************************//**
**
** FindAvailableControllerChallenge
**
** Find available controller challenge structure by controller endpoint id
** If the same controller sends a second request, the same structure is going to be cleaned and reused
**
** \param   controller_endpoint_id - pointer to structure containing the controller endpoint id
** \param   challenge_ref - pointer to challenge reference string got from the RequestChallenge
** \param   cci - pointer to controller_challenges structure containing the matching controller endpoint
** id or NULL if all are used or if a request already made by this controller and has not expired yet.
**
** \return  USP_ERR_OK if able to find a instance in controller_challenges structure
** USP_ERR_INVALID_VALUE if controller already requested a challenge but it has not expired yet
** and controller tries to create another challenge
**
**************************************************************************/
int FindAvailableControllerChallenge(char *controller_endpoint_id, char *challenge_ref, controller_challenge_t **cci)
{
    int i;
    controller_challenge_t *cc;
    time_t now;

    *cci = NULL;
    now = time(NULL);
    // verify if the controller has a request challenge
    for (i = 0; i < NUM_ELEM(controller_challenges); i++)
    {
        cc = &controller_challenges[i];

        // if it does, return it
        if ((cc->controller_endpoint_id != NULL)
                && (strcmp(cc->controller_endpoint_id, controller_endpoint_id) == 0))
        {
            // There is at most one (1) outstanding RequestChallenge for a requesting Controller.
            // As such, any new challenges with a different value of the ChallengeRef parameter are denied
            // until a successful response to the outstanding challenge is received by the Agent
            // or the current RequestChallenge expires.

            // Check if the RequestChallenge expired, if yes destroy the current context before using it
            if (cc->expiration > 0 && cc->expire_time < now)
            {
                DestroyControllerChallenge(cc);
                *cci = cc;
                return USP_ERR_OK;
            }

            // If challengeRef is not same as the earlier one then return the error, else return last
            // saved information
            if ((cc->challenge_ref != NULL) && (strcmp(cc->challenge_ref, challenge_ref) != 0))
            {
                return USP_ERR_INVALID_VALUE;
            }

            // RequestExpiration shall be adjusted in caller
            *cci = cc;
            return USP_ERR_OK;
        }
    }

    // there is no request challenge for the controller
    for (i = 0; i < NUM_ELEM(controller_challenges); i++)
    {
        cc = &controller_challenges[i];

        // check for unused challenges
        // no challenge id means there is no challenge ref and no controller endpoint id
        if (cc->challenge_id == NULL)
        {
            *cci = cc;
            return USP_ERR_OK;
        }
    }

    // null should never be returned since the amount of available controller challenges
    // is equals to the maximum amount of controllers and each controller can have
    // only one controller challenge
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** FindControllerChallengeByEndpointId
**
** Find controller challenge structure by controller endpoint id
**
** \param   controller_endpoint_id - pointer to structure containing the controller endpoint id
**
** \return  pointer to controller challenge structure or NULL if there's none
**
**************************************************************************/
controller_challenge_t *FindControllerChallengeByEndpointId(char *controller_endpoint_id)
{
    int i;
    controller_challenge_t *cc;

    for (i = 0; i < NUM_ELEM(controller_challenges); i++)
    {
        cc = &controller_challenges[i];
        if (cc->controller_endpoint_id != NULL
                && strcmp(controller_endpoint_id, cc->controller_endpoint_id) == 0)
        {
            return cc;
        }
    }

    return NULL;
}

/*********************************************************************//**
**
** ControllerTrustRequestChallenge
**
** Called when sync command Device.LocalAgent.ControllerTrust.RequestChallenge() is executed
** Validates and generates the RequestChallenge to be sent to the controller
** which requested it.
**
** \param   req - pointer to structure identifying the command
** \param   command_key - not used
** \param   input_args - RequestChallenge() input parameters
** \param   output_args - RequestChallenge() output parameters
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INVALID_VALUE if the challenge reference is empty
**          USP_ERR_INVALID_VALUE if the challenge reference doesn't exist
**          USP_ERR_INVALID_VALUE if the challenge is disabled
**          USP_ERR_INVALID_VALUE if the controller does a new challenge request with different challenge reference
**
**************************************************************************/
int ControllerTrustRequestChallenge(dm_req_t *req, char *command_key, kv_vector_t *input_args, kv_vector_t *output_args)
{
    int err = USP_ERR_OK;
    char path[MAX_DM_PATH];
    bool enabled;
    controller_challenge_t *cc;
    controller_info_t ci;
    int challenge_ref_instance;

    // Input variables
    char *challenge_ref;
    int request_expiration;

    // Output variables
    char instruction[MAX_DM_VALUE_LEN];
    char instruction_type[MAX_DM_SHORT_VALUE_LEN];
    char value_type[MAX_DM_SHORT_VALUE_LEN];
    char challenge_id[MAX_DM_VALUE_LEN];

    // Extract the input arguments using KV_VECTOR_ functions
    challenge_ref = USP_ARG_Get(input_args, "ChallengeRef", "");
    err = USP_ARG_GetInt(input_args, "RequestExpiration", DEFAULT_REQUEST_EXPIRATION, &request_expiration);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Execute operation
    // validate if challenge reference is not empty
    if (strcmp(challenge_ref, "") == 0)
    {
        // if it doesn't, return invalid value
        USP_ERR_SetMessage("%s: Invalid value - challenge reference is empty", __FUNCTION__);
        err = USP_ERR_INVALID_VALUE;
        goto exit;
    }

    // validate if challenge reference exists
    err = DM_ACCESS_ValidateReference(challenge_ref, DEVICE_CHALLENGE_ROOT, &challenge_ref_instance);
    if (err != USP_ERR_OK)
    {
        err = USP_ERR_INVALID_VALUE;  // Not strictly necessary, as DM_ACCESS_ValidateReference() returns invlaid value
        goto exit;
    }

    // controller info that sent the current message
    MSG_HANDLER_GetControllerInfo(&ci);

    // verify if the challenge is enabled
    USP_SNPRINTF(path, sizeof(path), "%s.Enable", challenge_ref);
    err = DM_ACCESS_GetBool(path, &enabled);
    if (err != USP_ERR_OK)
    {
        err = USP_ERR_INVALID_VALUE;
        goto exit;
    }

    if (!enabled)
    {
        USP_ERR_SetMessage("%s: Invalid value - challenge disabled", __FUNCTION__);
        err = USP_ERR_INVALID_VALUE;
        goto exit;
    }

    // set the output parameters
    USP_SNPRINTF(path, sizeof(path), "%s.Instruction", challenge_ref);
    err = DATA_MODEL_GetParameterValue(path, instruction, sizeof(instruction), 0);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    USP_SNPRINTF(path, sizeof(path), "%s.InstructionType", challenge_ref);
    err = DATA_MODEL_GetParameterValue(path, instruction_type, sizeof(instruction_type), 0);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    USP_SNPRINTF(path, sizeof(path), "%s.ValueType", challenge_ref);
    err = DATA_MODEL_GetParameterValue(path, value_type, sizeof(value_type), 0);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // each controller can have only one request challenge
    // 'There is at most one (1) outstanding RequestChallenge for a requesting Controller.
    // As such, any new challenges with a different value of the ChallengeRef parameter are
    // denied until a successful response to the outstanding challenge is received by
    // the Agent or the current RequestChallenge expires.
    // error occurs in case of same controller request another challenge with different challengeRef
    err = FindAvailableControllerChallenge(ci.endpoint_id, challenge_ref, &cc);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Duplicate challenge requested", __FUNCTION__);
        goto exit;
    }
    USP_ASSERT(cc != NULL);

    // store the new data if not already set
    if (cc->controller_endpoint_id == NULL) {
        GenerateChallengeId(challenge_id, sizeof(challenge_id));
        cc->controller_endpoint_id = USP_STRDUP(ci.endpoint_id);
        cc->challenge_id = USP_STRDUP(challenge_id);
        cc->challenge_ref = USP_STRDUP(challenge_ref);
    }

    // Request expiration always updated with a new RequestChallenge
    cc->expiration = request_expiration;
    cc->expire_time = time(NULL) + request_expiration;

    // Save all results into the output arguments using KV_VECTOR_ functions
    USP_ARG_Add(output_args, "Instruction", instruction);
    USP_ARG_Add(output_args, "InstructionType", instruction_type);
    USP_ARG_Add(output_args, "ValueType", value_type);
    USP_ARG_Add(output_args, "ChallengeID", cc->challenge_id);

    err = USP_ERR_OK;

exit:
    return err;
}

/*********************************************************************//**
**
** GenerateChallengeId
**
** Generate challenge id for the request
**
** \param   challenge_id - pointer to buffer in which to write the message id
** \param   len - length of buffer
**
** \return  pointer to challenge id string
**
**************************************************************************/
char *GenerateChallengeId(char *challenge_id, int len)
{
    char buf[MAX_ISO8601_LEN];

    request_challenge_count++;
    USP_SNPRINTF(challenge_id, len, "Challenge-%s-%d", iso8601_cur_time(buf, sizeof(buf)), request_challenge_count);


    return challenge_id;
}

/*********************************************************************//**
**
** Validate_ChallengeRole
**
** Validates Device.LocalAgent.ControllerTrust.Challenge.{i}.Role parameter
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_ChallengeRole(dm_req_t *req, char *value)
{
    int err;
    int instance;

    err = DM_ACCESS_ValidateReference(value, DEVICE_ROLE_ROOT, &instance);

    return err;
}

/*********************************************************************//**
**
** Validate_ChallengeType
**
** Validates Device.LocalAgent.ControllerTrust.Challenge.{i}.Type parameter
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_ChallengeType(dm_req_t *req, char *value)
{
    // Exit if trying to set a value outside of the range we accept
    if (strcmp(value, CHALLENGE_TYPE) != 0)
    {
        USP_ERR_SetMessage("%s: Only Challenge type supported is '%s'", __FUNCTION__, CHALLENGE_TYPE);
        return USP_ERR_INVALID_VALUE;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_ChallengeValueType
**
** Validates Device.LocalAgent.ControllerTrust.Challenge.{i}.ValueType parameter
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_ChallengeValueType(dm_req_t *req, char *value)
{
    // Exit if trying to set a value outside of the range we accept
    if (strcmp(value, CHALLENGE_VALUE_TYPE) != 0)
    {
        USP_ERR_SetMessage("%s: Only Challenge value type supported is '%s'", __FUNCTION__, CHALLENGE_VALUE_TYPE);
        return USP_ERR_INVALID_VALUE;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_ChallengeInstructionType
**
** Validates Device.LocalAgent.ControllerTrust.Challenge.{i}.InstructionType parameter
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_ChallengeInstructionType(dm_req_t *req, char *value)
{
    // Exit if trying to set a value outside of the range we accept
    if (strcmp(value, CHALLENGE_INSTRUCTION_TYPE) != 0)
    {
        USP_ERR_SetMessage("%s: Only Challenge instruction type supported is '%s'", __FUNCTION__, CHALLENGE_INSTRUCTION_TYPE);
        return USP_ERR_INVALID_VALUE;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ControllerTrustChallengeResponse
**
** Called when sync command Device.LocalAgent.ControllerTrust.ChallengeResponse() is executed
** Validates ChallengeResponse received by the agent.
** Matches the existing request with the response.
** Validates password.
** Assign new role to the controller.
**
** \param   req - pointer to structure identifying the command
** \param   command_key - not used
** \param   input_args - ChallengeResponse() input parameters
** \param   output_args - not used
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INVALID_VALUE if there is no challenge request
**          USP_ERR_INVALID_VALUE if the challenge id is not found
**          USP_ERR_INVALID_VALUE if the challenge expired
**          USP_ERR_COMMAND_FAILURE if the role is invalid
**          USP_ERR_REQUEST_DENIED if the lockout period is not expired
**          USP_ERR_COMMAND_FAILURE if the password doesn't match
**          USP_ERR_COMMAND_FAILURE if the controller is not found
**          USP_ERR_COMMAND_FAILURE if it's not possible to set the role on the controller
**
**************************************************************************/
int ControllerTrustChallengeResponse(dm_req_t *req, char *command_key, kv_vector_t *input_args, kv_vector_t *output_args)
{
    int err = USP_ERR_OK;
    char path[MAX_DM_PATH];
    char base64_value[MAX_DM_VALUE_LEN];
    unsigned char binary_value[MAX_DM_VALUE_LEN];
    unsigned char response_value[MAX_DM_VALUE_LEN];
    int binary_value_len;
    int response_value_len;
    unsigned retries;
    int lockout;
    controller_challenge_t *cc;
    controller_info_t ci;
    int challenge_ref_instance;
    int controller_instance;
    char role[MAX_DM_VALUE_LEN];

    // Input variables
    char *input_challenge_id;
    char *input_value;

    // Extract the input arguments using KV_VECTOR_ functions
    input_challenge_id = USP_ARG_Get(input_args, "ChallengeID", "");
    input_value = USP_ARG_Get(input_args, "Value", "");

    // Execute operation
    MSG_HANDLER_GetControllerInfo(&ci);

    cc = FindControllerChallengeByEndpointId(ci.endpoint_id);

    // if challenge was never requested
    if (cc == NULL)
    {
        USP_ERR_SetMessage("%s: Invalid value - challenge never requested", __FUNCTION__);
        err = USP_ERR_INVALID_VALUE;
        goto exit;
    }

    // if the challenge is not found (challenge id matches),
    // return 7012 Invalid value according to https://issues.broadband-forum.org/browse/DEV2DM-32
    if (strcmp(cc->challenge_id, input_challenge_id) != 0)
    {
        USP_ERR_SetMessage("%s: Invalid value - challenge id not found", __FUNCTION__);
        err = USP_ERR_INVALID_VALUE;
        goto exit;
    }

    // if the challenge expired,
    // return 7012 Invalid value according to https://issues.broadband-forum.org/browse/DEV2DM-32
    time_t now = time(NULL);
    if ((cc->expiration > 0) && (now >= cc->expire_time))
    {
        USP_ERR_SetMessage("%s: Invalid value - challenge expired", __FUNCTION__);
        err = USP_ERR_INVALID_VALUE;
        goto exit;
    }

    // verify if the challenge object was not removed before receiving challenge response
    err = DM_ACCESS_ValidateReference(cc->challenge_ref, DEVICE_CHALLENGE_ROOT, &challenge_ref_instance);
    if (err != USP_ERR_OK)
    {
        err = USP_ERR_COMMAND_FAILURE;
        // destroy existing request, new challenge request is required
        DestroyControllerChallenge(cc);
        goto exit;
    }

    // retrieve values from challenge
    // Value
    USP_SNPRINTF(path, sizeof(path), "%s.Value", cc->challenge_ref);
    err = DATA_MODEL_GetParameterValue(path, base64_value, sizeof(base64_value), SHOW_PASSWORD);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Convert base64_value from base64 to binary form
    err = TEXT_UTILS_Base64StringToBinary(base64_value, binary_value, sizeof(binary_value), &binary_value_len);
    USP_ASSERT(err == USP_ERR_OK);      // The code should have only allowed base64 values to be written

    // Retries
    USP_SNPRINTF(path, sizeof(path), "%s.Retries", cc->challenge_ref);
    err = DM_ACCESS_GetUnsigned(path, &retries);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // LockoutPeriod
    USP_SNPRINTF(path, sizeof(path), "%s.LockoutPeriod", cc->challenge_ref);
    err = DM_ACCESS_GetInteger(path, &lockout);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // verify if the locked period passed
    if (lockout > 0 && challenge_table[challenge_ref_instance].locked_time > now)
    {
        // it's still locked out
        USP_ERR_SetMessage("%s: Invalid value - lockout period hasn't expired", __FUNCTION__);
        err = USP_ERR_REQUEST_DENIED;
        goto exit;
    }

    // Attempt to convert challenge response input value from base64 to a binary value
    err = TEXT_UTILS_Base64StringToBinary(input_value, response_value, sizeof(response_value), &response_value_len);

    // Exit if challenge response input value was not valid base64, or it did not match the value setup in the Challenge table
    if ((err != USP_ERR_OK) || (binary_value_len != response_value_len) || (memcmp(binary_value, response_value, binary_value_len) != 0))
    {
        // wrong password
        // increase retries
        challenge_table[challenge_ref_instance].retries++;

        // The number of times a ControllerTrust.Challenge.{i}. entry can be consecutively failed
        // (across all Controllers, without intermediate success) is defined by Retries. Once the
        // number of failed consecutive attempts equals Retries, the ControllerTrust.Challenge.{i}.
        // cannot be retried until after LockoutPeriod has expired.
        if (challenge_table[challenge_ref_instance].retries >= retries && lockout > 0)
        {
            challenge_table[challenge_ref_instance].locked_time = now + lockout;
        }

        USP_ERR_SetMessage("%s: Command failure - invalid password", __FUNCTION__);
        err = USP_ERR_COMMAND_FAILURE;
        goto exit;
    }

    // get controller instance
    controller_instance = DEVICE_CONTROLLER_FindInstanceByEndpointId(ci.endpoint_id);
    if (controller_instance == INVALID)
    {
        USP_ERR_SetMessage("%s: Command failure - controller not found", __FUNCTION__);
        err = USP_ERR_COMMAND_FAILURE;
        goto exit;
    }

    // get role from challenge
    USP_SNPRINTF(path, sizeof(path), "%s.Role", cc->challenge_ref);
    err = DATA_MODEL_GetParameterValue(path, role, sizeof(role), 0);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Command failure - get role from challenge failed", __FUNCTION__);
        err = USP_ERR_COMMAND_FAILURE;
        goto exit;
    }

    // set role to controller
    USP_SNPRINTF(path, sizeof(path), "Device.LocalAgent.Controller.%d.AssignedRole", controller_instance);
    err = DATA_MODEL_SetParameterValue(path, role, 0);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Command failure - set role failed", __FUNCTION__);
        err = USP_ERR_COMMAND_FAILURE;
        goto exit;
    }

    // if the code reached here, everything worked out
    // that means the current challenge should be cleaned up
    DestroyControllerChallenge(cc);

    // Re-initialize the locked_time and retries with success
    challenge_table[challenge_ref_instance].locked_time = 0;
    challenge_table[challenge_ref_instance].retries = 0;

    err = USP_ERR_OK;

exit:
    return err;
}
#endif // REMOVE_DEVICE_SECURITY
