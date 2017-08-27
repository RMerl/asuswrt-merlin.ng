/*
 * place-holders to keep libasound linkable to old binaries
 */

#ifndef DOXYGEN

#include "local.h"

size_t snd_instr_header_sizeof(void)
{
	return 0;
}

int snd_instr_header_malloc(void **ptr ATTRIBUTE_UNUSED,
			    size_t len ATTRIBUTE_UNUSED)
{
	return -ENOMEM;
}

void snd_instr_header_free(void *obj ATTRIBUTE_UNUSED)
{
}

void snd_instr_header_copy(void *dst ATTRIBUTE_UNUSED,
			   const void *src ATTRIBUTE_UNUSED)
{
}

const void *snd_instr_header_get_id(const void *info ATTRIBUTE_UNUSED)
{
	return NULL;
}

int snd_instr_header_get_cluster(const void *info ATTRIBUTE_UNUSED)
{
	return 0;
}

unsigned int snd_instr_header_get_cmd(const void *info ATTRIBUTE_UNUSED)
{
	return 0;
}

size_t snd_instr_header_get_len(const void *info ATTRIBUTE_UNUSED)
{
	return 0;
}

const char *snd_instr_header_get_name(const void *info ATTRIBUTE_UNUSED)
{
	return NULL;
}

int snd_instr_header_get_type(const void *info ATTRIBUTE_UNUSED)
{
	return 0;
}

const char *snd_instr_header_get_format(const void *info ATTRIBUTE_UNUSED)
{
	return NULL;
}

const void *snd_instr_header_get_alias(const void *info ATTRIBUTE_UNUSED)
{
	return NULL;
}

void *snd_instr_header_get_data(const void *info ATTRIBUTE_UNUSED)
{
	return NULL;
}

int snd_instr_header_get_follow_alias(const void *info ATTRIBUTE_UNUSED)
{
	return 0;
}

void snd_instr_header_set_id(void *info ATTRIBUTE_UNUSED,
			     const void *id ATTRIBUTE_UNUSED)
{
}

void snd_instr_header_set_cluster(void *info ATTRIBUTE_UNUSED,
				  int cluster ATTRIBUTE_UNUSED)
{
}

void snd_instr_header_set_cmd(void *info ATTRIBUTE_UNUSED,
			      unsigned int cmd ATTRIBUTE_UNUSED)
{
}

void snd_instr_header_set_len(void *info ATTRIBUTE_UNUSED,
			      size_t len ATTRIBUTE_UNUSED)
{
}

void snd_instr_header_set_name(void *info ATTRIBUTE_UNUSED,
			       const char *name ATTRIBUTE_UNUSED)
{
}

void snd_instr_header_set_type(void *info ATTRIBUTE_UNUSED,
			       int type ATTRIBUTE_UNUSED)
{
}

void snd_instr_header_set_format(void *info ATTRIBUTE_UNUSED,
				 const char *format ATTRIBUTE_UNUSED)
{
}

void snd_instr_header_set_alias(void *info ATTRIBUTE_UNUSED,
				const void *instr ATTRIBUTE_UNUSED)
{
}

void snd_instr_header_set_follow_alias(void *info ATTRIBUTE_UNUSED,
				       int val ATTRIBUTE_UNUSED)
{
}

int snd_instr_fm_free(void *fm ATTRIBUTE_UNUSED)
{
	return 0;
}

int snd_instr_fm_convert_to_stream(void *fm ATTRIBUTE_UNUSED,
				   const char *name ATTRIBUTE_UNUSED,
				   void **__data ATTRIBUTE_UNUSED,
				   size_t *__size ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}

int snd_instr_fm_convert_from_stream(void *__data ATTRIBUTE_UNUSED,
				     size_t size ATTRIBUTE_UNUSED,
				     void **simple ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}


int snd_instr_iwffff_open(void **handle ATTRIBUTE_UNUSED,
			  const char *name_fff ATTRIBUTE_UNUSED,
			  const char *name_dat ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}

int snd_instr_iwffff_open_rom(void **handle ATTRIBUTE_UNUSED,
			      int card ATTRIBUTE_UNUSED,
			      int bank ATTRIBUTE_UNUSED,
			      int file ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}

int snd_instr_iwffff_open_rom_file(void **handle ATTRIBUTE_UNUSED,
				   const char *name ATTRIBUTE_UNUSED,
				   int bank ATTRIBUTE_UNUSED,
				   int file ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}

int snd_instr_iwffff_close(void *handle ATTRIBUTE_UNUSED)
{
	return 0;
}

int snd_instr_iwffff_free(void *__instr ATTRIBUTE_UNUSED)
{
	return 0;
}

int snd_instr_iwffff_load(void *iwf ATTRIBUTE_UNUSED,
			  int bank ATTRIBUTE_UNUSED,
			  int prg ATTRIBUTE_UNUSED,
			  void **__iwffff ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}

int snd_instr_iwffff_convert_to_stream(void *iwffff ATTRIBUTE_UNUSED,
				       const char *name ATTRIBUTE_UNUSED,
				       void **__data ATTRIBUTE_UNUSED,
				       size_t *__size ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}

int snd_instr_iwffff_convert_from_stream(void *data ATTRIBUTE_UNUSED,
					 size_t size ATTRIBUTE_UNUSED,
					 void **iwffff ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}


int snd_instr_simple_free(void *simple ATTRIBUTE_UNUSED)
{
	return 0;
}

int snd_instr_simple_convert_to_stream(void *simple ATTRIBUTE_UNUSED,
				       const char *name ATTRIBUTE_UNUSED,
				       void **__data ATTRIBUTE_UNUSED,
				       size_t *__size ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}

int snd_instr_simple_convert_from_stream(void *__data ATTRIBUTE_UNUSED,
					 size_t size ATTRIBUTE_UNUSED,
					 void **simple ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}

#endif /* !DOXYGEN */
