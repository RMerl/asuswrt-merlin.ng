/*
 * This is specific to pcie full dongle for now.
 * handles data structures and functions for rx completion operation
 * Shared utils between bus layer and WL layer
 * Copyright (C) 2021, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 * $Id$
 *
 */
#include <bcm_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <hnd_cplt.h>

/* rxcpl list management */
bcm_rxcplid_list_t *g_rxcplid_list = NULL;
uint32	g_rxcplid_num_alloc_failures = 0;

static const char BCMATTACHDATA(rstr_rxcplid_list_already_inited)[]  =
	"ERROR: rxcplid list already inited\n";
bool
BCMATTACHFN(bcm_alloc_rxcplid_list)(osl_t *osh, uint32 rxcpl_max)
{
	uint32 size;
	rxcpl_info_t *ptr;
	uint32 i;

	printf("allocating a max of %d rxcplid buffers\n", rxcpl_max);

	if (g_rxcplid_list != NULL) {
		printf(rstr_rxcplid_list_already_inited);
		return FALSE;
	}
	/* rxcpl_ptr[0] is not used, so add 1 additional rxcpl_info_t */
	size = sizeof(bcm_rxcplid_list_t) + ((rxcpl_max+1) * sizeof(rxcpl_info_t));
	g_rxcplid_list = (bcm_rxcplid_list_t *)MALLOC(osh, size);
	if (g_rxcplid_list == NULL) {
		printf("ERROR: rxcplid list allocation fail, size %d, items %d\n", size, rxcpl_max);
		return FALSE;
	}
	bzero(g_rxcplid_list, size);
	g_rxcplid_list->max = rxcpl_max;
	g_rxcplid_list->rxcpl_ptr = (rxcpl_info_t *)(g_rxcplid_list + 1);
	g_rxcplid_list->free_list = g_rxcplid_list->rxcpl_ptr + 1;
	ptr = g_rxcplid_list->free_list;
	for (i = 1; i <= g_rxcplid_list->max; i++) {
		ptr->rxcpl_id.idx = i;
		if (i != g_rxcplid_list->max) {
			ptr->free_next = ptr + 1;
			ptr = ptr->free_next;
		}
		else
			ptr->free_next = NULL;
	}
	g_rxcplid_list->avail = g_rxcplid_list->max;
	return TRUE;
}

rxcpl_info_t *
bcm_alloc_rxcplinfo()
{
	rxcpl_info_t *ptr;
	if (g_rxcplid_list == NULL)
		return NULL;
	/* If we run out of rxcpl ids, we should force a TRAP */
	if (g_rxcplid_list->avail == 0 ||
		g_rxcplid_list->free_list == NULL) {
		g_rxcplid_num_alloc_failures ++;
		return NULL;
	}
	ptr = g_rxcplid_list->free_list;
	g_rxcplid_list->free_list = ptr->free_next;

	g_rxcplid_list->avail--;

	/* Reset the fail count */
	g_rxcplid_num_alloc_failures = 0;

	if ((g_rxcplid_list->free_list == NULL) && (g_rxcplid_list->avail != 0)) {
		printf("ERROR :something is really wrong here, idx is %d, avail %d\n",
			ptr->rxcpl_id.idx, g_rxcplid_list->avail);
		ASSERT(0);
	}

	ptr->rxcpl_id.next_idx = 0;
	ptr->rxcpl_id.flags = 0;
	ptr->free_next = NULL;

	if (ptr->rxcpl_id.idx == 0) {
		printf("ERROR: allocating rxpl_info id %d\n", ptr->rxcpl_id.idx);
		ASSERT(0);
	}
	BCM_RXCPL_SET_IN_TRANSIT(ptr);

	return ptr;
}

void
bcm_free_rxcplinfo(rxcpl_info_t *ptr)
{
	if (g_rxcplid_list == NULL)
		return;

	if (g_rxcplid_list->avail == g_rxcplid_list->max) {
		printf("ERROR: fail to free the rxcpl entry %d, avail %d\n",
			ptr->rxcpl_id.idx, g_rxcplid_list->avail);
		ASSERT(0);
		return;
	}
	if (ptr->rxcpl_id.idx == 0) {
		printf("ERROR: freeing rxpl_info id %d\n", ptr->rxcpl_id.idx);
		ASSERT(0);
	}

	ptr->free_next = g_rxcplid_list->free_list;
	g_rxcplid_list->free_list = ptr;
	g_rxcplid_list->avail++;
	ptr->rxcpl_id.next_idx = 0;
	ptr->rxcpl_id.flags = 0;

}

void
bcm_chain_rxcplid(uint16 first,  uint16 next)
{
	rxcpl_info_t *ptr;
	uint32 count = 0;

	if (g_rxcplid_list == NULL)
		return;
	if ((first == 0) || (next == 0) || (first == next)) {
		printf("ERROR: chaining  going bad first %d, next %d\n", first, next);
		ASSERT(0);
		return;

	}

	if ((first > g_rxcplid_list->max) || (next > g_rxcplid_list->max))
		return;
	ptr = bcm_id2rxcplinfo(first);
#ifdef DEBUG
	if (bcm_rxcpllist_end(ptr, &count) != ptr) {
		printf("chaining with chain at front\n");
	}
#endif // endif
	ptr = bcm_rxcpllist_end(ptr, &count);
	ptr->rxcpl_id.next_idx = next;
}

rxcpl_info_t *
bcm_id2rxcplinfo(uint16 id)
{
	if (id == 0) {
		return NULL;
	}
	if (id > g_rxcplid_list->max) {
		ASSERT(0);
		return NULL;
	}
	return (&g_rxcplid_list->rxcpl_ptr[id]);

}

uint16
bcm_rxcplinfo2id(rxcpl_info_t *ptr)
{
	return (uint16)(ptr->rxcpl_id.idx);
}

rxcpl_info_t *
bcm_rxcpllist_end(rxcpl_info_t *ptr, uint32 *count)
{
	uint32 cnt = 1;
	while (ptr->rxcpl_id.next_idx != 0) {
		ptr = bcm_id2rxcplinfo((uint16)(ptr->rxcpl_id.next_idx));
		cnt++;
	}
	*count = cnt;
	return ptr;
}

/*  Debug util : Dump a list of rxcpl info given the head idx */
void
bcm_rxcpllist_dump(uint16 head)
{
	rxcpl_info_t *p_rxcpl_info;
	uint16 next_idx;

	p_rxcpl_info = bcm_id2rxcplinfo(head);

	/* Loop through each idx */
	while (p_rxcpl_info != NULL) {
		printf("RXCPL  idx %4d  cpl info 0x%p In Transit %d Valid %d \n",
			p_rxcpl_info->rxcpl_id.idx, p_rxcpl_info,
			BCM_RXCPL_IN_TRANSIT(p_rxcpl_info),
			BCM_RXCPL_VALID_INFO(p_rxcpl_info));

		/* Linked idx */
		next_idx = p_rxcpl_info->rxcpl_id.next_idx;
		p_rxcpl_info = bcm_id2rxcplinfo(next_idx);
	}
}
