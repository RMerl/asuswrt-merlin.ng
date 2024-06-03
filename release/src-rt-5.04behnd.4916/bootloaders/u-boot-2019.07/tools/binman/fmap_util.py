# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Support for flashrom's FMAP format. This supports a header followed by a
# number of 'areas', describing regions of a firmware storage device,
# generally SPI flash.

import collections
import struct

# constants imported from lib/fmap.h
FMAP_SIGNATURE = '__FMAP__'
FMAP_VER_MAJOR = 1
FMAP_VER_MINOR = 0
FMAP_STRLEN = 32

FMAP_AREA_STATIC = 1 << 0
FMAP_AREA_COMPRESSED = 1 << 1
FMAP_AREA_RO = 1 << 2

FMAP_HEADER_LEN = 56
FMAP_AREA_LEN = 42

FMAP_HEADER_FORMAT = '<8sBBQI%dsH'% (FMAP_STRLEN)
FMAP_AREA_FORMAT = '<II%dsH' % (FMAP_STRLEN)

FMAP_HEADER_NAMES = (
    'signature',
    'ver_major',
    'ver_minor',
    'base',
    'image_size',
    'name',
    'nareas',
)

FMAP_AREA_NAMES = (
    'offset',
    'size',
    'name',
    'flags',
)

# These are the two data structures supported by flashrom, a header (which
# appears once at the start) and an area (which is repeated until the end of
# the list of areas)
FmapHeader = collections.namedtuple('FmapHeader', FMAP_HEADER_NAMES)
FmapArea = collections.namedtuple('FmapArea', FMAP_AREA_NAMES)


def NameToFmap(name):
    return name.replace('\0', '').replace('-', '_').upper()

def ConvertName(field_names, fields):
    """Convert a name to something flashrom likes

    Flashrom requires upper case, underscores instead of hyphens. We remove any
    null characters as well. This updates the 'name' value in fields.

    Args:
        field_names: List of field names for this struct
        fields: Dict:
            key: Field name
            value: value of that field (string for the ones we support)
    """
    name_index = field_names.index('name')
    fields[name_index] = NameToFmap(fields[name_index])

def DecodeFmap(data):
    """Decode a flashmap into a header and list of areas

    Args:
        data: Data block containing the FMAP

    Returns:
        Tuple:
            header: FmapHeader object
            List of FmapArea objects
    """
    fields = list(struct.unpack(FMAP_HEADER_FORMAT, data[:FMAP_HEADER_LEN]))
    ConvertName(FMAP_HEADER_NAMES, fields)
    header = FmapHeader(*fields)
    areas = []
    data = data[FMAP_HEADER_LEN:]
    for area in range(header.nareas):
        fields = list(struct.unpack(FMAP_AREA_FORMAT, data[:FMAP_AREA_LEN]))
        ConvertName(FMAP_AREA_NAMES, fields)
        areas.append(FmapArea(*fields))
        data = data[FMAP_AREA_LEN:]
    return header, areas

def EncodeFmap(image_size, name, areas):
    """Create a new FMAP from a list of areas

    Args:
        image_size: Size of image, to put in the header
        name: Name of image, to put in the header
        areas: List of FmapArea objects

    Returns:
        String containing the FMAP created
    """
    def _FormatBlob(fmt, names, obj):
        params = [getattr(obj, name) for name in names]
        ConvertName(names, params)
        return struct.pack(fmt, *params)

    values = FmapHeader(FMAP_SIGNATURE, 1, 0, 0, image_size, name, len(areas))
    blob = _FormatBlob(FMAP_HEADER_FORMAT, FMAP_HEADER_NAMES, values)
    for area in areas:
        blob += _FormatBlob(FMAP_AREA_FORMAT, FMAP_AREA_NAMES, area)
    return blob
