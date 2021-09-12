/*
 * This tool creates a frame pattern data for debug purpose used by
 * test_qrspec. test_qrspec and create_frame_pattern uses the same function
 * of libqrencode. This means the test is meaningless if test_qrspec is run
 * with a pattern data created by create_frame_pattern of the same version.
 * In order to test it correctly, create a pattern data by the tool of the
 * previous version, or use the frame data attached to the package.
 */

#include <stdio.h>
#include <string.h>
#include <png.h>
#include "common.h"
#include "../qrspec.h"

void append_pattern(int version, FILE *fp)
{
	int width;
	unsigned char *frame;

	frame = QRspec_newFrame(version);
	width = QRspec_getWidth(version);
	fwrite(frame, 1, width * width, fp);
	free(frame);
}

static int writePNG(unsigned char *frame, int width, const char *outfile)
{
	static FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned char *row, *p, *q;
	int x, y, xx, yy, bit;
	int realwidth;
	const int margin = 0;
	const int size = 1;

	realwidth = (width + margin * 2) * size;
	row = (unsigned char *)malloc((realwidth + 7) / 8);
	if(row == NULL) {
		fprintf(stderr, "Failed to allocate memory.\n");
		exit(EXIT_FAILURE);
	}

	if(outfile[0] == '-' && outfile[1] == '\0') {
		fp = stdout;
	} else {
		fp = fopen(outfile, "wb");
		if(fp == NULL) {
			fprintf(stderr, "Failed to create file: %s\n", outfile);
			perror(NULL);
			exit(EXIT_FAILURE);
		}
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(png_ptr == NULL) {
		fclose(fp);
		fprintf(stderr, "Failed to initialize PNG writer.\n");
		exit(EXIT_FAILURE);
	}

	info_ptr = png_create_info_struct(png_ptr);
	if(info_ptr == NULL) {
		fclose(fp);
		fprintf(stderr, "Failed to initialize PNG write.\n");
		exit(EXIT_FAILURE);
	}

	if(setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		fprintf(stderr, "Failed to write PNG image.\n");
		exit(EXIT_FAILURE);
	}

	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr,
			realwidth, realwidth,
			1,
			PNG_COLOR_TYPE_GRAY,
			PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);

	/* top margin */
	memset(row, 0xff, (realwidth + 7) / 8);
	for(y=0; y<margin * size; y++) {
		png_write_row(png_ptr, row);
	}

	/* data */
	p = frame;
	for(y=0; y<width; y++) {
		bit = 7;
		memset(row, 0xff, (realwidth + 7) / 8);
		q = row;
		q += margin * size / 8;
		bit = 7 - (margin * size % 8);
		for(x=0; x<width; x++) {
			for(xx=0; xx<size; xx++) {
				*q ^= (*p & 1) << bit;
				bit--;
				if(bit < 0) {
					q++;
					bit = 7;
				}
			}
			p++;
		}
		for(yy=0; yy<size; yy++) {
			png_write_row(png_ptr, row);
		}
	}
	/* bottom margin */
	memset(row, 0xff, (realwidth + 7) / 8);
	for(y=0; y<margin * size; y++) {
		png_write_row(png_ptr, row);
	}

	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(fp);
	free(row);

	return 0;
}

void write_pattern_image(int version, const char *filename)
{
	int width;
	unsigned char *frame;
	static char str[256];

	frame = QRspec_newFrame(version);
	width = QRspec_getWidth(version);

	snprintf(str, 256, "%s-%d.png", filename, version);
	writePNG(frame, width, str);
	free(frame);
}

void write_pattern(const char *filename)
{
	FILE *fp;
	int i;

	fp = fopen(filename, "wb");
	if(fp == NULL) {
		perror("Failed to open a file to write:");
		abort();
	}
	for(i=1; i<=QRSPEC_VERSION_MAX; i++) {
		append_pattern(i, fp);
		write_pattern_image(i, filename);
	}
	fclose(fp);
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		printf("Create empty frame patterns.\nUsage: %s FILENAME\n", argv[0]);
		exit(0);
	}
	write_pattern(argv[1]);
	return 0;
}
