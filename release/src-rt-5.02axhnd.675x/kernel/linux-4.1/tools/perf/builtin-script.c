#include "builtin.h"

#include "perf.h"
#include "util/cache.h"
#include "util/debug.h"
#include "util/exec_cmd.h"
#include "util/header.h"
#include "util/parse-options.h"
#include "util/session.h"
#include "util/tool.h"
#include "util/symbol.h"
#include "util/thread.h"
#include "util/trace-event.h"
#include "util/util.h"
#include "util/evlist.h"
#include "util/evsel.h"
#include "util/sort.h"
#include "util/data.h"
#include <linux/bitmap.h>

static char const		*script_name;
static char const		*generate_script_lang;
static bool			debug_mode;
static u64			last_timestamp;
static u64			nr_unordered;
static bool			no_callchain;
static bool			latency_format;
static bool			system_wide;
static const char		*cpu_list;
static DECLARE_BITMAP(cpu_bitmap, MAX_NR_CPUS);

enum perf_output_field {
	PERF_OUTPUT_COMM            = 1U << 0,
	PERF_OUTPUT_TID             = 1U << 1,
	PERF_OUTPUT_PID             = 1U << 2,
	PERF_OUTPUT_TIME            = 1U << 3,
	PERF_OUTPUT_CPU             = 1U << 4,
	PERF_OUTPUT_EVNAME          = 1U << 5,
	PERF_OUTPUT_TRACE           = 1U << 6,
	PERF_OUTPUT_IP              = 1U << 7,
	PERF_OUTPUT_SYM             = 1U << 8,
	PERF_OUTPUT_DSO             = 1U << 9,
	PERF_OUTPUT_ADDR            = 1U << 10,
	PERF_OUTPUT_SYMOFFSET       = 1U << 11,
	PERF_OUTPUT_SRCLINE         = 1U << 12,
	PERF_OUTPUT_PERIOD          = 1U << 13,
};

struct output_option {
	const char *str;
	enum perf_output_field field;
} all_output_options[] = {
	{.str = "comm",  .field = PERF_OUTPUT_COMM},
	{.str = "tid",   .field = PERF_OUTPUT_TID},
	{.str = "pid",   .field = PERF_OUTPUT_PID},
	{.str = "time",  .field = PERF_OUTPUT_TIME},
	{.str = "cpu",   .field = PERF_OUTPUT_CPU},
	{.str = "event", .field = PERF_OUTPUT_EVNAME},
	{.str = "trace", .field = PERF_OUTPUT_TRACE},
	{.str = "ip",    .field = PERF_OUTPUT_IP},
	{.str = "sym",   .field = PERF_OUTPUT_SYM},
	{.str = "dso",   .field = PERF_OUTPUT_DSO},
	{.str = "addr",  .field = PERF_OUTPUT_ADDR},
	{.str = "symoff", .field = PERF_OUTPUT_SYMOFFSET},
	{.str = "srcline", .field = PERF_OUTPUT_SRCLINE},
	{.str = "period", .field = PERF_OUTPUT_PERIOD},
};

/* default set to maintain compatibility with current format */
static struct {
	bool user_set;
	bool wildcard_set;
	unsigned int print_ip_opts;
	u64 fields;
	u64 invalid_fields;
} output[PERF_TYPE_MAX] = {

	[PERF_TYPE_HARDWARE] = {
		.user_set = false,

		.fields = PERF_OUTPUT_COMM | PERF_OUTPUT_TID |
			      PERF_OUTPUT_CPU | PERF_OUTPUT_TIME |
			      PERF_OUTPUT_EVNAME | PERF_OUTPUT_IP |
			      PERF_OUTPUT_SYM | PERF_OUTPUT_DSO |
			      PERF_OUTPUT_PERIOD,

		.invalid_fields = PERF_OUTPUT_TRACE,
	},

	[PERF_TYPE_SOFTWARE] = {
		.user_set = false,

		.fields = PERF_OUTPUT_COMM | PERF_OUTPUT_TID |
			      PERF_OUTPUT_CPU | PERF_OUTPUT_TIME |
			      PERF_OUTPUT_EVNAME | PERF_OUTPUT_IP |
			      PERF_OUTPUT_SYM | PERF_OUTPUT_DSO |
			      PERF_OUTPUT_PERIOD,

		.invalid_fields = PERF_OUTPUT_TRACE,
	},

	[PERF_TYPE_TRACEPOINT] = {
		.user_set = false,

		.fields = PERF_OUTPUT_COMM | PERF_OUTPUT_TID |
				  PERF_OUTPUT_CPU | PERF_OUTPUT_TIME |
				  PERF_OUTPUT_EVNAME | PERF_OUTPUT_TRACE,
	},

	[PERF_TYPE_RAW] = {
		.user_set = false,

		.fields = PERF_OUTPUT_COMM | PERF_OUTPUT_TID |
			      PERF_OUTPUT_CPU | PERF_OUTPUT_TIME |
			      PERF_OUTPUT_EVNAME | PERF_OUTPUT_IP |
			      PERF_OUTPUT_SYM | PERF_OUTPUT_DSO |
			      PERF_OUTPUT_PERIOD,

		.invalid_fields = PERF_OUTPUT_TRACE,
	},
};

static bool output_set_by_user(void)
{
	int j;
	for (j = 0; j < PERF_TYPE_MAX; ++j) {
		if (output[j].user_set)
			return true;
	}
	return false;
}

static const char *output_field2str(enum perf_output_field field)
{
	int i, imax = ARRAY_SIZE(all_output_options);
	const char *str = "";

	for (i = 0; i < imax; ++i) {
		if (all_output_options[i].field == field) {
			str = all_output_options[i].str;
			break;
		}
	}
	return str;
}

#define PRINT_FIELD(x)  (output[attr->type].fields & PERF_OUTPUT_##x)

static int perf_evsel__check_stype(struct perf_evsel *evsel,
				   u64 sample_type, const char *sample_msg,
				   enum perf_output_field field)
{
	struct perf_event_attr *attr = &evsel->attr;
	int type = attr->type;
	const char *evname;

	if (attr->sample_type & sample_type)
		return 0;

	if (output[type].user_set) {
		evname = perf_evsel__name(evsel);
		pr_err("Samples for '%s' event do not have %s attribute set. "
		       "Cannot print '%s' field.\n",
		       evname, sample_msg, output_field2str(field));
		return -1;
	}

	/* user did not ask for it explicitly so remove from the default list */
	output[type].fields &= ~field;
	evname = perf_evsel__name(evsel);
	pr_debug("Samples for '%s' event do not have %s attribute set. "
		 "Skipping '%s' field.\n",
		 evname, sample_msg, output_field2str(field));

	return 0;
}

static int perf_evsel__check_attr(struct perf_evsel *evsel,
				  struct perf_session *session)
{
	struct perf_event_attr *attr = &evsel->attr;

	if (PRINT_FIELD(TRACE) &&
		!perf_session__has_traces(session, "record -R"))
		return -EINVAL;

	if (PRINT_FIELD(IP)) {
		if (perf_evsel__check_stype(evsel, PERF_SAMPLE_IP, "IP",
					    PERF_OUTPUT_IP))
			return -EINVAL;
	}

	if (PRINT_FIELD(ADDR) &&
		perf_evsel__check_stype(evsel, PERF_SAMPLE_ADDR, "ADDR",
					PERF_OUTPUT_ADDR))
		return -EINVAL;

	if (PRINT_FIELD(SYM) && !PRINT_FIELD(IP) && !PRINT_FIELD(ADDR)) {
		pr_err("Display of symbols requested but neither sample IP nor "
			   "sample address\nis selected. Hence, no addresses to convert "
		       "to symbols.\n");
		return -EINVAL;
	}
	if (PRINT_FIELD(SYMOFFSET) && !PRINT_FIELD(SYM)) {
		pr_err("Display of offsets requested but symbol is not"
		       "selected.\n");
		return -EINVAL;
	}
	if (PRINT_FIELD(DSO) && !PRINT_FIELD(IP) && !PRINT_FIELD(ADDR)) {
		pr_err("Display of DSO requested but neither sample IP nor "
			   "sample address\nis selected. Hence, no addresses to convert "
		       "to DSO.\n");
		return -EINVAL;
	}
	if (PRINT_FIELD(SRCLINE) && !PRINT_FIELD(IP)) {
		pr_err("Display of source line number requested but sample IP is not\n"
		       "selected. Hence, no address to lookup the source line number.\n");
		return -EINVAL;
	}

	if ((PRINT_FIELD(PID) || PRINT_FIELD(TID)) &&
		perf_evsel__check_stype(evsel, PERF_SAMPLE_TID, "TID",
					PERF_OUTPUT_TID|PERF_OUTPUT_PID))
		return -EINVAL;

	if (PRINT_FIELD(TIME) &&
		perf_evsel__check_stype(evsel, PERF_SAMPLE_TIME, "TIME",
					PERF_OUTPUT_TIME))
		return -EINVAL;

	if (PRINT_FIELD(CPU) &&
		perf_evsel__check_stype(evsel, PERF_SAMPLE_CPU, "CPU",
					PERF_OUTPUT_CPU))
		return -EINVAL;

	if (PRINT_FIELD(PERIOD) &&
		perf_evsel__check_stype(evsel, PERF_SAMPLE_PERIOD, "PERIOD",
					PERF_OUTPUT_PERIOD))
		return -EINVAL;

	return 0;
}

static void set_print_ip_opts(struct perf_event_attr *attr)
{
	unsigned int type = attr->type;

	output[type].print_ip_opts = 0;
	if (PRINT_FIELD(IP))
		output[type].print_ip_opts |= PRINT_IP_OPT_IP;

	if (PRINT_FIELD(SYM))
		output[type].print_ip_opts |= PRINT_IP_OPT_SYM;

	if (PRINT_FIELD(DSO))
		output[type].print_ip_opts |= PRINT_IP_OPT_DSO;

	if (PRINT_FIELD(SYMOFFSET))
		output[type].print_ip_opts |= PRINT_IP_OPT_SYMOFFSET;

	if (PRINT_FIELD(SRCLINE))
		output[type].print_ip_opts |= PRINT_IP_OPT_SRCLINE;
}

/*
 * verify all user requested events exist and the samples
 * have the expected data
 */
static int perf_session__check_output_opt(struct perf_session *session)
{
	int j;
	struct perf_evsel *evsel;

	for (j = 0; j < PERF_TYPE_MAX; ++j) {
		evsel = perf_session__find_first_evtype(session, j);

		/*
		 * even if fields is set to 0 (ie., show nothing) event must
		 * exist if user explicitly includes it on the command line
		 */
		if (!evsel && output[j].user_set && !output[j].wildcard_set) {
			pr_err("%s events do not exist. "
			       "Remove corresponding -f option to proceed.\n",
			       event_type(j));
			return -1;
		}

		if (evsel && output[j].fields &&
			perf_evsel__check_attr(evsel, session))
			return -1;

		if (evsel == NULL)
			continue;

		set_print_ip_opts(&evsel->attr);
	}

	if (!no_callchain) {
		bool use_callchain = false;

		evlist__for_each(session->evlist, evsel) {
			if (evsel->attr.sample_type & PERF_SAMPLE_CALLCHAIN) {
				use_callchain = true;
				break;
			}
		}
		if (!use_callchain)
			symbol_conf.use_callchain = false;
	}

	/*
	 * set default for tracepoints to print symbols only
	 * if callchains are present
	 */
	if (symbol_conf.use_callchain &&
	    !output[PERF_TYPE_TRACEPOINT].user_set) {
		struct perf_event_attr *attr;

		j = PERF_TYPE_TRACEPOINT;
		evsel = perf_session__find_first_evtype(session, j);
		if (evsel == NULL)
			goto out;

		attr = &evsel->attr;

		if (attr->sample_type & PERF_SAMPLE_CALLCHAIN) {
			output[j].fields |= PERF_OUTPUT_IP;
			output[j].fields |= PERF_OUTPUT_SYM;
			output[j].fields |= PERF_OUTPUT_DSO;
			set_print_ip_opts(attr);
		}
	}

out:
	return 0;
}

static void print_sample_start(struct perf_sample *sample,
			       struct thread *thread,
			       struct perf_evsel *evsel)
{
	struct perf_event_attr *attr = &evsel->attr;
	unsigned long secs;
	unsigned long usecs;
	unsigned long long nsecs;

	if (PRINT_FIELD(COMM)) {
		if (latency_format)
			printf("%8.8s ", thread__comm_str(thread));
		else if (PRINT_FIELD(IP) && symbol_conf.use_callchain)
			printf("%s ", thread__comm_str(thread));
		else
			printf("%16s ", thread__comm_str(thread));
	}

	if (PRINT_FIELD(PID) && PRINT_FIELD(TID))
		printf("%5d/%-5d ", sample->pid, sample->tid);
	else if (PRINT_FIELD(PID))
		printf("%5d ", sample->pid);
	else if (PRINT_FIELD(TID))
		printf("%5d ", sample->tid);

	if (PRINT_FIELD(CPU)) {
		if (latency_format)
			printf("%3d ", sample->cpu);
		else
			printf("[%03d] ", sample->cpu);
	}

	if (PRINT_FIELD(TIME)) {
		nsecs = sample->time;
		secs = nsecs / NSECS_PER_SEC;
		nsecs -= secs * NSECS_PER_SEC;
		usecs = nsecs / NSECS_PER_USEC;
		printf("%5lu.%06lu: ", secs, usecs);
	}
}

static void print_sample_addr(union perf_event *event,
			  struct perf_sample *sample,
			  struct thread *thread,
			  struct perf_event_attr *attr)
{
	struct addr_location al;

	printf("%16" PRIx64, sample->addr);

	if (!sample_addr_correlates_sym(attr))
		return;

	perf_event__preprocess_sample_addr(event, sample, thread, &al);

	if (PRINT_FIELD(SYM)) {
		printf(" ");
		if (PRINT_FIELD(SYMOFFSET))
			symbol__fprintf_symname_offs(al.sym, &al, stdout);
		else
			symbol__fprintf_symname(al.sym, stdout);
	}

	if (PRINT_FIELD(DSO)) {
		printf(" (");
		map__fprintf_dsoname(al.map, stdout);
		printf(")");
	}
}

static void print_sample_bts(union perf_event *event,
			     struct perf_sample *sample,
			     struct perf_evsel *evsel,
			     struct thread *thread,
			     struct addr_location *al)
{
	struct perf_event_attr *attr = &evsel->attr;
	bool print_srcline_last = false;

	/* print branch_from information */
	if (PRINT_FIELD(IP)) {
		unsigned int print_opts = output[attr->type].print_ip_opts;

		if (symbol_conf.use_callchain && sample->callchain) {
			printf("\n");
		} else {
			printf(" ");
			if (print_opts & PRINT_IP_OPT_SRCLINE) {
				print_srcline_last = true;
				print_opts &= ~PRINT_IP_OPT_SRCLINE;
			}
		}
		perf_evsel__print_ip(evsel, sample, al, print_opts,
				     PERF_MAX_STACK_DEPTH);
	}

	/* print branch_to information */
	if (PRINT_FIELD(ADDR) ||
	    ((evsel->attr.sample_type & PERF_SAMPLE_ADDR) &&
	     !output[attr->type].user_set)) {
		printf(" => ");
		print_sample_addr(event, sample, thread, attr);
	}

	if (print_srcline_last)
		map__fprintf_srcline(al->map, al->addr, "\n  ", stdout);

	printf("\n");
}

static void process_event(union perf_event *event, struct perf_sample *sample,
			  struct perf_evsel *evsel, struct addr_location *al)
{
	struct thread *thread = al->thread;
	struct perf_event_attr *attr = &evsel->attr;

	if (output[attr->type].fields == 0)
		return;

	print_sample_start(sample, thread, evsel);

	if (PRINT_FIELD(PERIOD))
		printf("%10" PRIu64 " ", sample->period);

	if (PRINT_FIELD(EVNAME)) {
		const char *evname = perf_evsel__name(evsel);
		printf("%s: ", evname ? evname : "[unknown]");
	}

	if (is_bts_event(attr)) {
		print_sample_bts(event, sample, evsel, thread, al);
		return;
	}

	if (PRINT_FIELD(TRACE))
		event_format__print(evsel->tp_format, sample->cpu,
				    sample->raw_data, sample->raw_size);
	if (PRINT_FIELD(ADDR))
		print_sample_addr(event, sample, thread, attr);

	if (PRINT_FIELD(IP)) {
		if (!symbol_conf.use_callchain)
			printf(" ");
		else
			printf("\n");

		perf_evsel__print_ip(evsel, sample, al,
				     output[attr->type].print_ip_opts,
				     PERF_MAX_STACK_DEPTH);
	}

	printf("\n");
}

static int default_start_script(const char *script __maybe_unused,
				int argc __maybe_unused,
				const char **argv __maybe_unused)
{
	return 0;
}

static int default_flush_script(void)
{
	return 0;
}

static int default_stop_script(void)
{
	return 0;
}

static int default_generate_script(struct pevent *pevent __maybe_unused,
				   const char *outfile __maybe_unused)
{
	return 0;
}

static struct scripting_ops default_scripting_ops = {
	.start_script		= default_start_script,
	.flush_script		= default_flush_script,
	.stop_script		= default_stop_script,
	.process_event		= process_event,
	.generate_script	= default_generate_script,
};

static struct scripting_ops	*scripting_ops;

static void setup_scripting(void)
{
	setup_perl_scripting();
	setup_python_scripting();

	scripting_ops = &default_scripting_ops;
}

static int flush_scripting(void)
{
	return scripting_ops->flush_script();
}

static int cleanup_scripting(void)
{
	pr_debug("\nperf script stopped\n");

	return scripting_ops->stop_script();
}

static int process_sample_event(struct perf_tool *tool __maybe_unused,
				union perf_event *event,
				struct perf_sample *sample,
				struct perf_evsel *evsel,
				struct machine *machine)
{
	struct addr_location al;

	if (debug_mode) {
		if (sample->time < last_timestamp) {
			pr_err("Samples misordered, previous: %" PRIu64
				" this: %" PRIu64 "\n", last_timestamp,
				sample->time);
			nr_unordered++;
		}
		last_timestamp = sample->time;
		return 0;
	}

	if (perf_event__preprocess_sample(event, machine, &al, sample) < 0) {
		pr_err("problem processing %d event, skipping it.\n",
		       event->header.type);
		return -1;
	}

	if (al.filtered)
		return 0;

	if (cpu_list && !test_bit(sample->cpu, cpu_bitmap))
		return 0;

	scripting_ops->process_event(event, sample, evsel, &al);

	return 0;
}

struct perf_script {
	struct perf_tool	tool;
	struct perf_session	*session;
	bool			show_task_events;
	bool			show_mmap_events;
};

static int process_attr(struct perf_tool *tool, union perf_event *event,
			struct perf_evlist **pevlist)
{
	struct perf_script *scr = container_of(tool, struct perf_script, tool);
	struct perf_evlist *evlist;
	struct perf_evsel *evsel, *pos;
	int err;

	err = perf_event__process_attr(tool, event, pevlist);
	if (err)
		return err;

	evlist = *pevlist;
	evsel = perf_evlist__last(*pevlist);

	if (evsel->attr.type >= PERF_TYPE_MAX)
		return 0;

	evlist__for_each(evlist, pos) {
		if (pos->attr.type == evsel->attr.type && pos != evsel)
			return 0;
	}

	set_print_ip_opts(&evsel->attr);

	return perf_evsel__check_attr(evsel, scr->session);
}

static int process_comm_event(struct perf_tool *tool,
			      union perf_event *event,
			      struct perf_sample *sample,
			      struct machine *machine)
{
	struct thread *thread;
	struct perf_script *script = container_of(tool, struct perf_script, tool);
	struct perf_session *session = script->session;
	struct perf_evsel *evsel = perf_evlist__first(session->evlist);
	int ret = -1;

	thread = machine__findnew_thread(machine, event->comm.pid, event->comm.tid);
	if (thread == NULL) {
		pr_debug("problem processing COMM event, skipping it.\n");
		return -1;
	}

	if (perf_event__process_comm(tool, event, sample, machine) < 0)
		goto out;

	if (!evsel->attr.sample_id_all) {
		sample->cpu = 0;
		sample->time = 0;
		sample->tid = event->comm.tid;
		sample->pid = event->comm.pid;
	}
	print_sample_start(sample, thread, evsel);
	perf_event__fprintf(event, stdout);
	ret = 0;

out:
	return ret;
}

static int process_fork_event(struct perf_tool *tool,
			      union perf_event *event,
			      struct perf_sample *sample,
			      struct machine *machine)
{
	struct thread *thread;
	struct perf_script *script = container_of(tool, struct perf_script, tool);
	struct perf_session *session = script->session;
	struct perf_evsel *evsel = perf_evlist__first(session->evlist);

	if (perf_event__process_fork(tool, event, sample, machine) < 0)
		return -1;

	thread = machine__findnew_thread(machine, event->fork.pid, event->fork.tid);
	if (thread == NULL) {
		pr_debug("problem processing FORK event, skipping it.\n");
		return -1;
	}

	if (!evsel->attr.sample_id_all) {
		sample->cpu = 0;
		sample->time = event->fork.time;
		sample->tid = event->fork.tid;
		sample->pid = event->fork.pid;
	}
	print_sample_start(sample, thread, evsel);
	perf_event__fprintf(event, stdout);

	return 0;
}
static int process_exit_event(struct perf_tool *tool,
			      union perf_event *event,
			      struct perf_sample *sample,
			      struct machine *machine)
{
	struct thread *thread;
	struct perf_script *script = container_of(tool, struct perf_script, tool);
	struct perf_session *session = script->session;
	struct perf_evsel *evsel = perf_evlist__first(session->evlist);

	thread = machine__findnew_thread(machine, event->fork.pid, event->fork.tid);
	if (thread == NULL) {
		pr_debug("problem processing EXIT event, skipping it.\n");
		return -1;
	}

	if (!evsel->attr.sample_id_all) {
		sample->cpu = 0;
		sample->time = 0;
		sample->tid = event->comm.tid;
		sample->pid = event->comm.pid;
	}
	print_sample_start(sample, thread, evsel);
	perf_event__fprintf(event, stdout);

	if (perf_event__process_exit(tool, event, sample, machine) < 0)
		return -1;

	return 0;
}

static int process_mmap_event(struct perf_tool *tool,
			      union perf_event *event,
			      struct perf_sample *sample,
			      struct machine *machine)
{
	struct thread *thread;
	struct perf_script *script = container_of(tool, struct perf_script, tool);
	struct perf_session *session = script->session;
	struct perf_evsel *evsel = perf_evlist__first(session->evlist);

	if (perf_event__process_mmap(tool, event, sample, machine) < 0)
		return -1;

	thread = machine__findnew_thread(machine, event->mmap.pid, event->mmap.tid);
	if (thread == NULL) {
		pr_debug("problem processing MMAP event, skipping it.\n");
		return -1;
	}

	if (!evsel->attr.sample_id_all) {
		sample->cpu = 0;
		sample->time = 0;
		sample->tid = event->mmap.tid;
		sample->pid = event->mmap.pid;
	}
	print_sample_start(sample, thread, evsel);
	perf_event__fprintf(event, stdout);

	return 0;
}

static int process_mmap2_event(struct perf_tool *tool,
			      union perf_event *event,
			      struct perf_sample *sample,
			      struct machine *machine)
{
	struct thread *thread;
	struct perf_script *script = container_of(tool, struct perf_script, tool);
	struct perf_session *session = script->session;
	struct perf_evsel *evsel = perf_evlist__first(session->evlist);

	if (perf_event__process_mmap2(tool, event, sample, machine) < 0)
		return -1;

	thread = machine__findnew_thread(machine, event->mmap2.pid, event->mmap2.tid);
	if (thread == NULL) {
		pr_debug("problem processing MMAP2 event, skipping it.\n");
		return -1;
	}

	if (!evsel->attr.sample_id_all) {
		sample->cpu = 0;
		sample->time = 0;
		sample->tid = event->mmap2.tid;
		sample->pid = event->mmap2.pid;
	}
	print_sample_start(sample, thread, evsel);
	perf_event__fprintf(event, stdout);

	return 0;
}

static void sig_handler(int sig __maybe_unused)
{
	session_done = 1;
}

static int __cmd_script(struct perf_script *script)
{
	int ret;

	signal(SIGINT, sig_handler);

	/* override event processing functions */
	if (script->show_task_events) {
		script->tool.comm = process_comm_event;
		script->tool.fork = process_fork_event;
		script->tool.exit = process_exit_event;
	}
	if (script->show_mmap_events) {
		script->tool.mmap = process_mmap_event;
		script->tool.mmap2 = process_mmap2_event;
	}

	ret = perf_session__process_events(script->session);

	if (debug_mode)
		pr_err("Misordered timestamps: %" PRIu64 "\n", nr_unordered);

	return ret;
}

struct script_spec {
	struct list_head	node;
	struct scripting_ops	*ops;
	char			spec[0];
};

static LIST_HEAD(script_specs);

static struct script_spec *script_spec__new(const char *spec,
					    struct scripting_ops *ops)
{
	struct script_spec *s = malloc(sizeof(*s) + strlen(spec) + 1);

	if (s != NULL) {
		strcpy(s->spec, spec);
		s->ops = ops;
	}

	return s;
}

static void script_spec__add(struct script_spec *s)
{
	list_add_tail(&s->node, &script_specs);
}

static struct script_spec *script_spec__find(const char *spec)
{
	struct script_spec *s;

	list_for_each_entry(s, &script_specs, node)
		if (strcasecmp(s->spec, spec) == 0)
			return s;
	return NULL;
}

static struct script_spec *script_spec__findnew(const char *spec,
						struct scripting_ops *ops)
{
	struct script_spec *s = script_spec__find(spec);

	if (s)
		return s;

	s = script_spec__new(spec, ops);
	if (!s)
		return NULL;

	script_spec__add(s);

	return s;
}

int script_spec_register(const char *spec, struct scripting_ops *ops)
{
	struct script_spec *s;

	s = script_spec__find(spec);
	if (s)
		return -1;

	s = script_spec__findnew(spec, ops);
	if (!s)
		return -1;

	return 0;
}

static struct scripting_ops *script_spec__lookup(const char *spec)
{
	struct script_spec *s = script_spec__find(spec);
	if (!s)
		return NULL;

	return s->ops;
}

static void list_available_languages(void)
{
	struct script_spec *s;

	fprintf(stderr, "\n");
	fprintf(stderr, "Scripting language extensions (used in "
		"perf script -s [spec:]script.[spec]):\n\n");

	list_for_each_entry(s, &script_specs, node)
		fprintf(stderr, "  %-42s [%s]\n", s->spec, s->ops->name);

	fprintf(stderr, "\n");
}

static int parse_scriptname(const struct option *opt __maybe_unused,
			    const char *str, int unset __maybe_unused)
{
	char spec[PATH_MAX];
	const char *script, *ext;
	int len;

	if (strcmp(str, "lang") == 0) {
		list_available_languages();
		exit(0);
	}

	script = strchr(str, ':');
	if (script) {
		len = script - str;
		if (len >= PATH_MAX) {
			fprintf(stderr, "invalid language specifier");
			return -1;
		}
		strncpy(spec, str, len);
		spec[len] = '\0';
		scripting_ops = script_spec__lookup(spec);
		if (!scripting_ops) {
			fprintf(stderr, "invalid language specifier");
			return -1;
		}
		script++;
	} else {
		script = str;
		ext = strrchr(script, '.');
		if (!ext) {
			fprintf(stderr, "invalid script extension");
			return -1;
		}
		scripting_ops = script_spec__lookup(++ext);
		if (!scripting_ops) {
			fprintf(stderr, "invalid script extension");
			return -1;
		}
	}

	script_name = strdup(script);

	return 0;
}

static int parse_output_fields(const struct option *opt __maybe_unused,
			    const char *arg, int unset __maybe_unused)
{
	char *tok;
	int i, imax = ARRAY_SIZE(all_output_options);
	int j;
	int rc = 0;
	char *str = strdup(arg);
	int type = -1;

	if (!str)
		return -ENOMEM;

	/* first word can state for which event type the user is specifying
	 * the fields. If no type exists, the specified fields apply to all
	 * event types found in the file minus the invalid fields for a type.
	 */
	tok = strchr(str, ':');
	if (tok) {
		*tok = '\0';
		tok++;
		if (!strcmp(str, "hw"))
			type = PERF_TYPE_HARDWARE;
		else if (!strcmp(str, "sw"))
			type = PERF_TYPE_SOFTWARE;
		else if (!strcmp(str, "trace"))
			type = PERF_TYPE_TRACEPOINT;
		else if (!strcmp(str, "raw"))
			type = PERF_TYPE_RAW;
		else {
			fprintf(stderr, "Invalid event type in field string.\n");
			rc = -EINVAL;
			goto out;
		}

		if (output[type].user_set)
			pr_warning("Overriding previous field request for %s events.\n",
				   event_type(type));

		output[type].fields = 0;
		output[type].user_set = true;
		output[type].wildcard_set = false;

	} else {
		tok = str;
		if (strlen(str) == 0) {
			fprintf(stderr,
				"Cannot set fields to 'none' for all event types.\n");
			rc = -EINVAL;
			goto out;
		}

		if (output_set_by_user())
			pr_warning("Overriding previous field request for all events.\n");

		for (j = 0; j < PERF_TYPE_MAX; ++j) {
			output[j].fields = 0;
			output[j].user_set = true;
			output[j].wildcard_set = true;
		}
	}

	tok = strtok(tok, ",");
	while (tok) {
		for (i = 0; i < imax; ++i) {
			if (strcmp(tok, all_output_options[i].str) == 0)
				break;
		}
		if (i == imax) {
			fprintf(stderr, "Invalid field requested.\n");
			rc = -EINVAL;
			goto out;
		}

		if (type == -1) {
			/* add user option to all events types for
			 * which it is valid
			 */
			for (j = 0; j < PERF_TYPE_MAX; ++j) {
				if (output[j].invalid_fields & all_output_options[i].field) {
					pr_warning("\'%s\' not valid for %s events. Ignoring.\n",
						   all_output_options[i].str, event_type(j));
				} else
					output[j].fields |= all_output_options[i].field;
			}
		} else {
			if (output[type].invalid_fields & all_output_options[i].field) {
				fprintf(stderr, "\'%s\' not valid for %s events.\n",
					 all_output_options[i].str, event_type(type));

				rc = -EINVAL;
				goto out;
			}
			output[type].fields |= all_output_options[i].field;
		}

		tok = strtok(NULL, ",");
	}

	if (type >= 0) {
		if (output[type].fields == 0) {
			pr_debug("No fields requested for %s type. "
				 "Events will not be displayed.\n", event_type(type));
		}
	}

out:
	free(str);
	return rc;
}

/* Helper function for filesystems that return a dent->d_type DT_UNKNOWN */
static int is_directory(const char *base_path, const struct dirent *dent)
{
	char path[PATH_MAX];
	struct stat st;

	sprintf(path, "%s/%s", base_path, dent->d_name);
	if (stat(path, &st))
		return 0;

	return S_ISDIR(st.st_mode);
}

#define for_each_lang(scripts_path, scripts_dir, lang_dirent)		\
	while ((lang_dirent = readdir(scripts_dir)) != NULL)		\
		if ((lang_dirent->d_type == DT_DIR ||			\
		     (lang_dirent->d_type == DT_UNKNOWN &&		\
		      is_directory(scripts_path, lang_dirent))) &&	\
		    (strcmp(lang_dirent->d_name, ".")) &&		\
		    (strcmp(lang_dirent->d_name, "..")))

#define for_each_script(lang_path, lang_dir, script_dirent)		\
	while ((script_dirent = readdir(lang_dir)) != NULL)		\
		if (script_dirent->d_type != DT_DIR &&			\
		    (script_dirent->d_type != DT_UNKNOWN ||		\
		     !is_directory(lang_path, script_dirent)))


#define RECORD_SUFFIX			"-record"
#define REPORT_SUFFIX			"-report"

struct script_desc {
	struct list_head	node;
	char			*name;
	char			*half_liner;
	char			*args;
};

static LIST_HEAD(script_descs);

static struct script_desc *script_desc__new(const char *name)
{
	struct script_desc *s = zalloc(sizeof(*s));

	if (s != NULL && name)
		s->name = strdup(name);

	return s;
}

static void script_desc__delete(struct script_desc *s)
{
	zfree(&s->name);
	zfree(&s->half_liner);
	zfree(&s->args);
	free(s);
}

static void script_desc__add(struct script_desc *s)
{
	list_add_tail(&s->node, &script_descs);
}

static struct script_desc *script_desc__find(const char *name)
{
	struct script_desc *s;

	list_for_each_entry(s, &script_descs, node)
		if (strcasecmp(s->name, name) == 0)
			return s;
	return NULL;
}

static struct script_desc *script_desc__findnew(const char *name)
{
	struct script_desc *s = script_desc__find(name);

	if (s)
		return s;

	s = script_desc__new(name);
	if (!s)
		goto out_delete_desc;

	script_desc__add(s);

	return s;

out_delete_desc:
	script_desc__delete(s);

	return NULL;
}

static const char *ends_with(const char *str, const char *suffix)
{
	size_t suffix_len = strlen(suffix);
	const char *p = str;

	if (strlen(str) > suffix_len) {
		p = str + strlen(str) - suffix_len;
		if (!strncmp(p, suffix, suffix_len))
			return p;
	}

	return NULL;
}

static int read_script_info(struct script_desc *desc, const char *filename)
{
	char line[BUFSIZ], *p;
	FILE *fp;

	fp = fopen(filename, "r");
	if (!fp)
		return -1;

	while (fgets(line, sizeof(line), fp)) {
		p = ltrim(line);
		if (strlen(p) == 0)
			continue;
		if (*p != '#')
			continue;
		p++;
		if (strlen(p) && *p == '!')
			continue;

		p = ltrim(p);
		if (strlen(p) && p[strlen(p) - 1] == '\n')
			p[strlen(p) - 1] = '\0';

		if (!strncmp(p, "description:", strlen("description:"))) {
			p += strlen("description:");
			desc->half_liner = strdup(ltrim(p));
			continue;
		}

		if (!strncmp(p, "args:", strlen("args:"))) {
			p += strlen("args:");
			desc->args = strdup(ltrim(p));
			continue;
		}
	}

	fclose(fp);

	return 0;
}

static char *get_script_root(struct dirent *script_dirent, const char *suffix)
{
	char *script_root, *str;

	script_root = strdup(script_dirent->d_name);
	if (!script_root)
		return NULL;

	str = (char *)ends_with(script_root, suffix);
	if (!str) {
		free(script_root);
		return NULL;
	}

	*str = '\0';
	return script_root;
}

static int list_available_scripts(const struct option *opt __maybe_unused,
				  const char *s __maybe_unused,
				  int unset __maybe_unused)
{
	struct dirent *script_dirent, *lang_dirent;
	char scripts_path[MAXPATHLEN];
	DIR *scripts_dir, *lang_dir;
	char script_path[MAXPATHLEN];
	char lang_path[MAXPATHLEN];
	struct script_desc *desc;
	char first_half[BUFSIZ];
	char *script_root;

	snprintf(scripts_path, MAXPATHLEN, "%s/scripts", perf_exec_path());

	scripts_dir = opendir(scripts_path);
	if (!scripts_dir)
		return -1;

	for_each_lang(scripts_path, scripts_dir, lang_dirent) {
		snprintf(lang_path, MAXPATHLEN, "%s/%s/bin", scripts_path,
			 lang_dirent->d_name);
		lang_dir = opendir(lang_path);
		if (!lang_dir)
			continue;

		for_each_script(lang_path, lang_dir, script_dirent) {
			script_root = get_script_root(script_dirent, REPORT_SUFFIX);
			if (script_root) {
				desc = script_desc__findnew(script_root);
				snprintf(script_path, MAXPATHLEN, "%s/%s",
					 lang_path, script_dirent->d_name);
				read_script_info(desc, script_path);
				free(script_root);
			}
		}
	}

	fprintf(stdout, "List of available trace scripts:\n");
	list_for_each_entry(desc, &script_descs, node) {
		sprintf(first_half, "%s %s", desc->name,
			desc->args ? desc->args : "");
		fprintf(stdout, "  %-36s %s\n", first_half,
			desc->half_liner ? desc->half_liner : "");
	}

	exit(0);
}

/*
 * Some scripts specify the required events in their "xxx-record" file,
 * this function will check if the events in perf.data match those
 * mentioned in the "xxx-record".
 *
 * Fixme: All existing "xxx-record" are all in good formats "-e event ",
 * which is covered well now. And new parsing code should be added to
 * cover the future complexing formats like event groups etc.
 */
static int check_ev_match(char *dir_name, char *scriptname,
			struct perf_session *session)
{
	char filename[MAXPATHLEN], evname[128];
	char line[BUFSIZ], *p;
	struct perf_evsel *pos;
	int match, len;
	FILE *fp;

	sprintf(filename, "%s/bin/%s-record", dir_name, scriptname);

	fp = fopen(filename, "r");
	if (!fp)
		return -1;

	while (fgets(line, sizeof(line), fp)) {
		p = ltrim(line);
		if (*p == '#')
			continue;

		while (strlen(p)) {
			p = strstr(p, "-e");
			if (!p)
				break;

			p += 2;
			p = ltrim(p);
			len = strcspn(p, " \t");
			if (!len)
				break;

			snprintf(evname, len + 1, "%s", p);

			match = 0;
			evlist__for_each(session->evlist, pos) {
				if (!strcmp(perf_evsel__name(pos), evname)) {
					match = 1;
					break;
				}
			}

			if (!match) {
				fclose(fp);
				return -1;
			}
		}
	}

	fclose(fp);
	return 0;
}

/*
 * Return -1 if none is found, otherwise the actual scripts number.
 *
 * Currently the only user of this function is the script browser, which
 * will list all statically runnable scripts, select one, execute it and
 * show the output in a perf browser.
 */
int find_scripts(char **scripts_array, char **scripts_path_array)
{
	struct dirent *script_dirent, *lang_dirent;
	char scripts_path[MAXPATHLEN], lang_path[MAXPATHLEN];
	DIR *scripts_dir, *lang_dir;
	struct perf_session *session;
	struct perf_data_file file = {
		.path = input_name,
		.mode = PERF_DATA_MODE_READ,
	};
	char *temp;
	int i = 0;

	session = perf_session__new(&file, false, NULL);
	if (!session)
		return -1;

	snprintf(scripts_path, MAXPATHLEN, "%s/scripts", perf_exec_path());

	scripts_dir = opendir(scripts_path);
	if (!scripts_dir) {
		perf_session__delete(session);
		return -1;
	}

	for_each_lang(scripts_path, scripts_dir, lang_dirent) {
		snprintf(lang_path, MAXPATHLEN, "%s/%s", scripts_path,
			 lang_dirent->d_name);
#ifdef NO_LIBPERL
		if (strstr(lang_path, "perl"))
			continue;
#endif
#ifdef NO_LIBPYTHON
		if (strstr(lang_path, "python"))
			continue;
#endif

		lang_dir = opendir(lang_path);
		if (!lang_dir)
			continue;

		for_each_script(lang_path, lang_dir, script_dirent) {
			/* Skip those real time scripts: xxxtop.p[yl] */
			if (strstr(script_dirent->d_name, "top."))
				continue;
			sprintf(scripts_path_array[i], "%s/%s", lang_path,
				script_dirent->d_name);
			temp = strchr(script_dirent->d_name, '.');
			snprintf(scripts_array[i],
				(temp - script_dirent->d_name) + 1,
				"%s", script_dirent->d_name);

			if (check_ev_match(lang_path,
					scripts_array[i], session))
				continue;

			i++;
		}
		closedir(lang_dir);
	}

	closedir(scripts_dir);
	perf_session__delete(session);
	return i;
}

static char *get_script_path(const char *script_root, const char *suffix)
{
	struct dirent *script_dirent, *lang_dirent;
	char scripts_path[MAXPATHLEN];
	char script_path[MAXPATHLEN];
	DIR *scripts_dir, *lang_dir;
	char lang_path[MAXPATHLEN];
	char *__script_root;

	snprintf(scripts_path, MAXPATHLEN, "%s/scripts", perf_exec_path());

	scripts_dir = opendir(scripts_path);
	if (!scripts_dir)
		return NULL;

	for_each_lang(scripts_path, scripts_dir, lang_dirent) {
		snprintf(lang_path, MAXPATHLEN, "%s/%s/bin", scripts_path,
			 lang_dirent->d_name);
		lang_dir = opendir(lang_path);
		if (!lang_dir)
			continue;

		for_each_script(lang_path, lang_dir, script_dirent) {
			__script_root = get_script_root(script_dirent, suffix);
			if (__script_root && !strcmp(script_root, __script_root)) {
				free(__script_root);
				closedir(lang_dir);
				closedir(scripts_dir);
				snprintf(script_path, MAXPATHLEN, "%s/%s",
					 lang_path, script_dirent->d_name);
				return strdup(script_path);
			}
			free(__script_root);
		}
		closedir(lang_dir);
	}
	closedir(scripts_dir);

	return NULL;
}

static bool is_top_script(const char *script_path)
{
	return ends_with(script_path, "top") == NULL ? false : true;
}

static int has_required_arg(char *script_path)
{
	struct script_desc *desc;
	int n_args = 0;
	char *p;

	desc = script_desc__new(NULL);

	if (read_script_info(desc, script_path))
		goto out;

	if (!desc->args)
		goto out;

	for (p = desc->args; *p; p++)
		if (*p == '<')
			n_args++;
out:
	script_desc__delete(desc);

	return n_args;
}

static int have_cmd(int argc, const char **argv)
{
	char **__argv = malloc(sizeof(const char *) * argc);

	if (!__argv) {
		pr_err("malloc failed\n");
		return -1;
	}

	memcpy(__argv, argv, sizeof(const char *) * argc);
	argc = parse_options(argc, (const char **)__argv, record_options,
			     NULL, PARSE_OPT_STOP_AT_NON_OPTION);
	free(__argv);

	system_wide = (argc == 0);

	return 0;
}

int cmd_script(int argc, const char **argv, const char *prefix __maybe_unused)
{
	bool show_full_info = false;
	bool header = false;
	bool header_only = false;
	bool script_started = false;
	char *rec_script_path = NULL;
	char *rep_script_path = NULL;
	struct perf_session *session;
	char *script_path = NULL;
	const char **__argv;
	int i, j, err = 0;
	struct perf_script script = {
		.tool = {
			.sample		 = process_sample_event,
			.mmap		 = perf_event__process_mmap,
			.mmap2		 = perf_event__process_mmap2,
			.comm		 = perf_event__process_comm,
			.exit		 = perf_event__process_exit,
			.fork		 = perf_event__process_fork,
			.attr		 = process_attr,
			.tracing_data	 = perf_event__process_tracing_data,
			.build_id	 = perf_event__process_build_id,
			.ordered_events	 = true,
			.ordering_requires_timestamps = true,
		},
	};
	struct perf_data_file file = {
		.mode = PERF_DATA_MODE_READ,
	};
	const struct option options[] = {
	OPT_BOOLEAN('D', "dump-raw-trace", &dump_trace,
		    "dump raw trace in ASCII"),
	OPT_INCR('v', "verbose", &verbose,
		 "be more verbose (show symbol address, etc)"),
	OPT_BOOLEAN('L', "Latency", &latency_format,
		    "show latency attributes (irqs/preemption disabled, etc)"),
	OPT_CALLBACK_NOOPT('l', "list", NULL, NULL, "list available scripts",
			   list_available_scripts),
	OPT_CALLBACK('s', "script", NULL, "name",
		     "script file name (lang:script name, script name, or *)",
		     parse_scriptname),
	OPT_STRING('g', "gen-script", &generate_script_lang, "lang",
		   "generate perf-script.xx script in specified language"),
	OPT_STRING('i', "input", &input_name, "file", "input file name"),
	OPT_BOOLEAN('d', "debug-mode", &debug_mode,
		   "do various checks like samples ordering and lost events"),
	OPT_BOOLEAN(0, "header", &header, "Show data header."),
	OPT_BOOLEAN(0, "header-only", &header_only, "Show only data header."),
	OPT_STRING('k', "vmlinux", &symbol_conf.vmlinux_name,
		   "file", "vmlinux pathname"),
	OPT_STRING(0, "kallsyms", &symbol_conf.kallsyms_name,
		   "file", "kallsyms pathname"),
	OPT_BOOLEAN('G', "hide-call-graph", &no_callchain,
		    "When printing symbols do not display call chain"),
	OPT_STRING(0, "symfs", &symbol_conf.symfs, "directory",
		    "Look for files with symbols relative to this directory"),
	OPT_CALLBACK('F', "fields", NULL, "str",
		     "comma separated output fields prepend with 'type:'. "
		     "Valid types: hw,sw,trace,raw. "
		     "Fields: comm,tid,pid,time,cpu,event,trace,ip,sym,dso,"
		     "addr,symoff,period", parse_output_fields),
	OPT_BOOLEAN('a', "all-cpus", &system_wide,
		    "system-wide collection from all CPUs"),
	OPT_STRING('S', "symbols", &symbol_conf.sym_list_str, "symbol[,symbol...]",
		   "only consider these symbols"),
	OPT_STRING('C', "cpu", &cpu_list, "cpu", "list of cpus to profile"),
	OPT_STRING('c', "comms", &symbol_conf.comm_list_str, "comm[,comm...]",
		   "only display events for these comms"),
	OPT_STRING(0, "pid", &symbol_conf.pid_list_str, "pid[,pid...]",
		   "only consider symbols in these pids"),
	OPT_STRING(0, "tid", &symbol_conf.tid_list_str, "tid[,tid...]",
		   "only consider symbols in these tids"),
	OPT_BOOLEAN('I', "show-info", &show_full_info,
		    "display extended information from perf.data file"),
	OPT_BOOLEAN('\0', "show-kernel-path", &symbol_conf.show_kernel_path,
		    "Show the path of [kernel.kallsyms]"),
	OPT_BOOLEAN('\0', "show-task-events", &script.show_task_events,
		    "Show the fork/comm/exit events"),
	OPT_BOOLEAN('\0', "show-mmap-events", &script.show_mmap_events,
		    "Show the mmap events"),
	OPT_BOOLEAN('f', "force", &file.force, "don't complain, do it"),
	OPT_END()
	};
	const char * const script_subcommands[] = { "record", "report", NULL };
	const char *script_usage[] = {
		"perf script [<options>]",
		"perf script [<options>] record <script> [<record-options>] <command>",
		"perf script [<options>] report <script> [script-args]",
		"perf script [<options>] <script> [<record-options>] <command>",
		"perf script [<options>] <top-script> [script-args]",
		NULL
	};

	setup_scripting();

	argc = parse_options_subcommand(argc, argv, options, script_subcommands, script_usage,
			     PARSE_OPT_STOP_AT_NON_OPTION);

	file.path = input_name;

	if (argc > 1 && !strncmp(argv[0], "rec", strlen("rec"))) {
		rec_script_path = get_script_path(argv[1], RECORD_SUFFIX);
		if (!rec_script_path)
			return cmd_record(argc, argv, NULL);
	}

	if (argc > 1 && !strncmp(argv[0], "rep", strlen("rep"))) {
		rep_script_path = get_script_path(argv[1], REPORT_SUFFIX);
		if (!rep_script_path) {
			fprintf(stderr,
				"Please specify a valid report script"
				"(see 'perf script -l' for listing)\n");
			return -1;
		}
	}

	/* make sure PERF_EXEC_PATH is set for scripts */
	perf_set_argv_exec_path(perf_exec_path());

	if (argc && !script_name && !rec_script_path && !rep_script_path) {
		int live_pipe[2];
		int rep_args;
		pid_t pid;

		rec_script_path = get_script_path(argv[0], RECORD_SUFFIX);
		rep_script_path = get_script_path(argv[0], REPORT_SUFFIX);

		if (!rec_script_path && !rep_script_path) {
			fprintf(stderr, " Couldn't find script %s\n\n See perf"
				" script -l for available scripts.\n", argv[0]);
			usage_with_options(script_usage, options);
		}

		if (is_top_script(argv[0])) {
			rep_args = argc - 1;
		} else {
			int rec_args;

			rep_args = has_required_arg(rep_script_path);
			rec_args = (argc - 1) - rep_args;
			if (rec_args < 0) {
				fprintf(stderr, " %s script requires options."
					"\n\n See perf script -l for available "
					"scripts and options.\n", argv[0]);
				usage_with_options(script_usage, options);
			}
		}

		if (pipe(live_pipe) < 0) {
			perror("failed to create pipe");
			return -1;
		}

		pid = fork();
		if (pid < 0) {
			perror("failed to fork");
			return -1;
		}

		if (!pid) {
			j = 0;

			dup2(live_pipe[1], 1);
			close(live_pipe[0]);

			if (is_top_script(argv[0])) {
				system_wide = true;
			} else if (!system_wide) {
				if (have_cmd(argc - rep_args, &argv[rep_args]) != 0) {
					err = -1;
					goto out;
				}
			}

			__argv = malloc((argc + 6) * sizeof(const char *));
			if (!__argv) {
				pr_err("malloc failed\n");
				err = -ENOMEM;
				goto out;
			}

			__argv[j++] = "/bin/sh";
			__argv[j++] = rec_script_path;
			if (system_wide)
				__argv[j++] = "-a";
			__argv[j++] = "-q";
			__argv[j++] = "-o";
			__argv[j++] = "-";
			for (i = rep_args + 1; i < argc; i++)
				__argv[j++] = argv[i];
			__argv[j++] = NULL;

			execvp("/bin/sh", (char **)__argv);
			free(__argv);
			exit(-1);
		}

		dup2(live_pipe[0], 0);
		close(live_pipe[1]);

		__argv = malloc((argc + 4) * sizeof(const char *));
		if (!__argv) {
			pr_err("malloc failed\n");
			err = -ENOMEM;
			goto out;
		}

		j = 0;
		__argv[j++] = "/bin/sh";
		__argv[j++] = rep_script_path;
		for (i = 1; i < rep_args + 1; i++)
			__argv[j++] = argv[i];
		__argv[j++] = "-i";
		__argv[j++] = "-";
		__argv[j++] = NULL;

		execvp("/bin/sh", (char **)__argv);
		free(__argv);
		exit(-1);
	}

	if (rec_script_path)
		script_path = rec_script_path;
	if (rep_script_path)
		script_path = rep_script_path;

	if (script_path) {
		j = 0;

		if (!rec_script_path)
			system_wide = false;
		else if (!system_wide) {
			if (have_cmd(argc - 1, &argv[1]) != 0) {
				err = -1;
				goto out;
			}
		}

		__argv = malloc((argc + 2) * sizeof(const char *));
		if (!__argv) {
			pr_err("malloc failed\n");
			err = -ENOMEM;
			goto out;
		}

		__argv[j++] = "/bin/sh";
		__argv[j++] = script_path;
		if (system_wide)
			__argv[j++] = "-a";
		for (i = 2; i < argc; i++)
			__argv[j++] = argv[i];
		__argv[j++] = NULL;

		execvp("/bin/sh", (char **)__argv);
		free(__argv);
		exit(-1);
	}

	if (!script_name)
		setup_pager();

	session = perf_session__new(&file, false, &script.tool);
	if (session == NULL)
		return -1;

	if (header || header_only) {
		perf_session__fprintf_info(session, stdout, show_full_info);
		if (header_only)
			goto out_delete;
	}

	if (symbol__init(&session->header.env) < 0)
		goto out_delete;

	script.session = session;

	if (cpu_list) {
		err = perf_session__cpu_bitmap(session, cpu_list, cpu_bitmap);
		if (err < 0)
			goto out_delete;
	}

	if (!no_callchain)
		symbol_conf.use_callchain = true;
	else
		symbol_conf.use_callchain = false;

	if (generate_script_lang) {
		struct stat perf_stat;
		int input;

		if (output_set_by_user()) {
			fprintf(stderr,
				"custom fields not supported for generated scripts");
			err = -EINVAL;
			goto out_delete;
		}

		input = open(file.path, O_RDONLY);	/* input_name */
		if (input < 0) {
			err = -errno;
			perror("failed to open file");
			goto out_delete;
		}

		err = fstat(input, &perf_stat);
		if (err < 0) {
			perror("failed to stat file");
			goto out_delete;
		}

		if (!perf_stat.st_size) {
			fprintf(stderr, "zero-sized file, nothing to do!\n");
			goto out_delete;
		}

		scripting_ops = script_spec__lookup(generate_script_lang);
		if (!scripting_ops) {
			fprintf(stderr, "invalid language specifier");
			err = -ENOENT;
			goto out_delete;
		}

		err = scripting_ops->generate_script(session->tevent.pevent,
						     "perf-script");
		goto out_delete;
	}

	if (script_name) {
		err = scripting_ops->start_script(script_name, argc, argv);
		if (err)
			goto out_delete;
		pr_debug("perf script started with script %s\n\n", script_name);
		script_started = true;
	}


	err = perf_session__check_output_opt(session);
	if (err < 0)
		goto out_delete;

	err = __cmd_script(&script);

	flush_scripting();

out_delete:
	perf_session__delete(session);

	if (script_started)
		cleanup_scripting();
out:
	return err;
}
