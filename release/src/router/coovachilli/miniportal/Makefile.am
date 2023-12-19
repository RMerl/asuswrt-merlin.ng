
chi_files=\
contact.chi login.chi map.chi register.chi terms.chi tos.chi payment.chi \
error.chi status.chi service.chi

tmpl_files=\
contact_form.tmpl    email_visitor.tmpl  login_map.tmpl       register_form.tmpl \
contact_thanks.tmpl  footer.tmpl         login_register.tmpl  register.tmpl \
contact.tmpl         header.tmpl         login_success.tmpl   terms_form.tmpl \
css.tmpl             js.tmpl             login.tmpl           terms.tmpl \
email_contact.tmpl   login_footer.tmpl   map.tmpl             title.tmpl \
email_owner.tmpl     login_form.tmpl     openid_form.tmpl error.tmpl

sh_files=config-local.sh functions.sh payment.sh uam.sh wispr.sh

wwwSCRIPTFILES = $(chi_files) $(sh_files)
wwwDATAFILES = $(tmpl_files)

chillidir = $(sysconfdir)/chilli
wwwdir = $(chillidir)/www

www_SCRIPTS = $(wwwSCRIPTFILES) 
www_DATA = $(wwwDATAFILES) 

EXTRA_DIST = $(wwwDATAFILES) $(chi_files:=.in) $(sh_files:=.in)

edit = sed \
	-e 's,@SBINDIR\@,$(sbindir),g' \
	-e 's,@INITDIR\@,$(initrddir),g' \
	-e 's,@VARTMP\@,$(localstatedir)/tmp,g' \
	-e 's,@VARRUN\@,$(localstatedir)/run,g' \
	-e 's,@ETCCHILLI\@,$(sysconfdir)/chilli,g' \
	-e 's,@SYSCONFDIR\@,$(sysconfdir),g' \
	-e 's,@PREFIX\@,$(prefix),g'

contact.chi: Makefile contact.chi.in
	rm -f contact.chi contact.chi.tmp
	$(edit) contact.chi.in >contact.chi.tmp
	mv contact.chi.tmp contact.chi

login.chi: Makefile login.chi.in
	rm -f login.chi login.chi.tmp
	$(edit) login.chi.in >login.chi.tmp
	mv login.chi.tmp login.chi

payment.chi: Makefile payment.chi.in
	rm -f payment.chi payment.chi.tmp
	$(edit) payment.chi.in >payment.chi.tmp
	mv payment.chi.tmp payment.chi

map.chi: Makefile map.chi.in
	rm -f map.chi map.chi.tmp
	$(edit) map.chi.in >map.chi.tmp
	mv map.chi.tmp map.chi

register.chi: Makefile register.chi.in
	rm -f register.chi register.chi.tmp
	$(edit) register.chi.in >register.chi.tmp
	mv register.chi.tmp register.chi

terms.chi: Makefile terms.chi.in
	rm -f terms.chi terms.chi.tmp
	$(edit) terms.chi.in >terms.chi.tmp
	mv terms.chi.tmp terms.chi

status.chi: Makefile status.chi.in
	rm -f status.chi status.chi.tmp
	$(edit) status.chi.in >status.chi.tmp
	mv status.chi.tmp status.chi

service.chi: Makefile service.chi.in
	rm -f service.chi service.chi.tmp
	$(edit) service.chi.in >service.chi.tmp
	mv service.chi.tmp service.chi

tos.chi: Makefile tos.chi.in
	rm -f tos.chi tos.chi.tmp
	$(edit) tos.chi.in >tos.chi.tmp
	mv tos.chi.tmp tos.chi

error.chi: Makefile error.chi.in
	rm -f error.chi error.chi.tmp
	$(edit) error.chi.in >error.chi.tmp
	mv error.chi.tmp error.chi

functions.sh: Makefile functions.sh.in
	rm -f functions.sh functions.sh.tmp
	$(edit) functions.sh.in >functions.sh.tmp
	mv functions.sh.tmp functions.sh

uam.sh: Makefile uam.sh.in
	rm -f uam.sh uam.sh.tmp
	$(edit) uam.sh.in >uam.sh.tmp
	mv uam.sh.tmp uam.sh

wispr.sh: Makefile wispr.sh.in
	rm -f wispr.sh wispr.sh.tmp
	$(edit) wispr.sh.in >wispr.sh.tmp
	mv wispr.sh.tmp wispr.sh

payment.sh: Makefile payment.sh.in
	rm -f payment.sh payment.sh.tmp
	$(edit) payment.sh.in >payment.sh.tmp
	mv payment.sh.tmp payment.sh

config-local.sh: Makefile config-local.sh.in
	rm -f config-local.sh config-local.sh.tmp
	$(edit) config-local.sh.in >config-local.sh.tmp
	mv config-local.sh.tmp config-local.sh

