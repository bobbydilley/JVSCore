#include "jvs.h"

int serialIO = -1;

int connectJVS()
{
	if ((serialIO = open(devicePath, O_RDWR | O_NOCTTY | O_SYNC)) < 0)
	{
		printf("Failed to open %s\n", devicePath);
		return 0;
	}

	setSerialAttributes(serialIO, B115200);

	return 1;
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

int readByte(unsigned char *byte)
{
	return read(serialIO, byte, 1);
}

int writeByte(unsigned char byte)
{
	char buffer[] = {0x00};
	buffer[0] = byte;
	return write(serialIO, buffer, sizeof(buffer));
}

int readPacket(JVSPacket *packet)
{
	unsigned char byte = 0;
	int timeout = 5;
	while (byte != SYNC && timeout > 0)
	{
		int n = readByte(&byte);
		if (n == 0)
		{
			timeout -= 1;
		}
	}

	if (timeout == 0)
	{
		return -1;
	}

	readByte(&packet->destination);
	readByte(&packet->length);

	unsigned char checksumComputed = packet->destination + packet->length;

	for (int i = 0; i < packet->length - 1; i++)
	{
		readByte(&packet->data[i]);
		if (packet->data[i] == ESCAPE)
		{
			readByte(&packet->data[i]);
			packet->data[i] += 1;
		}
		checksumComputed = (checksumComputed + packet->data[i]) & 0xFF;
	}
	unsigned char checksumReceived = 0;
	readByte(&checksumReceived);

	if (checksumReceived != checksumComputed)
	{
		printf("Checksum Error - The checksum is not correct\n");
		return 0;
	}

	return 1;
}

int writePacket(JVSPacket *packet)
{
	writeByte(SYNC);
	writeByte(packet->destination);
	writeByte(packet->length + 1);
	unsigned char checksum = packet->destination + packet->length + 1;
	for (int i = 0; i < packet->length; i++)
	{
		if (packet->data[i] == SYNC || packet->data[i] == ESCAPE)
		{
			writeByte(ESCAPE);
			writeByte(packet->data[i] - 1);
		}
		else
		{
			writeByte(packet->data[i]);
		}
		checksum = (checksum + packet->data[i]) & 0xFF;
	}
	writeByte(checksum);
	return 1;
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
