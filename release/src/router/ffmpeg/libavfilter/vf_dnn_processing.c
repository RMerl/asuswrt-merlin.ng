/*
 * Copyright (c) 2019 Guo Yejun
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * implementing a generic image processing filter using deep learning networks.
 */

#include "libavformat/avio.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "libavutil/avassert.h"
#include "libavutil/imgutils.h"
#include "filters.h"
#include "dnn_filter_common.h"
#include "formats.h"
#include "internal.h"
#include "libswscale/swscale.h"
#include "libavutil/time.h"

typedef struct DnnProcessingContext {
    const AVClass *class;
    DnnContext dnnctx;
    struct SwsContext *sws_uv_scale;
    int sws_uv_height;
} DnnProcessingContext;

#define OFFSET(x) offsetof(DnnProcessingContext, dnnctx.x)
#define FLAGS AV_OPT_FLAG_FILTERING_PARAM | AV_OPT_FLAG_VIDEO_PARAM
static const AVOption dnn_processing_options[] = {
    { "dnn_backend", "DNN backend",                OFFSET(backend_type),     AV_OPT_TYPE_INT,       { .i64 = 0 },    INT_MIN, INT_MAX, FLAGS, "backend" },
    { "native",      "native backend flag",        0,                        AV_OPT_TYPE_CONST,     { .i64 = 0 },    0, 0, FLAGS, "backend" },
#if (CONFIG_LIBTENSORFLOW == 1)
    { "tensorflow",  "tensorflow backend flag",    0,                        AV_OPT_TYPE_CONST,     { .i64 = 1 },    0, 0, FLAGS, "backend" },
#endif
#if (CONFIG_LIBOPENVINO == 1)
    { "openvino",    "openvino backend flag",      0,                        AV_OPT_TYPE_CONST,     { .i64 = 2 },    0, 0, FLAGS, "backend" },
#endif
    DNN_COMMON_OPTIONS
    { NULL }
};

AVFILTER_DEFINE_CLASS(dnn_processing);

static av_cold int init(AVFilterContext *context)
{
    DnnProcessingContext *ctx = context->priv;
    return ff_dnn_init(&ctx->dnnctx, DFT_PROCESS_FRAME, context);
}

static int query_formats(AVFilterContext *context)
{
    static const enum AVPixelFormat pix_fmts[] = {
        AV_PIX_FMT_RGB24, AV_PIX_FMT_BGR24,
        AV_PIX_FMT_GRAY8, AV_PIX_FMT_GRAYF32,
        AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUV422P,
        AV_PIX_FMT_YUV444P, AV_PIX_FMT_YUV410P, AV_PIX_FMT_YUV411P,
        AV_PIX_FMT_NV12,
        AV_PIX_FMT_NONE
    };
    AVFilterFormats *fmts_list = ff_make_format_list(pix_fmts);
    return ff_set_common_formats(context, fmts_list);
}

#define LOG_FORMAT_CHANNEL_MISMATCH()                       \
    av_log(ctx, AV_LOG_ERROR,                               \
           "the frame's format %s does not match "          \
           "the model input channel %d\n",                  \
           av_get_pix_fmt_name(fmt),                        \
           model_input->channels);

static int check_modelinput_inlink(const DNNData *model_input, const AVFilterLink *inlink)
{
    AVFilterContext *ctx   = inlink->dst;
    enum AVPixelFormat fmt = inlink->format;

    // the design is to add explicit scale filter before this filter
    if (model_input->height != -1 && model_input->height != inlink->h) {
        av_log(ctx, AV_LOG_ERROR, "the model requires frame height %d but got %d\n",
                                   model_input->height, inlink->h);
        return AVERROR(EIO);
    }
    if (model_input->width != -1 && model_input->width != inlink->w) {
        av_log(ctx, AV_LOG_ERROR, "the model requires frame width %d but got %d\n",
                                   model_input->width, inlink->w);
        return AVERROR(EIO);
    }
    if (model_input->dt != DNN_FLOAT) {
        avpriv_report_missing_feature(ctx, "data type rather than DNN_FLOAT");
        return AVERROR(EIO);
    }

    switch (fmt) {
    case AV_PIX_FMT_RGB24:
    case AV_PIX_FMT_BGR24:
        if (model_input->channels != 3) {
            LOG_FORMAT_CHANNEL_MISMATCH();
            return AVERROR(EIO);
        }
        return 0;
    case AV_PIX_FMT_GRAYF32:
    case AV_PIX_FMT_YUV420P:
    case AV_PIX_FMT_YUV422P:
    case AV_PIX_FMT_YUV444P:
    case AV_PIX_FMT_YUV410P:
    case AV_PIX_FMT_YUV411P:
    case AV_PIX_FMT_NV12:
        if (model_input->channels != 1) {
            LOG_FORMAT_CHANNEL_MISMATCH();
            return AVERROR(EIO);
        }
        return 0;
    default:
        avpriv_report_missing_feature(ctx, "%s", av_get_pix_fmt_name(fmt));
        return AVERROR(EIO);
    }

    return 0;
}

static int config_input(AVFilterLink *inlink)
{
    AVFilterContext *context     = inlink->dst;
    DnnProcessingContext *ctx = context->priv;
    DNNReturnType result;
    DNNData model_input;
    int check;

    result = ff_dnn_get_input(&ctx->dnnctx, &model_input);
    if (result != DNN_SUCCESS) {
        av_log(ctx, AV_LOG_ERROR, "could not get input from the model\n");
        return AVERROR(EIO);
    }

    check = check_modelinput_inlink(&model_input, inlink);
    if (check != 0) {
        return check;
    }

    return 0;
}

static av_always_inline int isPlanarYUV(enum AVPixelFormat pix_fmt)
{
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(pix_fmt);
    av_assert0(desc);
    return !(desc->flags & AV_PIX_FMT_FLAG_RGB) && desc->nb_components == 3;
}

static int prepare_uv_scale(AVFilterLink *outlink)
{
    AVFilterContext *context = outlink->src;
    DnnProcessingContext *ctx = context->priv;
    AVFilterLink *inlink = context->inputs[0];
    enum AVPixelFormat fmt = inlink->format;

    if (isPlanarYUV(fmt)) {
        if (inlink->w != outlink->w || inlink->h != outlink->h) {
            if (fmt == AV_PIX_FMT_NV12) {
                ctx->sws_uv_scale = sws_getContext(inlink->w >> 1, inlink->h >> 1, AV_PIX_FMT_YA8,
                                                   outlink->w >> 1, outlink->h >> 1, AV_PIX_FMT_YA8,
                                                   SWS_BICUBIC, NULL, NULL, NULL);
                ctx->sws_uv_height = inlink->h >> 1;
            } else {
                const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(fmt);
                int sws_src_h = AV_CEIL_RSHIFT(inlink->h, desc->log2_chroma_h);
                int sws_src_w = AV_CEIL_RSHIFT(inlink->w, desc->log2_chroma_w);
                int sws_dst_h = AV_CEIL_RSHIFT(outlink->h, desc->log2_chroma_h);
                int sws_dst_w = AV_CEIL_RSHIFT(outlink->w, desc->log2_chroma_w);
                ctx->sws_uv_scale = sws_getContext(sws_src_w, sws_src_h, AV_PIX_FMT_GRAY8,
                                                   sws_dst_w, sws_dst_h, AV_PIX_FMT_GRAY8,
                                                   SWS_BICUBIC, NULL, NULL, NULL);
                ctx->sws_uv_height = sws_src_h;
            }
        }
    }

    return 0;
}

static int config_output(AVFilterLink *outlink)
{
    AVFilterContext *context = outlink->src;
    DnnProcessingContext *ctx = context->priv;
    DNNReturnType result;
    AVFilterLink *inlink = context->inputs[0];

    // have a try run in case that the dnn model resize the frame
    result = ff_dnn_get_output(&ctx->dnnctx, inlink->w, inlink->h, &outlink->w, &outlink->h);
    if (result != DNN_SUCCESS) {
        av_log(ctx, AV_LOG_ERROR, "could not get output from the model\n");
        return AVERROR(EIO);
    }

    prepare_uv_scale(outlink);

    return 0;
}

static int copy_uv_planes(DnnProcessingContext *ctx, AVFrame *out, const AVFrame *in)
{
    const AVPixFmtDescriptor *desc;
    int uv_height;

    if (!ctx->sws_uv_scale) {
        av_assert0(in->height == out->height && in->width == out->width);
        desc = av_pix_fmt_desc_get(in->format);
        uv_height = AV_CEIL_RSHIFT(in->height, desc->log2_chroma_h);
        for (int i = 1; i < 3; ++i) {
            int bytewidth = av_image_get_linesize(in->format, in->width, i);
            av_image_copy_plane(out->data[i], out->linesize[i],
                                in->data[i], in->linesize[i],
                                bytewidth, uv_height);
        }
    } else if (in->format == AV_PIX_FMT_NV12) {
        sws_scale(ctx->sws_uv_scale, (const uint8_t **)(in->data + 1), in->linesize + 1,
                  0, ctx->sws_uv_height, out->data + 1, out->linesize + 1);
    } else {
        sws_scale(ctx->sws_uv_scale, (const uint8_t **)(in->data + 1), in->linesize + 1,
                  0, ctx->sws_uv_height, out->data + 1, out->linesize + 1);
        sws_scale(ctx->sws_uv_scale, (const uint8_t **)(in->data + 2), in->linesize + 2,
                  0, ctx->sws_uv_height, out->data + 2, out->linesize + 2);
    }

    return 0;
}

static int filter_frame(AVFilterLink *inlink, AVFrame *in)
{
    AVFilterContext *context  = inlink->dst;
    AVFilterLink *outlink = context->outputs[0];
    DnnProcessingContext *ctx = context->priv;
    DNNReturnType dnn_result;
    AVFrame *out;

    out = ff_get_video_buffer(outlink, outlink->w, outlink->h);
    if (!out) {
        av_frame_free(&in);
        return AVERROR(ENOMEM);
    }
    av_frame_copy_props(out, in);

    dnn_result = ff_dnn_execute_model(&ctx->dnnctx, in, out);
    if (dnn_result != DNN_SUCCESS){
        av_log(ctx, AV_LOG_ERROR, "failed to execute model\n");
        av_frame_free(&in);
        av_frame_free(&out);
        return AVERROR(EIO);
    }

    if (isPlanarYUV(in->format))
        copy_uv_planes(ctx, out, in);

    av_frame_free(&in);
    return ff_filter_frame(outlink, out);
}

static int activate_sync(AVFilterContext *filter_ctx)
{
    AVFilterLink *inlink = filter_ctx->inputs[0];
    AVFilterLink *outlink = filter_ctx->outputs[0];
    AVFrame *in = NULL;
    int64_t pts;
    int ret, status;
    int got_frame = 0;

    FF_FILTER_FORWARD_STATUS_BACK(outlink, inlink);

    do {
        // drain all input frames
        ret = ff_inlink_consume_frame(inlink, &in);
        if (ret < 0)
            return ret;
        if (ret > 0) {
            ret = filter_frame(inlink, in);
            if (ret < 0)
                return ret;
            got_frame = 1;
        }
    } while (ret > 0);

    // if frame got, schedule to next filter
    if (got_frame)
        return 0;

    if (ff_inlink_acknowledge_status(inlink, &status, &pts)) {
        if (status == AVERROR_EOF) {
            ff_outlink_set_status(outlink, status, pts);
            return ret;
        }
    }

    FF_FILTER_FORWARD_WANTED(outlink, inlink);

    return FFERROR_NOT_READY;
}

static int flush_frame(AVFilterLink *outlink, int64_t pts, int64_t *out_pts)
{
    DnnProcessingContext *ctx = outlink->src->priv;
    int ret;
    DNNAsyncStatusType async_state;

    ret = ff_dnn_flush(&ctx->dnnctx);
    if (ret != DNN_SUCCESS) {
        return -1;
    }

    do {
        AVFrame *in_frame = NULL;
        AVFrame *out_frame = NULL;
        async_state = ff_dnn_get_async_result(&ctx->dnnctx, &in_frame, &out_frame);
        if (out_frame) {
            if (isPlanarYUV(in_frame->format))
                copy_uv_planes(ctx, out_frame, in_frame);
            av_frame_free(&in_frame);
            ret = ff_filter_frame(outlink, out_frame);
            if (ret < 0)
                return ret;
            if (out_pts)
                *out_pts = out_frame->pts + pts;
        }
        av_usleep(5000);
    } while (async_state >= DAST_NOT_READY);

    return 0;
}

static int activate_async(AVFilterContext *filter_ctx)
{
    AVFilterLink *inlink = filter_ctx->inputs[0];
    AVFilterLink *outlink = filter_ctx->outputs[0];
    DnnProcessingContext *ctx = filter_ctx->priv;
    AVFrame *in = NULL, *out = NULL;
    int64_t pts;
    int ret, status;
    int got_frame = 0;
    int async_state;

    FF_FILTER_FORWARD_STATUS_BACK(outlink, inlink);

    do {
        // drain all input frames
        ret = ff_inlink_consume_frame(inlink, &in);
        if (ret < 0)
            return ret;
        if (ret > 0) {
            out = ff_get_video_buffer(outlink, outlink->w, outlink->h);
            if (!out) {
                av_frame_free(&in);
                return AVERROR(ENOMEM);
            }
            av_frame_copy_props(out, in);
            if (ff_dnn_execute_model_async(&ctx->dnnctx, in, out) != DNN_SUCCESS) {
                return AVERROR(EIO);
            }
        }
    } while (ret > 0);

    // drain all processed frames
    do {
        AVFrame *in_frame = NULL;
        AVFrame *out_frame = NULL;
        async_state = ff_dnn_get_async_result(&ctx->dnnctx, &in_frame, &out_frame);
        if (out_frame) {
            if (isPlanarYUV(in_frame->format))
                copy_uv_planes(ctx, out_frame, in_frame);
            av_frame_free(&in_frame);
            ret = ff_filter_frame(outlink, out_frame);
            if (ret < 0)
                return ret;
            got_frame = 1;
        }
    } while (async_state == DAST_SUCCESS);

    // if frame got, schedule to next filter
    if (got_frame)
        return 0;

    if (ff_inlink_acknowledge_status(inlink, &status, &pts)) {
        if (status == AVERROR_EOF) {
            int64_t out_pts = pts;
            ret = flush_frame(outlink, pts, &out_pts);
            ff_outlink_set_status(outlink, status, out_pts);
            return ret;
        }
    }

    FF_FILTER_FORWARD_WANTED(outlink, inlink);

    return 0;
}

static int activate(AVFilterContext *filter_ctx)
{
    DnnProcessingContext *ctx = filter_ctx->priv;

    if (ctx->dnnctx.async)
        return activate_async(filter_ctx);
    else
        return activate_sync(filter_ctx);
}

static av_cold void uninit(AVFilterContext *ctx)
{
    DnnProcessingContext *context = ctx->priv;

    sws_freeContext(context->sws_uv_scale);
    ff_dnn_uninit(&context->dnnctx);
}

static const AVFilterPad dnn_processing_inputs[] = {
    {
        .name         = "default",
        .type         = AVMEDIA_TYPE_VIDEO,
        .config_props = config_input,
    },
    { NULL }
};

static const AVFilterPad dnn_processing_outputs[] = {
    {
        .name = "default",
        .type = AVMEDIA_TYPE_VIDEO,
        .config_props  = config_output,
    },
    { NULL }
};

AVFilter ff_vf_dnn_processing = {
    .name          = "dnn_processing",
    .description   = NULL_IF_CONFIG_SMALL("Apply DNN processing filter to the input."),
    .priv_size     = sizeof(DnnProcessingContext),
    .init          = init,
    .uninit        = uninit,
    .query_formats = query_formats,
    .inputs        = dnn_processing_inputs,
    .outputs       = dnn_processing_outputs,
    .priv_class    = &dnn_processing_class,
    .activate      = activate,
};
