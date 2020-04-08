#include "jvscore.h"

#include "jvs.h"
#include "config.h"
#include "input.h"

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
        return EXIT_FAILURE;
    }

    if (!resetJVS())
    {
        printf("Error resetting jvs\n");
        return EXIT_FAILURE;
    }

    JVSCapabilities capabilities = {0};
    if (!getCapabilities(&capabilities))
    {
        printf("Error getting capabilities\n");
        return EXIT_FAILURE;
    }

    char name[1024];
    if (!getName(name))
    {
        printf("Error getting name of IO board\n");
        return EXIT_FAILURE;
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

    if (!initInput(capabilities))
    {
        printf("Failed to initalise inputs\n");
        return EXIT_FAILURE;
    }

    div_t switchDiv = div(capabilities.switches, 8);
    int switchBytes = switchDiv.quot + (switchDiv.rem ? 1 : 0);

    sleep(2);

    int running = 1;
    while (running)
    {
        /* Get and update the switches */
        char switches[switchBytes * capabilities.players + 1];
        if (!getSwitches(switches, capabilities.players, switchBytes))
        {
            printf("Error getting switches...\n");
            break;
        }
        updateSwitches(switches);

        /* Get and update the analogue channels */
        char analogues[2 * capabilities.analogueInChannels];
        if (!getAnalogue(analogues, capabilities.analogueInChannels))
        {
            printf("Error getting switches...\n");
            break;
        }
        updateAnalogues(analogues);

        /* Send the updates to the computer */
        sendUpdates();
        usleep(50);
    }

    ioctl(fd, UI_DEV_DESTROY);
    close(fd);

    return 0;
}
