/******************************************************************************
 * Copyright 2021 Mogi LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *****************************************************************************/

#ifndef MOGI_LOBOT__SERVO_H_
#define MOGI_LOBOT__SERVO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "port.h"

#define LOBOT_ANGLE_RAW_MIN (0)
#define LOBOT_ANGLE_RAW_MAX (1000)
#define LOBOT_MOVETIME_MS_MAX (30000)
#define LOBOT_OFFSET_RAW_MIN (-125)
#define LOBOT_OFFSET_RAW_MAX (125)

typedef enum {
    LOBOT_OK = 0,
    LOBOT_BAD_PORT = -1,
    LOBOT_BAD_CHKSUM = -2,
} lobot_error_t;

lobot_error_t lobot_get_id(struct lobot_port_t *port, uint8_t id, uint8_t* id_out);
lobot_error_t lobot_set_id(struct lobot_port_t *port, uint8_t id, uint8_t new_id);

lobot_error_t lobot_get_pos(struct lobot_port_t *port, uint8_t id, uint16_t* pos_out);
lobot_error_t lobot_set_pos(struct lobot_port_t *port, uint8_t id, uint16_t position, uint16_t time);

lobot_error_t lobot_get_offset(struct lobot_port_t *port, uint8_t id, int8_t* offset_out);
lobot_error_t lobot_set_offset(struct lobot_port_t *port, uint8_t id, int8_t offset);

lobot_error_t lobot_get_limit(struct lobot_port_t *port, uint8_t id, uint16_t* min_out, uint16_t* max_out);
lobot_error_t lobot_set_limit(struct lobot_port_t *port, uint8_t id, uint16_t min, uint16_t max);

lobot_error_t lobot_set_load(struct lobot_port_t *port, uint8_t id, uint8_t enable_load);

#ifdef __cplusplus
}
#endif

#endif
