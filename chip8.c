/*
// NOTE(lex): Thanks to http://devernay.free.fr/hacks/chip8/C8TECH10.HTM for chip8 documentation!
*/

#include "shared.h"
#include "memory.h"
#include "chip8_math.h"
#include "chip8_string.c"

#include "stdio.h"
#include "stdlib.h"

internal void DrawRectangle(screen *Screen, int MinX, int MinY, int MaxX, int MaxY, u32 Color);

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
        Result = *(State->MainMemory.Base + State->ProgramCounter++) << 8;
        Result |= *(State->MainMemory.Base + State->ProgramCounter++);
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
        
        file_contents PreloadedData = PlatformReadFile("preloaded_data.chip8_source");
        memcpy(State->MainMemory.Base, PreloadedData.Memory, InternalMemoryBytes);
        
        FILE *FileHandle = 0;
        fopen_s(&FileHandle, State->SourceFile, "rb");
        
        if(FileHandle)
        {
            fseek(FileHandle, 0, SEEK_END);
            long FileSize = ftell(FileHandle);
            rewind(FileHandle);
            
            if(FileSize <= ProgramMaxBytes)
            {
                fread(State->Program.Base, 1, FileSize, FileHandle);
                State->Program.Bytes = InternalMemoryBytes + FileSize;
                State->ProgramCounter = (u16)InternalMemoryBytes;
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
    
    if(State->HaltUntilKeyPress)
    {
        for(u8 ButtonIndex = 0; ButtonIndex <= 0xF; ButtonIndex++)
        {
            if(Input[ButtonIndex])
            {
                State->HaltUntilKeyPress = false;
                State->Registers[State->KeyPressRegister] = ButtonIndex;
            }
        }
    }
    
    //Assert((State->ProgramCounter >= 0) && (State->ProgramCounter < State->Program.Bytes));
    
    b32 RewdrawScreen = false;
    
    u16 Instruction = GetNextInstruction(State);
    
    // TODO(lex): Put this in ParseInstruction...
    
    u16 Quads[4];
    Quads[0] = GetQuadValue(Instruction, 3);
    Quads[1] = GetQuadValue(Instruction, 2);
    Quads[2] = GetQuadValue(Instruction, 1);
    Quads[3] = GetQuadValue(Instruction, 0);
    
    //printf("%x%x %x%x\n", Quads[0], Quads[1], Quads[2], Quads[3]);
    
    u16 Address = (Quads[1] << 8) | (Quads[2] << 4) | Quads[3];
    u8 RegX = (u8)Quads[1];
    u8 RegY = (u8)Quads[2];
    u8 Value = (u8)((Quads[2] << 4) | Quads[3]);
    u8 Nibble = (u8)Quads[3];
    
    if(State->DelayTimer > 0)
    {
        State->DelayTimer--;
    }
    
    if(Instruction == 0x00E0)
    {
        // NOTE(lex): CLS
        // NOTE(lex): Clear the screen
        printf("CLS\n");
        
        u32 *Pixel = Screen->Memory;
        for(int Y = 0; Y < Screen->Height; Y++)
        {
            for(int X = 0; X < Screen->Width; X++)
            {
                *Pixel++ = BLACK;
            }
        }
    }
    else if(Instruction == 0x00EE)
    {
        // NOTE(lex): RET
        // NOTE(lex): Return from subroutine
        printf("RET\n");
        
        if(State->StackPointer > 0)
        {
            State->ProgramCounter = State->Stack[--State->StackPointer];
        }
        else
        {
            Assert(!"Cannot return, stack underflow");
        }
    }
    else if(Quads[0] == 1)
    {
        // NOTE(lex): JP addr
        // NOTE(lex): Jump to address
        printf("JP %x\n", Address);
        
        State->ProgramCounter = Address;
    }
    else if(Quads[0] == 2)
    {
        // NOTE(lex): CALL addr
        // NOTE(lex): Calls function at addr
        printf("CALL %d\n", Address);
        
        if(State->StackPointer <= ArrayCount(State->Stack))
        {
            State->Stack[State->StackPointer++] = State->ProgramCounter;
            State->ProgramCounter = Address;
        }
        else
        {
            Assert(!"Cannot return, stack overflow");
        }
    }
    else if(Quads[0] == 3)
    {
        // NOTE(lex): SE Vx, value
        // NOTE(lex): Skip next instruction if Vx = value
        printf("SE V%d, %d\n", RegX, Value);
        
        if(State->Registers[RegX] == Value)
        {
            State->ProgramCounter += 2;
        }
    }
    else if(Quads[0] == 4)
    {
        // NOTE(lex): SNE Vx, value
        // NOTE(lex): Skip next instruction if Vx != value
        printf("SNE V%d, %d\n", RegX, Value);
        
        if(State->Registers[RegX] != Value)
        {
            State->ProgramCounter += 2;
        }
    }
    else if((Quads[0] == 5) && (Quads[3] == 0))
    {
        // NOTE(lex): SE Vx, Vy
        // NOTE(lex): Skip next instruction if Vx = Vy
        printf("SE V%d, V%d\n", RegX, RegY);
        
        if(State->Registers[RegX] == State->Registers[RegY])
        {
            State->ProgramCounter += 2;
        }
    }
    else if(Quads[0] == 6)
    {
        // NOTE(lex): LD Vx, value
        // NOTE(lex): Put value into register
        printf("LD V%d %d\n", RegX, Value);
        
        State->Registers[RegX] = Value;
    }
    else if(Quads[0] == 7)
    {
        // NOTE(lex): ADD Vx, value
        // NOTE(lex): Adds value to register
        printf("ADD V%d, %d\n", RegX, Value);
        
        State->Registers[RegX] += Value;
    }
    else if((Quads[0] == 8) && (Quads[3] == 0))
    {
        // NOTE(lex): LD Vx, Vy
        // NOTE(lex): Set RegX = RegY
        printf("LD V%d V%d\n", RegX, RegY);
        
        State->Registers[RegX] = State->Registers[RegY];
    }
    else if((Quads[0] == 8) && (Quads[3] == 1))
    {
        // NOTE(lex): OR Vx, Vy
        // NOTE(lex): Set RegX = (RegX | RegY)
        printf("OR V%d, V%d\n", RegX, RegY);
        
        State->Registers[RegX] = (State->Registers[RegX] | State->Registers[RegY]);
    }
    else if((Quads[0] == 8) && (Quads[3] == 2))
    {
        // NOTE(lex): AND Vx, Vy
        // NOTE(lex): Set RegX = (RegX & RegY)
        printf("AND V%d, V%d\n", RegX, RegY);
        
        State->Registers[RegX] = (State->Registers[RegX] & State->Registers[RegY]);
    }
    else if((Quads[0] == 8) && (Quads[3] == 3))
    {
        // NOTE(lex): XOR Vx, Vy
        // NOTE(lex): Set RegX = (RegX ^ RegY)
        printf("XOR V%d, V%d\n", RegX, RegY);
        
        State->Registers[RegX] = (State->Registers[RegX] ^ State->Registers[RegY]);
    }
    else if((Quads[0] == 8) && (Quads[3] == 4))
    {
        // NOTE(lex): ADD Vx, Vy
        // NOTE(lex): Set RegX = (RegX + RegY). If the results are greater than 8 bits, set RegF = 1, otherwise 0.
        printf("ADD V%d, V%d\n", RegX, RegY);
        
        u32 NewValue = (u32)State->Registers[RegX] + (u32)State->Registers[RegY];
        if(NewValue > 255)
        {
            State->Registers[0xF] = 1;
        }
        else
        {
            State->Registers[0xF] = 0;
        }
        State->Registers[RegX] = (u8)NewValue;
    }
    else if((Quads[0] == 8) && (Quads[3] == 5))
    {
        // NOTE(lex): SUB Vx, Vy
        // NOTE(lex): If RegX > RegY, set RegX = RegX - RegY and RegF = 1.
        // NOTE(lex): Otherwise RegF = 0.
        // TODO(lex): What if RegX <= RegY, do I still subtract? Aren't registers unsigned, how does that work???
        printf("SUB V%d, V%d\n", RegX, RegY);
        
        if(State->Registers[RegX] > State->Registers[RegY])
        {
            State->Registers[0xF] = 1;
        }
        else
        {
            State->Registers[0xF] = 0;
        }
        
        State->Registers[RegX] -= State->Registers[RegY];
    }
    else if((Quads[0] == 8) && (Quads[3] == 6))
    {
        // NOTE(lex): SHR Vx
        // NOTE(lex): Store the value in RegY shifted right one bit in RegX
        printf("SHR V%d\n", RegX);
        
        State->Registers[0xF] = State->Registers[RegY] & 0x1;
        State->Registers[RegX] = State->Registers[RegY] >> 1;
    }
    else if((Quads[0] == 8) && (Quads[3] == 7))
    {
        // NOTE(lex): SUBN Vx, Vy
        // NOTE(lex): If RegY > RegX, set RegX = RegY - RegX and RegF = 1.
        // NOTE(lex): Otherwise RegF = 0.
        // TODO(lex): What if RegY <= RegX, do I still subtract? Aren't registers unsigned, how does that work???
        printf("SUBN V%d, V%d\n", RegX, RegY);
        
        if(State->Registers[RegY] > State->Registers[RegX])
        {
            State->Registers[0xF] = 1;
        }
        else
        {
            State->Registers[0xF] = 0;
        }
        
        State->Registers[RegX] = State->Registers[RegY] - State->Registers[RegX];
    }
    else if((Quads[0] == 8) && (Quads[3] == 0xE))
    {
        // NOTE(lex): SHL Vx
        // NOTE(lex): Store the value in RegY shifted left one bit in RegX
        printf("SHL V%d\n", RegX);
        
        State->Registers[0xF] = (State->Registers[RegY] & (1 << 7)) >> 7;
        State->Registers[RegX] = State->Registers[RegY] << 1;
    }
    else if((Quads[0] == 9) && (Quads[3] == 0))
    {
        // NOTE(lex): SNE Vx, Vy
        // NOTE(lex): Skip next instruction if Vx != Vy
        printf("SNE V%d, V%d\n", RegX, RegY);
        
        if(State->Registers[RegX] != State->Registers[RegY])
        {
            State->ProgramCounter += 2;
        }
    }
    else if(Quads[0] == 0xA)
    {
        // NOTE(lex): LD I, addr
        // NOTE(lex): Set RegI = addr
        printf("LD I,%d\n", Address);
        
        State->AddressRegister = Address;
    }
    else if(Quads[0] == 0xB)
    {
        // NOTE(lex): JP V0, addr
        // NOTE(lex): Program counter set to Reg0 + addr
        printf("JP V0, %d\n", Address);
        
        State->ProgramCounter = State->Registers[0] + Address;
    }
    else if(Quads[0] == 0xC)
    {
        // NOTE(lex): RND Vx, value
        // NOTE(lex): Vx = random byte AND value
        printf("RND V%d, %d\n", RegX, Value);
        
        State->Registers[RegX] = rand() & Value;
    }
    else if(Quads[0] == 0xD)
    {
        // NOTE(lex): DRW Vx, Vy, nibble
        // NOTE(lex): Display nibble-byte sprite (memory set in I) at position (RegX, RegY). Set RegF = collision.
        printf("%DRW V%d, V%d, %d\n", RegX, RegY, Nibble);
        
        u8 *SpriteMemory = State->MainMemory.Base + State->AddressRegister;
        int BitIndex = 7;
        u32 SpriteHeight = Nibble;
        u32 SpriteWidth = 8;
        
        u32 StartPixelX = State->Registers[RegX];
        u32 EndPixelX = StartPixelX + SpriteWidth - 1;
        u32 StartPixelY = State->Registers[RegY];
        u32 EndPixelY = StartPixelY + SpriteHeight - 1;
        
        b32 Collision = false;
        
        for(u32 PixelY = StartPixelY; PixelY <= EndPixelY; PixelY++)
        {
            for(u32 PixelX = StartPixelX; PixelX <= EndPixelX; PixelX++)
            {
                u32 DrawX = PixelX;
                u32 DrawY = PixelY;
                Assert((DrawX >= 0) && (DrawY >= 0));
                
                DrawX = DrawX % Screen->Width;
                DrawY = DrawY % Screen->Height;
                
                b32 PixelSet = (*SpriteMemory & (1 << BitIndex)) >> BitIndex;
                if(--BitIndex == -1)
                {
                    SpriteMemory++;
                    BitIndex = 7;
                }
                
                u32 *Pixel = Screen->Memory + DrawY*Screen->Width + DrawX;
                if(*Pixel == WHITE && PixelSet)
                {
                    *Pixel = BLACK;
                    Collision = true;
                }
                else if(*Pixel == BLACK && PixelSet)
                {
                    *Pixel = WHITE;
                }
                else if(*Pixel == WHITE && !PixelSet)
                {
                    *Pixel = WHITE;
                }
                else
                {
                    *Pixel = BLACK;
                }
            }
        }
        
        State->Registers[0xF] = (u8)Collision;
        
        RewdrawScreen = true;
    }
    else if((Quads[0] == 0xE) && (Quads[2] == 9) && (Quads[3] == 0xE))
    {
        // NOTE(lex): SKP Vx
        // NOTE(lex): Skip next instruction if key with value RegX is pressed
        printf("SKP V%d\n", Quads[1]);
        
        if(Input[State->Registers[RegX]])
        {
            State->ProgramCounter += 2;
        }
    }
    else if((Quads[0] == 0xE) && (Quads[2] == 0xA) && (Quads[3] == 1))
    {
        // NOTE(lex): SKNP Vx
        // NOTE(lex): Skip next instruction if key with value RegX is not pressed
        printf("SKNP V%d\n", Quads[1]);
        
        if(!Input[State->Registers[RegX]])
        {
            State->ProgramCounter += 2;
        }
    }
    else if((Quads[0] == 0xF) && (Quads[2] == 0) && (Quads[3] == 7))
    {
        // NOTE(lex): LD Vx, DT
        // NOTE(lex): The value of the delay timer is placed into RegX
        printf("LD Vx, DT\n");
        
        State->Registers[RegX] = State->DelayTimer;
    }
    else if((Quads[0] == 0xF) && (Quads[2] == 0) && (Quads[3] == 0xA))
    {
        // NOTE(lex): LD Vx, K
        // NOTE(lex): Wait for key press, store value of the key in RegX
        printf("LD Vx, K\n");
        
        State->HaltUntilKeyPress = true;
        State->KeyPressRegister = RegX;
    }
    else if((Quads[0] == 0xF) && (Quads[2] == 1) && (Quads[3] == 5))
    {
        // NOTE(lex): LD DT, Vx
        // NOTE(lex): Set delay timer equal to value of RegX
        printf("LD DT, Vx\n");
        
        State->DelayTimer = State->Registers[RegX];
    }
    else if((Quads[0] == 0xF) && (Quads[2] == 1) && (Quads[3] == 8))
    {
        // NOTE(lex): LD ST, Vx
        // NOTE(lex): Sound timer set equal to RegX
        printf("LD ST, Vx\n");
        
        State->SoundTimer = State->Registers[RegX];
    }
    else if((Quads[0] == 0xF) && (Quads[2] == 1) && (Quads[3] == 0xE))
    {
        // NOTE(lex): ADD I, Vx
        // NOTE(lex): Add RegI and RegX, store value in I
        printf("ADD I, Vx\n");
        
        State->AddressRegister += State->Registers[RegX];
    }
    else if((Quads[0] == 0xF) && (Quads[2] == 2) && (Quads[3] == 9))
    {
        // NOTE(lex): LD F, Vx
        // NOTE(lex): Set RegI = Memory location of hexadecimal digit sprite location
        printf("LD F, Vx\n");
        
        u16 BytesPerNumber = 6;
        State->AddressRegister = BytesPerNumber*(u16)State->Registers[RegX];
    }
    else if((Quads[0] == 0xF) && (Quads[2] == 3) && (Quads[3] == 3))
    {
        // NOTE(lex): LD B, Vx
        // NOTE(lex): Takes decimal value of Regx and stores hundreds digit in I, tens digit in I+1, and ones in I+2.
        printf("LD B, Vx\n");
        
        int Value = State->Registers[RegX];
        State->MainMemory.Base[State->AddressRegister + 2] = Value % 10;
        State->MainMemory.Base[State->AddressRegister + 1] = (Value / 10) % 10;
        State->MainMemory.Base[State->AddressRegister + 0] = (Value / 100) % 10;
        
    }
    else if((Quads[0] == 0xF) && (Quads[2] == 5) && (Quads[3] == 5))
    {
        // NOTE(lex): LD [I], Vx
        // NOTE(lex): Store registers Reg0..RegX in memory starting at location I.
        printf("LD [I], Vx\n");
        
        for(int RegisterIndex = 0; RegisterIndex <= RegX; RegisterIndex++)
        {
            State->MainMemory.Base[State->AddressRegister + RegisterIndex] 
                = State->Registers[RegisterIndex];
        }
    }
    else if((Quads[0] == 0xF) && (Quads[2] == 6) && (Quads[3] == 5))
    {
        // NOTE(lex): LD Vx, [I]
        // NOTE(lex): Load memory starting from I into registers Reg0..RegX.
        printf("LD Vx, [I]\n");
        
        for(int RegisterIndex = 0; RegisterIndex <= RegX; RegisterIndex++)
        {
            State->Registers[RegisterIndex] = 
                State->MainMemory.Base[State->AddressRegister + RegisterIndex];
        }
    }
    
    return RewdrawScreen;
}