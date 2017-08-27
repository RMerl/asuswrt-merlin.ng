#***********************************************************************
#
#  Copyright (c) 2007  Broadcom Corporation
#  All Rights Reserved
#  No portions of this material may be reproduced in any form without the
#  written permission of:
#          Broadcom Corporation
#          5300 California Avenue
#          Irvine, California 92617
#  All information contained in this document is Broadcom Corporation
#  company private, proprietary, and trade secret.
#
#***********************************************************************/

OBJS = AddEditDialog.class AddEditObjectDialog.class AddEditParamDialog.class \
       Base64RestrictionPanel.class BooleanRestrictionPanel.class \
       HexBinaryRestrictionPanel.class LongRestrictionPanel.class UnsignedLongRestrictionPanel.class \
       DateTimeRestrictionPanel.class Desktop.class DesktopManager.class \
       IntRestrictionPanel.class PrimaryCommandPanel.class PrimaryInternalFrame.class \
       StringRestrictionPanel.class UnsignedIntRestrictionPanel.class \
       VsaCommandPanel.class VsaInternalFrame.class

all: $(OBJS)

#
# Javac will automatically compile classes that it needs, so don't
# be alarmed if you don't see one compile line from each class, it
# may have already been compiled as a side effect of compiling some
# other classes (even from other directories).
#
$(OBJS): %.class: %.java
	javac $(JFLAGS) $<


clean:
	rm -f *.class
