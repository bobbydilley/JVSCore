/**
 * Author: Bobby Dilley
 * Created: 2019
 * SPDX-FileCopyrightText: 2019 Bobby Dilley <bobby@dilley.uk>
 * SPDX-License-Identifier: GPL-3.0-or-later
 **/

#include <errno.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "input.h"

/* Reserve spaces for coin buttons at the start of the mapping */
#define COIN_KEYS 5
#define SYSTEM_KEYS 8

JVSCapabilities *capabilities;
int fd = -1;
int switchBytes = -1;
struct uinput_user_dev usetup;

/* Mappings for the key presses */
int systemKeys[] = {KEY_F2, KEY_F2, KEY_F2, KEY_F2, KEY_F2, KEY_F2, KEY_F2, KEY_F2};
int coinKeys[] = {KEY_5, KEY_6, KEY_6};
int playerOneKeys[] = {KEY_1, KEY_9, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_LEFTCTRL, KEY_LEFTALT, KEY_SPACE, KEY_LEFTSHIFT, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_V, KEY_V};
int playerTwoKeys[] = {KEY_2, KEY_9, KEY_R, KEY_F, KEY_D, KEY_G, KEY_A, KEY_S, KEY_Q, KEY_W, KEY_I, KEY_J, KEY_J, KEY_L, KEY_L, KEY_L};

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

/**
 * Create a new input device
 * 
 * Creates a new input device from the JVSCapabilities string that is sent.
 * Currently only creates switches coins and analogues.
 * 
 * @param sendCapabilities The capabilities object to create the device from
 * @param name The name to give the input device in linux
 * @param analogueFuzz The amount that the analogue channel must differ by before a report is sent.
 */
int initInput(JVSCapabilities *sentCapabilities, char *name, int analogueFuzz)
{
    capabilities = sentCapabilities;
    div_t switchDiv = div(capabilities->switches, 8);
    switchBytes = switchDiv.quot + (switchDiv.rem ? 1 : 0);

    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    ioctl(fd, UI_SET_EVBIT, EV_KEY);

    // Enable system keys
    for (int i = 0; i < 8; i++)
        ioctl(fd, UI_SET_KEYBIT, systemKeys[i]);

    // Enable coin keys
    for (int i = 0; i < 2; i++)
        ioctl(fd, UI_SET_KEYBIT, coinKeys[i]);

    // Enable player one keys
    for (int i = 0; i < 16; i++)
        ioctl(fd, UI_SET_KEYBIT, playerOneKeys[i]);

    // Enable player two keys
    if (capabilities->players > 1)
    {
        for (int i = 0; i < 16; i++)
            ioctl(fd, UI_SET_KEYBIT, playerTwoKeys[i]);
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

/**
 * Close the input device
 * 
 * Closes the input device to linux
 */
int closeInput()
{
    ioctl(fd, UI_DEV_DESTROY);
    return close(fd);
}

/**
 * Sends an update of the switches to linux
 * 
 * Given a raw bit array taken straight from the JVS IO, this function
 * will loop through all bits and send appropriate key presses
 * 
 * @param switches The raw bit array containing switch values
 */
int updateSwitches(unsigned char *switches)
{
    int byteCounter = 0;

    // Emit the system buttons
    for (int i = 0; i < 8; i++)
        emit(fd, EV_KEY, systemKeys[i], switches[byteCounter] >> (7 - i) & 0x01);
    byteCounter++;

    // First player buttons
    for (int i = 0; i < 8; i++)
        emit(fd, EV_KEY, playerOneKeys[i], switches[byteCounter] >> (7 - i) & 0x01);
    byteCounter++;

    if (switchBytes > 1)
    {
        for (int i = 0; i < 8; i++)
            emit(fd, EV_KEY, playerOneKeys[8 + i], switches[byteCounter] >> (7 - i) & 0x01);
        byteCounter++;
    }

    // If we only have 1 player, just end here
    if (capabilities->players == 1)
        return 1;

    // Second player buttons
    for (int i = 0; i < 8; i++)
        emit(fd, EV_KEY, playerTwoKeys[i], switches[byteCounter] >> (7 - i) & 0x01);
    byteCounter++;

    if (switchBytes > 1)
    {
        for (int i = 0; i < 8; i++)
            emit(fd, EV_KEY, playerTwoKeys[8 + i], switches[byteCounter] >> (7 - i) & 0x01);
        byteCounter++;
    }

    return 1;
}

/**
 * Sends an update of the analogues to linux
 * 
 * Given a raw analogue array taken straight from the JVS IO, this function
 * will loop through all bytes and send appropriate analogue updates
 * 
 * @param switches The raw byte array containing analogue values
 */
int updateAnalogues(int *analogues)
{
    for (int i = 0; i < capabilities->analogueInChannels; i++)
    {
        emit(fd, EV_ABS, i, analogues[i] >> (16 - capabilities->analogueInBits));
    }
    return 1;
}

/**
 * Emit a coin press
 * 
 * Given a slot this will emit a key press
 * used for when JVSCore detects a coin
 * 
 * @param slot Which slot the coin was inserted into
 */
int emitCoinPress(unsigned char slot)
{
    emit(fd, EV_KEY, coinKeys[slot], 1);
    sendUpdate();
    emit(fd, EV_KEY, coinKeys[slot], 0);
    sendUpdate();
    return 1;
}

/**
 * Report the input device update
 * 
 * This is called once every turn to send linux all
 * of the switch and analogue updates in one go
 */
int sendUpdate()
{
    emit(fd, EV_SYN, SYN_REPORT, 0);
    return 1;
}
