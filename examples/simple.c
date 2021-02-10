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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "lobot_servo/servo.h"
#include "lobot_servo/port.h"

struct lobot_port_t* port;

void term_handler(int sig)
{
    printf("closing...\n");
    lobot_port_close(port);
}

int main(void)
{
    struct sigaction act;
    act.sa_handler = term_handler;
    sigemptyset(&act.sa_mask);
    sigaction (SIGTERM, &act, NULL);

    port = lobot_port_open("/dev/ttyUSB0");

    if(!port) exit(-1);

    int t = 0;
    int delta = 1;
    while(1) {
        t += delta;
        if(t >= 1000 || t < 0) delta = -delta;
        printf("pos:%d\n", t);
        lobot_set_pos(port, 1, t, 0);
        usleep(800);
    }

    lobot_port_close(port);
}
