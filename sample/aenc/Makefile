# Project name

PROJ = ak_aenc_sample


LIB_DEP := mpi_aenc plat_ai plat_mem plat_thread plat_log  plat_common plat_osal plat_drv
LIB_DEP += akaudiofilter akaudiocodec

ifeq ($(OS_SUPPORT), 1)
    TARGET = libak_aenc_sample.a
else
    TARGET = ak_aenc_sample
    LIB_DEP += pthread
endif

LIBS = $(addprefix -l, $(LIB_DEP))

TOPDIR 	?= $(shell pwd)/../..

# Flags
INCLUDE    =	-I. \
		-I../../include_inner \
		-I../include \
		-I../../include \
		-I../../src/include \

#Locations

SRCDIR += .

CSRCS += $(foreach d, $(SRCDIR), $(wildcard $d/*.c))

COBJS += $(patsubst %.c, %.o, $(CSRCS))

LIB_DIR = $(TOPDIR)/lib
RTSP_LIB_DIR := $(TOPDIR)/lib/rtsp_lib

LD_FLAGS = -Xlinker "-(" $(LIBS) -Xlinker "-)" -L$(LIB_DIR) -L$(RTSP_LIB_DIR)

#build rule
.PHONY: all target install clean

all: $(COBJS)
ifeq ($(OS_SUPPORT), 1)
	$(AR) -rsv $(TARGET)  $(COBJS)
	@cp -r $(TARGET) $(TOPDIR)/lib
else
	$(CC) $(COBJS) $(LD_FLAGS) -o $(TARGET)
	$(STRIP) $(TARGET)
endif

install:
	@cp -f $(TARGET) $(ROOTFS_BIN_DIR)

clean : 
	$(RM) $(COBJS) $(TARGET)


# Rules
include $(TOPDIR)/rules.mk
