/*
 *
 * Copyright (C) 2019, Broadband Forum
 * Copyright (C) 2016-2019  CommScope, Inc
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
 * \file expr_vector.h
 *
 * Implements a vector of expression components
 *
 */

#ifndef EXPR_VECTOR_H
#define EXPR_VECTOR_H

#include "kv_vector.h"
#include "usp_api.h"   // for expr_op_t

//-----------------------------------------------------------------------------------------
// Type representing expression component
typedef struct
{
    char *param;
    expr_op_t op;
    char *value;
} expr_comp_t;

//-----------------------------------------------------------------------------------------
// Expression vector type
typedef struct
{
    expr_comp_t *vector;
    int num_entries;
} expr_vector_t;

//-----------------------------------------------------------------------------------------
// Export array used by debug to convert an operator back to a string representation
extern char *expr_op_2_str[kExprOp_Max];

//-----------------------------------------------------------------------------------------
// Defines for 'is_cli_parser' argument of EXPR_VECTOR_SplitExpressions()
#define EXPR_FROM_USP false
#define EXPR_FROM_CLI true

//-----------------------------------------------------------------------------------------
// Expression Vector API
void EXPR_VECTOR_Init(expr_vector_t *ev);
void EXPR_VECTOR_Add(expr_vector_t *ev, char *param, expr_op_t op, char *value);
void EXPR_VECTOR_ToKeyValueVector(expr_vector_t *ev, kv_vector_t *kvv);
void EXPR_VECTOR_Destroy(expr_vector_t *ev);
void EXPR_VECTOR_Dump(expr_vector_t *ev);
int EXPR_VECTOR_SplitExpressions(char *str, expr_vector_t *ev, char *separator, expr_op_t *valid_ops, int num_valid_ops, bool is_cli_parser);

#endif
