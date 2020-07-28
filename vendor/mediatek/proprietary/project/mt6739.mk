# 
# Copyright (C) 2015 MediaTek Inc. 
#
# Modification based on code covered by the below mentioned copyright
# and/or permission notice(S). 
#

# Copyright (C) 2015 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

KERNEL_32BIT := true

LOCAL_DIR := $(GET_LOCAL_DIR)

DEBUG ?= 2
SMP_MAX_CPUS ?= 4
SMP_CPU_CLUSTER_SHIFT ?= 2

TARGET := mt6739
CFG_CRYPTO_MODULE := HACC
CFG_GCPU_IP := common
ARM_GIC_V3_SUPPORT := 1

ifeq (false,$(call TOBOOL,$(KERNEL_32BIT)))

# Arm64 address space configuration
KERNEL_ASPACE_BASE := 0xffffffffe0000000
KERNEL_ASPACE_SIZE := 0x0000000020000000
KERNEL_BASE        := 0xffffffffe0000000
USER_ASPACE_BASE   := 0x0000000000008000
USER_ASPACE_SIZE   := 0x0000000001ff8000

GLOBAL_DEFINES += MMU_USER_SIZE_SHIFT=25 # 32 MB user-space address space

else

#################################
# Add project specific defines here w/ ':=' to override platform defaults
#################################
#KERNEL_BASE        := 0x4f040000
#MEM_SIZE           := 0x500000
#CFG_LOG_BAUDRATE   := 921600
#CFG_LOG_REG_BASE   := UART0_BASE # check reg_base.h for UARTX_BASE details

endif

# select timer
ifeq (true,$(call TOBOOL,$(KERNEL_32BIT)))
# 32 bit Secure EL1 with a 64 bit EL3 gets the non-secure physical timer
#GLOBAL_DEFINES += TIMER_ARM_GENERIC_SELECTED=CNTP
GLOBAL_DEFINES += TIMER_ARM_GENERIC_SELECTED=CNTV
else
GLOBAL_DEFINES += TIMER_ARM_GENERIC_SELECTED=CNTPS
endif

# Disable VFP and NEON for now
ARM_WITHOUT_VFP_NEON := true

#
# GLOBAL definitions
#

# requires linker GC
WITH_LINKER_GC := 1

# Need support for Non-secure memory mapping
WITH_NS_MAPPING := true

# do not relocate kernel in physical memory
GLOBAL_DEFINES += WITH_NO_PHYS_RELOCATION=1

# limit heap grows
GLOBAL_DEFINES += HEAP_GROW_SIZE=8192

# limit physical memory to 38 bit to prevert tt_trampiline from getting larger than arm64_kernel_translation_table
GLOBAL_DEFINES += MMU_IDENT_SIZE_SHIFT=38

GLOBAL_DEFINES += DISABLE_ATAP_SUPPORT

#
# Modules to be compiled into lk.bin
#
MODULES += \
	lib/sm \
	lib/trusty \
	lib/memlog \
	source/trusty-kernel/mtcrypto \
	source/trusty-kernel/mthkdf \
	source/trusty-kernel/mtsmcall \
	source/trusty-kernel/kmgetboot \

include project/sp_common.mk

TRUSTY_USER_ARCH := arm

#
# user tasks to be compiled into lk.bin
#

# prebuilt
TRUSTY_PREBUILT_USER_TASKS :=

# compiled from source
TRUSTY_ALL_USER_TASKS := \
	sample/ipc-unittest/main \
	sample/ipc-unittest/srv \
	sample/hwcrypto-unittest \
	app/gatekeeper \
	app/keymaster \
	app/storage \
	source/trusty-app/kmsetkey \

# This project requires trusty IPC
WITH_TRUSTY_IPC := true

EXTRA_BUILDRULES += app/trusty/user-tasks.mk
