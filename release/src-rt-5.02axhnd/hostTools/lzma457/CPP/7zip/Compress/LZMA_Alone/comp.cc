/*
 * Copyright (C) 2006-2007 Junjiro Okajima
 * Copyright (C) 2006-2007 Tomas Matejicek, slax.org
 *
 * LICENSE follows the described one in lzma.txt.
 */

/* $Id: comp.cc,v 1.3 2007-11-13 13:27:23 jro Exp $ */

// extract some parts from lzma443/C/7zip/Compress/LZMA_Alone/LzmaAlone.cpp

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include "StdAfx.h"
#include "../../../Common/MyInitGuid.h"
//#include "../../../Common/MyWindows.h"
#include "../../../Common/StringConvert.h"
//#include "../../../Common/StringToInt.h"
//#include "../../Common/StreamUtils.h"
#include "../LZMA/LZMAEncoder.h"

#include <pthread.h>
#include <zlib.h>
#include "sqlzma.h"

//////////////////////////////////////////////////////////////////////

class CMemoryStream {
protected:
	Bytef *m_data;
	UInt64 m_limit;
	UInt64 m_pos;

public:
	CMemoryStream(Bytef *data, UInt64 size)
		: m_data(data), m_limit(size), m_pos(0) {}

	virtual ~CMemoryStream() {}
};

class CInMemoryStream : public CMemoryStream, public IInStream,
			public CMyUnknownImp {
//protected:
	CMyComPtr<ISequentialInStream> m_stream;

public:
	MY_UNKNOWN_IMP1(IInStream);

	CInMemoryStream(Bytef *data, UInt64 size)
		: CMemoryStream(data, size), m_stream(this) {}

	virtual ~CInMemoryStream() {}

	STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize)
	{
		UInt64 room = m_limit - m_pos;
		if (size > room)
			size = room;
		if (size) {
			memcpy(data, m_data + m_pos, size);
			m_pos += size;
		}
		if (processedSize)
			*processedSize = size;
		return S_OK;
	}

	// disabled all
	STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) {
		assert(0);
		return E_NOTIMPL;
	}
};

class COutMemoryStream : public CMemoryStream, public IOutStream,
			 public CMyUnknownImp {
//protected:
	CMyComPtr<ISequentialOutStream> m_stream;

public:
	MY_UNKNOWN_IMP1(IOutStream);

	COutMemoryStream(Bytef *data, UInt64 size)
		: CMemoryStream(data, size), m_stream(this) {}

	virtual ~COutMemoryStream() {}

	UInt32 GetSize() {return m_pos;}

	STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize) {
		if (m_pos + size > m_limit)
			return -ENOSPC;
		memcpy(m_data + m_pos, data, size);
		m_pos += size;
		if (processedSize)
			*processedSize = size;
		return S_OK;
	}

	// disabled all
	STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) {
		assert(0);
		return E_NOTIMPL;
	}
	STDMETHOD(SetSize)(Int64 newSize) {
		assert(0);
		return E_NOTIMPL;
	}
};

//////////////////////////////////////////////////////////////////////

static int
LzmaCompress(Bytef *next_in, uInt avail_in, Bytef *next_out, uInt avail_out,
	     struct sqlzma_opts *opts, uLong *total_out)
{
	int err;
	HRESULT res;
	const Byte a[] = {
		avail_in, avail_in >> 8, avail_in >> 16, avail_in >> 24,
		0, 0, 0, 0
	};

	NCompress::NLZMA::CEncoder encoderSpec;
	CMyComPtr<ICompressCoder> encoder = &encoderSpec;
	encoder->AddRef();
	CInMemoryStream inStreamSpec(next_in, avail_in);
	CMyComPtr<ISequentialInStream> inStream = &inStreamSpec;
	inStream->AddRef();
	COutMemoryStream outStreamSpec(next_out, avail_out);
	CMyComPtr<ISequentialOutStream> outStream = &outStreamSpec;
	outStream->AddRef();

	// these values are dpending upon is_lzma() macro in sqlzma.h
	const UInt32 dictionary = opts->dicsize;
	//fprintf(stderr, "dic %u\n", dictionary);
	const UString mf = L"BT4";
	const UInt32 posStateBits = 2;
	const UInt32 litContextBits = 3; // for normal files
	// UInt32 litContextBits = 0; // for 32-bit data
	const UInt32 litPosBits = 0;
	// UInt32 litPosBits = 2; // for 32-bit data
	//const UInt32 algorithm = 2;
	const UInt32 algorithm = 1;
	const UInt32 numFastBytes = 128;
	const UInt32 matchFinderCycles = 16 + numFastBytes / 2;
	//const bool matchFinderCyclesDefined = false;
	const PROPID propIDs[] = {
		NCoderPropID::kDictionarySize,
		NCoderPropID::kPosStateBits,
		NCoderPropID::kLitContextBits,
		NCoderPropID::kLitPosBits,
		NCoderPropID::kAlgorithm,
		NCoderPropID::kNumFastBytes,
		NCoderPropID::kMatchFinder,
		NCoderPropID::kEndMarker,
		NCoderPropID::kNumThreads,
		NCoderPropID::kMatchFinderCycles
	};
	const int kNumPropsMax = sizeof(propIDs) / sizeof(propIDs[0]);
	PROPVARIANT properties[kNumPropsMax];
	for (int p = 0; p < 6; p++)
		properties[p].vt = VT_UI4;
	properties[0].ulVal = UInt32(dictionary);
	properties[1].ulVal = UInt32(posStateBits);
	properties[2].ulVal = UInt32(litContextBits);
	properties[3].ulVal = UInt32(litPosBits);
	properties[4].ulVal = UInt32(algorithm);
	properties[5].ulVal = UInt32(numFastBytes);

	properties[6].vt = VT_BSTR;
	properties[6].bstrVal = (BSTR)(const wchar_t *)mf;
	properties[7].vt = VT_BOOL;
	properties[7].boolVal = VARIANT_FALSE;	// EOS
	properties[8].vt = VT_UI4;
	properties[8].ulVal = 1; // numThreads
	properties[9].vt = VT_UI4;
	properties[9].ulVal = UInt32(matchFinderCycles);

	err = -EINVAL;
	res = encoderSpec.SetCoderProperties(propIDs, properties,
					     kNumPropsMax - 1);
	if (res)
		goto out;
	res = encoderSpec.WriteCoderProperties(outStream);
	if (res)
		goto out;

	UInt32 r;
	res = outStream->Write(a, sizeof(a), &r);
	if (res || r != sizeof(a))
		goto out;

	err = encoder->Code(inStream, outStream, 0, /*broken*/0, 0);
	if (err)
		goto out;
	*total_out = outStreamSpec.GetSize();

 out:
	return err;
}

//////////////////////////////////////////////////////////////////////

#define Failure(p) do { \
	fprintf(stderr, "%s:%d: please report to jro " \
		"{%02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x}\n", \
		__func__, __LINE__, \
		p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]); \
	abort(); \
} while(0)

extern "C" int
sqlzma_cm(struct sqlzma_opts *opts, z_stream *stream, Bytef *next_in, uInt
	  avail_in, Bytef *next_out, uInt avail_out)
{
	int err;
	Bytef *p = next_out;
	uInt l = avail_out;

	stream->next_in = next_in;
	stream->avail_in = avail_in;
	stream->next_out = p;
	stream->avail_out = l;
	err = deflate(stream, Z_FINISH);
	if (err != Z_STREAM_END && err != Z_OK)
		goto out_err;
	if (avail_in < stream->total_out)
		return err;
	if (is_lzma(*p))
		Failure(p);

	if (opts->try_lzma) {
		unsigned char a[stream->total_out];
		uLong processed=0;

		//printf("\n == LZMA == \n");

		memcpy(a, p, stream->total_out);

		// malloc family in glibc and stdc++ seems to be thread-safe
		err = LzmaCompress(next_in, avail_in, p, l, opts, &processed);
		
#if 1	// Disable ZLIB compression: this is the safer choice
		if (!err) {
			if (!is_lzma(*next_out))
				Failure(next_out);
			stream->total_out = processed;
			err = Z_STREAM_END;
		}
#else
		if (!err && processed <= stream->total_out) {
			if (!is_lzma(*next_out))
				Failure(next_out);
			stream->total_out = processed;
			err = Z_STREAM_END;
		} else {
			//puts("by zlib");
			memcpy(p, a, stream->total_out);
			err = Z_STREAM_END;
		}
#endif

	}
	return err;

 out_err:
	fprintf(stderr, "%s: ZLIB err %s\n", __func__, zError(err));
	return err;
}
