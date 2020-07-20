# Project name

PROJ = test

LIB_DEP = plat_vi plat_vo plat_common plat_log plat_mem plat_thread pthread plat_tde m plat_osal plat_drv plat_vpss plat_isp_sdk mpi_venc akv_encode mpi_vdec akv_decode

LIBS = $(addprefix -l,$(LIB_DEP))

TARGET = $(PROJ)

TOPDIR 	?= $(shell pwd)/../..

# Flags

INCLUDE    =	-I. \
		-I../../include_inner \
		-I../../include \
		-I../include \


#Locations

SRCDIR += .

CSRCS += $(foreach d, $(SRCDIR), $(wildcard $d/*.c))

# CPPSRCS += $(foreach d, $(SRCDIR), $(wildcard $d/*.cpp))

COBJS += $(patsubst %.c, %.o, $(CSRCS))

# CPPOBJS += $(patsubst %.cpp, %.o, $(CSRCS))

LIB_DIR = $(TOPDIR)/lib

LD_FLAGS = $(LIBS) -L$(LIB_DIR)

#build rule
.PHONY: all target install clean

all: $(COBJS)
	$(CC) $(COBJS) $(LD_FLAGS) -o $(TARGET)
	# $(CXX) $(CPPOBJS) $(LD_FLAGS) -o $(TARGET)
	$(STRIP) $(TARGET)

install:
	@cp -f $(TARGET) $(ROOTFS_BIN_DIR)

clean : 
	$(RM) $(COBJS) $(TARGET)


# Rules
include $(TOPDIR)/rules.mk
