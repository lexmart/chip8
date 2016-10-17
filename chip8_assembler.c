#include "shared.h"
#include <stdio.h>

#define MaxLineLength 128
#define MaxTokenLength 16

#include "chip8_string.c"
#include "chip8_token.h"
#include "chip8_label.h"

// NOTE(lex): xxxx yyyy yyyy yyyy
u16 Compose13(u16 X, u16 Y)
{
    Assert(X <= 0xF);
    Assert(Y <= 0xFFF);
    
    u16 Result = (X << 12) | Y;
    
    return Result;
}

// NOTE(lex): xxxx yyyy zzzz zzzz
u16 Compose112(u16 X, u16 Y, u16 Z)
{
    Assert(X <= 0xF);
    Assert(Y <= 0xF);
    Assert(Z <= 0xFF);
    
    u16 Result = (X << 12) | (Y << 8) | Z;
    
    return Result;
}

// NOTE(lex): xxxx yyyy zzzz wwww
u16 Compose1111(u16 X, u16 Y, u16 Z, u16 W)
{
    Assert(X <= 0xF);
    Assert(Y <= 0xF);
    Assert(Z <= 0xF);
    Assert(W <= 0xF);
    
    u16 Result = (X << 12) | (Y << 8) | (Z << 4) | W;
    
    return Result;
}

u16 OutputInstruction(char *Line, u16 *ProgramAddress, label_hash_table *LabelHashTable)
{
    u16 Result = 0;    
    char Token[MaxTokenLength + 1];
    char *LineLastChar = (Line + strlen(Line) - 1);
    
    if(LineLastChar > Line)
    {
        Line = ExpectToken(Line, LineLastChar, Token);
        if(!strcmp(Token, "cls"))
        {
            Result = 0x00E0;
        }
        else if(!strcmp(Token, "ret"))
        {
            Result = 0x00EE;
        }
        else if(!strcmp(Token, "jp"))
        {
            Line = ExpectToken(Line, LineLastChar, Token);
            if(!strcmp(Token, "v0"))
            {
                Line = ExpectToken(Line, LineLastChar, Token);
                u16 Address = GetFromLabelTable(LabelHashTable, Token);
                Result = Compose13(0xB, Address);
            }
            else
            {
                u16 Address = GetFromLabelTable(LabelHashTable, Token);
                Result = Compose13(0x1, Address);
            }
            
            ExpectNoTokens(Line, LineLastChar, Token);
        }
        else if(!strcmp(Token, "call"))
        {
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 Address = GetFromLabelTable(LabelHashTable, Token);
            Result = Compose13(0x2, Address);
            ExpectNoTokens(Line, LineLastChar, Token);
        }
        else if(!strcmp(Token, "se"))
        {
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 RegX = GetRegister(Token);
            Line = ExpectToken(Line, LineLastChar, Token);
            if(Token[0] != 'v')
            {
                u16 Byte = TokenToShort(Token);
                Result = Compose112(0x3, RegX, Byte);
            }
            else
            {
                u16 RegY = GetRegister(Token);
                Result = Compose1111(5, RegX, RegY, 0);
            }
            ExpectNoTokens(Line, LineLastChar, Token);
        }
        else if(!strcmp(Token, "ld"))
        {
            // LD Vx, Vy    8xy0
            // LD Vx, DT    Fx07
            // LD Vx, K     Fx0A
            // LD Vx, [I]   Fx65
            // LD Vx, byte  6xkk

            Line = ExpectToken(Line, LineLastChar, Token);
            if(Token[0] == 'v')
            {
                u16 RegX = GetRegister(Token);
                Line = ExpectToken(Line, LineLastChar, Token);
                
                if(Token[0] == 'v')
                {
                    u16 RegY = GetRegister(Token);
                    Result = Compose1111(0x8, RegX, RegY, 0);
                }
                else if(!strcmp(Token, "dt"))
                {
                    Result = Compose1111(0xF, RegX, 0x0, 0x7);
                }
                else if(!strcmp(Token, "k"))
                {
                    Result = Compose1111(0xF, RegX, 0x0, 0xA);
                }
                else if(!strcmp(Token, "[i]"))
                {
                    Result = Compose1111(0xF, RegX, 0x6, 0x5);
                }
                else
                {
                    u16 Byte = TokenToShort(Token);
                    Result = Compose112(0x6, RegX, Byte);
                }
            }
            
            // LD I, addr   Annn
            // LD DT, Vx    Fx15
            // LD ST, Vx    Fx18
            // LD F, Vx     Fx29
            // LD B, Vx     Fx33
            // LD [I], Vx   Fx55
            
            else if(!strcmp(Token, "i"))
            {
                Line = ExpectToken(Line, LineLastChar, Token);
                u16 Address = GetFromLabelTable(LabelHashTable, Token);
                Result = Compose13(0xA, Address);
            }
            else if(!strcmp(Token, "dt"))
            {
                Line = ExpectToken(Line, LineLastChar, Token);
                u16 RegX = GetRegister(Token);
                Result = Compose1111(0xF, RegX, 0x1, 0x5);
            }
            else if(!strcmp(Token, "st"))
            {
                Line = ExpectToken(Line, LineLastChar, Token);
                u16 RegX = GetRegister(Token);
                Result = Compose1111(0xF, RegX, 0x1, 0x8);
            }
            else if(!strcmp(Token, "f"))
            {
                Line = ExpectToken(Line, LineLastChar, Token);
                u16 RegX = GetRegister(Token);
                Result = Compose1111(0xF, RegX, 0x2, 0x9);
            }
            else if(!strcmp(Token, "b"))
            {
                Line = ExpectToken(Line, LineLastChar, Token);
                u16 RegX = GetRegister(Token);
                Result = Compose1111(0xF, RegX, 0x3, 0x3);
            }
            else if(!strcmp(Token, "[i]"))
            {
                Line = ExpectToken(Line, LineLastChar, Token);
                u16 RegX = GetRegister(Token);
                Result = Compose1111(0xF, RegX, 0x5, 0x5);
            }
            else
            {
                InvalidCodePath;
            }
            ExpectNoTokens(Line, LineLastChar, Token);
        }
        else if(!strcmp(Token, "add"))
        {
            Line = ExpectToken(Line, LineLastChar, Token);
            
            if(Token[0] == 'v')
            {
                u16 RegX = GetRegister(Token);
                Line = ExpectToken(Line, LineLastChar, Token);
                if(Token[0] == 'v')
                {
                    u16 RegY = GetRegister(Token);
                    Result = Compose1111(0x8, RegX, RegY, 0x4);
                }
                else
                {
                    u16 Byte = TokenToShort(Token);
                    Result = Compose112(0x7, RegX, Byte);
                }
            }
            else if(!strcmp(Token, "i"))
            {
                Line = ExpectToken(Line, LineLastChar, Token);
                u16 RegX = GetRegister(Token);
                Result = Compose1111(0xF, RegX, 0x1, 0xE);
            }
            else
            {
                InvalidCodePath;
            }
            
            ExpectNoTokens(Line, LineLastChar, Token);
        }
        else if(!strcmp(Token, "or"))
        {
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 RegX = GetRegister(Token);
            
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 RegY = GetRegister(Token);
            
            Result = Compose1111(0x8, RegX, RegY, 0x1);
            ExpectNoTokens(Line, LineLastChar, Token);
        }
        else if(!strcmp(Token, "and"))
        {
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 RegX = GetRegister(Token);
            
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 RegY = GetRegister(Token);
            
            Result = Compose1111(0x8, RegX, RegY, 0x2);
            ExpectNoTokens(Line, LineLastChar, Token);
        }
        else if(!strcmp(Token, "xor"))
        {
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 RegX = GetRegister(Token);
            
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 RegY = GetRegister(Token);
            
            Result = Compose1111(0x8, RegX, RegY, 0x3);
            ExpectNoTokens(Line, LineLastChar, Token);
        }
        else if(!strcmp(Token, "sub"))
        {
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 RegX = GetRegister(Token);
            
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 RegY = GetRegister(Token);
            
            Result = Compose1111(0x8, RegX, RegY, 0x5);
            ExpectNoTokens(Line, LineLastChar, Token);
        }
        else if(!strcmp(Token, "shr"))
        {
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 RegX = GetRegister(Token);
            
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 RegY = GetRegister(Token);
            
            Result = Compose1111(0x8, RegX, RegY, 0x6);
            ExpectNoTokens(Line, LineLastChar, Token);
        }
        else if(!strcmp(Token, "subn"))
        {
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 RegX = GetRegister(Token);
            
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 RegY = GetRegister(Token);
            
            Result = Compose1111(0x8, RegX, RegY, 0x7);
            ExpectNoTokens(Line, LineLastChar, Token);
        }
        else if(!strcmp(Token, "shl"))
        {
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 RegX = GetRegister(Token);
            
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 RegY = GetRegister(Token);
            
            Result = Compose1111(0x8, RegX, RegY, 0xE);
            ExpectNoTokens(Line, LineLastChar, Token);
        }
        else if(!strcmp(Token, "sne"))
        {
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 RegX = GetRegister(Token);
            
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 RegY = GetRegister(Token);
            
            Result = Compose1111(0x9, RegX, RegY, 0x0);
            ExpectNoTokens(Line, LineLastChar, Token);
        }
        else if(!strcmp(Token, "rnd"))
        {
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 RegX = GetRegister(Token);
            
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 Byte = TokenToShort(Token);
            
            Result = Compose112(0xC, RegX, Byte);
            ExpectNoTokens(Line, LineLastChar, Token);
        }
        else if(!strcmp(Token, "drw"))
        {
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 RegX = GetRegister(Token);
            
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 RegY = GetRegister(Token);
            
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 Bytes = TokenToShort(Token);
            
            Result = Compose1111(0xD, RegX, RegY, Bytes);
            ExpectNoTokens(Line, LineLastChar, Token);
        }
        else if(!strcmp(Token, "skp"))
        {
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 RegX = GetRegister(Token);
            
            Result = Compose1111(0xE, RegX, 0x9, 0xE);
            ExpectNoTokens(Line, LineLastChar, Token);
        }
        else if(!strcmp(Token, "sknp"))
        {
            Line = ExpectToken(Line, LineLastChar, Token);
            u16 RegX = GetRegister(Token);
            
            Result = Compose1111(0xE, RegX, 0xA, 0x1);
            ExpectNoTokens(Line, LineLastChar, Token);
        }
        else if(TokenContainsChar(Token, ':'))
        {
            // NOTE(lex): Ignore label
        }
        else if(strlen(Token) == 4)
        {
            // NOTE(lex): To insert bitmaps
            u16 RawBytes = TokenToShort(Token);
            Result = RawBytes;
        }
        else
        {
            InvalidCodePath;
        }
    }
    
    return Result;
}

void GetLabelAddresses(label_hash_table *LabelHashTable, file_contents SourceFileHandle)
{
    char *CurrentCharacter = (char *)SourceFileHandle.Memory;
    char *OnePastLastCharacter = (char *)SourceFileHandle.Memory + SourceFileHandle.Size;
    
    char InputLine[MaxLineLength];
    int LineInsertIndex = 0;
    ClearArray(InputLine);
    
    u16 ProgramAddress = 0x200;
    char Token[MaxTokenLength + 1];
    
    while(CurrentCharacter <= OnePastLastCharacter)
    {
        while((*CurrentCharacter != '\n') && (CurrentCharacter != OnePastLastCharacter))
        {
            InputLine[LineInsertIndex++] = *CurrentCharacter++;
        }
        CurrentCharacter++;
        
        Assert(LineInsertIndex < ArrayCount(InputLine));
        InputLine[LineInsertIndex] = 0;
        
        // NOTE(lex): Ignore empty lines
        if(strlen(InputLine) >= 1)
        {
            char *LastCharacter = InputLine + strlen(InputLine) - 1;
            GetNextToken(InputLine, LastCharacter, Token);
            
            if(TokenContainsChar(Token, ':'))
            {
                Token[strlen(Token) - 1] = 0;
                AddToLabelTable(LabelHashTable, Token, ProgramAddress);
            }
            else
            {
                ProgramAddress += 2;
            }
        }
        
        LineInsertIndex = 0;
        ClearArray(InputLine);
    }
}

int main(int argc, char *argv[])
{
    u32 OutputProgramBytes = KiloBytes(3);
    u16 *OutputProgram = (u16 *)malloc(OutputProgramBytes);
    int OutputProgramInstructions = 0;
    memset(OutputProgram, 0, OutputProgramBytes);
    
    char SourceFile[1024];
    char DestFile[1024];
    
    if(argc >= 2)
    {
        strncpy(SourceFile, argv[1], ArrayCount(SourceFile));
        
        char *FileExtension = argv[1];
        while(*FileExtension != '.')
        {
            FileExtension++;
        }
        FileExtension++;
        
        strcpy(FileExtension, "chip8_exe");
        strncpy(DestFile, argv[1], ArrayCount(DestFile));
    }
    else
    {
        strcpy(SourceFile, "test.chip8_source");
        strcpy(DestFile, "test.chip8_exe");
    }
    
    
    file_contents SourceFileHandle = PlatformReadFile(SourceFile);
    
    char *CurrentCharacter = (char *)SourceFileHandle.Memory;
    char *OnePastLastCharacter = (char *)SourceFileHandle.Memory + SourceFileHandle.Size;
    
    char InputLine[MaxLineLength];
    int LineInsertIndex = 0;
    ClearArray(InputLine);
    
    u16 ProgramAddress = 0x200;
    label_hash_table LabelHashTable = CreateLabelHashTable();
    GetLabelAddresses(&LabelHashTable, SourceFileHandle);
    
    while(CurrentCharacter <= OnePastLastCharacter)
    {
        while((*CurrentCharacter != '\n') && (CurrentCharacter != OnePastLastCharacter))
        {
            InputLine[LineInsertIndex++] = *CurrentCharacter++;
        }
        CurrentCharacter++;
        
        Assert(LineInsertIndex < ArrayCount(InputLine));
        InputLine[LineInsertIndex] = 0;
    
        u16 Instruction = OutputInstruction(InputLine, &ProgramAddress, &LabelHashTable);
        if(Instruction)
        {
            // NOTE(lex): win32 is little endian whereas chip8 is big endian
            u16 ReversedInstruction = (Instruction >> 8) | (Instruction << 8);
            
            OutputProgram[OutputProgramInstructions++] = ReversedInstruction;
            ProgramAddress += 2;
        }
        
        LineInsertIndex = 0;
        ClearArray(InputLine);
    }
    
    PlatformWriteFile(DestFile, OutputProgram, OutputProgramBytes);
    PlatformFreeFile(SourceFileHandle.Memory);
}