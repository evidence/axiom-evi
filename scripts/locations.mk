
SCRIPTS_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
AXIOM_DIR=$(realpath ${SCRIPTS_DIR}/..)

NIC_DIR := $(AXIOM_DIR)/axiom-evi-nic
APPS_DIR := $(AXIOM_DIR)/axiom-evi-apps
ALLOCATOR_DIR := $(AXIOM_DIR)/axiom-allocator
ALLOCATOR_DRV_DIR := $(AXIOM_DIR)/axiom-evi-allocator-drv
ALLOCATOR_LIB_DIR := $(AXIOM_DIR)/axiom-evi-allocator-lib
EXTRAE_DIR := $(AXIOM_DIR)/axiom-evi-extrae
GASNET_DIR := $(AXIOM_DIR)/axiom-evi-gasnet
NANOX_DIR := $(AXIOM_DIR)/axiom-evi-nanox
MCXX_DIR := $(AXIOM_DIR)/axiom-evi-mcxx

ALLOCATOR_BUILD_DIR := $(ALLOCATOR_DIR)/build
GASNET_BUILD_DIR_DEBUG := $(GASNET_DIR)/build_debug
GASNET_BUILD_DIR_PERF := $(GASNET_DIR)/build_perf
MCXX_BUILD_DIR := $(MCXX_DIR)/build
MCXX_BUILD_NATIVE_DIR := $(MCXX_DIR)/build_native
NANOX_BUILD_DIR_DEBUG := $(NANOX_DIR)/build_debug
NANOX_BUILD_DIR_PERF := $(NANOX_DIR)/build_perf
NANOX_BUILD_DIR_INSTR := $(NANOX_DIR)/build_instr
NANOX_BUILD_DIR_INSTR_DEBUG := $(NANOX_DIR)/build_instr_debug
EXTRAE_BUILD_DIR := $(EXTRAE_DIR)/build

#OUTPUT_DIR := $(shell realpath $(AXIOM_DIR)/output)

TESTS_DIR := $(AXIOM_DIR)/tests
OMPSS_TESTS_DIR := $(TESTS_DIR)/ompss
GASNET_TESTS_DIR := $(TESTS_DIR)/gasnet
AXIOM_TESTS_DIR := $(TESTS_DIR)/axiom
