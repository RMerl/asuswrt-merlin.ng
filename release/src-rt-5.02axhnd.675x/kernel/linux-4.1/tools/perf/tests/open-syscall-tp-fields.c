#include "perf.h"
#include "evlist.h"
#include "evsel.h"
#include "thread_map.h"
#include "tests.h"
#include "debug.h"

int test__syscall_open_tp_fields(void)
{
	struct record_opts opts = {
		.target = {
			.uid = UINT_MAX,
			.uses_mmap = true,
		},
		.no_buffering = true,
		.freq	      = 1,
		.mmap_pages   = 256,
		.raw_samples  = true,
	};
	const char *filename = "/etc/passwd";
	int flags = O_RDONLY | O_DIRECTORY;
	struct perf_evlist *evlist = perf_evlist__new();
	struct perf_evsel *evsel;
	int err = -1, i, nr_events = 0, nr_polls = 0;
	char sbuf[STRERR_BUFSIZE];

	if (evlist == NULL) {
		pr_debug("%s: perf_evlist__new\n", __func__);
		goto out;
	}

	evsel = perf_evsel__newtp("syscalls", "sys_enter_open");
	if (evsel == NULL) {
		pr_debug("%s: perf_evsel__newtp\n", __func__);
		goto out_delete_evlist;
	}

	perf_evlist__add(evlist, evsel);

	err = perf_evlist__create_maps(evlist, &opts.target);
	if (err < 0) {
		pr_debug("%s: perf_evlist__create_maps\n", __func__);
		goto out_delete_evlist;
	}

	perf_evsel__config(evsel, &opts);

	evlist->threads->map[0] = getpid();

	err = perf_evlist__open(evlist);
	if (err < 0) {
		pr_debug("perf_evlist__open: %s\n",
			 strerror_r(errno, sbuf, sizeof(sbuf)));
		goto out_delete_evlist;
	}

	err = perf_evlist__mmap(evlist, UINT_MAX, false);
	if (err < 0) {
		pr_debug("perf_evlist__mmap: %s\n",
			 strerror_r(errno, sbuf, sizeof(sbuf)));
		goto out_delete_evlist;
	}

	perf_evlist__enable(evlist);

	/*
	 * Generate the event:
	 */
	open(filename, flags);

	while (1) {
		int before = nr_events;

		for (i = 0; i < evlist->nr_mmaps; i++) {
			union perf_event *event;

			while ((event = perf_evlist__mmap_read(evlist, i)) != NULL) {
				const u32 type = event->header.type;
				int tp_flags;
				struct perf_sample sample;

				++nr_events;

				if (type != PERF_RECORD_SAMPLE) {
					perf_evlist__mmap_consume(evlist, i);
					continue;
				}

				err = perf_evsel__parse_sample(evsel, event, &sample);
				if (err) {
					pr_err("Can't parse sample, err = %d\n", err);
					goto out_delete_evlist;
				}

				tp_flags = perf_evsel__intval(evsel, &sample, "flags");

				if (flags != tp_flags) {
					pr_debug("%s: Expected flags=%#x, got %#x\n",
						 __func__, flags, tp_flags);
					goto out_delete_evlist;
				}

				goto out_ok;
			}
		}

		if (nr_events == before)
			perf_evlist__poll(evlist, 10);

		if (++nr_polls > 5) {
			pr_debug("%s: no events!\n", __func__);
			goto out_delete_evlist;
		}
	}
out_ok:
	err = 0;
out_delete_evlist:
	perf_evlist__delete(evlist);
out:
	return err;
}
