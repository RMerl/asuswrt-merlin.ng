/*
 * Copyright (C) 2013 Intel Corporation
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
 *
 */

#include <endian.h>

#include <hardware/bluetooth.h>

#define MAX_UUID_STR_LEN	37
#define HAL_UUID_LEN		16
#define MAX_ADDR_STR_LEN	18

static const char BT_BASE_UUID[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
	0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb
};

const char *bt_uuid_t2str(const uint8_t *uuid, char *buf);
const char *btuuid2str(const uint8_t *uuid);
const char *bt_bdaddr_t2str(const bt_bdaddr_t *bd_addr, char *buf);
void str2bt_bdaddr_t(const char *str, bt_bdaddr_t *bd_addr);
void str2bt_uuid_t(const char *str, bt_uuid_t *uuid);
const char *btproperty2str(const bt_property_t *property);
const char *bdaddr2str(const bt_bdaddr_t *bd_addr);

int get_config(const char *config_key, char *value, const char *fallback);

/*
 * Begin mapping section
 *
 * There are some mappings between integer values (enums) and strings
 * to be presented to user. To make it easier to convert between those two
 * set of macros is given. It is specially useful when we want to have
 * strings that match constants from header files like:
 *  BT_STATUS_SUCCESS (0) and corresponding "BT_STATUS_SUCCESS"
 * Example of usage:
 *
 * INTMAP(int, -1, "invalid")
 *   DELEMENT(BT_STATUS_SUCCESS)
 *   DELEMENT(BT_STATUS_FAIL)
 *   MELEMENT(123, "Some strange value")
 * ENDMAP
 *
 * Just by doing this we have mapping table plus two functions:
 *  int str2int(const char *str);
 *  const char *int2str(int v);
 *
 * second argument to INTMAP specifies value to be returned from
 * str2int function when there is not mapping for such number
 * third argument specifies default value to be returned from int2str
 *
 * If same mapping is to be used in several source files put
 * INTMAP in c file and DECINTMAP in h file.
 *
 * For mappings that are to be used in single file only
 * use SINTMAP which will create the same but everything will be marked
 * as static.
 */

struct int2str {
	int val;		/* int value */
	const char *str;	/* corresponding string */
};

int int2str_findint(int v, const struct int2str m[]);
int int2str_findstr(const char *str, const struct int2str m[]);
const char *enum_defines(void *v, int i);
const char *enum_strings(void *v, int i);
const char *enum_one_string(void *v, int i);

#define TYPE_ENUM(type) ((void *) &__##type##2str[0])
#define DECINTMAP(type) \
extern struct int2str __##type##2str[]; \
const char *type##2##str(type v); \
type str##2##type(const char *str); \

#define INTMAP(type, deft, defs) \
const char *type##2##str(type v) \
{ \
	int i = int2str_findint((int) v, __##type##2str); \
	return (i < 0) ? defs : __##type##2str[i].str; \
} \
type str##2##type(const char *str) \
{ \
	int i = int2str_findstr(str, __##type##2str); \
	return (i < 0) ? (type) deft : (type) (__##type##2str[i].val); \
} \
struct int2str __##type##2str[] = {

#define SINTMAP(type, deft, defs) \
static struct int2str __##type##2str[]; \
static inline const char *type##2##str(type v) \
{ \
	int i = int2str_findint((int) v, __##type##2str); \
	return (i < 0) ? defs : __##type##2str[i].str; \
} \
static inline type str##2##type(const char *str) \
{ \
	int i = int2str_findstr(str, __##type##2str); \
	return (i < 0) ? (type) deft : (type) (__##type##2str[i].val); \
} \
static struct int2str __##type##2str[] = {

#define ENDMAP {0, NULL} };

/* use this to generate string from header file constant */
#define MELEMENT(v, s) {v, s}
/* use this to have arbitrary mapping from int to string */
#define DELEMENT(s) {s, #s}
/* End of mapping section */

DECINTMAP(bt_status_t);
DECINTMAP(bt_state_t);
DECINTMAP(bt_device_type_t);
DECINTMAP(bt_scan_mode_t);
DECINTMAP(bt_discovery_state_t);
DECINTMAP(bt_acl_state_t);
DECINTMAP(bt_bond_state_t);
DECINTMAP(bt_ssp_variant_t);
DECINTMAP(bt_property_type_t);
DECINTMAP(bt_cb_thread_evt);

static inline uint16_t get_le16(const void *src)
{
	const struct __attribute__((packed)) {
		uint16_t le16;
	} *p = src;

	return le16toh(p->le16);
}

static inline void put_le16(uint16_t val, void *dst)
{
	struct __attribute__((packed)) {
		uint16_t le16;
	} *p = dst;

	p->le16 = htole16(val);
}
