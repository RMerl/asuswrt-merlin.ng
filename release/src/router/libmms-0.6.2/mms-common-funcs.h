/*
 * Copyright (C) 2002-2003 the xine project
 * Copyright (C) 2011 Hans de Goede <j.w.r.degoede@gmail.com>
 *
 * This file is part of libmms a free mms protocol library
 *
 * libmms is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * libmss is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#ifndef __MMS_COMMON_FUNCS_H
#define __MMS_COMMON_FUNCS_H

/*
 * returns 1 on error
 */
static int mms_tcp_connect(mms_io_t *io, TYPE *this)
{
  if (!this->connect_port) this->connect_port = DEFAULT_PORT;

  /* 
   * try to connect 
   */
  lprintf("trying to connect to %s on port %d\n", this->connect_host, this->connect_port);
  this->s = io_connect(io, this->connect_host, this->connect_port);
  if (this->s == -1) {
    lprintf("failed to connect to %s\n", this->connect_host);
    return 1;
  }

  lprintf("connected\n");
  return 0;
}

static int get_guid(unsigned char *buffer, int offset)
{
  int i;
  GUID g;
  
  g.Data1 = LE_32(buffer + offset);
  g.Data2 = LE_16(buffer + offset + 4);
  g.Data3 = LE_16(buffer + offset + 6);
  for(i = 0; i < 8; i++) {
    g.Data4[i] = buffer[offset + 8 + i];
  }
  
  for (i = 1; i < GUID_END; i++) {
    if (!memcmp(&g, &guids[i].guid, sizeof(GUID))) {
      lprintf("GUID: %s\n", guids[i].name);
      return i;
    }
  }

  lprintf("unknown GUID: 0x%x, 0x%x, 0x%x, "
          "{ 0x%hx, 0x%hx, 0x%hx, 0x%hx, 0x%hx, 0x%hx, 0x%hx, 0x%hx }\n",
          g.Data1, g.Data2, g.Data3,
          g.Data4[0], g.Data4[1], g.Data4[2], g.Data4[3], 
          g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);
  return GUID_ERROR;
}

static void interp_stream_properties(TYPE *this, int i)
{
  uint16_t flags;
  uint16_t stream_id;
  int type, encrypted, guid, stream_index;

  guid = get_guid(this->asf_header, i);
  switch (guid) {
    case GUID_ASF_AUDIO_MEDIA:
      type = ASF_STREAM_TYPE_AUDIO;
      this->has_audio = 1;
      break;

    case GUID_ASF_VIDEO_MEDIA:
    case GUID_ASF_JFIF_MEDIA:
    case GUID_ASF_DEGRADABLE_JPEG_MEDIA:
      type = ASF_STREAM_TYPE_VIDEO;
      this->has_video = 1;
      break;

    case GUID_ASF_COMMAND_MEDIA:
      type = ASF_STREAM_TYPE_CONTROL;
      break;

    default:
      type = ASF_STREAM_TYPE_UNKNOWN;
  }

  flags = LE_16(this->asf_header + i + 48);
  stream_id = flags & 0x7F;
  encrypted = flags >> 15;

  lprintf("stream object, stream id: %d, type: %d, encrypted: %d\n",
          stream_id, type, encrypted);

  for(stream_index = 0; stream_index < this->num_stream_ids; stream_index++) {
    if (this->streams[stream_index].stream_id == stream_id)
      break;
  }

  if (stream_index == this->num_stream_ids) {
    /* New stream, add it to our streams list */
    if (this->num_stream_ids >= ASF_MAX_NUM_STREAMS) {
      lprintf("too many streams, skipping\n");
      return;
    }
    this->streams[this->num_stream_ids].stream_id   = stream_id;
    this->streams[this->num_stream_ids].bitrate     = 0;
    this->streams[this->num_stream_ids].bitrate_pos = 0;
    this->num_stream_ids++;
  }
  this->streams[stream_index].stream_type = type;
}

static void interp_asf_header(TYPE *this)
{
  int i;

  this->asf_packet_len = 0;
  this->num_stream_ids = 0;
  this->asf_num_packets = 0;

  /*
   * parse asf header
   */

  i = 30;
  while ((i + 24) <= this->asf_header_len) {

    int guid;
    uint64_t length;

    guid = get_guid(this->asf_header, i);
    length = LE_64(this->asf_header + i + 16);

    if ((i + length) > this->asf_header_len) return;

    switch (guid) {

      case GUID_ASF_FILE_PROPERTIES:

        this->asf_packet_len = LE_32(this->asf_header + i + 92);
        if (this->asf_packet_len > BUF_SIZE) {
          lprintf("asf packet len too large: %d\n", this->asf_packet_len);
          this->asf_packet_len = 0;
          break;
        }
        this->file_len       = LE_64(this->asf_header + i + 40);
        this->file_time      = LE_64(this->asf_header + i + 48);
        this->time_len       = LE_64(this->asf_header + i + 64);
        //this->time_len       = LE_64(this->asf_header + i + 72);
        this->preroll        = LE_64(this->asf_header + i + 80);
        lprintf("file object, packet length = %d (%d)\n",
                this->asf_packet_len, LE_32(this->asf_header + i + 96));
        break;

      case GUID_ASF_STREAM_PROPERTIES:
        interp_stream_properties(this, i + 24);
        break;

      case GUID_ASF_STREAM_BITRATE_PROPERTIES:
        {
          uint16_t streams = LE_16(this->asf_header + i + 24);
          uint16_t stream_id;
          int j, stream_index, bitrate;

          for(j = 0; j < streams; j++) {
            stream_id = LE_16(this->asf_header + i + 24 + 2 + j * 6);
            bitrate   = LE_32(this->asf_header + i + 24 + 4 + j * 6);
            lprintf("stream id %d, bitrate %d\n", stream_id, bitrate);

            for(stream_index = 0; stream_index < this->num_stream_ids; stream_index++) {
              if (this->streams[stream_index].stream_id == stream_id)
                break;
            }
            if (stream_index == this->num_stream_ids) {
              /* New stream, add it to our streams list */
              if (this->num_stream_ids >= ASF_MAX_NUM_STREAMS) {
                lprintf("too many streams, skipping\n");
                continue;
              }
              this->streams[this->num_stream_ids].stream_id = stream_id;
              this->streams[this->num_stream_ids].stream_type =
                ASF_STREAM_TYPE_UNKNOWN;
              this->num_stream_ids++;
            }
            this->streams[stream_index].bitrate = bitrate;
            this->streams[stream_index].bitrate_pos = i + 24 + 4 + j * 6;
          }
        }
        break;

      case GUID_ASF_HEADER_EXTENSION:
        {
          if ((24 + 18 + 4) > length)
            break;

          int size = LE_32(this->asf_header + i + 24 + 18);
          int j = 24 + 18 + 4;
          int l;
          lprintf("Extension header data size: %d\n", size);

          while ((j + 24) <= length) {
            guid = get_guid(this->asf_header, i + j);
            l = LE_64(this->asf_header + i + j + 16);

            if ((j + l) > length)
              break;

            if (guid == GUID_ASF_EXTENDED_STREAM_PROPERTIES &&
                (24 + 64) <= l) {
                  int stream_no = LE_16(this->asf_header + i + j + 24 + 48);
                  int name_count = LE_16(this->asf_header + i + j + 24 + 60);
                  int ext_count = LE_16(this->asf_header + i + j + 24 + 62);
                  int ext_j = 24 + 64;
                  int x;

                  lprintf("l: %d\n", l);
                  lprintf("Stream No: %d\n", stream_no);
                  lprintf("ext_count: %d\n", ext_count);

                  // Loop through the number of stream names
                  for (x = 0; x < name_count && (ext_j + 4) <= l; x++) {
                    int lang_id_index;
                    int stream_name_len;

                    lang_id_index = LE_16(this->asf_header + i + j + ext_j);
                    ext_j += 2;

                    stream_name_len = LE_16(this->asf_header + i + j + ext_j);
                    ext_j += stream_name_len + 2;

                    lprintf("Language id index: %d\n", lang_id_index);
                    lprintf("Stream name Len: %d\n", stream_name_len);
                  }

                  // Loop through the number of extension system info
                  for (x = 0; x < ext_count && (ext_j + 22) <= l; x++) {
                    ext_j += 18;
                    int len = LE_16(this->asf_header + i + j + ext_j);
                    ext_j += 4 + len;
                  }

                  lprintf("ext_j: %d\n", ext_j);
                  // Finally, we arrive at the interesting point: The optional Stream Property Object
                  if ((ext_j + 24) <= l) {
                    guid = get_guid(this->asf_header, i + j + ext_j);
                    int len = LE_64(this->asf_header + i + j + ext_j + 16);
                    if (guid == GUID_ASF_STREAM_PROPERTIES &&
                        (ext_j + len) <= l) {
                      interp_stream_properties(this, i + j + ext_j + 24);
                    }
                  } else {
                    lprintf("Sorry, field not long enough\n");
                  }
            }
            j += l;
          }
        }
        break;

      case GUID_ASF_DATA:
        this->asf_num_packets = LE_64(this->asf_header + i + 40 - 24);
        lprintf("num_packets: %d\n", (int)this->asf_num_packets);
        break;
    }

    lprintf("length: %llu\n", (unsigned long long)length);
    i += length;
  }
}

static int mms_valid_proto(char *proto) {
  int i = 0;

  if (!proto)
    return 0;

  for (i = 0; mms_proto_s[i]; i++) {
    if (!strcasecmp(proto, mms_proto_s[i]))
      return 1;
  }

  return 0;
}

static void mms_get_best_stream_ids(TYPE *this, int *audio_stream_ret,
  int *video_stream_ret)
{
  int     i;
  int     audio_stream = -1;
  int     audio_rate   = 0;
  int     video_stream = -1;
  int     video_rate   = 0;
  int     min_bw_left  = 0;
  int     bandwidth_left;

  /* Step 1: choose the best quality for the audio stream */
  for (i = 0; i < this->num_stream_ids; i++) {
    switch (this->streams[i].stream_type) {
      case ASF_STREAM_TYPE_AUDIO:
        if (audio_stream == -1 || this->streams[i].bitrate > audio_rate) {
          audio_stream = this->streams[i].stream_id;
          audio_rate   = this->streams[i].bitrate;
        }
        break;
      default:
        break;
    }
  }

  /* Step 2: choose a video stream adapted to the user bandwidth */
  bandwidth_left = this->bandwidth - audio_rate;
  if (bandwidth_left < 0) {
    bandwidth_left = 0;
  }
  lprintf("Total bandwidth: %d, left for video: %d\n",
          this->bandwidth, bandwidth_left);

  min_bw_left = bandwidth_left;
  for (i = 0; i < this->num_stream_ids; i++) {
    switch (this->streams[i].stream_type) {
      case ASF_STREAM_TYPE_VIDEO:
        if (((bandwidth_left - this->streams[i].bitrate) < min_bw_left) &&
            (bandwidth_left >= this->streams[i].bitrate)) {
          video_stream = this->streams[i].stream_id;
          video_rate   = this->streams[i].bitrate;
          min_bw_left  = bandwidth_left - this->streams[i].bitrate;
        }
        break;
      default:
        break;
    }
  }

  /* Step 3: If we did not find a video stream fitting within the users
     bandwidth, choose the stream with the lowest bitrate */
  if ((video_stream == -1) && this->has_video) {
    for (i = 0; i < this->num_stream_ids; i++) {
      switch (this->streams[i].stream_type) {
        case ASF_STREAM_TYPE_VIDEO:
          if (video_stream == -1 || this->streams[i].bitrate < video_rate) {
            video_stream = this->streams[i].stream_id;
            video_rate   = this->streams[i].bitrate;
          }
          break;
        default:
          break;
      }
    }
  }

  /* Step 4, check if selecting the best audio stream did not push us over the
     available bandwidth. If it did, see if we can find one which does fit. */
  if ((audio_rate + video_rate) > this->bandwidth) {
    audio_stream = -1;

    /* Step 4a try to select the best audio stream fitting in the remaining
       bandwidth */
    bandwidth_left = this->bandwidth - video_rate;
    if (bandwidth_left < 0) {
      bandwidth_left = 0;
    }
    lprintf("Total bandwidth: %d, left for audio: %d\n",
            this->bandwidth, bandwidth_left);

    min_bw_left = bandwidth_left;
    for (i = 0; i < this->num_stream_ids; i++) {
      switch (this->streams[i].stream_type) {
        case ASF_STREAM_TYPE_AUDIO:
          if (((bandwidth_left - this->streams[i].bitrate) < min_bw_left) &&
              (bandwidth_left >= this->streams[i].bitrate)) {
            audio_stream = this->streams[i].stream_id;
            audio_rate   = this->streams[i].bitrate;
            min_bw_left  = bandwidth_left - this->streams[i].bitrate;
          }
          break;
        default:
          break;
      }
    }

    /* Step 4b: If we did not find an audio stream fitting within the users
       bandwidth, choose the stream with the lowest bitrate */
    if (audio_stream == -1) {
      for (i = 0; i < this->num_stream_ids; i++) {
        switch (this->streams[i].stream_type) {
          case ASF_STREAM_TYPE_AUDIO:
            if (audio_stream == -1 || this->streams[i].bitrate < audio_rate) {
              audio_stream = this->streams[i].stream_id;
              audio_rate   = this->streams[i].bitrate;
            }
            break;
          default:
            break;
        }
      }
    }
  }

  lprintf("selected streams: audio %d (%dbps), video %d (%dbps)\n",
          audio_stream, audio_rate, video_stream, video_rate);

  *audio_stream_ret = audio_stream;
  *video_stream_ret = video_stream;
}

static void mms_disable_disabled_streams_in_asf_header(TYPE *this,
  int audio_stream, int video_stream)
{
  int i;

  for (i = 0; i < this->num_stream_ids; i++) {
    if ((this->streams[i].stream_id != audio_stream) &&
        (this->streams[i].stream_id != video_stream)) {
      /* forces the asf demuxer to not choose this stream */
      if (this->streams[i].bitrate_pos) {
        if (this->streams[i].bitrate_pos + 3 < ASF_HEADER_SIZE) {
          this->asf_header[this->streams[i].bitrate_pos    ] = 0;
          this->asf_header[this->streams[i].bitrate_pos + 1] = 0;
          this->asf_header[this->streams[i].bitrate_pos + 2] = 0;
          this->asf_header[this->streams[i].bitrate_pos + 3] = 0;
        } else {
          lprintf("attempt to write beyond asf header limit\n");
        }
      }
    }
  }
}

#endif
