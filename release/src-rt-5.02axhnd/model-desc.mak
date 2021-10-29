ifeq ($(ISPCTRL),y)

#ISP ChinaTel
ifeq ($(ELINK),y)
ISPCTRL_DESC := CN.CT.elink
else ifeq ($(TR181),y)
ISPCTRL_DESC :=	tr181
else ifeq ($(TR069),y)
ISPCTRL_DESC := tr098
endif
ifneq ($(CHINATEL),)
ifeq ($(ISPCTRL_DESC),)
ISPCTRL_DESC := CN.CT.$(CHINATEL)
else
ISPCTRL_DESC := $(ISPCTRL_DESC).$(CHINATEL)
endif
endif
#ISP ChinaTel

#Other ISPs
#Please place other ISP's ISPCTRL_DESC related code here.
#Ohter ISPs

ifneq ($(ISPCTRL_DESC),)
export ISPCTRL_POSTFIX := _$(ISPCTRL_DESC)
endif

endif

all:
ifneq ($(ISPCTRL_DESC),)
	@echo '#define RT_MODELDESC "{\"ISPCtrl\":\"$(ISPCTRL_DESC)\"}"' >> router/shared/version.h
endif
