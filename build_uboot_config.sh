################################################################################
#
#  build_kernel_config.sh
#
#  Copyright (c) 2025 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
################################################################################

UBOOT_SUBPATH="bootable/bootloader/uboot-mtk/t61p/u-boot-2019.04"
UBOOT_DEFCONFIG_NAME="MT5896_defconfig"
TOOLCHAIN_REPO="https://releases.linaro.org/components/toolchain/binaries/7.2-2017.11/aarch64-linux-gnu/gcc-linaro-7.2.1-2017.11-x86_64_aarch64-linux-gnu.tar.xz"
TOOLCHAIN_NAME="gcc-linaro-7.2.1-2017.11-x86_64_aarch64-linux-gnu"
TOOLCHAIN_PREFIX="aarch64-linux-gnu-"
PARALLEL_EXECUTION="-j16"

# Expected image files are seperated with ":"
UBOOT_IMAGES="u-boot.bin:u-boot-dtb.bin:u-boot-nodtb.bin:u-boot.bin.signature.bin:chunk_header.bin:chunk_header.bin.signature.bin:uboot_version.bin"

