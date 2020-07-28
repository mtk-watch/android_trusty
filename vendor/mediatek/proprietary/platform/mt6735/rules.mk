# 
# Copyright (C) 2015 MediaTek Inc. 
#
# Modification based on code covered by the below mentioned copyright
# and/or permission notice(S). 
#

#
# Copyright (c) 2015, Google, Inc. All rights reserved
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files
# (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq (false,$(call TOBOOL,$(KERNEL_32BIT)))
ARCH := arm64
else
ARCH := arm
ARM_CPU := cortex-a15
endif
WITH_SMP := 1

KERNEL_BASE ?= 0x50000000
KERNEL_LOAD_OFFSET ?= 0x00040000
MEMBASE ?= $(KERNEL_BASE)
MEM_SIZE ?= 0x400000

ifeq (false,$(call TOBOOL,$(KERNEL_32BIT)))
MEMSIZE ?= 1
else
MEMSIZE ?= $(MEM_SIZE)
endif
CFG_LOG_REG_BASE ?= UART0_BASE
CFG_LOG_BAUDRATE ?= 921600

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include

MODULE_SRCS += \
	$(LOCAL_DIR)/debug.c \
	$(LOCAL_DIR)/platform.c \
	$(LOCAL_DIR)/smc.c \
	$(LOCAL_DIR)/uart.c \
	$(LOCAL_DIR)/gpt.c \
	$(LOCAL_DIR)/reg_base.c \

MODULE_DEPS += \
	dev/interrupt/arm_gic \
	dev/timer/arm_generic

GLOBAL_DEFINES += \
	MEMBASE=$(MEMBASE) \
	MEMSIZE=$(MEMSIZE) \
	MMU_WITH_TRAMPOLINE=1 \

ifeq (false,$(call TOBOOL,$(KERNEL_32BIT)))
else
GLOBAL_DEFINES += \
	WITH_TIMERS_MIGRATION=1 \
	UART_LOG_BAUDRATE=$(CFG_LOG_BAUDRATE) \
	UART_LOG_REG_BASE=$(CFG_LOG_REG_BASE) \
	DISABLE_TRACE_INIT_AFTER_BOOTUP=1 \

ifeq (eng,$(TARGET_BUILD_VARIANT))
GLOBAL_DEFINES += \
	WITH_MT_TRUSTY_DEBUGFS=1 \
	WITH_HWCRYPTO_UNITTEST=1
endif # TARGET_BUILD_VARIANT
endif # KERNEL_32BIT==true

LINKER_SCRIPT += \
	$(BUILDDIR)/system-onesegment.ld

include make/module.mk
