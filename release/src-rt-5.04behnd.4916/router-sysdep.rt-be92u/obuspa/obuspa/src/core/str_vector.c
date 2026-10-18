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
 * \file str_vector.c
 *
 * Implements a data structure containing a vector of strings
 *
 */
#include <stdlib.h>
#include <string.h>

#include "common_defs.h"
#include "str_vector.h"
#include "text_utils.h"

//------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int PtrToNaturalStrCmp(const void *arg1, const void *arg2);
int NaturalStrCmp(char *s1, char *s2);


/*********************************************************************//**
**
** STR_VECTOR_Init
**
** Initialises a string vector structure
**
** \param   sv - pointer to structure to initialise
**
** \return  None
**
**************************************************************************/
void STR_VECTOR_Init(str_vector_t *sv)
{
    sv->vector = NULL;
    sv->num_entries = 0;
}

/*********************************************************************//**
**
** STR_VECTOR_Clone
**
** Clones a string vector from the specified array of strings
**
** \param   sv - pointer to structure to initialise
**
** \return  None
**
**************************************************************************/
void STR_VECTOR_Clone(str_vector_t *sv, char **src_vector, int src_num_entries)
{
    int i;
    char *str;

    // Check that vector is empty
    USP_ASSERT(sv->vector == NULL);
    USP_ASSERT(sv->num_entries == 0);

    // Allocate memory for the vector
    sv->vector = USP_MALLOC(src_num_entries*sizeof(char *));
    sv->num_entries = src_num_entries;

    // Copy the string into the vector
    for (i=0; i<src_num_entries; i++)
    {
        str = src_vector[i];
        if (str==NULL)
        {
            str = "";
        }
        sv->vector[i] = USP_STRDUP(str);
    }
}

/*********************************************************************//**
**
** STR_VECTOR_Add
**
** Copies a string into the vector of strings
**
** \param   sv - pointer to structure to add the string to
** \param   str - pointer to string to copy
**
** \return  None
**
**************************************************************************/
void STR_VECTOR_Add(str_vector_t *sv, char *str)
{
    int new_num_entries;

    new_num_entries = sv->num_entries + 1;
    sv->vector = USP_REALLOC(sv->vector, new_num_entries*sizeof(char *));
    sv->vector[ sv->num_entries ] = USP_STRDUP(str);
    sv->num_entries = new_num_entries;
}

/*********************************************************************//**
**
** STR_VECTOR_Add_IfNotExist
**
** Adds a string into a vector of strings, if the string is not already present in the vector
** ie ensuring that there is only ever one copy of the string in the vector
**
** \param   sv - pointer to structure to add the string to
** \param   str - pointer to string to copy
**
** \return  None
**
**************************************************************************/
void STR_VECTOR_Add_IfNotExist(str_vector_t *sv, char *str)
{
    int index;

    // Exit if string is already present in the vector
    index = STR_VECTOR_Find(sv, str);
    if (index != INVALID)
    {
        return;
    }

    // Add the string to the vector
    STR_VECTOR_Add(sv, str);
}

/*********************************************************************//**
**
** STR_VECTOR_Find
**
** Finds the specified string within the vector and returns it's index
**
** \param   sv - pointer to vector containing strings to match against
** \param   str - pointer to string to find in the vector
**
** \return  Index of the string within the vector, or INVALID, if no match found
**
**************************************************************************/
int STR_VECTOR_Find(str_vector_t *sv, char *str)
{
    int i;
    char *s;

    // Iterate over all strings in the vector
    for (i=0; i < sv->num_entries; i++)
    {
        // Exit if found a string that matches
        s = sv->vector[i];
        if ((s != NULL) && (strcmp(s, str)==0))
        {
            return i;
        }
    }

    // if the code gets here, no string was found
    return INVALID;
}

/*********************************************************************//**
**
** STR_VECTOR_RemoveByIndex
**
** Removes the specified entry from the vector
**
** \param   sv - pointer to structure to remove an entry from
** \param   index - index of the string to remove from within the vector
**
** \return  None
**
**************************************************************************/
void STR_VECTOR_RemoveByIndex(str_vector_t *sv, int index)
{
    int num_items_after;

    // Remove the string at the specified index
    USP_SAFE_FREE(sv->vector[index]);

    // Move down all items in the vector after the one we just removed
    num_items_after = sv->num_entries-1 - index;
    if (num_items_after > 0)
    {
        memmove(&sv->vector[index], &sv->vector[index+1], num_items_after*sizeof(char *));
    }

    sv->num_entries--;

    // Ensure that the vector is freed, if it now contains no entries
    if (sv->num_entries == 0)
    {
        USP_FREE(sv->vector);
        sv->vector = NULL;
    }
}

/*********************************************************************//**
**
** STR_VECTOR_RemoveUnusedEntries
**
** Removes all NULL string entries from the vector, compacting the vector
** This function is usually called after marking some entries as NULL in the vector (after freeing them)
**
** \param   sv - pointer to structure to remove an entry from
**
** \return  None
**
**************************************************************************/
void STR_VECTOR_RemoveUnusedEntries(str_vector_t *sv)
{
    int i, j;

    // Move down all items in the vector
    j = 0;
    for (i=0; i < sv->num_entries; i++)
    {
        if (sv->vector[i] != NULL)
        {
            if (i != j)
            {
                sv->vector[j] = sv->vector[i];
            }
            j++;
        }
    }

    // Save the new number of entries, after
    sv->num_entries = j;

    // Ensure that the vector is freed, if it now contains no entries
    if (sv->num_entries == 0)
    {
        USP_FREE(sv->vector);
        sv->vector = NULL;
    }
}

/*********************************************************************//**
**
** STR_VECTOR_Destroy
**
** Deallocates all memory associated with the string vector
**
** \param   sv - pointer to structure to destroy all dynmically allocated memory it contains
**
** \return  None
**
**************************************************************************/
void STR_VECTOR_Destroy(str_vector_t *sv)
{
    int i;

    // Exit if vector is already empty
    if (sv->vector == NULL)
    {
        goto exit;
    }

    // Free all strings in the vector
    for (i=0; i < sv->num_entries; i++)
    {
        USP_SAFE_FREE( sv->vector[i] );
    }

    // Free the vector itself
    USP_FREE(sv->vector);

exit:
    // Ensure structure is re-initialised
    sv->vector = NULL;
    sv->num_entries = 0;
}

/*********************************************************************//**
**
** STR_VECTOR_Dump
**
** Logs all strings in the vector
**
** \param   sv - pointer to string vector structure
**
** \return  None
**
**************************************************************************/
void STR_VECTOR_Dump(str_vector_t *sv)
{
    int i;

    for (i=0; i < sv->num_entries; i++)
    {
        USP_DUMP("%s", sv->vector[i]);
    }
}

/*********************************************************************//**
**
** STR_VECTOR_ConvertToKeyValueVector
**
** Converts a string vector into a key-value pair vector,
** avoiding as much memory allocation and copying as possible.
** The strings in the string vector become keys in the key-value pair vector
** NOTE: The string vector is emptied as part of this operation
**
** \param   sv - pointer to string vector source structure to convert
** \param   kvv - pointer to key-value pair vector destination structure
**
** \return  None
**
**************************************************************************/
void STR_VECTOR_ConvertToKeyValueVector(str_vector_t *sv, kv_vector_t *kvv)
{
    int i;
    kv_pair_t *pair;

    // Allocate memory to store the array of the key-value pair vector
    kvv->vector = USP_MALLOC(sv->num_entries*sizeof(kv_pair_t));
    kvv->num_entries = sv->num_entries;

    // Move all entries in string vector to be keys in the key-value pair vector
    for (i=0; i < sv->num_entries; i++)
    {
        pair = &kvv->vector[i];
        pair->key = sv->vector[i];
        pair->value = NULL;
    }

    // Finally destroy the string vector, all memory referenced by it has been moved to the key-value pair vector
    USP_FREE(sv->vector);
    sv->vector = NULL;
    sv->num_entries = 0;
}

/*********************************************************************//**
**
** STR_VECTOR_Compare
**
** Compares the contents of two string vectors
** Assumes that the vectors have both been sorted into the same order before this function was called
**
** \param   sv1 - pointer to first vector to compare
** \param   sv2 - pointer to second vector to compare
**
** \return  true if the contants of the two vectors are equal, false otherwise
**
**************************************************************************/
bool STR_VECTOR_Compare(str_vector_t *sv1, str_vector_t *sv2)
{
    int i;

    // Exit if there are a different number of entries in each vector
    if (sv1->num_entries != sv2->num_entries)
    {
        return false;
    }

    // From this point on, both vectors have the same number of entries
    // So iterate over all entries, comparing their contents
    for (i=0; i < sv1->num_entries; i++)
    {
        // Exit if the contents are not identical
        if (strcmp(sv1->vector[i], sv2->vector[i]) != 0)
        {
            return false;
        }
    }

    // If the code gets here, then all entries were identical
    return true;
}

/*********************************************************************//**
**
** STR_VECTOR_Sort
**
** Sorts the vector, using a natural number alphanumeric sort
** This is used to sort data model path names before returning in the GetInstancesResponse
**
** \param   sv - pointer to vector to sort
**
** \return  None
**
**************************************************************************/
void STR_VECTOR_Sort(str_vector_t *sv)
{
    qsort(sv->vector, sv->num_entries, sizeof(sv->vector[0]), PtrToNaturalStrCmp);
}

/*********************************************************************//**
**
** PtrToNaturalStrCmp
**
** This function is used by STR_VECTOR_Sort() to determine the order of elements in the vector being sorted
**
** \param   arg1 - pointer to an entry in the vector
** \param   arg2 - pointer to another entry in the vector
**
** \return  Index of the string within the vector, or INVALID, if no match found
**
**************************************************************************/
int PtrToNaturalStrCmp(const void *arg1, const void *arg2)
{
    // qsort passes in pointers to the elements in the array to be compared
    // Hence we need to dereference the elements to get to the strings they point to
    char **p_s1 = (char **)arg1;
    char **p_s2 = (char **)arg2;
    char *s1;
    char *s2;

    s1 = *p_s1;
    s2 = *p_s2;

    return NaturalStrCmp(s1, s2);
}

/*********************************************************************//**
**
** NaturalStrCmp
**
** Compare 2 strings accounting correctly for natural numbers ie '2' comes before '11'
** This is used to sort path names correctly with instance numbers in them
**
** \param   s1 - pointer to first string
** \param   s2 - pointer to another entry in the vector
**
** \return  0 if strings are identical
**          negative number if s1 comes before s2
**          positive number if s1 comes after s2
**
**************************************************************************/
int NaturalStrCmp(char *s1, char *s2)
{
    char c1, c2;
    int num_digits_s1;
    int num_digits_s2;
    int delta;

    // Skip all characters which are the same
    while(true)
    {
        c1 = *s1;
        c2 = *s2;

        // Exit if reached the end of either string
        if ((c1 == '\0') || (c2 == '\0'))
        {
            // NOTE: The following comparision puts s1 before s2, if s1 terminates before s2 (and vice versa)
            return (int)c1 - (int)c2;
        }

        // Exit if the characters do not match
        if (c1 != c2)
        {
            break;
        }

        // As characters match, move to next characters
        s1++;
        s2++;
    }

    // If the code gets here, then we have reached a character which is different
    // Determine the number of digits in the rest of the string (this may be 0 if the first character is not a digit)
    num_digits_s1 = TEXT_UTILS_CountConsecutiveDigits(s1);
    num_digits_s2 = TEXT_UTILS_CountConsecutiveDigits(s2);

    // Determine if the number of digits in s1 is greater than in s2 (if so, s1 comes after s2)
    delta = num_digits_s1 - num_digits_s2;
    if (delta != 0)
    {
        return delta;
    }

    // If the code gets here, then the strings contain either no digits, or the same number of digits,
    // so just compare the characters (this also works if the characters are digits)
    return (int)c1 - (int)c2;
}

