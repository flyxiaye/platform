#! /bin/sh
### BEGIN INIT INFO
# File:				wifi_install.sh
# Provides:         driver install/uninstall, operate as station/ap 
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description: wifi_install.sh
# Author:			
# Email: 			
# Date:				2019-10-31
### END INIT INFO

PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin
DEVICE=$1
MODE=$2


usage()
{
	echo "Usage: $0 DEVICE MODE"
	echo "       DEVICE: rtl8188 | rtl8189 | atbm603x | none"
	echo "       MODE:	ap | station"
}

wifi_stop()
{
	killall wpa_supplicant
	killall udhcpc
	killall udhcpd
	killall hostapd
	rmmod rtl8188ftv
	rmmod rtl8189ftv
	rmmod ak_hcd
	rmmod atbm603x_wifi_HT40_usb
	rmmod mac80211
	rmmod cfg80211
}

wifi_start()
{
case "$MODE" in
	ap)
		ifconfig wlan0  172.14.10.1
		mkdir -p /var/lib/misc/
		touch /var/lib/misc/udhcpd.leases
		udhcpd /etc/jffs2/udhcpd.conf -S
		hostapd /etc/jffs2/hostapd.conf -B
		;;
	station)
		wpa_supplicant -Dnl80211 -i wlan0 -c /etc/jffs2/wpa_supplicant.conf -B
		sleep 1
		udhcpc -i wlan0
		;;
esac
}

####### main
case "$DEVICE" in
	rtl8188)
		wifi_stop
		insmod  /usr/modules/cfg80211.ko
		insmod  /usr/modules/mac80211.ko
		insmod  /usr/modules/ak_hcd.ko
		insmod 	/usr/modules/rtl8188ftv.ko
		sleep 2
		wifi_start
		;;	
	rtl8189)
		wifi_stop
		insmod /usr/modules/cfg80211.ko
		insmod /usr/modules/mac80211.ko
		insmod /usr/modules/rtl8189ftv.ko
		sleep 2
		wifi_start
		;;
	atbm603x)
		wifi_stop
		insmod /usr/modules/cfg80211.ko
		insmod /usr/modules/mac80211.ko
		insmod /usr/modules/ak_hcd.ko
		insmod /usr/modules/atbm603x_wifi_HT40_usb.ko
		sleep 2
		wifi_start
		;;
	none)
		wifi_stop
		;;
	*)
		usage
		;;
esac
exit 0


