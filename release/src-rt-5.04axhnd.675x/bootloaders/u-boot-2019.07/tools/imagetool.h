/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 *
 * Written by Guilherme Maciel Ferreira <guilherme.maciel.ferreira@gmail.com>
 */

#ifndef _IMAGETOOL_H_
#define _IMAGETOOL_H_

#include "os_support.h"
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <u-boot/sha1.h>

#include "fdt_host.h"

#define ARRAY_SIZE(x)		(sizeof(x) / sizeof((x)[0]))

#define IH_ARCH_DEFAULT		IH_ARCH_INVALID

/* Information about a file that needs to be placed into the FIT */
struct content_info {
	struct content_info *next;
	int type;		/* File type (IH_TYPE_...) */
	const char *fname;
};

/*
 * This structure defines all such variables those are initialized by
 * mkimage and dumpimage main core and need to be referred by image
 * type specific functions
 */
struct image_tool_params {
	int dflag;
	int eflag;
	int fflag;
	int iflag;
	int lflag;
	int pflag;
	int vflag;
	int xflag;
	int skipcpy;
	int os;
	int arch;
	int type;
	int comp;
	char *dtc;
	unsigned int addr;
	unsigned int ep;
	char *imagename;
	char *imagename2;
	char *datafile;
	char *imagefile;
	char *cmdname;
	const char *outfile;	/* Output filename */
	const char *keydir;	/* Directory holding private keys */
	const char *keydest;	/* Destination .dtb for public key */
	const char *comment;	/* Comment to add to signature node */
	int require_keys;	/* 1 to mark signing keys as 'required' */
	int file_size;		/* Total size of output file */
	int orig_file_size;	/* Original size for file before padding */
	bool auto_its;		/* Automatically create the .its file */
	int fit_image_type;	/* Image type to put into the FIT */
	char *fit_ramdisk;	/* Ramdisk file to include */
	struct content_info *content_head;	/* List of files to include */
	struct content_info *content_tail;
	bool external_data;	/* Store data outside the FIT */
	bool quiet;		/* Don't output text in normal operation */
	unsigned int external_offset;	/* Add padding to external data */
	const char *engine_id;	/* Engine to use for signing */
};

/*
 * image type specific variables and callback functions
 */
struct image_type_params {
	/* name is an identification tag string for added support */
	char *name;
	/*
	 * header size is local to the specific image type to be supported,
	 * mkimage core treats this as number of bytes
	 */
	uint32_t header_size;
	/* Image type header pointer */
	void *hdr;
	/*
	 * There are several arguments that are passed on the command line
	 * and are registered as flags in image_tool_params structure.
	 * This callback function can be used to check the passed arguments
	 * are in-lined with the image type to be supported
	 *
	 * Returns 1 if parameter check is successful
	 */
	int (*check_params) (struct image_tool_params *);
	/*
	 * This function is used by list command (i.e. mkimage -l <filename>)
	 * image type verification code must be put here
	 *
	 * Returns 0 if image header verification is successful
	 * otherwise, returns respective negative error codes
	 */
	int (*verify_header) (unsigned char *, int, struct image_tool_params *);
	/* Prints image information abstracting from image header */
	void (*print_header) (const void *);
	/*
	 * The header or image contents need to be set as per image type to
	 * be generated using this callback function.
	 * further output file post processing (for ex. checksum calculation,
	 * padding bytes etc..) can also be done in this callback function.
	 */
	void (*set_header) (void *, struct stat *, int,
					struct image_tool_params *);
	/*
	 * This function is used by the command to retrieve a component
	 * (sub-image) from the image (i.e. dumpimage -i <image> -p <position>
	 * <sub-image-name>).
	 * Thus the code to extract a file from an image must be put here.
	 *
	 * Returns 0 if the file was successfully retrieved from the image,
	 * or a negative value on error.
	 */
	int (*extract_subimage)(void *, struct image_tool_params *);
	/*
	 * Some image generation support for ex (default image type) supports
	 * more than one type_ids, this callback function is used to check
	 * whether input (-T <image_type>) is supported by registered image
	 * generation/list low level code
	 */
	int (*check_image_type) (uint8_t);
	/* This callback function will be executed if fflag is defined */
	int (*fflag_handle) (struct image_tool_params *);
	/*
	 * This callback function will be executed for variable size record
	 * It is expected to build this header in memory and return its length
	 * and a pointer to it by using image_type_params.header_size and
	 * image_type_params.hdr. The return value shall indicate if an
	 * additional padding should be used when copying the data image
	 * by returning the padding length.
	 */
	int (*vrec_header) (struct image_tool_params *,
		struct image_type_params *);
};

/**
 * imagetool_get_type() - find the image type params for a given image type
 *
 * It scans all registers image type supports
 * checks the input type for each supported image type
 *
 * if successful,
 *     returns respective image_type_params pointer if success
 * if input type_id is not supported by any of image_type_support
 *     returns NULL
 */
struct image_type_params *imagetool_get_type(int type);

/*
 * imagetool_verify_print_header() - verifies the image header
 *
 * Scan registered image types and verify the image_header for each
 * supported image type. If verification is successful, this prints
 * the respective header.
 *
 * @return 0 on success, negative if input image format does not match with
 * any of supported image types
 */
int imagetool_verify_print_header(
	void *ptr,
	struct stat *sbuf,
	struct image_type_params *tparams,
	struct image_tool_params *params);

/*
 * imagetool_verify_print_header_by_type() - verifies the image header
 *
 * Verify the image_header for the image type given by tparams.
 * If verification is successful, this prints the respective header.
 * @ptr: pointer the the image header
 * @sbuf: stat information about the file pointed to by ptr
 * @tparams: image type parameters
 * @params: mkimage parameters
 *
 * @return 0 on success, negative if input image format does not match with
 * the given image type
 */
int imagetool_verify_print_header_by_type(
	void *ptr,
	struct stat *sbuf,
	struct image_type_params *tparams,
	struct image_tool_params *params);

/**
 * imagetool_save_subimage - store data into a file
 * @file_name: name of the destination file
 * @file_data: data to be written
 * @file_len: the amount of data to store
 *
 * imagetool_save_subimage() store file_len bytes of data pointed by file_data
 * into the file name by file_name.
 *
 * returns:
 *     zero in case of success or a negative value if fail.
 */
int imagetool_save_subimage(
	const char *file_name,
	ulong file_data,
	ulong file_len);

/**
 * imagetool_get_filesize() - Utility function to obtain the size of a file
 *
 * This function prints a message if an error occurs, showing the error that
 * was obtained.
 *
 * @params:	mkimage parameters
 * @fname:	filename to check
 * @return size of file, or -ve value on error
 */
int imagetool_get_filesize(struct image_tool_params *params, const char *fname);

/**
 * imagetool_get_source_date() - Get timestamp for build output.
 *
 * Gets a timestamp for embedding it in a build output. If set
 * SOURCE_DATE_EPOCH is used. Else the given fallback value is returned. Prints
 * an error message if SOURCE_DATE_EPOCH contains an invalid value and returns
 * 0.
 *
 * @cmdname:	command name
 * @fallback:	timestamp to use if SOURCE_DATE_EPOCH isn't set
 * @return timestamp based on SOURCE_DATE_EPOCH
 */
time_t imagetool_get_source_date(
	const char *cmdname,
	time_t fallback);

/*
 * There is a c file associated with supported image type low level code
 * for ex. default_image.c, fit_image.c
 */


void pbl_load_uboot(int fd, struct image_tool_params *mparams);
int zynqmpbif_copy_image(int fd, struct image_tool_params *mparams);
int imx8image_copy_image(int fd, struct image_tool_params *mparams);
int imx8mimage_copy_image(int fd, struct image_tool_params *mparams);

#define ___cat(a, b) a ## b
#define __cat(a, b) ___cat(a, b)

/* we need some special handling for this host tool running eventually on
 * Darwin. The Mach-O section handling is a bit different than ELF section
 * handling. The differnces in detail are:
 *  a) we have segments which have sections
 *  b) we need a API call to get the respective section symbols */
#if defined(__MACH__)
#include <mach-o/getsect.h>

#define INIT_SECTION(name)  do {					\
		unsigned long name ## _len;				\
		char *__cat(pstart_, name) = getsectdata("__TEXT",	\
			#name, &__cat(name, _len));			\
		char *__cat(pstop_, name) = __cat(pstart_, name) +	\
			__cat(name, _len);				\
		__cat(__start_, name) = (void *)__cat(pstart_, name);	\
		__cat(__stop_, name) = (void *)__cat(pstop_, name);	\
	} while (0)
#define SECTION(name)   __attribute__((section("__TEXT, " #name)))

struct image_type_params **__start_image_type, **__stop_image_type;
#else
#define INIT_SECTION(name) /* no-op for ELF */
#define SECTION(name)   __attribute__((section(#name)))

/* We construct a table of pointers in an ELF section (pointers generally
 * go unpadded by gcc).  ld creates boundary syms for us. */
extern struct image_type_params *__start_image_type[], *__stop_image_type[];
#endif /* __MACH__ */

#if !defined(__used)
# if __GNUC__ == 3 && __GNUC_MINOR__ < 3
#  define __used			__attribute__((__unused__))
# else
#  define __used			__attribute__((__used__))
# endif
#endif

#define U_BOOT_IMAGE_TYPE( \
		_id, \
		_name, \
		_header_size, \
		_header, \
		_check_params, \
		_verify_header, \
		_print_header, \
		_set_header, \
		_extract_subimage, \
		_check_image_type, \
		_fflag_handle, \
		_vrec_header \
	) \
	static struct image_type_params __cat(image_type_, _id) = \
	{ \
		.name = _name, \
		.header_size = _header_size, \
		.hdr = _header, \
		.check_params = _check_params, \
		.verify_header = _verify_header, \
		.print_header = _print_header, \
		.set_header = _set_header, \
		.extract_subimage = _extract_subimage, \
		.check_image_type = _check_image_type, \
		.fflag_handle = _fflag_handle, \
		.vrec_header = _vrec_header \
	}; \
	static struct image_type_params *SECTION(image_type) __used \
		__cat(image_type_ptr_, _id) = &__cat(image_type_, _id)

#endif /* _IMAGETOOL_H_ */
