/*
 * builtin-list.c
 *
 * Builtin list command: list all event types
 *
 * Copyright (C) 2009, Thomas Gleixner <tglx@linutronix.de>
 * Copyright (C) 2008-2009, Red Hat Inc, Ingo Molnar <mingo@redhat.com>
 * Copyright (C) 2011, Red Hat Inc, Arnaldo Carvalho de Melo <acme@redhat.com>
 */
#include "builtin.h"

#include "perf.h"

#include "util/parse-events.h"
#include "util/cache.h"
#include "util/pmu.h"
#include "util/parse-options.h"

int cmd_list(int argc, const char **argv, const char *prefix __maybe_unused)
{
	int i;
	bool raw_dump = false;
	struct option list_options[] = {
		OPT_BOOLEAN(0, "raw-dump", &raw_dump, "Dump raw events"),
		OPT_END()
	};
	const char * const list_usage[] = {
		"perf list [hw|sw|cache|tracepoint|pmu|event_glob]",
		NULL
	};

	set_option_flag(list_options, 0, "raw-dump", PARSE_OPT_HIDDEN);

	argc = parse_options(argc, argv, list_options, list_usage,
			     PARSE_OPT_STOP_AT_NON_OPTION);

	setup_pager();

	if (!raw_dump)
		printf("\nList of pre-defined events (to be used in -e):\n\n");

	if (argc == 0) {
		print_events(NULL, raw_dump);
		return 0;
	}

	for (i = 0; i < argc; ++i) {
		if (strcmp(argv[i], "tracepoint") == 0)
			print_tracepoint_events(NULL, NULL, raw_dump);
		else if (strcmp(argv[i], "hw") == 0 ||
			 strcmp(argv[i], "hardware") == 0)
			print_symbol_events(NULL, PERF_TYPE_HARDWARE,
					event_symbols_hw, PERF_COUNT_HW_MAX, raw_dump);
		else if (strcmp(argv[i], "sw") == 0 ||
			 strcmp(argv[i], "software") == 0)
			print_symbol_events(NULL, PERF_TYPE_SOFTWARE,
					event_symbols_sw, PERF_COUNT_SW_MAX, raw_dump);
		else if (strcmp(argv[i], "cache") == 0 ||
			 strcmp(argv[i], "hwcache") == 0)
			print_hwcache_events(NULL, raw_dump);
		else if (strcmp(argv[i], "pmu") == 0)
			print_pmu_events(NULL, raw_dump);
		else {
			char *sep = strchr(argv[i], ':'), *s;
			int sep_idx;

			if (sep == NULL) {
				print_events(argv[i], raw_dump);
				continue;
			}
			sep_idx = sep - argv[i];
			s = strdup(argv[i]);
			if (s == NULL)
				return -1;

			s[sep_idx] = '\0';
			print_tracepoint_events(s, s + sep_idx + 1, raw_dump);
			free(s);
		}
	}
	return 0;
}
