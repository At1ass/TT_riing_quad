#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    uint32_t product_id;
} controller_t;

typedef struct
{
    uint8_t* values;
    char* name;
} preset_t;

typedef struct
{
    size_t controllers_len;
    size_t presets_len;
    controller_t *controllers;
    preset_t* presets;
} config_t;

config_t *read_config();
void free_config(config_t** config_data);

#endif /* __CONFIG_H__ */
