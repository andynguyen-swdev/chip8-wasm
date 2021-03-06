//
// Created by Andy Nguyen on 2021-03-23.
//

#include <iostream>
#include <emscripten/emscripten.h>
#include <thread>
#include "Chip8.h"

using namespace std;

static Chip8 chip8;
static char *rom;
static size_t romSize;

extern "C"
{
EMSCRIPTEN_KEEPALIVE void *getVideoPointer() {
    return chip8.GetVideoPointer();
}

EMSCRIPTEN_KEEPALIVE void *allocateROM(size_t size) {
    if (rom != nullptr) delete[] rom;
    romSize = size;
    return rom = new char[size];
}

EMSCRIPTEN_KEEPALIVE void *getROMPointer() {
    return rom;
}

EMSCRIPTEN_KEEPALIVE void loadROM() {
    chip8.LoadRom(rom, romSize);
}

EMSCRIPTEN_KEEPALIVE void resetState() {
    chip8.Reset();
}

EMSCRIPTEN_KEEPALIVE void setKey(size_t key, bool on) {
    chip8.SetKey(key, on);
}

EMSCRIPTEN_KEEPALIVE void cycle() {
    chip8.Cycle();
    MAIN_THREAD_EM_ASM({ chip8.updateCanvas($0); }, chip8.GetVideoPointer());
}
}