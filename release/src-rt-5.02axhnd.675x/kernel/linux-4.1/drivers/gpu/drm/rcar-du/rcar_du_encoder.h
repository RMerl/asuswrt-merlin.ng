/*
 * rcar_du_encoder.h  --  R-Car Display Unit Encoder
 *
 * Copyright (C) 2013-2014 Renesas Electronics Corporation
 *
 * Contact: Laurent Pinchart (laurent.pinchart@ideasonboard.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __RCAR_DU_ENCODER_H__
#define __RCAR_DU_ENCODER_H__

#include <drm/drm_crtc.h>
#include <drm/drm_encoder_slave.h>

struct rcar_du_device;
struct rcar_du_hdmienc;
struct rcar_du_lvdsenc;

enum rcar_du_encoder_type {
	RCAR_DU_ENCODER_UNUSED = 0,
	RCAR_DU_ENCODER_NONE,
	RCAR_DU_ENCODER_VGA,
	RCAR_DU_ENCODER_LVDS,
	RCAR_DU_ENCODER_HDMI,
};

struct rcar_du_encoder {
	struct drm_encoder_slave slave;
	enum rcar_du_output output;
	struct rcar_du_hdmienc *hdmi;
	struct rcar_du_lvdsenc *lvds;
};

#define to_rcar_encoder(e) \
	container_of(e, struct rcar_du_encoder, slave.base)

#define rcar_encoder_to_drm_encoder(e)	(&(e)->slave.base)

struct rcar_du_connector {
	struct drm_connector connector;
	struct rcar_du_encoder *encoder;
};

#define to_rcar_connector(c) \
	container_of(c, struct rcar_du_connector, connector)

struct drm_encoder *
rcar_du_connector_best_encoder(struct drm_connector *connector);

int rcar_du_encoder_init(struct rcar_du_device *rcdu,
			 enum rcar_du_encoder_type type,
			 enum rcar_du_output output,
			 struct device_node *enc_node,
			 struct device_node *con_node);

#endif /* __RCAR_DU_ENCODER_H__ */
