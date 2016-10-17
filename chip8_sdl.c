#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "SDL.h"
#include "SDL_keyboard.h"
#include "SDL_keycode.h"
#include "shared.h"

global_variable b32 Running;

CHIP8_CYCLE;

void HandleInput(b32 *Input, SDL_KeyboardEvent *KeyEvent)
{
    b32 KeyDown = (KeyEvent->type == SDL_KEYDOWN);
    
#define KEY_CASE(ActualKey, RemappedKey) case SDLK_##ActualKey: { Input[RemappedKey] = KeyDown; } break;
    switch(KeyEvent->keysym.sym)
    {
        case SDLK_ESCAPE:
        {
            Running = false;
        } break;
        
        KEY_CASE(x, 0);
        KEY_CASE(1, 1);
        KEY_CASE(2, 2);
        KEY_CASE(3, 3);
        KEY_CASE(q, 4);
        KEY_CASE(w, 5);
        KEY_CASE(e, 6);
        KEY_CASE(a, 7);
        KEY_CASE(s, 8);
        KEY_CASE(d, 9);
        KEY_CASE(z, 10);
        KEY_CASE(c, 11);
        KEY_CASE(4, 12);
        KEY_CASE(r, 13);
        KEY_CASE(f, 14);
        KEY_CASE(v, 15);
    }
}

int main(int argc, char *argv[])
{
    int ScreenWidth = 640;
    int ScreenHeight = 320;
    
    if(SDL_Init(SDL_INIT_VIDEO) >= 0)
    {
        SDL_Window *Window = SDL_CreateWindow("chip8 emulator", 
                                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                              ScreenWidth, ScreenHeight, SDL_WINDOW_SHOWN);
        if(Window)
        {
            SDL_Surface *WindowScreen = SDL_GetWindowSurface(Window);
            
            u32 RedMask = 0xFF000000;
            u32 GreenMask = 0x00FF0000;
            u32 BlueMask = 0x0000FF00;
            u32 AlphaMask = 0x000000FF;
            SDL_Surface *GameScreen = SDL_CreateRGBSurface(0, 64, 32, 32, 
                                                           RedMask, GreenMask, BlueMask, AlphaMask);
            
            screen Screen;
            Screen.Memory = (u32 *)GameScreen->pixels;
            Screen.Width = GameScreen->w;
            Screen.Height = GameScreen->h;
            
            for(int PixelIndex = 0; PixelIndex < Screen.Width*Screen.Height; PixelIndex++)
            {
                u32 *Pixel = Screen.Memory + PixelIndex;
                *Pixel = BLACK;
            }
            
            memory_block MainMemory;
            MainMemory.Bytes = 4096;
            MainMemory.Base = (u8 *)malloc(MainMemory.Bytes); 
            memset(MainMemory.Base, 0, MainMemory.Bytes);
            
            emulator_state State;
            memset(&State, 0, sizeof(emulator_state));
            if(argc >= 2)
            {
                strncpy(&State.SourceFile[0], argv[1], ArrayCount(State.SourceFile));
            }
            else
            {
                strcpy(&State.SourceFile[0], "test.chip8_exe");
            }
            
            b32 Input[16] = {0};
            
            Running = true;
            while(Running)
            {
                SDL_Event Event;
                while(SDL_PollEvent(&Event) != 0)
                {
                    switch(Event.type)
                    {
                        case SDL_QUIT:
                        {
                            Running = false;
                        } break;
                        
                        case SDL_KEYDOWN:
                        {
                            HandleInput(Input, &Event.key);
                        } break;
                        
                        case SDL_KEYUP:
                        {
                            HandleInput(Input, &Event.key);
                        } break;
                    }
                }
                
                SDL_Rect SourceRect = {0};
                SourceRect.w = GameScreen->w;
                SourceRect.h = GameScreen->h;
                
                SDL_Rect DestRect = {0};
                DestRect.w = WindowScreen->w;
                DestRect.h = WindowScreen->h;
                
                b32 RedrawScreen = Chip8Cycle(&State, &MainMemory, &Screen, Input);
                if(RedrawScreen)
                {
                    SDL_BlitScaled(GameScreen, &SourceRect, WindowScreen, &DestRect);
                }
                
                SDL_UpdateWindowSurface(Window);
                // TODO(lex): What is the clock speed? 60 HZ? <- That's the speed at which timer goes down
                // TODO(lex): Need to write code to take into account how long processing took
                SDL_Delay(5);
            }
            
            SDL_DestroyWindow(Window);
            SDL_Quit();
        }
        else
        {
            printf("failed to create window, error = %s\n", SDL_GetError());
        }
    }
    else
    {
        printf("failed to init window, error = %s\n", SDL_GetError());
    }
    
    return 0;
}