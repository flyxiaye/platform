#!/bin/sh
# File:                update.sh
# Provides:
# Description:      update uImage&rootfs under /tmp|/mnt|...
# Author:            xc
CM_NORMAL=0
CM_BOLD=1
CM_UNDERLINED=4
CM_BLINK=5
CM_NEGATIVE=7

CB_BLACK=40
CB_RED=41
CB_GREEN=42
CB_YELLOW=43
CB_BLUE=44
CB_PURPLE=45
CB_CYAN=46
CB_WHITE=47

CF_BLACK=30
CF_RED=31
CF_GREEN=32
CF_YELLOW=33
CF_BLUE=34
CF_PURPLE=35
CF_CYAN=36
CF_WHITE=37

UPDATER="updater"

DIR_TMP="/tmp"
DIR_MNT="/mnt"
DIR_RESERVE="/tmp/reserve"

FILE_TAR_GZ="update.tar.gz"

FILE_KERNEL="uImage"
FILE_ROOT="root.sqsh4"
FILE_USR="usr.sqsh4"
FILE_ETC="usr.jffs2"
FILE_DTB="anyka_ev500.dtb"
FILE_MAC="mac.bin"
FILE_ENV="env.bin"
FILE_LOGO="anyka_logo.rgb"

PARTITION_KERNEL="KERNEL"
PARTITION_ROOT="ROOTFS"
PARTITION_USR="APP"
PARTITION_ETC="CONFIG"
PARTITION_DTB="DTB"
PARTITION_MAC="MAC"
PARTITION_ENV="ENV"
PARTITION_LOGO="LOGO"

FILE_PARTITION="partition.conf"
FILE_RAM="ram.conf"

ENV_SIZE_FILE=4096                                                              #env文件的最大字节限制
ENV_SIZE_PARTITION=4                                                            #env分区的最大限制
USE_RESERVE_MEM=1                                                               #默认使用内存保留分区升级方式

update_voice_tip()
{
    echo "play update voice tips"
    ccli misc --tips "/usr/share/anyka_update_device.mp3"
    sleep 3
}

check_files()
{
    echo "check update image"
    for target in ${FILE_KERNEL} ${FILE_ROOT} ${FILE_USR} ${FILE_ETC} ${FILE_DTB} ${FILE_MAC} ${FILE_ENV} ${FILE_LOGO}
    do
        if [ -e ${DIR_UPDATE}/${target} ]; then
            echo "############ find a target ${target}, update in ${DIR_UPDATE} ############"
            return 1
        fi
    done
    return 0
}

check_partition()
{
    if [ -e ${DIR_UPDATE}/$FILE_PARTITION ]; then
        echo "find $FILE_PARTITION in ${DIR_UPDATE}"
        return 1
    fi

    return 0
}

check_ram()
{
    if [ -e ${DIR_UPDATE}/$FILE_RAM ]; then
        echo "find $FILE_RAM in ${DIR_UPDATE}"
        return 1
    fi
    return 0
}

mount_reserve_dir()
{
    #insmod /usr/modules/ak_ramdisk.ko                                          #加载保留内存分区的驱动
    HEXDUMP_REG="hexdump /proc/device-tree/reserved-memory/dma_reserved@81400000/reg"
    HEXDUMP_SIZE="hexdump /proc/device-tree/reserved-memory/dma_reserved@81400000/size"
    HEX_RAMDISK_SIZE=2000000                                                    #最大的内存保留区间大小32MB,此处为16进制

    REG_B=`$HEXDUMP_REG | grep -Eo '( [0-9A-Fa-f]{4})+' | sed 's/ //g' | grep -Eo '^[0-9A-Fa-f]{2}'`
    REG_A=`$HEXDUMP_REG | grep -Eo '( [0-9A-Fa-f]{4})+' | sed 's/ //g' | sed 's/^[0-9A-Fa-f]\{2\}//g' | grep -Eo '^[0-9A-Fa-f]{2}'`
    REG_D=`$HEXDUMP_REG | grep -Eo '( [0-9A-Fa-f]{4})+' | sed 's/ //g' | sed 's/^[0-9A-Fa-f]\{4\}//g' | grep -Eo '^[0-9A-Fa-f]{2}'`
    REG_C=`$HEXDUMP_REG | grep -Eo '( [0-9A-Fa-f]{4})+' | sed 's/ //g' | sed 's/^[0-9A-Fa-f]\{6\}//g' | grep -Eo '^[0-9A-Fa-f]{2}'`

    SIZE_B=`$HEXDUMP_SIZE | grep -Eo '( [0-9A-Fa-f]{4})+' | sed 's/ //g' | grep -Eo '^[0-9A-Fa-f]{2}'`
    SIZE_A=`$HEXDUMP_SIZE | grep -Eo '( [0-9A-Fa-f]{4})+' | sed 's/ //g' | sed 's/^[0-9A-Fa-f]\{2\}//g' | grep -Eo '^[0-9A-Fa-f]{2}'`
    SIZE_D=`$HEXDUMP_SIZE | grep -Eo '( [0-9A-Fa-f]{4})+' | sed 's/ //g' | sed 's/^[0-9A-Fa-f]\{4\}//g' | grep -Eo '^[0-9A-Fa-f]{2}'`
    SIZE_C=`$HEXDUMP_SIZE | grep -Eo '( [0-9A-Fa-f]{4})+' | sed 's/ //g' | sed 's/^[0-9A-Fa-f]\{6\}//g' | grep -Eo '^[0-9A-Fa-f]{2}'`
    DEC_HEXDUMP_SIZE=`echo "obase=10;ibase=16;$SIZE_A$SIZE_B$SIZE_C$SIZE_D"|bc`
    DEC_RAMDISK_SIZE=`echo "obase=10;ibase=16;$HEX_RAMDISK_SIZE"|bc`

    if [ $DEC_HEXDUMP_SIZE -gt $DEC_RAMDISK_SIZE ]; then                        #大于最大的内存保留分区大小则mount默认值
        MOUNT_SIZE=$HEX_RAMDISK_SIZE
    else
        MOUNT_SIZE=$SIZE_A$SIZE_B$SIZE_C$SIZE_D
    fi

    INSMOD="insmod /usr/modules/ak_ramdisk.ko disk_addr=0x$REG_A$REG_B$REG_C$REG_D disk_size=0x$MOUNT_SIZE"
    echo $INSMOD
    $INSMOD
    mkfs.vfat /dev/reserved_ram0
    mkdir -p ${DIR_RESERVE}
    umount /dev/reserved_ram0
    mount /dev/reserved_ram0 ${DIR_RESERVE}
    df
}

move_tmp_dir_files()                                                            #将tmp目录的升级文件移动到reserve目录
{
    for target in ${FILE_KERNEL} ${FILE_ROOT} ${FILE_USR} ${FILE_ETC} ${FILE_DTB} ${FILE_MAC} ${FILE_ENV} ${FILE_LOGO} ${FILE_RAM} ${FILE_PARTITION} ${FILE_TAR_GZ}
    do
        if [ -e ${DIR_TMP}/${target} ]; then
            echo "MOVE ${target} => ${DIR_RESERVE}"
            mv ${DIR_TMP}/${target} ${DIR_RESERVE}/
            DIR_UPDATE=${DIR_RESERVE}                                           #重新设置升级目录
        fi
    done
    return 0
}

check_uncompress_update_file()                                                  #检查是否存在压缩文件
{
    echo "check ${FILE_TAR_GZ}"

    for dir in ${DIR_RESERVE} ${DIR_MNT} ${DIR_OPTION}
    do
        if [ -e ${dir}/${FILE_TAR_GZ} ]; then
            echo "tar -zvxf ${dir}/${FILE_TAR_GZ} -C ${DIR_RESERVE}"
            tar -zvxf ${dir}/${FILE_TAR_GZ} -C ${DIR_RESERVE}
            rm -f ${DIR_RESERVE}/${FILE_TAR_GZ}                                 #删除DIR_RESERVE目录的升级文件压缩包,如果存在的话
            DIR_UPDATE=${DIR_RESERVE}                                           #重新设置升级目录
            return 1                                                            #确定了升级目录
        fi
    done
    return 0
}

get_update_dir()
{
    for dir in ${DIR_TMP} ${DIR_MNT} ${DIR_OPTION}
    do
        for target in ${FILE_KERNEL} ${FILE_ROOT} ${FILE_USR} ${FILE_ETC} ${FILE_DTB} ${FILE_MAC} ${FILE_ENV} ${FILE_LOGO} ${FILE_RAM} ${FILE_PARTITION} ${FILE_TAR_GZ}
        do
            #echo "check ${dir}/${target}"
            if [ -e ${dir}/${target} ]; then
                DIR_UPDATE="${dir}"
                return 1
            fi
        done
    done
    return 0
}
                                                                                #
                                                                                # 升级脚本入口
                                                                                #
while getopts ":d:n" opt
do
    case $opt in
    d)
        DIR_OPTION=$OPTARG
        ;;
    n)
        USE_RESERVE_MEM=0                                                       #不使用内存保留分区升级方式
        DIR_RESERVE=$DIR_TMP
        ;;
    ?)
        echo "$0"
        echo "Option: -d [directory] 'Specify the updrade directory.(指定升级目录.)'"
        echo "Option: -n             'Upgrade without using reserved memory.(不使用内存保留分区方式升级.)'"
        echo "For example:"
        echo "$0"
        echo "$0 -d /mnt/nfs/update"
        echo "$0 -n"
        echo "$0 -d /mnt/nfs/update -n"
        exit 1;;
    esac
done

get_update_dir
if [ "$?" = 0 ];then                                                            #未有升级文件退出
    echo "############ NO UPDATE FILES #############"
    exit 0
fi

if [ $USE_RESERVE_MEM = 1 ];then
    echo "############ MOUNT #############"
    mount_reserve_dir                                                           #加载保留分区
    move_tmp_dir_files                                                          #将tmp目录的文件移动到reserve目录,如果存在的话
fi

check_uncompress_update_file                                                    #判断是否有压缩文件类型升级包,如果是则解压文件

check_ram                                                                       #判断是否进行内存参数升级工作
if [ "$?" = 1 ];then
    $UPDATER --use-binary-data --dir $DIR_UPDATE --addr 0x2c --conf $FILE_RAM
    updateram=1
fi

check_partition                                                                 #检测是否存在分区表文件
if [ "$?" = 0 ];then                                                            #没有找到分区表
    check_files
    if [ "$?" = 0 ];then                                                        #没有找到升级文件
        if [ $updateram = 1 ];then                                              #假如没有升级文件升级了内存参数也进行重启
            echo "############ update ram parameter, reboot now #############"
            sleep 1
            echo "reboot -f"
            reboot -f
        else
            echo -e "\e["$CM_NORMAL";"$CF_RED";"$CB_BLACK"m"" NO UPDATE FILES. ""\e[0m"
            exit 0
        fi
    else                                                                        #发现升级文件则使用linux分区表方式进行升级
        updatecmd="${UPDATER} \
                    --use-linux-table \
                    --dir ${DIR_UPDATE} \
                    --file-partition ${FILE_KERNEL},${PARTITION_KERNEL} \
                    --file-partition ${FILE_MAC},${PARTITION_MAC} \
                    --file-partition ${FILE_ENV},${PARTITION_ENV} \
                    --file-partition ${FILE_DTB},${PARTITION_DTB} \
                    --file-partition ${FILE_ROOT},${PARTITION_ROOT} \
                    --file-partition ${FILE_ETC},${PARTITION_ETC} \
                    --file-partition ${FILE_USR},${PARTITION_USR} \
                    --file-partition ${FILE_LOGO},${PARTITION_LOGO}"
    fi
else                                                                            #找到分区表使用分区表升级方式
    if [ -e ${DIR_UPDATE}/${FILE_ENV} ]; then
        env_size_file=`ls -l ${DIR_UPDATE}/${FILE_ENV}| awk '{ print $5 }'`     #获取${FILE_ENV}文件大小
        if [ $env_size_file -gt $ENV_SIZE_FILE ];then                           #env.bin大于4096则不能升级
            echo -e "\e["$CM_NORMAL";"$CF_RED";"$CB_BLACK"m"" ${DIR_UPDATE}/${FILE_ENV} is too big. ""\e[0m"
            exit 0
        fi
    fi

    env_size_partition=`cat ${DIR_UPDATE}/${FILE_PARTITION} | grep "^env.bin" | grep -Eo "ENV\s+[0-9]+" | grep -Eo "[0-9]+$"`
    if [ $env_size_partition -gt $ENV_SIZE_PARTITION ];then                     #ENV分区大于4k也不能升级
        echo -e "\e["$CM_NORMAL";"$CF_RED";"$CB_BLACK"m"" Partition ENV is too big. ""\e[0m"
        exit 0
    fi
    updatecmd="${UPDATER} --use-conf-table --dir ${DIR_UPDATE}"
fi

# play update vioce tip
# update_voice_tip                                                              #暂时停止播放声音

killall -15 syslogd
killall -15 klogd
killall -15 tcpsvd
killall -15 udhcpc

# send signal to stop watchdog
#killall -12 daemon
#sleep 1
# kill apps, MUST use force kill
#killall -9 daemon
#killall -9 anyka_ipc
#killall -9 net_manage.sh
#/usr/sbin/wifi_manage.sh stop
#killall -9 smartlink

# sleep to wait the program exit

#i=5
#while [ $i -gt 0 ]
#do
#    sleep 1
#
#    pid=`pgrep anyka_ipc`
#    if [ -z "$pid" ];then
#        echo "The main app anyka_ipc has exited !!!"
#        break
#    fi
#
#    i=`expr $i - 1`
#done
#
#if [ $i -eq 0 ];then
#    echo "The main app anyka_ipc is still run, we don't do update, reboot now !!!"
#    reboot
#fi

echo "############ please wait a moment. And don't remove TFcard or power-off #############"
#led blink
#/usr/sbin/led.sh blink 50 50
#export LD_LIBRARY_PATH='/tmp/rootfstmp/lib:/lib:/usr/lib'
#export PATH='/tmp/rootfstmp/bin:/tmp/rootfstmp/sbin:/bin:/sbin:/usr/bin:/usr/sbin'
#echo 3 >/proc/sys/vm/drop_caches
echo "dd if=/dev/root of=/dev/rootfstmp"                                        #创建一个临时的根目录分区
dd if=/dev/root of=/dev/rootfstmp

echo "mkdir -p /tmp/rootfstmp"
mkdir -p /tmp/rootfstmp

echo "umount /tmp/rootfstmp"
umount /tmp/rootfstmp

sleep 1

echo "mount /dev/rootfstmp /tmp/rootfstmp"
mount -t squashfs /dev/rootfstmp /tmp/rootfstmp

export LD_LIBRARY_PATH='/tmp/rootfstmp/lib'
export PATH='/tmp/rootfstmp/bin:/tmp/rootfstmp/sbin'
export
#echo "which updater"
#which updater
#echo "which reboot"
#which reboot
echo "############ START UPDATE. #############"

$updatecmd

if [ "$?" = 0 ];then
    echo "############ update success, reboot now #############"
    sleep 1
    echo "reboot -f"
    reboot -f
fi
if [ updateram = 1 ];then                                                       #假如没有升级文件升级了内存参数也进行重启
    echo "############ update ram parameter, reboot now #############"
    sleep 1
    echo "reboot -f"
    reboot -f
fi
