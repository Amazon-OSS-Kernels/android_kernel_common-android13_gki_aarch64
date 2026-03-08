#!/bin/bash
################################################################################
#
#  build_kernel.sh
#
#  Copyright (c) 2025 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
################################################################################

################################################################################
#
#  I N P U T
#
################################################################################
# Target directory for output artifacts.
TARGET_DIR="${1}"

################################################################################
#
#  V A R I A B L E S
#
################################################################################

# Retrieve the directory where the script is currently held
SCRIPT_BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Configuration file for the build.
CONFIG_FILE="${SCRIPT_BASE_DIR}/build_kernel_config.sh"

# Workspace directory & relevant temp folders.
WORKSPACE_DIR="$(mktemp -d)"

TOOLCHAIN_DIR="${WORKSPACE_DIR}/toolchain"
PLATFORM_EXTRACT_DIR="${WORKSPACE_DIR}/src"
WORKSPACE_OUT_DIR="${PLATFORM_EXTRACT_DIR}/out/target/product/obj/KERNEL_OBJ"

mkdir -p "${TOOLCHAIN_DIR}" "${PLATFORM_EXTRACT_DIR}" "${WORKSPACE_OUT_DIR}"

# Remove workspace directory upon completion.
trap 'rm -rf "$WORKSPACE_DIR"' EXIT


function usage {
    echo "Usage: ${BASH_SOURCE[0]} output_folder" 1>&2
    exit 1
}

function validate_input_params {
    if [[ -z ${TARGET_DIR} ]]; then
        echo "ERROR: Output folder not provided"
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
    echo "REPO URL: ${REPO_URL}"
    echo "REPO GPG KEY": "${REPO_GPG_KEY}"
    echo "KERNEL MANIFEST: ${KERNEL_MANIFEST}"
    echo "GKI CONFIG: ${GKI_CONFIG}"
    echo "KERNEL BRANCH: ${KERNEL_BRANCH}"
    echo "PARALLEL_EXECUTION: ${PARALLEL_EXECUTION}"
    echo "TARGET DIRECTORY: ${TARGET_DIR}"
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
        if ! mkdir -vp "${TARGET_DIR}"/{out,dist}; then
            echo "ERROR: Could not make target directory ${TARGET_DIR}"
            exit 1
        fi
        TARGET_DIR="$(cd "${TARGET_DIR}" && pwd)"
    fi
}

function download_repo {
    REPO_BIN="${WORKSPACE_OUT_DIR}/.bin"
    mkdir -vp "${REPO_BIN}"
    curl -o "${REPO_BIN}/repo" "${REPO_URL}/repo"

    if ! gpg --recv-keys "${REPO_GPG_KEY}"; then
        echo "ERROR: Something went wrong trying to fetch the gpg key"
        exit 1
    fi

    if curl -s "${REPO_URL}/repo.asc" | gpg --verify - "${REPO_BIN}/repo"; then
        chmod 755 "${REPO_BIN}/repo"
    else
        echo "ERROR: repo could not be verified"
    fi

    if ! "${REPO_BIN}/repo" version; then
      echo "ERROR: repo could not be installed"
      exit 1
    fi
}

function download_kernel_source() {
    pushd "${PLATFORM_EXTRACT_DIR}" || exit 1
    if "${REPO_BIN}/repo" init -u "${KERNEL_MANIFEST}" -b "${KERNEL_BRANCH}" --depth 1; then
        "${REPO_BIN}/repo" sync -v $PARALLEL_EXECUTION;
    else
        echo "ERROR: repo could not be initialized"
    fi
}

function exec_build_kernel {
    if ! LTO=thin \
    FAST_BUILD=1 \
    OUT_DIR="${TARGET_DIR}/out" \
    DIST_DIR="${TARGET_DIR}/dist" \
    BUILD_CONFIG="common/${GKI_CONFIG}" \
    build/build.sh; then
        echo "ERROR: Can't build the kernel"
        exit 1
    fi
}

function validate_output {
    echo "Listing output files"
    local IFS=":"
    for IMAGE in ${KERNEL_IMAGES};do
        if [ ! -f "${TARGET_DIR}/${IMAGE}" ]; then
            echo "ERROR: Missing kernel output image ${IMAGE}" >&2
            exit 1
        fi
        ls -l "${TARGET_DIR}/${IMAGE}"
    done
    popd || exit 1
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
display_config

# Phase 2: download repo and kernel source
download_repo
download_kernel_source

# Phase 3: build and move to output
exec_build_kernel

# Phase 4: verify output
validate_output
