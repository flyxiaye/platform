# rootfs global info

WORKDIR:=$(shell pwd)
include $(WORKDIR)/config.mk

ROOTFS_BIN_DIR?=$(WORKDIR)/rootfs/rootfs/usr/bin
ROOTFS_LIB_DIR?=$(WORKDIR)/rootfs/rootfs/usr/lib
ROOTFS_SBIN_DIR?=$(WORKDIR)/rootfs/rootfs/usr/sbin

MAKE:=make

export WORKDIR ROOTFS_BIN_DIR ROOTFS_LIB_DIR ROOTFS_SBIN_DIR MAKE

LIB_DIR:=$(WORKDIR)/lib
INC_DIR:=$(WORKDIR)/include
INC_INNER_DIR:=$(WORKDIR)/include_inner

export LIB_DIR INC_DIR INC_INNER_DIR

# rootfs dir MUST be placed at first

dir-y := rootfs
dir-y += sample

all:
	@for i in $(dir-y); \
	do \
		$(MAKE) -C $$i; \
		echo ""; \
		if [ $$? -ne 0 ]; then exit 1; fi \
	done

install:
	@for i in $(dir-y); \
	do \
		$(MAKE) -C $$i install; \
		echo ""; \
	done
	@cp -r lib/*.so $(ROOTFS_LIB_DIR)

image:
	@for i in $(dir-y); \
	do \
		$(MAKE) -C $$i image; \
	done
ifeq ($(CONFIG_FLASH_TYPE), 0)
	@cp rootfs/usr.* ../tools/burntool
	@cp rootfs/root.s* ../tools/burntool
else
	@cp rootfs/*.yaffs2 ../tools/burntool
endif

clean:
	@for i in $(dir-y); \
	do \
		$(MAKE) -C $$i clean; \
		echo ""; \
	done

.PHONY: all install image clean
