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

    div_t switchDiv = div(capabilities.switches, 8);
    int switchBytes = switchDiv.quot + (switchDiv.rem ? 1 : 0);

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    for (int i = 0; i < switchBytes * capabilities.players + 8; i++)
    {
        ioctl(fd, UI_SET_KEYBIT, 2 + i);
    }

    ioctl(fd, UI_SET_EVBIT, EV_ABS);
    for (int i = 0; i < capabilities.analogueInChannels; i++)
    {
        ioctl(fd, UI_SET_ABSBIT, i);
    }

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x8371;
    usetup.id.product = 0x3551;
    usetup.id.version = 1;
    strcpy(usetup.name, name);

    for (int i = 0; i < capabilities.analogueInChannels; i++)
    {
        usetup.absmin[i] = 0;
        usetup.absmax[i] = pow(2, capabilities.analogueInBits) - 1;
        usetup.absfuzz[i] = analogueFuzz;
        usetup.absflat[i] = 0;
    }

    if (write(fd, &usetup, sizeof(usetup)) < 0)
        return -1;

    if (ioctl(fd, UI_DEV_CREATE) < 0)
        return -1;

    sleep(2);

    int running = 1;
    while (running)
    {

        char switches[switchBytes * capabilities.players + 8];
        if (!getSwitches(switches, capabilities.players, switchBytes))
        {
            printf("Error getting switches...\n");
            break;
        }

        char analogues[2 * capabilities.analogueInChannels];
        if (!getAnalogue(analogues, capabilities.analogueInChannels))
        {
            printf("Error getting switches...\n");
            break;
        }

        for (int i = 0; i < switchBytes * capabilities.players + 8; i++)
        {
            for (int j = 7; 0 <= j; j--)
            {
                emit(fd, EV_KEY, 2 + (i * 8) + j, (switches[i] >> j) & 0x01);
            }
        }

        for (int i = 0; i < capabilities.analogueInChannels; i++)
        {
            emit(fd, EV_ABS, i, ((analogues[(i * 2) + 1] & 0xFF) << (capabilities.analogueInChannels - 8)) + (analogues[(i * 2)] & 0xFF));
        }

        emit(fd, EV_SYN, SYN_REPORT, 0);

        usleep(50);
    }

    ioctl(fd, UI_DEV_DESTROY);
    close(fd);

    return 0;
}
