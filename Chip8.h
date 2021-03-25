#include <cstdint>
#include <string>
#include <random>

static const auto VIDEO_WIDTH = 64;
static const auto VIDEO_HEIGHT = 32;
static const auto KEYPAD_COUNT = 16;
static const auto REGISTER_COUNT = 16;

class Chip8 {
public:
    Chip8();

    void LoadRomFromPath(const std::string &fileName);

    void LoadRom(char *const buffer, size_t size);

    void Cycle();

    void Reset();

    uint32_t* const GetVideoPointer();

private:
    uint8_t registers[REGISTER_COUNT]{};
    uint8_t memory[4096]{};
    uint16_t index{};
    uint16_t pc{};
    uint16_t stack[16]{};
    uint8_t sp{};
    uint8_t delayTimer{};
    uint8_t soundTimer{};
    uint8_t keypad[KEYPAD_COUNT]{};
    uint32_t video[VIDEO_HEIGHT * VIDEO_WIDTH]{};
    uint16_t opcode{};

    std::random_device dev{};
    std::mt19937 rng{dev()};
    std::uniform_int_distribution<uint8_t> randDist{0, 255U};

    typedef void (Chip8::*Chip8Func)();

    Chip8Func table[0xF + 1]{};
    Chip8Func table0[0xE + 1]{};
    Chip8Func table8[0xE + 1]{};
    Chip8Func tableE[0xE + 1]{};
    Chip8Func tableF[0x65 + 1]{};

    uint8_t RandByte();

    void DecodeTable0();

    void DecodeTable8();

    void DecodeTableE();

    void DecodeTableF();

    // Instructions.
    void OP_00E0();

    void OP_00EE();

    void OP_1nnn();

    void OP_2nnn();

    void OP_3xkk();

    void OP_4xkk();

    void OP_5xy0();

    void OP_6xkk();

    void OP_7xkk();

    void OP_8xy0();

    void OP_8xy1();

    void OP_8xy2();

    void OP_8xy3();

    void OP_8xy4();

    void OP_8xy5();

    void OP_8xy6();

    void OP_8xy7();

    void OP_8xyE();

    void OP_9xy0();

    void OP_Annn();

    void OP_Bnnn();

    void OP_Cxkk();

    void OP_Dxyn();

    void OP_Ex9E();

    void OP_ExA1();

    void OP_Fx07();

    void OP_Fx0A();

    void OP_Fx15();

    void OP_Fx18();

    void OP_Fx1E();

    void OP_Fx29();

    void OP_Fx33();

    void OP_Fx55();

    void OP_Fx65();

    void OP_NOOP();

};