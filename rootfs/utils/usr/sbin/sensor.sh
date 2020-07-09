#!/bin/sh

pr2000_install() 
{ 
    echo "install pr2000 driver"
	/sbin/insmod /usr/modules/ak_isp.ko
	/sbin/insmod /usr/modules/sensor_pr2000.ko
	if test ! -f /etc/jffs2/isp_pr2000_dvp.conf ; then
	    rm /etc/jffs2/isp_*.conf
	    cp /etc/isp_pr2000_dvp.conf /etc/jffs2/
	fi
}

pr2000_uninstall()
{
    echo "uninstall pr2000 driver"
	/sbin/rmmod ak_isp.ko
	/sbin/rmmod sensor_pr2000.ko
}

sc4236_install()
{
    echo "install sc4236 driver"
	/sbin/insmod /usr/modules/ak_isp.ko
	/sbin/insmod /usr/modules/sensor_sc4236.ko
	if test ! -f /etc/jffs2/isp_sc4236_mipi_2lane.conf ; then
	    rm /etc/jffs2/isp_*.conf
	    cp /etc/isp_sc4236_mipi_2lane.conf /etc/jffs2/
	fi
}

sc4236_uninstall()
{
    echo "uninstall sc4236 driver"
	/sbin/rmmod ak_isp.ko
	/sbin/rmmod sensor_sc4236.ko
}

ar0230_install()
{
    echo "install ar0230 driver"
	/sbin/insmod /usr/modules/ak_isp.ko
	/sbin/insmod /usr/modules/sensor_ar0230.ko
	if test ! -f /etc/jffs2/isp_ar0230_dvp.conf ; then
	    rm /etc/jffs2/isp_*.conf
	    cp /etc/isp_ar0230_dvp.conf /etc/jffs2/
	fi
}

ar0230_uninstall()
{
    echo "uninstall ar0230 driver"
	/sbin/rmmod ak_isp.ko
	/sbin/rmmod sensor_ar0230.ko
}

usage()
{
    echo "usage : ./sensor.sh <operation> <sensor_type>" 
	echo "    install driver   : ./sensor.sh install pr2000"
    echo "    uninstall driver : ./sensor.sh uninstall pr2000"
}

if test $# -ne 2; then
	usage
	exit
fi

echo "Parameter 1 : "$1
echo "Parameter 2 : "$2

if test $1 != "install" -a $1 != "uninstall" ; then
	usage
	exit
fi

if test $2 != "pr2000" -a $2 != "sc4236" -a $2 != "ar0230" ; then
	usage
	exit
fi

$2_$1
