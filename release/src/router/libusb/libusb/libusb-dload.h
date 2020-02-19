#include "config.h"
#include <libusb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

#define libusb_bulk_transfer (dl_libusb_bulk_transfer)
static int (*dl_libusb_bulk_transfer)(libusb_device_handle *dev_handle,
	unsigned char endpoint, unsigned char *data, int length,
	int *actual_length, unsigned int timeout);

#define libusb_claim_interface (dl_libusb_claim_interface)
static int (*dl_libusb_claim_interface)(libusb_device_handle *dev,
	int interface_number);

#define libusb_release_interface (dl_libusb_release_interface)
static int (*dl_libusb_release_interface)(libusb_device_handle *dev,
	int interface_number);

#define libusb_clear_halt (dl_libusb_clear_halt)
static int (*dl_libusb_clear_halt)(libusb_device_handle *dev,
	unsigned char endpoint);

#define libusb_reset_device (dl_libusb_reset_device)
static int (*dl_libusb_reset_device)(libusb_device_handle *dev);

#define libusb_get_bus_number (dl_libusb_get_bus_number)
static uint8_t (*libusb_get_bus_number)(libusb_device *dev);

#define libusb_open (dl_libusb_open)
static int (*libusb_open)(libusb_device *dev, libusb_device_handle **handle);

#define libusb_close (dl_libusb_close)
static void (*dl_libusb_close)(libusb_device_handle *dev_handle);

#define libusb_set_configuration (dl_libusb_set_configuration)
static int (*dl_libusb_set_configuration)(libusb_device_handle *dev,
	int configuration);

#define libusb_control_transfer (dl_libusb_control_transfer)
static int (*dl_libusb_control_transfer)(libusb_device_handle *dev_handle,
	uint8_t request_type, uint8_t bRequest, uint16_t wValue, uint16_t wIndex,
	unsigned char *data, uint16_t wLength, unsigned int timeout);

#define libusb_detach_kernel_driver (dl_libusb_detach_kernel_driver)
static int (*dl_libusb_detach_kernel_driver)(libusb_device_handle *dev,
	int interface_number);

#define libusb_exit (dl_libusb_exit)
static void (*dl_libusb_exit)(libusb_context *ctx);

#define libusb_set_debug (dl_libusb_set_debug)
static void (*dl_libusb_set_debug)(libusb_context *ctx, int level);

#define libusb_free_config_descriptor (dl_libusb_free_config_descriptor)
static void (*dl_libusb_free_config_descriptor)(
	struct libusb_config_descriptor *config);

#define libusb_free_device_list (dl_libusb_free_device_list)
static void (*dl_libusb_free_device_list)(libusb_device **list,
	int unref_devices);

#define libusb_ref_device (dl_libusb_ref_device)
static libusb_device * (*dl_libusb_ref_device)(libusb_device *dev);

#define libusb_unref_device (dl_libusb_unref_device)
static void (*dl_libusb_unref_device)(libusb_device *dev);

#define libusb_get_config_descriptor (dl_libusb_get_config_descriptor)
static int (*dl_libusb_get_config_descriptor)(libusb_device *dev,
	uint8_t config_index, struct libusb_config_descriptor **config);

#define libusb_free_config_descriptor (dl_libusb_free_config_descriptor)
static void (*dl_libusb_free_config_descriptor)(
	struct libusb_config_descriptor *config);

#define libusb_get_device_address (dl_libusb_get_device_address)
static uint8_t (*dl_libusb_get_device_address)(libusb_device *dev);

#define libusb_get_device_descriptor (dl_libusb_get_device_descriptor)
static int (*dl_libusb_get_device_descriptor)(libusb_device *dev,
	struct libusb_device_descriptor *desc);

#define libusb_get_device_list (dl_libusb_get_device_list)
static ssize_t (*dl_libusb_get_device_list)(libusb_context *ctx,
	libusb_device ***list);

#define libusb_free_device_list (dl_libusb_free_device_list)
static void (*dl_libusb_free_device_list)(libusb_device **list,
	int unref_devices);

#define libusb_get_string_descriptor_ascii (dl_libusb_get_string_descriptor_ascii)
static int (*dl_libusb_get_string_descriptor_ascii)(libusb_device_handle *dev,
	uint8_t desc_index, unsigned char *data, int length);

#define libusb_init (dl_libusb_init)
static int (*dl_libusb_init)(libusb_context **ctx);

#define libusb_exit (dl_libusb_exit)
static void (*dl_libusb_exit)(libusb_context *ctx);

#define libusb_set_debug (dl_libusb_set_debug)
static void (*dl_libusb_set_debug)(libusb_context *ctx, int level);

#define libusb_interrupt_transfer (dl_libusb_interrupt_transfer)
static int (*dl_libusb_interrupt_transfer)(libusb_device_handle *dev_handle,
	unsigned char endpoint, unsigned char *data, int length,
	int *actual_length, unsigned int timeout);

#define libusb_kernel_driver_active (dl_libusb_kernel_driver_active)
static int (*dl_libusb_kernel_driver_active)(libusb_device_handle *dev,
	int interface_number);

#define libusb_detach_kernel_driver (dl_libusb_detach_kernel_driver)
static int (*dl_libusb_detach_kernel_driver)(libusb_device_handle *dev,
	int interface_number);

#define libusb_ref_device (dl_libusb_ref_device)
static libusb_device * (*dl_libusb_ref_device)(libusb_device *dev);

#define libusb_unref_device (dl_libusb_unref_device)
static void (*dl_libusb_unref_device)(libusb_device *dev);

#define libusb_release_interface (dl_libusb_release_interface)
static int (*dl_libusb_release_interface)(libusb_device_handle *dev,
	int interface_number);

#define libusb_reset_device (dl_libusb_reset_device)
static int (*dl_libusb_reset_device)(libusb_device_handle *dev);

#define libusb_set_configuration (dl_libusb_set_configuration)
static int (*dl_libusb_set_configuration)(libusb_device_handle *dev,
	int configuration);

#define libusb_claim_interface (dl_libusb_claim_interface)
static int (*dl_libusb_claim_interface)(libusb_device_handle *dev,
	int interface_number);

#define libusb_set_debug (dl_libusb_set_debug)
static void (*dl_libusb_set_debug)(libusb_context *ctx, int level);

#define libusb_set_interface_alt_setting (dl_libusb_set_interface_alt_setting)
static int (*dl_libusb_set_interface_alt_setting)(libusb_device_handle *dev,
	int interface_number, int alternate_setting);

#define libusb_clear_halt (dl_libusb_clear_halt)
static int (*dl_libusb_clear_halt)(libusb_device_handle *dev,
	unsigned char endpoint);

#define libusb_unref_device (dl_libusb_unref_device)
static void (*dl_libusb_unref_device)(libusb_device *dev);

/* NTH: these functions are copied from libusb.h. we can not use the originals because
 * the #define's above will not modify the calls to libusb_control_transfer. */
static inline int dl_libusb_get_descriptor(libusb_device_handle *dev,
        uint8_t desc_type, uint8_t desc_index, unsigned char *data, int length)
{
        return dl_libusb_control_transfer(dev, LIBUSB_ENDPOINT_IN,
                LIBUSB_REQUEST_GET_DESCRIPTOR, (uint16_t) ((desc_type << 8) | desc_index),
                0, data, (uint16_t) length, 1000);
}

static inline int dl_libusb_get_string_descriptor(libusb_device_handle *dev,
        uint8_t desc_index, uint16_t langid, unsigned char *data, int length)
{
        return dl_libusb_control_transfer(dev, LIBUSB_ENDPOINT_IN,
                LIBUSB_REQUEST_GET_DESCRIPTOR, (uint16_t)((LIBUSB_DT_STRING << 8) | desc_index),
                langid, data, (uint16_t) length, 1000);
}

static void *libusb_dl_handle;

#define libusb_dl_set_call(call)\
	if (!(dl_##call = dlsym(libusb_dl_handle, #call)))\
		goto failure;
static inline void libusb_dl_init(void) {
	if (!(libusb_dl_handle = dlopen(LIBUSB_1_0_SONAME, RTLD_NOW|RTLD_LOCAL)))
		goto failure;
	libusb_dl_set_call(libusb_bulk_transfer);
	libusb_dl_set_call(libusb_claim_interface);
	libusb_dl_set_call(libusb_clear_halt);
	libusb_dl_set_call(libusb_get_bus_number);
	libusb_dl_set_call(libusb_open);
	libusb_dl_set_call(libusb_close);
	libusb_dl_set_call(libusb_control_transfer);
	libusb_dl_set_call(libusb_detach_kernel_driver);
	libusb_dl_set_call(libusb_exit);
	libusb_dl_set_call(libusb_free_config_descriptor);
	libusb_dl_set_call(libusb_free_device_list);
	libusb_dl_set_call(libusb_get_config_descriptor);
	libusb_dl_set_call(libusb_get_device_address);
	libusb_dl_set_call(libusb_get_device_descriptor);
	libusb_dl_set_call(libusb_get_device_list);
	libusb_dl_set_call(libusb_get_string_descriptor_ascii);
	libusb_dl_set_call(libusb_init);
	libusb_dl_set_call(libusb_interrupt_transfer);
	libusb_dl_set_call(libusb_kernel_driver_active);
	libusb_dl_set_call(libusb_ref_device);
	libusb_dl_set_call(libusb_release_interface);
	libusb_dl_set_call(libusb_reset_device);
	libusb_dl_set_call(libusb_set_configuration);
	libusb_dl_set_call(libusb_set_debug);
	libusb_dl_set_call(libusb_set_interface_alt_setting);
	libusb_dl_set_call(libusb_unref_device);
	return;
failure:
#ifdef HAVE_GNU_ERRNO_H
	fprintf(stderr, "%s: error while loading " LIBUSB_1_0_SONAME " from libusb-0.1.so.4: %s\n",
		program_invocation_name, dlerror());
#else
	fprintf(stderr, "libusb-compat: error while loading " LIBUSB_1_0_SONAME " from libusb-0.1.so.4: %s\n",
		dlerror());
#endif
	exit(127);
};

static inline void libusb_dl_exit(void) {
	dlclose(libusb_dl_handle);
};
