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

	/* Assign the I/O device ID 0x01 */
	unsigned char deviceAssignData[] = {CMD_ASSIGN_ADDR, DEVICE_ID};
	packet.length = 2;
	memcpy(packet.data, deviceAssignData, packet.length);
	if (!runCommand(&packet, &returnedPacket))
	{
		printf("Failed to assign ID to device\n");
		return -1;
	}

	return 1;
}

int getSwitches(char *switches, int players)
{
	JVSPacket packet;
	JVSPacket returnedPacket;
	packet.destination = DEVICE_ID;

	unsigned char data[] = {CMD_READ_SWITCHES, players, 2};
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
	while (i < returnedPacket.length && returnedPacket.data[i] != 0x00)
	{
		name[i - 2] = returnedPacket.data[i];
		i++;
	}
	name[i + 2] = 0x00;
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
		}
		i += 4;
	}
	return 1;
}

int runCommand(JVSPacket *packet, JVSPacket *returnedPacket)
{

	writePacket(packet);
	readPacket(returnedPacket);

	if (returnedPacket->destination != BUS_MASTER)
	{
		printf("This packet is supposed to be for us but it's not\n");
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

	return 1;
}

unsigned char readByte()
{
	unsigned char buffer[] = {
		0x00};

	int n = -1;

	while (n < 1)
	{
		n = read(serialIO, buffer, 1);
	}
	usleep(50);

	if (buffer[0] == ESCAPE)
	{
		n = -1;
		while (n < 1)
		{
			n = read(serialIO, buffer, 1);
		}
		usleep(50);

		return buffer[0] + 1;
	}

	return buffer[0];
}

void writeByte(unsigned char byte)
{
	if (byte == SYNC || byte == ESCAPE)
	{
		unsigned char buffer[] = {
			ESCAPE};

		usleep(50);

		int n = write(serialIO, buffer, sizeof(buffer));
		if (n != 1)
		{
			printf("Error from write: %d, %d\n", n, errno);
		}
		byte -= 1;
	}

	unsigned char buffer[] = {
		byte};
	usleep(50);
	int n = write(serialIO, buffer, sizeof(buffer));
	if (n != 1)
	{
		printf("Error from write: %d, %d\n", n, errno);
	}
}

int readPacket(JVSPacket *packet)
{
	unsigned char byte = 0;
	while (byte != SYNC)
	{
		byte = readByte();
	}

	packet->destination = readByte();
	packet->length = readByte();

	unsigned char checksumComputed = packet->destination + packet->length;

	for (int i = 0; i < packet->length - 1; i++)
	{
		packet->data[i] = readByte();
		checksumComputed = (checksumComputed + packet->data[i]) & 0xFF;
	}
	unsigned char checksumReceived = readByte();

	if (checksumReceived != checksumComputed)
	{
		printf("Error checksum is not correct.\n");
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
		writeByte(packet->data[i]);
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

	usleep(10000); // 10mS

	return 0;
}
