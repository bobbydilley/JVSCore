/**
 * Author: Bobby Dilley
 * Created: 2019
 * SPDX-FileCopyrightText: 2019 Bobby Dilley <bobby@dilley.uk>
 * SPDX-License-Identifier: GPL-3.0-or-later
 **/

#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <linux/serial.h>
#include <termios.h>

#include "device.h"

int serialIO = -1;

/* Sets the configuration of the serial port */
int setSerialAttributes(int fd, int myBaud)
{
    struct termios options;
    int status;
    tcgetattr(fd, &options);

    cfmakeraw(&options);
    cfsetispeed(&options, myBaud);
    cfsetospeed(&options, myBaud);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;

    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 3; // One seconds (10 deciseconds)

    tcsetattr(fd, TCSANOW, &options);

    ioctl(fd, TIOCMGET, &status);

    status |= TIOCM_DTR;
    status |= TIOCM_RTS;

    ioctl(fd, TIOCMSET, &status);

    usleep(100 * 1000); // 10mS

    struct serial_struct serial_settings;

    ioctl(fd, TIOCGSERIAL, &serial_settings);

    serial_settings.flags |= ASYNC_LOW_LATENCY;
    ioctl(fd, TIOCSSERIAL, &serial_settings);

    tcflush(serialIO, TCIOFLUSH);
    usleep(100 * 1000); // Required to make flush work, for some reason

    return 0;
}

int initDevice(char *devicePath)
{
    if ((serialIO = open(devicePath, O_RDWR | O_NOCTTY)) < 0)
    {
        printf("Failed to open %s\n", devicePath);
        return 0;
    }

    /* Setup the serial connection */
    setSerialAttributes(serialIO, B115200);

    return 1;
}

int readBytes(char *buffer, int amount)
{
    return read(serialIO, buffer, amount);
}

int writeBytes(char *buffer, int amount)
{
    return write(serialIO, buffer, amount);
}

int drain()
{
    usleep(100 * 1000);
    return tcflush(serialIO, TCIOFLUSH);
}
