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


