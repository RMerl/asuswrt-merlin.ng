/*======================================================================
 FILE: sspm.h Mime Parser
 CREATOR: eric 25 June 2000

 (C) COPYRIGHT 2000, Eric Busboom <eric@softwarestudio.org>
     http://www.softwarestudio.org

 The contents of this file are subject to the Mozilla Public License
 Version 1.0 (the "License"); you may not use this file except in
 compliance with the License. You may obtain a copy of the License at
 http://www.mozilla.org/MPL/

 Software distributed under the License is distributed on an "AS IS"
 basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 the License for the specific language governing rights and
 limitations under the License.

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

 Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

  The Initial Developer of the Original Code is Eric Busboom
======================================================================*/

#ifndef ICAL_SSPM_H
#define ICAL_SSPM_H

#include "libical_ical_export.h"

enum sspm_major_type
{
    SSPM_NO_MAJOR_TYPE,
    SSPM_TEXT_MAJOR_TYPE,
    SSPM_IMAGE_MAJOR_TYPE,
    SSPM_AUDIO_MAJOR_TYPE,
    SSPM_VIDEO_MAJOR_TYPE,
    SSPM_APPLICATION_MAJOR_TYPE,
    SSPM_MULTIPART_MAJOR_TYPE,
    SSPM_MESSAGE_MAJOR_TYPE,
    SSPM_UNKNOWN_MAJOR_TYPE
};

enum sspm_minor_type
{
    SSPM_NO_MINOR_TYPE,
    SSPM_ANY_MINOR_TYPE,
    SSPM_PLAIN_MINOR_TYPE,
    SSPM_RFC822_MINOR_TYPE,
    SSPM_DIGEST_MINOR_TYPE,
    SSPM_CALENDAR_MINOR_TYPE,
    SSPM_MIXED_MINOR_TYPE,
    SSPM_RELATED_MINOR_TYPE,
    SSPM_ALTERNATIVE_MINOR_TYPE,
    SSPM_PARALLEL_MINOR_TYPE,
    SSPM_UNKNOWN_MINOR_TYPE
};

enum sspm_encoding
{
    SSPM_NO_ENCODING,
    SSPM_QUOTED_PRINTABLE_ENCODING,
    SSPM_8BIT_ENCODING,
    SSPM_7BIT_ENCODING,
    SSPM_BINARY_ENCODING,
    SSPM_BASE64_ENCODING,
    SSPM_UNKNOWN_ENCODING
};

enum sspm_error
{
    SSPM_NO_ERROR,
    SSPM_UNEXPECTED_BOUNDARY_ERROR,
    SSPM_WRONG_BOUNDARY_ERROR,
    SSPM_NO_BOUNDARY_ERROR,
    SSPM_NO_HEADER_ERROR,
    SSPM_MALFORMED_HEADER_ERROR
};

struct sspm_header
{
    int def;
    char *boundary;
    enum sspm_major_type major;
    enum sspm_minor_type minor;
    char *minor_text;
    char **content_type_params;
    char *charset;
    enum sspm_encoding encoding;
    char *filename;
    char *content_id;
    enum sspm_error error;
    char *error_text;
};

struct sspm_part
{
    struct sspm_header header;
    int level;
    size_t data_size;
    void *data;
};

struct sspm_action_map
{
    enum sspm_major_type major;
    enum sspm_minor_type minor;
    void *(*new_part) (void);
    void (*add_line) (void *part, struct sspm_header * header, const char *line, size_t size);
    void *(*end_part) (void *part);
    void (*free_part) (void *part);
};

LIBICAL_ICAL_EXPORT const char *sspm_major_type_string(enum sspm_major_type type);

LIBICAL_ICAL_EXPORT const char *sspm_minor_type_string(enum sspm_minor_type type);

LIBICAL_ICAL_EXPORT const char *sspm_encoding_string(enum sspm_encoding type);

LIBICAL_ICAL_EXPORT int sspm_parse_mime(struct sspm_part *parts,
                                        size_t max_parts,
                                        const struct sspm_action_map *actions,
                                        char *(*get_string) (char *s, size_t size, void *data),
                                        void *get_string_data, struct sspm_header *first_header);

LIBICAL_ICAL_EXPORT void sspm_free_parts(struct sspm_part *parts, size_t max_parts);

LIBICAL_ICAL_EXPORT char *decode_quoted_printable(char *dest, char *src, size_t *size);

LIBICAL_ICAL_EXPORT char *decode_base64(char *dest, char *src, size_t *size);

LIBICAL_ICAL_EXPORT int sspm_write_mime(struct sspm_part *parts, size_t num_parts,
                                        char **output_string, const char *header);

#endif /* ICAL_SSPM_H */
