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
#include <termios.h>
#include <fcntl.h>
#include <cutils/log.h>
#include <cutils/sockets.h>
#include <hardware/hardware.h>
#include <hardware/consumerir.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define HEADER_SIZE 3
#define FOOTER_SIZE 2

static int fd = 0;
static int socks[2];

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

static int consumerir_transmit(struct consumerir_device *dev __unused,
   int carrier_freq, const int pattern[], int pattern_len)
{
    int i = 0;
    int rc;

    unsigned char byte = 0;
    unsigned char buffer[32];
    unsigned char sum = 0;

    unsigned char result[32];

    size_t data_len = 0;

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
    buffer[0] = 0xFA;

    buffer[1] = 0xF1;
    rc = write(socks[0], buffer, 2);
    if (rc < 0) {
        ALOGE("failed to write buffer %d, error %d", data_len, rc);
        return -4;
    }

    ALOGI("send rc: %d", rc);

    rc = read(socks[1], result, 32);
    ALOGI("result: %s", result);


    // buffer[1] is data_len
    buffer[2] = carrier_freq;

    while (i < pattern_len) {
        int space = pattern[i + 1];
        ALOGI("[%d] %d", i, space);
        if (space <= 2000) { // normal
            byte = read_byte(pattern, i);

            buffer[HEADER_SIZE + data_len] = byte;
            sum = (sum + byte) & 0xFF;

            data_len += 1;
            if (data_len < 3) { // extended address
                i += 2 * 8;
            } else {
                i += 2 * 8 * 2;
            }
        } else if (space > 4000 && space <= 5000) { // start
            data_len = 0;
            sum = 0;
            i += 2 * 1;
        } else if (space > 5000) { // end
            if (data_len > 0) {
                buffer[1] = data_len + 1; // freq + data[]

                buffer[HEADER_SIZE + data_len] = (0x100 - sum) & 0xFF;  // sum
                buffer[HEADER_SIZE + data_len + 1] = 0xED;  // end

                rc = write(socks[0], buffer, HEADER_SIZE + data_len + FOOTER_SIZE);
                if (rc < 0) {
                    ALOGE("failed to write buffer %d, error %d", data_len, rc);
                    return -4;
                }

                ALOGI("send rc: %d", rc);
                ALOGI("sent buffer %d", data_len);

                rc = read(socks[1], result, 2);
                ALOGI("result: %02x %02x", result[0], result[1]);

                sum = 0;
                data_len = 0;
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
    close(socks[0]);
    close(socks[1]);
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

    struct termios ios;
    tcgetattr(fd, &ios);
    ios.c_lflag = 0;  /* disable ECHO, ICANON, etc... */
    ios.c_oflag &= (~ONLCR); /* Stop \n -> \r\n translation on output */
    ios.c_iflag &= (~(ICRNL | INLCR)); /* Stop \r -> \n & \n -> \r translation on input */
    ios.c_iflag |= (IGNCR | IXOFF);  /* Ignore \r & XON/XOFF on input */
    ios.c_cflag = B9600 | CRTSCTS | CS8 | CLOCAL | CREAD;
    tcsetattr(fd, TCSANOW, &ios);

    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, socks) < 0) {
        ALOGE("Could not create thread control socket pair: %s", strerror(errno));
        close(fd);
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
        .name               = "YK-010 IR Module",
        .author             = "XiNGRZ",
        .methods            = &consumerir_module_methods,
    },
};
