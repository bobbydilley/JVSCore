[![Actions Status](https://github.com/bobbydilley/JVSCore-Public/workflows/Build/badge.svg)](https://github.com/bobbydilley/JVSCore-Public/actions)

# JVSCore

JVSCore is a user space driver for using JVS I/O boards with Linux. It requires a USB RS485 converter wired to the JVS I/O.

The JVSCore device driver currently supports the following features of a JVS I/O:

- Coins
- Switches
- Analogue Inputs

## Installation

Installation is done from the git repository as follows:

```
sudo apt install build-essential cmake git
git clone https://github.com/bobbydilley/JVSCore
cd JVSCore
make
sudo make install
```

## Cable

I'd recommend watching the below video from the TecknoGods about how to create a JVS cable.

https://www.youtube.com/watch?v=kqXEYtvGzno

| RS485 Adapter Side | USB To Arcade Side |
| ------------------ | ------------------ |
| B-                 | DATA- (White)      |
| A+                 | DATA+ (Green)      |
| Not Required       | VCC (Red)          |
| GND                | GND (Black)        |

## Command Line Usage

To start JVSCore in the terminal to view debug messages, you can start it by typing the following:

```
sudo jvscore
```

There are various command line arguments that can be used to configure JVSCore. If running in the background as a service, these can be changed with the settings file as well.

```
Options:
	--disable-analogue     Disables analogue reading
	--analogue-fuzz        Specifies the analogue fuzz value
	--device-path          Specifies the RS485 device path
```

## Settings file

All settings can be set by changing values in the `/etc/jvscore.conf` settings file.

- The default RS485 converter device path can be changed using the `DEVICE_PATH` keyword.
- The default analogue fuzz value can be changed using the `ANALOGUE_FUZZ` keyword. Fuzz is how much the analogue value has to change by before it is reported to the computer. This is useful if you've got super noisy pots on your controllers!
- Enabling and disabling of analogue controls can be done with the `ENABLE_ANALOGUE` keyword, and `1` or `0` as the value. JVSCore will run faster with analogue controls disabled.

## Systemd Service

If you'd like JVSCore to run in the background, and automatically connect to JVS I/O boards you can set it to run as a service. To do so type the following:

```
sudo systemctl enable jvscore
sudo systemctl start jvscore
```

JVSCore will constantly look for new JVS I/O devices (up to a maximum of 1) every minute and when these are found will create a joystick device. Once the JVS I/O is unplugged or switched off the joystick device will disappear.

To view the logs that JVSCore creates while running as a service, type the following:

```
sudo journalctl -u jvscore
```

## Adapters known to work

The best adapters are those with an FTDI chipset.

## Latency Testing

Thanks to JaviRodasG for testing the latency of JVSCore.

Results for JVSCore **with** analogue controls enabled are as follows:

```
15.14 - 15.80  [  9]  **
15.80 - 16.46  [ 35]  ********
16.46 - 17.11  [116]  ***************************
17.11 - 17.77  [158]  ************************************
17.77 - 18.43  [128]  ******************************
18.43 - 19.08  [161]  *************************************
19.08 - 19.74  [167]  **************************************
19.74 - 20.40  [137]  ********************************
20.40 - 21.06  [165]  **************************************
21.06 - 21.71  [147]  **********************************
21.71 - 22.37  [148]  **********************************
22.37 - 23.03  [176]  ****************************************
23.03 - 23.68  [144]  *********************************
23.68 - 24.34  [145]  *********************************
24.34 - 25.00  [164]  **************************************

Samples: 2000 of 2000
Average: 20.7348 ms
Maximum: 24.997 ms
Minimum: 15.143 ms
Std-dev: 2.4883 ms
```

Results for JVSCore **without** analogue controls enabled are as follows:

```
12.29 - 12.75  [ 32]  ********
12.75 - 13.20  [ 72]  *****************
13.20 - 13.66  [101]  ***********************
13.66 - 14.12  [114]  **************************
14.12 - 14.58  [141]  ********************************
14.58 - 15.04  [179]  ****************************************
15.04 - 15.50  [158]  ************************************
15.50 - 15.96  [151]  **********************************
15.96 - 16.41  [141]  ********************************
16.41 - 16.87  [159]  ************************************
16.87 - 17.33  [162]  *************************************
17.33 - 17.79  [128]  *****************************
17.79 - 18.25  [162]  *************************************
18.25 - 18.71  [126]  *****************************
18.71 - 19.17  [ 94]  *********************
19.17 - 19.63  [ 58]  *************
19.63 - 20.08  [ 22]  *****

Samples: 2000 of 2000
Average: 16.1385 ms
Maximum: 20.084 ms
Minimum: 12.287 ms
Std-dev: 1.8312 ms
```

## Credits

Thank you very much to @chunksin and @JaviRodasG for helping to test and debug issues with the software!
