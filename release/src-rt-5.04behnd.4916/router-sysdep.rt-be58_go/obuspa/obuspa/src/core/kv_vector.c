/*
 *
 * Copyright (C) 2019-2022, Broadband Forum
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
 * \file kv_vector.c
 *
 * Implements a data structure containing a vector of a key-value pairs
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common_defs.h"
#include "kv_vector.h"
#include "text_utils.h"
#include "iso8601.h"


//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void AddKeyValueInternal(kv_vector_t *kvv, char *key, char *value);
int FindMatchingKey(char *key, char **valid_keys, int num_valid_keys);

/*********************************************************************//**
**
** KV_VECTOR_Init
**
** Initialises a key-value pair vector structure
**
** \param   kvv - pointer to structure to initialise
**
** \return  None
**
**************************************************************************/
void KV_VECTOR_Init(kv_vector_t *kvv)
{
    kvv->vector = NULL;
    kvv->num_entries = 0;
}

/*********************************************************************//**
**
** KV_VECTOR_Add
**
** Adds a key value pair into the vector, where the value is specified as a string
**
** \param   kvv - pointer to structure to add the string to
** \param   key - pointer to string to copy
** \param   value - pointer to string to copy
**
** \return  None
**
**************************************************************************/
void KV_VECTOR_Add(kv_vector_t *kvv, char *key, char *value)
{
    AddKeyValueInternal(kvv, USP_STRDUP(key), USP_STRDUP(value));
}

/*********************************************************************//**
**
** KV_VECTOR_Replace
**
** Replaces all values associated with the specified key
**
** \param   kvv - pointer to structure to replace the value in
** \param   key - pointer to key, whose value we want to replace
** \param   value - pointer to replacement value
**
** \return  true if the value was replaced, false if the key does not exist in the vector
**
**************************************************************************/
bool KV_VECTOR_Replace(kv_vector_t *kvv, char *key, char *value)
{
    int i;
    kv_pair_t *pair;
    bool is_replaced = false;

    // Iterate over all entries, replacing the value of all matching keys
    for (i=0; i < kvv->num_entries; i++)
    {
        pair = &kvv->vector[i];
        if (strcmp(pair->key, key)==0)
        {
            // Found a matching key, so replace its value
            USP_SAFE_FREE( pair->value );
            pair->value = USP_STRDUP(value);
            is_replaced = true;
        }
    }

    // If the code gets here, then no match was found
    return is_replaced;
}

/*********************************************************************//**
**
** KV_VECTOR_ReplaceWithHint
**
** Replaces the first value associated with the specified key at the
** suspected location in the vector given by hint
**
** \param   kvv - pointer to structure to replace the value in
** \param   key - pointer to key, whose value we want to replace
** \param   value - pointer to replacement value
** \param   hint - index of entry in key value vector at which the key is expected to be located
**
** \return  true if the value was replaced, false if the key does not exist in the vector
**
**************************************************************************/
bool KV_VECTOR_ReplaceWithHint(kv_vector_t *kvv, char *key, char *value, int hint)
{
    int index;
    kv_pair_t *pair;

    // Exit if unable to find the matching key (starting at the hint index)
    index = KV_VECTOR_FindKey(kvv, key, hint);
    if (index == INVALID)
    {
        return false;
    }

    // Found a matching key, so replace its value
    pair = &kvv->vector[index];
    USP_SAFE_FREE( pair->value );
    pair->value = USP_STRDUP(value);

    return true;
}

/*********************************************************************//**
**
** KV_VECTOR_AddUnsigned
**
** Adds a key value pair into the vector, where the value is specified as an unsigned number
**
** \param   kvv - pointer to structure to add the string to
** \param   key - pointer to string to copy
** \param   value - value to convert to a string and add to the vector
**
** \return  None
**
**************************************************************************/
void KV_VECTOR_AddUnsigned(kv_vector_t *kvv, char *key, unsigned value)
{
    char buf[32];

    USP_SNPRINTF(buf, sizeof(buf), "%u", value);
    KV_VECTOR_Add(kvv, key, buf);
}

/*********************************************************************//**
**
** KV_VECTOR_AddBool
**
** Adds a key value pair into the vector, where the value is specified as a boolean
**
** \param   kvv - pointer to structure to add the string to
** \param   key - pointer to string to copy
** \param   value - value to convert to a string and add to the vector
**
** \return  None
**
**************************************************************************/
void KV_VECTOR_AddBool(kv_vector_t *kvv, char *key, bool value)
{
    char *str;

    str = TEXT_UTILS_BoolToString(value);
    KV_VECTOR_Add(kvv, key, str);
}

/*********************************************************************//**
**
** KV_VECTOR_AddDateTime
**
** Adds a key value pair into the vector, where the value is specified as a date-time
**
** \param   kvv - pointer to structure to add the string to
** \param   key - pointer to string to copy
** \param   value - value to convert to a string and add to the vector
**
** \return  None
**
**************************************************************************/
void KV_VECTOR_AddDateTime(kv_vector_t *kvv, char *key, time_t value)
{
    char buf[MAX_ISO8601_LEN];

    iso8601_from_unix_time(value, buf, sizeof(buf));
    KV_VECTOR_Add(kvv, key, buf);
}

/*********************************************************************//**
**
** KV_VECTOR_AddEnum
**
** Adds a key value pair into the vector, where the value is specified as an enum
**
** \param   kvv - pointer to key-value pair vector structure
** \param   key - pointer to name of key to get the value of
** \param   value - pointer to variable in which to return the converted value
** \param   default_value - value to return if the specified parameter was not present in the vector
** \param   enums - pointer to conversion table containing a list of enumerations and their associated string representation
** \param   num_enums - number of enumerations in the table
**
** \return  None
**
**************************************************************************/
void KV_VECTOR_AddEnum(kv_vector_t *kvv, char *key, int value, const enum_entry_t *enums, int num_enums)
{
    char *str;

    str = TEXT_UTILS_EnumToString(value, enums, num_enums);

    KV_VECTOR_Add(kvv, key, str);
}

/*********************************************************************//**
**
** KV_VECTOR_AddHexNumber
**
** Adds a key value pair into the vector, where the value is specified as a binary array
**
** \param   kvv - pointer to key-value pair vector structure
** \param   key - pointer to name of key to get the value of
** \param   buf - pointer to buffer to convert to hex numbers
** \param   len - length of the buffer
**
** \return  USP_ERR_OK if successful.
**
**************************************************************************/
void KV_VECTOR_AddHexNumber(kv_vector_t *kvv, char *key, unsigned char *buf, int len)
{
    char *value;
    char *p;
    int i;

    // Allocate memory for a string to store the hex-ascii value in
    value = USP_MALLOC(len*2+1);        // *2 to convert to hex-ascii format, +1 to allow for NULL terminator

    // Iterate over all bytes in the buffer, converting them to hex-ascii numbers
    p = value;
    for (i=0; i<len; i++)
    {
        *p++ = TEXT_UTILS_ValueToHexDigit((buf[i] & 0xF0) >> 4, USE_UPPERCASE_HEX_DIGITS);
        *p++ = TEXT_UTILS_ValueToHexDigit(buf[i] & 0x0F, USE_UPPERCASE_HEX_DIGITS);
    }
    *p = '\0';

    AddKeyValueInternal(kvv, USP_STRDUP(key), value);
}

/*********************************************************************//**
**
** KV_VECTOR_Destroy
**
** Deallocates all memory associated with the key-value pair vector
**
** \param   kvv - pointer to structure to destroy all dynamically allocated memory it contains
**
** \return  None
**
**************************************************************************/
void KV_VECTOR_Destroy(kv_vector_t *kvv)
{
    int i;
    kv_pair_t *pair;

    // Exit if vector is already empty
    USP_ASSERT(kvv != NULL);
    if (kvv->vector == NULL)
    {
        goto exit;
    }

    // Free all strings in the vector
    for (i=0; i < kvv->num_entries; i++)
    {
        pair = &kvv->vector[i];
        USP_FREE( pair->key );
        USP_FREE( pair->value );
    }

    // Free the vector itself
    USP_FREE(kvv->vector);

exit:
    // Ensure structure is re-initialised
    kvv->vector = NULL;
    kvv->num_entries = 0;
}

/*********************************************************************//**
**
** KV_VECTOR_Dump
**
** Logs all key-value pairs in the vector
**
** \param   kvv - pointer to key-value pair vector structure
**
** \return  None
**
**************************************************************************/
void KV_VECTOR_Dump(kv_vector_t *kvv)
{
    int i;
    kv_pair_t *pair;

    for (i=0; i < kvv->num_entries; i++)
    {
        pair = &kvv->vector[i];
        USP_DUMP("%s => %s", pair->key, pair->value);
    }
}

/*********************************************************************//**
**
** KV_VECTOR_FindKey
**
** Finds the index of the specified key in the specified key-value pair vector
**
** \param   kvv - pointer to key-value pair vector structure
** \param   key - pointer to key to lookup the associated value of
** \param   start_index - hint at which to start the search in the vector looking for the matching key
**                        All entries in the vector will be searched if no keys match
**
** \return  index of matching key-value pair in the vector or INVALID if no match was found
**
**************************************************************************/
int KV_VECTOR_FindKey(kv_vector_t *kvv, char *key, int start_index)
{
    int i;
    kv_pair_t *pair;

    // Ensure that hint is within range of the array
    if ((start_index < 0) || (start_index > kvv->num_entries))
    {
        start_index = 0;
    }

    // Iterate from start index to end of array
    for (i=start_index; i < kvv->num_entries; i++)
    {
        pair = &kvv->vector[i];
        if (strcmp(pair->key, key)==0)
        {
            return i;
        }
    }

    // Iterate from 0 to just before start_index
    for (i=0; i < start_index; i++)
    {
        pair = &kvv->vector[i];
        if (strcmp(pair->key, key)==0)
        {
            return i;
        }
    }

    // If the code gets here, then no match was found
    return INVALID;
}

/*********************************************************************//**
**
** KV_VECTOR_Get
**
** Returns a pointer to the value associated with the specified key or the specified default value
**
** \param   kvv - pointer to key-value pair vector structure
** \param   key - pointer to name of key to get the value of
** \param   default_value - pointer to default value to return (this could be NULL if we want a return indicator that the key did not exist)
** \param   start_index - hint at which to start the search in the vector looking for the matching key
**                        All entries in the vector will be searched if no keys match
**
** \return  pointer to value
**
**************************************************************************/
char *KV_VECTOR_Get(kv_vector_t *kvv, char *key, char *default_value, int start_index)
{
    int index;

    // Exit, returning default string, if unable to find key
    index = KV_VECTOR_FindKey(kvv, key, start_index);
    if (index == INVALID)
    {
        return default_value;
    }

    return kvv->vector[index].value;
}

/*********************************************************************//**
**
** KV_VECTOR_GetUnsigned
**
** Gets the value of the specified parameter from the vector as an unsigned integer
**
** \param   kvv - pointer to key-value pair vector structure
** \param   key - pointer to name of key to get the value of
** \param   default_value - default value, if not present in the vector
** \param   value - pointer to variable in which to return the value
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INVALID_TYPE if unable to convert the key's value (given in the vector) to an unsigned integer
**
**************************************************************************/
int KV_VECTOR_GetUnsigned(kv_vector_t *kvv, char *key, unsigned default_value, unsigned *value)
{
    int index;
    int err;
    char *str_value;

    // Exit, returning default value, if unable to find key
    index = KV_VECTOR_FindKey(kvv, key, 0);
    if (index == INVALID)
    {
        *value = default_value;
        return USP_ERR_OK;
    }

    // Exit if the key's value could not be converted
    str_value = kvv->vector[index].value;
    err = TEXT_UTILS_StringToUnsigned(str_value, value);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Illegal value (%s) in argument %s", __FUNCTION__, str_value, key);
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** KV_VECTOR_GetUnsignedWihinRange
**
** Gets the value of the specified parameter from the vector as an unsigned integer,
** checking that it is within the specified range
**
** \param   kvv - pointer to key-value pair vector structure
** \param   key - pointer to name of key to get the value of
** \param   default_value - default value, if not present in the vector
** \param   min - minimum allowed value
** \param   min - maximum allowed value
** \param   value - pointer to variable in which to return the value
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INVALID_TYPE if unable to convert the key's value (given in the vector) to an unsigned integer
**          USP_ERR_INVALID_VALUE if value is out of range
**
**************************************************************************/
int KV_VECTOR_GetUnsignedWithinRange(kv_vector_t *kvv, char *key, unsigned default_value, unsigned min, unsigned max, unsigned *value)
{
    int err;

    // Exit if unable to convert the value
    err = KV_VECTOR_GetUnsigned(kvv, key, default_value, value);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if value is not in range
    if ((*value < min) || (*value > max))
    {
        USP_ERR_SetMessage("%s: Input argument (%s=%u) is out of range [%u:%u]", __FUNCTION__, key, *value, min, max);
        return USP_ERR_INVALID_VALUE;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** KV_VECTOR_GetInt
**
** Gets the value of the specified parameter from the vector as an integer
**
** \param   kvv - pointer to key-value pair vector structure
** \param   key - pointer to name of key to get the value of
** \param   default_value - default value, if not present in the vector
** \param   value - pointer to variable in which to return the value
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INVALID_TYPE if unable to convert the key's value (given in the vector) to an integer
**
**************************************************************************/
int KV_VECTOR_GetInt(kv_vector_t *kvv, char *key, int default_value, int *value)
{
    int index;
    int err;
    char *str_value;

    // Exit, returning default value, if unable to find key
    index = KV_VECTOR_FindKey(kvv, key, 0);
    if (index == INVALID)
    {
        *value = default_value;
        return USP_ERR_OK;
    }

    // Exit if the key's value could not be converted
    str_value = kvv->vector[index].value;
    err = TEXT_UTILS_StringToInteger(str_value, value);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Illegal value (%s) in argument %s", __FUNCTION__, str_value, key);
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** KV_VECTOR_GetIntWithinRange
**
** Gets the value of the specified parameter from the vector as an integer,
** checking that it is within the specified range
**
** \param   kvv - pointer to key-value pair vector structure
** \param   key - pointer to name of key to get the value of
** \param   default_value - default value, if not present in the vector
** \param   min - minimum allowed value
** \param   min - maximum allowed value
** \param   value - pointer to variable in which to return the value
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INVALID_TYPE if unable to convert the key's value (given in the vector) to an integer
**          USP_ERR_INVALID_VALUE if value is out of range
**
**************************************************************************/
int KV_VECTOR_GetIntWithinRange(kv_vector_t *kvv, char *key, int default_value, int min, int max, int *value)
{
    int err;

    // Exit if unable to convert the value
    err = KV_VECTOR_GetInt(kvv, key, default_value, value);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if value is not in range
    if ((*value < min) || (*value > max))
    {
        USP_ERR_SetMessage("%s: Input argument (%s=%u) is out of range [%u:%u]", __FUNCTION__, key, *value, min, max);
        return USP_ERR_INVALID_VALUE;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** KV_VECTOR_GetBool
**
** Gets the value of the specified parameter from the vector as a bool
**
** \param   kvv - pointer to key-value pair vector structure
** \param   key - pointer to name of key to get the value of
** \param   default_value - default value, if not present in the vector
** \param   value - pointer to variable in which to return the value
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int KV_VECTOR_GetBool(kv_vector_t *kvv, char *key, bool default_value, bool *value)
{
    int index;
    int err;
    char *str_value;

    // Exit, returning default value, if unable to find key
    index = KV_VECTOR_FindKey(kvv, key, 0);
    if (index == INVALID)
    {
        *value = default_value;
        return USP_ERR_OK;
    }

    // Exit if the key's value could not be converted
    str_value = kvv->vector[index].value;
    err = TEXT_UTILS_StringToBool(str_value, value);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Illegal value (%s) in argument %s", __FUNCTION__, str_value, key);
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** KV_VECTOR_GetDateTime
**
** Gets the value of the specified parameter from the vector as a time_t
**
** \param   kvv - pointer to key-value pair vector structure
** \param   key - pointer to name of key to get the value of
** \param   default_value - default value, if not present in the vector
** \param   value - pointer to variable in which to return the value
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int KV_VECTOR_GetDateTime(kv_vector_t *kvv, char *key, char *default_value, time_t *value)
{
    int index;
    int err;
    char *str_value;

    // Get the value of the key, or default the value if not present in the vector
    index = KV_VECTOR_FindKey(kvv, key, 0);
    str_value = (index == INVALID) ? default_value : kvv->vector[index].value;

    // Exit if the key's value could not be converted
    err = TEXT_UTILS_StringToDateTime(str_value, value);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Illegal value (%s) in argument %s", __FUNCTION__, str_value, key);
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** KV_VECTOR_GetHexNumber
**
** Gets the value of the specified parameter from the vector as a binary format sequence
**
** \param   kvv - pointer to key-value pair vector structure
** \param   key - pointer to name of key to get the value of
** \param   buf - pointer to buffer in which to write the binary data
** \param   len - length of the buffer
** \param   bytes_copied - pointer to variable in which to return the number of bytes written into the buffer
**
** \return  USP_ERR_OK if successful.
**
**************************************************************************/
int KV_VECTOR_GetHexNumber(kv_vector_t *kvv, char *key, unsigned char *buf, int len, int *bytes_copied)
{
    int index;
    int err;
    char *str_value;

    // Exit if key is not present, return the default values
    index = KV_VECTOR_FindKey(kvv, key, 0);
    if (index == INVALID)
    {
        memset(buf, 0, len);
        *bytes_copied = 0;
        return USP_ERR_OK;
    }

    // Exit if the string could not be converted, or was too long for the buffer
    str_value = kvv->vector[index].value;
    err = TEXT_UTILS_StringToBinary(str_value, buf, len, bytes_copied);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** KV_VECTOR_GetEnum
**
** Gets the value of the specified parameter from the vector as an enumeration
**
** \param   kvv - pointer to key-value pair vector structure
** \param   key - pointer to name of key to get the value of
** \param   value - pointer to variable in which to return the converted value
** \param   default_value - value to return if the specified parameter was not present in the vector
** \param   enums - pointer to conversion table containing a list of enumerations and their associated string representation
** \param   num_enums - number of enumerations in the table
**
** \return  Enumerated value or INVALID if unable to convert
**
**************************************************************************/
int KV_VECTOR_GetEnum(kv_vector_t *kvv, char *key, void *value, int default_value, const enum_entry_t *enums, int num_enums)
{
    int index;
    char *str_value;
    int enum_value;

    // Exit if key is not present, return the default value
    index = KV_VECTOR_FindKey(kvv, key, 0);
    if (index == INVALID)
    {
        *((int *)value) = default_value;
        return USP_ERR_OK;
    }

    // Exit if the string did not represent a valid enumeration
    str_value = kvv->vector[index].value;
    enum_value = TEXT_UTILS_StringToEnum(str_value, enums, num_enums);
    if (enum_value == INVALID)
    {
        USP_ERR_SetMessage("%s: Illegal value (%s) in argument %s", __FUNCTION__, str_value, key);
        return USP_ERR_INVALID_VALUE;
    }

    // Save the return value
    *((int *)value) = enum_value;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** KV_VECTOR_ValidateArguments
**
** Called to validate the keys (which contain argument names) against the specified schema
** This function is called to validate input and output arguments against the schema for those arguments, registered in the data model
** It also ensures that all argument names are unique
**
** \param   args - pointer to key-value vector containing the argument names in the key (may contain instance numbers)
** \param   buf - pointer to string vector containing the schema for the argument names
** \param   flags - bitmask of flags controlling operation of this function e.g. IGNORE_UNKNOWN_ARGS
**
** \return  USP_ERR_OK if argument names validate successfully
**
**************************************************************************/
int KV_VECTOR_ValidateArguments(kv_vector_t *args, str_vector_t *expected_schema, unsigned flags)
{
    int i;
    int index;
    char converted_path[MAX_DM_PATH];
    kv_pair_t *kv;

    // Iterate over all keys in the vector, seeing if each one has any duplicates
    // NOTE: we do not have to check the last key for duplicates, as it will not be a duplicate if none of the preceeding keys have matched it
    for (i=0; i< args->num_entries-1; i++)
    {
        kv = &args->vector[i];
        index = KV_VECTOR_FindKey(args, kv->key, i+1);
        if (index != i)
        {
            USP_ERR_SetMessage("%s: Duplicate input argument (%s) found", __FUNCTION__, kv->key);
            return USP_ERR_INVALID_ARGUMENTS;
        }
    }

    // Exit if the caller does not care about arguments which are not present in the data model schema
    // This case is used to graciously handle the case of the agent supporting an older version of the data model
    // than the controller (in the case of the USP data model being upgraded with extra arguments).
    if (flags & IGNORE_UNKNOWN_ARGS)
    {
        return USP_ERR_OK;
    }

    // Iterate over all args, checking that it matches a name registered in the schema
    for (i=0; i < args->num_entries; i++)
    {
        // Convert the argument to its schema form
        kv = &args->vector[i];
        TEXT_UTILS_PathToSchemaForm(kv->key, converted_path, sizeof(converted_path));

        // Check if the schema form of the argument is present in the expected schema
        if (STR_VECTOR_Find(expected_schema, converted_path) == INVALID)
        {
            USP_ERR_SetMessage("%s: Argument %s is not present in the data model schema", __FUNCTION__, kv->key);
            return USP_ERR_INVALID_ARGUMENTS;
        }
    }

    // If the code gets here, then all argument names match the schema
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** FindMatchingKey
**
** Finds wther the specified key matches any in the supplied array of valid keys
**
** \param   key - key to find in the array of valid keys
** \param   valid_keys - pointer to array of valid keys
** \param   num_valid_keys - number of keys in the valid keys array
**
** \return  USP_ERR_OK if the keys are valid
**
**************************************************************************/
int FindMatchingKey(char *key, char **valid_keys, int num_valid_keys)
{
    int i;

    // Iterate over the array of valid keys
    for (i=0; i < num_valid_keys; i++)
    {
        // Exit if the key has been found
        if (strcmp(key, valid_keys[i]) == 0)
        {
            return i;
        }
    }

    // If the code gets here, then no match was found
    return INVALID;
}

/*********************************************************************//**
**
** AddKeyValueInternal
**
** Adds a key value pair into the vector, where the key and value have already been dynamically allocated by the caller
**
** \param   kvv - pointer to structure to add the string to
** \param   key - pointer to string to attach
** \param   value - pointer to string to attach
**
** \return  None
**
**************************************************************************/
void AddKeyValueInternal(kv_vector_t *kvv, char *key, char *value)
{
    int new_num_entries;
    kv_pair_t *pair;

    new_num_entries = kvv->num_entries + 1;
    kvv->vector = USP_REALLOC(kvv->vector, new_num_entries*sizeof(kv_pair_t));

    pair = &kvv->vector[ kvv->num_entries ];
    pair->key = key;
    pair->value = value;

    kvv->num_entries = new_num_entries;
}

