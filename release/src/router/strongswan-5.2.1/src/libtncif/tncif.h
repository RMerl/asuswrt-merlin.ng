/* tncif.h
 *
 * Trusted Network Connect IF-IMC/IMV API version 1.30
 * Microsoft Windows DLL Platform Binding C Header
 * October 14, 2011
 *
 *   Common definitions for IF-IMC and IF-IMV
 *   extracted from tncifimc.h and tncifimv.h
 *
 * Copyright(c) 2005-2011, Trusted Computing Group, Inc. All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * o Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * o Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * o Neither the name of the Trusted Computing Group nor the names of
 *   its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written
 *   permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Contact the Trusted Computing Group at
 * admin@trustedcomputinggroup.org for information on specification
 * licensing through membership agreements.
 *
 * Any marks and brands contained herein are the property of their
 * respective owners.
 *
 */

/**
 * @defgroup tncif tncif
 * @{ @ingroup libtncif
 */

#ifndef TNCIF_H_
#define TNCIF_H_

/* Basic Types */

typedef unsigned long TNC_UInt32;
typedef unsigned char *TNC_BufferReference;

/* Derived Types */

typedef TNC_UInt32 TNC_ConnectionID;
typedef TNC_UInt32 TNC_ConnectionState;
typedef TNC_UInt32 TNC_RetryReason;
typedef TNC_UInt32 TNC_MessageType;
typedef TNC_MessageType *TNC_MessageTypeList;
typedef TNC_UInt32 TNC_VendorID;
typedef TNC_VendorID *TNC_VendorIDList;
typedef TNC_UInt32 TNC_MessageSubtype;
typedef TNC_MessageSubtype *TNC_MessageSubtypeList;
typedef TNC_UInt32 TNC_Version;
typedef TNC_UInt32 TNC_Result;
typedef TNC_UInt32 TNC_AttributeID;

/* Result Codes */

#define TNC_RESULT_SUCCESS 0
#define TNC_RESULT_NOT_INITIALIZED 1
#define TNC_RESULT_ALREADY_INITIALIZED 2
#define TNC_RESULT_NO_COMMON_VERSION 3
#define TNC_RESULT_CANT_RETRY 4
#define TNC_RESULT_WONT_RETRY 5
#define TNC_RESULT_INVALID_PARAMETER 6
#define TNC_RESULT_CANT_RESPOND 7
#define TNC_RESULT_ILLEGAL_OPERATION 8
#define TNC_RESULT_OTHER 9
#define TNC_RESULT_FATAL 10
#define TNC_RESULT_EXCEEDED_MAX_ROUND_TRIPS 0x00559700
#define TNC_RESULT_EXCEEDED_MAX_MESSAGE_SIZE 0x00559701
#define TNC_RESULT_NO_LONG_MESSAGE_TYPES 0x00559702
#define TNC_RESULT_NO_SOH_SUPPORT 0x00559703

/* Network Connection ID Values */

#define TNC_CONNECTIONID_ANY 0xFFFFFFFF

/* Network Connection State Values */

#define TNC_CONNECTION_STATE_CREATE 0
#define TNC_CONNECTION_STATE_HANDSHAKE 1
#define TNC_CONNECTION_STATE_ACCESS_ALLOWED 2
#define TNC_CONNECTION_STATE_ACCESS_ISOLATED 3
#define TNC_CONNECTION_STATE_ACCESS_NONE 4
#define TNC_CONNECTION_STATE_DELETE 5

/* IMC/IMV ID Values */

#define TNC_IMVID_ANY ((TNC_UInt32) 0xffff)
#define TNC_IMCID_ANY ((TNC_UInt32) 0xffff)

/* Vendor ID Values */

#define TNC_VENDORID_TCG 0
#define TNC_VENDORID_TCG_NEW 0x005597
#define TNC_VENDORID_ANY ((TNC_VendorID) 0xffffff)

/* Message Subtype Values */

#define TNC_SUBTYPE_ANY ((TNC_MessageSubtype) 0xff)

/* Message Flags Values */

#define TNC_MESSAGE_FLAGS_EXCLUSIVE ((TNC_UInt32) 0x80000000)

/* Message Attribute ID Values */

#define TNC_ATTRIBUTEID_PREFERRED_LANGUAGE ((TNC_AttributeID) 0x00000001)
#define TNC_ATTRIBUTEID_MAX_ROUND_TRIPS ((TNC_AttributeID) 0x00559700)
#define TNC_ATTRIBUTEID_MAX_MESSAGE_SIZE ((TNC_AttributeID) 0x00559701)
#define TNC_ATTRIBUTEID_DHPN ((TNC_AttributeID) 0x00559702)
#define TNC_ATTRIBUTEID_HAS_LONG_TYPES  ((TNC_AttributeID) 0x00559703)
#define TNC_ATTRIBUTEID_HAS_EXCLUSIVE ((TNC_AttributeID) 0x00559704)
#define TNC_ATTRIBUTEID_HAS_SOH ((TNC_AttributeID) 0x00559705)
#define TNC_ATTRIBUTEID_IFTNCCS_PROTOCOL ((TNC_AttributeID) 0x0055970A)
#define TNC_ATTRIBUTEID_IFTNCCS_VERSION ((TNC_AttributeID) 0x0055970B)
#define TNC_ATTRIBUTEID_IFT_PROTOCOL ((TNC_AttributeID) 0x0055970C)
#define TNC_ATTRIBUTEID_IFT_VERSION ((TNC_AttributeID) 0x0055970D)
#define TNC_ATTRIBUTEID_TLS_UNIQUE ((TNC_AttributeID) 0x0055970E)

#endif /** TNCIF_H_ @}*/
