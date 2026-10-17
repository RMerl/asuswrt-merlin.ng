include $(PROJECT_DIR)/make.proj_flags
include $(TOP_DIR)/make.common

CPP_SRCS := $(notdir $(shell ls *.cpp))
CPP_OBJS := $(CPP_SRCS:.cpp=.o)
C_SRCS := $(notdir $(shell ls *.c))
C_OBJS := $(C_SRCS:.c=.o)

OBJS := $(CPP_OBJS) $(C_OBJS)

CFLAGS = $(COMMON_CFLAGS) $(PROJ_CFLAGS) -DRDP_SIM
CPPFLAGS = $(COMMON_CFLAGS) $(PROJ_CFLAGS) -DRDP_SIM
LIB = $(PROJ_RDD_TARGET)/librdd.a

ifeq ("$(CONFIG_SYSTEM)", "sim")
	CFLAGS += -g -std=gnu99 $(HOST_PLATFORM_FLAGS)
	CPPFLAGS += -g -std=gnu++11 -DNO_SYSC
	LFLAGS = $(HOST_PLATFORM_FLAGS) -g
	ARLAGS = -s
endif

all: lib
	echo "Done building RDD lib"

validate:
	$(VALIDATE_FILES) rdd_*.[c,h]

lib: validate $(OBJS)
	$(AR) $(LIB) $(OBJS)

$(C_OBJS): %.o : %.c
	ls $<
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS)

$(CPP_OBJS): %.o : %.cpp
	ls $<
	$(CC) $(CPPFLAGS) $< -o $@ $(LFLAGS)

clean:
	rm -rf $(LIB) $(OBJS)
