/* bench-slope.c - for libgcrypt
 * Copyright (C) 2013 Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <float.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _GCRYPT_IN_LIBGCRYPT
# include "../src/gcrypt-int.h"
# include "../compat/libcompat.h"
#else
# include <gcrypt.h>
#endif

#ifndef STR
#define STR(v) #v
#define STR2(v) STR(v)
#endif

#define PGM "bench-slope"
#include "t-common.h"

static int verbose;
static int csv_mode;
static int unaligned_mode;
static int num_measurement_repetitions;

/* CPU Ghz value provided by user, allows constructing cycles/byte and other
   results.  */
static double cpu_ghz = -1;

/* Attempt to autodetect CPU Ghz. */
static int auto_ghz;

/* Whether we are running as part of the regression test suite.  */
static int in_regression_test;

/* The name of the currently printed section.  */
static char *current_section_name;
/* The name of the currently printed algorithm.  */
static char *current_algo_name;
/* The name of the currently printed mode.  */
static char *current_mode_name;


/* Currently used CPU Ghz (either user input or auto-detected. */
static double bench_ghz;

/* Current accuracy of auto-detected CPU Ghz. */
static double bench_ghz_diff;

static int in_fips_mode;

/*************************************** Default parameters for measurements. */

/* Start at small buffer size, to get reasonable timer calibration for fast
 * implementations (AES-NI etc). Sixteen selected to support the largest block
 * size of current set cipher blocks. */
#define BUF_START_SIZE			16

/* From ~0 to ~4kbytes give comparable results with results from academia
 * (SUPERCOP). */
#define BUF_END_SIZE			(BUF_START_SIZE + 4096)

/* With 128 byte steps, we get (4096)/64 = 64 data points. */
#define BUF_STEP_SIZE			64

/* Number of repeated measurements at each data point. The median of these
 * measurements is selected as data point further analysis. */
#define NUM_MEASUREMENT_REPETITIONS	64

/* Target accuracy for auto-detected CPU Ghz. */
#define AUTO_GHZ_TARGET_DIFF		(5e-5)

/**************************************************** High-resolution timers. */

/* This benchmarking module needs needs high resolution timer.  */
#undef NO_GET_NSEC_TIME
#if defined(_WIN32)
struct nsec_time
{
  LARGE_INTEGER perf_count;
};

static void
get_nsec_time (struct nsec_time *t)
{
  BOOL ok;

  ok = QueryPerformanceCounter (&t->perf_count);
  assert (ok);
}

static double
get_time_nsec_diff (struct nsec_time *start, struct nsec_time *end)
{
  static double nsecs_per_count = 0.0;
  double nsecs;

  if (nsecs_per_count == 0.0)
    {
      LARGE_INTEGER perf_freq;
      BOOL ok;

      /* Get counts per second. */
      ok = QueryPerformanceFrequency (&perf_freq);
      assert (ok);

      nsecs_per_count = 1.0 / perf_freq.QuadPart;
      nsecs_per_count *= 1000000.0 * 1000.0;	/* sec => nsec */

      assert (nsecs_per_count > 0.0);
    }

  nsecs = end->perf_count.QuadPart - start->perf_count.QuadPart;	/* counts */
  nsecs *= nsecs_per_count;	/* counts * (nsecs / count) => nsecs */

  return nsecs;
}
#elif defined(HAVE_CLOCK_GETTIME)
struct nsec_time
{
  struct timespec ts;
};

static void
get_nsec_time (struct nsec_time *t)
{
  int err;

  err = clock_gettime (CLOCK_REALTIME, &t->ts);
  assert (err == 0);
}

static double
get_time_nsec_diff (struct nsec_time *start, struct nsec_time *end)
{
  double nsecs;

  nsecs = end->ts.tv_sec - start->ts.tv_sec;
  nsecs *= 1000000.0 * 1000.0;	/* sec => nsec */

  /* This way we don't have to care if tv_nsec unsigned or signed. */
  if (end->ts.tv_nsec >= start->ts.tv_nsec)
    nsecs += end->ts.tv_nsec - start->ts.tv_nsec;
  else
    nsecs -= start->ts.tv_nsec - end->ts.tv_nsec;

  return nsecs;
}
#elif defined(HAVE_GETTIMEOFDAY)
struct nsec_time
{
  struct timeval tv;
};

static void
get_nsec_time (struct nsec_time *t)
{
  int err;

  err = gettimeofday (&t->tv, NULL);
  assert (err == 0);
}

static double
get_time_nsec_diff (struct nsec_time *start, struct nsec_time *end)
{
  double nsecs;

  nsecs = end->tv.tv_sec - start->tv.tv_sec;
  nsecs *= 1000000;		/* sec => µsec */

  /* This way we don't have to care if tv_usec unsigned or signed. */
  if (end->tv.tv_usec >= start->tv.tv_usec)
    nsecs += end->tv.tv_usec - start->tv.tv_usec;
  else
    nsecs -= start->tv.tv_usec - end->tv.tv_usec;

  nsecs *= 1000;		/* µsec => nsec */

  return nsecs;
}
#else
#define NO_GET_NSEC_TIME 1
#endif


/* If no high resolution timer found, provide dummy bench-slope.  */
#ifdef NO_GET_NSEC_TIME


int
main (void)
{
  /* No nsec timer => SKIP test. */
  return 77;
}


#else /* !NO_GET_NSEC_TIME */


/********************************************** Slope benchmarking framework. */

struct bench_obj
{
  const struct bench_ops *ops;

  unsigned int num_measure_repetitions;
  unsigned int min_bufsize;
  unsigned int max_bufsize;
  unsigned int step_size;

  void *priv;
  void *hd;
};

typedef int (*const bench_initialize_t) (struct bench_obj * obj);
typedef void (*const bench_finalize_t) (struct bench_obj * obj);
typedef void (*const bench_do_run_t) (struct bench_obj * obj, void *buffer,
				      size_t buflen);

struct bench_ops
{
  bench_initialize_t initialize;
  bench_finalize_t finalize;
  bench_do_run_t do_run;
};


static double
safe_div (double x, double y)
{
  union
  {
    double d;
    char buf[sizeof(double)];
  } u_neg_zero, u_y;

  if (y != 0)
    return x / y;

  u_neg_zero.d = -0.0;
  u_y.d = y;
  if (memcmp(u_neg_zero.buf, u_y.buf, sizeof(double)) == 0)
    return -DBL_MAX;

  return DBL_MAX;
}


static double
get_slope (double (*const get_x) (unsigned int idx, void *priv),
	   void *get_x_priv, double y_points[], unsigned int npoints,
	   double *overhead)
{
  double sumx, sumy, sumx2, sumy2, sumxy;
  unsigned int i;
  double b, a;

  sumx = sumy = sumx2 = sumy2 = sumxy = 0;

  if (npoints <= 1)
    {
      /* No slope with zero or one point. */
      return 0;
    }

  for (i = 0; i < npoints; i++)
    {
      double x, y;

      x = get_x (i, get_x_priv);	/* bytes */
      y = y_points[i];			/* nsecs */

      sumx += x;
      sumy += y;
      sumx2 += x * x;
      /*sumy2 += y * y;*/
      sumxy += x * y;
    }

  b = safe_div(npoints * sumxy - sumx * sumy, npoints * sumx2 - sumx * sumx);

  if (overhead)
    {
      a = safe_div(sumy - b * sumx, npoints);
      *overhead = a;		/* nsecs */
    }

  return b;			/* nsecs per byte */
}


double
get_bench_obj_point_x (unsigned int idx, void *priv)
{
  struct bench_obj *obj = priv;
  return (double) (obj->min_bufsize + (idx * obj->step_size));
}


unsigned int
get_num_measurements (struct bench_obj *obj)
{
  unsigned int buf_range = obj->max_bufsize - obj->min_bufsize;
  unsigned int num = buf_range / obj->step_size + 1;

  while (obj->min_bufsize + (num * obj->step_size) > obj->max_bufsize)
    num--;

  return num + 1;
}


static int
double_cmp (const void *_a, const void *_b)
{
  const double *a, *b;

  a = _a;
  b = _b;

  if (*a > *b)
    return 1;
  if (*a < *b)
    return -1;
  return 0;
}


double
do_bench_obj_measurement (struct bench_obj *obj, void *buffer, size_t buflen,
			  double *measurement_raw,
			  unsigned int loop_iterations)
{
  const unsigned int num_repetitions = obj->num_measure_repetitions;
  const bench_do_run_t do_run = obj->ops->do_run;
  struct nsec_time start, end;
  unsigned int rep, loop;
  double res;

  if (num_repetitions < 1 || loop_iterations < 1)
    return 0.0;

  for (rep = 0; rep < num_repetitions; rep++)
    {
      get_nsec_time (&start);

      for (loop = 0; loop < loop_iterations; loop++)
	do_run (obj, buffer, buflen);

      get_nsec_time (&end);

      measurement_raw[rep] = get_time_nsec_diff (&start, &end);
    }

  /* Return median of repeated measurements. */
  qsort (measurement_raw, num_repetitions, sizeof (measurement_raw[0]),
	 double_cmp);

  if (num_repetitions % 2 == 1)
    return measurement_raw[num_repetitions / 2];

  res = measurement_raw[num_repetitions / 2]
    + measurement_raw[num_repetitions / 2 - 1];
  return res / 2;
}


unsigned int
adjust_loop_iterations_to_timer_accuracy (struct bench_obj *obj, void *buffer,
					  double *measurement_raw)
{
  const double increase_thres = 3.0;
  double tmp, nsecs;
  unsigned int loop_iterations;
  unsigned int test_bufsize;

  test_bufsize = obj->min_bufsize;
  if (test_bufsize == 0)
    test_bufsize += obj->step_size;

  loop_iterations = 0;
  do
    {
      /* Increase loop iterations until we get other results than zero.  */
      nsecs =
	do_bench_obj_measurement (obj, buffer, test_bufsize,
				  measurement_raw, ++loop_iterations);
    }
  while (nsecs < 1.0 - 0.1);
  do
    {
      /* Increase loop iterations until we get reasonable increase for elapsed time.  */
      tmp =
	do_bench_obj_measurement (obj, buffer, test_bufsize,
				  measurement_raw, ++loop_iterations);
    }
  while (tmp < nsecs * (increase_thres - 0.1));

  return loop_iterations;
}


/* Benchmark and return linear regression slope in nanoseconds per byte.  */
double
slope_benchmark (struct bench_obj *obj)
{
  unsigned int num_measurements;
  double *measurements = NULL;
  double *measurement_raw = NULL;
  double slope, overhead;
  unsigned int loop_iterations, midx, i;
  unsigned char *real_buffer = NULL;
  unsigned char *buffer;
  size_t cur_bufsize;
  int err;

  err = obj->ops->initialize (obj);
  if (err < 0)
    return -1;

  num_measurements = get_num_measurements (obj);
  measurements = calloc (num_measurements, sizeof (*measurements));
  if (!measurements)
    goto err_free;

  measurement_raw =
    calloc (obj->num_measure_repetitions, sizeof (*measurement_raw));
  if (!measurement_raw)
    goto err_free;

  if (num_measurements < 1 || obj->num_measure_repetitions < 1 ||
      obj->max_bufsize < 1 || obj->min_bufsize > obj->max_bufsize)
    goto err_free;

  real_buffer = malloc (obj->max_bufsize + 128 + unaligned_mode);
  if (!real_buffer)
    goto err_free;
  /* Get aligned buffer */
  buffer = real_buffer;
  buffer += 128 - ((real_buffer - (unsigned char *) 0) & (128 - 1));
  if (unaligned_mode)
    buffer += unaligned_mode; /* Make buffer unaligned */

  for (i = 0; i < obj->max_bufsize; i++)
    buffer[i] = 0x55 ^ (-i);

  /* Adjust number of loop iterations up to timer accuracy.  */
  loop_iterations = adjust_loop_iterations_to_timer_accuracy (obj, buffer,
							      measurement_raw);

  /* Perform measurements */
  for (midx = 0, cur_bufsize = obj->min_bufsize;
       cur_bufsize <= obj->max_bufsize; cur_bufsize += obj->step_size, midx++)
    {
      measurements[midx] =
	do_bench_obj_measurement (obj, buffer, cur_bufsize, measurement_raw,
				  loop_iterations);
      measurements[midx] /= loop_iterations;
    }

  assert (midx == num_measurements);

  slope =
    get_slope (&get_bench_obj_point_x, obj, measurements, num_measurements,
	       &overhead);

  free (measurement_raw);
  free (measurements);
  free (real_buffer);
  obj->ops->finalize (obj);

  return slope;

err_free:
  if (measurement_raw)
    free (measurement_raw);
  if (measurements)
    free (measurements);
  if (real_buffer)
    free (real_buffer);
  obj->ops->finalize (obj);

  return -1;
}

/********************************************* CPU frequency auto-detection. */

static int
auto_ghz_init (struct bench_obj *obj)
{
  obj->min_bufsize = 16;
  obj->max_bufsize = 64 + obj->min_bufsize;
  obj->step_size = 8;
  obj->num_measure_repetitions = 16;

  return 0;
}

static void
auto_ghz_free (struct bench_obj *obj)
{
  (void)obj;
}

static void
auto_ghz_bench (struct bench_obj *obj, void *buf, size_t buflen)
{
  (void)obj;
  (void)buf;

  buflen *= 1024;

  /* Turbo frequency detection benchmark. Without CPU turbo-boost, this
   * function will give cycles/iteration result 1024.0 on high-end CPUs.
   * With turbo, result will be less and can be used detect turbo-clock. */

#ifdef HAVE_GCC_ASM_VOLATILE_MEMORY
  /* Auto-ghz operation takes two CPU cycles to perform. Memory barriers
   * are used to prevent compiler from optimizing this loop away. */
  #define AUTO_GHZ_OPERATION \
	asm volatile ("":"+r"(buflen)::"memory"); \
	buflen ^= 1; \
	asm volatile ("":"+r"(buflen)::"memory"); \
	buflen -= 2
#else
  /* TODO: Needs alternative way of preventing compiler optimizations.
   *       Mix of XOR and subtraction appears to do the trick for now. */
  #define AUTO_GHZ_OPERATION \
	buflen ^= 1; \
	buflen -= 2
#endif

#define AUTO_GHZ_OPERATION_2 \
	AUTO_GHZ_OPERATION; \
	AUTO_GHZ_OPERATION

#define AUTO_GHZ_OPERATION_4 \
	AUTO_GHZ_OPERATION_2; \
	AUTO_GHZ_OPERATION_2

#define AUTO_GHZ_OPERATION_8 \
	AUTO_GHZ_OPERATION_4; \
	AUTO_GHZ_OPERATION_4

#define AUTO_GHZ_OPERATION_16 \
	AUTO_GHZ_OPERATION_8; \
	AUTO_GHZ_OPERATION_8

#define AUTO_GHZ_OPERATION_32 \
	AUTO_GHZ_OPERATION_16; \
	AUTO_GHZ_OPERATION_16

#define AUTO_GHZ_OPERATION_64 \
	AUTO_GHZ_OPERATION_32; \
	AUTO_GHZ_OPERATION_32

#define AUTO_GHZ_OPERATION_128 \
	AUTO_GHZ_OPERATION_64; \
	AUTO_GHZ_OPERATION_64

  do
    {
      /* 1024 auto-ghz operations per loop, total 2048 instructions. */
      AUTO_GHZ_OPERATION_128; AUTO_GHZ_OPERATION_128;
      AUTO_GHZ_OPERATION_128; AUTO_GHZ_OPERATION_128;
      AUTO_GHZ_OPERATION_128; AUTO_GHZ_OPERATION_128;
      AUTO_GHZ_OPERATION_128; AUTO_GHZ_OPERATION_128;
    }
  while (buflen);
}

static struct bench_ops auto_ghz_detect_ops = {
  &auto_ghz_init,
  &auto_ghz_free,
  &auto_ghz_bench
};


double
get_auto_ghz (void)
{
  struct bench_obj obj = { 0 };
  double nsecs_per_iteration;
  double cycles_per_iteration;

  obj.ops = &auto_ghz_detect_ops;

  nsecs_per_iteration = slope_benchmark (&obj);

  cycles_per_iteration = nsecs_per_iteration * cpu_ghz;

  /* Adjust CPU Ghz so that cycles per iteration would give '1024.0'. */

  return safe_div(cpu_ghz * 1024, cycles_per_iteration);
}


double
do_slope_benchmark (struct bench_obj *obj)
{
  unsigned int try_count = 0;
  double ret;

  if (!auto_ghz)
    {
      /* Perform measurement without autodetection of CPU frequency. */

      do
        {
	  ret = slope_benchmark (obj);
        }
      while (ret <= 0 && try_count++ <= 4);

      bench_ghz = cpu_ghz;
      bench_ghz_diff = 0;
    }
  else
    {
      double target_diff = AUTO_GHZ_TARGET_DIFF;
      double cpu_auto_ghz_before;
      double cpu_auto_ghz_after;
      double nsecs_per_iteration;
      double diff;

      /* Perform measurement with CPU frequency autodetection. */

      do
        {
          /* Repeat measurement until CPU turbo frequency has stabilized. */

	  if ((++try_count % 4) == 0)
	    {
	      /* Too much frequency instability on the system, relax target
	       * accuracy. */
	      target_diff *= 2;
	    }

          cpu_auto_ghz_before = get_auto_ghz ();

          nsecs_per_iteration = slope_benchmark (obj);

          cpu_auto_ghz_after = get_auto_ghz ();

          diff = 1.0 - safe_div(cpu_auto_ghz_before, cpu_auto_ghz_after);
          diff = diff < 0 ? -diff : diff;
        }
      while ((nsecs_per_iteration <= 0 || diff > target_diff)
	     && try_count < 1000);

      ret = nsecs_per_iteration;

      bench_ghz = (cpu_auto_ghz_before + cpu_auto_ghz_after) / 2;
      bench_ghz_diff = diff;
    }

  return ret;
}


/********************************************************** Printing results. */

static void
double_to_str (char *out, size_t outlen, double value)
{
  const char *fmt;

  if (value < 1.0)
    fmt = "%.3f";
  else if (value < 100.0)
    fmt = "%.2f";
  else if (value < 1000.0)
    fmt = "%.1f";
  else
    fmt = "%.0f";

  snprintf (out, outlen, fmt, value);
}

static void
bench_print_result_csv (double nsecs_per_byte)
{
  double cycles_per_byte, mbytes_per_sec;
  char nsecpbyte_buf[16];
  char mbpsec_buf[16];
  char cpbyte_buf[16];
  char mhz_buf[16];
  char mhz_diff_buf[32];

  strcpy (mhz_diff_buf, "");
  *cpbyte_buf = 0;
  *mhz_buf = 0;

  double_to_str (nsecpbyte_buf, sizeof (nsecpbyte_buf), nsecs_per_byte);

  /* If user didn't provide CPU speed, we cannot show cycles/byte results.  */
  if (bench_ghz > 0.0)
    {
      cycles_per_byte = nsecs_per_byte * bench_ghz;
      double_to_str (cpbyte_buf, sizeof (cpbyte_buf), cycles_per_byte);
      double_to_str (mhz_buf, sizeof (mhz_buf), bench_ghz * 1000);
      if (auto_ghz && bench_ghz_diff * 1000 >= 1)
	{
	  snprintf(mhz_diff_buf, sizeof(mhz_diff_buf), ",%.0f,Mhz-diff",
		   bench_ghz_diff * 1000);
	}
    }

  mbytes_per_sec =
      safe_div(1000.0 * 1000.0 * 1000.0, nsecs_per_byte * 1024 * 1024);
  double_to_str (mbpsec_buf, sizeof (mbpsec_buf), mbytes_per_sec);

  /* We print two empty fields to allow for future enhancements.  */
  if (auto_ghz)
    {
      printf ("%s,%s,%s,,,%s,ns/B,%s,MiB/s,%s,c/B,%s,Mhz%s\n",
              current_section_name,
              current_algo_name? current_algo_name : "",
              current_mode_name? current_mode_name : "",
              nsecpbyte_buf,
              mbpsec_buf,
              cpbyte_buf,
              mhz_buf,
              mhz_diff_buf);
    }
  else
    {
      printf ("%s,%s,%s,,,%s,ns/B,%s,MiB/s,%s,c/B\n",
              current_section_name,
              current_algo_name? current_algo_name : "",
              current_mode_name? current_mode_name : "",
              nsecpbyte_buf,
              mbpsec_buf,
              cpbyte_buf);
    }
}

static void
bench_print_result_std (double nsecs_per_byte)
{
  double cycles_per_byte, mbytes_per_sec;
  char nsecpbyte_buf[16];
  char mbpsec_buf[16];
  char cpbyte_buf[16];
  char mhz_buf[16];
  char mhz_diff_buf[32];

  strcpy (mhz_diff_buf, "");

  double_to_str (nsecpbyte_buf, sizeof (nsecpbyte_buf), nsecs_per_byte);

  /* If user didn't provide CPU speed, we cannot show cycles/byte results.  */
  if (bench_ghz > 0.0)
    {
      cycles_per_byte = nsecs_per_byte * bench_ghz;
      double_to_str (cpbyte_buf, sizeof (cpbyte_buf), cycles_per_byte);
      double_to_str (mhz_buf, sizeof (mhz_buf), bench_ghz * 1000);
      if (auto_ghz && bench_ghz_diff * 1000 >= 0.5)
	{
	  snprintf(mhz_diff_buf, sizeof(mhz_diff_buf), "±%.0f",
		   bench_ghz_diff * 1000);
	}
    }
  else
    {
      strcpy (cpbyte_buf, "-");
      strcpy (mhz_buf, "-");
    }

  mbytes_per_sec =
      safe_div(1000.0 * 1000.0 * 1000.0, nsecs_per_byte * 1024 * 1024);
  double_to_str (mbpsec_buf, sizeof (mbpsec_buf), mbytes_per_sec);

  if (auto_ghz)
    {
      printf ("%9s ns/B %9s MiB/s %9s c/B %9s%s\n",
              nsecpbyte_buf, mbpsec_buf, cpbyte_buf, mhz_buf, mhz_diff_buf);
    }
  else
    {
      printf ("%9s ns/B %9s MiB/s %9s c/B\n",
              nsecpbyte_buf, mbpsec_buf, cpbyte_buf);
    }
}

static void
bench_print_result (double nsecs_per_byte)
{
  if (csv_mode)
    bench_print_result_csv (nsecs_per_byte);
  else
    bench_print_result_std (nsecs_per_byte);
}

static void
bench_print_result_nsec_per_iteration (double nsecs_per_iteration)
{
  double cycles_per_iteration;
  char nsecpiter_buf[16];
  char cpiter_buf[16];
  char mhz_buf[16];

  strcpy(cpiter_buf, csv_mode ? "" : "-");
  strcpy(mhz_buf, csv_mode ? "" : "-");

  double_to_str (nsecpiter_buf, sizeof (nsecpiter_buf), nsecs_per_iteration);

  /* If user didn't provide CPU speed, we cannot show cycles/iter results.  */
  if (bench_ghz > 0.0)
    {
      cycles_per_iteration = nsecs_per_iteration * bench_ghz;
      double_to_str (cpiter_buf, sizeof (cpiter_buf), cycles_per_iteration);
      double_to_str (mhz_buf, sizeof (mhz_buf), bench_ghz * 1000);
    }

  if (csv_mode)
    {
      if (auto_ghz)
        printf ("%s,%s,%s,,,,,,,,,%s,ns/iter,%s,c/iter,%s,Mhz\n",
                current_section_name,
                current_algo_name ? current_algo_name : "",
                current_mode_name ? current_mode_name : "",
                nsecpiter_buf,
                cpiter_buf,
                mhz_buf);
      else
        printf ("%s,%s,%s,,,,,,,,,%s,ns/iter,%s,c/iter\n",
                current_section_name,
                current_algo_name ? current_algo_name : "",
                current_mode_name ? current_mode_name : "",
                nsecpiter_buf,
                cpiter_buf);
    }
  else
    {
      if (auto_ghz)
        printf ("%14s %13s %9s\n", nsecpiter_buf, cpiter_buf, mhz_buf);
      else
        printf ("%14s %13s\n", nsecpiter_buf, cpiter_buf);
    }
}

static void
bench_print_section (const char *section_name, const char *print_name)
{
  if (csv_mode)
    {
      gcry_free (current_section_name);
      current_section_name = gcry_xstrdup (section_name);
    }
  else
    printf ("%s:\n", print_name);
}

static void
bench_print_header (int algo_width, const char *algo_name)
{
  if (csv_mode)
    {
      gcry_free (current_algo_name);
      current_algo_name = gcry_xstrdup (algo_name);
    }
  else
    {
      if (algo_width < 0)
        printf (" %-*s | ", -algo_width, algo_name);
      else
        printf (" %-*s | ", algo_width, algo_name);

      if (auto_ghz)
        printf ("%14s %15s %13s %9s\n", "nanosecs/byte", "mebibytes/sec",
                "cycles/byte", "auto Mhz");
      else
        printf ("%14s %15s %13s\n", "nanosecs/byte", "mebibytes/sec",
                "cycles/byte");
    }
}

static void
bench_print_header_nsec_per_iteration (int algo_width, const char *algo_name)
{
  if (csv_mode)
    {
      gcry_free (current_algo_name);
      current_algo_name = gcry_xstrdup (algo_name);
    }
  else
    {
      if (algo_width < 0)
        printf (" %-*s | ", -algo_width, algo_name);
      else
        printf (" %-*s | ", algo_width, algo_name);

      if (auto_ghz)
        printf ("%14s %13s %9s\n", "nanosecs/iter", "cycles/iter", "auto Mhz");
      else
        printf ("%14s %13s\n", "nanosecs/iter", "cycles/iter");
    }
}

static void
bench_print_algo (int algo_width, const char *algo_name)
{
  if (csv_mode)
    {
      gcry_free (current_algo_name);
      current_algo_name = gcry_xstrdup (algo_name);
    }
  else
    {
      if (algo_width < 0)
        printf (" %-*s | ", -algo_width, algo_name);
      else
        printf (" %-*s | ", algo_width, algo_name);
    }
}

static void
bench_print_mode (int width, const char *mode_name)
{
  if (csv_mode)
    {
      gcry_free (current_mode_name);
      current_mode_name = gcry_xstrdup (mode_name);
    }
  else
    {
      if (width < 0)
        printf (" %-*s | ", -width, mode_name);
      else
        printf (" %*s | ", width, mode_name);
      fflush (stdout);
    }
}

static void
bench_print_footer (int algo_width)
{
  if (!csv_mode)
    printf (" %-*s =\n", algo_width, "");
}


/********************************************************* Cipher benchmarks. */

struct bench_cipher_mode
{
  int mode;
  const char *name;
  struct bench_ops *ops;

  int algo;
};


static int
bench_encrypt_init (struct bench_obj *obj)
{
  struct bench_cipher_mode *mode = obj->priv;
  gcry_cipher_hd_t hd;
  int err, keylen;

  obj->min_bufsize = BUF_START_SIZE;
  obj->max_bufsize = BUF_END_SIZE;
  obj->step_size = BUF_STEP_SIZE;
  obj->num_measure_repetitions = num_measurement_repetitions;

  err = gcry_cipher_open (&hd, mode->algo, mode->mode, 0);
  if (err)
    {
      fprintf (stderr, PGM ": error opening cipher `%s'\n",
	       gcry_cipher_algo_name (mode->algo));
      exit (1);
    }

  keylen = gcry_cipher_get_algo_keylen (mode->algo);
  if (mode->mode == GCRY_CIPHER_MODE_SIV)
    {
      keylen *= 2;
    }

  if (keylen)
    {
      char key[keylen];
      int i;

      for (i = 0; i < keylen; i++)
	key[i] = 0x33 ^ (11 - i);

      err = gcry_cipher_setkey (hd, key, keylen);
      if (err)
	{
	  fprintf (stderr, PGM ": gcry_cipher_setkey failed: %s\n",
		   gpg_strerror (err));
	  gcry_cipher_close (hd);
	  exit (1);
	}
    }
  else
    {
      fprintf (stderr, PGM ": failed to get key length for algorithm `%s'\n",
	       gcry_cipher_algo_name (mode->algo));
      gcry_cipher_close (hd);
      exit (1);
    }

  obj->hd = hd;

  return 0;
}

static void
bench_encrypt_free (struct bench_obj *obj)
{
  gcry_cipher_hd_t hd = obj->hd;

  gcry_cipher_close (hd);
}

static void
bench_encrypt_do_bench (struct bench_obj *obj, void *buf, size_t buflen)
{
  gcry_cipher_hd_t hd = obj->hd;
  int err;

  err = gcry_cipher_reset (hd);
  if (!err)
    err = gcry_cipher_encrypt (hd, buf, buflen, buf, buflen);
  if (err)
    {
      fprintf (stderr, PGM ": gcry_cipher_encrypt failed: %s\n",
	       gpg_strerror (err));
      gcry_cipher_close (hd);
      exit (1);
    }
}

static void
bench_decrypt_do_bench (struct bench_obj *obj, void *buf, size_t buflen)
{
  gcry_cipher_hd_t hd = obj->hd;
  int err;

  err = gcry_cipher_reset (hd);
  if (!err)
    err = gcry_cipher_decrypt (hd, buf, buflen, buf, buflen);
  if (err)
    {
      fprintf (stderr, PGM ": gcry_cipher_encrypt failed: %s\n",
	       gpg_strerror (err));
      gcry_cipher_close (hd);
      exit (1);
    }
}

static struct bench_ops encrypt_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_encrypt_do_bench
};

static struct bench_ops decrypt_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_decrypt_do_bench
};


static int
bench_xts_encrypt_init (struct bench_obj *obj)
{
  struct bench_cipher_mode *mode = obj->priv;
  gcry_cipher_hd_t hd;
  int err, keylen;

  obj->min_bufsize = BUF_START_SIZE;
  obj->max_bufsize = BUF_END_SIZE;
  obj->step_size = BUF_STEP_SIZE;
  obj->num_measure_repetitions = num_measurement_repetitions;

  err = gcry_cipher_open (&hd, mode->algo, mode->mode, 0);
  if (err)
    {
      fprintf (stderr, PGM ": error opening cipher `%s'\n",
	       gcry_cipher_algo_name (mode->algo));
      exit (1);
    }

  /* Double key-length for XTS. */
  keylen = gcry_cipher_get_algo_keylen (mode->algo) * 2;
  if (keylen)
    {
      char key[keylen];
      int i;

      for (i = 0; i < keylen; i++)
	key[i] = 0x33 ^ (11 - i);

      err = gcry_cipher_setkey (hd, key, keylen);
      if (err)
	{
	  fprintf (stderr, PGM ": gcry_cipher_setkey failed: %s\n",
		   gpg_strerror (err));
	  gcry_cipher_close (hd);
	  exit (1);
	}
    }
  else
    {
      fprintf (stderr, PGM ": failed to get key length for algorithm `%s'\n",
	       gcry_cipher_algo_name (mode->algo));
      gcry_cipher_close (hd);
      exit (1);
    }

  obj->hd = hd;

  return 0;
}

static struct bench_ops xts_encrypt_ops = {
  &bench_xts_encrypt_init,
  &bench_encrypt_free,
  &bench_encrypt_do_bench
};

static struct bench_ops xts_decrypt_ops = {
  &bench_xts_encrypt_init,
  &bench_encrypt_free,
  &bench_decrypt_do_bench
};


static void
bench_ccm_encrypt_do_bench (struct bench_obj *obj, void *buf, size_t buflen)
{
  gcry_cipher_hd_t hd = obj->hd;
  int err;
  char tag[8];
  char nonce[11] = { 0x80, 0x01, };
  u64 params[3];

  gcry_cipher_setiv (hd, nonce, sizeof (nonce));

  /* Set CCM lengths */
  params[0] = buflen;
  params[1] = 0;		/*aadlen */
  params[2] = sizeof (tag);
  err =
    gcry_cipher_ctl (hd, GCRYCTL_SET_CCM_LENGTHS, params, sizeof (params));
  if (err)
    {
      fprintf (stderr, PGM ": gcry_cipher_ctl failed: %s\n",
	       gpg_strerror (err));
      gcry_cipher_close (hd);
      exit (1);
    }

  err = gcry_cipher_encrypt (hd, buf, buflen, buf, buflen);
  if (err)
    {
      fprintf (stderr, PGM ": gcry_cipher_encrypt failed: %s\n",
	       gpg_strerror (err));
      gcry_cipher_close (hd);
      exit (1);
    }

  err = gcry_cipher_gettag (hd, tag, sizeof (tag));
  if (err)
    {
      fprintf (stderr, PGM ": gcry_cipher_gettag failed: %s\n",
	       gpg_strerror (err));
      gcry_cipher_close (hd);
      exit (1);
    }
}

static void
bench_ccm_decrypt_do_bench (struct bench_obj *obj, void *buf, size_t buflen)
{
  gcry_cipher_hd_t hd = obj->hd;
  int err;
  char tag[8] = { 0, };
  char nonce[11] = { 0x80, 0x01, };
  u64 params[3];

  gcry_cipher_setiv (hd, nonce, sizeof (nonce));

  /* Set CCM lengths */
  params[0] = buflen;
  params[1] = 0;		/*aadlen */
  params[2] = sizeof (tag);
  err =
    gcry_cipher_ctl (hd, GCRYCTL_SET_CCM_LENGTHS, params, sizeof (params));
  if (err)
    {
      fprintf (stderr, PGM ": gcry_cipher_ctl failed: %s\n",
	       gpg_strerror (err));
      gcry_cipher_close (hd);
      exit (1);
    }

  err = gcry_cipher_decrypt (hd, buf, buflen, buf, buflen);
  if (err)
    {
      fprintf (stderr, PGM ": gcry_cipher_encrypt failed: %s\n",
	       gpg_strerror (err));
      gcry_cipher_close (hd);
      exit (1);
    }

  err = gcry_cipher_checktag (hd, tag, sizeof (tag));
  if (gpg_err_code (err) == GPG_ERR_CHECKSUM)
    err = gpg_error (GPG_ERR_NO_ERROR);
  if (err)
    {
      fprintf (stderr, PGM ": gcry_cipher_gettag failed: %s\n",
	       gpg_strerror (err));
      gcry_cipher_close (hd);
      exit (1);
    }
}

static void
bench_ccm_authenticate_do_bench (struct bench_obj *obj, void *buf,
				 size_t buflen)
{
  gcry_cipher_hd_t hd = obj->hd;
  int err;
  char tag[8] = { 0, };
  char nonce[11] = { 0x80, 0x01, };
  u64 params[3];
  char data = 0xff;

  gcry_cipher_setiv (hd, nonce, sizeof (nonce));

  /* Set CCM lengths */
  params[0] = sizeof (data);	/*datalen */
  params[1] = buflen;		/*aadlen */
  params[2] = sizeof (tag);
  err =
    gcry_cipher_ctl (hd, GCRYCTL_SET_CCM_LENGTHS, params, sizeof (params));
  if (err)
    {
      fprintf (stderr, PGM ": gcry_cipher_ctl failed: %s\n",
	       gpg_strerror (err));
      gcry_cipher_close (hd);
      exit (1);
    }

  err = gcry_cipher_authenticate (hd, buf, buflen);
  if (err)
    {
      fprintf (stderr, PGM ": gcry_cipher_authenticate failed: %s\n",
	       gpg_strerror (err));
      gcry_cipher_close (hd);
      exit (1);
    }

  err = gcry_cipher_encrypt (hd, &data, sizeof (data), &data, sizeof (data));
  if (err)
    {
      fprintf (stderr, PGM ": gcry_cipher_encrypt failed: %s\n",
	       gpg_strerror (err));
      gcry_cipher_close (hd);
      exit (1);
    }

  err = gcry_cipher_gettag (hd, tag, sizeof (tag));
  if (err)
    {
      fprintf (stderr, PGM ": gcry_cipher_gettag failed: %s\n",
	       gpg_strerror (err));
      gcry_cipher_close (hd);
      exit (1);
    }
}

static struct bench_ops ccm_encrypt_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_ccm_encrypt_do_bench
};

static struct bench_ops ccm_decrypt_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_ccm_decrypt_do_bench
};

static struct bench_ops ccm_authenticate_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_ccm_authenticate_do_bench
};


static void
bench_aead_encrypt_do_bench (struct bench_obj *obj, void *buf, size_t buflen,
			     const char *nonce, size_t noncelen)
{
  gcry_cipher_hd_t hd = obj->hd;
  int err;
  char tag[16];

  gcry_cipher_reset (hd);
  gcry_cipher_setiv (hd, nonce, noncelen);

  gcry_cipher_final (hd);
  err = gcry_cipher_encrypt (hd, buf, buflen, buf, buflen);
  if (err)
    {
      fprintf (stderr, PGM ": gcry_cipher_encrypt failed: %s\n",
           gpg_strerror (err));
      gcry_cipher_close (hd);
      exit (1);
    }

  err = gcry_cipher_gettag (hd, tag, sizeof (tag));
  if (err)
    {
      fprintf (stderr, PGM ": gcry_cipher_gettag failed: %s\n",
           gpg_strerror (err));
      gcry_cipher_close (hd);
      exit (1);
    }
}

static void
bench_aead_decrypt_do_bench (struct bench_obj *obj, void *buf, size_t buflen,
			     const char *nonce, size_t noncelen)
{
  gcry_cipher_hd_t hd = obj->hd;
  int err;
  char tag[16] = { 0, };

  gcry_cipher_reset (hd);
  gcry_cipher_set_decryption_tag (hd, tag, 16);

  gcry_cipher_setiv (hd, nonce, noncelen);

  gcry_cipher_final (hd);
  err = gcry_cipher_decrypt (hd, buf, buflen, buf, buflen);
  if (gpg_err_code (err) == GPG_ERR_CHECKSUM)
    err = gpg_error (GPG_ERR_NO_ERROR);
  if (err)
    {
      fprintf (stderr, PGM ": gcry_cipher_decrypt failed: %s\n",
           gpg_strerror (err));
      gcry_cipher_close (hd);
      exit (1);
    }

  err = gcry_cipher_checktag (hd, tag, sizeof (tag));
  if (gpg_err_code (err) == GPG_ERR_CHECKSUM)
    err = gpg_error (GPG_ERR_NO_ERROR);
  if (err)
    {
      fprintf (stderr, PGM ": gcry_cipher_gettag failed: %s\n",
           gpg_strerror (err));
      gcry_cipher_close (hd);
      exit (1);
    }
}

static void
bench_aead_authenticate_do_bench (struct bench_obj *obj, void *buf,
				  size_t buflen, const char *nonce,
				  size_t noncelen)
{
  gcry_cipher_hd_t hd = obj->hd;
  int err;
  char tag[16] = { 0, };
  char data = 0xff;

  gcry_cipher_reset (hd);

  if (noncelen > 0)
    {
      err = gcry_cipher_setiv (hd, nonce, noncelen);
      if (err)
	{
	  fprintf (stderr, PGM ": gcry_cipher_setiv failed: %s\n",
	       gpg_strerror (err));
	  gcry_cipher_close (hd);
	  exit (1);
	}
    }

  err = gcry_cipher_authenticate (hd, buf, buflen);
  if (err)
    {
      fprintf (stderr, PGM ": gcry_cipher_authenticate failed: %s\n",
           gpg_strerror (err));
      gcry_cipher_close (hd);
      exit (1);
    }

  gcry_cipher_final (hd);
  err = gcry_cipher_encrypt (hd, &data, sizeof (data), &data, sizeof (data));
  if (err)
    {
      fprintf (stderr, PGM ": gcry_cipher_encrypt failed: %s\n",
           gpg_strerror (err));
      gcry_cipher_close (hd);
      exit (1);
    }

  err = gcry_cipher_gettag (hd, tag, sizeof (tag));
  if (err)
    {
      fprintf (stderr, PGM ": gcry_cipher_gettag failed: %s\n",
           gpg_strerror (err));
      gcry_cipher_close (hd);
      exit (1);
    }
}


static void
bench_gcm_encrypt_do_bench (struct bench_obj *obj, void *buf,
			    size_t buflen)
{
  char nonce[12] = { 0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce,
                     0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88 };
  bench_aead_encrypt_do_bench (obj, buf, buflen, nonce, sizeof(nonce));
}

static void
bench_gcm_decrypt_do_bench (struct bench_obj *obj, void *buf,
			    size_t buflen)
{
  char nonce[12] = { 0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce,
                     0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88 };
  bench_aead_decrypt_do_bench (obj, buf, buflen, nonce, sizeof(nonce));
}

static void
bench_gcm_authenticate_do_bench (struct bench_obj *obj, void *buf,
				 size_t buflen)
{
  char nonce[12] = { 0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce,
                     0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88 };
  bench_aead_authenticate_do_bench (obj, buf, buflen, nonce, sizeof(nonce));
}

static struct bench_ops gcm_encrypt_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_gcm_encrypt_do_bench
};

static struct bench_ops gcm_decrypt_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_gcm_decrypt_do_bench
};

static struct bench_ops gcm_authenticate_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_gcm_authenticate_do_bench
};


static void
bench_ocb_encrypt_do_bench (struct bench_obj *obj, void *buf,
			    size_t buflen)
{
  char nonce[15] = { 0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce,
                     0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88,
                     0x00, 0x00, 0x01 };
  bench_aead_encrypt_do_bench (obj, buf, buflen, nonce, sizeof(nonce));
}

static void
bench_ocb_decrypt_do_bench (struct bench_obj *obj, void *buf,
			    size_t buflen)
{
  char nonce[15] = { 0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce,
                     0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88,
                     0x00, 0x00, 0x01 };
  bench_aead_decrypt_do_bench (obj, buf, buflen, nonce, sizeof(nonce));
}

static void
bench_ocb_authenticate_do_bench (struct bench_obj *obj, void *buf,
				 size_t buflen)
{
  char nonce[15] = { 0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce,
                     0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88,
                     0x00, 0x00, 0x01 };
  bench_aead_authenticate_do_bench (obj, buf, buflen, nonce, sizeof(nonce));
}

static struct bench_ops ocb_encrypt_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_ocb_encrypt_do_bench
};

static struct bench_ops ocb_decrypt_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_ocb_decrypt_do_bench
};

static struct bench_ops ocb_authenticate_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_ocb_authenticate_do_bench
};


static void
bench_siv_encrypt_do_bench (struct bench_obj *obj, void *buf,
			    size_t buflen)
{
  bench_aead_encrypt_do_bench (obj, buf, buflen, NULL, 0);
}

static void
bench_siv_decrypt_do_bench (struct bench_obj *obj, void *buf,
			    size_t buflen)
{
  bench_aead_decrypt_do_bench (obj, buf, buflen, NULL, 0);
}

static void
bench_siv_authenticate_do_bench (struct bench_obj *obj, void *buf,
				 size_t buflen)
{
  bench_aead_authenticate_do_bench (obj, buf, buflen, NULL, 0);
}

static struct bench_ops siv_encrypt_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_siv_encrypt_do_bench
};

static struct bench_ops siv_decrypt_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_siv_decrypt_do_bench
};

static struct bench_ops siv_authenticate_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_siv_authenticate_do_bench
};


static void
bench_gcm_siv_encrypt_do_bench (struct bench_obj *obj, void *buf,
				size_t buflen)
{
  char nonce[12] = { 0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce,
                     0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88 };
  bench_aead_encrypt_do_bench (obj, buf, buflen, nonce, sizeof(nonce));
}

static void
bench_gcm_siv_decrypt_do_bench (struct bench_obj *obj, void *buf,
				size_t buflen)
{
  char nonce[12] = { 0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce,
                     0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88 };
  bench_aead_decrypt_do_bench (obj, buf, buflen, nonce, sizeof(nonce));
}

static void
bench_gcm_siv_authenticate_do_bench (struct bench_obj *obj, void *buf,
				     size_t buflen)
{
  char nonce[12] = { 0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce,
                     0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88 };
  bench_aead_authenticate_do_bench (obj, buf, buflen, nonce, sizeof(nonce));
}

static struct bench_ops gcm_siv_encrypt_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_gcm_siv_encrypt_do_bench
};

static struct bench_ops gcm_siv_decrypt_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_gcm_siv_decrypt_do_bench
};

static struct bench_ops gcm_siv_authenticate_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_gcm_siv_authenticate_do_bench
};


static void
bench_eax_encrypt_do_bench (struct bench_obj *obj, void *buf,
			    size_t buflen)
{
  char nonce[16] = { 0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce,
                     0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88,
                     0x00, 0x00, 0x01, 0x00 };
  bench_aead_encrypt_do_bench (obj, buf, buflen, nonce, sizeof(nonce));
}

static void
bench_eax_decrypt_do_bench (struct bench_obj *obj, void *buf,
			    size_t buflen)
{
  char nonce[16] = { 0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce,
                     0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88,
                     0x00, 0x00, 0x01, 0x00 };
  bench_aead_decrypt_do_bench (obj, buf, buflen, nonce, sizeof(nonce));
}

static void
bench_eax_authenticate_do_bench (struct bench_obj *obj, void *buf,
				 size_t buflen)
{
  char nonce[16] = { 0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce,
                     0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88,
                     0x00, 0x00, 0x01, 0x00 };
  bench_aead_authenticate_do_bench (obj, buf, buflen, nonce, sizeof(nonce));
}

static struct bench_ops eax_encrypt_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_eax_encrypt_do_bench
};

static struct bench_ops eax_decrypt_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_eax_decrypt_do_bench
};

static struct bench_ops eax_authenticate_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_eax_authenticate_do_bench
};

static void
bench_poly1305_encrypt_do_bench (struct bench_obj *obj, void *buf,
				 size_t buflen)
{
  char nonce[8] = { 0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad };
  bench_aead_encrypt_do_bench (obj, buf, buflen, nonce, sizeof(nonce));
}

static void
bench_poly1305_decrypt_do_bench (struct bench_obj *obj, void *buf,
				 size_t buflen)
{
  char nonce[8] = { 0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad };
  bench_aead_decrypt_do_bench (obj, buf, buflen, nonce, sizeof(nonce));
}

static void
bench_poly1305_authenticate_do_bench (struct bench_obj *obj, void *buf,
				      size_t buflen)
{
  char nonce[8] = { 0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad };
  bench_aead_authenticate_do_bench (obj, buf, buflen, nonce, sizeof(nonce));
}

static struct bench_ops poly1305_encrypt_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_poly1305_encrypt_do_bench
};

static struct bench_ops poly1305_decrypt_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_poly1305_decrypt_do_bench
};

static struct bench_ops poly1305_authenticate_ops = {
  &bench_encrypt_init,
  &bench_encrypt_free,
  &bench_poly1305_authenticate_do_bench
};


static struct bench_cipher_mode cipher_modes[] = {
  {GCRY_CIPHER_MODE_ECB, "ECB enc", &encrypt_ops},
  {GCRY_CIPHER_MODE_ECB, "ECB dec", &decrypt_ops},
  {GCRY_CIPHER_MODE_CBC, "CBC enc", &encrypt_ops},
  {GCRY_CIPHER_MODE_CBC, "CBC dec", &decrypt_ops},
  {GCRY_CIPHER_MODE_CFB, "CFB enc", &encrypt_ops},
  {GCRY_CIPHER_MODE_CFB, "CFB dec", &decrypt_ops},
  {GCRY_CIPHER_MODE_OFB, "OFB enc", &encrypt_ops},
  {GCRY_CIPHER_MODE_OFB, "OFB dec", &decrypt_ops},
  {GCRY_CIPHER_MODE_CTR, "CTR enc", &encrypt_ops},
  {GCRY_CIPHER_MODE_CTR, "CTR dec", &decrypt_ops},
  {GCRY_CIPHER_MODE_XTS, "XTS enc", &xts_encrypt_ops},
  {GCRY_CIPHER_MODE_XTS, "XTS dec", &xts_decrypt_ops},
  {GCRY_CIPHER_MODE_CCM, "CCM enc", &ccm_encrypt_ops},
  {GCRY_CIPHER_MODE_CCM, "CCM dec", &ccm_decrypt_ops},
  {GCRY_CIPHER_MODE_CCM, "CCM auth", &ccm_authenticate_ops},
  {GCRY_CIPHER_MODE_EAX, "EAX enc",  &eax_encrypt_ops},
  {GCRY_CIPHER_MODE_EAX, "EAX dec",  &eax_decrypt_ops},
  {GCRY_CIPHER_MODE_EAX, "EAX auth", &eax_authenticate_ops},
  {GCRY_CIPHER_MODE_GCM, "GCM enc", &gcm_encrypt_ops},
  {GCRY_CIPHER_MODE_GCM, "GCM dec", &gcm_decrypt_ops},
  {GCRY_CIPHER_MODE_GCM, "GCM auth", &gcm_authenticate_ops},
  {GCRY_CIPHER_MODE_OCB, "OCB enc",  &ocb_encrypt_ops},
  {GCRY_CIPHER_MODE_OCB, "OCB dec",  &ocb_decrypt_ops},
  {GCRY_CIPHER_MODE_OCB, "OCB auth", &ocb_authenticate_ops},
  {GCRY_CIPHER_MODE_SIV, "SIV enc", &siv_encrypt_ops},
  {GCRY_CIPHER_MODE_SIV, "SIV dec", &siv_decrypt_ops},
  {GCRY_CIPHER_MODE_SIV, "SIV auth", &siv_authenticate_ops},
  {GCRY_CIPHER_MODE_GCM_SIV, "GCM-SIV enc", &gcm_siv_encrypt_ops},
  {GCRY_CIPHER_MODE_GCM_SIV, "GCM-SIV dec", &gcm_siv_decrypt_ops},
  {GCRY_CIPHER_MODE_GCM_SIV, "GCM-SIV auth", &gcm_siv_authenticate_ops},
  {GCRY_CIPHER_MODE_POLY1305, "POLY1305 enc", &poly1305_encrypt_ops},
  {GCRY_CIPHER_MODE_POLY1305, "POLY1305 dec", &poly1305_decrypt_ops},
  {GCRY_CIPHER_MODE_POLY1305, "POLY1305 auth", &poly1305_authenticate_ops},
  {0},
};


static void
cipher_bench_one (int algo, struct bench_cipher_mode *pmode)
{
  struct bench_cipher_mode mode = *pmode;
  struct bench_obj obj = { 0 };
  double result;
  unsigned int blklen;
  unsigned int keylen;

  mode.algo = algo;

  /* Check if this mode is ok */
  blklen = gcry_cipher_get_algo_blklen (algo);
  if (!blklen)
    return;

  keylen = gcry_cipher_get_algo_keylen (algo);
  if (!keylen)
    return;

  /* Stream cipher? Only test with "ECB" and POLY1305. */
  if (blklen == 1 && (mode.mode != GCRY_CIPHER_MODE_ECB &&
		      mode.mode != GCRY_CIPHER_MODE_POLY1305))
    return;
  if (blklen == 1 && mode.mode == GCRY_CIPHER_MODE_ECB)
    {
      mode.mode = GCRY_CIPHER_MODE_STREAM;
      mode.name = mode.ops == &encrypt_ops ? "STREAM enc" : "STREAM dec";
    }

  /* Poly1305 has restriction for cipher algorithm */
  if (mode.mode == GCRY_CIPHER_MODE_POLY1305 && algo != GCRY_CIPHER_CHACHA20)
    return;

  /* CCM has restrictions for block-size */
  if (mode.mode == GCRY_CIPHER_MODE_CCM && blklen != GCRY_CCM_BLOCK_LEN)
    return;

  /* GCM has restrictions for block-size; not allowed in FIPS mode */
  if (mode.mode == GCRY_CIPHER_MODE_GCM && (in_fips_mode || blklen != GCRY_GCM_BLOCK_LEN))
    return;

  /* XTS has restrictions for block-size */
  if (mode.mode == GCRY_CIPHER_MODE_XTS && blklen != GCRY_XTS_BLOCK_LEN)
    return;

  /* SIV has restrictions for block-size */
  if (mode.mode == GCRY_CIPHER_MODE_SIV && blklen != GCRY_SIV_BLOCK_LEN)
    return;

  /* GCM-SIV has restrictions for block-size */
  if (mode.mode == GCRY_CIPHER_MODE_GCM_SIV && blklen != GCRY_SIV_BLOCK_LEN)
    return;

  /* GCM-SIV has restrictions for key length */
  if (mode.mode == GCRY_CIPHER_MODE_GCM_SIV && !(keylen == 16 || keylen == 32))
    return;

  /* Our OCB implementation has restrictions for block-size.  */
  if (mode.mode == GCRY_CIPHER_MODE_OCB && blklen != GCRY_OCB_BLOCK_LEN)
    return;

  bench_print_mode (14, mode.name);

  obj.ops = mode.ops;
  obj.priv = &mode;

  result = do_slope_benchmark (&obj);

  bench_print_result (result);
}


static void
_cipher_bench (int algo)
{
  const char *algoname;
  int i;

  algoname = gcry_cipher_algo_name (algo);

  bench_print_header (14, algoname);

  for (i = 0; cipher_modes[i].mode; i++)
    cipher_bench_one (algo, &cipher_modes[i]);

  bench_print_footer (14);
}


void
cipher_bench (char **argv, int argc)
{
  int i, algo;

  bench_print_section ("cipher", "Cipher");

  if (argv && argc)
    {
      for (i = 0; i < argc; i++)
        {
          algo = gcry_cipher_map_name (argv[i]);
          if (algo)
            _cipher_bench (algo);
        }
    }
  else
    {
      for (i = 1; i < 400; i++)
        if (!gcry_cipher_test_algo (i))
          _cipher_bench (i);
    }
}


/*********************************************************** Hash benchmarks. */

struct bench_hash_mode
{
  const char *name;
  struct bench_ops *ops;

  int algo;
};


static int
bench_hash_init (struct bench_obj *obj)
{
  struct bench_hash_mode *mode = obj->priv;
  gcry_md_hd_t hd;
  int err;

  obj->min_bufsize = BUF_START_SIZE;
  obj->max_bufsize = BUF_END_SIZE;
  obj->step_size = BUF_STEP_SIZE;
  obj->num_measure_repetitions = num_measurement_repetitions;

  err = gcry_md_open (&hd, mode->algo, 0);
  if (err)
    {
      fprintf (stderr, PGM ": error opening hash `%s'\n",
	       gcry_md_algo_name (mode->algo));
      exit (1);
    }

  obj->hd = hd;

  return 0;
}

static void
bench_hash_free (struct bench_obj *obj)
{
  gcry_md_hd_t hd = obj->hd;

  gcry_md_close (hd);
}

static void
bench_hash_do_bench (struct bench_obj *obj, void *buf, size_t buflen)
{
  gcry_md_hd_t hd = obj->hd;

  gcry_md_reset (hd);
  gcry_md_write (hd, buf, buflen);
  gcry_md_final (hd);
}

static struct bench_ops hash_ops = {
  &bench_hash_init,
  &bench_hash_free,
  &bench_hash_do_bench
};


static struct bench_hash_mode hash_modes[] = {
  {"", &hash_ops},
  {0},
};


static void
hash_bench_one (int algo, struct bench_hash_mode *pmode)
{
  struct bench_hash_mode mode = *pmode;
  struct bench_obj obj = { 0 };
  double result;

  mode.algo = algo;

  if (mode.name[0] == '\0')
    bench_print_algo (-14, gcry_md_algo_name (algo));
  else
    bench_print_algo (14, mode.name);

  obj.ops = mode.ops;
  obj.priv = &mode;

  result = do_slope_benchmark (&obj);

  bench_print_result (result);
}

static void
_hash_bench (int algo)
{
  int i;

  for (i = 0; hash_modes[i].name; i++)
    hash_bench_one (algo, &hash_modes[i]);
}

void
hash_bench (char **argv, int argc)
{
  int i, algo;

  bench_print_section ("hash", "Hash");
  bench_print_header (14, "");

  if (argv && argc)
    {
      for (i = 0; i < argc; i++)
	{
	  algo = gcry_md_map_name (argv[i]);
	  if (algo)
	    _hash_bench (algo);
	}
    }
  else
    {
      for (i = 1; i < 400; i++)
	if (!gcry_md_test_algo (i))
	  _hash_bench (i);
    }

  bench_print_footer (14);
}


/************************************************************ MAC benchmarks. */

struct bench_mac_mode
{
  const char *name;
  struct bench_ops *ops;

  int algo;
};


static int
bench_mac_init (struct bench_obj *obj)
{
  struct bench_mac_mode *mode = obj->priv;
  gcry_mac_hd_t hd;
  int err;
  unsigned int keylen;
  void *key;

  obj->min_bufsize = BUF_START_SIZE;
  obj->max_bufsize = BUF_END_SIZE;
  obj->step_size = BUF_STEP_SIZE;
  obj->num_measure_repetitions = num_measurement_repetitions;

  keylen = gcry_mac_get_algo_keylen (mode->algo);
  if (keylen == 0)
    keylen = 32;
  key = malloc (keylen);
  if (!key)
    {
      fprintf (stderr, PGM ": couldn't allocate %d bytes\n", keylen);
      exit (1);
    }
  memset(key, 42, keylen);

  err = gcry_mac_open (&hd, mode->algo, 0, NULL);
  if (err)
    {
      fprintf (stderr, PGM ": error opening mac `%s'\n",
	       gcry_mac_algo_name (mode->algo));
      free (key);
      exit (1);
    }

  err = gcry_mac_setkey (hd, key, keylen);
  if (err)
    {
      fprintf (stderr, PGM ": error setting key for mac `%s'\n",
	       gcry_mac_algo_name (mode->algo));
      free (key);
      exit (1);
    }

  switch (mode->algo)
    {
    default:
      break;
    case GCRY_MAC_POLY1305_AES:
    case GCRY_MAC_POLY1305_CAMELLIA:
    case GCRY_MAC_POLY1305_TWOFISH:
    case GCRY_MAC_POLY1305_SERPENT:
    case GCRY_MAC_POLY1305_SEED:
      gcry_mac_setiv (hd, key, 16);
      break;
    }

  obj->hd = hd;

  free (key);
  return 0;
}

static void
bench_mac_free (struct bench_obj *obj)
{
  gcry_mac_hd_t hd = obj->hd;

  gcry_mac_close (hd);
}

static void
bench_mac_do_bench (struct bench_obj *obj, void *buf, size_t buflen)
{
  gcry_mac_hd_t hd = obj->hd;
  size_t bs;
  char b;

  gcry_mac_reset (hd);
  gcry_mac_write (hd, buf, buflen);
  bs = sizeof(b);
  gcry_mac_read (hd, &b, &bs);
}

static struct bench_ops mac_ops = {
  &bench_mac_init,
  &bench_mac_free,
  &bench_mac_do_bench
};


static struct bench_mac_mode mac_modes[] = {
  {"", &mac_ops},
  {0},
};


static void
mac_bench_one (int algo, struct bench_mac_mode *pmode)
{
  struct bench_mac_mode mode = *pmode;
  struct bench_obj obj = { 0 };
  double result;

  mode.algo = algo;

  if (mode.name[0] == '\0')
    bench_print_algo (-18, gcry_mac_algo_name (algo));
  else
    bench_print_algo (18, mode.name);

  obj.ops = mode.ops;
  obj.priv = &mode;

  result = do_slope_benchmark (&obj);

  bench_print_result (result);
}

static void
_mac_bench (int algo)
{
  int i;

  for (i = 0; mac_modes[i].name; i++)
    mac_bench_one (algo, &mac_modes[i]);
}

void
mac_bench (char **argv, int argc)
{
  int i, algo;

  bench_print_section ("mac", "MAC");
  bench_print_header (18, "");

  if (argv && argc)
    {
      for (i = 0; i < argc; i++)
	{
	  algo = gcry_mac_map_name (argv[i]);
	  if (algo)
	    _mac_bench (algo);
	}
    }
  else
    {
      for (i = 1; i < 600; i++)
	if (!gcry_mac_test_algo (i))
	  _mac_bench (i);
    }

  bench_print_footer (18);
}


/************************************************************ KDF benchmarks. */

struct bench_kdf_mode
{
  struct bench_ops *ops;

  int algo;
  int subalgo;
};


static int
bench_kdf_init (struct bench_obj *obj)
{
  struct bench_kdf_mode *mode = obj->priv;

  if (mode->algo == GCRY_KDF_PBKDF2)
    {
      obj->min_bufsize = 2;
      obj->max_bufsize = 2 * 32;
      obj->step_size = 2;
    }

  obj->num_measure_repetitions = num_measurement_repetitions;

  return 0;
}

static void
bench_kdf_free (struct bench_obj *obj)
{
  (void)obj;
}

static void
bench_kdf_do_bench (struct bench_obj *obj, void *buf, size_t buflen)
{
  struct bench_kdf_mode *mode = obj->priv;
  char keybuf[16];

  (void)buf;

  if (mode->algo == GCRY_KDF_PBKDF2)
    {
      gcry_kdf_derive("qwerty", 6, mode->algo, mode->subalgo, "01234567", 8,
		      buflen, sizeof(keybuf), keybuf);
    }
}

static struct bench_ops kdf_ops = {
  &bench_kdf_init,
  &bench_kdf_free,
  &bench_kdf_do_bench
};


static void
kdf_bench_one (int algo, int subalgo)
{
  struct bench_kdf_mode mode = { &kdf_ops };
  struct bench_obj obj = { 0 };
  double nsecs_per_iteration;
  char algo_name[32];

  mode.algo = algo;
  mode.subalgo = subalgo;

  switch (subalgo)
    {
    case GCRY_MD_CRC32:
    case GCRY_MD_CRC32_RFC1510:
    case GCRY_MD_CRC24_RFC2440:
    case GCRY_MD_MD4:
      /* Skip CRC32s. */
      return;
    }

  if (gcry_md_get_algo_dlen (subalgo) == 0)
    {
      /* Skip XOFs */
      return;
    }

  *algo_name = 0;

  if (algo == GCRY_KDF_PBKDF2)
    {
      snprintf (algo_name, sizeof(algo_name), "PBKDF2-HMAC-%s",
		gcry_md_algo_name (subalgo));
    }

  bench_print_algo (-24, algo_name);

  obj.ops = mode.ops;
  obj.priv = &mode;

  nsecs_per_iteration = do_slope_benchmark (&obj);
  bench_print_result_nsec_per_iteration (nsecs_per_iteration);
}

void
kdf_bench (char **argv, int argc)
{
  char algo_name[32];
  int i, j;

  bench_print_section ("kdf", "KDF");

  bench_print_header_nsec_per_iteration (24, "");

  if (argv && argc)
    {
      for (i = 0; i < argc; i++)
	{
	  for (j = 1; j < 400; j++)
	    {
	      if (gcry_md_test_algo (j))
		continue;

	      snprintf (algo_name, sizeof(algo_name), "PBKDF2-HMAC-%s",
			gcry_md_algo_name (j));

	      if (!strcmp(argv[i], algo_name))
		kdf_bench_one (GCRY_KDF_PBKDF2, j);
	    }
	}
    }
  else
    {
      for (i = 1; i < 400; i++)
	if (!gcry_md_test_algo (i))
	  kdf_bench_one (GCRY_KDF_PBKDF2, i);
    }

  bench_print_footer (24);
}


/************************************************************ ECC benchmarks. */

#if USE_ECC
enum bench_ecc_algo
{
  ECC_ALGO_ED25519 = 0,
  ECC_ALGO_ED448,
  ECC_ALGO_X25519,
  ECC_ALGO_X448,
  ECC_ALGO_NIST_P192,
  ECC_ALGO_NIST_P224,
  ECC_ALGO_NIST_P256,
  ECC_ALGO_NIST_P384,
  ECC_ALGO_NIST_P521,
  ECC_ALGO_SECP256K1,
  ECC_ALGO_BRAINP256R1,
  __MAX_ECC_ALGO
};

enum bench_ecc_operation
{
  ECC_OPER_MULT = 0,
  ECC_OPER_KEYGEN,
  ECC_OPER_SIGN,
  ECC_OPER_VERIFY,
  __MAX_ECC_OPER
};

struct bench_ecc_oper
{
  enum bench_ecc_operation oper;
  const char *name;
  struct bench_ops *ops;

  enum bench_ecc_algo algo;
};

struct bench_ecc_mult_hd
{
  gcry_ctx_t ec;
  gcry_mpi_t k, x, y;
  gcry_mpi_point_t G, Q;
};

struct bench_ecc_hd
{
  gcry_sexp_t key_spec;
  gcry_sexp_t data;
  gcry_sexp_t pub_key;
  gcry_sexp_t sec_key;
  gcry_sexp_t sig;
};


static int
ecc_algo_fips_allowed (int algo)
{
  switch (algo)
    {
      case ECC_ALGO_NIST_P224:
      case ECC_ALGO_NIST_P256:
      case ECC_ALGO_NIST_P384:
      case ECC_ALGO_NIST_P521:
	return 1;
      case ECC_ALGO_SECP256K1:
      case ECC_ALGO_BRAINP256R1:
      case ECC_ALGO_ED25519:
      case ECC_ALGO_ED448:
      case ECC_ALGO_X25519:
      case ECC_ALGO_X448:
      case ECC_ALGO_NIST_P192:
      default:
	return 0;
    }
}

static const char *
ecc_algo_name (int algo)
{
  switch (algo)
    {
      case ECC_ALGO_ED25519:
	return "Ed25519";
      case ECC_ALGO_ED448:
	return "Ed448";
      case ECC_ALGO_X25519:
	return "X25519";
      case ECC_ALGO_X448:
	return "X448";
      case ECC_ALGO_NIST_P192:
	return "NIST-P192";
      case ECC_ALGO_NIST_P224:
	return "NIST-P224";
      case ECC_ALGO_NIST_P256:
	return "NIST-P256";
      case ECC_ALGO_NIST_P384:
	return "NIST-P384";
      case ECC_ALGO_NIST_P521:
	return "NIST-P521";
      case ECC_ALGO_SECP256K1:
	return "secp256k1";
      case ECC_ALGO_BRAINP256R1:
	return "brainpoolP256r1";
      default:
	return NULL;
    }
}

static const char *
ecc_algo_curve (int algo)
{
  switch (algo)
    {
      case ECC_ALGO_ED25519:
	return "Ed25519";
      case ECC_ALGO_ED448:
	return "Ed448";
      case ECC_ALGO_X25519:
	return "Curve25519";
      case ECC_ALGO_X448:
	return "X448";
      case ECC_ALGO_NIST_P192:
	return "NIST P-192";
      case ECC_ALGO_NIST_P224:
	return "NIST P-224";
      case ECC_ALGO_NIST_P256:
	return "NIST P-256";
      case ECC_ALGO_NIST_P384:
	return "NIST P-384";
      case ECC_ALGO_NIST_P521:
	return "NIST P-521";
      case ECC_ALGO_SECP256K1:
	return "secp256k1";
      case ECC_ALGO_BRAINP256R1:
	return "brainpoolP256r1";
      default:
	return NULL;
    }
}

static int
ecc_nbits (int algo)
{
  switch (algo)
    {
      case ECC_ALGO_ED25519:
	return 255;
      case ECC_ALGO_ED448:
	return 448;
      case ECC_ALGO_X25519:
	return 255;
      case ECC_ALGO_X448:
	return 448;
      case ECC_ALGO_NIST_P192:
	return 192;
      case ECC_ALGO_NIST_P224:
	return 224;
      case ECC_ALGO_NIST_P256:
	return 256;
      case ECC_ALGO_NIST_P384:
	return 384;
      case ECC_ALGO_NIST_P521:
	return 521;
      case ECC_ALGO_SECP256K1:
	return 256;
      case ECC_ALGO_BRAINP256R1:
	return 256;
      default:
	return 0;
    }
}

static int
ecc_map_name (const char *name)
{
  int i;

  for (i = 0; i < __MAX_ECC_ALGO; i++)
    {
      if (strcmp(ecc_algo_name(i), name) == 0)
	{
	  return i;
	}
    }

  return -1;
}


static int
bench_ecc_mult_init (struct bench_obj *obj)
{
  struct bench_ecc_oper *oper = obj->priv;
  struct bench_ecc_mult_hd *hd;
  int p_size = ecc_nbits (oper->algo);
  gpg_error_t err;
  gcry_mpi_t p;

  obj->min_bufsize = 1;
  obj->max_bufsize = 4;
  obj->step_size = 1;
  obj->num_measure_repetitions =
    num_measurement_repetitions / obj->max_bufsize;

  while (obj->num_measure_repetitions == 0)
    {
      if (obj->max_bufsize == 2)
	{
	  obj->num_measure_repetitions = 2;
	}
      else
	{
	  obj->max_bufsize--;
	  obj->num_measure_repetitions =
	    num_measurement_repetitions / obj->max_bufsize;
	}
    }

  hd = calloc (1, sizeof(*hd));
  if (!hd)
    return -1;

  err = gcry_mpi_ec_new (&hd->ec, NULL, ecc_algo_curve(oper->algo));
  if (err)
    {
      fprintf (stderr, PGM ": gcry_mpi_ec_new failed: %s\n",
	      gpg_strerror (err));
      exit (1);
    }
  hd->G = gcry_mpi_ec_get_point ("g", hd->ec, 1);
  hd->Q = gcry_mpi_point_new (0);
  hd->x = gcry_mpi_new (0);
  hd->y = gcry_mpi_new (0);
  hd->k = gcry_mpi_new (p_size);
  gcry_mpi_randomize (hd->k, p_size, GCRY_WEAK_RANDOM);
  p = gcry_mpi_ec_get_mpi ("p", hd->ec, 1);
  gcry_mpi_mod (hd->k, hd->k, p);
  gcry_mpi_release (p);

  obj->hd = hd;
  return 0;
}

static void
bench_ecc_mult_free (struct bench_obj *obj)
{
  struct bench_ecc_mult_hd *hd = obj->hd;

  gcry_mpi_release (hd->k);
  gcry_mpi_release (hd->y);
  gcry_mpi_release (hd->x);
  gcry_mpi_point_release (hd->Q);
  gcry_mpi_point_release (hd->G);
  gcry_ctx_release (hd->ec);
  free (hd);
  obj->hd = NULL;
}

static void
bench_ecc_mult_do_bench (struct bench_obj *obj, void *buf, size_t num_iter)
{
  struct bench_ecc_oper *oper = obj->priv;
  struct bench_ecc_mult_hd *hd = obj->hd;
  gcry_mpi_t y;
  size_t i;

  (void)buf;

  if (oper->algo == ECC_ALGO_X25519 || oper->algo == ECC_ALGO_X448)
    {
      y = NULL;
    }
  else
    {
      y = hd->y;
    }

  for (i = 0; i < num_iter; i++)
    {
      gcry_mpi_ec_mul (hd->Q, hd->k, hd->G, hd->ec);
      if (gcry_mpi_ec_get_affine (hd->x, y, hd->Q, hd->ec))
	{
	  fprintf (stderr, PGM ": gcry_mpi_ec_get_affine failed\n");
	  exit (1);
	}
    }
}


static int
bench_ecc_init (struct bench_obj *obj)
{
  struct bench_ecc_oper *oper = obj->priv;
  struct bench_ecc_hd *hd;
  int p_size = ecc_nbits (oper->algo);
  gpg_error_t err;
  gcry_mpi_t x;

  obj->min_bufsize = 1;
  obj->max_bufsize = 4;
  obj->step_size = 1;
  obj->num_measure_repetitions =
    num_measurement_repetitions / obj->max_bufsize;

  while (obj->num_measure_repetitions == 0)
    {
      if (obj->max_bufsize == 2)
	{
	  obj->num_measure_repetitions = 2;
	}
      else
	{
	  obj->max_bufsize--;
	  obj->num_measure_repetitions =
	    num_measurement_repetitions / obj->max_bufsize;
	}
    }

  hd = calloc (1, sizeof(*hd));
  if (!hd)
    return -1;

  x = gcry_mpi_new (p_size);
  gcry_mpi_randomize (x, p_size, GCRY_WEAK_RANDOM);

  switch (oper->algo)
    {
      default:
        gcry_mpi_release (x);
        free (hd);
        return -1;

      case ECC_ALGO_ED25519:
        err = gcry_sexp_build (&hd->key_spec, NULL,
                               "(genkey (ecdsa (curve \"Ed25519\")"
                               "(flags eddsa)))");
	if (err)
	  break;
        err = gcry_sexp_build (&hd->data, NULL,
                               "(data (flags eddsa)(hash-algo sha512)"
                               " (value %m))", x);
	break;

      case ECC_ALGO_ED448:
        err = gcry_sexp_build (&hd->key_spec, NULL,
                               "(genkey (ecdsa (curve \"Ed448\")"
                               "(flags eddsa)))");
	if (err)
	  break;
        err = gcry_sexp_build (&hd->data, NULL,
                               "(data (flags eddsa)(hash-algo shake256)"
                               " (value %m))", x);
	break;

      case ECC_ALGO_NIST_P192:
      case ECC_ALGO_NIST_P224:
      case ECC_ALGO_NIST_P256:
      case ECC_ALGO_NIST_P384:
      case ECC_ALGO_NIST_P521:
        err = gcry_sexp_build (&hd->key_spec, NULL,
                               "(genkey (ECDSA (nbits %d)))", p_size);
	if (err)
	  break;
        err = gcry_sexp_build (&hd->data, NULL,
			       "(data (flags raw) (value %m))", x);
	break;
      case ECC_ALGO_BRAINP256R1:
        err = gcry_sexp_build (&hd->key_spec, NULL,
                               "(genkey (ECDSA (curve brainpoolP256r1)))");
	if (err)
	  break;
        err = gcry_sexp_build (&hd->data, NULL,
			       "(data (flags raw) (value %m))", x);
	break;
    }

  gcry_mpi_release (x);

  if (err)
    {
      fprintf (stderr, PGM ": gcry_sexp_build failed: %s\n",
	       gpg_strerror (err));
      exit (1);
    }

  obj->hd = hd;
  return 0;
}

static void
bench_ecc_free (struct bench_obj *obj)
{
  struct bench_ecc_hd *hd = obj->hd;

  gcry_sexp_release (hd->sig);
  gcry_sexp_release (hd->pub_key);
  gcry_sexp_release (hd->sec_key);
  gcry_sexp_release (hd->data);
  gcry_sexp_release (hd->key_spec);
  free (hd);
  obj->hd = NULL;
}

static void
bench_ecc_keygen (struct bench_ecc_hd *hd)
{
  gcry_sexp_t key_pair;
  gpg_error_t err;

  err = gcry_pk_genkey (&key_pair, hd->key_spec);
  if (err)
    {
      fprintf (stderr, PGM ": gcry_pk_genkey failed: %s\n",
		gpg_strerror (err));
      exit (1);
    }

  hd->pub_key = gcry_sexp_find_token (key_pair, "public-key", 0);
  if (!hd->pub_key)
    {
      fprintf (stderr, PGM ": public part missing in key\n");
      exit (1);
    }
  hd->sec_key = gcry_sexp_find_token (key_pair, "private-key", 0);
  if (!hd->sec_key)
    {
      fprintf (stderr, PGM ": private part missing in key\n");
      exit (1);
    }

  gcry_sexp_release (key_pair);
}

static void
bench_ecc_keygen_do_bench (struct bench_obj *obj, void *buf, size_t num_iter)
{
  struct bench_ecc_hd *hd = obj->hd;
  size_t i;

  (void)buf;

  for (i = 0; i < num_iter; i++)
    {
      bench_ecc_keygen (hd);
      gcry_sexp_release (hd->pub_key);
      gcry_sexp_release (hd->sec_key);
    }

  hd->pub_key = NULL;
  hd->sec_key = NULL;
}

static void
bench_ecc_sign_do_bench (struct bench_obj *obj, void *buf, size_t num_iter)
{
  struct bench_ecc_hd *hd = obj->hd;
  gpg_error_t err;
  size_t i;

  (void)buf;

  bench_ecc_keygen (hd);

  for (i = 0; i < num_iter; i++)
    {
      err = gcry_pk_sign (&hd->sig, hd->data, hd->sec_key);
      if (err)
	{
	  fprintf (stderr, PGM ": gcry_pk_sign failed: %s\n",
		  gpg_strerror (err));
	  exit (1);
	}
      gcry_sexp_release (hd->sig);
    }

  gcry_sexp_release (hd->pub_key);
  gcry_sexp_release (hd->sec_key);
  hd->sig = NULL;
  hd->pub_key = NULL;
  hd->sec_key = NULL;
}

static void
bench_ecc_verify_do_bench (struct bench_obj *obj, void *buf, size_t num_iter)
{
  struct bench_ecc_hd *hd = obj->hd;
  gpg_error_t err;
  int i;

  (void)buf;

  bench_ecc_keygen (hd);
  err = gcry_pk_sign (&hd->sig, hd->data, hd->sec_key);
  if (err)
    {
      fprintf (stderr, PGM ": gcry_pk_sign failed: %s\n",
	      gpg_strerror (err));
      exit (1);
    }

  for (i = 0; i < num_iter; i++)
    {
      err = gcry_pk_verify (hd->sig, hd->data, hd->pub_key);
      if (err)
	{
	  fprintf (stderr, PGM ": gcry_pk_verify failed: %s\n",
		  gpg_strerror (err));
	  exit (1);
	}
    }

  gcry_sexp_release (hd->sig);
  gcry_sexp_release (hd->pub_key);
  gcry_sexp_release (hd->sec_key);
  hd->sig = NULL;
  hd->pub_key = NULL;
  hd->sec_key = NULL;
}


static struct bench_ops ecc_mult_ops = {
  &bench_ecc_mult_init,
  &bench_ecc_mult_free,
  &bench_ecc_mult_do_bench
};

static struct bench_ops ecc_keygen_ops = {
  &bench_ecc_init,
  &bench_ecc_free,
  &bench_ecc_keygen_do_bench
};

static struct bench_ops ecc_sign_ops = {
  &bench_ecc_init,
  &bench_ecc_free,
  &bench_ecc_sign_do_bench
};

static struct bench_ops ecc_verify_ops = {
  &bench_ecc_init,
  &bench_ecc_free,
  &bench_ecc_verify_do_bench
};


static struct bench_ecc_oper ecc_operations[] = {
  { ECC_OPER_MULT,   "mult",   &ecc_mult_ops },
  { ECC_OPER_KEYGEN, "keygen", &ecc_keygen_ops },
  { ECC_OPER_SIGN,   "sign",   &ecc_sign_ops },
  { ECC_OPER_VERIFY, "verify", &ecc_verify_ops },
  { 0, NULL, NULL }
};


static void
cipher_ecc_one (enum bench_ecc_algo algo, struct bench_ecc_oper *poper)
{
  struct bench_ecc_oper oper = *poper;
  struct bench_obj obj = { 0 };
  double result;

  if ((algo == ECC_ALGO_X25519 || algo == ECC_ALGO_X448 ||
       algo == ECC_ALGO_SECP256K1) && oper.oper != ECC_OPER_MULT)
    return;

  oper.algo = algo;

  bench_print_mode (14, oper.name);

  obj.ops = oper.ops;
  obj.priv = &oper;

  result = do_slope_benchmark (&obj);
  bench_print_result_nsec_per_iteration (result);
}


static void
_ecc_bench (int algo)
{
  const char *algo_name;
  int i;

  /* Skip not allowed mechanisms */
  if (in_fips_mode && !ecc_algo_fips_allowed (algo))
    return;

  algo_name = ecc_algo_name (algo);

  bench_print_header_nsec_per_iteration (14, algo_name);

  for (i = 0; ecc_operations[i].name; i++)
    cipher_ecc_one (algo, &ecc_operations[i]);

  bench_print_footer (14);
}
#endif


void
ecc_bench (char **argv, int argc)
{
#if USE_ECC
  int i, algo;

  bench_print_section ("ecc", "ECC");

  if (argv && argc)
    {
      for (i = 0; i < argc; i++)
        {
          algo = ecc_map_name (argv[i]);
          if (algo >= 0)
            _ecc_bench (algo);
        }
    }
  else
    {
      for (i = 0; i < __MAX_ECC_ALGO; i++)
        _ecc_bench (i);
    }
#else
  (void)argv;
  (void)argc;
#endif
}

/************************************************************** Main program. */

void
print_help (void)
{
  static const char *help_lines[] = {
    "usage: bench-slope [options] [hash|mac|cipher|kdf|ecc [algonames]]",
    "",
    " options:",
    "   --cpu-mhz <mhz>           Set CPU speed for calculating cycles",
    "                             per bytes results.  Set as \"auto\"",
    "                             for auto-detection of CPU speed.",
    "   --disable-hwf <features>  Disable hardware acceleration feature(s)",
    "                             for benchmarking.",
    "   --repetitions <n>         Use N repetitions (default "
                                     STR2(NUM_MEASUREMENT_REPETITIONS) ")",
    "   --unaligned               Use unaligned input buffers.",
    "   --csv                     Use CSV output format",
    NULL
  };
  const char **line;

  for (line = help_lines; *line; line++)
    fprintf (stdout, "%s\n", *line);
}


/* Warm up CPU.  */
static void
warm_up_cpu (void)
{
  struct nsec_time start, end;

  get_nsec_time (&start);
  do
    {
      get_nsec_time (&end);
    }
  while (get_time_nsec_diff (&start, &end) < 1000.0 * 1000.0 * 1000.0);
}


int
main (int argc, char **argv)
{
  int last_argc = -1;

  if (argc)
    {
      argc--;
      argv++;
    }

  /* We skip this test if we are running under the test suite (no args
     and srcdir defined) and GCRYPT_NO_BENCHMARKS is set.  */
  if (!argc && getenv ("srcdir") && getenv ("GCRYPT_NO_BENCHMARKS"))
    exit (77);

  if (getenv ("GCRYPT_IN_REGRESSION_TEST"))
    {
      in_regression_test = 1;
      num_measurement_repetitions = 2;
    }
  else
    num_measurement_repetitions = NUM_MEASUREMENT_REPETITIONS;

  while (argc && last_argc != argc)
    {
      last_argc = argc;

      if (!strcmp (*argv, "--"))
	{
	  argc--;
	  argv++;
	  break;
	}
      else if (!strcmp (*argv, "--help"))
	{
	  print_help ();
	  exit (0);
	}
      else if (!strcmp (*argv, "--verbose"))
	{
	  verbose++;
	  argc--;
	  argv++;
	}
      else if (!strcmp (*argv, "--debug"))
	{
	  verbose += 2;
	  debug++;
	  argc--;
	  argv++;
	}
      else if (!strcmp (*argv, "--csv"))
	{
	  csv_mode = 1;
	  argc--;
	  argv++;
	}
      else if (!strcmp (*argv, "--unaligned"))
	{
	  unaligned_mode = 1;
	  argc--;
	  argv++;
	}
      else if (!strcmp (*argv, "--disable-hwf"))
	{
	  argc--;
	  argv++;
	  if (argc)
	    {
	      if (gcry_control (GCRYCTL_DISABLE_HWF, *argv, NULL))
		fprintf (stderr,
			 PGM
			 ": unknown hardware feature `%s' - option ignored\n",
			 *argv);
	      argc--;
	      argv++;
	    }
	}
      else if (!strcmp (*argv, "--cpu-mhz"))
	{
	  argc--;
	  argv++;
	  if (argc)
	    {
              if (!strcmp (*argv, "auto"))
                {
                  auto_ghz = 1;
                }
              else
                {
                  cpu_ghz = atof (*argv);
                  cpu_ghz /= 1000;	/* Mhz => Ghz */
                }

	      argc--;
	      argv++;
	    }
	}
      else if (!strcmp (*argv, "--repetitions"))
	{
	  argc--;
	  argv++;
	  if (argc)
	    {
	      num_measurement_repetitions = atof (*argv);
              if (num_measurement_repetitions < 2)
                {
                  fprintf (stderr,
                           PGM
                           ": value for --repetitions too small - using %d\n",
                           NUM_MEASUREMENT_REPETITIONS);
                  num_measurement_repetitions = NUM_MEASUREMENT_REPETITIONS;
                }
	      argc--;
	      argv++;
	    }
	}
    }

  xgcry_control ((GCRYCTL_SET_VERBOSITY, (int) verbose));

  if (!gcry_check_version (GCRYPT_VERSION))
    {
      fprintf (stderr, PGM ": version mismatch; pgm=%s, library=%s\n",
	       GCRYPT_VERSION, gcry_check_version (NULL));
      exit (1);
    }

  if (debug)
    xgcry_control ((GCRYCTL_SET_DEBUG_FLAGS, 1u, 0));

  xgcry_control ((GCRYCTL_DISABLE_SECMEM, 0));
  xgcry_control ((GCRYCTL_INITIALIZATION_FINISHED, 0));
  xgcry_control ((GCRYCTL_ENABLE_QUICK_RANDOM, 0));

  if (gcry_fips_mode_active ())
    in_fips_mode = 1;

  if (in_regression_test)
    fputs ("Note: " PGM " running in quick regression test mode.\n", stdout);

  if (!argc)
    {
      warm_up_cpu ();
      hash_bench (NULL, 0);
      mac_bench (NULL, 0);
      cipher_bench (NULL, 0);
      kdf_bench (NULL, 0);
      ecc_bench (NULL, 0);
    }
  else if (!strcmp (*argv, "hash"))
    {
      argc--;
      argv++;

      warm_up_cpu ();
      hash_bench ((argc == 0) ? NULL : argv, argc);
    }
  else if (!strcmp (*argv, "mac"))
    {
      argc--;
      argv++;

      warm_up_cpu ();
      mac_bench ((argc == 0) ? NULL : argv, argc);
    }
  else if (!strcmp (*argv, "cipher"))
    {
      argc--;
      argv++;

      warm_up_cpu ();
      cipher_bench ((argc == 0) ? NULL : argv, argc);
    }
  else if (!strcmp (*argv, "kdf"))
    {
      argc--;
      argv++;

      warm_up_cpu ();
      kdf_bench ((argc == 0) ? NULL : argv, argc);
    }
  else if (!strcmp (*argv, "ecc"))
    {
      argc--;
      argv++;

      warm_up_cpu ();
      ecc_bench ((argc == 0) ? NULL : argv, argc);
    }
  else
    {
      fprintf (stderr, PGM ": unknown argument: %s\n", *argv);
      print_help ();
    }

  return 0;
}

#endif /* !NO_GET_NSEC_TIME */
