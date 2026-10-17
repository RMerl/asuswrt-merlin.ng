/*
 *  Kernel memory monitor
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Dual:>>
 *
 * $Id: kmem_monitor.c 855186 2025-05-01 00:03:29Z $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <ctype.h>
#include <dirent.h>
#include <limits.h>

#include <bcmstdlib_s.h>
#include <bcmnvram.h>

#define TOP_N_PROCESSES	10
#define TOP_N_MODULES	10
#define KMEM_THRESHOLD	20	// In percentage

#define PID_MAX_DEFAULT	32768

// Structure to store kernel module information
typedef struct {
	char kmodule_name[NAME_MAX];
	long kmem_usage;
} kernel_module_t;

typedef struct {
	int pid;
	long rss;
	char name[NAME_MAX];
} process_info_t;

// Global variables
bool skip_process_sizing  = FALSE;
bool skip_module_sizing   = FALSE;
bool skip_skb_slab_sizing = FALSE;
bool skip_kmem_monitor    = FALSE;

process_info_t processes[PID_MAX_DEFAULT];  // Array to store process memory info

// Function to get total and free memory
static void
get_kmem_info(long *total_memory, long *free_memory, float *free_percentage)
{
	char line[256];
	long total = 0, free = 0, inactive = 0, cached = 0;

	FILE *fp = fopen("/proc/meminfo", "r");
	if (fp == NULL) {
		perror("Failed to open /proc/meminfo");
		skip_kmem_monitor = TRUE;
		return;
	}

	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "MemTotal:", 9) == 0) {
			sscanf(line, "MemTotal: %ld kB", &total);
		} else if (strncmp(line, "MemFree:", 8) == 0) {
			sscanf(line, "MemFree: %ld kB", &free);
		} else if (strncmp(line, "Cached:", 7) == 0) {
			sscanf(line, "Cached: %ld kB", &cached);
		} else if (strncmp(line, "Inactive:", 8) == 0) {
			sscanf(line, "Inactive: %ld kB", &inactive);
		}
	}
	fclose(fp);

	// Calculate free memory including cached and inactive
	*free_memory = free + cached + inactive;
	*total_memory = total;

	if (total) {
		*free_percentage = ((float)(*free_memory) / total) * 100;
	} else {
		*free_percentage = 0;	// should never happen
	}
}

// Function to get total memory used by SKBs
static long
get_skb_cache_memory(void)
{
	char line[256];
	long total_skb_memory = 0;

	FILE *fp = fopen("/proc/slabinfo", "r");
	if (fp == NULL) {
		perror("Failed to open /proc/slabinfo");
		skip_skb_slab_sizing = TRUE;
		return 0;
	}

	while (fgets(line, sizeof(line), fp)) {
		if (strstr(line, "skbuff_")) {
			long active_objs = 0, obj_size = 0;
			sscanf(line, "%*s %ld %*d %ld", &active_objs, &obj_size);
			total_skb_memory += active_objs * obj_size;
		}
	}
	fclose(fp);
	return total_skb_memory / 1024; // Convert to kB
}

static int
compare_by_kmem_usage(const void *a, const void *b)
{
	kernel_module_t *module_a = (kernel_module_t *)a;
	kernel_module_t *module_b = (kernel_module_t *)b;

	// Compare by kmem_usage, sort in descending order
	if (module_a->kmem_usage > module_b->kmem_usage) {
		return -1;
	} else if (module_a->kmem_usage < module_b->kmem_usage) {
		return 1;
	} else {
		return 0;
	}
}

static int
compare_by_rss(const void *a, const void *b)
{
	process_info_t *process_a = (process_info_t *)a;
	process_info_t *process_b = (process_info_t *)b;

	// Compare by rss, sort in descending order
	if (process_a->rss > process_b->rss) {
		return -1;
	} else if (process_a->rss < process_b->rss) {
		return 1;
	} else {
		return 0;
	}
}

// Function that shows the top modules consuming the most memory
static void
show_top_kmodules(void)
{
	struct dirent *entry;
	char km_path[PATH_MAX];
	FILE *fp;
	char line[256];
	int module_count = 0;
	int i;
	kernel_module_t modules[NAME_MAX];  // Array to store module info

	DIR *dir = opendir("/sys/module");
	if (dir == NULL) {
		perror("Error opening /sys/module");
		skip_module_sizing = TRUE;
		return;
	}

	// Loop through each module directory in /sys/module
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] != '.') {  // Ignore . and .. directories
			// Prepare the path to the meminfo file
			snprintf(km_path, sizeof(km_path), "/sys/module/%s/coresize",
					entry->d_name);

			fp = fopen(km_path, "r");
			if (fp == NULL) {
				continue;
			}
			if (fgets(line, sizeof(line), fp)) {
				sscanf(line, "%ld", &modules[module_count].kmem_usage);
				strlcpy((char *) &modules[module_count].kmodule_name,
						entry->d_name, NAME_MAX);
				module_count++;
			}
			fclose(fp);
		}
	}
	closedir(dir);

	// If no modules found, print an error and return
	if (module_count == 0) {
		return;
	}

	// Sort the modules by memory usage in descending order
	qsort(modules, module_count, sizeof(kernel_module_t), compare_by_kmem_usage);

	// Print the top N memory-consuming modules
	printf("\tTop %d Kernel Modules (by memory usage):\n", TOP_N_MODULES);
	for (i = 0; i < (module_count < TOP_N_MODULES ? module_count : TOP_N_MODULES); i++) {
		printf("\t\t%-28s %ld kB\n",
				modules[i].kmodule_name, (long) modules[i].kmem_usage/1024);
	}

	return;
}

// Function that shows the top user processes consuming the most memory
static void
show_top_processes()
{
	struct dirent *entry;
	FILE *fp;
	char path[PATH_MAX], line[NAME_MAX];
	long pid_memory;
	char pname[NAME_MAX];
	int process_count = 0;
	int i;

	DIR *dir = opendir("/proc");
	if (dir == NULL) {
		perror("Error opening /proc");
		skip_process_sizing = TRUE;
		return;
	}

	// Iterate over all process directories in /proc
	while ((entry = readdir(dir)) != NULL) {
		if (isdigit(entry->d_name[0])) {  // Check if entry is a process directory
			// Form path to the /proc/[pid]/status file
			snprintf(path, sizeof(path), "/proc/%s/status", entry->d_name);

			// Open the status file of the process
			fp = fopen(path, "r");
			if (fp == NULL)
				continue;

			// Read the VmRSS field to get memory usage
			while (fgets(line, sizeof(line), fp)) {
				if (strncmp(line, "Name:", 5) == 0) {
					if (sscanf(line, "Name: %s", pname) > 0) {
						strlcpy(processes[process_count].name,
								pname, NAME_MAX);
					}
					continue;
				}

				if (strncmp(line, "VmRSS:", 6) == 0) {
					sscanf(line, "VmRSS: %ld kB", &pid_memory);
					processes[process_count].rss = pid_memory;
					processes[process_count].pid = atoi(entry->d_name);
					process_count++;
					break;
				}
			}
			fclose(fp);

			// Check if process count exceeds array bounds
			if (process_count >= 1024) {
				printf("Warning: Exceeded process array limit 1024."
						" Stopping process collection.\n");
				break;
			}
		}
	}

	closedir(dir);

	// If no processes found, print an error and return
	if (process_count == 0) {
		printf("No processes found with memory usage.\n");
		return;
	}

	// Sort processes by memory usage in descending order
	qsort(processes, process_count, sizeof(process_info_t), compare_by_rss);

	// Print the top N memory-consuming processes, limit to TOP_N_PROCESSES
	printf("\tTop %d Memory-Consuming Processes (by RSS):\n", TOP_N_PROCESSES);
	for (i = 0; i < (process_count < TOP_N_PROCESSES ? process_count : TOP_N_PROCESSES); i++) {
		printf("\t\t%-6d %-21s %ld kB\n", processes[i].pid,
				processes[i].name, processes[i].rss);
	}
}

// Main routine that monitor kernel memory available
void
kmem_watchdog(void)
{
	long total_memory, free_memory, skb_memory;
	float free_percentage;
	int low_mem_threshold = KMEM_THRESHOLD;
	char *nv_str;

	if (((nv_str = (char *) nvram_get("kmem_threshold")) != NULL)) {
		low_mem_threshold = (uint32)strtoul(nv_str, NULL, 0);
	}

	if (skip_kmem_monitor)
		return;

	// Get kernel memory info
	get_kmem_info(&total_memory, &free_memory, &free_percentage);

	// Check if free memory is below threshold
	if (free_percentage < low_mem_threshold) {
		char timestr[256];
		time_t t = time(NULL);
		struct tm *tmp_tm = localtime(&t);

		if (strftime(timestr, sizeof(timestr), "\n[%F %T]\n", tmp_tm) != 0)
			printf(timestr);

		printf("WARNING: Free memory is below %d%% of total memory!\n"
				"\tTotal Memory: %ld kB\n"
				"\tFree Memory (including buffers & inactive): %ld kB\n"
				"\tFree Memory Percentage: %.2f%%\n",
				low_mem_threshold, total_memory, free_memory,
				free_percentage);

		if (!skip_skb_slab_sizing) {
			// Get total SKB memory
			skb_memory = get_skb_cache_memory();
			printf("\tSKB Cache Memory: %ld kB\n", skb_memory);
		}

		if (!skip_module_sizing)
			show_top_kmodules();

		if (!skip_process_sizing)
			show_top_processes();

		printf("\n");
	}
}
