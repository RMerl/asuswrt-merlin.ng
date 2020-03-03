SHA_AUTH_ONLY?=0
BSPOBJS += bcm63xx_auth_if.o
ifneq ($(SHA_AUTH_ONLY),1)
BSPOBJS +=  bcm63xx_rsa.o bcm63xx_sha.o
endif
