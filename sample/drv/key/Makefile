# Project name

PROJ = ak_drv_key_sample

LIB_DEP = plat_drv plat_common plat_log plat_mem plat_thread plat_osal

LIBS = $(addprefix -l,$(LIB_DEP))

TARGET = ak_drv_key_sample

TOPDIR 	?= $(shell pwd)/../../..

# Flags

INCLUDE    =	-I. \
		-I../../../include_inner \
		-I../../../include \

#Locations

SRCDIR += .

CSRCS += $(foreach d, $(SRCDIR), $(wildcard $d/*.c))

COBJS += $(patsubst %.c, %.o, $(CSRCS))

LIB_DIR = $(TOPDIR)/lib

LD_FLAGS = $(LIBS) -L$(LIB_DIR)

#build rule
.PHONY: all target install clean

all: $(COBJS)
	$(CC) $(COBJS) $(LD_FLAGS) -o $(TARGET)
	$(STRIP) $(TARGET)

install:
	@cp -f $(TARGET) $(ROOTFS_BIN_DIR)

clean : 
	$(RM) $(COBJS) $(TARGET)


# Rules
include $(TOPDIR)/rules.mk
