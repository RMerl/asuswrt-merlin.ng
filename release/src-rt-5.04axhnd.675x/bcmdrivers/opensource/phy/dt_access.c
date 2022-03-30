#include "dt_access.h"

const bool dt_is_available(const dt_handle_t handle)
{
    return of_device_is_available(handle);
}

const bool dt_is_valid(const dt_handle_t handle)
{
    return handle ? 1 : 0;
}

const dt_handle_t dt_parent(const dt_handle_t handle)
{
    return of_get_parent(handle);
}

const bool dt_is_equal(const dt_handle_t handle1, const dt_handle_t handle2)
{
    return handle1 == handle2;
}

const dt_handle_t dt_parse_phandle(const dt_handle_t handle, const char *phandle_name, int index)
{
    return of_parse_phandle(handle, phandle_name, index);
}

const dt_handle_t dt_find_compatible_node(dt_handle_t handle, const char *compat)
{
    return of_find_compatible_node(handle, NULL, compat);
}

const void *dt_get_property(const dt_handle_t handle, const char *propname, int *lenp)
{
    return of_get_property(handle, propname, lenp);
}

const bool dt_property_read_bool(const dt_handle_t handle, const char *propname)
{
    return dt_get_property(handle, propname, NULL) ? 1 : 0;
}

const uint32_t dt_property_read_u32(const dt_handle_t handle, const char *propname)
{
    uint32_t *val = dt_get_property(handle, propname, NULL);

    if (!val)
        return -1;

    return be32_to_cpup(val);
}

const uint32_t dt_property_read_u32_default(const dt_handle_t handle, const char *propname, uint32_t defval)
{
    uint32_t *val = dt_get_property(handle, propname, NULL);

    if (!val)
        return defval;

    return be32_to_cpup(val);
}

int dt_property_read_u32_index(const dt_handle_t handle, const char *propname, uint32_t index, uint32_t *out_val)
{
    return of_property_read_u32_index(handle, propname, index, out_val);
}

const char *dt_property_read_string(const dt_handle_t handle, const char *propname)
{
    return dt_get_property(handle, propname, NULL);
}

const char *dt_get_name(const dt_handle_t handle)
{
    return of_node_full_name(handle);
}

int dt_gpio_request_by_name_optional(const dt_handle_t handle, const char *propname, int index, const char *label, dt_gpio_desc *desc)
{
    *desc = gpiod_get_from_of_node(handle, propname, index, GPIOD_ASIS, label);
    if (PTR_ERR(*desc) == -ENOENT || PTR_ERR(*desc) == -ENOSYS)
    {
        *desc = NULL;
        return 0;
    }

    if (IS_ERR(desc))
         return PTR_ERR(desc);

    return 0;
}

void dt_gpio_set_value(dt_gpio_desc desc, int value)
{
    gpiod_set_value(desc, value);
}

void dt_gpio_set_direction_output(dt_gpio_desc desc, int value)
{
    gpiod_direction_output(desc, value);
}

int dt_gpio_exists(dt_gpio_desc desc)
{
    return !!desc;
}

dt_handle_t dt_dev_get_handle(dt_device_t *pdev)
{
    return pdev->dev.of_node;
}

void *dt_dev_read_addr(dt_device_t *pdev, int index)
{
    struct resource *res;

    res = platform_get_resource(pdev, IORESOURCE_MEM, index);
    return res->start;
}

void *dt_dev_remap(dt_device_t *pdev, int index)
{
    struct resource *res;
    resource_size_t size;

    res = platform_get_resource(pdev, IORESOURCE_MEM, index);
    size = resource_size(res);
    return devm_ioremap(&pdev->dev, res->start, size);
}

void *dt_dev_remap_resource(dt_device_t *pdev, int index)
{
    struct resource *res;

    res = platform_get_resource(pdev, IORESOURCE_MEM, index);
    return devm_ioremap_resource(&pdev->dev, res);
}
