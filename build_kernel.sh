#!/bin/sh
export KERNELDIR=`readlink -f .`
export INITRAMFS_SOURCE=`readlink -f $KERNELDIR/../initramfs3`
export PARENT_DIR=`readlink -f ..`

if [ "${1}" != "" ];then
  export KERNELDIR=`readlink -f ${1}`
fi

INITRAMFS_TMP="/tmp/initramfs-source"

if [ ! -f $KERNELDIR/.config ];
then
  make siyah_defconfig
fi

. $KERNELDIR/.config

rm -rf $INITRAMFS_TMP
rm -rf $INITRAMFS_TMP.cpio
mkdir $INITRAMFS_TMP

export ARCH=arm
export CROSS_COMPILE=$PARENT_DIR/toolchain/bin/arm-eabi-

cd $KERNELDIR/
nice -n 10 make -j4 || exit 1

rm -rf $INITRAMFS_TMP
cp -ax $INITRAMFS_SOURCE $INITRAMFS_TMP
find $INITRAMFS_TMP -name .git -exec rm -rf {} \;
rm -rf $INITRAMFS_TMP/.hg
mkdir -p $INITRAMFS/lib/modules
find -name '*.ko' -exec cp -av {} $INITRAMFS_TMP/lib/modules/ \;

cd $INITRAMFS_TMP
find | fakeroot cpio -H newc -o > $INITRAMFS_TMP.cpio 2>/dev/null
ls -lh $INITRAMFS_TMP.cpio
cd -

nice -n 10 make -j3 zImage CONFIG_INITRAMFS_SOURCE="$INITRAMFS_TMP.cpio" || exit 1

cd ../payload
rm -f ../payload.cpio
find | fakeroot cpio -H newc -o > ../payload.cpio
cd -

cd ../recovery
rm -f ../recovery.cpio
rm -f ../recovery.cpio.xz
find | fakeroot cpio -H newc -o > ../recovery.cpio
cd -
xz -1 ../recovery.cpio

cp $KERNELDIR/arch/arm/boot/zImage zImage
#$KERNELDIR/mkshbootimg.py $KERNELDIR/zImage $KERNELDIR/arch/arm/boot/zImage $KERNELDIR/../payload.cpio $KERNELDIR/../recovery.cpio.xz

