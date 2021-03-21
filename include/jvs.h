/**
 * Author: Bobby Dilley
 * Created: 2019
 * SPDX-FileCopyrightText: 2019 Bobby Dilley <bobby@dilley.uk>
 * SPDX-License-Identifier: GPL-3.0-or-later
 **/

#ifndef JVS_H_
#define JVS_H_

#include "constants.h"
#include "config.h"

#define JVS_RETRY_COUNT 3
#define JVS_MAX_PACKET_SIZE 255

typedef struct
{
    unsigned char destination;
    unsigned char length;
    unsigned char data[MAX_PACKET_SIZE];
} JVSPacket;

typedef enum
{
    JVS_STATUS_SUCCESS,
    JVS_STATUS_NOT_FOR_US,
    JVS_STATUS_ERROR,
    JVS_STATUS_ERROR_TIMEOUT,
    JVS_STATUS_ERROR_CHECKSUM,
    JVS_STATUS_ERROR_WRITE_FAIL,
    JVS_STATUS_ERROR_UNSUPPORTED_COMMAND,
} JVSStatus;

typedef struct
{
    unsigned char players;
    unsigned char switches;
    unsigned char coins;
    unsigned char analogueInChannels;
    unsigned char analogueInBits;
    unsigned char rotaryChannels;
    unsigned char keypad;
    unsigned char gunChannels;
    unsigned char gunXBits;
    unsigned char gunYBits;
    unsigned char generalPurposeInputs;
    unsigned char card;
    unsigned char hopper;
    unsigned char generalPurposeOutputs;
    unsigned char analogueOutChannels;
    unsigned char displayOutRows;
    unsigned char displayOutColumns;
    unsigned char displayOutEncodings;
    unsigned char backup;
} JVSCapabilities;

int connectJVS(char *devicePath);
JVSStatus readPacket(JVSPacket *packet);
JVSStatus writePacket(JVSPacket *packet);
int runCommand(JVSPacket *packet, JVSPacket *returnedPacket);

int resetJVS();
int getCapabilities(JVSCapabilities *capabilities);
int decreaseCoins(unsigned char amount, unsigned char slot);
int increaseCoins(unsigned char amount, unsigned char slot);
int getSupported(JVSCapabilities *capabilities, unsigned char *coins, unsigned char *switches, int *analogues);
int getName(char *name);
int getJVSVersion();
int getCommsVersion();
unsigned char getSwitchBytesPerPlayer();

#endif // JVS_H_
