
typedef struct label
{
    char Token[MaxTokenLength + 1];
    u16 Address;
    struct label *NextLabel;
} label;

typedef struct
{
    label *Buckets[64];
} label_hash_table;

label_hash_table CreateLabelHashTable()
{
    label_hash_table LabelHashTable = {0};
    return LabelHashTable;
}

// TODO(lex): This is REALLY dumb
int HashLabel(char *Label, u32 NumBuckets)
{
    int HashValue = 0;
    
    int LabelLength = (int)strlen(Label);
    for(int CharIndex = 0; CharIndex < LabelLength; CharIndex++)
    {
        HashValue += Label[CharIndex];
    }
    
    HashValue = HashValue % NumBuckets;
    
    return HashValue;
}

void AddToLabelTable(label_hash_table *LabelHashTable, char *Label, u16 Address)
{
    int HashValue = HashLabel(Label, ArrayCount(LabelHashTable->Buckets));
    
    label *NewLabel = malloc(sizeof(label));
    strncpy(NewLabel->Token, Label, MaxTokenLength);
    NewLabel->NextLabel = LabelHashTable->Buckets[HashValue];
    NewLabel->Address = Address;
    LabelHashTable->Buckets[HashValue] = NewLabel;
}

u16 GetFromLabelTable(label_hash_table *LabelHashTable, char *LabelString)
{
    u16 Address = 0;
    
    int HashValue = HashLabel(LabelString, ArrayCount(LabelHashTable->Buckets));
    
    for(label *Label = LabelHashTable->Buckets[HashValue]; Label != 0; Label++)
    {
        if(!strcmp(&Label->Token[0], LabelString))
        {
            Address = Label->Address;
            break;
        }
    }
    
    if(Address == 0)
    {
        Assert(!"Label was not found");
    }

    return Address;
}