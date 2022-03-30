/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018, Tuomas Tynkkynen <tuomas.tynkkynen@iki.fi>
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 *
 * VirtIO is a virtualization standard for network and disk device drivers
 * where just the guest's device driver "knows" it is running in a virtual
 * environment, and cooperates with the hypervisor. This enables guests to
 * get high performance network and disk operations, and gives most of the
 * performance benefits of paravirtualization. In the U-Boot case, the guest
 * is U-Boot itself, while the virtual environment are normally QEMU targets
 * like ARM, RISC-V and x86.
 *
 * See http://docs.oasis-open.org/virtio/virtio/v1.0/virtio-v1.0.pdf for
 * the VirtIO specification v1.0.
 *
 * This file is largely based on Linux kernel virtio_*.h files
 */

#ifndef __VIRTIO_H__
#define __VIRTIO_H__

#define VIRTIO_ID_NET		1 /* virtio net */
#define VIRTIO_ID_BLOCK		2 /* virtio block */
#define VIRTIO_ID_MAX_NUM	3

#define VIRTIO_NET_DRV_NAME	"virtio-net"
#define VIRTIO_BLK_DRV_NAME	"virtio-blk"

/* Status byte for guest to report progress, and synchronize features */

/* We have seen device and processed generic fields (VIRTIO_CONFIG_F_VIRTIO) */
#define VIRTIO_CONFIG_S_ACKNOWLEDGE	1
/* We have found a driver for the device */
#define VIRTIO_CONFIG_S_DRIVER		2
/* Driver has used its parts of the config, and is happy */
#define VIRTIO_CONFIG_S_DRIVER_OK	4
/* Driver has finished configuring features */
#define VIRTIO_CONFIG_S_FEATURES_OK	8
/* Device entered invalid state, driver must reset it */
#define VIRTIO_CONFIG_S_NEEDS_RESET	0x40
/* We've given up on this device */
#define VIRTIO_CONFIG_S_FAILED		0x80

/*
 * Virtio feature bits VIRTIO_TRANSPORT_F_START through VIRTIO_TRANSPORT_F_END
 * are reserved for the transport being used (eg: virtio_ring, virtio_pci etc.),
 * the rest are per-device feature bits.
 */
#define VIRTIO_TRANSPORT_F_START	28
#define VIRTIO_TRANSPORT_F_END		38

#ifndef VIRTIO_CONFIG_NO_LEGACY
/*
 * Do we get callbacks when the ring is completely used,
 * even if we've suppressed them?
 */
#define VIRTIO_F_NOTIFY_ON_EMPTY	24

/* Can the device handle any descriptor layout? */
#define VIRTIO_F_ANY_LAYOUT		27
#endif /* VIRTIO_CONFIG_NO_LEGACY */

/* v1.0 compliant */
#define VIRTIO_F_VERSION_1		32

/*
 * If clear - device has the IOMMU bypass quirk feature.
 * If set - use platform tools to detect the IOMMU.
 *
 * Note the reverse polarity (compared to most other features),
 * this is for compatibility with legacy systems.
 */
#define VIRTIO_F_IOMMU_PLATFORM		33

/* Does the device support Single Root I/O Virtualization? */
#define VIRTIO_F_SR_IOV			37

/**
 * virtio scatter-gather struct
 *
 * @addr:		sg buffer address
 * @lengh:		sg buffer length
 */
struct virtio_sg {
	void *addr;
	size_t length;
};

struct virtqueue;

/* virtio bus operations */
struct dm_virtio_ops {
	/**
	 * get_config() - read the value of a configuration field
	 *
	 * @vdev:	the real virtio device
	 * @offset:	the offset of the configuration field
	 * @buf:	the buffer to write the field value into
	 * @len:	the length of the buffer
	 * @return 0 if OK, -ve on error
	 */
	int (*get_config)(struct udevice *vdev, unsigned int offset,
			  void *buf, unsigned int len);
	/**
	 * set_config() - write the value of a configuration field
	 *
	 * @vdev:	the real virtio device
	 * @offset:	the offset of the configuration field
	 * @buf:	the buffer to read the field value from
	 * @len:	the length of the buffer
	 * @return 0 if OK, -ve on error
	 */
	int (*set_config)(struct udevice *vdev, unsigned int offset,
			  const void *buf, unsigned int len);
	/**
	 * generation() - config generation counter
	 *
	 * @vdev:	the real virtio device
	 * @counter:	the returned config generation counter
	 * @return 0 if OK, -ve on error
	 */
	int (*generation)(struct udevice *vdev, u32 *counter);
	/**
	 * get_status() - read the status byte
	 *
	 * @vdev:	the real virtio device
	 * @status:	the returned status byte
	 * @return 0 if OK, -ve on error
	 */
	int (*get_status)(struct udevice *vdev, u8 *status);
	/**
	 * set_status() - write the status byte
	 *
	 * @vdev:	the real virtio device
	 * @status:	the new status byte
	 * @return 0 if OK, -ve on error
	 */
	int (*set_status)(struct udevice *vdev, u8 status);
	/**
	 * reset() - reset the device
	 *
	 * @vdev:	the real virtio device
	 * @return 0 if OK, -ve on error
	 */
	int (*reset)(struct udevice *vdev);
	/**
	 * get_features() - get the array of feature bits for this device
	 *
	 * @vdev:	the real virtio device
	 * @features:	the first 32 feature bits (all we currently need)
	 * @return 0 if OK, -ve on error
	 */
	int (*get_features)(struct udevice *vdev, u64 *features);
	/**
	 * set_features() - confirm what device features we'll be using
	 *
	 * @vdev:	the real virtio device
	 * @return 0 if OK, -ve on error
	 */
	int (*set_features)(struct udevice *vdev);
	/**
	 * find_vqs() - find virtqueues and instantiate them
	 *
	 * @vdev:	the real virtio device
	 * @nvqs:	the number of virtqueues to find
	 * @vqs:	on success, includes new virtqueues
	 * @return 0 if OK, -ve on error
	 */
	int (*find_vqs)(struct udevice *vdev, unsigned int nvqs,
			struct virtqueue *vqs[]);
	/**
	 * del_vqs() - free virtqueues found by find_vqs()
	 *
	 * @vdev:	the real virtio device
	 * @return 0 if OK, -ve on error
	 */
	int (*del_vqs)(struct udevice *vdev);
	/**
	 * notify() - notify the device to process the queue
	 *
	 * @vdev:	the real virtio device
	 * @vq:		virtqueue to process
	 * @return 0 if OK, -ve on error
	 */
	int (*notify)(struct udevice *vdev, struct virtqueue *vq);
};

/* Get access to a virtio bus' operations */
#define virtio_get_ops(dev)	((struct dm_virtio_ops *)(dev)->driver->ops)

/**
 * virtio uclass per device private data
 *
 * @vqs:			virtualqueue for the virtio device
 * @vdev:			the real virtio device underneath
 * @legacy:			is it a legacy device?
 * @device:			virtio device ID
 * @vendor:			virtio vendor ID
 * @features:			negotiated supported features
 * @feature_table:		an array of feature supported by the driver
 * @feature_table_size:		number of entries in the feature table array
 * @feature_table_legacy:	same as feature_table but working in legacy mode
 * @feature_table_size_legacy:	number of entries in feature table legacy array
 */
struct virtio_dev_priv {
	struct list_head vqs;
	struct udevice *vdev;
	bool legacy;
	u32 device;
	u32 vendor;
	u64 features;
	const u32 *feature_table;
	u32 feature_table_size;
	const u32 *feature_table_legacy;
	u32 feature_table_size_legacy;
};

/**
 * virtio_get_config() - read the value of a configuration field
 *
 * @vdev:	the real virtio device
 * @offset:	the offset of the configuration field
 * @buf:	the buffer to write the field value into
 * @len:	the length of the buffer
 * @return 0 if OK, -ve on error
 */
int virtio_get_config(struct udevice *vdev, unsigned int offset,
		      void *buf, unsigned int len);

/**
 * virtio_set_config() - write the value of a configuration field
 *
 * @vdev:	the real virtio device
 * @offset:	the offset of the configuration field
 * @buf:	the buffer to read the field value from
 * @len:	the length of the buffer
 * @return 0 if OK, -ve on error
 */
int virtio_set_config(struct udevice *vdev, unsigned int offset,
		      void *buf, unsigned int len);

/**
 * virtio_generation() - config generation counter
 *
 * @vdev:	the real virtio device
 * @counter:	the returned config generation counter
 * @return 0 if OK, -ve on error
 */
int virtio_generation(struct udevice *vdev, u32 *counter);

/**
 * virtio_get_status() - read the status byte
 *
 * @vdev:	the real virtio device
 * @status:	the returned status byte
 * @return 0 if OK, -ve on error
 */
int virtio_get_status(struct udevice *vdev, u8 *status);

/**
 * virtio_set_status() - write the status byte
 *
 * @vdev:	the real virtio device
 * @status:	the new status byte
 * @return 0 if OK, -ve on error
 */
int virtio_set_status(struct udevice *vdev, u8 status);

/**
 * virtio_reset() - reset the device
 *
 * @vdev:	the real virtio device
 * @return 0 if OK, -ve on error
 */
int virtio_reset(struct udevice *vdev);

/**
 * virtio_get_features() - get the array of feature bits for this device
 *
 * @vdev:	the real virtio device
 * @features:	the first 32 feature bits (all we currently need)
 * @return 0 if OK, -ve on error
 */
int virtio_get_features(struct udevice *vdev, u64 *features);

/**
 * virtio_set_features() - confirm what device features we'll be using
 *
 * @vdev:	the real virtio device
 * @return 0 if OK, -ve on error
 */
int virtio_set_features(struct udevice *vdev);

/**
 * virtio_find_vqs() - find virtqueues and instantiate them
 *
 * @vdev:	the real virtio device
 * @nvqs:	the number of virtqueues to find
 * @vqs:	on success, includes new virtqueues
 * @return 0 if OK, -ve on error
 */
int virtio_find_vqs(struct udevice *vdev, unsigned int nvqs,
		    struct virtqueue *vqs[]);

/**
 * virtio_del_vqs() - free virtqueues found by find_vqs()
 *
 * @vdev:	the real virtio device
 * @return 0 if OK, -ve on error
 */
int virtio_del_vqs(struct udevice *vdev);

/**
 * virtio_notify() - notify the device to process the queue
 *
 * @vdev:	the real virtio device
 * @vq:		virtqueue to process
 * @return 0 if OK, -ve on error
 */
int virtio_notify(struct udevice *vdev, struct virtqueue *vq);

/**
 * virtio_add_status() - helper to set a new status code to the device
 *
 * @vdev:	the real virtio device
 * @status:	new status code to be added
 */
void virtio_add_status(struct udevice *vdev, u8 status);

/**
 * virtio_finalize_features() - helper to finalize features
 *
 * @vdev:	the real virtio device
 * @return 0 if OK, -ve on error
 */
int virtio_finalize_features(struct udevice *vdev);

/**
 * virtio_driver_features_init() - initialize driver supported features
 *
 * This fills in the virtio device parent per child private data with the given
 * information, which contains driver supported features and legacy features.
 *
 * This API should be called in the virtio device driver's bind method, so that
 * later virtio transport uclass driver can utilize the driver supplied features
 * to negotiate with the device on the final supported features.
 *
 * @priv:		virtio uclass per device private data
 * @feature:		an array of feature supported by the driver
 * @feature_size:	number of entries in the feature table array
 * @feature_legacy:	same as feature_table but working in legacy mode
 * @feature_legacy_size:number of entries in feature table legacy array
 */
void virtio_driver_features_init(struct virtio_dev_priv *priv,
				 const u32 *feature,
				 u32 feature_size,
				 const u32 *feature_legacy,
				 u32 feature_legacy_size);

/**
 * virtio_init() - helper to enumerate all known virtio devices
 *
 * @return 0 if OK, -ve on error
 */
int virtio_init(void);

static inline u16 __virtio16_to_cpu(bool little_endian, __virtio16 val)
{
	if (little_endian)
		return le16_to_cpu((__force __le16)val);
	else
		return be16_to_cpu((__force __be16)val);
}

static inline __virtio16 __cpu_to_virtio16(bool little_endian, u16 val)
{
	if (little_endian)
		return (__force __virtio16)cpu_to_le16(val);
	else
		return (__force __virtio16)cpu_to_be16(val);
}

static inline u32 __virtio32_to_cpu(bool little_endian, __virtio32 val)
{
	if (little_endian)
		return le32_to_cpu((__force __le32)val);
	else
		return be32_to_cpu((__force __be32)val);
}

static inline __virtio32 __cpu_to_virtio32(bool little_endian, u32 val)
{
	if (little_endian)
		return (__force __virtio32)cpu_to_le32(val);
	else
		return (__force __virtio32)cpu_to_be32(val);
}

static inline u64 __virtio64_to_cpu(bool little_endian, __virtio64 val)
{
	if (little_endian)
		return le64_to_cpu((__force __le64)val);
	else
		return be64_to_cpu((__force __be64)val);
}

static inline __virtio64 __cpu_to_virtio64(bool little_endian, u64 val)
{
	if (little_endian)
		return (__force __virtio64)cpu_to_le64(val);
	else
		return (__force __virtio64)cpu_to_be64(val);
}

/**
 * __virtio_test_bit - helper to test feature bits
 *
 * For use by transports. Devices should normally use virtio_has_feature,
 * which includes more checks.
 *
 * @udev: the transport device
 * @fbit: the feature bit
 */
static inline bool __virtio_test_bit(struct udevice *udev, unsigned int fbit)
{
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(udev);

	/* Did you forget to fix assumptions on max features? */
	if (__builtin_constant_p(fbit))
		BUILD_BUG_ON(fbit >= 64);
	else
		WARN_ON(fbit >= 64);

	return uc_priv->features & BIT_ULL(fbit);
}

/**
 * __virtio_set_bit - helper to set feature bits
 *
 * For use by transports.
 *
 * @udev: the transport device
 * @fbit: the feature bit
 */
static inline void __virtio_set_bit(struct udevice *udev, unsigned int fbit)
{
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(udev);

	/* Did you forget to fix assumptions on max features? */
	if (__builtin_constant_p(fbit))
		BUILD_BUG_ON(fbit >= 64);
	else
		WARN_ON(fbit >= 64);

	uc_priv->features |= BIT_ULL(fbit);
}

/**
 * __virtio_clear_bit - helper to clear feature bits
 *
 * For use by transports.
 *
 * @vdev: the transport device
 * @fbit: the feature bit
 */
static inline void __virtio_clear_bit(struct udevice *udev, unsigned int fbit)
{
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(udev);

	/* Did you forget to fix assumptions on max features? */
	if (__builtin_constant_p(fbit))
		BUILD_BUG_ON(fbit >= 64);
	else
		WARN_ON(fbit >= 64);

	uc_priv->features &= ~BIT_ULL(fbit);
}

/**
 * virtio_has_feature - helper to determine if this device has this feature
 *
 * Note this API is only usable after the virtio device driver's bind phase,
 * as the feature has been negotiated between the device and the driver.
 *
 * @vdev: the virtio device
 * @fbit: the feature bit
 */
static inline bool virtio_has_feature(struct udevice *vdev, unsigned int fbit)
{
	if (!(vdev->flags & DM_FLAG_BOUND))
		WARN_ON(true);

	return __virtio_test_bit(vdev->parent, fbit);
}

static inline bool virtio_legacy_is_little_endian(void)
{
#ifdef __LITTLE_ENDIAN
	return true;
#else
	return false;
#endif
}

static inline bool virtio_is_little_endian(struct udevice *vdev)
{
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(vdev->parent);

	return !uc_priv->legacy || virtio_legacy_is_little_endian();
}

/* Memory accessors */
static inline u16 virtio16_to_cpu(struct udevice *vdev, __virtio16 val)
{
	return __virtio16_to_cpu(virtio_is_little_endian(vdev), val);
}

static inline __virtio16 cpu_to_virtio16(struct udevice *vdev, u16 val)
{
	return __cpu_to_virtio16(virtio_is_little_endian(vdev), val);
}

static inline u32 virtio32_to_cpu(struct udevice *vdev, __virtio32 val)
{
	return __virtio32_to_cpu(virtio_is_little_endian(vdev), val);
}

static inline __virtio32 cpu_to_virtio32(struct udevice *vdev, u32 val)
{
	return __cpu_to_virtio32(virtio_is_little_endian(vdev), val);
}

static inline u64 virtio64_to_cpu(struct udevice *vdev, __virtio64 val)
{
	return __virtio64_to_cpu(virtio_is_little_endian(vdev), val);
}

static inline __virtio64 cpu_to_virtio64(struct udevice *vdev, u64 val)
{
	return __cpu_to_virtio64(virtio_is_little_endian(vdev), val);
}

/* Read @count fields, @bytes each */
static inline void __virtio_cread_many(struct udevice *vdev,
				       unsigned int offset,
				       void *buf, size_t count, size_t bytes)
{
	u32 old, gen;
	int i;

	/* no need to check return value as generation can be optional */
	virtio_generation(vdev, &gen);
	do {
		old = gen;

		for (i = 0; i < count; i++)
			virtio_get_config(vdev, offset + bytes * i,
					  buf + i * bytes, bytes);

		virtio_generation(vdev, &gen);
	} while (gen != old);
}

static inline void virtio_cread_bytes(struct udevice *vdev,
				      unsigned int offset,
				      void *buf, size_t len)
{
	__virtio_cread_many(vdev, offset, buf, len, 1);
}

static inline u8 virtio_cread8(struct udevice *vdev, unsigned int offset)
{
	u8 ret;

	virtio_get_config(vdev, offset, &ret, sizeof(ret));
	return ret;
}

static inline void virtio_cwrite8(struct udevice *vdev,
				  unsigned int offset, u8 val)
{
	virtio_set_config(vdev, offset, &val, sizeof(val));
}

static inline u16 virtio_cread16(struct udevice *vdev,
				 unsigned int offset)
{
	u16 ret;

	virtio_get_config(vdev, offset, &ret, sizeof(ret));
	return virtio16_to_cpu(vdev, (__force __virtio16)ret);
}

static inline void virtio_cwrite16(struct udevice *vdev,
				   unsigned int offset, u16 val)
{
	val = (__force u16)cpu_to_virtio16(vdev, val);
	virtio_set_config(vdev, offset, &val, sizeof(val));
}

static inline u32 virtio_cread32(struct udevice *vdev,
				 unsigned int offset)
{
	u32 ret;

	virtio_get_config(vdev, offset, &ret, sizeof(ret));
	return virtio32_to_cpu(vdev, (__force __virtio32)ret);
}

static inline void virtio_cwrite32(struct udevice *vdev,
				   unsigned int offset, u32 val)
{
	val = (__force u32)cpu_to_virtio32(vdev, val);
	virtio_set_config(vdev, offset, &val, sizeof(val));
}

static inline u64 virtio_cread64(struct udevice *vdev,
				 unsigned int offset)
{
	u64 ret;

	__virtio_cread_many(vdev, offset, &ret, 1, sizeof(ret));
	return virtio64_to_cpu(vdev, (__force __virtio64)ret);
}

static inline void virtio_cwrite64(struct udevice *vdev,
				   unsigned int offset, u64 val)
{
	val = (__force u64)cpu_to_virtio64(vdev, val);
	virtio_set_config(vdev, offset, &val, sizeof(val));
}

/* Config space read accessor */
#define virtio_cread(vdev, structname, member, ptr)			\
	do {								\
		/* Must match the member's type, and be integer */	\
		if (!typecheck(typeof((((structname *)0)->member)), *(ptr))) \
			(*ptr) = 1;					\
									\
		switch (sizeof(*ptr)) {					\
		case 1:							\
			*(ptr) = virtio_cread8(vdev,			\
					       offsetof(structname, member)); \
			break;						\
		case 2:							\
			*(ptr) = virtio_cread16(vdev,			\
						offsetof(structname, member)); \
			break;						\
		case 4:							\
			*(ptr) = virtio_cread32(vdev,			\
						offsetof(structname, member)); \
			break;						\
		case 8:							\
			*(ptr) = virtio_cread64(vdev,			\
						offsetof(structname, member)); \
			break;						\
		default:						\
			WARN_ON(true);					\
		}							\
	} while (0)

/* Config space write accessor */
#define virtio_cwrite(vdev, structname, member, ptr)			\
	do {								\
		/* Must match the member's type, and be integer */	\
		if (!typecheck(typeof((((structname *)0)->member)), *(ptr))) \
			WARN_ON((*ptr) == 1);				\
									\
		switch (sizeof(*ptr)) {					\
		case 1:							\
			virtio_cwrite8(vdev,				\
				       offsetof(structname, member),	\
				       *(ptr));				\
			break;						\
		case 2:							\
			virtio_cwrite16(vdev,				\
					offsetof(structname, member),	\
					*(ptr));			\
			break;						\
		case 4:							\
			virtio_cwrite32(vdev,				\
					offsetof(structname, member),	\
					*(ptr));			\
			break;						\
		case 8:							\
			virtio_cwrite64(vdev,				\
					offsetof(structname, member),	\
					*(ptr));			\
			break;						\
		default:						\
			WARN_ON(true);					\
		}							\
	} while (0)

/* Conditional config space accessors */
#define virtio_cread_feature(vdev, fbit, structname, member, ptr)	\
	({								\
		int _r = 0;						\
		if (!virtio_has_feature(vdev, fbit))			\
			_r = -ENOENT;					\
		else							\
			virtio_cread(vdev, structname, member, ptr);	\
		_r;							\
	})

#endif /* __VIRTIO_H__ */
