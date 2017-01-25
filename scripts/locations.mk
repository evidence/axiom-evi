
LOCFILE_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
AXIOMHOME=$(realpath ${LOCFILE_DIR}/..)

QEMU_DIR := $(LOCFILE_DIR)/../axiom-evi-qemu
BUILDROOT_DIR := $(LOCFILE_DIR)/../axiom-evi-buildroot
ARM_FIRMWARE_DIR := $(LOCFILE_DIR)/../arm-trusted-firmware
QEMU_DTS_DIR := $(LOCFILE_DIR)/./buildroot/qemu-dts
EVI_NIC_DIR := $(LOCFILE_DIR)/../axiom-evi-nic
EVI_APPS_DIR := $(LOCFILE_DIR)/../axiom-evi-apps
GASNET_DIR := $(LOCFILE_DIR)/../axiom-evi-gasnet
GASNET_BUILD_DIR_DEBUG := $(LOCFILE_DIR)/../axiom-evi-gasnet/build_debug
GASNET_BUILD_DIR_PERF := $(LOCFILE_DIR)/../axiom-evi-gasnet/build_perf
MCXX_DIR := $(LOCFILE_DIR)/../axiom-evi-mcxx
MCXX_BUILD_DIR := $(MCXX_DIR)/build
NANOX_DIR := $(LOCFILE_DIR)/../axiom-evi-nanox
NANOX_BUILD_DIR_DEBUG := $(NANOX_DIR)/build_debug
NANOX_BUILD_DIR_PERF := $(NANOX_DIR)/build_perf
EXTRAE_DIR := $(LOCFILE_DIR)/../axiom-evi-extrae
EXTRAE_BUILD_DIR := $(EXTRAE_DIR)/build
EVI_ALLOC_DRV_DIR := $(LOCFILE_DIR)/../axiom-evi-allocator-drv
EVI_ALLOC_LIB_DIR := $(LOCFILE_DIR)/../axiom-evi-allocator-lib
ALLOCATOR_DIR := $(LOCFILE_DIR)/../axiom-allocator
OVERLAY_DIR := $(LOCFILE_DIR)/buildroot/overlay
SCRIPTS_DIR := $(LOCFILE_DIR)

OUTPUT_DIR = $(realpath $(LOCFILE_DIR)/../output)
TARGET_DIR := $(realpath ${OUTPUT_DIR}/target)
SYSROOT_DIR := $(realpath ${OUTPUT_DIR}/staging)
HOST_DIR := $(realpath ${OUTPUT_DIR}/host)

IMAGES := $(OUTPUT_DIR)/images
ARCHIVE_VMS_DIR := archive_vms
ARCHIVE_TOOLCHAIN_DIR := archive_toolchain

TESTS_DIR := $(LOCFILE_DIR)/../tests/
OMPSS_TESTS_DIR := $(TESTS_DIR)/ompss
GASNET_TESTS_DIR := $(TESTS_DIR)/gasnet
AXIOM_TESTS_DIR := $(TESTS_DIR)/axiom
ARCHIVE_DIR := archive
