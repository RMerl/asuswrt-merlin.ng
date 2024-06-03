/*
** Easylogo TGA->header converter
** ==============================
** (C) 2000 by Paolo Scaffardi (arsenio@tin.it)
** AIRVENT SAM s.p.a - RIMINI(ITALY)
** (C) 2007-2008 Mike Frysinger <vapier@gentoo.org>
**
** This is still under construction!
*/

#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#pragma pack(1)

/*#define ENABLE_ASCII_BANNERS */

typedef struct {
	unsigned char id;
	unsigned char ColorMapType;
	unsigned char ImageTypeCode;
	unsigned short ColorMapOrigin;
	unsigned short ColorMapLenght;
	unsigned char ColorMapEntrySize;
	unsigned short ImageXOrigin;
	unsigned short ImageYOrigin;
	unsigned short ImageWidth;
	unsigned short ImageHeight;
	unsigned char ImagePixelSize;
	unsigned char ImageDescriptorByte;
} tga_header_t;

typedef struct {
	unsigned char r, g, b;
} rgb_t;

typedef struct {
	unsigned char b, g, r;
} bgr_t;

typedef struct {
	unsigned char Cb, y1, Cr, y2;
} yuyv_t;

typedef struct {
	void *data, *palette;
	int width, height, pixels, bpp, pixel_size, size, palette_size, yuyv;
} image_t;

void *xmalloc (size_t size)
{
	void *ret = malloc (size);
	if (!ret) {
		fprintf (stderr, "\nerror: malloc(%zu) failed: %s",
			size, strerror(errno));
		exit (1);
	}
	return ret;
}

void StringUpperCase (char *str)
{
	int count = strlen (str);
	char c;

	while (count--) {
		c = *str;
		if ((c >= 'a') && (c <= 'z'))
			*str = 'A' + (c - 'a');
		str++;
	}
}

void StringLowerCase (char *str)
{
	int count = strlen (str);
	char c;

	while (count--) {
		c = *str;
		if ((c >= 'A') && (c <= 'Z'))
			*str = 'a' + (c - 'A');
		str++;
	}
}
void pixel_rgb_to_yuyv (rgb_t * rgb_pixel, yuyv_t * yuyv_pixel)
{
	unsigned int pR, pG, pB;

	/* Transform (0-255) components to (0-100) */
	pR = rgb_pixel->r * 100 / 255;
	pG = rgb_pixel->g * 100 / 255;
	pB = rgb_pixel->b * 100 / 255;

	/* Calculate YUV values (0-255) from RGB beetween 0-100 */
	yuyv_pixel->y1 = yuyv_pixel->y2 = 209 * (pR + pG + pB) / 300 + 16;
	yuyv_pixel->Cb = pB - (pR / 4) - (pG * 3 / 4) + 128;
	yuyv_pixel->Cr = pR - (pG * 3 / 4) - (pB / 4) + 128;

	return;
}

void printlogo_rgb (rgb_t * data, int w, int h)
{
	int x, y;

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++, data++)
			if ((data->r <
			     30) /*&&(data->g == 0)&&(data->b == 0) */ )
				printf (" ");
			else
				printf ("X");
		printf ("\n");
	}
}

void printlogo_yuyv (unsigned short *data, int w, int h)
{
	int x, y;

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++, data++)
			if (*data == 0x1080)	/* Because of inverted on i386! */
				printf (" ");
			else
				printf ("X");
		printf ("\n");
	}
}

static inline unsigned short le16_to_cpu (unsigned short val)
{
	union {
		unsigned char pval[2];
		unsigned short val;
	} swapped;

	swapped.val = val;
	return (swapped.pval[1] << 8) + swapped.pval[0];
}

int image_load_tga (image_t * image, char *filename)
{
	FILE *file;
	tga_header_t header;
	int i;
	unsigned char app;
	rgb_t *p;

	if ((file = fopen (filename, "rb")) == NULL)
		return -1;

	fread (&header, sizeof (header), 1, file);

	/* byte swap: tga is little endian, host is ??? */
	header.ColorMapOrigin = le16_to_cpu (header.ColorMapOrigin);
	header.ColorMapLenght = le16_to_cpu (header.ColorMapLenght);
	header.ImageXOrigin = le16_to_cpu (header.ImageXOrigin);
	header.ImageYOrigin = le16_to_cpu (header.ImageYOrigin);
	header.ImageWidth = le16_to_cpu (header.ImageWidth);
	header.ImageHeight = le16_to_cpu (header.ImageHeight);

	image->width = header.ImageWidth;
	image->height = header.ImageHeight;

	switch (header.ImageTypeCode) {
	case 2:		/* Uncompressed RGB */
		image->yuyv = 0;
		image->palette_size = 0;
		image->palette = NULL;
		break;

	default:
		printf ("Format not supported!\n");
		return -1;
	}

	image->bpp = header.ImagePixelSize;
	image->pixel_size = ((image->bpp - 1) / 8) + 1;
	image->pixels = image->width * image->height;
	image->size = image->pixels * image->pixel_size;
	image->data = xmalloc (image->size);

	if (image->bpp != 24) {
		printf ("Bpp not supported: %d!\n", image->bpp);
		return -1;
	}

	fread (image->data, image->size, 1, file);

/* Swapping R and B values */

	p = image->data;
	for (i = 0; i < image->pixels; i++, p++) {
		app = p->r;
		p->r = p->b;
		p->b = app;
	}

/* Swapping image */

	if (!(header.ImageDescriptorByte & 0x20)) {
		unsigned char *temp = xmalloc (image->size);
		int linesize = image->pixel_size * image->width;
		void *dest = image->data,
			*source = temp + image->size - linesize;

		printf ("S");
		if (temp == NULL) {
			printf ("Cannot alloc temp buffer!\n");
			return -1;
		}

		memcpy (temp, image->data, image->size);
		for (i = 0; i < image->height;
		     i++, dest += linesize, source -= linesize)
			memcpy (dest, source, linesize);

		free (temp);
	}
#ifdef ENABLE_ASCII_BANNERS
	printlogo_rgb (image->data, image->width, image->height);
#endif

	fclose (file);
	return 0;
}

void image_free (image_t * image)
{
	free (image->data);
	free (image->palette);
}

int image_rgb_to_yuyv (image_t * rgb_image, image_t * yuyv_image)
{
	rgb_t *rgb_ptr = (rgb_t *) rgb_image->data;
	yuyv_t yuyv;
	unsigned short *dest;
	int count = 0;

	yuyv_image->pixel_size = 2;
	yuyv_image->bpp = 16;
	yuyv_image->yuyv = 1;
	yuyv_image->width = rgb_image->width;
	yuyv_image->height = rgb_image->height;
	yuyv_image->pixels = yuyv_image->width * yuyv_image->height;
	yuyv_image->size = yuyv_image->pixels * yuyv_image->pixel_size;
	dest = (unsigned short *) (yuyv_image->data =
				   xmalloc (yuyv_image->size));
	yuyv_image->palette = 0;
	yuyv_image->palette_size = 0;

	while ((count++) < rgb_image->pixels) {
		pixel_rgb_to_yuyv (rgb_ptr++, &yuyv);

		if ((count & 1) == 0)	/* Was == 0 */
			memcpy (dest, ((void *) &yuyv) + 2, sizeof (short));
		else
			memcpy (dest, (void *) &yuyv, sizeof (short));

		dest++;
	}

#ifdef ENABLE_ASCII_BANNERS
	printlogo_yuyv (yuyv_image->data, yuyv_image->width,
			yuyv_image->height);
#endif
	return 0;
}

int image_rgb888_to_rgb565(image_t *rgb888_image, image_t *rgb565_image)
{
	rgb_t *rgb_ptr = (rgb_t *) rgb888_image->data;
	unsigned short *dest;
	int count = 0;

	rgb565_image->pixel_size = 2;
	rgb565_image->bpp = 16;
	rgb565_image->yuyv = 0;
	rgb565_image->width = rgb888_image->width;
	rgb565_image->height = rgb888_image->height;
	rgb565_image->pixels = rgb565_image->width * rgb565_image->height;
	rgb565_image->size = rgb565_image->pixels * rgb565_image->pixel_size;
	dest = (unsigned short *) (rgb565_image->data =
				   xmalloc(rgb565_image->size));
	rgb565_image->palette = 0;
	rgb565_image->palette_size = 0;

	while ((count++) < rgb888_image->pixels) {

		*dest++ = ((rgb_ptr->b & 0xF8) << 8) |
			((rgb_ptr->g & 0xFC) << 3) |
			(rgb_ptr->r >> 3);
		rgb_ptr++;
	}

	return 0;
}

enum comp_t {
	COMP_NONE,
	COMP_GZIP,
	COMP_LZMA,
};
static enum comp_t compression = COMP_NONE;
static bool bss_storage = false;

int image_save_header (image_t * image, char *filename, char *varname)
{
	FILE *file = fopen (filename, "w");
	char app[256], str[256] = "", def_name[64];
	int count = image->size, col = 0;
	unsigned char *dataptr = image->data;

	if (file == NULL)
		return -1;

	/*  Author information */
	fprintf (file,
		 "/*\n * Generated by EasyLogo, (C) 2000 by Paolo Scaffardi\n *\n");
	fprintf (file,
		 " * To use this, include it and call: easylogo_plot(screen,&%s, width,x,y)\n *\n",
		 varname);
	fprintf (file,
		 " * Where:\t'screen'\tis the pointer to the frame buffer\n");
	fprintf (file, " *\t\t'width'\tis the screen width\n");
	fprintf (file, " *\t\t'x'\t\tis the horizontal position\n");
	fprintf (file, " *\t\t'y'\t\tis the vertical position\n */\n\n");

	/* image compress */
	if (compression != COMP_NONE) {
		const char *errstr = NULL;
		unsigned char *compressed;
		const char *comp_name;
		struct stat st;
		FILE *compfp;
		size_t filename_len = strlen(filename);
		char *compfilename = xmalloc(filename_len + 20);
		char *compcmd = xmalloc(filename_len + 50);

		sprintf(compfilename, "%s.bin", filename);
		switch (compression) {
		case COMP_GZIP:
			strcpy(compcmd, "gzip");
			comp_name = "GZIP";
			break;
		case COMP_LZMA:
			strcpy(compcmd, "lzma");
			comp_name = "LZMA";
			break;
		default:
			errstr = "\nerror: unknown compression method";
			goto done;
		}
		strcat(compcmd, " > ");
		strcat(compcmd, compfilename);
		compfp = popen(compcmd, "w");
		if (!compfp) {
			errstr = "\nerror: popen() failed";
			goto done;
		}
		if (fwrite(image->data, image->size, 1, compfp) != 1) {
			errstr = "\nerror: writing data to gzip failed";
			goto done;
		}
		if (pclose(compfp)) {
			errstr = "\nerror: gzip process failed";
			goto done;
		}

		compfp = fopen(compfilename, "r");
		if (!compfp) {
			errstr = "\nerror: open() on gzip data failed";
			goto done;
		}
		if (stat(compfilename, &st)) {
			errstr = "\nerror: stat() on gzip file failed";
			goto done;
		}
		compressed = xmalloc(st.st_size);
		if (fread(compressed, st.st_size, 1, compfp) != 1) {
			errstr = "\nerror: reading gzip data failed";
			goto done;
		}
		fclose(compfp);

		unlink(compfilename);

		dataptr = compressed;
		count = st.st_size;
		fprintf(file, "#define EASYLOGO_ENABLE_%s %i\n\n", comp_name, count);
		if (bss_storage)
			fprintf (file, "static unsigned char EASYLOGO_DECOMP_BUFFER[%i];\n\n", image->size);

 done:
		free(compfilename);
		free(compcmd);

		if (errstr) {
			perror (errstr);
			return -1;
		}
	}

	/*	Headers */
	fprintf (file, "#include <video_easylogo.h>\n\n");
	/*	Macros */
	strcpy (def_name, varname);
	StringUpperCase (def_name);
	fprintf (file, "#define	DEF_%s_WIDTH\t\t%d\n", def_name,
		 image->width);
	fprintf (file, "#define	DEF_%s_HEIGHT\t\t%d\n", def_name,
		 image->height);
	fprintf (file, "#define	DEF_%s_PIXELS\t\t%d\n", def_name,
		 image->pixels);
	fprintf (file, "#define	DEF_%s_BPP\t\t%d\n", def_name, image->bpp);
	fprintf (file, "#define	DEF_%s_PIXEL_SIZE\t%d\n", def_name,
		 image->pixel_size);
	fprintf (file, "#define	DEF_%s_SIZE\t\t%d\n\n", def_name,
		 image->size);
	/*  Declaration */
	fprintf (file, "unsigned char DEF_%s_DATA[] = {\n",
		 def_name);

	/*	Data */
	while (count)
		switch (col) {
		case 0:
			sprintf (str, " 0x%02x", *dataptr++);
			col++;
			count--;
			break;

		case 16:
			fprintf (file, "%s", str);
			if (count > 0)
				fprintf (file, ",");
			fprintf (file, "\n");

			col = 0;
			break;

		default:
			strcpy (app, str);
			sprintf (str, "%s, 0x%02x", app, *dataptr++);
			col++;
			count--;
			break;
		}

	if (col)
		fprintf (file, "%s\n", str);

	/*	End of declaration */
	fprintf (file, "};\n\n");
	/*	Variable */
	fprintf (file, "fastimage_t %s = {\n", varname);
	fprintf (file, "		DEF_%s_DATA,\n", def_name);
	fprintf (file, "		DEF_%s_WIDTH,\n", def_name);
	fprintf (file, "		DEF_%s_HEIGHT,\n", def_name);
	fprintf (file, "		DEF_%s_BPP,\n", def_name);
	fprintf (file, "		DEF_%s_PIXEL_SIZE,\n", def_name);
	fprintf (file, "		DEF_%s_SIZE\n};\n", def_name);

	fclose (file);

	return 0;
}

#define DEF_FILELEN	256

static void usage (int exit_status)
{
	puts (
		"EasyLogo 1.0 (C) 2000 by Paolo Scaffardi\n"
		"\n"
		"Syntax:	easylogo [options] inputfile [outputvar [outputfile]]\n"
		"\n"
		"Options:\n"
		"  -r     Output RGB888 instead of YUYV\n"
		"  -s     Output RGB565 instead of YUYV\n"
		"  -g     Compress with gzip\n"
		"  -l     Compress with lzma\n"
		"  -b     Preallocate space in bss for decompressing image\n"
		"  -h     Help output\n"
		"\n"
		"Where: 'inputfile'   is the TGA image to load\n"
		"       'outputvar'   is the variable name to create\n"
		"       'outputfile'  is the output header file (default is 'inputfile.h')"
	);
	exit (exit_status);
}

int main (int argc, char *argv[])
{
	int c;
	bool use_rgb888 = false;
	bool use_rgb565 = false;
	char inputfile[DEF_FILELEN],
		outputfile[DEF_FILELEN], varname[DEF_FILELEN];

	image_t rgb888_logo, rgb565_logo, yuyv_logo;

	while ((c = getopt(argc, argv, "hrsglb")) > 0) {
		switch (c) {
		case 'h':
			usage (0);
			break;
		case 'r':
			use_rgb888 = true;
			puts("Using 24-bit RGB888 Output Fromat");
			break;
		case 's':
			use_rgb565 = true;
			puts("Using 16-bit RGB565 Output Fromat");
			break;
		case 'g':
			compression = COMP_GZIP;
			puts("Compressing with gzip");
			break;
		case 'l':
			compression = COMP_LZMA;
			puts("Compressing with lzma");
			break;
		case 'b':
			bss_storage = true;
			puts("Preallocating bss space for decompressing image");
			break;
		default:
			usage (1);
			break;
		}
	}

	c = argc - optind;
	if (c > 4 || c < 1)
		usage (1);

	strcpy (inputfile, argv[optind]);

	if (c > 1)
		strcpy (varname, argv[optind + 1]);
	else {
		/* transform "input.tga" to just "input" */
		char *dot;
		strcpy (varname, inputfile);
		dot = strchr (varname, '.');
		if (dot)
			*dot = '\0';
	}

	if (c > 2)
		strcpy (outputfile, argv[optind + 2]);
	else {
		/* just append ".h" to input file name */
		strcpy (outputfile, inputfile);
		strcat (outputfile, ".h");
	}

	/* Make sure the output is sent as soon as we printf() */
	setbuf(stdout, NULL);

	printf ("Doing '%s' (%s) from '%s'...",
		outputfile, varname, inputfile);

	/* Import TGA logo */

	printf ("L");
	if (image_load_tga(&rgb888_logo, inputfile) < 0) {
		printf ("input file not found!\n");
		exit (1);
	}

	/* Convert, save, and free the image */

	if (!use_rgb888 && !use_rgb565) {
		printf ("C");
		image_rgb_to_yuyv(&rgb888_logo, &yuyv_logo);

		printf("S");
		image_save_header(&yuyv_logo, outputfile, varname);
		image_free(&yuyv_logo);
	} else if (use_rgb565) {
		printf("C");
		image_rgb888_to_rgb565(&rgb888_logo, &rgb565_logo);

		printf("S");
		image_save_header(&rgb565_logo, outputfile, varname);
		image_free(&rgb565_logo);
	} else {
		printf("S");
		image_save_header(&rgb888_logo, outputfile, varname);
	}

	/* Free original image and copy */

	image_free(&rgb888_logo);

	printf ("\n");

	return 0;
}
