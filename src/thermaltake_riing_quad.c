#include <hidapi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <unistd.h>

#define THERMALTAKE_VID     0x264a
#define HID_MAX_STR 255
#define THERMALTAKE_QUAD_PACKET_SIZE 193
#define THERMALTAKE_QUAD_INTERRUPT_TIMEOUT 250
#define THERMALTAKE_NUM_CHANNELS 5
enum
{
    THERMALTAKE_FAN_MODE_FIXED      = 0x01,
    THERMALTAKE_FAN_MODE_PWM        = 0x02
};

typedef struct {
    char *key;
    int val;
}pair;

static pair lookup[] = {
    {"set", 1},
    {"increase", 2},
    {"decrease", 3}
};

hid_device* dev;

int
key_from_string(char *str) {
    for (size_t i = 0; i < 3; i++) {
        if (strcmp(lookup[i].key, str) == 0) {
            return lookup[i].val;
        }
    }
    return -1;
}

void send_init()
{
    unsigned char usb_buf[THERMALTAKE_QUAD_PACKET_SIZE];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up Init packet                                    |
    \*-----------------------------------------------------*/
    usb_buf[0x00]   = 0x00;
    usb_buf[0x01]   = 0xFE;
    usb_buf[0x02]   = 0x33;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, THERMALTAKE_QUAD_PACKET_SIZE);
    hid_read_timeout(dev, usb_buf, THERMALTAKE_QUAD_PACKET_SIZE, THERMALTAKE_QUAD_INTERRUPT_TIMEOUT);
}

char ret_str[HID_MAX_STR];
void get_firmware_version()
{
    unsigned char usb_buf[THERMALTAKE_QUAD_PACKET_SIZE];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up Get Firmware Version packet                    |
    \*-----------------------------------------------------*/
    usb_buf[0x00]   = 0x00;
    usb_buf[0x01]   = 0x33;
    usb_buf[0x02]   = 0x50;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, THERMALTAKE_QUAD_PACKET_SIZE);
    hid_read_timeout(dev, usb_buf, THERMALTAKE_QUAD_PACKET_SIZE, THERMALTAKE_QUAD_INTERRUPT_TIMEOUT);

    sprintf(ret_str, "%d.%d.%d", usb_buf[2], usb_buf[3], usb_buf[4]);
}

void get_fan_data
    (
        unsigned char       port,
        unsigned char *     speed,
        unsigned short *    rpm
    )
{
    unsigned char usb_buf[THERMALTAKE_QUAD_PACKET_SIZE];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up Get Fan Data packet                            |
    \*-----------------------------------------------------*/
    usb_buf[0x00]   = 0x00;
    usb_buf[0x01]   = 0x33;
    usb_buf[0x02]   = 0x51;
    usb_buf[0x03]   = port;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, THERMALTAKE_QUAD_PACKET_SIZE);
    hid_read_timeout(dev, usb_buf, THERMALTAKE_QUAD_PACKET_SIZE, THERMALTAKE_QUAD_INTERRUPT_TIMEOUT);

    *speed = usb_buf[0x04];
    *rpm   = (usb_buf[0x06] << 8) + usb_buf[0x05];
}

void send_fan
    (
        unsigned char       port,
        unsigned char       mode,
        unsigned char       speed
    )
{
    unsigned char usb_buf[THERMALTAKE_QUAD_PACKET_SIZE];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up RGB packet                                     |
    \*-----------------------------------------------------*/
    usb_buf[0x00]   = 0x00;
    usb_buf[0x01]   = 0x32;
    usb_buf[0x02]   = 0x51;
    usb_buf[0x03]   = port;
    usb_buf[0x04]   = mode;
    usb_buf[0x05]   = speed;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, THERMALTAKE_QUAD_PACKET_SIZE);
    hid_read_timeout(dev, usb_buf, THERMALTAKE_QUAD_PACKET_SIZE, THERMALTAKE_QUAD_INTERRUPT_TIMEOUT);
}

void
get_info(hid_device *dev) {
        wchar_t name_string[HID_MAX_STR];

        hid_get_manufacturer_string(dev, name_string, HID_MAX_STR);

        printf("Name: %ls\n", name_string);

        hid_get_product_string(dev, name_string, HID_MAX_STR);

        printf("Prod Name: %ls\n", name_string);
}


int
main(int argc, char *argv[]) {
    int value = 0;
    unsigned PID;
    int res;
    int c;
    char *mode = NULL;
    size_t key_mode;

    if (argc < 2) {
        printf("Can't run without params\n");
        exit(EXIT_FAILURE);
    }

    while ((c = getopt(argc, argv, "m:v:p:")) != -1) {
            switch (c) {
                case 'm':
                    mode = optarg;
                    break;
                case 'v':
                    value = atoi(optarg);
                    break;
                case 'p':
                    PID = strtol(optarg, NULL, 16);
                    break;
                default:
                    abort();
            }
    }

    key_mode = key_from_string(mode);

    if (key_mode == -1) {
        printf("Incorrect mode\n");
        exit(EXIT_FAILURE);
    }

    printf("Value: %d | PID: %d\n", value, PID);

    res = hid_init();
    dev = hid_open(THERMALTAKE_VID, PID, NULL);

    if (dev) {
        get_info(dev);

        send_init();

        get_firmware_version();

        printf("Firmware Version: %s\n", ret_str);

        printf("Fans:\n");
        for (size_t fan_index = 0; fan_index < THERMALTAKE_NUM_CHANNELS; fan_index++) {
            unsigned char speed;
            unsigned short rpm;

            get_fan_data(fan_index + 1, &speed, &rpm);

            printf("--Fan #%zu: speed - %d, rpm - %d\n", fan_index + 1, speed, rpm);

            switch (key_from_string(mode)) {
                case 1:
                    send_fan(fan_index + 1, THERMALTAKE_FAN_MODE_FIXED, value);
                    get_fan_data(fan_index + 1, &speed, &rpm);
                    printf("----New Fan #%zu: speed - %d, rpm - %d\n", fan_index + 1, speed, rpm);
                    break;
                case 2:
                    send_fan(fan_index + 1, THERMALTAKE_FAN_MODE_FIXED, speed + value);
                    get_fan_data(fan_index + 1, &speed, &rpm);
                    printf("----New Fan #%zu: speed - %d, rpm - %d\n", fan_index + 1, speed, rpm);
                    break;
                case 3:
                    send_fan(fan_index + 1, THERMALTAKE_FAN_MODE_FIXED, speed + value);
                    get_fan_data(fan_index + 1, &speed, &rpm);
                    printf("----New Fan #%zu: speed - %d, rpm - %d\n", fan_index + 1, speed, rpm);
                    break;
            }
        }

        hid_close(dev);

    }

    return 0;
}
