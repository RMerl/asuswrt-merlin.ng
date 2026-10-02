/*
 *
 * Copyright (C) 2021-2022, Broadband Forum
 * Copyright (C) 2016-2022  CommScope, Inc
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
 * \file dm_access.c
 *
 * Implements functions to get and set parameters by type in the data model
 * As the data model (and database) store all parameters as strings, the functions in this file convert to and from strings
 *
 */

#include <stdlib.h>
#include <string.h>

#include "common_defs.h"
#include "data_model.h"
#include "dm_access.h"
#include "str_vector.h"
#include "iso8601.h"
#include "text_utils.h"
#include "expr_vector.h"
#include "nu_ipaddr.h"

/*********************************************************************//**
**
** DM_ACCESS_GetString
**
** Gets the value of the specified parameter as a dynamically allocated string
** NOTE: The caller is responsible for freeing the memory that this function allocates.
** NOTE: As all parameters are stored internally as a string, this function can be called for any parameter
**       It will return a blank string for passwords.
**
** \param   path - pointer to string containing complete data model path to the parameter
** \param   p_str - pointer to variable in which to return a pointer to a dynamically allocated string containing the value
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_ACCESS_GetString(char *path, char **p_str)
{
    int err;
    char buf[MAX_DM_VALUE_LEN];

    // Exit if unable to get the value of the parameter
    err = DATA_MODEL_GetParameterValue(path, buf, sizeof(buf), 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Dynamically allocate a copy of the string
    *p_str = USP_STRDUP(buf);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_ACCESS_GetPassword
**
** Gets the value of the password. This is a special function, as normally
** when you read a password, it is read back as an empty string
** NOTE: The caller is responsible for freeing the memory that this function allocates.
**
** \param   path - pointer to string containing complete data model path to the parameter
** \param   p_str - pointer to variable in which to return a pointer to a dynamically allocated string containing the value
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_ACCESS_GetPassword(char *path, char **p_str)
{
    int err;
    char buf[MAX_DM_SHORT_VALUE_LEN];

    // Exit if unable to get the value of the parameter
    err = DATA_MODEL_GetParameterValue(path, buf, sizeof(buf), SHOW_PASSWORD);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Dynamically allocate a copy of the string
    *p_str = USP_STRDUP(buf);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_ACCESS_GetInteger
**
** Gets the value of the specified parameter as an integer value
**
** \param   path - pointer to string containing complete path
** \param   value - pointer to variable in which to return the value
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_ACCESS_GetInteger(char *path, int *value)
{
    int err;
    char buf[MAX_DM_SHORT_VALUE_LEN];

    // Exit if unable to get the parameter as a string
    err = DATA_MODEL_GetParameterValue(path, buf, sizeof(buf), 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if the parameter could not be converted
    err = TEXT_UTILS_StringToInteger(buf, value);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Illegal value (%s) in parameter %s", __FUNCTION__, buf, path);
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_ACCESS_GetUnsigned
**
** Gets the value of the specified parameter as an unsigned value
**
** \param   value - pointer to variable in which to return the value
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_ACCESS_GetUnsigned(char *path, unsigned *value)
{
    int err;
    char buf[MAX_DM_SHORT_VALUE_LEN];

    // Exit if unable to get the parameter as a string
    err = DATA_MODEL_GetParameterValue(path, buf, sizeof(buf), 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if the parameter could not be converted
    err = TEXT_UTILS_StringToUnsigned(buf, value);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Illegal value (%s) in parameter %s", __FUNCTION__, buf, path);
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_ACCESS_GetBool
**
** Gets a parameter value as a boolean
**
** \param   path - pointer to string containing complete path
** \param   value - pointer to variable in which to return the value
**
** \return  USP_ERR_OK if retrieved successfully
**          USP_ERR_INVALID_TYPE if string value does not represent a boolean
**
**************************************************************************/
int DM_ACCESS_GetBool(char *path, bool *value)
{
    int err;
    char buf[MAX_DM_SHORT_VALUE_LEN];

    // Exit if unable to get the parameter as a string
    err = DATA_MODEL_GetParameterValue(path, buf, sizeof(buf), 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if the parameter could not be converted
    err = TEXT_UTILS_StringToBool(buf, value);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Illegal value (%s) in parameter %s", __FUNCTION__, buf, path);
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_ACCESS_GetEnum
**
** Gets a parameter value as an enumerated integer
**
** \param   path - pointer to string containing complete path
** \param   value - pointer to variable in which to return the enumerated value
** \param   enums - pointer to conversion table containing a list of enumerations and their associated string representation
** \param   num_enums - number of enumerations in the table
**
** \return  USP_ERR_OK if retrieved successfully
**          USP_ERR_INVALID_TYPE if string value does not represent a boolean
**
**************************************************************************/
int DM_ACCESS_GetEnum(char *path, void *value, const enum_entry_t *enums, int num_enums)
{
    int err;
    char buf[MAX_DM_SHORT_VALUE_LEN];
    int e;

    // Exit if unable to get the parameter as a string
    err = DATA_MODEL_GetParameterValue(path, buf, sizeof(buf), 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if the parameter could not be converted
    e = TEXT_UTILS_StringToEnum(buf, enums, num_enums);
    if (e == INVALID)
    {
        USP_ERR_SetMessage("%s: Unknown or unsupported enumeration (%s) in parameter %s", __FUNCTION__, buf, path);
        return USP_ERR_INVALID_VALUE;
    }
    *((int *)value) = e;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_ACCESS_GetDateTime
**
** Gets a parameter value as a unix time type
**
** \param   path - pointer to string containing complete path
** \param   value - pointer to variable to return converted value in
**
** \return  USP_ERR_OK if retrieved successfully
**          USP_ERR_INVALID_TYPE if string value does not represent a boolean
**
**************************************************************************/
int DM_ACCESS_GetDateTime(char *path, time_t *value)
{
    int err;
    char buf[MAX_DM_SHORT_VALUE_LEN];

    // Exit if unable to get the parameter as a string
    err = DATA_MODEL_GetParameterValue(path, buf, sizeof(buf), 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if the parameter could not be converted
    err = TEXT_UTILS_StringToDateTime(buf, value);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Illegal value (%s) in parameter %s", __FUNCTION__, buf, path);
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_ACCESS_GetStringVector
**
** Gets a parameter value as a list of substrings
** NOTE: In the database, the sub-strings are stored comma separated in a single string
**
** \param   path - pointer to string containing complete path
** \param   sv - pointer to variable to return string vector in
**
** \return  USP_ERR_OK if retrieved successfully
**          USP_ERR_INVALID_TYPE if string value does not represent a boolean
**
**************************************************************************/
int DM_ACCESS_GetStringVector(char *path, str_vector_t *sv)
{
    int err;
    char buf[MAX_DM_VALUE_LEN];

    // Exit if unable to get the parameter as a string
    err = DATA_MODEL_GetParameterValue(path, buf, sizeof(buf), 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    TEXT_UTILS_SplitString(buf, sv, ",");

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_ACCESS_GetIpAddr
**
** Gets a parameter value as an IP address
** NOTE: An empty string is also considered valid (used by UDP EchoConfig)
**
** \param   path - pointer to string containing complete path
** \param   ip_addr - pointer to variable to return IP address in
**
** \return  USP_ERR_OK if retrieved successfully
**          USP_ERR_INVALID_TYPE if string value does not represent an IP address
**
**************************************************************************/
int DM_ACCESS_GetIpAddr(char *path, nu_ipaddr_t *ip_addr)
{
    int err;
    char buf[NU_IPADDRSTRLEN];

    // Exit if unable to get the parameter as a string
    err = DATA_MODEL_GetParameterValue(path, buf, sizeof(buf), 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to convert the string to an IP address
    err = TEXT_UTILS_StringToIpAddr(buf, ip_addr);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_ACCESS_SetInteger
**
** Sets the value of the specified parameter to an integer value
**
** \param   value - pointer to variable in which to return the value
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_ACCESS_SetInteger(char *path, int value)
{
    int err;
    char buf[MAX_DM_SHORT_VALUE_LEN];

    // Exit if unable to set the parameter
    USP_SNPRINTF(buf, sizeof(buf), "%d", value);
    err = DATA_MODEL_SetParameterValue(path, buf, 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_ACCESS_ValidateBool
**
** Validates that the specified value string represents a boolean condition
** This function is supplied to be used as a vendor hook validation callback
**
** \param   req - pointer to structure containing path information
** \param   value - value of the parameter to check
**
** \return  USP_ERR_OK if validated successfully
**          USP_ERR_INVALID_TYPE if string value does not represent a boolean
**
**************************************************************************/
int DM_ACCESS_ValidateBool(dm_req_t *req, char *value)
{
    int err;
    bool b;

    err = TEXT_UTILS_StringToBool(value, &b);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Expected a boolean for param=%s (got=%s)", __FUNCTION__, req->path, value);
        return USP_ERR_INVALID_TYPE;
    }

    return err;
}

/*********************************************************************//**
**
** DM_ACCESS_ValidateBase64
**
** Validates that the specified value string represents a base64 encoded binary blob
** This function is supplied to be used as a vendor hook validation callback
**
** \param   req - pointer to structure containing path information
** \param   value - value of the parameter to check
**
** \return  USP_ERR_OK if validated successfully
**          USP_ERR_INVALID_TYPE if string value does not represent a base64 encoded string
**
**************************************************************************/
int DM_ACCESS_ValidateBase64(dm_req_t *req, char *value)
{
    int err;
    unsigned char buf[MAX_DM_VALUE_LEN];

    err = TEXT_UTILS_Base64StringToBinary(value, buf, sizeof(buf), NULL);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Invalid base64 encoded string for param=%s", __FUNCTION__, req->path);
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_ACCESS_ValidatePort
**
** Validates that the specified value is a valid port number
** This function is supplied to be used as a vendor hook validation callback
**
** \param   req - pointer to structure containing path information
** \param   value - value of the parameter to check
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_ACCESS_ValidatePort(dm_req_t *req, char *value)
{
    // Exit if port is out of range
    if ((val_uint < 1) || (val_uint > 65535))
    {
        USP_ERR_SetMessage("%s: Port (%d) is not within the range 1-65535", __FUNCTION__, val_uint);
        return USP_ERR_INVALID_VALUE;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_ACCESS_ValidateRange_Unsigned
**
** Validates that the specified unsigned value falls within the specified range
**
** \param   req - pointer to structure containing path information and value
** \param   min_value - minimum allowable value for the parameter
** \param   min_value - maximum allowable value for the parameter
**
** \return  USP_ERR_OK if validated successfully
**
**************************************************************************/
int DM_ACCESS_ValidateRange_Unsigned(dm_req_t *req, unsigned min_value, unsigned max_value)
{
    char *name;

    // Exit if value is not within range
    if ((val_uint < min_value) || (val_uint > max_value))
    {
        // Calculate the name of the parameter
        name = strrchr(req->path, '.');
        if (name == NULL)
        {
            name = req->path;
        }
        else
        {
            name++;     // Skip the '.'
        }

        USP_ERR_SetMessage("%s: %s must be in the range [%u:%u], got %u", __FUNCTION__, name, min_value, max_value, val_uint);
        return USP_ERR_INVALID_VALUE;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_ACCESS_ValidateRange_Signed
**
** Validates that the specified signed value falls within the specified range
**
** \param   req - pointer to structure containing path information and value
** \param   min_value - minimum allowable value for the parameter
** \param   min_value - maximum allowable value for the parameter
**
** \return  USP_ERR_OK if validated successfully
**
**************************************************************************/
int DM_ACCESS_ValidateRange_Signed(dm_req_t *req, int min_value, int max_value)
{
    char *name;

    // Exit if value is not within range
    if ((val_int < min_value) || (val_int > max_value))
    {
        // Calculate the name of the parameter
        name = strrchr(req->path, '.');
        if (name == NULL)
        {
            name = req->path;
        }
        else
        {
            name++;     // Skip the '.'
        }

        USP_ERR_SetMessage("%s: %s must be in the range [%d:%d], got %d", __FUNCTION__, name, min_value, max_value, val_int);
        return USP_ERR_INVALID_VALUE;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_ACCESS_ValidateReference
**
** Validates that specified reference is to an instance which exists in the specified table
** NOTE: The specified table must be a multi-instance or nested multi-instance table that exists in the data model
**
** \param   reference - path to the instance in the specified table
** \param   table - data model schema path of table eg Device.STOMP.Connection.{i}
** \param   instance - pointer to variable in which to return the instance number in the specified table
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_ACCESS_ValidateReference(char *reference, char *table, int *instance)
{
    int err;
    char *schema_path;
    dm_req_instances_t inst;
    bool instances_exist;
    dm_node_t *node;

    // Exit if the reference is not present in the data model
    err = DATA_MODEL_SplitPath(reference, &schema_path, &inst, &instances_exist);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if the reference is to the wrong table
    if (strcmp(schema_path, table) != 0)
    {
        USP_ERR_SetMessage("%s: Reference (%s) is to the wrong data model table (expecting %s)", __FUNCTION__, reference, table);
        return USP_ERR_INVALID_VALUE;
    }

    // Determine the expected number of instance numbers in the specified table
    node = DM_PRIV_GetNodeFromPath(table, NULL, NULL);
    USP_ASSERT(node != NULL);       // These asserts check that the caller provided a multi-instance table that exists in the supported data model
    USP_ASSERT(node->order > 0);

    // Exit if the reference does not have the expected number of instance numbers in the path
    if (inst.order != node->order)
    {
        USP_ERR_SetMessage("%s: Reference (%s) does not contain trailing instance number", __FUNCTION__, reference);
        return USP_ERR_INVALID_VALUE;
    }

    // Exit if the reference is to a object instance which is not instantiated - this could occur at bootup
    if (instances_exist == false)
    {
        USP_ERR_SetMessage("%s: Reference (%s) is not instantiated", __FUNCTION__, reference);
        return USP_ERR_INVALID_VALUE;
    }

    // Extract the trailing instance number of the reference in the table
    *instance = inst.instances[inst.order-1];

    // If code gets here, then the reference was valid
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_ACCESS_ValidateIpAddr
**
** Validates that the new value for Device.IP.Diagnostics.UDPEchoConfig.SourceIPAddress is valid
** NOTE: An empty string is also considered valid (used by UDP EchoConfig)
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_ACCESS_ValidateIpAddr(dm_req_t *req, char *value)
{
    int err;
    nu_ipaddr_t ip_addr;

    // Exit if unable to convert the IP address string
    err = TEXT_UTILS_StringToIpAddr(value, &ip_addr);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_ACCESS_CompareString
**
** Compares two strings
**
** \param   lhs - string representing the left hand operand to compare
** \param   op - operator to use when comparing the values
** \param   rhs - string representing the right hand operand to compare
** \param   result - pointer to boolean in which to return whether the comparison matched or not
**
** \return  USP_ERR_OK if validated successfully
**
**************************************************************************/
int DM_ACCESS_CompareString(char *lhs, expr_op_t op, char *rhs, bool *result)
{
    int err;

    *result = false;    // Assume that comparison failed to match
    err = USP_ERR_OK;   // Assume that comparison operator was valid
    switch(op)
    {
        case kExprOp_Equal:
            if (strcmp(lhs, rhs)==0)
            {
                *result = true;
            }
            break;

        case kExprOp_NotEqual:
            if (strcmp(lhs, rhs)!=0)
            {
                *result = true;
            }
            break;

        case kExprOp_LessThanOrEqual:
        case kExprOp_GreaterThanOrEqual:
        case kExprOp_LessThan:
        case kExprOp_GreaterThan:
            USP_ERR_SetMessage("%s: Operator '%s' not supported for strings", __FUNCTION__, expr_op_2_str[op]);
            err = USP_ERR_INVALID_PATH_SYNTAX;
            break;

        default:
            TERMINATE_BAD_CASE(op);
            break;
    }

    return err;
}

/*********************************************************************//**
**
** DM_ACCESS_CompareNumber
**
** Compares two numeric values, supplied as strings
** NOTE: In order that this function can support all numbers, internally it converts to long doubles
**
** \param   lhs - string representing the left hand operand to compare
** \param   op - operator to use when comparing the values
** \param   rhs - string representing the right hand operand to compare
** \param   result - pointer to boolean in which to return whether the comparison matched or not
**
** \return  USP_ERR_OK if validated successfully
**
**************************************************************************/
int DM_ACCESS_CompareNumber(char *lhs, expr_op_t op, char *rhs, bool *result)
{
    long double lh_value;
    long double rh_value;
    int err;
    int num_converted;

    // Exit if the left hand operand could not be converted
    // NOTE: This is unexpected behaviour, as the left hand operand will have previously been read from the data model
    num_converted = sscanf(lhs, "%Lf", &lh_value);
    if (num_converted == 0)
    {
        USP_ERR_SetMessage("%s: Expecting expression parameter's value ('%s') to be a number", __FUNCTION__, lhs);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if the right hand operand could not be converted
    // NOTE: This could occur if the search expression contained errors in it
    num_converted = sscanf(rhs, "%Lf", &rh_value);
    if (num_converted == 0)
    {
        USP_ERR_SetMessage("%s: Expecting expression constant ('%s') to be a number", __FUNCTION__, rhs);
        return USP_ERR_INVALID_PATH_SYNTAX;
    }

    *result = false;    // Assume that comparison failed to match
    err = USP_ERR_OK;   // Assume that comparison operator was valid
    switch(op)
    {
        case kExprOp_Equal:
            if (lh_value == rh_value)
            {
                *result = true;
            }
            break;

        case kExprOp_NotEqual:
            if (lh_value != rh_value)
            {
                *result = true;
            }
            break;

        case kExprOp_LessThanOrEqual:
            if (lh_value <= rh_value)
            {
                *result = true;
            }
            break;

        case kExprOp_GreaterThanOrEqual:
            if (lh_value >= rh_value)
            {
                *result = true;
            }
            break;

        case kExprOp_LessThan:
            if (lh_value < rh_value)
            {
                *result = true;
            }
            break;

        case kExprOp_GreaterThan:
            if (lh_value > rh_value)
            {
                *result = true;
            }
            break;

        default:
            TERMINATE_BAD_CASE(op);
            break;
    }

    return err;
}

/*********************************************************************//**
**
** DM_ACCESS_CompareBool
**
** Compares two boolean values, supplied as strings
**
** \param   lhs - string representing the left hand operand to compare
** \param   op - operator to use when comparing the values
** \param   rhs - string representing the right hand operand to compare
** \param   result - pointer to boolean in which to return whether the comparison matched or not
**
** \return  USP_ERR_OK if validated successfully
**
**************************************************************************/
int DM_ACCESS_CompareBool(char *lhs, expr_op_t op, char *rhs, bool *result)
{
    bool lh_value;
    bool rh_value;
    int err;

    // Exit if the left hand operand could not be converted
    // NOTE: This is unexpected behaviour, as the left hand operand will have previously been read from the data model
    err = TEXT_UTILS_StringToBool(lhs, &lh_value);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Expecting expression parameter's value ('%s') to be a boolean", __FUNCTION__, lhs);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if the right hand operand could not be converted
    // NOTE: This could occur if the search expression contained errors in it
    err = TEXT_UTILS_StringToBool(rhs, &rh_value);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Expecting expression constant ('%s') to be a boolean", __FUNCTION__, rhs);
        return USP_ERR_INVALID_PATH_SYNTAX;
    }

    *result = false;    // Assume that comparison failed to match
    err = USP_ERR_OK;   // Assume that comparison operator was valid
    switch(op)
    {
        case kExprOp_Equal:
            if (lh_value == rh_value)
            {
                *result = true;
            }
            break;

        case kExprOp_NotEqual:
            if (lh_value != rh_value)
            {
                *result = true;
            }
            break;

        case kExprOp_LessThanOrEqual:
        case kExprOp_GreaterThanOrEqual:
        case kExprOp_LessThan:
        case kExprOp_GreaterThan:
            USP_ERR_SetMessage("%s: Operator '%s' not supported for booleans", __FUNCTION__, expr_op_2_str[op]);
            err = USP_ERR_INVALID_PATH_SYNTAX;
            break;

        default:
            TERMINATE_BAD_CASE(op);
            break;
    }

    return err;
}

/*********************************************************************//**
**
** DM_ACCESS_CompareDateTime
**
** Compares two date-time values, supplied as strings
**
** \param   lhs - string representing the left hand operand to compare
** \param   op - operator to use when comparing the values
** \param   rhs - string representing the right hand operand to compare
** \param   result - pointer to boolean in which to return whether the comparison matched or not
**
** \return  USP_ERR_OK if validated successfully
**
**************************************************************************/
int DM_ACCESS_CompareDateTime(char *lhs, expr_op_t op, char *rhs, bool *result)
{
    time_t lh_value;
    time_t rh_value;
    int err;

    // Exit if the left hand operand could not be converted
    // NOTE: This is unexpected behaviour, as the left hand operand will have previously been read from the data model
    err = TEXT_UTILS_StringToDateTime(lhs, &lh_value);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Expecting expression parameter's value ('%s') to be an ISO8601 dateTime", __FUNCTION__, lhs);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if the right hand operand could not be converted
    // NOTE: This could occur if the search expression contained errors in it
    err = TEXT_UTILS_StringToDateTime(rhs, &rh_value);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Expecting expression constant ('%s') to be an ISO8601 dateTime", __FUNCTION__, rhs);
        return USP_ERR_INVALID_PATH_SYNTAX;
    }

    *result = false;    // Assume that comparison failed to match
    err = USP_ERR_OK;   // Assume that comparison operator was valid
    switch(op)
    {
        case kExprOp_Equal:
            if (lh_value == rh_value)
            {
                *result = true;
            }
            break;

        case kExprOp_NotEqual:
            if (lh_value != rh_value)
            {
                *result = true;
            }
            break;

        case kExprOp_LessThanOrEqual:
            if (lh_value <= rh_value)
            {
                *result = true;
            }
            break;

        case kExprOp_GreaterThanOrEqual:
            if (lh_value >= rh_value)
            {
                *result = true;
            }
            break;

        case kExprOp_LessThan:
            if (lh_value < rh_value)
            {
                *result = true;
            }
            break;

        case kExprOp_GreaterThan:
            if (lh_value > rh_value)
            {
                *result = true;
            }
            break;

        default:
            TERMINATE_BAD_CASE(op);
            break;
    }

    return err;
}

/*********************************************************************//**
**
** DM_ACCESS_RestartAsyncOperation
**
** This function is a convenience function for the restart_cb callback of an Async Operation
** This function will cause the async operation to be restarted on bootup, if it is still present in the Request table
**
** \param   req - pointer to structure containing path information
** \param   instance - instance number of this operation in the Request table
** \param   is_restart - pointer to variable in which to return whether the operation should be restarted or not
**
**                     The following parameters are only used if the operation should not be restarted
**                     They determine the values placed in the operation complete message
** \param   err_code - pointer to variable in which to return an error code
** \param   err_msg - pointer to buffer in which to return an error message (only used if error code is failed)
** \param   err_msg_len - length of buffer in which to return an error message (only used if error code is failed)
** \param   output_args - pointer to structure in which to return output arguments for the operation
**
** \return  USP_ERR_OK if validated successfully
**
**************************************************************************/
int DM_ACCESS_RestartAsyncOperation(dm_req_t *req, int instance, bool *is_restart, int *err_code, char *err_msg, int err_msg_len, kv_vector_t *output_args)
{
    *is_restart = true;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_ACCESS_DontRestartAsyncOperation
**
** This function is a convenience function for the restart_cb callback of an Async Operation
** This function will cause the async operation to not be restarted on bootup, if it is still present in the Request table
** NOTE: This function is equivalent to registering NULL for restart_cb
**
** \param   req - pointer to structure containing path information
** \param   instance - instance number of this operation in the Request table
** \param   is_restart - pointer to variable in which to return whether the operation should be restarted or not
**
**                     The following parameters are only used if the operation should not be restarted
**                     They determine the values placed in the operation complete message
** \param   err_code - pointer to variable in which to return an error code
** \param   err_msg - pointer to buffer in which to return an error message (only used if error code is failed)
** \param   err_msg_len - length of buffer in which to return an error message (only used if error code is failed)
** \param   output_args - pointer to structure in which to return output arguments for the operation
**
** \return  USP_ERR_OK if validated successfully
**
**************************************************************************/
int DM_ACCESS_DontRestartAsyncOperation(dm_req_t *req, int instance, bool *is_restart, int *err_code, char *err_msg, int err_msg_len, kv_vector_t *output_args)
{
    *is_restart = false;
    *err_code = USP_ERR_COMMAND_FAILURE;
    USP_SNPRINTF(err_msg, err_msg_len, "%s: Operation %s did not complete before reboot", __FUNCTION__, req->path);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_ACCESS_PopulateAliasParam
**
** Called to get an auto-populated parameter value for an Alias parameter
** This function takes account of the multi-dimensionality of the object
** Using the highest order dimension to number the instance
**
** \param   req - pointer to structure identifying the path
** \param   buf - pointer to buffer in which to store the value to use to auto-populate the parameter's value
** \param   len - length of return buffer
**
** \return  USP_ERR_OK if retrieved successfully
**
**************************************************************************/
int DM_ACCESS_PopulateAliasParam(dm_req_t *req, char *buf, int len)
{
    int instance;

    // Exit if Alias parameter has not been used correctly ie. is not a member of a table
    if (req->inst->order == 0)
    {
        USP_ERR_SetMessage("%s: Auto-populate Alias parameter is not in a table (%s)", __FUNCTION__, req->path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Generate the value to auto-populate the Alias parameter with
    instance = req->inst->instances[req->inst->order-1];
    USP_SNPRINTF(buf, len, DEFAULT_ALIAS_PREFIX "%d", instance);

    return USP_ERR_OK;
}

