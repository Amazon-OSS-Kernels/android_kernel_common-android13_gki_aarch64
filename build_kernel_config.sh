################################################################################
#
#  build_kernel_config.sh
#
#  Copyright (c) 2024 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
################################################################################

REPO_URL="https://storage.googleapis.com/git-repo-downloads"
REPO_GPG_KEY="8BB9AD793E8E6153AF0F9A4416530D5E920F5C65"
KERNEL_MANIFEST="https://android.googlesource.com/kernel/manifest"
GKI_CONFIG="build.config.gki.aarch64"
KERNEL_BRANCH="common-android13-5.15"
PARALLEL_EXECUTION="-j8"

# Expected image files are seperated with ":"
KERNEL_IMAGES="dist/Image:dist/Image.gz:dist/Image.lz4:dist/vmlinux"
