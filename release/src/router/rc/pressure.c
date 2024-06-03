/*
 * Copyright (C) 2020 InvenSense, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <inttypes.h>
#include <poll.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <getopt.h>
#include <math.h>
#include <string.h>
#include <linux/input.h>
#include <unistd.h>
#include <time.h>
#include <bcmnvram.h>
#include <sys/sysinfo.h>
#include <shared.h>

/* version */
#define VERSION_STR         "0.0.1_test1"

/* device files for sensor data (system dependent) */
#define DEV_NAME_SENSOR      "/dev/iio:device"

/* all control is done by sysfs under accel */
#define SYSFS_PATH    "/sys/bus/iio/devices/iio:device"

/* sysfs attribute names */
#define ATTR_BUFFER "buffer/enable"
#define ATTR_BUFFER_LENGTH "buffer/length"
#define ATTR_SCAN_ELEMENT "scan_elements/in_timestamp_en"
#define ATTR_CHIP_NAME      "name"
#define ATTR_SAMPLE_FREQUENCY "sampling_frequency"
#define ATTR_MODE "mode"
#define ATTR_SAMPLE_FREQ_AVAIL "sampling_frequency_available"
#define BUF_MAX             24

/* sensor type */
enum sensor_type {
	SENSOR_PRESS = 1,
	SENSOR_TEMP = 2,
	SENSOR_MAX
};

struct sensor_data {
	int32_t press;
	int32_t temp;
	uint64_t timestamp;
};

/* sensor names */
static const char *sensor_type_name[SENSOR_MAX] = {
	"None",
	"Pressure",    /* SENSOR_PRESS */
	"Temperature",     /* SENSOR_TEMP */
};

static int64_t prev_ts[SENSOR_MAX];

/* variables */
static int device_id = 0;
static int skip_packet = 0;


static struct sensor_data_statistics{
	float press;
	float temp;
	int detection_count;
} statistic_data = {0.0, 0.0, 0};

/* commandline options */
static const struct option options[] = {
	{"help", no_argument, NULL, 'h'},
	{"rate", required_argument, NULL, 'r'},
	{"mode", required_argument, NULL, 'm'},
	{"device_num", required_argument, NULL, 'd'},
	{"timeout", required_argument, NULL, 't'},
	{0, 0, 0, 0},
};

static const char *options_descriptions[] = {
	"Show this help and quit.",
	"Set sensor rate",
	"Set pressure sensor mode",
	"Set sysfs device number",
	"Set a timeout for the program to exit automatically. \nTimeout should be set between 2 and 20 seconds.",
};

/* get the current time */
static int64_t get_current_timestamp(void)
{
	struct timespec tp;

	clock_gettime(CLOCK_REALTIME, &tp);
	return  (int64_t)tp.tv_sec * 1000000000LL + (int64_t)tp.tv_nsec;
}


/* write a value to sysfs */
static int write_sysfs_int(const char *sysfs_path, const char *attr, int data)
{
	FILE *fp;
	int ret;
	char path[1024];

	ret = snprintf(path, sizeof(path), "%s%d/%s", sysfs_path, device_id, attr);
	//printf("sysfs: %d (0x%x) -> %s\n", data, data, path);

	if (ret < 0 || ret >= (int)sizeof(path)) {
		return -1;
	}

	ret = 0;
	//printf("sysfs: %d (0x%x) -> %s\n", data, data, path);
	fp = fopen(path, "w");
	if (fp == NULL) {
		ret = -errno;
		printf("Failed to open %s\n", path);
	} else {
		if (fprintf(fp, "%d\n", data) < 0) {
			printf("Failed to write to %s\n", path);
			ret = -errno;
		}
		printf("Set %s to %d\n", path, data);
		fclose(fp);
	}
	fflush(stdout);
	return ret;
}

/* write a string value to sysfs */
static int write_sysfs_string(const char *sysfs_path, const char *attr, char *data)
{
	FILE *fp;
	int ret;
	char path[1024];

	ret = snprintf(path, sizeof(path), "%s%d/%s", sysfs_path,device_id ,attr);
	if (ret < 0 || ret >= (int)sizeof(path)) {
		return -1;
	}

	ret = 0;
	//printf("sysfs: %s -> %s\n", data, path);
	fp = fopen(path, "w");
	if (fp == NULL) {
		ret = -errno;
		printf("Failed to open %s\n", path);
	} else {
		if (fprintf(fp, "%s\n", data) < 0) {
			printf("Failed to write to %s\n", path);
			ret = -errno;
		}
		fclose(fp);
	}
	fflush(stdout);
	return ret;
}

/* get chip name from sysfs */
static int show_chip_name(const char *sysfs_path)
{
	FILE *fp;
	int ret;
	char name[256];
	char path[1024];

	ret = snprintf(path, sizeof(path), "%s%d/%s", sysfs_path, device_id, ATTR_CHIP_NAME);
	if (ret < 0 || ret >= (int)sizeof(path)) {
		return -1;
	}

	ret = 0;
	fp = fopen(path, "r");
	if (fp == NULL) {
		ret = -errno;
		printf("Failed to open %s\n", path);
	} else {
		if (fscanf(fp, "%s", name) != 1) {
			printf("Failed to read chip name\n");
			ret = -1;
		} else
			printf("chip : %s\n", name);
		fclose(fp);
	}
	fflush(stdout);
	return ret;
}

/* get chip name from sysfs */
static int read_sysfs(const char *sysfs_path, const char *attr_name)
{
	FILE *fp;
	int ret;
	char result[256];
	char path[1024];

	ret = snprintf(path, sizeof(path), "%s%d/%s", sysfs_path, device_id, attr_name);
	if (ret < 0 || ret >= (int)sizeof(path)) {
		return -1;
	}

	ret = 0;
	fp = fopen(path, "r");
	if (fp == NULL) {
		ret = -errno;
		printf("Failed to open %s\n", path);
	} else {
		if (fscanf(fp, "%s", result) != 1) {
			printf("Failed to read chip name\n");
			ret = -1;
		} else
			printf("sysfs [%s%d/%s], data [%s]\n",sysfs_path, device_id, attr_name, result );
		fclose(fp);
	}
	fflush(stdout);
	return ret;
}

static int read_sysfs_int(const char *sysfs_path, const char *attr_name, int *val)
{
	FILE *fp;
	int ret;
	char result[256];
	char path[1024];

	ret = snprintf(path, sizeof(path), "%s%d/%s", sysfs_path, device_id, attr_name);
	if (ret < 0 || ret >= (int)sizeof(path)) {
		return -1;
	}

	ret = 0;
	fp = fopen(path, "r");
	if (fp == NULL) {
		ret = -errno;
		printf("Failed to open %s\n", path);
	} else {
		if (fscanf(fp, "%s", result) != 1) {
			printf("Failed to read chip name\n");
			ret = -1;
		} else {
			*val = atoi(result);
		}
		fclose(fp);
	}
	fflush(stdout);
	return ret;
}

/* show usage */
static void usage(void)
{
	unsigned int i;

	printf("Usage:\n\t sensors-sysfs [-h] [-r] [-m] [-d] [-t]"
			"\n\nOptions:\n");
	for (i = 0; options[i].name; i++)
		printf("\t-%c, --%s\n\t\t\t%s\n",
				options[i].val, options[i].name,
				options_descriptions[i]);
	printf("Version:\n\t%s\n", VERSION_STR);
	fflush(stdout);
}

static int stop_prog()
{
	int ret = 0;
	int i;
	char press_buf[15], temp_buf[15];

	usleep(20000);
	if (write_sysfs_int(SYSFS_PATH, ATTR_SCAN_ELEMENT, 0)) return 1;
	usleep(20000);
	if (write_sysfs_int(SYSFS_PATH, ATTR_BUFFER, 0)) return 1;
	usleep(20000);

	if (statistic_data.press == 0 || statistic_data.temp == 0 || statistic_data.detection_count == 0){
		_dprintf("ICP Sensor: Didn't get any data \n");
		return 1;
	}

	_dprintf("ICP Sensor: Total =>\t Press: %5.8f kPa\t Temp: %5.8f C  Count: %d \n", 
		statistic_data.press, statistic_data.temp, statistic_data.detection_count);
	

	/* Calculate average */	
	// pressure
	ret = snprintf(press_buf, sizeof(press_buf), "%.4f", statistic_data.press/statistic_data.detection_count);
	if (ret < 0 || ret >= (int)sizeof(press_buf)) {
		_dprintf("error: ret = %d \n", ret);
		return 1;
	}
	nvram_set("icp-pressure", press_buf);
	
	// temperature
	ret = snprintf(temp_buf, sizeof(temp_buf), "%.4f", statistic_data.temp/statistic_data.detection_count);
	if (ret < 0 || ret >= (int)sizeof(temp_buf)) {
		_dprintf("error: ret = %d \n", ret);
		return 1;
	}
	nvram_set("icp-temperature", temp_buf);		

	_dprintf("ICP Sensor: Average =>\t Press: %s kPa\t Temp: %s C \n", press_buf, temp_buf);
	fflush(stdout);


	return 0;
}

/* handle data packet */
static void handle_one_packet(struct sensor_data *data)
{
	/* TODO:
	 * Integrate a function here according to the requirement on own platform.
	 * The below is to print data for test.
	 */

	int64_t ts, ts_gap_prev;
	float press = (float)((data->press / 131072.0) * 40.0 + 70.0); 	
	float temp = (float)((data->temp / 262144.0) * 65.0 + 25.0);
	
	statistic_data.press += press;
	statistic_data.temp += temp;
	statistic_data.detection_count += 1;

	ts = data->timestamp;
	ts_gap_prev = (ts - prev_ts[SENSOR_PRESS]) / 1000;
	prev_ts[SENSOR_PRESS] = ts;
	printf("PRESS (LSB) :(%d) %5.8f kPa  Temp :(%d) %5.8f C, timestamp  %llu,  gap %llu  Hz %d\n",data->press , 
			press, data->temp, temp, ts, ts_gap_prev, 1000000 / ts_gap_prev);

}

void init_nvram_unset(){
	nvram_unset("icp-temperature");
	nvram_unset("icp-pressure");
}

/* main */
int
pressure_main(int argc, char **argv)
{	
	init_nvram_unset();

	int ret;
	struct sigaction sig_action;
	int opt, option_index;
	int fd;
	struct pollfd fds[2];
	char buf[BUF_MAX];
	ssize_t num;
	bool delta_ang_en = false;
	int frequency = -1; // 40Hz
	int timeout = 8; // seconds
	int mode = -1; // default mode
	char dev_path[256];
	sigset_t set;	

	int ac = 0;
	for(ac=1;ac<argc;ac++)
	{
		_dprintf("%s ",argv[ac]);
	}
	_dprintf("\n");
	
	while ((opt = getopt_long(argc, argv, "hcr:swy:r:m:t:", options, &option_index)) != -1) {
		switch (opt) {
			case 'r':
				frequency = atoi(optarg);
				break;
			case 'm':
				mode = atoi(optarg);
				break;
			case 't':
				timeout = atoi(optarg);
				if (timeout < 2 || timeout > 20) {
					_dprintf("timeout invaild \n");
					return 1;
				}
				break;
			case 'h':
				usage();
				return 0;
		}
	}

	if (argc < 2) {
		ret = read_sysfs_int(SYSFS_PATH, ATTR_SAMPLE_FREQUENCY, &frequency);
		if (ret){
			_dprintf("Failed to read %s%d/%s \n", SYSFS_PATH, device_id, ATTR_SAMPLE_FREQUENCY);
			fflush(stdout);
			return 1;
		}

		ret = read_sysfs_int(SYSFS_PATH, ATTR_MODE, &mode);
		if (ret) {
			_dprintf("Failed to read %s%d/%s \n", SYSFS_PATH, device_id, ATTR_MODE);
			fflush(stdout);
			return 1;
		}

		_dprintf("Start with default latest setting, Mode %d, Freq %dHz, timeout %d sec\n", mode, frequency, timeout);
	}
	
	_dprintf("ICP Sensor: Timeout: %d sec \n", timeout);
	
	{		
		if (mode != -1) {
			_dprintf(">Set mode to %d\n", mode);
			fflush(stdout);
			ret = write_sysfs_int(SYSFS_PATH, ATTR_MODE, mode);
			if (ret) {
				_dprintf("Failed to set mode\n");
				fflush(stdout);
				return 1;
			};
		}

		if (frequency != -1) {
			_dprintf(">Set frequency to %d\n", frequency);
			fflush(stdout);
			ret = write_sysfs_int(SYSFS_PATH, ATTR_SAMPLE_FREQUENCY, frequency);
			if (ret) {
				_dprintf("Failed to set frequency\n");
				fflush(stdout);
				return 1;
			};
		}
		
		
		if (write_sysfs_int(SYSFS_PATH, ATTR_SCAN_ELEMENT, 1)) {
			_dprintf("Failed to set %s%d/%s", SYSFS_PATH, device_id, ATTR_SCAN_ELEMENT);
			fflush(stdout);
			return 1;
		}
		usleep(200000);

		if (write_sysfs_int(SYSFS_PATH, ATTR_BUFFER_LENGTH, 65536)) {
			_dprintf("Failed to set %s%d/%s", SYSFS_PATH, device_id, ATTR_BUFFER_LENGTH);
			fflush(stdout);
			return 1;
		}
		usleep(200000);

		if (write_sysfs_int(SYSFS_PATH, ATTR_BUFFER, 1)) {
			_dprintf("Failed to set %s%d/%s", SYSFS_PATH, device_id, ATTR_BUFFER);
			fflush(stdout);
			return 1;
		}
		usleep(200000);

		/* input devices */
		snprintf(dev_path, sizeof(dev_path), "%s%d", DEV_NAME_SENSOR, device_id);

		fd = open(dev_path, O_RDONLY | O_NONBLOCK);
		if (fd < 0) {
			_dprintf("Failed to open %s\n", dev_path);
			return 1;
		}


		printf(">Start\n");
		fflush(stdout);

		/* show chip name */
		ret = show_chip_name(SYSFS_PATH);
		if (ret) {
			return 1;
		}

		usleep(10000);

		fds[0].fd = fd;
		fds[0].events = POLLIN;// POLLPRI | POLLIN;
		fds[0].revents = 0;


		/* dummy read */
		int i = 0;
		int total = 0;
		char read_buf[32] = {0,};
		
		/* system uptime */
		struct sysinfo info;
		if (sysinfo(&info)) stop_prog();
		long target_time = info.uptime + timeout;
				
		/* poll loop */
		while(info.uptime < target_time) {
			ret = poll(fds, 2, 3);
			if (ret > 0) {
				if (fds[0].events & POLLIN) {
					num = read(fd, buf, 8);
					if (num <= 0) {
						//	printf("No data from buf\n");
						continue;
					}

					if (total + num < 16) {
						memcpy(read_buf + total, buf, num);
						total+= (int)num;
					} else {
						if(total + num == 16) {
							memcpy(read_buf + total, buf, num);
							handle_one_packet((struct sensor_data *)read_buf);
							total = 0;
							memset(read_buf, '\0', 32);
						}  
					}
				}
			}
			sysinfo(&info); // refresh system uptime
		}
	}
	
	return stop_prog();
}
