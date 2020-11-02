/*
 * Ecounters interface
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 * $Id:$
 */

#ifndef __ECOUNTERS_H_
#define __ECOUNTERS_H_

#if defined(WL_ENAB_RUNTIME_CHECK)
	#define ECOUNTERS_ENAB()   (ecounters_enabled())
#elif defined(ECOUNTERS_DISABLED)
	#define ECOUNTERS_ENAB()   (0)
#else
	#define ECOUNTERS_ENAB()   (ecounters_enabled())
#endif // endif

/* A top level module like WL_RTE can tell ecounters to
 * call its entry point instead of calling its clients directly.
 * If an entry point is not defined, or top level module is
 * not present in client, the registered function is called
 * directly.
 */
#define ECOUNTERS_TOP_LEVEL_SW_ENTITY_WL	0
#define ECOUNTERS_TOP_LEVEL_SW_ENTITY_BUS	1
#define ECOUNTERS_TOP_LEVEL_SW_ENTITY_MAX	2

typedef struct ecounters_info ecounters_info_t;

/* ecounters_get_stats: Called by ecounters to collect stats from a
 * registered source.
 * stats_type: statistics type advertised by this source.
 *	Since event logs are being used, create a tag there and thats the
 *	statistics type advertised.
 * context: Any context information required by source to collect
 *	stats
 */
typedef int (*ecounters_get_stats)(uint16 stats_type, void *context);
typedef int (*ecounters_entity_entry_point)(ecounters_get_stats fn, uint16 stats_type,
	void *context);

extern int ecounters_init(si_t *sih);

/* A top level software entity needs to register its entry point with ecounters.
 * ecounters calls this entry point so callbacks of interested sources is called with right
 * set of parameters. ecounters, by itself, does not call the registered callbacks.
 */
extern int ecounters_register_entity_entry_point(uint16 id, ecounters_entity_entry_point fn);

/* ecounters_register_source: Each source advertises its type supported by
 * registring itself with ecounters.
 * stats_type: Which statistics are advertised.
 * top_level_module: The top level entity to which this module belongs. The top level
 *	entity may have to expose an entry point if this function cannot be called
 *	directly.
 * context: Any context information that is required for this client to
 *	execute.
 * some_fn: Some client's function that needs to be passed to the entry point for
 *	execution.
 */
extern int ecounters_register_source(uint16 stats_type, uint16 top_level_module,
	ecounters_get_stats some_fn, void* context);

/* Called from wl_doiovar() to handle ecounters config */
extern int ecounters_config(void *params, uint32 p_len);

extern int ecounters_write(uint16 stats_type, uint8 *buf, uint16 buf_len);

extern int ecounters_trigger(ecounters_trigger_config_t *ecounters_data,
	uint16 reason, uint32 sub_reason_code);

/* return TRUE if all passed tags are valid, FALSE otherwise
 * tags are valid if they are < EVENT_LOG_TAG_MAX
 */
extern bool ecounters_check_tag_validity(uint16 ntypes, uint16* type);

extern ecounters_info_t *ecounters_enabled(void);
#endif /* __ECOUNTERS_H */
