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
static unsigned delay = 1000;
static bool playing;

EM_JS(void, jsUpdateCanvas, (void * videoPointer), {
    postMessage({cmd: "updateCanvas", data: videoPointer});
});

void _play() {
    playing = true;

    auto lastCycleTime = std::chrono::high_resolution_clock::now();
    while (playing) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

        if (dt > delay) {
            lastCycleTime = currentTime;
            chip8.Cycle();
            jsUpdateCanvas(chip8.GetVideoPointer());
        }
    }
}

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

EMSCRIPTEN_KEEPALIVE void setDelay(unsigned d) {
    delay = d;
}

EMSCRIPTEN_KEEPALIVE void start() {
    std::thread thread(_play);
    thread.detach();
}

EMSCRIPTEN_KEEPALIVE void stop() {
    playing = false;
}

EMSCRIPTEN_KEEPALIVE void resetState() {
    chip8.Reset();
}

}