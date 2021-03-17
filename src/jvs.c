/**
 * Author: Bobby Dilley
 * Created: 2019
 * SPDX-FileCopyrightText: 2019 Bobby Dilley <bobby@dilley.uk>
 * SPDX-License-Identifier: GPL-3.0-or-later
 **/
 
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/serial.h>

#include "jvs.h"
#include "device.h"

/* The in and out packets used to read and write to and from*/
JVSPacket inputPacket, outputPacket;

/* The in and out buffer used to read and write to and from */
unsigned char outputBuffer[JVS_MAX_PACKET_SIZE], inputBuffer[JVS_MAX_PACKET_SIZE];

int connectJVS(char *devicePath)
{
	return initDevice(devicePath);
}

int resetJVS()
{
	outputPacket.destination = BROADCAST;

	/* Send the reset command */
	unsigned char resetData[] = {CMD_RESET, CMD_RESET_ARG};
	outputPacket.length = 2;
	memcpy(outputPacket.data, resetData, outputPacket.length);
	if (writePacket(&outputPacket) != JVS_STATUS_SUCCESS)
	{
		printf("Failed to reset device\n");
		return -1;
	}

	usleep(1000 * 1000);

	/* Assign the I/O device ID 0x01 */
	unsigned char deviceAssignData[] = {CMD_ASSIGN_ADDR, DEVICE_ID};
	outputPacket.length = 2;
	memcpy(outputPacket.data, deviceAssignData, outputPacket.length);
	if (!runCommand(&outputPacket, &inputPacket))
	{
		printf("Failed to assign ID to device\n");
		return 0;
	}

	return 1;
}

int getSwitches(char *switches, int players, int bytes)
{
	outputPacket.destination = DEVICE_ID;

	unsigned char data[] = {CMD_READ_SWITCHES, players, bytes};
	outputPacket.length = 3;
	memcpy(outputPacket.data, data, outputPacket.length);
	if (!runCommand(&outputPacket, &inputPacket))
	{
		printf("Failed to send switches question to device\n");
		return 0;
	}

	int i = 2;
	while (i < inputPacket.length)
	{
		switches[i - 2] = inputPacket.data[i];
		i++;
	}
	return 1;
}

int getAnalogue(char *analogues, int channels)
{
	outputPacket.destination = DEVICE_ID;

	unsigned char data[] = {CMD_READ_ANALOGS, channels};
	outputPacket.length = 2;
	memcpy(outputPacket.data, data, outputPacket.length);
	if (!runCommand(&outputPacket, &inputPacket))
	{
		printf("Failed to send analogue question to device\n");
		return 0;
	}

	int i = 2;
	while (i < inputPacket.length)
	{
		analogues[i - 2] = inputPacket.data[i];
		i++;
	}
	return 1;
}

int getLightGun(int *gunPosition, unsigned char player)
{
	outputPacket.destination = DEVICE_ID;

	unsigned char data[] = {CMD_READ_LIGHTGUN, player};
	outputPacket.length = 2;
	memcpy(outputPacket.data, data, outputPacket.length);
	if (!runCommand(&outputPacket, &inputPacket))
	{
		printf("Failed to send light gun question to device\n");
		return 0;
	}

	int i = 1;
	while (i < inputPacket.length)
	{
		gunPosition[i - 2] = inputPacket.data[i] << 8 | inputPacket.data[i + 1];
		i += 2;
	}
	return 1;
}

int getName(char *name)
{
	outputPacket.destination = DEVICE_ID;

	unsigned char data[] = {CMD_REQUEST_ID};
	outputPacket.length = 1;
	memcpy(outputPacket.data, data, outputPacket.length);
	if (!runCommand(&outputPacket, &inputPacket))
	{
		printf("Failed to send get name question to device\n");
		return 0;
	}

	int i = 2;
	while (i < inputPacket.length && inputPacket.data[i] != '\0')
	{
		name[i - 2] = inputPacket.data[i];
		i++;
	}
	name[i - 2] = '\0';
	return 1;
}

int getCapabilities(JVSCapabilities *capabilities)
{
	outputPacket.destination = DEVICE_ID;

	unsigned char data[] = {CMD_CAPABILITIES};
	outputPacket.length = 1;
	memcpy(outputPacket.data, data, outputPacket.length);
	if (!runCommand(&outputPacket, &inputPacket))
	{
		printf("Failed to send capabilities question to device\n");
		return -1;
	}

	int i = 2;
	int finished = 0;

	while (!finished && i < inputPacket.length)
	{
		switch (inputPacket.data[i])
		{
		case CAP_END:
			finished = 1;
			break;
		case CAP_PLAYERS:
			capabilities->players = inputPacket.data[i + 1];
			capabilities->switches = inputPacket.data[i + 2];
			break;
		case CAP_ANALOG_IN:
			capabilities->analogueInChannels = inputPacket.data[i + 1];
			capabilities->analogueInBits = inputPacket.data[i + 2] ? inputPacket.data[i + 2] : 8;
			break;
		case CAP_COINS:
			capabilities->coins = inputPacket.data[i + 1];
			break;
		case CAP_ROTARY:
			capabilities->rotaryChannels = inputPacket.data[i + 1];
			break;
		case CAP_KEYPAD:
			capabilities->keypad = 1;
			break;
		case CAP_LIGHTGUN:
			capabilities->gunXBits = inputPacket.data[i + 1];
			capabilities->gunYBits = inputPacket.data[i + 2];
			capabilities->gunChannels = inputPacket.data[i + 3];
			break;
		case CAP_GPI:
			capabilities->generalPurposeOutputs = inputPacket.data[i + 1] << 8 | inputPacket.data[i + 2];
			break;
		case CAP_CARD:
			capabilities->card = inputPacket.data[i + 1];
			break;
		case CAP_HOPPER:
			capabilities->hopper = inputPacket.data[i + 1];
			break;
		case CAP_GPO:
			capabilities->generalPurposeOutputs = inputPacket.data[i + 1];
			break;
		case CAP_ANALOG_OUT:
			capabilities->analogueOutChannels = inputPacket.data[i + 1];
			break;
		case CAP_DISPLAY:
			capabilities->displayOutColumns = inputPacket.data[i + 1];
			capabilities->displayOutRows = inputPacket.data[i + 2];
			capabilities->displayOutEncodings = inputPacket.data[i + 3];
			break;
		case CAP_BACKUP:
			capabilities->backup = 1;
			break;
		}
		i += 4;
	}
	return 1;
}

int runCommand(JVSPacket *outputPacket, JVSPacket *inputPacket)
{
	int timeout = 3;
	while (timeout > 0)
	{
		JVSStatus writePacketResponse = writePacket(outputPacket);
		if (writePacketResponse != JVS_STATUS_SUCCESS)
		{
			printf("Write Error - Failed to write the packet\n");
			timeout--;
			continue;
		}

		usleep(500);

		JVSStatus readPacketResponse = readPacket(inputPacket);

		if (readPacketResponse != JVS_STATUS_SUCCESS)
		{
			switch (readPacketResponse)
			{
			case JVS_STATUS_ERROR_CHECKSUM:
				printf("Checksum Error - The device replied a non valid packet\n");
				break;
			case JVS_STATUS_ERROR_TIMEOUT:
				printf("Timeout Error - The device did not reply in time\n");
				break;
			default:
				printf("Unhandled Error - Another error occoured\n");
			}
			timeout--;
			continue;
		}

		if (inputPacket->destination != BUS_MASTER)
		{
			printf("Destination Error - This packet is supposed to be for us but it's not\n");
			timeout--;
			continue;
		}

		if (inputPacket->data[0] != STATUS_SUCCESS)
		{
			printf("Status Error - Error with the device");
			timeout--;
			continue;
		}

		if (inputPacket->data[1] != REPORT_SUCCESS)
		{
			printf("Report Error - Error with this command in particular");
			timeout--;
			continue;
		}
		usleep(10 * 1000);

		return 1;
	}

	return 0;
}

/**
 * Read a JVS Packet
 *
 * A single JVS packet is read into the packet pointer
 * after it has been received, unescaped and checked
 * for any checksum errors.
 *
 * @param packet The packet to read into
 */
JVSStatus readPacket(JVSPacket *packet)
{
	int bytesAvailable = 0, escape = 0, phase = 0, index = 0, dataIndex = 0, finished = 0;
	unsigned char checksum = 0x00;

	while (!finished)
	{
		int bytesRead = readBytes(inputBuffer + bytesAvailable, JVS_MAX_PACKET_SIZE - bytesAvailable);

		if (bytesRead < 0)
			return JVS_STATUS_ERROR_TIMEOUT;

		bytesAvailable += bytesRead;

		while ((index < bytesAvailable) && !finished)
		{
			/* If we encounter a SYNC start again */
			if (!escape && (inputBuffer[index] == SYNC))
			{
				phase = 0;
				dataIndex = 0;
				index++;
				continue;
			}

			/* If we encounter an ESCAPE byte escape the next byte */
			if (!escape && inputBuffer[index] == ESCAPE)
			{
				escape = 1;
				index++;
				continue;
			}

			/* Escape next byte by adding 1 to it */
			if (escape)
			{
				inputBuffer[index]++;
				escape = 0;
			}

			/* Deal with the main bulk of the data */
			switch (phase)
			{
			case 0: // If we have not yet got the address
				packet->destination = inputBuffer[index];
				checksum = packet->destination & 0xFF;
				phase++;
				break;
			case 1: // If we have not yet got the length
				packet->length = inputBuffer[index];
				checksum = (checksum + packet->length) & 0xFF;
				phase++;
				break;
			case 2: // If there is still data to read
				if (dataIndex == (packet->length - 1))
				{
					if (checksum != inputBuffer[index])
						return JVS_STATUS_ERROR_CHECKSUM;
					finished = 1;
					break;
				}
				packet->data[dataIndex++] = inputBuffer[index];
				checksum = (checksum + inputBuffer[index]) & 0xFF;
				break;
			default:
				return JVS_STATUS_ERROR;
			}
			index++;
		}
	}

	return JVS_STATUS_SUCCESS;
}

/**
 * Write a JVS Packet
 *
 * A single JVS Packet is written to the arcade
 * system after it has been escaped and had
 * a checksum calculated.
 *
 * @param packet The packet to send
 */
JVSStatus writePacket(JVSPacket *packet)
{
	/* Don't return anything if there isn't anything to write! */
	if (packet->length < 1)
		return JVS_STATUS_SUCCESS;

	/* Get pointer to raw data in packet */
	unsigned char *packetPointer = (unsigned char *)packet;

	/* Add SYNC and reset buffer */
	int checksum = 0;
	int outputIndex = 1;
	outputBuffer[0] = SYNC;

	packet->length++;

	/* Write out entire packet */
	for (int i = 0; i < packet->length + 1; i++)
	{
		if (packetPointer[i] == SYNC || packetPointer[i] == ESCAPE)
		{
			outputBuffer[outputIndex++] = ESCAPE;
			outputBuffer[outputIndex++] = (packetPointer[i] - 1);
		}
		else
		{
			outputBuffer[outputIndex++] = (packetPointer[i]);
		}
		checksum = (checksum + packetPointer[i]) & 0xFF;
	}

	/* Write out escaped checksum */
	if (checksum == SYNC || checksum == ESCAPE)
	{
		outputBuffer[outputIndex++] = ESCAPE;
		outputBuffer[outputIndex++] = (checksum - 1);
	}
	else
	{
		outputBuffer[outputIndex++] = checksum;
	}

	int written = 0, timeout = 0;
	while (written < outputIndex)
	{
		if (written != 0)
			timeout = 0;

		if (timeout > JVS_RETRY_COUNT)
			return JVS_STATUS_ERROR_WRITE_FAIL;

		written += writeBytes(outputBuffer + written, outputIndex - written);
		timeout++;
	}

	return JVS_STATUS_SUCCESS;
}
