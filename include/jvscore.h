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

#include "input.h"
#include "jvs.h"
#include "config.h"

int main(int argc, char **argv);

#endif // JVSCORE_H_
