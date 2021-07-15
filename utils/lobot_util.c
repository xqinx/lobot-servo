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

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>

#include "lobot_servo/servo.h"
#include "lobot_servo/port.h"

#define VERSION_STRING "1.0"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a[0])))

struct args
{
    uint8_t id;
    const char* dev_path;
    const char* command;
    bool write_enable;
    uint16_t write_val1;
    uint16_t write_val2;
};

static void func_id(struct lobot_port_t* port, struct args* args);
static void func_pos(struct lobot_port_t* port, struct args* args);
static void func_offset(struct lobot_port_t* port, struct args* args);
static void func_limit(struct lobot_port_t* port, struct args* args);
static void func_load(struct lobot_port_t* port, struct args* args);

static void usage(const char* name, const char* fmt, ...)
    __attribute__ ((format(printf, 2, 3)));

static const struct command_table {
    const char* name;
    const char* des;
    void (*func)(struct lobot_port_t* port, struct args* args);
} command_table[] = {
    {"id"    , "Read/Write(-w new_id) servo ID"                      , func_id},
    {"pos"   , "Read/Write(-w angle,time) servo position"            , func_pos},
    {"offset", "Read/Write(-w new_offset) servo angle offset"        , func_offset},
    {"limit" , "Read/Write(-w angle_min,angle_max) servo angle limit", func_limit },
    {"load"  , "Enable([-w 1])/Disable(-w 0) servo load output"      , func_load },
};

static void about(void)
{
    fprintf(stdout,
            "lobot_util "VERSION_STRING
            "\nlobot_utils -- a util program to communicate with lobot serial servos\n"
            "\nReport bugs to <xiao@mogi.io>\n"
           );
}

static void more_help(void)
{
    int num = ARRAY_SIZE(command_table);

    fprintf(stdout,
            "\nCOMMAND:\n"
           );
    for (int i = 0; i < num; ++i) {
        fprintf(stdout,
            "\t%-30s%s\n",
            command_table[i].name,
            command_table[i].des
            );
    }
    fprintf(stdout,
            "\n"
            "Options:\n"
            "\t-i|--id id                    Target servo ID to communicate with, default 254(broadcast)\n"
            "\t-d|--device port              Serial port for Lobot servo, default /dev/tty/USB0\n"
            "\t-w|--write VAL1[,VAL2]        Write VAL1 [and VAL2 if applicable] to command\n"
            "\n"
            "\t-v|--version                  Version information\n"
            "\t-h|--help                     This message\n"
            "\n"
            "When write option is not present, value corresponding to "
            "command will be read and returned, otherwise a write will be "
            "performed instead of a read operation\n"
            "\n"
            "Examples:\n"
            "lobot_util id\n"
            "  read servo ID on default port /dev/ttyUSB0\n"
            "lobot_util id -w 2\n"
            "  set servo ID to 2 for all servos on default port /dev/ttyUSB0\n"
            "lobot_util id -i 1 -w 2 -d /dev/ttyUSB1\n"
            "  change servo ID from 1 to 2 on port /dev/ttyUSB1\n"
            "lobot_util pos -i 1 -w 20 -d /dev/ttyUSB1\n"
            "  move servo (ID==1) on port /dev/ttyUSB1 to position 20\n"
            "lobot_util -i 1 load -w 0\n"
            "  disable(unload) servo (ID==1) output load\n"
           );
}

static void usage(const char* name, const char* fmt, ...)
{
    if (fmt) {
        va_list args;
        va_start(args, fmt);
        fprintf(stderr, "Error: ");
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
        va_end(args);
    }
    fprintf(stdout,
            "Usage: %s COMMAND [-i id] [-w VAL1[,VAL2] [-d port] [-h] [-v]\n"
            ,name);
}

static struct option options[] = 
{
    {"help", no_argument, 0, 'h'},
    {"version", no_argument, 0, 'v'},

    {"device", required_argument, 0, 'd'},
    {"id", required_argument, 0, 'i'},
    {"write", required_argument, 0, 'w'},
};

static void parse_option(struct args* args, int argc, char* argv[])
{
    int opt, opt_index = 0;
    unsigned long temp;
    char* temp_str_end;

    while ((opt = getopt_long(argc, argv, "-:i:d:w:hv",
                    options, &opt_index)) != -1) {
        switch (opt) {
            case 1:
                if(args->command) {
                    /* too many commands */
                    usage(argv[0], "Too many commands");
                    exit(-EINVAL);
                }
                args->command = optarg;
                break;
            case 'i':
                temp= strtoul(optarg, &temp_str_end, 10);
                if(temp > 0xFE || temp_str_end == optarg) {
                    usage(argv[0], "Servo ID range should be [0,254]");
                    exit(-EINVAL);
                }
                args->id = temp;
                break; 
            case 'w':
                temp= strtoul(optarg, &temp_str_end, 10);
                if(temp_str_end == optarg) {
                    usage(argv[0], "Invalid write value1");
                    exit(-EINVAL);
                }
                args->write_val1 = temp;
                if(*temp_str_end == ',') {
                    optarg = temp_str_end+1;
                    temp= strtoul(optarg, &temp_str_end, 10);
                    if(temp_str_end == optarg) {
                        usage(argv[0], "Invalid write value2");
                        exit(-EINVAL);
                    }
                    args->write_val2 = temp;
                }
                if (*temp_str_end != '\0') {
                    usage(argv[0], "Invalid write value(s)");
                    exit(-EINVAL);
                }
                args->write_enable = true;
                break; 
            case 'd':
                args->dev_path = optarg;
                break;
            case ':':
                usage(argv[0], "Missing argument for option %s", argv[optind-1]);
                exit(-EINVAL);
            case 'v':
                about();
                exit(0);
            case 'h':
            default: /* '?' */
                usage(argv[0], NULL);
                more_help();
                exit(0);
        }
    }
}

static void func_id(struct lobot_port_t* port, struct args* args)
{
    uint8_t id_out = 0;
    if (args->write_enable) {
        if(args->write_val1 > 0xFE) {
            fprintf(stderr, "Error: ID should be in range of [0, 254]\n");
        } else {
            lobot_set_id(port, args->id, args->write_val1);
            fprintf(stdout, "=>Write %s: %d\n", args->command, args->write_val1);
        }
    } else {
        lobot_get_id(port, args->id, &id_out);
        fprintf(stdout, "=>Read %s: %d\n", args->command, id_out);
    }
}

static void func_pos(struct lobot_port_t* port, struct args* args)
{
    uint16_t pos_out;
    if (args->write_enable) {
        if((int16_t) args->write_val1 < 0) {
            args->write_val1 = 0;
        }
        if(args->write_val1 > 1000) {
            args->write_val1 = 1000;
        }

        lobot_set_pos(port, args->id, args->write_val1, args->write_val2);
        fprintf(stdout, "=>Write %s: %d,%d\n", args->command,
                args->write_val1, args->write_val2);
    } else {
        lobot_get_pos(port, args->id, &pos_out);
        fprintf(stdout, "=>Read %s: %d\n", args->command, pos_out);
    }
}

static void func_offset(struct lobot_port_t* port, struct args* args)
{
    int8_t offset_out;
    if (args->write_enable) {
        if((int8_t) args->write_val1 < LOBOT_OFFSET_RAW_MIN) {
            args->write_val1 = LOBOT_OFFSET_RAW_MIN;
        }
        if((int8_t) args->write_val1 > LOBOT_OFFSET_RAW_MAX) {
            args->write_val1 = LOBOT_OFFSET_RAW_MAX;
        }

        lobot_set_offset(port, args->id, (int8_t) args->write_val1);
        fprintf(stdout, "=>Write %s: %d\n", args->command,
                (int8_t) args->write_val1);
    } else {
        lobot_get_offset(port, args->id, &offset_out);
        fprintf(stdout, "=>Read %s: %d\n", args->command, offset_out);
    }
}

static void func_limit(struct lobot_port_t* port, struct args* args)
{
    uint16_t angle_min_out, angle_max_out;
    if (args->write_enable) {
        if((int16_t) args->write_val1 < LOBOT_ANGLE_RAW_MIN) {
            args->write_val1 = LOBOT_ANGLE_RAW_MIN;
        }
        if(args->write_val1 > LOBOT_ANGLE_RAW_MAX) {
            args->write_val1 = LOBOT_ANGLE_RAW_MAX;
        }
        if((int16_t) args->write_val2 < LOBOT_ANGLE_RAW_MIN) {
            args->write_val2 = LOBOT_ANGLE_RAW_MIN;
        }
        if(args->write_val2 > LOBOT_ANGLE_RAW_MAX) {
            args->write_val2 = LOBOT_ANGLE_RAW_MAX;
        }

        lobot_set_limit(port, args->id, args->write_val1, args->write_val2);
        fprintf(stdout, "=>Write %s: %d,%d\n", args->command,
                args->write_val1, args->write_val2);
    } else {
        lobot_get_limit(port, args->id, &angle_min_out, &angle_max_out);
        fprintf(stdout, "=>Read %s: %d,%d\n", args->command,
                angle_min_out, angle_max_out);
    }
}

static void func_load(struct lobot_port_t* port, struct args* args)
{
    if (args->write_enable) {
        if(args->write_val1 == 0) {
            lobot_set_load(port, args->id, 0);
            fprintf(stdout, "=>Servo[%d] Unloaded\n", args->id);
        } else {
            lobot_set_load(port, args->id, 1);
            fprintf(stdout, "=>Servo[%d] Loaded\n", args->id);
        }
    } else {
        /* enable load if no write args provided */
        lobot_set_load(port, args->id, 1);
        fprintf(stdout, "=>Servo[%d] Loaded\n", args->id);
    }
}


int main(int argc, char* argv[])
{
    struct lobot_port_t* port;
    struct args args = {0};
    int num = ARRAY_SIZE(command_table);
    int index;
    int ret = 0;

    /* check ENV for LOBOT_DEVICE_PATH */
    args.dev_path = getenv("LOBOT_DEVICE_PATH");
    if(args.dev_path == NULL) {
        args.dev_path = "/dev/ttyUSB0";
    }
    args.write_enable = false;
    args.id = 0xFE;

    parse_option(&args, argc, argv);

    if(!args.command){
        usage(argv[0], "Missing command");
        exit(-EINVAL);
    }

    printf("\nDev:%s\nWrite:%d\nCMD:%s\nID:%#x\nValues:%d,%d\n",
            args.dev_path, args.write_enable,
            args.command, args.id, args.write_val1, args.write_val2);

    port = lobot_port_open(args.dev_path);
    if(!port) {
        fprintf(stderr, "Cannot open port %s\n", args.dev_path);
        exit(-ENODEV);
    }

    for (index = 0; index < num; ++index) {
        if (strncmp(command_table[index].name, args.command,
                    strlen(command_table[index].name)) == 0) {
            break;
        }
    }

    if (index == num) {
        usage(argv[0], "Unrecognized command: %s", args.command);
        ret = -EINVAL;
        goto out;
    }

    command_table[index].func(port, &args);
out:
    lobot_port_close(port);
    exit(ret);
}
