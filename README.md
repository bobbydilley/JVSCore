
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
```

## Cable

> ATTENTION! THIS MAY BE INCORRECT. PLEASE RESEAERCH HOW TO MAKE THE CABLE YOURSELF.

The below table explains how to connect up the cable

|JVS RS485|Arcade USB|
|---|---|
|A|DATA-|
|B|DATA+|
|GND|VCC|
|GND|GND|

## Usage

At the moment you have to start jvscore, as there is no system service. After its installed you can just run `jvscore`.
