#ifndef __DT_ACCESS_H__
#define __DT_ACCESS_H__

#include "linux/of.h"
#include <linux/gpio/consumer.h>
#include <linux/of_platform.h>
#include <linux/io.h>
#include <linux/module.h>
typedef struct device_node * dt_handle_t;
typedef struct platform_device dt_device_t;
extern struct gpio_desc *gpiod_get_from_of_node(struct device_node *node,
        const char *propname, int index,
        enum gpiod_flags dflags,
        const char *label);
typedef struct gpio_desc *dt_gpio_desc;

#define dt_for_each_child_of_node(parent, child)  for_each_child_of_node(parent, child)

const bool dt_is_available(const dt_handle_t handle);
const bool dt_is_valid(const dt_handle_t handle);
const dt_handle_t dt_parent(const dt_handle_t handle);
const bool dt_is_equal(const dt_handle_t handle1, const dt_handle_t handle2);
const dt_handle_t dt_parse_phandle(const dt_handle_t handle, const char *phandle_name, int index);
const dt_handle_t dt_find_compatible_node(dt_handle_t handle, const char *compat);
const void *dt_get_property(const dt_handle_t handle, const char *propname, int *lenp);
const bool dt_property_read_bool(const dt_handle_t handle, const char *propname);
const char *dt_property_read_string(const dt_handle_t handle, const char *propname);
/* Note that if property is not found, return value will be -1 (0xffffffff) */
const uint32_t dt_property_read_u32(const dt_handle_t handle, const char *propname);
const uint32_t dt_property_read_u32_default(const dt_handle_t handle, const char *propname, uint32_t defval);
int dt_property_read_u32_index(const dt_handle_t handle, const char *propname, uint32_t index, uint32_t *out_val);
const char *dt_get_name(const dt_handle_t handle);
int dt_gpio_request_by_name_optional(const dt_handle_t handle, const char *propname, int index, const char *label, dt_gpio_desc *desc);
void dt_gpio_set_value(dt_gpio_desc desc, int value);
void dt_gpio_set_direction_output(dt_gpio_desc desc, int value);
int dt_gpio_exists(dt_gpio_desc desc);

dt_handle_t dt_dev_get_handle(dt_device_t *pdev);
void *dt_dev_read_addr(dt_device_t *pdev, int index);
void *dt_dev_remap(dt_device_t *pdev, int index);
void *dt_dev_remap_resource(dt_device_t *pdev, int index);

#endif

