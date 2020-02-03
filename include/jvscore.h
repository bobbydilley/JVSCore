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

#include "jvs.h"
#include "config.h"

int switchMappings[] = {BTN_DPAD_UP, BTN_DPAD_DOWN, BTN_DPAD_LEFT, BTN_DPAD_RIGHT, BTN_1, BTN_2, BTN_3, BTN_4, BTN_5, BTN_NORTH, BTN_EAST, BTN_SOUTH, BTN_WEST, BTN_A, BTN_B, BTN_C};
int drivingMappings[] = {BTN_A, BTN_B, BTN_C};
int shootingMappings[] = {BTN_A, BTN_B, BTN_C};

int main(int argc, char **argv);
void emit(int fd, int type, int code, int val);

#endif // JVSCORE_H_
