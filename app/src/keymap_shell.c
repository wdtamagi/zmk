/*
 * Copyright (c) 2024 Pete Johanson
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ctype.h>
#include <stdlib.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/util.h>

#include <zmk/behavior.h>
#include <zmk/keymap.h>
#include <zmk/matrix.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(keymap_shell);

static int get_layer_index_from_name(char *name) {
    for (int i = 0; i < ZMK_KEYMAP_LAYERS_LEN; i++) {
        if (strcmp(zmk_keymap_layer_name(i), name) == 0) {
            return i;
        }
    }

    return -ENODEV;
}

static int16_t get_key_position(char *str) {
    char *rest;
    int16_t key_position = strtoul(str, &rest, 10);

    if (*rest != '\0') {
        return -ENODEV;
    }

    if (key_position >= ZMK_KEYMAP_LEN) {
        return -ENODEV;
    }

    return key_position;
}

static int bindings_get(const struct shell *shell, size_t argc, char **argv) {
    uint16_t key_position = get_key_position(argv[1]);

    if (key_position < 0) {
        shell_print(shell, "Invalid key position");
        return -ENODEV;
    }

    int layer = get_layer_index_from_name(argv[-2]);
    if (layer < 0) {
        shell_print(shell, "Not a layer name");
        return -ENODEV;
    }

    struct zmk_behavior_binding *binding = zmk_keymap_layer_get_binding(layer, key_position);

    shell_print(shell, "Got the binding name: %s with args %d,%d", binding->behavior_dev,
                binding->param1, binding->param2);

    return 0;
}

static int bindings_set(const struct shell *shell, size_t argc, char **argv) {
    uint16_t key_position = get_key_position(argv[1]);

    if (key_position < 0) {
        shell_print(shell, "Invalid key position");
        return -ENODEV;
    }

    int layer = get_layer_index_from_name(argv[-2]);
    if (layer < 0) {
        shell_print(shell, "Not a layer name");
        return -ENODEV;
    }

    char *param1_rest;
    uint32_t param1 = strtoul(argv[3], &param1_rest, 10);

    if (*param1_rest != '\0') {
        shell_print(shell, "Invalid 1st parameter");
        return -ENODEV;
    }

    struct zmk_behavior_binding *binding = zmk_keymap_layer_get_binding(layer, key_position);

    binding->param1 = param1;

    shell_print(shell, "Set the binding name: %s with args %d,%d", binding->behavior_dev,
                binding->param1, binding->param2);

    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(keymap_layer_bindings_commands,
                               SHELL_CMD_ARG(get, NULL, "Get a layer bindings", bindings_get, 2, 0),
                               SHELL_CMD_ARG(set, NULL, "Set a layer bindings", bindings_set, 5, 0),
                               SHELL_SUBCMD_SET_END);

SHELL_STATIC_SUBCMD_SET_CREATE(keymap_layer_commands,
                               SHELL_CMD(bindings, &keymap_layer_bindings_commands,
                                         "Interact with layers", NULL),
                               SHELL_SUBCMD_SET_END);

static void cmd_keymap_layer_get(size_t idx, struct shell_static_entry *entry) {
    /* -1 because the last element in the list is a "list terminator" */
    if (idx < ZMK_KEYMAP_LAYERS_LEN) {
        entry->syntax = zmk_keymap_layer_name(idx);
        entry->handler = NULL;
        entry->subcmd = &keymap_layer_commands;
        entry->help = "Layer commands for this layer";
    } else {
        entry->syntax = NULL;
    }
}

SHELL_DYNAMIC_CMD_CREATE(sub_keymap_layer, cmd_keymap_layer_get);

SHELL_CMD_REGISTER(keymap, &sub_keymap_layer, "Keymap commands", NULL);
