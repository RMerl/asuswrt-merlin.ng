%pure-parser
%parse-param {void *_data}
%parse-param {void *scanner}
%lex-param {void* scanner}

%{

#define YYDEBUG 1

#include <linux/compiler.h>
#include <linux/list.h>
#include <linux/types.h>
#include "util.h"
#include "parse-events.h"
#include "parse-events-bison.h"

extern int parse_events_lex (YYSTYPE* lvalp, void* scanner);

#define ABORT_ON(val) \
do { \
	if (val) \
		YYABORT; \
} while (0)

#define ALLOC_LIST(list) \
do { \
	list = malloc(sizeof(*list)); \
	ABORT_ON(!list);              \
	INIT_LIST_HEAD(list);         \
} while (0)

static inc_group_count(struct list_head *list,
		       struct parse_events_evlist *data)
{
	/* Count groups only have more than 1 members */
	if (!list_is_last(list->next, list))
		data->nr_groups++;
}

%}

%token PE_START_EVENTS PE_START_TERMS
%token PE_VALUE PE_VALUE_SYM_HW PE_VALUE_SYM_SW PE_RAW PE_TERM
%token PE_EVENT_NAME
%token PE_NAME
%token PE_MODIFIER_EVENT PE_MODIFIER_BP
%token PE_NAME_CACHE_TYPE PE_NAME_CACHE_OP_RESULT
%token PE_PREFIX_MEM PE_PREFIX_RAW PE_PREFIX_GROUP
%token PE_ERROR
%token PE_PMU_EVENT_PRE PE_PMU_EVENT_SUF PE_KERNEL_PMU_EVENT
%type <num> PE_VALUE
%type <num> PE_VALUE_SYM_HW
%type <num> PE_VALUE_SYM_SW
%type <num> PE_RAW
%type <num> PE_TERM
%type <str> PE_NAME
%type <str> PE_NAME_CACHE_TYPE
%type <str> PE_NAME_CACHE_OP_RESULT
%type <str> PE_MODIFIER_EVENT
%type <str> PE_MODIFIER_BP
%type <str> PE_EVENT_NAME
%type <str> PE_PMU_EVENT_PRE PE_PMU_EVENT_SUF PE_KERNEL_PMU_EVENT
%type <num> value_sym
%type <head> event_config
%type <term> event_term
%type <head> event_pmu
%type <head> event_legacy_symbol
%type <head> event_legacy_cache
%type <head> event_legacy_mem
%type <head> event_legacy_tracepoint
%type <head> event_legacy_numeric
%type <head> event_legacy_raw
%type <head> event_def
%type <head> event_mod
%type <head> event_name
%type <head> event
%type <head> events
%type <head> group_def
%type <head> group
%type <head> groups

%union
{
	char *str;
	u64 num;
	struct list_head *head;
	struct parse_events_term *term;
}
%%

start:
PE_START_EVENTS start_events
|
PE_START_TERMS  start_terms

start_events: groups
{
	struct parse_events_evlist *data = _data;

	parse_events_update_lists($1, &data->list);
}

groups:
groups ',' group
{
	struct list_head *list  = $1;
	struct list_head *group = $3;

	parse_events_update_lists(group, list);
	$$ = list;
}
|
groups ',' event
{
	struct list_head *list  = $1;
	struct list_head *event = $3;

	parse_events_update_lists(event, list);
	$$ = list;
}
|
group
|
event

group:
group_def ':' PE_MODIFIER_EVENT
{
	struct list_head *list = $1;

	ABORT_ON(parse_events__modifier_group(list, $3));
	$$ = list;
}
|
group_def

group_def:
PE_NAME '{' events '}'
{
	struct list_head *list = $3;

	inc_group_count(list, _data);
	parse_events__set_leader($1, list);
	$$ = list;
}
|
'{' events '}'
{
	struct list_head *list = $2;

	inc_group_count(list, _data);
	parse_events__set_leader(NULL, list);
	$$ = list;
}

events:
events ',' event
{
	struct list_head *event = $3;
	struct list_head *list  = $1;

	parse_events_update_lists(event, list);
	$$ = list;
}
|
event

event: event_mod

event_mod:
event_name PE_MODIFIER_EVENT
{
	struct list_head *list = $1;

	/*
	 * Apply modifier on all events added by single event definition
	 * (there could be more events added for multiple tracepoint
	 * definitions via '*?'.
	 */
	ABORT_ON(parse_events__modifier_event(list, $2, false));
	$$ = list;
}
|
event_name

event_name:
PE_EVENT_NAME event_def
{
	ABORT_ON(parse_events_name($2, $1));
	free($1);
	$$ = $2;
}
|
event_def

event_def: event_pmu |
	   event_legacy_symbol |
	   event_legacy_cache sep_dc |
	   event_legacy_mem |
	   event_legacy_tracepoint sep_dc |
	   event_legacy_numeric sep_dc |
	   event_legacy_raw sep_dc

event_pmu:
PE_NAME '/' event_config '/'
{
	struct parse_events_evlist *data = _data;
	struct list_head *list;

	ALLOC_LIST(list);
	ABORT_ON(parse_events_add_pmu(list, &data->idx, $1, $3));
	parse_events__free_terms($3);
	$$ = list;
}
|
PE_NAME '/' '/'
{
	struct parse_events_evlist *data = _data;
	struct list_head *list;

	ALLOC_LIST(list);
	ABORT_ON(parse_events_add_pmu(list, &data->idx, $1, NULL));
	$$ = list;
}
|
PE_KERNEL_PMU_EVENT sep_dc
{
	struct parse_events_evlist *data = _data;
	struct list_head *head;
	struct parse_events_term *term;
	struct list_head *list;

	ALLOC_LIST(head);
	ABORT_ON(parse_events_term__num(&term, PARSE_EVENTS__TERM_TYPE_USER,
					$1, 1));
	list_add_tail(&term->list, head);

	ALLOC_LIST(list);
	ABORT_ON(parse_events_add_pmu(list, &data->idx, "cpu", head));
	parse_events__free_terms(head);
	$$ = list;
}
|
PE_PMU_EVENT_PRE '-' PE_PMU_EVENT_SUF sep_dc
{
	struct parse_events_evlist *data = _data;
	struct list_head *head;
	struct parse_events_term *term;
	struct list_head *list;
	char pmu_name[128];
	snprintf(&pmu_name, 128, "%s-%s", $1, $3);

	ALLOC_LIST(head);
	ABORT_ON(parse_events_term__num(&term, PARSE_EVENTS__TERM_TYPE_USER,
					&pmu_name, 1));
	list_add_tail(&term->list, head);

	ALLOC_LIST(list);
	ABORT_ON(parse_events_add_pmu(list, &data->idx, "cpu", head));
	parse_events__free_terms(head);
	$$ = list;
}

value_sym:
PE_VALUE_SYM_HW
|
PE_VALUE_SYM_SW

event_legacy_symbol:
value_sym '/' event_config '/'
{
	struct parse_events_evlist *data = _data;
	struct list_head *list;
	int type = $1 >> 16;
	int config = $1 & 255;

	ALLOC_LIST(list);
	ABORT_ON(parse_events_add_numeric(list, &data->idx,
					  type, config, $3));
	parse_events__free_terms($3);
	$$ = list;
}
|
value_sym sep_slash_dc
{
	struct parse_events_evlist *data = _data;
	struct list_head *list;
	int type = $1 >> 16;
	int config = $1 & 255;

	ALLOC_LIST(list);
	ABORT_ON(parse_events_add_numeric(list, &data->idx,
					  type, config, NULL));
	$$ = list;
}

event_legacy_cache:
PE_NAME_CACHE_TYPE '-' PE_NAME_CACHE_OP_RESULT '-' PE_NAME_CACHE_OP_RESULT
{
	struct parse_events_evlist *data = _data;
	struct list_head *list;

	ALLOC_LIST(list);
	ABORT_ON(parse_events_add_cache(list, &data->idx, $1, $3, $5));
	$$ = list;
}
|
PE_NAME_CACHE_TYPE '-' PE_NAME_CACHE_OP_RESULT
{
	struct parse_events_evlist *data = _data;
	struct list_head *list;

	ALLOC_LIST(list);
	ABORT_ON(parse_events_add_cache(list, &data->idx, $1, $3, NULL));
	$$ = list;
}
|
PE_NAME_CACHE_TYPE
{
	struct parse_events_evlist *data = _data;
	struct list_head *list;

	ALLOC_LIST(list);
	ABORT_ON(parse_events_add_cache(list, &data->idx, $1, NULL, NULL));
	$$ = list;
}

event_legacy_mem:
PE_PREFIX_MEM PE_VALUE '/' PE_VALUE ':' PE_MODIFIER_BP sep_dc
{
	struct parse_events_evlist *data = _data;
	struct list_head *list;

	ALLOC_LIST(list);
	ABORT_ON(parse_events_add_breakpoint(list, &data->idx,
					     (void *) $2, $6, $4));
	$$ = list;
}
|
PE_PREFIX_MEM PE_VALUE '/' PE_VALUE sep_dc
{
	struct parse_events_evlist *data = _data;
	struct list_head *list;

	ALLOC_LIST(list);
	ABORT_ON(parse_events_add_breakpoint(list, &data->idx,
					     (void *) $2, NULL, $4));
	$$ = list;
}
|
PE_PREFIX_MEM PE_VALUE ':' PE_MODIFIER_BP sep_dc
{
	struct parse_events_evlist *data = _data;
	struct list_head *list;

	ALLOC_LIST(list);
	ABORT_ON(parse_events_add_breakpoint(list, &data->idx,
					     (void *) $2, $4, 0));
	$$ = list;
}
|
PE_PREFIX_MEM PE_VALUE sep_dc
{
	struct parse_events_evlist *data = _data;
	struct list_head *list;

	ALLOC_LIST(list);
	ABORT_ON(parse_events_add_breakpoint(list, &data->idx,
					     (void *) $2, NULL, 0));
	$$ = list;
}

event_legacy_tracepoint:
PE_NAME '-' PE_NAME ':' PE_NAME
{
	struct parse_events_evlist *data = _data;
	struct list_head *list;
	char sys_name[128];
	snprintf(&sys_name, 128, "%s-%s", $1, $3);

	ALLOC_LIST(list);
	ABORT_ON(parse_events_add_tracepoint(list, &data->idx, &sys_name, $5));
	$$ = list;
}
|
PE_NAME ':' PE_NAME
{
	struct parse_events_evlist *data = _data;
	struct list_head *list;

	ALLOC_LIST(list);
	ABORT_ON(parse_events_add_tracepoint(list, &data->idx, $1, $3));
	$$ = list;
}

event_legacy_numeric:
PE_VALUE ':' PE_VALUE
{
	struct parse_events_evlist *data = _data;
	struct list_head *list;

	ALLOC_LIST(list);
	ABORT_ON(parse_events_add_numeric(list, &data->idx, (u32)$1, $3, NULL));
	$$ = list;
}

event_legacy_raw:
PE_RAW
{
	struct parse_events_evlist *data = _data;
	struct list_head *list;

	ALLOC_LIST(list);
	ABORT_ON(parse_events_add_numeric(list, &data->idx,
					  PERF_TYPE_RAW, $1, NULL));
	$$ = list;
}

start_terms: event_config
{
	struct parse_events_terms *data = _data;
	data->terms = $1;
}

event_config:
event_config ',' event_term
{
	struct list_head *head = $1;
	struct parse_events_term *term = $3;

	ABORT_ON(!head);
	list_add_tail(&term->list, head);
	$$ = $1;
}
|
event_term
{
	struct list_head *head = malloc(sizeof(*head));
	struct parse_events_term *term = $1;

	ABORT_ON(!head);
	INIT_LIST_HEAD(head);
	list_add_tail(&term->list, head);
	$$ = head;
}

event_term:
PE_NAME '=' PE_NAME
{
	struct parse_events_term *term;

	ABORT_ON(parse_events_term__str(&term, PARSE_EVENTS__TERM_TYPE_USER,
					$1, $3));
	$$ = term;
}
|
PE_NAME '=' PE_VALUE
{
	struct parse_events_term *term;

	ABORT_ON(parse_events_term__num(&term, PARSE_EVENTS__TERM_TYPE_USER,
					$1, $3));
	$$ = term;
}
|
PE_NAME '=' PE_VALUE_SYM_HW
{
	struct parse_events_term *term;
	int config = $3 & 255;

	ABORT_ON(parse_events_term__sym_hw(&term, $1, config));
	$$ = term;
}
|
PE_NAME
{
	struct parse_events_term *term;

	ABORT_ON(parse_events_term__num(&term, PARSE_EVENTS__TERM_TYPE_USER,
					$1, 1));
	$$ = term;
}
|
PE_VALUE_SYM_HW
{
	struct parse_events_term *term;
	int config = $1 & 255;

	ABORT_ON(parse_events_term__sym_hw(&term, NULL, config));
	$$ = term;
}
|
PE_TERM '=' PE_NAME
{
	struct parse_events_term *term;

	ABORT_ON(parse_events_term__str(&term, (int)$1, NULL, $3));
	$$ = term;
}
|
PE_TERM '=' PE_VALUE
{
	struct parse_events_term *term;

	ABORT_ON(parse_events_term__num(&term, (int)$1, NULL, $3));
	$$ = term;
}
|
PE_TERM
{
	struct parse_events_term *term;

	ABORT_ON(parse_events_term__num(&term, (int)$1, NULL, 1));
	$$ = term;
}

sep_dc: ':' |

sep_slash_dc: '/' | ':' |

%%

void parse_events_error(void *data __maybe_unused, void *scanner __maybe_unused,
			char const *msg __maybe_unused)
{
}
