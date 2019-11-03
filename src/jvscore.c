#include "jvscore.h"

int main(int argc, char **argv)
{
    printf("JVSCore Device Driver 1.1\n");
    struct uinput_user_dev usetup;
    char *configFilePath = "/etc/jvscore.conf";

    if (!parseConfig(configFilePath))
    {
        printf("Failed to open config file at %s, using default values.\n", configFilePath);
    }

    if (!connectJVS())
    {
        printf("Error connecting to serial\n");
        return 1;
    }

    if (!resetJVS())
    {
        printf("Error resetting jvs\n");
        return 1;
    }

    JVSCapabilities capabilities = {0};
    if (!getCapabilities(&capabilities))
    {
        printf("Error getting capabilities...\n");
        return 1;
    }

    char name[1024];
    if (!getName(name))
    {
        printf("Error getting name of IO board\n");
        return 1;
    }

    printf("Device Connected: %s\n", name);
    printf("  Players: %d\n", capabilities.players);
    printf("  Switches: %d\n", capabilities.switches);
    if (capabilities.coins > 0)
        printf("  Coins: %d\n", capabilities.coins);
    if (capabilities.analogueInChannels > 0)
        printf("  Analogue Inputs: %d channels, %d bits\n", capabilities.analogueInChannels, capabilities.analogueInBits);
    if (capabilities.analogueOutChannels > 0)
        printf("  Analogue Outputs: %d channels\n", capabilities.analogueOutChannels);
    if (capabilities.rotaryChannels > 0)
        printf("  Rotary: %d\n", capabilities.rotaryChannels);
    if (capabilities.keypad > 0)
        printf("  Keypad: %d\n", capabilities.keypad);
    if (capabilities.gunChannels > 0)
        printf("  Lightgun: %d x-bits, %d y-bits, %d channels\n", capabilities.gunXBits, capabilities.gunYBits, capabilities.gunChannels);
    if (capabilities.generalPurposeOutputs > 0)
        printf("  General Purpose Outputs: %d\n", capabilities.generalPurposeOutputs);
    if (capabilities.generalPurposeInputs > 0)
        ("  General Purpose Inputs: %d\n", capabilities.generalPurposeInputs);
    if (capabilities.card > 0)
        printf("  Card: %d\n", capabilities.card);
    if (capabilities.hopper > 0)
        printf("  Hopper: %d\n", capabilities.hopper);
    if (capabilities.displayOutRows > 0)
        printf("  Display: %d rows, %d columns, %d encodings\n", capabilities.displayOutRows, capabilities.displayOutColumns, capabilities.displayOutEncodings);
    if (capabilities.backup > 0)
        printf("  Backup: %d\n", capabilities.backup);

    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    for (int i = 0; i < capabilities.switches * capabilities.players; i++)
    {
        ioctl(fd, UI_SET_KEYBIT, 2 + i);
    }

    ioctl(fd, UI_SET_EVBIT, EV_ABS);

       ioctl(fd, UI_SET_ABSBIT, ABS_X);
    ioctl(fd, UI_SET_ABSBIT, ABS_Y);
    ioctl(fd, UI_SET_ABSBIT, ABS_Z);
    ioctl(fd, UI_SET_ABSBIT, ABS_RX);

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x8371;  /* sample vendor */
    usetup.id.product = 0x3551; /* sample product */
    usetup.id.version = 1;
    strcpy(usetup.name, name); //"SEGA ENTERPRISESLTD.;I/O BD JVS;837-13551;Ver1.00;98/10"

    usetup.absmin[ABS_X] = 43;
    usetup.absmax[ABS_X] = 225;
    usetup.absfuzz[ABS_X] = 0;
    usetup.absflat[ABS_X] = 0;
    usetup.absmin[ABS_Y] = 25;
    usetup.absmax[ABS_Y] = 250;
    usetup.absfuzz[ABS_Y] = 0;
    usetup.absflat[ABS_Y] = 0;

    if (write(fd, &usetup, sizeof(usetup)) < 0)
        return -1;

    if (ioctl(fd, UI_DEV_CREATE) < 0)
        return -1;

    sleep(2);

    int running = 1;
    while (running)
    {

        char switches[2 * capabilities.players];
        if (!getSwitches(switches, capabilities.players))
        {
            printf("Error getting switches...\n");
            break;
        }

        char analogues[1 * capabilities.analogueInChannels];
        if (!getAnalogue(analogues, capabilities.analogueInChannels))
        {
            printf("Error getting switches...\n");
            break;
        }

        for (int i = 0; i < 2 * capabilities.players; i++)
        {
            for (int j = 7; 0 <= j; j--)
            {
                //printf("%d %d\n", (i * 8) + j, (switches[i] >> j) & 0x01);
            }
        }

        for (int i = 0; i < 2 * capabilities.analogueInChannels; i += 2)
        {
            //printf("%d\n", analogues[i] & 0xFF);
        }
        //printf("\n");

        for (int i = 0; i < 2 * capabilities.players; i++)
        {
            for (int j = 7; 0 <= j; j--)
            {
                emit(fd, EV_KEY, 2 + (i * 8) + j, (switches[i] >> j) & 0x01);
            }
        }

        emit(fd, EV_ABS, ABS_X, 225 - (analogues[0] & 0xFF));
        emit(fd, EV_ABS, ABS_Y, 250 - (analogues[2] & 0xFF));
        //emit(fd, EV_ABS, ABS_X, analogues[4] & 0xFF);
        //emit(fd, EV_ABS, ABS_Y, analogues[6] & 0xFF);

        emit(fd, EV_SYN, SYN_REPORT, 0);

        usleep(50);
    }

    ioctl(fd, UI_DEV_DESTROY);
    close(fd);

    return 0;
}
