#  Module.mk
#  Copyright (c) 1998, 1999  Frodo Looijaard <frodol@dds.nl>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301 USA.

MODULE_DIR := prog/pwm
PROGPWMDIR := $(MODULE_DIR)

PROGPWMMAN8DIR := $(MANDIR)/man8
PROGPWMMAN8FILES := $(MODULE_DIR)/fancontrol.8 $(MODULE_DIR)/pwmconfig.8

PROGPWMTARGETS := $(MODULE_DIR)/fancontrol \
                  $(MODULE_DIR)/pwmconfig

# The vt1211_pwm script is not installed by default, pass VT1211_PWM=1
# to get it 
ifeq ($(VT1211_PWM),1)
PROGPWMMAN8FILES += $(MODULE_DIR)/vt1211_pwm.8
PROGPWMTARGETS += $(MODULE_DIR)/vt1211_pwm
endif

REMOVEPWMBIN := $(patsubst $(MODULE_DIR)/%,$(DESTDIR)$(SBINDIR)/%,$(PROGPWMTARGETS))
REMOVEPWMMAN := $(patsubst $(MODULE_DIR)/%,$(DESTDIR)$(PROGPWMMAN8DIR)/%,$(PROGPWMMAN8FILES))

install-prog-pwm: $(PROGPWMTARGETS)
	$(MKDIR) $(DESTDIR)$(SBINDIR) $(DESTDIR)$(PROGPWMMAN8DIR)
	$(INSTALL) -m 755 $(PROGPWMTARGETS) $(DESTDIR)$(SBINDIR)
	$(INSTALL) -m 644 $(PROGPWMMAN8FILES) $(DESTDIR)$(PROGPWMMAN8DIR)

user_install :: install-prog-pwm

user_uninstall::
	$(RM) $(REMOVEPWMBIN)
	$(RM) $(REMOVEPWMMAN)
