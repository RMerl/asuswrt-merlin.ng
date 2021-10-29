all: $(if $(wildcard prebuilt/acsd2),prebuilt/acsd2,acsd2) $(if $(wildcard prebuilt/acs_cli2),prebuilt/acs_cli2,acs_cli2)

install: all

clean:
	rm -f *.o *.a *.so acsd2 acs_cli2
