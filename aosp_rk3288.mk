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

$(call inherit-product, $(SRC_TARGET_DIR)/product/aosp_base.mk)

# Inherit device configuration
$(call inherit-product, device/firefly/rk3288/device.mk)

PRODUCT_NAME := aosp_rk3288
PRODUCT_DEVICE := rk3288
PRODUCT_BRAND := Firefly
PRODUCT_MODEL := RK3288
PRODUCT_MANUFACTURER := Rockchip

PRODUCT_BUILD_PROP_OVERRIDES += \
    BUILD_PRODUCT=RK3288 \
    TARGET_DEVICE=RK3288

$(call inherit-product-if-exists, vendor/firefly/rk3288/device-vendor.mk)
