#include "hidapi_wrapper.h"
#include "hidapi.h"
#include <stdio.h>
#include <string.h>

hid_device *dev;

int
initialize(unsigned device_id)
{
    hid_init();
    dev = hid_open(THERMALTAKE_VID, device_id, NULL);

    return dev != NULL;
}

void
send_init()
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

void
show_firmware_version()
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

    printf("Firmware version: %d.%d.%d\n", usb_buf[2], usb_buf[3], usb_buf[4]);
}

void
get_fan_data ( unsigned char       port,
               unsigned char *     speed,
               unsigned short *    rpm)
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

void
send_fan ( unsigned char       port,
           unsigned char       mode,
           unsigned char       speed)
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
show_info() {
        wchar_t name_string[HID_MAX_STR];

        hid_get_manufacturer_string(dev, name_string, HID_MAX_STR);

        printf("Name: %ls\n", name_string);

        hid_get_product_string(dev, name_string, HID_MAX_STR);

        printf("Prod Name: %ls\n", name_string);

        for (size_t fan_index = 0; fan_index < THERMALTAKE_NUM_CHANNELS; fan_index++) {
            unsigned char speed;
            unsigned short rpm;

            get_fan_data(fan_index + 1, &speed, &rpm);

            printf("---- Fan %zu: speed - %hhu; rpm - %d\n",fan_index + 1, speed, rpm);
        }
}

void
end_hid()
{
    hid_close(dev);
}
