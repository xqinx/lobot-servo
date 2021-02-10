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

#include <stdint.h>
#include <stddef.h>

#include "lobot_servo/servo.h"
#include "lobot_servo/port.h"

#define PACKET_INDEX_HEADER 0
#define PACKET_INDEX_ID     2
#define PACKET_INDEX_LEN    3
#define PACKET_INDEX_CMD    4
#define PACKET_INDEX_PARAM  5

/* Total packet length */
#define PACKET_LEN_0       6        /* zero parameter */
#define PACKET_LEN_1       7        /* 1 uint8 parameter */
#define PACKET_LEN_2       8        /* 1 uint16 parameter */
#define PACKET_LEN_4       10       /* 2 uint16 parameters */

typedef enum {
    LOBOT_CMD_MOVE_TIME_WRITE      = 1,
    LOBOT_CMD_MOVE_TIME_READ       = 2,
    LOBOT_CMD_MOVE_TIME_WAIT_WRITE = 7,
    LOBOT_CMD_MOVE_TIME_WAIT_READ  = 8,
    LOBOT_CMD_MOVE_START           = 11,
    LOBOT_CMD_MOVE_STOP            = 12,
    LOBOT_CMD_ID_WRITE             = 13,
    LOBOT_CMD_ID_READ              = 14,
    LOBOT_CMD_ANGLE_OFFSET_ADJUST  = 17,
    LOBOT_CMD_ANGLE_OFFSET_WRITE   = 18,
    LOBOT_CMD_ANGLE_OFFSET_READ    = 19,
    LOBOT_CMD_ANGLE_LIMIT_WRITE    = 20,
    LOBOT_CMD_ANGLE_LIMIT_READ     = 21,
    LOBOT_CMD_VIN_LIMIT_WRITE      = 22,
    LOBOT_CMD_VIN_LIMIT_READ       = 23,
    LOBOT_CMD_TEMP_MAX_LIMIT_WRITE = 24,
    LOBOT_CMD_TEMP_MAX_LIMIT_READ  = 25,
    LOBOT_CMD_TEMP_READ            = 26,
    LOBOT_CMD_VIN_READ             = 27,
    LOBOT_CMD_POS_READ             = 28,
    LOBOT_CMD_OR_MOTOR_MODE_WRITE  = 29,
    LOBOT_CMD_OR_MOTOR_MODE_READ   = 30,
    LOBOT_CMD_LOAD_OR_UNLOAD_WRITE = 31,
    LOBOT_CMD_LOAD_OR_UNLOAD_READ  = 32,
    LOBOT_CMD_LED_CTRL_WRITE       = 33,
    LOBOT_CMD_LED_CTRL_READ        = 34,
    LOBOT_CMD_LED_ERROR_WRITE      = 35,
    LOBOT_CMD_LED_ERROR_READ       = 36,
} cmd_t;

static const uint8_t LOBOT_FRAME_HEADER = 0x55;

#define LOW_BYTE(a) ((uint8_t)((a) & 0xFF))
#define HIGH_BYTE(a) ((uint8_t)(((a) >> 8) & 0xFF))

static uint8_t check_sum(uint8_t *buffer)
{
    uint8_t checksum = 0;
    int i;
    for (i = PACKET_INDEX_ID; i < (PACKET_INDEX_ID + buffer[PACKET_INDEX_LEN]); ++i) {
        checksum += buffer[i];
    }
    return ~(checksum & 0xFF);
}

static void lobot_packet_0(uint8_t id, cmd_t cmd, uint8_t *buffer)
{
    buffer[PACKET_INDEX_HEADER] = LOBOT_FRAME_HEADER;
    buffer[PACKET_INDEX_HEADER+1] = LOBOT_FRAME_HEADER;
    buffer[PACKET_INDEX_ID] = id;
    buffer[PACKET_INDEX_LEN] = PACKET_LEN_0 - 3;
    buffer[PACKET_INDEX_CMD] = (uint8_t)cmd;
    buffer[PACKET_LEN_0 - 1] = check_sum(buffer);
}

static void lobot_packet_1(uint8_t id, cmd_t cmd, uint8_t param, uint8_t *buffer)
{
    buffer[PACKET_INDEX_HEADER] = LOBOT_FRAME_HEADER;
    buffer[PACKET_INDEX_HEADER+1] = LOBOT_FRAME_HEADER;
    buffer[PACKET_INDEX_ID] = id;
    buffer[PACKET_INDEX_LEN] = PACKET_LEN_1 - 3;
    buffer[PACKET_INDEX_CMD] = (uint8_t)cmd;
    buffer[PACKET_INDEX_PARAM] = param;
    buffer[PACKET_LEN_1 - 1] = check_sum(buffer);
}

static void lobot_packet_4(uint8_t id, cmd_t cmd, uint16_t v1, uint16_t v2, uint8_t *buffer)
{
    buffer[PACKET_INDEX_HEADER] = LOBOT_FRAME_HEADER;
    buffer[PACKET_INDEX_HEADER+1] = LOBOT_FRAME_HEADER;
    buffer[PACKET_INDEX_ID] = id;
    buffer[PACKET_INDEX_LEN] = PACKET_LEN_4 - 3;
    buffer[PACKET_INDEX_CMD] = (uint8_t)cmd;
    buffer[PACKET_INDEX_PARAM] = LOW_BYTE(v1);
    buffer[PACKET_INDEX_PARAM+1] = HIGH_BYTE(v1);
    buffer[PACKET_INDEX_PARAM+2] = LOW_BYTE(v2);
    buffer[PACKET_INDEX_PARAM+3] = HIGH_BYTE(v2);
    buffer[PACKET_LEN_4 - 1] = check_sum(buffer);
}

lobot_error_t lobot_set_id(struct lobot_port_t *port, uint8_t id, uint8_t new_id)
{
    uint8_t buffer[PACKET_LEN_1];

    if (port == NULL) {
        return LOBOT_BAD_PORT;
    }

    lobot_packet_1(id, LOBOT_CMD_ID_WRITE, new_id, buffer);

    lobot_port_write(port, buffer, PACKET_LEN_1);

    return LOBOT_OK;
}

lobot_error_t lobot_get_id(struct lobot_port_t *port, uint8_t id, uint8_t* id_out)
{
    uint8_t buffer[PACKET_LEN_1];

    if (port == NULL) {
        return LOBOT_BAD_PORT;
    }

    lobot_packet_0(id, LOBOT_CMD_ID_READ, buffer);
    lobot_port_write(port, buffer, PACKET_LEN_0);

    lobot_port_read(port, buffer, PACKET_LEN_1);
    if(check_sum(buffer) != buffer[PACKET_INDEX_ID + buffer[PACKET_INDEX_LEN]]){
        return LOBOT_BAD_CHKSUM;
    }

    *id_out = buffer[PACKET_INDEX_PARAM];

    return LOBOT_OK;
}

lobot_error_t lobot_set_pos(struct lobot_port_t *port, uint8_t id, uint16_t position, uint16_t time)
{
    uint8_t buffer[PACKET_LEN_4];

    if (port == NULL) {
        return LOBOT_BAD_PORT;
    }
    if(position > LOBOT_ANGLE_RAW_MAX) {
        position = LOBOT_ANGLE_RAW_MAX;
    }
    if(time > LOBOT_MOVETIME_MS_MAX) {
        position = LOBOT_MOVETIME_MS_MAX;
    }

    lobot_packet_4(id, LOBOT_CMD_MOVE_TIME_WRITE, position, time, buffer);

    lobot_port_write(port, buffer, PACKET_LEN_4);

    return LOBOT_OK;
}

lobot_error_t lobot_get_pos(struct lobot_port_t *port, uint8_t id, uint16_t* pos_out)
{
    uint8_t buffer[PACKET_LEN_2];

    if (port == NULL) {
        return LOBOT_BAD_PORT;
    }

    lobot_packet_0(id, LOBOT_CMD_POS_READ, buffer);
    lobot_port_write(port, buffer, PACKET_LEN_0);

    lobot_port_read(port, buffer, PACKET_LEN_2);
    if(check_sum(buffer) != buffer[PACKET_INDEX_ID + buffer[PACKET_INDEX_LEN]]){
        return LOBOT_BAD_CHKSUM;
    }

    *pos_out = *((uint16_t*)(&buffer[PACKET_INDEX_PARAM]));

    return LOBOT_OK;
}

lobot_error_t lobot_set_offset(struct lobot_port_t *port, uint8_t id, int8_t offset)
{
    uint8_t buffer[PACKET_LEN_1];

    if (port == NULL) {
        return LOBOT_BAD_PORT;
    }

    if(offset < -125) {
        offset = -125;
    }
    if(offset > 125) {
        offset = 125;
    }

    lobot_packet_1(id, LOBOT_CMD_ANGLE_OFFSET_ADJUST, offset, buffer);
    lobot_port_write(port, buffer, PACKET_LEN_1);

    lobot_packet_0(id, LOBOT_CMD_ANGLE_OFFSET_WRITE, buffer);
    lobot_port_write(port, buffer, PACKET_LEN_0);

    return LOBOT_OK;
}

lobot_error_t lobot_get_offset(struct lobot_port_t *port, uint8_t id, int8_t* offset_out)
{
    uint8_t buffer[PACKET_LEN_1];

    if (port == NULL) {
        return LOBOT_BAD_PORT;
    }

    lobot_packet_0(id, LOBOT_CMD_ANGLE_OFFSET_READ, buffer);
    lobot_port_write(port, buffer, PACKET_LEN_0);

    lobot_port_read(port, buffer, PACKET_LEN_1);
    if(check_sum(buffer) != buffer[PACKET_INDEX_ID + buffer[PACKET_INDEX_LEN]]){
        return LOBOT_BAD_CHKSUM;
    }

    *offset_out = buffer[PACKET_INDEX_PARAM];

    return LOBOT_OK;
}

lobot_error_t lobot_set_limit(struct lobot_port_t *port, uint8_t id, uint16_t min, uint16_t max)
{
    uint8_t buffer[PACKET_LEN_4];

    if (port == NULL) {
        return LOBOT_BAD_PORT;
    }
    if(min > LOBOT_ANGLE_RAW_MAX) {
        min = LOBOT_ANGLE_RAW_MAX;
    }
    if(max > LOBOT_ANGLE_RAW_MAX) {
        max = LOBOT_ANGLE_RAW_MAX;
    }

    lobot_packet_4(id, LOBOT_CMD_ANGLE_LIMIT_WRITE, min, max, buffer);

    lobot_port_write(port, buffer, PACKET_LEN_4);

    return LOBOT_OK;
}

lobot_error_t lobot_get_limit(struct lobot_port_t *port, uint8_t id, uint16_t* min_out, uint16_t* max_out)
{
    uint8_t buffer[PACKET_LEN_4];

    if (port == NULL) {
        return LOBOT_BAD_PORT;
    }

    lobot_packet_0(id, LOBOT_CMD_ANGLE_LIMIT_READ, buffer);
    lobot_port_write(port, buffer, PACKET_LEN_0);

    lobot_port_read(port, buffer, PACKET_LEN_4);
    if(check_sum(buffer) != buffer[PACKET_INDEX_ID + buffer[PACKET_INDEX_LEN]]){
        return LOBOT_BAD_CHKSUM;
    }

    *min_out = *((uint16_t*)(&buffer[PACKET_INDEX_PARAM]));
    *max_out = *((uint16_t*)(&buffer[PACKET_INDEX_PARAM+2]));

    return LOBOT_OK;
}

lobot_error_t lobot_set_load(struct lobot_port_t *port, uint8_t id, uint8_t enable_load)
{
    uint8_t buffer[PACKET_LEN_1];

    if (port == NULL) {
        return LOBOT_BAD_PORT;
    }

    if(enable_load != 0) {
        enable_load = 1;
    }

    lobot_packet_1(id, LOBOT_CMD_LOAD_OR_UNLOAD_WRITE, enable_load, buffer);

    lobot_port_write(port, buffer, PACKET_LEN_1);

    return LOBOT_OK;
}
