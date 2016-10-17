#include "windows.h"

typedef struct
{
    u64 Size;
    void *Memory;
} file_contents;


void PlatformFreeFile(void *Memory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

file_contents PlatformReadFile(char *FileName)
{
    HANDLE FileHandle = CreateFile(FileName,
                                   GENERIC_READ,
                                   0,
                                   0,
                                   OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,
                                   0);
    
    file_contents Result = {0};
    
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD FileSize = GetFileSize(FileHandle, 0);
        Result.Size = FileSize;
        
        Result.Memory = VirtualAlloc(0, Result.Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE); 
        if(Result.Memory)
        {
            DWORD BytesRead;
            if(!ReadFile(FileHandle, Result.Memory, FileSize, &BytesRead, 0) ||
               BytesRead != FileSize)
            {
                VirtualFree(Result.Memory, 0, MEM_RELEASE);
                Result.Size = 0;
                Result.Memory = 0;
            }
            
            CloseHandle(FileHandle);
        }        
    }
    
    return Result;
}

b32 PlatformWriteFile(char *FileName, void *Memory, u32 BytesToWrite)
{
    b32 Result = false;
    
    HANDLE FileHandle = CreateFile(FileName,
                                   GENERIC_WRITE,
                                   0,
                                   0,
                                   CREATE_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL,
                                   0);
    
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if(WriteFile(FileHandle, Memory, BytesToWrite, &BytesWritten, 0))
        {
            if(BytesWritten == BytesToWrite)
            {
                Result = true;
            }
            else
            {
                // NOTE: Failure
            }
        }
        else
        {
            // NOTE: Failure
        }
        CloseHandle(FileHandle);
    }
    
    return Result;
}
