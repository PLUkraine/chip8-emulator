# Chip8 Emulator
## About
This is a simple [CHIP-8](https://en.wikipedia.org/wiki/CHIP-8) emulator. Can be compiled on Windows and Linux.

Games for CHIP-8 can be found [here](http://www.zophar.net/pdroms/chip8/chip-8-games-pack.html). Not all games work properly (it might be caused either by bugs or specification ambiguity).

[Usefull CHIP-8 documentation](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
## Dependencies
This emulator was written using [SDL2](https://www.libsdl.org/), SDL2_mixer, SDL2_image development libraries and a g++ compiler.
# Build
To build and run this project, install SDL libraries. Installation process really depends on the OS of your choise. Try [this](http://lazyfoo.net/SDL_tutorials/lesson01/) guide.

Besides GCC you can use Visual C++ compiler.

Then, on Linux, simply type in the console in the project's folder
```shell
make
./chip8
```

