/*
 *
 *	V 4 L 2   D R I V E R   H E L P E R   A P I
 *
 * Moved from videodev2.h
 *
 *	Some commonly needed functions for drivers (v4l2-common.o module)
 */
#ifndef _V4L2_IOCTL_H
#define _V4L2_IOCTL_H

#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/compiler.h> /* need __user */
#include <linux/videodev2.h>

struct v4l2_fh;

struct v4l2_ioctl_ops {
	/* ioctl callbacks */

	/* VIDIOC_QUERYCAP handler */
	int (*vidioc_querycap)(struct file *file, void *fh, struct v4l2_capability *cap);

	/* VIDIOC_ENUM_FMT handlers */
	int (*vidioc_enum_fmt_vid_cap)     (struct file *file, void *fh,
					    struct v4l2_fmtdesc *f);
	int (*vidioc_enum_fmt_vid_overlay) (struct file *file, void *fh,
					    struct v4l2_fmtdesc *f);
	int (*vidioc_enum_fmt_vid_out)     (struct file *file, void *fh,
					    struct v4l2_fmtdesc *f);
	int (*vidioc_enum_fmt_vid_cap_mplane)(struct file *file, void *fh,
					      struct v4l2_fmtdesc *f);
	int (*vidioc_enum_fmt_vid_out_mplane)(struct file *file, void *fh,
					      struct v4l2_fmtdesc *f);
	int (*vidioc_enum_fmt_sdr_cap)     (struct file *file, void *fh,
					    struct v4l2_fmtdesc *f);

	/* VIDIOC_G_FMT handlers */
	int (*vidioc_g_fmt_vid_cap)    (struct file *file, void *fh,
					struct v4l2_format *f);
	int (*vidioc_g_fmt_vid_overlay)(struct file *file, void *fh,
					struct v4l2_format *f);
	int (*vidioc_g_fmt_vid_out)    (struct file *file, void *fh,
					struct v4l2_format *f);
	int (*vidioc_g_fmt_vid_out_overlay)(struct file *file, void *fh,
					struct v4l2_format *f);
	int (*vidioc_g_fmt_vbi_cap)    (struct file *file, void *fh,
					struct v4l2_format *f);
	int (*vidioc_g_fmt_vbi_out)    (struct file *file, void *fh,
					struct v4l2_format *f);
	int (*vidioc_g_fmt_sliced_vbi_cap)(struct file *file, void *fh,
					struct v4l2_format *f);
	int (*vidioc_g_fmt_sliced_vbi_out)(struct file *file, void *fh,
					struct v4l2_format *f);
	int (*vidioc_g_fmt_vid_cap_mplane)(struct file *file, void *fh,
					   struct v4l2_format *f);
	int (*vidioc_g_fmt_vid_out_mplane)(struct file *file, void *fh,
					   struct v4l2_format *f);
	int (*vidioc_g_fmt_sdr_cap)    (struct file *file, void *fh,
					struct v4l2_format *f);

	/* VIDIOC_S_FMT handlers */
	int (*vidioc_s_fmt_vid_cap)    (struct file *file, void *fh,
					struct v4l2_format *f);
	int (*vidioc_s_fmt_vid_overlay)(struct file *file, void *fh,
					struct v4l2_format *f);
	int (*vidioc_s_fmt_vid_out)    (struct file *file, void *fh,
					struct v4l2_format *f);
	int (*vidioc_s_fmt_vid_out_overlay)(struct file *file, void *fh,
					struct v4l2_format *f);
	int (*vidioc_s_fmt_vbi_cap)    (struct file *file, void *fh,
					struct v4l2_format *f);
	int (*vidioc_s_fmt_vbi_out)    (struct file *file, void *fh,
					struct v4l2_format *f);
	int (*vidioc_s_fmt_sliced_vbi_cap)(struct file *file, void *fh,
					struct v4l2_format *f);
	int (*vidioc_s_fmt_sliced_vbi_out)(struct file *file, void *fh,
					struct v4l2_format *f);
	int (*vidioc_s_fmt_vid_cap_mplane)(struct file *file, void *fh,
					   struct v4l2_format *f);
	int (*vidioc_s_fmt_vid_out_mplane)(struct file *file, void *fh,
					   struct v4l2_format *f);
	int (*vidioc_s_fmt_sdr_cap)    (struct file *file, void *fh,
					struct v4l2_format *f);

	/* VIDIOC_TRY_FMT handlers */
	int (*vidioc_try_fmt_vid_cap)    (struct file *file, void *fh,
					  struct v4l2_format *f);
	int (*vidioc_try_fmt_vid_overlay)(struct file *file, void *fh,
					  struct v4l2_format *f);
	int (*vidioc_try_fmt_vid_out)    (struct file *file, void *fh,
					  struct v4l2_format *f);
	int (*vidioc_try_fmt_vid_out_overlay)(struct file *file, void *fh,
					  struct v4l2_format *f);
	int (*vidioc_try_fmt_vbi_cap)    (struct file *file, void *fh,
					  struct v4l2_format *f);
	int (*vidioc_try_fmt_vbi_out)    (struct file *file, void *fh,
					  struct v4l2_format *f);
	int (*vidioc_try_fmt_sliced_vbi_cap)(struct file *file, void *fh,
					  struct v4l2_format *f);
	int (*vidioc_try_fmt_sliced_vbi_out)(struct file *file, void *fh,
					  struct v4l2_format *f);
	int (*vidioc_try_fmt_vid_cap_mplane)(struct file *file, void *fh,
					     struct v4l2_format *f);
	int (*vidioc_try_fmt_vid_out_mplane)(struct file *file, void *fh,
					     struct v4l2_format *f);
	int (*vidioc_try_fmt_sdr_cap)    (struct file *file, void *fh,
					  struct v4l2_format *f);

	/* Buffer handlers */
	int (*vidioc_reqbufs) (struct file *file, void *fh, struct v4l2_requestbuffers *b);
	int (*vidioc_querybuf)(struct file *file, void *fh, struct v4l2_buffer *b);
	int (*vidioc_qbuf)    (struct file *file, void *fh, struct v4l2_buffer *b);
	int (*vidioc_expbuf)  (struct file *file, void *fh,
				struct v4l2_exportbuffer *e);
	int (*vidioc_dqbuf)   (struct file *file, void *fh, struct v4l2_buffer *b);

	int (*vidioc_create_bufs)(struct file *file, void *fh, struct v4l2_create_buffers *b);
	int (*vidioc_prepare_buf)(struct file *file, void *fh, struct v4l2_buffer *b);

	int (*vidioc_overlay) (struct file *file, void *fh, unsigned int i);
	int (*vidioc_g_fbuf)   (struct file *file, void *fh,
				struct v4l2_framebuffer *a);
	int (*vidioc_s_fbuf)   (struct file *file, void *fh,
				const struct v4l2_framebuffer *a);

		/* Stream on/off */
	int (*vidioc_streamon) (struct file *file, void *fh, enum v4l2_buf_type i);
	int (*vidioc_streamoff)(struct file *file, void *fh, enum v4l2_buf_type i);

		/* Standard handling
			ENUMSTD is handled by videodev.c
		 */
	int (*vidioc_g_std) (struct file *file, void *fh, v4l2_std_id *norm);
	int (*vidioc_s_std) (struct file *file, void *fh, v4l2_std_id norm);
	int (*vidioc_querystd) (struct file *file, void *fh, v4l2_std_id *a);

		/* Input handling */
	int (*vidioc_enum_input)(struct file *file, void *fh,
				 struct v4l2_input *inp);
	int (*vidioc_g_input)   (struct file *file, void *fh, unsigned int *i);
	int (*vidioc_s_input)   (struct file *file, void *fh, unsigned int i);

		/* Output handling */
	int (*vidioc_enum_output) (struct file *file, void *fh,
				  struct v4l2_output *a);
	int (*vidioc_g_output)   (struct file *file, void *fh, unsigned int *i);
	int (*vidioc_s_output)   (struct file *file, void *fh, unsigned int i);

		/* Control handling */
	int (*vidioc_queryctrl)        (struct file *file, void *fh,
					struct v4l2_queryctrl *a);
	int (*vidioc_query_ext_ctrl)   (struct file *file, void *fh,
					struct v4l2_query_ext_ctrl *a);
	int (*vidioc_g_ctrl)           (struct file *file, void *fh,
					struct v4l2_control *a);
	int (*vidioc_s_ctrl)           (struct file *file, void *fh,
					struct v4l2_control *a);
	int (*vidioc_g_ext_ctrls)      (struct file *file, void *fh,
					struct v4l2_ext_controls *a);
	int (*vidioc_s_ext_ctrls)      (struct file *file, void *fh,
					struct v4l2_ext_controls *a);
	int (*vidioc_try_ext_ctrls)    (struct file *file, void *fh,
					struct v4l2_ext_controls *a);
	int (*vidioc_querymenu)        (struct file *file, void *fh,
					struct v4l2_querymenu *a);

	/* Audio ioctls */
	int (*vidioc_enumaudio)        (struct file *file, void *fh,
					struct v4l2_audio *a);
	int (*vidioc_g_audio)          (struct file *file, void *fh,
					struct v4l2_audio *a);
	int (*vidioc_s_audio)          (struct file *file, void *fh,
					const struct v4l2_audio *a);

	/* Audio out ioctls */
	int (*vidioc_enumaudout)       (struct file *file, void *fh,
					struct v4l2_audioout *a);
	int (*vidioc_g_audout)         (struct file *file, void *fh,
					struct v4l2_audioout *a);
	int (*vidioc_s_audout)         (struct file *file, void *fh,
					const struct v4l2_audioout *a);
	int (*vidioc_g_modulator)      (struct file *file, void *fh,
					struct v4l2_modulator *a);
	int (*vidioc_s_modulator)      (struct file *file, void *fh,
					const struct v4l2_modulator *a);
	/* Crop ioctls */
	int (*vidioc_cropcap)          (struct file *file, void *fh,
					struct v4l2_cropcap *a);
	int (*vidioc_g_crop)           (struct file *file, void *fh,
					struct v4l2_crop *a);
	int (*vidioc_s_crop)           (struct file *file, void *fh,
					const struct v4l2_crop *a);
	int (*vidioc_g_selection)      (struct file *file, void *fh,
					struct v4l2_selection *s);
	int (*vidioc_s_selection)      (struct file *file, void *fh,
					struct v4l2_selection *s);
	/* Compression ioctls */
	int (*vidioc_g_jpegcomp)       (struct file *file, void *fh,
					struct v4l2_jpegcompression *a);
	int (*vidioc_s_jpegcomp)       (struct file *file, void *fh,
					const struct v4l2_jpegcompression *a);
	int (*vidioc_g_enc_index)      (struct file *file, void *fh,
					struct v4l2_enc_idx *a);
	int (*vidioc_encoder_cmd)      (struct file *file, void *fh,
					struct v4l2_encoder_cmd *a);
	int (*vidioc_try_encoder_cmd)  (struct file *file, void *fh,
					struct v4l2_encoder_cmd *a);
	int (*vidioc_decoder_cmd)      (struct file *file, void *fh,
					struct v4l2_decoder_cmd *a);
	int (*vidioc_try_decoder_cmd)  (struct file *file, void *fh,
					struct v4l2_decoder_cmd *a);

	/* Stream type-dependent parameter ioctls */
	int (*vidioc_g_parm)           (struct file *file, void *fh,
					struct v4l2_streamparm *a);
	int (*vidioc_s_parm)           (struct file *file, void *fh,
					struct v4l2_streamparm *a);

	/* Tuner ioctls */
	int (*vidioc_g_tuner)          (struct file *file, void *fh,
					struct v4l2_tuner *a);
	int (*vidioc_s_tuner)          (struct file *file, void *fh,
					const struct v4l2_tuner *a);
	int (*vidioc_g_frequency)      (struct file *file, void *fh,
					struct v4l2_frequency *a);
	int (*vidioc_s_frequency)      (struct file *file, void *fh,
					const struct v4l2_frequency *a);
	int (*vidioc_enum_freq_bands) (struct file *file, void *fh,
				    struct v4l2_frequency_band *band);

	/* Sliced VBI cap */
	int (*vidioc_g_sliced_vbi_cap) (struct file *file, void *fh,
					struct v4l2_sliced_vbi_cap *a);

	/* Log status ioctl */
	int (*vidioc_log_status)       (struct file *file, void *fh);

	int (*vidioc_s_hw_freq_seek)   (struct file *file, void *fh,
					const struct v4l2_hw_freq_seek *a);

	/* Debugging ioctls */
#ifdef CONFIG_VIDEO_ADV_DEBUG
	int (*vidioc_g_register)       (struct file *file, void *fh,
					struct v4l2_dbg_register *reg);
	int (*vidioc_s_register)       (struct file *file, void *fh,
					const struct v4l2_dbg_register *reg);

	int (*vidioc_g_chip_info)      (struct file *file, void *fh,
					struct v4l2_dbg_chip_info *chip);
#endif

	int (*vidioc_enum_framesizes)   (struct file *file, void *fh,
					 struct v4l2_frmsizeenum *fsize);

	int (*vidioc_enum_frameintervals) (struct file *file, void *fh,
					   struct v4l2_frmivalenum *fival);

	/* DV Timings IOCTLs */
	int (*vidioc_s_dv_timings) (struct file *file, void *fh,
				    struct v4l2_dv_timings *timings);
	int (*vidioc_g_dv_timings) (struct file *file, void *fh,
				    struct v4l2_dv_timings *timings);
	int (*vidioc_query_dv_timings) (struct file *file, void *fh,
				    struct v4l2_dv_timings *timings);
	int (*vidioc_enum_dv_timings) (struct file *file, void *fh,
				    struct v4l2_enum_dv_timings *timings);
	int (*vidioc_dv_timings_cap) (struct file *file, void *fh,
				    struct v4l2_dv_timings_cap *cap);
	int (*vidioc_g_edid) (struct file *file, void *fh, struct v4l2_edid *edid);
	int (*vidioc_s_edid) (struct file *file, void *fh, struct v4l2_edid *edid);

	int (*vidioc_subscribe_event)  (struct v4l2_fh *fh,
					const struct v4l2_event_subscription *sub);
	int (*vidioc_unsubscribe_event)(struct v4l2_fh *fh,
					const struct v4l2_event_subscription *sub);

	/* For other private ioctls */
	long (*vidioc_default)	       (struct file *file, void *fh,
					bool valid_prio, unsigned int cmd, void *arg);
};


/* v4l debugging and diagnostics */

/* Device debug flags to be used with the video device debug attribute */

/* Just log the ioctl name + error code */
#define V4L2_DEV_DEBUG_IOCTL		0x01
/* Log the ioctl name arguments + error code */
#define V4L2_DEV_DEBUG_IOCTL_ARG	0x02
/* Log the file operations open, release, mmap and get_unmapped_area */
#define V4L2_DEV_DEBUG_FOP		0x04
/* Log the read and write file operations and the VIDIOC_(D)QBUF ioctls */
#define V4L2_DEV_DEBUG_STREAMING	0x08
/* Log poll() */
#define V4L2_DEV_DEBUG_POLL		0x10

/*  Video standard functions  */
extern const char *v4l2_norm_to_name(v4l2_std_id id);
extern void v4l2_video_std_frame_period(int id, struct v4l2_fract *frameperiod);
extern int v4l2_video_std_construct(struct v4l2_standard *vs,
				    int id, const char *name);
/* Prints the ioctl in a human-readable format. If prefix != NULL,
   then do printk(KERN_DEBUG "%s: ", prefix) first. */
extern void v4l_printk_ioctl(const char *prefix, unsigned int cmd);

/* Internal use only: get the mutex (if any) that we need to lock for the
   given command. */
struct video_device;
extern struct mutex *v4l2_ioctl_get_lock(struct video_device *vdev, unsigned cmd);

/* names for fancy debug output */
extern const char *v4l2_field_names[];
extern const char *v4l2_type_names[];

#ifdef CONFIG_COMPAT
/* 32 Bits compatibility layer for 64 bits processors */
extern long v4l2_compat_ioctl32(struct file *file, unsigned int cmd,
				unsigned long arg);
#endif

typedef long (*v4l2_kioctl)(struct file *file,
		unsigned int cmd, void *arg);

/* Include support for obsoleted stuff */
extern long video_usercopy(struct file *file, unsigned int cmd,
				unsigned long arg, v4l2_kioctl func);

/* Standard handlers for V4L ioctl's */
extern long video_ioctl2(struct file *file,
			unsigned int cmd, unsigned long arg);

#endif /* _V4L2_IOCTL_H */
