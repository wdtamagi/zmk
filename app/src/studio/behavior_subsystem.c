/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <pb_encode.h>
#include <zmk/studio/rpc.h>
#include <drivers/behavior.h>

ZMK_RPC_SUBSYSTEM(behaviors)

#define BEHAVIOR_RESPONSE(type, ...) ZMK_RPC_RESPONSE(behaviors, type, __VA_ARGS__)

static bool encode_behavior_summaries(pb_ostream_t *stream, const pb_field_t *field,
                                      void *const *arg) {
    uint32_t i = 0;
    STRUCT_SECTION_FOREACH(zmk_behavior_ref, beh) {
        if (!pb_encode_tag_for_field(stream, field))
            return false;

        if (!pb_encode_varint(stream, i++)) {
            LOG_ERR("Failed to encode behavior ID");
            return false;
        }
    }

    return true;
}

zmk_Response list_all_behaviors(const zmk_Request *req) {
    zmk_behaviors_ListAllBehaviorsResponse beh_resp =
        zmk_behaviors_ListAllBehaviorsResponse_init_zero;
    beh_resp.behaviors.funcs.encode = encode_behavior_summaries;

    return BEHAVIOR_RESPONSE(list_all_behaviors, beh_resp);
}

struct encode_custom_sets_state {
    const struct behavior_parameter_metadata_custom *metadata;
    uint8_t i;
};

static bool encode_value_description_name(pb_ostream_t *stream, const pb_field_t *field,
                                          void *const *arg) {
    struct behavior_parameter_value_metadata *state =
        (struct behavior_parameter_value_metadata *)*arg;

    if (!state->friendly_name) {
        return true;
    }

    if (!pb_encode_tag_for_field(stream, field)) {
        return false;
    }

    return pb_encode_string(stream, state->friendly_name, strlen(state->friendly_name));
}

static bool encode_value_description(pb_ostream_t *stream, const pb_field_t *field,
                                     void *const *arg) {
    struct encode_custom_sets_state *state = (struct encode_custom_sets_state *)*arg;

    const struct behavior_parameter_metadata_custom_set *set = &state->metadata->sets[state->i];

    for (int val_i = 0; val_i < set->values_len; val_i++) {
        const struct behavior_parameter_value_metadata *val = &set->values[val_i];

        if (!((val->position == 0 &&
               field->tag == zmk_behaviors_BehaviorBindingParametersCustomSet_param1_tag) ||
              (val->position == 1 &&
               field->tag == zmk_behaviors_BehaviorBindingParametersCustomSet_param2_tag))) {
            continue;
        }

        if (!pb_encode_tag_for_field(stream, field))
            return false;

        zmk_behaviors_BehaviorParameterValueDescription desc =
            zmk_behaviors_BehaviorParameterValueDescription_init_zero;
        desc.name.funcs.encode = encode_value_description_name;
        desc.name.arg = val;

        switch (val->type) {
        case BEHAVIOR_PARAMETER_VALUE_METADATA_TYPE_VALUE:
            desc.which_value_type = zmk_behaviors_BehaviorParameterValueDescription_constant_tag;
            desc.value_type.constant = val->value;
            break;
        case BEHAVIOR_PARAMETER_VALUE_METADATA_TYPE_RANGE:
            desc.which_value_type = zmk_behaviors_BehaviorParameterValueDescription_range_tag;
            desc.value_type.range.min = val->range.min;
            desc.value_type.range.max = val->range.max;
            break;
        case BEHAVIOR_PARAMETER_VALUE_METADATA_TYPE_STANDARD:
            desc.which_value_type = zmk_behaviors_BehaviorParameterValueDescription_standard_tag;
            desc.value_type.standard = val->standard;
            break;
        default:
            LOG_ERR("Unknown value description type %d", val->type);
            return false;
        }

        if (!pb_encode_submessage(stream, &zmk_behaviors_BehaviorParameterValueDescription_msg,
                                  &desc)) {
            LOG_WRN("Failed to encode submessage!");
            return false;
        }
    }

    return true;
}

static bool encode_custom_sets(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
    struct encode_custom_sets_state *state = (struct encode_custom_sets_state *)*arg;
    bool ret = true;

    for (int i = 0; i < state->metadata->sets_len; i++) {
        if (!pb_encode_tag_for_field(stream, field))
            return false;

        state->i = i;
        zmk_behaviors_BehaviorBindingParametersCustomSet msg =
            zmk_behaviors_BehaviorBindingParametersCustomSet_init_zero;
        msg.param1.funcs.encode = encode_value_description;
        msg.param1.arg = state;
        msg.param2.funcs.encode = encode_value_description;
        msg.param2.arg = state;
        ret = pb_encode_submessage(stream, &zmk_behaviors_BehaviorBindingParametersCustomSet_msg,
                                   &msg);
        if (!ret) {
            LOG_WRN("Failed to encode submessage for set %d", i);
            break;
        }
    }

    return ret;
}

static bool encode_behavior_name(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
    struct zmk_behavior_ref *zbm = (struct zmk_behavior_ref *)*arg;

    if (!pb_encode_tag_for_field(stream, field))
        return false;

    return pb_encode_string(stream, zbm->metadata.friendly_name,
                            strlen(zbm->metadata.friendly_name));
}

static struct encode_custom_sets_state state = {};

zmk_Response get_behavior_details(const zmk_Request *req) {
    uint32_t behavior_id = req->subsystem.behaviors.request_type.get_behavior_details.behavior_id;

    uint16_t zbm_count;
    STRUCT_SECTION_COUNT(zmk_behavior_ref, &zbm_count);
    if (behavior_id < 0 || behavior_id >= zbm_count) {
        return ZMK_RPC_SIMPLE_ERR(GENERIC);
    }

    struct zmk_behavior_ref *zbm;
    STRUCT_SECTION_GET(zmk_behavior_ref, behavior_id, &zbm);
    struct behavior_parameter_metadata desc = {0};
    behavior_get_parameter_domains(zbm->device, &desc);

    zmk_behaviors_GetBehaviorDetailsResponse resp =
        zmk_behaviors_GetBehaviorDetailsResponse_init_zero;
    resp.id = behavior_id;
    resp.friendly_name.funcs.encode = encode_behavior_name;
    resp.friendly_name.arg = zbm;

    switch (desc.type) {
    case BEHAVIOR_PARAMETER_METADATA_STANDARD:
        if (desc.standard.param1 > 0) {
            resp.which_parameters_type = zmk_behaviors_GetBehaviorDetailsResponse_standard_tag;
            resp.parameters_type.standard.param1 = desc.standard.param1;
        }

        if (desc.standard.param2 > 0) {
            resp.which_parameters_type = zmk_behaviors_GetBehaviorDetailsResponse_standard_tag;
            resp.parameters_type.standard.param2 = desc.standard.param2;
        }
        break;
    case BEHAVIOR_PARAMETER_METADATA_CUSTOM:
        state.metadata = desc.custom;

        resp.which_parameters_type = zmk_behaviors_GetBehaviorDetailsResponse_custom_tag;
        resp.parameters_type.custom.param_sets.funcs.encode = encode_custom_sets;
        resp.parameters_type.custom.param_sets.arg = &state;
        break;
    default:
        break;
    }

    return BEHAVIOR_RESPONSE(get_behavior_details, resp);
}

ZMK_RPC_SUBSYSTEM_HANDLER(behaviors, list_all_behaviors, false);
ZMK_RPC_SUBSYSTEM_HANDLER(behaviors, get_behavior_details, true);
