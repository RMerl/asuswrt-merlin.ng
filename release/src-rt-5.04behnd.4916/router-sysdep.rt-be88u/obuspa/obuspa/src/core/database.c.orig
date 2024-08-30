/*
 *
 * Copyright (C) 2019-2021, Broadband Forum
 * Copyright (C) 2016-2021  CommScope, Inc
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
 * \file database.c
 *
 * Implements the API to the USP database (storing parameter values persistently)
 *
 */

#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <errno.h>

#include "common_defs.h"
#include "data_model.h"
#include "database.h"
#include "int_vector.h"
#include "dm_inst_vector.h"
#include "os_utils.h"
#include "text_utils.h"
#include "vendor_api.h"

//--------------------------------------------------------------------
// Prepared SQL statements
typedef enum
{
    kSqlStmt_Get=0,
    kSqlStmt_Set,
    kSqlStmt_Del,
    kSqlStmt_AllEntriesForHash,

    kSqlStmt_Max            // Always last in the enumeration - used to size arrays
} sql_stmt_t;

// Array of handles to prepared statements
static sqlite3_stmt *prepared_stmts[kSqlStmt_Max];

// Array of SQL code to use for prepared statements
static char *prepared_stmt_sql[kSqlStmt_Max] =
{
    "select value from data_model where hash = ?1 and instances = ?2;",           // kSqlStmt_Get
    "insert or replace into data_model(hash,instances,value) values(?1, ?2, ?3);", // kSqlStmt_Set
    "delete from data_model where hash = ?1 and instances = ?2;",                 // kSqlStmt_Del
    "select instances, value from data_model where hash= ?1;"                     // kSqlStmt_AllEntriesForHash
};

//--------------------------------------------------------------------
static sqlite3 *db_handle;      // handle to the USP database

//--------------------------------------------------------------------
// Copy of full filesystem path to database file
char database_filename[128];

//--------------------------------------------------------------------
// SQLite uses -1 as a length argument to denote that the string is zero terminated (so no length needs to be passed)
#define SQLITE_ZERO_TERMINATED (-1)

//--------------------------------------------------------------------
// Set if a factory reset initialisation of the database should potentially be performed in DATABASE_Start()
static bool schedule_factory_reset_init = false;

//--------------------------------------------------------------------
// String, set by '-r' command line option to specify a text file containing the factory reset database parameters
char *factory_reset_text_file = NULL;

//--------------------------------------------------------------------
typedef struct
{
    char *old_path;         // Schema path to migrate from
    char *new_path;         // Schema path to migrate to
    db_hash_t old_hash;     // hash for the old_path, calculated in DATABASE_Start()
    db_hash_t new_hash;     // hash for the new_path, calculated in DATABASE_Start()
} path_migrate_t;

// List of parameters to migrate to new parameter names in the USP DB
path_migrate_t paths_to_migrate[] =
{
#ifndef DISABLE_STOMP
    { "Device.STOMP.Connection.{i}.X_ARRIS-COM_EnableEncryption", "Device.STOMP.Connection.{i}.EnableEncryption", 0, 0},
#endif
};

//--------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int PrepareSQLStatements(void);
int OpenUspDatabase(char *db_file);
void ObfuscatedCopy(unsigned char *dest, unsigned char *src, int len);
int CopyFactoryResetDatabase(char *reset_file, char *db_file);
int ResetFactoryParameters(void);
int ResetFactoryParametersFromFile(char *file);
void LogSQLStatement(char *op, char *path, sqlite3_stmt *stmt);
int CalcPathMigrationHashes(void);
int MigratePath(path_migrate_t *pm);
int GetAllEntriesForParameter(db_hash_t hash, kv_vector_t *kvv);

/*********************************************************************//**
**
** DATABASE_Init
**
** Initialises the database component
**
** \param   db_file - path to file to use for the database
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATABASE_Init(char *db_file)
{
    int err;
    FILE *fp;
    char *factory_reset_file = FACTORY_RESET_FILE;

    // Keep a copy of the database filename, this will be needed when performing a controller initiated factory reset
    USP_STRNCPY(database_filename, db_file, sizeof(database_filename));

    // Perform a factory reset, if no database file exists at the specified location
    fp = fopen(db_file, "r");
    if (fp == NULL)
    {
        // Copy across the factory reset database (if specified)
        if (factory_reset_file[0] != '\0')
        {
            USP_LOG_Info("%s: No database file exists at %s", __FUNCTION__, db_file);
            USP_LOG_Info("%s: Copying from factory reset database (%s)", __FUNCTION__, factory_reset_file);
            err = CopyFactoryResetDatabase(factory_reset_file, db_file);
            if (err != USP_ERR_OK)
            {
                return err;
            }
        }

        // Signal that factory reset initialisation should be performed later (when the data model has been fully registered)
        schedule_factory_reset_init = true;
    }
    else
    {
        fclose(fp);
    }

    // Exit if unable to open the database
    USP_LOG_Info("%s: Opening database %s", __FUNCTION__, db_file);
    err = OpenUspDatabase(db_file);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DATABASE_Start
**
** Starts the database
** Currently this involves potentially performing a programmatic factory reset of the parameters
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATABASE_Start(void)
{
    int i;
    int err;
    path_migrate_t *pm;

    // Calculate the DB hash of each parameter that is to be migrated
    err = CalcPathMigrationHashes();
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Initialise the database with its factory reset parameters (if required)
    if (schedule_factory_reset_init)
    {
#ifdef INCLUDE_PROGRAMMATIC_FACTORY_RESET
        // Initialise using the programmatic method
        err = ResetFactoryParameters();
        if (err != USP_ERR_OK)
        {
            return err;
        }
#endif

        // Initialise using the factory reset text file
        if (factory_reset_text_file != NULL)
        {
            err = ResetFactoryParametersFromFile(factory_reset_text_file);
            if (err != USP_ERR_OK)
            {
                return err;
            }
        }

        schedule_factory_reset_init = false;
    }

    // Migrate all paths that have changed to new DB entries
    for (i=0; i<NUM_ELEM(paths_to_migrate); i++)
    {
        pm = &paths_to_migrate[i];
        (void)MigratePath(pm);  // Intentionally ignoring errors
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DATABASE_Destroy
**
** Cleanly shuts down the database, freeing all memory used by SQLite in the process
**
** \param   None
**
** \return  None
**
**************************************************************************/
void DATABASE_Destroy(void)
{
    int err;
    int i;

    // Iterate over the prepared SQL statements, finalizing them
    for (i=0; i<NUM_ELEM(prepared_stmts); i++)
    {
        err = sqlite3_finalize(prepared_stmts[i]);
        if (err != SQLITE_OK)
        {
            USP_ERR_SQL(db_handle,"sqlite3_finalize");
        }
    }

    // Close the database
    err = sqlite3_close(db_handle);
    if (err != SQLITE_OK)
    {
        USP_LOG_Error("%s: sqlite3_close() failed", __FUNCTION__);
    }

    // Finally shutdown SQLite
    sqlite3_shutdown();
}

/*********************************************************************//**
**
** DATABASE_PerformFactoryReset_ControllerInitiated
**
** Performs a factory reset of the database, ensuring that the reboot cause is set to "RemoteFactoryReset"
** NOTE: This function must only be called when the database is not open (ie after DATABASE_Destroy has been called)
**
** \param   None
**
** \return  None
**
**************************************************************************/
void DATABASE_PerformFactoryReset_ControllerInitiated(void)
{
    char *db_file = database_filename;
    int err;

    // Close the current database
    DATABASE_Destroy();

    // Exit if unable to delete the current database file
    err = remove(db_file);
    if ((err == -1) && (errno != ENOENT))
    {
        USP_ERR_ERRNO("remove", errno);
        return;
    }

    // Copy across the factory reset database (which has reboot cause set to "LocalFactoryReset")
    CopyFactoryResetDatabase(FACTORY_RESET_FILE, db_file);

    // Exit if unable to open the database
    err = OpenUspDatabase(db_file);
    if (err != USP_ERR_OK)
    {
        return;
    }

#ifdef INCLUDE_PROGRAMMATIC_FACTORY_RESET
    // Exit if unable to setup the parameters specified by the vendor
    err = ResetFactoryParameters();
    if (err != USP_ERR_OK)
    {
        return;
    }
#endif

    // Exit if unable to setup the parameters specified in the factory reset text file
    if (factory_reset_text_file != NULL)
    {
        err = ResetFactoryParametersFromFile(factory_reset_text_file);
        if (err != USP_ERR_OK)
        {
            return;
        }
    }

    // Finally set the reboot cause to "RemoteFactoryReset"
    err = DATA_MODEL_SetParameterInDatabase(reboot_cause_path, "RemoteFactoryReset");
    if (err != USP_ERR_OK)
    {
        return;
    }

    // NOTE: No need to close this database, as it will be closed by DM_EXEC_Destroy()
}

/*********************************************************************//**
**
** PrepareSQLStatements
**
** Creates handles for all SQL statements used by the system
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int PrepareSQLStatements(void)
{
    int err;
    int i;
    char *sql;

    // Iterate over the SQL statements, creating prepared statement handles for each statement
    for (i=0; i<NUM_ELEM(prepared_stmt_sql); i++)
    {
        sql = prepared_stmt_sql[i];
        err = sqlite3_prepare_v2(db_handle, sql, SQLITE_ZERO_TERMINATED, &prepared_stmts[i], NULL);
        if (err != SQLITE_OK)
        {
            USP_ERR_SQL(db_handle,"sqlite3_prepare_v2");
            return USP_ERR_INTERNAL_ERROR;
        }
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DATABASE_GetParameterValue
**
** Gets the value of the specified parameter
**
** \param   path - data model path to parameter to get (only used for debug)
** \param   hash - hash identifying the data model parameter to get
** \param   instances - string identifying which instance of the data model parameter to get
**                      If the object is a single instance object, then this parameter should point to an empty string
** \param   buf - pointer to buffer in which to return the value
** \param   buflen - length of buffer in which to return the value
** \param   flags - flags controlling getting the value (eg OBFUSCATED_VALUE)
**
** \return  USP_ERR_OK if successful
**          USP_ERR_OBJECT_DOES_NOT_EXIST if no entry in the database
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int DATABASE_GetParameterValue(char *path, dm_hash_t hash, char *instances, char *buf, int buflen, unsigned flags)
{
    sqlite3_stmt *stmt;
    int value_len;
    const unsigned char *value;
    int err;
    int result = USP_ERR_INTERNAL_ERROR;        // Assume an error

    // Exit if this function is not being called from the data model thread
    if (OS_UTILS_IsDataModelThread(__FUNCTION__, PRINT_WARNING)==false)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Decide which prepared statement to use
    stmt = prepared_stmts[kSqlStmt_Get];

    // Exit if unable to set the value of the hash in the prepared statement
    err = sqlite3_bind_int64(stmt, 1, (db_hash_t)hash);
    if (err != SQLITE_OK)
    {
        USP_ERR_SQL_PARAM(db_handle, "sqlite3_bind_int");
        result = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Exit if unable to set the instance numbers for the parameter
    err = sqlite3_bind_text(stmt, 2, instances, SQLITE_ZERO_TERMINATED, SQLITE_STATIC);
    if (err != SQLITE_OK)
    {
        USP_ERR_SQL_PARAM(db_handle, "sqlite3_bind_text");
        result = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    //LogSQLStatement("GET", path, stmt);

    // Exit if the get failed
    err = sqlite3_step(stmt);
    if (err == SQLITE_DONE)
    {
        // No entry exists (yet) in the database. The data model will use the registered default value.
        // NOTE: Do not set USP error message, as this is not really an error (handled by caller)
        result = USP_ERR_OBJECT_DOES_NOT_EXIST;
        goto exit;
    }
    else if (err != SQLITE_ROW)
    {
        // An error occurred
        USP_ERR_SQL_PARAM(db_handle, "sqlite3_step");
        result = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Determine the length of the value string to copy into the return buffer, truncating it, if it is too long
    value_len = sqlite3_column_bytes(stmt, 0);
    value_len = MIN(value_len, buflen-1);

    // Get a pointer to the data
    value = sqlite3_column_text(stmt, 0);

    // Copy the value into the return buffer
    if ((value != NULL) && (value_len >0))
    {
        if (flags & OBFUSCATED_VALUE)
        {
            // Unobfuscate value
            ObfuscatedCopy((unsigned char *)buf, (unsigned char *)value, value_len);
        }
        else
        {
            // Normal case: value is not obfuscated (or we don't want to return an unobfuscated value)
            memcpy(buf, value, value_len);
        }
        buf[value_len] = '\0'; // Ensure return buffer is always zero terminated
    }
    else
    {
        *buf = '\0';        // Case of value set to NULL in DB
    }

    // If the code gets here, then the parameter has been successfully retrieved from the database
    result = USP_ERR_OK;

exit:
    // Always reset the statement in preparation for next time, even if an error occurred
    err = sqlite3_reset(stmt);
    if (err != SQLITE_OK)
    {
        USP_ERR_SQL_PARAM(db_handle, "sqlite3_reset");
    }

    return result;
}

/*********************************************************************//**
**
** DATABASE_SetParameterValue
**
** Sets the value of the specified parameter
**
** \param   path - data model path to parameter to set (only used for debug)
** \param   hash - hash identifying the data model parameter to set
** \param   instances - string identifying which instance of the data model parameter to set.
**                      If the object is a single instance object, then this parameter should point to an empty string
** \param   new_value - pointer to buffer containing the value to set
** \param   flags - flags controlling setting the value (eg OBFUSCATED_VALUE)
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int DATABASE_SetParameterValue(char *path, dm_hash_t hash, char *instances, char *new_value, unsigned flags)
{
    sqlite3_stmt *stmt;
    int err;
    int result = USP_ERR_INTERNAL_ERROR;        // Assume an error
    char *value_to_bind;
    char obfuscated_value[MAX_DM_SHORT_VALUE_LEN];
    int len;

    // Exit if this function is not being called from the data model thread
    if (OS_UTILS_IsDataModelThread(__FUNCTION__, PRINT_WARNING)==false)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Decide whether to obfuscate the value
    len = strlen(new_value);
    if (flags & OBFUSCATED_VALUE)
    {
        ObfuscatedCopy((unsigned char *)obfuscated_value, (unsigned char *)new_value, len);
        value_to_bind = obfuscated_value;
    }
    else
    {
        value_to_bind = new_value;
    }

    // Exit if unable to set the value of the hash in the prepared statement
    stmt = prepared_stmts[kSqlStmt_Set];
    err = sqlite3_bind_int64(stmt, 1, (db_hash_t)hash);
    if (err != SQLITE_OK)
    {
        USP_ERR_SQL_PARAM(db_handle, "sqlite3_bind_int");
        goto exit;
    }

    // Exit if unable to set the value of the instances in the prepared statement
    err = sqlite3_bind_text(stmt, 2, instances, SQLITE_ZERO_TERMINATED, SQLITE_STATIC);
    if (err != SQLITE_OK)
    {
        USP_ERR_SQL_PARAM(db_handle, "sqlite3_bind_text");
        goto exit;
    }

    // Exit if unable to set the new value of the parameter in the prepared statement
    err = sqlite3_bind_text(stmt, 3, value_to_bind, len, SQLITE_STATIC);
    if (err != SQLITE_OK)
    {
        USP_ERR_SQL_PARAM(db_handle, "sqlite3_bind_text");
        goto exit;
    }

    //LogSQLStatement("SET", path, stmt);

    // Exit if unable to perform the set
    err = sqlite3_step(stmt);
    if (err != SQLITE_DONE)     // We are not expecting any rows
    {
        USP_ERR_SQL_PARAM(db_handle, "sqlite3_step");
        goto exit;
    }

    // If the code gets here, then the parameter has been successfully set in the database
    result = USP_ERR_OK;

exit:
    // Always reset the statement in preparation for next time, even if an error occurred
    err = sqlite3_reset(stmt);
    if ((err != SQLITE_OK) && (result == USP_ERR_OK))
    {
        USP_ERR_SQL_PARAM(db_handle, "sqlite3_reset");
    }

    return result;
}

/*********************************************************************//**
**
** DATABASE_DeleteParameter
**
** Deletes the specified parameter from the database
**
** \param   path - data model path to parameter to delete (only used for debug)
** \param   hash - hash identifying the data model parameter to delete
** \param   instances - string identifying which instance of the data model parameter to delete.
**                      If the object is a single instance object, then this parameter should point to an empty string
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int DATABASE_DeleteParameter(char *path, dm_hash_t hash, char *instances)
{
    sqlite3_stmt *stmt;
    int err;
    int result = USP_ERR_INTERNAL_ERROR;        // Assume an error

    // Exit if this function is not being called from the data model thread
    if (OS_UTILS_IsDataModelThread(__FUNCTION__, PRINT_WARNING)==false)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    stmt = prepared_stmts[kSqlStmt_Del];

    // Exit if unable to set the value of the hash in the prepared statement
    err = sqlite3_bind_int64(stmt, 1, (db_hash_t)hash);
    if (err != SQLITE_OK)
    {
        USP_ERR_SQL_PARAM(db_handle, "sqlite3_bind_int");
        goto exit;
    }

    // Exit if unable to set the value of the instances in the prepared statement
    err = sqlite3_bind_text(stmt, 2, instances, SQLITE_ZERO_TERMINATED, SQLITE_STATIC);
    if (err != SQLITE_OK)
    {
        USP_ERR_SQL_PARAM(db_handle, "sqlite3_bind_text");
        goto exit;
    }

    //LogSQLStatement("DEL", path, stmt);

    // Exit if unable to perform the delete
    // NOTE: If the parameter is not present in the DB, then SQLite still returns OK
    err = sqlite3_step(stmt);
    if (err != SQLITE_DONE)     // We are not expecting any rows
    {
        USP_ERR_SQL_PARAM(db_handle, "sqlite3_step");
        goto exit;
    }

    // If the code gets here, then the parameter has been successfully deleted from the database
    result = USP_ERR_OK;

exit:
    // Always reset the statement in preparation for next time, even if an error occurred
    err = sqlite3_reset(stmt);
    if ((err != SQLITE_OK) && (result == USP_ERR_OK))
    {
        USP_ERR_SQL_PARAM(db_handle, "sqlite3_reset");
    }

    return result;
}

/*********************************************************************//**
**
** DATABASE_StartTransaction
**
** Starts a transaction for subsequent set parameter values
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATABASE_StartTransaction(void)
{
    int err;

    // Exit if this function is not being called from the data model thread
    if (OS_UTILS_IsDataModelThread(__FUNCTION__, PRINT_WARNING)==false)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    err = sqlite3_exec(db_handle, "begin transaction;", NULL, NULL, NULL);
    if (err != SQLITE_OK)
    {
        USP_ERR_SQL(db_handle,"sqlite3_exec");
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DATABASE_CommitTransaction
**
** Commits the current set parameter values transaction
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATABASE_CommitTransaction(void)
{
    int err;

    // Exit if this function is not being called from the data model thread
    if (OS_UTILS_IsDataModelThread(__FUNCTION__, PRINT_WARNING)==false)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    err = sqlite3_exec(db_handle, "commit transaction;", NULL, NULL, NULL);
    if (err != SQLITE_OK)
    {
        USP_ERR_SQL(db_handle,"sqlite3_exec");
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DATABASE_AbortTransaction
**
** Aborts the current set parameter values transaction
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATABASE_AbortTransaction(void)
{
    // Exit if this function is not being called from the data model thread
    if (OS_UTILS_IsDataModelThread(__FUNCTION__, PRINT_WARNING)==false)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Intentionally ignoring errors because if the database has already been rolled back because of an error
    // whilst writing the transactions, then an error will be returned here
    sqlite3_exec(db_handle, "rollback;", NULL, NULL, NULL);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DATABASE_ReadDataModelInstanceNumbers
**
** Reads the instance numbers of all objects in the database, and adds them to the data model
** This function also removes all unknown (not in schema) parameters from the database
**
** \param   remove_unknown_params - set to true if unknown parameters should be cleaned from the database
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int DATABASE_ReadDataModelInstanceNumbers(bool remove_unknown_params)
{
    sqlite3_stmt *stmt;
    int sql_err;
    int err;
    int result = USP_ERR_INTERNAL_ERROR;        // Assume an error
    char *instances;
    dm_hash_t hash;

    // Exit if unable to prepare the SQL statement
    #define SELECT_ALL_INST_STR   "select hash,instances from data_model;"
    sql_err = sqlite3_prepare_v2(db_handle, SELECT_ALL_INST_STR, SQLITE_ZERO_TERMINATED, &stmt, NULL);
    if (sql_err != SQLITE_OK)
    {
        USP_ERR_SQL(db_handle,"sqlite3_prepare_v2");
        return USP_ERR_INTERNAL_ERROR;
    }

    // Iterate over all rows
    sql_err = SQLITE_ROW;
    while (sql_err == SQLITE_ROW)
    {
        sql_err = sqlite3_step(stmt);
        if (sql_err == SQLITE_DONE)
        {
            // Exit loop if we have processed all rows
            result = USP_ERR_OK;
            break;
        }
        else if (sql_err != SQLITE_ROW)
        {
            // An error occurred
            USP_ERR_SQL(db_handle,"sqlite3_step");
            result = USP_ERR_INTERNAL_ERROR;
            break;
        }

        // Determine the hash and the instances string of the parameter in the database
        hash = sqlite3_column_int(stmt, 0);
        instances = (char *)sqlite3_column_text(stmt, 1);
        instances = (instances == NULL) ? "" : instances;   // Ensure that instances variable points to a string

        // Add the object instances (if this parameter has any instances) to the data model
        // NOTE: DATA_MODEL_AddParameterInstances() is called even if we know that the object has no instances,
        //       as we use the return code to delete the parameter if it does not exist in the schema
        err = DATA_MODEL_AddParameterInstances(hash, instances);
        if ((err != USP_ERR_OK) && (remove_unknown_params))
        {
            // Remove this parameter from the database. It is no longer in the data model schema.
            USP_LOG_Warning("Removing unknown parameter (hash=%d, instances='%s') from the database", hash, instances);
            DATABASE_DeleteParameter("Unknown", hash, instances);
        }
    }

    // 'Free' the statement
    sql_err = sqlite3_finalize(stmt);
    if (sql_err != SQLITE_OK)
    {
        USP_ERR_SQL(db_handle,"sqlite3_finalize");
        result = USP_ERR_INTERNAL_ERROR;
    }

    return result;
}

/*********************************************************************//**
**
** DATABASE_GetMigratedHash
**
** Returns a different parameter hash value if the given parameter is to be migrated
**
** \param   hash - hash identifying the data model parameter to see if it needs migrating
**
** \return  hash of the parameter. This will be a different hash if migrated, or the same hash if parameter does not have to be migrated
**
**************************************************************************/
db_hash_t DATABASE_GetMigratedHash(db_hash_t hash)
{
    int i;
    path_migrate_t *pm;

    // Iterate over all paths to migrate, exiting if the specified hash matches one to migrate
    for (i=0; i < NUM_ELEM(paths_to_migrate); i++)
    {
        pm = &paths_to_migrate[i];
        if (pm->old_hash == hash)
        {
            return pm->new_hash;
        }
    }

    // If the code gets here, then the path was not migrated, so return the originally specified hash without modification
    return hash;
}

/*********************************************************************//**
**
** DATABASE_Dump
**
** Logs the contents of the database
**
** \param   None
**
** \return  None
**
**************************************************************************/
void DATABASE_Dump(void)
{
    sqlite3_stmt *stmt;
    int err;
    int result;
    char path[MAX_DM_PATH];
    char *instances;
    char *value;
    dm_hash_t hash;

    // Exit if unable to prepare the SQL statement
    #define SELECT_ALL_STR   "select hash,instances,value from data_model;"
    err = sqlite3_prepare_v2(db_handle, SELECT_ALL_STR, SQLITE_ZERO_TERMINATED, &stmt, NULL);
    if (err != SQLITE_OK)
    {
        USP_ERR_SQL(db_handle,"sqlite3_prepare_v2");
        return;
    }

    // Iterate over all rows
    err = SQLITE_ROW;
    while (err == SQLITE_ROW)
    {
        err = sqlite3_step(stmt);
        if (err == SQLITE_DONE)
        {
            // Exit loop if we have processed all rows
            break;
        }
        else if (err != SQLITE_ROW)
        {
            // An error occurred
            USP_ERR_SQL(db_handle,"sqlite3_step");
            break;
        }

        // Print out this parameter and its value
        hash = sqlite3_column_int(stmt, 0);
        instances = (char *)sqlite3_column_text(stmt, 1);
        value = (char *)sqlite3_column_text(stmt, 2);

        result = DM_PRIV_FormPath_FromDB(hash, instances, path, sizeof(path));
        if (result == USP_ERR_OK)
        {
            USP_DUMP("%s => %s", path, value);
        }
    }

    // Always reset the statement in preparation for next time, even if an error occurred
    err = sqlite3_finalize(stmt);
    if (err != SQLITE_OK)
    {
        USP_ERR_SQL(db_handle,"sqlite3_finalize");
    }
}

/*********************************************************************//**
**
** OpenUspDatabase
**
** Opens the USP database, ensures the table is created in it and the SQL statements prepared
**
** \param   db_file - path to file to use for the database
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int OpenUspDatabase(char *db_file)
{
    int err;

    // Exit if unable to open the database
    err = sqlite3_open(db_file, &db_handle);
    if (err != SQLITE_OK)
    {
        USP_ERR_SQL(db_handle,"sqlite3_open");
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to create the data model parameter table (if it does not already exist)
    #define CREATE_TABLE_STR "create table if not exists data_model (hash integer, instances text, value text, primary key (hash, instances));"
    err = sqlite3_exec(db_handle, CREATE_TABLE_STR, NULL, NULL, NULL);
    if (err != SQLITE_OK)
    {
        USP_ERR_SQL(db_handle,"sqlite3_exec");
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to prepare all SQL statements to be used
    err = PrepareSQLStatements();
    if (err != USP_ERR_OK)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** CopyFactoryResetDatabase
**
** Performs a factory reset of the database, by copying a factory reset database file
** NOTE: The factory reset database will have been generated with the reboot cause set to "LocalFactoryReset"
** NOTE: This function must only be called when the database is not open
**
** \param   reset_file - filename of the file containing the SQLite factory reset database
**                       NOTE: This maybe an empty string or NULL if factory reset is not implemented by a static file
** \param   db_file - filename
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int CopyFactoryResetDatabase(char *reset_file, char *db_file)
{
    #define CHUNK_SIZE 1024             // source and destination files are read and written in byte sized chunks
    unsigned char chunk[CHUNK_SIZE];
    FILE *src = NULL;
    FILE *dest = NULL;
    char buf[128];
    int err;
    int bytes_read;
    int bytes_written;

    // Exit if no file containing the factory reset database has been specified
    // NOTE: This is not an error, as it could be the case if the database is supposed to be generated programatically
    if ((reset_file == NULL) || (*reset_file == '\0'))
    {
        err = USP_ERR_OK;
        goto exit;
    }

    // Exit if unable to open the factory reset file for reading
    src = fopen(reset_file, "r");
    if (src == NULL)
    {
        USP_LOG_Error("%s: Failed to open factory reset database %s for reading: %s", __FUNCTION__, reset_file, USP_ERR_ToString(errno, buf, sizeof(buf)) );
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Exit if unable to open the target database file for writing
    dest = fopen(db_file, "w");
    if (src == NULL)
    {
        USP_LOG_Error("%s: Failed to open destination database %s for writing: %s", __FUNCTION__, db_file, USP_ERR_ToString(errno, buf, sizeof(buf)) );
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Exit if an error occurred whilst reading the first chunk of the source file
    bytes_read = fread(chunk, 1, sizeof(chunk), src);
    if (ferror(src) != 0)
    {
        USP_LOG_Error("%s: Failed to read factory reset database %s", __FUNCTION__, reset_file);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Write the destination file in chunks, exiting the loop when we've reached the end of the source file
    while (bytes_read > 0)
    {
        // Exit if an error occurred whilst writing a chunk of the destination file
        bytes_written = fwrite(chunk, 1, bytes_read, dest);
        if (bytes_written != bytes_read)
        {
            USP_LOG_Error("%s: Failed to write destination database %s", __FUNCTION__, db_file);
            err = USP_ERR_INTERNAL_ERROR;
            goto exit;
        }

        // Exit if an error occurred whilst reading a chunk of the source file
        bytes_read = fread(chunk, 1, sizeof(chunk), src);
        if (ferror(src) != 0)
        {
            USP_LOG_Error("%s: Failed to read factory reset database %s", __FUNCTION__, reset_file);
            err = USP_ERR_INTERNAL_ERROR;
            goto exit;
        }
    }

    // If the code gets here, then the factory reset database has been copied successfully
    err = USP_ERR_OK;

exit:
    if (dest != NULL)
    {
        fclose(dest);
    }

    if (src != NULL)
    {
        fclose(src);
    }

    return err;
}

#ifdef INCLUDE_PROGRAMMATIC_FACTORY_RESET
/*********************************************************************//**
**
** ResetFactoryParameters
**
** Sets the parameters specified by VENDOR_GetFactoryResetParams()
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ResetFactoryParameters(void)
{
    int i;
    int err;
    kv_vector_t params;
    kv_pair_t *kv;

    // Exit if unable to get the factory reset parameters
    KV_VECTOR_Init(&params);
    err = VENDOR_GetFactoryResetParams(&params);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    USP_LOG_Info("%s: Setting factory reset parameters", __FUNCTION__);

    // Set all factory reset parameters provided by the vendor in the database
    for (i=0; i<params.num_entries; i++)
    {
        kv = &params.vector[i];

        // Exit if a parameter was not in the data model or failed to set
        err = DATA_MODEL_SetParameterInDatabase(kv->key, kv->value);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }
    }

exit:
    // Ensure that the parameters signalled by the vendor are freed
    KV_VECTOR_Destroy(&params);

    return err;
}
#endif // INCLUDE_PROGRAMMATIC_FACTORY_RESET

/*********************************************************************//**
**
** ResetFactoryParametersFromFile
**
** Sets the data model parameters specified in the file
**
** \param   file - name of file containing parameters to set
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ResetFactoryParametersFromFile(char *file)
{
    FILE *fp;
    char *result;
    char buf[MAX_DM_PATH + MAX_DM_VALUE_LEN + 4]; // Plus 4 to allow for separator, line ending characters and NULL terminator
    int err;
    char *key;
    char *value;
    int line_number = 1;

    USP_LOG_Info("%s: Setting factory reset parameters", __FUNCTION__);

    // Exit if unable to open the file containing factory reset parameters
    fp = fopen(file, "r");
    if (fp == NULL)
    {
        USP_LOG_Error("%s: Failed to open factory reset file (%s)", __FUNCTION__, file);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Iterate over all lines in the file
    result = fgets(buf, sizeof(buf), fp);
    while (result != NULL)
    {
        // Exit if an error occurred when parsing this line
        err = TEXT_UTILS_KeyValueFromString(buf, &key, &value);
        if (err != USP_ERR_OK)
        {
            USP_LOG_Error("%s: Syntax error in %s at line %d", __FUNCTION__, file, line_number);
            goto exit;
        }

        // Set the parameter (if the line was not blank or a comment)
        if ((key != NULL) & (value != NULL))
        {
            err = DATA_MODEL_SetParameterInDatabase(key, value);
            if (err != USP_ERR_OK)
            {
                USP_LOG_Error("%s: Failed to set parameter at line %d of %s", __FUNCTION__, line_number, file);
                goto exit;
            }
        }

        // Get the next line
        line_number++;
        result = fgets(buf, sizeof(buf), fp);
    }

    // If the code gets here, then all parameters in the file have been set successfully
    err = USP_ERR_OK;

exit:
    fclose(fp);
    return err;
}

/*********************************************************************//**
**
** LogSQLStatement
**
** Logs the SQL Statement used for the specified operation
** This is useful for debugging purposes
**
** \param   op - string representing the operation being performed
** \param   path - data model path involved in the operation
** \param   stmt - prepared statement with bound values
**
** \return  None
**
**************************************************************************/
void LogSQLStatement(char *op, char *path, sqlite3_stmt *stmt)
{
#if SQLITE_VERSION_NUMBER >= 3014000
    char *sql_str;

    sql_str = sqlite3_expanded_sql(stmt);
    USP_LOG_Info("%s(%s): %s", op, path, sql_str);
    sqlite3_free(sql_str);
#endif // SQLITE_VERSION_NUMBER

    // Ensure that compiler does not complain about unused arguments if SQLITE_VERSION_NUMBER < 3014000
    (void)op;
    (void)path;
    (void)stmt;
}

/*********************************************************************//**
**
** ObfuscatedCopy
**
** Copies the specified source value into the destination buffer,
** whilst obfuscating/unobfuscating it
** NOTE: Obfuscation may result in embedded NULLs within the destination, or
**       be the cause of embedded NULLs in the src
**
** \param   dest - pointer to buffer in which to copy the unobfuscated value
** \param   src - pointer to value to unobfuscate
** \param   len - length of value
**
** \return  None
**
**************************************************************************/
void ObfuscatedCopy(unsigned char *dest, unsigned char *src, int len)
{
    int i;

    unsigned char *key = (unsigned char *) PASSWORD_OBFUSCATION_KEY;
    int key_len;

    key_len = strlen((char *) key);
    for (i=0; i<len; i++)
    {
        *dest = (*src) ^ key[ i % key_len ];
        dest++;
        src++;
    }
}

/*********************************************************************//**
**
** CalcPathMigrationHashes
**
** Populates the paths_to_migrate[] structure with the hashes of the old and new schema paths
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int CalcPathMigrationHashes(void)
{
    int i;
    path_migrate_t *pm;
    dm_node_t *node;
    dm_hash_t old_hash;
    int err;

    for (i=0; i < NUM_ELEM(paths_to_migrate); i++)
    {
        pm = &paths_to_migrate[i];

        // Exit if unable to obtain the hash of the legacy parameter
        // NOTE: The legacy parameter does not have to be present in the data model
        err = DM_PRIV_CalcHashFromPath(pm->old_path, NULL, &old_hash);
        if (err != USP_ERR_OK)
        {
            USP_LOG_Error("%s: Legacy schema path '%s' incorrect in paths_to_migrate[%d] (%s)", __FUNCTION__, pm->old_path, i, USP_ERR_GetMessage());
            return USP_ERR_INVALID_PATH;
        }
        pm->old_hash = (db_hash_t) old_hash;

        // Exit if unable to obtain the hash of the new parameter
        // NOTE: This also checks that the new parameter is present in the data model
        node = DM_PRIV_GetNodeFromPath(pm->new_path, NULL, NULL);
        if (node == NULL)
        {
            USP_LOG_Error("%s: new schema path '%s' in paths_to_migrate[%d] is not present in the data model", __FUNCTION__, pm->new_path, i);
            return USP_ERR_INVALID_PATH;
        }
        pm->new_hash = (db_hash_t) node->hash;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** MigratePath
**
** Migrates all entries in the DB of the specified schema path to the new schema path
**
** \param   pm - pointer to entry in the paths_to_migrate[] array specifyting the migration information
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int MigratePath(path_migrate_t *pm)
{
    int i;
    int err;
    kv_vector_t kvv;
    kv_pair_t *kv;
    char *instances;
    dm_instances_t inst;
    char old_param[MAX_DM_PATH];
    char new_param[MAX_DM_PATH];

    // Exit if unable to get all entries for the legacy parameter
    err = GetAllEntriesForParameter(pm->old_hash, &kvv);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Iterate over all entries, replacing the old entry with the new entry
    for (i=0; i < kvv.num_entries; i++)
    {
        kv = &kvv.vector[i];
        instances = kv->key;

        // Exit if unable to parse the instance numbers from the string
        memset(&inst, 0, sizeof(inst));
        err = DM_PRIV_ParseInstanceString(instances, &inst);
        if (err != USP_ERR_OK)
        {
            USP_LOG_Error("%s: Instance numbers ('%s') for hash=%d are invalid", __FUNCTION__, instances, pm->old_hash);
            return USP_ERR_INTERNAL_ERROR;
        }

        // Skip if unable to form old instantiated path of this entry
        // NOTE: This will only occur if the number of instance numbers in the database do not match the number in the schema path
        err = DM_PRIV_FormInstantiatedPath(pm->old_path, &inst, old_param, sizeof(old_param));
        if (err != USP_ERR_OK)
        {
            USP_LOG_Error("%s: Skipping migrating %s for instances %s (%s)", __FUNCTION__, pm->old_path, instances, USP_ERR_GetMessage());
            continue;
        }

        // Skip if unable to form new instantiated path of this entry
        // NOTE: This will only occur if the number of instance numbers in the database do not match the number in the schema path
        err = DM_PRIV_FormInstantiatedPath(pm->new_path, &inst, new_param, sizeof(new_param));
        if (err != USP_ERR_OK)
        {
            USP_LOG_Error("%s: Skipping migrating to %s for instances %s (%s)", __FUNCTION__, pm->new_path, instances, USP_ERR_GetMessage());
            continue;
        }
        USP_LOG_Info("%s: Migrating '%s' to '%s'", __FUNCTION__, old_param, new_param);

        // Add the new entry first, so that if it fails, the old entry will still be present in the DB
        err = DATABASE_SetParameterValue(new_param, pm->new_hash, instances, kv->value, 0);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }

        // Exit if unable to delete legacy entry
        err = DATABASE_DeleteParameter(old_param, pm->old_hash, instances);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }
    }

exit:
    KV_VECTOR_Destroy(&kvv);
    return err;
}

/*********************************************************************//**
**
** GetAllEntriesForParameter
**
** Gets the value of all instantiated parameters for the path specified by hash
**
** \param   hash - hash identifying the data model parameter to get
** \param   kvv - pointer to key value vector in which to return all instances (key) and values (value) of the specified parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetAllEntriesForParameter(db_hash_t hash, kv_vector_t *kvv)
{
    sqlite3_stmt *stmt;
    int sql_err;
    int err;
    int result = USP_ERR_INTERNAL_ERROR;        // Assume an error
    char *instances;
    char *value;

    // Set default return values
    KV_VECTOR_Init(kvv);

    // Decide which prepared statement to use
    stmt = prepared_stmts[kSqlStmt_AllEntriesForHash];

    // Exit if unable to set the value of the hash in the statement
    err = sqlite3_bind_int64(stmt, 1, hash);
    if (err != SQLITE_OK)
    {
        USP_ERR_SQL_PARAM(db_handle, "sqlite3_bind_int");
        return USP_ERR_INTERNAL_ERROR;
    }

    // Iterate over all instances of the specified parameter
    sql_err = SQLITE_ROW;
    while (sql_err == SQLITE_ROW)
    {
        sql_err = sqlite3_step(stmt);
        if (sql_err == SQLITE_DONE)
        {
            // Exit loop if we have processed all rows
            result = USP_ERR_OK;
            break;
        }
        else if (sql_err != SQLITE_ROW)
        {
            // An error occurred
            USP_ERR_SQL(db_handle,"sqlite3_step");
            result = USP_ERR_INTERNAL_ERROR;
            break;
        }

        // Skip this row if no instances or value found
        // NOTE: This should never happen if the software is working correctly
        instances = (char *)sqlite3_column_text(stmt, 0);
        value = (char *)sqlite3_column_text(stmt, 1);
        if ((instances == NULL) || (value == NULL))
        {
            continue;
        }

        // Add this entry to the key-value vector
        KV_VECTOR_Add(kvv, instances, value);
    }

    // Always reset the statement in preparation for next time, even if an error occurred
    err = sqlite3_reset(stmt);
    if (err != SQLITE_OK)
    {
        USP_ERR_SQL_PARAM(db_handle, "sqlite3_reset");
    }

    return result;
}

