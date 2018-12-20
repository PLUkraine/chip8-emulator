# Chip8 Emulator

![Screenshot](https://i.postimg.cc/MGhg8hzF/Screen-Shot-2018-12-20-at-3-01-18-PM.png)

## About
This is a **[CHIP-8](https://en.wikipedia.org/wiki/CHIP-8)** computer emulator. This app will compile on *Windows*, *Linux* and *Mac*.  
Games for CHIP-8 can be found [here](http://www.zophar.net/pdroms/chip8/chip-8-games-pack.html) or [on GitHub](https://github.com/dmatlack/chip8/tree/master/roms). Note that some games may not work.  
See [CHIP-8 documentation](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM) for further details. It is the essential document for the developers.

## Build
To compile this app use **[CMake](https://cmake.org/)** >= 3.1. You will need to download and install **[SDL2](https://www.libsdl.org/download-2.0.php)** development library.  
For **Linux** users: SDL2 can be installed through *package manager*.  
For **Mac** users: SDL2 can be installed through *Homebrew*.

## Run
Download some ROM file from this [page](https://github.com/dmatlack/chip8/tree/master/roms). Start this emulator from the command line with 1 argument - path to the ROM file.

## Controls
CHIP-8 uses numpad as a input device. In this emulator, keys

1 2 3 4

q w e r

a s d f

z x c v

are inputs for the emulator. Each game uses different control scheme so have fun finding out what key does what!
