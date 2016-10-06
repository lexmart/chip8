#include "shared.h"
#include "chip8_string.c"

global variable char NextToken[64];

enum op_code
{
    CLS, SYS, RET, JP, CALL, SE, SNE, LD, ADD, OR, AND, XOR, SHR, SUBN, SHL, SNE, RND, DRW, SKP, SKPN
};

enum param_type
{
    Vx, I, F, B, Address, Byte, Nibble
};

struct instruction
{
    op_code Operation;
    param_type ParamType1, ParamType2;
    int Param1, Param2;
};

internal b32
SkipCharacter(char Character)
{
    return ((Input == ' ') ||  (Input == '\n') || (Input == ',') || (Input == ':'));
}

internal b32
ProcessCharacter(char Character)
{
    return ((Input != ' ') && (Input != '\n') && (Input != ',') && (Input != ':'));
}

internal char *
SkipToNextToken(char *Input)
{
    while(SkipCharacter(*Input))
    {
        Input++;
    }
}

internal string
GetNextToken(char *Input, char *Ouput, int OutputLength)
{
    string Token = CreateString(16);
    while(ProcessCharacter(*Input))
    {
        AddCharacter(Token, *Input++);
    }
    return Token;
}

internal line
ReadLine(char *Input)
{
    line Line = {0};
    string CurrentToken = CreateString(16);
    Input = SkipToNextToken(Input);
    
    FreeString(CurrentToken);
}

int main()
{
    char Program[64] = "CLS\n";
    char *Input = CurrentLine = Program;
}