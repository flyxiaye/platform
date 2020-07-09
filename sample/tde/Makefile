# Project name

PROJ = ak_tde_sample

LIB_DEP =  plat_common plat_mem plat_log plat_thread pthread plat_tde plat_osal plat_drv

LIBS = $(addprefix -l,$(LIB_DEP))

TARGET = $(PROJ)

TOPDIR 	?= $(shell pwd)/../..

# Flags

INCLUDE    =	-I. \
		-I../../include_inner \
		-I../../include \
		-I../include \
		-I../../src_linux/include

#Locations

SRCDIR += .

CSRCS += $(foreach d, $(SRCDIR), $(wildcard $d/*.c))

COBJS += $(patsubst %.c, %.o, $(CSRCS))

LIB_DIR = $(TOPDIR)/lib

LD_FLAGS = -Xlinker "-(" $(LIBS) -Xlinker "-)" -L$(LIB_DIR)

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
