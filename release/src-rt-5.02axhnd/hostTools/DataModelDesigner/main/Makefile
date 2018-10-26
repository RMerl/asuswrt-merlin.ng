#***********************************************************************
#
#  Copyright (c) 2006  Broadcom Corporation
#  All Rights Reserved
#  No portions of this material may be reproduced in any form without the
#  written permission of:
#          Broadcom Corporation
#          16215 Alton Parkway
#          Irvine, California 92619
#  All information contained in this document is Broadcom Corporation
#  company private, proprietary, and trade secret.
#
#***********************************************************************/

OBJS = CmsLog.class MainClass.class

all: $(OBJS)

#
# Javac will automatically compile classes that it needs, so don't
# be alarmed if you only see one compile line
#
$(OBJS): %.class: %.java
	javac $(JFLAGS) $<

clean:
	rm -f *.class core.*


