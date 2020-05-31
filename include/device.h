#ifndef DEVICE_H_
#define DEVICE_H_

int initDevice(char *devicePath);
int closeDevice();
int readBytes(char *buffer, int amount);
int writeBytes(char *buffer, int amount);
int drain();

#endif // DEVICE_H_
