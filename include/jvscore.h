/**
 * Author: Bobby Dilley
 * Created: 2019
 * SPDX-FileCopyrightText: 2019 Bobby Dilley <bobby@dilley.uk>
 * SPDX-License-Identifier: GPL-3.0-or-later
 **/

#ifndef JVSCORE_H_
#define JVSCORE_H_

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

int switchMappings[] = {BTN_DPAD_UP, BTN_DPAD_DOWN, BTN_DPAD_LEFT, BTN_DPAD_RIGHT, BTN_1, BTN_2, BTN_3, BTN_4, BTN_5, BTN_NORTH, BTN_EAST, BTN_SOUTH, BTN_WEST, BTN_A, BTN_B, BTN_C};
int drivingMappings[] = {BTN_A, BTN_B, BTN_C};
int shootingMappings[] = {BTN_A, BTN_B, BTN_C};

int main();

#endif // JVSCORE_H_
