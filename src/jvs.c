/**
 * Author: Bobby Dilley
 * Created: 2019
 * SPDX-FileCopyrightText: 2019 Bobby Dilley <bobby@dilley.uk>
 * SPDX-License-Identifier: GPL-3.0-or-later
 **/

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
				drain();
				break;
			case JVS_STATUS_ERROR_TIMEOUT:
				printf("Timeout Error - The device did not reply in time\n");
				drain();
				break;
			default:
				printf("Unhandled Error - Another error occoured\n");
				drain();
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
	int read = 0, timeout = 0;

	/* Wait for the SYNC signal */
	read = readBytes(inputBuffer, 1);
	while (inputBuffer[0] != SYNC)
	{
		if (read != 0)
			timeout = 0;

		if (timeout > JVS_RETRY_COUNT)
			return JVS_STATUS_ERROR_TIMEOUT;

		read = readBytes(inputBuffer, 1);
		timeout++;
	}

	/* Read the length and destination */
	read = 0, timeout = 0;
	while (read < 2)
	{
		if (read != 0)
			timeout = 0;

		if (timeout > JVS_RETRY_COUNT)
			return JVS_STATUS_ERROR_TIMEOUT;

		read += readBytes((unsigned char *)packet + read, 2 - read);
		timeout++;
	}

	unsigned char checksumComputed = packet->destination + packet->length;

	/* Read the payload of the packet */
	read = 0, timeout = 0;
	while (read < packet->length)
	{
		if (read != 0)
			timeout = 0;

		if (timeout > JVS_RETRY_COUNT)
			return JVS_STATUS_ERROR_TIMEOUT;

		read += readBytes(inputBuffer + read, packet->length - read);
		timeout++;
	}

	/* Unescape the packet and calculate the checksum */
	int inputIndex = 0;
	for (int i = 0; i < packet->length - 1; i++)
	{
		packet->data[inputIndex] = inputBuffer[i];
		if (packet->data[inputIndex] == ESCAPE)
		{
			i++;
			packet->data[inputIndex] = inputBuffer[i] + 1;
		}
		checksumComputed = (checksumComputed + packet->data[inputIndex]) & 0xFF;
		inputIndex++;
	}

	unsigned char checksumReceived = inputBuffer[packet->length - 1];

	/* Verify checksum */
	if (checksumReceived != checksumComputed)
	{
		return JVS_STATUS_ERROR_CHECKSUM;
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

	int outputIndex = 0;

	outputBuffer[outputIndex] = SYNC;
	outputBuffer[outputIndex + 1] = packet->destination;
	outputBuffer[outputIndex + 2] = packet->length + 1;
	outputIndex += 3;

	unsigned char checksum = packet->destination + packet->length + 1;
	for (int i = 0; i < packet->length; i++)
	{
		if (packet->data[i] == SYNC || packet->data[i] == ESCAPE)
		{
			outputBuffer[outputIndex] = ESCAPE;
			outputBuffer[outputIndex + 1] = (packet->data[i] - 1);
			outputIndex += 2;
		}
		else
		{
			outputBuffer[outputIndex] = (packet->data[i]);
			outputIndex++;
		}
		checksum = (checksum + packet->data[i]) & 0xFF;
	}
	outputBuffer[outputIndex] = checksum;
	outputIndex += 1;

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
