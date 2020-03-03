#ifndef __NVKM_I2C_H__
#define __NVKM_I2C_H__
#include <core/subdev.h>
#include <core/event.h>

#include <subdev/bios.h>
#include <subdev/bios/i2c.h>

#define NV_I2C_PORT(n)    (0x00 + (n))
#define NV_I2C_AUX(n)     (0x10 + (n))
#define NV_I2C_EXT(n)     (0x20 + (n))
#define NV_I2C_DEFAULT(n) (0x80 + (n))

#define NV_I2C_TYPE_DCBI2C(n) (0x0000 | (n))
#define NV_I2C_TYPE_EXTDDC(e) (0x0005 | (e) << 8)
#define NV_I2C_TYPE_EXTAUX(e) (0x0006 | (e) << 8)

struct nvkm_i2c_ntfy_req {
#define NVKM_I2C_PLUG                                                      0x01
#define NVKM_I2C_UNPLUG                                                    0x02
#define NVKM_I2C_IRQ                                                       0x04
#define NVKM_I2C_DONE                                                      0x08
#define NVKM_I2C_ANY                                                       0x0f
	u8 mask;
	u8 port;
};

struct nvkm_i2c_ntfy_rep {
	u8 mask;
};

struct nvkm_i2c_port {
	struct nvkm_object base;
	struct i2c_adapter adapter;
	struct mutex mutex;

	struct list_head head;
	u8  index;
	int aux;

	const struct nvkm_i2c_func *func;
};

struct nvkm_i2c_func {
	void (*drive_scl)(struct nvkm_i2c_port *, int);
	void (*drive_sda)(struct nvkm_i2c_port *, int);
	int  (*sense_scl)(struct nvkm_i2c_port *);
	int  (*sense_sda)(struct nvkm_i2c_port *);

	int  (*aux)(struct nvkm_i2c_port *, bool, u8, u32, u8 *, u8);
	int  (*pattern)(struct nvkm_i2c_port *, int pattern);
	int  (*lnk_ctl)(struct nvkm_i2c_port *, int nr, int bw, bool enh);
	int  (*drv_ctl)(struct nvkm_i2c_port *, int lane, int sw, int pe);
};

struct nvkm_i2c_board_info {
	struct i2c_board_info dev;
	u8 udelay; /* set to 0 to use the standard delay */
};

struct nvkm_i2c {
	struct nvkm_subdev base;
	struct nvkm_event event;

	struct nvkm_i2c_port *(*find)(struct nvkm_i2c *, u8 index);
	struct nvkm_i2c_port *(*find_type)(struct nvkm_i2c *, u16 type);
	int  (*acquire_pad)(struct nvkm_i2c_port *, unsigned long timeout);
	void (*release_pad)(struct nvkm_i2c_port *);
	int  (*acquire)(struct nvkm_i2c_port *, unsigned long timeout);
	void (*release)(struct nvkm_i2c_port *);
	int  (*identify)(struct nvkm_i2c *, int index,
			 const char *what, struct nvkm_i2c_board_info *,
			 bool (*match)(struct nvkm_i2c_port *,
				       struct i2c_board_info *, void *),
			 void *);

	wait_queue_head_t wait;
	struct list_head ports;
};

static inline struct nvkm_i2c *
nvkm_i2c(void *obj)
{
	return (void *)nvkm_subdev(obj, NVDEV_SUBDEV_I2C);
}

extern struct nvkm_oclass *nv04_i2c_oclass;
extern struct nvkm_oclass *nv4e_i2c_oclass;
extern struct nvkm_oclass *nv50_i2c_oclass;
extern struct nvkm_oclass *g94_i2c_oclass;
extern struct nvkm_oclass *gf110_i2c_oclass;
extern struct nvkm_oclass *gf117_i2c_oclass;
extern struct nvkm_oclass *gk104_i2c_oclass;
extern struct nvkm_oclass *gm204_i2c_oclass;

static inline int
nv_rdi2cr(struct nvkm_i2c_port *port, u8 addr, u8 reg)
{
	u8 val;
	struct i2c_msg msgs[] = {
		{ .addr = addr, .flags = 0, .len = 1, .buf = &reg },
		{ .addr = addr, .flags = I2C_M_RD, .len = 1, .buf = &val },
	};

	int ret = i2c_transfer(&port->adapter, msgs, 2);
	if (ret != 2)
		return -EIO;

	return val;
}

static inline int
nv_wri2cr(struct nvkm_i2c_port *port, u8 addr, u8 reg, u8 val)
{
	u8 buf[2] = { reg, val };
	struct i2c_msg msgs[] = {
		{ .addr = addr, .flags = 0, .len = 2, .buf = buf },
	};

	int ret = i2c_transfer(&port->adapter, msgs, 1);
	if (ret != 1)
		return -EIO;

	return 0;
}

static inline bool
nv_probe_i2c(struct nvkm_i2c_port *port, u8 addr)
{
	return nv_rdi2cr(port, addr, 0) >= 0;
}

int nv_rdaux(struct nvkm_i2c_port *, u32 addr, u8 *data, u8 size);
int nv_wraux(struct nvkm_i2c_port *, u32 addr, u8 *data, u8 size);
#endif
