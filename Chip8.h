/*
 * Chip8.h
 *
 *  Created on: May 25, 2016
 *      Author: plukraine
 */

#ifndef CHIP8_H_
#define CHIP8_H_
#include <stack>

typedef unsigned char byte;
typedef unsigned short ushort;
const int CHIP8_W = 64;
const int CHIP8_H = 32;
const int PC_INIT = 0x200;
const int CHIP8_MEMSIZE = 4096;
struct Chip8Struct {
	byte mem[CHIP8_MEMSIZE];
	byte disp[CHIP8_H*CHIP8_W];
	byte V[16];
	byte keys[16];
	byte DelayTimer;
	byte SoundTimer;
	ushort I;
	ushort PC;
	std::stack<ushort> st;
	int waitKey;

	void clear();
};

/* Logic for the Chip8 cpu emulator */
class Chip8Logic {
public:
	/* Set or unset key in given array */
	void setKeypad(byte V[], byte keys[], int &waitkey, int keynum, bool value);
	/* Load game into memory mem and set PC to PC_INIT */
	void loadProgram(byte mem[], int &waitKey, ushort &PC, char* filename);
	/* Check if CPU is waititng for the key */
	bool isWaitingForKey(int waitKey);
	/* Render graphics to the pixels array */
	void renderTo(byte disp[], unsigned int *pixels);
	/* Execute given command */
	void executeCommand(Chip8Struct& c, ushort opcode);
	/* Get next opcode from memory and advance PC. If is waiting for key, returns 0 */
	ushort getNextOpcode(byte mem[], ushort &PC, int waitKey);
	/* Load fontset of the Chip8 into given memory (from 0x00 to 0x4F) */
	void loadFontset(byte mem[]);
	/* Dump memory (first CHIP8_MEMSIZE bytes) to a file */
	void dumpMem(byte mem[], char* filename);
};

#endif /* CHIP8_H_ */
