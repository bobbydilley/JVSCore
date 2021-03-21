/**
 * Author: Bobby Dilley
 * Created: 2019
 * SPDX-FileCopyrightText: 2019 Bobby Dilley <bobby@dilley.uk>
 * SPDX-License-Identifier: GPL-3.0-or-later
 **/

#include <linux/uinput.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <math.h>

#include "input.h"

JVSCapabilities *capabilities;
int switchBytes = -1;
int fd = -1;
struct uinput_user_dev usetup;

#define COIN_KEYS 5

void emit(int fd, int type, int code, int val)
{
    struct input_event ie;

    ie.type = type;
    ie.code = code;
    ie.value = val;
    /* timestamp values below are ignored */
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;

    write(fd, &ie, sizeof(ie));
}

int initInput(JVSCapabilities *sentCapabilities, char *name, int analogueFuzz)
{
    capabilities = sentCapabilities;
    div_t switchDiv = div(capabilities->switches, 8);
    switchBytes = switchDiv.quot + (switchDiv.rem ? 1 : 0);

    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    for (int i = 0; i < (8 * switchBytes) * capabilities->players + 8; i++)
    {

        ioctl(fd, UI_SET_KEYBIT, 2 + COIN_KEYS + i);
    }

    ioctl(fd, UI_SET_EVBIT, EV_ABS);
    for (int i = 0; i < capabilities->analogueInChannels; i++)
    {
        ioctl(fd, UI_SET_ABSBIT, i);
    }

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x8371;
    usetup.id.product = 0x3551;
    usetup.id.version = 1;
    strcpy(usetup.name, name);

    for (int i = 0; i < capabilities->analogueInChannels; i++)
    {
        usetup.absmin[i] = 0;
        usetup.absmax[i] = pow(2, capabilities->analogueInBits) - 1;
        usetup.absfuzz[i] = analogueFuzz;
        usetup.absflat[i] = 0;
    }

    if (write(fd, &usetup, sizeof(usetup)) < 0)
        return 0;

    if (ioctl(fd, UI_DEV_CREATE) < 0)
        return 0;

    return 1;
}

int closeInput()
{
    ioctl(fd, UI_DEV_DESTROY);
    return close(fd);
}

int updateSwitches(unsigned char *switches)
{
    for (int i = 0; i < switchBytes * capabilities->players + 1; i++)
    {
        for (int j = 7; 0 <= j; j--)
        {
            int switchNumber = (i * 8) + j;
            emit(fd, EV_KEY, 2 + COIN_KEYS + switchNumber, (switches[i] >> j) & 0x01);
        }
    }
    return 1;
}

int updateAnalogues(int *analogues)
{
    for (int i = 0; i < capabilities->analogueInChannels; i++)
    {
        emit(fd, EV_ABS, i, analogues[i] >> (16 - capabilities->analogueInBits));
    }
    return 1;
}

int emitCoinPress(unsigned char slot)
{
    int key = 2 + (int)slot;
    emit(fd, EV_KEY, key, 1);
    sendUpdate();
    emit(fd, EV_KEY, key, 0);
    sendUpdate();
    return 1;
}

int sendUpdate()
{
    emit(fd, EV_SYN, SYN_REPORT, 0);
    return 1;
}
