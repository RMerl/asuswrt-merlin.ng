#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <rc.h>
#include <shared.h>
#include <shutils.h>

#define PRESSURE_PATH "/sys/bus/iio/devices/iio:device0/in_pressure_input"
#define TEMP_PATH     "/sys/bus/iio/devices/iio:device0/in_temp_input"

typedef struct {
	int t_sec;              // -t; <=1 : get onew time; >1 : get average value
	int sample_sec;         // -s; default 10 times per sec
	bool want_pressure;     // -p
	bool want_temp;         // -T
} cmp_cfg_t;

typedef struct {
    double sum;
    int count;
} agg_t;

static void agg_reset(agg_t *a)
{
	a->sum = 0.0; a->count = 0;
}

static void agg_add(agg_t *a, double v)
{
	a->sum += v; a->count++;
}

static bool agg_has(const agg_t *a)
{
	return a->count > 0;
}

static double agg_avg(const agg_t *a)
{
	return (a->count > 0) ? (a->sum / a->count) : 0.0;
}

static void usage(const char *prog)
{
	printf("Usage: %s [options]\n", prog);
	printf("Options:\n");
	printf("  -t <sec>   measure duration in seconds (default: 1; <=1 treated as 1)\n");
	printf("  -s <n>     samples per second (default: 10)\n");
	printf("  -p         pressure only\n");
	printf("  -T         temperature only\n");
	printf("  -h         help\n");
	printf("\nExamples:\n");
	printf("  %s                 # avg over 1 sec, 10 samples/sec\n", prog);
	printf("  %s -t 2            # avg over 2 sec, 10 samples/sec\n", prog);
	printf("  %s -t 5 -s 20 -p   # avg pressure over 5 sec, 20 samples/sec\n", prog);
}

static int read_file_double(const char *path, double *out)
{
	FILE *fp = fopen(path, "r");
	if (!fp) return -1;

	char buf[128];
	if (!fgets(buf, sizeof(buf), fp)) {
		fclose(fp);
		return -1;
	}

	fclose(fp);

	char *p = buf;
	while (*p == ' ' || *p == '\t') p++;

	errno = 0;
	char *end = NULL;
	double v = strtod(buf, &end);
	if (errno != 0 || end == buf) return -1;

	*out = v;
	return 0;
}

static int read_pressure(double *out)
{
	double raw;
	if (read_file_double(PRESSURE_PATH, &raw) < 0)
		return -1;

	// raw / 1000 = real pressure
	*out = raw / 1000.0;
	printf("Pressure : %.6f\n", *out);
	return 0;
}

static int read_temp(double *out)
{
	double raw;
	if (read_file_double(TEMP_PATH, &raw) < 0)
		return -1;

	// raw --> Celsius
	double f = raw / 655360.0;

	*out = f;
	printf("Temperature(Celsius) : %.6f\n", *out);
	return 0;
}

static inline long long now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

static void sleep_ns(long long ns)
{
    if (ns <= 0) return;
    struct timespec req;
    req.tv_sec  = (time_t)(ns / 1000000000LL);
    req.tv_nsec = (long)(ns % 1000000000LL);
    while (nanosleep(&req, &req) == -1 && errno == EINTR) {
        // retry
    }
}

static void init_nvram_unset()
{
	nvram_unset("cmp-temperature");
	nvram_unset("cmp-pressure");
}

int cmp_pressure_main(int argc, char **argv)
{
	init_nvram_unset();

	cmp_cfg_t cfg;
	memset(&cfg, 0, sizeof(cfg));
	cfg.t_sec = 1;                  // default: at least 1 second
	cfg.sample_sec = 10;   // default : 10 samples per second
	cfg.want_pressure = true;
	cfg.want_temp = true;

	static const struct option opts[] = {
		{"time",     required_argument, 0, 't'},
		{"sleep",    required_argument, 0, 's'},
		{"pressure", no_argument,       0, 'p'},
		{"temp",     no_argument,       0, 'T'},
		{"help",     no_argument,       0, 'h'},
		{0, 0, 0, 0}
	};

	bool user_select = false;
	int opt;
	while ((opt = getopt_long(argc, argv, "t:s:pTh", opts, NULL)) != -1) {
		switch (opt) {
			case 't':
				int t = safe_atoi(optarg);
				cfg.t_sec = (t <= 1) ? 1 : t;
				if (cfg.t_sec < 1 || cfg.t_sec > 3600) {
					fprintf(stderr, "invalid -t value\n");
					return 1;
				}
				break;
			case 's':
				int s = safe_atoi(optarg);
				if (s < 1 || s > 1000) {
					fprintf(stderr, "invalid -s value (1-1000)\n");
					return 1;
				}
				cfg.sample_sec = s;
				break;
			case 'p':
				if (!user_select) {
					cfg.want_pressure = false;
					cfg.want_temp = false;
					user_select = true;
				}
				cfg.want_pressure = true;
				break;
			case 'T':
				if (!user_select) {
					cfg.want_pressure = false;
					cfg.want_temp = false;
					user_select = true;
				}
				cfg.want_temp = true;
				break;
			case 'h':
			default:
				usage(argv[0]);
			return 0;
		}
	}

	// Here just for protection while pressure disable + temperature disable (won't happen for now)
	if (!cfg.want_pressure && !cfg.want_temp) {
		fprintf(stderr, "Nothing selected (-p/-T)\n");
		return 1;
	}

	const long long interval_ns = 1000000000LL / cfg.sample_sec;
	const long long end_ns = now_ns() + (long long)cfg.t_sec * 1000000000LL;

	agg_t p_agg, t_agg;
	agg_reset(&p_agg);
	agg_reset(&t_agg);

	long long next_sample_ns = now_ns(); // sample immediately

	while (1) {
		long long n = now_ns();
		if (n >= end_ns) break;

		if (n < next_sample_ns) {
			sleep_ns(next_sample_ns - n);
		}

		double v;
		if (cfg.want_pressure && read_pressure(&v) == 0)
			agg_add(&p_agg, v);
		if (cfg.want_temp && read_temp(&v) == 0)
			agg_add(&t_agg, v);

		next_sample_ns += interval_ns;

		long long n2 = now_ns();
		if (next_sample_ns < n2 - interval_ns) {
			next_sample_ns = n2;
		}
	}

	if (cfg.want_pressure && !agg_has(&p_agg)) {
		fprintf(stderr, "no pressure samples\n");
		return 1;
	}
	if (cfg.want_temp && !agg_has(&t_agg)) {
		fprintf(stderr, "no temp samples\n");
		return 1;
	}

	if (cfg.want_pressure && cfg.want_temp) {
		double pressure_val = agg_avg(&p_agg);
		double temperature_val = agg_avg(&t_agg);
		char pressure_buf[64] = {0};
		char temperature_buf[64] = {0};

		snprintf(pressure_buf, sizeof(pressure_buf), "%.6f", pressure_val);
		snprintf(temperature_buf, sizeof(temperature_buf), "%.6f", temperature_val);
		printf("pressure_avg=%.6f temperature_avg=%.6f\n", agg_avg(&p_agg), agg_avg(&t_agg));
		nvram_set("cmp-pressure", pressure_buf);
		nvram_set("cmp-temperature", temperature_buf);
	}
	else if (cfg.want_pressure) {
		printf("pressure_avg=%.6f\n", agg_avg(&p_agg));
	}
	else { // cfg.want_temp
		printf("temperature_avg=%.6f\n", agg_avg(&t_agg));
	}

	return 0;
}
