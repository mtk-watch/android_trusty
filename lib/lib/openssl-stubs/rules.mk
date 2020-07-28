LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)
MODULE_USER := true

MODULE_SRCS := \
	$(LOCAL_DIR)/rand.c \
	$(LOCAL_DIR)/bio.c \
	$(LOCAL_DIR)/sscanf.c

MODULE_SRCS += \
	$(LOCAL_DIR)/time.c \
	$(ANDROIDMAKEROOT)/external/boringssl/src/crypto/asn1/time_support.c


MODULE_DEPS := \
	lib/libc-trusty \
	lib/rng

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

include make/module.mk
