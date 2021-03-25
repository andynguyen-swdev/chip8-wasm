#include "Chip8.h"
#include <fstream>
#include <algorithm>
#include <cstring>
#include <cstdint>
#include <iostream>
#include <stdio.h>

using namespace std;

const unsigned int ROM_START_ADDR = 0x200;
const unsigned int FONTSET_START_ADDR = 0x50;

Chip8::Chip8() : pc{ROM_START_ADDR} {
    uint8_t fontset[] =
            {
                    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
                    0x20, 0x60, 0x20, 0x20, 0x70, // 1
                    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
                    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
                    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
                    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
                    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
                    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
                    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
                    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
                    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
                    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
                    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
                    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
                    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
                    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
            };

    // Load fontset into memory.
    copy_n(fontset, sizeof(fontset) / sizeof(uint8_t), memory + FONTSET_START_ADDR);

    fill(begin(table), end(table), &Chip8::OP_NOOP);
    fill(begin(table0), end(table0), &Chip8::OP_NOOP);
    fill(begin(table8), end(table8), &Chip8::OP_NOOP);
    fill(begin(tableE), end(tableE), &Chip8::OP_NOOP);
    fill(begin(tableF), end(tableF), &Chip8::OP_NOOP);

    table[0x0] = &Chip8::DecodeTable0;
    table[0x1] = &Chip8::OP_1nnn;
    table[0x2] = &Chip8::OP_2nnn;
    table[0x3] = &Chip8::OP_3xkk;
    table[0x4] = &Chip8::OP_4xkk;
    table[0x5] = &Chip8::OP_5xy0;
    table[0x6] = &Chip8::OP_6xkk;
    table[0x7] = &Chip8::OP_7xkk;
    table[0x8] = &Chip8::DecodeTable8;
    table[0x9] = &Chip8::OP_9xy0;
    table[0xA] = &Chip8::OP_Annn;
    table[0xB] = &Chip8::OP_Bnnn;
    table[0xC] = &Chip8::OP_Cxkk;
    table[0xD] = &Chip8::OP_Dxyn;
    table[0xE] = &Chip8::DecodeTableE;
    table[0xF] = &Chip8::DecodeTableF;

    table0[0x0] = &Chip8::OP_00E0;
    table0[0xE] = &Chip8::OP_00EE;

    table8[0x0] = &Chip8::OP_8xy0;
    table8[0x1] = &Chip8::OP_8xy1;
    table8[0x2] = &Chip8::OP_8xy2;
    table8[0x3] = &Chip8::OP_8xy3;
    table8[0x4] = &Chip8::OP_8xy4;
    table8[0x5] = &Chip8::OP_8xy5;
    table8[0x6] = &Chip8::OP_8xy6;
    table8[0x7] = &Chip8::OP_8xy7;
    table8[0xE] = &Chip8::OP_8xyE;

    tableE[0x1] = &Chip8::OP_ExA1;
    tableE[0xE] = &Chip8::OP_Ex9E;

    tableF[0x07] = &Chip8::OP_Fx07;
    tableF[0x0A] = &Chip8::OP_Fx0A;
    tableF[0x15] = &Chip8::OP_Fx15;
    tableF[0x18] = &Chip8::OP_Fx18;
    tableF[0x1E] = &Chip8::OP_Fx1E;
    tableF[0x29] = &Chip8::OP_Fx29;
    tableF[0x33] = &Chip8::OP_Fx33;
    tableF[0x55] = &Chip8::OP_Fx55;
    tableF[0x65] = &Chip8::OP_Fx65;
}

uint32_t *const Chip8::GetVideoPointer() {
    return video;
}

void Chip8::Reset() {
    fill(begin(registers), end(registers), (uint8_t) 0);
    fill(begin(video), end(video), (uint32_t) 0);
    sp = 0;
    delayTimer = 0;
    soundTimer = 0;
    index = 0;
    pc = ROM_START_ADDR;
}

/**
 * Load ROM content into memory.
 */
void Chip8::LoadRomFromPath(const std::string &fileName) {
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        return;

    auto size = file.tellg();
    char buffer[(size_t) size];
    file.seekg(0, std::ios::beg);
    file.read(buffer, size);

    LoadRom(buffer, size);
}

void Chip8::LoadRom(char *const buffer, size_t size) {
    std::copy_n(buffer, size, memory + ROM_START_ADDR);
}


void Chip8::Cycle() {
    opcode = (memory[pc] << 8) | memory[pc + 1];
    pc += 2;

    (this->*table[(opcode & 0xF000) >> 12])();

    if (delayTimer > 0) delayTimer--;
    if (soundTimer > 0) soundTimer--;
}

void Chip8::DecodeTable0() {
    (this->*table0[opcode & 0x000F])();
}

void Chip8::DecodeTable8() {
    (this->*table8[opcode & 0x000F])();
}

void Chip8::DecodeTableE() {
    (this->*tableE[opcode & 0x000F])();
}

void Chip8::DecodeTableF() {
    (this->*tableF[opcode & 0x00FF])();
}

uint8_t Chip8::RandByte() {
    return randDist(rng);
}

//================================================================================
// Chip-8 Instructions
//================================================================================

/**
 * Clear the display.
 */
void Chip8::OP_00E0() {
    memset(video, 0, sizeof(video));
}

/**
 * Return from a subroutine.
 * The interpreter sets the program counter to the address at the top of the stack, then subtracts 1 from the stack pointer.
 */
void Chip8::OP_00EE() {
    pc = stack[sp];
    sp--;
}

/**
 * Jump to location nnn.
 * The interpreter sets the program counter to nnn.
 */
void Chip8::OP_1nnn() {
    pc = opcode & 0xFFF;
}

/**
 * Call subroutine at nnn.
 * The interpreter increments the stack pointer, then puts the current PC on the top of the stack. The PC is then set to nnn.
 */
void Chip8::OP_2nnn() {
    sp++;
    stack[sp] = pc;
    pc = opcode & 0xFFF;
}

/**
 * Skip next instruction if Vx = kk.
 * The interpreter compares register Vx to kk, and if they are equal, increments the program counter by 2.
 */
void Chip8::OP_3xkk() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t kk = opcode & 0xFF;
    if (registers[x] == kk)
        pc += 2;
}

/**
 * Skip next instruction if Vx != kk.
 * The interpreter compares register Vx to kk, and if they are not equal, increments the program counter by 2.
 */
void Chip8::OP_4xkk() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t kk = opcode & 0xFF;
    if (registers[x] != kk)
        pc += 2;
}

/**
 * Skip next instruction if Vx = Vy.
 * The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.
 */
void Chip8::OP_5xy0() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t y = (opcode & 0xF0) >> 4;
    if (registers[x] == registers[y])
        pc += 2;
}

/**
 * Set Vx = kk.
 * The interpreter puts the value kk into register Vx.
 */
void Chip8::OP_6xkk() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t kk = opcode & 0xFF;
    registers[x] = kk;
}

/**
 * Set Vx = Vx + kk.
 * Adds the value kk to the value of register Vx, then stores the result in Vx. 
 */
void Chip8::OP_7xkk() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t kk = opcode & 0xFF;
    registers[x] += kk;
}

/**
 * Set Vx = Vy.
 * Stores the value of register Vy in register Vx.
 */
void Chip8::OP_8xy0() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t y = (opcode & 0xF0) >> 4;
    registers[x] = registers[y];
}

/**
 * Set Vx = Vx OR Vy.
 * Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx. A bitwise OR compares the corrseponding bits from two values, and if either bit is 1, then the same bit in the result is also 1. Otherwise, it is 0. 
 */
void Chip8::OP_8xy1() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t y = (opcode & 0xF0) >> 4;
    registers[x] |= registers[y];
}

/**
 * Set Vx = Vx AND Vy.
 * Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx. A bitwise AND compares the corrseponding bits from two values, and if both bits are 1, then the same bit in the result is also 1. Otherwise, it is 0. 
 */
void Chip8::OP_8xy2() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t y = (opcode & 0xF0) >> 4;
    registers[x] &= registers[y];
}

/**
 * Set Vx = Vx XOR Vy.
 * Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx. An exclusive OR compares the corrseponding bits from two values, and if the bits are not both the same, then the corresponding bit in the result is set to 1. Otherwise, it is 0. 
 */
void Chip8::OP_8xy3() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t y = (opcode & 0xF0) >> 4;
    registers[x] ^= registers[y];
}

/**
 * Set Vx = Vx + Vy, set VF = carry.
 * The values of Vx and Vy are added together. If the result is greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0. Only the lowest 8 bits of the result are kept, and stored in Vx.
 */
void Chip8::OP_8xy4() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t y = (opcode & 0xF0) >> 4;

    uint16_t sum = registers[x] + registers[y];
    registers[0xF] = sum > 255;
    registers[x] = sum;
}

/**
 * Set Vx = Vx - Vy, set VF = NOT borrow.
 * If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and the results stored in Vx.
 */
void Chip8::OP_8xy5() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t y = (opcode & 0xF0) >> 4;

    registers[0xF] = registers[x] > registers[y];
    registers[x] -= registers[y];
}

/**
 * Set Vx = Vx SHR 1.
 * If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
 */
void Chip8::OP_8xy6() {
    uint8_t x = (opcode & 0xF00) >> 8;
    registers[0xF] = registers[x] & 1;
    registers[x] >>= 1;
}

/**
 * Set Vx = Vy - Vx, set VF = NOT borrow.
 * If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
 */
void Chip8::OP_8xy7() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t y = (opcode & 0xF0) >> 4;

    registers[0xF] = registers[y] > registers[x];
    registers[x] = registers[y] - registers[x];
}

/**
 * Set Vx = Vx SHL 1.
 * If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.
 */
void Chip8::OP_8xyE() {
    uint8_t x = (opcode & 0xF00) >> 8;
    registers[0xF] = (registers[x] & 0x80) >> 7;
    registers[x] <<= 1;
}

/**
 * Skip next instruction if Vx != Vy.
 * The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.
 */
void Chip8::OP_9xy0() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t y = (opcode & 0xF0) >> 4;
    if (registers[x] != registers[y])
        pc += 2;
}

/**
 * Set I = nnn.
 * The value of register I is set to nnn.
 */
void Chip8::OP_Annn() {
    uint16_t nnn = opcode & 0xFFF;
    index = nnn;
}

/**
 * Jump to location nnn + V0.
 * The program counter is set to nnn plus the value of V0.
 */
void Chip8::OP_Bnnn() {
    uint16_t nnn = opcode & 0xFFF;
    pc = nnn + registers[0];
}

/**
 * Set Vx = random byte AND kk.
 * The interpreter generates a random number from 0 to 255, which is then ANDed with the value kk. The results are stored in Vx. See instruction 8xy2 for more information on AND.
 */
void Chip8::OP_Cxkk() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t kk = opcode & 0xFF;
    registers[x] = kk & RandByte();
}

/**
 * Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
 * 
 * The interpreter reads n bytes from memory, starting at the address stored in I. 
 * These bytes are then displayed as sprites on screen at coordinates (Vx, Vy). 
 * Sprites are XORed onto the existing screen. 
 * If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0. 
 * If the sprite is positioned so part of it is outside the coordinates of the display, 
 * it wraps around to the opposite side of the screen. 
 * See instruction 8xy3 for more information on XOR, and section 2.4, Display, 
 * for more information on the Chip-8 screen and sprites.
 */
void Chip8::OP_Dxyn() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t y = (opcode & 0xF0) >> 4;
    uint8_t n = (opcode & 0xF);

    auto startX = registers[x] % VIDEO_WIDTH;
    auto startY = registers[y] % VIDEO_HEIGHT;

    registers[0xF] = 0; // Reset VF.

    for (decltype(n) row = 0; row < n; row++) {
        auto spriteRow = memory[index + row];
        auto posY = (startY + row) % VIDEO_HEIGHT;

        // Each sprite is guaranteed to be 8 bits wide.
        for (auto col = 0; col < 8; col++) {
            auto spritePixel = spriteRow & (0x80 >> col);
            if (!spritePixel) // Don't need to do anything if the sprite pixel is off.
                continue;

            auto posX = (startX + col) % VIDEO_WIDTH;
            auto &screenPixel = video[posX + posY * VIDEO_WIDTH];

            if (screenPixel) {
                registers[0xF] = 1; // Set VF = 1 as this pixel is erased.
                screenPixel = 0;    // XOR this pixel
            } else {
                screenPixel = 0xFFFFFFFF; // XOR this pixel
            }
        }
    }
}

/**
 * Skip next instruction if key with the value of Vx is pressed.
 * 
 * Checks the keyboard, and if the key corresponding to the value of Vx is currently in the down position, PC is increased by 2.
 */
void Chip8::OP_Ex9E() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    if (keypad[registers[x]])
        pc += 2;
}

/**
 * Skip next instruction if key with the value of Vx is not pressed.
 *  
 * Checks the keyboard, and if the key corresponding to the value of Vx is currently in the up position, PC is increased by 2.
 */
void Chip8::OP_ExA1() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    if (!keypad[registers[x]])
        pc += 2;
}

/**
 * Set Vx = delay timer value.
 * 
 * The value of DT is placed into Vx.
 */
void Chip8::OP_Fx07() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    registers[x] = delayTimer;
}

/**
 * Wait for a key press, store the value of the key in Vx.
 * 
 * All execution stops until a key is pressed, then the value of that key is stored in Vx.
 */
void Chip8::OP_Fx0A() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    for (auto i = 0; i < KEYPAD_COUNT; i++) {
        if (keypad[i]) {
            registers[x] = i;
            return;
        }
    }

    // If no key is pressed then decrease PC by 2, effectively repeat the instruction.
    pc -= 2;
}

/**
 * Set delay timer = Vx.
 *
 * DT is set equal to the value of Vx.
 */
void Chip8::OP_Fx15() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    delayTimer = registers[x];
}

/**
 * Set sound timer = Vx.
 *
 * ST is set equal to the value of Vx.
 */
void Chip8::OP_Fx18() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    soundTimer = registers[x];
}

/**
 * Set I = I + Vx.
 * 
 * The values of I and Vx are added, and the results are stored in I.
 */
void Chip8::OP_Fx1E() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    index += registers[x];
}

/**
 * Set I = location of sprite for digit Vx.
 *
 * The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx. 
 * See section 2.4, Display, for more information on the Chip-8 hexadecimal font.
 */
void Chip8::OP_Fx29() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    index = FONTSET_START_ADDR + 5 * registers[x];
}

/**
 * Store BCD representation of Vx in memory locations I, I+1, and I+2.
 * 
 * The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in I, 
 * the tens digit at location I+1, and the ones digit at location I+2.
 */
void Chip8::OP_Fx33() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    auto Vx = registers[x];

    // ones
    memory[index + 2] = Vx % 10;
    Vx /= 10;

    // tens
    memory[index + 1] = Vx % 10;
    Vx /= 10;

    // hundreds
    memory[index] = Vx % 10;
}

/**
 * Store registers V0 through Vx in memory starting at location I.
 *
 * The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.
 */
void Chip8::OP_Fx55() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    std::copy_n(registers, x + 1, &memory[index]);
}

/**
 * Read registers V0 through Vx from memory starting at location I.
 *
 * The interpreter reads values from memory starting at location I into registers V0 through Vx.
 */
void Chip8::OP_Fx65() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    std::copy_n(&memory[index], x + 1, registers);
}

void Chip8::OP_NOOP() {}

