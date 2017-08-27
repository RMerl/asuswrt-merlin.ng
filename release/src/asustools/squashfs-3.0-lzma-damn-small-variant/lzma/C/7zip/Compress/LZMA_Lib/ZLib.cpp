/*
 * lzma zlib simplified wrapper
 *
 * Copyright (c) 2005-2006 Oleg I. Vdovikin <oleg@cs.msu.su>
 *
 * This library is free software; you can redistribute 
 * it and/or modify it under the terms of the GNU Lesser 
 * General Public License as published by the Free Software 
 * Foundation; either version 2.1 of the License, or 
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be 
 * useful, but WITHOUT ANY WARRANTY; without even the implied 
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE. See the GNU Lesser General Public License 
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General 
 * Public License along with this library; if not, write to 
 * the Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA 02111-1307 USA 
 *
 * 07/10/06 - jc - Added LZMA encoding parameter specification (_LZMA_PARAMS)
 *				   contact: jeremy@bitsum.com
 */

/*
 * default values for encoder/decoder used by wrapper
 */

#include <zlib.h>


/* jc: undef to kill compress2_lzma */
#define _LZMA_PARAMS 

#define ZLIB_LC 3
#define ZLIB_LP 0
#define ZLIB_PB 2
#define ZLIB_FB 128 /* jc: add default fast bytes param */

#ifdef WIN32
#include <initguid.h>
#else
#define INITGUID
#endif

#include "../../../Common/MyWindows.h"
#include "../LZMA/LZMADecoder.h"
#include "../LZMA/LZMAEncoder.h"

#define STG_E_SEEKERROR                  ((HRESULT)0x80030019L)
#define STG_E_MEDIUMFULL                 ((HRESULT)0x80030070L)

class CInMemoryStream: 
  public IInStream,
  public IStreamGetSize,
  public CMyUnknownImp
{
public:
  CInMemoryStream(const Bytef *data, UInt64 size) : 
	  m_data(data), m_size(size), m_offset(0) {}

  virtual ~CInMemoryStream() {}

  MY_UNKNOWN_IMP2(IInStream, IStreamGetSize)

  STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize)
  {
	  if (size > m_size - m_offset) 
		  size = m_size - m_offset;

	  if (size) {
		  memcpy(data, m_data + m_offset, size);
	  }

	  m_offset += size;

	  if (processedSize) 
		  *processedSize = size;

	  return S_OK;
  }

  STDMETHOD(ReadPart)(void *data, UInt32 size, UInt32 *processedSize)
  {
	return Read(data, size, processedSize);
  }

  STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition)
  {
	  UInt64 _offset;

	  if (seekOrigin == STREAM_SEEK_SET) _offset = offset;
	  else if (seekOrigin == STREAM_SEEK_CUR) _offset = m_offset + offset; 
	  else if (seekOrigin == STREAM_SEEK_END) _offset = m_size;
	  else return STG_E_INVALIDFUNCTION;

	  if (_offset < 0 || _offset > m_size)
		  return STG_E_SEEKERROR;

	  m_offset = _offset;

	  if (newPosition)
		  *newPosition = m_offset;

	  return S_OK;
  }

  STDMETHOD(GetSize)(UInt64 *size)
  {
	  *size = m_size;
	  return S_OK;
  }
protected:
	const Bytef *m_data;
	UInt64 m_size;
	UInt64 m_offset;
};

class COutMemoryStream: 
  public IOutStream,
  public CMyUnknownImp
{
public:
  COutMemoryStream(Bytef *data, UInt64 maxsize) : 
	  m_data(data), m_size(0), m_maxsize(maxsize), m_offset(0) {}
  virtual ~COutMemoryStream() {}
  
  MY_UNKNOWN_IMP1(IOutStream)

  STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize)
  {
	  if (size > m_maxsize - m_offset) 
		  size = m_maxsize - m_offset;

	  if (size) {
		  memcpy(m_data + m_offset, data, size);
	  }

	  m_offset += size;

	  if (m_offset > m_size)
		m_size = m_offset;

	  if (processedSize) 
		  *processedSize = size;

	  return S_OK;
  }
  
  STDMETHOD(WritePart)(const void *data, UInt32 size, UInt32 *processedSize)
  {
	  return Write(data, size, processedSize);
  }

  STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition)
  {
	  UInt64 _offset;

	  if (seekOrigin == STREAM_SEEK_SET) _offset = offset;
	  else if (seekOrigin == STREAM_SEEK_CUR) _offset = m_offset + offset; 
	  else if (seekOrigin == STREAM_SEEK_END) _offset = m_size;
	  else return STG_E_INVALIDFUNCTION;

	  if (_offset < 0 || _offset > m_maxsize)
		  return STG_E_SEEKERROR;

	  m_offset = _offset;

	  if (newPosition)
		  *newPosition = m_offset;

	  return S_OK;
  }
  
  STDMETHOD(SetSize)(Int64 newSize)
  {
	  if ((UInt64)newSize > m_maxsize) 
		  return STG_E_MEDIUMFULL;

	  return S_OK;
  }
protected:
	Bytef *m_data;
	UInt64 m_size;
	UInt64 m_maxsize;
	UInt64 m_offset;
};


#ifdef _LZMA_PARAMS



/* jc: new compress2 proxy that allows lzma param specification */
extern "C" int compress2_lzma_test (Bytef *dest,   uLongf *destLen,
                                  	const Bytef *source, uLong sourceLen,
                                  	int level, int fb, int lc, int lp, int pb)
{	
	CInMemoryStream *inStreamSpec = new CInMemoryStream(source, sourceLen);
	CMyComPtr<ISequentialInStream> inStream = inStreamSpec;
	
	COutMemoryStream *outStreamSpec = new COutMemoryStream(dest, *destLen);
	CMyComPtr<ISequentialOutStream> outStream = outStreamSpec;
	
	NCompress::NLZMA::CEncoder *encoderSpec = 
		new NCompress::NLZMA::CEncoder;
	CMyComPtr<ICompressCoder> encoder = encoderSpec;
	
	PROPID propIDs[] = 
	{
		NCoderPropID::kDictionarySize,
		NCoderPropID::kPosStateBits,
		NCoderPropID::kLitContextBits,
		NCoderPropID::kLitPosBits,
		NCoderPropID::kAlgorithm,
		NCoderPropID::kNumFastBytes,
		NCoderPropID::kMatchFinder,
		NCoderPropID::kEndMarker
	};
	const int kNumProps = sizeof(propIDs) / sizeof(propIDs[0]);
	
	if(pb<0) pb=ZLIB_PB;
	if(lc<0) lc=ZLIB_LC;
	if(lp<0) lp=ZLIB_LP;
	if(fb<0) fb=ZLIB_FB;
	PROPVARIANT properties[kNumProps];
	for (int p = 0; p < 6; p++)
		properties[p].vt = VT_UI4;
	properties[0].ulVal = UInt32(1 << (level + 14));
	properties[1].ulVal = UInt32(pb);
	properties[2].ulVal = UInt32(lc); // for normal files
	properties[3].ulVal = UInt32(lp); // for normal files
	properties[4].ulVal = UInt32(2);       // todo
	properties[5].ulVal = UInt32(fb); 
	
	properties[6].vt = VT_BSTR;
	properties[6].bstrVal = (BSTR)(const wchar_t *)L"BT4";
	
	properties[7].vt = VT_BOOL;
	properties[7].boolVal = VARIANT_TRUE;
	
	if (encoderSpec->SetCoderProperties(propIDs, properties, kNumProps) != S_OK)
		return Z_MEM_ERROR; // should not happen
	
	HRESULT result = encoder->Code(inStream, outStream, 0, 0, 0);
	if (result == E_OUTOFMEMORY)
	{
		return Z_MEM_ERROR;
	}   
	else if (result != S_OK)
	{
		return Z_BUF_ERROR;	// should not happen
	}   
	
	UInt64 fileSize;
	outStreamSpec->Seek(0, STREAM_SEEK_END, &fileSize);
	*destLen = fileSize;
	
	return Z_OK;
}

#include <malloc.h>
#include <stdio.h>
#include <pthread.h>


unsigned char pbmatrix[3]={0,1,2};
unsigned char lcmatrix[4]={0,1,2,3};
unsigned char lpmatrix[4]={0,1,2,3};

struct MATRIXENTRY
{
int pb;
int lc;
int lp;
};

struct MATRIXENTRY matrix[]={
{2,0,0},
{2,0,1},
{2,0,2},
{2,1,0},
{2,1,2},
{2,2,0},
{2,3,0},
{0,2,0},
{0,2,1},
{0,3,0},
{0,0,0},
{0,0,2},
{1,0,1},
{1,2,0},
{1,3,0}
};



int pbsave = -1;
int lcsave = -1;
int lpsave = -1;


int testcount=0;

int testlevel;
int testfb;
pthread_mutex_t	pos_mutex;
Bytef *test1;
const Bytef *testsource;

uLongf test2len;
uLongf test1len;
uLongf testsourcelen;
int running=0;
extern "C" void *brute(void *arg)
{
	int oldstate;
	uLongf test3len = test2len;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldstate);

pthread_mutex_lock(&pos_mutex);
int takelcvalue = matrix[testcount].lc;
int takepbvalue = matrix[testcount].pb;
int takelpvalue = matrix[testcount].lp;
testcount++;
if (testcount==(sizeof(matrix)/sizeof(struct MATRIXENTRY)))
    {
    running--;
    pthread_mutex_unlock(&pos_mutex);
    return NULL;
    }
pthread_mutex_unlock(&pos_mutex);
Bytef *test2 = (Bytef*)malloc(test2len);
//fprintf(stderr,"try method [pb:%d lc:%d lp:%d fb:%d]\n",pbtest,lctest,lptest,testfb);
int ret =  compress2_lzma_test(test2,&test3len,testsource,testsourcelen,testlevel,testfb,takelcvalue,takelpvalue,takepbvalue);
//fprintf(stderr,"test return %d\n",ret);
pthread_mutex_lock(&pos_mutex);
if (test3len<test1len)
    {
    test1len = test3len;
    memcpy(test1,test2,test3len);
    pbsave = takepbvalue;
    lcsave = takelcvalue;
    lpsave = takelpvalue;
    }
//fprintf(stderr,"finished %d running\n",running);
running--;
pthread_mutex_unlock(&pos_mutex);
free(test2);
return NULL;
}



extern "C" int compress2_lzma (Bytef *dest,   uLongf *destLen,
                                  	const Bytef *source, uLong sourceLen,
                                  	int level, int fb, int lc, int lp, int pb)
{
int i,a;
pthread_t *thread;
test1 = (Bytef*)malloc(*destLen);
test1len = *destLen+*destLen;
test2len = *destLen;
testsource = source;
testfb = fb;
testsourcelen = sourceLen;
testlevel = level;
testcount=0;
	if((thread = (pthread_t *)malloc((4) * sizeof(pthread_t))) == NULL)
		fprintf(stderr,"Out of memory allocating thread descriptors\n");
for (a=0;a<2;a++)
{
running=4;
	for(i = 0; i < 4; i++) {
		if(pthread_create(&thread[i], NULL, brute, NULL) != 0 )
			fprintf(stderr,"Failed to create thread\n");
	}
	for (i=0;i<4;i++)
	    {
	    pthread_join(thread[i],NULL);
	    }
}
    fprintf(stderr,"use method [pb:%d lc:%d lp:%d fb:%d] (len %d)\n",pbsave,lcsave,lpsave,fb,test1len);
    memcpy(dest+4,test1,test1len);
    dest[0]=pbsave;
    dest[1]=lcsave;
    dest[2]=lpsave;
    dest[3]=fb;
    *destLen=test1len+4;
free(thread);
    free(test1);
    return Z_OK;
}


#endif

ZEXTERN int ZEXPORT compress2 OF((Bytef *dest,   uLongf *destLen,
                                  const Bytef *source, uLong sourceLen,
                                  int level))
{
	CInMemoryStream *inStreamSpec = new CInMemoryStream(source, sourceLen);
	CMyComPtr<ISequentialInStream> inStream = inStreamSpec;
	
	COutMemoryStream *outStreamSpec = new COutMemoryStream(dest, *destLen);
	CMyComPtr<ISequentialOutStream> outStream = outStreamSpec;
	
	NCompress::NLZMA::CEncoder *encoderSpec = 
		new NCompress::NLZMA::CEncoder;
	CMyComPtr<ICompressCoder> encoder = encoderSpec;
	
	PROPID propIDs[] = 
	{
		NCoderPropID::kDictionarySize,
		NCoderPropID::kPosStateBits,
		NCoderPropID::kLitContextBits,
		NCoderPropID::kLitPosBits,
		NCoderPropID::kAlgorithm,
		NCoderPropID::kNumFastBytes,
		NCoderPropID::kMatchFinder,
		NCoderPropID::kEndMarker
	};
	const int kNumProps = sizeof(propIDs) / sizeof(propIDs[0]);
	
	PROPVARIANT properties[kNumProps];
	for (int p = 0; p < 6; p++)
		properties[p].vt = VT_UI4;
	properties[0].ulVal = UInt32(1 << (level + 14));
	properties[1].ulVal = UInt32(ZLIB_PB);
	properties[2].ulVal = UInt32(ZLIB_LC); // for normal files
	properties[3].ulVal = UInt32(ZLIB_LP); // for normal files
	properties[4].ulVal = UInt32(2);
	properties[5].ulVal = UInt32(ZLIB_FB);
	
	properties[6].vt = VT_BSTR;
	properties[6].bstrVal = (BSTR)(const wchar_t *)L"BT4";
	
	properties[7].vt = VT_BOOL;
	properties[7].boolVal = VARIANT_TRUE;
	
	if (encoderSpec->SetCoderProperties(propIDs, properties, kNumProps) != S_OK)
		{
		fprintf(stderr,"coder properties error\n");
		return Z_MEM_ERROR; // should not happen
		}
	HRESULT result = encoder->Code(inStream, outStream, 0, 0, 0);
	if (result == E_OUTOFMEMORY)
	{
		fprintf(stderr,"mem error\n");
		return Z_MEM_ERROR;
	}   
	else if (result != S_OK)
	{
		return Z_BUF_ERROR;	// should not happen
	}   
	
	UInt64 fileSize;
	outStreamSpec->Seek(0, STREAM_SEEK_END, &fileSize);
	*destLen = fileSize;
	
	return Z_OK;
}

ZEXTERN int ZEXPORT uncompress OF((Bytef *dest,   uLongf *destLen,
                                   const Bytef *source, uLong sourceLen))
{
	CInMemoryStream *inStreamSpec = new CInMemoryStream(source+4, sourceLen-4);
	CMyComPtr<ISequentialInStream> inStream = inStreamSpec;
	
	COutMemoryStream *outStreamSpec = new COutMemoryStream(dest, *destLen);
	CMyComPtr<ISequentialOutStream> outStream = outStreamSpec;
	
	NCompress::NLZMA::CDecoder *decoderSpec = 
		new NCompress::NLZMA::CDecoder;
	CMyComPtr<ICompressCoder> decoder = decoderSpec;
	
	if (decoderSpec->SetDecoderPropertiesRaw(source[1], 
		source[2], source[0], (1 << 23)) != S_OK) return Z_DATA_ERROR;
	
	UInt64 fileSize = *destLen;
	
	if (decoder->Code(inStream, outStream, 0, &fileSize, 0) != S_OK)
	{
		return Z_DATA_ERROR;
	}
	
	outStreamSpec->Seek(0, STREAM_SEEK_END, &fileSize);
	*destLen = fileSize;
	
	return Z_OK;
}
