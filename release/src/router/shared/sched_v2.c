#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#if 0
#include <math.h>
#endif
#include <shutils.h>
#include <sched_v2.h>
#include <bcmnvram.h>
#include <shared.h>

static int strtonl(const char *ptr, int n, int base) {
	char *buf = (char *)malloc(n+1);
	char *pNext;
	//char buf[MAX_NVRAM_SCHED_LEN];
	int lvalue;
	int ret;
	if (!buf) {
		_dprintf("No memory!!(strtonl)\n");
		return -1;
	}
	snprintf(buf, n+1, "%s", ptr);
	//SCHED_DBG("buf=%s", buf);
	lvalue = strtol(buf, &pNext, base);
	//SCHED_DBG("buf=%s(%d), buf=%p, pNext=%p", buf, lvalue, buf, pNext);
	if(buf+n != pNext)
		ret = -1;
	else
		ret = lvalue;
	free(buf);
	return ret;
}

static unsigned int strtonul(const char *ptr, int n, int base) {
	char *buf = (char *)malloc(n+1);
	//char buf[MAX_NVRAM_SCHED_LEN];
	unsigned int ulvalue;
	if (!buf) {
		_dprintf("No memory!!(strtonul)\n");
		return 0;
	}
	snprintf(buf, n+1, "%s", ptr);
	//SCHED_DBG("buf=%s", buf);
	ulvalue = strtoul(buf, NULL, base);
	free(buf);
	return ulvalue;
}

static int time_to_minute(int hour, int minute) {
	return hour*60+minute;
}

static void free_sched_v1_list(sched_v1_t **sched_v1_list) {
	sched_v1_t *tmp_sched_v1, *old_sched_v1;

	if(sched_v1_list == NULL)
		return;

	tmp_sched_v1 = *sched_v1_list;
	while(tmp_sched_v1 != NULL) {
		old_sched_v1 = tmp_sched_v1;
		tmp_sched_v1 = tmp_sched_v1->next;
		free(old_sched_v1);
	}

	return;
}

static int parse_str_v1_to_sched_v1_list(const char *str_sched_v1, sched_v1_t **sched_v1_list) {
	int ret = -1, i;
	char tmp_str_sched[MAX_NVRAM_SCHED_LEN];
	char word[MAX_NVRAM_SCHED_LEN], *next_word;
	sched_v1_t **tmp_sched_v1, *prev_v1 = NULL;

	if (!str_sched_v1)
		return ret;

	snprintf(tmp_str_sched, sizeof(tmp_str_sched), "%s", str_sched_v1);
	SCHED_DBG("tmp_str_sched:%s", tmp_str_sched);

	*sched_v1_list  = NULL;
	tmp_sched_v1 = sched_v1_list;

	foreach_60(word, tmp_str_sched, next_word) {
		*tmp_sched_v1 = (sched_v1_t *)malloc(sizeof(sched_v1_t));
		if(*tmp_sched_v1 == NULL) {
			ret = -2;
			_dprintf("No memory!!(parse_str_v1_to_sched_v1_list)\n");
			return ret;
		}

		if (!strcmp(word, "T")) // for parantal control.
			goto NEXT;

		memset(*tmp_sched_v1, 0, sizeof(sched_v1_t));
		(*tmp_sched_v1)->prev = prev_v1;

		for (i = 0; i < strlen(word); i++) {
			switch(i) {
			case SCHED_V1_IDX_START_DAY:
				(*tmp_sched_v1)->start_day = strtonl(&word[i], 1, 10);
				if((*tmp_sched_v1)->start_day >= 7){
					(*tmp_sched_v1)->start_day -= 7;
				}
				if((*tmp_sched_v1)->start_day < 0)
					goto NEXT;
				break;
			case SCHED_V1_IDX_END_DAY:
				(*tmp_sched_v1)->end_day = strtonl(&word[i], 1, 10);
				if((*tmp_sched_v1)->end_day >= 7){
					(*tmp_sched_v1)->end_day -= 7;
				}
				if((*tmp_sched_v1)->end_day < 0)
					goto NEXT;
				break;
			case SCHED_V1_IDX_START_HOUR:
				(*tmp_sched_v1)->start_hour = strtonl(&word[i], 2, 10);
				if((*tmp_sched_v1)->start_hour >= 24){
					(*tmp_sched_v1)->start_day += 1;
					(*tmp_sched_v1)->start_hour -= 24;
				}
				if((*tmp_sched_v1)->start_hour < 0)
					goto NEXT;
				break;
			case SCHED_V1_IDX_END_HOUR:
				(*tmp_sched_v1)->end_hour = strtonl(&word[i], 2, 10);
				if((*tmp_sched_v1)->end_hour >= 24){
					(*tmp_sched_v1)->end_day += 1;
					(*tmp_sched_v1)->end_hour -= 24;
				}
				if((*tmp_sched_v1)->end_hour < 0)
					goto NEXT;
				break;
			}
		}
		/*SCHED_DBG("word:%s, start_day:%d, end_day:%d, start_hour:%d, end_hour:%d", 
			word, (*tmp_sched_v1)->start_day, (*tmp_sched_v1)->end_day, (*tmp_sched_v1)->start_hour, (*tmp_sched_v1)->end_hour);*/

		prev_v1 = *tmp_sched_v1;
		while(*tmp_sched_v1 != NULL)
			tmp_sched_v1 = &((*tmp_sched_v1)->next);

		continue;
NEXT:
		SCHED_DBG("skip:%s", word);
		free(*tmp_sched_v1);
		*tmp_sched_v1 = NULL;
		continue;
	}
	ret = 0;
	return ret;
}

void free_sched_v2_list(sched_v2_t **sched_v2_list){
	sched_v2_t *tmp_sched_v2, *old_sched_v2;

	if(sched_v2_list == NULL)
		return;

	tmp_sched_v2 = *sched_v2_list;
	while(tmp_sched_v2 != NULL){
		old_sched_v2 = tmp_sched_v2;
		tmp_sched_v2 = tmp_sched_v2->next;
		free(old_sched_v2);
	}

	return;
}

static sched_v2_type get_sched_v2_etype(const char c_type) {
	sched_v2_type e_type = SCHED_V2_TYPE_UNKNOWN;
	//SCHED_DBG("%d", c_type);
	if (c_type == 'W')
		e_type = SCHED_V2_TYPE_WEEK;
	else if (c_type == 'M')
		e_type = SCHED_V2_TYPE_WEEK_ONLINE;
	else if (c_type == 'D')
		e_type = SCHED_V2_TYPE_DAY;
	else if (c_type == 'T')
		e_type = SCHED_V2_TYPE_TIMESTAMP;
	return e_type;
}

static char get_sched_v2_ctype(const sched_v2_type e_type) {
	char c_type = 'U';
	//SCHED_DBG("%d", c_type);
	if (e_type == SCHED_V2_TYPE_WEEK)
		c_type = 'W';
	else if (e_type == SCHED_V2_TYPE_WEEK_ONLINE)
		c_type = 'M';
	else if (e_type == SCHED_V2_TYPE_DAY)
		c_type = 'D';
	return c_type;
}

inline static int is_merged_day_of_week(int day_of_week) {
	if(day_of_week != (1 << 0) && 
		day_of_week != (1 << 1) && 
		day_of_week != (1 << 2) && 
		day_of_week != (1 << 3) && 
		day_of_week != (1 << 4) && 
		day_of_week != (1 << 5) && 
		day_of_week != (1 << 6))
		return 1;
	else
		return 0;
}

inline static int is_same_w_period(const sched_v2_t *sched1, const sched_v2_t *sched2) {
	if (sched1 && sched2 && 
		sched1->type == sched2->type && 
		(((sched1->type == SCHED_V2_TYPE_WEEK || sched1->type == SCHED_V2_TYPE_WEEK_ONLINE) && 
			sched1->value_w.enable == sched2->value_w.enable && 
			sched1->value_w.start_hour == sched2->value_w.start_hour && 
			sched1->value_w.start_minute == sched2->value_w.start_minute && 
			sched1->value_w.end_hour == sched2->value_w.end_hour && 
			sched1->value_w.end_minute == sched2->value_w.end_minute)))
		return 1;
	return 0;
}

static int expand_sched_v2_same_w_period(sched_v2_t **sched_v2_list) {
	sched_v2_t *tmp_sched_v2_list = *sched_v2_list;
	sched_v2_t *head, *curr, *new, *tmp;
	int i;

	/*for(curr1 = tmp_sched_v2_list; curr1 != NULL; curr1 = curr1->next) {
		SCHED_DBG("%p", curr1);
	}*/
	if (!*sched_v2_list)
		return -1;

	head = *sched_v2_list;

	for(curr = tmp_sched_v2_list; curr != NULL; curr = curr->next) {
		//SCHED_DBG("is_merged_day_of_week %u", curr->value_w.rule_to_number);
		if (is_merged_day_of_week(curr->value_w.day_of_week)) {
			SCHED_DBG("Merged day of week detected %u", curr->value_w.rule_to_number);
			for (i = 6; i >= 0; i--) {
				if ((curr->value_w.day_of_week & (1 << i)) > 0) {
					char tmp_rule[7];
					new = malloc(sizeof(sched_v2_t));
					if (!new) {
						_dprintf("No memory!!(expand_sched_v2_same_w_period)\n");
						continue;
					}
					memset(new, 0, sizeof(sched_v2_t));
					memcpy(&new->value_w, &curr->value_w, sizeof(sched_v2_w_t));
					new->value_w.day_of_week = (1 << i);
					snprintf(tmp_rule, sizeof(tmp_rule), "%02x%02d%02d", new->value_w.day_of_week, new->value_w.start_hour, new->value_w.start_minute);
					//SCHED_DBG("is_merged_day_of_week tmp_rule=%s", tmp_rule);
					new->value_w.rule_to_number = strtonul(tmp_rule, strlen(tmp_rule), 10);
					new->prev = NULL;
					head->prev = new;
					new->next = head;
					head = new;
				}
			}

			if (curr->prev)
				curr->prev->next = curr->next;
			if (curr->next)
				curr->next->prev = curr->prev;

			tmp = curr;
			curr = curr->prev;
			tmp->prev = NULL;
			tmp->next = NULL;
			free(tmp);
		}
	}

	*sched_v2_list = head;
	/*for(curr = *sched_v2_list; curr != NULL; curr = curr->next) {
		SCHED_DBG("prev=%p, curr=%p, next=%p, rule_to_number=%d", curr->prev, curr, curr->next, curr->value_w.rule_to_number);
	}*/
	return 0;
}

static int sort_sched_v2_list(sched_v2_t **sched_v2_list) {
	sched_v2_t *curr = NULL, *index = NULL, *head = *sched_v2_list;
	sched_v2_w_t temp;
	//Check whether list is empty
	if(head == NULL) {
		return -1;
	}
	else {
		//Current will point to head
		for(curr = head; curr->next != NULL; curr = curr->next) {
			//Index will point to node next to curr
			for(index = curr->next; index != NULL; index = index->next) {
				//If curr's data is greater than index's data, swap the data of curr and index
				if ((curr->type == index->type) && 
					(curr->type == SCHED_V2_TYPE_WEEK || curr->type == SCHED_V2_TYPE_WEEK_ONLINE) &&
					(curr->value_w.rule_to_number > index->value_w.rule_to_number)) {
					temp = curr->value_w;
					curr->value_w = index->value_w;
					index->value_w = temp;
				}
			}
		}
	}
	/*for(curr = *sched_v2_list; curr != NULL; curr = curr->next) {
		SCHED_DBG("prev=%p, curr=%p, next=%p, rule_to_number=%d", curr->prev, curr, curr->next, curr->value_w.rule_to_number);
	}*/
	return 0;
}

static int merge_sched_v2_same_w_period(sched_v2_t **sched_v2_list) {
	sched_v2_t *tmp_sched_v2_list = *sched_v2_list;
	sched_v2_t *curr1, *curr2, *next;

	/*for(curr1 = tmp_sched_v2_list; curr1 != NULL; curr1 = curr1->next) {
		SCHED_DBG("%p", curr1);
	}*/
	if (!*sched_v2_list)
		return -1;

	for(curr1 = tmp_sched_v2_list; curr1 != NULL; curr1 = curr1->next) {
		next = curr1->next;
		//SCHED_DBG("%p:type1=%d", next, curr1->type);
		for (curr2 = next; curr2 != NULL; curr2 = curr2->next) {
			//SCHED_DBG("%p:type2=%d", curr2, curr2->type);
			if (is_same_w_period(curr1, curr2)) {
				curr1->value_w.day_of_week |= curr2->value_w.day_of_week;
				//SCHED_DBG("day_of_week=%d", curr1->value_w.day_of_week);
				// remove sched_v2
				sched_v2_t *prev_curr2 = curr2->prev, *next_curr2 = curr2->next;
				//SCHED_DBG("prev_curr2=%p, next_curr2=%p", prev_curr2, next_curr2);
				if (prev_curr2)
					prev_curr2->next = next_curr2;
				if (next_curr2)
					next_curr2->prev = prev_curr2;
				curr2->prev = curr2->next = NULL;
				free_sched_v2_list(&curr2);
				if (next_curr2)
					curr2 = next_curr2;
			}
		}
	}
	/*for(curr1 = tmp_sched_v2_list; curr1 != NULL; curr1 = curr1->next) {
		SCHED_DBG("%p", curr1);
	}*/
	return 0;
}

static char *gen_v2_str_from_sched_v2_list(sched_v2_t *sched_v2_list, char *buf, int buf_size) {
	sched_v2_t *tmp_sched_v2_list = sched_v2_list;
	sched_v2_t *sched_v2;
	char *tmp_buf = buf;

	for(sched_v2 = tmp_sched_v2_list; sched_v2 != NULL; sched_v2 = sched_v2->next) {
		if (sched_v2->type == SCHED_V2_TYPE_DAY)
			snprintf(tmp_buf+strlen(tmp_buf), buf_size-strlen(tmp_buf), "%c%d%02d%02d%02d%02d<", 
				get_sched_v2_ctype(sched_v2->type), sched_v2->value_d.enable, 
				sched_v2->value_d.start_hour, sched_v2->value_d.start_minute, 
				sched_v2->value_d.end_hour, sched_v2->value_d.end_minute);
		else if (sched_v2->type == SCHED_V2_TYPE_WEEK || sched_v2->type == SCHED_V2_TYPE_WEEK_ONLINE)
			snprintf(tmp_buf+strlen(tmp_buf), buf_size-strlen(tmp_buf), "%c%d%02X%02d%02d%02d%02d<", 
				get_sched_v2_ctype(sched_v2->type), 
				sched_v2->value_w.enable, sched_v2->value_w.day_of_week, 
				sched_v2->value_w.start_hour, sched_v2->value_w.start_minute, 
				sched_v2->value_w.end_hour, sched_v2->value_w.end_minute);
	}

	if (strlen(tmp_buf) && tmp_buf[strlen(tmp_buf)-1] == '<')
		tmp_buf[strlen(tmp_buf)-1] = '\0'; // strip the last char '<'

	if (strlen(buf))
		return buf;

	return NULL;
}

static int parse_sched_v2_rule(const char *word, sched_v2_t **tmp_sched_v2, int skip_disabled) {
	int i, ret = -1;
	i = 0;
	if (!word)
		return ret;

	(*tmp_sched_v2)->type = get_sched_v2_etype(word[i]);

	if ((*tmp_sched_v2)->type == SCHED_V2_TYPE_UNKNOWN) {
		SCHED_DBG("unknown type. rule=%s", word);
		return ret;
	}
	if (((*tmp_sched_v2)->type == SCHED_V2_TYPE_WEEK && strlen(word) != SCHED_V2_W_IDX_NULL) ||
		((*tmp_sched_v2)->type == SCHED_V2_TYPE_WEEK_ONLINE && strlen(word) != SCHED_V2_W_IDX_NULL) ||
	    ((*tmp_sched_v2)->type == SCHED_V2_TYPE_DAY && strlen(word) != SCHED_V2_D_IDX_NULL) ||
	    ((*tmp_sched_v2)->type == SCHED_V2_TYPE_TIMESTAMP && strlen(word) != SCHED_V2_T_IDX_NULL)) {
		SCHED_DBG("invalid length of rule. rule=%s", word);
		return ret;
	}

	//SCHED_DBG("%s", word);
	for (i = 1; i < strlen(word); i++) {
		if ((*tmp_sched_v2)->type == SCHED_V2_TYPE_DAY) {
			switch(i) {
				case SCHED_V2_D_IDX_ENABLE:
					(*tmp_sched_v2)->value_d.enable = strtonl(&word[i], (SCHED_V2_D_IDX_START_HOUR - SCHED_V2_D_IDX_ENABLE), 10);
					if (skip_disabled && (*tmp_sched_v2)->value_d.enable <= 0) { // Skip disabled rules
						//free((*tmp_sched_v2));
						//*tmp_sched_v2 = NULL;
						SCHED_DBG("invalid enable flag or rule disabled. rule=%s", word);
						goto END;
					}
					break;
				case SCHED_V2_D_IDX_START_HOUR:
					(*tmp_sched_v2)->value_d.start_hour = strtonl(&word[i], (SCHED_V2_D_IDX_START_MINUTE - SCHED_V2_D_IDX_START_HOUR), 10);
					if ((*tmp_sched_v2)->value_d.start_hour < 0 || (*tmp_sched_v2)->value_d.start_hour > 24) {
						SCHED_DBG("invalid start_hour. rule=%s", word);
						goto END;
					}
					(*tmp_sched_v2)->value_w.rule_to_number = strtonul(&word[i], (SCHED_V2_D_IDX_END_HOUR - SCHED_V2_D_IDX_START_HOUR), 10);
					break;
				case SCHED_V2_D_IDX_START_MINUTE:
					(*tmp_sched_v2)->value_d.start_minute = strtonl(&word[i], (SCHED_V2_D_IDX_END_HOUR - SCHED_V2_D_IDX_START_MINUTE), 10);
					if ((*tmp_sched_v2)->value_d.start_minute < 0 || (*tmp_sched_v2)->value_d.start_minute > 59) {
						SCHED_DBG("invalid start_minute. rule=%s", word);
						goto END;
					}
					break;
				case SCHED_V2_D_IDX_END_HOUR:
					(*tmp_sched_v2)->value_d.end_hour = strtonl(&word[i], (SCHED_V2_D_IDX_END_MINUTE - SCHED_V2_D_IDX_END_HOUR), 10);
					if ((*tmp_sched_v2)->value_d.end_hour < 0 || (*tmp_sched_v2)->value_d.end_hour > 24) {
						SCHED_DBG("invalid end_hour. rule=%s", word);
						goto END;
					}
					break;
				case SCHED_V2_D_IDX_END_MINUTE:
					(*tmp_sched_v2)->value_d.end_minute = strtonl(&word[i], (SCHED_V2_D_IDX_NULL - SCHED_V2_D_IDX_END_MINUTE), 10);
					if ((*tmp_sched_v2)->value_d.end_minute < 0 || (*tmp_sched_v2)->value_d.end_minute > 59) {
						SCHED_DBG("invalid end_minute rule=%s", word);
						goto END;
					}
					break;
			}
		} else if ((*tmp_sched_v2)->type == SCHED_V2_TYPE_WEEK || (*tmp_sched_v2)->type == SCHED_V2_TYPE_WEEK_ONLINE) {
			switch(i) {
				case SCHED_V2_W_IDX_ENABLE:
					(*tmp_sched_v2)->value_w.enable = strtonl(&word[i], (SCHED_V2_W_IDX_DAY_OF_WEEK - SCHED_V2_W_IDX_ENABLE), 10);
					if (skip_disabled && (*tmp_sched_v2)->value_w.enable <= 0) {
						//free((*tmp_sched_v2));
						//*tmp_sched_v2 = NULL;
						SCHED_DBG("invalid enable flag or rule disabled. rule=%s", word);
						goto END;
					}
					break;
				case SCHED_V2_W_IDX_DAY_OF_WEEK:
					(*tmp_sched_v2)->value_w.day_of_week = strtonl(&word[i], (SCHED_V2_W_IDX_START_HOUR - SCHED_V2_W_IDX_DAY_OF_WEEK), 16);
					if ((*tmp_sched_v2)->value_w.day_of_week <= 0 || (*tmp_sched_v2)->value_w.day_of_week > 0x7F) {
						SCHED_DBG("invalid day_of_week. rule=%s", word);
						goto END;
					}
					(*tmp_sched_v2)->value_w.rule_to_number = strtonul(&word[i], (SCHED_V2_W_IDX_END_HOUR - SCHED_V2_W_IDX_DAY_OF_WEEK), 10);
					break;
				case SCHED_V2_W_IDX_START_HOUR:
					(*tmp_sched_v2)->value_w.start_hour = strtonl(&word[i], (SCHED_V2_W_IDX_START_MINUTE - SCHED_V2_W_IDX_START_HOUR), 10);
					if ((*tmp_sched_v2)->value_w.start_hour < 0 || (*tmp_sched_v2)->value_w.start_hour > 24) {
						SCHED_DBG("invalid start_hour. rule=%s", word);
						goto END;
					}
					break;
				case SCHED_V2_W_IDX_START_MINUTE:
					(*tmp_sched_v2)->value_w.start_minute = strtonl(&word[i], (SCHED_V2_W_IDX_END_HOUR - SCHED_V2_W_IDX_START_MINUTE), 10);
					if ((*tmp_sched_v2)->value_w.start_minute < 0 || (*tmp_sched_v2)->value_w.start_minute > 59) {
						SCHED_DBG("invalid start_minute. rule=%s", word);
						goto END;
					}
					break;
				case SCHED_V2_W_IDX_END_HOUR:
					(*tmp_sched_v2)->value_w.end_hour = strtonl(&word[i], (SCHED_V2_W_IDX_END_MINUTE - SCHED_V2_W_IDX_END_HOUR), 10);
					if ((*tmp_sched_v2)->value_w.end_hour < 0 || (*tmp_sched_v2)->value_w.end_hour > 24) {
						SCHED_DBG("invalid end_hour. rule=%s", word);
						goto END;
					}
					break;
				case SCHED_V2_W_IDX_END_MINUTE:
					(*tmp_sched_v2)->value_w.end_minute = strtonl(&word[i], (SCHED_V2_W_IDX_NULL - SCHED_V2_W_IDX_END_MINUTE), 10);
					if ((*tmp_sched_v2)->value_w.end_minute < 0 || (*tmp_sched_v2)->value_w.end_minute > 59) {
						SCHED_DBG("invalid end_minute. rule=%s", word);
						goto END;
					}
					break;
			}
		} else if ((*tmp_sched_v2)->type == SCHED_V2_TYPE_TIMESTAMP) {
			switch(i) {
				case SCHED_V2_T_IDX_S_FLAG:
					(*tmp_sched_v2)->value_t.s_flag = strtonl(&word[i], (SCHED_V2_T_IDX_S_TIMESTAMP - SCHED_V2_T_IDX_S_FLAG), 10);
					break;
				case SCHED_V2_T_IDX_S_TIMESTAMP:
					(*tmp_sched_v2)->value_t.s_ts = strtonul(&word[i], (SCHED_V2_T_IDX_S_Z_MARK - SCHED_V2_T_IDX_S_TIMESTAMP), 10);
					/*if ((*tmp_sched_v2)->value_w.day_of_week <= 0 || (*tmp_sched_v2)->value_w.day_of_week > 0x7F) {
						SCHED_DBG("invalid day_of_week. rule=%s", word);
						goto END;
					}*/
					//(*tmp_sched_v2)->value_w.rule_to_number = strtonul(&word[i], (SCHED_V2_W_IDX_END_HOUR - SCHED_V2_W_IDX_DAY_OF_WEEK), 10);
					break;
				case SCHED_V2_T_IDX_S_Z_MARK:
					(*tmp_sched_v2)->value_t.s_z_mark = word[i];
					if ((*tmp_sched_v2)->value_t.s_z_mark != 'Z') {
						SCHED_DBG("invalid z mark. rule=%s", word);
						goto END;
					}
					break;
				case SCHED_V2_T_IDX_E_TYPE:
					if (word[i] != 'T') {
						SCHED_DBG("second rule is not start with T. rule=%s", word);
						goto END;
					}
					break;
				case SCHED_V2_T_IDX_E_FLAG:
					(*tmp_sched_v2)->value_t.e_flag = strtonl(&word[i], (SCHED_V2_T_IDX_E_TIMESTAMP - SCHED_V2_T_IDX_E_FLAG), 10);
					break;
				case SCHED_V2_T_IDX_E_TIMESTAMP:
					(*tmp_sched_v2)->value_t.e_ts = strtonul(&word[i], (SCHED_V2_T_IDX_E_Z_MARK - SCHED_V2_T_IDX_E_TIMESTAMP), 10);
					/*if ((*tmp_sched_v2)->value_w.day_of_week <= 0 || (*tmp_sched_v2)->value_w.day_of_week > 0x7F) {
						SCHED_DBG("invalid day_of_week. rule=%s", word);
						goto END;
					}*/
					//(*tmp_sched_v2)->value_w.rule_to_number = strtonul(&word[i], (SCHED_V2_W_IDX_END_HOUR - SCHED_V2_W_IDX_DAY_OF_WEEK), 10);
					break;
				case SCHED_V2_T_IDX_E_Z_MARK:
					(*tmp_sched_v2)->value_t.e_z_mark = word[i];
					if ((*tmp_sched_v2)->value_t.e_z_mark != 'Z') {
						SCHED_DBG("invalid z mark. rule=%s", word);
						goto END;
					}
					break;
			}
		}
	}
	ret = 0;

END:
	return ret;
}

int parse_str_v2_to_sched_v2_list(const char *str_sched_v2, sched_v2_t **sched_v2_list, int merge_same_period, int skip_disabled) {
	int ret = -1;
	char tmp_str_sched[MAX_NVRAM_SCHED_LEN];
	char word[MAX_NVRAM_SCHED_LEN], *next_word;
	sched_v2_t **tmp_sched_v2, *prev_v2 = NULL;
	int rules_cnt = 0;

	if (!str_sched_v2 || !strlen(str_sched_v2))
		return ret;

	snprintf(tmp_str_sched, sizeof(tmp_str_sched), "%s", str_sched_v2);
	SCHED_DBG("tmp_str_sched:%s", tmp_str_sched);

	*sched_v2_list  = NULL;
	tmp_sched_v2 = sched_v2_list;

	foreach_60(word, tmp_str_sched, next_word) {
		if (++rules_cnt > MAX_SCHED_RULES) {
			SCHED_DBG("The number of rules reach the limit %d.", MAX_SCHED_RULES);
			break;
		}

		*tmp_sched_v2 = (sched_v2_t *)malloc(sizeof(sched_v2_t));
		//SCHED_DBG("%p", *tmp_sched_v2);
		if(*tmp_sched_v2 == NULL) {
			ret = -2;
			_dprintf("No memory!!(parse_str_v2_to_sched_v2_list)\n");
			return ret;
		}

		memset(*tmp_sched_v2, 0, sizeof(sched_v2_t));
		(*tmp_sched_v2)->prev = prev_v2;
		//SCHED_DBG("%p", *tmp_sched_v2);

		if (parse_sched_v2_rule(word, tmp_sched_v2, skip_disabled) < 0) { // If parse failed or need to skip, free the tmp_sched_v2.
			free((*tmp_sched_v2));
			*tmp_sched_v2 = NULL;
			SCHED_DBG("Skip the rule. rule=%s", word);
			continue;
		}

		/*if ((*tmp_sched_v2)->type == SCHED_V2_TYPE_TIMESTAMP) {

		} else if ((*tmp_sched_v2)->type == SCHED_V2_TYPE_DAY) {
			SCHED_DBG("word:%s, type:%d, enable:%d, start_hour:%d, start_minute:%d, end_hour:%d, end_minute:%d", 
				word, (*tmp_sched_v2)->type, (*tmp_sched_v2)->value_d.enable, 
				(*tmp_sched_v2)->value_d.start_hour, (*tmp_sched_v2)->value_d.start_minute, 
				(*tmp_sched_v2)->value_d.end_hour, (*tmp_sched_v2)->value_d.end_minute);
		} else if ((*tmp_sched_v2)->type == SCHED_V2_TYPE_WEEK || (*tmp_sched_v2)->type == SCHED_V2_TYPE_WEEK_ONLINE) {
			SCHED_DBG("word:%s, type:%d, enable:%d, day_of_week:%d, start_hour:%d, start_minute:%d, end_hour:%d, end_minute:%d", 
				word, (*tmp_sched_v2)->type, (*tmp_sched_v2)->value_w.enable, (*tmp_sched_v2)->value_w.day_of_week, 
				(*tmp_sched_v2)->value_w.start_hour, (*tmp_sched_v2)->value_w.start_minute, 
				(*tmp_sched_v2)->value_w.end_hour, (*tmp_sched_v2)->value_w.end_minute);
		}*/

		prev_v2 = *tmp_sched_v2;
		while(*tmp_sched_v2 != NULL)
			tmp_sched_v2 = &((*tmp_sched_v2)->next);
	}

	if (merge_same_period)
		merge_sched_v2_same_w_period(sched_v2_list);
	else
		expand_sched_v2_same_w_period(sched_v2_list);

	sort_sched_v2_list(sched_v2_list);
	ret = 0;
	return ret;
}

static char *convert_v1_str_to_v2(sched_v1_t *curr_v1, sched_v1_t *next_v1, char *buf, int *buf_size) {
	if (!curr_v1 || !next_v1 || !buf)
		return NULL;

	int day_idx;
	for (day_idx = curr_v1->end_day; day_idx <= next_v1->start_day; day_idx++) {
		if (!curr_v1->end_hour && next_v1->start_day == 7) // special case
			break;

		if (day_idx != curr_v1->end_day && day_idx != next_v1->start_day) { // period of whole day
			snprintf(buf+strlen(buf), *buf_size-strlen(buf), "W%d%02X%02d%02d%02d%02d<", SCHED_V2_ENABLE, 1 << day_idx, 0, 0, 24, 0);
		} else {
			if (curr_v1->end_day == next_v1->start_day) { // period in a single day
				if (curr_v1->end_hour || next_v1->start_hour)
					snprintf(buf+strlen(buf), *buf_size-strlen(buf), "W%d%02X%02d%02d%02d%02d<", SCHED_V2_ENABLE, 1 << day_idx, curr_v1->end_hour, 0, next_v1->start_hour, 0);
			} else {
				if (day_idx == curr_v1->end_day) { // period of the beginning day
					/*if (day_idx == -1)
						continue;*/
					snprintf(buf+strlen(buf), *buf_size-strlen(buf), "W%d%02X%02d%02d%02d%02d<", SCHED_V2_ENABLE, 1 << day_idx, curr_v1->end_hour, 0, 24, 0);
				} else if (day_idx == next_v1->start_day) { // period the ending day
					/*if (day_idx == 7)
						continue;*/
					if (next_v1->start_hour) {
						snprintf(buf+strlen(buf), *buf_size-strlen(buf), "W%d%02X%02d%02d%02d%02d<", SCHED_V2_ENABLE, 1 << day_idx, 0, 0, next_v1->start_hour, 0);
					}
				}
			}
		}
	}

	if (strlen(buf))
		return buf;

	return NULL;
}

#if 0
static char *convert_v1_str_to_v2_by_enable(sched_v1_t *sched_v1, char *buf, int *buf_size) {
	if (!sched_v1 || !buf)
		return NULL;

	int day_idx;
	int end_day = sched_v1->end_day;

	if (!sched_v1->next && !sched_v1->end_day && !sched_v1->end_hour)
		end_day = 7;

	for (day_idx = sched_v1->start_day; day_idx <= end_day; day_idx++) {
		//if (!sched_v1->start_hour && sched_v1->end_day == 7) // special case
		//	break;

		if (day_idx != sched_v1->start_day && day_idx != end_day) { // period of whole day
			snprintf(buf+strlen(buf), *buf_size-strlen(buf), "W1%02X%02d%02d%02d%02d<", 1 << day_idx, 0, 0, 24, 0);
		} else {
			if (sched_v1->start_day == end_day) { // period in a single day
				if (sched_v1->start_hour || sched_v1->end_hour)
					snprintf(buf+strlen(buf), *buf_size-strlen(buf), "W1%02X%02d%02d%02d%02d<", 1 << day_idx, sched_v1->start_hour, 0, sched_v1->end_hour, 0);
			} else {
				if (day_idx == sched_v1->start_day) { // period of the beginning day
					/*if (day_idx == -1)
						continue;*/
					snprintf(buf+strlen(buf), *buf_size-strlen(buf), "W1%02X%02d%02d%02d%02d<", 1 << day_idx, sched_v1->start_hour, 0, 24, 0);
				} else if (day_idx == end_day) { // period the ending day
					/*if (day_idx == 7)
						continue;*/
					if (sched_v1->end_hour) {
						snprintf(buf+strlen(buf), *buf_size-strlen(buf), "W1%02X%02d%02d%02d%02d<", 1 << day_idx, 0, 0, sched_v1->end_hour, 0);
					}
				}
			}
		}
	}

	if (strlen(buf))
		return buf;

	return NULL;
}
#endif

static char *convert_to_str_sched_v2(const char *str_sched, int merge_same_period, char *buf, int buf_size) {
	char tmp_sched[MAX_NVRAM_SCHED_LEN];
	sched_v1_t *sched_v1_list, *prev_v1, *curr_v1, *next_v1, fake_v1;
	sched_v2_t *sched_v2_list;
	char *tmp_buf = buf;
	int tmp_buf_size = buf_size;

	if (!buf || buf_size < MAX_NVRAM_SCHED_LEN)
		return NULL;

	snprintf(tmp_sched, sizeof(tmp_sched), "%s", str_sched ? str_sched: "");

	if (!str_sched || !strcmp(tmp_sched, "000000") || !strcmp(tmp_sched, "T<000000")) // all open
		snprintf(tmp_buf, buf_size, "%s", "");
	else if (!strcmp(tmp_sched, "") || !strcmp(tmp_sched, "<")) // all close
		snprintf(tmp_buf, buf_size, "%s", "W10100002400<W10200002400<W10400002400<W10800002400<W11000002400<W12000002400<W14000002400");
	else {
		if (!parse_str_v1_to_sched_v1_list(str_sched, &sched_v1_list)) {
			int sched_idx = 0;
			memset(buf, 0, buf_size);
			for(curr_v1 = sched_v1_list; curr_v1 != NULL; curr_v1 = curr_v1->next, sched_idx++) {
				prev_v1 = curr_v1->prev;
				next_v1 = curr_v1->next;
				//SCHED_DBG("prev_v1=%p, curr_v1=%p, next_v1=%p", prev_v1, curr_v1, next_v1);
				if (!prev_v1) { // the firste schedule
					memset(&fake_v1, 0, sizeof(fake_v1));
					fake_v1.end_day = 0;
					fake_v1.end_hour = 0;
					convert_v1_str_to_v2(&fake_v1, curr_v1, tmp_buf, &tmp_buf_size);
				}

				if (curr_v1 && next_v1) { // the normal schedule
					convert_v1_str_to_v2(curr_v1, next_v1, tmp_buf, &tmp_buf_size);
				}

				if (!next_v1) { // the last schedule
					memset(&fake_v1, 0, sizeof(fake_v1));
					fake_v1.start_day = 7;
					fake_v1.start_hour = 0;
					convert_v1_str_to_v2(curr_v1, &fake_v1, tmp_buf, &tmp_buf_size);
				}
			}

			free_sched_v1_list(&sched_v1_list);

			if (strlen(tmp_buf) && tmp_buf[strlen(tmp_buf)-1] == '<')
				tmp_buf[strlen(tmp_buf)-1] = '\0'; // strip the last char '<'
		} else {
			return NULL;
		}
	}

	//SCHED_DBG("%s", tmp_buf);
	// merge the schedule string with same period
	if (!parse_str_v2_to_sched_v2_list(tmp_buf, &sched_v2_list, merge_same_period, 1)) {
		if (merge_same_period)
			merge_sched_v2_same_w_period(&sched_v2_list);
		snprintf(buf, buf_size, "%s", ""); // clean buffer
		if (!gen_v2_str_from_sched_v2_list(sched_v2_list, buf, buf_size)) {
			SCHED_DBG("free_sched_v2_list");
			free_sched_v2_list(&sched_v2_list);
			return NULL;
		}

		SCHED_DBG("free_sched_v2_list");
		free_sched_v2_list(&sched_v2_list);
	}

	return buf;
}

#if 0
#define GET_DOW(n) (int)log2f(n)

inline void get_real_day_and_hour(int *day, int *hour, int is_start_time) {
	if (*hour == 24) {
		if (!is_start_time)
			*day = *day + 1;
		*hour = 0;
	}
}

char *convert_to_str_sched_v1(const char *str_sched, char *buf, int buf_size) {
	char tmp_sched[MAX_NVRAM_SCHED_LEN];
	sched_v2_t *sched_v2_list, *prev_v2, *curr_v2, *next_v2;
	char *tmp_buf = buf;
	int tmp_buf_size = buf_size;

	if (!buf || tmp_buf_size < MAX_NVRAM_SCHED_LEN)
		return NULL;

	snprintf(tmp_sched, sizeof(tmp_sched), "%s", str_sched ? str_sched: "");

	if (!str_sched || !strcmp(tmp_sched, "")) // all open
		snprintf(tmp_buf, tmp_buf_size, "%s", "000000");
	else if (!strcmp(tmp_sched, "W10100002400<W10200002400<W10400002400<W10800002400<W11000002400<W12000002400<W14000002400")) // all close
		snprintf(tmp_buf, tmp_buf_size, "%s", "");
	else {
		if (!parse_str_v2_to_sched_v2_list(str_sched, &sched_v2_list, 0, 1)) {
			int sched_idx = 0;
			int real_curr_day, real_curr_hour, real_next_day, real_next_hour;
			memset(buf, 0, tmp_buf_size);
			for(curr_v2 = sched_v2_list; curr_v2 != NULL; curr_v2 = curr_v2->next, sched_idx++) {
				prev_v2 = curr_v2->prev;
				next_v2 = curr_v2->next;

				//SCHED_DBG("prev_v1=%p, curr_v1=%p, next_v1=%p", prev_v1, curr_v1, next_v1);
				if (!prev_v2) { // the firste schedule
					if (curr_v2->value_w.start_hour) {
						real_curr_day = GET_DOW(curr_v2->value_w.day_of_week);
						real_curr_hour = curr_v2->value_w.start_hour;
						get_real_day_and_hour(&real_curr_day, &real_curr_hour, 0);
						snprintf(buf+strlen(buf), tmp_buf_size-strlen(buf), "%d%d%02d%02d<", 0, real_curr_day, 0, real_curr_hour);
					}
				}

				if (curr_v2 && next_v2) { // the normal schedule
					real_curr_day = GET_DOW(curr_v2->value_w.day_of_week);
					real_curr_hour = curr_v2->value_w.end_hour;
					get_real_day_and_hour(&real_curr_day, &real_curr_hour, 1);
					real_next_day = GET_DOW(next_v2->value_w.day_of_week);
					real_next_hour = next_v2->value_w.start_hour;
					get_real_day_and_hour(&real_next_day, &real_next_hour, 0);
					snprintf(buf+strlen(buf), tmp_buf_size-strlen(buf), "%d%d%02d%02d<", real_curr_day, real_next_day, real_curr_hour, real_next_hour);
				}

				if (!next_v2) { // the last schedule
					if (curr_v2->value_w.end_hour) {
						real_curr_day = GET_DOW(curr_v2->value_w.day_of_week);
						real_curr_hour = curr_v2->value_w.end_hour;
						get_real_day_and_hour(&real_curr_day, &real_curr_hour, 1);
						snprintf(buf+strlen(buf), tmp_buf_size-strlen(buf), "%d%d%02d%02d<", real_curr_day, 0, real_curr_hour, 0);
					}
				}
			}

			free_sched_v2_list(&sched_v2_list);

			if (strlen(tmp_buf) && tmp_buf[strlen(tmp_buf)-1] == '<')
				tmp_buf[strlen(tmp_buf)-1] = '\0'; // strip the last char '<'
		} else {
			return NULL;
		}
	}

	return buf;
}
#endif

#if defined(RTCONFIG_WL_SCHED_V3) || defined(RTCONFIG_PC_SCHED_V3)
#define DAY1_END_MIN  time_to_minute(24, 0)
#define DAY2_START_MIN  0
#endif

#if 1
static int is_online_mode(sched_v2_t *sched_v2_list)
{
	return (sched_v2_list && sched_v2_list->type == SCHED_V2_TYPE_WEEK_ONLINE) ? 1: 0;
}
#else
static int weekday_rotate(int n, unsigned int d)
{
	if (d > 7)
		d = 7;
	/* In n<<d, last d bits are 0. To put first 3 bits of n at 
		last, do bitwise or of n<<d with n >>(INT_BITS - d) */
	return (((n << d)|(n >> (7 - d))) & 0x7F);
}

static void convert_week_online_rule_to_offline(sched_v2_t *sched_v2_list)
{
	sched_v2_t *sched_v2, *new, *tail_sched_v2 = NULL;
	int all_online_dow = 0, all_offline_dow = 0;
	int first_sched_mode = -1;
	for (sched_v2 = sched_v2_list; sched_v2 != NULL; sched_v2 = sched_v2->next) {
		if (first_sched_mode == -1) {
			if (sched_v2->type == SCHED_V2_TYPE_WEEK_ONLINE)
				first_sched_mode = SCHED_V2_TYPE_WEEK_ONLINE;
			else if (sched_v2->type == SCHED_V2_TYPE_WEEK)
				first_sched_mode = SCHED_V2_TYPE_WEEK;
		}

		if (sched_v2->value_w.enable == 0)
			continue;

		SCHED_DBG("type=%d, enable=%d, dow=%d, sh=%d, sm=%d, eh=%d, em=%d", 
			sched_v2->type,
			sched_v2->value_w.enable,
			sched_v2->value_w.day_of_week,
			sched_v2->value_w.start_hour,
			sched_v2->value_w.start_minute,
			sched_v2->value_w.end_hour,
			sched_v2->value_w.end_minute);
		//get_event_day_limits(sched_v2, &(*follow_e_list)->start_day, &(*follow_e_list)->end_day);
		tail_sched_v2 = sched_v2;
		SCHED_DBG("prev=%p, tail_sched_v2=%p, next=%p", tail_sched_v2->prev, tail_sched_v2, tail_sched_v2->next);

		int s_min = (sched_v2->value_w.start_hour*60) + sched_v2->value_w.start_minute;
		int e_min = (sched_v2->value_w.end_hour*60) + sched_v2->value_w.end_minute;
		int start_hour = sched_v2->value_w.start_hour;
		int end_hour = sched_v2->value_w.end_hour;
		int start_minute = sched_v2->value_w.start_minute;
		int end_minute = sched_v2->value_w.end_minute;
		if (sched_v2->type == SCHED_V2_TYPE_WEEK_ONLINE) {  // convert to offline period

			all_online_dow |= sched_v2->value_w.day_of_week;
			sched_v2->type = SCHED_V2_TYPE_WEEK;

			if(s_min >= e_min) {  // over one day
				if (s_min == 0 && e_min == 0) {  // all day online, no offline period
					sched_v2->value_w.day_of_week = sched_v2->value_w.day_of_week;
					sched_v2->value_w.start_hour = 24;
					sched_v2->value_w.end_hour = 24;
					sched_v2->value_w.start_minute = 0;
					sched_v2->value_w.end_minute = 0;
				} else {
					sched_v2->value_w.day_of_week = sched_v2->value_w.day_of_week;
					sched_v2->value_w.start_hour = 0;
					sched_v2->value_w.end_hour = (s_min == 1440 && e_min == 1440) ? 24 : start_hour;
					sched_v2->value_w.start_minute = 0;
					sched_v2->value_w.end_minute = (s_min == 1440 && e_min == 1440) ? 0 : start_minute;
				}

				if (s_min != e_min) { // need two offline period
					new = (sched_v2_t *)malloc(sizeof(sched_v2_t));
					if(!new){
						_dprintf("No memory!!(follow_e_list)\n");
						continue;
					}
					memset(new, 0, sizeof(sched_v2_t));
					memcpy(&new->value_w, &sched_v2->value_w, sizeof(sched_v2_w_t));

					new->type = SCHED_V2_TYPE_WEEK;
					new->value_w.day_of_week = weekday_rotate(sched_v2->value_w.day_of_week, 1);
					all_online_dow |= new->value_w.day_of_week;
					new->value_w.start_hour = end_hour;
					new->value_w.end_hour = 24;
					new->value_w.start_minute = end_minute;
					new->value_w.end_minute = 0;

					new->prev = sched_v2;
					new->next = sched_v2->next;

					if (sched_v2->next)
						sched_v2->next->prev = new;
					sched_v2->next = new;

					sched_v2 = sched_v2->next; // skip new
					tail_sched_v2 = new;
					SCHED_DBG("add new over one day!!, end_hour=%d, end_miniute=%d", end_hour, end_minute);
					SCHED_DBG("prev=%p, tail_sched_v2=%p, next=%p", tail_sched_v2->prev, tail_sched_v2, tail_sched_v2->next);
				}
			} else { // not over one day
				if (s_min == 0 && e_min  == 1440) {
					sched_v2->value_w.day_of_week = sched_v2->value_w.day_of_week;
					sched_v2->value_w.start_hour = 24;
					sched_v2->value_w.end_hour = 24;
					sched_v2->value_w.start_minute = 0;
					sched_v2->value_w.end_minute = 0;
				} else if (s_min == 0 && e_min  != 1440) {
					sched_v2->value_w.day_of_week = sched_v2->value_w.day_of_week;
					sched_v2->value_w.start_hour = end_hour;
					sched_v2->value_w.end_hour = 24;
					sched_v2->value_w.start_minute = end_minute;
					sched_v2->value_w.end_minute = 0;
				} else if ((s_min != 0 && e_min  == 1440) || (s_min != 0 && e_min != 1440)) {
					sched_v2->value_w.day_of_week = sched_v2->value_w.day_of_week;
					sched_v2->value_w.start_hour = 0;
					sched_v2->value_w.end_hour = start_hour;
					sched_v2->value_w.start_minute = 0;
					sched_v2->value_w.end_minute = start_minute;

					if (s_min != 0 && e_min != 1440) {
						new = (sched_v2_t *)malloc(sizeof(sched_v2_t));
						if(!new){
							_dprintf("No memory!!(follow_e_list)\n");
							continue;
						}
						memset(new, 0, sizeof(sched_v2_t));
						memcpy(&new->value_w, &sched_v2->value_w, sizeof(sched_v2_w_t));

						new->type = SCHED_V2_TYPE_WEEK;
						new->value_w.day_of_week = sched_v2->value_w.day_of_week;
						new->value_w.start_hour = end_hour;
						new->value_w.end_hour = 24;
						new->value_w.start_minute = end_minute;
						new->value_w.end_minute = 0;

						new->prev = sched_v2;
						new->next = sched_v2->next;

						if (sched_v2->next)
							sched_v2->next->prev = new;
						sched_v2->next = new;

						sched_v2 = sched_v2->next; // skip new
						tail_sched_v2 = new;
						SCHED_DBG("add new not over one day!!, end_hour=%d, end_miniute=%d", end_hour, end_minute);
						SCHED_DBG("prev=%p, tail_sched_v2=%p, next=%p", tail_sched_v2->prev, tail_sched_v2, tail_sched_v2->next);
					}
				}
			}
		} else if (sched_v2->type == SCHED_V2_TYPE_WEEK) {  // offline mode. no need to convert
			all_offline_dow |= sched_v2->value_w.day_of_week;
			/*if(s_min >= e_min) {  // over one day
				if (s_min == 0 && e_min == 0) {  // all day online, no offline period
					sched_v2->value_w.day_of_week = sched_v2->value_w.day_of_week;
					sched_v2->value_w.start_hour = 24;
					sched_v2->value_w.end_hour = 24;
					sched_v2->value_w.start_minute = 0;
					sched_v2->value_w.end_minute = 0;
				} else {
					sched_v2->value_w.day_of_week = sched_v2->value_w.day_of_week;
					sched_v2->value_w.start_hour = (s_min == 1440 && e_min == 1440) ? 24 : start_hour;
					sched_v2->value_w.end_hour = 24;
					sched_v2->value_w.start_minute = (s_min == 1440 && e_min == 1440) ? 0 : start_minute;
					sched_v2->value_w.end_minute = 0;
				}

				if (s_min != e_min) { // need two offline period
					new = (sched_v2_t *)malloc(sizeof(sched_v2_t));
					if(!new){
						_dprintf("No memory!!(follow_e_list)\n");
						continue;
					}
					memset(new, 0, sizeof(sched_v2_t));
					memcpy(&new->value_w, &sched_v2->value_w, sizeof(sched_v2_w_t));

					new->type = SCHED_V2_TYPE_WEEK;
					new->value_w.day_of_week = weekday_rotate(sched_v2->value_w.day_of_week, 1);
					all_offline_dow |= new->value_w.day_of_week;
					new->value_w.start_hour = 0;
					new->value_w.end_hour = end_hour;
					new->value_w.start_minute = 0;
					new->value_w.end_minute = end_minute;

					new->prev = sched_v2;
					new->next = sched_v2->next;

					if (sched_v2->next)
						sched_v2->next->prev = new;
					sched_v2->next = new;

					sched_v2 = sched_v2->next; // skip new
					tail_sched_v2 = new;
					SCHED_DBG("add new over one day!!, end_hour=%d, end_miniute=%d", end_hour, end_minute);
					SCHED_DBG("prev=%p, tail_sched_v2=%p, next=%p", tail_sched_v2->prev, tail_sched_v2, tail_sched_v2->next);
				}
			}*/
		} else {
			SCHED_DBG("type does not support!!!");
			continue;
		}
	}

	//SCHED_DBG("tail_sched_v2=%p", tail_sched_v2);
	SCHED_DBG("first_sched_mode=%d, all_offline_dow=%d, all_online_dow=%d", first_sched_mode, all_offline_dow, all_online_dow);

	// gen a rule for the days not exists in online mode rules.
	if (tail_sched_v2 &&
		(((first_sched_mode == SCHED_V2_TYPE_WEEK_ONLINE && all_offline_dow == 0) || (first_sched_mode == SCHED_V2_TYPE_WEEK && all_offline_dow == 0 && all_online_dow != 0)) && 
		((~all_online_dow & 0x7F) != 0))) {
		new = (sched_v2_t *)malloc(sizeof(sched_v2_t));
		if(!new){
			_dprintf("No memory!!(follow_e_list)\n");
			return;
		}
		memset(new, 0, sizeof(sched_v2_t));
		memcpy(&new->value_w, &tail_sched_v2->value_w, sizeof(sched_v2_w_t));

		new->type = SCHED_V2_TYPE_WEEK;
		new->value_w.day_of_week = (~all_online_dow & 0x7F);
		new->value_w.start_hour = 0;
		new->value_w.end_hour = 24;
		new->value_w.start_minute = 0;
		new->value_w.end_minute = 0;
		SCHED_DBG("add new rules=%p", new);
		SCHED_DBG("prev=%p, tail_sched_v2=%p, next=%p", tail_sched_v2->prev, tail_sched_v2, tail_sched_v2->next);

		new->prev = tail_sched_v2;
		new->next = tail_sched_v2->next;

		if (tail_sched_v2->next)
			tail_sched_v2->next->prev = new;
		tail_sched_v2->next = new;
		SCHED_DBG("prev=%p, tail_sched_v2=%p, next=%p", tail_sched_v2->prev, tail_sched_v2, tail_sched_v2->next);
		SCHED_DBG("prev=%p, tail_sched_v2=%p, next=%p", tail_sched_v2->next->prev, tail_sched_v2->next, tail_sched_v2->next->next);
	}
}
#endif

/*
	For the rules which are started with 'W' or 'M'.
*/
int check_sched_v2_on_off(const char *sched_str) {
	int ret = 1;
	sched_v2_t *sched_v2_list;
	if (!parse_str_v2_to_sched_v2_list(sched_str, &sched_v2_list, 1, 0)) {
		time_t now;
		struct tm *ptm;
		sched_v2_t *sched_v2;
		int start_min, curr_min, end_min;
#ifndef RTCONFIG_AVOID_TZ_ENV
		setenv("TZ", nvram_safe_get("time_zone_x"), 1);
#endif
		now = time(NULL);
		ptm = localtime(&now);
		curr_min = time_to_minute(ptm->tm_hour, ptm->tm_min);
		//SCHED_DBG("now=%lu", now);
#if 1
		ret = is_online_mode(sched_v2_list) ? 0 : 1; // set default to off or on.
		SCHED_DBG("online_mode=%d", ret == 0 ? 1 : 0);
#else
		convert_week_online_rule_to_offline(sched_v2_list); // If any
#endif
		//SCHED_DBG("now=%ld", now);
		for (sched_v2 = sched_v2_list; sched_v2 != NULL; sched_v2 = sched_v2->next) {

			if (sched_v2->type == SCHED_V2_TYPE_DAY) { 
				start_min = time_to_minute(sched_v2->value_d.start_hour, sched_v2->value_d.start_minute);
				end_min = time_to_minute(sched_v2->value_d.end_hour, sched_v2->value_d.end_minute);
				SCHED_DBG("curr_min=%d, start_min=%d, end_min=%d", curr_min, start_min, end_min);
				if ((start_min <= curr_min) && (curr_min < end_min)) {
					ret = 0;
					break;
				}
			} else if ((sched_v2->type == SCHED_V2_TYPE_WEEK) || (sched_v2->type == SCHED_V2_TYPE_WEEK_ONLINE)) {
				SCHED_DBG("sched_v2=%p, type=%d, enable=%d, dow=%d, sh=%d, sm=%d, eh=%d, em=%d", 
					sched_v2,
					sched_v2->type,
					sched_v2->value_w.enable,
					sched_v2->value_w.day_of_week,
					sched_v2->value_w.start_hour,
					sched_v2->value_w.start_minute,
					sched_v2->value_w.end_hour,
					sched_v2->value_w.end_minute);

				if (sched_v2->value_w.enable == 0)
					continue;
				start_min = time_to_minute(sched_v2->value_w.start_hour, sched_v2->value_w.start_minute);
				end_min = time_to_minute(sched_v2->value_w.end_hour, sched_v2->value_w.end_minute);
				SCHED_DBG("curr_min=%d, start_min=%d, end_min=%d", curr_min, start_min, end_min);
#if defined(RTCONFIG_WL_SCHED_V3) || defined(RTCONFIG_PC_SCHED_V3)
				int over_wday = !ptm->tm_wday ? 6 : (ptm->tm_wday-1);
				if (start_min >= end_min) {  // over one day
					if ((((sched_v2->value_w.day_of_week & (1 << ptm->tm_wday)) > 0) &&	(start_min <= curr_min) && (curr_min < DAY1_END_MIN)) || 
						(((sched_v2->value_w.day_of_week & (1 << over_wday)) > 0) && (DAY2_START_MIN <= curr_min) && (curr_min < end_min))) {
						ret = (sched_v2->type == SCHED_V2_TYPE_WEEK) ? 0 : 1;
						SCHED_DBG("over one day matched. ret=%d", ret);
						break;
					}
				} else
#endif
				{
					if (((sched_v2->value_w.day_of_week & (1 << ptm->tm_wday)) > 0) &&
						(start_min <= curr_min) && (curr_min < end_min)) {
						ret = (sched_v2->type == SCHED_V2_TYPE_WEEK) ? 0 : 1;
						SCHED_DBG("over one day not matched. ret=%d", ret);
						break;
					}
				}
			}
		}
		free_sched_v2_list(&sched_v2_list);
		return ret;
	} else
		return ret;
}

/*
	For the rules which are started with 'T'.
	return value :
		 0 : in time period, actual meaning is off.
		 1 : in time period, actual meaning is on.
		-1 : the input rule is empty or invalid format, actual meaning is on.
		-2 : not in time period, actual meaning is off. (T11666281054Z,T01666381054Z)
		-3 : not in time period, actual meaning is on. (T01666281054Z,T11666381054Z)
*/
int check_expire_on_off(const char *sched_str) {
	int ret = -1;
	sched_v2_t *sched_v2_list;
	if (!parse_str_v2_to_sched_v2_list(sched_str, &sched_v2_list, 1, 0)) {
		time_t now;
		sched_v2_t *sched_v2;
#ifndef RTCONFIG_AVOID_TZ_ENV
		setenv("TZ", nvram_safe_get("time_zone_x"), 1);
#endif
		now = time(NULL);
		SCHED_DBG("now=%lu", now);

		for (sched_v2 = sched_v2_list; sched_v2 != NULL; sched_v2 = sched_v2->next) {
			if (sched_v2->type == SCHED_V2_TYPE_TIMESTAMP) {
				SCHED_DBG("s_flag=%d, now=%lu, s_ts=%lu", sched_v2->value_t.s_flag, now, sched_v2->value_t.s_ts);
				SCHED_DBG("e_flag=%d, now=%lu, e_ts=%lu", sched_v2->value_t.e_flag, now, sched_v2->value_t.e_ts);
				if ((sched_v2->value_t.s_flag == sched_v2->value_t.e_flag) ||
					((sched_v2->value_t.s_flag != 0) && (sched_v2->value_t.s_flag != 1)) || 
					((sched_v2->value_t.e_flag != 0) && (sched_v2->value_t.e_flag != 1)) || 
					(sched_v2->value_t.s_ts >= sched_v2->value_t.e_ts)) {
					ret = -1;
					SCHED_DBG("the format of the rule is invalid. ret=%d", ret);
				} else {
					if (sched_v2->value_t.s_ts <= now && sched_v2->value_t.e_ts > now) {
						ret = sched_v2->value_t.s_flag;
						SCHED_DBG("ts is in time preiod. ret=%d", ret);
					} else {
						ret = (sched_v2->value_t.s_flag == 1) ? -2 : -3;
						SCHED_DBG("ts is not in time preiod. ret=%d", ret);
					}
				}
			}
		}
		free_sched_v2_list(&sched_v2_list);
		SCHED_DBG("ret=%d", ret);
		return ret;
	} else {
		SCHED_DBG("ret=%d", ret);
		return ret;
	}
}

/*For wireless scheduler*/
void convert_wl_sched_v1_to_sched_v2() {
	char wl_ifnames[512];
	char word[256], *next, tmp[100];
	char prefix[]="wlXXXXXX_", str_sched_v2[MAX_NVRAM_SCHED_LEN];
	int unit = 0, changed = 0;
	snprintf(wl_ifnames, sizeof(wl_ifnames), "%s", nvram_safe_get("wl_ifnames"));
	foreach (word, wl_ifnames, next) {
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);

#if 0 //def RTCONFIG_AMAS
		if (nvram_match("re_mode", "1"))
		{
			/*transfer wl_sched NULL value to 000000 value, because
			of old version firmware with wrong default value*/
			if (!nvram_get(strlcat_r(prefix, "sched", tmp, sizeof(tmp))))
				nvram_set(strlcat_r(prefix, "sched", tmp, sizeof(tmp)), "000000");

			snprintf(str_sched_v2, sizeof(str_sched_v2), "%s", "");
			if (convert_to_str_sched_v2(nvram_safe_get(strlcat_r(prefix, "sched", tmp, sizeof(tmp))), 0, str_sched_v2, sizeof(str_sched_v2))) {
				if (strcmp(str_sched_v2, nvram_safe_get(strlcat_r(prefix, "sched_v2", tmp, sizeof(tmp))))) { // Changed
					nvram_set(strlcat_r(prefix, "sched_v2", tmp, sizeof(tmp)), str_sched_v2);
					//nvram_unset(strlcat_r(prefix, "sched", tmp, sizeof(tmp)));
					SCHED_DBG("str_sched_v2 : %s", str_sched_v2);
					changed = 1;
				}
			}
		} else
#endif
		{
			//if (nvram_get(strlcat_r(prefix, "sched", tmp, sizeof(tmp))) || !nvram_get(strlcat_r(prefix, "sched_v2", tmp, sizeof(tmp)))) {
			//fprintf((stderr), "[SCHED] %s=%s\n", strlcat_r(prefix, "sched_v2_converted", tmp, sizeof(tmp)), nvram_get(strlcat_r(prefix, "sched_v2_converted", tmp, sizeof(tmp))));
			if (!nvram_match(strlcat_r(prefix, "sched_v2_converted", tmp, sizeof(tmp)), "1")) {
				snprintf(str_sched_v2, sizeof(str_sched_v2), "%s", "");
				if (convert_to_str_sched_v2(nvram_safe_get(strlcat_r(prefix, "sched", tmp, sizeof(tmp))), 0, str_sched_v2, sizeof(str_sched_v2))) {
					//if (strcmp(str_sched_v2, nvram_safe_get(strcat_r(prefix, "sched_v2", tmp)))) { // Changed
						nvram_set(strlcat_r(prefix, "sched_v2", tmp, sizeof(tmp)), str_sched_v2);
						nvram_set(strlcat_r(prefix, "sched_v2_converted", tmp, sizeof(tmp)), "1");
#ifdef RTCONFIG_AMAS
						if (!nvram_match("re_mode", "1"))
#endif
						nvram_set(strlcat_r(prefix, "sched", tmp, sizeof(tmp)), "000000");
						SCHED_DBG("str_sched_v2 : %s", str_sched_v2);
						changed = 1;
					//}
				}
			}
		}
		//}
		unit++;
	}

#if 0 //def RTCONFIG_AMAS
		if (nvram_match("re_mode", "1"))
		{
			/*transfer wl_sched NULL value to 000000 value, because
			of old version firmware with wrong default value*/
			if (!nvram_get("wl_sched"))
				nvram_set("wl_sched", "000000");

			snprintf(str_sched_v2, sizeof(str_sched_v2), "%s", "");
			if (convert_to_str_sched_v2(nvram_get("wl_sched"), 0, str_sched_v2, sizeof(str_sched_v2))) {
				if (strcmp(str_sched_v2, nvram_safe_get("wl_sched_v2"))) { // Changed
					nvram_set("wl_sched_v2", str_sched_v2);
					//nvram_unset("wl_sched");
					SCHED_DBG("str_sched_v2 : %s", str_sched_v2);
					changed = 1;
				}
			}
		} else
#endif
		{
			//if (nvram_get("wl_sched") || !nvram_get("wl_sched_v2")) {
			//fprintf((stderr), "[SCHED] %s=%s\n", "wl_sched_v2_converted", nvram_get("wl_sched_v2_converted"));
			if (!nvram_match("wl_sched_v2_converted", "1")) {
				snprintf(str_sched_v2, sizeof(str_sched_v2), "%s", "");
				if (convert_to_str_sched_v2(nvram_get("wl_sched"), 0, str_sched_v2, sizeof(str_sched_v2))) {
					//if (strcmp(str_sched_v2, nvram_safe_get("wl_sched_v2"))) { // Changed
						nvram_set("wl_sched_v2", str_sched_v2);
						nvram_set("wl_sched_v2_converted", "1");
#ifdef RTCONFIG_AMAS
						if (!nvram_match("re_mode", "1"))
#endif
						nvram_set("wl_sched", "000000");
						SCHED_DBG("str_sched_v2 : %s", str_sched_v2);
						changed = 1;
					//}
				}
			}
		}

	if (changed)
		nvram_commit();
}
/*For wireless scheduler*/

/*For parental control scheduler*/
void convert_pc_sched_v1_to_sched_v2() {
	char str_sched_v1[MAX_NVRAM_SCHED_LEN];
	char str_sched_v2[MAX_NVRAM_SCHED_LEN];
	char tmp_str_sched_v2[MAX_NVRAM_SCHED_LEN];
	char word[MAX_NVRAM_SCHED_LEN], *next_word;
	int changed = 0, count;

	if (!nvram_match("MULTIFILTER_MACFILTER_DAYTIME_V2_CONVERTED", "1")) {
		snprintf(str_sched_v1, sizeof(str_sched_v1), "%s", nvram_safe_get("MULTIFILTER_MACFILTER_DAYTIME"));
		snprintf(str_sched_v2, sizeof(str_sched_v2), "%s", "");

		if (strlen(str_sched_v1) == 0 && 
			strlen(nvram_safe_get("MULTIFILTER_MAC")) == 0 && 
			strlen(nvram_safe_get("MULTIFILTER_ENABLE")) == 0) { //all rules are empty, we also write empty rule.
			snprintf(str_sched_v2, sizeof(str_sched_v2), "");
			changed = 1;
		} else {
		//if (!nvram_get("MULTIFILTER_MACFILTER_DAYTIME_V2")) {
			foreach_62_keep_empty_string(count, word, str_sched_v1, next_word) {
				snprintf(tmp_str_sched_v2, sizeof(tmp_str_sched_v2), "%s", "");
				// convert to v2 and merge same period to one rule
				if (convert_to_str_sched_v2(word, 1, tmp_str_sched_v2, sizeof(tmp_str_sched_v2))) {
					SCHED_DBG("tmp_str_sched_v2  : %s", tmp_str_sched_v2);
					snprintf(str_sched_v2 + strlen(str_sched_v2), sizeof(str_sched_v2) - strlen(str_sched_v2), "%s>", tmp_str_sched_v2);
					changed = 1;
				}
			}

			/*if (strlen(str_sched_v2) && str_sched_v2[strlen(str_sched_v2)-1] == '>')
				str_sched_v2[strlen(str_sched_v2)-1] = '\0'; // strip the last char '<'*/

			//SCHED_DBG("by_disable : %s", str_sched_v2);

			if (strlen(str_sched_v2) && str_sched_v2[strlen(str_sched_v2)-1] == '>')
				str_sched_v2[strlen(str_sched_v2)-1] = '\0'; // strip the last char '<'
		}

			SCHED_DBG("final : %s", str_sched_v2);

			if (changed) {
				nvram_set("MULTIFILTER_MACFILTER_DAYTIME_V2", str_sched_v2);
				nvram_set("MULTIFILTER_MACFILTER_DAYTIME_V2_CONVERTED", "1");
				nvram_commit();
			}
		//}
	}
}
/*For parental control scheduler*/