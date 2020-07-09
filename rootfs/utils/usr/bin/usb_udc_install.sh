#!/bin/sh

insmod /usr/modules/ak_udc.ko
insmod /usr/modules/configfs.ko
insmod /usr/modules/libcomposite.ko
insmod /usr/modules/usb_f_mass_storage.ko
insmod /usr/modules/g_mass_storage.ko file=/dev/mmcblk0p1 stall=0 removable=1
