#include <linux/virtio_byteorder.h>
#include <linux/virtio.h>
#include <uapi/linux/virtio_config.h>

/*
 * __virtio_test_bit - helper to test feature bits. For use by transports.
 *                     Devices should normally use virtio_has_feature,
 *                     which includes more checks.
 * @vdev: the device
 * @fbit: the feature bit
 */
static inline bool __virtio_test_bit(const struct virtio_device *vdev,
				     unsigned int fbit)
{
	return vdev->features & (1ULL << fbit);
}

/**
 * __virtio_set_bit - helper to set feature bits. For use by transports.
 * @vdev: the device
 * @fbit: the feature bit
 */
static inline void __virtio_set_bit(struct virtio_device *vdev,
				    unsigned int fbit)
{
	vdev->features |= (1ULL << fbit);
}

/**
 * __virtio_clear_bit - helper to clear feature bits. For use by transports.
 * @vdev: the device
 * @fbit: the feature bit
 */
static inline void __virtio_clear_bit(struct virtio_device *vdev,
				      unsigned int fbit)
{
	vdev->features &= ~(1ULL << fbit);
}

#define virtio_has_feature(dev, feature) \
	(__virtio_test_bit((dev), feature))

static inline u16 virtio16_to_cpu(struct virtio_device *vdev, __virtio16 val)
{
	return __virtio16_to_cpu(virtio_has_feature(vdev, VIRTIO_F_VERSION_1), val);
}

static inline __virtio16 cpu_to_virtio16(struct virtio_device *vdev, u16 val)
{
	return __cpu_to_virtio16(virtio_has_feature(vdev, VIRTIO_F_VERSION_1), val);
}

static inline u32 virtio32_to_cpu(struct virtio_device *vdev, __virtio32 val)
{
	return __virtio32_to_cpu(virtio_has_feature(vdev, VIRTIO_F_VERSION_1), val);
}

static inline __virtio32 cpu_to_virtio32(struct virtio_device *vdev, u32 val)
{
	return __cpu_to_virtio32(virtio_has_feature(vdev, VIRTIO_F_VERSION_1), val);
}

static inline u64 virtio64_to_cpu(struct virtio_device *vdev, __virtio64 val)
{
	return __virtio64_to_cpu(virtio_has_feature(vdev, VIRTIO_F_VERSION_1), val);
}

static inline __virtio64 cpu_to_virtio64(struct virtio_device *vdev, u64 val)
{
	return __cpu_to_virtio64(virtio_has_feature(vdev, VIRTIO_F_VERSION_1), val);
}

