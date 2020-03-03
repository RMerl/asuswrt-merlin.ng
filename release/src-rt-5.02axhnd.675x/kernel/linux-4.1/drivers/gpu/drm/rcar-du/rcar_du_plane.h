/*
 * rcar_du_plane.h  --  R-Car Display Unit Planes
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

#ifndef __RCAR_DU_PLANE_H__
#define __RCAR_DU_PLANE_H__

#include <drm/drmP.h>
#include <drm/drm_crtc.h>

struct rcar_du_format_info;
struct rcar_du_group;

/* The RCAR DU has 8 hardware planes, shared between primary and overlay planes.
 * As using overlay planes requires at least one of the CRTCs being enabled, no
 * more than 7 overlay planes can be available. We thus create 1 primary plane
 * per CRTC and 7 overlay planes, for a total of up to 9 KMS planes.
 */
#define RCAR_DU_NUM_KMS_PLANES		9
#define RCAR_DU_NUM_HW_PLANES		8

struct rcar_du_plane {
	struct drm_plane plane;
	struct rcar_du_group *group;
};

static inline struct rcar_du_plane *to_rcar_plane(struct drm_plane *plane)
{
	return container_of(plane, struct rcar_du_plane, plane);
}

struct rcar_du_planes {
	struct rcar_du_plane planes[RCAR_DU_NUM_KMS_PLANES];

	struct drm_property *alpha;
	struct drm_property *colorkey;
	struct drm_property *zpos;
};

struct rcar_du_plane_state {
	struct drm_plane_state state;

	const struct rcar_du_format_info *format;
	int hwindex;		/* 0-based, -1 means unused */

	unsigned int alpha;
	unsigned int colorkey;
	unsigned int zpos;
};

static inline struct rcar_du_plane_state *
to_rcar_du_plane_state(struct drm_plane_state *state)
{
	return container_of(state, struct rcar_du_plane_state, state);
}

int rcar_du_planes_init(struct rcar_du_group *rgrp);

void rcar_du_plane_setup(struct rcar_du_plane *plane);

#endif /* __RCAR_DU_PLANE_H__ */
