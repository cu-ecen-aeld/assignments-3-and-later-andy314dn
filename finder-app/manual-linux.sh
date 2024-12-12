#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    printf "\033[0;32m TODO1 \033[0m\n"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    make -j12 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
    printf "\033[0;33m DONE TODO1 \033[0m\n"
fi

echo "Adding the Image in outdir"

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
printf "\033[0;32m TODO2 \033[0m\n"

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    printf "\033[0;32m TODO3 \033[0m\n"
else
    cd busybox
fi

# TODO: Make and install busybox
printf "\033[0;32m TODO4 \033[0m\n"

echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs
printf "\033[0;32m TODO5 \033[0m\n"

# TODO: Make device nodes
printf "\033[0;32m TODO6 \033[0m\n"

# TODO: Clean and build the writer utility
printf "\033[0;32m TODO7 \033[0m\n"

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
printf "\033[0;32m TODO8 \033[0m\n"

# TODO: Chown the root directory
printf "\033[0;32m TODO9 \033[0m\n"

# TODO: Create initramfs.cpio.gz
printf "\033[0;32m TODO10 \033[0m\n"
printf "\033[0;31m END \033[0m\n"
exit 0
