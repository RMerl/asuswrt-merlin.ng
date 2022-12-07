/*
 * Copyright (C) 2017 foo86
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

#include "dolby_e.h"
#include "get_bits.h"
#include "put_bits.h"

typedef struct DBEParseContext {
    DBEContext dectx;
} DBEParseContext;

static int dolby_e_parse(AVCodecParserContext *s2, AVCodecContext *avctx,
                        const uint8_t **poutbuf, int *poutbuf_size,
                        const uint8_t *buf, int buf_size)
{
    DBEParseContext *s1 = s2->priv_data;
    DBEContext *s = &s1->dectx;
    int ret;

    if ((ret = ff_dolby_e_parse_header(s, buf, buf_size)) < 0)
        goto end;

    s2->duration = FRAME_SAMPLES;
    switch (s->metadata.nb_channels) {
    case 4:
        avctx->channel_layout = AV_CH_LAYOUT_4POINT0;
        break;
    case 6:
        avctx->channel_layout = AV_CH_LAYOUT_5POINT1;
        break;
    case 8:
        avctx->channel_layout = AV_CH_LAYOUT_7POINT1;
        break;
    }

    avctx->channels    = s->metadata.nb_channels;
    avctx->sample_rate = s->metadata.sample_rate;
    avctx->sample_fmt  = AV_SAMPLE_FMT_FLTP;

end:
    /* always return the full packet. this parser isn't doing any splitting or
       combining, only packet analysis */
    *poutbuf      = buf;
    *poutbuf_size = buf_size;
    return buf_size;
}

AVCodecParser ff_dolby_e_parser = {
    .codec_ids      = { AV_CODEC_ID_DOLBY_E },
    .priv_data_size = sizeof(DBEParseContext),
    .parser_parse   = dolby_e_parse,
};
