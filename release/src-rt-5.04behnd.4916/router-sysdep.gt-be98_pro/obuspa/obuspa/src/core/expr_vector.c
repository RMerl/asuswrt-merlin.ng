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
 * \file expr_vector.c
 *
 * Implements a data structure containing a vector of expression components ie. {param, op, value}
 *
 */
#include <stdlib.h>
#include <string.h>

#include "common_defs.h"
#include "expr_vector.h"
#include "kv_vector.h"
#include "str_vector.h"
#include "text_utils.h"

//------------------------------------------------------------------------------
// Array used to convert from an enumerated value to a string
char *expr_op_2_str[kExprOp_Max] =
{
    "==", // kExprOp_Equal
    "!=", // kExprOp_NotEqual
    "<=", // kExprOp_LessThanOrEqual
    ">=", // kExprOp_GreaterThanOrEqual
    "<",  // kExprOp_LessThan
    ">",  // kExprOp_GreaterThan
    "=",  // kExprOp_Equals
};


//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
bool IsOperatorInArray(expr_op_t expr_op, expr_op_t *valid_ops, int num_valid_ops);
int ParseExprComponent(char *buf, char **p_relative_path, expr_op_t *p_op, char **p_value, bool is_cli_parser);
char *SplitOnOperator(char *buf, expr_op_t *p_op);


/*********************************************************************//**
**
** EXPR_VECTOR_Init
**
** Initialises the vector structure
**
** \param   ev - pointer to expression vector structure
**
** \return  None
**
**************************************************************************/
void EXPR_VECTOR_Init(expr_vector_t *ev)
{
    ev->vector = NULL;
    ev->num_entries = 0;
}

/*********************************************************************//**
**
** EXPR_VECTOR_Add
**
** Adds an expression component to the vector
**
** \param   ev - pointer to expression vector structure
** \param   param - pointer to expression parameter to add
** \param   op - pointer to expression operator to add
** \param   value - pointer to expression constant to add
**
** \return  None
**
**************************************************************************/
void EXPR_VECTOR_Add(expr_vector_t *ev, char *param, expr_op_t op, char *value)
{
    int new_num_entries;
    expr_comp_t *ec;

    new_num_entries = ev->num_entries + 1;
    ev->vector = USP_REALLOC(ev->vector, new_num_entries*sizeof(expr_comp_t));

    ec = &ev->vector[ ev->num_entries ];
    ec->param = USP_STRDUP(param);
    ec->op = op;
    ec->value = USP_STRDUP(value);

    ev->num_entries = new_num_entries;
}

/*********************************************************************//**
**
** EXPR_VECTOR_ToKeyValueVector
**
** Converts an expression vector into a key-value vector
** NOTE: The expression vector is destroyed after being converted
** NOTE: This routine avoids as much memory allocation as possible
**
** \param   ev - pointer to expression vector to convert
** \param   kvv - pointer to key-value pair vector structure to convert the expression vector into
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void EXPR_VECTOR_ToKeyValueVector(expr_vector_t *ev, kv_vector_t *kvv)
{
    int i;
    int num_entries;
    kv_pair_t *kv;
    expr_comp_t *ec;

    // Exit if nothing to do (no entries in the expression vector)
    KV_VECTOR_Init(kvv);
    num_entries = ev->num_entries;
    if (num_entries == 0)
    {
        return;
    }

    // Allocate memory to store the key-value vector
    kvv->vector = USP_MALLOC(num_entries*sizeof(kv_pair_t));
    kvv->num_entries = num_entries;

    // Iterate over all entries in the expression vector, moving them to the key-value vector
    for (i=0; i<num_entries; i++)
    {
        ec = &ev->vector[i];
        kv = &kvv->vector[i];

        kv->key = ec->param;
        kv->value = ec->value;
    }

    // Finally destroy the expression vector
    USP_FREE(ev->vector);
    ev->vector = NULL;
    ev->num_entries = 0;
}


/*********************************************************************//**
**
** EXPR_VECTOR_Destroy
**
** Deallocates all memory associated with the vector
**
** \param   ev - pointer to expression vector structure
**
** \return  None
**
**************************************************************************/
void EXPR_VECTOR_Destroy(expr_vector_t *ev)
{
    int i;
    expr_comp_t *ec;

    // Exit if vector is already empty
    if (ev->vector == NULL)
    {
        goto exit;
    }

    // Free all strings in the vector
    for (i=0; i < ev->num_entries; i++)
    {
        ec = &ev->vector[i];
        USP_FREE( ec->param );
        USP_FREE( ec->value );
    }

    // Free the vector itself
    USP_FREE(ev->vector);

exit:
    // Ensure structure is re-initialised
    ev->vector = NULL;
    ev->num_entries = 0;
}

/*********************************************************************//**
**
** EXPR_VECTOR_Dump
**
** Logs all expressions in the vector
**
** \param   ev - pointer to expression vector structure
**
** \return  None
**
**************************************************************************/
void EXPR_VECTOR_Dump(expr_vector_t *ev)
{
    int i;
    expr_comp_t *ec;

    for (i=0; i < ev->num_entries; i++)
    {
        ec = &ev->vector[i];
        USP_DUMP("%s %s %s", ec->param, expr_op_2_str[ec->op], ec->value);
    }
}

/*********************************************************************//**
**
** EXPR_VECTOR_SplitExpressions
**
** Splits a string on a specified separator into expressions,
** checking that the operator in the expression matches one of those in a list supplied to this function
** NOTE: Substrings are trimmed of whitespace at the start and end
**
** \param   str - string containing comma-delimited substrings
** \param   ev - pointer to vector to return expressions in
** \param   separator - pointer to string containing separator of expressions
** \param   valid_ops - pointer to array of operators which are accepted in the expression
** \param   num_valid_ops - number of acceptable operators in the array
** \param   is_cli_parser - set if this function is to follow the USP Agent CLI rules for parsing the expression
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int EXPR_VECTOR_SplitExpressions(char *str, expr_vector_t *ev, char *separator, expr_op_t *valid_ops, int num_valid_ops, bool is_cli_parser)
{
    int i;
    int err;
    str_vector_t key_expressions;
    char *relative_path;
    expr_op_t expr_op;
    char *value;
    bool is_accepted;

    // Split the string into a vector of substrings containing key expressions
    EXPR_VECTOR_Init(ev);
    STR_VECTOR_Init(&key_expressions);
    TEXT_UTILS_SplitString(str, &key_expressions, separator);

    // Exit if no key expressions were found
    if (key_expressions.num_entries == 0)
    {
        err = USP_ERR_OK;       // NOTE: Caller must decide whether it is an error to have no expressions returned
        goto exit;
    }

    // Iterate over all key expressions
    for (i=0; i< key_expressions.num_entries; i++)
    {
        // Exit if cannot extract the relative path and value
        err = ParseExprComponent(key_expressions.vector[i], &relative_path, &expr_op, &value, is_cli_parser);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }

        // Exit if expression operator was not acceptable
        is_accepted = IsOperatorInArray(expr_op, valid_ops, num_valid_ops);
        if (is_accepted == false)
        {
            USP_ERR_SetMessage("%s: Expression '%s' contains invalid operator '%s'", __FUNCTION__, key_expressions.vector[i], expr_op_2_str[expr_op] );
            err = USP_ERR_INVALID_PATH_SYNTAX;
            goto exit;
        }

        // Add the relative_path and value to the expression vector
        EXPR_VECTOR_Add(ev, relative_path, expr_op, value);
    }

    err = USP_ERR_OK;

exit:
    STR_VECTOR_Destroy(&key_expressions);       // Delete key expressions, we have finished with them now (minimises recursive memory usage)
    return err;
}

/*********************************************************************//**
**
** IsOperatorInArray
**
** Determines whether the specified operator is present in the supplied array of acceptable operators
**
** \param   expr_op
** \param   valid_ops - pointer to array of operators which are accepted in the expression
** \param   num_valid_ops - number of acceptable operators in the array
**
** \return  true if specified operator is present in the array
**
**************************************************************************/
bool IsOperatorInArray(expr_op_t expr_op, expr_op_t *valid_ops, int num_valid_ops)
{
    int i;

    // Iterate over all operators in the array, seeing if they match the specified operator
    for (i=0; i<num_valid_ops; i++)
    {
        if (valid_ops[i] == expr_op)
        {
            return true;
        }
    }

    // If the code gets here, the specified operator did not match any of the operators in the array
    return false;
}

/*********************************************************************//**
**
** ParseExprComponent
**
** This function parses expression components such as:-
**            RelativePath >= Value
** and        RelativePath == "Value"
** It returns pointers to the relative_path and value within the supplied buffer,
** along with an enumeration of the associated operator
** (If the value is enclosed in quotes, then the Value returned is stripped of quotes
** NOTE: This function modifies the buffer to create the strings within it.
** NOTE: This function assumes that leading and trailing whitespace has already been removed from the input buffer
**
** \param   buf - pointer to a buffer containing the expression component
** \param   p_relative_path - pointer to variable in which to return a pointer to the
**                          relative path string contained in the supplied buffer
** \param   p_op - pointer to variable in which to return the parsed operator
** \param   p_value - pointer to variable in which to return a pointer to the value
**                  string contained in the supplied buffer
** \param   is_cli_parser - set if this function is to follow the USP Agent CLI rules for parsing the expression
**
** \return  USP_ERR_OK if successful, or no instances found
**
**************************************************************************/
int ParseExprComponent(char *buf, char **p_relative_path, expr_op_t *p_op, char **p_value, bool is_cli_parser)
{
    char *after_op;
    char *expr_param;
    char *expr_const;
    int len;

    // Split the buffer into 2 strings on the operator. Exiting if unable to find an operator.
    after_op = SplitOnOperator(buf, p_op);
    if (after_op == NULL)
    {
        USP_ERR_SetMessage("%s: Unable to parse expression component '%s' (operator not found)", __FUNCTION__, buf);
        return USP_ERR_INVALID_PATH_SYNTAX;
    }

    // Trim the strings either side of the operator of whitespace
    expr_param = TEXT_UTILS_TrimBuffer(buf);
    expr_const = TEXT_UTILS_TrimBuffer(after_op);

    // Exit if expression param is empty
    if (*expr_param == '\0')
    {
        USP_ERR_SetMessage("%s: missing expression parameter before %s", __FUNCTION__, expr_const);
        return USP_ERR_INVALID_PATH_SYNTAX;
    }

    // Exit if expression constant is empty
    if (*expr_const == '\0')
    {
        USP_ERR_SetMessage("%s: missing expression constant after %s", __FUNCTION__, expr_param);
        return USP_ERR_INVALID_PATH_SYNTAX;
    }

    // Strip the expression constant of speech marks, if it is referring to a string value
    len = strlen(expr_const);
    if ((*expr_const == '\"') && (expr_const[len-1] == '\"'))
    {
        expr_const[len-1] = '\0';
        expr_const++;
    }

    // Strip the expression constant of quotes (only for CLI supplied expressions), if it is referring to a string value
    if (is_cli_parser)
    {
        if ((*expr_const == '\'') && (expr_const[len-1] == '\''))
        {
            expr_const[len-1] = '\0';
            expr_const++;
        }
    }

    // Exit if expression param contains illegal characters (these characters should be % encoded)
    if (strcspn(expr_param, " \t\"\'") != strlen(expr_param))
    {
        USP_ERR_SetMessage("%s: Expression parameter '%s' is not valid", __FUNCTION__, expr_param);
        return USP_ERR_INVALID_PATH_SYNTAX;
    }

    // Exit if the expression constant still contains speech marks
    if (strchr(expr_const, '\"') != NULL)
    {
        USP_ERR_SetMessage("%s: Expression constant '%s' is not valid", __FUNCTION__, expr_const);
        return USP_ERR_INVALID_PATH_SYNTAX;
    }

    // Convert % escaped characters in the expression constant to their equivalent value
    TEXT_UTILS_PercentDecodeString(expr_const);

    // If the code gets here, then the values were extracted successfully
    *p_relative_path = expr_param;
    *p_value = expr_const;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** SplitOnOperator
**
** This function finds an expression operator in a buffer
** It then splits the buffer at the expression operator, and returns a
** pointer to the string following the expression operator in the buffer
**
** \param   buf - pointer to a buffer containing an expression component
** \param   p_op - pointer to variable in which to return the parsed operator
**
** \return  Pointer to string following the operator
**
**************************************************************************/
char *SplitOnOperator(char *buf, expr_op_t *p_op)
{
    char *op;

    // Exit if found the "==" operator
    // NOTE: We test for this before trying to find the '=' operator
    op = strstr(buf, "==");
    if (op != NULL)
    {
        *p_op = kExprOp_Equal;
        *op = '\0';
        return &op[2];
    }

    // Exit if found the "!=" operator
    op = strstr(buf, "!=");
    if (op != NULL)
    {
        *p_op = kExprOp_NotEqual;
        *op = '\0';
        return &op[2];
    }

    // Exit if found the "<=" operator
    // NOTE: We test for this before trying to find the '<' or '=' operator
    op = strstr(buf, "<=");
    if (op != NULL)
    {
        *p_op = kExprOp_LessThanOrEqual;
        *op = '\0';
        return &op[2];
    }

    // Exit if found the ">=" operator
    // NOTE: We test for this before trying to find the '>' or '=' operator
    op = strstr(buf, ">=");
    if (op != NULL)
    {
        *p_op = kExprOp_GreaterThanOrEqual;
        *op = '\0';
        return &op[2];
    }

    // Exit if found the "<" operator
    op = strchr(buf, '<');
    if (op != NULL)
    {
        *p_op = kExprOp_LessThan;
        *op = '\0';
        return &op[1];
    }

    // Exit if found the ">" operator
    op = strchr(buf, '>');
    if (op != NULL)
    {
        *p_op = kExprOp_GreaterThan;
        *op = '\0';
        return &op[1];
    }

    // Exit if found the "=" operator
    op = strchr(buf, '=');
    if (op != NULL)
    {
        *p_op = kExprOp_Equals;
        *op = '\0';
        return &op[1];
    }

    // If the code gets here, then no operator was found
    return NULL;
}

//------------------------------------------------------------------------------------------
// Code to test the ParseExprComponent() function
#if 0
char *parse_expr_comp_test_cases[] =
{
    // Test case                // Expected relative_path   // Expected Value
    // SUCCESS
    "Param==1234",              "Param",                    "1234",
    "Param  ==  1234",              "Param",                    "1234",
    "Param  ==  \"1234\"",              "Param",                    "1234",
    "Param  ==  \"\"",              "Param",                    "",
    "Param.Param2==\"Hello There\"",              "Param.Param2",                    "Hello There",
    "Param  !=  1234",              "Param",                    "1234",
    "Param  <  1234",              "Param",                    "1234",
    "Param  >  1234",              "Param",                    "1234",
    "Param  >=  1234",              "Param",                    "1234",
    "Param  <=  1234",              "Param",                    "1234",
    "Param<1234",              "Param",                    "1234",
    "Param>1234",              "Param",                    "1234",
    "Param>=1234",              "Param",                    "1234",
    "Param<=1234",              "Param",                    "1234",

    // FAILURE
    "Param && 1234",              NULL, NULL,
    "Param=1234",              NULL, NULL,
    "==1234",              NULL, NULL,
    "Param==",              NULL, NULL,
    "Param==\"my_value",              NULL, NULL,
    "Param==my_value\"",              NULL, NULL,
    "Param1 P==1234",              NULL, NULL,
    "Param1 Param2  ==1234",              NULL, NULL,
    "Param1 \"x==\"2\"",              NULL, NULL,

};

void TestParseExprComponent(void)
{
    int i;
    int err;
    char buf[256];
    char *relative_path;
    char *value;
    expr_op_t op;

    for (i=0; i < NUM_ELEM(parse_expr_comp_test_cases); i+=3)
    {
        strcpy(buf, parse_expr_comp_test_cases[i]);

        printf("[%d] %s\n", i/3, buf);
        err = ParseExprComponent(buf, &relative_path, &op, &value, false);
        if (err == USP_ERR_OK)
        {
            // Print error if results do not match those expected
            if ((strcmp(relative_path, parse_expr_comp_test_cases[i+1]) != 0) ||
                (strcmp(value, parse_expr_comp_test_cases[i+2]) != 0))
            {
                printf("ERROR: [%d] Test case result for '%s' is {'%s','%s'} (expected {'%s','%s'})\n",
                       i/3, parse_expr_comp_test_cases[i],
                       relative_path, value,
                       parse_expr_comp_test_cases[i+1], parse_expr_comp_test_cases[i+2]);
            }
            else
            {
                printf("PASS\n");
            }
        }
        else
        {
            // Print error if results do not match those expected
            if ((parse_expr_comp_test_cases[i+1] != NULL) ||
                (parse_expr_comp_test_cases[i+2] != NULL))
            {
                printf("ERROR: [%d] Test case result for '%s' is {'%s','%s'} (expected an error returned)\n",
                       i/3, parse_expr_comp_test_cases[i],
                       relative_path, value);
            }
            else
            {
                printf("PASS\n");
            }
        }
    }
}
#endif







