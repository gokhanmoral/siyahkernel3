export ARCH=arm
export CROSS_COMPILE=/opt/toolchains/arm-eabi-4.4.3/bin/arm-eabi-
export USE_SEC_FIPS_MODE=true

make u1_defconfig
make