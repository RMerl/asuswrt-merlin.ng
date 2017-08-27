#
# Edit the following for your setup
#
USER=apache
# CRONTAB not used
#CRONTAB=/var/spool/cron/tabs/$(USER)
RRDPATH=/usr/bin
BINPATH=/usr/local/bin
RRDDIR=/var/lib/sensors-rrd
APACHE=/var/www/html
APACHDIR=$(APACHE)/senspix
MACH=`uname -n`
SENSDEV=hwmon0
SENSDIR=/sys/class/hwmon/$(SENSDEV)/device
################################################
# Everything below here should be fine
#
RRDB=$(RRDDIR)/sensors.rrd

all: sens_day.cgi sens_week.cgi summ_week.cgi $(SENSDIR)

%.cgi : %.in Makefile
	sed -e "s#%%RRDPATH%%#$(RRDPATH)#g;s#%%APACHDIR%%#$(APACHDIR)#g;s#%%RRDDIR%%#$(RRDDIR)#g;s#%%MACH%%#$(MACH)#g" $< > $@

$(RRDDIR) :
	install -d -o $(USER) $(RRDDIR)

$(RRDB) : $(RRDDIR)
	./sens_create_rrd $(RRDB)
	chown $(USER) $(RRDB)
	
$(SENSDIR) :
	$(error error - sensor $(SENSDEV) not installed - check SENSDEV definition in Makefile)

$(CRONTAB) :
	$(error error - crontab for user $(USER) not present - check CRONTAB definition in Makefile)

$(APACHE) :
	$(error error - Web server directory $(APACHE) not present - check APACHE definition in Makefile)

$(APACHDIR)/pix : $(APACHE)
	install -d -o $(USER) -m 777 $(APACHDIR)/pix

install: all $(RRDB) $(SENSDIR) $(CRONTAB) $(APACHDIR)/pix
	install -m 755 sens_update_rrd $(BINPATH)
	install -m 755 sens_week.cgi $(APACHDIR)
	install -m 755 sens_day.cgi $(APACHDIR)
	install -m 755 summ_week.cgi $(APACHDIR)
#	grep sens_update_rrd $(CRONTAB) > /dev/null 2>&1 || echo '*/5 * * * *	/usr/local/bin/sens_update_rrd' $(RRDB) $(SENSDEV) >> $(CRONTAB)
	@echo
	@echo Note!!! You must manually install the following line in the crontab for user $(USER):
	@echo '   */5 * * * *    ' /usr/local/bin/sens_update_rrd $(RRDB) $(SENSDEV)

clean:
	rm -f sens_day.cgi sens_week.cgi summ_week.cgi
