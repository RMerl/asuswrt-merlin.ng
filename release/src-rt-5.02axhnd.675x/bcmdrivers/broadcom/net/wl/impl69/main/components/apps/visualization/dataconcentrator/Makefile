ifeq ($(or $(RTAX56U),$(RTAX58U)),y)
BCM_CHIP=63178
else
BCM_CHIP=4908
endif

all:
	@cp ../installbin/bin//vis-dcon_saved-$(BCM_CHIP)-arm vis-dcon

install:
	@install -D vis-dcon ../installbin/bin/vis-dcon

clean:
	@rm ../installbin/bin/vis-dcon

install-strip: install

