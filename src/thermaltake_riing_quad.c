#include <hidapi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <unistd.h>

#include "hidapi_wrapper.h"

typedef struct {
    char *key;
    int val;
}pair;

static pair lookup[] = {
    {"set", 1},
    {"increase", 2},
    {"decrease", 3},
    {"show", 4},
};
#define LOOKUPSIZE 4

int
key_from_string(char *str) {
    for (size_t i = 0; i < 4; i++) {
        if (strcmp(lookup[i].key, str) == 0) {
            return lookup[i].val;
        }
    }
    return -1;
}

int
main(int argc, char *argv[]) {
    int value = 0;
    unsigned PID;
    int res;
    int c;
    char *mode = NULL;
    size_t key_mode;
    size_t info = 0;

    if (argc < 2) {
        printf("Can't run without params\n");
        exit(EXIT_FAILURE);
    }

    while ((c = getopt(argc, argv, "m:v:ip:")) != -1) {
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
                case 'i':
                    info = 1;
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

    if (initialize(PID)) {

        send_init();

        if (info) {
            show_info();
            show_firmware_version();
        }

        for (size_t fan_index = 0; fan_index < THERMALTAKE_NUM_CHANNELS; fan_index++) {
            unsigned char speed;
            unsigned short rpm;

            get_fan_data(fan_index + 1, &speed, &rpm);

            switch (key_from_string(mode)) {
                case 1:
                    send_fan(fan_index + 1, THERMALTAKE_FAN_MODE_FIXED, value);
                    break;
                case 2:
                    send_fan(fan_index + 1, THERMALTAKE_FAN_MODE_FIXED, speed + value);
                    break;
                case 3:
                    send_fan(fan_index + 1, THERMALTAKE_FAN_MODE_FIXED, speed + value);
                    break;
                case 4:
                    get_fan_data(fan_index + 1, &speed, &rpm);
                    break;
            }
        }
        end_hid();
    }

    return 0;
}
