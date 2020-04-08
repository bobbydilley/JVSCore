#include <termios.h>
#include <fcntl.h>

#include "device.h"

int serialIO = -1;

int initDevice(char *devicePath)
{
    if ((serialIO = open(devicePath, O_RDWR | O_NOCTTY | O_SYNC)) < 0)
    {
        printf("Failed to open %s\n", devicePath);
        return 0;
    }

    /* Setup the serial connection */
    setSerialAttributes(serialIO, B115200);
    setSerialLowLatency(serialIO);

    usleep(100 * 1000); //required to make flush work, for some reason

    tcflush(serialIO, TCIOFLUSH);
    usleep(100 * 1000); //required to make flush work, for some reason

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
    options.c_cc[VTIME] = 100; // Ten seconds (100 deciseconds)

    tcsetattr(fd, TCSANOW, &options);

    ioctl(fd, TIOCMGET, &status);

    status |= TIOCM_DTR;
    status |= TIOCM_RTS;

    ioctl(fd, TIOCMSET, &status);

    usleep(100 * 1000); // 10mS

    return 0;
}

/* Sets the serial port to low latency mode */
int setSerialLowLatency(int fd)
{
    struct serial_struct serial_settings;

    if (ioctl(fd, TIOCGSERIAL, &serial_settings) < 0)
    {
        printf("Serial Error - Failed to read serial settings for low latency mode\n");
        return 0;
    }

    serial_settings.flags |= ASYNC_LOW_LATENCY;
    if (ioctl(fd, TIOCSSERIAL, &serial_settings) < 0)
    {
        printf("Serial Error - Failed to write serial settings for low latency mode\n");
        return 0;
    }
    return 1;
}