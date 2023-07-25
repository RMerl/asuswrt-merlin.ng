LIB:=OpenSC-0.22.0
all install: conditional_build


ifeq ($(strip $(BCM_OPTEE)),y)
conditional_build: opensc
clean: opensc_clean
else
conditional_build clean: ;
endif


opensc:
	$(MAKE) -f Makefile install
	@cp -a $(LIB)/src/pkcs11/.libs/opensc-pkcs11.so $(INSTALL_DIR)/lib/
	@cp -a $(LIB)/src/libopensc/.libs/libopensc.so $(INSTALL_DIR)/lib/
	@cp -a $(LIB)/src/libopensc/.libs/libopensc.so.8 $(INSTALL_DIR)/lib/
	@cp -a $(LIB)/src/libopensc/.libs/libopensc.so.8.0.0 $(INSTALL_DIR)/lib/
	@cp $(LIB)/src/tools/.libs/pkcs11-tool $(INSTALL_DIR)/bin/

opensc_clean:
	$(MAKE) -f Makefile clean
