
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
|B-|DATA-|
|A+|DATA+|
|GND|VCC|
|GND|GND|

> It is always a good idea to research how this cable works before creating it. I'm not responsible for any damage you may cause by creating this wrong.

## Usage

Once JVSCore has been enabled with `sudo systemctl enable jvscore` it will start when the system loads, and connect to the I/O at bootup. Make sure the I/O is connected before powering on the machine.

To start JVSCore in the terminal to view debug messages, you can start it by running `sudo ./usr/bin/jvscore`.

The RS485 converter device path is set in `/etc/jvscore.conf`. You will need to be root to edit this file, so something like `sudo vim jvscore.conf` is required.
