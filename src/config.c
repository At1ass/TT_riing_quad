#include "config.h"
#include "toml.h"
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_CONFIG_PATH "/etc/TT_riing_quad/config.toml"

static void
error(const char *msg, const char *msg2)
{
    fprintf(stderr, "ERROR: %s%s\n", msg, msg2?msg2:"");
    exit(1);
}

static void
parse_controller(toml_table_t* config, const char* controller_name)
{
    toml_table_t* controller = toml_table_in(config, controller_name);
    if (!controller) {
        fprintf(stderr, "ERROR: missing [%s]\n", controller_name);
        exit(1);
    }

    toml_datum_t product_id = toml_int_in(controller, "product_id");
    if (!product_id.ok) {
        error("Cannot read controller.product_id", "");
    }
    if (product_id.u.i <= 0) {
        error("controller.product_id must be positive", "");
    }

    toml_datum_t value = toml_int_in(controller, "value");
    if (!value.ok) {
        error("Cannot read controller.value", "");
    }
    if (value.u.i <= 0) {
        error("controller.value must be positive", "");
    }

    printf("Config readed: PID - %04lX, value - %ld\n", (uint64_t)product_id.u.i, value.u.i);

}

static void
fill_config_data(toml_table_t *config, config_t *config_data)
{
    size_t len = config_data->controllers_len;
    toml_array_t* controllers = toml_array_in(config, "controllers");
    if (!controllers) {
        error("Missing controllers aray", "");
        exit(1);
    }

    for (size_t i = 0; i < len; i++) {
        toml_datum_t elem = toml_int_at(controllers, i);
        if (!elem.ok || elem.u.i <= 0) {
            error("Incorrect contoller product_id", "");
        }
        config_data->controllers[i].product_id = elem.u.i;
    }


    toml_array_t* presets = toml_array_in(config, "presets");
    if (!presets) {
        error("Missing presets aray", "");
        exit(1);
    }

    size_t presets_len = toml_array_nelem(presets);
    config_data->presets_len = presets_len;
    config_data->presets = malloc(sizeof(preset_t) * presets_len);
    for (size_t i = 0; i < presets_len; i++) {
        config_data->presets[i].values = malloc(sizeof(uint8_t) * len);
        toml_table_t* elem = toml_table_at(presets, i);
        if (!elem) {
            error("Incorrect preset", "");
            exit(1);
        }

        toml_datum_t name = toml_string_in(elem, "name");
        if (!name.ok) {
            error("Incorrect preset name", "");
            exit(1);
        }
        config_data->presets[i].name = name.u.s;

        toml_array_t* values = toml_array_in(elem, "values");
        if (!values || toml_array_nelem(values) != len) {
            error("Incorrect preset controller values", "");
            exit(1);
        }

        size_t l = toml_array_nelem(values);
        for (size_t j = 0; j < l; j++) {
            toml_datum_t val = toml_int_at(values, j);
            if (!val.ok || val.u.i <= 0) {
                error("Incorrect value", "");
                exit(1);
            }
            config_data->presets[i].values[j] = val.u.i;
        }
    }
}

static void
print_config_data(config_t* config_data)
{
    printf("Controllers:\n");
    for (size_t i = 0; i < config_data->controllers_len; i++) {
        printf("---- %04X\n", config_data->controllers[i].product_id);
    }
    printf("\n");

    printf("Presets:\n");
    for (size_t i = 0; i < config_data->presets_len; i++) {
        printf("----Name: %s\n", config_data->presets[i].name);
        printf("--------[");
        for (size_t j = 0; j < config_data->controllers_len; j++) {
            printf(" %d ", config_data->presets[i].values[j]);
        }
        printf("]\n");
    }
}

static size_t
num_of_controllers(toml_table_t *config)
{
    toml_array_t* controllers = toml_array_in(config, "controllers");
    if (!controllers) {
        error("Missing controllers aray", "");
        exit(1);
    }

    return toml_array_nelem(controllers);
}


config_t *
read_config()
{
    FILE *fp;
    char errbuf[200];
    size_t len;
    config_t *config_data;

    printf("Read conf\n");

    fp = fopen(DEFAULT_CONFIG_PATH, "r");
    if (!fp) {
        error("Cannot open config.toml - ", strerror(errno));
    }

    toml_table_t* config = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!config) {
        error("Cannot parse - ", errbuf);
    }

    len = num_of_controllers(config);
    config_data = malloc(sizeof(config_t));
    config_data->controllers_len = len;
    config_data->controllers = malloc(len * sizeof(controller_t));

    fill_config_data(config, config_data);
    print_config_data(config_data);

    parse_controller(config, "controller");
    parse_controller(config, "controller2");

    toml_free(config);

    return config_data;

}

void
free_config(config_t** config)
{
    config_t* config_data = *(config);
    printf("Free Controllers:\n");
    free(config_data->controllers);

    printf("Free Presets:\n");
    for (size_t i = 0; i < config_data->presets_len; i++) {
        printf("--Free %s", config_data->presets[i].name);
        free(config_data->presets[i].values);
        free(config_data->presets[i].name);
        printf("\n");
    }
    free(config_data->presets);
}
