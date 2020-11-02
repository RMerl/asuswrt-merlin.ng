ifeq ($(or $(RTAX56U),$(RTAX58U)),y)
BCM_CHIP=63178
else
BCM_CHIP=4908
endif

all:
	@cp ../installbin/bin//vis-datacollector_saved-$(BCM_CHIP)-arm vis-datacollector

install:
	@install -D vis-datacollector ../installbin/bin/vis-datacollector

clean:
	@rm ../installbin/bin/vis-datacollector

install-strip: install
