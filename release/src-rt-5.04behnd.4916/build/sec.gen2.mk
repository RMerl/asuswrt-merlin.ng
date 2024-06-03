####
#### Gen2 Sec declarations/definitions  
#### Can only be included after make.common
####

# OP(FLD)/MFG demo credentials input; switches

CRED_DIR:=$(subst ",,$(BUILD_DIR)/$(SECURE_BOOT_KEY_DIR))

# OP mode in GEN2 described below as FLD 



#ifeq ($(SECURE_BOOT_ENCRYPT_BTLDRS),y)
MFG_AES_EK:='$(CRED_DIR)/mfg.ek.bin'
MFG_AES_IV:='$(CRED_DIR)/mfg.iv.bin'
MFG_ENC:='.mfg.enc'
OP_AES_EK:='$(CRED_DIR)/op.ek.bin'
OP_AES_IV:='$(CRED_DIR)/op.iv.bin'
OP_ENC:='.op.enc'
#endif

# IN OP mode  an MFG rsa key is used to sign images  
# of the BL1 
OP_KEY:='$(CRED_DIR)/mfg.pem'
# Due to GEN2 brom implementations both MFG and OP credentials are packed togehter
# In this case we use copy of the the same; so that an SBI would never be seen as both mfg/op signed/encrypted  
OP_CRED_LIST:='cot<$(BRCM_CHIP)/mfg.cot.bin $(BRCM_CHIP)/mfg.cot.sig $(BRCM_CHIP)/op.cot.bin $(BRCM_CHIP)/op.cot.sig>dir<$(CRED_DIR)>$(OPT)'

MFG_KEY:='$(CRED_DIR)/mfg.pem'
MFG_CRED_LIST:='cot<$(BRCM_CHIP)/mfg.cot.bin $(BRCM_CHIP)/mfg.cot.sig $(BRCM_CHIP)/mfg.cot.bin $(BRCM_CHIP)/mfg.cot.sig>dir<$(CRED_DIR)>$(OPT)'

# turnkey definitions
ifeq ($(strip $(SECURE_BOOT_TURNKEY)),y)

export KEYSTORE_ARGS:=--args req=$(SECURE_BOOT_TK_MODE_REQ) --args arch=$(SECURE_BOOT_ARCH) --args abort_timeout=$(SECURE_BOOT_TK_ABORT_TIMEOUT) --args byteorder=$(ARCH_ENDIAN) --args chip=$(BRCM_CHIP)

endif


ifeq ($(strip $(SECURE_BOOT_TURNKEY)),y)
#Suffix to point to CFEROM generated with TK
SFX=_TK
endif
export MFG_KEY OP_KEY MFG_AES_EK MFG_AES_IV OP_AES_EK OP_AES_IV OP_CRED_LIST MFG_CRED_LIST

