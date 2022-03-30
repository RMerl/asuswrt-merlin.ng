####
#### Gen3/2 Sec declarations/definitions  
#### Can only be included after make.common
####

# FLD/MFG demo credentials input; switches

CRED_DIR:=$(subst ",,$(BUILD_DIR)/$(SECURE_BOOT_KEY_DIR))

ifeq ($(strip $(SECURE_BOOT_PROCESS_FLD_OEM_COT)),y)

FLD_AES_EK:='$(CRED_DIR)/Kaes-fld-ek.bin'
FLD_AES_IV:='$(CRED_DIR)/Kaes-fld-iv.bin'
ifeq ($(SECURE_BOOT_ENCRYPT_BTLDRS),y)
FLD_ENC:='.fld.oem.enc'
OPT:=opt<enc>
endif

FLD_CRED_LIST:='rot<mid.bin Krot-fld-pub.bin>oem<fldOemData.sig Krsa-fld-pub.bin Kaes-fld.enc>dir<$(CRED_DIR)>$(OPT)'
FLD_KEY:='$(CRED_DIR)/Krsa-fld.pem'

else

FLD_AES_EK:='$(CRED_DIR)/Kroe-fld-ek.bin'
FLD_AES_IV:='$(CRED_DIR)/Kroe-fld-iv.bin'
ifeq ($(SECURE_BOOT_ENCRYPT_BTLDRS),y)
FLD_ENC:='.fld.enc'
OPT:=opt<enc>
endif

FLD_KEY:='$(CRED_DIR)/Krot-fld.pem'
FLD_CRED_LIST:='rot<mid.bin Krot-fld-pub.bin>dir<$(CRED_DIR)>$(OPT)'
endif

MFG_KEY:='$(CRED_DIR)/Krsa-mfg.pem'
MFG_AES_EK:='$(CRED_DIR)/Kaes-mfg-ek.bin'
MFG_AES_IV:='$(CRED_DIR)/Kaes-mfg-iv.bin'
ifeq ($(SECURE_BOOT_ENCRYPT_BTLDRS),y)
MFG_ENC:='.mfg.enc'
OPT:=opt<enc>
endif
ifeq ($(strip $(BRCM_CHIP)),4908) 
MFG_CRED_LIST:='oem<mfgOemData.sig mid.bin Krsa-mfg-pub.bin mid+Kaes-mfg.enc>roe<$(BRCM_CHIP)/mfgRoeData.sig Kroe2-mfg-pub.bin $(BRCM_CHIP)/Kroe2-mfg-priv.enc mid.bin>dir<$(CRED_DIR)>$(OPT)'
else
MFG_CRED_LIST:='oem<mfgOemData2.sig mid.bin Krsa-mfg-pub.bin mid+Kaes-mfg.enc>roe<$(BRCM_CHIP)/mfgRoeData.sig Kroe2-mfg-pub.bin $(BRCM_CHIP)/Kroe2-mfg-priv.enc mid.bin>dir<$(CRED_DIR)>$(OPT)'
endif

# turnkey definitions
ifeq ($(strip $(SECURE_BOOT_TURNKEY)),y)

export TK_FLD_IV:='$(CRED_DIR)/Kroe-fld-iv.bin' 
export TK_FLD_EK:='$(CRED_DIR)/Kroe-fld-ek.bin' 
export TK_FLD_HASH:='$(CRED_DIR)/Hmid-rot-fld-pub.bin'
MFG_AES_EK:='$(CRED_DIR)/Kaes-mfg-ek.bin'
MFG_AES_IV:='$(CRED_DIR)/Kaes-mfg-iv.bin'
export KEYSTORE_ARGS:=--args req=$(SECURE_BOOT_TK_MODE_REQ) --args arch=$(SECURE_BOOT_ARCH) --args abort_timeout=$(SECURE_BOOT_TK_ABORT_TIMEOUT) --args byteorder=$(ARCH_ENDIAN) --args chip=$(BRCM_CHIP)

endif


ifeq ($(strip $(SECURE_BOOT_TURNKEY)),y)
#Suffix to point to CFEROM generated with TK
SFX=_TK
endif

export MFG_KEY FLD_KEY MFG_AES_EK MFG_AES_IV FLD_AES_EK FLD_AES_IV FLD_CRED_LIST MFG_CRED_LIST
