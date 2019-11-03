
# JVSCore

JVSCore is a user space driver for using JVS I/O boards with Linux. It requires a USB RS485 converter wired to the JVS I/O.

> Please not this doesn't yet work. Although it can connect to boards and create an input device, it's not currently ready to use.

## Installation

Installation is done from the git repository as follows:

```
sudo apt install build-essential git
git clone https://github.com/bobbydilley/JVSCore-Public
cd JVSCore-Public
make
sudo make install
sudo jvscore
```

## Cable

I'd reccomend watching the below video from the TecknoGods about how to create a JVS cable.

https://www.youtube.com/watch?v=kqXEYtvGzno


|RS485 Adapter Side|USB To Arcade Side|
|---|---|
|B-|DATA-|
|A+|DATA+|
|5-12V|VCC|
|GND|GND|

> I'm not 100% sure if the 5-12V line is giving that, or will just take that - so please be carefull what you plug in!

## Usage

To start JVSCore in the terminal to view debug messages, you can start it by running `sudo jvscore`.

The RS485 converter device path is set in `/etc/jvscore.conf`. If you only have one serial device plugged in, you shouldn't have to change it!
