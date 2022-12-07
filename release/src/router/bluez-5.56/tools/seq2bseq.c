// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2013  Intel Corporation
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <sys/stat.h>

static int convert_line(int fd, const char *line)
{
	const char *ptr = line;
	char str[3];
	unsigned char val;

	if (line[0] == '*' || line[0] == '\r' || line[0] == '\n')
		return 0;

	while (1) {
		str[0] = *ptr++;
		str[1] = *ptr++;
		str[2] = '\0';

		val = strtol(str, NULL, 16);

		if (write(fd, &val, 1) < 0)
			return -errno;

		if (*ptr == '\r' || *ptr == '\n')
			break;

		while (*ptr == ' ')
			ptr++;
	}

	return 0;
}

static void convert_file(const char *input_path, const char *output_path)
{
	size_t line_size = 1024;
	char line_buffer[line_size];
	char *path;
	const char *ptr;
	FILE *fp;
	struct stat st;
	off_t cur = 0;
	int fd;

	if (output_path) {
		path = strdup(output_path);
		if (!path) {
			perror("Failed to allocate string");
			return;
		}
	} else {
		ptr = strrchr(input_path, '.');
		if (ptr) {
			path = malloc(ptr - input_path + 6);
			if (!path) {
				perror("Failed to allocate string");
				return;
			}
			strncpy(path, input_path, ptr - input_path);
			strcpy(path + (ptr - input_path), ".bseq");
		} else {
			if (asprintf(&path, "%s.bseq", input_path) < 0) {
				perror("Failed to allocate string");
				return;
			}
		}
	}

	printf("Converting %s to %s\n", input_path, path);

	fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);

	free(path);

	if (fd < 0) {
		perror("Failed to create output file");
		return;
	}

	if (stat(input_path, &st) < 0) {
		fprintf(stderr, "Failed get file size\n");
		close(fd);
		return;
	}

	if (st.st_size == 0) {
		fprintf(stderr, "Empty file\n");
		close(fd);
		return;
	}

	fp = fopen(input_path, "r");
	if (!fp) {
		fprintf(stderr, "Failed to open input file\n");
		close(fd);
		return;
	}

	while (1) {
		char *str;
		int err;

		str = fgets(line_buffer, line_size - 1, fp);
		if (!str)
			break;

		cur += strlen(str);

		err = convert_line(fd, str);
		if (err < 0) {
			fprintf(stderr, "Failed to convert file (%s)\n",
								strerror(-err));
			break;
		}
	}

	fclose(fp);

	close(fd);
}

static void usage(void)
{
	printf("Intel Bluetooth firmware converter\n"
		"Usage:\n");
	printf("\tseq2bseq [options] <file>\n");
	printf("Options:\n"
		"\t-o, --output <file>    Provide firmware output file\n"
		"\t-h, --help             Show help options\n");
}

static const struct option main_options[] = {
	{ "output",  required_argument, NULL, 'o' },
	{ "version", no_argument,       NULL, 'v' },
	{ "help",    no_argument,       NULL, 'h' },
	{ }
};

int main(int argc, char *argv[])
{
	const char *output_path = NULL;
	int i;

	for (;;) {
		int opt;

		opt = getopt_long(argc, argv, "o:vh", main_options, NULL);
		if (opt < 0)
			break;

		switch (opt) {
		case 'o':
			output_path = optarg;
			break;
		case 'v':
			printf("%s\n", VERSION);
			return EXIT_SUCCESS;
		case 'h':
			usage();
			return EXIT_SUCCESS;
		default:
			return EXIT_FAILURE;
		}
	}

	if (argc - optind < 1) {
		fprintf(stderr, "No input firmware files provided\n");
		return EXIT_FAILURE;
	}

	if (output_path && argc - optind > 1) {
		fprintf(stderr, "Only single input firmware supported\n");
		return EXIT_FAILURE;
	}

	for (i = optind; i < argc; i++)
		convert_file(argv[i], output_path);

	return EXIT_SUCCESS;
}
