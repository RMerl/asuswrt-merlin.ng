/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __DCB_H__
#define __DCB_H__ 1

#include <libmnl/libmnl.h>
#include <stdbool.h>
#include <stddef.h>

/* dcb.c */

struct dcb {
	char *buf;
	struct mnl_socket *nl;
	bool json_output;
	bool stats;
	bool use_iec;
	bool numeric;
};

int dcb_parse_mapping(const char *what_key, __u32 key, __u32 max_key,
		      const char *what_value, __u64 value, __u64 max_value,
		      void (*set_array)(__u32 index, __u64 value, void *data),
		      void *set_array_data);
int dcb_cmd_parse_dev(struct dcb *dcb, int argc, char **argv,
		      int (*and_then)(struct dcb *dcb, const char *dev,
				      int argc, char **argv),
		      void (*help)(void));

void dcb_set_u8(__u32 key, __u64 value, void *data);
void dcb_set_u32(__u32 key, __u64 value, void *data);
void dcb_set_u64(__u32 key, __u64 value, void *data);

int dcb_get_attribute(struct dcb *dcb, const char *dev, int attr,
		      void *data, size_t data_len);
int dcb_set_attribute(struct dcb *dcb, const char *dev, int attr,
		      const void *data, size_t data_len);
int dcb_get_attribute_va(struct dcb *dcb, const char *dev, int attr,
			 void **payload_p, __u16 *payload_len_p);
int dcb_set_attribute_va(struct dcb *dcb, int command, const char *dev,
			 int (*cb)(struct dcb *dcb, struct nlmsghdr *nlh, void *data),
			 void *data);
int dcb_get_attribute_bare(struct dcb *dcb, int cmd, const char *dev, int attr,
			   void **payload_p, __u16 *payload_len_p);
int dcb_set_attribute_bare(struct dcb *dcb, int command, const char *dev,
			   int attr, const void *data, size_t data_len,
			   int response_attr);

void dcb_print_named_array(const char *json_name, const char *fp_name,
			   const __u8 *array, size_t size,
			   void (*print_array)(const __u8 *, size_t));
void dcb_print_array_u8(const __u8 *array, size_t size);
void dcb_print_array_u64(const __u64 *array, size_t size);
void dcb_print_array_on_off(const __u8 *array, size_t size);
void dcb_print_array_kw(const __u8 *array, size_t array_size,
			const char *const kw[], size_t kw_size);

/* dcb_app.c */

int dcb_cmd_app(struct dcb *dcb, int argc, char **argv);

/* dcb_buffer.c */

int dcb_cmd_buffer(struct dcb *dcb, int argc, char **argv);

/* dcb_dcbx.c */

int dcb_cmd_dcbx(struct dcb *dcb, int argc, char **argv);

/* dcb_ets.c */

int dcb_cmd_ets(struct dcb *dcb, int argc, char **argv);

/* dcb_maxrate.c */

int dcb_cmd_maxrate(struct dcb *dcb, int argc, char **argv);

/* dcb_pfc.c */

int dcb_cmd_pfc(struct dcb *dcb, int argc, char **argv);

#endif /* __DCB_H__ */
