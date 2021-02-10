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
#include <stdlib.h>
#include <stddef.h>

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#include "lobot_servo/port.h"

struct lobot_port_t {
    int fd;
};

struct lobot_port_t* lobot_port_open(const char* dev)
{
    struct termios newtio;

    int fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);

    if(fd < 0) {
        return NULL;
    }

    tcgetattr(fd, &newtio);
    cfmakeraw(&newtio);
    newtio.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;

    tcsetattr(fd, TCSANOW, &newtio);

    struct lobot_port_t* port = malloc(sizeof *port);
    if(port == NULL) {
        close(fd);
        return NULL;
    }

    port->fd = fd;
    return port;
}

int lobot_port_read(struct lobot_port_t* port, uint8_t* buffer, size_t len)
{
    if (port == NULL) {
        return -ENODEV;
    }

    return read(port->fd, buffer, len);
}

int lobot_port_write(struct lobot_port_t* port, uint8_t* buffer, size_t len)
{
    if (port == NULL) {
        return -ENODEV;
    }

    return write(port->fd, buffer, len);
}

void lobot_port_close(struct lobot_port_t* port)
{
    if(port) {
        if(port->fd) {
            close(port->fd);
        }
    }
}
