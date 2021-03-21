/**
 * Author: Bobby Dilley
 * Created: 2019
 * SPDX-FileCopyrightText: 2019 Bobby Dilley <bobby@dilley.uk>
 * SPDX-License-Identifier: GPL-3.0-or-later
 **/

#ifndef DEVICE_H_
#define DEVICE_H_

int initDevice(char *devicePath);
int closeDevice();
int readBytes(unsigned char *buffer, int amount);
int writeBytes(unsigned char *buffer, int amount);

#endif // DEVICE_H_
