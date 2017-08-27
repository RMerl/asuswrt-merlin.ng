sbin_SCRIPTS = alsaconf
man_MANS = alsaconf.8 alsaconf.fr.8
EXTRA_DIST = $(man_MANS)
SUBDIRS = po

install-man8:
	@for i in $(man_MANS); do \
	  ext=`echo $$i | sed -e 's/^.*\\.//'`; \
	  inst=`echo $$i | sed -e 's/\\.[^.]*$$//'`; \
	  case $$inst in \
	  *.[a-za-z]*)\
	    loc=`echo $$inst | sed -e 's/^.*\\.//'`; \
	    loc="/$$loc"; \
	    inst=`echo $$inst | sed -e 's/\\.[^.]*$$//'`;; \
	  *)\
	    loc="";; \
	  esac; \
	  $(mkinstalldirs) $(DESTDIR)$(mandir)$$loc/man$$ext; \
	  echo " $(INSTALL_DATA) $$i $(DESTDIR)$(mandir)$$loc/man$$ext/$$inst.$$ext"; \
	  $(INSTALL_DATA) $(srcdir)/$$i $(DESTDIR)$(mandir)$$loc/man$$ext/$$inst.$$ext; \
	done

uninstall-man8:
	@for i in $(man_MANS); do \
	  ext=`echo $$i | sed -e 's/^.*\\.//'`; \
	  inst=`echo $$i | sed -e 's/\\.[^.]*$$//'`; \
	  case $$inst in \
	  *.[a-za-z]*)\
	    loc=`echo $$inst | sed -e 's/^.*\\.//'`; \
	    loc="/$$loc"; \
	    inst=`echo $$inst | sed -e 's/\\.[^.]*$$//'`;; \
	  *)\
	    loc="";; \
	  esac; \
	  echo " rm -f $(DESTDIR)$(mandir)$$loc/man$$ext/$$inst.$$ext"; \
	  rm -f $(DESTDIR)$(mandir)$$loc/man$$ext/$$inst.$$ext; \
	done
