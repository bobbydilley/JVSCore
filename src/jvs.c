#include "jvs.h"
#include "device.h"

int connectJVS(char *devicePath)
{
	return initDevice(devicePath);
}

int resetJVS()
{
	JVSPacket packet;
	JVSPacket returnedPacket;
	packet.destination = BROADCAST;

	/* Send the reset command */
	unsigned char resetData[] = {CMD_RESET, CMD_RESET_ARG};
	packet.length = 2;
	memcpy(packet.data, resetData, packet.length);
	if (!writePacket(&packet))
	{
		printf("Failed to reset device\n");
		return -1;
	}

	usleep(1000 * 1000);

	/* Assign the I/O device ID 0x01 */
	unsigned char deviceAssignData[] = {CMD_ASSIGN_ADDR, DEVICE_ID};
	packet.length = 2;
	memcpy(packet.data, deviceAssignData, packet.length);
	if (!runCommand(&packet, &returnedPacket))
	{
		printf("Failed to assign ID to device\n");
		return 0;
	}

	return 1;
}

int getSwitches(char *switches, int players, int bytes)
{
	JVSPacket packet;
	JVSPacket returnedPacket;
	packet.destination = DEVICE_ID;

	unsigned char data[] = {CMD_READ_SWITCHES, players, bytes};
	packet.length = 3;
	memcpy(packet.data, data, packet.length);
	if (!runCommand(&packet, &returnedPacket))
	{
		printf("Failed to send switches question to device\n");
		return 0;
	}

	int i = 2;
	while (i < returnedPacket.length)
	{
		switches[i - 2] = returnedPacket.data[i];
		i++;
	}
	return 1;
}

int getAnalogue(char *analogues, int channels)
{
	JVSPacket packet;
	JVSPacket returnedPacket;
	packet.destination = DEVICE_ID;

	unsigned char data[] = {CMD_READ_ANALOGS, channels};
	packet.length = 2;
	memcpy(packet.data, data, packet.length);
	if (!runCommand(&packet, &returnedPacket))
	{
		printf("Failed to send analogue question to device\n");
		return 0;
	}

	int i = 2;
	while (i < returnedPacket.length)
	{
		analogues[i - 2] = returnedPacket.data[i];
		i++;
	}
	return 1;
}

int getName(char *name)
{
	JVSPacket packet;
	JVSPacket returnedPacket;
	packet.destination = DEVICE_ID;

	unsigned char data[] = {CMD_REQUEST_ID};
	packet.length = 1;
	memcpy(packet.data, data, packet.length);
	if (!runCommand(&packet, &returnedPacket))
	{
		printf("Failed to send get name question to device\n");
		return 0;
	}

	int i = 2;
	while (i < returnedPacket.length && returnedPacket.data[i] != '\0')
	{
		name[i - 2] = returnedPacket.data[i];
		i++;
	}
	name[i - 2] = '\0';
	return 1;
}

int getCapabilities(JVSCapabilities *capabilities)
{
	JVSPacket packet;
	JVSPacket returnedPacket;
	packet.destination = DEVICE_ID;

	unsigned char data[] = {CMD_CAPABILITIES};
	packet.length = 1;
	memcpy(packet.data, data, packet.length);
	if (!runCommand(&packet, &returnedPacket))
	{
		printf("Failed to send capabilities question to device\n");
		return -1;
	}

	int i = 2;
	int finished = 0;

	while (!finished && i < returnedPacket.length)
	{
		switch (returnedPacket.data[i])
		{
		case CAP_END:
			finished = 1;
			break;
		case CAP_PLAYERS:
			capabilities->players = returnedPacket.data[i + 1];
			capabilities->switches = returnedPacket.data[i + 2];
			break;
		case CAP_ANALOG_IN:
			capabilities->analogueInChannels = returnedPacket.data[i + 1];
			capabilities->analogueInBits = returnedPacket.data[i + 2] ? returnedPacket.data[i + 2] : 8;
			break;
		case CAP_COINS:
			capabilities->coins = returnedPacket.data[i + 1];
			break;
		case CAP_ROTARY:
			capabilities->rotaryChannels = returnedPacket.data[i + 1];
			break;
		case CAP_KEYPAD:
			capabilities->keypad = 1;
			break;
		case CAP_LIGHTGUN:
			capabilities->gunXBits = returnedPacket.data[i + 1];
			capabilities->gunYBits = returnedPacket.data[i + 2];
			capabilities->gunChannels = returnedPacket.data[i + 3];
			break;
		case CAP_GPI:
			capabilities->generalPurposeOutputs = returnedPacket.data[i + 1] << 8 | returnedPacket.data[i + 2];
			break;
		case CAP_CARD:
			capabilities->card = returnedPacket.data[i + 1];
			break;
		case CAP_HOPPER:
			capabilities->hopper = returnedPacket.data[i + 1];
			break;
		case CAP_GPO:
			capabilities->generalPurposeOutputs = returnedPacket.data[i + 1];
			break;
		case CAP_ANALOG_OUT:
			capabilities->analogueOutChannels = returnedPacket.data[i + 1];
			break;
		case CAP_DISPLAY:
			capabilities->displayOutColumns = returnedPacket.data[i + 1];
			capabilities->displayOutRows = returnedPacket.data[i + 2];
			capabilities->displayOutEncodings = returnedPacket.data[i + 3];
			break;
		case CAP_BACKUP:
			capabilities->backup = 1;
			break;
		}
		i += 4;
	}
	return 1;
}

int runCommand(JVSPacket *packet, JVSPacket *returnedPacket)
{

	writePacket(packet);

	int readPacketResponse = readPacket(returnedPacket);
	if (readPacketResponse == -1)
	{
		printf("Timeout Error - The device did not reply in time\n");
		return 0;
	}

	if (returnedPacket->destination != BUS_MASTER)
	{
		printf("Destination Error - This packet is supposed to be for us but it's not\n");
		return 0;
	}

	if (returnedPacket->data[0] != STATUS_SUCCESS)
	{
		printf("Status Error - Error with the device");
		return 0;
	}

	if (returnedPacket->data[1] != REPORT_SUCCESS)
	{
		printf("Report Error - Error with this command in particular");
		return 0;
	}
	usleep(10 * 1000);
	return 1;
}

int readPacket(JVSPacket *packet)
{

	unsigned char byte = 0;
	int n = readBytes(&byte, 1);
	while (byte != SYNC || n < 1)
	{
		n = readBytes(&byte, 1);
	}

	readBytes(&packet->destination, 1);
	readBytes(&packet->length, 1);
	unsigned char checksumComputed = packet->destination + packet->length;

	unsigned char inputBuffer[MAX_PACKET_SIZE];
	int read = 0;
	while (read < packet->length)
	{
		read += readBytes(inputBuffer + read, packet->length - read);
	}

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

	if (checksumReceived != checksumComputed)
	{
		printf("Error: The checksum is not correct\n");
		return 0;
	}

	return 1;
}

int writePacket(JVSPacket *packet)
{
	/* Don't return anything if there isn't anything to write! */
	if (packet->length < 1)
	{
		return 1;
	}

	int outputIndex = 0;
	unsigned char outputBuffer[MAX_PACKET_SIZE];

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

	if (writeBytes(outputBuffer, outputIndex) < outputIndex)
	{
		printf("Failure: Could not write enough bytes\n");
		return 0;
	}
	return 1;
}
