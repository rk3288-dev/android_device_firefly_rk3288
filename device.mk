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

$(call inherit-product, device/rockchip/common/common.mk)

$(call inherit-product, frameworks/native/build/tablet-10in-xhdpi-2048-dalvik-heap.mk)

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/fstab.rk30board:root/fstab.rk30board

$(call inherit-product, device/rockchip/common/samba/rk31_samba.mk)

# GPS
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml

PRODUCT_PACKAGES += \
    gps.rk3288

PRODUCT_PROPERTY_OVERRIDES += \
    ro.kernel.android.gps=ttyS3 \
    ro.kernel.android.gpsttybaud=9600

# Lights
PRODUCT_PACKAGES += \
    lights.rk3288

# TWRP
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/recovery.fstab:recovery/root/etc/twrp.fstab
