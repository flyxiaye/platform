# Project name

PROJ = ak_venc_sample

LIB_DEP = plat_common  plat_mem plat_thread pthread  plat_log plat_osal mpi_venc akv_encode plat_drv plat_vi plat_vpss plat_isp_sdk

LIBS = $(addprefix -l, $(LIB_DEP))

TARGET = ak_venc_sample

ifeq ($(OS_SUPPORT), 1)
    TARGET = libak_venc_sample.a
    LIB_DEP = plat_common plat_mem plat_pthread plat_log mpi_venc akv_encode plat_drv plat_osal plat_vi plat_vpss plat_isp_sdk
else
    TARGET = ak_venc_sample
    LIB_DEP = plat_common  plat_mem plat_thread pthread  plat_log plat_osal mpi_venc akv_encode plat_drv plat_vi plat_vpss plat_isp_sdk
endif

TOPDIR 	?= $(shell pwd)/../..

# Flags

INCLUDE    =	-I. \
		-I../../include_inner \
		-I../../include \
		-I../include \

#Locations

SRCDIR += .

CSRCS += $(foreach d, $(SRCDIR), $(wildcard $d/*.c))

COBJS += $(patsubst %.c, %.o, $(CSRCS))

LIB_DIR = $(TOPDIR)/lib

LD_FLAGS = -Xlinker "-(" $(LIBS) -Xlinker "-)"  -L$(LIB_DIR)

#build rule
.PHONY: all target install clean

all: $(COBJS)
ifeq ($(OS_SUPPORT), 1)
	$(AR) -rsv $(TARGET) $(COBJS)
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
