/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2016-2024  CommScope, Inc
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
 * \file database.h
 *
 * Header file containing the API to access the USP Database
 *
 */
#ifndef DATABASE_H
#define DATABASE_H
#include "data_model.h"

//------------------------------------------------------------------------------
// Defines for bits in flag variable used by DATABASE_GetParameterValue() and DATABASE_SetParameterValue()
#define OBFUSCATED_VALUE 0x00000001

//------------------------------------------------------------------------------
// String, set by '-r' command line option to specify a text file containing the factory reset database parameters
extern char *factory_reset_text_file;

//------------------------------------------------------------------------------
// API
int DATABASE_Init(char *db_file);
int DATABASE_Start(void);
void DATABASE_Destroy(void);
void DATABASE_PerformFactoryReset_ControllerInitiated(void);
int DATABASE_GetParameterValue(char *path, dm_hash_t hash, char *instances, char *buf, int buflen, unsigned flags);
int DATABASE_SetParameterValue(char *path, dm_hash_t hash, char *instances, char *new_value, unsigned flags);
int DATABASE_DeleteParameter(char *path, dm_hash_t hash, char *instances);
int DATABASE_StartTransaction(void);
int DATABASE_CommitTransaction(void);
int DATABASE_AbortTransaction(void);
void DATABASE_Dump(void);
int DATABASE_ReadDataModelInstanceNumbers(bool remove_unknown_params);
db_hash_t DATABASE_GetMigratedHash(db_hash_t hash);

#endif

