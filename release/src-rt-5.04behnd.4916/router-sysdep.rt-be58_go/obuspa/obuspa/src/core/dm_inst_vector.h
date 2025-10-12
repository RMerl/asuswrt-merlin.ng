/*
 *
 * Copyright (C) 2019-2023, Broadband Forum
 * Copyright (C) 2016-2023  CommScope, Inc
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
 * \file dm_inst_vector.h
 *
 * Implements a vector of dm_instances structures
 * This is basically a list of all object instances instantiated in the data model
 *
 */

#ifndef DM_INST_VECTOR_H
#define DM_INST_VECTOR_H

//-----------------------------------------------------------------------------------------
// API
void DM_INST_VECTOR_Init(dm_instances_vector_t *div);
void DM_INST_VECTOR_Destroy(dm_instances_vector_t *div);
int DM_INST_VECTOR_Add(dm_instances_t *inst);
void DM_INST_VECTOR_Remove(dm_instances_t *inst);
int DM_INST_VECTOR_IsExist(dm_instances_t *match, bool *exists);
int DM_INST_VECTOR_GetNextInstance(dm_node_t *node, dm_instances_t *inst, int *next_instance);
int DM_INST_VECTOR_GetNumInstances(dm_node_t *node, dm_instances_t *inst, int *num_instances);
int DM_INST_VECTOR_GetInstances(dm_node_t *node, dm_instances_t *inst, int_vector_t *iv);
int DM_INST_VECTOR_GetAllInstancePaths_Unqualified(dm_node_t *node, dm_instances_t *inst, str_vector_t *sv, combined_role_t *combined_role);
int DM_INST_VECTOR_GetAllInstancePaths_Qualified(dm_instances_t *inst, str_vector_t *sv, combined_role_t *combined_role);
void DM_INST_VECTOR_RefreshBaselineInstances(dm_node_t *parent);
void DM_INST_VECTOR_Dump(dm_instances_vector_t *div);
int DM_INST_VECTOR_RefreshInstance(char *path);
int DM_INST_VECTOR_RefreshTopLevelObjectInstances(dm_node_t *node);
void DM_INST_VECTOR_NextLockPeriod(void);

#endif
