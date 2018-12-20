all:
	# Do nothing

%.obj: %.bin
	@echo " [rc - $(BUILD_NAME)] LD $@"
	@$(LD) -r -b binary -o $@ $<

install:
	# Do nothing

clean:
	$(RM) *.obj
