LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS := \
	$(LOCAL_DIR)/mtktimer.c \

MODULE_INCLUDES := \
	$(LOCAL_DIR) \

include make/module.mk
