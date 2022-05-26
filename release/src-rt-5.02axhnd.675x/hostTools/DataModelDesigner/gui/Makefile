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
