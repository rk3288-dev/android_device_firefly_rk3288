#
# Copyright 2014 The Android Open-Source Project
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

DEVICE_PATH := device/firefly/rk3288

# Use the non-open-source parts, if they're present
-include vendor/firefly/rk3288/BoardConfigVendor.mk
-include device/rockchip/common/BoardConfigCommon.mk

BOARD_VENDOR := rockchip

# Bootloader
TARGET_NO_BOOTLOADER := true

TARGET_PARAMETERS_MTDPARTS := $(DEVICE_PATH)/mtdparts.txt

# Platform
TARGET_BOARD_PLATFORM := rk3288
TARGET_BOARD_PLATFORM_GPU := mali-t760
TARGET_BOARD_PLATFORM_PRODUCT := box

# Architecture
TARGET_ARCH := arm
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_CPU_VARIANT := cortex-a15
TARGET_CPU_SMP := true

# Kernel
TARGET_KERNEL_SOURCE := kernel/firefly/rk3288
TARGET_KERNEL_CONFIG := firefly-rk3288_defconfig

# HAX: Remove this once done
BOARD_KERNEL_CMDLINE += androidboot.selinux=permissive

# Filesystem
TARGET_USERIMAGES_USE_F2FS := true

# Battery
BOARD_HAS_BATTERY := false

# Bluetooth
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := $(DEVICE_PATH)/bluetooth

# Connectivity
BOARD_CONNECTIVITY_VENDOR := Broadcom
BOARD_CONNECTIVITY_MODULE := ap6xxx

# Display
TARGET_DISPLAY_POLICY_BOX := box
BOARD_SUPPORT_HDMI_CEC := true

# Overlays
PRODUCT_PACKAGE_OVERLAYS += $(DEVICE_PATH)/overlay

# Include an expanded selection of fonts
EXTENDED_FONT_FOOTPRINT := true

# Sensors
BOARD_SENSOR_ST := false
BOARD_SENSOR_MPU := true
BOARD_USES_GENERIC_INVENSENSE := false

# TWRP
RECOVERY_VARIANT := twrp

TARGET_RECOVERY_FSTAB := $(DEVICE_PATH)/fstab.rk30board
TARGET_RECOVERY_PIXEL_FORMAT := RGBX_8888

TW_THEME := landscape_mdpi
TW_EXTRA_LANGUAGES := true
TW_NO_BATT_PERCENT := true
TW_NO_SCREEN_TIMEOUT := true
TW_INCLUDE_CRYPTO := true

TWRP_INCLUDE_LOGCAT := true
