all: json
include ../rules.include

OBJS += $(SRC)/utils/base64.o
OBJS += $(SRC)/utils/common.o
OBJS += $(SRC)/utils/json.o
OBJS += $(SRC)/utils/os_unix.o
OBJS += $(SRC)/utils/wpa_debug.o
OBJS += $(SRC)/utils/wpabuf.o

json: json.o $(OBJS) $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $^ $(LIBS) $(ELIBS)

clean:
	$(MAKE) -C $(SRC) clean
	$(MAKE) -C $(WPAS_SRC) clean
	rm -f json *~ *.o *.d ../*~ ../*.o ../*.d

-include $(OBJS:%.o=%.d)
