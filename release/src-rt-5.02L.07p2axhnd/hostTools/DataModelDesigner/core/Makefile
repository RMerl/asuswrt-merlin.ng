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
