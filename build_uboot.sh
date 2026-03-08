#!/bin/bash
################################################################################
#
#  build_uboot.sh
#
#  Copyright (c) 2024 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
################################################################################

################################################################################
#
#  I N P U T
#
################################################################################
# Folder for platform tarball.
PLATFORM_TARBALL="${1}"

# Target directory for output artifacts.
TARGET_DIR="${2}/uboot"

################################################################################
#
#  V A R I A B L E S
#
################################################################################

# Retrieve the directory where the script is currently held
SCRIPT_BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Configuration file for the build.
CONFIG_FILE="${SCRIPT_BASE_DIR}/build_uboot_config.sh"

# Workspace directory & relevant temp folders.
if [ -d "${PLATFORM_TARBALL}" ]; then
    WORKSPACE_DIR="$(cd "${PLATFORM_TARBALL}" && pwd)"
else
    WORKSPACE_DIR="$(mktemp -d)"
fi

TOOLCHAIN_DIR="${WORKSPACE_DIR}/toolchain"
PLATFORM_EXTRACT_DIR="${WORKSPACE_DIR}/src"

mkdir -vp "${TOOLCHAIN_DIR}" "${PLATFORM_EXTRACT_DIR}"

# Remove workspace directory upon completion.
trap "rm -rf $WORKSPACE_DIR" EXIT

function usage {
    echo "Usage: ${BASH_SOURCE[0]} <path_to_platform_tar> <output_folder>" 1>&2
    exit 1
}

function replace() {
  local old_text=$1
  local new_text=$2
  local filename=$3
  [ ! -f "$filename" ] && echo "File not found $filename in $(pwd)" && exit 1
  sed -i "/${old_text}/ s/.*/${new_text}/" $filename
}

function validate_input_params {
    if [[ ! -f "${PLATFORM_TARBALL}" ]] && [[ ! -d "${PLATFORM_TARBALL}" ]]
    then
        echo "ERROR: Platform tarball or directory not found."
        usage
    fi

    if [[ ! -f "${CONFIG_FILE}" ]]
    then
        echo "ERROR: Could not find config file ${CONFIG_FILE}. Please check" \
             "that you have extracted the build script properly and try again."
        usage
    fi
}

function display_config {
    echo "-------------------------------------------------------------------------"
    echo "SOURCE TARBALL: ${PLATFORM_TARBALL}"
    echo "TARGET DIRECTORY: ${TARGET_DIR}"
    echo "UBOOT SUBPATH: ${UBOOT_SUBPATH}"
    echo "UBOOT DEFCONFIG: ${UBOOT_DEFCONFIG_NAME}"
    echo "-------------------------------------------------------------------------"
    echo "Sleeping 3 seconds before continuing."
    sleep 3
}

function setup_output_dir {
    if [[ -d "${TARGET_DIR}" ]]
    then
        FILECOUNT=$(find "${TARGET_DIR}" -type f | wc -l)
        if [[ ${FILECOUNT} -gt 0 ]]
        then
            echo "ERROR: Destination folder is not empty. Refusing to build" \
                 "to a non-clean target"
            exit 3
        fi
    else
        echo "Making target directory ${TARGET_DIR}"
        mkdir -vp "${TARGET_DIR}"

        if [[ $? -ne 0 ]]
        then
            echo "ERROR: Could not make target directory ${TARGET_DIR}"
            exit 1
        fi
    fi
}

function download_toolchain {
    echo "Downloading toolchain ${TOOLCHAIN_REPO} to ${TOOLCHAIN_DIR}"
    curl -sS -L ${TOOLCHAIN_REPO} | tar -xJ -C ${TOOLCHAIN_DIR} || {
        echo "ERROR: Could not download toolchain from ${TOOLCHAIN_REPO}"
        exit 2
    }
}

function extract_tarball {
    echo "Extracting tarball to ${PLATFORM_EXTRACT_DIR}"
    tar xf "${PLATFORM_TARBALL}" -C ${PLATFORM_EXTRACT_DIR}
}

function exec_build_uboot {
    local CCOMPILE="${TOOLCHAIN_DIR}/$TOOLCHAIN_NAME/bin/${TOOLCHAIN_PREFIX}"
    local INC="-I${PLATFORM_EXTRACT_DIR}/external/libtomcrypt/src/headers"
    local MAKE_ARGS="${MAKE_ARGS} O=${UBOOT_OUT_DIR} CROSS_COMPILE=${CCOMPILE} INC=${INC}"

    echo ${MAKE_ARGS}

    pushd "${PLATFORM_EXTRACT_DIR}/${UBOOT_SUBPATH}" || exit 1

    ln -vsf "${PWD}"/amazon/security/unlock_pub_key.h "${PWD}"/amazon/include/

    local old_conf='UFBL_PATH       :='
    local new_conf='UFBL_PATH       := $(PWD)\/..\/..\/..\/ufbl-features'
    replace "$old_conf" "$new_conf" Makefile.ufbl

    # Step 1: make defconfig
    if ! make ${MAKE_ARGS} ${UBOOT_DEFCONFIG_NAME}; then
        echo "An error occurred while processing make defconfig"
        return 1
    fi

    replace "CONFIG_DIAG_TRANSITION_DIALOG" "CONFIG_DIAG_TRANSITION_DIALOG=y" .config
    replace "CONFIG_ROLLBACK_INDEX_IN_EFUSE" "CONFIG_ROLLBACK_INDEX_IN_EFUSE=y" .config

    # Step 2: full make
    if ! make ${MAKE_ARGS} ${PARALLEL_EXECUTION}; then
        echo "An error occurred while processing full make."
        return 1
    fi
}

function copy_to_output {
    echo "Copying Uboot files to output"
    local IFS=":"
    UBOOT_OUT_DIR="${PLATFORM_EXTRACT_DIR}/${UBOOT_SUBPATH}"
    pushd "${UBOOT_OUT_DIR}"
    for IMAGE in ${UBOOT_IMAGES};do
        local BASEDIR="$(dirname "${IMAGE}")"
        if [[ ! -d "${TARGET_DIR}/${BASEDIR}" ]]
        then
            mkdir -p "${TARGET_DIR}/${BASEDIR}"
        fi
        cp -v ${IMAGE} "${TARGET_DIR}/${IMAGE}"
    done
    popd
}

function validate_output {
    echo "Listing output files"
    local IFS=":"
    for IMAGE in ${UBOOT_IMAGES};do
        if [ ! -f ${TARGET_DIR}/${IMAGE} ]; then
            echo "ERROR: Missing uboot output image ${IMAGE}" >&2
            exit 1
        fi
        ls -l ${TARGET_DIR}/${IMAGE}
    done
}

################################################################################
#
#  M A I N
#
################################################################################

# Phase 1: Set up execution
validate_input_params
source "${CONFIG_FILE}"

setup_output_dir
TARGET_DIR="$(cd "${TARGET_DIR}" && pwd)"
display_config

# Phase 2: Set up environment
if [ -n "${TOOLCHAIN_NAME}" ] && [ -n "$(ls -A /tmp/${TOOLCHAIN_NAME})" ]; then
   TOOLCHAIN_DIR="/tmp"
else
    download_toolchain
fi
extract_tarball
# Phase 3: build
exec_build_uboot

# Phase 4: move to output
copy_to_output

# Phase 5: verify output
validate_output
