/*
 * Copyright (c) 2010 Red Hat Inc.
 * Author : Dave Airlie <airlied@redhat.com>
 *
 * Licensed under GPLv2
 *
 * vga_switcheroo.h - Support for laptop with dual GPU using one set of outputs
 */

#ifndef _LINUX_VGA_SWITCHEROO_H_
#define _LINUX_VGA_SWITCHEROO_H_

#include <linux/fb.h>

struct pci_dev;

enum vga_switcheroo_state {
	VGA_SWITCHEROO_OFF,
	VGA_SWITCHEROO_ON,
	/* below are referred only from vga_switcheroo_get_client_state() */
	VGA_SWITCHEROO_INIT,
	VGA_SWITCHEROO_NOT_FOUND,
};

enum vga_switcheroo_client_id {
	VGA_SWITCHEROO_IGD,
	VGA_SWITCHEROO_DIS,
	VGA_SWITCHEROO_MAX_CLIENTS,
};

struct vga_switcheroo_handler {
	int (*switchto)(enum vga_switcheroo_client_id id);
	int (*power_state)(enum vga_switcheroo_client_id id,
			   enum vga_switcheroo_state state);
	int (*init)(void);
	int (*get_client_id)(struct pci_dev *pdev);
};

struct vga_switcheroo_client_ops {
	void (*set_gpu_state)(struct pci_dev *dev, enum vga_switcheroo_state);
	void (*reprobe)(struct pci_dev *dev);
	bool (*can_switch)(struct pci_dev *dev);
};

#if defined(CONFIG_VGA_SWITCHEROO)
void vga_switcheroo_unregister_client(struct pci_dev *dev);
int vga_switcheroo_register_client(struct pci_dev *dev,
				   const struct vga_switcheroo_client_ops *ops,
				   bool driver_power_control);
int vga_switcheroo_register_audio_client(struct pci_dev *pdev,
					 const struct vga_switcheroo_client_ops *ops,
					 int id, bool active);

void vga_switcheroo_client_fb_set(struct pci_dev *dev,
				  struct fb_info *info);

int vga_switcheroo_register_handler(struct vga_switcheroo_handler *handler);
void vga_switcheroo_unregister_handler(void);

int vga_switcheroo_process_delayed_switch(void);

int vga_switcheroo_get_client_state(struct pci_dev *dev);

void vga_switcheroo_set_dynamic_switch(struct pci_dev *pdev, enum vga_switcheroo_state dynamic);

int vga_switcheroo_init_domain_pm_ops(struct device *dev, struct dev_pm_domain *domain);
void vga_switcheroo_fini_domain_pm_ops(struct device *dev);
int vga_switcheroo_init_domain_pm_optimus_hdmi_audio(struct device *dev, struct dev_pm_domain *domain);
#else

static inline void vga_switcheroo_unregister_client(struct pci_dev *dev) {}
static inline int vga_switcheroo_register_client(struct pci_dev *dev,
		const struct vga_switcheroo_client_ops *ops, bool driver_power_control) { return 0; }
static inline void vga_switcheroo_client_fb_set(struct pci_dev *dev, struct fb_info *info) {}
static inline int vga_switcheroo_register_handler(struct vga_switcheroo_handler *handler) { return 0; }
static inline int vga_switcheroo_register_audio_client(struct pci_dev *pdev,
	const struct vga_switcheroo_client_ops *ops,
	int id, bool active) { return 0; }
static inline void vga_switcheroo_unregister_handler(void) {}
static inline int vga_switcheroo_process_delayed_switch(void) { return 0; }
static inline int vga_switcheroo_get_client_state(struct pci_dev *dev) { return VGA_SWITCHEROO_ON; }

static inline void vga_switcheroo_set_dynamic_switch(struct pci_dev *pdev, enum vga_switcheroo_state dynamic) {}

static inline int vga_switcheroo_init_domain_pm_ops(struct device *dev, struct dev_pm_domain *domain) { return -EINVAL; }
static inline void vga_switcheroo_fini_domain_pm_ops(struct device *dev) {}
static inline int vga_switcheroo_init_domain_pm_optimus_hdmi_audio(struct device *dev, struct dev_pm_domain *domain) { return -EINVAL; }

#endif
#endif /* _LINUX_VGA_SWITCHEROO_H_ */
