#include "dt_access.h"

const bool dt_is_available(const dt_handle_t handle)
{
    return ofnode_is_available(handle);
}

const bool dt_is_valid(const dt_handle_t handle)
{
    return ofnode_valid(handle);
}

const dt_handle_t dt_parent(const dt_handle_t handle)
{
    return ofnode_get_parent(handle);
}

const bool dt_is_equal(const dt_handle_t handle1, const dt_handle_t handle2)
{
    return ofnode_equal(handle1, handle2);
}

const dt_handle_t dt_parse_phandle(const dt_handle_t handle, const char *phandle_name, int index)
{
    ofnode node = ofnode_null();
    struct ofnode_phandle_args out_args;

    if (ofnode_parse_phandle_with_args(handle, phandle_name, NULL, 0, index, &out_args) == 0)
        node = out_args.node;

    return node;
}

const dt_handle_t dt_find_compatible_node(dt_handle_t handle, const char *compat)
{
    return ofnode_by_compatible(handle, compat);
}

const void *dt_get_property(const dt_handle_t handle, const char *propname, int *lenp)
{
    return ofnode_get_property(handle, propname, lenp);
}

const bool dt_property_read_bool(const dt_handle_t handle, const char *propname)
{
    return dt_get_property(handle, propname, NULL) ? 1 : 0;
}

const uint32_t dt_property_read_u32(const dt_handle_t handle, const char *propname)
{
    const uint32_t *val = dt_get_property(handle, propname, NULL);

    if (!val)
        return -1;

    return be32_to_cpup(val);
}

const uint32_t dt_property_read_u32_default(const dt_handle_t handle, const char *propname, uint32_t defval)
{
    const uint32_t *val = dt_get_property(handle, propname, NULL);

    if (!val)
        return defval;

    return be32_to_cpup(val);
}

const char *dt_property_read_string(const dt_handle_t handle, const char *propname)
{
    return dt_get_property(handle, propname, NULL);
}

const char *dt_get_name(const dt_handle_t handle)
{
    return ofnode_get_name(handle);
}

int dt_gpio_request_by_name(const dt_handle_t handle, const char *propname, int index, const char *label, dt_gpio_desc *desc, int value)
{
    int ret;

    ret = gpio_request_by_name_nodev(handle, propname, index, desc, GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
    if (ret == -ENOENT)
        return 0;

    return dm_gpio_set_value(desc, value);
}

void dt_gpio_set_value(dt_gpio_desc desc, int value)
{
    dm_gpio_set_value(&desc, value);
}

int dt_gpio_exists(dt_gpio_desc desc)
{
    return !!desc.dev;
}

dt_handle_t dt_dev_get_handle(dt_device_t *pdev)
{
    return pdev->node;
}

void *dt_dev_read_addr(dt_device_t *pdev, int index)
{
    return (void*)dev_read_addr_index(pdev, index);
}

void *dt_dev_remap(dt_device_t *pdev, int index)
{
    return dev_remap_addr_index(pdev, index);
}

void *dt_dev_remap_resource(dt_device_t *pdev, int index)
{
    return dev_remap_addr_index(pdev, index);
}
