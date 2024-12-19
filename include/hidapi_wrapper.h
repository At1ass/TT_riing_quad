#ifndef __HIDAPI_WRAPPER_H__
#define __HIDAPI_WRAPPER_H__

#include <hidapi.h>

#define THERMALTAKE_VID     0x264a
#define HID_MAX_STR         256
#define THERMALTAKE_QUAD_PACKET_SIZE 193
#define THERMALTAKE_QUAD_INTERRUPT_TIMEOUT 250
#define THERMALTAKE_NUM_CHANNELS 5

enum
{
    THERMALTAKE_FAN_MODE_FIXED      = 0x01,
    THERMALTAKE_FAN_MODE_PWM        = 0x02
};

int initialize(unsigned device_id);

void send_init();

void show_firmware_version();

void get_fan_data (unsigned char port, unsigned char *speed, unsigned short *rpm);

void send_fan (unsigned char port, unsigned char mode, unsigned char speed);

void show_info ();

void end_hid();

#endif //__HIDAPI_WRAPPER_H__
