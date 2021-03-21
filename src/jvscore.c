/**
 * Author: Bobby Dilley
 * Created: 2019
 * SPDX-FileCopyrightText: 2019 Bobby Dilley <bobby@dilley.uk>
 * SPDX-License-Identifier: GPL-3.0-or-later
 **/

#include <stdio.h>
#include <string.h>
#include <linux/uinput.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>

#include "jvs.h"
#include "config.h"
#include "input.h"
#include "version.h"

int main()
{
    printf("JVSCore Device Driver Version %s\n", PROJECT_VER);

    char *configFilePath = "/etc/jvscore.conf";

    JVSConfig config = {0};
    if (!parseConfig(configFilePath, &config))
    {
        printf("Failed to open config file at %s, using default values.\n", configFilePath);
    }

    if (!connectJVS(config.devicePath))
    {
        printf("Error connecting to serial\n");
        return EXIT_FAILURE;
    }

    if (!resetJVS())
    {
        printf("Error resetting jvs\n");
        return EXIT_FAILURE;
    }

    JVSCapabilities capabilities = {0};
    if (!getCapabilities(&capabilities))
    {
        printf("Error getting capabilities\n");
        return EXIT_FAILURE;
    }

    char name[1024];
    if (!getName(name))
    {
        printf("Error getting name of IO board\n");
        return EXIT_FAILURE;
    }

    printf("Device Connected: %s\n", name);
    if (capabilities.players > 0)
        printf("  Players: %d\n", capabilities.players);
    if (capabilities.switches > 0)
        printf("  Switches Per Player: %d\n", capabilities.switches);
    if (capabilities.coins > 0)
        printf("  Coins: %d\n", capabilities.coins);
    if (capabilities.analogueInChannels > 0)
        printf("  Analogue Inputs: %d channels, %d bits\n", capabilities.analogueInChannels, capabilities.analogueInBits);
    if (capabilities.analogueOutChannels > 0)
        printf("  Analogue Outputs: %d channels\n", capabilities.analogueOutChannels);
    if (capabilities.rotaryChannels > 0)
        printf("  Rotary: %d\n", capabilities.rotaryChannels);
    if (capabilities.keypad > 0)
        printf("  Keypad: %d\n", capabilities.keypad);
    if (capabilities.gunChannels > 0)
        printf("  Lightgun: %d x-bits, %d y-bits, %d channels\n", capabilities.gunXBits, capabilities.gunYBits, capabilities.gunChannels);
    if (capabilities.generalPurposeOutputs > 0)
        printf("  General Purpose Outputs: %d\n", capabilities.generalPurposeOutputs);
    if (capabilities.generalPurposeInputs > 0)
        printf("  General Purpose Inputs: %d\n", capabilities.generalPurposeInputs);
    if (capabilities.card > 0)
        printf("  Card: %d\n", capabilities.card);
    if (capabilities.hopper > 0)
        printf("  Hopper: %d\n", capabilities.hopper);
    if (capabilities.displayOutRows > 0)
        printf("  Display: %d rows, %d columns, %d encodings\n", capabilities.displayOutRows, capabilities.displayOutColumns, capabilities.displayOutEncodings);
    if (capabilities.backup > 0)
        printf("  Backup: %d\n", capabilities.backup);

    if (!initInput(&capabilities, name, config.analogueFuzz))
    {
        printf("Failed to initalise inputs\n");
        return EXIT_FAILURE;
    }

    sleep(1);

    unsigned char coins[capabilities.coins];
    unsigned char switches[getSwitchBytesPerPlayer() * capabilities.players + 1];
    int analogues[capabilities.analogueInChannels];

    int running = 1;
    while (running)
    {
        /* Request all supported functions from the JVS IO */
        if (!getSupported(&capabilities, coins, switches, analogues))
        {
            printf("Error: Failed to request from the JVS IO\n");
            running = 0;
        }

        /* See if we need to press any keys for a coin update */
        for (int i = 0; i < capabilities.coins; i++)
        {
            if (coins[i] > 0)
            {
                emitCoinPress(i);
                decreaseCoins(coins[i], (unsigned char)i + 1);
            }
        }

        /* Update the switches */
        if (capabilities.switches > 0)
        {
            updateSwitches(switches);
        }

        /* Update the analogue channels */
        if (capabilities.analogueInChannels > 0)
        {
            updateAnalogues(analogues);
        }

        /* Send the updates to the computer */
        sendUpdate();
    }

    closeInput();

    return EXIT_FAILURE;
}
