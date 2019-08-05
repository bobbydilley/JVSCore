
# JVSCore

JVSCore is a user space driver for using JVS I/O boards with Linux. It requires a USB RS485 converter wired to the JVS I/O.

## Installation

Installation is done from the git repository as follows:

```
sudo apt install build-essential git
git clone https://github.com/bobbydilley/JVSCore
cd JVSCore
make
sudo make install
sudo systemctl enable jvscore
```

## Cable

The below table explains how to connect up the cable

|JVS RS485|Arcade USB|
|---|---|
|A|DATA-|
|B|DATA+|
|GND|VCC|
|GND|GND|

## Usage

JVSCore runs as a background systemd service, and requires no interaction once installed and started. I/O boards are polled for every 30 seconds by default if one isn't already connected. Simply connect the I/O board, and a device will appear within the polling time named after the device name the board sends.

## Compatibility

This is known to work with:

- MAME
- Demul
- Vivanono
