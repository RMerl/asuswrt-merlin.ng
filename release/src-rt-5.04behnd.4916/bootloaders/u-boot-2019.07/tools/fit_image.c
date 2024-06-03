// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2004
 * DENX Software Engineering
 * Wolfgang Denk, wd@denx.de
 *
 * Updated-by: Prafulla Wadaskar <prafulla@marvell.com>
 *		FIT image specific code abstracted from mkimage.c
 *		some functions added to address abstraction
 *
 * All rights reserved.
 */

#include "imagetool.h"
#include "fit_common.h"
#include "mkimage.h"
#include <image.h>
#include <stdarg.h>
#include <version.h>
#include <u-boot/crc.h>

static image_header_t header;

static int fit_add_file_data(struct image_tool_params *params, size_t size_inc,
			     const char *tmpfile)
{
	int tfd, destfd = 0;
	void *dest_blob = NULL;
	off_t destfd_size = 0;
	struct stat sbuf;
	void *ptr;
	int ret = 0;

	tfd = mmap_fdt(params->cmdname, tmpfile, size_inc, &ptr, &sbuf, true,
		       false);
	if (tfd < 0)
		return -EIO;

	if (params->keydest) {
		struct stat dest_sbuf;

		destfd = mmap_fdt(params->cmdname, params->keydest, size_inc,
				  &dest_blob, &dest_sbuf, false,
				  false);
		if (destfd < 0) {
			ret = -EIO;
			goto err_keydest;
		}
		destfd_size = dest_sbuf.st_size;
	}

	/* for first image creation, add a timestamp at offset 0 i.e., root  */
	if (params->datafile) {
		time_t time = imagetool_get_source_date(params->cmdname,
							sbuf.st_mtime);
		ret = fit_set_timestamp(ptr, 0, time);
	}

	if (!ret) {
		ret = fit_add_verification_data(params->keydir, dest_blob, ptr,
						params->comment,
						params->require_keys,
						params->engine_id,
						params->cmdname);
	}

	if (dest_blob) {
		munmap(dest_blob, destfd_size);
		close(destfd);
	}

err_keydest:
	munmap(ptr, sbuf.st_size);
	close(tfd);

	return ret;
}

/**
 * fit_calc_size() - Calculate the approximate size of the FIT we will generate
 */
static int fit_calc_size(struct image_tool_params *params)
{
	struct content_info *cont;
	int size, total_size;

	size = imagetool_get_filesize(params, params->datafile);
	if (size < 0)
		return -1;
	total_size = size;

	if (params->fit_ramdisk) {
		size = imagetool_get_filesize(params, params->fit_ramdisk);
		if (size < 0)
			return -1;
		total_size += size;
	}

	for (cont = params->content_head; cont; cont = cont->next) {
		size = imagetool_get_filesize(params, cont->fname);
		if (size < 0)
			return -1;

		/* Add space for properties */
		total_size += size + 300;
	}

	/* Add plenty of space for headers, properties, nodes, etc. */
	total_size += 4096;

	return total_size;
}

static int fdt_property_file(struct image_tool_params *params,
			     void *fdt, const char *name, const char *fname)
{
	struct stat sbuf;
	void *ptr;
	int ret;
	int fd;

	fd = open(fname, O_RDWR | O_BINARY);
	if (fd < 0) {
		fprintf(stderr, "%s: Can't open %s: %s\n",
			params->cmdname, fname, strerror(errno));
		return -1;
	}

	if (fstat(fd, &sbuf) < 0) {
		fprintf(stderr, "%s: Can't stat %s: %s\n",
			params->cmdname, fname, strerror(errno));
		goto err;
	}

	ret = fdt_property_placeholder(fdt, "data", sbuf.st_size, &ptr);
	if (ret)
		goto err;
	ret = read(fd, ptr, sbuf.st_size);
	if (ret != sbuf.st_size) {
		fprintf(stderr, "%s: Can't read %s: %s\n",
			params->cmdname, fname, strerror(errno));
		goto err;
	}
	close(fd);

	return 0;
err:
	close(fd);
	return -1;
}

static int fdt_property_strf(void *fdt, const char *name, const char *fmt, ...)
{
	char str[100];
	va_list ptr;

	va_start(ptr, fmt);
	vsnprintf(str, sizeof(str), fmt, ptr);
	va_end(ptr);
	return fdt_property_string(fdt, name, str);
}

static void get_basename(char *str, int size, const char *fname)
{
	const char *p, *start, *end;
	int len;

	/*
	 * Use the base name as the 'name' field. So for example:
	 *
	 * "arch/arm/dts/sun7i-a20-bananapro.dtb"
	 * becomes "sun7i-a20-bananapro"
	 */
	p = strrchr(fname, '/');
	start = p ? p + 1 : fname;
	p = strrchr(fname, '.');
	end = p ? p : fname + strlen(fname);
	len = end - start;
	if (len >= size)
		len = size - 1;
	memcpy(str, start, len);
	str[len] = '\0';
}

/**
 * fit_write_images() - Write out a list of images to the FIT
 *
 * We always include the main image (params->datafile). If there are device
 * tree files, we include an fdt- node for each of those too.
 */
static int fit_write_images(struct image_tool_params *params, char *fdt)
{
	struct content_info *cont;
	const char *typename;
	char str[100];
	int upto;
	int ret;

	fdt_begin_node(fdt, "images");

	/* First the main image */
	typename = genimg_get_type_short_name(params->fit_image_type);
	snprintf(str, sizeof(str), "%s-1", typename);
	fdt_begin_node(fdt, str);
	fdt_property_string(fdt, FIT_DESC_PROP, params->imagename);
	fdt_property_string(fdt, FIT_TYPE_PROP, typename);
	fdt_property_string(fdt, FIT_ARCH_PROP,
			    genimg_get_arch_short_name(params->arch));
	fdt_property_string(fdt, FIT_OS_PROP,
			    genimg_get_os_short_name(params->os));
	fdt_property_string(fdt, FIT_COMP_PROP,
			    genimg_get_comp_short_name(params->comp));
	fdt_property_u32(fdt, FIT_LOAD_PROP, params->addr);
	fdt_property_u32(fdt, FIT_ENTRY_PROP, params->ep);

	/*
	 * Put data last since it is large. SPL may only load the first part
	 * of the DT, so this way it can access all the above fields.
	 */
	ret = fdt_property_file(params, fdt, FIT_DATA_PROP, params->datafile);
	if (ret)
		return ret;
	fdt_end_node(fdt);

	/* Now the device tree files if available */
	upto = 0;
	for (cont = params->content_head; cont; cont = cont->next) {
		if (cont->type != IH_TYPE_FLATDT)
			continue;
		snprintf(str, sizeof(str), "%s-%d", FIT_FDT_PROP, ++upto);
		fdt_begin_node(fdt, str);

		get_basename(str, sizeof(str), cont->fname);
		fdt_property_string(fdt, FIT_DESC_PROP, str);
		ret = fdt_property_file(params, fdt, FIT_DATA_PROP,
					cont->fname);
		if (ret)
			return ret;
		fdt_property_string(fdt, FIT_TYPE_PROP, typename);
		fdt_property_string(fdt, FIT_ARCH_PROP,
				    genimg_get_arch_short_name(params->arch));
		fdt_property_string(fdt, FIT_COMP_PROP,
				    genimg_get_comp_short_name(IH_COMP_NONE));
		fdt_end_node(fdt);
	}

	/* And a ramdisk file if available */
	if (params->fit_ramdisk) {
		fdt_begin_node(fdt, FIT_RAMDISK_PROP "-1");

		fdt_property_string(fdt, FIT_TYPE_PROP, FIT_RAMDISK_PROP);
		fdt_property_string(fdt, FIT_OS_PROP,
				    genimg_get_os_short_name(params->os));

		ret = fdt_property_file(params, fdt, FIT_DATA_PROP,
					params->fit_ramdisk);
		if (ret)
			return ret;

		fdt_end_node(fdt);
	}

	fdt_end_node(fdt);

	return 0;
}

/**
 * fit_write_configs() - Write out a list of configurations to the FIT
 *
 * If there are device tree files, we include a configuration for each, which
 * selects the main image (params->datafile) and its corresponding device
 * tree file.
 *
 * Otherwise we just create a configuration with the main image in it.
 */
static void fit_write_configs(struct image_tool_params *params, char *fdt)
{
	struct content_info *cont;
	const char *typename;
	char str[100];
	int upto;

	fdt_begin_node(fdt, "configurations");
	fdt_property_string(fdt, FIT_DEFAULT_PROP, "conf-1");

	upto = 0;
	for (cont = params->content_head; cont; cont = cont->next) {
		if (cont->type != IH_TYPE_FLATDT)
			continue;
		typename = genimg_get_type_short_name(cont->type);
		snprintf(str, sizeof(str), "conf-%d", ++upto);
		fdt_begin_node(fdt, str);

		get_basename(str, sizeof(str), cont->fname);
		fdt_property_string(fdt, FIT_DESC_PROP, str);

		typename = genimg_get_type_short_name(params->fit_image_type);
		snprintf(str, sizeof(str), "%s-1", typename);
		fdt_property_string(fdt, typename, str);
		fdt_property_string(fdt, FIT_LOADABLE_PROP, str);

		if (params->fit_ramdisk)
			fdt_property_string(fdt, FIT_RAMDISK_PROP,
					    FIT_RAMDISK_PROP "-1");

		snprintf(str, sizeof(str), FIT_FDT_PROP "-%d", upto);
		fdt_property_string(fdt, FIT_FDT_PROP, str);
		fdt_end_node(fdt);
	}

	if (!upto) {
		fdt_begin_node(fdt, "conf-1");
		typename = genimg_get_type_short_name(params->fit_image_type);
		snprintf(str, sizeof(str), "%s-1", typename);
		fdt_property_string(fdt, typename, str);

		if (params->fit_ramdisk)
			fdt_property_string(fdt, FIT_RAMDISK_PROP,
					    FIT_RAMDISK_PROP "-1");

		fdt_end_node(fdt);
	}

	fdt_end_node(fdt);
}

static int fit_build_fdt(struct image_tool_params *params, char *fdt, int size)
{
	int ret;

	ret = fdt_create(fdt, size);
	if (ret)
		return ret;
	fdt_finish_reservemap(fdt);
	fdt_begin_node(fdt, "");
	fdt_property_strf(fdt, FIT_DESC_PROP,
			  "%s image with one or more FDT blobs",
			  genimg_get_type_name(params->fit_image_type));
	fdt_property_strf(fdt, "creator", "U-Boot mkimage %s", PLAIN_VERSION);
	fdt_property_u32(fdt, "#address-cells", 1);
	ret = fit_write_images(params, fdt);
	if (ret)
		return ret;
	fit_write_configs(params, fdt);
	fdt_end_node(fdt);
	ret = fdt_finish(fdt);
	if (ret)
		return ret;

	return fdt_totalsize(fdt);
}

static int fit_build(struct image_tool_params *params, const char *fname)
{
	char *buf;
	int size;
	int ret;
	int fd;

	size = fit_calc_size(params);
	if (size < 0)
		return -1;
	buf = malloc(size);
	if (!buf) {
		fprintf(stderr, "%s: Out of memory (%d bytes)\n",
			params->cmdname, size);
		return -1;
	}
	ret = fit_build_fdt(params, buf, size);
	if (ret < 0) {
		fprintf(stderr, "%s: Failed to build FIT image\n",
			params->cmdname);
		goto err_buf;
	}
	size = ret;
	fd = open(fname, O_RDWR | O_CREAT | O_TRUNC | O_BINARY, 0666);
	if (fd < 0) {
		fprintf(stderr, "%s: Can't open %s: %s\n",
			params->cmdname, fname, strerror(errno));
		goto err_buf;
	}
	ret = write(fd, buf, size);
	if (ret != size) {
		fprintf(stderr, "%s: Can't write %s: %s\n",
			params->cmdname, fname, strerror(errno));
		goto err;
	}
	close(fd);
	free(buf);

	return 0;
err:
	close(fd);
err_buf:
	free(buf);
	return -1;
}

/**
 * fit_extract_data() - Move all data outside the FIT
 *
 * This takes a normal FIT file and removes all the 'data' properties from it.
 * The data is placed in an area after the FIT so that it can be accessed
 * using an offset into that area. The 'data' properties turn into
 * 'data-offset' properties.
 *
 * This function cannot cope with FITs with 'data-offset' properties. All
 * data must be in 'data' properties on entry.
 */
static int fit_extract_data(struct image_tool_params *params, const char *fname)
{
	void *buf;
	int buf_ptr;
	int fit_size, new_size;
	int fd;
	struct stat sbuf;
	void *fdt;
	int ret;
	int images;
	int node;

	fd = mmap_fdt(params->cmdname, fname, 0, &fdt, &sbuf, false, false);
	if (fd < 0)
		return -EIO;
	fit_size = fdt_totalsize(fdt);

	/* Allocate space to hold the image data we will extract */
	buf = malloc(fit_size);
	if (!buf) {
		ret = -ENOMEM;
		goto err_munmap;
	}
	buf_ptr = 0;

	images = fdt_path_offset(fdt, FIT_IMAGES_PATH);
	if (images < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__, images);
		ret = -EINVAL;
		goto err_munmap;
	}

	for (node = fdt_first_subnode(fdt, images);
	     node >= 0;
	     node = fdt_next_subnode(fdt, node)) {
		const char *data;
		int len;

		data = fdt_getprop(fdt, node, FIT_DATA_PROP, &len);
		if (!data)
			continue;
		memcpy(buf + buf_ptr, data, len);
		debug("Extracting data size %x\n", len);

		ret = fdt_delprop(fdt, node, FIT_DATA_PROP);
		if (ret) {
			ret = -EPERM;
			goto err_munmap;
		}
		if (params->external_offset > 0) {
			/* An external offset positions the data absolutely. */
			fdt_setprop_u32(fdt, node, FIT_DATA_POSITION_PROP,
					params->external_offset + buf_ptr);
		} else {
			fdt_setprop_u32(fdt, node, FIT_DATA_OFFSET_PROP,
					buf_ptr);
		}
		fdt_setprop_u32(fdt, node, FIT_DATA_SIZE_PROP, len);

		buf_ptr += (len + 3) & ~3;
	}

	/* Pack the FDT and place the data after it */
	fdt_pack(fdt);

	debug("Size reduced from %x to %x\n", fit_size, fdt_totalsize(fdt));
	debug("External data size %x\n", buf_ptr);
	new_size = fdt_totalsize(fdt);
	new_size = (new_size + 3) & ~3;
	munmap(fdt, sbuf.st_size);

	if (ftruncate(fd, new_size)) {
		debug("%s: Failed to truncate file: %s\n", __func__,
		      strerror(errno));
		ret = -EIO;
		goto err;
	}

	/* Check if an offset for the external data was set. */
	if (params->external_offset > 0) {
		if (params->external_offset < new_size) {
			debug("External offset %x overlaps FIT length %x",
			      params->external_offset, new_size);
			ret = -EINVAL;
			goto err;
		}
		new_size = params->external_offset;
	}
	if (lseek(fd, new_size, SEEK_SET) < 0) {
		debug("%s: Failed to seek to end of file: %s\n", __func__,
		      strerror(errno));
		ret = -EIO;
		goto err;
	}
	if (write(fd, buf, buf_ptr) != buf_ptr) {
		debug("%s: Failed to write external data to file %s\n",
		      __func__, strerror(errno));
		ret = -EIO;
		goto err;
	}
	free(buf);
	close(fd);
	return 0;

err_munmap:
	munmap(fdt, sbuf.st_size);
err:
	if (buf)
		free(buf);
	close(fd);
	return ret;
}

static int fit_import_data(struct image_tool_params *params, const char *fname)
{
	void *fdt, *old_fdt;
	int fit_size, new_size, size, data_base;
	int fd;
	struct stat sbuf;
	int ret;
	int images;
	int node;

	fd = mmap_fdt(params->cmdname, fname, 0, &old_fdt, &sbuf, false, false);
	if (fd < 0)
		return -EIO;
	fit_size = fdt_totalsize(old_fdt);
	data_base = (fit_size + 3) & ~3;

	/* Allocate space to hold the new FIT */
	size = sbuf.st_size + 16384;
	fdt = malloc(size);
	if (!fdt) {
		fprintf(stderr, "%s: Failed to allocate memory (%d bytes)\n",
			__func__, size);
		ret = -ENOMEM;
		goto err_has_fd;
	}
	ret = fdt_open_into(old_fdt, fdt, size);
	if (ret) {
		debug("%s: Failed to expand FIT: %s\n", __func__,
		      fdt_strerror(errno));
		ret = -EINVAL;
		goto err_has_fd;
	}

	images = fdt_path_offset(fdt, FIT_IMAGES_PATH);
	if (images < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__, images);
		ret = -EINVAL;
		goto err_has_fd;
	}

	for (node = fdt_first_subnode(fdt, images);
	     node >= 0;
	     node = fdt_next_subnode(fdt, node)) {
		int buf_ptr;
		int len;

		buf_ptr = fdtdec_get_int(fdt, node, "data-offset", -1);
		len = fdtdec_get_int(fdt, node, "data-size", -1);
		if (buf_ptr == -1 || len == -1)
			continue;
		debug("Importing data size %x\n", len);

		ret = fdt_setprop(fdt, node, "data", fdt + data_base + buf_ptr,
				  len);
		if (ret) {
			debug("%s: Failed to write property: %s\n", __func__,
			      fdt_strerror(ret));
			ret = -EINVAL;
			goto err_has_fd;
		}
	}

	/* Close the old fd so we can re-use it. */
	close(fd);

	/* Pack the FDT and place the data after it */
	fdt_pack(fdt);

	new_size = fdt_totalsize(fdt);
	debug("Size expanded from %x to %x\n", fit_size, new_size);

	fd = open(fname, O_RDWR | O_CREAT | O_TRUNC | O_BINARY, 0666);
	if (fd < 0) {
		fprintf(stderr, "%s: Can't open %s: %s\n",
			params->cmdname, fname, strerror(errno));
		ret = -EIO;
		goto err_no_fd;
	}
	if (write(fd, fdt, new_size) != new_size) {
		debug("%s: Failed to write external data to file %s\n",
		      __func__, strerror(errno));
		ret = -EIO;
		goto err_has_fd;
	}

	ret = 0;

err_has_fd:
	close(fd);
err_no_fd:
	munmap(old_fdt, sbuf.st_size);
	free(fdt);
	return ret;
}

/**
 * fit_handle_file - main FIT file processing function
 *
 * fit_handle_file() runs dtc to convert .its to .itb, includes
 * binary data, updates timestamp property and calculates hashes.
 *
 * datafile  - .its file
 * imagefile - .itb file
 *
 * returns:
 *     only on success, otherwise calls exit (EXIT_FAILURE);
 */
static int fit_handle_file(struct image_tool_params *params)
{
	char tmpfile[MKIMAGE_MAX_TMPFILE_LEN];
	char cmd[MKIMAGE_MAX_DTC_CMDLINE_LEN];
	size_t size_inc;
	int ret;

	/* Flattened Image Tree (FIT) format  handling */
	debug ("FIT format handling\n");

	/* call dtc to include binary properties into the tmp file */
	if (strlen (params->imagefile) +
		strlen (MKIMAGE_TMPFILE_SUFFIX) + 1 > sizeof (tmpfile)) {
		fprintf (stderr, "%s: Image file name (%s) too long, "
				"can't create tmpfile",
				params->imagefile, params->cmdname);
		return (EXIT_FAILURE);
	}
	sprintf (tmpfile, "%s%s", params->imagefile, MKIMAGE_TMPFILE_SUFFIX);

	/* We either compile the source file, or use the existing FIT image */
	if (params->auto_its) {
		if (fit_build(params, tmpfile)) {
			fprintf(stderr, "%s: failed to build FIT\n",
				params->cmdname);
			return EXIT_FAILURE;
		}
		*cmd = '\0';
	} else if (params->datafile) {
		/* dtc -I dts -O dtb -p 500 -o tmpfile datafile */
		snprintf(cmd, sizeof(cmd), "%s %s -o \"%s\" \"%s\"",
			 MKIMAGE_DTC, params->dtc, tmpfile, params->datafile);
		debug("Trying to execute \"%s\"\n", cmd);
	} else {
		snprintf(cmd, sizeof(cmd), "cp \"%s\" \"%s\"",
			 params->imagefile, tmpfile);
	}
	if (*cmd && system(cmd) == -1) {
		fprintf (stderr, "%s: system(%s) failed: %s\n",
				params->cmdname, cmd, strerror(errno));
		goto err_system;
	}

	/* Move the data so it is internal to the FIT, if needed */
	ret = fit_import_data(params, tmpfile);
	if (ret)
		goto err_system;

	/*
	 * Set hashes for images in the blob. Unfortunately we may need more
	 * space in either FDT, so keep trying until we succeed.
	 *
	 * Note: this is pretty inefficient for signing, since we must
	 * calculate the signature every time. It would be better to calculate
	 * all the data and then store it in a separate step. However, this
	 * would be considerably more complex to implement. Generally a few
	 * steps of this loop is enough to sign with several keys.
	 */
	for (size_inc = 0; size_inc < 64 * 1024; size_inc += 1024) {
		ret = fit_add_file_data(params, size_inc, tmpfile);
		if (!ret || ret != -ENOSPC)
			break;
	}

	if (ret) {
		fprintf(stderr, "%s Can't add hashes to FIT blob: %d\n",
			params->cmdname, ret);
		goto err_system;
	}

	/* Move the data so it is external to the FIT, if requested */
	if (params->external_data) {
		ret = fit_extract_data(params, tmpfile);
		if (ret)
			goto err_system;
	}

	if (rename (tmpfile, params->imagefile) == -1) {
		fprintf (stderr, "%s: Can't rename %s to %s: %s\n",
				params->cmdname, tmpfile, params->imagefile,
				strerror (errno));
		unlink (tmpfile);
		unlink (params->imagefile);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;

err_system:
	unlink(tmpfile);
	return -1;
}

/**
 * fit_image_extract - extract a FIT component image
 * @fit: pointer to the FIT format image header
 * @image_noffset: offset of the component image node
 * @file_name: name of the file to store the FIT sub-image
 *
 * returns:
 *     zero in case of success or a negative value if fail.
 */
static int fit_image_extract(
	const void *fit,
	int image_noffset,
	const char *file_name)
{
	const void *file_data;
	size_t file_size = 0;

	/* get the "data" property of component at offset "image_noffset" */
	fit_image_get_data(fit, image_noffset, &file_data, &file_size);

	/* save the "file_data" into the file specified by "file_name" */
	return imagetool_save_subimage(file_name, (ulong) file_data, file_size);
}

/**
 * fit_extract_contents - retrieve a sub-image component from the FIT image
 * @ptr: pointer to the FIT format image header
 * @params: command line parameters
 *
 * returns:
 *     zero in case of success or a negative value if fail.
 */
static int fit_extract_contents(void *ptr, struct image_tool_params *params)
{
	int images_noffset;
	int noffset;
	int ndepth;
	const void *fit = ptr;
	int count = 0;
	const char *p;

	/* Indent string is defined in header image.h */
	p = IMAGE_INDENT_STRING;

	if (!fit_check_format(fit)) {
		printf("Bad FIT image format\n");
		return -1;
	}

	/* Find images parent node offset */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images_noffset < 0) {
		printf("Can't find images parent node '%s' (%s)\n",
		       FIT_IMAGES_PATH, fdt_strerror(images_noffset));
		return -1;
	}

	/* Avoid any overrun */
	count = fit_get_subimage_count(fit, images_noffset);
	if ((params->pflag < 0) || (count <= params->pflag)) {
		printf("No such component at '%d'\n", params->pflag);
		return -1;
	}

	/* Process its subnodes, extract the desired component from image */
	for (ndepth = 0, count = 0,
		noffset = fdt_next_node(fit, images_noffset, &ndepth);
		(noffset >= 0) && (ndepth > 0);
		noffset = fdt_next_node(fit, noffset, &ndepth)) {
		if (ndepth == 1) {
			/*
			 * Direct child node of the images parent node,
			 * i.e. component image node.
			 */
			if (params->pflag == count) {
				printf("Extracted:\n%s Image %u (%s)\n", p,
				       count, fit_get_name(fit, noffset, NULL));

				fit_image_print(fit, noffset, p);

				return fit_image_extract(fit, noffset,
						params->outfile);
			}

			count++;
		}
	}

	return 0;
}

static int fit_check_params(struct image_tool_params *params)
{
	if (params->auto_its)
		return 0;
	return	((params->dflag && (params->fflag || params->lflag)) ||
		(params->fflag && (params->dflag || params->lflag)) ||
		(params->lflag && (params->dflag || params->fflag)));
}

U_BOOT_IMAGE_TYPE(
	fitimage,
	"FIT Image support",
	sizeof(image_header_t),
	(void *)&header,
	fit_check_params,
	fit_verify_header,
	fit_print_contents,
	NULL,
	fit_extract_contents,
	fit_check_image_types,
	fit_handle_file,
	NULL /* FIT images use DTB header */
);
