#!/bin/sh
module="aesdchar"
device=$module
mode="664"

if grep -q '^staff:' /etc/group; then
    group="staff"
else
    group="wheel"
fi

echo "Load our module, exit on failure"
modprobe $module.ko $* || exit 1
echo "Get the major number (allocated with allocate_chrdev_region) from /proc/devices"
major=$(awk "\$2==\"$module\" {print \$1}" /proc/devices)
if [ ! -z ${major} ]; then
    echo "Remove any existing /dev node for /dev/${device}"
    rm -f /dev/${device}
    echo "Add a node for our device at /dev/${device} using mknod"
    mknod /dev/${device} c $major 0
    echo "Change group owner to ${group}"
    chgrp $group /dev/${device}
    echo "Change access mode to ${mode}"
    chmod $mode  /dev/${device}
else
    echo "No device found in /proc/devices for driver ${module} (this driver may not allocate a device)"
fi
