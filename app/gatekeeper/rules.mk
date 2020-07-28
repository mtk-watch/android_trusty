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

LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GATEKEEPER_ROOT := $(ANDROIDMAKEROOT)/system/gatekeeper

MODULE_SRCS += \
	$(LOCAL_DIR)/manifest.c \
	$(LOCAL_DIR)/trusty_gatekeeper.cpp \
	$(GATEKEEPER_ROOT)/gatekeeper_messages.cpp \
	$(GATEKEEPER_ROOT)/gatekeeper.cpp

GLOBAL_CPPFLAGS += -std=c++11 -Dhtobe32\(x\)=__builtin_bswap32\(x\)

IPC := ipc

MODULE_DEPS += \
	app/trusty \
	lib/libc-trusty \
	lib/libstdc++-trusty \
	lib/rng \
	lib/hwkey \
	lib/storage \
	lib/keymaster \

MODULE_INCLUDES += \
	$(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(GATEKEEPER_ROOT)/../../hardware/libhardware/include \
	$(GATEKEEPER_ROOT)/include

include $(LOCAL_DIR)/$(IPC)/rules.mk

include make/module.mk

