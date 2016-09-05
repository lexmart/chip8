global variable char NextToken[64];

internal char *
SkipToNextToken(char *Input)
{
    while((Input == ' ') || (Input == '\n') || (Input == ',') || (Input == ':'))
    {
        Input++;
    }
}

internal char *
GetNextToken(char *Input)
{
    SkipToNextToken(Input);
    if(Input == EOF)
    {
        return 0;
    }
    
    char *CurrentChar = CurrentLine;
    
}

int main()
{
    char Program[64] = "CLS\n";
    char *Input = CurrentLine = Program;
    
}