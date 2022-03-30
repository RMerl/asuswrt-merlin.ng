#include "dt_access.h"

const bool dt_is_available(const dt_handle_t handle)
{
#ifdef __UBOOT__
    return ofnode_is_available(handle);
#else
    return of_device_is_available(handle);
#endif
}

const bool dt_is_valid(const dt_handle_t handle)
{
#ifdef __UBOOT__
    return ofnode_valid(handle);
#else
    return handle ? 1 : 0;
#endif
}

const dt_handle_t dt_parent(const dt_handle_t handle)
{
#ifdef __UBOOT__
    return ofnode_get_parent(handle);
#else
    return of_get_parent(handle);
#endif
}

const bool dt_is_equal(const dt_handle_t handle1, const dt_handle_t handle2)
{
#ifdef __UBOOT__
    return ofnode_equal(handle1, handle2);
#else
    return handle1 == handle2;
#endif
}

const dt_handle_t dt_parse_phandle(const dt_handle_t handle, const char *phandle_name, int index)
{
#ifdef __UBOOT__
    ofnode node = ofnode_null();
    struct ofnode_phandle_args out_args;

    if (ofnode_parse_phandle_with_args(handle, phandle_name, NULL, 0, index, &out_args) == 0)
        node = out_args.node;

    return node;
#else
    return of_parse_phandle(handle, phandle_name, index);
#endif
}

const dt_handle_t dt_find_compatible_node(dt_handle_t handle, const char *compat)
{
#ifdef __UBOOT__
    return ofnode_by_compatible(handle, compat);
#else
    return of_find_compatible_node(handle, NULL, compat);
#endif
}

const void *dt_get_property(const dt_handle_t handle, const char *propname, int *lenp)
{
#ifdef __UBOOT__
    return ofnode_get_property(handle, propname, lenp);
#else
    return of_get_property(handle, propname, lenp);
#endif
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

const char *dt_property_read_string(const dt_handle_t handle, const char *propname)
{
    return dt_get_property(handle, propname, NULL);
}

const char *dt_get_name(const dt_handle_t handle)
{
#ifdef __UBOOT__
    return ofnode_get_name(handle);
#else
    return of_node_full_name(handle);
#endif
}

int dt_gpio_request_by_name_optional(const dt_handle_t handle, const char *propname, int index, const char *label, dt_gpio_desc *desc)
{
#ifdef __UBOOT__
    int ret;

    ret = gpio_request_by_name_nodev(handle, propname, index, desc, GPIOD_IS_OUT);
    if (ret == -ENOENT)
        return 0;

    return ret;
#else
    /* XXX: Silently do not support Kernel 4.1 - remove after testing */
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
    *desc = gpiod_get_from_of_node(handle, propname, index, GPIOD_ASIS, label);
    if (PTR_ERR(*desc) == -ENOENT || PTR_ERR(*desc) == -ENOSYS)
    {
        *desc = NULL;
        return 0;
    }

    if (IS_ERR(desc))
         return PTR_ERR(desc);

    return 0;
#else
    *desc = NULL;
    return 0;
#endif
#endif
}

void dt_gpio_set_value(dt_gpio_desc desc, int value)
{
#ifdef __UBOOT__
    dm_gpio_set_value(&desc, value);
#else
    gpiod_set_value(desc, value);
#endif
}

void dt_gpio_set_direction_output(dt_gpio_desc desc, int value)
{
#ifdef __UBOOT__
    dm_gpio_set_dir_flags(&desc, value ? GPIOD_IS_OUT : GPIOD_IS_IN);
#else
    gpiod_direction_output(desc, value);
#endif
}

int dt_gpio_exists(dt_gpio_desc desc)
{
#ifdef __UBOOT__
    return !!desc.dev;
#else
    return !!desc;
#endif
}

dt_handle_t dt_dev_get_handle(dt_device_t *pdev)
{
#ifdef __UBOOT__
    return pdev->node;
#else
    return pdev->dev.of_node;
#endif
}

void *dt_dev_read_addr(dt_device_t *pdev, int index)
{
#ifdef __UBOOT__
    return dev_read_addr_index(pdev, index);
#else
    struct resource *res;

    res = platform_get_resource(pdev, IORESOURCE_MEM, index);
    return res->start;
#endif
}

void *dt_dev_remap(dt_device_t *pdev, int index)
{
#ifdef __UBOOT__
    return dev_remap_addr_index(pdev, index);
#else
    struct resource *res;
    resource_size_t size;

    res = platform_get_resource(pdev, IORESOURCE_MEM, index);
    size = resource_size(res);
    return devm_ioremap(&pdev->dev, res->start, size);
#endif
}

void *dt_dev_remap_resource(dt_device_t *pdev, int index)
{
#ifdef __UBOOT__
    return dev_remap_addr_index(pdev, index);
#else
    struct resource *res;

    res = platform_get_resource(pdev, IORESOURCE_MEM, index);
    return devm_ioremap_resource(&pdev->dev, res);
#endif
}
