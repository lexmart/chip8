/*
// NOTE(lex): Thanks to http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#Dxyn for chip8 documentation!
*/

#include "shared.h"
#include "memory.h"
#include "chip8_math.h"

// TODO(lex): We don't compile with CRT here, remove asap-ly
#include "stdio.h"

#define WHITE 0xFFFFFFFF
#define BLACK 0x000000FF

internal void DrawRectangle(screen *Screen, int MinX, int MinY, int MaxX, int MaxY, u32 Color);

#include "chip8_instructions.c"

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
    u16 Result = 0;
    
    if((State->ProgramCounter >= 0) && (State->ProgramCounter < State->Program.Bytes))
    {
        Result = *(State->Program.Base + State->ProgramCounter++) << 8;
        Result |= *(State->Program.Base + State->ProgramCounter++);
    }
    
    return Result;
}

internal u16
GetQuadValue(u16 Instruction, int QuadIndex)
{
    u16 Result = (Instruction >> (4*QuadIndex)) & 0xF;
    return Result;
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
                State->ProgramCounter = 0;
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
    
    //Assert((State->ProgramCounter >= 0) && (State->ProgramCounter < State->Program.Bytes));
    
    u16 Instruction = GetNextInstruction(State);
    
    // TODO(lex): Put this in ParseInstruction...
    
    u16 Quads[4];
    Quads[0] = GetQuadValue(Instruction, 3);
    Quads[1] = GetQuadValue(Instruction, 2);
    Quads[2] = GetQuadValue(Instruction, 1);
    Quads[3] = GetQuadValue(Instruction, 0);
    
    u16 Address = (Quads[1] << 8) | (Quads[2] << 4) | Quads[3];
    u8 RegX = (u8)Quads[1];
    u8 RegY = (u8)Quads[2];
    u8 Value = (u8)((Quads[2] << 4) | Quads[3]);
    
    if(Instruction == 0x00E0)
    {
        // NOTE(lex): CLS
        // NOTE(lex): Clear the screen
        printf("CLS\n");
    }
    else if(Instruction == 0x00EE)
    {
        // NOTE(lex): RET
        // NOTE(lex): Return from subroutine
        printf("RET\n");
    }
    else if(Quads[0] == 1)
    {
        // NOTE(lex): JP addr
        // NOTE(lex): Jump to address
        printf("JP %d\n", Address);
    }
    else if(Quads[0] == 2)
    {
        // NOTE(lex): CALL addr
        // NOTE(lex): Calls function at addr
        printf("CALL %d\n", Address);
    }
    else if(Quads[0] == 3)
    {
        // NOTE(lex): SE Vx, value
        // NOTE(lex): Skip next instruction if Vx = value
        printf("SE V%d, %d\n", RegX, Value);
    }
    else if(Quads[0] == 4)
    {
        // NOTE(lex): SNE Vx, value
        // NOTE(lex): Skip next instruction if Vx != value
        printf("SNE V%d, %d\n", RegX, Value);
    }
    else if((Quads[0] == 5) && (Quads[3] == 0))
    {
        // NOTE(lex): SE Vx, Vy
        // NOTE(lex): Skip next instruction if Vx = Vy
        printf("SE V%d, V%d\n", RegX, RegY);
    }
    else if(Quads[0] == 6)
    {
        // NOTE(lex): LD Vx, value
        // NOTE(lex): Put value into register
        printf("LD V%d %d\n", RegX, Value);
    }
    else if(Quads[0] == 7)
    {
        // NOTE(lex): ADD Vx, value
        // NOTE(lex): Adds value to register
        printf("ADD V%d, %d\n", RegX, Value);
    }
    else if((Quads[0] == 8) && (Quads[3] == 0))
    {
        // NOTE(lex): LD Vx, Vy
        // NOTE(lex): Set RegX = RegY
        printf("LD V%d V%d\n", RegX, RegY);
    }
    else if((Quads[0] == 8) && (Quads[3] == 1))
    {
        // NOTE(lex): OR Vx, Vy
        // NOTE(lex): Set RegX = (RegX | RegY)
        printf("OR V%d, V%d\n", RegX, RegY);
    }
    else if((Quads[0] == 8) && (Quads[3] == 2))
    {
        // NOTE(lex): AND Vx, Vy
        // NOTE(lex): Set RegX = (RegX & RegY)
        printf("AND V%d, V%d\n", RegX, RegY);
    }
    else if((Quads[0] == 8) && (Quads[3] == 3))
    {
        // NOTE(lex): XOR Vx, Vy
        // NOTE(lex): Set RegX = (RegX ^ RegY)
        printf("XOR V%d, V%d\n", RegX, RegY);
    }
    else if((Quads[0] == 8) && (Quads[3] == 4))
    {
        // NOTE(lex): ADD Vx, Vy
        // NOTE(lex): Set RegX = (RegX + RegY). If the results are greater than 8 bits, set RegF = 1, otherwise 0.
        printf("ADD V%d, V%d\n", RegX, RegY);
    }
    else if((Quads[0] == 8) && (Quads[3] == 5))
    {
        // NOTE(lex): SUB Vx, Vy
        // NOTE(lex): If RegX > RegY, set RegX = RegX - RegY and RegF = 1.
        // NOTE(lex): Otherwise RegF = 0.
        // TODO(lex): What if RegX <= RegY, do I still subtract? Aren't registers unsigned, how does that work???
        printf("XOR V%d, V%d\n", RegX, RegY);
    }
    else if((Quads[0] == 8) && (Quads[3] == 6))
    {
        // NOTE(lex): SHR Vx
        // NOTE(lex): Set RegX = RegX >> 1. If LSB of RegX is 1, set RegF = 1 otherwise 0.
        printf("SHR V%d\n", RegX);
    }
    else if((Quads[0] == 8) && (Quads[3] == 7))
    {
        // NOTE(lex): SUBN Vx, Vy
        // NOTE(lex): If RegY > RegX, set RegX = RegY - RegX and RegF = 1.
        // NOTE(lex): Otherwise RegF = 0.
        // TODO(lex): What if RegY <= RegX, do I still subtract? Aren't registers unsigned, how does that work???
        printf("XOR V%d, V%d\n", RegX, RegY);
    }
    else if((Quads[0] == 8) && (Quads[3] == 0xE))
    {
        // NOTE(lex): SHL Vx
        // NOTE(lex): Set RegX = RegX << 1. If MSB of RegX is 1, set RegF = 1 otherwise 0.
        printf("SHL V%d\n", RegX);
    }
    else if((Quads[0] == 9) && (Quads[3] == 0))
    {
        // NOTE(lex): SNE Vx, Vy
        // NOTE(lex): Skip next instruction if Vx != Vy
        printf("SNE V%d, V%d\n", RegX, RegY);
    }
    else if(Quads[0] == 0xA)
    {
        // NOTE(lex): LD I, addr
        // NOTE(lex): Set RegI = addr
        printf("LD I,%d\n", Address);
    }
    else if(Quads[0] == 0xB)
    {
        // NOTE(lex): JP V0, addr
        // NOTE(lex): Program counter set to Reg0 + addr
        printf("JP V0, %d\n", Address);
    }
    else if(Quads[0] == 0xC)
    {
        // NOTE(lex): RND Vx, value
        // NOTE(lex): Vx = random byte AND value
        printf("RND V%d, %d\n", RegX, Value);
    }
    else if(Quads[0] == 0xD)
    {
        // NOTE(lex): DRW Vx, Vy, nibble
        // NOTE(lex): Display nibble-byte sprite (memory set in I) at position (RegX, RegY). Set RegF = collision.
    }
    else if((Quads[0] == 0xE) && (Quads[2] == 9) && (Quads[3] == 0xE))
    {
        // NOTE(lex): SKP Vx
        // NOTE(lex): Skip next instruction if key with value RegX is pressed
        printf("SKP V%d\n", Quads[1]);
    }
    else if((Quads[0] == 0xE) && (Quads[2] == 0xA) && (Quads[3] == 1))
    {
        // NOTE(lex): SKNP Vx
        // NOTE(lex): Skil next instruction if key with value RegX is not pressed
        printf("SKNP V%d\n", Quads[1]);
    }
    else if((Quads[0] == 0xF) && (Quads[2] == 0) && (Quads[3] == 7))
    {
        // NOTE(lex): LD Vx, DT
        // NOTE(lex): The value of the delay timer is placed into RegX
        printf("LD Vx, DT\n");
    }
    else if((Quads[0] == 0xF) && (Quads[2] == 0) && (Quads[3] == 0xA))
    {
        // NOTE(lex): LD Vx, K
        // NOTE(lex): Wait for key press, store value of the key in RegX
        printf("LD Vx, K\n");
    }
    else if((Quads[0] == 0xF) && (Quads[2] == 1) && (Quads[3] == 5))
    {
        // NOTE(lex): LD DT, Vx
        // NOTE(lex): Set delay timer equal to value of RegX
        printf("LD DT, Vx\n");
    }
    else if((Quads[0] == 0xF) && (Quads[2] == 1) && (Quads[3] == 8))
    {
        // NOTE(lex): LD ST, Vx
        // NOTE(lex): Sound timer set equal to RegX
        printf("LD ST, Vx\n");
    }
    else if((Quads[0] == 0xF) && (Quads[2] == 1) && (Quads[3] == 0xE))
    {
        // NOTE(lex): ADD I, Vx
        // NOTE(lex): Add RegI and RegX, store value in I
        printf("ADD I, Vx\n");
    }
    else if((Quads[0] == 0xF) && (Quads[2] == 2) && (Quads[3] == 9))
    {
        // NOTE(lex): LD F, Vx
        // NOTE(lex): Set RegI = Memory location of hexadecimal digit sprite location
        printf("LD F, Vx\n");
    }
    else if((Quads[0] == 0xF) && (Quads[2] == 3) && (Quads[3] == 3))
    {
        // NOTE(lex): LD B, Vx
        // NOTE(lex): Takes decimal value of Regx and stores hundreds digit in I, tens digit in I+1, and ones in I+2.
        printf("LD B, Vx\n");
    }
    else if((Quads[0] == 0xF) && (Quads[2] == 5) && (Quads[3] == 5))
    {
        // NOTE(lex): LD [I], Vx
        // NOTE(lex): Store registers Reg0..RegX in memory starting at location I.
        printf("LD [I], Vx\n");
    }
    else if((Quads[0] == 0xF) && (Quads[2] == 6) && (Quads[3] == 5))
    {
        // NOTE(lex): LD Vx, [I]
        // NOTE(lex): Load memory starting from I into registers Reg0..RegX.
        printf("LD Vx, [I]\n");
    }
}