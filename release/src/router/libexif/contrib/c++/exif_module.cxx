/* exif-content.c
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

#include "exif.hxx"
#include <string>
#include <iostream>

#include <Python.h>
#include <boost/python.hpp>
using namespace boost::python;

template<class Wrapper, class Pointer>
struct WrappedObjectIterator
{
	//typedef Wrapper value_type;
	Pointer *it_, *end_;

	WrappedObjectIterator(Pointer *it, Pointer *end)
		: it_(it), end_(end)
	{}

	Wrapper next()
	{
		if(it_ == end_)
		{
			PyErr_SetString(PyExc_StopIteration, "iterator exhausted");
			throw_error_already_set();
		}
		return Wrapper(*it_++);
	}
};

struct PythonEntry : public Exif::Entry
{
	PythonEntry() {}
	PythonEntry(Exif::Entry const &other) : Exif::Entry(other) {}

	object component(long index) const
	{
		switch(format())
		{
		case EXIF_FORMAT_BYTE:
			return object(getByte(index));
		case EXIF_FORMAT_SHORT:
			return object(getShort(index));
		case EXIF_FORMAT_LONG:
			return object(getLong(index));
		case EXIF_FORMAT_SLONG:
			return object(getSLong(index));
		case EXIF_FORMAT_RATIONAL:
			return object(getRational(index));
		case EXIF_FORMAT_SRATIONAL:
			return object(getSRational(index));
		case EXIF_FORMAT_ASCII:
			//std::cerr << "returning " << entry_->size << " bytes of data..\n";
			//std::cerr << " (copied into " << std::string((char *)data, entry_->size).size() << "-character string)\n";
			return object(std::string((char *)entry_->data, entry_->size));
		default:
			break;
		}
		return object();
	}

	object data() const
	{
		if((format() == EXIF_FORMAT_ASCII) || (components()==1))
			return component(0);
		else
		{
			list result;
			for(unsigned int i=0; i<components(); ++i)
				result.append(component(i));
			return result;
		}
	}

	template<class Type>
	Type extractComponent(unsigned int index, object value,
						  const char *errorString)
	{
		extract<Type> extr(value);
		if(!extr.check())
		{
			PyErr_SetString(PyExc_TypeError, errorString);
			throw_error_already_set();
		}
		return extr();
	}

	void setComponent(unsigned int index, object value)
	{
		unsigned char *data= entry_->data
							 + index * exif_format_get_size(format());
		ExifByteOrder bo = exif_data_get_byte_order(entry_->parent->parent);

		switch(format())
		{
		case EXIF_FORMAT_BYTE:
			*data= extractComponent<ExifByte>(index, value, "invalid assignment to data: could not convert value to byte format");
			break;
		case EXIF_FORMAT_SHORT:
			exif_set_short(data, bo, extractComponent<ExifShort>(index, value, "invalid assignment to data: could not convert value to short format"));
			break;
		case EXIF_FORMAT_LONG:
			exif_set_long(data, bo, extractComponent<ExifLong>(index, value, "invalid assignment to data: could not convert value to long format"));
			break;
		case EXIF_FORMAT_SLONG:
			exif_set_slong(data, bo, extractComponent<ExifSLong>(index, value, "invalid assignment to data: could not convert value to signed long format"));
			break;
		case EXIF_FORMAT_RATIONAL:
			exif_set_rational(data, bo, extractComponent<ExifRational>(index, value, "invalid assignment to data: could not convert value to rational format (2-tuple expected)"));
			break;
		case EXIF_FORMAT_SRATIONAL:
			exif_set_srational(data, bo, extractComponent<ExifSRational>(index, value, "invalid assignment to data: could not convert value to signed rational format (2-tuple expected)"));
			break;
		case EXIF_FORMAT_ASCII: // handled in setData directly
		case EXIF_FORMAT_UNDEFINED:
			break;
		}
		return;
	}

	void setData(object data)
	{
		if(format() == EXIF_FORMAT_ASCII)
		{
			extract<std::string> xstr(data);
			if(xstr.check())
			{
				std::string s= xstr();
				if(entry_->data)
					free(entry_->data);
				entry_->components= s.size();
				//std::cerr << "assigning " << s.size() << "-character string..\n";
				entry_->size=
					exif_format_get_size(format()) * entry_->components;
				entry_->data= (unsigned char *)calloc(entry_->size+1,1);
				memcpy(entry_->data, s.data(), entry_->size);
			}
			else
			{
				PyErr_SetString(PyExc_TypeError,
								"invalid assignment to data of ASCII format entry: string expected");
				throw_error_already_set();
			}
		}
		else
		{
			if(components()==1)
				setComponent(0, data);
			else
			{
				extract<list> xlist(data);
				if(xlist.check())
				{
					list l= xlist();
					for(unsigned i=0; i<components(); ++i)
						setComponent(i, l[i]);
				}
				else
				{
					PyErr_SetString(PyExc_TypeError,
									"invalid assignment to data of entry with more than one component: list expected");
					throw_error_already_set();
				}
			}
		}
	}
};

struct PythonContent : public Exif::Content
{
	typedef WrappedObjectIterator<PythonEntry, ExifEntry *> iterator;

	PythonContent() {}
	PythonContent(Exif::Content const &other) : Exif::Content(other) {}

	PythonEntry entry(object index)
	{
		// TODO: use Exif::Content::entry() functions

		extract<ExifTag> xtag(index);
		if(xtag.check())
		{
			ExifTag index= xtag();
			for(unsigned int i=0; i<size(); i++)
			{
				if(content_->entries[i]->tag == index)
					return Exif::Entry(content_->entries[i]);
			}
			PyErr_SetString(PyExc_KeyError,
							"tag not present in IFD content");
			throw_error_already_set();
		}
		extract<int> xint(index);
		if(xint.check())
		{
			int index= xint();
			if((index>=0) && (index<(long)size()))
				return Exif::Entry(content_->entries[index]);
			if((index<0) && (index>=-(long)size()))
				return Exif::Entry(content_->entries[size()+index]);
			PyErr_SetString(PyExc_IndexError,
							"invalid integer index into IFD content");
			throw_error_already_set();
		}
		PyErr_SetString(PyExc_TypeError,
						"invalid index into EXIF data (integer or IFD expected)");
		throw_error_already_set();
		return Exif::Entry(); // never reached
	}

	iterator __iter__()
	{
		// FIXME: the public API is exif_content_foreach,
		// relying on memory layout here!
		return iterator(content_->entries,
						content_->entries + content_->count);
	}
};

struct PythonData : public Exif::Data
{
	typedef WrappedObjectIterator<PythonContent, ExifContent *> iterator;
	bool success_;

	PythonData() {}
	PythonData(const char *path)
	: Exif::Data(path, &success_)
	{
		if(!success_)
		{
			PyErr_SetFromErrno(PyExc_IOError);
			//PyErr_SetString(PyExc_IOError, "");
			throw_error_already_set();
		}
	}
	PythonData(const unsigned char *data,
			   unsigned int size) : Exif::Data(data, size) {}
	PythonData(Exif::Data const &other) : Exif::Data(other) {}

	PythonContent ifdContent(object index)
	{
		extract<ExifIfd> xifd(index);
		if(xifd.check())
		{
			ExifIfd index= xifd();
			if(index<EXIF_IFD_COUNT)
				return Exif::Content(data_->ifd[index]);
			PyErr_SetString(PyExc_IndexError,
							"invalid IFD index into EXIF data");
			throw_error_already_set();
		}
		extract<int> xint(index);
		if(xint.check())
		{
			int index= xint();
			if((index>=0) && (index<(long)size()))
				return Exif::Content(data_->ifd[index]);
			if((index<0) && (index>=-(long)size()))
				return Exif::Content(data_->ifd[size()+index]);
			PyErr_SetString(PyExc_IndexError,
							"invalid integer index into EXIF data");
			throw_error_already_set();
		}
		PyErr_SetString(PyExc_TypeError,
						"invalid index into EXIF data (integer or IFD expected)");
		throw_error_already_set();
		return Exif::Content(); // never reached
	}

	iterator __iter__()
	{
		return iterator(data_->ifd, data_->ifd + EXIF_IFD_COUNT);
	}
};

template<class Rational, class Component>
struct RationalConverter
{
	RationalConverter()
	{
		converter::registry::insert(&convertible, &construct,
									type_id<Rational>());
	}

    static void* convertible(PyObject* obj)
    {
		extract<tuple> xtup(obj);
		if(xtup.check())
		{
			tuple t= xtup();
			if((t.attr("__len__")() == 2) &&
			   extract<Component>(t[0]).check() &&
			   extract<Component>(t[1]).check())
			{
				Rational *result = new Rational;
				result->numerator =   extract<Component>(t[0])();
				result->denominator = extract<Component>(t[1])();
				return result;
			}
		}
		return NULL;
    }

    static void construct(PyObject* obj, converter::rvalue_from_python_stage1_data* data)
    {
		Rational const* r =
			static_cast<Rational*>(data->convertible);
        void* storage =
			((converter::rvalue_from_python_storage<Rational>*)data)->storage.bytes;
        new (storage) Rational();
		((Rational*)storage)->numerator = r->numerator;
		((Rational*)storage)->denominator = r->denominator;
        data->convertible = storage;
		delete r;
	}

	static PyObject *convert(Rational r)
	{
		tuple t= make_tuple(r.numerator, r.denominator);
		PyObject *result= t.ptr();
		Py_INCREF(result);
		return result;
	}
};

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(entrydumps, Exif::Entry::dump, 0, 1)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(contentdumps, Exif::Content::dump, 0, 1)

BOOST_PYTHON_MODULE(exif)
{
	RationalConverter<ExifRational, ExifLong>();
	RationalConverter<ExifSRational, ExifSLong>();
	to_python_converter<ExifRational,
		RationalConverter<ExifRational, ExifLong> >();
	to_python_converter<ExifSRational,
		RationalConverter<ExifSRational, ExifSLong> >();

	enum_<ExifByteOrder>("ByteOrder")
		.value("MOTOROLA", EXIF_BYTE_ORDER_MOTOROLA)
		.value("INTEL", EXIF_BYTE_ORDER_INTEL);

	def("name", &exif_byte_order_get_name);

	enum_<ExifIfd>("IFD")
		.value("ZERO", EXIF_IFD_0)
		.value("ONE", EXIF_IFD_1)
		.value("EXIF", EXIF_IFD_EXIF)
		.value("GPS", EXIF_IFD_GPS)
		.value("INTEROPERABILITY", EXIF_IFD_INTEROPERABILITY);
	//.value("COUNT", EXIF_IFD_COUNT)

	def("name", &exif_ifd_get_name);

	enum_<ExifFormat>("Format")
		.value("BYTE", EXIF_FORMAT_BYTE)
		.value("ASCII", EXIF_FORMAT_ASCII)
		.value("SHORT", EXIF_FORMAT_SHORT)
		.value("LONG", EXIF_FORMAT_LONG)
		.value("RATIONAL", EXIF_FORMAT_RATIONAL)
		.value("UNDEFINED", EXIF_FORMAT_UNDEFINED)
		.value("SLONG", EXIF_FORMAT_SLONG)
		.value("SRATIONAL", EXIF_FORMAT_SRATIONAL);

	def("name", &exif_format_get_name);
	def("size", &exif_format_get_size);

	enum_<ExifTag>("Tag")
		.value("INTEROPERABILITY_INDEX", EXIF_TAG_INTEROPERABILITY_INDEX)
		.value("INTEROPERABILITY_VERSION", EXIF_TAG_INTEROPERABILITY_VERSION)
		.value("IMAGE_WIDTH", EXIF_TAG_IMAGE_WIDTH)
		.value("IMAGE_LENGTH", EXIF_TAG_IMAGE_LENGTH)
		.value("BITS_PER_SAMPLE", EXIF_TAG_BITS_PER_SAMPLE)
		.value("COMPRESSION", EXIF_TAG_COMPRESSION)
		.value("PHOTOMETRIC_INTERPRETATION", EXIF_TAG_PHOTOMETRIC_INTERPRETATION)
		.value("FILL_ORDER", EXIF_TAG_FILL_ORDER)
		.value("DOCUMENT_NAME", EXIF_TAG_DOCUMENT_NAME)
		.value("IMAGE_DESCRIPTION", EXIF_TAG_IMAGE_DESCRIPTION)
		.value("MAKE", EXIF_TAG_MAKE)
		.value("MODEL", EXIF_TAG_MODEL)
		.value("STRIP_OFFSETS", EXIF_TAG_STRIP_OFFSETS)
		.value("ORIENTATION", EXIF_TAG_ORIENTATION)
		.value("SAMPLES_PER_PIXEL", EXIF_TAG_SAMPLES_PER_PIXEL)
		.value("ROWS_PER_STRIP", EXIF_TAG_ROWS_PER_STRIP)
		.value("STRIP_BYTE_COUNTS", EXIF_TAG_STRIP_BYTE_COUNTS)
		.value("X_RESOLUTION", EXIF_TAG_X_RESOLUTION)
		.value("Y_RESOLUTION", EXIF_TAG_Y_RESOLUTION)
		.value("PLANAR_CONFIGURATION", EXIF_TAG_PLANAR_CONFIGURATION)
		.value("RESOLUTION_UNIT", EXIF_TAG_RESOLUTION_UNIT)
		.value("TRANSFER_FUNCTION", EXIF_TAG_TRANSFER_FUNCTION)
		.value("SOFTWARE", EXIF_TAG_SOFTWARE)
		.value("DATE_TIME", EXIF_TAG_DATE_TIME)
		.value("ARTIST", EXIF_TAG_ARTIST)
		.value("WHITE_POINT", EXIF_TAG_WHITE_POINT)
		.value("PRIMARY_CHROMATICITIES", EXIF_TAG_PRIMARY_CHROMATICITIES)
		.value("TRANSFER_RANGE", EXIF_TAG_TRANSFER_RANGE)
		.value("JPEG_PROC", EXIF_TAG_JPEG_PROC)
		.value("JPEG_INTERCHANGE_FORMAT", EXIF_TAG_JPEG_INTERCHANGE_FORMAT)
		.value("JPEG_INTERCHANGE_FORMAT_LENGTH", EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH)
		.value("YCBCR_COEFFICIENTS", EXIF_TAG_YCBCR_COEFFICIENTS)
		.value("YCBCR_SUB_SAMPLING", EXIF_TAG_YCBCR_SUB_SAMPLING)
		.value("YCBCR_POSITIONING", EXIF_TAG_YCBCR_POSITIONING)
		.value("REFERENCE_BLACK_WHITE", EXIF_TAG_REFERENCE_BLACK_WHITE)
		.value("RELATED_IMAGE_FILE_FORMAT", EXIF_TAG_RELATED_IMAGE_FILE_FORMAT)
		.value("RELATED_IMAGE_WIDTH", EXIF_TAG_RELATED_IMAGE_WIDTH)
		.value("RELATED_IMAGE_LENGTH", EXIF_TAG_RELATED_IMAGE_LENGTH)
		.value("CFA_REPEAT_PATTERN_DIM", EXIF_TAG_CFA_REPEAT_PATTERN_DIM)
		.value("CFA_PATTERN", EXIF_TAG_CFA_PATTERN)
		.value("BATTERY_LEVEL", EXIF_TAG_BATTERY_LEVEL)
		.value("COPYRIGHT", EXIF_TAG_COPYRIGHT)
		.value("EXPOSURE_TIME", EXIF_TAG_EXPOSURE_TIME)
		.value("FNUMBER", EXIF_TAG_FNUMBER)
		.value("IPTC_NAA", EXIF_TAG_IPTC_NAA)
		.value("EXIF_IFD_POINTER", EXIF_TAG_EXIF_IFD_POINTER)
		.value("INTER_COLOR_PROFILE", EXIF_TAG_INTER_COLOR_PROFILE)
		.value("EXPOSURE_PROGRAM", EXIF_TAG_EXPOSURE_PROGRAM)
		.value("SPECTRAL_SENSITIVITY", EXIF_TAG_SPECTRAL_SENSITIVITY)
		.value("GPS_INFO_IFD_POINTER", EXIF_TAG_GPS_INFO_IFD_POINTER)
		.value("ISO_SPEED_RATINGS", EXIF_TAG_ISO_SPEED_RATINGS)
		.value("OECF", EXIF_TAG_OECF)
		.value("EXIF_VERSION", EXIF_TAG_EXIF_VERSION)
		.value("DATE_TIME_ORIGINAL", EXIF_TAG_DATE_TIME_ORIGINAL)
		.value("DATE_TIME_DIGITIZED", EXIF_TAG_DATE_TIME_DIGITIZED)
		.value("COMPONENTS_CONFIGURATION", EXIF_TAG_COMPONENTS_CONFIGURATION)
		.value("COMPRESSED_BITS_PER_PIXEL", EXIF_TAG_COMPRESSED_BITS_PER_PIXEL)
		.value("SHUTTER_SPEED_VALUE", EXIF_TAG_SHUTTER_SPEED_VALUE)
		.value("APERTURE_VALUE", EXIF_TAG_APERTURE_VALUE)
		.value("BRIGHTNESS_VALUE", EXIF_TAG_BRIGHTNESS_VALUE)
		.value("EXPOSURE_BIAS_VALUE", EXIF_TAG_EXPOSURE_BIAS_VALUE)
		.value("MAX_APERTURE_VALUE", EXIF_TAG_MAX_APERTURE_VALUE)
		.value("SUBJECT_DISTANCE", EXIF_TAG_SUBJECT_DISTANCE)
		.value("METERING_MODE", EXIF_TAG_METERING_MODE)
		.value("LIGHT_SOURCE", EXIF_TAG_LIGHT_SOURCE)
		.value("FLASH", EXIF_TAG_FLASH)
		.value("FOCAL_LENGTH", EXIF_TAG_FOCAL_LENGTH)
		.value("SUBJECT_AREA", EXIF_TAG_SUBJECT_AREA)
		.value("MAKER_NOTE", EXIF_TAG_MAKER_NOTE)
		.value("USER_COMMENT", EXIF_TAG_USER_COMMENT)
		.value("SUBSEC_TIME", EXIF_TAG_SUBSEC_TIME)
		.value("SUB_SEC_TIME_ORIGINAL", EXIF_TAG_SUB_SEC_TIME_ORIGINAL)
		.value("SUB_SEC_TIME_DIGITIZED", EXIF_TAG_SUB_SEC_TIME_DIGITIZED)
		.value("FLASH_PIX_VERSION", EXIF_TAG_FLASH_PIX_VERSION)
		.value("COLOR_SPACE", EXIF_TAG_COLOR_SPACE)
		.value("PIXEL_X_DIMENSION", EXIF_TAG_PIXEL_X_DIMENSION)
		.value("PIXEL_Y_DIMENSION", EXIF_TAG_PIXEL_Y_DIMENSION)
		.value("RELATED_SOUND_FILE", EXIF_TAG_RELATED_SOUND_FILE)
		.value("INTEROPERABILITY_IFD_POINTER", EXIF_TAG_INTEROPERABILITY_IFD_POINTER)
		.value("FLASH_ENERGY", EXIF_TAG_FLASH_ENERGY)
		.value("SPATIAL_FREQUENCY_RESPONSE", EXIF_TAG_SPATIAL_FREQUENCY_RESPONSE)
		.value("FOCAL_PLANE_X_RESOLUTION", EXIF_TAG_FOCAL_PLANE_X_RESOLUTION)
		.value("FOCAL_PLANE_Y_RESOLUTION", EXIF_TAG_FOCAL_PLANE_Y_RESOLUTION)
		.value("FOCAL_PLANE_RESOLUTION_UNIT", EXIF_TAG_FOCAL_PLANE_RESOLUTION_UNIT)
		.value("SUBJECT_LOCATION", EXIF_TAG_SUBJECT_LOCATION)
		.value("EXPOSURE_INDEX", EXIF_TAG_EXPOSURE_INDEX)
		.value("SENSING_METHOD", EXIF_TAG_SENSING_METHOD)
		.value("FILE_SOURCE", EXIF_TAG_FILE_SOURCE)
		.value("SCENE_TYPE", EXIF_TAG_SCENE_TYPE)
		.value("NEW_CFA_PATTERN", EXIF_TAG_NEW_CFA_PATTERN)
		.value("CUSTOM_RENDERED", EXIF_TAG_CUSTOM_RENDERED)
		.value("EXPOSURE_MODE", EXIF_TAG_EXPOSURE_MODE)
		.value("WHITE_BALANCE", EXIF_TAG_WHITE_BALANCE)
		.value("DIGITAL_ZOOM_RATIO", EXIF_TAG_DIGITAL_ZOOM_RATIO)
		.value("FOCAL_LENGTH_IN_35MM_FILM", EXIF_TAG_FOCAL_LENGTH_IN_35MM_FILM)
		.value("SCENE_CAPTURE_TYPE", EXIF_TAG_SCENE_CAPTURE_TYPE)
		.value("GAIN_CONTROL", EXIF_TAG_GAIN_CONTROL)
		.value("CONTRAST", EXIF_TAG_CONTRAST)
		.value("SATURATION", EXIF_TAG_SATURATION)
		.value("SHARPNESS", EXIF_TAG_SHARPNESS)
		.value("DEVICE_SETTING_DESCRIPTION", EXIF_TAG_DEVICE_SETTING_DESCRIPTION)
		.value("SUBJECT_DISTANCE_RANGE", EXIF_TAG_SUBJECT_DISTANCE_RANGE)
		.value("IMAGE_UNIQUE_ID", EXIF_TAG_IMAGE_UNIQUE_ID);

	def("name", &exif_tag_get_name);
	def("title", &exif_tag_get_title);
	def("description", &exif_tag_get_description);

	class_<PythonEntry>("Entry")
		.add_property("tag", &Exif::Entry::tag)
		.add_property("format", &Exif::Entry::format)
		.add_property("components", &Exif::Entry::components)
		.add_property("data", &PythonEntry::data,
					  &PythonEntry::setData)
		.def("value", &Exif::Entry::value)
		.def("briefValue", &Exif::Entry::briefValue)
		.def("dump", &Exif::Entry::dump);//, entrydumps());

	class_<PythonContent::iterator>("ContentIterator", no_init)
		.def("next", &PythonContent::iterator::next);
	class_<PythonContent>("Content")
		.def("__len__", &Exif::Content::size)
		.def("__getitem__", &PythonContent::entry)
		.def("__iter__", &PythonContent::__iter__)
 		.def("dump", &Exif::Content::dump);//, contentdumps());

	class_<PythonData::iterator>("DataIterator", no_init)
		.def("next", &PythonData::iterator::next);
	class_<PythonData>("Data")
		.def(init<const char *>())
		.def(init<const unsigned char *, unsigned int>())
		.def("__len__", &Exif::Data::size)
		.def("__getitem__", &PythonData::ifdContent)
		.def("__iter__", &PythonData::__iter__)
		.def("byteOrder", &Exif::Data::byteOrder)
		.def("dump", &Exif::Data::dump);
}
