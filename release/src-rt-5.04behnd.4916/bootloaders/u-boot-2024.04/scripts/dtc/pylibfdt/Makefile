# Unfortunately setup.py below cannot handle srctree being ".." which it often
# is. It fails with an error like:
# Fatal error: can't create build/temp.linux-x86_64-2.7/../lib/libfdt/fdt.o:
#    No such file or directory
# To fix this, use an absolute path.
LIBFDT_srcdir = $(abspath $(srctree)/$(src)/../libfdt)

include $(LIBFDT_srcdir)/Makefile.libfdt

# Unfortunately setup.py (or actually the Python distutil implementation) puts
# files into the same directory as the .i file. We cannot touch the source
# directory, so we "ship" .i file into the objtree.
PYLIBFDT_srcs = $(addprefix $(LIBFDT_srcdir)/,$(LIBFDT_SRCS)) \
		$(obj)/libfdt.i

# create a version string compliant with PEP 440
PEP_VERSION = $(VERSION)$(if $(PATCHLEVEL),.$(PATCHLEVEL)$(if $(SUBLEVEL),.$(SUBLEVEL)))$(subst -,,$(EXTRAVERSION))

quiet_cmd_pymod = PYMOD   $@
      cmd_pymod = unset CROSS_COMPILE; unset CFLAGS; \
		CC="$(HOSTCC)" LDSHARED="$(HOSTCC) -shared " \
		LDFLAGS="$(HOSTLDFLAGS)" \
		VERSION="$(PEP_VERSION)" \
		CPPFLAGS="$(HOSTCFLAGS) -I$(LIBFDT_srcdir)" OBJDIR=$(obj) \
		SOURCES="$(PYLIBFDT_srcs)" \
		SWIG_OPTS="-I$(LIBFDT_srcdir) -I$(LIBFDT_srcdir)/.." \
		$(PYTHON3) $< --quiet build_ext --inplace

rebuild: $(src)/setup.py $(PYLIBFDT_srcs)
	@# Remove the library since otherwise Python doesn't seem to regenerate
	@# the libfdt.py file if it is missing.
	@rm -f $(obj)/_libfdt*.so
	$(call if_changed,pymod)
	@# Rename the file to _libfdt.so so this Makefile doesn't run every time
	@if [ ! -e $(obj)/_libfdt.so ]; then \
		mv $(obj)/_libfdt*.so $(obj)/_libfdt.so; \
	fi

$(obj)/_libfdt.so $(obj)/libfdt.py &: rebuild
	@:

always += _libfdt.so libfdt.py

clean-files += libfdt.i _libfdt.so libfdt.py libfdt_wrap.c
