#include "jvscore.h"

int main(int argc, char **argv)
{
    printf("JVSCore 1.0\n");
    struct uinput_setup usetup;

    parseConfig("/etc/jvscore.conf");

    if (!connectJVS())
    {
        printf("Error connecting to serial\n");
        return 1;
    }

    if (!resetJVS())
    {
        printf("Error resetting jvs...\n");
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
        printf("Error getting name...\n");
        return 1;
    }

    printf("Device Connected: %s\n", name);
    printf("  Players: %d\n", capabilities.players);
    printf("  Switches: %d\n", capabilities.switches);
    printf("  Coins: %d\n", capabilities.coins);
    printf("  Analogue: %d (%d bits)\n", capabilities.analogueInChannels, capabilities.analogueInBits);
    printf("  Rotary: %d\n", capabilities.rotaryChannels);
    printf("  Backup: %d\n", capabilities.backup);

    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    for (int i = 0; i < capabilities.switches * capabilities.players; i++)
    {
        ioctl(fd, UI_SET_KEYBIT, 2 + i);
    }

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x8371;  /* sample vendor */
    usetup.id.product = 0x3551; /* sample product */
    strcpy(usetup.name, name);  //"SEGA ENTERPRISESLTD.;I/O BD JVS;837-13551;Ver1.00;98/10"

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);

    int running = 1;
    while (running)
    {

        char switches[2 * capabilities.players];
        if (!getSwitches(switches, capabilities.players))
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

        for (int i = 0; i < 2 * capabilities.players; i++)
        {
            for (int j = 7; 0 <= j; j--)
                emit(fd, EV_KEY, 2 + (i * 8) + j, (switches[i] >> j) & 0x01);
        }

        emit(fd, EV_SYN, SYN_REPORT, 0);

        usleep(50);
    }

    ioctl(fd, UI_DEV_DESTROY);
    close(fd);

    return 0;
}
