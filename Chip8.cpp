/*
 * Chip8.cpp
 *
 *  Created on: May 25, 2016
 *      Author: plukraine
 */

// #define DEBUG_OUTPUT

#include <stdio.h>
#include "Chip8.h"
#include <cstring>
#include <random>
#include <fstream>

static std::random_device rd;   // non-deterministic generator
static std::mt19937 gen(rd());  // to seed mersenne twister.
static std::uniform_int_distribution<> dist(0,255); // distribute results between 0 and 255 inclusive.
byte logXor(byte a, byte b) {
    return (!a && b) || (a && !b) ? 1 : 0;
}


void Chip8Struct::clear()
{
    memset(mem, 0, sizeof(mem));
    memset(disp, 0, sizeof(disp));
    memset(V, 0, sizeof(V));
    memset(keys, 0, sizeof(keys));
    DelayTimer = 0;
    SoundTimer = 0;
    I = 0;
    PC = 0;
    while (!st.empty()) st.pop();
    waitKey = 0;
}


void Chip8Logic::setKeypad(byte V[], byte keys[], int &waitkey, int keynum, bool value)
{
    keys[keynum] = value;
    if (value && waitkey) {
#ifdef DEBUG_OUTPUT
        printf("Lock has broken down! Wrote 0x%X to %X register\n", keynum, 0xF & waitkey);
#endif
        V[0xF & waitkey] = keynum;
        waitkey = 0;
    }
}


void Chip8Logic::loadProgram(byte mem[], int &waitKey, ushort &PC, char* filename)
{
    PC = PC_INIT;
    waitKey = 0;
    std::ifstream fin(filename, std::ios::in | std::ios::binary);
    for (int i = PC_INIT; fin.good(); ++i) {
        byte piece = fin.get();
        mem[i] = piece;
    }
    fin.close();
}


bool Chip8Logic::isWaitingForKey(int waitKey)
{
    return waitKey != 0;
}


void Chip8Logic::renderTo(byte disp[], unsigned int *pixels)
{
    for (unsigned pos = 0; pos < CHIP8_W*CHIP8_H; ++pos)
        pixels[pos] = disp[pos] ? 0xFFFFFF : 0;
}


ushort Chip8Logic::getNextOpcode(byte mem[], ushort &PC, int waitKey)
{
    if (isWaitingForKey(waitKey)) return 0;
    ushort opcode = (mem[PC] << 8) | mem[PC + 1];
    PC += 2;
    return opcode;
}


void Chip8Logic::loadFontset(byte mem[])
{
    byte font[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0,
        0x20, 0x60, 0x20, 0x20, 0x70,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x10, 0xF0,
        0x90, 0x90, 0xF0, 0x10, 0x10,
        0xF0, 0x80, 0xF0, 0x10, 0xF0,
        0xF0, 0x80, 0xF0, 0x90, 0xF0,
        0xF0, 0x10, 0x20, 0x40, 0x40,
        0xF0, 0x90, 0xF0, 0x90, 0xF0,
        0xF0, 0x90, 0xF0, 0x10, 0xF0,
        0xF0, 0x90, 0xF0, 0x90, 0x90,
        0xE0, 0x90, 0xE0, 0x90, 0xE0,
        0xF0, 0x80, 0x80, 0x80, 0xF0,
        0xE0, 0x90, 0x90, 0x90, 0xE0,
        0xF0, 0x80, 0xF0, 0x80, 0xF0,
        0xF0, 0x80, 0xF0, 0x80, 0x80
    };

    memcpy(mem, font, sizeof(font));
}


void Chip8Logic::dumpMem(byte mem[], char* filename)
{
    std::ofstream fout(filename, std::ios::out | std::ios::binary);
    fout.write((char*)mem, CHIP8_MEMSIZE);
    fout.close();
}


void Chip8Logic::executeCommand(Chip8Struct& c, ushort opcode)
{
#ifdef DEBUG_OUTPUT
    static int cnt = 0;
    //printf("opcode %X; PC %X\n", opcode, c.PC);
#endif
    if (isWaitingForKey(c.waitKey)) return;

    ushort addr = 0xFFF & opcode;
    byte x = 0xF & (opcode >> 8);
    byte y = 0xF & (opcode >> 4);
    byte kk = 0xFF & opcode;
    byte n = 0xF & opcode;
    byte& Vx = c.V[x];
    byte& Vy = c.V[y];
    byte& Vf = c.V[0xF];
    // for cycles
    int p;

#ifdef DEBUG_OUTPUT
#define p(mnem) printf("%6d 0x%X %s\n", cnt++, opcode, mnem)
#else
#define p(mnem) ;
#endif
#define BAD_COMMAND printf("unrecognized command %X, PC %X\n", opcode, c.PC); dumpMem(c.mem, "memdump.bin"); exit(1);

    switch (opcode >> 12) {
    case 0:
        switch (kk) {
        case 0xe0:
            p("cls");
            memset(c.disp, 0, sizeof(c.disp));
            break;
        case 0xee:
            p("ret");
            c.PC = c.st.top();
            c.st.pop();
            break;
        default:
            BAD_COMMAND break;
        }
        break;
    case 1:
        p("jump addr");
        c.PC = addr;
        break;
    case 2:
        p("call addr");
        c.st.push(c.PC);
        c.PC = addr;
        break;
    case 3:
        p("se Vx, kk");
        if (Vx == kk) c.PC += 2;
        break;
    case 4:
        p("sne Vx, kk");
        if (Vx != kk) c.PC += 2;
        break;
    case 5:
        p("se Vx, Vy");
        if (Vx == Vy) c.PC += 2;
        break;
    case 6:
        p("ld Vx, kk");
        Vx = kk;
        break;
    case 7:
        p("add Vx, kk");
        Vx += kk;
        break;
    case 8:
        switch (n) {
        case 0:
            p("ld Vx, Vy");
            Vx = Vy;
            break;
        case 1:
            p("or Vx, Vy");
            Vx |= Vy;
            break;
        case 2:
            p("and Vx, Vy");
            Vx &= Vy;
            break;
        case 3:
            p("xor Vx, Vy");
            Vx ^= Vy;
            break;
        case 4:
            p("add Vx, Vy");
            Vf = (Vx + Vy) >> 8;
            Vx += Vy;
            break;
        case 5:
            p("sub Vx, Vy");
            Vf = !((Vx - Vy) >> 8);
            Vx = Vx - Vy;
            break;
        case 6:
            p("shr Vx, Vy");
            Vf = Vx & 1;
            Vx = Vx >> 1;
            break;
        case 7:
            p("subn Vx, Vy");
            Vf = !((Vy - Vx) >> 8);
            Vx = Vy - Vx;
            break;
        case 0xE:
            p("shl Vx, Vy");
            Vf = Vx >> 7;
            Vx = Vx << 1;
            break;
        default:
            BAD_COMMAND
                break;
        }
        break;
    case 9:
        p("sne Vx, Vy");
        if (Vx != Vy) c.PC += 2;
        break;
    case 0xA:
        p("ld I, addr");
        c.I = addr;
        break;
    case 0xB:
        p("jump v0, addr");
        c.PC = addr + c.V[0];
        break;
    case 0xC:
        p("rnd Vx, kk");
        Vx = dist(gen) & kk;
        break;
    case 0xD:
    {
                p("drw Vx, Vy, n");
                Vf = 0;
                for (int i = 0; i<n; ++i) {
                    // one row of sprite
                    byte row = c.mem[0xFFF & (c.I + i)];
                    int ypos = (Vy + i) % CHIP8_H;
                    for (int j = 7; j >= 0; --j) {
                        int xpos = (Vx + 7 - j) % CHIP8_W;
                        int pos = ypos * CHIP8_W + xpos;
                        byte bit = (row >> j) & 1;
                        // if pixel switches off, put 1 in VF
                        if (c.disp[pos] && bit)
                            Vf = 1;
                        c.disp[pos] = logXor(c.disp[pos], bit);
                    }
                }
                break;
    }
    case 0xE:
        switch (kk) {
        case 0x9E:
            p("skp Vx");
            if (c.keys[Vx & 0xF]) c.PC += 2;
            break;
        case 0xA1:
            p("sknp Vx");
            if (!c.keys[Vx & 0xF]) c.PC += 2;
            break;
        default:
            BAD_COMMAND break;
        }
        break;
    case 0xF:
        switch (kk) {
        case 0x07:
            p("ld Vx, dt");
            Vx = c.DelayTimer;
            break;
        case 0x0A:
            p("wait Vx");
            c.waitKey = 0xF0 | x;
            break;
        case 0x15:
            p("ld dt, Vx");
            c.DelayTimer = Vx;
            break;
        case 0x18:
            p("ld st, Vx");
            c.SoundTimer = Vx;
            break;
        case 0x1E:
        {
                     p("add I, Vx");
                     int lal = (c.I & 0xFFF) + Vx;
                     Vf = lal >> 12;
                     c.I = lal;
                     break;
        }
        case 0x29:
            p("ld f, Vx");
            c.I = (Vx & 15) * 5;
            break;
        case 0x33:
            p("ld B, Vx");
            c.mem[(c.I + 0) & 0xFFF] = (Vx / 100) % 10;
                c.mem[(c.I + 1) & 0xFFF] = (Vx / 10) % 10;
                c.mem[(c.I + 2) & 0xFFF] = (Vx / 1) % 10;
            break;
        case 0x55:
            p("ld [i],Vx");
            for (p = 0; p <= x; ++p) c.mem[c.I++ & 0xFFF] = c.V[p];
            break;
        case 0x65:
            p("ld Vx,[i]");
            for (p = 0; p <= x; ++p) c.V[p] = c.mem[c.I++ & 0xFFF];
            break;
        default:
            BAD_COMMAND
                break;
        }
    }

#undef p
}
