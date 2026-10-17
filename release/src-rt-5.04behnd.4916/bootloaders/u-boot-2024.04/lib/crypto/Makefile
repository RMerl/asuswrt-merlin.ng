# SPDX-License-Identifier: GPL-2.0+
#
# Makefile for asymmetric cryptographic keys
#

obj-$(CONFIG_$(SPL_)ASYMMETRIC_KEY_TYPE) += asymmetric_keys.o

asymmetric_keys-y := asymmetric_type.o

obj-$(CONFIG_$(SPL_)ASYMMETRIC_PUBLIC_KEY_SUBTYPE) += public_key.o

#
# RSA public key parser
#
obj-$(CONFIG_$(SPL_)RSA_PUBLIC_KEY_PARSER) += rsa_public_key.o
rsa_public_key-y := \
	rsapubkey.asn1.o \
	rsa_helper.o

$(obj)/rsapubkey.asn1.o: $(obj)/rsapubkey.asn1.c $(obj)/rsapubkey.asn1.h
ifdef CONFIG_SPL_BUILD
CFLAGS_rsapubkey.asn1.o += -I$(obj)
endif

$(obj)/rsa_helper.o: $(obj)/rsapubkey.asn1.h
ifdef CONFIG_SPL_BUILD
CFLAGS_rsa_helper.o += -I$(obj)
endif

#
# X.509 Certificate handling
#
obj-$(CONFIG_$(SPL_)X509_CERTIFICATE_PARSER) += x509_key_parser.o
x509_key_parser-y := \
	x509.asn1.o \
	x509_akid.asn1.o \
	x509_cert_parser.o \
	x509_public_key.o

$(obj)/x509_cert_parser.o: \
	$(obj)/x509.asn1.h \
	$(obj)/x509_akid.asn1.h

$(obj)/x509.asn1.o: $(obj)/x509.asn1.c $(obj)/x509.asn1.h
$(obj)/x509_akid.asn1.o: $(obj)/x509_akid.asn1.c $(obj)/x509_akid.asn1.h

#
# PKCS#7 message handling
#
obj-$(CONFIG_$(SPL_)PKCS7_MESSAGE_PARSER) += pkcs7_message.o
pkcs7_message-y := \
	pkcs7.asn1.o \
	pkcs7_parser.o
obj-$(CONFIG_$(SPL_)PKCS7_VERIFY) += pkcs7_verify.o

$(obj)/pkcs7_parser.o: $(obj)/pkcs7.asn1.h
$(obj)/pkcs7.asn1.o: $(obj)/pkcs7.asn1.c $(obj)/pkcs7.asn1.h

#
# Signed PE binary-wrapped key handling
#
obj-$(CONFIG_$(SPL_)MSCODE_PARSER) += mscode.o

mscode-y := \
	mscode_parser.o \
	mscode.asn1.o

$(obj)/mscode_parser.o: $(obj)/mscode.asn1.h $(obj)/mscode.asn1.h
$(obj)/mscode.asn1.o: $(obj)/mscode.asn1.c $(obj)/mscode.asn1.h
