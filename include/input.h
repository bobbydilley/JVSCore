#ifndef INPUT_H_
#define INPUT_H_

#include <stdio.h>
#include <string.h>
#include <linux/uinput.h>
#include <unistd.h>

void emit(int fd, int type, int code, int val);

#endif // INPUT_H_
