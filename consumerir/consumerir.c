/*
 * Copyright (C) 2013 The Android Open Source Project
 * Copyright (C) 2017 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define LOG_TAG "ConsumerIrHal"

#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <cutils/log.h>
#include <hardware/hardware.h>
#include <hardware/consumerir.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

static const consumerir_freq_range_t consumerir_freqs[] = {
    {.min = 30000, .max = 30000},
    {.min = 33000, .max = 33000},
    {.min = 36000, .max = 36000},
    {.min = 38000, .max = 38000},
    {.min = 40000, .max = 40000},
    {.min = 56000, .max = 56000},
};

static unsigned char read_byte(const int pattern[], unsigned int offset)
{
    int result = 0;

    for (unsigned int i = offset; i < offset + 16; i += 2) {
        // int pulse = pattern[i]; // we don't care
        int space = pattern[i + 1];
        // ALOGI("%d %d", pulse, space);
        if (space > 1000) {
            result = (result >> 1) | 0x80;
        } else {
            result = (result >> 1);
        }
    }

    return (unsigned char) (result & 0xFF);
}

int fd = 0;
static int consumerir_transmit(struct consumerir_device *dev __unused,
   int carrier_freq __unused, const int pattern[], int pattern_len)
{
    int i = 0, j = 0;
    int rc;

    unsigned char buffer[32];
    size_t buffer_len = 0;

    // unsigned char address_low;
    // unsigned char address_high;
    // unsigned char command;

    /* 1 pulse + 1 space + 32 bit */
    /* 1 bit = 1 pulse + 1 space  */
    if (pattern_len < 66) {
        ALOGE("invalid pattern_len: %d", pattern_len);
        return -1;
    }

    memset(buffer, 0, sizeof(buffer));

    buffer[0] = 0xA1;
    buffer[1] = 0xF1;

    while (i < pattern_len) {
        int space = pattern[i + 1];
        ALOGI("[%d] %d", i, space);
        if (space <= 2000) { // normal
            buffer[2 + buffer_len] = read_byte(pattern, i);
            buffer_len += 1;
            if (buffer_len < 3) { // extended address
                i += 2 * 8;
            } else {
                i += 2 * 8 * 2;
            }
        } else if (space > 4000 && space <= 5000) { // start
            buffer_len = 0;
            i += 2 * 1;
        } else if (space > 5000) { // end
            if (buffer_len > 0) {
                rc = write(fd, buffer, buffer_len + 2);
                if (rc < 0) {
                    ALOGE("failed to write buffer %d, error %d", buffer_len, rc);
                    return -4;
                }

                ALOGI("sent buffer %d", buffer_len);
                buffer_len = 0;
            }

            i += 2 * 1;
        } else {
            i += 2 * 1;
        }
    }

    return 0;
}

static int consumerir_get_num_carrier_freqs(struct consumerir_device *dev __unused)
{
    return ARRAY_SIZE(consumerir_freqs);
}

static int consumerir_get_carrier_freqs(struct consumerir_device *dev __unused,
    size_t len, consumerir_freq_range_t *ranges)
{
    size_t to_copy = ARRAY_SIZE(consumerir_freqs);

    to_copy = len < to_copy ? len : to_copy;
    memcpy(ranges, consumerir_freqs, to_copy * sizeof(consumerir_freq_range_t));
    return to_copy;
}

static int consumerir_close(hw_device_t *dev)
{
    free(dev);
    close(fd);
    return 0;
}

/*
 * Generic device handling
 */
static int consumerir_open(const hw_module_t* module, const char* name,
        hw_device_t** device)
{
    if (strcmp(name, CONSUMERIR_TRANSMITTER) != 0) {
        return -EINVAL;
    }

    if (device == NULL) {
        ALOGE("%s: NULL device on open", __func__);
        return -EINVAL;
    }

    consumerir_device_t *dev = malloc(sizeof(consumerir_device_t));
    memset(dev, 0, sizeof(consumerir_device_t));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*) module;
    dev->common.close = consumerir_close;

    dev->transmit = consumerir_transmit;
    dev->get_num_carrier_freqs = consumerir_get_num_carrier_freqs;
    dev->get_carrier_freqs = consumerir_get_carrier_freqs;

    *device = (hw_device_t*) dev;

    fd = open("/dev/ttyS1", O_RDWR);

    if (!fd) {
        ALOGE("%s: Failed to open /dev/ttyS1", __func__);
        return -EINVAL;
    }

    return 0;
}

static struct hw_module_methods_t consumerir_module_methods = {
    .open = consumerir_open,
};

consumerir_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag                = HARDWARE_MODULE_TAG,
        .module_api_version = CONSUMERIR_MODULE_API_VERSION_1_0,
        .hal_api_version    = HARDWARE_HAL_API_VERSION,
        .id                 = CONSUMERIR_HARDWARE_MODULE_ID,
        .name               = "NEC IR Module",
        .author             = "The LineageOS Project & XiNGRZ",
        .methods            = &consumerir_module_methods,
    },
};
