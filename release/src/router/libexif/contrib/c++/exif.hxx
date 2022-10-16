/* exif.hxx
 *
 * Copyright  2002,2003 Hans Meine <hans_meine@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA.
 */

#ifndef EXIF_HXX
#define EXIF_HXX

#include <libexif/exif-entry.h>
#include <libexif/exif-content.h>
#include <libexif/exif-ifd.h>
#include <libexif/exif-data.h>
#include <libexif/exif-format.h>
#include <libexif/exif-utils.h>
#include <stdexcept>
#include <string>

namespace Exif {

#ifndef EXIF_NO_EXCEPTIONS
struct InvalidIndex : std::runtime_error
{
    InvalidIndex(const std::string& s)
    : std::runtime_error(s) {}
};

struct InvalidFormat : std::runtime_error
{
    InvalidFormat(const std::string& s)
    : std::runtime_error(s) {}
};

struct IOError : std::runtime_error
{
    IOError(const std::string& s)
    : std::runtime_error(s) {}
};
#endif // EXIF_NO_EXCEPTIONS

struct Entry
{
    ExifEntry *entry_;

        // construct an empty entry, FIXME: is this needed in the public API?
    Entry()
    : entry_(exif_entry_new())
    {}

        // construct an entry for the given tag
    Entry(ExifTag tag)
    : entry_(exif_entry_new())
    {
        exif_entry_initialize(entry_, tag);
    }

        // copy constructor
    Entry(Entry const &other)
    : entry_(other.entry_)
    {
        exif_entry_ref(entry_);
    }

        // internal, do not use directly
    Entry(ExifEntry *entry)
    : entry_(entry)
    {
        exif_entry_ref(entry_);
    }

    ~Entry()
    {
        exif_entry_unref(entry_);
    }

    Entry &operator=(Entry const &other)
    {
        exif_entry_unref(entry_);
        entry_ = other.entry_;
        exif_entry_ref(entry_);
        return *this;
    }

    ExifTag tag() const
    {
        return entry_->tag;
    }

/*
    void setTag(ExifTag tag)
    {
        entry_->tag = tag;
    }
*/

    ExifFormat format() const
    {
        return entry_->format;
    }

/*
    void setFormat(ExifFormat format)
    {
        entry_->format = format;
    }
*/

    unsigned long components() const
    {
        return entry_->components;
    }

/*
    void setComponents(unsigned long components)
    {
        entry_->components = components;
    }

    void initialize(ExifTag tag)
    {
        exif_entry_initialize(entry_, tag);
    }
*/

    ExifByte getByte(unsigned int index) const
    {
#ifndef EXIF_NO_EXCEPTIONS
        if(entry_->format != EXIF_FORMAT_BYTE)
            throw InvalidFormat(
                "Exif::Entry::getByte(): Format is not EXIF_FORMAT_BYTE");
        if(index >= components())
            throw InvalidIndex(
                "Exif::getByte: component index out of range");
#endif
        return *(entry_->data
                 + index * exif_format_get_size(entry_->format));
    }

    const ExifAscii getAscii() const
    {
#ifndef EXIF_NO_EXCEPTIONS
        if(entry_->format != EXIF_FORMAT_ASCII)
            throw InvalidFormat(
                "Exif::Entry::getAscii(): Format is not EXIF_FORMAT_ASCII");
#endif
        return (ExifAscii)entry_->data;
    }

    ExifShort getShort(unsigned int index) const
    {
#ifndef EXIF_NO_EXCEPTIONS
        if(entry_->format != EXIF_FORMAT_SHORT)
            throw InvalidFormat(
                "Exif::Entry::getShort(): Format is not EXIF_FORMAT_SHORT");
        if(index >= components())
            throw InvalidIndex(
                "Exif::getShort: component index out of range");
#endif
        return exif_get_short(entry_->data
                              + index * exif_format_get_size(entry_->format),
                              exif_data_get_byte_order(entry_->parent->parent));
    }

    ExifLong getLong(unsigned int index) const
    {
#ifndef EXIF_NO_EXCEPTIONS
        if(entry_->format != EXIF_FORMAT_LONG)
            throw InvalidFormat(
                "Exif::Entry::getLong(): Format is not EXIF_FORMAT_LONG");
        if(index >= components())
            throw InvalidIndex(
                "Exif::getLong: component index out of range");
#endif
        return exif_get_long(entry_->data
                             + index * exif_format_get_size(entry_->format),
                             exif_data_get_byte_order(entry_->parent->parent));
    }

    ExifSLong getSLong(unsigned int index) const
    {
#ifndef EXIF_NO_EXCEPTIONS
        if(entry_->format != EXIF_FORMAT_SLONG)
            throw InvalidFormat(
                "Exif::Entry::getSLong(): Format is not EXIF_FORMAT_SLONG");
        if(index >= components())
            throw InvalidIndex(
                "Exif::getSLong: component index out of range");
#endif
        return exif_get_slong(entry_->data
                              + index * exif_format_get_size(entry_->format),
                              exif_data_get_byte_order(entry_->parent->parent));
    }

    ExifRational getRational(unsigned int index) const
    {
#ifndef EXIF_NO_EXCEPTIONS
        if(entry_->format != EXIF_FORMAT_RATIONAL)
            throw InvalidFormat(
                "Exif::Entry::getRational(): Format is not EXIF_FORMAT_RATIONAL");
        if(index >= components())
            throw InvalidIndex(
                "Exif::getRational: component index out of range");
#endif
        return exif_get_rational(entry_->data
                                 + index * exif_format_get_size(entry_->format),
                                 exif_data_get_byte_order(entry_->parent->parent));
    }

    ExifSRational getSRational(unsigned int index) const
    {
#ifndef EXIF_NO_EXCEPTIONS
        if(entry_->format != EXIF_FORMAT_SRATIONAL)
            throw InvalidFormat(
                "Exif::Entry::getSRational(): Format is not EXIF_FORMAT_SRATIONAL");
        if(index >= components())
            throw InvalidIndex(
                "Exif::getSRational: component index out of range");
#endif
        return exif_get_srational(entry_->data
                                  + index * exif_format_get_size(entry_->format),
                                  exif_data_get_byte_order(entry_->parent->parent));
    }

    void setByte(unsigned int index, ExifByte value) const
    {
#ifndef EXIF_NO_EXCEPTIONS
        if(entry_->format != EXIF_FORMAT_BYTE)
            throw InvalidFormat(
                "Exif::Entry::setByte(): Format is not EXIF_FORMAT_BYTE");
        if(index >= components())
            throw InvalidIndex(
                "Exif::setByte: component index out of range");
#endif
        *(entry_->data
          + index * exif_format_get_size(entry_->format)) = value;
    }

/*
    const ExifAscii setAscii() const
    {
#ifndef EXIF_NO_EXCEPTIONS
        if(entry_->format != EXIF_FORMAT_ASCII)
            throw InvalidFormat(
                "Exif::Entry::setAscii(): Format is not EXIF_FORMAT_ASCII");
#endif
        return (ExifAscii)entry_->data;
    }
*/

    void setShort(unsigned int index, ExifShort value) const
    {
#ifndef EXIF_NO_EXCEPTIONS
        if(entry_->format != EXIF_FORMAT_SHORT)
            throw InvalidFormat(
                "Exif::Entry::setShort(): Format is not EXIF_FORMAT_SHORT");
        if(index >= components())
            throw InvalidIndex(
                "Exif::setShort: component index out of range");
#endif
        return exif_set_short(entry_->data
                              + index * exif_format_get_size(entry_->format),
                              exif_data_get_byte_order(entry_->parent->parent),
                              value);
    }

    void setLong(unsigned int index, ExifLong value) const
    {
#ifndef EXIF_NO_EXCEPTIONS
        if(entry_->format != EXIF_FORMAT_LONG)
            throw InvalidFormat(
                "Exif::Entry::setLong(): Format is not EXIF_FORMAT_LONG");
        if(index >= components())
            throw InvalidIndex(
                "Exif::setLong: component index out of range");
#endif
        return exif_set_long(entry_->data
                             + index * exif_format_get_size(entry_->format),
                             exif_data_get_byte_order(entry_->parent->parent),
                             value);
    }

    void setSLong(unsigned int index, ExifSLong value) const
    {
#ifndef EXIF_NO_EXCEPTIONS
        if(entry_->format != EXIF_FORMAT_SLONG)
            throw InvalidFormat(
                "Exif::Entry::setSLong(): Format is not EXIF_FORMAT_SLONG");
        if(index >= components())
            throw InvalidIndex(
                "Exif::setSLong: component index out of range");
#endif
        return exif_set_slong(entry_->data
                              + index * exif_format_get_size(entry_->format),
                              exif_data_get_byte_order(entry_->parent->parent),
                              value);
    }

    void setRational(unsigned int index, ExifRational value) const
    {
#ifndef EXIF_NO_EXCEPTIONS
        if(entry_->format != EXIF_FORMAT_RATIONAL)
            throw InvalidFormat(
                "Exif::Entry::setRational(): Format is not EXIF_FORMAT_RATIONAL");
        if(index >= components())
            throw InvalidIndex(
                "Exif::setRational: component index out of range");
#endif
        return exif_set_rational(entry_->data
                                 + index * exif_format_get_size(entry_->format),
                                 exif_data_get_byte_order(entry_->parent->parent),
                                 value);
    }

    void setSRational(unsigned int index, ExifSRational value) const
    {
#ifndef EXIF_NO_EXCEPTIONS
        if(entry_->format != EXIF_FORMAT_SRATIONAL)
            throw InvalidFormat(
                "Exif::Entry::setSRational(): Format is not EXIF_FORMAT_SRATIONAL");
        if(index >= components())
            throw InvalidIndex(
                "Exif::setSRational: component index out of range");
#endif
        return exif_set_srational(entry_->data
                                  + index * exif_format_get_size(entry_->format),
                                  exif_data_get_byte_order(entry_->parent->parent),
                                  value);
    }

    const char *value()
    {
        return exif_entry_get_value(entry_);
    }

    const char *briefValue()
    {
        return exif_entry_get_value_brief(entry_);
    }

    void dump(unsigned int indent = 0) const
    {
        exif_entry_dump(entry_, indent);
    }
};

struct Content
{
    ExifContent *content_;

    Content()
    : content_(exif_content_new())
    {}

    Content(Content const &other)
    : content_(other.content_)
    {
        exif_content_ref(content_);
    }

        // internal, do not use directly
    Content(ExifContent *content)
    : content_(content)
    {
        exif_content_ref(content_);
    }

    ~Content()
    {
        exif_content_unref(content_);
    }

    Content &operator=(Content const &other)
    {
        exif_content_unref(content_);
        content_ = other.content_;
        exif_content_ref(content_);
        return *this;
    }

    Entry operator[](ExifTag tag)
    {
        ExifEntry *result = exif_content_get_entry(content_, tag);
        if(result)
            return Entry(result);
#ifndef EXIF_NO_EXCEPTIONS
        throw InvalidIndex(
            "Exif::Content: IFD does not contain given tag");
#endif
        return Entry();
    }

    Entry operator[](unsigned int index)
    {
        if(index < size())
            return Entry(content_->entries[index]);
#ifndef EXIF_NO_EXCEPTIONS
        throw InvalidIndex(
            "Exif::Content: numeric entry index out of range");
#endif // EXIF_NO_EXCEPTIONS
        return Entry();
    }

    unsigned int size() const
    {
        // FIXME: content_ should never be NULL, so this is unneeded!?
        return content_ ? content_->count : 0;
    }

    void add(Entry &entry)
    {
        exif_content_add_entry(content_, entry.entry_);
    }

    void remove(Entry &entry)
    {
        exif_content_remove_entry(content_, entry.entry_);
    }

        // for your convenience
    const char *value(ExifTag tag)
    {
        return exif_content_get_value(content_, tag);
    }

        // for your convenience
    const char *briefValue(ExifTag tag)
    {
        return exif_content_get_value_brief(content_, tag);
    }

    void dump(unsigned int indent = 0) const
    {
        exif_content_dump(content_, indent);
    }
};

struct Data
{
    ExifData *data_;

    Data()
    : data_(exif_data_new())
    {}

    Data(const char *path, bool *success = 0)
    : data_(exif_data_new_from_file(path))
    {
        if(success)
            *success = data_;
#ifndef EXIF_NO_EXCEPTIONS
        else
            if(!data_)
                throw IOError("Exif::Data: Could not load file");
#endif // EXIF_NO_EXCEPTIONS
        if(!data_)
            exif_data_new();
    }

    Data(const unsigned char *data,
         unsigned int size)
    : data_(exif_data_new_from_data(data, size))
    {}

    Data(Data const &other)
    : data_(other.data_)
    {
        exif_data_ref(data_);
    }

    ~Data()
    {
        exif_data_unref(data_);
    }

    Data &operator=(Data const &other)
    {
        exif_data_unref(data_);
        data_ = other.data_;
        exif_data_ref(data_);
        return *this;
    }

    void save(unsigned char **d, unsigned int *size)
    {
        exif_data_save_data(data_, d, size);
    }

    unsigned int size() const
    {
        return EXIF_IFD_COUNT;
    }

    Content operator[](unsigned int index)
    {
        if(index < size())
            return Content(data_->ifd[index]);
#ifndef EXIF_NO_EXCEPTIONS
        throw InvalidIndex(
            "Exif::Data: IFD index out of range");
#endif // EXIF_NO_EXCEPTIONS
        return Content();
    }

    ExifByteOrder byteOrder() const
    {
        return exif_data_get_byte_order(data_);
    }

    void setByteOrder(ExifByteOrder bo) const
    {
        exif_data_set_byte_order(data_, bo);
    }

    void dump()
    {
        exif_data_dump(data_);
    }
};

} // namespace Exif

#endif // EXIF_HXX
