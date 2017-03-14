LOCAL_PATH := $(call my-dir)

ifeq ($(TARGET_DEVICE),rk3288)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
