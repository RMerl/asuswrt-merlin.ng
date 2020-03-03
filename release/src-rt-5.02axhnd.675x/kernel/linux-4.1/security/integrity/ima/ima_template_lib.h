/*
 * Copyright (C) 2013 Politecnico di Torino, Italy
 *                    TORSEC group -- http://security.polito.it
 *
 * Author: Roberto Sassu <roberto.sassu@polito.it>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2 of the
 * License.
 *
 * File: ima_template_lib.h
 *      Header for the library of supported template fields.
 */
#ifndef __LINUX_IMA_TEMPLATE_LIB_H
#define __LINUX_IMA_TEMPLATE_LIB_H

#include <linux/seq_file.h>
#include "ima.h"

void ima_show_template_digest(struct seq_file *m, enum ima_show_type show,
			      struct ima_field_data *field_data);
void ima_show_template_digest_ng(struct seq_file *m, enum ima_show_type show,
				 struct ima_field_data *field_data);
void ima_show_template_string(struct seq_file *m, enum ima_show_type show,
			      struct ima_field_data *field_data);
void ima_show_template_sig(struct seq_file *m, enum ima_show_type show,
			   struct ima_field_data *field_data);
int ima_eventdigest_init(struct integrity_iint_cache *iint, struct file *file,
			 const unsigned char *filename,
			 struct evm_ima_xattr_data *xattr_value, int xattr_len,
			 struct ima_field_data *field_data);
int ima_eventname_init(struct integrity_iint_cache *iint, struct file *file,
		       const unsigned char *filename,
		       struct evm_ima_xattr_data *xattr_value, int xattr_len,
		       struct ima_field_data *field_data);
int ima_eventdigest_ng_init(struct integrity_iint_cache *iint,
			    struct file *file, const unsigned char *filename,
			    struct evm_ima_xattr_data *xattr_value,
			    int xattr_len, struct ima_field_data *field_data);
int ima_eventname_ng_init(struct integrity_iint_cache *iint, struct file *file,
			  const unsigned char *filename,
			  struct evm_ima_xattr_data *xattr_value, int xattr_len,
			  struct ima_field_data *field_data);
int ima_eventsig_init(struct integrity_iint_cache *iint, struct file *file,
		      const unsigned char *filename,
		      struct evm_ima_xattr_data *xattr_value, int xattr_len,
		      struct ima_field_data *field_data);
#endif /* __LINUX_IMA_TEMPLATE_LIB_H */
