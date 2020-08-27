/*
 * Copyright (C) 2009-2016  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stddef.h>
#include <netinet/ether.h>

#include "bisect_iv.h"
#include "bat-hosts.h"
#include "hash.h"
#include "functions.h"

static struct hashtable_t *node_hash = NULL;
static struct bat_node *curr_bat_node = NULL;

static void bisect_iv_usage(void)
{
	fprintf(stderr, "Usage: batctl bisect_iv [parameters] <file1> <file2> .. <fileN>\n");
	fprintf(stderr, "parameters:\n");
	fprintf(stderr, " \t -h print this help\n");
	fprintf(stderr, " \t -l run a loop detection of given mac address or bat-host (default)\n");
	fprintf(stderr, " \t -n don't convert addresses to bat-host names\n");
	fprintf(stderr, " \t -o only display orig events that affect given mac address or bat-host\n");
	fprintf(stderr, " \t -r print routing tables of given mac address or bat-host\n");
	fprintf(stderr, " \t -s seqno range to limit the output\n");
	fprintf(stderr, " \t -t trace seqnos of given mac address or bat-host\n");
}

static int compare_name(void *data1, void *data2)
{
	return (memcmp(data1, data2, NAME_LEN) == 0 ? 1 : 0);
}

static int choose_name(void *data, int32_t size)
{
	unsigned char *key= data;
	uint32_t hash = 0, m_size = NAME_LEN - 1;
	size_t i;

	for (i = 0; i < m_size; i++) {
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return (hash % size);
}

static struct bat_node *node_get(char *name)
{
	struct bat_node *bat_node;

	if (!name)
		return NULL;

	bat_node = (struct bat_node *)hash_find(node_hash, name);
	if (bat_node)
		goto out;

	bat_node = malloc(sizeof(struct bat_node));
	if (!bat_node) {
		fprintf(stderr, "Could not allocate memory for data structure (out of mem?) - skipping");
		return NULL;
	}

	strncpy(bat_node->name, name, NAME_LEN);
	bat_node->name[NAME_LEN - 1] = '\0';
	INIT_LIST_HEAD_FIRST(bat_node->orig_event_list);
	INIT_LIST_HEAD_FIRST(bat_node->rt_table_list);
	memset(bat_node->loop_magic, 0, sizeof(bat_node->loop_magic));
	memset(bat_node->loop_magic2, 0, sizeof(bat_node->loop_magic2));
	hash_add(node_hash, bat_node);

out:
	return bat_node;
}

static struct orig_event *orig_event_new(struct bat_node *bat_node, struct bat_node *orig_node)
{
	struct orig_event *orig_event;

	orig_event = malloc(sizeof(struct orig_event));
	if (!orig_event) {
		fprintf(stderr, "Could not allocate memory for orig event structure (out of mem?) - skipping");
		return NULL;
	}

	INIT_LIST_HEAD(&orig_event->list);
	INIT_LIST_HEAD_FIRST(orig_event->event_list);
	INIT_LIST_HEAD_FIRST(orig_event->rt_hist_list);
	orig_event->orig_node = orig_node;
	list_add_tail(&orig_event->list, &bat_node->orig_event_list);

	return orig_event;
}

static struct orig_event *orig_event_get_by_name(struct bat_node *bat_node, char *orig)
{
	struct bat_node *orig_node;
	struct orig_event *orig_event;

	if (!bat_node)
		return NULL;

	list_for_each_entry(orig_event, &bat_node->orig_event_list, list) {
		if (compare_name(orig_event->orig_node->name, orig))
			return orig_event;
	}

	orig_node = node_get(orig);
	if (!orig_node)
		return NULL;

	return orig_event_new(bat_node, orig_node);
}

static struct orig_event *orig_event_get_by_ptr(struct bat_node *bat_node, struct bat_node *orig_node)
{
	struct orig_event *orig_event;

	if (!bat_node)
		return NULL;

	list_for_each_entry(orig_event, &bat_node->orig_event_list, list) {
		if (orig_event->orig_node == orig_node)
			return orig_event;
	}

	return orig_event_new(bat_node, orig_node);
}

static void node_free(void *data)
{
	struct orig_event *orig_event, *orig_event_tmp;
	struct seqno_event *seqno_event, *seqno_event_tmp;
	struct rt_table *rt_table, *rt_table_tmp;
	struct rt_hist *rt_hist, *rt_hist_tmp;
	struct bat_node *bat_node = (struct bat_node *)data;

	list_for_each_entry_safe(orig_event, orig_event_tmp, &bat_node->orig_event_list, list) {
		list_for_each_entry_safe(seqno_event, seqno_event_tmp, &orig_event->event_list, list) {
			list_del((struct list_head *)&orig_event->event_list, &seqno_event->list, &orig_event->event_list);
			free(seqno_event);
		}

		list_for_each_entry_safe(rt_hist, rt_hist_tmp, &orig_event->rt_hist_list, list) {
			list_del((struct list_head *)&orig_event->rt_hist_list, &rt_hist->list, &orig_event->rt_hist_list);
			free(rt_hist);
		}

		list_del((struct list_head *)&bat_node->orig_event_list, &orig_event->list, &bat_node->orig_event_list);
		free(orig_event);
	}

	list_for_each_entry_safe(rt_table, rt_table_tmp, &bat_node->rt_table_list, list) {
		list_del((struct list_head *)&bat_node->rt_table_list, &rt_table->list, &bat_node->rt_table_list);

		free(rt_table->entries);
		free(rt_table);
	}

	free(bat_node);
}

static int routing_table_new(char *orig, char *next_hop, char *old_next_hop, char rt_flag)
{
	struct bat_node *next_hop_node;
	struct orig_event *orig_event;
	struct seqno_event *seqno_event;
	struct rt_table *rt_table, *prev_rt_table = NULL;
	struct rt_hist *rt_hist;
	int i, j = -1;

	if (!curr_bat_node) {
		fprintf(stderr, "Routing table change without preceding OGM - skipping");
		goto err;
	}

	if (!orig) {
		fprintf(stderr, "Invalid originator found - skipping");
		goto err;
	}

	if ((rt_flag != RT_FLAG_DELETE) && (!next_hop)) {
		fprintf(stderr, "Invalid next hop found - skipping");
		goto err;
	}

	if ((rt_flag == RT_FLAG_UPDATE) && (!old_next_hop)) {
		fprintf(stderr, "Invalid old next hop found - skipping");
		goto err;
	}

	next_hop_node = node_get(next_hop);
	if ((rt_flag != RT_FLAG_DELETE) && (!next_hop_node))
		goto err;

	orig_event = orig_event_get_by_name(curr_bat_node, orig);
	if (!orig_event)
		goto err;

	if (list_empty(&orig_event->event_list)) {
		fprintf(stderr, "Routing table change without any preceding OGM of that originator - skipping");
		goto err;
	}

	if (!compare_name(((struct seqno_event *)(orig_event->event_list.prev))->orig->name, orig)) {
		fprintf(stderr, "Routing table change does not match with last received OGM - skipping");
		goto err;
	}

	rt_table = malloc(sizeof(struct rt_table));
	if (!rt_table) {
		fprintf(stderr, "Could not allocate memory for routing table (out of mem?) - skipping");
		goto err;
	}

	rt_hist = malloc(sizeof(struct rt_hist));
	if (!rt_hist) {
		fprintf(stderr, "Could not allocate memory for routing history (out of mem?) - skipping");
		goto table_free;
	}

	INIT_LIST_HEAD(&rt_table->list);
	rt_table->num_entries = 1;

	INIT_LIST_HEAD(&rt_hist->list);
	rt_hist->prev_rt_hist = NULL;
	rt_hist->next_hop = next_hop_node;
	rt_hist->flags = rt_flag;
	memset(rt_hist->loop_magic, 0, sizeof(rt_hist->loop_magic));

	if (!(list_empty(&orig_event->rt_hist_list)))
		rt_hist->prev_rt_hist = (struct rt_hist *)(orig_event->rt_hist_list.prev);

	if (!(list_empty(&curr_bat_node->rt_table_list)))
		prev_rt_table = (struct rt_table *)(curr_bat_node->rt_table_list.prev);

	switch (rt_flag) {
	case RT_FLAG_ADD:
		if (prev_rt_table)
			rt_table->num_entries = prev_rt_table->num_entries + 1;
		break;
	case RT_FLAG_UPDATE:
		if (prev_rt_table) {
			rt_table->num_entries = prev_rt_table->num_entries + 1;

			/* if we had that route already we just change the entry */
			for (i = 0; i < prev_rt_table->num_entries; i++) {
				if (compare_name(orig, prev_rt_table->entries[i].orig)) {
					rt_table->num_entries = prev_rt_table->num_entries;
					break;
				}
			}
		}
		break;
	case RT_FLAG_DELETE:
		if (prev_rt_table) {
			rt_table->num_entries = prev_rt_table->num_entries + 1;

			/* if we had that route already we just change the entry */
			for (i = 0; i < prev_rt_table->num_entries; i++) {
				if (compare_name(orig, prev_rt_table->entries[i].orig)) {
					rt_table->num_entries = prev_rt_table->num_entries;
					break;
				}
			}

			if (rt_table->num_entries != prev_rt_table->num_entries) {
				fprintf(stderr,
				        "Found a delete entry of orig '%s' but no existing record - skipping",
				        orig);
				goto rt_hist_free;
			}

			/**
			 * we need to create a special seqno event as a timer instead
			 * of an OGM triggered that event
			 */
			seqno_event = malloc(sizeof(struct seqno_event));
			if (!seqno_event) {
				fprintf(stderr, "Could not allocate memory for delete seqno event (out of mem?) - skipping");
				goto rt_hist_free;
			}

			INIT_LIST_HEAD(&seqno_event->list);
			seqno_event->orig = node_get(orig);
			seqno_event->neigh = NULL;
			seqno_event->prev_sender = NULL;
			seqno_event->seqno = -1;
			seqno_event->tq = -1;
			seqno_event->ttl = -1;
			seqno_event->rt_hist = NULL;
			list_add_tail(&seqno_event->list, &orig_event->event_list);
		}
		break;
	default:
		fprintf(stderr, "Unknown rt_flag received: %i - skipping", rt_flag);
		goto rt_hist_free;
	}

	rt_table->entries = malloc(sizeof(struct rt_entry) * rt_table->num_entries);
	if (!rt_table->entries) {
		fprintf(stderr, "Could not allocate memory for routing table entries (out of mem?) - skipping");
		goto rt_hist_free;
	}

	if (prev_rt_table) {
		for (i = 0; i < prev_rt_table->num_entries; i++) {
			/* if we have a previously deleted item don't copy it over */
			if (prev_rt_table->entries[i].flags == RT_FLAG_DELETE) {
				rt_table->num_entries--;
				continue;
			}

			/**
			 * if we delete one item the entries are not in sync anymore,
			 * therefore we need to counters: one for the old and one for
			 * the new routing table
			 */
			j++;

			memcpy((char *)&rt_table->entries[j],
			       (char *)&prev_rt_table->entries[i],
			       sizeof(struct rt_entry));

			if (compare_name(orig, rt_table->entries[j].orig)) {
				if (rt_flag != RT_FLAG_DELETE)
					rt_table->entries[j].next_hop = next_hop_node;
				rt_table->entries[j].flags = rt_flag;
				continue;
			}

			rt_table->entries[j].flags = 0;
		}
	}

	if ((rt_table->num_entries == 1) || (rt_table->num_entries != j + 1)) {
		i = rt_table->num_entries;
		strncpy(rt_table->entries[i - 1].orig, orig, NAME_LEN);
		rt_table->entries[i - 1].orig[NAME_LEN - 1] = '\0';
		rt_table->entries[i - 1].next_hop = next_hop_node;
		rt_table->entries[i - 1].flags = rt_flag;
	}

	rt_table->rt_hist = rt_hist;
	rt_hist->seqno_event = (struct seqno_event *)(orig_event->event_list.prev);
	rt_hist->seqno_event->rt_hist = rt_hist;
	rt_hist->rt_table = rt_table;
	list_add_tail(&rt_table->list, &curr_bat_node->rt_table_list);
	list_add_tail(&rt_hist->list, &orig_event->rt_hist_list);

	return 1;

rt_hist_free:
	free(rt_hist);
table_free:
	free(rt_table);
err:
	return 0;
}

static int seqno_event_new(char *iface_addr, char *orig, char *prev_sender, char *neigh, long long seqno, int tq, int ttl)
{
	struct bat_node *orig_node, *neigh_node, *prev_sender_node;
	struct orig_event *orig_event;
	struct seqno_event *seqno_event;

	if (!iface_addr) {
		fprintf(stderr, "Invalid interface address found - skipping");
		goto err;
	}

	if (!orig) {
		fprintf(stderr, "Invalid originator found - skipping");
		goto err;
	}

	if (!neigh) {
		fprintf(stderr, "Invalid neighbor found - skipping");
		goto err;
	}

	if ((seqno < 0) || (seqno > UINT32_MAX)) {
		fprintf(stderr, "Invalid sequence number found (%lli) - skipping", seqno);
		goto err;
	}

	if ((tq < 0) || (tq > UINT8_MAX)) {
		fprintf(stderr, "Invalid tq value found (%i) - skipping", tq);
		goto err;
	}

	if ((ttl < 0) || (ttl > UINT8_MAX)) {
		fprintf(stderr, "Invalid ttl value found (%i) - skipping", ttl);
		goto err;
	}

	curr_bat_node = node_get(iface_addr);
	if (!curr_bat_node)
		goto err;

	orig_node = node_get(orig);
	if (!orig_node)
		goto err;

	neigh_node = node_get(neigh);
	if (!neigh_node)
		goto err;

	prev_sender_node = node_get(prev_sender);
	if (!prev_sender_node)
		goto err;

	orig_event = orig_event_get_by_ptr(curr_bat_node, orig_node);
	if (!orig_event)
		goto err;

	seqno_event = malloc(sizeof(struct seqno_event));
	if (!seqno_event) {
		fprintf(stderr, "Could not allocate memory for seqno event (out of mem?) - skipping");
		goto err;
	}

	INIT_LIST_HEAD(&seqno_event->list);
	seqno_event->orig = orig_node;
	seqno_event->neigh = neigh_node;
	seqno_event->prev_sender = prev_sender_node;
	seqno_event->seqno = seqno;
	seqno_event->tq = tq;
	seqno_event->ttl = ttl;
	seqno_event->rt_hist = NULL;
	list_add_tail(&seqno_event->list, &orig_event->event_list);

	return 1;

err:
	return 0;
}

static int parse_log_file(char *file_path)
{
	FILE *fd;
	char line_buff[MAX_LINE], *start_ptr, *start_ptr_safe, *tok_ptr;
	char *neigh, *iface_addr, *orig, *prev_sender, rt_flag;
	int line_count = 0, tq, ttl, i, res, max;
	long long seqno;

	fd = fopen(file_path, "r");

	if (!fd) {
		fprintf(stderr, "Error - could not open file '%s': %s\n", file_path, strerror(errno));
		return 0;
	}

	while (fgets(line_buff, sizeof(line_buff), fd) != NULL) {
		/* ignore the timestamp at the beginning of each line */
		start_ptr = line_buff + 13;
		line_count++;

		if (strstr(start_ptr, "Received BATMAN packet via NB")) {
			strtok_r(start_ptr, " ", &start_ptr_safe);
			neigh = iface_addr = orig = prev_sender = NULL;
			seqno = tq = ttl = -1;

			for (i = 0; i < 21; i++) {
				tok_ptr = strtok_r(NULL, " ", &start_ptr_safe);
				if (!tok_ptr)
					break;

				switch (i) {
				case 4:
					neigh = tok_ptr;
					neigh[strlen(neigh) - 1] = 0;
					break;
				case 7:
					iface_addr = tok_ptr + 1;
					iface_addr[strlen(iface_addr) - 1] = 0;
					break;
				case 10:
					orig = tok_ptr;
					orig[strlen(orig) - 1] = 0;
					break;
				case 14:
					prev_sender = tok_ptr;
					prev_sender[strlen(prev_sender) - 1] = 0;
					break;
				case 16:
					seqno = strtoll(tok_ptr, NULL, 10);
					break;
				case 18:
					tq = strtol(tok_ptr, NULL, 10);
					break;
				case 20:
					ttl = strtol(tok_ptr, NULL, 10);
					break;
				}
			}

			if (ttl ==  -1) {
				fprintf(stderr, "Broken 'received packet' line found - skipping [file: %s, line: %i]\n", file_path, line_count);
				continue;
			}

// 			fprintf(stderr, "received packet  (line %i): neigh: '%s', iface_addr: '%s', orig: '%s', prev_sender: '%s', seqno: %i, tq: %i, ttl: %i\n", line_count, neigh, iface_addr, orig, prev_sender, seqno, tq, ttl);

			res = seqno_event_new(iface_addr, orig, prev_sender, neigh, seqno, tq, ttl);
			if (res < 1)
				fprintf(stderr, " [file: %s, line: %i]\n", file_path, line_count);

		} else if (strstr(start_ptr, "Adding route towards") ||
			   strstr(start_ptr, "Changing route towards") ||
			   strstr(start_ptr, "Deleting route towards")) {

			rt_flag = RT_FLAG_UPDATE;
			max = 12;

			if (strstr(start_ptr, "Adding route towards")) {
				rt_flag = RT_FLAG_ADD;
				max = 5;
			} else if (strstr(start_ptr, "Deleting route towards")) {
				rt_flag = RT_FLAG_DELETE;
				max = 3;
			}

			strtok_r(start_ptr, " ", &start_ptr_safe);
			orig = neigh = prev_sender = NULL;

			for (i = 0; i < max; i++) {
				tok_ptr = strtok_r(NULL, " ", &start_ptr_safe);
				if (!tok_ptr)
					break;

				switch (i) {
				case 2:
					orig = tok_ptr;
					if (rt_flag == RT_FLAG_DELETE)
						orig[strlen(orig) - 1] = 0;
					break;
				case 4:
					if (rt_flag == RT_FLAG_ADD) {
						neigh = tok_ptr;
						neigh[strlen(neigh) - 2] = 0;
					}
					break;
				case 5:
					neigh = tok_ptr;
					break;
				case 9:
					prev_sender = tok_ptr;
					prev_sender[strlen(prev_sender) - 2] = 0;
					break;
				}
			}

// 			printf("route (file: %s, line %i): orig: '%s', neigh: '%s', prev_sender: '%s'\n",
// 			       file_path, line_count, orig, neigh, prev_sender);

			if (((rt_flag == RT_FLAG_ADD) && (!neigh)) ||
			    ((rt_flag == RT_FLAG_UPDATE) && (!prev_sender)) ||
			    ((rt_flag == RT_FLAG_DELETE) && (!orig))) {
				fprintf(stderr, "Broken '%s route' line found - skipping [file: %s, line: %i]\n",
				        (rt_flag == RT_FLAG_UPDATE ? "changing" :
				        (rt_flag == RT_FLAG_ADD ? "adding" : "deleting")),
				        file_path, line_count);
				continue;
			}

			res = routing_table_new(orig, neigh, prev_sender, rt_flag);
			if (res < 1)
				fprintf(stderr, " [file: %s, line: %i]\n", file_path, line_count);
		}
	}

// 	printf("File '%s' parsed (lines: %i)\n", file_path, line_count);
	fclose(fd);
	curr_bat_node = NULL;
	return 1;
}

static struct rt_hist *get_rt_hist_by_seqno(struct orig_event *orig_event, long long seqno)
{
	struct seqno_event *seqno_event;
	struct rt_hist *rt_hist = NULL;

	list_for_each_entry(seqno_event, &orig_event->event_list, list) {
		if (seqno_event->seqno > seqno)
			break;

		if (seqno_event->rt_hist)
			rt_hist = seqno_event->rt_hist;
	}

	return rt_hist;
}

static struct rt_hist *get_rt_hist_by_node_seqno(struct bat_node *bat_node, struct bat_node *orig_node, long long seqno)
{
	struct orig_event *orig_event;
	struct rt_hist *rt_hist;

	orig_event = orig_event_get_by_ptr(bat_node, orig_node);
	if (!orig_event)
		return NULL;

	rt_hist = get_rt_hist_by_seqno(orig_event, seqno);
	return rt_hist;
}

static int print_rt_path_at_seqno(struct bat_node *src_node, struct bat_node *dst_node,
                            struct bat_node *next_hop, long long seqno, long long seqno_rand, int read_opt)
{
	struct bat_node *next_hop_tmp;
	struct orig_event *orig_event;
	struct rt_hist *rt_hist;
	char curr_loop_magic[LOOP_MAGIC_LEN];

	snprintf(curr_loop_magic, sizeof(curr_loop_magic), "%s%s%lli%lli", src_node->name,
	         dst_node->name, seqno, seqno_rand);

	printf("Path towards %s (seqno %lli ",
	       get_name_by_macstr(dst_node->name, read_opt), seqno);

	printf("via neigh %s):", get_name_by_macstr(next_hop->name, read_opt));

	next_hop_tmp = next_hop;

	while (1) {
		printf(" -> %s%s",
		       get_name_by_macstr(next_hop_tmp->name, read_opt),
		       (dst_node == next_hop_tmp ? "." : ""));

		/* destination reached */
		if (dst_node == next_hop_tmp)
			break;

		orig_event = orig_event_get_by_ptr(next_hop_tmp, dst_node);
		if (!orig_event)
			goto out;

		/* no more data - path seems[tm] fine */
		if (list_empty(&orig_event->event_list))
			goto out;

		/* same here */
		if (list_empty(&orig_event->rt_hist_list))
			goto out;

		/* we are running in a loop */
		if (memcmp(curr_loop_magic, next_hop_tmp->loop_magic, LOOP_MAGIC_LEN) == 0) {
			printf("   aborted due to loop!");
			goto out;
		}

		memcpy(next_hop_tmp->loop_magic, curr_loop_magic, sizeof(next_hop_tmp->loop_magic));

		rt_hist = get_rt_hist_by_seqno(orig_event, seqno);

		/* no more routing data - what can we do ? */
		if (!rt_hist)
			break;

		next_hop_tmp = rt_hist->next_hop;
	}

out:
	printf("\n");
	return 1;
}

static int find_rt_table_change(struct bat_node *src_node, struct bat_node *dst_node,
                                struct bat_node *curr_node, long long seqno_min, long long seqno_max,
                                long long seqno_rand, int read_opt)
{
	struct orig_event *orig_event;
	struct rt_hist *rt_hist, *rt_hist_tmp;
	char curr_loop_magic[LOOP_MAGIC_LEN], loop_check = 0;
	int res;
	long long seqno_tmp, seqno_min_tmp = seqno_min;

	/* printf("%i: curr_node: %s ", bla,
		       get_name_by_macstr(curr_node->name, read_opt));

	printf("dst_node: %s [%i - %i]\n",
	       get_name_by_macstr(dst_node->name, read_opt), seqno_min, seqno_max); */

	/* recursion ends here */
	if (curr_node == dst_node) {
		rt_hist = get_rt_hist_by_node_seqno(src_node, dst_node, seqno_max);

		if (rt_hist)
			print_rt_path_at_seqno(src_node, dst_node, rt_hist->next_hop,
			                       seqno_max, seqno_rand, read_opt);
		return 0;
	}

	snprintf(curr_loop_magic, sizeof(curr_loop_magic), "%s%s%lli%lli",
	         src_node->name, dst_node->name,
	         seqno_min_tmp, seqno_rand);

	orig_event = orig_event_get_by_ptr(curr_node, dst_node);
	if (!orig_event)
		goto out;

	list_for_each_entry(rt_hist, &orig_event->rt_hist_list, list) {
		/* special seqno that indicates an originator timeout */
		if (rt_hist->seqno_event->seqno == -1) {
			printf("Woot - originator timeout ??\n");
			continue;
		}

		if ((seqno_min_tmp != -1) && (rt_hist->seqno_event->seqno < seqno_min_tmp))
			continue;

		if ((seqno_max != -1) && (rt_hist->seqno_event->seqno >= seqno_max))
			continue;

		/* we are running in a loop */
		if (memcmp(curr_loop_magic, rt_hist->loop_magic, LOOP_MAGIC_LEN) == 0) {
			rt_hist_tmp = get_rt_hist_by_node_seqno(src_node, dst_node,
			                                        rt_hist->seqno_event->seqno);

			if (rt_hist_tmp)
				print_rt_path_at_seqno(src_node, dst_node, rt_hist_tmp->next_hop,
				                       rt_hist->seqno_event->seqno, seqno_rand, read_opt);
			goto loop;
		}

		memcpy(rt_hist->loop_magic, curr_loop_magic, sizeof(rt_hist->loop_magic));
		loop_check = 1;

		/* printf("validate route after change (seqno %i) at node: %s\n",
		       rt_hist->seqno_event->seqno,
		       get_name_by_macstr(curr_node->name, read_opt)); */

		res = find_rt_table_change(src_node, dst_node, rt_hist->next_hop,
		                           seqno_min_tmp, rt_hist->seqno_event->seqno,
		                           seqno_rand, read_opt);

		seqno_min_tmp = rt_hist->seqno_event->seqno + 1;

		/* find_rt_table_change() did not run into a loop and printed the path */
		if (res == 0)
			continue;

		/**
		 * retrieve routing table towards dst at that point and
		 * print the routing path
		 **/
		rt_hist_tmp = get_rt_hist_by_node_seqno(src_node, dst_node, rt_hist->seqno_event->seqno);

		if (!rt_hist_tmp)
			continue;

		print_rt_path_at_seqno(src_node, dst_node, rt_hist_tmp->next_hop,
		      rt_hist->seqno_event->seqno, seqno_rand, read_opt);
	}

	/**
	 * if we have no routing table changes within the seqno range
	 * the loop detection above won't be triggered
	 **/
	if (!loop_check) {
		if (memcmp(curr_loop_magic, curr_node->loop_magic2, LOOP_MAGIC_LEN) == 0) {
			rt_hist_tmp = get_rt_hist_by_node_seqno(src_node, dst_node, seqno_min);

			if (rt_hist_tmp)
				print_rt_path_at_seqno(src_node, dst_node, rt_hist_tmp->next_hop,
				                       seqno_min, seqno_rand, read_opt);

			/* no need to print the path twice */
			if (seqno_min == seqno_max)
				goto out;
			else
				goto loop;
		}

		memcpy(curr_node->loop_magic2, curr_loop_magic, sizeof(curr_node->loop_magic2));
	}

	seqno_tmp = seqno_max - 1;
	if (seqno_min == seqno_max)
		seqno_tmp = seqno_max;

	rt_hist = get_rt_hist_by_seqno(orig_event, seqno_tmp);

	if (rt_hist)
		return find_rt_table_change(src_node, dst_node, rt_hist->next_hop,
		                            seqno_min_tmp, seqno_max, seqno_rand, read_opt);

out:
	return -1;
loop:
	return -2;
}

static void loop_detection(char *loop_orig, long long seqno_min, long long seqno_max, char *filter_orig, int read_opt)
{
	struct bat_node *bat_node;
	struct orig_event *orig_event;
	struct hash_it_t *hashit = NULL;
	struct rt_hist *rt_hist, *prev_rt_hist;
	long long last_seqno = -1, seqno_count = 0;
	int res;
	char check_orig[NAME_LEN];

	printf("\nAnalyzing routing tables ");

	/* if no option was given loop_orig is empty */
	memset(check_orig, 0, NAME_LEN);
	if (!compare_name(loop_orig, check_orig))
		printf("of originator: %s ",
		       get_name_by_macstr(loop_orig, read_opt));

	if ((seqno_min == -1) && (seqno_max == -1))
		printf("[all sequence numbers]");
	else if (seqno_min == seqno_max)
		printf("[sequence number: %lli]", seqno_min);
	else
		printf("[sequence number range: %lli-%lli]", seqno_min, seqno_max);

	if (!compare_name(filter_orig, check_orig))
		printf(" [filter originator: %s]",
		       get_name_by_macstr(filter_orig, read_opt));

	printf("\n");

	while (NULL != (hashit = hash_iterate(node_hash, hashit))) {
		bat_node = hashit->bucket->data;

		if (!compare_name(loop_orig, check_orig) &&
		    !compare_name(loop_orig, bat_node->name))
			continue;

		printf("\nChecking host: %s\n",
		       get_name_by_macstr(bat_node->name, read_opt));

		list_for_each_entry(orig_event, &bat_node->orig_event_list, list) {
			if (bat_node == orig_event->orig_node)
				continue;

			if (!compare_name(filter_orig, check_orig) &&
			    !compare_name(filter_orig, orig_event->orig_node->name))
				continue;

			/* we might have no log file from this node */
			if (list_empty(&orig_event->event_list)) {
				fprintf(stderr, "No seqno data of originator '%s' - skipping\n",
				get_name_by_macstr(orig_event->orig_node->name, read_opt));
				continue;
			}

			/* or routing tables */
			if (list_empty(&orig_event->rt_hist_list)) {
				fprintf(stderr, "No routing history of originator '%s' - skipping\n",
				get_name_by_macstr(orig_event->orig_node->name, read_opt));
				continue;
			}

			list_for_each_entry(rt_hist, &orig_event->rt_hist_list, list) {
				/* special seqno that indicates an originator timeout */
				if (rt_hist->seqno_event->seqno == -1)
					continue;

				if ((seqno_min != -1) && (rt_hist->seqno_event->seqno < seqno_min))
					continue;

				if ((seqno_max != -1) && (rt_hist->seqno_event->seqno > seqno_max))
					continue;

				/**
				 * sometime we change the routing table more than once
				 * with the same seqno
				 */
				if (last_seqno == rt_hist->seqno_event->seqno)
					seqno_count++;
				else
					seqno_count = 0;

				last_seqno = rt_hist->seqno_event->seqno;

				if (rt_hist->flags == RT_FLAG_DELETE) {
					printf("Path towards %s deleted (originator timeout)\n",
						get_name_by_macstr(rt_hist->seqno_event->orig->name, read_opt));
					continue;
				}

				prev_rt_hist = rt_hist->prev_rt_hist;

				if ((prev_rt_hist) &&
				    (rt_hist->seqno_event->seqno != prev_rt_hist->seqno_event->seqno)) {
					if (rt_hist->seqno_event->seqno < prev_rt_hist->seqno_event->seqno) {
						fprintf(stderr,
						        "Smaller seqno (%lli) than previously received seqno (%lli) of orig %s triggered routing table change - skipping recursive check\n",
						        rt_hist->seqno_event->seqno, prev_rt_hist->seqno_event->seqno,
						        get_name_by_macstr(rt_hist->seqno_event->orig->name, read_opt));
						goto validate_path;
					}

					if (rt_hist->seqno_event->seqno == prev_rt_hist->seqno_event->seqno + 1)
						goto validate_path;

					/* printf("\n=> checking orig %s in seqno range of: %i - %i ",
						get_name_by_macstr(rt_hist->seqno_event->orig->name, read_opt),
						prev_rt_hist->seqno_event->seqno + 1,
						rt_hist->seqno_event->seqno);

					printf("(prev nexthop: %s)\n",
						get_name_by_macstr(prev_rt_hist->next_hop->name, read_opt)); */

					res = find_rt_table_change(bat_node, rt_hist->seqno_event->orig,
					                           prev_rt_hist->next_hop,
					                           prev_rt_hist->seqno_event->seqno + 1,
					                           rt_hist->seqno_event->seqno,
					                           seqno_count, read_opt);

					if (res != -2)
						continue;
				}

validate_path:
				print_rt_path_at_seqno(bat_node, rt_hist->seqno_event->orig, rt_hist->next_hop,
				                       rt_hist->seqno_event->seqno, seqno_count, read_opt);
			}
		}
	}
}

static void seqno_trace_print_neigh(struct seqno_trace_neigh *seqno_trace_neigh,
			            struct seqno_event *seqno_event_parent,
			            int num_sisters, char *head, int read_opt)
{
	char new_head[MAX_LINE];
	int i;

	printf("%s%s- %s [tq: %i, ttl: %i", head,
	               (strlen(head) == 1 ? "" : num_sisters == 0 ? "\\" : "|"),
	               get_name_by_macstr(seqno_trace_neigh->bat_node->name, read_opt),
	               seqno_trace_neigh->seqno_event->tq,
	               seqno_trace_neigh->seqno_event->ttl);

	printf(", neigh: %s", get_name_by_macstr(seqno_trace_neigh->seqno_event->neigh->name, read_opt));
	printf(", prev_sender: %s]", get_name_by_macstr(seqno_trace_neigh->seqno_event->prev_sender->name, read_opt));

	if ((seqno_event_parent) &&
		(seqno_trace_neigh->seqno_event->tq > seqno_event_parent->tq))
		printf("  TQ UP!\n");
	else
		printf("\n");

	for (i = 0; i < seqno_trace_neigh->num_neighbors; i++) {
		snprintf(new_head, sizeof(new_head), "%s%s",
		         (strlen(head) > 1 ? head : num_sisters == 0 ? " " : head),
		         (strlen(head) == 1 ? "   " :
		         num_sisters == 0 ? "    " : "|   "));

		seqno_trace_print_neigh(seqno_trace_neigh->seqno_trace_neigh[i], seqno_trace_neigh->seqno_event,
		                        seqno_trace_neigh->num_neighbors - i - 1, new_head, read_opt);
	}
}

static void seqno_trace_print(struct list_head_first *trace_list, char *trace_orig,
                              long long seqno_min, long long seqno_max, char *filter_orig, int read_opt)
{
	struct seqno_trace *seqno_trace;
	char head[MAX_LINE], check_orig[NAME_LEN];
	int i;

	/* if no option was given filter_orig is empty */
	memset(check_orig, 0, NAME_LEN);

	printf("Sequence number flow of originator: %s ",
	       get_name_by_macstr(trace_orig, read_opt));

	if ((seqno_min == -1) && (seqno_max == -1))
		printf("[all sequence numbers]");
	else if (seqno_min == seqno_max)
		printf("[sequence number: %lli]", seqno_min);
	else
		printf("[sequence number range: %lli-%lli]", seqno_min, seqno_max);

	if (!compare_name(filter_orig, check_orig))
		printf(" [filter originator: %s]",
		       get_name_by_macstr(filter_orig, read_opt));

	printf("\n");

	list_for_each_entry(seqno_trace, trace_list, list) {
		if (!seqno_trace->print)
			continue;

		printf("+=> %s (seqno %lli)\n",
		       get_name_by_macstr(trace_orig, read_opt),
		       seqno_trace->seqno);


		for (i = 0; i < seqno_trace->seqno_trace_neigh.num_neighbors; i++) {

			snprintf(head, sizeof(head), "%c",
			         (seqno_trace->seqno_trace_neigh.num_neighbors == i + 1 ? '\\' : '|'));

			seqno_trace_print_neigh(seqno_trace->seqno_trace_neigh.seqno_trace_neigh[i],
			                        NULL,
			                        seqno_trace->seqno_trace_neigh.num_neighbors - i - 1,
			                        head, read_opt);
		}

		printf("\n");
	}
}

static int _seqno_trace_neigh_add(struct seqno_trace_neigh *seqno_trace_mom,
					struct seqno_trace_neigh *seqno_trace_child)
{
	struct seqno_trace_neigh **data_ptr;

	data_ptr = malloc((seqno_trace_mom->num_neighbors + 1) * sizeof(struct seqno_trace_neigh *));
	if (!data_ptr)
		return 0;

	if (seqno_trace_mom->num_neighbors > 0) {
		memcpy(data_ptr, seqno_trace_mom->seqno_trace_neigh,
		       seqno_trace_mom->num_neighbors * sizeof(struct seqno_trace_neigh *));
		free(seqno_trace_mom->seqno_trace_neigh);
	}

	seqno_trace_mom->num_neighbors++;
	seqno_trace_mom->seqno_trace_neigh = data_ptr;
	seqno_trace_mom->seqno_trace_neigh[seqno_trace_mom->num_neighbors - 1] = seqno_trace_child;
	return 1;
}

static struct seqno_trace_neigh *seqno_trace_neigh_add(struct seqno_trace_neigh *seqno_trace_neigh,
		                      struct bat_node *bat_node, struct seqno_event *seqno_event)
{
	struct seqno_trace_neigh *seqno_trace_neigh_new;
	int res;

	seqno_trace_neigh_new = malloc(sizeof(struct seqno_trace_neigh));
	if (!seqno_trace_neigh_new)
		goto err;

	seqno_trace_neigh_new->bat_node = bat_node;
	seqno_trace_neigh_new->seqno_event = seqno_event;
	seqno_trace_neigh_new->num_neighbors = 0;

	res = _seqno_trace_neigh_add(seqno_trace_neigh, seqno_trace_neigh_new);

	if (res < 1)
		goto free_neigh;

	return seqno_trace_neigh_new;

free_neigh:
	free(seqno_trace_neigh_new);
err:
	return NULL;
}

static struct seqno_trace_neigh *seqno_trace_find_neigh(struct bat_node *neigh, struct bat_node *prev_sender,
				struct seqno_trace_neigh *seqno_trace_neigh)
{
	struct seqno_trace_neigh *seqno_trace_neigh_tmp, *seqno_trace_neigh_ret;
	int i;

	for (i = 0; i < seqno_trace_neigh->num_neighbors; i++) {
		seqno_trace_neigh_tmp = seqno_trace_neigh->seqno_trace_neigh[i];

		if ((neigh == seqno_trace_neigh_tmp->bat_node) &&
		    (prev_sender == seqno_trace_neigh_tmp->seqno_event->neigh))
			return seqno_trace_neigh_tmp;

		seqno_trace_neigh_ret = seqno_trace_find_neigh(neigh, prev_sender, seqno_trace_neigh_tmp);

		if (seqno_trace_neigh_ret)
			return seqno_trace_neigh_ret;
	}

	return NULL;
}

static void seqno_trace_neigh_free(struct seqno_trace_neigh *seqno_trace_neigh)
{
	int i;

	for (i = 0; i < seqno_trace_neigh->num_neighbors; i++)
		seqno_trace_neigh_free(seqno_trace_neigh->seqno_trace_neigh[i]);

	if (seqno_trace_neigh->num_neighbors > 0)
		free(seqno_trace_neigh->seqno_trace_neigh);

	free(seqno_trace_neigh);
}

static int seqno_trace_fix_leaf(struct seqno_trace_neigh *seqno_trace_mom,
					struct seqno_trace_neigh *seqno_trace_old_mom,
					struct seqno_trace_neigh *seqno_trace_child)
{
	struct seqno_trace_neigh **data_ptr, *seqno_trace_neigh;
	int i, j = 0;

	data_ptr = malloc((seqno_trace_old_mom->num_neighbors - 1) * sizeof(struct seqno_trace_neigh *));
	if (!data_ptr)
		return 0;

	/* copy all children except the child that is going to move */
	for (i = 0; i < seqno_trace_old_mom->num_neighbors; i++) {
		seqno_trace_neigh = seqno_trace_old_mom->seqno_trace_neigh[i];

		if (seqno_trace_neigh != seqno_trace_child) {
			data_ptr[j] = seqno_trace_neigh;
			j++;
		}
	}

	seqno_trace_old_mom->num_neighbors--;
	free(seqno_trace_old_mom->seqno_trace_neigh);
	seqno_trace_old_mom->seqno_trace_neigh = data_ptr;

	return _seqno_trace_neigh_add(seqno_trace_mom, seqno_trace_child);
}

static int seqno_trace_check_leaves(struct seqno_trace *seqno_trace, struct seqno_trace_neigh *seqno_trace_neigh_new)
{
	struct seqno_trace_neigh *seqno_trace_neigh_tmp;
	int i, res;

	for (i = 0; i < seqno_trace->seqno_trace_neigh.num_neighbors; i++) {
		seqno_trace_neigh_tmp = seqno_trace->seqno_trace_neigh.seqno_trace_neigh[i];

		if ((seqno_trace_neigh_tmp->seqno_event->neigh == seqno_trace_neigh_new->bat_node) &&
		    (seqno_trace_neigh_tmp->seqno_event->prev_sender == seqno_trace_neigh_new->seqno_event->neigh)) {
			res = seqno_trace_fix_leaf(seqno_trace_neigh_new, &seqno_trace->seqno_trace_neigh, seqno_trace_neigh_tmp);

			if (res < 1)
				return res;

			/* restart checking procedure because we just changed the array we are working on */
			return seqno_trace_check_leaves(seqno_trace, seqno_trace_neigh_new);
		}
	}

	return 1;
}

static struct seqno_trace *seqno_trace_new(struct seqno_event *seqno_event)
{
	struct seqno_trace *seqno_trace;

	seqno_trace = malloc(sizeof(struct seqno_trace));
	if (!seqno_trace) {
		fprintf(stderr, "Could not allocate memory for seqno tracing data (out of mem?)\n");
		return NULL;
	}

	INIT_LIST_HEAD(&seqno_trace->list);
	seqno_trace->seqno = seqno_event->seqno;
	seqno_trace->print = 0;

	seqno_trace->seqno_trace_neigh.num_neighbors = 0;

	return seqno_trace;
}

static void seqno_trace_free(struct seqno_trace *seqno_trace)
{
	int i;

	for (i = 0; i < seqno_trace->seqno_trace_neigh.num_neighbors; i++)
		seqno_trace_neigh_free(seqno_trace->seqno_trace_neigh.seqno_trace_neigh[i]);

	free(seqno_trace);
}

static int seqno_trace_add(struct list_head_first *trace_list, struct bat_node *bat_node,
		           struct seqno_event *seqno_event, char print_trace)
{
	struct seqno_trace *seqno_trace = NULL, *seqno_trace_tmp = NULL, *seqno_trace_prev = NULL;
	struct seqno_trace_neigh *seqno_trace_neigh;

	list_for_each_entry(seqno_trace_tmp, trace_list, list) {
		if (seqno_trace_tmp->seqno == seqno_event->seqno) {
			seqno_trace = seqno_trace_tmp;
			break;
		}

		if (seqno_trace_tmp->seqno > seqno_event->seqno)
			break;

		seqno_trace_prev = seqno_trace_tmp;
	}

	if (!seqno_trace) {
		seqno_trace = seqno_trace_new(seqno_event);
		if (!seqno_trace)
			goto err;

		if ((list_empty(trace_list)) ||
		    (seqno_event->seqno > ((struct seqno_trace *)trace_list->prev)->seqno))
			list_add_tail(&seqno_trace->list, trace_list);
		else if (seqno_event->seqno < ((struct seqno_trace *)trace_list->next)->seqno)
			list_add_before((struct list_head *)trace_list, trace_list->next, &seqno_trace->list);
		else
			list_add_before(&seqno_trace_prev->list, &seqno_trace_tmp->list, &seqno_trace->list);
	}

	if (print_trace)
		seqno_trace->print = print_trace;

	seqno_trace_neigh = seqno_trace_find_neigh(seqno_event->neigh,
				                   seqno_event->prev_sender,
				                   &seqno_trace->seqno_trace_neigh);

	/* no neighbor found to hook up to - adding new root node */
	if (!seqno_trace_neigh)
		seqno_trace_neigh = seqno_trace_neigh_add(&seqno_trace->seqno_trace_neigh,
				                          bat_node, seqno_event);
	else
		seqno_trace_neigh = seqno_trace_neigh_add(seqno_trace_neigh, bat_node, seqno_event);

	if (seqno_trace_neigh)
		seqno_trace_check_leaves(seqno_trace, seqno_trace_neigh);

	return 1;

err:
	return 0;
}

static void trace_seqnos(char *trace_orig, long long seqno_min, long long seqno_max, char *filter_orig, int read_opt)
{
	struct bat_node *bat_node;
	struct orig_event *orig_event;
	struct seqno_event *seqno_event;
	struct hash_it_t *hashit = NULL;
	struct list_head_first trace_list;
	struct seqno_trace *seqno_trace, *seqno_trace_tmp;
	char check_orig[NAME_LEN], print_trace;
	int res;

	/* if no option was given filter_orig is empty */
	memset(check_orig, 0, NAME_LEN);
	INIT_LIST_HEAD_FIRST(trace_list);

	while (NULL != (hashit = hash_iterate(node_hash, hashit))) {
		bat_node = hashit->bucket->data;

		list_for_each_entry(orig_event, &bat_node->orig_event_list, list) {

			/* we might have no log file from this node */
			if (list_empty(&orig_event->event_list))
				continue;

			list_for_each_entry(seqno_event, &orig_event->event_list, list) {
				/* special seqno that indicates an originator timeout */
				if (seqno_event->seqno == -1)
					continue;

				if (!compare_name(trace_orig, seqno_event->orig->name))
					continue;

				if ((seqno_min != -1) && (seqno_event->seqno < seqno_min))
					continue;

				if ((seqno_max != -1) && (seqno_event->seqno > seqno_max))
					continue;

				/* if no filter option was given all seqno traces are to be printed */
				print_trace = compare_name(filter_orig, check_orig);

				if (!compare_name(filter_orig, check_orig) &&
				    compare_name(filter_orig, bat_node->name))
					print_trace = 1;

				res = seqno_trace_add(&trace_list, bat_node, seqno_event, print_trace);

				if (res < 1) {
					hash_iterate_free(hashit);
					goto out;
				}
			}
		}
	}

	seqno_trace_print(&trace_list, trace_orig, seqno_min, seqno_max, filter_orig, read_opt);

out:
	list_for_each_entry_safe(seqno_trace, seqno_trace_tmp, &trace_list, list) {
		list_del((struct list_head *)&trace_list, &seqno_trace->list, &trace_list);
		seqno_trace_free(seqno_trace);
	}

	return;
}

static void print_rt_tables(char *rt_orig, long long seqno_min, long long seqno_max, char *filter_orig, int read_opt)
{
	struct bat_node *bat_node;
	struct rt_table *rt_table;
	struct seqno_event *seqno_event;
	char check_orig[NAME_LEN];
	int i;

	/* if no option was given filter_orig is empty */
	memset(check_orig, 0, NAME_LEN);

	printf("Routing tables of originator: %s ",
	       get_name_by_macstr(rt_orig, read_opt));

	if ((seqno_min == -1) && (seqno_max == -1))
		printf("[all sequence numbers]");
	else if (seqno_min == seqno_max)
		printf("[sequence number: %lli]", seqno_min);
	else
		printf("[sequence number range: %lli-%lli]", seqno_min, seqno_max);

	if (!compare_name(filter_orig, check_orig))
		printf(" [filter originator: %s]",
		       get_name_by_macstr(filter_orig, read_opt));

	printf("\n");

	bat_node = node_get(rt_orig);
	if (!bat_node)
		goto out;

	/* we might have no log file from this node */
	if (list_empty(&bat_node->rt_table_list))
		goto out;

	list_for_each_entry(rt_table, &bat_node->rt_table_list, list) {
		seqno_event = rt_table->rt_hist->seqno_event;

		if (!compare_name(filter_orig, check_orig) &&
		    !compare_name(filter_orig, seqno_event->orig->name))
			continue;

		if ((seqno_min != -1) && (seqno_event->seqno < seqno_min))
			continue;

		if ((seqno_max != -1) && (seqno_event->seqno > seqno_max))
			continue;

		if (seqno_event->seqno > -1) {
			printf("rt change triggered by OGM from: %s (tq: %i, ttl: %i, seqno %lli",
			       get_name_by_macstr(seqno_event->orig->name, read_opt),
			       seqno_event->tq, seqno_event->ttl, seqno_event->seqno);
			printf(", neigh: %s",
			       get_name_by_macstr(seqno_event->neigh->name, read_opt));
			printf(", prev_sender: %s)\n",
			       get_name_by_macstr(seqno_event->prev_sender->name, read_opt));
		} else {
			printf("rt change triggered by originator timeout: \n");
		}

		for (i = 0; i < rt_table->num_entries; i++) {
			printf("%s %s via next hop",
			       (rt_table->entries[i].flags ? "   *" : "    "),
			       get_name_by_macstr(rt_table->entries[i].orig, read_opt));
			printf(" %s",
			       get_name_by_macstr(rt_table->entries[i].next_hop->name, read_opt));

			switch (rt_table->entries[i].flags) {
			case RT_FLAG_ADD:
				printf(" (route added)\n");
				break;
			case RT_FLAG_UPDATE:
				printf(" (next hop changed)\n");
				break;
			case RT_FLAG_DELETE:
				printf(" (route deleted)\n");
				break;
			default:
				printf("\n");
				break;
			}
		}

		printf("\n");
	}

out:
	return;
}

static int get_orig_addr(char *orig_name, char *orig_addr)
{
	struct bat_host *bat_host;
	struct ether_addr *orig_mac;
	char *orig_name_tmp = orig_name;

	bat_host = bat_hosts_find_by_name(orig_name_tmp);

	if (bat_host) {
		orig_name_tmp = ether_ntoa_long((struct ether_addr *)&bat_host->mac_addr);
		goto copy_name;
	}

	orig_mac = ether_aton(orig_name_tmp);

	if (!orig_mac) {
		fprintf(stderr, "Error - the originator is not a mac address or bat-host name: %s\n", orig_name);
		goto err;
	}

	/**
	* convert the given mac address to the long format to
	* make sure we can find it
	*/
	orig_name_tmp = ether_ntoa_long(orig_mac);

copy_name:
	strncpy(orig_addr, orig_name_tmp, NAME_LEN);
	orig_addr[NAME_LEN - 1] = '\0';
	return 1;

err:
	return 0;
}

int bisect_iv(int argc, char **argv)
{
	int ret = EXIT_FAILURE, res, optchar, found_args = 1;
	int read_opt = USE_BAT_HOSTS, num_parsed_files;
	long long tmp_seqno, seqno_max = -1, seqno_min = -1;
	char *trace_orig_ptr = NULL, *rt_orig_ptr = NULL, *loop_orig_ptr = NULL;
	char orig[NAME_LEN], filter_orig[NAME_LEN], *dash_ptr, *filter_orig_ptr = NULL;

	memset(orig, 0, NAME_LEN);
	memset(filter_orig, 0, NAME_LEN);

	while ((optchar = getopt(argc, argv, "hl:no:r:s:t:")) != -1) {
		switch (optchar) {
		case 'h':
			bisect_iv_usage();
			return EXIT_SUCCESS;
		case 'l':
			loop_orig_ptr = optarg;
			found_args += ((*((char*)(optarg - 1)) == optchar ) ? 1 : 2);
			break;
		case 'n':
			read_opt &= ~USE_BAT_HOSTS;
			found_args += 1;
			break;
		case 'o':
			filter_orig_ptr = optarg;
			found_args += ((*((char*)(optarg - 1)) == optchar ) ? 1 : 2);
			break;
		case 'r':
			rt_orig_ptr = optarg;
			found_args += ((*((char*)(optarg - 1)) == optchar ) ? 1 : 2);
			break;
		case 's':
			dash_ptr = strchr(optarg, '-');
			if (dash_ptr)
				*dash_ptr = 0;

			tmp_seqno = strtol(optarg, NULL , 10);
			if ((tmp_seqno >= 0) && (tmp_seqno <= UINT32_MAX))
				seqno_min = tmp_seqno;
			else
				fprintf(stderr, "Warning - given sequence number is out of range: %lli\n", tmp_seqno);

			if (dash_ptr) {
				tmp_seqno = strtol(dash_ptr + 1, NULL , 10);
				if ((tmp_seqno >= 0) && (tmp_seqno <= UINT32_MAX))
					seqno_max = tmp_seqno;
				else
					fprintf(stderr, "Warning - given sequence number is out of range: %lli\n", tmp_seqno);

				*dash_ptr = '-';
			}

			found_args += ((*((char*)(optarg - 1)) == optchar ) ? 1 : 2);
			break;
		case 't':
			trace_orig_ptr = optarg;
			found_args += ((*((char*)(optarg - 1)) == optchar ) ? 1 : 2);
			break;
		default:
			bisect_iv_usage();
			return EXIT_FAILURE;
		}
	}

	if (argc <= found_args + 1) {
		fprintf(stderr, "Error - need at least 2 log files to compare\n");
		bisect_iv_usage();
		goto err;
	}

	node_hash = hash_new(64, compare_name, choose_name);

	if (!node_hash) {
		fprintf(stderr, "Error - could not create node hash table\n");
		goto err;
	}

	bat_hosts_init(read_opt);
	num_parsed_files = 0;

	if ((rt_orig_ptr) && (trace_orig_ptr)) {
		fprintf(stderr, "Error - the 'print routing table' option can't be used together with the 'trace seqno' option\n");
		goto err;
	} else if ((loop_orig_ptr) && (trace_orig_ptr)) {
		fprintf(stderr, "Error - the 'loop detection' option can't be used together with the 'trace seqno' option\n");
		goto err;
	} else if ((loop_orig_ptr) && (rt_orig_ptr)) {
		fprintf(stderr, "Error - the 'loop detection' option can't be used together with the 'print routing table' option\n");
		goto err;
	} else if (rt_orig_ptr) {
		res = get_orig_addr(rt_orig_ptr, orig);

		if (res < 1)
			goto err;
	} else if (trace_orig_ptr) {
		res = get_orig_addr(trace_orig_ptr, orig);

		if (res < 1)
			goto err;
	} else if (loop_orig_ptr) {
		res = get_orig_addr(loop_orig_ptr, orig);

		if (res < 1)
			goto err;
	}

	/* we search a specific seqno - no range */
	if ((seqno_min > 0) && (seqno_max == -1))
		seqno_max = seqno_min;

	if (seqno_min > seqno_max) {
		fprintf(stderr, "Error - the sequence range minimum (%lli) should be smaller than the maximum (%lli)\n",
		       seqno_min, seqno_max);
		goto err;
	}

	if (filter_orig_ptr) {
		res = get_orig_addr(filter_orig_ptr, filter_orig);

		if (res < 1)
			goto err;
	}

	while (argc > found_args) {
		res = parse_log_file(argv[found_args]);

		if (res > 0)
			num_parsed_files++;

		found_args++;
	}

	if (num_parsed_files < 2) {
		fprintf(stderr, "Error - need at least 2 log files to compare\n");
		goto err;
	}

	if (trace_orig_ptr)
		trace_seqnos(orig, seqno_min, seqno_max, filter_orig, read_opt);
	else if (rt_orig_ptr)
		print_rt_tables(orig, seqno_min, seqno_max, filter_orig, read_opt);
	else
		loop_detection(orig, seqno_min, seqno_max, filter_orig, read_opt);

	ret = EXIT_SUCCESS;

err:
	if (node_hash)
		hash_delete(node_hash, node_free);
	bat_hosts_free();
	return ret;
}
