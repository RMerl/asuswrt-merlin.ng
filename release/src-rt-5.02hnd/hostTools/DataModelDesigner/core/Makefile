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

OBJS = AllProfiles.class AllValidStringArrays.class NodeData.class \
       ObjectNodeData.class ParameterNodeData.class SaxCallbackBuilder.class \
       TreeSerializer.class ValidStringArray.class \
       TreeCrossReferencer.class

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
