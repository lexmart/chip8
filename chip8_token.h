
b32 TokenContainsChar(char *Token, char Char)
{
    while(*Token)
    {
        if(*Token++ == Char)
        {
            return true;
        }
    }
    
    return false;
}

u16 TokenToShort(char *Token)
{
    u16 Result = 0;
    sscanf_s(Token, "%x", &Result);
    
    return Result;
}

u16 GetRegister(char *Token)
{
    Assert((*Token == 'v') && (strlen(Token) == 2));
    u16 Result = 0;
    sscanf_s(++Token, "%x", &Result);
    
    return Result;
}

b32 IsJunkCharacter(char Character)
{
    b32 Result = false;
    
    char JunkCharacters[] = {' ', '\n', ','};
    
    for(int Index = 0; Index < ArrayCount(JunkCharacters); Index++)
    {
        if(Character == JunkCharacters[Index])
        {
            Result = true;
        }
    }
    
    return Result;
}

char *GetNextToken(char *Input, char *InputLastChar, char *Output)
{
    char *Result = 0;
    
    while(IsJunkCharacter(*Input))
    {
        if(Input < InputLastChar)
        {
            Input++;
        }
        else
        {
            break;
        }
    }
    
    int OutputIndex = 0;
    while((Input <= InputLastChar) && !IsJunkCharacter(*Input))
    {
        if(OutputIndex < MaxTokenLength)
        {
            *(Output + OutputIndex) = *Input++;
            OutputIndex++;
        }
        else
        {
            InvalidCodePath;
        }
        
        Result = Input;
    }
    *(Output + OutputIndex) = 0;
    
    return Result;
}

char *ExpectToken(char *Input, char *InputLastChar, char *Output)
{
    char *Result = GetNextToken(Input, InputLastChar, Output);
    if(Result == 0)
    {
        InvalidCodePath;
    }
    
    return Result;
}

char *ExpectNoTokens(char *Input, char *InputLastChar, char *Output)
{
    char *Result = GetNextToken(Input, InputLastChar, Output);
    if(Result != 0)
    {
        InvalidCodePath;
    }
    
    return Result;
}