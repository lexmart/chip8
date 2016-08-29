/* TODO
-Build an assembler to make writing programs easier? (With labels for code)
*/

/* NOTES
-All instructions are on even bytes (pad sprites)
*/

#include "shared.h"
#include "memory.h"
#include "chip8_math.h"

// TODO(lex): We don't compile with CRT here, remove asap-ly
#include "stdio.h"

#define WHITE 0xFFFFFFFF
#define BLACK 0x000000FF

internal void
DrawRectangle(screen *Screen, int MinX, int MinY, int MaxX, int MaxY, u32 Color)
{
    if(MinX < 0)
    {
        MinX = 0;
    }
    if(MaxX >= Screen->Width)
    {
        MaxX = Screen->Width - 1;
    }
    if(MinY < 0)
    {
        MinY = 0;
    }
    if(MaxY >= Screen->Height)
    {
        MaxY = Screen->Height - 1;
    }
    
    u32 *Row = Screen->Memory + MinY*Screen->Width + MinX;
    
    for(int Y = MinY; Y <= MaxY; Y++)
    {
        u32 *Pixel = Row;
        for(int X = MinX; X <= MaxX; X++)
        {
            *Pixel++ = Color;
        }
        Row += Screen->Width;
    }
}

internal b32
IsPointerBounded(void *Base, int Size, void *Pointer)
{
    u8 *MinBound = (u8 *)Base;
    u8 *MaxBound = (u8 *)Base + Size - 1;
    b32 Result = (((u8 *)Pointer >= MinBound) && ((u8 *)Pointer <= MaxBound));
    return Result;
}

internal u16
GetNextInstruction(emulator_state *State)
{
    Assert(State->ProgramLocation < (State->MainMemory.Base + State->MainMemory.Bytes));
    
    u8 *ProgramLocation = State->ProgramLocation;
    u16 Result = (*ProgramLocation++ << 8);
    Result |= *ProgramLocation++;
    
    if(IsPointerBounded(State->Program.Base, State->Program.Bytes, ProgramLocation))
    {
        State->ProgramLocation = ProgramLocation;
    }
    
    return Result;
}

internal u16
GetQuadValue(u16 Instruction, int QuadIndex)
{
    u16 Result = (Instruction >> (4*QuadIndex)) & 0xF;
    return Result;
}

internal void
JumpToAddress(emulator_state *State, u16 Address)
{
    printf("Jumping to address 0x%x\n", Address);
}

internal void
ReturnFromFunction(emulator_state *State)
{
    printf("returning from function\n");
    // TODO(lex): Set the program counter to address at the top of the stack, subtract 1 from the stack pointer
}

CHIP8_CYCLE
{
    if(!State->Initialized)
    {
        State->Initialized = true;
        
        int InternalMemoryBytes = 512;
        State->MainMemory = CreateMemoryBlock(MainMemory->Base, MainMemory->Bytes);
        int ProgramMaxBytes = MainMemory->Bytes - InternalMemoryBytes;
        State->Program.Base = State->MainMemory.Base + 512;
        
        FILE *FileHandle = 0;
        fopen_s(&FileHandle, "test.chip8", "rb");
        
        if(FileHandle)
        {
            fseek(FileHandle, 0, SEEK_END);
            long FileSize = ftell(FileHandle);
            rewind(FileHandle);
            
            if(FileSize <= ProgramMaxBytes)
            {
                fread(State->Program.Base, 1, FileSize, FileHandle);
                State->Program.Bytes = FileSize;
                State->ProgramLocation = State->Program.Base;
            }
            else
            {
                Assert(!"Program too big");
            }
            
            fclose(FileHandle);
        }
        else
        {
            Assert(!"Could not open file");
        }
    }
    
    Assert(IsPointerBounded(State->Program.Base, State->Program.Bytes, State->ProgramLocation));
    
    u16 Instruction = GetNextInstruction(State);
    u16 Quads[4];
    Quads[0] = GetQuadValue(Instruction, 3);
    Quads[1] = GetQuadValue(Instruction, 2);
    Quads[2] = GetQuadValue(Instruction, 1);
    Quads[3] = GetQuadValue(Instruction, 0);
    
    if(Quads[0] == 0)
    {
        // NOTE(lex): Jump to address
        u16 JumpAddress = (Quads[1] << 12) | (Quads[2] << 4) | (Quads[3]);
        JumpToAddress(State, JumpAddress);
    }
    else if(Instruction == 0x00E0)
    {
        // NOTE(lex): Clear screen
        DrawRectangle(Screen, 0, 0, Screen->Width - 1, Screen->Height - 1, BLACK);
    }
    else if(Instruction == 0x00EE)
    {
        ReturnFromFunction(State);
    }
}