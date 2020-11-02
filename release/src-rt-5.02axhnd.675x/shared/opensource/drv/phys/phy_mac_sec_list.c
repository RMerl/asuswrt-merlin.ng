/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

:>
*/

#include "phy_mac_sec.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

typedef struct
{
    /* macsec_List head */
    List_Element_t * Head_p;

    /* macsec_List tail */
    List_Element_t * Tail_p;

    /* Number of elements in the list */
    uint32_t ElementCount;

} List_t;

/*----------------------------------------------------------------------------
 * Local variables
 */

/* Statically allocated list instances. This will be deprecated in future. */
static List_t macsec_List[6];


/*----------------------------------------------------------------------------
 * macsec_List_Init
 *
 */
List_Status_t macsec_List_Init(const uint32_t ListID, void * const ListInstance_p)
{
    List_t * List_p;

    if (ListInstance_p)
        List_p = (List_t*)ListInstance_p;
    else
        List_p = &macsec_List[ListID];

    List_p->ElementCount    = 0;
    List_p->Head_p          = NULL;
    List_p->Tail_p          = NULL;

    return LIST_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * macsec_List_AddToHead
 *
 */
List_Status_t macsec_List_AddToHead(const uint32_t ListID, void * const ListInstance_p, List_Element_t * const Element_p)
{

    List_Element_t * TempElement_p;
    List_t * List_p;

    if (ListInstance_p)
        List_p = (List_t*)ListInstance_p;
    else
        List_p = &macsec_List[ListID];

    TempElement_p           = List_p->Head_p;
    List_p->Head_p          = Element_p;

    /* Previous element in the list, this is a head */
    Element_p->Internal[0]  = NULL;

    /* Next element in the list */
    Element_p->Internal[1]  = TempElement_p;

    /* Check if this is the first element */
    if (List_p->ElementCount == 0)
        List_p->Tail_p = List_p->Head_p;
    else
        /* Link the old head to the new head */
        TempElement_p->Internal[0] = Element_p;

    List_p->ElementCount++;
    
    return LIST_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * macsec_List_RemoveFromTail
 *
 */
List_Status_t macsec_List_RemoveFromTail(const uint32_t ListID, void * const ListInstance_p, List_Element_t ** const Element_pp)
{

    List_Element_t * TempElement_p;
    List_t * List_p;

    if (ListInstance_p)
        List_p = (List_t*)ListInstance_p;
    else
        List_p = &macsec_List[ListID];

    if (List_p->ElementCount == 0)
        return LIST_ERROR_BAD_ARGUMENT;

    /* Get the previous for the tail element in the list */
    TempElement_p = (List_Element_t*)List_p->Tail_p->Internal[0];

    List_p->Tail_p->Internal[0] = NULL;
    List_p->Tail_p->Internal[1] = NULL;
    *Element_pp                 = List_p->Tail_p;

    /* Set the new tail */
    List_p->Tail_p              = TempElement_p;

    /* Check if this is the last element */
    if (List_p->ElementCount == 1)
        List_p->Head_p = NULL;
    else
        /* New tail must have no next element */
        List_p->Tail_p->Internal[1] = NULL;

    List_p->ElementCount--;

    return LIST_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * macsec_List_RemoveAnywhere
 */
List_Status_t macsec_List_RemoveAnywhere(const uint32_t ListID, void * const ListInstance_p, List_Element_t * const Element_p)
{
    List_Element_t * PrevElement_p, * NextElement_p;
    List_t * List_p;

    if (ListInstance_p)
        List_p = (List_t*)ListInstance_p;
    else
        List_p = &macsec_List[ListID];

    if (List_p->ElementCount == 0)
        return LIST_ERROR_BAD_ARGUMENT;

    /* Check element belongs to this list */
    {
        uint32_t i;
        List_Element_t * TempElement_p = List_p->Head_p;

        for (i = 0; i < List_p->ElementCount; i++)
        {
            List_Element_t * p;

            if (TempElement_p == Element_p)
                break; /* Found */

            p = TempElement_p->Internal[1];
            if (p)
                TempElement_p = p; /* not end of list yet */
            else
                return LIST_ERROR_BAD_ARGUMENT; /* Not found */
        }

        if (TempElement_p != Element_p)
            return LIST_ERROR_BAD_ARGUMENT; /* Not found */
    }

    PrevElement_p = Element_p->Internal[0];
    NextElement_p = Element_p->Internal[1];

    Element_p->Internal[0] = NULL;
    Element_p->Internal[1] = NULL;

    if (PrevElement_p)
        PrevElement_p->Internal[1] = NextElement_p;
    else
        List_p->Head_p = NextElement_p;

    if (NextElement_p)
        NextElement_p->Internal[0] = PrevElement_p;
    else
        List_p->Tail_p = NULL;


    List_p->ElementCount--;

    return LIST_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * macsec_List_GetListElementCount
 *
 */
List_Status_t macsec_List_GetListElementCount(const uint32_t ListID, void * const ListInstance_p, uint32_t * const Count_p)
{
    if (Count_p == NULL)
        return LIST_ERROR_BAD_ARGUMENT;
    {
        List_t * List_p;

        if (ListInstance_p)
            List_p = (List_t*)ListInstance_p;
        else
            List_p = &macsec_List[ListID];

        *Count_p = List_p->ElementCount;
    }

    return LIST_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * macsec_List_RemoveFromHead
 *
 */
List_Status_t macsec_List_RemoveFromHead(const uint32_t ListID, void * const ListInstance_p, List_Element_t ** const Element_pp)
{
    List_t * List_p;

    if (ListInstance_p)
        List_p = (List_t*)ListInstance_p;
    else
        List_p = &macsec_List[ListID];

    if (Element_pp == NULL)
        return LIST_ERROR_BAD_ARGUMENT;

    if (List_p->ElementCount == 0)
        return LIST_ERROR_BAD_ARGUMENT;

    /* Remove the element from the list head */
    {
        List_Element_t * TempElement_p;

        /* Get the next for the head element in the list */
        TempElement_p = (List_Element_t*)List_p->Head_p->Internal[1];

        List_p->Head_p->Internal[0] = NULL;
        List_p->Head_p->Internal[1] = NULL;
        *Element_pp                 = List_p->Head_p;

        List_p->Head_p              = TempElement_p;

        /* Check if this is the last element */
        if (List_p->ElementCount == 1)
            List_p->Tail_p = NULL;
        else
            List_p->Head_p->Internal[0] = NULL;

        List_p->ElementCount--;
    }

    return LIST_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * macsec_List_GetHead
 */
List_Status_t macsec_List_GetHead(const uint32_t ListID, void * const ListInstance_p, const List_Element_t ** const Element_pp)
{
    if (Element_pp == NULL)
        return LIST_ERROR_BAD_ARGUMENT;

    /* Get the list head */
    {
        List_t * List_p;

        if (ListInstance_p)
            List_p = (List_t*)ListInstance_p;
        else
            List_p = &macsec_List[ListID];

        *Element_pp = List_p->Head_p;
    }

    return LIST_STATUS_OK;
}

/*----------------------------------------------------------------------------
 * macsec_List_GetInstanceByteCount
 *
 * Gets the memory size of the list instance (in bytes) excluding the list
 * elements memory size. This list memory size can be used to allocate a list
 * instance and pass a pointer to it subsequently to the List_*() functions.
 *
 * This function is re-entrant and can be called any time.
 *
 * Return Values
 *     Size of the list administration memory in bytes.
 */
uint32_t macsec_List_GetInstanceByteCount(void)
{
    return sizeof(List_t);
}

List_Element_t * macsec_List_GetNextElement(const List_Element_t * const Element_p)
{
    /* Get the next element from the list */
    return Element_p->Internal[1];
}


/* end of file list.c */
