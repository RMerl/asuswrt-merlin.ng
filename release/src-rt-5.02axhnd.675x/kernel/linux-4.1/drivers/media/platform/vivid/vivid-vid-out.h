/*
 * vivid-vid-out.h - video output support functions.
 *
 * Copyright 2014 Cisco Systems, Inc. and/or its affiliates. All rights reserved.
 *
 * This program is free software; you may redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _VIVID_VID_OUT_H_
#define _VIVID_VID_OUT_H_

extern const struct vb2_ops vivid_vid_out_qops;

void vivid_update_format_out(struct vivid_dev *dev);

int vivid_g_fmt_vid_out(struct file *file, void *priv, struct v4l2_format *f);
int vivid_try_fmt_vid_out(struct file *file, void *priv, struct v4l2_format *f);
int vivid_s_fmt_vid_out(struct file *file, void *priv, struct v4l2_format *f);
int vidioc_g_fmt_vid_out_mplane(struct file *file, void *priv, struct v4l2_format *f);
int vidioc_try_fmt_vid_out_mplane(struct file *file, void *priv, struct v4l2_format *f);
int vidioc_s_fmt_vid_out_mplane(struct file *file, void *priv, struct v4l2_format *f);
int vidioc_g_fmt_vid_out(struct file *file, void *priv, struct v4l2_format *f);
int vidioc_try_fmt_vid_out(struct file *file, void *priv, struct v4l2_format *f);
int vidioc_s_fmt_vid_out(struct file *file, void *priv, struct v4l2_format *f);
int vivid_vid_out_g_selection(struct file *file, void *priv, struct v4l2_selection *sel);
int vivid_vid_out_s_selection(struct file *file, void *fh, struct v4l2_selection *s);
int vivid_vid_out_cropcap(struct file *file, void *fh, struct v4l2_cropcap *cap);
int vidioc_enum_fmt_vid_out_overlay(struct file *file, void  *priv, struct v4l2_fmtdesc *f);
int vidioc_g_fmt_vid_out_overlay(struct file *file, void *priv, struct v4l2_format *f);
int vidioc_try_fmt_vid_out_overlay(struct file *file, void *priv, struct v4l2_format *f);
int vidioc_s_fmt_vid_out_overlay(struct file *file, void *priv, struct v4l2_format *f);
int vivid_vid_out_overlay(struct file *file, void *fh, unsigned i);
int vivid_vid_out_g_fbuf(struct file *file, void *fh, struct v4l2_framebuffer *a);
int vivid_vid_out_s_fbuf(struct file *file, void *fh, const struct v4l2_framebuffer *a);
int vidioc_enum_output(struct file *file, void *priv, struct v4l2_output *out);
int vidioc_g_output(struct file *file, void *priv, unsigned *i);
int vidioc_s_output(struct file *file, void *priv, unsigned i);
int vidioc_enumaudout(struct file *file, void *fh, struct v4l2_audioout *vout);
int vidioc_g_audout(struct file *file, void *fh, struct v4l2_audioout *vout);
int vidioc_s_audout(struct file *file, void *fh, const struct v4l2_audioout *vout);
int vivid_vid_out_s_std(struct file *file, void *priv, v4l2_std_id id);
int vivid_vid_out_s_dv_timings(struct file *file, void *_fh, struct v4l2_dv_timings *timings);
int vivid_vid_out_g_parm(struct file *file, void *priv, struct v4l2_streamparm *parm);

#endif
