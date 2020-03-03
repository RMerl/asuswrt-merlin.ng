#include <linux/configfs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/nls.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget_configfs.h>
#include "configfs.h"
#include "u_f.h"
#include "u_os_desc.h"

int check_user_usb_string(const char *name,
		struct usb_gadget_strings *stringtab_dev)
{
	unsigned primary_lang;
	unsigned sub_lang;
	u16 num;
	int ret;

	ret = kstrtou16(name, 0, &num);
	if (ret)
		return ret;

	primary_lang = num & 0x3ff;
	sub_lang = num >> 10;

	/* simple sanity check for valid langid */
	switch (primary_lang) {
	case 0:
	case 0x62 ... 0xfe:
	case 0x100 ... 0x3ff:
		return -EINVAL;
	}
	if (!sub_lang)
		return -EINVAL;

	stringtab_dev->language = num;
	return 0;
}

#define MAX_NAME_LEN	40
#define MAX_USB_STRING_LANGS 2

struct gadget_info {
	struct config_group group;
	struct config_group functions_group;
	struct config_group configs_group;
	struct config_group strings_group;
	struct config_group os_desc_group;
	struct config_group *default_groups[5];

	struct mutex lock;
	struct usb_gadget_strings *gstrings[MAX_USB_STRING_LANGS + 1];
	struct list_head string_list;
	struct list_head available_func;

	const char *udc_name;
#ifdef CONFIG_USB_OTG
	struct usb_otg_descriptor otg;
#endif
	struct usb_composite_driver composite;
	struct usb_composite_dev cdev;
	bool use_os_desc;
	char b_vendor_code;
	char qw_sign[OS_STRING_QW_SIGN_LEN];
};

struct config_usb_cfg {
	struct config_group group;
	struct config_group strings_group;
	struct config_group *default_groups[2];
	struct list_head string_list;
	struct usb_configuration c;
	struct list_head func_list;
	struct usb_gadget_strings *gstrings[MAX_USB_STRING_LANGS + 1];
};

struct gadget_strings {
	struct usb_gadget_strings stringtab_dev;
	struct usb_string strings[USB_GADGET_FIRST_AVAIL_IDX];
	char *manufacturer;
	char *product;
	char *serialnumber;

	struct config_group group;
	struct list_head list;
};

struct os_desc {
	struct config_group group;
};

struct gadget_config_name {
	struct usb_gadget_strings stringtab_dev;
	struct usb_string strings;
	char *configuration;

	struct config_group group;
	struct list_head list;
};

static int usb_string_copy(const char *s, char **s_copy)
{
	int ret;
	char *str;
	char *copy = *s_copy;
	ret = strlen(s);
	if (ret > 126)
		return -EOVERFLOW;

	str = kstrdup(s, GFP_KERNEL);
	if (!str)
		return -ENOMEM;
	if (str[ret - 1] == '\n')
		str[ret - 1] = '\0';
	kfree(copy);
	*s_copy = str;
	return 0;
}

CONFIGFS_ATTR_STRUCT(gadget_info);
CONFIGFS_ATTR_STRUCT(config_usb_cfg);

#define GI_DEVICE_DESC_ITEM_ATTR(name)	\
	static struct gadget_info_attribute gadget_cdev_desc_##name = \
		__CONFIGFS_ATTR(name,  S_IRUGO | S_IWUSR,		\
				gadget_dev_desc_##name##_show,		\
				gadget_dev_desc_##name##_store)

#define GI_DEVICE_DESC_SIMPLE_R_u8(__name)	\
	static ssize_t gadget_dev_desc_##__name##_show(struct gadget_info *gi, \
			char *page)	\
{	\
	return sprintf(page, "0x%02x\n", gi->cdev.desc.__name);	\
}

#define GI_DEVICE_DESC_SIMPLE_R_u16(__name)	\
	static ssize_t gadget_dev_desc_##__name##_show(struct gadget_info *gi, \
			char *page)	\
{	\
	return sprintf(page, "0x%04x\n", le16_to_cpup(&gi->cdev.desc.__name)); \
}


#define GI_DEVICE_DESC_SIMPLE_W_u8(_name)		\
	static ssize_t gadget_dev_desc_##_name##_store(struct gadget_info *gi, \
		const char *page, size_t len)		\
{							\
	u8 val;						\
	int ret;					\
	ret = kstrtou8(page, 0, &val);			\
	if (ret)					\
		return ret;				\
	gi->cdev.desc._name = val;			\
	return len;					\
}

#define GI_DEVICE_DESC_SIMPLE_W_u16(_name)	\
	static ssize_t gadget_dev_desc_##_name##_store(struct gadget_info *gi, \
		const char *page, size_t len)		\
{							\
	u16 val;					\
	int ret;					\
	ret = kstrtou16(page, 0, &val);			\
	if (ret)					\
		return ret;				\
	gi->cdev.desc._name = cpu_to_le16p(&val);	\
	return len;					\
}

#define GI_DEVICE_DESC_SIMPLE_RW(_name, _type)	\
	GI_DEVICE_DESC_SIMPLE_R_##_type(_name)	\
	GI_DEVICE_DESC_SIMPLE_W_##_type(_name)

GI_DEVICE_DESC_SIMPLE_R_u16(bcdUSB);
GI_DEVICE_DESC_SIMPLE_RW(bDeviceClass, u8);
GI_DEVICE_DESC_SIMPLE_RW(bDeviceSubClass, u8);
GI_DEVICE_DESC_SIMPLE_RW(bDeviceProtocol, u8);
GI_DEVICE_DESC_SIMPLE_RW(bMaxPacketSize0, u8);
GI_DEVICE_DESC_SIMPLE_RW(idVendor, u16);
GI_DEVICE_DESC_SIMPLE_RW(idProduct, u16);
GI_DEVICE_DESC_SIMPLE_R_u16(bcdDevice);

static ssize_t is_valid_bcd(u16 bcd_val)
{
	if ((bcd_val & 0xf) > 9)
		return -EINVAL;
	if (((bcd_val >> 4) & 0xf) > 9)
		return -EINVAL;
	if (((bcd_val >> 8) & 0xf) > 9)
		return -EINVAL;
	if (((bcd_val >> 12) & 0xf) > 9)
		return -EINVAL;
	return 0;
}

static ssize_t gadget_dev_desc_bcdDevice_store(struct gadget_info *gi,
		const char *page, size_t len)
{
	u16 bcdDevice;
	int ret;

	ret = kstrtou16(page, 0, &bcdDevice);
	if (ret)
		return ret;
	ret = is_valid_bcd(bcdDevice);
	if (ret)
		return ret;

	gi->cdev.desc.bcdDevice = cpu_to_le16(bcdDevice);
	return len;
}

static ssize_t gadget_dev_desc_bcdUSB_store(struct gadget_info *gi,
		const char *page, size_t len)
{
	u16 bcdUSB;
	int ret;

	ret = kstrtou16(page, 0, &bcdUSB);
	if (ret)
		return ret;
	ret = is_valid_bcd(bcdUSB);
	if (ret)
		return ret;

	gi->cdev.desc.bcdUSB = cpu_to_le16(bcdUSB);
	return len;
}

static ssize_t gadget_dev_desc_UDC_show(struct gadget_info *gi, char *page)
{
	return sprintf(page, "%s\n", gi->udc_name ?: "");
}

static int unregister_gadget(struct gadget_info *gi)
{
	int ret;

	if (!gi->udc_name)
		return -ENODEV;

	ret = usb_gadget_unregister_driver(&gi->composite.gadget_driver);
	if (ret)
		return ret;
	kfree(gi->udc_name);
	gi->udc_name = NULL;
	return 0;
}

static ssize_t gadget_dev_desc_UDC_store(struct gadget_info *gi,
		const char *page, size_t len)
{
	char *name;
	int ret;

	name = kstrdup(page, GFP_KERNEL);
	if (!name)
		return -ENOMEM;
	if (name[len - 1] == '\n')
		name[len - 1] = '\0';

	mutex_lock(&gi->lock);

	if (!strlen(name)) {
		ret = unregister_gadget(gi);
		if (ret)
			goto err;
		kfree(name);
	} else {
		if (gi->udc_name) {
			ret = -EBUSY;
			goto err;
		}
		ret = usb_udc_attach_driver(name, &gi->composite.gadget_driver);
		if (ret)
			goto err;
		gi->udc_name = name;
	}
	mutex_unlock(&gi->lock);
	return len;
err:
	kfree(name);
	mutex_unlock(&gi->lock);
	return ret;
}

GI_DEVICE_DESC_ITEM_ATTR(bDeviceClass);
GI_DEVICE_DESC_ITEM_ATTR(bDeviceSubClass);
GI_DEVICE_DESC_ITEM_ATTR(bDeviceProtocol);
GI_DEVICE_DESC_ITEM_ATTR(bMaxPacketSize0);
GI_DEVICE_DESC_ITEM_ATTR(idVendor);
GI_DEVICE_DESC_ITEM_ATTR(idProduct);
GI_DEVICE_DESC_ITEM_ATTR(bcdDevice);
GI_DEVICE_DESC_ITEM_ATTR(bcdUSB);
GI_DEVICE_DESC_ITEM_ATTR(UDC);

static struct configfs_attribute *gadget_root_attrs[] = {
	&gadget_cdev_desc_bDeviceClass.attr,
	&gadget_cdev_desc_bDeviceSubClass.attr,
	&gadget_cdev_desc_bDeviceProtocol.attr,
	&gadget_cdev_desc_bMaxPacketSize0.attr,
	&gadget_cdev_desc_idVendor.attr,
	&gadget_cdev_desc_idProduct.attr,
	&gadget_cdev_desc_bcdDevice.attr,
	&gadget_cdev_desc_bcdUSB.attr,
	&gadget_cdev_desc_UDC.attr,
	NULL,
};

static inline struct gadget_info *to_gadget_info(struct config_item *item)
{
	 return container_of(to_config_group(item), struct gadget_info, group);
}

static inline struct gadget_strings *to_gadget_strings(struct config_item *item)
{
	 return container_of(to_config_group(item), struct gadget_strings,
			 group);
}

static inline struct gadget_config_name *to_gadget_config_name(
		struct config_item *item)
{
	 return container_of(to_config_group(item), struct gadget_config_name,
			 group);
}

static inline struct config_usb_cfg *to_config_usb_cfg(struct config_item *item)
{
	return container_of(to_config_group(item), struct config_usb_cfg,
			group);
}

static inline struct usb_function_instance *to_usb_function_instance(
		struct config_item *item)
{
	 return container_of(to_config_group(item),
			 struct usb_function_instance, group);
}

static void gadget_info_attr_release(struct config_item *item)
{
	struct gadget_info *gi = to_gadget_info(item);

	WARN_ON(!list_empty(&gi->cdev.configs));
	WARN_ON(!list_empty(&gi->string_list));
	WARN_ON(!list_empty(&gi->available_func));
	kfree(gi->composite.gadget_driver.function);
	kfree(gi);
}

CONFIGFS_ATTR_OPS(gadget_info);

static struct configfs_item_operations gadget_root_item_ops = {
	.release                = gadget_info_attr_release,
	.show_attribute         = gadget_info_attr_show,
	.store_attribute        = gadget_info_attr_store,
};

static void gadget_config_attr_release(struct config_item *item)
{
	struct config_usb_cfg *cfg = to_config_usb_cfg(item);

	WARN_ON(!list_empty(&cfg->c.functions));
	list_del(&cfg->c.list);
	kfree(cfg->c.label);
	kfree(cfg);
}

static int config_usb_cfg_link(
	struct config_item *usb_cfg_ci,
	struct config_item *usb_func_ci)
{
	struct config_usb_cfg *cfg = to_config_usb_cfg(usb_cfg_ci);
	struct usb_composite_dev *cdev = cfg->c.cdev;
	struct gadget_info *gi = container_of(cdev, struct gadget_info, cdev);

	struct config_group *group = to_config_group(usb_func_ci);
	struct usb_function_instance *fi = container_of(group,
			struct usb_function_instance, group);
	struct usb_function_instance *a_fi;
	struct usb_function *f;
	int ret;

	mutex_lock(&gi->lock);
	/*
	 * Make sure this function is from within our _this_ gadget and not
	 * from another gadget or a random directory.
	 * Also a function instance can only be linked once.
	 */
	list_for_each_entry(a_fi, &gi->available_func, cfs_list) {
		if (a_fi == fi)
			break;
	}
	if (a_fi != fi) {
		ret = -EINVAL;
		goto out;
	}

	list_for_each_entry(f, &cfg->func_list, list) {
		if (f->fi == fi) {
			ret = -EEXIST;
			goto out;
		}
	}

	f = usb_get_function(fi);
	if (IS_ERR(f)) {
		ret = PTR_ERR(f);
		goto out;
	}

	/* stash the function until we bind it to the gadget */
	list_add_tail(&f->list, &cfg->func_list);
	ret = 0;
out:
	mutex_unlock(&gi->lock);
	return ret;
}

static int config_usb_cfg_unlink(
	struct config_item *usb_cfg_ci,
	struct config_item *usb_func_ci)
{
	struct config_usb_cfg *cfg = to_config_usb_cfg(usb_cfg_ci);
	struct usb_composite_dev *cdev = cfg->c.cdev;
	struct gadget_info *gi = container_of(cdev, struct gadget_info, cdev);

	struct config_group *group = to_config_group(usb_func_ci);
	struct usb_function_instance *fi = container_of(group,
			struct usb_function_instance, group);
	struct usb_function *f;

	/*
	 * ideally I would like to forbid to unlink functions while a gadget is
	 * bound to an UDC. Since this isn't possible at the moment, we simply
	 * force an unbind, the function is available here and then we can
	 * remove the function.
	 */
	mutex_lock(&gi->lock);
	if (gi->udc_name)
		unregister_gadget(gi);
	WARN_ON(gi->udc_name);

	list_for_each_entry(f, &cfg->func_list, list) {
		if (f->fi == fi) {
			list_del(&f->list);
			usb_put_function(f);
			mutex_unlock(&gi->lock);
			return 0;
		}
	}
	mutex_unlock(&gi->lock);
	WARN(1, "Unable to locate function to unbind\n");
	return 0;
}

CONFIGFS_ATTR_OPS(config_usb_cfg);

static struct configfs_item_operations gadget_config_item_ops = {
	.release                = gadget_config_attr_release,
	.show_attribute         = config_usb_cfg_attr_show,
	.store_attribute        = config_usb_cfg_attr_store,
	.allow_link             = config_usb_cfg_link,
	.drop_link              = config_usb_cfg_unlink,
};


static ssize_t gadget_config_desc_MaxPower_show(struct config_usb_cfg *cfg,
		char *page)
{
	return sprintf(page, "%u\n", cfg->c.MaxPower);
}

static ssize_t gadget_config_desc_MaxPower_store(struct config_usb_cfg *cfg,
		const char *page, size_t len)
{
	u16 val;
	int ret;
	ret = kstrtou16(page, 0, &val);
	if (ret)
		return ret;
	if (DIV_ROUND_UP(val, 8) > 0xff)
		return -ERANGE;
	cfg->c.MaxPower = val;
	return len;
}

static ssize_t gadget_config_desc_bmAttributes_show(struct config_usb_cfg *cfg,
		char *page)
{
	return sprintf(page, "0x%02x\n", cfg->c.bmAttributes);
}

static ssize_t gadget_config_desc_bmAttributes_store(struct config_usb_cfg *cfg,
		const char *page, size_t len)
{
	u8 val;
	int ret;
	ret = kstrtou8(page, 0, &val);
	if (ret)
		return ret;
	if (!(val & USB_CONFIG_ATT_ONE))
		return -EINVAL;
	if (val & ~(USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER |
				USB_CONFIG_ATT_WAKEUP))
		return -EINVAL;
	cfg->c.bmAttributes = val;
	return len;
}

#define CFG_CONFIG_DESC_ITEM_ATTR(name)	\
	static struct config_usb_cfg_attribute gadget_usb_cfg_##name = \
		__CONFIGFS_ATTR(name,  S_IRUGO | S_IWUSR,		\
				gadget_config_desc_##name##_show,	\
				gadget_config_desc_##name##_store)

CFG_CONFIG_DESC_ITEM_ATTR(MaxPower);
CFG_CONFIG_DESC_ITEM_ATTR(bmAttributes);

static struct configfs_attribute *gadget_config_attrs[] = {
	&gadget_usb_cfg_MaxPower.attr,
	&gadget_usb_cfg_bmAttributes.attr,
	NULL,
};

static struct config_item_type gadget_config_type = {
	.ct_item_ops	= &gadget_config_item_ops,
	.ct_attrs	= gadget_config_attrs,
	.ct_owner	= THIS_MODULE,
};

static struct config_item_type gadget_root_type = {
	.ct_item_ops	= &gadget_root_item_ops,
	.ct_attrs	= gadget_root_attrs,
	.ct_owner	= THIS_MODULE,
};

static void composite_init_dev(struct usb_composite_dev *cdev)
{
	spin_lock_init(&cdev->lock);
	INIT_LIST_HEAD(&cdev->configs);
	INIT_LIST_HEAD(&cdev->gstrings);
}

static struct config_group *function_make(
		struct config_group *group,
		const char *name)
{
	struct gadget_info *gi;
	struct usb_function_instance *fi;
	char buf[MAX_NAME_LEN];
	char *func_name;
	char *instance_name;
	int ret;

	ret = snprintf(buf, MAX_NAME_LEN, "%s", name);
	if (ret >= MAX_NAME_LEN)
		return ERR_PTR(-ENAMETOOLONG);

	func_name = buf;
	instance_name = strchr(func_name, '.');
	if (!instance_name) {
		pr_err("Unable to locate . in FUNC.INSTANCE\n");
		return ERR_PTR(-EINVAL);
	}
	*instance_name = '\0';
	instance_name++;

	fi = usb_get_function_instance(func_name);
	if (IS_ERR(fi))
		return ERR_CAST(fi);

	ret = config_item_set_name(&fi->group.cg_item, name);
	if (ret) {
		usb_put_function_instance(fi);
		return ERR_PTR(ret);
	}
	if (fi->set_inst_name) {
		ret = fi->set_inst_name(fi, instance_name);
		if (ret) {
			usb_put_function_instance(fi);
			return ERR_PTR(ret);
		}
	}

	gi = container_of(group, struct gadget_info, functions_group);

	mutex_lock(&gi->lock);
	list_add_tail(&fi->cfs_list, &gi->available_func);
	mutex_unlock(&gi->lock);
	return &fi->group;
}

static void function_drop(
		struct config_group *group,
		struct config_item *item)
{
	struct usb_function_instance *fi = to_usb_function_instance(item);
	struct gadget_info *gi;

	gi = container_of(group, struct gadget_info, functions_group);

	mutex_lock(&gi->lock);
	list_del(&fi->cfs_list);
	mutex_unlock(&gi->lock);
	config_item_put(item);
}

static struct configfs_group_operations functions_ops = {
	.make_group     = &function_make,
	.drop_item      = &function_drop,
};

static struct config_item_type functions_type = {
	.ct_group_ops   = &functions_ops,
	.ct_owner       = THIS_MODULE,
};

CONFIGFS_ATTR_STRUCT(gadget_config_name);
GS_STRINGS_RW(gadget_config_name, configuration);

static struct configfs_attribute *gadget_config_name_langid_attrs[] = {
	&gadget_config_name_configuration.attr,
	NULL,
};

static void gadget_config_name_attr_release(struct config_item *item)
{
	struct gadget_config_name *cn = to_gadget_config_name(item);

	kfree(cn->configuration);

	list_del(&cn->list);
	kfree(cn);
}

USB_CONFIG_STRING_RW_OPS(gadget_config_name);
USB_CONFIG_STRINGS_LANG(gadget_config_name, config_usb_cfg);

static struct config_group *config_desc_make(
		struct config_group *group,
		const char *name)
{
	struct gadget_info *gi;
	struct config_usb_cfg *cfg;
	char buf[MAX_NAME_LEN];
	char *num_str;
	u8 num;
	int ret;

	gi = container_of(group, struct gadget_info, configs_group);
	ret = snprintf(buf, MAX_NAME_LEN, "%s", name);
	if (ret >= MAX_NAME_LEN)
		return ERR_PTR(-ENAMETOOLONG);

	num_str = strchr(buf, '.');
	if (!num_str) {
		pr_err("Unable to locate . in name.bConfigurationValue\n");
		return ERR_PTR(-EINVAL);
	}

	*num_str = '\0';
	num_str++;

	if (!strlen(buf))
		return ERR_PTR(-EINVAL);

	ret = kstrtou8(num_str, 0, &num);
	if (ret)
		return ERR_PTR(ret);

	cfg = kzalloc(sizeof(*cfg), GFP_KERNEL);
	if (!cfg)
		return ERR_PTR(-ENOMEM);
	cfg->c.label = kstrdup(buf, GFP_KERNEL);
	if (!cfg->c.label) {
		ret = -ENOMEM;
		goto err;
	}
	cfg->c.bConfigurationValue = num;
	cfg->c.MaxPower = CONFIG_USB_GADGET_VBUS_DRAW;
	cfg->c.bmAttributes = USB_CONFIG_ATT_ONE;
	INIT_LIST_HEAD(&cfg->string_list);
	INIT_LIST_HEAD(&cfg->func_list);

	cfg->group.default_groups = cfg->default_groups;
	cfg->default_groups[0] = &cfg->strings_group;

	config_group_init_type_name(&cfg->group, name,
				&gadget_config_type);
	config_group_init_type_name(&cfg->strings_group, "strings",
			&gadget_config_name_strings_type);

	ret = usb_add_config_only(&gi->cdev, &cfg->c);
	if (ret)
		goto err;

	return &cfg->group;
err:
	kfree(cfg->c.label);
	kfree(cfg);
	return ERR_PTR(ret);
}

static void config_desc_drop(
		struct config_group *group,
		struct config_item *item)
{
	config_item_put(item);
}

static struct configfs_group_operations config_desc_ops = {
	.make_group     = &config_desc_make,
	.drop_item      = &config_desc_drop,
};

static struct config_item_type config_desc_type = {
	.ct_group_ops   = &config_desc_ops,
	.ct_owner       = THIS_MODULE,
};

CONFIGFS_ATTR_STRUCT(gadget_strings);
GS_STRINGS_RW(gadget_strings, manufacturer);
GS_STRINGS_RW(gadget_strings, product);
GS_STRINGS_RW(gadget_strings, serialnumber);

static struct configfs_attribute *gadget_strings_langid_attrs[] = {
	&gadget_strings_manufacturer.attr,
	&gadget_strings_product.attr,
	&gadget_strings_serialnumber.attr,
	NULL,
};

static void gadget_strings_attr_release(struct config_item *item)
{
	struct gadget_strings *gs = to_gadget_strings(item);

	kfree(gs->manufacturer);
	kfree(gs->product);
	kfree(gs->serialnumber);

	list_del(&gs->list);
	kfree(gs);
}

USB_CONFIG_STRING_RW_OPS(gadget_strings);
USB_CONFIG_STRINGS_LANG(gadget_strings, gadget_info);

static inline struct os_desc *to_os_desc(struct config_item *item)
{
	return container_of(to_config_group(item), struct os_desc, group);
}

CONFIGFS_ATTR_STRUCT(os_desc);
CONFIGFS_ATTR_OPS(os_desc);

static ssize_t os_desc_use_show(struct os_desc *os_desc, char *page)
{
	struct gadget_info *gi;

	gi = to_gadget_info(os_desc->group.cg_item.ci_parent);

	return sprintf(page, "%d", gi->use_os_desc);
}

static ssize_t os_desc_use_store(struct os_desc *os_desc, const char *page,
				 size_t len)
{
	struct gadget_info *gi;
	int ret;
	bool use;

	gi = to_gadget_info(os_desc->group.cg_item.ci_parent);

	mutex_lock(&gi->lock);
	ret = strtobool(page, &use);
	if (!ret) {
		gi->use_os_desc = use;
		ret = len;
	}
	mutex_unlock(&gi->lock);

	return ret;
}

static struct os_desc_attribute os_desc_use =
	__CONFIGFS_ATTR(use, S_IRUGO | S_IWUSR,
			os_desc_use_show,
			os_desc_use_store);

static ssize_t os_desc_b_vendor_code_show(struct os_desc *os_desc, char *page)
{
	struct gadget_info *gi;

	gi = to_gadget_info(os_desc->group.cg_item.ci_parent);

	return sprintf(page, "%d", gi->b_vendor_code);
}

static ssize_t os_desc_b_vendor_code_store(struct os_desc *os_desc,
					   const char *page, size_t len)
{
	struct gadget_info *gi;
	int ret;
	u8 b_vendor_code;

	gi = to_gadget_info(os_desc->group.cg_item.ci_parent);

	mutex_lock(&gi->lock);
	ret = kstrtou8(page, 0, &b_vendor_code);
	if (!ret) {
		gi->b_vendor_code = b_vendor_code;
		ret = len;
	}
	mutex_unlock(&gi->lock);

	return ret;
}

static struct os_desc_attribute os_desc_b_vendor_code =
	__CONFIGFS_ATTR(b_vendor_code, S_IRUGO | S_IWUSR,
			os_desc_b_vendor_code_show,
			os_desc_b_vendor_code_store);

static ssize_t os_desc_qw_sign_show(struct os_desc *os_desc, char *page)
{
	struct gadget_info *gi;

	gi = to_gadget_info(os_desc->group.cg_item.ci_parent);

	memcpy(page, gi->qw_sign, OS_STRING_QW_SIGN_LEN);

	return OS_STRING_QW_SIGN_LEN;
}

static ssize_t os_desc_qw_sign_store(struct os_desc *os_desc, const char *page,
				     size_t len)
{
	struct gadget_info *gi;
	int res, l;

	gi = to_gadget_info(os_desc->group.cg_item.ci_parent);
	l = min((int)len, OS_STRING_QW_SIGN_LEN >> 1);
	if (page[l - 1] == '\n')
		--l;

	mutex_lock(&gi->lock);
	res = utf8s_to_utf16s(page, l,
			      UTF16_LITTLE_ENDIAN, (wchar_t *) gi->qw_sign,
			      OS_STRING_QW_SIGN_LEN);
	if (res > 0)
		res = len;
	mutex_unlock(&gi->lock);

	return res;
}

static struct os_desc_attribute os_desc_qw_sign =
	__CONFIGFS_ATTR(qw_sign, S_IRUGO | S_IWUSR,
			os_desc_qw_sign_show,
			os_desc_qw_sign_store);

static struct configfs_attribute *os_desc_attrs[] = {
	&os_desc_use.attr,
	&os_desc_b_vendor_code.attr,
	&os_desc_qw_sign.attr,
	NULL,
};

static void os_desc_attr_release(struct config_item *item)
{
	struct os_desc *os_desc = to_os_desc(item);
	kfree(os_desc);
}

static int os_desc_link(struct config_item *os_desc_ci,
			struct config_item *usb_cfg_ci)
{
	struct gadget_info *gi = container_of(to_config_group(os_desc_ci),
					struct gadget_info, os_desc_group);
	struct usb_composite_dev *cdev = &gi->cdev;
	struct config_usb_cfg *c_target =
		container_of(to_config_group(usb_cfg_ci),
			     struct config_usb_cfg, group);
	struct usb_configuration *c;
	int ret;

	mutex_lock(&gi->lock);
	list_for_each_entry(c, &cdev->configs, list) {
		if (c == &c_target->c)
			break;
	}
	if (c != &c_target->c) {
		ret = -EINVAL;
		goto out;
	}

	if (cdev->os_desc_config) {
		ret = -EBUSY;
		goto out;
	}

	cdev->os_desc_config = &c_target->c;
	ret = 0;

out:
	mutex_unlock(&gi->lock);
	return ret;
}

static int os_desc_unlink(struct config_item *os_desc_ci,
			  struct config_item *usb_cfg_ci)
{
	struct gadget_info *gi = container_of(to_config_group(os_desc_ci),
					struct gadget_info, os_desc_group);
	struct usb_composite_dev *cdev = &gi->cdev;

	mutex_lock(&gi->lock);
	if (gi->udc_name)
		unregister_gadget(gi);
	cdev->os_desc_config = NULL;
	WARN_ON(gi->udc_name);
	mutex_unlock(&gi->lock);
	return 0;
}

static struct configfs_item_operations os_desc_ops = {
	.release                = os_desc_attr_release,
	.show_attribute         = os_desc_attr_show,
	.store_attribute        = os_desc_attr_store,
	.allow_link		= os_desc_link,
	.drop_link		= os_desc_unlink,
};

static struct config_item_type os_desc_type = {
	.ct_item_ops	= &os_desc_ops,
	.ct_attrs	= os_desc_attrs,
	.ct_owner	= THIS_MODULE,
};

CONFIGFS_ATTR_STRUCT(usb_os_desc);
CONFIGFS_ATTR_OPS(usb_os_desc);


static inline struct usb_os_desc_ext_prop
*to_usb_os_desc_ext_prop(struct config_item *item)
{
	return container_of(item, struct usb_os_desc_ext_prop, item);
}

CONFIGFS_ATTR_STRUCT(usb_os_desc_ext_prop);
CONFIGFS_ATTR_OPS(usb_os_desc_ext_prop);

static ssize_t ext_prop_type_show(struct usb_os_desc_ext_prop *ext_prop,
				  char *page)
{
	return sprintf(page, "%d", ext_prop->type);
}

static ssize_t ext_prop_type_store(struct usb_os_desc_ext_prop *ext_prop,
				   const char *page, size_t len)
{
	struct usb_os_desc *desc = to_usb_os_desc(ext_prop->item.ci_parent);
	u8 type;
	int ret;

	if (desc->opts_mutex)
		mutex_lock(desc->opts_mutex);
	ret = kstrtou8(page, 0, &type);
	if (ret)
		goto end;
	if (type < USB_EXT_PROP_UNICODE || type > USB_EXT_PROP_UNICODE_MULTI) {
		ret = -EINVAL;
		goto end;
	}

	if ((ext_prop->type == USB_EXT_PROP_BINARY ||
	    ext_prop->type == USB_EXT_PROP_LE32 ||
	    ext_prop->type == USB_EXT_PROP_BE32) &&
	    (type == USB_EXT_PROP_UNICODE ||
	    type == USB_EXT_PROP_UNICODE_ENV ||
	    type == USB_EXT_PROP_UNICODE_LINK))
		ext_prop->data_len <<= 1;
	else if ((ext_prop->type == USB_EXT_PROP_UNICODE ||
		   ext_prop->type == USB_EXT_PROP_UNICODE_ENV ||
		   ext_prop->type == USB_EXT_PROP_UNICODE_LINK) &&
		   (type == USB_EXT_PROP_BINARY ||
		   type == USB_EXT_PROP_LE32 ||
		   type == USB_EXT_PROP_BE32))
		ext_prop->data_len >>= 1;
	ext_prop->type = type;
	ret = len;

end:
	if (desc->opts_mutex)
		mutex_unlock(desc->opts_mutex);
	return ret;
}

static ssize_t ext_prop_data_show(struct usb_os_desc_ext_prop *ext_prop,
				  char *page)
{
	int len = ext_prop->data_len;

	if (ext_prop->type == USB_EXT_PROP_UNICODE ||
	    ext_prop->type == USB_EXT_PROP_UNICODE_ENV ||
	    ext_prop->type == USB_EXT_PROP_UNICODE_LINK)
		len >>= 1;
	memcpy(page, ext_prop->data, len);

	return len;
}

static ssize_t ext_prop_data_store(struct usb_os_desc_ext_prop *ext_prop,
				   const char *page, size_t len)
{
	struct usb_os_desc *desc = to_usb_os_desc(ext_prop->item.ci_parent);
	char *new_data;
	size_t ret_len = len;

	if (page[len - 1] == '\n' || page[len - 1] == '\0')
		--len;
	new_data = kmemdup(page, len, GFP_KERNEL);
	if (!new_data)
		return -ENOMEM;

	if (desc->opts_mutex)
		mutex_lock(desc->opts_mutex);
	kfree(ext_prop->data);
	ext_prop->data = new_data;
	desc->ext_prop_len -= ext_prop->data_len;
	ext_prop->data_len = len;
	desc->ext_prop_len += ext_prop->data_len;
	if (ext_prop->type == USB_EXT_PROP_UNICODE ||
	    ext_prop->type == USB_EXT_PROP_UNICODE_ENV ||
	    ext_prop->type == USB_EXT_PROP_UNICODE_LINK) {
		desc->ext_prop_len -= ext_prop->data_len;
		ext_prop->data_len <<= 1;
		ext_prop->data_len += 2;
		desc->ext_prop_len += ext_prop->data_len;
	}
	if (desc->opts_mutex)
		mutex_unlock(desc->opts_mutex);
	return ret_len;
}

static struct usb_os_desc_ext_prop_attribute ext_prop_type =
	__CONFIGFS_ATTR(type, S_IRUGO | S_IWUSR,
			ext_prop_type_show, ext_prop_type_store);

static struct usb_os_desc_ext_prop_attribute ext_prop_data =
	__CONFIGFS_ATTR(data, S_IRUGO | S_IWUSR,
			ext_prop_data_show, ext_prop_data_store);

static struct configfs_attribute *ext_prop_attrs[] = {
	&ext_prop_type.attr,
	&ext_prop_data.attr,
	NULL,
};

static void usb_os_desc_ext_prop_release(struct config_item *item)
{
	struct usb_os_desc_ext_prop *ext_prop = to_usb_os_desc_ext_prop(item);

	kfree(ext_prop); /* frees a whole chunk */
}

static struct configfs_item_operations ext_prop_ops = {
	.release		= usb_os_desc_ext_prop_release,
	.show_attribute		= usb_os_desc_ext_prop_attr_show,
	.store_attribute	= usb_os_desc_ext_prop_attr_store,
};

static struct config_item *ext_prop_make(
		struct config_group *group,
		const char *name)
{
	struct usb_os_desc_ext_prop *ext_prop;
	struct config_item_type *ext_prop_type;
	struct usb_os_desc *desc;
	char *vlabuf;

	vla_group(data_chunk);
	vla_item(data_chunk, struct usb_os_desc_ext_prop, ext_prop, 1);
	vla_item(data_chunk, struct config_item_type, ext_prop_type, 1);

	vlabuf = kzalloc(vla_group_size(data_chunk), GFP_KERNEL);
	if (!vlabuf)
		return ERR_PTR(-ENOMEM);

	ext_prop = vla_ptr(vlabuf, data_chunk, ext_prop);
	ext_prop_type = vla_ptr(vlabuf, data_chunk, ext_prop_type);

	desc = container_of(group, struct usb_os_desc, group);
	ext_prop_type->ct_item_ops = &ext_prop_ops;
	ext_prop_type->ct_attrs = ext_prop_attrs;
	ext_prop_type->ct_owner = desc->owner;

	config_item_init_type_name(&ext_prop->item, name, ext_prop_type);

	ext_prop->name = kstrdup(name, GFP_KERNEL);
	if (!ext_prop->name) {
		kfree(vlabuf);
		return ERR_PTR(-ENOMEM);
	}
	desc->ext_prop_len += 14;
	ext_prop->name_len = 2 * strlen(ext_prop->name) + 2;
	if (desc->opts_mutex)
		mutex_lock(desc->opts_mutex);
	desc->ext_prop_len += ext_prop->name_len;
	list_add_tail(&ext_prop->entry, &desc->ext_prop);
	++desc->ext_prop_count;
	if (desc->opts_mutex)
		mutex_unlock(desc->opts_mutex);

	return &ext_prop->item;
}

static void ext_prop_drop(struct config_group *group, struct config_item *item)
{
	struct usb_os_desc_ext_prop *ext_prop = to_usb_os_desc_ext_prop(item);
	struct usb_os_desc *desc = to_usb_os_desc(&group->cg_item);

	if (desc->opts_mutex)
		mutex_lock(desc->opts_mutex);
	list_del(&ext_prop->entry);
	--desc->ext_prop_count;
	kfree(ext_prop->name);
	desc->ext_prop_len -= (ext_prop->name_len + ext_prop->data_len + 14);
	if (desc->opts_mutex)
		mutex_unlock(desc->opts_mutex);
	config_item_put(item);
}

static struct configfs_group_operations interf_grp_ops = {
	.make_item	= &ext_prop_make,
	.drop_item	= &ext_prop_drop,
};

static struct configfs_item_operations interf_item_ops = {
	.show_attribute		= usb_os_desc_attr_show,
	.store_attribute	= usb_os_desc_attr_store,
};

static ssize_t interf_grp_compatible_id_show(struct usb_os_desc *desc,
					     char *page)
{
	memcpy(page, desc->ext_compat_id, 8);
	return 8;
}

static ssize_t interf_grp_compatible_id_store(struct usb_os_desc *desc,
					      const char *page, size_t len)
{
	int l;

	l = min_t(int, 8, len);
	if (page[l - 1] == '\n')
		--l;
	if (desc->opts_mutex)
		mutex_lock(desc->opts_mutex);
	memcpy(desc->ext_compat_id, page, l);

	if (desc->opts_mutex)
		mutex_unlock(desc->opts_mutex);

	return len;
}

static struct usb_os_desc_attribute interf_grp_attr_compatible_id =
	__CONFIGFS_ATTR(compatible_id, S_IRUGO | S_IWUSR,
			interf_grp_compatible_id_show,
			interf_grp_compatible_id_store);

static ssize_t interf_grp_sub_compatible_id_show(struct usb_os_desc *desc,
						 char *page)
{
	memcpy(page, desc->ext_compat_id + 8, 8);
	return 8;
}

static ssize_t interf_grp_sub_compatible_id_store(struct usb_os_desc *desc,
						  const char *page, size_t len)
{
	int l;

	l = min_t(int, 8, len);
	if (page[l - 1] == '\n')
		--l;
	if (desc->opts_mutex)
		mutex_lock(desc->opts_mutex);
	memcpy(desc->ext_compat_id + 8, page, l);

	if (desc->opts_mutex)
		mutex_unlock(desc->opts_mutex);

	return len;
}

static struct usb_os_desc_attribute interf_grp_attr_sub_compatible_id =
	__CONFIGFS_ATTR(sub_compatible_id, S_IRUGO | S_IWUSR,
			interf_grp_sub_compatible_id_show,
			interf_grp_sub_compatible_id_store);

static struct configfs_attribute *interf_grp_attrs[] = {
	&interf_grp_attr_compatible_id.attr,
	&interf_grp_attr_sub_compatible_id.attr,
	NULL
};

int usb_os_desc_prepare_interf_dir(struct config_group *parent,
				   int n_interf,
				   struct usb_os_desc **desc,
				   char **names,
				   struct module *owner)
{
	struct config_group **f_default_groups, *os_desc_group,
				**interface_groups;
	struct config_item_type *os_desc_type, *interface_type;

	vla_group(data_chunk);
	vla_item(data_chunk, struct config_group *, f_default_groups, 2);
	vla_item(data_chunk, struct config_group, os_desc_group, 1);
	vla_item(data_chunk, struct config_group *, interface_groups,
		 n_interf + 1);
	vla_item(data_chunk, struct config_item_type, os_desc_type, 1);
	vla_item(data_chunk, struct config_item_type, interface_type, 1);

	char *vlabuf = kzalloc(vla_group_size(data_chunk), GFP_KERNEL);
	if (!vlabuf)
		return -ENOMEM;

	f_default_groups = vla_ptr(vlabuf, data_chunk, f_default_groups);
	os_desc_group = vla_ptr(vlabuf, data_chunk, os_desc_group);
	os_desc_type = vla_ptr(vlabuf, data_chunk, os_desc_type);
	interface_groups = vla_ptr(vlabuf, data_chunk, interface_groups);
	interface_type = vla_ptr(vlabuf, data_chunk, interface_type);

	parent->default_groups = f_default_groups;
	os_desc_type->ct_owner = owner;
	config_group_init_type_name(os_desc_group, "os_desc", os_desc_type);
	f_default_groups[0] = os_desc_group;

	os_desc_group->default_groups = interface_groups;
	interface_type->ct_item_ops = &interf_item_ops;
	interface_type->ct_group_ops = &interf_grp_ops;
	interface_type->ct_attrs = interf_grp_attrs;
	interface_type->ct_owner = owner;

	while (n_interf--) {
		struct usb_os_desc *d;

		d = desc[n_interf];
		d->owner = owner;
		config_group_init_type_name(&d->group, "", interface_type);
		config_item_set_name(&d->group.cg_item, "interface.%s",
				     names[n_interf]);
		interface_groups[n_interf] = &d->group;
	}

	return 0;
}
EXPORT_SYMBOL(usb_os_desc_prepare_interf_dir);

static int configfs_do_nothing(struct usb_composite_dev *cdev)
{
	WARN_ON(1);
	return -EINVAL;
}

int composite_dev_prepare(struct usb_composite_driver *composite,
		struct usb_composite_dev *dev);

int composite_os_desc_req_prepare(struct usb_composite_dev *cdev,
				  struct usb_ep *ep0);

static void purge_configs_funcs(struct gadget_info *gi)
{
	struct usb_configuration	*c;

	list_for_each_entry(c, &gi->cdev.configs, list) {
		struct usb_function *f, *tmp;
		struct config_usb_cfg *cfg;

		cfg = container_of(c, struct config_usb_cfg, c);

		list_for_each_entry_safe(f, tmp, &c->functions, list) {

			list_move_tail(&f->list, &cfg->func_list);
			if (f->unbind) {
				dev_err(&gi->cdev.gadget->dev, "unbind function"
						" '%s'/%p\n", f->name, f);
				f->unbind(c, f);
			}
		}
		c->next_interface_id = 0;
		memset(c->interface, 0, sizeof(c->interface));
		c->superspeed = 0;
		c->highspeed = 0;
		c->fullspeed = 0;
	}
}

static int configfs_composite_bind(struct usb_gadget *gadget,
		struct usb_gadget_driver *gdriver)
{
	struct usb_composite_driver     *composite = to_cdriver(gdriver);
	struct gadget_info		*gi = container_of(composite,
						struct gadget_info, composite);
	struct usb_composite_dev	*cdev = &gi->cdev;
	struct usb_configuration	*c;
	struct usb_string		*s;
	unsigned			i;
	int				ret;

	/* the gi->lock is hold by the caller */
	cdev->gadget = gadget;
	set_gadget_data(gadget, cdev);
	ret = composite_dev_prepare(composite, cdev);
	if (ret)
		return ret;
	/* and now the gadget bind */
	ret = -EINVAL;

	if (list_empty(&gi->cdev.configs)) {
		pr_err("Need at least one configuration in %s.\n",
				gi->composite.name);
		goto err_comp_cleanup;
	}


	list_for_each_entry(c, &gi->cdev.configs, list) {
		struct config_usb_cfg *cfg;

		cfg = container_of(c, struct config_usb_cfg, c);
		if (list_empty(&cfg->func_list)) {
			pr_err("Config %s/%d of %s needs at least one function.\n",
			      c->label, c->bConfigurationValue,
			      gi->composite.name);
			goto err_comp_cleanup;
		}
	}

	/* init all strings */
	if (!list_empty(&gi->string_list)) {
		struct gadget_strings *gs;

		i = 0;
		list_for_each_entry(gs, &gi->string_list, list) {

			gi->gstrings[i] = &gs->stringtab_dev;
			gs->stringtab_dev.strings = gs->strings;
			gs->strings[USB_GADGET_MANUFACTURER_IDX].s =
				gs->manufacturer;
			gs->strings[USB_GADGET_PRODUCT_IDX].s = gs->product;
			gs->strings[USB_GADGET_SERIAL_IDX].s = gs->serialnumber;
			i++;
		}
		gi->gstrings[i] = NULL;
		s = usb_gstrings_attach(&gi->cdev, gi->gstrings,
				USB_GADGET_FIRST_AVAIL_IDX);
		if (IS_ERR(s)) {
			ret = PTR_ERR(s);
			goto err_comp_cleanup;
		}

		gi->cdev.desc.iManufacturer = s[USB_GADGET_MANUFACTURER_IDX].id;
		gi->cdev.desc.iProduct = s[USB_GADGET_PRODUCT_IDX].id;
		gi->cdev.desc.iSerialNumber = s[USB_GADGET_SERIAL_IDX].id;
	}

	if (gi->use_os_desc) {
		cdev->use_os_string = true;
		cdev->b_vendor_code = gi->b_vendor_code;
		memcpy(cdev->qw_sign, gi->qw_sign, OS_STRING_QW_SIGN_LEN);
	}

	/* Go through all configs, attach all functions */
	list_for_each_entry(c, &gi->cdev.configs, list) {
		struct config_usb_cfg *cfg;
		struct usb_function *f;
		struct usb_function *tmp;
		struct gadget_config_name *cn;

		cfg = container_of(c, struct config_usb_cfg, c);
		if (!list_empty(&cfg->string_list)) {
			i = 0;
			list_for_each_entry(cn, &cfg->string_list, list) {
				cfg->gstrings[i] = &cn->stringtab_dev;
				cn->stringtab_dev.strings = &cn->strings;
				cn->strings.s = cn->configuration;
				i++;
			}
			cfg->gstrings[i] = NULL;
			s = usb_gstrings_attach(&gi->cdev, cfg->gstrings, 1);
			if (IS_ERR(s)) {
				ret = PTR_ERR(s);
				goto err_comp_cleanup;
			}
			c->iConfiguration = s[0].id;
		}

		list_for_each_entry_safe(f, tmp, &cfg->func_list, list) {
			list_del(&f->list);
			ret = usb_add_function(c, f);
			if (ret) {
				list_add(&f->list, &cfg->func_list);
				goto err_purge_funcs;
			}
		}
		usb_ep_autoconfig_reset(cdev->gadget);
	}
	if (cdev->use_os_string) {
		ret = composite_os_desc_req_prepare(cdev, gadget->ep0);
		if (ret)
			goto err_purge_funcs;
	}

	usb_ep_autoconfig_reset(cdev->gadget);
	return 0;

err_purge_funcs:
	purge_configs_funcs(gi);
err_comp_cleanup:
	composite_dev_cleanup(cdev);
	return ret;
}

static void configfs_composite_unbind(struct usb_gadget *gadget)
{
	struct usb_composite_dev	*cdev;
	struct gadget_info		*gi;

	/* the gi->lock is hold by the caller */

	cdev = get_gadget_data(gadget);
	gi = container_of(cdev, struct gadget_info, cdev);

	purge_configs_funcs(gi);
	composite_dev_cleanup(cdev);
	usb_ep_autoconfig_reset(cdev->gadget);
	cdev->gadget = NULL;
	set_gadget_data(gadget, NULL);
}

static const struct usb_gadget_driver configfs_driver_template = {
	.bind           = configfs_composite_bind,
	.unbind         = configfs_composite_unbind,

	.setup          = composite_setup,
	.reset          = composite_disconnect,
	.disconnect     = composite_disconnect,

	.suspend	= composite_suspend,
	.resume		= composite_resume,

	.max_speed	= USB_SPEED_SUPER,
	.driver = {
		.owner          = THIS_MODULE,
		.name		= "configfs-gadget",
	},
};

static struct config_group *gadgets_make(
		struct config_group *group,
		const char *name)
{
	struct gadget_info *gi;

	gi = kzalloc(sizeof(*gi), GFP_KERNEL);
	if (!gi)
		return ERR_PTR(-ENOMEM);

	gi->group.default_groups = gi->default_groups;
	gi->group.default_groups[0] = &gi->functions_group;
	gi->group.default_groups[1] = &gi->configs_group;
	gi->group.default_groups[2] = &gi->strings_group;
	gi->group.default_groups[3] = &gi->os_desc_group;

	config_group_init_type_name(&gi->functions_group, "functions",
			&functions_type);
	config_group_init_type_name(&gi->configs_group, "configs",
			&config_desc_type);
	config_group_init_type_name(&gi->strings_group, "strings",
			&gadget_strings_strings_type);
	config_group_init_type_name(&gi->os_desc_group, "os_desc",
			&os_desc_type);

	gi->composite.bind = configfs_do_nothing;
	gi->composite.unbind = configfs_do_nothing;
	gi->composite.suspend = NULL;
	gi->composite.resume = NULL;
	gi->composite.max_speed = USB_SPEED_SUPER;

	mutex_init(&gi->lock);
	INIT_LIST_HEAD(&gi->string_list);
	INIT_LIST_HEAD(&gi->available_func);

	composite_init_dev(&gi->cdev);
	gi->cdev.desc.bLength = USB_DT_DEVICE_SIZE;
	gi->cdev.desc.bDescriptorType = USB_DT_DEVICE;
	gi->cdev.desc.bcdDevice = cpu_to_le16(get_default_bcdDevice());

	gi->composite.gadget_driver = configfs_driver_template;

	gi->composite.gadget_driver.function = kstrdup(name, GFP_KERNEL);
	gi->composite.name = gi->composite.gadget_driver.function;

	if (!gi->composite.gadget_driver.function)
		goto err;

#ifdef CONFIG_USB_OTG
	gi->otg.bLength = sizeof(struct usb_otg_descriptor);
	gi->otg.bDescriptorType = USB_DT_OTG;
	gi->otg.bmAttributes = USB_OTG_SRP | USB_OTG_HNP;
#endif

	config_group_init_type_name(&gi->group, name,
				&gadget_root_type);
	return &gi->group;
err:
	kfree(gi);
	return ERR_PTR(-ENOMEM);
}

static void gadgets_drop(struct config_group *group, struct config_item *item)
{
	config_item_put(item);
}

static struct configfs_group_operations gadgets_ops = {
	.make_group     = &gadgets_make,
	.drop_item      = &gadgets_drop,
};

static struct config_item_type gadgets_type = {
	.ct_group_ops   = &gadgets_ops,
	.ct_owner       = THIS_MODULE,
};

static struct configfs_subsystem gadget_subsys = {
	.su_group = {
		.cg_item = {
			.ci_namebuf = "usb_gadget",
			.ci_type = &gadgets_type,
		},
	},
	.su_mutex = __MUTEX_INITIALIZER(gadget_subsys.su_mutex),
};

void unregister_gadget_item(struct config_item *item)
{
	struct gadget_info *gi = to_gadget_info(item);

	unregister_gadget(gi);
}
EXPORT_SYMBOL_GPL(unregister_gadget_item);

static int __init gadget_cfs_init(void)
{
	int ret;

	config_group_init(&gadget_subsys.su_group);

	ret = configfs_register_subsystem(&gadget_subsys);
	return ret;
}
module_init(gadget_cfs_init);

static void __exit gadget_cfs_exit(void)
{
	configfs_unregister_subsystem(&gadget_subsys);
}
module_exit(gadget_cfs_exit);
