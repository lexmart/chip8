#include <stdint.h>
#include <limits.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float r32;
typedef double r64;

typedef int b32;
#define true 1
#define false 0

#define global_variable static
#define local_persist static
#define internal static

#define KiloBytes(Num) ((Num)*1024)
#define MegaBytes(Num) (KiloBytes((Num))*1024)
#define GigaBytes(Num) (MegaBytes((Num))*1024)

#define ArrayCount(Array) (sizeof(Array)/(sizeof((Array)[0])))

void ClearArray_(char *Array, int Length)
{
    for(int Index = 0; Index < Length; Index++)
    {
        Array[Index] = 0;
    }
}
#define ClearArray(Array) ClearArray_(Array, ArrayCount(Array))

#define Assert(Expression) if(!(Expression)) { *((int *)0) = 0; }

#define InvalidCodePath Assert(!"Invalid code path");

#define WHITE 0xFFFFFFFF
#define BLACK 0x000000FF

typedef struct
{
    u32 *Memory;
    int Width;
    int Height;
} screen;

typedef struct
{
    u8 *Base;
    int Bytes;
} memory_block;

typedef struct
{
    b32 Initialized;
    memory_block MainMemory;
    memory_block Program;
    u16 ProgramCounter;
    
    u8 Registers[16];
    u16 AddressRegister;
    
    u8 StackPointer;
    u16 Stack[16];
    
    u8 DelayTimer;
    u8 SoundTimer;
    
    b32 HaltUntilKeyPress;
    u8 KeyPressRegister;
    
    char SourceFile[1024];
} emulator_state;

internal memory_block
CreateMemoryBlock(u8 *Base, int Bytes)
{
    memory_block MemoryBlock;
    MemoryBlock.Base = Base;
    MemoryBlock.Bytes = Bytes;
    return MemoryBlock;
}

#define CHIP8_CYCLE b32 Chip8Cycle(emulator_state *State, memory_block *MainMemory, screen *Screen, b32 *Input)