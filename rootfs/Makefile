BUILD_JFFS2:=./create_jffs2fs.sh
BUILD_SQUASHFS:=./create_squashfs.sh
BUILD_YAFFS:=./create_yaffs2fs.sh
EXTRACT_FS:=./extract.sh

all:
	@echo ""

install :
	@$(EXTRACT_FS) 
	@rm -rf rootfs/usr/modules/*
	@cp -r ko/* rootfs/usr/modules/
ifeq ($(CONFIG_FLASH_TYPE), 0)
	@cp ./scripts/rc.local.nor rootfs/etc/init.d/rc.local
	@cp ./scripts/update.nor.sh rootfs/sbin/update.sh
else
	@cp ./scripts/rc.local.nand rootfs/etc/init.d/rc.local
	@cp ./scripts/update.nand.sh rootfs/sbin/update.sh
endif
ifeq ($(CONFIG_SCREEN_SUPPORT), 0)
	sed -i 's/ts_gsl/ts_icn85xx/' utils/usr/sbin/screen.sh
else
	sed -i 's/ts_icn85xx/ts_gsl/' utils/usr/sbin/screen.sh
endif
ifeq ($(CONFIG_WIFI_DEVICE), 0)
	@rm -rf rootfs/usr/modules/rtl8189ftv.ko
	@rm -rf rootfs/usr/modules/rtl8189ftv.ko
	@rm -rf rootfs/usr/modules/atbm603x_wifi_HT40_usb.ko
endif

ifeq ($(CONFIG_WIFI_DEVICE), 1)
	@rm -rf rootfs/usr/modules/rtl8189ftv.ko
	@rm -rf rootfs/usr/modules/atbm603x_wifi_HT40_usb.ko
	@cp -rf ./wifi/* rootfs/
endif
ifeq ($(CONFIG_WIFI_DEVICE), 2)
	@rm -rf rootfs/usr/modules/rtl8188ftv.ko
	@rm -rf rootfs/usr/modules/atbm603x_wifi_HT40_usb.ko
	@cp -rf ./wifi/* rootfs/
endif
ifeq ($(CONFIG_WIFI_DEVICE), 3)
	@rm -rf rootfs/usr/modules/rtl8188ftv.ko
	@rm -rf rootfs/usr/modules/rtl8189ftv.ko
	@cp -rf ./wifi/* rootfs/
endif
	@cp -rf utils/* rootfs/

image :
ifeq ($(CONFIG_FLASH_TYPE), 0)
	@$(BUILD_JFFS2)
	@$(BUILD_SQUASHFS)
else
	@$(BUILD_YAFFS)
endif

clean :
	@rm -rf usr.jffs2 root.sqsh4 usr.sqsh4 rootfs/
