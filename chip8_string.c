#include "stdlib.h"

typedef struct
{
    u8 *Base;
    int Bytes;
    int CurrentIndex;
} string;

internal string
CreateString(int Size)
{
    string String;
    String.Bytes = Size;
    String.Base = malloc(String.Bytes);
    memset(String.Base, 0, String.Bytes);
    String.CurrentIndex = 0;
    return String;
}

internal void
AddCharacter(string *String, char Character)
{
    Assert(String->CurrentIndex < String->Bytes);
    *(String->Base + String->CurrentIndex) = Character;
    String->CurrentIndex++;
}

internal void
FreeString(string String)
{
    free(String->Base);
}